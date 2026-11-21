#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu371", func_002C0D90);

INCLUDE_ASM("asm/main/nonmatchings/tu371", func_002C1028);

typedef int s32;
extern s32 D_005E255C[];

s32 dataTableGetWord(s32 tableWordPos) {
    return D_005E255C[tableWordPos];
}
