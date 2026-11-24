#include "common.h"

typedef unsigned char u8;
typedef int s32;

/* One view of the CFL light test object: the flags word and the four file blocks. */
typedef struct CflLightTestView {
    u8 unk_000[0x90];
    s32 unmodeled_090;
    u8 unk_094[0x504 - 0x94];
    s32 pxyFile;
    s32 txyFile;
    s32 xhrFile;
    u8 unk_510[4];
    s32 xapFile;
} CflLightTestView;

extern char cflLightTestMessageBuffer[];
extern s32 gCflLightTestMessageFrames;
char *strcpy(char *destination, const char *source);

/* Shows a caption message for 60 frames (0x3C) and copies the text into the message buffer. */
void cflLightTestSetMessage(const char *text) {
    gCflLightTestMessageFrames = 0x3C;
    strcpy(cflLightTestMessageBuffer, text);
}

extern char cflLightTestCaption[];
void xglFontPrintDirect(const char *text);
void xglFontPrint(int x, int y, unsigned int color, const char *text);

/* Counts the message down one frame and draws the caption and the message while frames remain. */
void cflLightTestDrawMessage(void) {
    s32 remaining;

    remaining = gCflLightTestMessageFrames - 1;
    if (remaining < 0) {
        gCflLightTestMessageFrames = 0;
        return;
    }
    gCflLightTestMessageFrames = remaining;
    if (!(remaining > 0)) {
        return;
    }
    xglFontPrint(0xA, 0x190, 0xFFFFFF, cflLightTestCaption);
    xglFontPrintDirect(cflLightTestMessageBuffer);
}

extern char cflLightTestSourceFile[];
extern char cflLightTestAllocFailText[];
s32 RgHeapAllocById(s32, s32, const char *, s32);
void assertFailed(const char *message, const char *file, int line);
s32 xglDebFileRead(s32 fileId, s32 destination);
s32 xglDebFileSize(s32);

/* Loads fileId into a heap block of its size plus 0x10 bytes; returns the block, or 0 for an empty file. */
s32 cflLightTestLoadFile(s32 heapId, s32 fileId) {
    s32 fileSize;
    s32 loadedFileAddr;

    loadedFileAddr = 0;
    fileSize = xglDebFileSize(fileId);
    if (fileSize > 0) {
        loadedFileAddr = RgHeapAllocById(heapId, fileSize + 0x10, cflLightTestSourceFile, 0x86);
        if (loadedFileAddr == 0) {
            assertFailed(cflLightTestAllocFailText, cflLightTestSourceFile, 0x87);
        }
        xglDebFileRead(fileId, loadedFileAddr);
    }
    return loadedFileAddr;
}

void RgHeapFree(s32 address, void *name, s32 line);

/* Frees the four file blocks of a view and clears their fields. */
void cflLightTestFreeBuffers(CflLightTestView *viewer) {
    s32 pxyFile;
    s32 txyFile;
    s32 xhrFile;
    s32 xapFile;

    if (viewer != 0) {
        pxyFile = viewer->pxyFile;
        if (pxyFile != 0) {
            RgHeapFree(pxyFile, cflLightTestSourceFile, 0x98);
            viewer->pxyFile = 0;
        }
        txyFile = viewer->txyFile;
        if (txyFile != 0) {
            RgHeapFree(txyFile, cflLightTestSourceFile, 0x9C);
            viewer->txyFile = 0;
        }
        xhrFile = viewer->xhrFile;
        if (xhrFile != 0) {
            RgHeapFree(xhrFile, cflLightTestSourceFile, 0xA0);
            viewer->xhrFile = 0;
        }
        xapFile = viewer->xapFile;
        if (xapFile != 0) {
            RgHeapFree(xapFile, cflLightTestSourceFile, 0xA4);
            viewer->xapFile = 0;
        }
    }
}

extern char D_0053A168[];
extern char D_0053A170[];
extern char D_0053A188[];
extern char D_0053A198[];
char *func_0030EF98(s32 fileId, const char *extension);
CflLightTestView *func_0028D670(void);
void XenoActorRelease(CflLightTestView *viewer);
void func_0028D760(CflLightTestView *viewer);

