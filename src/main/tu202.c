#include "common.h"

#include "main/tu007.h"
#include "main/xgl_2.h"

typedef int s32;

typedef unsigned char u8;

typedef short s16;

/* Movie request block at movieTextRequest: text window at 0x10, requested file name at 0x11C. */
typedef struct MovieTextRequest {
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
} MovieTextRequest;

typedef struct MovieSlot {
    u8 unk_00[0x2C];
} MovieSlot;

#define gMovieTextRequest (*(MovieTextRequest *) movieTextRequest)
/* The file name block is the fileName field of the request block (0x11C in). */
#define offsetof(type, field) ((s32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))

typedef struct MovieRequestFileName {
    char fileName[0x102];
} MovieRequestFileName;

#define gMovieFileName (*(MovieRequestFileName *) movieFileName)

extern unsigned char movieTextRequest[];
extern const char D_0053AD60[];
extern const char D_0053AD50[];
extern const char D_0053AD40[];
extern const char D_0053AD30[];
s32 strcpy();
extern unsigned char xglFontDebugPrintf[];

void movieTextWindowInitAlt(const char *text)
{
  gMovieTextRequest.textX = 0x14;
  gMovieTextRequest.textY = 0x14;
  gMovieTextRequest.unk_00 = 8;
  gMovieTextRequest.fileRequestPending = 0;
  strcpy(gMovieTextRequest.message, text);
  gMovieTextRequest.printFunction = xglFontDebugPrintf;
  gMovieTextRequest.unk_110 = 0x027B0000;
  gMovieTextRequest.unk_114 = 0x100000;
  gMovieTextRequest.fileName[0] = 0;
}

extern unsigned char movieFileName[];
s32 xglRenderIoBegin();
s32 xglRenderIoEnd();
s32 xglSleep();

s32 movieFileRequestAlt(const char *fileName)
{
  MovieTextRequest *fileRequest;

  strcpy(gMovieFileName.fileName, fileName);
  fileRequest = CONTAINER_OF(&gMovieFileName, MovieTextRequest, fileName);
  fileRequest->unk_220 = fileRequest->unk_21E = 0;
  fileRequest->fileRequestPending = 1;
  xglRenderIoBegin();
  xglSleep();
  xglRenderIoEnd();
  xglSleep();
  return -1;
}

struct MovieSlotCycle {
    u8 unmodeled_00[0x8];
    unsigned int frameCount; /* +0x08 */
};

struct MovieSlotPlayback {
    u8 unmodeled_00[0x4F0];
    float unk_4F0; /* +0x4F0: converted to an int for the info line */
    u8 unmodeled_4F4[0x20];
    struct MovieSlotCycle *cycle; /* +0x514 */
};

typedef struct MovieSlotEntry {
    char fileName[0x20];             /* +0x00 */
    struct MovieSlotPlayback *actor; /* +0x20 */
    s32 id;                          /* +0x24 */
    s32 playMode;                    /* +0x28 */
} MovieSlotEntry;

extern MovieSlotEntry movieSlotTable[];
extern s32 D_00550D58;
void XenoActorSetChannelValue(struct MovieSlotPlayback *actor, float value);
void XenoActorEnableChannel(struct MovieSlotPlayback *actor);
void XenoActorSetMode(struct MovieSlotPlayback *actor, s32 mode);

/* Starts the selected movie slot (D_00550D58): channel value 12.0, channel enabled, its play mode applied, and the
 * play mode stored as the slot id. */
void movieSlotPlay(void) {
    struct MovieSlotPlayback *actor = movieSlotTable[D_00550D58].actor;

    if (actor != 0) {
        XenoActorSetChannelValue(actor, 12.0f);
        XenoActorEnableChannel(actor);
        XenoActorSetMode(actor, movieSlotTable[D_00550D58].playMode);
        movieSlotTable[D_00550D58].id = movieSlotTable[D_00550D58].playMode;
    }
}

/* Same start as movieSlotPlay, then swaps the slot's id and play mode. */
void movieSlotPlaySwapId(void) {
    struct MovieSlotPlayback *actor = movieSlotTable[D_00550D58].actor;

    if (actor != 0) {
        XenoActorSetChannelValue(actor, 12.0f);
        XenoActorEnableChannel(actor);
        XenoActorSetMode(actor, movieSlotTable[D_00550D58].playMode);
        {
            s32 previousId = movieSlotTable[D_00550D58].id;

            movieSlotTable[D_00550D58].id = movieSlotTable[D_00550D58].playMode;
            movieSlotTable[D_00550D58].playMode = previousId;
        }
    }
}

