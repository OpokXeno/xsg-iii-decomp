#include "common.h"
#include "shared.h"
#include "vu0_macro.h"
#include "main/xenoActor.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef signed char s8;
static s32 gActorSlotsReady;

/* 16 bytes at +0x10 of a slot, copied with ld/sd. */
typedef DoublewordAlignedVector4 SlotPlacement;

/* One 0x60-byte slot of the actor table at gActorSlots (8 slots); fields are at their +offsets. */
typedef struct ActorSlot {
    s8 loadedBits; /* +0x00: bit 0 = slot loaded, bit 1 = radius set by the caller. kept: signed, the field type decides the emitted access */
    u8 pad_01[0x3];
    struct XenoActor *actor; /* +0x04 */
    u8 pad_08[0x08];
    SlotPlacement placement; /* +0x10 */
    Vector4 vectors[3];      /* +0x20: indexed by k */
    f32 radius;              /* +0x50 */
    s32 level;               /* +0x54 */
    s32 extent;              /* +0x58 */
    u8 pad_5c[0x60 - 0x5C];
} ActorSlot;

/* Typed views of gActorSlots: element i of each view is the field of slot i (0x60-byte stride). */
typedef struct SlotActorView { struct XenoActor *actor; u8 pad_04[0x60 - 0x04]; } SlotActorView;
typedef struct SlotRadiusView { f32 radius; u8 pad_04[0x60 - 0x04]; } SlotRadiusView;
typedef struct SlotLevelView { s32 level; u8 pad_04[0x60 - 0x04]; } SlotLevelView;
typedef struct SlotExtentView { s32 extent; u8 pad_04[0x60 - 0x04]; } SlotExtentView;

static ActorSlot gActorSlots[8];
extern SlotActorView gActorSlotActorView[];
extern SlotRadiusView gActorSlotRadiusView[];
extern SlotLevelView gActorSlotLevelView[];
extern SlotExtentView gActorSlotExtentView[];
void *memset(void *destination, s32 value, unsigned int count);

void actorSlotsInit(void) {
    memset(gActorSlots, 0, 0x300);
    gActorSlotsReady = 1;
}

void actorSlotLoad(s32 actorSlotIndex);

/* Loads every slot from the battle actors and then clears gActorSlotsReady. */
void actorSlotsLoadAll(void) {
    s32 i;

    for (i = 0; i < 8; i++) {
        actorSlotLoad(i);
    }
    gActorSlotsReady = 0;
}

void actorSlotClampVector(AlignedVector4 *clampedPosition, struct XenoActor *actor, s32 tag);
struct XenoActor *battleSlotActorGet(s32 actorSlotIndex);
f32 battleSlotRadiusGet(s32 actorSlotIndex);
s32 battleCheckModeZero(s32 actorSlotIndex);
s32 battleCheckModeOne(s32 actorSlotIndex);
extern s8 gBattleModeActors[];

/* Loads slot actorSlotIndex from its battle actor unless bit 0 is set. Radius is computed unless bit 1 is set. */
void actorSlotLoad(s32 actorSlotIndex) {
    struct XenoActor *actor;

    actor = battleSlotActorGet(actorSlotIndex);
    gActorSlots[actorSlotIndex].actor = actor;
    if (actor == 0) {
        return;
    }
    if (gBattleModeActors[3] == 4) {
        gActorSlots[actorSlotIndex].placement = actor->position;
    }
    actorSlotClampVector(&gActorSlots[actorSlotIndex].vectors[0], gActorSlots[actorSlotIndex].actor, 32);
    actorSlotClampVector(&gActorSlots[actorSlotIndex].vectors[1], gActorSlots[actorSlotIndex].actor, 33);
    actorSlotClampVector(&gActorSlots[actorSlotIndex].vectors[2], gActorSlots[actorSlotIndex].actor, 34);
    if (gActorSlots[actorSlotIndex].loadedBits & 1) {
        return;
    }
    if (!(gActorSlots[actorSlotIndex].loadedBits & 2)) {
        gActorSlots[actorSlotIndex].radius = battleSlotRadiusGet(actorSlotIndex);
    }
    gActorSlots[actorSlotIndex].level = battleCheckModeZero(actorSlotIndex);
    gActorSlots[actorSlotIndex].extent = battleCheckModeOne(actorSlotIndex);
    gActorSlots[actorSlotIndex].loadedBits |= 1;
}

