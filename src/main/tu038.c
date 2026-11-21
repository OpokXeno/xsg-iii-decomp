#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu038", func_0019CF80);

INCLUDE_ASM("asm/main/nonmatchings/tu038", func_0019D3F8);

INCLUDE_ASM("asm/main/nonmatchings/tu038", func_0019D4E8);

typedef int s32;
typedef float f32;
extern s32 D_00551230;
extern f32 D_00551234;
extern s32 D_00551238;

void setKindWeightCount(s32 kind, s32 count, f32 weight) {
    D_00551230 = kind;
    D_00551234 = weight;
    D_00551238 = count;
}

INCLUDE_ASM("asm/main/nonmatchings/tu038", func_0019D660);
