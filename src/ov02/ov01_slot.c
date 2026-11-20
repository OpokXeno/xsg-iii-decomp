#include "common.h"
#include "shared.h"

/* Bits of a slot record's flags word (CallbackSlot, TaskRecord, MonsSlot, UnitSideSlot, DataObjectSlot, ObjSlot):
 * the claim bit of ov01SlotClaim, the update sweep skips a record with SKIP_UPDATE, the draw sweep with SKIP_DRAW. */
#define OV01_SLOT_CLAIM 0x1

#define OV01_SLOT_SKIP_UPDATE 0x2
#define OV01_SLOT_SKIP_DRAW 0x4

int ov01SlotClaim(int *flags);

void ov01SlotClear(void *slot);
void ov01SlotRelease();
void *ov01CallbackSlotClaim(void);
void ov01CallbackSlotRelease(void *slot);
void ov01CallbackSlotsClear(void);
void ov01CallbackSlotsRunUpdate(void);

/* Entries in the two-entry callback slot table. */
#define OV01_CALLBACK_SLOT_COUNT 2

#include "ov02/tu025.h"


extern CallbackSlot ov01CallbackSlotTable[OV01_CALLBACK_SLOT_COUNT];

void ov01SlotClear(void *slot) {
    CallbackSlot *callback = slot;

    callback->flags = 0;
    callback->update = 0;
    callback->draw = 0;
}

/* Claims a slot whose claim bit is clear: sets flags to 1 and returns 1. */
int ov01SlotClaim(int *flags) {
    if (!(*flags & OV01_SLOT_CLAIM)) {
        *flags = OV01_SLOT_CLAIM;
        return 1;
    }
    return 0;
}

/* Releases one slot: clears the claim bit and both callbacks. Same body as ov01SlotClear. */
void ov01SlotRelease(CallbackSlot *slot) {
    slot->flags = 0;
    slot->update = 0;
    slot->draw = 0;
}

/* Returns nonzero while the claim bit 0x1 of *flags is set. */
int ov01SlotInUse(int *flags) {
    if ((*flags & OV01_SLOT_CLAIM) != 0) {
        return 1;
    }
    return 0;
}

void ov01CallbackSlotsClear(void) {
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        ov01SlotClear(&ov01CallbackSlotTable[i]);
    }
}

/* Update sweep over the two-entry table: runs each slot's update unless flags & 2. */
void ov01CallbackSlotsRunUpdate(void) {
    SlotCallback update;
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        update = ov01CallbackSlotTable[i].update;
        if (update != 0 && !(ov01CallbackSlotTable[i].flags & OV01_SLOT_SKIP_UPDATE)) {
            update(&ov01CallbackSlotTable[i]);
        }
    }
}

/* Draw sweep over the two-entry table: runs each slot's draw unless flags & 4. */
void ov01CallbackSlotsRunDraw(void) {
    SlotCallback draw;
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        draw = ov01CallbackSlotTable[i].draw;
        if (draw != 0 && !(ov01CallbackSlotTable[i].flags & OV01_SLOT_SKIP_DRAW)) {
            draw(&ov01CallbackSlotTable[i]);
        }
    }
}

/* Claims the first free slot of the table: returns it, or 0 when both slots are claimed. */
void *ov01CallbackSlotClaim(void) {
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        if (ov01SlotClaim(&ov01CallbackSlotTable[i].flags)) {
            return &ov01CallbackSlotTable[i];
        }
    }
    return 0;
}

void ov01CallbackSlotRelease(void *slot) {
    ov01SlotRelease(slot);
}
