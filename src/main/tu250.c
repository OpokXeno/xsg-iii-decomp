#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu250", func_00261538);

INCLUDE_ASM("asm/main/nonmatchings/tu250", func_00261608);

typedef float f32;

f32 fixedQ15ToFloat(f32 fixedSample) {
    return fixedSample * 0.000030517578f;
}
