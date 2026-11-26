#include "common.h"
#include "shared.h"
#include "main/xgl_cd.h"

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_002213A8);

typedef signed char s8;
typedef int s32;

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

/* 0x10-byte roster of the table at gBattleRosterTable: roster 0 holds entry ids below 0xD (at most 7),
   roster 1 the ids from 0xD on (at most 4). */
typedef struct BattleRoster
{
  u8 entryIdCount;             /* +0x00 */
  u8 cursor;          /* +0x01: index of the selected entry */
  u8 entryIds[0xE];   /* +0x02 */
} BattleRoster;

/* 0x24-byte per-character record of gBattleRosterTable, from +0x48 (func_00222768 returns &values[slot]).
   The stride-0x24 accessors in tu177 use other bases into the same array: the skill count and cost of
   character c sit at table +0x28 + c * 0x24 (+0x04/+0x05 of record c - 1), and the skill-board nibbles
   at table +0x28 + c * 0x24 + (page >> 1) * 8 + row. */
typedef struct BattleCharacterRecord
{
  s16 values[2];                /* +0x00: newGameStateReset writes the new-game defaults */
  u8 unmodeled_04[0x24 - 0x04];
} BattleCharacterRecord;

/* gBattleRosterTable, 0x2B0 bytes (newGameStateReset clears it with one memset). The game also addresses parts of it
   through their own symbols: gBattleRosterCursors (+0x01), gBattleRosterEntryIds (+0x02), D_003E6BA0 (+0x20),
   gBattleCurrentRosterIndex (+0x26), gRecordSlotBitTable (+0x28), D_003E6BC8 (+0x48). */
typedef struct BattleRosterTable
{
  BattleRoster rosters[2];      /* +0x00 */
  s32 entryMask;                /* +0x20: bit n set when entry id n is in a roster (testFlagBit) */
  s8 firstValue;                /* +0x24 */
  s8 secondValue;               /* +0x25 */
  u8 currentRosterIndex;        /* +0x26 */
  u8 unlockFlags;               /* +0x27 */
  u8 slotBitNibbles[0x48 - 0x28]; /* +0x28: skill-board nibbles (gRecordSlotBitTable) */
  BattleCharacterRecord characters[(0x2AC - 0x48) / 0x24]; /* +0x48 */
  u8 unmodeled_2AC[0x2B0 - 0x2AC];
} BattleRosterTable;

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

/* Play-state header at D_003E6E30 (res_get_path.c SaveGameHeader). */
typedef struct SaveGameHeader {
    s32 startTime; /* +0x00 */
    unsigned char unmodeled_04[0x21];
    unsigned char port;      /* +0x25 */
    unsigned char fileIndex; /* +0x26 */
} SaveGameHeader;

/* The 0x130B0-byte memory-card save image at gXglMcAgedEntries (tu178 AgedEntryRing, extended with the
   regions the new-game reset clears). */
typedef struct AgedEntryRing {
    unsigned char unmodeled_0000[0x10];
    s32 newGameCounters[2];                     /* +0x10: zeroed by the new-game reset */
    XglClock newGameClock;                      /* +0x18: clock read when a new game starts */
    unsigned char unmodeled_0020[0x4A - 0x20];
    unsigned char flag4A;                       /* +0x4A */
    unsigned char flag4B;                       /* +0x4B */
    unsigned char unmodeled_004C[0x1330 - 0x4C];
    unsigned char bytes1330[0x1408 - 0x1330];   /* +0x1330 */
    unsigned short agedEntries[50];             /* +0x1408 */
    unsigned char unmodeled_146C[0x146D - 0x146C];
    unsigned char bytes146D[0x147C - 0x146D];   /* +0x146D */
    unsigned char bytes147C[0x157C - 0x147C];   /* +0x147C */
    unsigned char bytes157C[0x177C - 0x157C];   /* +0x157C */
    unsigned char unmodeled_177C[0x3A0C - 0x177C];
    unsigned char bytes3A0C[0x1302C - 0x3A0C];  /* +0x3A0C */
    unsigned char unmodeled_1302C[0x130B0 - 0x1302C]; /* tu178: aged-entry ring cursors at +0x130AA */
} AgedEntryRing;

