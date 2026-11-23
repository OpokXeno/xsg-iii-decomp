#include "common.h"

#include "shared.h"
#include "quadword.h"
typedef int s32;

typedef unsigned short u16;

typedef float f32;

typedef unsigned char u8;

static inline void CopyVectorMemOut(AlignedVector4 *destination, const AlignedVector4 *source) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z),
                           "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

typedef struct ModeEffectRequest {
    unsigned char unmodeled_00[0x2C];
    s32 eventId; /* +0x2C */
    unsigned char unmodeled_30[0x40 - 0x30];
    AlignedVector4 from;   /* +0x40: copy of the slot vector at +0x40 */
    AlignedVector4 to;     /* +0x50: copy of the slot vector at +0x60 */
    AlignedVector4 offset; /* +0x60: the VU0 result */
    unsigned char unmodeled_70[0x80 - 0x70];
} ModeEffectRequest;

/* The model a mode slot drives (+0x24 of the slot). */
typedef struct ModeModel {
    unsigned char unmodeled_00[0x20];
    union {
        Vector4 position; /* +0x20 */
        struct {
            unsigned char unmodeled_00[4];
            union {
                f32 value;
                s32 bits;
            } y; /* +0x24: kept as a union with the word view: the access is alias set 0 and decides the emitted order */
        } coordinates;
    };
    unsigned char unmodeled_30[0x4EC - 0x30];
    s32 stateFlags; /* +0x4EC: bit 0x1 cleared when the trigger fires */
} ModeModel;

/* The mode block of the slot at +0x150. */
typedef struct ModeTimerBlock {
    u16 modeFlagBits; /* +0x00: 0x10 = trigger done, 0x40 = waiting for the model, 0x50 set by modeSlotTrigger */
    u8 unmodeled_02[0x08 - 0x02];
    s32 scanBound; /* +0x08 */
    u8 unmodeled_0C[0x18 - 0x0C];
    f32 scale; /* +0x18 */
    s32 timer; /* +0x1C */
} ModeTimerBlock;

/* Slot object allocated by RgDispModelAlloc (stride 0x230, see rg_dispmodel.c). */
typedef struct ModeSlot {
    s32 flags; /* +0x00 */
    u8 unmodeled_04[0x1C - 0x04];
    void (*update)(); /* +0x1C: the state handler the slot calls */
    s32 unmodeled_20;
    ModeModel *model;  /* +0x24 */
    s32 lastScanIndex; /* +0x28 */
    u8 unmodeled_2C[0x34 - 0x2C];
    s32 foundSlotKind; /* +0x34: slot kind handed to RgDispModelFindByKey */
    u8 unmodeled_38[0x40 - 0x38];
    AlignedVector4 position; /* +0x40 */
    u8 unmodeled_50[0x60 - 0x50];
    AlignedVector4 target; /* +0x60 */
    u8 unmodeled_70[0x12C - 0x70];
    f32 unmodeled_12C; /* +0x12C */
    f32 unmodeled_130; /* +0x130 */
    f32 maxDistance;   /* +0x134 */
    u8 unmodeled_138[0x150 - 0x138];
    ModeTimerBlock modeTimer; /* +0x150 */
} ModeSlot;

void sefObjectSetStatePlay(ModeEffectRequest *request);
void func_001E9888(ModeEffectRequest *request);
void seVoiceStartDefault(unsigned int handle);
ModeSlot *RgDispModelFindByKey(s32 mode);
s32 SpawnSlotSetRotation(ModeSlot *slot, AlignedVector4 *vector);
s32 sbNativeHandlerMessagePost(s32 mode, s32 index, s32 option);
s32 SpawnSlotSetPosition(ModeSlot *slot, AlignedVector4 *position);

static inline void StoreOffsetVector(AlignedVector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %1\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%0)"
        :
        : "r"(destination), "f"(value)
        : "$8", "memory");
}

