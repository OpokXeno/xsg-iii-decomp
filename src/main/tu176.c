#include "common.h"
#include "shared.h"
#include "main/tu176.h"
#include "main/tu178.h"

INCLUDE_ASM("asm/main/nonmatchings/tu176", func_002213A8);

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

extern u8 gSaveAbortFlag;

void *memset(void *destination, int value, unsigned int count);
void xglClockRead(XglClock *clock);
unsigned int xglClockDayTime2UInt(XglClock *clock_time);
s32 func_001B8D08(s32 category, s32 item, s32 amount);

/* New-game reset: clears the roster table, the play-state header and the save image regions, stamps the
   start time and writes the per-character defaults. */
void newGameStateReset(void)
{
  memset(&gBattleRosterTable, 0, sizeof(gBattleRosterTable));
  memset(&gSaveGameHeader, 0, sizeof(gSaveGameHeader));
  xglClockRead(&gXglMcAgedEntries.newGameClock);
  gSaveGameHeader.startTime = xglClockDayTime2UInt(&gXglMcAgedEntries.newGameClock);
  gBattleRosterTable.characters[1].newGameDefaults[0] = 1;
  gBattleRosterTable.characters[2].newGameDefaults[0] = 0xC;
  gBattleRosterTable.characters[3].newGameDefaults[0] = 0x1B;
  gBattleRosterTable.characters[4].newGameDefaults[0] = 0x25;
  gBattleRosterTable.characters[5].newGameDefaults[0] = 0x34;
  gBattleRosterTable.characters[6].newGameDefaults[0] = 0x41;
  gBattleRosterTable.characters[7].newGameDefaults[0] = 0x51;
  gBattleRosterTable.characters[8].newGameDefaults[0] = 0x61;
  gBattleRosterTable.characters[9].newGameDefaults[0] = 0x62;
  gBattleRosterTable.characters[10].newGameDefaults[0] = 0x63;
  gBattleRosterTable.characters[13].newGameDefaults[0] = 0x6E;
  gBattleRosterTable.characters[13].newGameDefaults[1] = 0x6C;
  gBattleRosterTable.characters[14].newGameDefaults[0] = 0x7E;
  gBattleRosterTable.characters[14].newGameDefaults[1] = 0x80;
  gBattleRosterTable.characters[15].newGameDefaults[0] = 0x8E;
  gBattleRosterTable.characters[15].newGameDefaults[1] = 0x8F;
  gBattleRosterTable.characters[16].newGameDefaults[0] = 0x9E;
  gBattleRosterTable.characters[16].newGameDefaults[1] = 0;
  memset(gXglMcAgedEntries.newGameClearedRegion3A0C, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion3A0C));
  memset(gXglMcAgedEntries.newGameClearedRegion157C, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion157C));
  memset(gXglMcAgedEntries.newGameClearedRegion146D, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion146D));
  memset(gXglMcAgedEntries.newGameClearedRegion147C, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion147C));
  xglMcAgedEntriesReset();
  if (gSaveAbortFlag)
  {
    func_001B8D08(2, 0x4D, 1);
  }
  gSaveGameHeader.fileIndex = -1;
  gXglMcAgedEntries.newGameCounters[0] = 0;
  gXglMcAgedEntries.newGameCounters[1] = 0;
  gXglMcAgedEntries.newGameClearedFlag4A = 0;
  gXglMcAgedEntries.newGameClearedFlag4B = 0;
  memset(gXglMcAgedEntries.newGameClearedRegion1330, 0, sizeof(gXglMcAgedEntries.newGameClearedRegion1330));
}

BattleRoster *battleRosterGet(s32 rosterIndex) {
    return &gBattleRosterTable.rosterPair.rosters[rosterIndex];
}

u8 battleRosterEntryIdCountGet(s32 rosterIndex) {
    return gBattleRosterTable.rosterPair.rosters[rosterIndex].entryIdCount;
}

u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex)
{
  if (!(entryIdIndex < battleRosterEntryIdCountGet(rosterIndex)))
  {
    return 0;
  }
  return gBattleRosterTable.rosterPair.rosters[rosterIndex].entryIds[entryIdIndex];
}

/* The call goes through an int-returning view of battleRosterEntryIdGet: with the plain call (u8 result promoted to
   s32) the two cursor getters come out 8 bytes longer in total and the link no longer fits. */
s32 battleRosterCursorEntryIdGet(s32 rosterIndex) {
    return ((s32 (*)()) battleRosterEntryIdGet)(gBattleRosterTable.rosterPair.rosters[rosterIndex].cursor, rosterIndex);
}

u8 battleCurrentRosterEntryCountGet(void)
{
  return gBattleRosterTable.rosterPair.rosters[gBattleRosterTable.currentRosterIndex].entryIdCount;
}

u8 battleCurrentRosterEntryIdGet(s32 entryIdIndex)
{
  return gBattleRosterTable.rosterPair.rosters[gBattleRosterTable.currentRosterIndex].entryIds[entryIdIndex];
}

/* Same int-returning view of battleRosterEntryIdGet as battleRosterCursorEntryIdGet. */
s32 battleCurrentRosterCursorEntryIdGet(void)
{
  return ((s32 (*)()) battleRosterEntryIdGet)(
      gBattleRosterTable.rosterPair.rosters[gBattleRosterTable.currentRosterIndex].cursor,
      gBattleRosterTable.currentRosterIndex);
}

