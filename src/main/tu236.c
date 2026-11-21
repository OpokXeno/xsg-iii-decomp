#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef signed char s8;
extern s32 gActorSlotsReady;

/* One 0x60-byte slot of the actor table at gActorSlots (8 slots); fields are at their +offsets. */
typedef struct ActorSlot {
    s8 loadedBits;                           /* +0x00: bit 0 set when the slot is loaded (signed: the clear loop keeps its access) */
    u8 pad_01[0x3];
    struct XenoActor *actor;            /* +0x04 */
    u8 pad_08[0x50 - 0x08];
    f32 radius;                         /* +0x50 */
    s32 level;                          /* +0x54 */
    s32 extent;                         /* +0x58 */
    u8 pad_5c[0x60 - 0x5C];
} ActorSlot;

/* The slot table is also reached through four labels, each 0x60-byte stride from the field offset: element 0 of a view is that field. */
typedef struct SlotActorView { struct XenoActor *actor; u8 pad_04[0x60 - 0x04]; } SlotActorView;
typedef struct SlotRadiusView { f32 radius; u8 pad_04[0x60 - 0x04]; } SlotRadiusView;
typedef struct SlotLevelView { s32 level; u8 pad_04[0x60 - 0x04]; } SlotLevelView;
typedef struct SlotExtentView { s32 extent; u8 pad_04[0x60 - 0x04]; } SlotExtentView;

extern ActorSlot gActorSlots[];
extern SlotActorView gActorSlotActorView[];
extern SlotRadiusView gActorSlotRadiusView[];
extern SlotLevelView gActorSlotLevelView[];
extern SlotExtentView gActorSlotExtentView[];
void *memset(void *destination, s32 value, unsigned int count);

void actorSlotsInit(void) {
    memset(gActorSlots, 0, 0x300);
    gActorSlotsReady = 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu236", func_0025BAB8);

INCLUDE_ASM("asm/main/nonmatchings/tu236", func_0025BAF8);

INCLUDE_ASM("asm/main/nonmatchings/tu236", func_0025BC10);

void actorSlotsSetReady(void) {
    gActorSlotsReady = 1;
}

void actorSlotsClearFlags(void)
{
    s32 i;
    ActorSlot *actorSlot;
    actorSlot = gActorSlots;
    for (i = 0; i < 8; i++)
    {
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

INCLUDE_ASM("asm/main/nonmatchings/tu236", func_0025BE00);

s32 func_0025BAF8();

void actorSlotEnsureLoaded(s32 actorSlotIndex)
{
  if (!(gActorSlots[actorSlotIndex].loadedBits & 1))

  {
    func_0025BAF8(actorSlotIndex);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu236", func_0025BEE0);

INCLUDE_ASM("asm/main/nonmatchings/tu236", func_0025BF58);