/* Fires the slot's trigger: spawns the effect (event 0x10064023) and plays voice 0x18, then releases the found slot. */
void modeSlotTrigger(ModeSlot *slot, s32 silent) {
    ModeEffectRequest request;
    ModeTimerBlock *modeTimer;
    ModeSlot *foundSlot;

    modeTimer = &slot->modeTimer;
    modeTimer->modeFlagBits |= 0x50;
    slot->model->stateFlags &= ~1;
    if (silent != 0) {
        return;
    }
    sefObjectSetStatePlay(&request);
    request.eventId = 0x10064023;
    CopyVectorMemOut(&request.from, &slot->position);
    CopyVectorMemOut(&request.to, &slot->target);
    StoreOffsetVector(&request.offset, modeTimer->scale);
    func_001E9888(&request);
    seVoiceStartDefault(0x18);
    if (slot->foundSlotKind != 0) {
        foundSlot = RgDispModelFindByKey(slot->foundSlotKind);
        if (foundSlot != 0) {
            foundSlot->flags |= 0x40;
            SpawnSlotSetPosition(foundSlot, &slot->position);
            SpawnSlotSetRotation(foundSlot, &slot->target);
            sbNativeHandlerMessagePost(2, slot->lastScanIndex, 0);
        }
        slot->foundSlotKind = 0;
    }
}

void XenoActorIdleHandler(void) {

}

/* Lookup request filled for actorScanLookupStart: the next index after the slot's lastScanIndex at 0x28, a mode of 1, the
 * bound from 0x158 and the found flag the lookup sets. */
struct ModeScanRequest {
    s32 scanFromIndex; /* 0x00 */
    s32 scanMode;      /* 0x04 */
    s32 bound;         /* 0x08 */
    s32 found;         /* 0x0C */
};

s32 actorScanLookupStart(struct ModeScanRequest *scanRequest);

s32 XenoActorAdvanceLookup(ModeSlot *slot) {
    struct ModeScanRequest scanRequest;
    s32 bound;
    s32 scanOutcome;
    s32 lookupHandled;

    bound = slot->modeTimer.scanBound;
    scanRequest.scanMode = 1;
    scanRequest.scanFromIndex = slot->lastScanIndex + 1;
    scanRequest.bound = bound;
    scanOutcome = actorScanLookupStart(&scanRequest);
    lookupHandled = scanOutcome;
    if (lookupHandled != 0) {
        if (scanRequest.found != 0) {
            slot->foundSlotKind = scanRequest.scanFromIndex;
        } else {
            modeSlotTrigger(slot, 1);
        }
        lookupHandled = 1;
    }
    return lookupHandled;
}

extern u16 gPadDataPressed[];
f32 MMathCalcDist(const Vector4 *fromPoint, const Vector4 *toPoint);
f32 actorFacingError(ModeModel *model, const Vector4 *position);
s32 modeSlotTriggerCheck(ModeSlot *slot);

s32 modeSlotTriggerCheck(ModeSlot *slot) {
    ModeSlot *player;
    ModeTimerBlock *modeTimer;
    u16 padPressed;

    player = RgDispModelFindByKey(0);
    padPressed = gPadDataPressed[0];
    modeTimer = &slot->modeTimer;
    if (player == 0) {
        return 0;
    }
    if (modeTimer->modeFlagBits & 0x40) {
        return 0;
    }
    if (!(padPressed & 0x20)) {
        return 0;
    }
    if (slot->maxDistance < MMathCalcDist(&slot->position, &player->position)) {
        return 0;
    }
    if (0.5235988f < actorFacingError(slot->model, &player->position)) {
        return 0;
    }
    if (0.78539819f < actorFacingError(player->model, &slot->position)) {
        return 0;
    }
    return 1;
}

s32 XenoActorIsDone(ModeModel *model);
f32 func_0027DEF0(ModeModel *model);
s32 slotFollowTargetHeight(ModeSlot *slot, f32 arg);

