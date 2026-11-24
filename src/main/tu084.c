#include "common.h"
#include "quadword.h"
#include "vu0_macro.h"
#include "main/sound_driver.h"
#include "shared.h"

typedef unsigned char u8;

typedef signed char s8;

typedef short s16;

typedef unsigned short u16;

typedef int s32;

typedef float f32;

/* Stored kind and flag words of a slot object; only the declared width matters. */
enum SlotKind {
    SLOT_KIND_NONE = 0,
    SLOT_KIND_WIDTH = 0x7fffffff
};

enum SlotFlags {
    SLOT_FLAGS_NONE = 0,
    SLOT_FLAGS_WIDTH = 0x7fffffff
};

/* The model (actor) a spawned slot object draws, allocated by func_0028D670. */
typedef struct SpawnedModel {
    s32 unmodeled_00;
    s32 active; /* +0x04 */
    u8 unmodeled_08[8];
    union {
        Vector4 vector;
        s32 words[4];
    } extents; /* +0x10: (half scale, 0.15, 0, half scale) */
    Vector4 position; /* +0x20 */
    Vector4 unmodeled_30;
    Vector4 unmodeled_40;
    Vector4 rotation; /* +0x50 */
    Vector4 unmodeled_60;
    Vector4 unmodeled_70;
    Vector4 scale; /* +0x80 */
    s32 drawFlags; /* +0x90: bits 0xC set on creation */
    u8 unmodeled_94[0x6a0 - 0x94];
    u8 collisionObjectHead[0x6aa - 0x6a0]; /* +0x6A0, passed to func_002A70E8 */
    s16 collisionBits;                      /* +0x6AA */
    u8 unmodeled_6AC[0x6b3 - 0x6ac];
    s8 collisionProbeMode; /* +0x6B3 */
    u8 unmodeled_6B4[0x6bc - 0x6b4];
    s32 collisionBlock; /* +0x6BC */
    u8 unmodeled_6C0[0x6cc - 0x6c0];
    f32 copiedScale; /* +0x6CC: copy of the value at +0x10 */
} SpawnedModel;

typedef struct SpawnSlotState {
    s32 spawnParam; /* +0x0: the request word at +0x10 */
    s32 unmodeled_04;
    s32 unmodeled_08;
    s32 clearedWord; /* +0xC: cleared on creation */
} SpawnSlotState;

/* Slot object allocated by RgDispModelAlloc (stride 0x230, see rg_dispmodel.c). */
typedef struct SpawnSlot {
    enum SlotFlags flags;
    u8 unmodeled_04[0xC];
    enum SlotKind kind; /* +0x10 */
    u8 unmodeled_14[8];
    void (*update)();  /* +0x1C */
    void (*release)(); /* +0x20 */
    SpawnedModel *model; /* +0x24 */
    s32 slotId;          /* +0x28: the id from RgDispModelNextId */
    s32 objectId;        /* +0x2C */
    u8 unmodeled_30[0x134 - 0x30];
    f32 halfScale; /* +0x134 */
    u8 unmodeled_138[0x150 - 0x138];
    SpawnSlotState state; /* +0x150 */
} SpawnSlot;

typedef struct SpawnRequest {
    Vector4 position;
    s32 spawnParam; /* +0x10 */
    s32 layerId;      /* +0x14 */
} SpawnRequest;

/* The part of the game loop runtime this TU reads. */
typedef struct GameLoopRuntimeView {
    s32 flags;
    u8 unmodeled_04[0x168];
    f32 spawnUnitScale; /* +0x16C */
    u16 spawnedCount;   /* +0x170: incremented for every slot spawned here */
    u8 unmodeled_172[0x100];
} GameLoopRuntimeView;

extern GameLoopRuntimeView gGameLoopRuntimeStorage;

s32 RgDispModelNextId(void);

SpawnSlot *RgDispModelAlloc(s32 id);

SpawnedModel *func_0028D670(void);

void RgDispModelSetLocal();

s32 func_001B93C0(s32 index);

void payloadSectionsLocate(SpawnedModel *model, s32 value);

void func_001C7C48();

void actorChildBindLayer(SpawnSlot *slot, s32 layerId);

static inline void ZeroVectorMemory(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

static inline void ZeroVector(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void CopyVectorT0(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(destination), "r"(source) : "$8", "memory");
}

static inline void BroadcastScalarXYZ(Vector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %5\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%4)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
        : "r"(destination), "f"(value)
        : "$8");
}

