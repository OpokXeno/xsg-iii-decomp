#include "common.h"

typedef signed char s8;
typedef int s32;
extern void *xglCdPreloadBuffer;
extern s8 xglCdPreloadState;
s32 xglCdGetAlignedFileSize(s32 *cdFileInfo);
s32 func_001B6F18(s32 *cdFileInfo, s32 flags);
void *memPoolAlloc(s32 size);
s32 completionRequestStart(void *preloadBuffer, s32 flags, s32 size);

void xglCdPreloadStart(void) {
    s32 cdFileInfo[0x20];
    s32 alignedSize;
    void *preloadBuffer;

    if ((xglCdPreloadState <= 0) && (func_001B6F18(&cdFileInfo[0], 0x09010005) != 0)) {
        alignedSize = xglCdGetAlignedFileSize(&cdFileInfo[0]);
        if (alignedSize >= 0) {
            preloadBuffer = memPoolAlloc(alignedSize);
            xglCdPreloadBuffer = preloadBuffer;
            if ((preloadBuffer != 0) && (completionRequestStart(preloadBuffer, 0x09010005, 0x1000) != 0)) {
                xglCdPreloadState = 1;
            }
        }
    }
}

s32 memPoolFree(void *block);
s32 completionFlagsTest(s32);
s32 func_00213C50();

void xglCdPreloadRelease(void) {
    if (xglCdPreloadState == 1) {
        while (completionFlagsTest(0x1000) != 0) {
            func_00213C50();
        }
    }
    if (xglCdPreloadBuffer != 0) {
        memPoolFree(xglCdPreloadBuffer);
        xglCdPreloadBuffer = 0;
    }
    xglCdPreloadState = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu164", func_00217E28);

s32 func_00215570(s32, s32, s32, s32);

void xglDrawCornerLines(s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 cornerY;
    s32 cornerX;
    s32 bottomY;

    cornerY = startY & 0xFFFF;
    cornerX = endX & 0xFFFF;
    bottomY = endY & 0xFFFF;
    func_00215570(startX & 0xFFFF, cornerY, cornerX, cornerY);
    func_00215570(cornerX, cornerY, cornerX, bottomY);
}

INCLUDE_ASM("asm/main/nonmatchings/tu164", func_00217ED8);

INCLUDE_ASM("asm/main/nonmatchings/tu164", func_002180A8);