extern BattleRosterTable gBattleRosterTable;
extern SaveGameHeader D_003E6E30;
extern AgedEntryRing gXglMcAgedEntries;
extern u8 gSaveAbortFlag;

void *memset(void *destination, int value, unsigned int count);
void xglClockRead(XglClock *clock);
unsigned int xglClockDayTime2UInt(XglClock *clock_time);
void xglMcAgedEntriesReset(void);
s32 func_001B8D08(s32 category, s32 item, s32 amount);

/* New-game reset: clears the roster table, the play-state header and the save image regions, stamps the
   start time and writes the per-character defaults. */
void newGameStateReset(void)
{
  memset(&gBattleRosterTable, 0, 0x2B0);
  memset(&D_003E6E30, 0, 0x2C);
  xglClockRead(&gXglMcAgedEntries.newGameClock);
  D_003E6E30.startTime = xglClockDayTime2UInt(&gXglMcAgedEntries.newGameClock);
  gBattleRosterTable.characters[1].values[0] = 1;
  gBattleRosterTable.characters[2].values[0] = 0xC;
  gBattleRosterTable.characters[3].values[0] = 0x1B;
  gBattleRosterTable.characters[4].values[0] = 0x25;
  gBattleRosterTable.characters[5].values[0] = 0x34;
  gBattleRosterTable.characters[6].values[0] = 0x41;
  gBattleRosterTable.characters[7].values[0] = 0x51;
  gBattleRosterTable.characters[8].values[0] = 0x61;
  gBattleRosterTable.characters[9].values[0] = 0x62;
  gBattleRosterTable.characters[10].values[0] = 0x63;
  gBattleRosterTable.characters[13].values[0] = 0x6E;
  gBattleRosterTable.characters[13].values[1] = 0x6C;
  gBattleRosterTable.characters[14].values[0] = 0x7E;
  gBattleRosterTable.characters[14].values[1] = 0x80;
  gBattleRosterTable.characters[15].values[0] = 0x8E;
  gBattleRosterTable.characters[15].values[1] = 0x8F;
  gBattleRosterTable.characters[16].values[0] = 0x9E;
  gBattleRosterTable.characters[16].values[1] = 0;
  memset(gXglMcAgedEntries.bytes3A0C, 0, sizeof(gXglMcAgedEntries.bytes3A0C));
  memset(gXglMcAgedEntries.bytes157C, 0, sizeof(gXglMcAgedEntries.bytes157C));
  memset(gXglMcAgedEntries.bytes146D, 0, sizeof(gXglMcAgedEntries.bytes146D));
  memset(gXglMcAgedEntries.bytes147C, 0, sizeof(gXglMcAgedEntries.bytes147C));
  xglMcAgedEntriesReset();
  if (gSaveAbortFlag)
  {
    func_001B8D08(2, 0x4D, 1);
  }
  D_003E6E30.fileIndex = -1;
  gXglMcAgedEntries.newGameCounters[0] = 0;
  gXglMcAgedEntries.newGameCounters[1] = 0;
  gXglMcAgedEntries.flag4A = 0;
  gXglMcAgedEntries.flag4B = 0;
  memset(gXglMcAgedEntries.bytes1330, 0, sizeof(gXglMcAgedEntries.bytes1330));
}

BattleRoster *battleRosterGet(s32 rosterIndex) {
    return &gBattleRosterTable.rosters[rosterIndex];
}

u8 battleRosterEntryIdCountGet(s32 rosterIndex) {
    return gBattleRosterTable.rosters[rosterIndex].entryIdCount;
}

u8 battleRosterEntryIdCountGet(s32 rosterIndex);

u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex)
{
  if (!(entryIdIndex < battleRosterEntryIdCountGet(rosterIndex)))
  {
    return 0;
  }
  return gBattleRosterTable.rosters[rosterIndex].entryIds[entryIdIndex];
}

u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex);

