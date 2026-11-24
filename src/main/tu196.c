#include "common.h"

#include "main/tu007.h"
#include "main/xgl_2.h"

typedef int s32;

typedef unsigned char u8;

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

#define gMovieRequest (*(MovieRequestState *) gMovieRequestStorage)
/* The file name block is the fileName field of the request block (0x11C in). */
#define offsetof(type, field) ((s32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))

typedef struct MovieFileNameBlock {
    char fileName[0x102];
} MovieFileNameBlock;

#define gMovieFileName (*(MovieFileNameBlock *) gMovieFileNameStorage)

extern unsigned char gMovieRequestStorage[];
s32 strcpy();
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

typedef struct MovieSlotEntry {
    MovieSlotPlayback *actor; /* +0x00 */
    s32 id;                   /* +0x04: third GameMpeg2Play argument */
    s32 playMode;                 /* +0x08 */
    u8 unmodeled_0C[0x20];
} MovieSlotEntry;

typedef struct MovieSlotTable {
    u8 unmodeled_00[0x20];
    MovieSlotEntry movieSlots[8];
} MovieSlotTable;

extern MovieSlotTable gMovieSlotTable;
extern s32 gMovieSlotIndex;
void XenoActorSetChannelValue(void *actor, float value);
void XenoActorEnableChannel(void *actor);
void XenoActorSetMode(void *actor, s32 mode);

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

extern unsigned char D_0053A1F8[];
extern unsigned char D_0053A218[];
s32 GameMpeg2Play();
s32 xglFontPrint();
s32 xglFontPrintDirect();

/* Draws the movie info line of the current slot when its actor has a cycle. */
void moviePlaybackDrawInfo(s32 x, s32 y) {
    char infoText[0x40];

    if (gMovieSlotTable.movieSlots[gMovieSlotIndex].actor != 0 && gMovieSlotTable.movieSlots[gMovieSlotIndex].actor->cycle != 0) {
        GameMpeg2Play(infoText, D_0053A1F8, gMovieSlotTable.movieSlots[gMovieSlotIndex].id,
                      gMovieSlotTable.movieSlots[gMovieSlotIndex].playMode,
                      gMovieSlotTable.movieSlots[gMovieSlotIndex].actor->cycle->frameCount - 1);
        xglFontPrint(x + 0x82, y, 0xFFFFFF, D_0053A218);
        xglFontPrintDirect(infoText);
    }
}

extern MovieSlotView gMovieSlotEntries[];
extern unsigned char gMovieSourceFileName[];
extern unsigned char gMovieMessageFormat[];
extern char *gMovieMessageText;
extern unsigned char D_00550D40[];
void movieTextWindowInit(const char *text);
s32 movieFileRequest(const char *fileName);
s32 func_0030E68C();
s32 func_0030E8E4();
s32 cflLightTestViewLoadAll();
s32 memset();

/* Requests the next movie file, decodes its name and loads it into the slot; flags slot 1 playback. */
void movieFileLoadNext(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if (movieFileRequest(gMovieSourceFileName) == 1) {
        if (func_0030E68C(gMovieFileNameStorage, D_00550D40)) {
            char *scanStart = gMovieSourceFileName - 1;
            char *baseNameStart;

            GameMpeg2Play(formattedMessage, gMovieMessageFormat, gMovieMessageText, gMovieFileNameStorage);
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

extern MovieSlotPlayback *gMovieFixedSlot;
s32 cflLightTestViewLoadPxyTxy();

/* Requests the movie, decodes its message and loads it into the fixed slot through cflLightTestViewLoadPxyTxy. */
void movieDecodeToFixed(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if ((movieFileRequest(gMovieSourceFileName) == 1) && (func_0030E68C(gMovieFileNameStorage, D_00550D40) != 0)) {
        GameMpeg2Play(formattedMessage, gMovieMessageFormat, gMovieMessageText, gMovieFileNameStorage);
        cflLightTestViewLoadPxyTxy(&gMovieFixedSlot, formattedMessage);
    }
}

s32 cflLightTestViewReloadXap();

/* Requests the movie, decodes its message and loads it into the current slot through cflLightTestViewReloadXap. */
void movieDecodeToSlot(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInit(gMovieMessageText);
    if ((movieFileRequest(gMovieSourceFileName) == 1) && (func_0030E68C(gMovieFileNameStorage, D_00550D40) != 0)) {
        GameMpeg2Play(formattedMessage, gMovieMessageFormat, gMovieMessageText, gMovieFileNameStorage);
        cflLightTestViewReloadXap(&gMovieSlotEntries[gMovieSlotIndex], formattedMessage);
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

typedef struct MovieSlotEntryView {
    u8 unmodeled_00[0x20];
    MovieSlotPlayback *actor; /* +0x20 */
    s32 id;                          /* +0x24 */
    s32 playMode;                        /* +0x28 */
} MovieSlotEntryView;

typedef struct MovieSlotTableView {
    MovieSlotEntryView movieSlots[8];
} MovieSlotTableView;

extern unsigned char D_0053A258[];
extern unsigned char D_0053A268[];
extern unsigned char D_0053A278[];

/* Draws the current slot text lines: the movie name and, when the actor exists, its frame time. */
void moviePlaybackDrawText(s32 x, s32 y) {
    char infoText[0x40];
    MovieSlotPlayback *actor = gMovieSlotTable.movieSlots[gMovieSlotIndex].actor;

    if (func_0030E68C(&(*(MovieSlotTableView *) &gMovieSlotTable).movieSlots[gMovieSlotIndex], D_00550D40)) {
        GameMpeg2Play(infoText, D_0053A258, gMovieSlotIndex,
                      &(*(MovieSlotTableView *) &gMovieSlotTable).movieSlots[gMovieSlotIndex]);
    } else {
        GameMpeg2Play(infoText, D_0053A268, gMovieSlotIndex);
    }
    xglFontPrint(x + 130, y, 0xFFFFFF, D_0053A218);
    xglFontPrintDirect(infoText);
    if (actor != 0) {
        xglFontPrint(330, 380, 0xFFFFFF, D_0053A218);
        GameMpeg2Play(infoText, D_0053A278, (s32) actor->frameTime);
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
    MovieSlotPlayback *actor = gMovieSlotEntries[gMovieSlotIndex].playback;

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

typedef struct MovieMenuItem {
    char *label;
    void (*onTriangle)(void); /* +0x04: pad button 0x10 */
    void (*onCircle)(void);   /* +0x08: pad button 0x20 */
    void (*onCross)(void);    /* +0x0C: pad button 0x40 */
    void (*onSquare)(void);   /* +0x10: pad button 0x80 */
    void (*update)(void);
    void (*draw)(s32 x, s32 y);
} MovieMenuItem;

extern MovieMenuItem gMovieMenuItems[];
extern s32 gMovieTestMode;
extern s32 gMovieMenuCursor;
extern u8 gMovieObjectFlags[];
extern char D_0053A288[];
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
                    xglFontPrint(10, i * 20 + 10, 0xFFFFFF, D_0053A288);
                } else {
                    xglFontPrint(10, i * 20 + 10, 0xFFFFFF, D_0053A218);
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
