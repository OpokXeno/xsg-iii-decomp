#include "common.h"

typedef int s32;
typedef struct RgDispModel RgDispModel;

/* Argument block passed by address to the native lookup helpers. */
typedef struct NativeArgPair {
    /* 0x0 */ s32 first;
    /* 0x4 */ s32 second;
} NativeArgPair;

typedef struct NativeArgTriple {
    /* 0x0 */ s32 first;
    /* 0x4 */ s32 second;
    /* 0x8 */ s32 third;
} NativeArgTriple;

void func_001C3FD0();

void nativeInvokePairVia3FD0(s32 first, s32 second) {
    NativeArgPair nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    func_001C3FD0(&nativeArgs.first);
}

void func_001C2990();

void nativeInvokeTripleVia2990(s32 second, s32 third) {
    NativeArgTriple nativeArgs;

    nativeArgs.second = second;
    nativeArgs.third = third;
    nativeArgs.first = 0;
    func_001C2990(&nativeArgs.first);
}

INCLUDE_ASM("asm/main/nonmatchings/find_native_method", func_001CAFD8);

void func_001C3A40();

void nativeInvokePairVia3A40(s32 first, s32 second) {
    NativeArgPair nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    func_001C3A40(&nativeArgs.first);
}

void func_001C4410();

void nativeInvokePairVia4410(s32 first, s32 second) {
    NativeArgPair nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    func_001C4410(&nativeArgs.first);
}

s32 func_001C0788(s32 *argument);

void getClassFromSignature(s32 signature) {
    s32 argument;

    argument = signature;
    func_001C0788(&argument);
}

void func_001C1C88();

void nativeInvokePairVia1C88(s32 first, s32 second) {
    NativeArgPair nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    func_001C1C88(&nativeArgs.first);
}

void *func_001C0570(s32 index);
void RgDispModelLink(RgDispModel *model, RgDispModel *partner);

s32 rgDispModelLinkPairAt(s32 modelIndex) {
    RgDispModel *model;
    RgDispModel *partner;

    model = func_001C0570(modelIndex);
    partner = func_001C0570(modelIndex + 1);
    if (!((model != 0) && (partner != 0))) {
        return 0;
    }
    RgDispModelLink(model, partner);
    return 1;
}

void func_001C7100();

void nativeInvokeTripleSwapVia7100(s32 first, s32 second, s32 third) {
    NativeArgTriple nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = third;
    nativeArgs.third = second;
    func_001C7100(&nativeArgs.first);
}

void func_001C2F78();

void nativeInvokeTripleSwapVia2F78(s32 first, s32 second, s32 third) {
    NativeArgTriple nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = third;
    nativeArgs.third = second;
    func_001C2F78(&nativeArgs.first);
}

void func_001C27C0();

void nativeInvokeTripleVia27C0(s32 first, s32 second, s32 third) {
    NativeArgTriple nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    nativeArgs.third = third;
    func_001C27C0(&nativeArgs.first);
}

void func_001C2670();
s32 func_0020F790();

void nativeInvokeTripleLookupVia2670(s32 first, s32 second, s32 third) {
    NativeArgTriple nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = third;
    nativeArgs.third = func_0020F790(second);
    func_001C2670(&nativeArgs.first);
}

void func_001C6830();

void nativeInvokeTripleVia6830(s32 first, s32 second, s32 third) {
    NativeArgTriple nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    nativeArgs.third = third;
    func_001C6830(&nativeArgs.first);
}

void func_001C6E10();

void nativeInvokePairVia6E10(s32 first, s32 second) {
    NativeArgPair nativeArgs;

    nativeArgs.first = first;
    nativeArgs.second = second;
    func_001C6E10(&nativeArgs.first);
}