s32 battleRosterCursorEntryIdGet(s32 rosterIndex) {
    return ((s32 (*)()) battleRosterEntryIdGet)(gBattleRosterTable.rosters[rosterIndex].cursor, rosterIndex);
}

u8 battleCurrentRosterEntryCountGet(void)
{
  return gBattleRosterTable.rosters[gBattleRosterTable.currentRosterIndex].entryIdCount;
}

u8 battleCurrentRosterEntryIdGet(s32 entryIdIndex)
{
  return gBattleRosterTable.rosters[gBattleRosterTable.currentRosterIndex].entryIds[entryIdIndex];
}

s32 battleCurrentRosterCursorEntryIdGet(void)
{
  return ((s32 (*)()) battleRosterEntryIdGet)(gBattleRosterTable.rosters[gBattleRosterTable.currentRosterIndex].cursor, gBattleRosterTable.currentRosterIndex);
}

u8 battleRosterHasEntryId(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  roster = (entryId < 0xD) ? &gBattleRosterTable.rosters[0] : &gBattleRosterTable.rosters[1];
  for (index = 0; index < roster->entryIdCount; index++)
  {
    if (roster->entryIds[index] == entryId)
    {
      return 1;
    }
  }
  return 0;
}

/* Selects the current roster (0 or 1). */
void battleCurrentRosterIndexSet(s32 rosterIndex)
{
  gBattleRosterTable.currentRosterIndex = rosterIndex;
  gBattleRosterTable.currentRosterIndex %= 2;
}

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_00221FB8);

/* Removes entry id from its roster: later entries move down one slot and the cursor follows its entry
   (a cursor on the removed entry goes back to 0). */
void battleRosterEntryIdRemove(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  roster = (entryId < 0xD) ? &gBattleRosterTable.rosters[0] : &gBattleRosterTable.rosters[1];
  for (index = 0; index < roster->entryIdCount; index++)
  {
    if (roster->entryIds[index] == entryId)
    {
      if (roster->cursor == index)
      {
        roster->cursor = 0;
      }
      index++;
      if (index < roster->entryIdCount)
      {
        do
        {
          if (roster->cursor == index)
          {
            roster->cursor = index - 1;
          }
          roster->entryIds[index - 1] = roster->entryIds[index];
          index++;
        } while (index < roster->entryIdCount);
      }
      roster->entryIdCount--;
      return;
    }
  }
}

void battleRosterPairReset(void) {
    gBattleRosterTable.rosters[0].entryIdCount = 0;
    gBattleRosterTable.rosters[1].entryIdCount = 0;
    gBattleRosterTable.rosters[0].cursor = 0;
    gBattleRosterTable.rosters[1].cursor = 0;
}

s32 battleRosterCursorSet(s32 index, s32 rosterIndex);

/* Swaps entries indexA and indexB of a roster; a cursor on one of the first three entries follows its entry.
   For roster 0, a cursor left on an entry id of 8 or more moves to the first of the first three entries below 8. */
void battleRosterEntrySwap(s32 indexA, s32 indexB, s32 rosterIndex)
{
  s32 value; /* the entry count, then the moved entry id, then the cursor */
  s32 index;
  s32 entryId;
  BattleRoster *roster;
  value = battleRosterEntryIdCountGet(rosterIndex);
  if (!(indexA < value))
  {
    return;
  }
  if (!(indexB < value))
  {
    return;
  }
  roster = battleRosterGet(rosterIndex);
  value = roster->entryIds[indexA];
  roster->entryIds[indexA] = roster->entryIds[indexB];
  roster->entryIds[indexB] = value;
  if ((indexA < 3) && (indexB < 3))
  {
    value = gBattleRosterTable.rosters[rosterIndex].cursor;
    if (value == indexA)
    {
      battleRosterCursorSet(indexB, rosterIndex);
    }
    else if (value == indexB)
    {
      battleRosterCursorSet(indexA, rosterIndex);
    }
  }
  if (rosterIndex == 0)
  {
    if (battleRosterCursorEntryIdGet(0) >= 8)
    {
      for (index = 0; index < 3; index++)
      {
        entryId = battleRosterEntryIdGet(index, 0);
        if (entryId < 8)
        {
          battleRosterCursorSet(index, 0);
          break;
        }
      }
    }
  }
}

