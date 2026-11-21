#include "common.h"
#include "shared.h"

#include "main/res_get_path.h"
#include "main/tu352.h"
#include "main/xenoActor.h"
#include "main/xgl_cd.h"
#include "ov02/tu012.h"
#include "ov02/tu060.h"






struct XenoActor;






/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyModelHeader *), the texture (.txy), the
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





void counterBlockIncrementById(short key);

/* Side argument of unitTblGet and the count helpers: players, enemies, or anything else for both tables. */
enum {
    UNIT_SIDE_PLAYER = 0,
    UNIT_SIDE_ENEMY = 1,
    UNIT_SIDE_ALL = 2
};

int unitTblGet(int side, BattleUnit ***sideUnitTable);
int unitTblRemove(BattleUnit *battleUnit);
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
    void *workAllocation; /* textEntryRemove passes this slot to objWorkFree. */
};

extern void *memset(void *destination, int value, unsigned int count);

/*
 * Partial view of the unit record beyond the published ObjectTask prefix
 * (task+work, ending at +0x14, include/shared.h): unitSetInterpTime and
 * unitMotGet both follow a pointer immediately after that prefix, and
 * unitWpnMotSet reads a 4-entry pointer array starting eight bytes further.
 * Both point at actors (struct XenoActor): unitWpnMotSet forwards the entry it reads to
 * ACT_animGetData/ACT_setMotion. No other byte of this wider record is
 * claimed, and the relationship between this pointer and ObjectTask.workAllocation is
 * not evidenced here.
 */
typedef struct BattleUnitActors BattleUnitActors;

struct BattleUnitActors {
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
    BattleUnitActors *actorSets[8];
    ObjectTask *taskHeaders[8];
} UnitTableStorage;

void unitInit(void)
{
    memset(&unitTbl, 0, sizeof(unitTbl));
    unitPlNum = 0;
    unitEnNum = 0;
}

int unitTblGet(int side, BattleUnit ***sideUnitTable)
{
    if (side == UNIT_SIDE_PLAYER) {
        *sideUnitTable = unitTbl;
        return UNIT_PL_SLOTS;
    }
    if (side == UNIT_SIDE_ENEMY) {
        *sideUnitTable = &unitTbl[UNIT_PL_SLOTS];
        return UNIT_EN_SLOTS;
    }
    *sideUnitTable = unitTbl;
    return UNIT_TBL_SLOTS;
}

/*
 * Stores object in the first empty slot of the side's range: side 0 uses slots 0..2 (players), side 1 uses
 * slots 3..7 (enemies). Returns the slot index, or -1 when the side is unknown or no slot is free.
 */
int unitTblSet(int side, BattleUnit *battleUnit)
{
    int sideFirst;
    int sideEnd;
    int i;

    if (side == UNIT_SIDE_PLAYER) {
        sideFirst = 0;
        sideEnd = UNIT_PL_SLOTS;
    } else if (side == UNIT_SIDE_ENEMY) {
        sideFirst = UNIT_PL_SLOTS;
        sideEnd = UNIT_TBL_SLOTS;
    } else {
        return -1;
    }
    for (i = sideFirst; i < sideEnd; i++) {
        if (unitTbl[i] == 0) {
            unitTbl[i] = battleUnit;
            if (i < UNIT_PL_SLOTS) {
                counterBlockIncrementById(battleUnit->lookupKeyRecord->lookupKey);
                unitPlNum++;
            } else {
                unitEnNum++;
            }
            return i;
        }
    }
    return -1;
}

int unitTblRemove(BattleUnit *battleUnit)
{
    BattleUnit **units = unitTbl;
    int i;

    for (i = 0; i < UNIT_TBL_SLOTS; i++) {
        if (units[i] == battleUnit) {
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
    BattleUnit **sideUnitTable;
    int sideUnitCount;
    int i;
    int acceptedCount;

    acceptedCount = 0;
    i = 0;
    sideUnitCount = unitTblGet(side, &sideUnitTable);
    if (sideUnitCount > 0) {
        do {
            if (sideUnitTable[i] != 0) {
                if (!(sideUnitTable[i]->targetingFlags & 0x8000) || calcStatGet(sideUnitTable[i], 1, 1) != 0 ||
                    calcStatGet(sideUnitTable[i], 1, 2) != 0) {
                    acceptedCount++;
                }
            }
            i++;
        } while (i < sideUnitCount);
    }
    return acceptedCount;
}

/* Counts the non-null units of side whose flags word has bit 0x1 set. */
int unitCountFlagged(int side)
{
    BattleUnit **sideUnitTable;
    BattleUnit **cursor;
    int sideUnitCount;
    int remaining;
    int flaggedUnitCount;

    sideUnitCount = unitTblGet(side, &sideUnitTable);
    flaggedUnitCount = 0;
    if (sideUnitCount > 0) {
        cursor = sideUnitTable;
        remaining = sideUnitCount;
        do {
            BattleUnit *sideUnit = *cursor;

            cursor++;
            if (sideUnit != 0) {
                flaggedUnitCount += sideUnit->targetingFlags & 1;
            }
            remaining--;
        } while (remaining != 0);
    }
    return flaggedUnitCount;
}

typedef int s32;

s32 unitPlNumGet(void) {
    return unitPlNum;
}

s32 unitEnNumGet(void) {
    return unitEnNum;
}

/* Entry index of the combined unit table (side 2). The check is index > count,
 * so index == count is not rejected by it. */
BattleUnit *unitTblEntryGet(int combinedUnitIndex)
{
    BattleUnit **combinedUnits;
    int combinedUnitCount;

    combinedUnitCount = unitTblGet(UNIT_SIDE_ALL, &combinedUnits);
    if (combinedUnitCount < combinedUnitIndex || combinedUnitIndex < 0) {
        return 0;
    }
    return combinedUnits[combinedUnitIndex];
}