s32 battleActorMatrixLoad(AlignedVector4 *position);
f32 xglPointLength(AlignedVector4 *a, AlignedVector4 *b);
void xglVectorNormal(AlignedVector4 *dst, AlignedVector4 *src);
void xglVectorScale(AlignedVector4 *dst, AlignedVector4 *src, f32 scale);
extern s8 gBattleModeActors[];

/* Pulls *clampedPosition back to within limit of the actor position (limit is 2.0 in battle mode 0, else 0.2). */
void actorSlotClampVector(AlignedVector4 *clampedPosition, struct XenoActor *actor, s32 tag) {
    AlignedVector4 actorPosition;
    AlignedVector4 offsetFromPosition;
    f32 limit;

    if (!battleActorMatrixLoad(&actorPosition)) {
        return;
    }
    limit = 0.2f;
    if (gBattleModeActors[0] != 0) {
        limit = 2.0f;
    }
    if (gActorSlotsReady != 0) {
        *clampedPosition = actorPosition;
        return;
    }
    if (xglPointLength(&actorPosition, clampedPosition) < limit) {
        return;
    }
    Vu0SubVector(&offsetFromPosition, clampedPosition, &actorPosition);
    xglVectorNormal(&offsetFromPosition, &offsetFromPosition);
    xglVectorScale(&offsetFromPosition, &offsetFromPosition, limit);
    Vu0AddVector(clampedPosition, &actorPosition, &offsetFromPosition);
}

void actorSlotsSetReady(void) {
    gActorSlotsReady = 1;
}

void actorSlotsClearFlags(void) {
    s32 i;
    ActorSlot *actorSlot;

    actorSlot = gActorSlots;
    for (i = 0; i < 8; i++) {
        actorSlot->loadedBits &= ~1;
        actorSlot++;
    }
}

struct XenoActor *actorSlotGetActor(s32 actorSlotIndex) {
    return gActorSlotActorView[actorSlotIndex].actor;
}

void actorSlotEnsureLoaded(s32 actorSlotIndex);

f32 actorSlotGetRadius(s32 actorSlotIndex) {
    actorSlotEnsureLoaded(actorSlotIndex);
    return gActorSlotRadiusView[actorSlotIndex].radius;
}

s32 actorSlotGetLevel(s32 actorSlotIndex) {
    actorSlotEnsureLoaded(actorSlotIndex);
    return gActorSlotLevelView[actorSlotIndex].level;
}

s32 actorSlotGetExtent(s32 actorSlotIndex) {
    actorSlotEnsureLoaded(actorSlotIndex);
    return gActorSlotExtentView[actorSlotIndex].extent;
}

/* Copies vectors[k] of the slot to *dst. Returns 0 when the slot has no actor. */
s32 actorSlotGetVector(Vector4 *vectorOut, s32 actorSlotIndex, s32 k) {
    actorSlotEnsureLoaded(actorSlotIndex);
    if (gActorSlots[actorSlotIndex].actor == 0) {
        return 0;
    }
    *vectorOut = gActorSlots[actorSlotIndex].vectors[k];
    return 1;
}

void actorSlotLoad(s32 actorSlotIndex);

void actorSlotEnsureLoaded(s32 actorSlotIndex) {
    if (!(gActorSlots[actorSlotIndex].loadedBits & 1)) {
        actorSlotLoad(actorSlotIndex);
    }
}

/* Copies the slot placement to *dst. Returns 0 when the slot has no actor. */
s32 actorSlotGetPlacement(SlotPlacement *placementOut, s32 actorSlotIndex) {
    actorSlotEnsureLoaded(actorSlotIndex);
    if (gActorSlots[actorSlotIndex].actor == 0) {
        return 0;
    }
    *placementOut = gActorSlots[actorSlotIndex].placement;
    return 1;
}

/* Sets the slot radius and marks it set (loadedBits bit 1). */
void actorSlotSetRadius(s32 actorSlotIndex, f32 radius) {
    gActorSlots[actorSlotIndex].radius = radius;
    gActorSlots[actorSlotIndex].loadedBits |= 2;
}