s32 battleRosterCursorSet(s32 index, s32 rosterIndex)
{
  s32 entryId;
  if (index < 3)
  {
    if (rosterIndex == 0)
    {
      entryId = battleRosterEntryIdGet(index, rosterIndex);
      if (entryId >= 8)
      {
        return 0;
      }
    }
    gBattleRosterTable.rosters[rosterIndex].cursor = index;
    return 1;
  }
  return 0;
}

u8 battleCurrentRosterIndexGet(void);

/* Moves the cursor of the current roster forward to the next entry battleRosterCursorSet accepts, wrapping over the
   first one to three entries. Returns 1 when the cursor moved. */
s32 battleCurrentRosterCursorAdvance(void)
{
  s32 step;
  s32 rosterIndex;
  s32 cursor;
  s32 count;
  s32 wrapCount;
  rosterIndex = battleCurrentRosterIndexGet();
  cursor = battleRosterGet(rosterIndex)->cursor;
  count = battleRosterEntryIdCountGet(rosterIndex);
  wrapCount = count;
  if (count >= 4)
  {
    wrapCount = 3;
  }
  else if (count <= 0)
  {
    wrapCount = 1;
  }
  for (step = 1; step < wrapCount; step++)
  {
    if (battleRosterCursorSet((cursor + step) % wrapCount, rosterIndex))
    {
      break;
    }
  }
  return cursor != battleRosterGet(rosterIndex)->cursor;
}

s32 battleRosterEntryIdIndexGet(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  s32 result;
  roster = battleRosterGet(entryId >= 0xD);
  result = -1;
  for (index = 0; index < roster->entryIdCount; index++)
  {
    if (roster->entryIds[index] == entryId)
    {
      result = index;
      break;
    }
  }
  return result;
}

s32 func_00221FB8();

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
            resolvedKind = battleRosterEntryIdIndexGet(entryId);
            if (resolvedKind == -1) {
                func_00221FB8(entryIds[handleOrdinal]);
                retriedKind = battleRosterEntryIdIndexGet(entryIds[handleOrdinal]);
                resolvedKind = retriedKind;
            }
            battleRosterEntrySwap(handleOrdinal, resolvedKind, entryIds[handleOrdinal] >= 0xD);
        }
    }
}

u8 battleCurrentRosterIndexGet(void) {
    return gBattleRosterTable.currentRosterIndex;
}

s32 testFlagBit(s32 bitIndex)
{
  int mask;
  mask = 1 << bitIndex;
  return (gBattleRosterTable.entryMask & mask) != 0;
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
  gBattleRosterTable.firstValue = storedFirst;
  gBattleRosterTable.secondValue = storedSecond;
}

/* For a roster 1 entry id (0xD..0x10), returns its linked roster 0 entry id number index & 1, counting only the
   linked ids present in roster 0 (present ones first); 0 when there is none or for any other entry id. */
s32 battleRosterLinkedEntryIdGet(s32 entryId, s32 index)
{
  static const u8 linkedEntryIds[4][2] = { { 7, 6 }, { 3, 4 }, { 5, 1 }, { 2, 0 } };
  s32 linked[2];
  s32 linkedEntryId;
  memset(linked, 0, sizeof(linked));
  linkedEntryId = 0;
  if (entryId >= 0xD && entryId < 0x11)
  {
    linked[0] = linkedEntryIds[entryId - 0xD][0];
    linked[1] = linkedEntryIds[entryId - 0xD][1];
    if (battleRosterHasEntryId(linked[1]) == 0)
    {
      linked[1] = 0;
    }
    if (battleRosterHasEntryId(linked[0]) == 0)
    {
      linked[0] = linked[1];
      linked[1] = 0;
    }
    linkedEntryId = linked[index & 1];
  }
  return linkedEntryId;
}
