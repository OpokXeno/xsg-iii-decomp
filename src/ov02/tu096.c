/*
 * OV02 original TU 96: 0x00ab6298..0x00ab6770 (11 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB6298);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB62E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB63A0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB6460);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB64E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB65A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB66B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu096", func_00AB66E0);

typedef int s32;
s32 func_00AB62E8();
s32 func_00AB66B0(s32);

void roboScriptApplySecond(void) {
    s32 resolvedSecondArg;
    resolvedSecondArg = func_00AB62E8();
    func_00AB66B0(resolvedSecondArg);
}

s32 func_00AB63A0();
s32 func_00AB66E0(s32);

void roboScriptApplyThird(void) {
    s32 resolvedThirdArg;
    resolvedThirdArg = func_00AB63A0();
    func_00AB66E0(resolvedThirdArg);
}

s32 memset();

void roboScriptBufferClear(void *buffer) {
    memset(buffer, 0, 0x44);
}
