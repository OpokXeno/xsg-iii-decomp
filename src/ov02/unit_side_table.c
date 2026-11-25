#include "common.h"
#include "shared.h"

void battleSlotClear(void *slot);
void battleCallbackSlotsClear(void);
void battleTaskClearAll(void);
void monsSlotClearAll(void);
void unitSideClearAll(void);
void battleClearTaskAndSlotTables(void);
void battleResetCallbacksAndTables(void);
void dataObjResetAll(void);

#define OBJ_SLOT_COUNT 64         /* slots in objSlotTable */

struct ObjSlot;

/* One 0xB0-byte slot of the 64-slot table objSlotTable.
 * claimAndSkipFlags bit 0x1: in use (the claim in objSlotAlloc sets it, battleSlotClear clears the word);
 * bit 0x2 skips the update pass (objSlotRunUpdate), bit 0x4 skips the draw pass (objSlotRunDraw). */
struct ObjSlot {
    int claimAndSkipFlags;
    void (*update)(struct ObjSlot *);  /* +0x04: first callback, run by the update pass */
    void (*draw)(struct ObjSlot *);    /* +0x08: second callback, run by the draw pass */
    unsigned char unmodeled_0c[0xA4];
};

extern struct ObjSlot objSlotTable[OBJ_SLOT_COUNT];
void objSlotClearAll(void);
void battleSlotRelease();
void monsSlotReleaseRecord(void);
struct BattleUnit;
void unitSideRelease(struct BattleUnit *battleUnit);

/*
 * Releases one MonsActorRecord record (monsSlotReleaseRecord): the only caller is monsterRelease (mons_slot.c,
 * monsterRelease), which calls it before it releases the actor. Same body as the other release wrappers.
 */
void monsSlotReleaseRecord(void)
{
    battleSlotRelease();
}

#define UNIT_SIDE_COUNT 9

struct UnitSideSlot {
    int claimAndSkipFlags;
    void (*update)(struct UnitSideSlot *);
    void (*draw)(struct UnitSideSlot *);
    unsigned char unmodeled_0c[0x304];
};

static struct UnitSideSlot gUnitSideSlotTable[UNIT_SIDE_COUNT];

void unitSideClearAll(void)
{
    int i;

    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        battleSlotClear(&gUnitSideSlotTable[i]);
    }
}

#define UNIT_SIDE_SKIP_UPDATE 0x2

void unitSideRunUpdate(void)
{
    struct UnitSideSlot *unitSideSlot;
    int i;

    unitSideSlot = gUnitSideSlotTable;
    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        if (unitSideSlot->update != 0 && !(unitSideSlot->claimAndSkipFlags & UNIT_SIDE_SKIP_UPDATE)) {
            unitSideSlot->update(unitSideSlot);
        }
        unitSideSlot++;
    }
}

#define UNIT_SIDE_SKIP_DRAW 0x4

void unitSideRunDraw(void)
{
    struct UnitSideSlot *unitSideSlot;
    int i;

    unitSideSlot = gUnitSideSlotTable;
    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        if (unitSideSlot->draw != 0 && !(unitSideSlot->claimAndSkipFlags & UNIT_SIDE_SKIP_DRAW)) {
            unitSideSlot->draw(unitSideSlot);
        }
        unitSideSlot++;
    }
}

int battleSlotClaim(int *claimFlags);

struct UnitSideSlot *unitSideClaim(void)
{
    struct UnitSideSlot *unitSideSlot;
    int i;

    unitSideSlot = gUnitSideSlotTable;
    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        if (battleSlotClaim(&unitSideSlot->claimAndSkipFlags)) {
            return unitSideSlot;
        }
        unitSideSlot++;
    }
    return 0;
}

/* Releases the unit side slot of a unit (unitRemove calls it; unit side slots). */
void unitSideRelease(struct BattleUnit *battleUnit) {
    battleSlotRelease(battleUnit);
}

void monsSlotClearAll(void);
void unitSideClearAll(void);

void battleClearTaskAndSlotTables(void)
{
    battleTaskClearAll();
    monsSlotClearAll();
    unitSideClearAll();
}

/* battleResetCallbacksAndTables: clears the two-slot callback table of battle_slot.c, then every table (battleClearTaskAndSlotTables). */
void battleResetCallbacksAndTables(void) {
    battleCallbackSlotsClear();
    battleClearTaskAndSlotTables();
}

void battleCallbackSlotsRunUpdate(void);
void battleTaskRunUpdate(void);
void monsSlotRunUpdate(void);
void unitSideRunUpdate(void);

void menuSceneUnloadAssets(void)
{
    battleCallbackSlotsRunUpdate();
    battleTaskRunUpdate();
    monsSlotRunUpdate();
    unitSideRunUpdate();
}

void monsSlotRunDraw(void);
void unitSideRunDraw(void);
void battleTaskRunDraw(void);

void battleRunDrawAll(void)
{
    monsSlotRunDraw();
    unitSideRunDraw();
    battleTaskRunDraw();
}