/* Spawns a slot of kind 0x10000 with a small flat model at the request position, scaled by the runtime's unit scale. */
s32 flatSlotSpawn(SpawnRequest *request) {
    SpawnSlot *slot;
    SpawnedModel *model;
    s32 slotId;

    slotId = RgDispModelNextId();
    slot = RgDispModelAlloc(slotId);
    if (slot == 0) {
        return 0;
    }
    {
        SpawnSlotState *state = &slot->state;
        slot->kind = 0x10000;
        slot->slotId = slotId;
        slot->halfScale = gGameLoopRuntimeStorage.spawnUnitScale * 3.2f;
        slot->update = func_001C7C48;
        state->spawnParam = request->spawnParam;
        state->clearedWord = 0;
        model = func_0028D670();
        slot->model = model;
        if (model == 0) {
            RgDispModelSetLocal(slot);
            return 0;
        }
        model->drawFlags |= 0xc;
        ZeroVectorMemory(&model->extents.vector);
        model->active = 0;
        model->extents.vector.x = model->extents.vector.w = gGameLoopRuntimeStorage.spawnUnitScale * 0.5f;
        model->extents.vector.y = 0.15f;
        CopyVectorT0(&model->position, &request->position);
        ZeroVector(&model->rotation);
        BroadcastScalarXYZ(&model->scale, gGameLoopRuntimeStorage.spawnUnitScale);
        payloadSectionsLocate(model, func_001B93C0(0x16));
        actorChildBindLayer(slot, request->layerId);
        gGameLoopRuntimeStorage.spawnedCount++;
        return 1;
    }
}

s32 func_002A6FA8();

void func_002A70E8();

/* Stores the layer id in the slot and restarts the model's collision block with it. */
void actorChildBindLayer(SpawnSlot *slot, s32 layerId) {
    f32 baseScale;
    s32 collisionBlock;
    s32 storedLayerId;
    SpawnedModel *model;

    storedLayerId = layerId;
    slot->objectId = storedLayerId;
    model = slot->model;
    if (model != 0) {
        func_002A70E8(model->collisionObjectHead);
        model->collisionBits = 0x102;
        model->collisionProbeMode = 3;
        collisionBlock = func_002A6FA8(0x100, slot->objectId);
        baseScale = model->extents.vector.x;
        model->copiedScale = baseScale;
        model->collisionBlock = collisionBlock;
    }
}

struct EffectActor;

typedef struct RgDispModel {
    s32 modelFlags; /* +0x00, bit 0x10000 is set here */
    unsigned char unmodeled_04[0x10 - 0x04];
    s32 kindBits; /* +0x10, ANDed with the iterator kind */
    unsigned char unmodeled_14[0x1C - 0x14];
    void (*handlerFn)(struct EffectActor *self); /* +0x1C */
    unsigned char unmodeled_20[0x230 - 0x20];
} RgDispModel;

RgDispModel *RgDispModelNextByKind(s32 kind, RgDispModel *previous);

void func_001C7E08(struct EffectActor *self);

/* Counts the slots of kind 0x10000 whose update is func_001C7E08, flagging each with 0x100000. */
s32 flatSlotMarkAll(void) {
    RgDispModel *model;
    s32 count;

    count = 0;
    model = 0;
    while ((model = RgDispModelNextByKind(0x10000, model)) != 0) {
        if (model->handlerFn == func_001C7E08) {
            model->modelFlags |= 0x100000;
            count++;
        }
    }
    return count;
}

typedef struct EffectProgress EffectProgress;

typedef struct EffectState EffectState;

typedef struct EffectActor EffectActor;

struct SefEffectObject;

/* Progress block of the effect actor at +0x150. */
struct EffectProgress {
    int startDelay; /* +0x00: frames before the first effect, counted down by func_001C7C48 */
    int hitCount;   /* +0x04: remaining pad hits, counted down by func_001C7CE8 */
    int timer;      /* +0x08: frames left of the final effect */
    struct SefEffectObject *effectObject; /* +0x0C: effect started by func_001BF848 */
};

/* State block of the effect actor (+0x24). */
struct EffectState {
    unsigned char unmodeled_00[0x10];
    union {
        float triggerRadius; /* +0x10 */
        int radiusWord;
    };
    unsigned char unmodeled_14[0xC];
    union {
        Vector4 probePoint; /* +0x20; y is written from the float result */
        int probeWords[4];
    };
    unsigned char unmodeled_30[0x60];
    union {
        int stateFlags; /* +0x90 */
        struct {
            u32 low2 : 2;
            u32 hidden : 1;
            u32 high : 29;
        } bits;
    };
    unsigned char unmodeled_94[0x6A0 - 0x94];
    unsigned char probeRecord[0x10]; /* +0x6A0: passed by address to the probe calls */
};

/* Slot object allocated by RgDispModelAlloc (stride 0x230, see rg_dispmodel.c). */
struct EffectActor {
    union {
        int flags; /* +0x00 */
        struct {
            u32 low21 : 21;
            u32 stageDone : 1;
            u32 high : 10;
        } bits;
    };
    unsigned char unmodeled_04[0x18];
    void (*update)(EffectActor *self); /* +0x1C */
    unsigned char unmodeled_20[4];
    EffectState *state; /* +0x24 */
    unsigned char unmodeled_28[0x18];
    Vector4 position; /* +0x40 */
    unsigned char unmodeled_50[0x100];
    EffectProgress progress; /* +0x150 */
};

