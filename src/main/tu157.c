#include "common.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

/* Header of a memory-pool block; it sits 0x0C bytes before the block address handed out to callers. */
typedef struct MemPoolBlock {
    /* 0x00 */ s32 blockSizeCopy;
    /* 0x04 */ s32 footprintBytes;
    /* 0x08 */ s32 previousBlock;
} MemPoolBlock;

s32 unsignedDecimalDigits(u32 number) {
    s32 digits;
    u32 remaining;

    remaining = number;
    digits = 1;
    if (remaining >= 0xAU) {
        do {
            remaining = remaining / 10U;
            digits += 1;
        } while (remaining >= 0xAU);
    }
    return digits;
}

INCLUDE_ASM("asm/main/nonmatchings/tu157", func_00213490);

extern s32 D_00550B50;
extern s32 D_00550B54;
extern s32 D_00550B58;
extern s32 memPoolTop;
extern s32 memPoolLastBlock;
extern s32 D_00550B64;

void memPoolReset(void) {
    D_00550B64 = 0;
    memPoolLastBlock = 0;
    memPoolTop = 0;
    D_00550B58 = 0;
    D_00550B54 = 0;
    D_00550B50 = 0;
}

s32 memPoolAlloc(s32 requestedBytes) {
    s32 blockAddress;
    s32 oldTop;
    s32 previousBlock;
    s32 newTop;
    s32 blockFootprintBytes;
    MemPoolBlock *memPoolHeader;

    oldTop = memPoolTop;
    previousBlock = memPoolLastBlock;
    memPoolHeader = (MemPoolBlock *) (oldTop - 0xC);
    memPoolHeader->previousBlock = previousBlock;
    newTop = ((oldTop + requestedBytes) + 0x80B) & (~0x7FF);
    blockAddress = oldTop;
    blockFootprintBytes = newTop - oldTop;
    memPoolTop = newTop;
    memPoolHeader->footprintBytes = blockFootprintBytes;
    memPoolHeader->blockSizeCopy = blockFootprintBytes;
    memPoolLastBlock = oldTop;
    return blockAddress;
}

void func_002135C0(void);

void memPoolFree(s32 address) {
    s32 mergedSize;
    s32 nextAddress;
    s32 freeEndBelowTop;
    u32 freedBlockEnd;
    u32 poolTop;
    MemPoolBlock *memPoolHeader;
    MemPoolBlock *nextMemPoolHeader;

    memPoolHeader = (MemPoolBlock *) (address - 0xC);
    nextAddress = memPoolHeader->previousBlock;
    nextMemPoolHeader = (MemPoolBlock *) (nextAddress - 0xC);
    if (address != nextAddress) {
        poolTop = memPoolTop;
        mergedSize = nextMemPoolHeader->footprintBytes + memPoolHeader->footprintBytes;
        nextMemPoolHeader->footprintBytes = mergedSize;
        freedBlockEnd = address + memPoolHeader->footprintBytes;
        freeEndBelowTop = freedBlockEnd < poolTop;
        if (freeEndBelowTop != 0) {
            ((s32 *) freedBlockEnd)[-1] = nextAddress;
        }
        memPoolHeader->previousBlock = address;
        func_002135C0();
    }
}

void func_00213490(s32, s32);

void memPoolFlush(void) {
    func_00213490(D_00550B64, D_00550B54);
}

INCLUDE_ASM("asm/main/nonmatchings/tu157", func_002135C0);

extern s32 completionDoneFlags;

/* Completion request: filled in by completionRequestStart and handed to the library's request queue. */
typedef struct CompletionRequest {
    /* 0x00 */ s32 queueStatus;
    /* 0x04 */ s32 parameter;
    /* 0x08 */ s32 address;
    /* 0x0C */ void (*onComplete)(struct CompletionRequest *completedRequest);
    /* 0x10 */ s32 doneFlag;
} CompletionRequest;

void completionRequestClearFlag(CompletionRequest *completedRequest) {
    completionDoneFlags &= ~completedRequest->doneFlag;
}

void completionFlagsReset(void) {
    completionDoneFlags = 0;
}

void completionFlagsSet(s32 mask) {
    completionDoneFlags |= mask;
}

s32 completionFlagsAny(void) {
    return completionDoneFlags > 0;
}

s32 completionFlagsTest(s32 mask) {
    return (completionDoneFlags & mask) > 0;
}

void xcBlockClear(void *blockAddress);
s32 func_001B70F0();
void completionRequestClearFlag(CompletionRequest *completedRequest);
void completionFlagsSet(s32 mask);

s32 completionRequestStart(s32 address, s32 parameter, s32 doneFlag) {
    CompletionRequest completionRequest; /* compiler-managed */
    s32 succeeded;

    xcBlockClear(&completionRequest.queueStatus);
    completionRequest.parameter = parameter;
    completionRequest.onComplete = completionRequestClearFlag;
    completionRequest.address = address;
    completionRequest.queueStatus = 0;
    completionRequest.doneFlag = doneFlag;
    succeeded = func_001B70F0(&completionRequest.queueStatus);
    if (succeeded != 0) {
        completionFlagsSet(doneFlag);
        succeeded = 1;
    }
    return succeeded;
}
