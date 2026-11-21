#include "common.h"

typedef int s32;

s32 stubReturnZeroPrimary(void) {
    return 0;
}

s32 stubReturnZeroSecondary(void) {
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu180", func_00225780);

INCLUDE_ASM("asm/main/nonmatchings/tu180", func_002257F0);