/* Loads the .pxy and .txy files of fileId into a new view in viewerSlot; a failed load releases the view. */
void cflLightTestViewLoadPxyTxy(CflLightTestView **viewerSlot, s32 fileId) {
    char *extension;

    extension = func_0030EF98(fileId, D_0053A168);
    if (extension == 0) {
        cflLightTestSetMessage(D_0053A170);
        return;
    }
    if (*viewerSlot != 0) {
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    }
    *viewerSlot = func_0028D670();
    extension[1] = 'p';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->pxyFile = cflLightTestLoadFile(1, fileId);
    if ((*viewerSlot)->pxyFile == 0) {
        cflLightTestSetMessage(D_0053A188);
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    extension[1] = 't';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->txyFile = cflLightTestLoadFile(1, fileId);
    if ((*viewerSlot)->txyFile == 0) {
        cflLightTestSetMessage(D_0053A198);
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    func_0028D760(*viewerSlot);
}

extern char D_0053A1A8[];
extern u8 D_00550D40[];

/* Loads all four files of fileId (.pxy, .txy, .xhr, .xap) into a new view; baseName receives the file's base name. */
void cflLightTestViewLoadAll(CflLightTestView **viewerSlot, s32 fileId, char *baseName) {
    char *extension;

    extension = func_0030EF98(fileId, D_0053A168);
    if (extension == 0) {
        cflLightTestSetMessage(D_0053A170);
        return;
    }
    baseName[0] = D_00550D40[0];
    if (*viewerSlot != 0) {
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    }
    *viewerSlot = func_0028D670();
    (*viewerSlot)->unmodeled_090 |= 0x08000000;
    extension[1] = 'p';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->pxyFile = cflLightTestLoadFile(1, fileId);
    if ((*viewerSlot)->pxyFile == 0) {
        cflLightTestSetMessage(D_0053A188);
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    } else {
        extension[1] = 't';
        extension[2] = 'x';
        extension[3] = 'y';
        (*viewerSlot)->txyFile = cflLightTestLoadFile(1, fileId);
        if ((*viewerSlot)->txyFile == 0) {
            cflLightTestSetMessage(D_0053A198);
            cflLightTestFreeBuffers(*viewerSlot);
            XenoActorRelease(*viewerSlot);
            *viewerSlot = 0;
        } else {
            extension[1] = 'x';
            extension[2] = 'h';
            extension[3] = 'r';
            (*viewerSlot)->xhrFile = cflLightTestLoadFile(1, fileId);
            if ((*viewerSlot)->xhrFile == 0) {
                cflLightTestSetMessage(D_0053A1A8);
                cflLightTestFreeBuffers(*viewerSlot);
                XenoActorRelease(*viewerSlot);
                *viewerSlot = 0;
            } else {
                extension[1] = 'x';
                extension[2] = 'a';
                extension[3] = 'p';
                (*viewerSlot)->xapFile = cflLightTestLoadFile(2, fileId);
                extension[0] = 0;
                do {
                    extension--;
                } while (*extension != '/');
                strcpy(baseName, extension + 1);
                func_0028D760(*viewerSlot);
                return;
            }
        }
    }
}

extern char D_0053A1B8[];
extern char D_0053A1C0[];
extern char D_0053A1D8[];
void GameMpeg2Play(char *buffer, const char *text);
void XenoActorSetMode(CflLightTestView *viewer, s32 mode);

/* Reloads the .xap block of an existing view; without the extension it shows a movie message instead. */
void cflLightTestViewReloadXap(CflLightTestView **viewerSlot, s32 fileId) {
    char movieName[64];
    char *extension;
    CflLightTestView *viewer;

    extension = func_0030EF98(fileId, D_0053A1B8);
    if (extension == 0) {
        GameMpeg2Play(movieName, D_0053A1C0);
        cflLightTestSetMessage(movieName);
        return;
    }
    viewer = *viewerSlot;
    if (viewer == 0) {
        cflLightTestSetMessage(D_0053A1D8);
        return;
    }
    if (viewer->xapFile != 0) {
        RgHeapFree(viewer->xapFile, cflLightTestSourceFile, 0x13B);
        (*viewerSlot)->xapFile = 0;
    }
    extension[1] = 'x';
    extension[2] = 'a';
    extension[3] = 'p';
    (*viewerSlot)->xapFile = cflLightTestLoadFile(2, fileId);
    func_0028D760(*viewerSlot);
    XenoActorSetMode(*viewerSlot, 0);
}
