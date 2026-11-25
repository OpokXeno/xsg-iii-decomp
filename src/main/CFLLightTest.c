#include "common.h"
#include "main/tu007.h"
#include "main/xgl_2.h"

typedef unsigned char u8;

typedef int s32;

typedef struct MovieSlotCycle {
    u8 unmodeled_00[0x8];
    unsigned int frameCount; /* +0x08 */
} MovieSlotCycle;

/* One playback object of a movie slot; the fields are the ones any of the slot functions reads. */
typedef struct MovieSlotPlayback {
    u8 unk_00[0x20];
    float posX; /* +0x20 */
    float posY; /* +0x24 */
    float posZ; /* +0x28 */
    u8 unk_2C[0x28];
    float offsetX; /* +0x54 */
    u8 unk_58[0x38];
    unsigned int stateFlags; /* +0x90: bit 0x1000 is set on slot 1 */
    u8 unk_94[0x45C];
    float frameTime; /* +0x4F0: converted to int for GameMpeg2Play */
    u8 unk_4F4[0x20];
    MovieSlotCycle *cycle; /* +0x514 */
} MovieSlotPlayback;

typedef struct MovieMenuItem {
    const char *label;
    void (*onTriangle)(void); /* +0x04: pad button 0x10 */
    void (*onCircle)(void);   /* +0x08: pad button 0x20 */
    void (*onCross)(void);    /* +0x0C: pad button 0x40 */
    void (*onSquare)(void);   /* +0x10: pad button 0x80 */
    void (*update)(void);
    void (*draw)(s32 x, s32 y);
} MovieMenuItem;

void movieDecodeToFixed(void);
void movieFileLoadNext(void);
void movieDecodeToSlot(void);
void moviePlaybackStartNext(void);
void movieSlotMoveCursor(void);
void moviePlaybackStartSwapIds(void);
void moviePlaybackStepFrame(void);
void moviePlaybackDrawInfo(s32 x, s32 y);
void moviePlaybackDrawText(s32 x, s32 y);

/* The four rows of the movie test menu. */
MovieMenuItem gMovieMenuItems[4] = {
    { "\245\336\245\303\245\327", 0, movieDecodeToFixed, 0, 0, 0, 0 },
    { "\245\242\245\257\245\277\241\274", 0, movieFileLoadNext, 0, 0, movieSlotMoveCursor, moviePlaybackDrawText },
    { "\245\342\241\274\245\267\245\347\245\363", 0, movieDecodeToSlot, 0, 0, 0, 0 },
    { "\245\326\245\354\245\363\245\311\263\253\273\317", 0, moviePlaybackStartNext, 0, moviePlaybackStartSwapIds, moviePlaybackStepFrame, moviePlaybackDrawInfo },
};

s32 gMovieSlotIndex = 0;
MovieSlotPlayback *gMovieFixedSlot = 0;
s32 gMovieTestMode = 0;
s32 gMovieMenuCursor = 0;
const char *gMovieMessageText = "host0:/home/xeno/work/";
s32 gCflLightTestMessageFrames = 0;
char gMovieSourceFileName[256] = { 0 };

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


char *strcpy(char *destination, const char *source);

/* Shows a caption message for 60 frames (0x3C) and copies the text into the message buffer. */
void cflLightTestSetMessage(const char *text) {
    gCflLightTestMessageFrames = 0x3C;
    strcpy(cflLightTestMessageBuffer, text);
}

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
    xglFontPrint(0xA, 0x190, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col1060f0;");
    xglFontPrintDirect(cflLightTestMessageBuffer);
}

s32 RgHeapAllocById(s32, s32, const char *, s32);

void assertFailed(const char *message, const char *file, int line);

s32 xglDebFileRead(char *fileName, s32 destination);

s32 xglDebFileSize(s32);

