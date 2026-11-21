#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu093", func_001CD5B0);

INCLUDE_ASM("asm/main/nonmatchings/tu093", func_001CD610);

typedef int s32;
s32 sefFlagSetBit26();

s32 sbNativeSynopsisSkip(void) {
    sefFlagSetBit26();
    return 1;
}

s32 func_001C0570();
s32 vmArgGet();

s32 sbNativeArgFlagIsSet(s32 thread) {
    s32 stateSourceId;
    stateSourceId = vmArgGet(thread, 1);
    return func_001C0570(stateSourceId) != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu093", func_001CD758);

INCLUDE_ASM("asm/main/nonmatchings/tu093", func_001CD808);

INCLUDE_ASM("asm/main/nonmatchings/tu093", func_001CDA70);

s32 func_001BBC68();
s32 pendingCountSet();
s32 vmArgTagIsZero();

s32 sbNativeWaitArg(s32 thread) {
    s32 pendingCount;

    if (vmArgTagIsZero(thread, 1) == 0) {
        pendingCount = vmArgGet(thread, 1);
    } else {
        pendingCount = 0;
    }
    func_001BBC68();
    if (pendingCount > 0) {
        pendingCountSet(pendingCount);
    }
    return 1;
}
