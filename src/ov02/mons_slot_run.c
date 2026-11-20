#include "common.h"
#include "shared.h"

void ov01SlotRelease();
void ov01TaskRelease(void *task);

/* Ends a task (ov01TaskRelease): the fade steps, screenFlashTick, screenFillEnd and msgObjRelease call it. */
void ov01TaskRelease(void *task)
{
    ov01SlotRelease(task);
}

void ov01SlotClear(void *slot);
int ov01SlotClaim(int *flags);

/* Bits of a slot record's flags word: the claim bit of ov01SlotClaim, and the update and draw sweeps skip a record with the matching bit. */
#define OV01_SLOT_CLAIM 0x1
#define OV01_SLOT_SKIP_UPDATE 0x2
#define OV01_SLOT_SKIP_DRAW 0x4
/* Entries in the two-entry callback table at 0x00abfeb0. */
#define OV01_CALLBACK_SLOT_COUNT 2

#include "ov02/tu025.h"



extern CallbackSlot D_00ABFEB0[OV01_CALLBACK_SLOT_COUNT];

/* Clears both entries of the callback table (ov01SlotClear on each). */
void monsSlotClearAll(void)
{
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        ov01SlotClear(&D_00ABFEB0[i]);
    }
}

typedef struct SlotEntry SlotEntry;

typedef void (*SlotEntryCallback)(SlotEntry *slot);

/* One 0x60-byte entry: flags, the update and draw callbacks, then bytes this TU does not read. */
struct SlotEntry {
    int flags;
    SlotEntryCallback update;
    SlotEntryCallback draw;
    unsigned char unmodeled_0C[0x54];
};

/* Update sweep over the callback table: runs each entry's update unless flags has the update-skip bit. */
void monsSlotRunUpdate(void)
{
    SlotEntryCallback update;
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        update = D_00ABFEB0[i].update;
        if (update != 0 && !(D_00ABFEB0[i].flags & OV01_SLOT_SKIP_UPDATE)) {
            update(&D_00ABFEB0[i]);
        }
    }
}

/* Draw sweep over the callback table: runs each entry's draw unless flags has the draw-skip bit. */
void monsSlotRunDraw(void)
{
    SlotEntryCallback draw;
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        draw = D_00ABFEB0[i].draw;
        if (draw != 0 && !(D_00ABFEB0[i].flags & OV01_SLOT_SKIP_DRAW)) {
            draw(&D_00ABFEB0[i]);
        }
    }
}

/* Claims the first free entry of the callback table: returns it, or 0 when both entries are claimed. */
SlotEntry *monsSlotClaim(void)
{
    int i;

    for (i = 0; i < OV01_CALLBACK_SLOT_COUNT; i++) {
        if (ov01SlotClaim(&D_00ABFEB0[i].flags)) {
            return &D_00ABFEB0[i];
        }
    }
    return 0;
}
