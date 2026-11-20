#include "common.h"
#include "shared.h"

void dataVPadModeSet(int mode);
void dataVPadSet(int virtualPad);

extern int padMode;
extern short padData;

#define VIRTUAL_PAD_BIT_COUNT 14

extern unsigned short padConvTbl[];

#define SAVE_STACK_LEVEL_CELLS 5  /* cells per SaveStack level */
#define SAVE_STACK_LEVELS 4       /* levels of a SaveStack (a push to this depth is refused) */

struct SaveStack;

#include "ov02/tu030.h"


/* ov01StackSetSlot: writes value to slot of the current level; returns the cell's byte offset. */
int ov01StackSetSlot(struct SaveStack *stack, int slot, int value) {
    int cellIndex;
    int *cell;

    cellIndex = stack->count * SAVE_STACK_LEVEL_CELLS + slot;
    cell = &stack->cells[cellIndex];
    *cell = value;
    return cellIndex * 4;
}

/* ov01StackGetSlot: reads slot of the current level. */
int ov01StackGetSlot(struct SaveStack *stack, int slot)
{
    int *cell = &stack->cells[stack->count * SAVE_STACK_LEVEL_CELLS + slot];
    return *cell;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BAE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BB60);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BBB0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BC80);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BDC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BDF8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BF90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C058);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C1D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C270);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C2C8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C3B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C418);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C430);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C450);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C468);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C478);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C480);

void dataVPadModeSet(int mode)
{
    padMode = mode;
    padData = 0;
}

void dataVPadSet(int virtualPad)
{
    unsigned short result = 0;
    int shift = 0;
    const unsigned short *entry = padConvTbl;
    virtualPad &= 0xFFFF;
    /* Original entry at 0x00A1D2A0 enters the body before testing the
     * loop count; keep the table pointer advance and 16-bit mask. */
    do {
        if ((virtualPad >> shift) & 1) {
            result |= *entry;
        }
        shift++;
        entry++;
    } while (shift < VIRTUAL_PAD_BIT_COUNT);
    padData = result;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C4F8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C570);
