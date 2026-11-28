#include "common.h"

typedef unsigned char u8;

typedef int s32;

typedef struct MotionViewerView {
    u8 unk_000[0x504];
    s32 pxyModelData;
    s32 txyTextureData;
    s32 xhrData;
    u8 unk_510[4];
    s32 xapMotionData;
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

void xglFontPrintDirect(const char *text);
void xglFontPrint(int x, int y, unsigned int color, const char *text);

void motionViewerDrawMessage(void) {
    s32 remaining;
    remaining = gMotionViewerMessageFrames - 1;
    if (remaining < 0) {
        gMotionViewerMessageFrames = 0;
        return;
    }
    gMotionViewerMessageFrames = remaining;
    if (remaining <= 0) {
        return;
    }
    {
        xglFontPrint(0xA, 0x190, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col1060f0;");
        xglFontPrintDirect(motionViewerMessageBuffer);
    }
}

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
        fileBuffer = RgHeapAllocById(heapId, fileSize + 0x10, "MotionViewer.euc.c", 0x86);
        if (fileBuffer == 0) {
            assertFailed("pData", "MotionViewer.euc.c", 0x87);
        }
        xglDebFileRead(fileId, fileBuffer);
    }
    return fileBuffer;
}

void RgHeapFree(s32 address, void *name, s32 line);

void motionViewerFreeBuffers(MotionViewerView *viewer) {
    s32 pxyModelData;
    s32 txyTextureData;
    s32 xhrData;
    s32 xapMotionData;

    if (viewer != 0) {
        pxyModelData = viewer->pxyModelData;
        if (pxyModelData != 0) {
            RgHeapFree(pxyModelData, "MotionViewer.euc.c", 0x98);
            viewer->pxyModelData = 0;
        }
        txyTextureData = viewer->txyTextureData;
        if (txyTextureData != 0) {
            RgHeapFree(txyTextureData, "MotionViewer.euc.c", 0x9C);
            viewer->txyTextureData = 0;
        }
        xhrData = viewer->xhrData;
        if (xhrData != 0) {
            RgHeapFree(xhrData, "MotionViewer.euc.c", 0xA0);
            viewer->xhrData = 0;
        }
        xapMotionData = viewer->xapMotionData;
        if (xapMotionData != 0) {
            RgHeapFree(xapMotionData, "MotionViewer.euc.c", 0xA4);
            viewer->xapMotionData = 0;
        }
    }
}

char *func_0030EF98(char *fileName, const char *extension);
MotionViewerView *func_0028D670(void);
void XenoActorRelease(void *actor);

/* Opens the .pxy model and .txy texture of fileName into *viewerSlot (a new view from func_0028D670).
 * A missing extension or a failed load shows a message; a failed load also releases the slot. */
void motionViewerOpenPxyTxy(MotionViewerView **viewerSlot, char *fileName) {
    char *extension;

    extension = func_0030EF98(fileName, ".pxy");
    if (extension == 0) {
        motionViewerSetMessage("PXY\244\362\301\252\302\362\244\267\244\306\244\257\244\300\244\265\244\244");
        return;
    }
    if (*viewerSlot != 0) {
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    }
    *viewerSlot = func_0028D670();
    extension[1] = 'p';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->pxyModelData = motionViewerLoadFile(1, fileName);
    if ((*viewerSlot)->pxyModelData == 0) {
        motionViewerSetMessage("PXY\244\254\244\242\244\352\244\336\244\273\244\363");
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    extension[1] = 't';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->txyTextureData = motionViewerLoadFile(1, fileName);
    if ((*viewerSlot)->txyTextureData == 0) {
        motionViewerSetMessage("TEX\244\254\244\242\244\352\244\336\244\273\244\363");
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    }
}

extern u8 motionViewerEmptyName[];
void func_0028D760(MotionViewerView *viewer);

/* Opens the .pxy, .txy, .xhr and .xap files of fileName into *viewerSlot, the same way as motionViewerOpenPxyTxy.
 * Also copies the file name after the last '/' to baseName and calls func_0028D760 on the view. */
void motionViewerOpenSet(MotionViewerView **viewerSlot, char *fileName, char *baseName) {
    char *extension;

    extension = func_0030EF98(fileName, ".pxy");
    if (extension == 0) {
        motionViewerSetMessage("PXY\244\362\301\252\302\362\244\267\244\306\244\257\244\300\244\265\244\244");
        return;
    }
    *baseName = motionViewerEmptyName[0];
    if (*viewerSlot != 0) {
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    }
    *viewerSlot = func_0028D670();
    extension[1] = 'p';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->pxyModelData = motionViewerLoadFile(1, fileName);
    if ((*viewerSlot)->pxyModelData == 0) {
        motionViewerSetMessage("PXY\244\254\244\242\244\352\244\336\244\273\244\363");
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    extension[1] = 't';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->txyTextureData = motionViewerLoadFile(1, fileName);
    if ((*viewerSlot)->txyTextureData == 0) {
        motionViewerSetMessage("TEX\244\254\244\242\244\352\244\336\244\273\244\363");
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    extension[1] = 'x';
    extension[2] = 'h';
    extension[3] = 'r';
    (*viewerSlot)->xhrData = motionViewerLoadFile(1, fileName);
    if ((*viewerSlot)->xhrData == 0) {
        motionViewerSetMessage("XHR\244\254\244\242\244\352\244\336\244\273\244\363");
        motionViewerFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    extension[1] = 'x';
    extension[2] = 'a';
    extension[3] = 'p';
    (*viewerSlot)->xapMotionData = motionViewerLoadFile(2, fileName);
    extension[0] = 0;
    /* kept: the do-while (not while) decides the emitted loop shape */
    do {
        extension--;
    } while (*extension != '/');
    strcpy(baseName, extension + 1);
    func_0028D760(*viewerSlot);
}

s32 sprintf(char *movieName, const char *text);
void XenoActorSetMode(void *actor, s32 mode);

/* Replaces the .xap motion of the open view with fileName, then sets the view's mode to 0. */
void motionViewerLoadMotion(MotionViewerView **viewerSlot, char *fileName) {
    char movieName[64];
    char *extension;
    MotionViewerView *viewer;

    extension = func_0030EF98(fileName, ".xap");
    if (extension == 0) {
        sprintf(movieName, ".xap\244\307\244\317\244\242\244\352\244\336\244\273\244\363");
        motionViewerSetMessage(movieName);
        return;
    }
    viewer = *viewerSlot;
    if (viewer == 0) {
        motionViewerSetMessage(
            "\245\342\245\307\245\353\244\362\306\311\244\337\271\376\244\363\244\307\244\257\244\300\244\265\244\244");
        return;
    }
    if (viewer->xapMotionData != 0) {
        RgHeapFree(viewer->xapMotionData, "MotionViewer.euc.c", 0x136);
        (*viewerSlot)->xapMotionData = 0;
    }
    extension[1] = 'x';
    extension[2] = 'a';
    extension[3] = 'p';
    (*viewerSlot)->xapMotionData = motionViewerLoadFile(2, fileName);
    func_0028D760(*viewerSlot);
    XenoActorSetMode(*viewerSlot, 0);
}

char motionViewerMessageBuffer[64] = { 0 };
s32 gMotionViewerMessageFrames = 0;
u8 motionViewerEmptyName[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
