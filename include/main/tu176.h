/* header_harvest: hand-maintained (owner header of main/tu176; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU176_H
#define INCLUDE_MAIN_TU176_H

/* Types owned by main/tu176 and shared with other TUs. */

/* 0x10-byte roster of the table at gBattleRosterTable: roster 0 holds entry ids below 0xD (at most 7),
   roster 1 the ids from 0xD on (at most 4). */
typedef struct BattleRoster {
    unsigned char entryIdCount;   /* +0x00 */
    unsigned char cursor;         /* +0x01: index of the selected entry */
    unsigned char entryIds[0xE];  /* +0x02 */
} BattleRoster;

/* 0x24-byte per-character record of gBattleRosterTable, from +0x48. func_00222768 (tu177) returns the address of
   s16 slot n of a record. The stride-0x24 accessors in tu177 use other bases into the same array: the skill count
   and cost of character c sit at table +0x28 + c * 0x24 (+0x04/+0x05 of record c - 1), and the skill-board nibbles
   at table +0x28 + c * 0x24 + (page >> 1) * 8 + row. */
typedef struct BattleCharacterRecord {
    short newGameDefaults[2];     /* +0x00: the two slots newGameStateReset gives a starting value */
    unsigned char unmodeled_04[0x24 - 0x04];
} BattleCharacterRecord;

/* gBattleRosterTable, 0x2B0 bytes (newGameStateReset clears it with one memset). The game also addresses parts of it
   through their own symbols: gBattleRosterCursors (+0x01), gBattleRosterEntryIds (+0x02), D_003E6BA0 (+0x20),
   gBattleCurrentRosterIndex (+0x26), gRecordSlotBitTable (+0x28), D_003E6BC8 (+0x48). */
typedef struct BattleRosterTable {
    BattleRoster rosters[2];                                     /* +0x00 */
    int entryMask;                                               /* +0x20: bitmask read by testFlagBit */
    signed char primaryGuestEntryId;                             /* +0x24: set by battleRosterGuestPairSet (ptSetGuest) */
    signed char secondaryGuestEntryId;                           /* +0x25 */
    unsigned char currentRosterIndex;                            /* +0x26 */
    unsigned char unlockFlags;                                   /* +0x27: bit 0 set and cleared by vmSetRosterTableFlag */
    unsigned char slotBitNibbles[0x48 - 0x28];                   /* +0x28: skill-board nibbles (gRecordSlotBitTable) */
    BattleCharacterRecord characters[(0x2AC - 0x48) / 0x24];     /* +0x48 */
    unsigned char unmodeled_2AC[0x2B0 - 0x2AC];
} BattleRosterTable;

/* Play-state header at D_003E6E30, 0x2C bytes (newGameStateReset clears it): the start time of the game, the copy of
   the two rosters battleRosterSnapshotTake (ptSaveUMNParty) takes, and the memory-card port and file of the last save
   (res_get_path.c). */
typedef struct SaveGameHeader {
    int startTime;                        /* +0x00: xglClockDayTime2UInt of the new-game clock */
    BattleRoster savedRosters[2];         /* +0x04: copied from gBattleRosterTable.rosters */
    unsigned char snapshotFlags;          /* +0x24: bit 1 set by battleRosterSnapshotTake */
    unsigned char port;                   /* +0x25 */
    unsigned char fileIndex;              /* +0x26 */
    unsigned char unmodeled_27[0x2C - 0x27];
} SaveGameHeader;

extern BattleRosterTable gBattleRosterTable;
extern SaveGameHeader D_003E6E30;

void newGameStateReset(void);
void battleCurrentRosterIndexSet(int rosterIndex);
void battleRosterEntryIdRemove(int entryId);
void battleRosterEntrySwap(int indexA, int indexB, int rosterIndex);
int battleRosterCursorSet(int index, int rosterIndex);
int battleRosterEntryIdIndexGet(int entryId);
int battleRosterLinkedEntryIdGet(int entryId, int index);

#endif /* INCLUDE_MAIN_TU176_H */
