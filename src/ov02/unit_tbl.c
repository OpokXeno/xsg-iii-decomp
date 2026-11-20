#include "common.h"
#include "shared.h"

#include "main/res_get_path.h"
#include "main/tu352.h"
#include "main/xenoActor.h"
#include "main/xgl_cd.h"
#include "ov02/tu012.h"
#include "ov02/tu060.h"






struct XenoActor;






/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyHeader *), the texture (.txy), the
 * hierarchy (.xhr, XenoActorAllocMatrices builds the matrices from it), the motion tables (.xap: slot n is
 * XAct_Rsrc_XAP + n, slots 0..5 = indices 4..9) and a few entries xenoActor.c reads as pointers. The names of the
 * first seven are those of the original's asserts; no assert names the rest. */
enum XActRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,                            /* entry passed to tiMRPackageSetAnimData */
    XAct_Rsrc_XAP,                          /* OffsetTable * of slot 0 */
    XAct_Rsrc_5,
    XAct_Rsrc_XAP_Ext2,
    XAct_Rsrc_7,
    XAct_Rsrc_SlotEntry,                    /* slot 4, read as an int entry (the motion set's second entry) */
    XAct_Rsrc_SlotTableB,                   /* slot 5, read as another table */
    XAct_Rsrc_PAM,                          /* non-zero when a PAM pose is active */
    XAct_Rsrc_EntryTableC,                  /* tiMRPackage range entry C */
    XAct_Rsrc_Count
};




#define DATA_LOOKUP_CAPACITY 12 /* items in a DataLookup block */




struct BattleUnit;

/* A unit has three child units (and three slot flags, one per child); the command table has 40 handlers. */
#define UNIT_CHILD_COUNT 3


struct FadeTask;





void func_00A970E0(short key);

/* Side argument of unitTblGet and the count helpers: players, enemies, or anything else for both tables. */
enum {
    UNIT_SIDE_PLAYER = 0,
    UNIT_SIDE_ENEMY = 1,
    UNIT_SIDE_ALL = 2
};

int unitTblGet(int side, BattleUnit ***table);
int unitTblRemove(BattleUnit *object);
BattleUnit *unitTblEntryGet(int index);

/* Slot layout of unitTbl: players use slots 0 to UNIT_PL_SLOTS - 1, enemies the next UNIT_EN_SLOTS of UNIT_TBL_SLOTS. */
#define UNIT_PL_SLOTS 3
#define UNIT_EN_SLOTS 5
#define UNIT_TBL_SLOTS 8

/* The unit objects of the battle: players first, then enemies. */
extern BattleUnit *unitTbl[UNIT_TBL_SLOTS];
extern int unitPlNum;
extern int unitEnNum;

typedef struct ObjectTask ObjectTask;

void unitInit(void);
struct XenoLight;





extern XglTaskPrefix *xglTaskEntryPrev(XglTaskScheduler *scheduler,
                                int (*callback)(XglTaskPrefix *task),
                                XglTaskPrefix *entry);
extern XglTaskPrefix *xglTaskEntryNext(XglTaskScheduler *scheduler,
                                int (*callback)(XglTaskPrefix *task),
                                XglTaskPrefix *entry);

struct ObjectTask {
    XglTaskPrefix task; /* the scheduler header xglTaskEntryNext maintains. */
    void *work; /* textEntryRemove passes this slot to objWorkFree. */
};

extern void *memset(void *destination, int value, unsigned int count);

/*
 * Partial view of the unit record beyond the published ObjectTask prefix
 * (task+work, ending at +0x14, include/shared.h): unitSetInterpTime and
 * unitMotGet both follow a pointer immediately after that prefix, and
 * unitWpnMotSet reads a 4-entry pointer array starting eight bytes further.
 * Both point at actors (struct XenoActor): unitWpnMotSet forwards the entry it reads to
 * ACT_animGetData/ACT_setMotion. No other byte of this wider record is
 * claimed, and the relationship between this pointer and ObjectTask.work is
 * not evidenced here.
 */
typedef struct UnitRecord UnitRecord;

struct UnitRecord {
    ObjectTask task;                   /* +0x00..+0x13: the ObjectTask
                                         * prefix, opaque here */
    struct XenoActor *motionActor;                /* +0x14 */
    unsigned char unmodeled_018[0x1c - 0x18]; /* +0x18..+0x1b */
    struct XenoActor *wpnActor[4];                /* +0x1c */
    struct XenoActor *subActor;                   /* +0x2c: unitRemoveActor disposes it
                                         * first when it is set */
};