typedef struct MovieSlotView {
    struct MovieSlotPlayback *playback; /* +0x00 */
    char *messageText;                  /* +0x04 */
    s32 frameIndex;                     /* +0x08 */
    char nextFileName[0x20];            /* +0x0C */
} MovieSlotView;

extern unsigned short gObjectFlagMask[];
extern s32 D_005A7D18[];

/* Advances the frame index of the selected slot's view (11 words = one 0x2C-byte MovieSlot) through its playback
 * cycle: flag 0x2000 counts forward and wraps to 0 at frameCount, flag 0x8000 counts back and wraps to frameCount - 1. */
void movieSlotAdvanceFrame(void) {
    s32 playingSlot = D_00550D58;
    MovieSlotView *movieSlotView =
        CONTAINER_OF(D_005A7D18 + playingSlot * 11, MovieSlotView, frameIndex);
    s32 *slotFrameIndex = D_005A7D18 + playingSlot * 11;
    struct MovieSlotCycle *cycle;

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

s32 xglFontPrint();
s32 xglFontPrintDirect();
s32 sprintf();

/* Draws the info line of the selected slot (ID, play mode and the last frame) when it has an actor with a cycle. */
void movieSlotDrawInfo(s32 x, s32 y) {
    char infoText[0x40];

    if (movieSlotTable[D_00550D58].actor != 0 && movieSlotTable[D_00550D58].actor->cycle != 0) {
        sprintf(infoText, "ID %3d -> NEXT %3d / %3d", movieSlotTable[D_00550D58].id,
                      movieSlotTable[D_00550D58].playMode, movieSlotTable[D_00550D58].actor->cycle->frameCount - 1);
        xglFontPrint(x + 0x82, y, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
        xglFontPrintDirect(infoText);
    }
}

extern char movieFilePath[];
extern unsigned char motionViewerEmptyName[];
extern const char *movieDirectoryPrefix;
extern MovieSlot D_005A7D10[];
s32 strcmp();
s32 strlen();
s32 motionViewerOpenSet();
s32 movieFileRequestAlt(const char *fileName);
void movieTextWindowInitAlt(const char *text);
s32 memset();

/* Requests the movie named at movieFilePath; when it is found, cuts its path after the last '/' (the directory part is
 * kept) and passes that with the message built from movieDirectoryPrefix to motionViewerOpenSet for the selected slot. */
void movieSlotRequestFile(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInitAlt(movieDirectoryPrefix);
    if (movieFileRequestAlt(movieFilePath) == 1) {
        if (strcmp(movieFileName, motionViewerEmptyName) != 0) {
            char *scanStart = movieFilePath - 1;
            char *baseNameStart;

            sprintf(formattedMessage, "%s%s", movieDirectoryPrefix, movieFileName);
            strcpy(movieFilePath, movieFileName);
            baseNameStart = scanStart + strlen(movieFilePath);
            while (baseNameStart != movieFilePath) {
                if (*baseNameStart == '/') {
                    baseNameStart++;
                    break;
                }
                baseNameStart--;
            }
            *baseNameStart = 0;
            motionViewerOpenSet(
                &D_005A7D10[D_00550D58], formattedMessage,
                (MovieSlotEntry *) ((char *) &D_005A7D10[D_00550D58] - 0x20));
        }
    }
}

extern s32 D_00550D5C;
s32 motionViewerOpenPxyTxy();

/* Requests the movie in movieFilePath and, when found, hands the message to motionViewerOpenPxyTxy (fixed target D_00550D5C). */
void movieDecodeToFixedAlt(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInitAlt(movieDirectoryPrefix);
    if ((movieFileRequestAlt(movieFilePath) == 1) && (strcmp(movieFileName, motionViewerEmptyName) != 0)) {
        sprintf(formattedMessage, "%s%s", movieDirectoryPrefix, movieFileName);
        motionViewerOpenPxyTxy(&D_00550D5C, formattedMessage);
    }
}

s32 motionViewerLoadMotion();

/* Requests the movie in movieFilePath and, when found, hands the message to motionViewerLoadMotion for the selected slot. */
void movieDecodeToSlotAlt(void) {
    u8 formattedMessage[0x100];

    memset(formattedMessage, 0, 0x100);
    movieTextWindowInitAlt(movieDirectoryPrefix);
    if ((movieFileRequestAlt(movieFilePath) == 1) && (strcmp(movieFileName, motionViewerEmptyName) != 0)) {
        sprintf(formattedMessage, "%s%s", movieDirectoryPrefix, movieFileName);
        motionViewerLoadMotion(&D_005A7D10[D_00550D58], formattedMessage);
    }
}

/* Moves the selected slot (D_00550D58) by one with the pad flags 0x2000 (next, wraps to 0) and 0x8000 (previous, wraps to 7). */
void movieSlotStep(void) {
    s32 nextMovieSlot;

    if (gObjectFlagMask[0] & 0x2000) {
        nextMovieSlot = D_00550D58 + 1;
        D_00550D58 = nextMovieSlot;
        if (nextMovieSlot >= 8) {
            D_00550D58 = 0;
        }
    } else if (gObjectFlagMask[0] & 0x8000) {
        nextMovieSlot = D_00550D58 - 1;
        if (nextMovieSlot < 0) {
            D_00550D58 = 7;
        } else {
            D_00550D58 = nextMovieSlot;
        }
    }
}

/* Draws the info line of the selected slot: its name, or Empty, and the current frame when it has an actor. */
void movieSlotDrawName(s32 x, s32 y) {
    char infoText[0x40];
    struct MovieSlotPlayback *actor = movieSlotTable[D_00550D58].actor;

    if (strcmp(movieSlotTable[D_00550D58].fileName, motionViewerEmptyName)) {
        sprintf(infoText, "ID %3d Name %s", D_00550D58, movieSlotTable[D_00550D58].fileName);
    } else {
        sprintf(infoText, "ID %3d Empty", D_00550D58);
    }
    xglFontPrint(x + 0x82, y, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
    xglFontPrintDirect(infoText);
    if (actor != 0) {
        xglFontPrint(330, 380, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
        sprintf(infoText, "FRAME %4d\n", (s32) actor->unk_4F0);
        xglFontPrintDirect(infoText);
    }
}

struct MovieSlotPlaybackView {
    u8 unmodeled_00[0x20];
    float posX; /* +0x20 */
    float posY; /* +0x24 */
    float posZ; /* +0x28 */
    u8 unmodeled_2C[0x28];
    float offsetX; /* +0x54 */
    u8 unmodeled_58[0x498];
    float unk_4F0; /* +0x4F0: converted to an int for the info line */
    u8 unmodeled_4F4[0x20];
    struct MovieSlotCycle *cycle; /* +0x514 */
};

typedef struct MovieSlotAlt {
    struct MovieSlotPlaybackView *playback; /* +0x00 */
    char *messageText;                         /* +0x04 */
    s32 frameIndex;                            /* +0x08 */
    char nextFileName[0x20];                   /* +0x0C */
} MovieSlotAlt;

extern PadRecord PadData;
void xglCurSetRotateY(float angle);
void xglCurApplyVector(Vector4 *destination, const Vector4 *source);

/* Moves the selected slot's actor from the pad: the right stick changes offsetX, the left stick changes posX and
 * posZ, and held bits 4 and 1 move posY up and down; every step is scaled by speed. */
void movieActorPadMove(void) {
    Vector4 rightStick;
    Vector4 leftStick;
    float speed = 5.0f;
    struct MovieSlotPlaybackView *actor = ((MovieSlotAlt *) &D_005A7D10)[D_00550D58].playback;

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
    const char *label;          /* +0x00 */
    void (*onTriangle)(void);   /* +0x04 */
    void (*onCircle)(void);     /* +0x08 */
    void (*onCross)(void);      /* +0x0C */
    void (*onSquare)(void);     /* +0x10 */
    void (*update)(void);       /* +0x14 */
    void (*draw)(s32 x, s32 y); /* +0x18 */
} MovieMenuItem;

extern MovieMenuItem movieMenuItems[];
extern s32 movieMenuOpen;
extern s32 movieMenuRow;
void *InstanceOfRgHeapById(s32 id);
void InitRgHeap(void *heap, unsigned int address, unsigned int size);
void XenoActorReleaseAll(void);
void XenoActorPoolInitDefault(void);
void XenoActorSetHoldFlag(void);
void XenoActorClearHoldFlag(void);
void xglStudioReset();
void setCurrentStudio();
void xglStudioMainCameraInit();
void xglCameraSetPadPort();
void sdvStoreValue();
void studioCameraAt();
void pxySetHeaderScanEnabled();
void motionViewerDrawMessage();
void func_002911C0();
void func_002912B0();
void xglRenderClearFrame();

/* Movie menu loop: sets up the heaps, actor pool and studio camera, then runs until the held-and-pressed combination
 * 0x8000100. movieMenuOpen is 1 while the four-row menu is drawn and handled, 0 otherwise; movieMenuRow is the selected row. */
void movieMenuRun(void) {
    struct XglCamera *camera;
    unsigned int i;
    s32 holdState = 0;

    InitRgHeap(InstanceOfRgHeapById(0), 0xE00000, 0x2EE000);
    InitRgHeap(InstanceOfRgHeapById(1), 0x10EE000, 0x7D0000);
    InitRgHeap(InstanceOfRgHeapById(2), 0x18BE000, 0x2EE000);
    XenoActorPoolInitDefault();
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera = xglStudioGetCamera2(0);
    camera->cameraState = 2;
    xglCameraSetPadPort(camera, 0);
    memset(movieSlotTable, 0, 0x160);
    sdvStoreValue(1);
    while ((PadData.heldAndPressed & 0x8000100) != 0x8000100) {
        studioCameraAt(&camera, 0);
        pxySetHeaderScanEnabled(1);
        if (movieMenuOpen == 1) {
            if (PadData.repeated & 0x1000) {
                if (--movieMenuRow < 0) {
                    movieMenuRow = 3;
                }
            } else if (PadData.repeated & 0x4000) {
                if (++movieMenuRow >= 4) {
                    movieMenuRow = 0;
                }
            } else if (PadData.pressed & 0x10) {
                if (movieMenuItems[movieMenuRow].onTriangle) {
                    movieMenuItems[movieMenuRow].onTriangle();
                }
            } else if (PadData.pressed & 0x40) {
                if (movieMenuItems[movieMenuRow].onCross) {
                    movieMenuItems[movieMenuRow].onCross();
                }
            } else if (PadData.pressed & 0x20) {
                if (movieMenuItems[movieMenuRow].onCircle) {
                    movieMenuItems[movieMenuRow].onCircle();
                }
            } else if (PadData.pressed & 0x80) {
                if (movieMenuItems[movieMenuRow].onSquare) {
                    movieMenuItems[movieMenuRow].onSquare();
                }
            }
            for (i = 0; i < 4; i++) {
                if (i == movieMenuRow) {
                    xglFontPrint(10, i * 20 + 10, 0xFFFFFF,
                                 "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col60c000;");
                } else {
                    xglFontPrint(10, i * 20 + 10, 0xFFFFFF,
                                 "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
                }
                xglFontPrintDirect(movieMenuItems[i].label);
                if (movieMenuItems[i].draw) {
                    movieMenuItems[i].draw(10, i * 20 + 10);
                }
            }
            if (movieMenuItems[movieMenuRow].update) {
                movieMenuItems[movieMenuRow].update();
            }
            camera->cameraState = 1;
            movieActorPadMove();
        } else if (movieMenuOpen == 0) {
            camera->cameraState = 2;
        }
        if (PadData.pressed & 0x800) {
            holdState = holdState != 1;
        }
        if (PadData.repeated & 0x100) {
            holdState = 2;
        }
        if (PadData.pressed & 0x10) {
            if (movieMenuOpen == 1) {
                movieMenuOpen = 0;
            } else if (movieMenuOpen == 0) {
                movieMenuOpen = 1;
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
        motionViewerDrawMessage();
        func_002911C0();
        func_002912B0();
        ((void (*)()) xglSleep)();
    }
    sdvStoreValue(0);
    XenoActorReleaseAll();
    xglRenderClearFrame();
}

/* Data of this TU in address order (the labels of the menu and the host prefix are Gtest rodata). */
MovieMenuItem movieMenuItems[4] = {
    { D_0053AD60, 0, movieDecodeToFixedAlt, 0, 0, 0, 0 },
    { D_0053AD50, 0, movieSlotRequestFile, 0, 0, movieSlotStep, movieSlotDrawName },
    { D_0053AD40, 0, movieDecodeToSlotAlt, 0, 0, 0, 0 },
    { D_0053AD30, 0, movieSlotPlay, 0, movieSlotPlaySwapId, movieSlotAdvanceFrame, movieSlotDrawInfo },
};
char movieFilePath[256] = { 0 };
s32 D_00550D58 = 0;
s32 D_00550D5C = 0;
s32 movieMenuOpen = 0;
s32 movieMenuRow = 0;
const char *movieDirectoryPrefix = D_0053AD60 + 8;
MovieSlotEntry movieSlotTable[8];
unsigned char movieTextRequest[0x11C];
unsigned char movieFileName[0x114];
