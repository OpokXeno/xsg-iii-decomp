#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu239", func_0025D518);

typedef unsigned char u8;

typedef int s32;

extern unsigned char gBattleChatterControlStorage[];
extern unsigned char D_003E21A8[];
extern unsigned char D_003E2218[];
extern unsigned char D_003E2288[];

/* Battle chatter control block at gBattleChatterControlStorage (gBattleChatterControl), the one the three queue submitters below share. */
typedef struct BattleChatterControl {
    u8 chatterFlags;                   /* +0x00, bit 0 set or cleared by the submitters */
    unsigned char unk_01[0x7];
    s32 lastLineId;              /* +0x08, id returned by textPickRandomEntry for the last submission */
    void *queue;                /* +0x0C, the queue last submitted */
} BattleChatterControl;

/* The control block at gBattleChatterControlStorage, read through this one typed view. */
#define gBattleChatterControl (*(BattleChatterControl *) gBattleChatterControlStorage)

extern void *D_003E23C0[];
s32 textApplyPickedEntry(s32, s32, s32);
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
  textApplyPickedEntry(pickedId, firstActorSlot, secondActorSlot);
  gBattleChatterControl.lastLineId = pickedId;
}

typedef struct BattleChatterControlView {
    u8 chatterFlags; /* +0x00, bit 0 set or cleared by the submitters */
    unsigned char unk_01[0x3];
    s32 lastModeLineId; /* +0x04, id of the last mode-line submission */
    s32 lastLineId;     /* +0x08, id returned by textPickRandomEntry for the last submission */
    void *queue;        /* +0x0C, the queue last submitted */
} BattleChatterControlView;

#define gBattleChatterControlView (*(BattleChatterControlView *) gBattleChatterControlStorage)

void *actorSlotGetActor(s32 slot);
s32 actorSlotGetExtent(s32 slot);

typedef struct BattleActorSlots {
    u8 battleMode;
    u8 firstActorSlot;
    u8 secondActorSlot;
} BattleActorSlots;

extern unsigned char gBattleModeActors[];

#define gBattleActorSlots (*(BattleActorSlots *) gBattleModeActors)

extern void *D_003E25B8[];
extern void *D_003E25D8[];
extern void *D_003E2490[];

/* Battle mode-line chatter: does nothing while chatter bit 0 is set. Otherwise it picks a random line
 * table by the two actors' extents (0xAF and 0xAB are the two values that select the second table),
 * never repeats the last pick, and submits it through textApplyPickedEntry. */
void textSubmitBattleModeLine(void) {
    s32 firstExtent;
    s32 secondExtent;
    void **lineTable;
    s32 pickedId;
    s32 flags;

    flags = gBattleChatterControlView.chatterFlags;
    if (flags & 1) {
        return;
    }
    firstExtent = actorSlotGetExtent(gBattleActorSlots.firstActorSlot);
    secondExtent = actorSlotGetExtent(gBattleActorSlots.secondActorSlot);
    if (firstExtent != 0xAF && firstExtent != 0xAB && secondExtent != 0xAF && secondExtent != 0xAB) {
        lineTable = D_003E25B8;
    } else {
        lineTable = D_003E25D8;
    }
    do {
        pickedId = textPickRandomEntry(lineTable);
        if (pickedId == (s32) D_003E2490) {
            if (actorSlotGetActor(gBattleActorSlots.secondActorSlot) == 0) {
                return;
            }
        }
    } while (gBattleChatterControlView.lastModeLineId == pickedId);
    gBattleChatterControlView.queue = lineTable;
    textApplyPickedEntry(pickedId, gBattleActorSlots.firstActorSlot, gBattleActorSlots.secondActorSlot);
    gBattleChatterControlView.lastModeLineId = pickedId;
}

extern void *D_003E26B0[];
s32 textSubmitRandomEntry(void *, s32, s32);

void textSubmitFixedClearA(s32 firstActorSlot, s32 secondActorSlot) {
    gBattleChatterControl.chatterFlags &= 0xFE;
    textSubmitRandomEntry(D_003E26B0, firstActorSlot, secondActorSlot);
}

extern void *D_003E2748[];

void textSubmitFixedSetB(s32 firstActorSlot, s32 secondActorSlot) {
    gBattleChatterControl.chatterFlags |= 1;
    textSubmitRandomEntry(D_003E2748, firstActorSlot, secondActorSlot);
}

extern void *D_003E2868[];

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
  textApplyPickedEntry(pickedId, firstActorSlot, secondActorSlot);
  gBattleChatterControl.lastLineId = pickedId;
}

extern void *D_003E2980[];

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
  textApplyPickedEntry(pickedId, firstActorSlot, secondActorSlot);
  gBattleChatterControl.lastLineId = pickedId;
}