/* Loads fileName into a heap block of its size plus 0x10 bytes; returns the block, or 0 for an empty file. */
s32 cflLightTestLoadFile(s32 heapId, char *fileName) {
    s32 fileSize;
    s32 loadedFileAddr;

    loadedFileAddr = 0;
    fileSize = xglDebFileSize(fileName);
    if (fileSize > 0) {
        loadedFileAddr = RgHeapAllocById(heapId, fileSize + 0x10, "CFLLightTest.euc.c", 0x86);
        if (loadedFileAddr == 0) {
            assertFailed("pData", "CFLLightTest.euc.c", 0x87);
        }
        xglDebFileRead(fileName, loadedFileAddr);
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
            RgHeapFree(pxyFile, "CFLLightTest.euc.c", 0x98);
            viewer->pxyFile = 0;
        }
        txyFile = viewer->txyFile;
        if (txyFile != 0) {
            RgHeapFree(txyFile, "CFLLightTest.euc.c", 0x9C);
            viewer->txyFile = 0;
        }
        xhrFile = viewer->xhrFile;
        if (xhrFile != 0) {
            RgHeapFree(xhrFile, "CFLLightTest.euc.c", 0xA0);
            viewer->xhrFile = 0;
        }
        xapFile = viewer->xapFile;
        if (xapFile != 0) {
            RgHeapFree(xapFile, "CFLLightTest.euc.c", 0xA4);
            viewer->xapFile = 0;
        }
    }
}

char *func_0030EF98(char *fileName, const char *extension);

CflLightTestView *func_0028D670(void);

void XenoActorRelease(CflLightTestView *viewer);

void func_0028D760(CflLightTestView *viewer);

