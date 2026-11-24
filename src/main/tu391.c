#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu391", func_002D3890);

typedef int s32;
typedef unsigned char u8;
extern s32 D_004C83F8[];

void gameClearRuntimeFlag(void) {
    D_004C83F8[0] = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu391", func_002D3900);

signed char soundTableByteGet(s32 index);
s32 vmArgGet(s32 vm, s32 argIndex);

void sbNativeTableByteRead(s32 vm) {
    s32 byteRow;

    byteRow = vmArgGet(vm, 1);
    soundTableByteGet(byteRow - 1);
}

extern u8 D_00402858[];

s32 sbStatusFlag10Test(void) {
    if ((D_00402858[0] & 0x10) != 0) {
        return 1;
    }
    return 0;
}

s32 func_002C9820(s32 count, s32 index);

s32 sbNativeIndexedCall(s32 vm) {
    s32 callCount;
    s32 callIndex;
    s32 callResult;

    callCount = vmArgGet(vm, 1);
    callIndex = vmArgGet(vm, 2);
    callResult = 0;
    if (callCount > 0) {
        callResult = func_002C9820(callCount, callIndex);
    }
    return callResult;
}

s32 sbStatusLevelGet(void) {
    if (!(!(D_00402858[0] & 4))) {
        return 2;
    }
    if ((D_00402858[0] & 2) != 0) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu391", func_002D3B28);
