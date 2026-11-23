#include "common.h"

typedef int s32;

typedef struct RgDispModel RgDispModel;

/* Argument block passed by address to the native lookup helpers. */
typedef struct NativeArgPair {
    /* 0x0 */ s32 firstNativeArg;
    /* 0x4 */ s32 secondNativeArg;
} NativeArgPair;

typedef struct NativeArgTriple {
    /* 0x0 */ s32 firstNativeArg;
    /* 0x4 */ s32 secondNativeArg;
    /* 0x8 */ s32 thirdNativeArg;
} NativeArgTriple;

void func_001C3FD0();

void nativeInvokePairVia3FD0(s32 firstNativeArg, s32 secondNativeArg) {
    NativeArgPair nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    func_001C3FD0(&nativeArgs.firstNativeArg);
}

void kindFourSlotCreate();

void nativeInvokeTripleVia2990(s32 secondNativeArg, s32 thirdNativeArg) {
    NativeArgTriple nativeArgs;

    nativeArgs.secondNativeArg = secondNativeArg;
    nativeArgs.thirdNativeArg = thirdNativeArg;
    nativeArgs.firstNativeArg = 0;
    kindFourSlotCreate(&nativeArgs.firstNativeArg);
}

typedef struct NativeMethodEntry {
    /* 0x0 */ short unmodeled_0x0;
    /* 0x2 */ short methodSlot;
} NativeMethodEntry;

NativeMethodEntry *nativeMethodEntryFind(s32 index);
void func_001B5C68(short methodSlot);

s32 func_001CAFD8(s32 firstNativeArg, s32 secondNativeArg) {
    NativeMethodEntry *methodEntry;
    NativeArgTriple nativeArgs;
    s32 invokeResult;

    methodEntry = nativeMethodEntryFind(secondNativeArg);
    if (methodEntry == 0) {
        return 0;
    }
    nativeArgs.firstNativeArg = secondNativeArg;
    nativeArgs.secondNativeArg = firstNativeArg;
    nativeArgs.thirdNativeArg = methodEntry->methodSlot;
    invokeResult = ((s32 (*)(s32 *argument)) kindFourSlotCreate)(&nativeArgs.firstNativeArg);
    if (invokeResult != 0) {
        func_001B5C68(methodEntry->methodSlot);
    }
    return invokeResult;
}

void spawnDisplayRecordBind();

void nativeInvokePairVia3A40(s32 firstNativeArg, s32 secondNativeArg) {
    NativeArgPair nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    spawnDisplayRecordBind(&nativeArgs.firstNativeArg);
}

void func_001C4410();

void nativeInvokePairVia4410(s32 firstNativeArg, s32 secondNativeArg) {
    NativeArgPair nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    func_001C4410(&nativeArgs.firstNativeArg);
}

s32 actorChildSlotCreate(s32 *argument);

void getClassFromSignature(s32 signature) {
    s32 argument;

    argument = signature;
    actorChildSlotCreate(&argument);
}

void slotEntryDoorCreate();

void nativeInvokePairVia1C88(s32 firstNativeArg, s32 secondNativeArg) {
    NativeArgPair nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    slotEntryDoorCreate(&nativeArgs.firstNativeArg);
}

void *RgDispModelFindByKey(s32 index);
void RgDispModelLink(RgDispModel *model, RgDispModel *partner);

s32 rgDispModelLinkPairAt(s32 modelIndex) {
    RgDispModel *model;
    RgDispModel *partner;

    model = RgDispModelFindByKey(modelIndex);
    partner = RgDispModelFindByKey(modelIndex + 1);
    if (!((model != 0) && (partner != 0))) {
        return 0;
    }
    RgDispModelLink(model, partner);
    return 1;
}

void descriptorSlotSpawn();

void nativeInvokeTripleSwapVia7100(s32 firstNativeArg, s32 secondNativeArg, s32 thirdNativeArg) {
    NativeArgTriple nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = thirdNativeArg;
    nativeArgs.thirdNativeArg = secondNativeArg;
    descriptorSlotSpawn(&nativeArgs.firstNativeArg);
}

void actorScanLookupStart();

void nativeInvokeTripleSwapVia2F78(s32 firstNativeArg, s32 secondNativeArg, s32 thirdNativeArg) {
    NativeArgTriple nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = thirdNativeArg;
    nativeArgs.thirdNativeArg = secondNativeArg;
    actorScanLookupStart(&nativeArgs.firstNativeArg);
}

void func_001C27C0();

void nativeInvokeTripleVia27C0(s32 firstNativeArg, s32 secondNativeArg, s32 thirdNativeArg) {
    NativeArgTriple nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    nativeArgs.thirdNativeArg = thirdNativeArg;
    func_001C27C0(&nativeArgs.firstNativeArg);
}

void effectSlotCreate();
s32 func_0020F790();

void nativeInvokeTripleLookupVia2670(s32 firstNativeArg, s32 secondNativeArg, s32 thirdNativeArg) {
    NativeArgTriple nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = thirdNativeArg;
    nativeArgs.thirdNativeArg = func_0020F790(secondNativeArg);
    effectSlotCreate(&nativeArgs.firstNativeArg);
}

void func_001C6830();

void nativeInvokeTripleVia6830(s32 firstNativeArg, s32 secondNativeArg, s32 thirdNativeArg) {
    NativeArgTriple nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    nativeArgs.thirdNativeArg = thirdNativeArg;
    func_001C6830(&nativeArgs.firstNativeArg);
}

void childTableSlotSpawn();

void nativeInvokePairVia6E10(s32 firstNativeArg, s32 secondNativeArg) {
    NativeArgPair nativeArgs;

    nativeArgs.firstNativeArg = firstNativeArg;
    nativeArgs.secondNativeArg = secondNativeArg;
    childTableSlotSpawn(&nativeArgs.firstNativeArg);
}
