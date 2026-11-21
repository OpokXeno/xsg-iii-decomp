#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B45F8);

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B46B0);

typedef unsigned char u8;
typedef int s32;
extern s32 D_005D0EC4[];

/* Stores value clamped to 0..0x80 in slot index; returns 1 when it was above 0x80. */
s32 stateSlotStoreClamped(s32 index, s32 unclampedValue) {
    s32 overflowed;
    s32 clampedValue;

    clampedValue = unclampedValue;
    overflowed = 0;
    if (clampedValue >= 0x81) {
        clampedValue = 0x80;
        overflowed = 1;
    }
    if (clampedValue < 0) {
        clampedValue = 0;
        overflowed = 0;
    }
    D_005D0EC4[index] = clampedValue;
    return overflowed;
}

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B47A0);

extern s32 gMrPackStateAIndex[];

s32 mrPackStateAIndexGet(void) {
    return gMrPackStateAIndex[0];
}

/* The state A block at D_005D0670 is an array of 32-bit words; its 0x18-byte slots are six words each. */
#define STATE_A_TAIL_INDEX_WORD 0x213   /* byte 0x84C */
#define STATE_A_SLOT_WORDS 6
#define STATE_A_VALUE_WORD 0x1D9        /* byte 0x764 inside a slot */

extern s32 D_005D0670[];

/* Reads field +0x764 of the table entry picked by the index stored at +0x84C. */
s32 stateATailValueGet(void)
{
    s32 selectedEntry;

    selectedEntry = D_005D0670[STATE_A_TAIL_INDEX_WORD];
    return D_005D0670[selectedEntry * STATE_A_SLOT_WORDS + STATE_A_VALUE_WORD];
}

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B4848);

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B48C8);

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B4928);

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B4980);

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B4A10);

INCLUDE_ASM("asm/main/nonmatchings/tu365", func_002B4C58);
