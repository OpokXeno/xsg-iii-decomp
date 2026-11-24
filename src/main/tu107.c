#include "common.h"

#include "main/xgl_2.h"
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
void slotReleaseFixedSlots();
void func_001E8550();
void sbNativeOwnerAdd(s32 *, s32 *);
extern s32 D_00407D00;
extern s32 D_00407D48;
extern s32 D_0053E418;
extern s32 D_0053E420;

/* VM native: word 1 is 2, words 0 and 2 are VM args; sent with cmd 0x1F. */
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

/* VM native: word 1 is 2, word 0 is one VM arg; sent with cmd 0x0A. */
s32 vmSpadSubmitTypedOne(s32 vm) {
    s32 packetFirstWord;

    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    SCRATCHPAD_WORDS[1] = 2;
    scratchpadSendCmd0A(SCRATCHPAD_BASE);
    return 1;
}

/* VM native: five VM args as packet words 0-4, sent through scratchpadSendCmd27Mode4. */
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

/* Same five-word packet as vmSpadSubmitFiveWords, sent through func_001E82E8. */
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

/* VM native: calls slotReleaseFixedSlots and sets the global fade value to 0. */
s32 vmSpadFlush(void) {
    slotReleaseFixedSlots();
    xglRenderGlobalFadeValueSet(0);
    return 1;
}

/* VM native: five VM args as packet words 0-4, command 0x49 (9 words). */
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

XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

/* VM native: builds a 6-word packet (command 0x46) from the active camera window: word 0 is 0xffffff, words 1-2
   the window centre, words 3-4 its size, then three VM args. */
s32 func_001D47A0(s32 vm) {
    XglCamera *camera;
    s32 left;
    s32 top;
    s32 width;
    s32 height;
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;

    camera = xglStudioSelectGetActiveCamera(0);
    left = (s32) camera->screen.window.component[0];
    top = (s32) camera->screen.window.component[1];
    width = (s32) camera->screen.window.component[2] - left;
    height = (s32) camera->screen.window.component[3] - top;
    SCRATCHPAD_WORDS[0] = 0xffffff;
    SCRATCHPAD_WORDS[1] = left + (width >> 1);
    SCRATCHPAD_WORDS[2] = top + (height >> 1);
    SCRATCHPAD_WORDS[3] = width;
    SCRATCHPAD_WORDS[4] = height;
    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[5] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[6] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[7] = packetThirdWord;
    scratchpadSendMode0(0x46, 6, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: five VM args as packet words 0-4, command 0x5D (0x1D words). */
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

/* VM native: six VM args as packet words 0-5, words 6-8 zeroed, command 0x60 (0x20 words). */
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

/* VM native: command 0x60 packet from the active camera window: five VM args, with words 4-6 offset by 8 and by the
   window's half width and half height (the offsets are in words 4, 5 and 6). */
s32 func_001D49E8(s32 vm) {
    XglCamera *camera;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 width;
    s32 height;
    s32 packetFirstWord;
    s32 packetSecondWord;
    s32 packetThirdWord;
    s32 packetFourthWord;
    s32 packetFifthWord;
    s32 packetSixthWord;
    s32 packetSeventhWord;
    s32 packetEighthWord;
    s32 packetNinthWord;

    camera = xglStudioSelectGetActiveCamera(0);
    left = (s32) camera->screen.window.component[0];
    top = (s32) camera->screen.window.component[1];
    right = (s32) camera->screen.window.component[2];
    bottom = (s32) camera->screen.window.component[3];
    packetFirstWord = vmArgGet(vm, 1);
    SCRATCHPAD_WORDS[0] = packetFirstWord;
    packetSecondWord = vmArgGet(vm, 2);
    SCRATCHPAD_WORDS[1] = packetSecondWord;
    packetThirdWord = vmArgGet(vm, 3);
    SCRATCHPAD_WORDS[2] = packetThirdWord;
    packetFourthWord = vmArgGet(vm, 4);
    SCRATCHPAD_WORDS[3] = packetFourthWord;
    packetFifthWord = vmArgGet(vm, 5);
    width = right - left;
    SCRATCHPAD_WORDS[4] = packetFifthWord + 8;
    packetSixthWord = vmArgGet(vm, 6);
    height = bottom - top;
    SCRATCHPAD_WORDS[5] = packetSixthWord + width / 2;
    packetSeventhWord = vmArgGet(vm, 7);
    SCRATCHPAD_WORDS[6] = packetSeventhWord + height / 2;
    packetEighthWord = vmArgGet(vm, 8);
    SCRATCHPAD_WORDS[7] = packetEighthWord;
    packetNinthWord = vmArgGet(vm, 9);
    SCRATCHPAD_WORDS[8] = packetNinthWord;
    scratchpadSendMode0(0x60, 0x20, SCRATCHPAD_BASE);
    return 1;
}

/* VM native: four VM args as packet words 0-3, command 0x64 (0x24 words). */
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

/* VM native: calls func_001E8550 (meaning not yet identified) and returns 1. */
s32 vmSpadPoll(void) {
    func_001E8550();
    return 1;
}

/* Registers the two native owners (their data addresses) with sbNativeOwnerAdd. */
void sbNativeOwnersRegister(void) {
    sbNativeOwnerAdd(&D_0053E418, &D_00407D00);
    sbNativeOwnerAdd(&D_0053E420, &D_00407D48);
}
