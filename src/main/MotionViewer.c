#include "common.h"

typedef unsigned char u8;
typedef int s32;

typedef struct MotionViewerView {
    u8 unk_000[0x504];
    s32 fileBuffer504;
    s32 fileBuffer508;
    s32 fileBuffer50c;
    u8 unk_510[4];
    s32 fileBuffer514;
} MotionViewerView;

void motionViewerNoOp(void) {

}

extern char motionViewerMessageBuffer[];
extern s32 gMotionViewerMessageFrames;
char *strcpy(char *destination, const char *source);

void motionViewerSetMessage(const char *text) {
    gMotionViewerMessageFrames = 0x3C;
    strcpy(motionViewerMessageBuffer, text);
}

extern char motionViewerCaption[];
void xglFontPrintDirect(const char *text);
void xglFontPrint(int x, int y, unsigned int color, const char *text);

void motionViewerDrawMessage(void)
{
  s32 remaining;
  remaining = gMotionViewerMessageFrames - 1;
  if (remaining < 0)
  {
    gMotionViewerMessageFrames = 0;
    return;
  }
  gMotionViewerMessageFrames = remaining;
  if (!(remaining > 0))
  {
    return;
  }
  {
    xglFontPrint(0xA, 0x190, 0xFFFFFF, motionViewerCaption);
    xglFontPrintDirect(motionViewerMessageBuffer);
  }
}

extern char motionViewerSourceFile[];
extern char motionViewerAllocFailText[];
s32 RgHeapAllocById(s32, s32, const char *, s32);
void assertFailed(const char *message, const char *file, int line);
s32 xglDebFileRead(s32 fileId, s32 fileBuffer);
s32 xglDebFileSize(s32);

s32 motionViewerLoadFile(s32 heapId, s32 fileId) {
    s32 fileSize;
    s32 fileBuffer;

    fileBuffer = 0;
    fileSize = xglDebFileSize(fileId);
    if (fileSize > 0) {
        fileBuffer = RgHeapAllocById(heapId, fileSize + 0x10, motionViewerSourceFile, 0x86);
        if (fileBuffer == 0) {
            assertFailed(motionViewerAllocFailText, motionViewerSourceFile, 0x87);
        }
        xglDebFileRead(fileId, fileBuffer);
    }
    return fileBuffer;
}

void RgHeapFree(s32 address, void *name, s32 line);

void motionViewerFreeBuffers(MotionViewerView *viewer) {
    s32 pendingFree504;
    s32 pendingFree508;
    s32 pendingFree50c;
    s32 pendingFree514;

    if (viewer != 0) {
        pendingFree504 = viewer->fileBuffer504;
        if (pendingFree504 != 0) {
            RgHeapFree(pendingFree504, motionViewerSourceFile, 0x98);
            viewer->fileBuffer504 = 0;
        }
        pendingFree508 = viewer->fileBuffer508;
        if (pendingFree508 != 0) {
            RgHeapFree(pendingFree508, motionViewerSourceFile, 0x9C);
            viewer->fileBuffer508 = 0;
        }
        pendingFree50c = viewer->fileBuffer50c;
        if (pendingFree50c != 0) {
            RgHeapFree(pendingFree50c, motionViewerSourceFile, 0xA0);
            viewer->fileBuffer50c = 0;
        }
        pendingFree514 = viewer->fileBuffer514;
        if (pendingFree514 != 0) {
            RgHeapFree(pendingFree514, motionViewerSourceFile, 0xA4);
            viewer->fileBuffer514 = 0;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/MotionViewer", func_00248060);

INCLUDE_ASM("asm/main/nonmatchings/MotionViewer", func_00248198);

INCLUDE_ASM("asm/main/nonmatchings/MotionViewer", func_002483C8);
