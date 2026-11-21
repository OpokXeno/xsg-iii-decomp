#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu083", func_001C7450);

typedef int s32;
typedef unsigned short u16;
typedef float f32;
typedef unsigned char u8;

void XenoActorIdleHandler(void) {

}

/* Lookup request filled for func_001C2F78: the next index after the owner's lastScanIndex at 0x28, a mode of 1, the
 * bound from 0x158 and the found flag the lookup sets. */
struct XenoActorScanRequest {
    s32 scanFromIndex; /* 0x00 */
    s32 scanMode; /* 0x04 */
    s32 bound;   /* 0x08 */
    s32 found;   /* 0x0C */
};

s32 func_001C2F78(struct XenoActorScanRequest *scanRequest);

/* The mode block of the object at +0x150 (the state pointer of the mode code). */
typedef struct XenoActorModeTimerBlock {
    u16 modeFlagBits;                  /* +0x00 */
    u8 unk_02[0x08 - 0x02];
    s32 bound;                  /* +0x08 */
    u8 unk_0C[0x1C - 0x0C];
    s32 timer;                  /* +0x1C */
} XenoActorModeTimerBlock;

/* The object of this TU group (the offsets are read from the original code). */
typedef struct XenoActorHead {
    s32 actorFlags;                  /* +0x00 */
    u8 unk_04[0x1C - 0x04];
    u8 *handler;                /* +0x1C: the state handler the object calls */
    s32 unk_20;
    s32 actorId;                /* +0x24 */
    s32 lastScanIndex;           /* +0x28 */
    u8 unk_2C[0x34 - 0x2C];
    s32 foundIndex;             /* +0x34 */
    u8 unk_38[0x150 - 0x38];
    XenoActorModeTimerBlock modeTimer;    /* +0x150 */
} XenoActorHead;

s32 func_001C7450(XenoActorHead *actor, s32 arg);

s32 XenoActorAdvanceLookup(XenoActorHead *actor)
{
  struct XenoActorScanRequest scanRequest;
  s32 bound;
  s32 scanOutcome;
  s32 lookupHandled;
  bound = actor->modeTimer.bound;
  scanRequest.scanMode = 1;
  scanRequest.scanFromIndex = actor->lastScanIndex + 1;
  scanRequest.bound = bound;
  scanOutcome = func_001C2F78(&scanRequest);
  lookupHandled = scanOutcome;
  if (lookupHandled != 0)
  {
    if (scanRequest.found != 0)
    {
      actor->foundIndex = scanRequest.scanFromIndex;
    }
    else
    {
      func_001C7450(actor, 1);
    }
    lookupHandled = 1;
  }
  return lookupHandled;
}

INCLUDE_ASM("asm/main/nonmatchings/tu083", func_001C75E0);

INCLUDE_ASM("asm/main/nonmatchings/tu083", func_001C76B8);

INCLUDE_ASM("asm/main/nonmatchings/tu083", func_001C7778);

s32 XenoActorIsDone(s32);
s32 func_001C0B38(XenoActorHead *actor, s32 arg);
s32 func_001C1388(XenoActorHead *actor, f32 arg);
s32 func_001C75E0(XenoActorHead *actor);
f32 func_0027DEF0(s32);

void XenoActorStepModeTimer(XenoActorHead *actor)
{
  XenoActorModeTimerBlock *actorModeTimer;
  s32 actorId;
  s32 timer;
  actorModeTimer = &actor->modeTimer;
  actorId = actor->actorId;
  if (func_001C1388(actor, func_0027DEF0(actorId)) != 0)
  {
    func_001C0B38(actor, actorId + 0x20);
  }
  timer = actorModeTimer->timer;
  if (timer != 0)
  {
    actorModeTimer->timer = timer - 1;
  }
  else
  {
    /* The do/while(0) block keeps the join before the epilogue: without it the call below becomes a tail call. */
    do
    {
      if (actor->modeTimer.modeFlagBits & 0x40)
      {
        if (XenoActorIsDone(actorId) != 0)
        {
          actor->modeTimer.modeFlagBits &= 0xFFBF;
        }
      }
      else if ((func_001C75E0(actor) != 0) && (!(actor->modeTimer.modeFlagBits & 0x10)))
      {
        func_001C7450(actor, 0);
      }
    } while (0);
  }
}

s32 func_001C1590(XenoActorHead *actor, s32 arg);
extern unsigned char func_001C7940[];

void XenoActorSwitchHandler(XenoActorHead *actor)
{
  s32 clearedFlags;
  s32 actorFlagsNow;
  actorFlagsNow = actor->actorFlags;
  if (actorFlagsNow & 0x40)
  {
    clearedFlags = actorFlagsNow & (~0x48);
    actor->actorFlags = clearedFlags;
    actor->handler = func_001C7940;
    func_001C1590(actor, 1);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu083", func_001C7940);
