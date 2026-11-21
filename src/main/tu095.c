#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu095", func_001CDC98);

typedef int s32;
void *func_001CED38();
s32 vmArgGet(s32 vm, s32 argIndex);

void vmNativeSetKind1Value(s32 vm) {
    s32 value;
    value = vmArgGet(vm, 1);
    func_001CED38(1, value);
}

INCLUDE_ASM("asm/main/nonmatchings/tu095", func_001CDDD8);

INCLUDE_ASM("asm/main/nonmatchings/tu095", func_001CDEB0);
