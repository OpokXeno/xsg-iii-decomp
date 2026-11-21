/*
 * OV02 original TU 13: 0x00a86508..0x00a86638 (4 functions)
 */
#include "common.h"

typedef unsigned short u16;
typedef int s32;
extern u16 D_00AB7AAE[];

u16 ov02GetU16Entry(s32 tableRow) {
    if (!(tableRow < 0x12)) {
        return 0;
    }
    return D_00AB7AAE[tableRow];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu013", func_00A86530);

s32 Get_Rnd_001B8CD0(s32);
s32 func_001B8D08(s32, s32, s32);

void ov02AdjustRollDown(s32 dropCategory, s32 dropId) {
    s32 dropCategoryIndex;
    s32 dropChancePercent;

    dropCategoryIndex = dropCategory - 1;
    dropChancePercent = Get_Rnd_001B8CD0(dropCategoryIndex) - 1;
    if (dropChancePercent >= 0) {
        func_001B8D08(dropCategoryIndex, dropId, dropChancePercent);
    }
}

void ov02AdjustRollUp(s32 dropCategory, s32 dropId) {
    s32 dropCategoryIndex;
    s32 dropChancePercent;

    dropCategoryIndex = dropCategory - 1;
    dropChancePercent = Get_Rnd_001B8CD0(dropCategoryIndex) + 1;
    if (dropChancePercent < 0x64) {
        func_001B8D08(dropCategoryIndex, dropId, dropChancePercent);
    }
}
