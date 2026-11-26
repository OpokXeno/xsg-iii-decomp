/* header_harvest: hand-maintained (owner header of main/tu176; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU176_H
#define INCLUDE_MAIN_TU176_H

/* Types owned by main/tu176 and shared with other TUs. */

#include "shared.h"

typedef signed char s8;

typedef int s32;

/* 0x10-byte roster of the table at gBattleRosterTable: roster 0 holds entry ids below 0xD (at most 7),
   roster 1 the ids from 0xD on (at most 4). */
typedef struct BattleRoster {
    u8 entryIdCount;                                          /* +0x00 */
    u8 cursor;                                                /* +0x01: index of the selected entry */
    u8 entryIds[0xE];                                         /* +0x02 */
} BattleRoster;

/* The two rosters, the part of gBattleRosterTable that battleRosterSnapshotTake (VM native ptSaveUMNParty) copies
   into the save header as one block. */
typedef struct BattleRosterPair {
    BattleRoster rosters[2];                                  /* +0x00 */
} BattleRosterPair;

/* 0x24-byte per-character record of gBattleRosterTable, from +0x48. func_00222768 (tu177) returns the address of
   s16 slot n of a record. The stride-0x24 accessors in tu177 use other bases into the same array: the skill count
   and cost of character c sit at table +0x28 + c * 0x24 (+0x04/+0x05 of record c - 1), and the skill-board nibbles
   at table +0x28 + c * 0x24 + (page >> 1) * 8 + row. */
typedef struct BattleCharacterRecord {
    s16 newGameDefaults[2];     /* +0x00: the two slots newGameStateReset gives a starting value */
    u8 unmodeled_04[0x24 - 0x04];
} BattleCharacterRecord;

/* gBattleRosterTable, 0x2B0 bytes (newGameStateReset clears it with one memset). The game also addresses parts of it
   through their own symbols: gBattleRosterCursors (+0x01), gBattleRosterEntryIds (+0x02), D_003E6BA0 (+0x20),
   gBattleCurrentRosterIndex (+0x26), gRecordSlotBitTable (+0x28), D_003E6BC8 (+0x48). */
typedef struct BattleRosterTable {
    BattleRosterPair rosterPair;                              /* +0x00 */
    s32 entryMask;                                            /* +0x20: bitmask read by testFlagBit */
    s8 firstGuestEntryId;                                     /* +0x24: battleRosterHeaderPairSet (ptSetGuest) */
    s8 secondGuestEntryId;                                    /* +0x25 */
    u8 currentRosterIndex;                                    /* +0x26 */
    u8 unlockFlags;                                           /* +0x27: bit 0 written by vmSetRosterTableFlag */
    u8 slotBitNibbles[0x48 - 0x28];                           /* +0x28: skill-board nibbles (gRecordSlotBitTable) */
    BattleCharacterRecord characters[(0x2AC - 0x48) / 0x24];  /* +0x48 */
    u8 unmodeled_2AC[0x2B0 - 0x2AC];
} BattleRosterTable;

/* Play-state header at gSaveGameHeader, 0x2C bytes (newGameStateReset clears it): the start time of the game, the
   copy of the roster pair battleRosterSnapshotTake (ptSaveUMNParty) takes, and the memory-card port and file of the
   last save (res_get_path.c). */
typedef struct SaveGameHeader {
    s32 startTime;                                            /* +0x00: xglClockDayTime2UInt of the new-game clock */
    BattleRosterPair roster;                                  /* +0x04: copied from gBattleRosterTable.rosterPair */
    u8 snapshotFlags;                                         /* +0x24: bit 1 set by battleRosterSnapshotTake */
    u8 port;                                                  /* +0x25 */
    u8 fileIndex;                                             /* +0x26 */
    u8 unmodeled_27[0x2C - 0x27];
} SaveGameHeader;

extern BattleRosterTable gBattleRosterTable;
extern SaveGameHeader gSaveGameHeader;

void newGameStateReset(void);
BattleRoster *battleRosterGet(s32 rosterIndex);
u8 battleRosterEntryIdCountGet(s32 rosterIndex);
u8 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex);
s32 battleRosterCursorEntryIdGet(s32 rosterIndex);
u8 battleCurrentRosterEntryCountGet(void);
u8 battleCurrentRosterEntryIdGet(s32 entryIdIndex);
s32 battleCurrentRosterCursorEntryIdGet(void);
u8 battleRosterHasEntryId(s32 entryId);
void battleCurrentRosterIndexSet(s32 rosterIndex);
void battleRosterEntryIdRemove(s32 entryId);
void battleRosterPairReset(void);
void battleRosterEntrySwap(s32 indexA, s32 indexB, s32 rosterIndex);
s32 battleRosterCursorSet(s32 index, s32 rosterIndex);
s32 battleCurrentRosterCursorAdvance(void);
s32 battleRosterEntryIdIndexGet(s32 entryId);
void battleUpdateEntryKinds(s32 firstEntryId, s32 secondEntryId, s32 thirdEntryId);
u8 battleCurrentRosterIndexGet(void);
void battleRosterHeaderPairSet(s32 firstGuestEntryId, s32 secondGuestEntryId);
s32 battleRosterLinkedEntryIdGet(s32 entryId, s32 index);

#endif /* INCLUDE_MAIN_TU176_H */