struct SefEffectObject *func_001BF848(u32 id, Vector4 *position);

void func_001C7CE8(EffectActor *self);

/* First stage: after the start delay it plays voice 0x12 and the two opening effects, then hands over to func_001C7CE8. */
void func_001C7C48(EffectActor *self) {
    EffectProgress *progress = &self->progress;

    if (--progress->startDelay > 0) {
        return;
    }
    self->bits.stageDone = 1;
    self->state->bits.hidden = 0;
    self->update = func_001C7CE8;
    seVoiceStartDefault(0x12);
    func_001BF848(0x10064027, &self->position);
    progress->effectObject = func_001BF848(0x1006402C, &self->position);
}

void mapProbeFlagOn(void *source, void *id, void *value);

float func_002AA490(void *a0, void *a1, void *a2);

void func_001EAA08(void *a0, void *a1);

void sefEffectHandleRelease(struct SefEffectObject *effectObject);

void vibrationModeSet(int a0);

void func_001C7E08(EffectActor *self);

extern f32 D_003E481C[];

/* Second stage: probes the ground under the effect each frame and, on the pad press, counts down to the final effect. */
void func_001C7CE8(EffectActor *self) {
    Vector4 probe;
    int flags;
    int newFlags;
    int hitCount;
    EffectProgress *progress = &self->progress;
    EffectState *state = self->state;

    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&probe) : "memory");
    mapProbeFlagOn(&probe, &probe, state->probeRecord);
    state->probePoint.y = func_002AA490(&state->probePoint, &probe, state->probeRecord);
    func_001EAA08(progress->effectObject, &state->probePoint);
    flags = self->flags;
    if (!(flags & 0x20)) {
        return;
    }
    newFlags = flags & ~0x200000;
    hitCount = progress->hitCount - 1;
    self->flags = newFlags;
    progress->hitCount = hitCount;
    if (hitCount > 0) {
        return;
    }
    sefEffectHandleRelease(progress->effectObject);
    progress->effectObject = func_001BF848(0x10064028, &self->position);
    seVoiceStartDefault(0x13);
    vibrationModeSet(1);
    self->update = func_001C7E08;
    self->state->triggerRadius = D_003E481C[0] * 4.0f;
    self->state->stateFlags |= 4;
    progress->timer = 300;
}

typedef struct EffectListEntry {
    int flags; /* +0x00 */
    unsigned char unmodeled_04[0x24];
    int id28; /* +0x28 */
    unsigned char unmodeled_2C[0x40 - 0x2C];
    Vector4 position; /* +0x40 */
} EffectListEntry;

float MMathCalcDist(Vector4 *a, Vector4 *b);

void sefRunEffectFrames(struct SefEffectObject *effectObject, int timer, float remaining, float value);

void slotEffectStartOnce(void *entry, int mode);

void sbNativeHandlerMessagePost(int kind, int value, int extra);

/* Final stage: counts the timer down, damages the slots of kind 6 inside the radius, and releases itself at zero. */
void func_001C7E08(EffectActor *self) {
    EffectProgress *progress;
    EffectListEntry *entry;
    int timer;
    int remaining;
    int oldTimer;
    int remainingTimer;
    unsigned short *activeEffectCounter;
    int counterValue;

    progress = &self->progress;
    entry = 0;
    if (self->flags & 0x100000) {
        timer = progress->timer;
        remaining = timer - 30;
        self->flags = (self->flags & 0xFFEFFFFF) | 0x400000;
        if (remaining > 0) {
            progress->timer = timer - remaining;
            sefRunEffectFrames(progress->effectObject, timer, (float) remaining, 10.0f);
            oldTimer = progress->timer;
        } else {
            oldTimer = timer;
        }
    } else {
        oldTimer = progress->timer;
    }
    remainingTimer = oldTimer - 1;
    progress->timer = remainingTimer;
    if (remainingTimer <= 0) {
        activeEffectCounter = (unsigned short *) &gGameLoopRuntimeStorage;
        counterValue = activeEffectCounter[0xB8];
        activeEffectCounter[0xB8] = counterValue - 1;
        RgDispModelSetLocal(self, oldTimer);
        return;
    }
    if (!(self->flags & 0x400000)) {
        while ((entry = (EffectListEntry *) RgDispModelNextByKind(6, (RgDispModel *) entry)) != 0) {
            if (entry->flags & 0x2000) {
                continue;
            }
            if (!(entry->flags & 0x20)) {
                continue;
            }
            if (MMathCalcDist(&self->position, &entry->position) < self->state->triggerRadius) {
                slotEffectStartOnce(entry, 0);
                sbNativeHandlerMessagePost(5, entry->id28, 0);
            }
        }
    }
}
