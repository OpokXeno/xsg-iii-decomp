#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_002213A8);

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;

/* Counter entry: a step direction pair (+0x08, +0x09), a phase (+0x0B), a limit and a count. */
typedef struct ClampedCounter
{
  u8 unmodeled_00[0x8];
  u8 positive;          /* +0x08: 1 when the step is positive */
  s8 negative;          /* +0x09: 0x80 when it is not */
  u8 state;             /* +0x0A */
  u8 phase;             /* +0x0B: 0x80 until the phase is advanced */
  u8 unmodeled_0C[0x1C - 0x0C];
  s16 limit;            /* +0x1C */
  u8 unmodeled_1E[0x20 - 0x1E];
  u16 clampedValue;            /* +0x20 */
} ClampedCounter;

/* 0x10-byte record of the table at gBattleRosterTable. */
typedef struct BattleRoster
{
  u8 entryIdCount;             /* +0x00 */
  u8 cursor;          /* +0x01 */
  u8 entryIds[0xE];
} BattleRoster;

/* The first two records of the table, as one pair. */
typedef struct BattleRosterPair
{
  BattleRoster firstRoster;
  BattleRoster secondRoster;
} BattleRosterPair;

/* Table header: two values at +0x24 and +0x25, the current record index at +0x26. */

typedef struct BattleRosterHeader
{
  u8 unmodeled_00[0x24];
  s8 firstValue;        /* +0x24 */
  s8 secondValue;       /* +0x25 */
  u8 currentRosterIndex;     /* +0x26 */
} BattleRosterHeader;

void clampedCounterInit(ClampedCounter *clampedCounter, s32 stepDelta)
{
  s32 isPositive;
  isPositive = stepDelta > 0;
  clampedCounter->phase = 0x80;
  clampedCounter->positive = isPositive;
  clampedCounter->state = 0;
  clampedCounter->negative = (1 - isPositive) << 7;
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221440);

extern u8 gVsyncsPerFrame;

void counterEntryAdvancePhase(ClampedCounter *clampedCounter)
{
  if (clampedCounter->phase == 0x80)
  {
    clampedCounter->phase = clampedCounter->phase - (gVsyncsPerFrame * 0x10);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_002215A0);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221A88);

void addClampedCounter(ClampedCounter *clampedCounter, s16 delta)
{
  int unclampedValue;
  s16 truncatedValue;
  unclampedValue = delta + clampedCounter->clampedValue;
  truncatedValue = unclampedValue;
  clampedCounter->clampedValue = unclampedValue;
  if (clampedCounter->limit < truncatedValue)
  {
    clampedCounter->clampedValue = clampedCounter->limit;
    return;
  }
  if (truncatedValue < 0)
  {
    clampedCounter->clampedValue = 0U;
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221B40);

s32 drawEntrySetPosition();
s32 func_002BE480(s32);

void tableSelectValue(s32 drawEntry) {
    drawEntrySetPosition();
    func_002BE480(drawEntry);
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221C18);

extern s8 D_0055144A;

s8 sheetLatchedPositionGet(void) {
    return D_0055144A;
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221C50);

extern unsigned char gBattleRosterTable[];
/* Typed views of the table at gBattleRosterTable: 0x10-byte records, and the header overlaying them at +0x24. */
#define gBattleRosters ((BattleRoster *) gBattleRosterTable)
#define gBattleRosterHeader (*(BattleRosterHeader *) gBattleRosterTable)
#define gBattleRosterPair ((BattleRosterPair *) gBattleRosterTable)

BattleRoster *battleRosterGet(s32 rosterIndex) {
    return &gBattleRosters[rosterIndex];
}

u8 battleRosterEntryIdCountGet(s32 rosterIndex) {
    return gBattleRosters[rosterIndex].entryIdCount;
}

u8 battleRosterEntryIdCountGet(s32 rosterIndex);
extern unsigned char gBattleRosterEntryIds[];

u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex)
{
  if (!(entryIdIndex < battleRosterEntryIdCountGet(rosterIndex)))
  {
    return 0;
  }
  return gBattleRosterEntryIds[entryIdIndex + (rosterIndex << 4)];
}

u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex);
extern unsigned char gBattleRosterCursors[];

s32 battleRosterCursorEntryIdGet(s32 rosterIndex) {
    return ((s32 (*)()) battleRosterEntryIdGet)(gBattleRosterCursors[rosterIndex * 0x10], rosterIndex);
}

u8 battleCurrentRosterEntryCountGet(void)
{
  return gBattleRosters[gBattleRosterHeader.currentRosterIndex].entryIdCount;
}

u8 battleCurrentRosterEntryIdGet(s32 entryIdIndex)
{
  return (&gBattleRosterTable[entryIdIndex + (gBattleRosterHeader.currentRosterIndex << 4)])[2];
}

/* The same records seen from their subIndex byte (+0x01). */
typedef struct BattleRosterCursorView
{
  u8 cursor;
  u8 unmodeled_01[0x0F];
} BattleRosterCursorView;

s32 battleCurrentRosterCursorEntryIdGet(void)
{
  return ((s32 (*)()) battleRosterEntryIdGet)(((BattleRosterCursorView *) &gBattleRosterTable[1])[gBattleRosterHeader.currentRosterIndex].cursor, gBattleRosterHeader.currentRosterIndex);
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221F40);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221FA0);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221FB8);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00222048);

void battleRosterPairReset(void) {
    gBattleRosterPair->firstRoster.entryIdCount = 0;
    gBattleRosterPair->secondRoster.entryIdCount = 0;
    gBattleRosterPair->firstRoster.cursor = 0;
    gBattleRosterPair->secondRoster.cursor = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00222120);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00222238);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_002222A0);

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00222360);

s32 func_00221FB8();
s32 func_00222120();
s32 func_00222360();

void battleUpdateEntryKinds(s32 firstEntryId, s32 secondEntryId, s32 thirdEntryId) {
    s32 entryIds[4];
    s32 entryId;
    s32 handleOrdinal;
    s32 resolvedKind;
    s32 retriedKind;

    entryIds[0] = firstEntryId;
    entryIds[1] = secondEntryId;
    entryIds[2] = thirdEntryId;
    for (handleOrdinal = 0; handleOrdinal < 3; handleOrdinal++) {
        entryId = entryIds[handleOrdinal];
        if (entryId != 0) {
            resolvedKind = func_00222360(entryId);
            if (resolvedKind == -1) {
                func_00221FB8(entryIds[handleOrdinal]);
                retriedKind = func_00222360(entryIds[handleOrdinal]);
                resolvedKind = retriedKind;
            }
            func_00222120(handleOrdinal, resolvedKind, entryIds[handleOrdinal] >= 0xD);
        }
    }
}

extern unsigned char gBattleCurrentRosterIndex[];

u8 battleCurrentRosterIndexGet(void) {
    return gBattleCurrentRosterIndex[0];
}

extern unsigned char D_003E6BA0[];

s32 testFlagBit(s32 bitIndex)
{
  int mask;
  mask = 1 << bitIndex;
  return (*(s32 *) D_003E6BA0 & mask) != 0;
}

void battleRosterHeaderPairSet(s32 firstArg, s32 secondArg)
{
  s32 storedFirst;
  s32 storedSecond;
  storedFirst = firstArg;
  storedSecond = secondArg;
  if ((storedFirst == 0) && (storedSecond != 0))
  {
    storedFirst = storedSecond;
    storedSecond = 0;
  }
  gBattleRosterHeader.firstValue = storedFirst;
  gBattleRosterHeader.secondValue = storedSecond;
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_002224E8);
