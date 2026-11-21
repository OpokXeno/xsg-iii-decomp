#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu239", func_0025D518);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
extern unsigned char gBattleChatterControlStorage[];
/* Battle chatter control block at gBattleChatterControlStorage (gBattleChatterControl), the one the three queue submitters below share. */
typedef struct BattleChatterControl {
    u8 chatterFlags;                   /* +0x00, bit 0 set or cleared by the submitters */
    unsigned char unk_01[0x7];
    s32 lastLineId;              /* +0x08, id returned by textPickRandomEntry for the last submission */
    void *queue;                /* +0x0C, the queue last submitted */
} BattleChatterControl;

/* The control block at gBattleChatterControlStorage, read through this one typed view. */
#define gBattleChatterControl (*(BattleChatterControl *) gBattleChatterControlStorage)
extern unsigned char D_003E23C0[];
s32 func_0025C5D0(s32, s32, s32);
s32 textPickRandomEntry(void *);

void textSubmitRandomClearA(s32 firstActorSlot, s32 secondActorSlot)
{
  s32 pickedId;

  gBattleChatterControl.chatterFlags &= 0xFE;
  do
  {
    pickedId = textPickRandomEntry(D_003E23C0);
  }
  while (gBattleChatterControl.lastLineId == pickedId);
  gBattleChatterControl.queue = D_003E23C0;
  func_0025C5D0(pickedId, firstActorSlot, secondActorSlot);
  gBattleChatterControl.lastLineId = pickedId;
}

INCLUDE_ASM("asm/main/nonmatchings/tu239", func_0025D688);

extern unsigned char D_003E26B0[];
s32 textSubmitRandomEntry(void *, s32, s32);

void textSubmitFixedClearA(s32 firstActorSlot, s32 secondActorSlot) {
    gBattleChatterControl.chatterFlags &= 0xFE;
    textSubmitRandomEntry(D_003E26B0, firstActorSlot, secondActorSlot);
}

extern unsigned char D_003E2748[];

void textSubmitFixedSetB(s32 firstActorSlot, s32 secondActorSlot) {
    gBattleChatterControl.chatterFlags |= 1;
    textSubmitRandomEntry(D_003E2748, firstActorSlot, secondActorSlot);
}

extern unsigned char D_003E2868[];

void textSubmitRandomClearB(s32 firstActorSlot, s32 secondActorSlot)
{
  s32 pickedId;

  do
  {
    pickedId = textPickRandomEntry(D_003E2868);
  }
  while (gBattleChatterControl.lastLineId == pickedId);
  gBattleChatterControl.queue = D_003E2868;
  gBattleChatterControl.chatterFlags &= 0xFE;
  func_0025C5D0(pickedId, firstActorSlot, secondActorSlot);
  gBattleChatterControl.lastLineId = pickedId;
}

extern unsigned char D_003E2980[];

void textSubmitRandomSetC(s32 firstActorSlot, s32 secondActorSlot)
{
  s32 pickedId;

  do
  {
    pickedId = textPickRandomEntry(D_003E2980);
  }
  while (gBattleChatterControl.lastLineId == pickedId);
  gBattleChatterControl.queue = D_003E2980;
  gBattleChatterControl.chatterFlags |= 1;
  func_0025C5D0(pickedId, firstActorSlot, secondActorSlot);
  gBattleChatterControl.lastLineId = pickedId;
}
