#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu283", func_0026BDA0);

INCLUDE_ASM("asm/main/nonmatchings/tu283", func_0026BE00);

INCLUDE_ASM("asm/main/nonmatchings/tu283", func_0026BE60);

typedef int s32;
typedef float f32;
s32 VM_PLUGIN_CNT_GET();
s32 func_0026CE08(s32, s32);
s32 vmArgGet();
s32 vmKeyedEntryGet();

s32 vmKeyedEntryTestFlag(s32 thread, s32 *testPassFlag) {
    s32 testOperand;
    s32 entryKey;
    s32 keyedEntry;

    testOperand = 0;
    if ((VM_PLUGIN_CNT_GET() <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3)) {
        return 0;
    }
    if (VM_PLUGIN_CNT_GET(thread) >= 2) {
        testOperand = vmArgGet(thread, 2);
    }
    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    if (func_0026CE08(keyedEntry, testOperand) != 0) {
        *testPassFlag = 1;
    }
    return 1;
}

s32 func_0026CE48(s32, s32);

s32 vmKeyedEntryTestFlagAlt(s32 thread, s32 *testPassFlag) {
    s32 testOperand;
    s32 entryKey;
    s32 keyedEntry;

    testOperand = 0;
    if ((VM_PLUGIN_CNT_GET() <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3)) {
        return 0;
    }
    if (VM_PLUGIN_CNT_GET(thread) >= 2) {
        testOperand = vmArgGet(thread, 2);
    }
    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    if (func_0026CE48(keyedEntry, testOperand) != 0) {
        *testPassFlag = 1;
    }
    return 1;
}

void func_0026D0E8();

s32 vmKeyedEntryDispatch(s32 thread) {
    s32 entryKey;
    s32 keyedEntry;

    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    func_0026D0E8(keyedEntry);
    return 1;
}

s32 func_0026D238();

s32 vmNativeCheckSetDone(s32 unused_arg0, s32 *doneFlag) {
    s32 nativeCheckResult;

    nativeCheckResult = func_0026D238();
    if (nativeCheckResult == 0) {
        *doneFlag = 1;
    }
    return nativeCheckResult;
}

void movieAuxSetC(f32, f32);

s32 vmArgSetFloatPair(s32 thread) {
    f32 movieAuxFieldC;
    s32 movieAuxFieldD;

    movieAuxFieldC = vmArgGet(thread, 1);
    movieAuxFieldD = vmArgGet(thread, 2);
    movieAuxSetC(movieAuxFieldC, movieAuxFieldD);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu283", func_0026C1A8);

void movieSwitchSet();

s32 vmKeyedEntrySetSwitch(s32 thread) {
    s32 entryKey;
    s32 keyedEntry;

    entryKey = vmArgGet(thread, 1);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    movieSwitchSet(keyedEntry);
    return 1;
}

s32 func_0026D7B8();

s32 msgPutWait(s32 unused_arg0, s32 *done) {
    if (func_0026D7B8() == 0) {
        *done = 1;
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu283", func_0026C378);
