#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu289", func_0026F370);

INCLUDE_ASM("asm/main/nonmatchings/tu289", func_0026F700);

typedef int s32;
typedef long long s64;
typedef float f32;
s32 func_0026FA00(s32, s32, s64);
s64 func_003098D0(f32);

void movieFrameSetFade(s32 scratchQuads, s32 scratchQuadsUsed, f32 fadeAmount)
{
    func_0026FA00(scratchQuads, scratchQuadsUsed, func_003098D0(fadeAmount * 70.0f) << 0x18);
}

INCLUDE_ASM("asm/main/nonmatchings/tu289", func_0026F9B0);
