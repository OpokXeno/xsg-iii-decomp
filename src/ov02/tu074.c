/*
 * OV02 original TU 74: 0x00aa52f8..0x00aa6348 (11 functions)
 */
#include "common.h"
#include "ov02/tu060.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA52F8);

typedef int s32;

/* The battle actor the status and state helpers work on. */
typedef struct BattleActor {
    unsigned char unmodeled_00[0x90];
    struct XenoActor *members[4];   /* +0x90: the member actors whose resetFlags are edited */
    struct BattleUnit *unitSide;    /* +0xA0: the unit whose motion actor owns the status word */
    unsigned char unmodeled_A4[0xF0 - 0xA4];
    unsigned char stateBlock[0x220 - 0xF0]; /* +0xF0: passed to func_00A9E738 */
    s32 unk_220;
    s32 unk_224;
    s32 stateValue;                 /* +0x228 */
} BattleActor;

extern char gStatusBitsAssertFile[];
extern char gStatusBitsAssertMessage[];
void assertFailed(const char *message, const char *file, int line);

void unitClearStatusBits(BattleActor *actor, s32 mask)
{
  s32 memberIndex;
  XenoActor **memberSlot;
  BattleUnit *sideUnit;
  XenoActor *member;
  XenoActor *ownerCopy;
  XenoActor *owner;
  if (actor == 0 || (sideUnit = actor->unitSide) == 0 || (sideUnit->motionActor) == 0)
  {
    assertFailed(gStatusBitsAssertMessage, gStatusBitsAssertFile, 0x64);
  }
  owner = 0;
  if (actor->unitSide != 0)
  {
    owner = actor->unitSide->motionActor;
  }
  ownerCopy = 0;
  if (actor->unitSide != 0)
  {
    ownerCopy = actor->unitSide->motionActor;
  }
  memberSlot = actor->members;
  owner->resetFlags = ownerCopy->resetFlags & mask;
  for (memberIndex = 0; memberIndex < 4; memberIndex++)
  {
    member = *memberSlot;
    memberSlot = memberSlot + 1;
    if (member != 0)
    {
      member->resetFlags &= mask;
    }
  }

}

void unitSetStatusBits(BattleActor *actor, s32 mask)
{
  s32 memberIndex;
  XenoActor **memberSlot;
  BattleUnit *sideUnit;
  XenoActor *member;
  XenoActor *ownerCopy;
  XenoActor *owner;
  if (actor == 0 || (sideUnit = actor->unitSide) == 0 || (sideUnit->motionActor) == 0)
  {
    assertFailed(gStatusBitsAssertMessage, gStatusBitsAssertFile, 0x73);
  }
  owner = 0;
  if (actor->unitSide != 0)
  {
    owner = actor->unitSide->motionActor;
  }
  ownerCopy = 0;
  if (actor->unitSide != 0)
  {
    ownerCopy = actor->unitSide->motionActor;
  }
  memberSlot = actor->members;
  owner->resetFlags = ownerCopy->resetFlags | mask;
  for (memberIndex = 0; memberIndex < 4; memberIndex++)
  {
    member = *memberSlot;
    memberSlot = memberSlot + 1;
    if (member != 0)
    {
      member->resetFlags |= mask;
    }
  }

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA5598);

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA5788);

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA5918);

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA59C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA5A68);

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA5B20);

s32 func_00A9E738(void *);

void battleActorStateReset(BattleActor *actor, s32 actorStateValue)
{
  func_00A9E738(actor->stateBlock);
  actor->unk_220 = 0;
  actor->stateValue = actorStateValue;
  actor->unk_224 = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu074", func_00AA6148);
