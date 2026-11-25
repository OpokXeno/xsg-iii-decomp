/* header_harvest: hand-maintained (objects shared by main and the ov02 battle overlay; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_BATTLE_SHARED_H
#define INCLUDE_MAIN_BATTLE_SHARED_H

/* Objects of src/main/battle_shared.c (and gSbThreadBlock of src/main/sb_thread_block.c), which the battle overlay (ov02)
 * and main read and write. */

/* Battle units: the key slots and the experience records have one entry each. */
#define BATTLE_UNIT_COUNT 18

typedef struct SbThreadEntry {
    int entryWord0;                   /* +0x00 */
    int entryWord4;                   /* +0x04: copied into SbThreadBlock.unk_00 */
    int threadSlot;                   /* +0x08 */
} SbThreadEntry;

/* The thread table made by RgDispModelFindByActor; its SbThreadEntry array starts at +0x150. */
typedef struct SbThreadEntryTable {
    int tableFlags;                   /* +0x00 */
    unsigned char unk_004[0x10 - 0x4];
    int tableTypeId;                   /* +0x10: must be 4 to start the threads */
    unsigned char unk_014[0x28 - 0x14];
    int unk_28;                       /* +0x28: passed to sbNativeHandlerMessagePost */
    unsigned char unk_2C[0x150 - 0x2C];
    SbThreadEntry threadEntries[1];   /* +0x150 */
} SbThreadEntryTable;

typedef struct SbThreadBlock {
    int unk_00;                       /* +0x00 */
    int unk_04;                       /* +0x04 */
    int unk_08[4];                    /* +0x08: 16 bytes copied from the game loop runtime */
    int threadFlags;                  /* +0x18 */
    int resumeAction;                 /* +0x1C: selects the resume action in func_001BDEC8; runWithStateBlockOnce skips its run when it is 2 */
    int threadMode;                   /* +0x20 */
    int currentThreadSlot;            /* +0x24 */
    SbThreadEntryTable *threadEntryTable; /* +0x28 */
} SbThreadBlock;

/* One 232-byte entry of the two lookup pools of src/ov02/calc.c; its lookup key is the halfword at +62. */
typedef struct BattlePoolLookup {
    unsigned char unmodeled_00[8];
    int word56;
    unsigned short half60;
    short key;
    unsigned char unmodeled_10[64];
    unsigned char unmodeled_50[2];
    unsigned short half130;
    unsigned char unmodeled_54[4];
    unsigned short half136;
    unsigned char unmodeled_5C[4];
    unsigned short half142;
    unsigned char unmodeled_60[88];
} BattlePoolLookup;

typedef struct BattlePoolEntry {
    unsigned char unmodeled_00[6];
    short unk_06;
    short unk_08;
    short unk_0A;
    short unk_0C;
    unsigned char unmodeled_0E[2];
    signed char unk_10;
    signed char unk_11;
    signed char unk_12;
    signed char unk_13;
    unsigned char unk_14;
    unsigned char unmodeled_15[27];
    BattlePoolLookup keyBlock;
} BattlePoolEntry;

typedef struct Ov02ExpRecord {
    unsigned char unk_00[0x8];
    int expPoints;
    unsigned char unmodeled_0C[0x38 - 0x0C];
    unsigned char flagBits[15];
    unsigned char unmodeled_47[0x58 - 0x47];
} Ov02ExpRecord;

typedef struct Ov02BattleCounterBlock {
    unsigned char unk_000[0x564];
    int eventCountA;            /* +0x564 */
    int eventCountB;            /* +0x568 */
} Ov02BattleCounterBlock;

/* One word per party / enemy record; the save code copies both tables whole. */
#define PARTY_RECORD_WORDS 16
#define ENEMY_RECORD_WORDS 24

/* The battle status words the save code copies: +0x00 compared with 0x63 in ov02. */
typedef struct BattleStatusBlock {
    int statusWords[11];
} BattleStatusBlock;

/* The battle data file loaded by ov02: a section table, then the sections. */
#define BATTLE_DATA_BYTES 0x20000

extern SbThreadBlock gSbThreadBlock;
extern BattlePoolEntry gBattleKeySlots[];
extern Ov02ExpRecord gOv02ExpRecords[];
extern int partyRecordTable[];
extern int enemyRecordTable[];
extern BattleStatusBlock D_004CC320;
extern Ov02BattleCounterBlock gOv02BattleCounters;
extern unsigned char ov02DataSectionTable[];

#endif
