#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_0023FD38);

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef unsigned int u32;
s32 memset(void *dst, s32 value, s32 size);
s32 xglJpegDecode(void *jpegRequest);
/* A zeroed 0x20-byte jpegRequest block for xglJpegDecode: words 0 and 1 hold the two arguments. */
void decodeJpegRequest(void *jpegOutput, void *jpegInput) {
    void *jpegRequest[8];

    memset(&jpegRequest[0], 0, 0x20);
    jpegRequest[0] = jpegInput;
    jpegRequest[1] = jpegOutput;
    xglJpegDecode(&jpegRequest[0]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_0023FDF0);

s32 func_0023FD38(s32 streamSlot);
void func_0023FDF0();
s32 xglCdReadFile(s32 handle, void *buffer, s32 sectors, void (*callback)());
s32 xglCdSetReadTag(s32 slot);

/* The stream state at gStreamPlayerBlock (0x7E3C0 bytes, allocated by allocStreamState). Fields are at their original offsets. */
typedef struct StreamPlayer {
    u8 unmodeled_000[0x70000];
    u8 cdSectorBuffer[0x7E000 - 0x70000];  /* the buffer address passed to xglCdReadFile */
    u8 decodeBlock[0x7E37C - 0x7E000];  /* the block whose address shardFieldInit takes */
    u8 sheetControlBlock[0x7E394 - 0x7E37C];  /* the block whose address setStreamActive passes on */
    u16 channelBaseOffset;                         /* +0x7E394 */
    u8 unmodeled_7e396[0x7E39D - 0x7E396];
    s8 channel;                             /* +0x7E39D */
    u8 unmodeled_7e39e[0x7E3A2 - 0x7E39E];
    s8 readPending[0x7E3AC - 0x7E3A2];     /* +0x7E3A2: per-slot flags, set by startTaggedCdRead */
    s16 channelStartOffset;                        /* +0x7E3AC */
    s8 active;                              /* +0x7E3AE */
    s8 playState;                           /* +0x7E3AF */
    s8 stopLatch;                         /* +0x7E3B0 */
    u8 unmodeled_7e3b1[0x7E3C0 - 0x7E3B1];
} StreamPlayer;

extern StreamPlayer *gStreamPlayerBlock;
#define gStreamPlayer (*gStreamPlayerBlock)

void startTaggedCdRead(s32 streamSlot) {
    s32 cdFileHandle;
    gStreamPlayer.readPending[streamSlot] = 1;
    xglCdSetReadTag(streamSlot);
    cdFileHandle = func_0023FD38(streamSlot);
    xglCdReadFile(cdFileHandle, &gStreamPlayer.cdSectorBuffer, 1, func_0023FDF0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_0023FF00);

s32 messageTextGet(s32);
void sheetSetIndex(s32);
void mcStateSetupRequest(s32, s32);
void func_002213A8(void *, s32, s32, s32);

void setStreamActive(s32 enableStream) {
    s32 primarySheetHandle;
    s32 secondarySheetHandle;
    s32 stopSheetHandle;
    if (enableStream != 0) {
        mcStateSetupRequest(6, 0);
        primarySheetHandle = messageTextGet(0x30F);
        sheetSetIndex(primarySheetHandle);
        secondarySheetHandle = messageTextGet(0x30D);
        func_002213A8(&gStreamPlayer.sheetControlBlock, secondarySheetHandle, 1, 0);
        gStreamPlayer.active = 1;
        gStreamPlayer.playState = 1;
        gStreamPlayer.stopLatch = -0x80;
        gStreamPlayer.channelStartOffset = (gStreamPlayer.channel * 0x670) - gStreamPlayer.channelBaseOffset;
        return;
    }
    mcStateSetupRequest(7, 0);
    stopSheetHandle = messageTextGet(0x30E);
    sheetSetIndex(stopSheetHandle);
    gStreamPlayer.active = 0;
    if ((gStreamPlayer.stopLatch >= 0) && (gStreamPlayer.playState == 1)) {
        gStreamPlayer.playState = 2;
        return;
    }
    gStreamPlayer.playState = 0;
}

void *memPoolAlloc(s32 size);
s32 bufferPointersRewind();
s32 completionBlockClear();
s32 pagedTablePoolInit();

/* Allocates and zeroes the 0x7E3C0-byte stream state at gStreamPlayerBlock. Returns 1 on success. */
s32 allocStreamState(void) {
    StreamPlayer *newStreamPlayer;

    completionBlockClear();
    pagedTablePoolInit();
    newStreamPlayer = memPoolAlloc(0x7E3C0);
    gStreamPlayerBlock = newStreamPlayer;
    memset(newStreamPlayer, 0, 0x7E3C0);
    bufferPointersRewind();
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_00240300);

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_00240360);

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_00240578);

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_00240698);

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_002409A8);

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_00241088);

INCLUDE_ASM("asm/main/nonmatchings/tu193", func_002417E8);

extern unsigned char gStreamTrackTable[];
extern u8 gStreamTrackIndex;
extern s8 gVsyncsPerFrame;
void memPoolInit(s32 base, s32 size);
void memPoolReset();
void func_00225938(s32 a, s16 b, s32 c);
void pagedTablePoolShutdown();
s32 runDecodePipeline();

s32 runStreamPlayback(void) {
    s32 pipelineResult;
    s32 savedPipelineGuard;
    u32 trackRecordWord;
    u32 *trackRecord;

    savedPipelineGuard = gVsyncsPerFrame;
    gVsyncsPerFrame = 1;
    memPoolInit(0x980000, 0x100000);
    gStreamTrackIndex = -1;
    pipelineResult = runDecodePipeline();
    pagedTablePoolShutdown();
    memPoolReset();
    gVsyncsPerFrame = savedPipelineGuard;
    if (gStreamTrackIndex != 0xFF) {
        trackRecord = &((u32 *) gStreamTrackTable)[gStreamTrackIndex * 2];
        trackRecordWord = trackRecord[0];
        func_00225938(trackRecordWord & 0x3FFF, ((s16 *) trackRecord)[1], (trackRecordWord >> 0xE) & 3);
    }
    return pipelineResult;
}

s32 shardFieldInit(void *block);
s32 allocStreamState();
s32 func_00240300();
s32 func_00240360();
s32 func_002417E8();
void seVoiceStartDefault(unsigned int handle);

s32 runDecodePipeline(void) {
    s32 pipelineResult;

    if (allocStreamState() == 0) {
        return -1;
    }
    shardFieldInit(&gStreamPlayer.decodeBlock);
    seVoiceStartDefault(0xB);
    func_00240360();
    pipelineResult = func_002417E8();
    func_00240300();
    return pipelineResult;
}
