#ifndef INCLUDE_OV02_TU025_H
#define INCLUDE_OV02_TU025_H

/* Types owned by ov02/tu025 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

typedef struct CallbackSlot CallbackSlot;

typedef void (*SlotCallback)(CallbackSlot *slot);

/*
 * One 0x60-byte entry of the two-entry table at ov01CallbackSlotTable. Bit 0x1 of flags
 * is the claim bit (ov01SlotClaim). update runs while flags & 2 is clear and
 * is called from the update sweep ov01CallbackSlotsRunUpdate; draw runs while flags & 4 is
 * clear. ov01CallbackSlotsRunDraw (draw sweep) has no caller inside this overlay.
 */
struct CallbackSlot {
    int flags;
    SlotCallback update;
    SlotCallback draw;
    unsigned char unmodeled_0C[0x54];
};

#endif