/* The same eight unit pointers have full-record and scheduler-prefix views. */
typedef union UnitTableStorage {
    UnitRecord *records[8];
    ObjectTask *tasks[8];
} UnitTableStorage;

void unitInit(void)
{
    memset(&unitTbl, 0, sizeof(unitTbl));
    unitPlNum = 0;
    unitEnNum = 0;
}

int unitTblGet(int side, BattleUnit ***table)
{
    if (side == UNIT_SIDE_PLAYER) {
        *table = unitTbl;
        return UNIT_PL_SLOTS;
    }
    if (side == UNIT_SIDE_ENEMY) {
        *table = &unitTbl[UNIT_PL_SLOTS];
        return UNIT_EN_SLOTS;
    }
    *table = unitTbl;
    return UNIT_TBL_SLOTS;
}

/*
 * Stores object in the first empty slot of the side's range: side 0 uses slots 0..2 (players), side 1 uses
 * slots 3..7 (enemies). Returns the slot index, or -1 when the side is unknown or no slot is free.
 */
int unitTblSet(int side, BattleUnit *object)
{
    int first;
    int end;
    int i;

    if (side == UNIT_SIDE_PLAYER) {
        first = 0;
        end = UNIT_PL_SLOTS;
    } else if (side == UNIT_SIDE_ENEMY) {
        first = UNIT_PL_SLOTS;
        end = UNIT_TBL_SLOTS;
    } else {
        return -1;
    }
    for (i = first; i < end; i++) {
        if (unitTbl[i] == 0) {
            unitTbl[i] = object;
            if (i < UNIT_PL_SLOTS) {
                func_00A970E0(object->poolEntry->lookupKey);
                unitPlNum++;
            } else {
                unitEnNum++;
            }
            return i;
        }
    }
    return -1;
}

int unitTblRemove(BattleUnit *object)
{
    BattleUnit **units = unitTbl;
    int i;

    for (i = 0; i < UNIT_TBL_SLOTS; i++) {
        if (units[i] == object) {
            units[i] = 0;
            if (i < UNIT_PL_SLOTS) {
                unitPlNum--;
            } else {
                unitEnNum--;
            }
            return 1;
        }
    }
    return 0;
}

int calcStatGet(BattleUnit *unit, int row, int flag);

/* Counts the units of side that are not flagged 0x8000, or that calcStatGet accepts for row 1 with flag 1 or 2. */
int unitLiveNumGet(int side)
{
    BattleUnit **records;
    int count;
    int i;
    int acceptedCount;

    acceptedCount = 0;
    i = 0;
    count = unitTblGet(side, &records);
    if (count > 0) {
        do {
            if (records[i] != 0) {
                if (!(records[i]->flags & 0x8000) || calcStatGet(records[i], 1, 1) != 0 ||
                    calcStatGet(records[i], 1, 2) != 0) {
                    acceptedCount++;
                }
            }
            i++;
        } while (i < count);
    }
    return acceptedCount;
}

/* Counts the non-null units of side whose flags word has bit 0x1 set. */
int func_00A99160(int side)
{
    BattleUnit **records;
    BattleUnit **cursor;
    int count;
    int remaining;
    int flagOneCount;

    count = unitTblGet(side, &records);
    flagOneCount = 0;
    if (count > 0) {
        cursor = records;
        remaining = count;
        do {
            BattleUnit *record = *cursor;

            cursor++;
            if (record != 0) {
                flagOneCount += record->flags & 1;
            }
            remaining--;
        } while (remaining != 0);
    }
    return flagOneCount;
}

INCLUDE_ASM("asm/nonmatchings/ov02/unit_tbl", func_00A991C0);

INCLUDE_ASM("asm/nonmatchings/ov02/unit_tbl", func_00A991D0);

/* Entry index of the combined unit table (side 2). The check is index > count,
 * so index == count is not rejected by it. */
BattleUnit *unitTblEntryGet(int index)
{
    BattleUnit **records;
    int count;

    count = unitTblGet(UNIT_SIDE_ALL, &records);
    if (count < index || index < 0) {
        return 0;
    }
    return records[index];
}
