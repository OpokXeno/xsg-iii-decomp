/* header_harvest: hand-maintained (owner header of main/tu176; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU176_H
#define INCLUDE_MAIN_TU176_H

/* Types owned by main/tu176 and shared with other TUs. */

#include "shared.h"

typedef signed char s8;

typedef int s32;

/* First entry id of roster 1; lower ids belong to roster 0. */
#define BATTLE_ROSTER1_FIRST_ENTRY_ID 0xD
/* Last roster 1 entry id battleRosterLinkedEntryIdGet has linked ids for. */
#define BATTLE_ROSTER1_LAST_ENTRY_ID 0x10
/* Exclusive upper bound on the index used by the cursor setter and the cursor scans; not an entry-count check. */
#define BATTLE_ROSTER_CURSOR_ENTRY_COUNT 3
/* A roster 0 entry the cursor selects must have an id below this. */
#define BATTLE_ROSTER0_CURSOR_ENTRY_ID_LIMIT 8

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

/* 0x24-byte per-character record of gBattleRosterTable, from +0x48. func_00222768 returns the address of s16
   slot n of a record. The stride-0x24 skill accessors use other bases into the same array: the skill count
   and cost of character c sit at table +0x28 + c * 0x24 (+0x04/+0x05 of record c - 1), and the skill-board nibbles
   at table +0x28 + c * 0x24 + (page >> 1) * 8 + row. */
typedef struct BattleCharacterRecord {
    s16 newGameDefaults[2];     /* +0x00: the two slots newGameStateReset gives a starting value */
    u8 skillCount;              /* +0x04: skill count of character c + 1 */
    u8 skillCost;               /* +0x05: skill cost of character c + 1 */
    u8 unmodeled_06[0x24 - 0x06];                             /* +0x06 */
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
    u8 recordSlotBitTable[0x48 - 0x28];                       /* +0x28: skill-board nibbles (gRecordSlotBitTable) */
    BattleCharacterRecord characters[(0x2AC - 0x48) / 0x24];  /* +0x48 */
    u8 unmodeled_2AC[0x2B0 - 0x2AC];                          /* +0x2AC */
} BattleRosterTable;

/* Play-state header at gSaveGameHeader, 0x2C bytes (newGameStateReset clears it): the start time of the game, the
   copy of the roster pair battleRosterSnapshotTake (ptSaveUMNParty) takes, and the memory-card port and file of the
   last save. */
typedef struct SaveGameHeader {
    s32 startTime;                                            /* +0x00: xglClockDayTime2UInt of the new-game clock */
    BattleRosterPair roster;                                  /* +0x04: copied from gBattleRosterTable.rosterPair */
    u8 snapshotFlags;                                         /* +0x24: bit 1 set by battleRosterSnapshotTake */
    u8 port;                                                  /* +0x25 */
    u8 fileIndex;                                             /* +0x26 */
    u8 unmodeled_27[0x2C - 0x27];                             /* +0x27 */
} SaveGameHeader;

/* 0xC-byte Yes/No choice prompt (choicePromptInit / clampedCounterInit, choicePromptUpdate, choicePromptDraw). The
   save screen embeds one at +0x5B610; the stream player and the SE/voice screen embed others. */
typedef struct McGauge {
    const char *text;           /* +0x00: title text, drawn above the Yes/No pair */
    s32 width;                  /* +0x04: box width, at least the text width + 0x30 */
    u8 state;                   /* +0x08: selected answer (0 = message 0x74, 1 = message 0x75) */
    u8 negative;                /* +0x09: highlight fade of the other answer, (1 - state) << 7 at init */
    s8 fadeIn;                  /* +0x0A: fade-in level, 0 at init; the fade stops when it goes negative */
    u8 flag;                    /* +0x0B: phase, 0x80 until counterEntryAdvancePhase starts the fade */
} McGauge;

/* One save file of the card directory, as read into the file list. */
typedef struct McFileEntry {
    u32 playTime;               /* +0x00 */
    s32 unmodeled04;            /* +0x04 */
    s32 textIds[2];             /* +0x08: message ids of the two text lines, 0 = none */
    s16 row;                    /* +0x10 */
    s16 rowSecond;              /* +0x12 */
    s8 rowFlag;                 /* +0x14 */
    u8 fileState;               /* +0x15: file state */
    u8 pictureState;            /* +0x16: picture slot state */
    u8 thumbnailSlot;           /* +0x17: slot that loads the entry */
} McFileEntry;

