#include "common.h"
#include "shared.h"

void battleSlotRelease();
void battleTaskRelease(void *task);

/* Ends a task (battleTaskRelease): the fade steps, screenFlashTick, screenFillEnd and msgObjRelease call it. */
void battleTaskRelease(void *task)
{
    battleSlotRelease(task);
}

void battleSlotClear(void *slot);
int battleSlotClaim(int *flags);

/* Bits of a slot record's flags word: the claim bit of battleSlotClaim, and the update and draw sweeps skip a record with the matching bit. */
#define BATTLE_SLOT_CLAIM 0x1
#define BATTLE_SLOT_SKIP_UPDATE 0x2
#define BATTLE_SLOT_SKIP_DRAW 0x4
/* Entries in the two-entry callback table at 0x00abfeb0. */
#define BATTLE_CALLBACK_SLOT_COUNT 2

#include "ov02/tu025.h"



extern CallbackSlot D_00ABFEB0[BATTLE_CALLBACK_SLOT_COUNT];

/* Clears both entries of the callback table (battleSlotClear on each). */
void monsSlotClearAll(void)
{
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        battleSlotClear(&D_00ABFEB0[i]);
    }
}

/* Update sweep over the callback table: runs each entry's update unless flags has the update-skip bit. */
void monsSlotRunUpdate(void)
{
    SlotCallback updateHook;
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        updateHook = D_00ABFEB0[i].updateHook;
        if (updateHook != 0 && !(D_00ABFEB0[i].claimAndSkipFlags & BATTLE_SLOT_SKIP_UPDATE)) {
            updateHook(&D_00ABFEB0[i]);
        }
    }
}

/* Draw sweep over the callback table: runs each entry's draw unless flags has the draw-skip bit. */
void monsSlotRunDraw(void)
{
    SlotCallback drawHook;
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        drawHook = D_00ABFEB0[i].drawHook;
        if (drawHook != 0 && !(D_00ABFEB0[i].claimAndSkipFlags & BATTLE_SLOT_SKIP_DRAW)) {
            drawHook(&D_00ABFEB0[i]);
        }
    }
}

/* Claims the first free entry of the callback table: returns it, or 0 when both entries are claimed. */
CallbackSlot *monsSlotClaim(void)
{
    int i;

    for (i = 0; i < BATTLE_CALLBACK_SLOT_COUNT; i++) {
        if (battleSlotClaim(&D_00ABFEB0[i].claimAndSkipFlags)) {
            return &D_00ABFEB0[i];
        }
    }
    return 0;
}
