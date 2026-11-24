#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef signed char s8;
typedef unsigned short u16;
extern unsigned char gDefaultNativeMethods[];
extern unsigned char gDefaultNativeOwnerName[];
s32 sbNativeOwnerAdd();

void addDefaultNativeOwner(void) {
    sbNativeOwnerAdd(gDefaultNativeOwnerName, gDefaultNativeMethods);
}

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237A30);

typedef struct SeVoiceState {
    u8 unk000[0x37C];
    u8 voiceKindCursor[2];
    u16 voiceKindCount;
    u8 unk380[0x38];
    u8 voiceTableCursor[2];
    u16 voiceTableCount;
    u8 unk3BC[0x38];
    u8 unk3F4[0x74];
    u8 unk468[0xE8];
    u8 unk550[0x472];
    u8 voiceFlags;
} SeVoiceState;

extern SeVoiceState *gSeVoiceState;
#define seVoiceState gSeVoiceState
s32 slotKindResolve(s32);
s32 func_00222C60(s32, s32);
s32 pagedTableGetCurrentEntry();
u8 *func_00A843F8(s32 index);

s32 seVoiceLookupSlot(void) {
    s32 resolvedSlot;
    s32 currentVoiceSlot;
    s32 voiceSlot;
    s32 keyedVoiceSlot;
    s32 voiceKindEntry;
    s32 lookupKey;

    voiceSlot = 0;
    if (seVoiceState->voiceTableCount != 0) {
        currentVoiceSlot = pagedTableGetCurrentEntry(&seVoiceState->voiceTableCursor, 0);
        if ((seVoiceState->voiceKindCount != 0) && (seVoiceState->voiceFlags & 2)) {
            voiceKindEntry = pagedTableGetCurrentEntry(&seVoiceState->voiceKindCursor);
            resolvedSlot = slotKindResolve(voiceKindEntry);
            lookupKey = pagedTableGetCurrentEntry(&seVoiceState->voiceTableCursor);
            keyedVoiceSlot = func_00222C60(resolvedSlot, lookupKey);
        } else {
            keyedVoiceSlot = func_00A843F8(currentVoiceSlot);
        }
        voiceSlot = keyedVoiceSlot;
    }
    return voiceSlot;
}

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237B18);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237B98);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237C30);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237C70);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237CB0);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00237CD0);

s32 xglPadSetDefaults();
s32 memPoolFree(void *);
s32 statusBlockFrameUpdate();
s32 xcBlockFlagsReset(void *);
s32 xglCdServiceIfPending();

/* Waits until the CD service is idle, releases the state block gSeVoiceState and clears the pointer. */
void cdStateDestroy(void) {
    xglPadSetDefaults();
    while (xglCdServiceIfPending() != 0) {
        statusBlockFrameUpdate();
    }
    xcBlockFlagsReset(gSeVoiceState);
    memPoolFree(gSeVoiceState);
    gSeVoiceState = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00238238);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00238728);

s32 func_00229828(s32, void *, void *, void *);

void seVoiceStartEntry(s32 entryId) {
    func_00229828(entryId & 0xFF, &seVoiceState->unk550, &seVoiceState->unk3F4, &seVoiceState->unk468);
}

extern u8 gSeVoiceCallbackFlag;
extern s8 gVsyncsPerFrame;
s32 func_001B9228();
s32 memPoolInit(s32, s32);
s32 memPoolReset();
s32 pagedTablePoolShutdown();
s32 startSeVoiceAndWait();

/* Runs startSeVoiceAndWait with the guard flag gVsyncsPerFrame set, then calls func_001B9228 when gSeVoiceCallbackFlag is set. */
s32 seVoiceStartGuarded(s32 voiceArg) {
    s32 voiceResult;
    s32 savedFlag;

    savedFlag = gVsyncsPerFrame;
    gVsyncsPerFrame = 1;
    memPoolInit(0x980000, 0x100000);
    voiceResult = startSeVoiceAndWait(voiceArg);
    pagedTablePoolShutdown();
    memPoolReset();
    gVsyncsPerFrame = savedFlag;
    if (gSeVoiceCallbackFlag != 0) {
        func_001B9228();
    }
    return voiceResult;
}

s32 func_00237CD0();
void cdStateDestroy();
s32 func_0023BAF8();
void seVoiceStartDefault(unsigned int handle);

/* Starts the default sound effect voice 0xB and waits for its result; returns -1 when func_00237CD0 fails. */
s32 startSeVoiceAndWait(void) {
    s32 voiceResult;

    if (func_00237CD0() == 0) {
        return -1;
    }
    seVoiceStartDefault(0xB);
    voiceResult = func_0023BAF8();
    cdStateDestroy();
    return voiceResult;
}

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_002390E0);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_002391C8);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00239988);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_00239C80);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_0023A718);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_0023AB88);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_0023AE58);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_0023B1A0);

INCLUDE_ASM("asm/main/nonmatchings/tu187", func_0023BAF8);