/* Save file list with its scroll bar (mcFileListScrollInit, addClampedCounter, mcFileListScrollStep). It starts with
   the 0x1C-byte DrawEntry (drawEntryReset / SetValue / SetFlag / SetAlpha / SetScale) that draws the bar;
   that entry's f32 at +0x10 is the scale base mcFileListScrollStep multiplies by. The save screen embeds the list at
   +0x5B61C. addClampedCounter moves position (+0x20) and clamps it to 0..cursor (+0x1C);
   mcFileListScrollStep steps previousCursor (+0x1E) toward position. */
typedef struct McFileList {
    u8 unmodeled_00[0x10];      /* +0x00: DrawEntry */
    f32 spriteOffset;           /* +0x10: DrawEntry spriteOffset, the scale base of the bar */
    u8 unmodeled_14[0x1C - 0x14];
    s16 cursor;                 /* +0x1C: upper limit of position */
    s16 previousCursor;         /* +0x1E: displayed value, stepped toward position */
    u16 position;               /* +0x20: u16, addClampedCounter loads it with lhu */
    u8 unmodeled_22[2];
    s16 drawAlpha;              /* +0x24 */
    u8 unmodeled_26[2];
    s16 fadingAlpha;            /* +0x28 */
    u8 unmodeled_2A[2];
    u16 portResult[2];          /* +0x2C, indexed by port */
    McFileEntry fileEntries[50]; /* +0x30 */
    s32 freeSize;               /* +0x4E0 */
    s32 scroll;                 /* +0x4E4 */
    s32 fadeBase;               /* +0x4E8 */
    s32 scrollTarget;           /* +0x4EC */
} McFileList;

extern BattleRosterTable gBattleRosterTable;
extern SaveGameHeader gSaveGameHeader;

void choicePromptInit(McGauge *gauge, const char *text, s32 stepDelta, s32 width);
void clampedCounterInit(McGauge *gauge, s32 stepDelta);
s32 choicePromptUpdate(McGauge *gauge, s32 active);
void counterEntryAdvancePhase(McGauge *gauge);
void choicePromptDraw(McGauge *gauge, s32 visible, s32 y);
void mcFileListScrollInit(McFileList *fileList, s32 position, s32 cursor);
void addClampedCounter(McFileList *fileList, s16 delta);
void mcFileListScrollStep(McFileList *fileList, u16 step);
s32 sheetLatchedPositionGet(void);
s32 testFlagBit(s32 bitIndex);
void newGameStateReset(void);
BattleRoster *battleRosterGet(s32 rosterIndex);
s32 battleRosterEntryIdCountGet(s32 rosterIndex);
s32 battleRosterEntryIdGet(s32 entryIdIndex, s32 rosterIndex);
s32 battleRosterCursorEntryIdGet(s32 rosterIndex);
s32 battleCurrentRosterEntryCountGet(void);
s32 battleCurrentRosterEntryIdGet(s32 entryIdIndex);
s32 battleCurrentRosterCursorEntryIdGet(void);
s32 battleRosterHasEntryId(s32 entryId);
void battleCurrentRosterIndexSet(s32 rosterIndex);
void battleRosterEntryIdAdd(s32 entryId);
void battleRosterEntryIdRemove(s32 entryId);
void battleRosterPairReset(void);
void battleRosterEntrySwap(s32 indexA, s32 indexB, s32 rosterIndex);
s32 battleRosterCursorSet(s32 index, s32 rosterIndex);
s32 battleCurrentRosterCursorAdvance(void);
s32 battleRosterEntryIdIndexGet(s32 entryId);
void battleUpdateEntryKinds(s32 firstEntryId, s32 secondEntryId, s32 thirdEntryId);
s32 battleCurrentRosterIndexGet(void);
void battleRosterHeaderPairSet(s32 firstGuestEntryId, s32 secondGuestEntryId);
s32 battleRosterLinkedEntryIdGet(s32 entryId, s32 index);

#endif /* INCLUDE_MAIN_TU176_H */
