#include "common.h"
#include "shared.h"

/* Bits of a slot record's flags word (CallbackSlot, TaskRecord, MonsActorRecord, UnitSideSlot, DataObjectSlot, ObjSlot):
 * the claim bit of battleSlotClaim, the update sweep skips a record with SKIP_UPDATE, the draw sweep with SKIP_DRAW. */
#define BATTLE_SLOT_CLAIM 0x1

#define BATTLE_SLOT_SKIP_UPDATE 0x2
#define BATTLE_SLOT_SKIP_DRAW 0x4

int battleSlotClaim(int *flags);

void battleSlotClear(void *slot);
void battleSlotRelease();
void *battleCallbackSlotClaim(void);
void battleCallbackSlotRelease(void *slot);
void battleCallbackSlotsClear(void);
void battleCallbackSlotsRunUpdate(void);

/* Entries in the two-entry callback slot table. */
#define BATTLE_CALLBACK_SLOT_COUNT 2

#include "ov02/tu025.h"


extern CallbackSlot battleCallbackSlotTable[BATTLE_CALLBACK_SLOT_COUNT];

void battleSlotClear(void *slot) {
    CallbackSlot *hookSlot = slot;

    hookSlot->claimAndSkipFlags = 0;
    hookSlot->updateHook = 0;
    hookSlot->drawHook = 0;
}

/* Claims a slot whose claim bit is clear: sets flags to 1 and returns 1. */
int battleSlotClaim(int *claimFlags) {
    if (!(*claimFlags & BATTLE_SLOT_CLAIM)) {
        *claimFlags = BATTLE_SLOT_CLAIM;
        return 1;
    }
    return 0;
}

/* Releases one slot: clears the claim bit and both callbacks. Same body as battleSlotClear. */
void battleSlotRelease(CallbackSlot *slot) {
    slot->claimAndSkipFlags = 0;
    slot->updateHook = 0;
    slot->drawHook = 0;
}

/* Returns nonzero while the claim bit 0x1 of *flags is set. */
int battleSlotInUse(int *claimFlags) {
    if ((*claimFlags & BATTLE_SLOT_CLAIM) != 0) {
        return 1;
    }
    return 0;
}

void battleCallbackSlotsClear(void) {
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        battleSlotClear(&battleCallbackSlotTable[i]);
    }
}

/* Update sweep over the two-entry table: runs each slot's update unless flags & 2. */
void battleCallbackSlotsRunUpdate(void) {
    SlotCallback updateHook;
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        updateHook = battleCallbackSlotTable[i].updateHook;
        if (updateHook != 0 && !(battleCallbackSlotTable[i].claimAndSkipFlags & BATTLE_SLOT_SKIP_UPDATE)) {
            updateHook(&battleCallbackSlotTable[i]);
        }
    }
}

/* Draw sweep over the two-entry table: runs each slot's draw unless flags & 4. */
void battleCallbackSlotsRunDraw(void) {
    SlotCallback drawHook;
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        drawHook = battleCallbackSlotTable[i].drawHook;
        if (drawHook != 0 && !(battleCallbackSlotTable[i].claimAndSkipFlags & BATTLE_SLOT_SKIP_DRAW)) {
            drawHook(&battleCallbackSlotTable[i]);
        }
    }
}

/* Claims the first free slot of the table: returns it, or 0 when both slots are claimed. */
void *battleCallbackSlotClaim(void) {
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        if (battleSlotClaim(&battleCallbackSlotTable[i].claimAndSkipFlags)) {
            return &battleCallbackSlotTable[i];
        }
    }
    return 0;
}

void battleCallbackSlotRelease(void *slot) {
    battleSlotRelease(slot);
}