void modeSlotWaitHandler(ModeSlot *slot) {
    ModeTimerBlock *modeTimer;
    ModeModel *model;

    modeTimer = &slot->modeTimer;
    model = slot->model;
    if (slotFollowTargetHeight(slot, func_0027DEF0(model)) != 0) {
        SpawnSlotSetPosition(slot, &model->position);
    }
    /* The do/while(0) block keeps the join before the epilogue: without it the call below becomes a tail call. */
    do {
        if (modeTimer->modeFlagBits & 0x40) {
            if (XenoActorIsDone(model) != 0) {
                modeTimer->modeFlagBits &= 0xFFBF;
            }
        } else if ((modeSlotTriggerCheck(slot) != 0) && (!(modeTimer->modeFlagBits & 0x10))) {
            modeSlotTrigger(slot, 0);
        }
    } while (0);
}

void XenoActorStepModeTimer(ModeSlot *slot);

s32 slotChildStateSelect(ModeSlot *slot, s32 mode);

void modeSlotActivate(ModeSlot *slot) {
    ModeTimerBlock *modeTimer;
    s32 flags;
    f32 step;
    f32 scaled;

    flags = slot->flags;
    if (flags & 0x40) {
        modeTimer = &slot->modeTimer;
        step = modeTimer->scale * 0.2f;
        slot->flags = (flags & ~0x48) | 0x11000;
        slot->update = XenoActorStepModeTimer;
        slot->position.y += step;
        slot->model->coordinates.y.value += step;
        scaled = modeTimer->scale * 0.27f;
        slot->unmodeled_130 = 0.5f;
        slot->unmodeled_12C = scaled;
        slotChildStateSelect(slot, 1);
        modeTimer->timer = 30;
    }
}

void XenoActorStepModeTimer(ModeSlot *slot) {
    ModeTimerBlock *modeTimer;
    ModeModel *model;
    s32 framesLeft;

    modeTimer = &slot->modeTimer;
    model = slot->model;
    if (slotFollowTargetHeight(slot, func_0027DEF0(model)) != 0) {
        SpawnSlotSetPosition(slot, &model->position);
    }
    framesLeft = modeTimer->timer;
    if (framesLeft != 0) {
        modeTimer->timer = framesLeft - 1;
    } else {
        /* The do/while(0) block keeps the join before the epilogue: without it the call below becomes a tail call. */
        do {
            if (slot->modeTimer.modeFlagBits & 0x40) {
                if (XenoActorIsDone(model) != 0) {
                    slot->modeTimer.modeFlagBits &= 0xFFBF;
                }
            } else if ((modeSlotTriggerCheck(slot) != 0) && (!(slot->modeTimer.modeFlagBits & 0x10))) {
                modeSlotTrigger(slot, 0);
            }
        } while (0);
    }
}

void modeSlotSwitchedHandler(ModeSlot *slot);

void XenoActorSwitchHandler(ModeSlot *slot) {
    s32 clearedFlags;
    s32 slotFlags;

    slotFlags = slot->flags;
    if (slotFlags & 0x40) {
        clearedFlags = slotFlags & (~0x48);
        slot->flags = clearedFlags;
        slot->update = modeSlotSwitchedHandler;
        slotChildStateSelect(slot, 1);
    }
}

void modeSlotSwitchedHandler(ModeSlot *slot) {
    ModeTimerBlock *modeTimer;
    ModeModel *model;

    modeTimer = &slot->modeTimer;
    model = slot->model;
    if (slot->flags & 0x10000) {
        if (slotFollowTargetHeight(slot, func_0027DEF0(model)) != 0) {
            SpawnSlotSetPosition(slot, &model->position);
        }
    }
    /* The do/while(0) block keeps the join before the epilogue: without it the call below becomes a tail call. */
    do {
        if (modeTimer->modeFlagBits & 0x40) {
            if (XenoActorIsDone(model) != 0) {
                modeTimer->modeFlagBits &= 0xFFBF;
            }
        } else if ((modeSlotTriggerCheck(slot) != 0) && (!(modeTimer->modeFlagBits & 0x10))) {
            modeSlotTrigger(slot, 0);
        }
    } while (0);
}
