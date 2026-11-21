/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_OV02_TU025_H
#define INCLUDE_OV02_TU025_H

/* Types owned by ov02/tu025 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

typedef struct CallbackSlot CallbackSlot;

typedef void (*SlotCallback)(CallbackSlot *slot);

/*
 * One 0x60-byte entry of the two-entry table at battleCallbackSlotTable. Bit 0x1 of flags
 * is the claim bit (battleSlotClaim). update runs while flags & 2 is clear and
 * is called from the update sweep battleCallbackSlotsRunUpdate; draw runs while flags & 4 is
 * clear. battleCallbackSlotsRunDraw (draw sweep) has no caller inside this overlay.
 */
struct CallbackSlot {
    int claimAndSkipFlags;
    SlotCallback updateHook;
    SlotCallback drawHook;
    unsigned char unmodeled_0C[0x54];
};

#endif
