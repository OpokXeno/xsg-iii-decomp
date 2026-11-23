#include "common.h"

typedef int s32;

s32 xglCdRaiseMaxByte(s32);
s32 vmArgGetOrDefault(s32, s32, s32);

/* Script native: raises the CD max byte to argument 1 (default 0x1E). */
s32 sbNativeCdUpdateMaxByte(s32 vm) {
    s32 cdByteCeiling;

    cdByteCeiling = vmArgGetOrDefault(vm, 1, 0x1E);
    xglCdRaiseMaxByte(cdByteCeiling);
    return 1;
}

extern s32 gScriptStoredValue;
s32 vmArgGet(s32 vm, s32 index);

/* Script native: stores argument 1 in the global gScriptStoredValue. */
s32 sbNativeStoreValueArg(s32 vm) {
    s32 storedArg;

    storedArg = vmArgGet(vm, 1);
    gScriptStoredValue = storedArg;
    return 1;
}

s32 VM_PLUGIN_CNT_GET(s32 vm);
s32 vmArgTagIsZero(s32 vm, s32 index);
void sbSlotJumpRequest(s32 vm, s32 key);
void func_001B7A60(s32 a, s32 b, s32 c, s32 d);

/* Script native: needs at most 3 arguments (else requests a slot jump); stores arg 1 in gScriptStoredValue and starts func_001B7A60. */
s32 sbNativeStoreValueAndStartFx(s32 vm) {
    s32 storedArg;
    s32 secondArg;
    s32 thirdArg;

    if (VM_PLUGIN_CNT_GET(vm) >= 4) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    storedArg = vmArgGet(vm, 1);
    if (vmArgTagIsZero(vm, 2) == 0) {
        secondArg = vmArgGet(vm, 2);
    } else {
        secondArg = 2;
    }
    if (vmArgTagIsZero(vm, 3) == 0) {
        thirdArg = vmArgGet(vm, 3);
    } else {
        thirdArg = 0x7F;
    }
    gScriptStoredValue = storedArg;
    func_001B7A60(storedArg, thirdArg, secondArg, secondArg);
    return 1;
}
