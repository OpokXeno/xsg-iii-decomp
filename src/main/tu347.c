#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
extern unsigned char gUndulateActiveFlags[];

void undulateSetAllFlags(void)
{
    s32 index;
    u8 *cursor;

    if (gUndulateActiveFlags)
    {
        cursor = &gUndulateActiveFlags[0x1F];
    }
    else
    {
        cursor = &gUndulateActiveFlags[0x1F];
    }
    for (index = 0; index < 32; index++)
    {
        *cursor = 1;
        cursor -= 1;
    }
}

void undulateSetFlag(u32 index, s32 value) {
    if (index < 0x20U) {
        gUndulateActiveFlags[index] = value;
    }
}

extern s32 D_00551598;

void undulateSetAllBits(void) {
    D_00551598 = -1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu347", func_002A4E10);
