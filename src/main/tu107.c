#include "common.h"

typedef int s32;

/* EE scratchpad: the VM natives below build their command packets here. */
#define SCRATCHPAD_BASE 0x70000000
#define SCRATCHPAD_WORDS ((s32 *) SCRATCHPAD_BASE)

s32 vmArgGet();
s32 scratchpadSendCmd1F();
void scratchpadSendCmd0A();
void scratchpadSendCmd27Mode4();
void func_001E82E8();
void scratchpadSendMode0();
void xglRenderGlobalFadeValueSet(s32);
void func_001E8498();
void func_001E8550();
void sbNativeOwnerAdd(s32 *, s32 *);
extern s32 D_00407D00;
extern s32 D_00407D48;
extern s32 D_0053E418;
extern s32 D_0053E420;

s32 vmSpadSubmitTypedPair(s32 vm) {
    s32 packetFirstWord;
    s32 packetThirdWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[1] = 2;
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetThirdWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    scratchpadSendCmd1F(SCRATCHPAD_BASE);
    return 1;
}

s32 vmSpadSubmitTypedOne(s32 vm) {
    s32 packetFirstWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    SCRATCHPAD_WORDS[1] = 2;
    scratchpadSendCmd0A(SCRATCHPAD_BASE);
    return 1;
}

s32 vmSpadSubmitFiveWords(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    scratchpadSendCmd27Mode4(SCRATCHPAD_BASE);
    return 1;
}

s32 vmSpadSubmitFiveWordsAlt(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    func_001E82E8(SCRATCHPAD_BASE);
    return 1;
}

s32 vmSpadFlush(void) {
    func_001E8498();
    xglRenderGlobalFadeValueSet(0);
    return 1;
}

s32 vmSpadCommand49(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    scratchpadSendMode0(0x49, 9, SCRATCHPAD_BASE);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu107", func_001D47A0);

s32 vmSpadCommand5D(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    scratchpadSendMode0(0x5D, 0x1D, SCRATCHPAD_BASE);
    return 1;
}

s32 vmSpadCommand60(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;
    s32 packetSixthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    SCRATCHPAD_WORDS[4] = packetFifthWord;
    packetSixthWord = vmArgGet(vm, 6);
    SCRATCHPAD_WORDS[5] = packetSixthWord;
    SCRATCHPAD_WORDS[6] = 0;
    SCRATCHPAD_WORDS[7] = 0;
    SCRATCHPAD_WORDS[8] = 0;
    scratchpadSendMode0(0x60, 0x20, SCRATCHPAD_BASE);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu107", func_001D49E8);

s32 vmSpadCommand64(s32 vm) {
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    scratchpadSendMode0(0x64, 0x24, SCRATCHPAD_BASE);
    return 1;
}

s32 vmSpadPoll(void) {
    func_001E8550();
    return 1;
}

void sbNativeOwnersRegister(void) {
    sbNativeOwnerAdd(&D_0053E418, &D_00407D00);
    sbNativeOwnerAdd(&D_0053E420, &D_00407D48);
}