/* read-only names and records of the menu tables below */
const unsigned int D_005309F8[4] = { 0xC0C0400C, 0x35307220, 0, 0 };
const unsigned int D_00530A08[4] = { 0xC0C0400C, 0x36327220, 0, 0 };
const char D_00530A18[8] = "r302";
const char D_00530A20[8] = "r312";

extern char D_00550708[];
extern char D_00550710[];
extern char D_00550718[];
extern char D_00550720[];
extern char D_00550728[];
extern char D_00550730[];
extern char D_00550738[];
extern char D_00550740[];
extern char D_00550748[];
extern char D_00550750[];
extern char D_00550758[];
extern char D_00550760[];
extern char D_00550768[];
extern char D_00550770[];
extern char D_00550778[];

void *D_003E2330[2] = {
    D_003E2288 + 0x70,
    D_005309F8
};
float D_003E2338[26] = {
    2.0f, 98313.0f, 3.0f, 0.449999988f,
    0.0f, -1.79999995f, 0.0f, 4.0f,
    0.400000006f, 1.70000005f, -1.5f, 8.5f,
    5.0f, 1.0f, 10.0f, 3.0f,
    0.449999988f, 0.0f, -1.79999995f, 0.0f,
    4.0f, 0.400000006f, 1.70000005f, -1.5f,
    8.5f, 0.0f
};
void *D_003E23A0[2] = {
    D_003E2338,
    D_00550708
};
void *D_003E23A8[6] = {
    D_003E21A8 + 0x68,
    D_003E2218 + 0x68,
    D_003E2288 + 0x68,
    D_003E2330,
    D_003E23A0,
    0
};
void *D_003E23C0[6] = {
    D_003E21A8 + 0x68,
    D_003E2218 + 0x68,
    D_003E2288 + 0x68,
    D_003E23A0,
    0,
    0
};
float D_003E23D8[14] = {
    2.0f, 32769.0f, 3.0f, 0.5f,
    0.0f, -2.0f, 0.0f, 4.0f,
    0.200000003f, 17.5f, -5.0f, 20.0f,
    0.0f, 0.0f
};
void *D_003E2410[2] = {
    D_003E23D8,
    D_00550710
};
float D_003E2418[14] = {
    2.0f, 129.0f, 3.0f, 0.600000024f,
    0.0f, -2.5f, 0.0f, 4.0f,
    0.800000012f, 21.0f, -2.29999995f, -20.0f,
    0.0f, 0.0f
};
void *D_003E2450[2] = {
    D_003E2418,
    D_00550718
};
float D_003E2458[14] = {
    2.0f, 2050.0f, 3.0f, 0.699999988f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.699999988f, 25.0f, -16.5f, -4.0f,
    0.0f, 0.0f
};
void *D_003E2490[2] = {
    D_003E2458,
    D_00530A08
};
float D_003E2498[14] = {
    2.0f, 32769.0f, 3.0f, 0.300000012f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.5f, 10.5f, 7.55000019f, 21.5f,
    0.0f, 0.0f
};
void *D_003E24D0[2] = {
    D_003E2498,
    D_00550720
};
float D_003E24D8[26] = {
    2.0f, 2048.0f, 3.0f, 0.5f,
    0.0f, -2.5f, 0.0f, 4.0f,
    0.0f, 25.0f, -13.0f, 35.0f,
    5.0f, 4.0f, 15.0f, 3.0f,
    0.5f, 0.0f, -2.5f, 0.0f,
    4.0f, 0.0f, 16.0f, -12.0f,
    20.0f, 0.0f
};
void *D_003E2540[2] = {
    D_003E24D8,
    D_00550728
};
float D_003E2548[26] = {
    2.0f, 2048.0f, 3.0f, 0.300000012f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.5f, 15.5f, -2.0f, -6.30000019f,
    5.0f, 4.0f, 25.0f, 3.0f,
    0.600000024f, 0.0f, -3.0f, 0.0f,
    4.0f, 1.0f, 26.0f, -1.5f,
    7.5f, 0.0f
};
void *D_003E25B0[2] = {
    D_003E2548,
    D_00550730
};
void *D_003E25B8[8] = {
    D_003E2410,
    D_003E2450,
    D_003E2490,
    D_003E24D0,
    D_003E2540,
    D_003E25B0,
    0,
    0
};
void *D_003E25D8[6] = {
    D_003E2410,
    D_003E24D0,
    D_003E2540,
    D_003E25B0,
    0,
    0
};
float D_003E25F0[14] = {
    2.0f, 6144.0f, 3.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 4.0f,
    0.0f, 12.5f, -5.0f, -70.0f,
    0.0f, 0.0f
};
void *D_003E2628[2] = {
    D_003E25F0,
    D_00550738
};
float D_003E2630[14] = {
    2.0f, 2048.0f, 3.0f, 0.0f,
    0.0f, -2.0f, 0.0f, 4.0f,
    0.0f, 10.0f, -7.0f, -40.0f,
    0.0f, 0.0f
};
void *D_003E2668[2] = {
    D_003E2630,
    D_00550740
};
float D_003E2670[14] = {
    2.0f, 4099.0f, 3.0f, 0.5f,
    0.0f, -3.20000005f, 0.0f, 4.0f,
    0.0f, 4.5f, 0.5f, 28.0f,
    0.0f, 0.0f
};
void *D_003E26A8[2] = {
    D_003E2670,
    D_00550748
};
void *D_003E26B0[4] = {
    D_003E2628,
    D_003E2668,
    D_003E26A8,
    0
};
float D_003E26C0[14] = {
    2.0f, 37123.0f, 3.0f, 0.349999994f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 3.5f, 1.0f, 18.0f,
    0.0f, 0.0f
};
void *D_003E26F8[2] = {
    D_003E26C0,
    D_00530A18
};
float D_003E2700[16] = {
    2.0f, 37123.0f, 3.0f, 0.300000012f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 4.0f, -1.5f, 20.0f,
    6.0f, 0.0925023779f, 0.0f, 0.0f
};
void *D_003E2740[2] = {
    D_003E2700,
    D_00530A20
};
void *D_003E2748[4] = {
    D_003E26F8,
    D_003E2740,
    0,
    0
};
float D_003E2758[16] = {
    2.0f, 71681.0f, 3.0f, 0.349999994f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 3.5f, 1.70000005f, 22.0f,
    6.0f, 0.209439352f, 0.0f, 0.0f
};
void *D_003E2798[2] = {
    D_003E2758,
    D_00550750
};
float D_003E27A0[16] = {
    2.0f, 69633.0f, 3.0f, 0.0f,
    0.0f, -3.5f, -5.5f, 4.0f,
    0.0f, 8.5f, -4.0f, -35.0f,
    6.0f, -0.209439352f, 0.0f, 0.0f
};
void *D_003E27E0[2] = {
    D_003E27A0,
    D_00550758
};
float D_003E27E8[30] = {
    2.0f, 72192.0f, 3.0f, 0.0f,
    0.0f, -3.5f, -5.5f, 4.0f,
    0.0f, 3.0f, -4.0f, -30.0f,
    6.0f, -0.174532801f, 5.0f, 4.0f,
    25.0f, 3.0f, 0.5f, 0.0f,
    3.0f, 0.0f, 4.0f, 1.0f,
    3.0f, 2.4000001f, -17.0f, 6.0f,
    -0.174532801f, 0.0f
};
void *D_003E2860[2] = {
    D_003E27E8,
    D_00550760
};
void *D_003E2868[4] = {
    D_003E2798,
    D_003E27E0,
    D_003E2860,
    0
};
float D_003E2878[16] = {
    2.0f, 4353.0f, 3.0f, 0.349999994f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 3.70000005f, 1.70000005f, 19.0f,
    6.0f, 0.174532801f, 0.0f, 0.0f
};
void *D_003E28B8[2] = {
    D_003E2878,
    D_00550768
};
float D_003E28C0[28] = {
    2.0f, 6144.0f, 3.0f, 0.0f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 3.79999995f, 0.5f, -19.0f,
    1.0f, 40.0f, 5.0f, 7.0f,
    5.0f, 3.0f, 1.0f, 0.0f,
    -3.0f, 0.0f, 4.0f, 1.0f,
    3.0f, 1.0f, 22.0f, 0.0f
};
void *D_003E2930[2] = {
    D_003E28C0,
    D_00550770
};
float D_003E2938[16] = {
    2.0f, 4097.0f, 3.0f, 0.0f,
    0.0f, -3.0f, 0.0f, 4.0f,
    1.0f, 6.5f, -1.5f, -25.0f,
    6.0f, 0.174532801f, 0.0f, 0.0f
};
void *D_003E2978[2] = {
    D_003E2938,
    D_00550778
};
void *D_003E2980[4] = {
    D_003E28B8,
    D_003E2930,
    D_003E2978,
    0
};

/* .sdata: the room code strings (4-byte arrays, 8-aligned). */
char D_00550708[] = "r06";
char D_00550710[] = "r23";
char D_00550718[] = "r24";
char D_00550720[] = "r27";
char D_00550728[] = "r28";
char D_00550730[] = "r29";
char D_00550738[] = "r30";
char D_00550740[] = "r31";
char D_00550748[] = "r32";
char D_00550750[] = "r33";
char D_00550758[] = "r34";
char D_00550760[] = "r35";
char D_00550768[] = "r36";
char D_00550770[] = "r37";
char D_00550778[] = "r38";