/* Loads the .pxy and .txy files of fileName into a new view in viewerSlot; a failed load releases the view. */
void cflLightTestViewLoadPxyTxy(CflLightTestView **viewerSlot, char *fileName) {
    char *extension;

    extension = func_0030EF98(fileName, ".pxy");
    if (extension == 0) {
        cflLightTestSetMessage("PXY\244\362\301\252\302\362\244\267\244\306\244\257\244\300\244\265\244\244");
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
    (*viewerSlot)->pxyFile = cflLightTestLoadFile(1, fileName);
    if ((*viewerSlot)->pxyFile == 0) {
        cflLightTestSetMessage("PXY\244\254\244\242\244\352\244\336\244\273\244\363");
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    extension[1] = 't';
    extension[2] = 'x';
    extension[3] = 'y';
    (*viewerSlot)->txyFile = cflLightTestLoadFile(1, fileName);
    if ((*viewerSlot)->txyFile == 0) {
        cflLightTestSetMessage("TEX\244\254\244\242\244\352\244\336\244\273\244\363");
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
        return;
    }
    func_0028D760(*viewerSlot);
}


/* Loads all four files of fileName (.pxy, .txy, .xhr, .xap) into a new view; baseName receives the file's base name. */
void cflLightTestViewLoadAll(CflLightTestView **viewerSlot, char *fileName, char *baseName) {
    char *extension;

    extension = func_0030EF98(fileName, ".pxy");
    if (extension == 0) {
        cflLightTestSetMessage("PXY\244\362\301\252\302\362\244\267\244\306\244\257\244\300\244\265\244\244");
        return;
    }
    __builtin_strcpy(baseName, "");
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
    (*viewerSlot)->pxyFile = cflLightTestLoadFile(1, fileName);
    if ((*viewerSlot)->pxyFile == 0) {
        cflLightTestSetMessage("PXY\244\254\244\242\244\352\244\336\244\273\244\363");
        cflLightTestFreeBuffers(*viewerSlot);
        XenoActorRelease(*viewerSlot);
        *viewerSlot = 0;
    } else {
        extension[1] = 't';
        extension[2] = 'x';
        extension[3] = 'y';
        (*viewerSlot)->txyFile = cflLightTestLoadFile(1, fileName);
        if ((*viewerSlot)->txyFile == 0) {
            cflLightTestSetMessage("TEX\244\254\244\242\244\352\244\336\244\273\244\363");
            cflLightTestFreeBuffers(*viewerSlot);
            XenoActorRelease(*viewerSlot);
            *viewerSlot = 0;
        } else {
            extension[1] = 'x';
            extension[2] = 'h';
            extension[3] = 'r';
            (*viewerSlot)->xhrFile = cflLightTestLoadFile(1, fileName);
            if ((*viewerSlot)->xhrFile == 0) {
                cflLightTestSetMessage("XHR\244\254\244\242\244\352\244\336\244\273\244\363");
                cflLightTestFreeBuffers(*viewerSlot);
                XenoActorRelease(*viewerSlot);
                *viewerSlot = 0;
            } else {
                extension[1] = 'x';
                extension[2] = 'a';
                extension[3] = 'p';
                (*viewerSlot)->xapFile = cflLightTestLoadFile(2, fileName);
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

s32 GameMpeg2Play();

void XenoActorSetMode(void *actor, s32 mode);

/* Reloads the .xap block of an existing view; without the extension it shows a movie message instead. */
void cflLightTestViewReloadXap(CflLightTestView **viewerSlot, char *fileName) {
    char movieName[64];
    char *extension;
    CflLightTestView *viewer;

    extension = func_0030EF98(fileName, ".xap");
    if (extension == 0) {
        GameMpeg2Play(movieName, ".xap\244\307\244\317\244\242\244\352\244\336\244\273\244\363");
        cflLightTestSetMessage(movieName);
        return;
    }
    viewer = *viewerSlot;
    if (viewer == 0) {
        cflLightTestSetMessage("\245\342\245\307\245\353\244\362\306\311\244\337\271\376\244\363\244\307\244\257\244\300\244\265\244\244");
        return;
    }
    if (viewer->xapFile != 0) {
        RgHeapFree(viewer->xapFile, "CFLLightTest.euc.c", 0x13B);
        (*viewerSlot)->xapFile = 0;
    }
    extension[1] = 'x';
    extension[2] = 'a';
    extension[3] = 'p';
    (*viewerSlot)->xapFile = cflLightTestLoadFile(2, fileName);
    func_0028D760(*viewerSlot);
    XenoActorSetMode(*viewerSlot, 0);
}

char cflLightTestMessageBuffer[64] = { 0 };

typedef short s16;

/* Movie request block at gMovieRequestStorage: text window at 0x10, requested file name at 0x11C. */
typedef struct MovieRequestState {
    s32 unk_00;
    s32 textX;
    s32 textY;
    s32 fileRequestPending;           /* 0x0C: set to 1 while a file request is pending */
    char message[0x100];        /* 0x10 */
    s32 unk_110;
    s32 unk_114;
    void *printFunction;        /* 0x118: xglFontDebugPrintf */
    char fileName[0x102];       /* 0x11C */
    s16 unk_21E;
    s16 unk_220;
} MovieRequestState;

#define gMovieRequest gMovieRequestStorage

/* The file name block is the fileName field of the request block (0x11C in). */

#define offsetof(type, field) ((s32) &((type *) 0)->field)

#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))

typedef struct MovieFileNameBlock {
    char fileName[0x102];
} MovieFileNameBlock;

#define gMovieFileName (*(MovieFileNameBlock *) gMovieFileNameStorage)

extern MovieRequestState gMovieRequestStorage;


extern unsigned char xglFontDebugPrintf[];

/* Sets the text window position, copies the message and clears the requested file name. */
void movieTextWindowInit(const char *text)
{
  gMovieRequest.textX = 0x14;
  gMovieRequest.textY = 0x14;
  gMovieRequest.unk_00 = 8;
  gMovieRequest.fileRequestPending = 0;
  strcpy(gMovieRequest.message, text);
  gMovieRequest.printFunction = xglFontDebugPrintf;
  gMovieRequest.unk_110 = 0x027B0000;
  gMovieRequest.unk_114 = 0x100000;
  gMovieRequest.fileName[0] = 0;
}

extern unsigned char gMovieFileNameStorage[];

s32 xglRenderIoBegin();

s32 xglRenderIoEnd();

s32 xglSleep();

/* Stores the requested file name, marks the request pending and waits for two IO cycles; returns -1. */
s32 movieFileRequest(const char *fileName)
{
  MovieRequestState *fileRequest;

  strcpy(gMovieFileName.fileName, fileName);
  fileRequest = CONTAINER_OF(&gMovieFileName, MovieRequestState, fileName);
  fileRequest->unk_220 = fileRequest->unk_21E = 0;
  fileRequest->fileRequestPending = 1;
  xglRenderIoBegin();
  xglSleep();
  xglRenderIoEnd();
  xglSleep();
  return -1;
}


/* One of the eight movie slots: the loaded file name, the actor and its two motion numbers. */
typedef struct MovieSlotEntry {
    char fileName[0x20];      /* +0x00 */
    MovieSlotPlayback *actor; /* +0x20 */
    s32 id;                   /* +0x24: third GameMpeg2Play argument */
    s32 playMode;             /* +0x28 */
} MovieSlotEntry;

typedef struct MovieSlotTable {
    MovieSlotEntry movieSlots[8];
} MovieSlotTable;

extern MovieSlotTable gMovieSlotTable;


void XenoActorSetChannelValue(void *actor, float value);

void XenoActorEnableChannel(void *actor);


/* Starts the current movie slot actor on channel 12 and copies the next value into its id. */
void moviePlaybackStartNext(void) {
    MovieSlotPlayback *actor = gMovieSlotTable.movieSlots[gMovieSlotIndex].actor;

    if (actor != 0) {
        XenoActorSetChannelValue(actor, 12.0f);
        XenoActorEnableChannel(actor);
        XenoActorSetMode(actor, gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode);
        gMovieSlotTable.movieSlots[gMovieSlotIndex].id = gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode;
    }
}

/* Starts the current movie slot actor on channel 12, then swaps its id and next values. */
void moviePlaybackStartSwapIds(void) {
    MovieSlotPlayback *actor = gMovieSlotTable.movieSlots[gMovieSlotIndex].actor;

    if (actor != 0) {
        XenoActorSetChannelValue(actor, 12.0f);
        XenoActorEnableChannel(actor);
        XenoActorSetMode(actor, gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode);
        {
            s32 id = gMovieSlotTable.movieSlots[gMovieSlotIndex].id;

            gMovieSlotTable.movieSlots[gMovieSlotIndex].id = gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode;
            gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode = id;
        }
    }
}

/* One movie slot as the slot functions see it; the entries are 0x2C bytes each. */
typedef struct MovieSlotView {
    MovieSlotPlayback *playback; /* +0x00 */
    char *messageText;           /* +0x04: third GameMpeg2Play argument */
    s32 frameIndex;              /* +0x08 */
    char nextFileName[0x20];     /* +0x0C: file name of the following slot */
} MovieSlotView;

extern unsigned short gObjectFlagMask[];

extern unsigned char gMovieSlotViews[];

/* Steps the current slot frame index with pad 0x2000 (forward) or 0x8000 (back) and wraps it at the cycle frame count. */
void moviePlaybackStepFrame(void) {
    s32 playingSlot = gMovieSlotIndex;
    MovieSlotView *movieSlotView = CONTAINER_OF((s32 *) gMovieSlotViews + playingSlot * 11, MovieSlotView, frameIndex);
    s32 *slotFrameIndex = (s32 *) gMovieSlotViews + playingSlot * 11;
    MovieSlotCycle *cycle;

    if (movieSlotView->playback == 0) {
        return;
    }
    cycle = movieSlotView->playback->cycle;
    if (cycle == 0) {
        return;
    }
    if (gObjectFlagMask[0] & 0x2000) {
        *slotFrameIndex = *slotFrameIndex + 1;
        if (*slotFrameIndex < cycle->frameCount) {
            return;
        }
        *slotFrameIndex = 0;
    } else if (gObjectFlagMask[0] & 0x8000) {
        *slotFrameIndex = *slotFrameIndex - 1;
        if (*slotFrameIndex >= 0) {
            return;
        }
        *slotFrameIndex = cycle->frameCount - 1;
    }
}




/* Draws the movie info line of the current slot when its actor has a cycle. */
void moviePlaybackDrawInfo(s32 x, s32 y) {
    char infoText[0x40];

    if (gMovieSlotTable.movieSlots[gMovieSlotIndex].actor != 0 && gMovieSlotTable.movieSlots[gMovieSlotIndex].actor->cycle != 0) {
        GameMpeg2Play(infoText, "ID %3d -> NEXT %3d / %3d", gMovieSlotTable.movieSlots[gMovieSlotIndex].id,
                      gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode,
                      gMovieSlotTable.movieSlots[gMovieSlotIndex].actor->cycle->frameCount - 1);
        xglFontPrint(x + 0x82, y, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
        xglFontPrintDirect(infoText);
    }
}





void movieTextWindowInit(const char *text);

s32 movieFileRequest(const char *fileName);

s32 strcmp();

s32 func_0030E8E4();


s32 memset();

extern MovieSlotView gMovieSlotEntries[];

/* Requests the next movie file, decodes its name and loads it into the slot; flags slot 1 playback. */
void movieFileLoadNext(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if (movieFileRequest(gMovieSourceFileName) == 1) {
        if (strcmp(gMovieFileNameStorage, "")) {
            char *scanStart = gMovieSourceFileName - 1;
            char *baseNameStart;

            GameMpeg2Play(formattedMessage, "%s%s", gMovieMessageText, gMovieFileNameStorage);
            strcpy(gMovieSourceFileName, gMovieFileNameStorage);
            baseNameStart = scanStart + func_0030E8E4(gMovieSourceFileName);
            while (baseNameStart != gMovieSourceFileName) {
                if (*baseNameStart == '/') {
                    baseNameStart++;
                    break;
                }
                baseNameStart--;
            }
            *baseNameStart = 0;
            cflLightTestViewLoadAll(&gMovieSlotEntries[gMovieSlotIndex], formattedMessage,
                          gMovieSlotEntries[gMovieSlotIndex - 1].nextFileName);
            if (gMovieSlotIndex == 1) {
                gMovieSlotEntries[1].playback->stateFlags |= 0x1000;
            }
        }
    }
}



/* Requests the movie, decodes its message and loads it into the fixed slot through cflLightTestViewLoadPxyTxy. */
void movieDecodeToFixed(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if ((movieFileRequest(gMovieSourceFileName) == 1) && (strcmp(gMovieFileNameStorage, "") != 0)) {
        GameMpeg2Play(formattedMessage, "%s%s", gMovieMessageText, gMovieFileNameStorage);
        cflLightTestViewLoadPxyTxy(&gMovieFixedSlot, formattedMessage);
    }
}


/* Requests the movie, decodes its message and loads it into the current slot through cflLightTestViewReloadXap. */
void movieDecodeToSlot(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if ((movieFileRequest(gMovieSourceFileName) == 1) && (strcmp(gMovieFileNameStorage, "") != 0)) {
        GameMpeg2Play(formattedMessage, "%s%s", gMovieMessageText, gMovieFileNameStorage);
        cflLightTestViewReloadXap(&gMovieSlotTable.movieSlots[gMovieSlotIndex].actor, formattedMessage);
    }
}

/* Moves the current movie slot with pad 0x2000 (next) or 0x8000 (previous); wraps at 8 movieSlots. */
void movieSlotMoveCursor(void) {
    s32 nextMovieSlot;

    if (gObjectFlagMask[0] & 0x2000) {
        nextMovieSlot = gMovieSlotIndex + 1;
        gMovieSlotIndex = nextMovieSlot;
        if (nextMovieSlot >= 8) {
            gMovieSlotIndex = 0;
        }
    } else if (gObjectFlagMask[0] & 0x8000) {
        nextMovieSlot = gMovieSlotIndex - 1;
        if (nextMovieSlot < 0) {
            gMovieSlotIndex = 7;
        } else {
            gMovieSlotIndex = nextMovieSlot;
        }
    }
}

/* Draws the current slot text lines: the movie name and, when the actor exists, its frame time. */
void moviePlaybackDrawText(s32 x, s32 y) {
    char infoText[0x40];
    MovieSlotPlayback *actor = gMovieSlotTable.movieSlots[gMovieSlotIndex].actor;

    if (strcmp(gMovieSlotTable.movieSlots[gMovieSlotIndex].fileName, "")) {
        GameMpeg2Play(infoText, "ID %3d Name %s", gMovieSlotIndex,
                      gMovieSlotTable.movieSlots[gMovieSlotIndex].fileName);
    } else {
        GameMpeg2Play(infoText, "ID %3d Empty", gMovieSlotIndex);
    }
    xglFontPrint(x + 130, y, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
    xglFontPrintDirect(infoText);
    if (actor != 0) {
        xglFontPrint(330, 380, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
        GameMpeg2Play(infoText, "FRAME %4d\n", (s32) actor->frameTime);
        xglFontPrintDirect(infoText);
    }
}

extern PadRecord PadData;

void xglCurSetRotateY(float angle);

void xglCurApplyVector(Vector4 *destination, const Vector4 *source);

/* Moves the current slot actor with the pad sticks, rotated by the camera, and with the up/down buttons. */
void moviePlaybackMoveActor(void) {
    Vector4 rightStick;
    Vector4 leftStick;
    float speed = 5.0f;
    MovieSlotPlayback *actor = gMovieSlotTable.movieSlots[gMovieSlotIndex].actor;

    if (actor != 0) {
        rightStick.x = PadData.rightX;
        rightStick.y = 0.0f;
        rightStick.z = PadData.rightY;
        rightStick.w = 1.0f;
        leftStick.x = PadData.leftX;
        leftStick.y = 0.0f;
        leftStick.z = PadData.leftY;
        leftStick.w = 1.0f;
        xglCurSetRotateY(xglStudioGetCamera2(0)->travel.rotation.y);
        xglCurApplyVector(&leftStick, &leftStick);
        actor->offsetX += rightStick.x * 0.0003f * speed;
        actor->posX += leftStick.x * 0.0002f * speed;
        actor->posZ += leftStick.z * 0.0002f * speed;
        if (PadData.held & 4) {
            actor->posY += 0.002f * speed;
        }
        if (PadData.held & 1) {
            actor->posY -= 0.002f * speed;
        }
    }
}





extern u8 gMovieObjectFlags[];


void *InstanceOfRgHeapById(s32 id);

void InitRgHeap(void *heap, unsigned int address, unsigned int size);

void XenoActorReleaseAll(void);

void XenoActorPoolInitDefault(void);

void xglStudioReset(void);

void setCurrentStudio(s32 studio);

void xglStudioMainCameraInit(void);

void xglCameraSetPadPort(struct XglCamera *camera, s32 port);

void gifStateSetTimings(s32 a, s32 b, s32 c);

void sdvStoreValue(s32 value);

void studioCameraAt(struct XglCamera **camera, s32 index);

void pxySetHeaderScanEnabled(s32 enabled);

void pxyEnableFlag8(void);

void XenoActorSetHoldFlag(void);

void XenoActorClearHoldFlag(void);

void func_0028F8B8(MovieSlotPlayback *actor, u8 *flags);

void cflLightTestDrawMessage(void);

void func_002911C0(void);

void func_002912B0(void);

void xglRenderClearFrame(void);


/* Movie test main loop: runs the item menu until SELECT+START, then releases the actors. */
void movieTestMainLoop(void) {
    struct XglCamera *camera;
    unsigned int i;
    s32 holdState = 0;

    for (i = 0; i < 0x100; i++) {
        gMovieObjectFlags[i] = 1;
    }
    InitRgHeap(InstanceOfRgHeapById(0), 0xE00000, 0x2EE000);
    InitRgHeap(InstanceOfRgHeapById(1), 0x10EE000, 0x7D0000);
    InitRgHeap(InstanceOfRgHeapById(2), 0x18BE000, 0x2EE000);
    XenoActorReleaseAll();
    XenoActorPoolInitDefault();
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera = xglStudioGetCamera2(0);
    camera->cameraState = 2;
    xglCameraSetPadPort(camera, 0);
    gifStateSetTimings(5, 0x80, 15);
    memset(&gMovieSlotTable, 0, 0x160);
    sdvStoreValue(1);
    while ((PadData.heldAndPressed & 0x8000100) != 0x8000100) {
        studioCameraAt(&camera, 0);
        pxySetHeaderScanEnabled(1);
        pxyEnableFlag8();
        if (gMovieTestMode == 1) {
            if (PadData.repeated & 0x1000) {
                if (--gMovieMenuCursor < 0) {
                    gMovieMenuCursor = 3;
                }
            } else if (PadData.repeated & 0x4000) {
                if (++gMovieMenuCursor >= 4) {
                    gMovieMenuCursor = 0;
                }
            } else if (PadData.pressed & 0x10) {
                if (gMovieMenuItems[gMovieMenuCursor].onTriangle) {
                    gMovieMenuItems[gMovieMenuCursor].onTriangle();
                }
            } else if (PadData.pressed & 0x40) {
                if (gMovieMenuItems[gMovieMenuCursor].onCross) {
                    gMovieMenuItems[gMovieMenuCursor].onCross();
                }
            } else if (PadData.pressed & 0x20) {
                if (gMovieMenuItems[gMovieMenuCursor].onCircle) {
                    gMovieMenuItems[gMovieMenuCursor].onCircle();
                }
            } else if (PadData.pressed & 0x80) {
                if (gMovieMenuItems[gMovieMenuCursor].onSquare) {
                    gMovieMenuItems[gMovieMenuCursor].onSquare();
                }
            }
            for (i = 0; i < 4; i++) {
                if (i == gMovieMenuCursor) {
                    xglFontPrint(10, i * 20 + 10, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col60c000;");
                } else {
                    xglFontPrint(10, i * 20 + 10, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
                }
                xglFontPrintDirect(gMovieMenuItems[i].label);
                if (gMovieMenuItems[i].draw) {
                    gMovieMenuItems[i].draw(10, i * 20 + 10);
                }
            }
            if (gMovieMenuItems[gMovieMenuCursor].update) {
                gMovieMenuItems[gMovieMenuCursor].update();
            }
            camera->cameraState = 1;
            moviePlaybackMoveActor();
        } else if (gMovieTestMode == 0) {
            camera->cameraState = 2;
        }
        if (PadData.pressed & 0x800) {
            holdState = holdState != 1;
        }
        if (PadData.repeated & 0x100) {
            holdState = 2;
        }
        if (PadData.pressed & 0x10) {
            if (gMovieTestMode == 1) {
                gMovieTestMode = 0;
            } else if (gMovieTestMode == 0) {
                gMovieTestMode = 1;
            }
        }
        if (holdState == 1) {
            XenoActorSetHoldFlag();
        } else {
            XenoActorClearHoldFlag();
        }
        if (holdState == 2) {
            holdState = 1;
        }
        if (gMovieFixedSlot != 0) {
            gMovieFixedSlot->stateFlags |= 0x40000000;
            func_0028F8B8(gMovieFixedSlot, gMovieObjectFlags);
        }
        cflLightTestDrawMessage();
        func_002911C0();
        func_002912B0();
        ((void (*)(void)) xglSleep)();
    }
    sdvStoreValue(0);
    XenoActorReleaseAll();
    xglRenderClearFrame();
}

u8 gMovieObjectFlags[256];

MovieSlotTable gMovieSlotTable;

MovieRequestState gMovieRequestStorage;