/* Returns 1 when entryId is in its roster (roster 0 below BATTLE_ROSTER1_FIRST_ENTRY_ID, else roster 1), else 0. */
u8 battleRosterHasEntryId(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  roster = (entryId < BATTLE_ROSTER1_FIRST_ENTRY_ID) ? &gBattleRosterTable.rosterPair.rosters[0]
                                                     : &gBattleRosterTable.rosterPair.rosters[1];
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
  roster = (entryId < BATTLE_ROSTER1_FIRST_ENTRY_ID) ? &gBattleRosterTable.rosterPair.rosters[0]
                                                     : &gBattleRosterTable.rosterPair.rosters[1];
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
    gBattleRosterTable.rosterPair.rosters[0].entryIdCount = 0;
    gBattleRosterTable.rosterPair.rosters[1].entryIdCount = 0;
    gBattleRosterTable.rosterPair.rosters[0].cursor = 0;
    gBattleRosterTable.rosterPair.rosters[1].cursor = 0;
}

/* Swaps entries indexA and indexB of a roster. When both swapped indices are below 3, a cursor on either index is
   moved through battleRosterCursorSet. For roster 0, a cursor left on an entry id of 8 or more moves to the first of
   the first three entries below 8. */
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
  if ((indexA < BATTLE_ROSTER_CURSOR_ENTRY_COUNT) && (indexB < BATTLE_ROSTER_CURSOR_ENTRY_COUNT))
  {
    value = gBattleRosterTable.rosterPair.rosters[rosterIndex].cursor;
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
    if (battleRosterCursorEntryIdGet(0) >= BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT)
    {
      for (index = 0; index < BATTLE_ROSTER_CURSOR_ENTRY_COUNT; index++)
      {
        entryId = battleRosterEntryIdGet(index, 0);
        if (entryId < BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT)
        {
          battleRosterCursorSet(index, 0);
          break;
        }
      }
    }
  }
}

/* Puts the cursor of a roster on entry index when index is below BATTLE_ROSTER_CURSOR_ENTRY_COUNT; for roster 0 the
   entry id battleRosterEntryIdGet gives there must also be below BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT. Returns 1 when
   the cursor was set, 0 when it was left alone. */
s32 battleRosterCursorSet(s32 index, s32 rosterIndex)
{
  s32 entryId;
  if (index < BATTLE_ROSTER_CURSOR_ENTRY_COUNT)
  {
    if (rosterIndex == 0)
    {
      entryId = battleRosterEntryIdGet(index, rosterIndex);
      if (entryId >= BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT)
      {
        return 0;
      }
    }
    gBattleRosterTable.rosterPair.rosters[rosterIndex].cursor = index;
    return 1;
  }
  return 0;
}

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
  if (count > BATTLE_ROSTER_CURSOR_ENTRY_COUNT)
  {
    wrapCount = BATTLE_ROSTER_CURSOR_ENTRY_COUNT;
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

/* Returns the index of entryId in its roster (roster 1 from BATTLE_ROSTER1_FIRST_ENTRY_ID on, else roster 0), or -1
   when the roster does not hold it. */
s32 battleRosterEntryIdIndexGet(s32 entryId)
{
  BattleRoster *roster;
  s32 index;
  s32 result;
  roster = battleRosterGet(entryId >= BATTLE_ROSTER1_FIRST_ENTRY_ID);
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
    s32 entryIds[3];
    s32 entryId;
    s32 handleOrdinal;
    s32 resolvedEntryIndex;
    s32 retriedEntryIndex;

    entryIds[0] = firstEntryId;
    entryIds[1] = secondEntryId;
    entryIds[2] = thirdEntryId;
    for (handleOrdinal = 0; handleOrdinal < 3; handleOrdinal++) {
        entryId = entryIds[handleOrdinal];
        if (entryId != 0) {
            resolvedEntryIndex = battleRosterEntryIdIndexGet(entryId);
            if (resolvedEntryIndex == -1) {
                func_00221FB8(entryIds[handleOrdinal]);
                retriedEntryIndex = battleRosterEntryIdIndexGet(entryIds[handleOrdinal]);
                resolvedEntryIndex = retriedEntryIndex;
            }
            battleRosterEntrySwap(handleOrdinal, resolvedEntryIndex,
                                  entryIds[handleOrdinal] >= BATTLE_ROSTER1_FIRST_ENTRY_ID);
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

/* Stores the two guest entry ids (VM native ptSetGuest); a lone second guest moves to the first place. */
void battleRosterHeaderPairSet(s32 firstGuestEntryId, s32 secondGuestEntryId)
{
  s32 firstGuest;
  s32 secondGuest;
  firstGuest = firstGuestEntryId;
  secondGuest = secondGuestEntryId;
  if ((firstGuest == 0) && (secondGuest != 0))
  {
    firstGuest = secondGuest;
    secondGuest = 0;
  }
  gBattleRosterTable.firstGuestEntryId = firstGuest;
  gBattleRosterTable.secondGuestEntryId = secondGuest;
}

/* For a roster 1 entry id (0xD..0x10), returns its linked roster 0 entry id number index & 1, counting only the
   linked ids present in roster 0 (present ones first); 0 when there is none or for any other entry id. */
s32 battleRosterLinkedEntryIdGet(s32 entryId, s32 index)
{
  static const u8 gBattleRosterLinkedEntryIds[4][2] = { { 7, 6 }, { 3, 4 }, { 5, 1 }, { 2, 0 } };
  s32 linked[2];
  s32 linkedEntryId;
  memset(linked, 0, sizeof(linked));
  linkedEntryId = 0;
  if (entryId >= BATTLE_ROSTER1_FIRST_ENTRY_ID && entryId <= BATTLE_ROSTER1_LAST_ENTRY_ID)
  {
    linked[0] = gBattleRosterLinkedEntryIds[entryId - BATTLE_ROSTER1_FIRST_ENTRY_ID][0];
    linked[1] = gBattleRosterLinkedEntryIds[entryId - BATTLE_ROSTER1_FIRST_ENTRY_ID][1];
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
