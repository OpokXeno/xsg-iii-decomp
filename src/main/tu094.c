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

INCLUDE_ASM("asm/main/nonmatchings/tu094", func_001CDBB8);
