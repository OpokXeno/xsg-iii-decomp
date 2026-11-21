/*
 * OV02 original TU 56: 0x00a96d38..0x00a96e70 (4 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu056", func_00A96D38);

typedef int s32;
void actorApplyLevelMode(s32 owner, s32 mode);
s32 func_00A8C2C8(s32 owner, s32 mode);
s32 battleRollPercent(s32);

void unitRollChanceApply(s32 battleUnit, s32 percent) {
    if ((battleRollPercent(percent) != 0) && (func_00A8C2C8(battleUnit, 1) != 0)) {
        actorApplyLevelMode(battleUnit, 1);
    }
}

void ov02EmptyHook(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu056", func_00A96E00);
