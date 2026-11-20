#include "common.h"
#include "shared.h"

void ov01SlotClear(void *slot);
void ov01CallbackSlotsClear(void);
void ov01TaskClearAll(void);
void monsSlotClearAll(void);
void unitSideClearAll(void);
void ov01ClearTables(void);
void ov01ResetAll(void);
void dataObjResetAll(void);

#define OBJ_SLOT_COUNT 64         /* slots in objSlotTable */

struct ObjSlot;

/* One 0xB0-byte slot of the 64-slot table objSlotTable.
 * flags bit 0x1: in use (the claim in objSlotAlloc sets it, ov01SlotClear clears the word);
 * bit 0x2 skips the update pass (objSlotRunUpdate), bit 0x4 skips the draw pass (objSlotRunDraw). */
struct ObjSlot {
    int flags;
    void (*update)(struct ObjSlot *);  /* +0x04: first callback, run by the update pass */
    void (*draw)(struct ObjSlot *);    /* +0x08: second callback, run by the draw pass */
    unsigned char unmodeled_0c[0xA4];
};

extern struct ObjSlot objSlotTable[OBJ_SLOT_COUNT];
void objSlotClearAll(void);
void ov01SlotRelease();
void monsSlotReleaseRecord(void);
struct BattleUnit;
void unitSideRelease(struct BattleUnit *unit);

/*
 * Releases one MonsSlot record (monsSlotReleaseRecord): the only caller is monsSlotRelease (mons_slot.c,
 * monsSlotRelease), which calls it before it releases the actor. Same body as the other release wrappers.
 */
void monsSlotReleaseRecord(void)
{
    ov01SlotRelease();
}

#define UNIT_SIDE_COUNT 9

struct UnitSideSlot {
    int flags;
    void (*update)(struct UnitSideSlot *);
    void (*draw)(struct UnitSideSlot *);
    unsigned char unmodeled_0c[0x304];
};

extern struct UnitSideSlot D_00ABFF70[UNIT_SIDE_COUNT];

void unitSideClearAll(void)
{
    int i;

    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        ov01SlotClear(&D_00ABFF70[i]);
    }
}

#define UNIT_SIDE_SKIP_UPDATE 0x2

void unitSideRunUpdate(void)
{
    struct UnitSideSlot *slot;
    int i;

    slot = D_00ABFF70;
    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        if (slot->update != 0 && !(slot->flags & UNIT_SIDE_SKIP_UPDATE)) {
            slot->update(slot);
        }
        slot++;
    }
}

#define UNIT_SIDE_SKIP_DRAW 0x4

void unitSideRunDraw(void)
{
    struct UnitSideSlot *slot;
    int i;

    slot = D_00ABFF70;
    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        if (slot->draw != 0 && !(slot->flags & UNIT_SIDE_SKIP_DRAW)) {
            slot->draw(slot);
        }
        slot++;
    }
}

int ov01SlotClaim(int *flags);

struct UnitSideSlot *unitSideClaim(void)
{
    struct UnitSideSlot *slot;
    int i;

    slot = D_00ABFF70;
    for (i = 0; i < UNIT_SIDE_COUNT; i++) {
        if (ov01SlotClaim(&slot->flags)) {
            return slot;
        }
        slot++;
    }
    return 0;
}

/* Releases the unit side slot of a unit (unitRemove calls it; unit side slots). */
void unitSideRelease(struct BattleUnit *unit) {
    ov01SlotRelease(unit);
}

void monsSlotClearAll(void);
void unitSideClearAll(void);

void ov01ClearTables(void)
{
    ov01TaskClearAll();
    monsSlotClearAll();
    unitSideClearAll();
}

/* ov01ResetAll: clears the two-slot callback table of ov01_slot.c, then every table (ov01ClearTables). */
void ov01ResetAll(void) {
    ov01CallbackSlotsClear();
    ov01ClearTables();
}

void ov01CallbackSlotsRunUpdate(void);
void ov01TaskRunUpdate(void);
void monsSlotRunUpdate(void);
void unitSideRunUpdate(void);

void menuSceneUnloadAssets(void)
{
    ov01CallbackSlotsRunUpdate();
    ov01TaskRunUpdate();
    monsSlotRunUpdate();
    unitSideRunUpdate();
}

void monsSlotRunDraw(void);
void unitSideRunDraw(void);
void ov01TaskRunDraw(void);

void func_00A8B9C0(void)
{
    monsSlotRunDraw();
    unitSideRunDraw();
    ov01TaskRunDraw();
}
