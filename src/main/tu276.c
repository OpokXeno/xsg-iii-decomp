#include "common.h"

typedef float f32;
typedef int s32;

void blendWeightSetFull(f32 *blendWeight) {
    *blendWeight = 1.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu276", func_00269A78);

s32 fractionSquareTo30(f32 *fraction) {
    f32 fractionValue;

    fractionValue = *fraction;
    if (fractionValue <= 0.0f) {
        return 0;
    }
    return fractionValue * fractionValue * 30.0f;
}

s32 fractionInvSquareTo128(f32 *fraction) {
    f32 squareComplement;

    if (*fraction <= 0.0f) {
        return 0x80;
    }
    squareComplement = *fraction * *fraction;
    squareComplement = 1.0f - squareComplement;
    return squareComplement * 128.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu276", func_00269B40);
