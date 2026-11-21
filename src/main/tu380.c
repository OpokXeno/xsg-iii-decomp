#include "common.h"

typedef unsigned char u8;
typedef int s32;
extern unsigned char D_00402858[];
extern unsigned char D_00402B38[];
extern unsigned char D_00403AF0[];
void *memset(void *s, int c, unsigned int n);

/* The SE control block at gSeSoundContext: only the workAreaSelector word at +0x1DBC is claimed here. */
typedef struct SeChannelBank {
    u8 unmodeled_000[0x1DBC];
    s32 workAreaSelector;                           /* +0x1DBC */
} SeChannelBank;

void seScriptTablesClear(void) {
    memset(D_00402858, 0, 0x200);
    memset(D_00403AF0, 0, 0xF);
    memset(D_00402B38, 0, 0x100);
}

s32 tableGetWord(s32 *indexedWords, s32 wordPosition) {
    return indexedWords[wordPosition];
}

INCLUDE_ASM("asm/main/nonmatchings/tu380", func_002C7C50);

void seChannelBankClear(void *channelBank) {
    memset(channelBank, 0, 0x1DC4);
}

void seChannelBankSetSelector(SeChannelBank *channelBank, s32 workAreaSelector) {
    channelBank->workAreaSelector = workAreaSelector;
}

INCLUDE_ASM("asm/main/nonmatchings/tu380", func_002C7C98);

INCLUDE_ASM("asm/main/nonmatchings/tu380", func_002C7CE8);

extern unsigned char gSeIopCommandBuffer[];
s32 func_001B5F28();
s32 func_001B70F0();

void seSendBufferInit(void) {
    extern void xcBlockClear();
    s32 commandBlock[8];

    memset(gSeIopCommandBuffer, 0, 0x800);
    xcBlockClear(&commandBlock[0]);
    commandBlock[2] = (s32) gSeIopCommandBuffer;
    commandBlock[1] = 0x09000006;
    func_001B70F0(&commandBlock[0]);
    func_001B5F28(0, 0, 0);
}

s32 tableGetWord(s32 *indexedWords, s32 wordPosition);

s32 seGetChannelHandle(s32 channelIndex) {
    extern s32 func_002C7C50();
    s32 channelEntryOffset;
    channelEntryOffset = tableGetWord((s32 *) gSeIopCommandBuffer, 1);
    return func_002C7C50(&gSeIopCommandBuffer[channelEntryOffset], channelIndex);
}

extern s32 gSeIopTransferBuffer;
s32 memPoolAlloc(s32);

void seIopTransferBufferAlloc(void) {
    gSeIopTransferBuffer = memPoolAlloc(0x80000);
}

void seSendWorkAreaInit(SeChannelBank *channelBank) {
    extern s32 xcBlockClear();
    s32 commandBlock[8];

    memset((void *) gSeIopTransferBuffer, 0, 0x80000);
    xcBlockClear(&commandBlock[0]);
    if (channelBank->workAreaSelector == 0) {
        commandBlock[1] = 0x09000007;
    } else {
        commandBlock[1] = 0x09000008;
    }
    commandBlock[2] = gSeIopTransferBuffer;
    func_001B70F0(&commandBlock[0]);
    func_001B5F28(0, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu380", func_002C7E88);

s32 memPoolFree(s32);

void seIopTransferBufferFree(void) {
    memPoolFree(gSeIopTransferBuffer);
}

INCLUDE_ASM("asm/main/nonmatchings/tu380", func_002C8128);

s32 seIopTransferBufferGet(void) {
    return gSeIopTransferBuffer;
}

INCLUDE_ASM("asm/main/nonmatchings/tu380", func_002C8498);

void seIdleNoop(void) {

}

void seIopTransferChannelLookup(s32 channelIndex) {
    extern s32 func_002C7C50(s32, s32);
    s32 channelEntryOffset;
    channelEntryOffset = tableGetWord((s32 *) gSeIopTransferBuffer, 0x3D);
    func_002C7C50(gSeIopTransferBuffer + channelEntryOffset, channelIndex);
}
