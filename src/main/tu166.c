#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu166", func_00218590);

typedef int s32;
void setSteppedValue(s32 stepped, s32 value, s32 target);

void actorSteppedValueClear(s32 actor) {
    setSteppedValue(actor + 0x54, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu166", func_00218660);
