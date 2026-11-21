#include "common.h"

typedef unsigned char u8;
typedef int s32;

typedef struct CflLightTestView {
    u8 unk_000[0x504];
    s32 fileBuffer504;
    s32 fileBuffer508;
    s32 fileBuffer50c;
    u8 unk_510[4];
    s32 fileBuffer514;
} CflLightTestView;

extern char cflLightTestMessageBuffer[];
extern s32 gCflLightTestMessageFrames;
char *strcpy(char *destination, const char *source);

void cflLightTestSetMessage(const char *text) {
    gCflLightTestMessageFrames = 0x3C;
    strcpy(cflLightTestMessageBuffer, text);
}

extern char cflLightTestCaption[];
void xglFontPrintDirect(const char *text);
void xglFontPrint(int x, int y, unsigned int color, const char *text);

void cflLightTestDrawMessage(void)
{
  s32 remaining;
  remaining = gCflLightTestMessageFrames - 1;
  if (remaining < 0)
  {
    gCflLightTestMessageFrames = 0;
    return;
  }
  gCflLightTestMessageFrames = remaining;
  if (!(remaining > 0))
  {
    return;
  }
  {
    xglFontPrint(0xA, 0x190, 0xFFFFFF, cflLightTestCaption);
    xglFontPrintDirect(cflLightTestMessageBuffer);
  }
}

extern char cflLightTestSourceFile[];
extern char cflLightTestAllocFailText[];
s32 RgHeapAllocById(s32, s32, const char *, s32);
void assertFailed(const char *message, const char *file, int line);
s32 xglDebFileRead(s32 fileId, s32 fileBuffer);
s32 xglDebFileSize(s32);

s32 cflLightTestLoadFile(s32 heapId, s32 fileId) {
    s32 fileSize;
    s32 fileBuffer;

    fileBuffer = 0;
    fileSize = xglDebFileSize(fileId);
    if (fileSize > 0) {
        fileBuffer = RgHeapAllocById(heapId, fileSize + 0x10, cflLightTestSourceFile, 0x86);
        if (fileBuffer == 0) {
            assertFailed(cflLightTestAllocFailText, cflLightTestSourceFile, 0x87);
        }
        xglDebFileRead(fileId, fileBuffer);
    }
    return fileBuffer;
}

void RgHeapFree(s32 address, void *name, s32 line);

void cflLightTestFreeBuffers(CflLightTestView *viewer) {
    s32 pendingFree504;
    s32 pendingFree508;
    s32 pendingFree50c;
    s32 pendingFree514;

    if (viewer != 0) {
        pendingFree504 = viewer->fileBuffer504;
        if (pendingFree504 != 0) {
            RgHeapFree(pendingFree504, cflLightTestSourceFile, 0x98);
            viewer->fileBuffer504 = 0;
        }
        pendingFree508 = viewer->fileBuffer508;
        if (pendingFree508 != 0) {
            RgHeapFree(pendingFree508, cflLightTestSourceFile, 0x9C);
            viewer->fileBuffer508 = 0;
        }
        pendingFree50c = viewer->fileBuffer50c;
        if (pendingFree50c != 0) {
            RgHeapFree(pendingFree50c, cflLightTestSourceFile, 0xA0);
            viewer->fileBuffer50c = 0;
        }
        pendingFree514 = viewer->fileBuffer514;
        if (pendingFree514 != 0) {
            RgHeapFree(pendingFree514, cflLightTestSourceFile, 0xA4);
            viewer->fileBuffer514 = 0;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/CFLLightTest", func_00242080);

INCLUDE_ASM("asm/main/nonmatchings/CFLLightTest", func_002421E0);

INCLUDE_ASM("asm/main/nonmatchings/CFLLightTest", func_00242420);
