#include "common.h"

typedef int s32;
s32 runWithStateBlockOnce(s32 mode);

void blendModeClear(void) {
    runWithStateBlockOnce(0);
}

void blendNoopStart(void) {
}

void blendNoopStop(void) {
}

INCLUDE_ASM("asm/main/nonmatchings/BlendTest", func_002419F0);
