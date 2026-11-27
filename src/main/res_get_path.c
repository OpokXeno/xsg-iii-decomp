#include "common.h"
#include "shared.h"

#include "main/res_get_path.h"
#include "main/xgl_render.h"
#include "main/xgl_cd.h"
#include "gs.h"
#include "main/xgl_debug_file_004.h"
#include "main/tu176.h"
#include "main/tu178.h"

void *memset(void *destination, int value, unsigned int count);

void sprintf(const char *name);
u8 *resCmdMpeg2(u8 *cursor);

struct Func430Row {
    unsigned short key;
    unsigned short extra;
};

extern struct Func430Row saveScreenRecordTable[];

/*
 * Looks up the row of saveScreenRecordTable whose key is recordId / 10 (row 0 when none matches). Returns the row's text id
 * (rows 10 and up, id = row + 0x2C6) or else its extra value; when outPair is set, the record words +0x08 and +0x0C
 * get the extra value and the text id.
 */
int saveScreenRecordLookup(int *outPair, int recordId) {
    int rowExtra = 0;
    int rowTextId;
    int rowIndex;

    if (outPair != 0) {
        outPair[2] = 0;
        outPair[3] = 0;
    }
    rowTextId = 0;
    rowIndex = 0;
    while (1) {
        if (recordId / 10 == saveScreenRecordTable[rowIndex].key) {
            rowExtra = saveScreenRecordTable[rowIndex].extra;
            rowTextId = (rowIndex >= 10) ? rowIndex + 0x2C6 : 0;
            break;
        }
        if (saveScreenRecordTable[rowIndex].key == 0 && saveScreenRecordTable[rowIndex].extra == 0xFFFF) {
            rowIndex = 0;
            break;
        }
        rowIndex++;
    }
    if (outPair != 0) {
        outPair[2] = rowExtra;
        outPair[3] = rowTextId;
    }
    return (rowTextId != 0) ? rowTextId : rowExtra;
}

/* One save file of the card directory, as read into the file list. */
struct McFileEntry {
    unsigned int playTime;                  /* +0x00 */
    int unmodeled04;                        /* +0x04 */
    int textIds[2];                         /* +0x08: message ids of the two text lines, 0 = none */
    short row;                              /* +0x10 */
    short rowSecond;                        /* +0x12 */
    signed char rowFlag;                    /* +0x14 */
    unsigned char fileState;                    /* +0x15: file state */
    unsigned char pictureState;                    /* +0x16: picture slot state */
    unsigned char thumbnailSlot;            /* +0x17: slot that loads the entry */
};

struct SaveMenuKey {
    unsigned short flags;
};

/* Paged cursor table of one menu. */
struct SaveMenuTable {
    unsigned short cursor;
    unsigned short filledEntryCount;
    unsigned short unmodeled_04;
    unsigned short columnFlags;
    unsigned char unmodeled_08[0x18];
    int flags;                              /* +0x20 */
    void **entryPointers;
    struct SaveMenuKey *entryKeys;
    unsigned char unmodeled_2C[8];
    unsigned short capacity;
    unsigned short top;                     /* +0x36: first visible row */
    unsigned char unmodeled_38[4];
};

struct McGauge {
    unsigned char unmodeled_00[8];
    unsigned char state;                    /* +0x08 */
    unsigned char unmodeled_09[2];
    unsigned char flag;                     /* +0x0B */
};

struct McFileList {
    unsigned char unmodeled_00[0x1C];
    short cursor;                           /* +0x1C */
    short previousCursor;                   /* +0x1E */
    short position;                         /* +0x20 */
    unsigned char unmodeled_22[2];
    short drawAlpha;                    /* +0x24 */
    unsigned char unmodeled_26[2];
    short fadingAlpha;                      /* +0x28 */
    unsigned char unmodeled_2A[2];
    unsigned short portResult[2];           /* +0x2C, indexed by port */
    struct McFileEntry fileEntries[50];     /* +0x30 */
    int freeSize;                           /* +0x4E0 */
    int scroll;                             /* +0x4E4 */
    int fadeBase;                           /* +0x4E8 */
    int scrollTarget;                       /* +0x4EC */
};

struct McSlots {
    unsigned char fileIndexes[5];           /* file entry index per slot */
    unsigned char fileStatusBytes[5];       /* cleared by saveScreenClearStatusBytes */
};

/* A menu step: setup != 0 prepares it, 0 runs one frame. */
typedef void (*SaveMenuCallback)(int setup);
/* Draws one screen at the given alpha. */
typedef void (*SaveDrawCallback)(unsigned char alpha);

/* The save screen state block at gSaveScreen (0x5BB40 bytes). */
struct McScreen {
    unsigned char fileUsed[100];            /* +0x000: card directory, nonzero = the file exists */
    unsigned char lastSavedFile;            /* +0x064 */
    unsigned char unmodeled_065[0x2000 - 0x65];
    unsigned char picture[0x130B0];         /* +0x2000: file being loaded */
    unsigned char scanBuffer[0x10];         /* +0x150B0: buffer xglMcRequest fills */
    unsigned char thumbnails[5][0xE000];    /* +0x150C0: one decoded picture per slot */
    signed char message[0x80];              /* +0x5B0C0 */
    unsigned char fields[0x340];            /* +0x5B140 */
    SaveDrawCallback draw;           /* +0x5B480 */
    SaveDrawCallback drawFading;                  /* +0x5B484 */
    SaveMenuCallback menu;                /* +0x5B488 */
    SaveDrawCallback onDone;                        /* +0x5B48C */
    SaveMenuCallback onCancel;                        /* +0x5B490 */
    char textBuffer[0x100];                 /* +0x5B494: the formatted text of messageText */
    int messageText;                        /* +0x5B594 */
    struct SaveMenuTable portTable;         /* +0x5B598: menu of the two card ports */
    struct SaveMenuTable fileTable;         /* +0x5B5D4: menu of the files */
    struct McGauge gauge;                   /* +0x5B610 */
    struct McFileList fileList;             /* +0x5B61C */
    signed char lastFile;                   /* +0x5BB0C */
    signed char cursorRow;                  /* +0x5BB0D */
    signed char fileCount;                  /* +0x5BB0E */
    unsigned char ringAlpha[3];             /* +0x5BB0F: one level per ring slot */
    struct McSlots slots;                   /* +0x5BB12 */
    short fade;                             /* +0x5BB1C: fade step, full at 16 */
    short fadeLevel;                        /* +0x5BB1E: the level value saveScreenBeginLevelFade sets */
    unsigned char port;                     /* +0x5BB20 */
    unsigned char levelIndex[1];            /* +0x5BB21 */
    unsigned char mode;                     /* +0x5BB22 */
    unsigned char uiState;                  /* +0x5BB23 */
    unsigned char requestBits[1];           /* +0x5BB24 */
    signed char messagePending;                      /* +0x5BB25 */
    signed char cardBusy;                   /* +0x5BB26 */
    signed char fadeStage;                  /* +0x5BB27 */
    unsigned char fadeProgress[1];          /* +0x5BB28 */
    signed char cardReadPending;                      /* +0x5BB29 */
    unsigned char loadingSlot;              /* +0x5BB2A */
    signed char fileImageReady;             /* +0x5BB2B */
    signed char writeConfirmPending;                    /* +0x5BB2C */
    signed char unmodeled_5BB2D;
    signed char displayState;               /* +0x5BB2E: set to 2 by saveScreenResetDisplayState */
    signed char fileLoaded;                     /* +0x5BB2F */
    unsigned char unmodeled_5BB30[0x10];
};
extern struct McScreen *gSaveScreen;

/* Formats text (a messageTextGet result) with value into the screen message; clears the message when text is 0. */
void saveScreenSetFormattedMessage(int text, int value) {
    if (text != 0) {
        ((void (*)()) sprintf)(&gSaveScreen->message[0], text, value);
        return;
    }
    gSaveScreen->message[0] = 0;
}

void seVoiceStartDefault(unsigned int handle);

/*
 * Sets the menu step: a nonzero menu is set up and stored as screen->menu. With none, the display state is set to 1 (after the default voice when the screen mode is not 2).
 */
void saveScreenSetMenu(SaveMenuCallback menu, SaveDrawCallback draw) {
    struct McScreen *screen;

    if (menu != 0) {
        menu(1);
    }
    screen = gSaveScreen;
    screen->menu = menu;
    if (menu != 0) {
        return;
    }
    if (screen->mode != 2) {
        seVoiceStartDefault(7);
        screen = gSaveScreen;
    }
    screen->displayState = 1;
}

void setSteppedValue(s32 stepped, s32 value, s32 target);

/*
 * Makes draw the current screen when it changed (the old one fades out), and steps the file list
 * alpha values. Falls through without a return value.
 */
int saveScreenSetDraw(SaveDrawCallback draw) {
    struct McScreen *screen;
    struct McScreen *last;
    SaveDrawCallback previousDraw;

    screen = gSaveScreen;
    if (draw != screen->draw) {
        setSteppedValue((s32) &screen->fileList.fadingAlpha, screen->fileList.drawAlpha, 0);
        setSteppedValue((s32) &gSaveScreen->fileList.drawAlpha, 0, 0x80);
        last = gSaveScreen;
        previousDraw = last->draw;
        last->draw = draw;
        last->drawFading = previousDraw;
    }
}

/* Sets the menu step, then the screen. */
int saveScreenSetMenuAndDraw(SaveMenuCallback menu, SaveDrawCallback draw) {
    saveScreenSetMenu(menu, draw);
    return saveScreenSetDraw(draw);
}

struct McCardRequest {
    unsigned char unmodeled_00[4];
    void *screen;
    unsigned char unmodeled_08[4];
    void (*callback)(void);
};

s32 sheetSetIndex(s32);

void saveScreenSetMessage(s32 sheetIndex) {
    sheetSetIndex(0);
    sheetSetIndex(sheetIndex);
}

/*
 * Script command ";mpeg2" (name and handler of its resCommandTable entry; proposed resCmdMpeg2).
 * It plays the movie named by the script text at the cursor and returns the cursor unchanged, as the other
 * resCmd handlers do.
 */
u8 *resCmdMpeg2(u8 *cursor) {
    sprintf(cursor);
    return cursor;
}

u8 *saveScreenFormatText(int text, int value, int flag);

/* Formats text into the screen's text buffer with value and flag; returns the buffer. */
u8 *saveScreenFormatText(int text, int value, int flag) {
    return ((u8 * (*) (u8 * cursor, int arg1, int arg2, int arg3))
                resCmdMpeg2)(&gSaveScreen->textBuffer[0], text, value, flag);
}

void saveScreenSetMessageText(s32 text) {
    struct McScreen *screen = gSaveScreen;

    screen->messageText = text;
    screen->messagePending = 1;
}

void saveScreenShowSheet(int sheetIndex, int withSetup, SaveMenuCallback onCancel, SaveDrawCallback onDone);
void saveScreenPortMenuUpdate(s32 setup);
void saveScreenDrawMenu(u8 alpha);
void saveScreenHandleError(struct McScreen *screen, unsigned short cardResult);
void xglMcReset(void);

/*
 * Handles a card error: card results 1, 2 and 255 are not errors. Otherwise clears the file state, shows the
 * error message and resets the card.
 */
void saveScreenHandleCardResult(void) {
    u16 result = gSaveScreen->fileList.portResult[gSaveScreen->port];

    if ((u16) (result - 1) < 2 || result == 255) {
        return;
    }
    gSaveScreen->messagePending = 0;
    gSaveScreen->cardBusy = 0;
    gSaveScreen->fadeStage = 0;
    ((void (*)(void)) saveScreenHandleError)();
    gSaveScreen->cardReadPending = 0;
    xglMcReset();
    saveScreenShowSheet(saveScreenFormatText(gSaveScreen->messageText, gSaveScreen->port + 1, 0), 1,
                  (void *) saveScreenPortMenuUpdate, (void *) saveScreenDrawMenu);
    seVoiceStartDefault(3);
}

void saveScreenSetupMenuHandler(int arg0);
void saveScreenPlainMenuHandler(int arg0);
void saveScreenSetMessage(s32 sheetIndex);

/* Shows sheet sheetIndex with its cancel and done callbacks; withSetup picks the setup or the plain menu handler. */
void saveScreenShowSheet(int sheetIndex, int withSetup, SaveMenuCallback onCancel, SaveDrawCallback onDone) {
    struct McScreen *screen;

    saveScreenSetMessage(sheetIndex);
    screen = gSaveScreen;
    screen->onCancel = onCancel;
    screen->onDone = onDone;
    saveScreenSetMenuAndDraw(withSetup != 0 ? saveScreenSetupMenuHandler : saveScreenPlainMenuHandler, 0);
}

int messageTextGet(int id);
int statusHeaderIsReadyMode20(void);
void mcClearSetupStatus(void);
void func_00220B90(int a, int b);
void func_00220D28(void);
s32 statusHeaderIsReadyMode40(void);

/* Menu handler: setup != 0 clears the setup status; otherwise, in ready mode 0x20 or 0x40, runs the stored event. */
void saveScreenSetupMenuHandler(int setup) {
    struct McScreen *screen;

    if (setup != 0) {
        mcClearSetupStatus();
        func_00220B90(0x40, messageTextGet(0x1F));
        func_00220B90(0x20, 0);
        func_00220D28();
        return;
    }
    if (statusHeaderIsReadyMode20() || statusHeaderIsReadyMode40()) {
        SaveMenuCallback menu;
        SaveDrawCallback draw;

        screen = gSaveScreen;
        menu = screen->onCancel;
        do {
            draw = gSaveScreen->onDone;
        } while (0);
        saveScreenSetMenuAndDraw(menu, draw);
        seVoiceStartDefault(8);
        screen = gSaveScreen;
        screen->onCancel = 0;
        screen->onDone = 0;
    }
}

/* Menu handler: setup != 0 clears the setup status; otherwise, in ready mode 0x20, runs the stored event. */
void saveScreenPlainMenuHandler(int setup) {
    struct McScreen *screen;
    struct McScreen *after;

    if (setup != 0) {
        mcClearSetupStatus();
        func_00220B90(0x20, 0);
        func_00220D28();
        return;
    }
    if (statusHeaderIsReadyMode20()) {
        SaveMenuCallback menu;
        SaveDrawCallback draw;

        screen = gSaveScreen;
        menu = screen->onCancel;
        do {
            draw = gSaveScreen->onDone;
        } while (0);
        saveScreenSetMenuAndDraw(menu, draw);
        seVoiceStartDefault(8);
        after = gSaveScreen;
        after->onCancel = 0;
        after->onDone = 0;
    }
}

void func_002213A8(void *dst, int a, int b, int zero);
void mcStateSetupRequest(int a, int b);

/* Starts the gauge with text and gaugeArg, stores the cancel and done callbacks and requests state 6. */
void saveScreenStartGauge(int text, int gaugeArg, SaveMenuCallback onCancel, SaveDrawCallback onDone) {
    struct McScreen *screen;

    func_002213A8(&gSaveScreen->gauge, text, gaugeArg, 0);
    screen = gSaveScreen;
    screen->onCancel = onCancel;
    screen->onDone = onDone;
    mcStateSetupRequest(6, 0);
    gSaveScreen->requestBits[0] = 1;
}

int func_00221440(void *a, int b);

/* Per-frame step of the gauge request: when it finishes, runs the stored callbacks and clears them. */
void saveScreenGaugeStep(void) {
    if (func_00221440(&gSaveScreen->gauge, 1)) {
        if (gSaveScreen->gauge.state == 0) {
            saveScreenSetMenuAndDraw(gSaveScreen->onCancel,
                                  gSaveScreen->onDone);
            seVoiceStartDefault(2);
        } else {
            if (gSaveScreen->draw == 0) {
                saveScreenSetDraw(gSaveScreen->drawFading);
            }
            if (gSaveScreen->menu != 0) {
                gSaveScreen->menu(1);
            }
            seVoiceStartDefault(8);
        }
        gSaveScreen->requestBits[0] &= 0xFE;
        gSaveScreen->onCancel = 0;
        gSaveScreen->onDone = 0;
    }
}

s32 counterEntryAdvancePhase();

void saveScreenHandleError(struct McScreen *screen, unsigned short cardResult) {
    gSaveScreen->requestBits[0] &= 0xFE;
    counterEntryAdvancePhase(&gSaveScreen->gauge);
}

void saveScreenSetRequestFlag(void) {
    gSaveScreen->gauge.flag = 0x80;
    gSaveScreen->requestBits[0] |= 2;
}

void saveScreenBeginLevelFade(void) {
    gSaveScreen->fadeStage = 1;
    gSaveScreen->fadeProgress[0] = 0x80;
    gSaveScreen->fadeLevel = (gSaveScreen->levelIndex[0] * 0x670) - gSaveScreen->fileList.fadeBase;
}

void saveScreenStepFlagB(void) {
    if (((s8) gSaveScreen->fadeProgress[0] >= 0) && (gSaveScreen->fadeStage == 1)) {
        gSaveScreen->fadeStage = 2;
        return;
    }
    gSaveScreen->fadeStage = 0;
}

extern signed char gVsyncsPerFrame;

/* Steps the fade byte of gSaveScreen once per frame: stage 1 fades out to zero, stage 2 fades in until it turns
 * negative and returns to stage 0. */
void saveScreenStepFade(void) {
    if (gSaveScreen->fadeStage == 1) {
        if (gSaveScreen->fadeProgress[0] != 0) {
            gSaveScreen->fadeProgress[0] = gSaveScreen->fadeProgress[0] - ((u8) gVsyncsPerFrame << 4);
            return;
        }
    }
    if (gSaveScreen->fadeStage == 2) {
        if ((signed char) gSaveScreen->fadeProgress[0] >= 0) {
            gSaveScreen->fadeProgress[0] = gSaveScreen->fadeProgress[0] + ((u8) gVsyncsPerFrame << 4);
        } else {
            gSaveScreen->fadeStage = 0;
        }
    }
}

void saveScreenClearStatusBytes(void) {
    s32 slot;

    slot = 0;
    do {
        gSaveScreen->slots.fileStatusBytes[slot] = 0;
        slot++;
    } while (slot < 5);
}

void saveScreenClearStatusBytes();

void saveScreenClearRecords(void) {
    s32 fileEntryIndex;

    for (fileEntryIndex = 0; fileEntryIndex < 50; fileEntryIndex++) {
        gSaveScreen->fileList.fileEntries[fileEntryIndex].fileState = 0;
        gSaveScreen->fileList.fileEntries[fileEntryIndex].pictureState = 0;
    }
    saveScreenClearStatusBytes();
}

struct McSlotRecord {
    unsigned int playTime; /* +0x00 */
    int unmodeled04;       /* +0x04 */
    unsigned char unmodeled_08[0x10 - 0x08];
    short row;           /* +0x10 */
    short rowSecond;     /* +0x12 */
    signed char rowFlag; /* +0x14 */
    signed char fileState;   /* +0x15: the entry's fileState */
    unsigned char unmodeled_16[0x18 - 0x16];
};

static inline struct McSlotRecord *saveScreenSlotRecord(int slotIndex) {
    return &((struct McSlotRecord *) gSaveScreen->fileList.fileEntries)[slotIndex];
}

int func_00224958(void *data, int mode);
unsigned int xglClockDayTime2UInt(XglClock *clockTime);

/* Reads one card file from saveFile (the picture buffer): a file that fails the check gets fileState 4, a loaded one
 * fileState 2 and the play time, row and row flag of its slot record. */
int saveScreenLoadFileRecord(int slotIndex, unsigned char *saveFile) {
    struct McSlotRecord *record = saveScreenSlotRecord(slotIndex);

    if (func_00224958(saveFile, 0) == 0) {
        gSaveScreen->fileList.fileEntries[slotIndex].fileState = 4;
        return 0;
    }
    gSaveScreen->fileList.fileEntries[slotIndex].fileState = 2;
    record->playTime = xglClockDayTime2UInt((XglClock *) (saveFile + 0x50));
    record->unmodeled04 = *(int *) (saveFile + 0x2230);
    record->row = saveFile[0x1059];
    record->row = saveFile[(unsigned short) record->row + 0x105A];
    record->rowSecond = (signed char) saveFile[0x21C4 + (unsigned short) record->row * 0xE8];
    record->rowFlag = saveFile[0x4B];
    saveScreenRecordLookup(saveScreenSlotRecord(slotIndex),
                  *(int *) (saveFile + 0x1838));
    return 1;
}

/* The tail of a McFileEntry, reached from +4 of the entry; kept apart: the address form decides the emitted order. */
struct McFileFlags {
    unsigned char unmodeled_00[0x11];
    unsigned char fileState;
    unsigned char pictureState;
};

/* Gives every free file entry (fileState 0) a state: 1 while the card's free space holds another 95 blocks, else 3.
 * Returns how many entries got state 1. */
s32 saveScreenGrantFileStates(void) {
    s32 cardFreeBlocks;
    s32 i;
    s32 grantedCount;
    s32 fileEntryOffset;
    struct McFileFlags *fileEntryFlags;
    unsigned char grantedFileState;

    grantedCount = 0;
    cardFreeBlocks = gSaveScreen->fileList.freeSize;
    for (i = 0; i < 50; i++) {
        fileEntryOffset = 0x5B650 + i * 24;
        fileEntryFlags = (struct McFileFlags *) &((unsigned char *) gSaveScreen)[fileEntryOffset];
        if (fileEntryFlags->fileState == 0) {
            if (cardFreeBlocks >= 95) {
                grantedFileState = 1;
                cardFreeBlocks -= 95;
                grantedCount++;
            } else {
                grantedFileState = 3;
            }
            fileEntryFlags->fileState = grantedFileState;
        }
    }
    return grantedCount;
}

void func_002E7348(void *start, void *end);
int sceGsSyncPath(int mode, int timeout);
void xglDmaDirectNormal(unsigned int channel, unsigned int address, unsigned int count);
int xglJpegEncode(JpegEncodeRequest *request);
extern XglRenderState sRender;
extern unsigned long long D_003E6F80[24];

int saveThumbnailEncode(int quality, unsigned char *jpegBuffer)
{
    JpegEncodeRequest encodeRequest;
    unsigned long long bitbltWord;

    bitbltWord = 0x24020000 | (sRender.vram.draw_base << 5);
    D_003E6F80[10] = bitbltWord | (0xC800ULL << 19);
    D_003E6F80[8] = 0x801C0;
    func_002E7348(D_003E6F80, D_003E6F80 + 24);
    sceGsSyncPath(0, 0);
    xglDmaDirectNormal(2, (unsigned int) D_003E6F80, 12);
    sceGsSyncPath(0, 0);

    memset(&encodeRequest, 0, sizeof(encodeRequest));
    encodeRequest.outputBuffer = jpegBuffer;
    encodeRequest.frameBufferBase = 0x3800;
    encodeRequest.frameBufferWidth = 8;
    encodeRequest.width = 128;
    encodeRequest.height = 112;
    encodeRequest.originX = 0;
    encodeRequest.originY = 0;
    if (quality <= 0) {
        encodeRequest.luminanceQuality = 10;
        encodeRequest.chrominanceQuality = 10;
    } else {
        encodeRequest.luminanceQuality = quality;
        encodeRequest.chrominanceQuality = quality;
    }
    encodeRequest.samplingMode = 0;
    xglJpegEncode(&encodeRequest);
    return encodeRequest.outputSize;
}

typedef struct GifPacketData {
    u64 value;
    u64 address;
} GifPacketData;

#define GIF_PUT(gifCursor, registerValue, registerAddress) \
    (gifCursor)->value = (registerValue); \
    (gifCursor)->address = (registerAddress); \
    (gifCursor)++

typedef struct SaveIconUpload {
    u64 header[8];
    u64 transferSize; /* +0x40: TRXREG value */
    u64 registers[3];
    unsigned int loopCount; /* +0x60: GIFtag NLOOP */
} SaveIconUpload;

XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddDataN(XglPacket *packet, const void *data, unsigned int qwc);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a, unsigned int b, unsigned int c);
void fontQueueColorState(int mask);
void fontQueueSprite(unsigned short startX, unsigned short startY, unsigned short endX, unsigned short endY);
void nmlPacketGsInit(void);
int xglFontGetColorIndex(void);
void xglDirectQueueSubmitRef(void);
extern int xglDirectQueueCount;
extern GifPacketData *xglDirectQueueBuffer;
extern SaveIconUpload gSaveIconUpload;

static inline int gsPrimitiveX(int x) {
    x <<= 4;
    x += 0x7000;
    return x;
}

static inline int gsPrimitiveY(int y) {
    y <<= 4;
    y += 0x7200;
    return y;
}

/* Queues the 128 x 112 save-icon sprite of pictureData at (screenX, screenY): the DMA packet of the picture, then
 * a GIF packet that sets the texture state and the two corners of the sprite. */
void saveScreenDrawIcon(int screenX, int screenY, void *pictureData, unsigned char alpha) {
    XglPacket *packet;
    GifPacketData *gifCursor;
    GifPacketData *gifStart;
    int depth;

    packet = xglPacketGetCurrent();
    gSaveIconUpload.transferSize = GS_SET_TRXREG(0x80, 0x70);
    gSaveIconUpload.loopCount = 0xE00;
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, &gSaveIconUpload, 28);
    sceVif1PkRef(packet, pictureData, 0xE00, 0, 0x51000E00, 0);
    fontQueueColorState(GS_SET_RGBAQ(0, 0, 0, alpha, 0));
    fontQueueSprite((gsPrimitiveX(screenX) - 0x28) & 0xFFF8, (gsPrimitiveY(screenY) - 0x28) & 0xFFF8,
                    (gsPrimitiveX(screenX) + 0x518) & 0xFFF8, (gsPrimitiveY(screenY) + 0x478) & 0xFFF8);
    fontQueueColorState(GS_SET_RGBAQ(0x80, 0x80, 0x80, alpha, 0));
    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, 0, GS_TEXFLUSH);
    GIF_PUT(gifCursor, 0x264023800UL, GS_TEX0_1);
    GIF_PUT(gifCursor, (u64) 0x80 | ((u64) 0x80 << 32), GS_TEXA);
    GIF_PUT(gifCursor, GS_SET_CLAMP(1, 1, 0, 0, 0, 0), GS_CLAMP_1);
    GIF_PUT(gifCursor, 0x60, GS_TEX1_1);
    GIF_PUT(gifCursor, 0x156, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_UV(0, 0), GS_UV);
    depth = xglFontGetColorIndex();
    GIF_PUT(gifCursor, GS_SET_XYZ(gsPrimitiveX(screenX), gsPrimitiveY(screenY), (unsigned int) depth), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0x800, 0x700), GS_UV);
    depth = xglFontGetColorIndex();
    GIF_PUT(gifCursor, GS_SET_XYZ(gsPrimitiveX(screenX) + 0x500, gsPrimitiveY(screenY) + 0x460, (unsigned int) depth),
            GS_XYZ2);
    xglDirectQueueCount += ((unsigned int) gifCursor - (unsigned int) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

s32 xglJpegDecode();

void saveThumbnailDecode(s32 encodedJpeg, s32 thumbnail) {
    s32 decodeRequest[8];

    memset(&decodeRequest[0], 0, 0x20U);
    decodeRequest[0] = thumbnail + 0x58;
    decodeRequest[1] = encodedJpeg;
    xglJpegDecode(&decodeRequest[0]);
}

/* Returns 1 when fileIndex is one of the five file indexes that start at fileTable.cursor - cursorRow + 49 (mod 50). */
int saveScreenFileInWindow(s32 fileIndex) {
    s32 windowFile;
    s32 i;

    windowFile = gSaveScreen->fileTable.cursor -
                 gSaveScreen->cursorRow + 49;
    for (i = 0; i < 5; i++) {
        windowFile %= 50;
        if (fileIndex == windowFile) {
            return 1;
        }
        windowFile++;
    }
    return 0;
}

struct McCardRequestView {
    s32 fileIndex;
    void *screen;
    s32 size;
    void (*callback)(void);
};

int xglMcRequest(int port, int command, struct McCardRequest *cardRequest, void *buffer);

/* Starts loading file fileIndex of the card into slot: the slot's previous file loses its pictureState, the file's picture
 * buffer is cleared and an xglMcRequest read (command 4) fills it. */
void saveScreenStartSlotLoad(s32 loadSlot, s32 fileIndex) {
    struct McCardRequestView cardRequest;
    s32 pictureSize;

    if (gSaveScreen->cardReadPending == 0) {
        if (gSaveScreen->slots.fileStatusBytes[loadSlot] == 2) {
            gSaveScreen
                ->fileList.fileEntries[gSaveScreen->slots.fileIndexes[loadSlot]]
                .pictureState = 0;
        }
        pictureSize = 0x130B0;
        gSaveScreen->slots.fileStatusBytes[loadSlot] = 1;
        gSaveScreen->slots.fileIndexes[loadSlot] = fileIndex;
        gSaveScreen->fileList.fileEntries[fileIndex].thumbnailSlot = loadSlot;
        gSaveScreen->fileList.fileEntries[fileIndex].pictureState = 1;
        memset(gSaveScreen->picture, 0, pictureSize);
        cardRequest.fileIndex = fileIndex;
        cardRequest.size = pictureSize;
        cardRequest.screen = gSaveScreen->picture;
        xglMcRequest(gSaveScreen->port, 4, &cardRequest,
                     gSaveScreen->scanBuffer);
        gSaveScreen->loadingSlot = loadSlot;
        gSaveScreen->cardReadPending = 1;
    }
}

void saveThumbnailDecode(s32 encodedJpeg, s32 thumbnail);

/* Finishes the card read of the loading slot: marks the slot loaded (status 2), then decodes the file's header
 * (saveScreenLoadFileRecord, only for fileState 5) and its thumbnail, unless that header read fails. */
void saveScreenFinishSlotLoad(void) {
    signed char slotIndex;
    s32 fileIndex;

    if (gSaveScreen
            ->fileList.portResult[gSaveScreen->port] == 255) {
        return;
    }
    slotIndex = (signed char) gSaveScreen->loadingSlot;
    fileIndex = gSaveScreen->slots.fileIndexes[slotIndex];
    gSaveScreen->slots.fileStatusBytes[slotIndex] = 2;
    gSaveScreen->fileList.fileEntries[fileIndex].pictureState = 2;
    if (!(gSaveScreen->fileList.fileEntries[fileIndex].fileState == 5 &&
          saveScreenLoadFileRecord(fileIndex, gSaveScreen->picture) ==
              0)) {
        saveThumbnailDecode(
            (s32) gSaveScreen->thumbnails[slotIndex],
            (s32) gSaveScreen->picture);
    }
    gSaveScreen->cardReadPending = 0;
}

/* Loads fileIndex into the first slot that is empty or holds a file outside the window (saveScreenFileInWindow). */
void saveScreenLoadFileIntoFreeSlot(s32 fileIndex) {
    s32 i;

    for (i = 0; i < 5; i++) {
        if (gSaveScreen->slots.fileStatusBytes[i] == 0 ||
            (gSaveScreen->slots.fileStatusBytes[i] == 2 &&
             !saveScreenFileInWindow(gSaveScreen->slots.fileIndexes[i]))) {
            saveScreenStartSlotLoad(i, fileIndex);
            return;
        }
    }
}

extern signed char gSaveWindowOffsets[];

/* Once no card read is pending, starts the load of every file of the five-file window that is marked 2 or 5 and
 * not loaded yet. */
void saveScreenStartPendingLoads(void) {
    s32 windowStart;
    s32 i;
    s32 fileIndex;
    s32 fileEntryOffset;
    struct McFileFlags *fileEntryFlags;

    if (gSaveScreen->cardReadPending != 0) {
        saveScreenFinishSlotLoad();
    }
    if (gSaveScreen->cardReadPending != 0) {
        return;
    }
    windowStart =
        gSaveScreen->fileTable.cursor -
        gSaveScreen->cursorRow + 50;
    windowStart %= 50;
    for (i = 0; i < 5; i++) {
        fileIndex = (windowStart + gSaveWindowOffsets[i] + 50) % 50;
        fileEntryOffset = 0x5B650 + fileIndex * 24;
        fileEntryFlags = (struct McFileFlags *) ((unsigned char *) gSaveScreen + fileEntryOffset);
        if ((fileEntryFlags->fileState == 2 || fileEntryFlags->fileState == 5) && fileEntryFlags->pictureState == 0) {
            saveScreenLoadFileIntoFreeSlot(fileIndex);
        }
    }
}

typedef struct SaveStatusHeader {
    unsigned short unk_00;
    unsigned short buttons;
} SaveStatusHeader;

extern s16 gGameBootMode;
SaveStatusHeader *getStatusHeader(void);
void pagedTableAnimateSlots(struct SaveMenuTable *table);
s32 pagedTableUpdate(struct SaveMenuTable *table, s32 *selection);
void decodeIndexSet(s32 index);
void saveScreenFileListUpdate(s32 setup);
void saveScreenFormatCardUpdate(int formatRequested);

static inline void saveMenuKeyClear(struct SaveMenuTable *menu, s32 menuRowIndex, unsigned short keyBits) {
    if (menu->capacity != 0) {
        menu->entryKeys[menuRowIndex].flags &= ~keyBits;
    }
}

static inline void saveMenuKeySet(struct SaveMenuTable *menu, s32 menuRowIndex, unsigned short keyBits) {
    if (menu->capacity != 0) {
        menu->entryKeys[menuRowIndex].flags |= keyBits;
    }
}

/* Port menu. Key bit 0x100 of each port entry is cleared when its card result is 1 or 2, else set. setup builds
 * the port menu texts; otherwise the port menu runs (pagedTableUpdate) and its selection picks a port. */
void saveScreenPortMenuUpdate(s32 setup) {
    s32 selection;
    s32 port;
    struct SaveMenuTable *table = &gSaveScreen->portTable;

    for (port = 0; port < 2; port++) {
        if (gSaveScreen->fileList.portResult[port] == 1 ||
            gSaveScreen->fileList.portResult[port] == 2) {
            saveMenuKeyClear(table, port, 0x100);
        } else {
            saveMenuKeySet(table, port, 0x100);
        }
    }
    if (setup != 0) {
        mcClearSetupStatus();
        func_00220B90(0x10, messageTextGet(0x20));
        func_00220B90(0x40, messageTextGet(gGameBootMode == 2 ? 0x9E : 0x1F));
        func_00220B90(0x20, messageTextGet(0x1E));
        func_00220D28();
        saveScreenSetMessage(messageTextGet(gSaveScreen->mode + 0x28A));
        gSaveScreen->messagePending = 0;
        saveScreenSetFormattedMessage(messageTextGet(gSaveScreen->mode + 0x258), 0);
        return;
    }
    pagedTableAnimateSlots(table);
    if (pagedTableUpdate(table, &selection)) {
        if (selection >= 3) {
            if (gGameBootMode == 2) {
                decodeIndexSet(0);
            }
            saveScreenSetMenuAndDraw(0, 0);
        } else if (selection != 0) {
            seVoiceStartDefault(2);
            gSaveScreen->port = selection - 1;
            switch (gSaveScreen->fileList.portResult[selection - 1]) {
            case 1:
                saveScreenSetMenuAndDraw(saveScreenFileListUpdate,
                                      saveScreenDrawMenu);
                break;
            case 2:
                saveScreenSetMessage(
                    saveScreenFormatText(messageTextGet(0x28E), selection, 0));
                if (gSaveScreen->mode == 1) {
                    saveScreenSetMenuAndDraw(saveScreenFileListUpdate,
                                          saveScreenDrawMenu);
                } else {
                    saveScreenSetDraw(0);
                    saveScreenStartGauge(messageTextGet(0x260), 1, saveScreenFormatCardUpdate, 0);
                    saveScreenSetMessageText(messageTextGet(0x28C));
                }
                break;
            }
        } else {
            if (statusHeaderIsReadyMode20()) {
                saveScreenShowSheet(saveScreenFormatText(
                                  messageTextGet(0x28C),
                                  gSaveScreen->portTable.cursor + 1, 0),
                              1, saveScreenPortMenuUpdate, saveScreenDrawMenu);
            }
        }
    } else {
        if (getStatusHeader()->buttons & 0x50) {
            if (gGameBootMode == 2) {
                if (getStatusHeader()->buttons & 0x40) {
                    seVoiceStartDefault(2);
                    decodeIndexSet(2);
                } else {
                    seVoiceStartDefault(7);
                    decodeIndexSet(1);
                }
                gSaveScreen->displayState = 1;
            } else {
                saveScreenSetMenuAndDraw(0, 0);
            }
        }
    }
}

typedef struct MenuRect {
    u16 x;
    u16 y;
    u16 x1;
    u16 y1;
} MenuRect;

s32 xglFontSetAlphaScale();
void xglFontDrawBoxBlend(MenuRect *rect, s32 a, s32 b, s32 c);
s32 xglFontDrawStringScaled(u16 x, u16 y, s32 text, s32 size);
void fontQueueLine(u16 x, u16 y, u16 x1, u16 y1);
void func_0021E9A8(u16 x, u16 y, u16 width, u16 height);
void xglFontPrintTitle(void);
void xglFontPrintExtendedColor(s32 a, s32 b, s32 c);
s32 pagedTableGetFlagByte(struct SaveMenuTable *table, s32 index);
s32 pagedTableGetSlotByte(struct SaveMenuTable *table, s32 index);
s32 pagedTableGetEntryAt(struct SaveMenuTable *table, s32 index);
s32 xglFontGetFormattedWidth(s32 text);
void xglFontSetColor(s32 red, s32 green, s32 blue);
void xglFontPrintIndexed(s32 x, s32 y, s32 text);

/* Draws the save menu panel: the title bar, the three rows of the file list (or of the port list) and the three
 * text rows of the portMenu, each row's brightness taken from its flag byte. */
void saveScreenDrawMenu(u8 alpha) {
    s32 i;
    MenuRect rect;
    struct SaveMenuTable *portMenu = &gSaveScreen->portTable;

    xglFontSetAlphaScale(alpha);
    rect.x = 0x79C8;
    rect.y = 0x7CF8;
    rect.x1 = 0x8668;
    rect.y1 = 0x8268;
    fontQueueColorState(0x73221E1D);
    xglFontDrawBoxBlend(&rect, 2, 0, 0);
    fontQueueColorState(0x80562E2D);
    xglFontDrawStringScaled(0x7A48, 0x7D18, messageTextGet(0x25F), 0xD);
    rect.y = 0x7DE8;
    fontQueueColorState(0x73423732);
    for (i = 3; i >= 0; i--) {
        fontQueueLine(0x79E8, rect.y, 0x8648, rect.y);
        rect.y += 0x160;
    }
    if (gSaveScreen->menu == saveScreenFileListUpdate) {
        fontQueueColorState(0x5AF98980);
        func_0021E9A8(0x79E8, (0x7DE8 + gSaveScreen->port * 0x160) & 0xFFF8,
                      0xC60, 0x160);
    } else {
        rect.y = 0x7DE8;
        for (i = 0; i < 3; i++) {
            fontQueueColorState(((u64) (u8) ((u32) pagedTableGetSlotByte(portMenu, i) >> 1) << 24) | 0xE3FF5F);
            func_0021E9A8(0x79E8, rect.y, 0xC60, 0x160);
            rect.y += 0x160;
        }
    }
    rect.y = 0xBF;
    xglFontPrintTitle();
    xglFontPrintExtendedColor(0, 0, 0);
    for (i = 0; i < 3; i++) {
        s32 brightness = pagedTableGetFlagByte(portMenu, i);
        s32 text = pagedTableGetEntryAt(portMenu, i);
        s32 width = xglFontGetFormattedWidth(text);
        u32 color = 0x80626262;

        if (i == portMenu->cursor) {
            color = gSaveScreen->menu == saveScreenFileListUpdate ? 0x807C4440 : 0x80767B5D;
        }
        xglFontSetColor((u8) (((color & 0xFF) * brightness) >> 7),
                        (u8) ((((color >> 8) & 0xFF) * brightness) >> 7),
                        (u8) ((((color >> 16) & 0xFF) * brightness) >> 7));
        xglFontPrintIndexed(0x100 - (width >> 1), rect.y, text);
        rect.y += 0x16;
    }
}

void saveScreenSetRequestFlag(void);
void statusBlockFrameUpdate(void);
s32 xglMcMain(s32 mode, struct McScreen *screen);

typedef struct PortResultWord {
    s32 value;
} PortResultWord;

/* Card format and check screen. formatRequested starts the format request (command 7); otherwise polls the card
 * (xglMcMain, at most 101 times while it stays busy) and shows the result of the port. */
void saveScreenFormatCardUpdate(int formatRequested) {
    int pollCount;

    if (formatRequested != 0) {
        mcStateSetupRequest(0, 0);
        saveScreenSetMessage(
            saveScreenFormatText(messageTextGet(0x28F), gSaveScreen->port + 1, 0));
        saveScreenSetMessageText(messageTextGet(0x290));
        saveScreenSetRequestFlag();
        xglMcRequest(gSaveScreen->port, 7, 0, gSaveScreen->scanBuffer);
        return;
    }
    pollCount = 0;
    if (gSaveScreen->fileList.portResult[gSaveScreen->port] != 0xFF) {
    poll:
        if (gSaveScreen->fileList.portResult[gSaveScreen->port] == 2) {
            ((PortResultWord *) gSaveScreen->fileList.portResult)->value = xglMcMain(2, gSaveScreen);
            pollCount++;
            statusBlockFrameUpdate();
            if (pollCount < 101) {
                goto poll;
            }
        }
        gSaveScreen->messagePending = 0;
        if (gSaveScreen->fileList.portResult[gSaveScreen->port] == 1) {
            saveScreenSetMenuAndDraw(saveScreenFileListUpdate, saveScreenDrawMenu);
        } else {
            saveScreenShowSheet(saveScreenFormatText(messageTextGet(0x290), gSaveScreen->port + 1, 0),
                          1, saveScreenPortMenuUpdate, saveScreenDrawMenu);
        }
        ((void (*)()) saveScreenHandleError)();
    }
}

struct McCardRequestAlt {
    s32 fileIndex;
    void *screen;
    s32 size;
    void (*callback)(void);
};

void pagedTableSetPageSizes(struct SaveMenuTable *table, s32 a, s32 b);
void pagedTableAppend(struct SaveMenuTable *table, s32 value, s32 flags);
void pagedTableSeek(struct SaveMenuTable *table, u16 position);
void saveScreenClearRecords(void);

void func_00221A88(void *obj, int a, int b);
void addClampedCounter(void *obj, short value);
void func_00221B40(void *obj, int value);
void saveScreenMenuStep(s32 setup);
void saveScreenDrawFileList(u8 alpha);

/* Save-file list screen. setup starts the card directory read (command 2) and builds the file table; otherwise
 * the uiState machine reads the free space, then counts the saved files and grants the file entries. */
void saveScreenFileListUpdate(s32 setup) {
    struct McCardRequestAlt cardRequest;
    s32 i;
    s32 fileIndex;
    s32 grantedCount;
    struct SaveMenuTable *fileMenu;

    if (setup != 0) {
        gSaveScreen->uiState = 0;
        saveScreenSetMessage(saveScreenFormatText(
            messageTextGet(0x291), gSaveScreen->port + 1, 0));
        saveScreenSetMessageText(messageTextGet(0x28C));
        mcStateSetupRequest(0, 0);
        gSaveScreen->cardBusy = 0;
        func_00221A88(&gSaveScreen->fileList, 0xC6, 4);
        addClampedCounter(&gSaveScreen->fileList,
                          gSaveScreen->fileList.cursor);
        fileMenu = &gSaveScreen->fileTable;
        pagedTableSetPageSizes(fileMenu, 3, 0);
        for (i = 0; i < 50; i++) {
            pagedTableAppend(fileMenu, i, 2);
        }
        fileMenu->flags |= 0x16;
        gSaveScreen->cursorRow = 0;
        gSaveScreen->ringAlpha[0] = -0x80;
        gSaveScreen->ringAlpha[1] = 0;
        gSaveScreen->ringAlpha[2] = 0;
        saveScreenClearRecords();
        xglMcReset();
        cardRequest.fileIndex = -1;
        cardRequest.screen = gSaveScreen;
        xglMcRequest(gSaveScreen->port, 2, &cardRequest,
                     gSaveScreen->scanBuffer);
        return;
    }
    switch (gSaveScreen->uiState) {
    case 0:
        if (gSaveScreen
                ->fileList.portResult[gSaveScreen->port] != 0xFF) {
            gSaveScreen->fileCount = 0;
            for (fileIndex = 0; fileIndex < 50; fileIndex++) {
                if (gSaveScreen->fileUsed[fileIndex] != 0) {
                    gSaveScreen->fileList.fileEntries[fileIndex].fileState = 5;
                    gSaveScreen->fileCount++;
                }
            }
            gSaveScreen->lastFile =
                gSaveScreen->lastSavedFile;
            if (gSaveGameHeader.port != gSaveScreen->port) {
                gSaveGameHeader.fileIndex = -1;
            }
            if (gSaveGameHeader.fileIndex == 0xFF) {
                pagedTableSeek(&gSaveScreen->fileTable,
                               gSaveScreen->lastFile);
            } else {
                pagedTableSeek(&gSaveScreen->fileTable, gSaveGameHeader.fileIndex);
            }
            gSaveScreen->uiState = 1;
        }
        break;
    case 1:
        xglMcRequest(gSaveScreen->port, 1, 0,
                     &gSaveScreen->fileList.freeSize);
        gSaveScreen->uiState = 10;
        break;
    case 10:
        if (gSaveScreen
                ->fileList.portResult[gSaveScreen->port] != 0xFF) {
            if (gSaveScreen
                    ->fileList.portResult[gSaveScreen->port] == 2) {
                gSaveScreen->fileList.freeSize = 0x2000;
            }
            gSaveScreen->uiState = 11;
        }
        break;
    default:
        gSaveScreen->uiState++;
        break;
    case 16:
        grantedCount = saveScreenGrantFileStates();
        gSaveScreen->cardBusy = 0;
        if (gSaveScreen->fileCount == 0 && grantedCount == 0 &&
            gSaveScreen->mode == 0) {
            saveScreenShowSheet(
                saveScreenFormatText(
                    messageTextGet(0x28D), gSaveScreen->port + 1, 0x5F),
                1, saveScreenPortMenuUpdate, saveScreenDrawMenu);
            return;
        }
        gSaveScreen->fileList.scroll =
            gSaveScreen->fileList.fadeBase =
                gSaveScreen->fileList.scrollTarget =
                    gSaveScreen->fileTable.cursor * 0x670 - 0x3F0;
        saveScreenSetMenuAndDraw(saveScreenMenuStep, saveScreenDrawFileList);
        break;
    }
    func_00221B40(&gSaveScreen->fileList, 1);
}

/* Pad state of the status header: pressed this frame, held, and auto-repeated. */
typedef struct StatusPadState {
    unsigned short held;
    unsigned short pressed;
    unsigned short repeated;
} StatusPadState;

void saveScreenEntriesReadState(s32 setup);
void saveScreenFileReadState(s32 setup);
void pagedTableMoveCursor(struct SaveMenuTable *table, s32 step);

typedef struct SaveScreenRef {
    struct McScreen *screen;
} SaveScreenRef;

/* Reads gSaveScreen through a struct view: the plain global read compiles differently. */
static inline struct McScreen *saveScreen(void) {
    return ((SaveScreenRef *) &gSaveScreen)->screen;
}

/* Kept a macro: a static inline wrapper of getStatusHeader changes the emitted code. */
#define statusPad() ((StatusPadState *) getStatusHeader())

/* Save screen step. setup != 0 enters the screen. Otherwise the cursor moves over
   the file list with the up and down pad state, the three cursor rings fade, and
   the chosen file starts its message and its load or save screen. */
void saveScreenMenuStep(s32 setup) {
    s32 ringIndex;
    struct SaveMenuTable *fileTable;

    if (setup != 0) {
        saveScreenSetMessage(messageTextGet(saveScreen()->mode + 0x292));
        saveScreenSetMessageText(messageTextGet(0x28C));
        mcStateSetupRequest(7, 0);
        saveScreenStepFlagB();
        saveScreenSetFormattedMessage(messageTextGet(saveScreen()->mode + 0x25D), saveScreen()->port + 1);
        saveScreen()->fileList.scroll = saveScreen()->fileList.scrollTarget = saveScreen()->fileList.fadeBase;
        return;
    }
    fileTable = &saveScreen()->fileTable;
    pagedTableAnimateSlots(fileTable);
    if ((statusPad()->repeated & 0x1000) && !(statusPad()->held & 0x4000) &&
        (fileTable->cursor != 0 || (statusPad()->pressed & 0x1000))) {
        pagedTableMoveCursor(fileTable, -1);
        if (saveScreen()->cursorRow > 0) {
            saveScreen()->cursorRow--;
        } else {
            saveScreen()->fileList.fadeBase -= 0x670;
        }
        fileTable->top = (fileTable->cursor - saveScreen()->cursorRow + 50) % 50;
        if (fileTable->cursor == 49) {
            saveScreen()->fileList.scroll += 50 * 0x670;
            saveScreen()->fileList.scrollTarget += 50 * 0x670;
            saveScreen()->fileList.fadeBase += 50 * 0x670;
        }
        seVoiceStartDefault(1);
    }
    if ((statusPad()->repeated & 0x4000) && !(statusPad()->held & 0x1000) &&
        (fileTable->cursor != 49 || (statusPad()->pressed & 0x4000))) {
        pagedTableMoveCursor(fileTable, 1);
        if (saveScreen()->cursorRow < 2) {
            saveScreen()->cursorRow++;
        } else {
            saveScreen()->fileList.fadeBase += 0x670;
        }
        fileTable->top = (fileTable->cursor - saveScreen()->cursorRow + 50) % 50;
        if (fileTable->cursor == 0) {
            saveScreen()->fileList.scroll -= 50 * 0x670;
            saveScreen()->fileList.scrollTarget -= 50 * 0x670;
            saveScreen()->fileList.fadeBase -= 50 * 0x670;
        }
        seVoiceStartDefault(1);
    }
    for (ringIndex = 0; ringIndex < 3; ringIndex++) {
        if (saveScreen()->cursorRow == ringIndex) {
            if (saveScreen()->ringAlpha[ringIndex] < 0x80) {
                saveScreen()->ringAlpha[ringIndex] += (u8) gVsyncsPerFrame * 16;
            }
        } else if (saveScreen()->ringAlpha[ringIndex] > 0) {
            saveScreen()->ringAlpha[ringIndex] -= (u8) gVsyncsPerFrame * 16;
        }
    }
    saveScreenStartPendingLoads();
    if (statusHeaderIsReadyMode20() && (saveScreen()->mode != 0 || saveScreen()->fileImageReady != 0)) {
        saveScreen()->levelIndex[0] = saveScreen()->fileTable.cursor;
        switch (saveScreen()->fileList.fileEntries[saveScreen()->levelIndex[0]].fileState) {
        case 0:
        case 5:
        default:
            seVoiceStartDefault(3);
            break;
        case 1:
            if (saveScreen()->mode != 0) {
                seVoiceStartDefault(3);
                saveScreenShowSheet(messageTextGet(0x29F), 1, saveScreenMenuStep, saveScreenDrawFileList);
                saveScreenBeginLevelFade();
                break;
            }
        case 2:
            seVoiceStartDefault(2);
            saveScreenSetDraw(0);
            saveScreenBeginLevelFade();
            if (saveScreen()->fileList.fileEntries[saveScreen()->levelIndex[0]].fileState == 1) {
                saveScreenSetMessage(saveScreenFormatText(
                    messageTextGet(0x294), saveScreen()->port + 1, 0));
            } else {
                saveScreenSetMessage(saveScreenFormatText(
                    messageTextGet(saveScreen()->mode + 0x295), saveScreen()->port + 1, 0));
            }
            saveScreenStartGauge(messageTextGet(saveScreen()->mode + 0x26F), saveScreen()->mode == 0,
                          saveScreen()->mode != 0 ? saveScreenEntriesReadState : saveScreenFileReadState, 0);
            break;
        case 3:
            seVoiceStartDefault(3);
            if (saveScreen()->mode != 0) {
                saveScreenShowSheet(messageTextGet(0x29F), 1, saveScreenMenuStep, saveScreenDrawFileList);
                saveScreenBeginLevelFade();
            } else {
                saveScreenShowSheet(saveScreenFormatText(messageTextGet(0x28D),
                                                                             saveScreen()->port + 1, 0x5F),
                              1, saveScreenMenuStep, saveScreenDrawFileList);
                saveScreenBeginLevelFade();
            }
            break;
        case 4:
            seVoiceStartDefault(3);
            saveScreenShowSheet(messageTextGet(saveScreen()->mode + 0x29D), 1, saveScreenMenuStep, saveScreenDrawFileList);
            saveScreenBeginLevelFade();
            break;
        }
    } else if (statusHeaderIsReadyMode40() && saveScreen()->cardReadPending == 0) {
        seVoiceStartDefault(8);
        saveScreenSetMenuAndDraw(saveScreenPortMenuUpdate, saveScreenDrawMenu);
    } else if ((getStatusHeader()->buttons & 0x10) && saveScreen()->cardReadPending == 0) {
        saveScreenSetMenuAndDraw(0, 0);
    }
}

s32 xglFontDrawRightAlignedHalfCell(s32 baseX, s32 baseY, s32 glyphKind, s32 glyphIndex);

/* Draws one column of the save file list. packedGlyphs holds the glyph index above
   bit 8 and the glyph count in its low byte (the register is reused after the first sprite).
   The glyph kinds are the message ids from firstTextId on. */
void saveScreenDrawGlyphColumn(s32 column, s32 firstTextId, s32 packedGlyphs, s32 colour) {
    s32 drawn;
    s32 cellPosition;
    s32 cellX;
    s32 glyphKind;
    s32 glyphIndex;
    u8 alpha;

    drawn = 0;
    glyphIndex = packedGlyphs >> 8;
    alpha = colour >> 24;
    fontQueueColorState(alpha << 24);
    fontQueueSprite(
        29480, (((column + 1848) << 4) + 8) & 0xfff8, 30824, (((column + 1922) << 4) + 8) & 0xfff8);
    packedGlyphs &= 0xff;
    fontQueueColorState(((u64) alpha << 24) | 0x404040);
    fontQueueSprite(
        29512, (((column + 1850) << 4) + 8) & 0xfff8, 30792, (((column + 1920) << 4) + 8) & 0xfff8);
    fontQueueColorState(colour);
    column = column - packedGlyphs * 8 + 64;
    if (packedGlyphs != 0) {
        cellPosition = column * 16 + 29176;
        do {
            cellX = cellPosition & 0xffff;
            cellPosition += 256;
            glyphKind = messageTextGet(firstTextId + drawn);
            drawn++;
            xglFontDrawRightAlignedHalfCell(30152, cellX, glyphKind, glyphIndex);
        } while (drawn < packedGlyphs);
    }
}

void fontQueueRectTwoColors(u16 *rect, u32 first, u32 second, u32 nibbles);
s32 xglFontDrawCharScaled(u16 x, u16 y, s32 code, s32 size);
void xglFontPrintAlphaCommand();
void xglFontPrintEscapeByte();
s32 xglFontDrawDigitsMinTwo(u16 x, u16 y, s32 value, s32 size);
s32 xglFontDrawDigits(u16 x, u16 y, s32 value, s32 size);
void xglFontFreeBuffer();

static inline unsigned long long alphaMul(s32 scale, s32 alpha) {
    return (unsigned long long) (alpha * scale) >> 7;
}

void saveScreenDrawFileRow(s32 fileIndex, s32 row, u8 alpha);

/* Draws one save file row: the frame, the file number and the details of the file
   state (record->fileState). row is the screen row, alpha the fade. */
void saveScreenDrawFileRow(s32 fileIndex, s32 row, u8 alpha) {
    u16 rect[4];
    char numberText[16];
    struct McFileEntry *record =
        &gSaveScreen->fileList.fileEntries[fileIndex];
    unsigned long long shade = (unsigned long long) alphaMul(alpha, 115) << 24;
    u16 rowY = ((row + 0x71F) << 4) + 8;
    u32 playSeconds;

    rect[0] = 0x7288;
    rect[1] = rowY;
    rect[2] = 0x8D78;
    rect[3] = rowY + 0x670;
    fontQueueColorState(shade | 0x221E1D);
    xglFontDrawBoxBlend(rect, 2, 0, 0);
    rect[0] = 0x72C8;
    rect[1] = rowY + 0x40;
    rect[2] = rect[0] + 0x1A80;
    rect[3] = rect[1] + 0x120;
    fontQueueRectTwoColors(rect, ((unsigned long long) alpha << 24) | 0x3A2827, 0x3A2827, 0x101);
    fontQueueColorState(((unsigned long long) alpha << 24) | 0x804443);
    xglFontDrawCharScaled(0x7348, rowY + 0x60, 42, 16);
    xglFontPrintTitle();
    xglFontPrintAlphaCommand(0x44, 0, alpha);
    xglFontPrintExtendedColor(0, 0, 0);
    xglFontPrintEscapeByte(14);
    xglFontPrintIndexed(
        69, row + 3, ((u8 * (*) ()) resCmdMpeg2)(numberText, messageTextGet(609), fileIndex + 1, 0));
    switch (record->fileState) {
    case 2:
        xglFontSetColor(90, 91, 172);
        xglFontPrintEscapeByte(13);
        xglFontPrintIndexed(151, row + 3, messageTextGet((u16) record->row + 679));
        fontQueueColorState(((unsigned long long) alpha << 24) | 0x545934);
        xglFontDrawStringScaled(0x7ED8, rowY + 0x70, messageTextGet(60), 13);
        xglFontDrawStringScaled(0x8348, rowY + 0x70, messageTextGet(610), 13);
        xglFontDrawStringScaled(0x8348, rowY + 0x1A0, messageTextGet(87), 13);
        fontQueueColorState(((unsigned long long) alpha << 24) | 0xAC5B5A);
        xglFontDrawDigits(0x8158, rowY + 0x40, (u16) record->rowSecond, 16);
        fontQueueColorState(((unsigned long long) alpha << 24) | 0x626262);
        /* Play time in seconds, clamped to 999:59:59 (0x36EE7F). */
        playSeconds = record->playTime > 0x36EE7F ? 0x36EE7F : record->playTime;
        xglFontDrawDigitsMinTwo(0x8CB8, rowY + 0x40, playSeconds % 60, 16);
        playSeconds /= 60;
        xglFontDrawDigitsMinTwo(0x8AB8, rowY + 0x40, playSeconds % 60, 16);
        playSeconds /= 60;
        xglFontDrawDigits(0x88A8, rowY + 0x40, playSeconds, 16);
        xglFontDrawCharScaled(0x88D8, rowY + 0x50, 58, 16);
        xglFontDrawCharScaled(0x8AE8, rowY + 0x50, 58, 16);
        xglFontDrawDigits(0x8BC8, rowY + 0x160, record->unmodeled04, 16);
        xglFontDrawStringScaled(0x8C08, rowY + 0x180, messageTextGet(92), 16);
        fontQueueColorState(shade | 0x423732);
        fontQueueLine(0x7938, rowY + 0x310, 0x8C38, rowY + 0x310);
        fontQueueLine(0x7938, rowY + 0x470, 0x8C38, rowY + 0x470);
        fontQueueLine(0x7938, rowY + 0x5D0, 0x8C38, rowY + 0x5D0);
        xglFontPrintEscapeByte(16);
        xglFontSetColor(128, 128, 128);
        if (record->textIds[0] != 0) {
            xglFontPrintIndexed(150, row + 49,
                                               messageTextGet(record->textIds[0]));
        }
        if (record->textIds[1] != 0) {
            xglFontPrintIndexed(150, row + 71,
                                               messageTextGet(record->textIds[1]));
        }
        if ((u8) record->rowFlag != 0) {
            xglFontPrintEscapeByte(14);
            xglFontPrintIndexed(169, row + 26, messageTextGet(642));
        }
        if (record->pictureState == 2) {
            saveScreenDrawIcon(
                53, row + 27, gSaveScreen->thumbnails[record->thumbnailSlot], alpha);
            xglFontFreeBuffer();
        } else {
            saveScreenDrawGlyphColumn(row, 612, 0xC02, ((unsigned long long) alpha << 24) | 0x562E2D);
        }
        break;
    case 1:
        xglFontPrintEscapeByte(16);
        xglFontPrintIndexed(
            149, row + 50, messageTextGet(gSaveScreen->mode + 618));
        saveScreenDrawGlyphColumn(row, 611, 0xC01, ((unsigned long long) alpha << 24) | 0x545934);
        break;
    case 3:
        xglFontPrintEscapeByte(16);
        xglFontPrintIndexed(
            149, row + 50, messageTextGet(gSaveScreen->mode + 620));
        saveScreenDrawGlyphColumn(row, 614, 0xC02, ((unsigned long long) alpha << 24) | 0x562E2D);
        break;
    case 4:
        xglFontPrintEscapeByte(16);
        xglFontPrintIndexed(149, row + 50, messageTextGet(622));
        saveScreenDrawGlyphColumn(row, 616, 0xB02, ((unsigned long long) alpha << 24) | 0x16164A);
        break;
    default:
        saveScreenDrawGlyphColumn(row, 612, 0xC02, ((unsigned long long) alpha << 24) | 0x562E2D);
        break;
    }
}

void xglFontSetColorScale(s32 scale);
void xglFontResetState(void);
u8 statusByte0Get(void);
u8 statusByte1Get(void);
void fontDrawPanelFrame(u16 *pos, s32 a, s32 b);
void fontDrawRotatedSpriteU143(u16 *pos, s32 index, s32 w, s32 h);

/* Draws the whole save file list (rows at the scroll position), the three cursor
   rings and the two picture frames. alpha is the fade of the screen. */
void saveScreenDrawFileList(u8 alpha) {
    s32 rowY;
    s32 loopIndex;
    s32 scroll;
    s32 fadeBase;
    s32 scrollTarget;
    s32 scrollPosition;
    s32 nextRowY;
    s32 rowFade;
    s32 tableRow;
    u16 queuePos[2];
    struct SaveMenuTable *fileTable = &gSaveScreen->fileTable;

    xglFontSetAlphaScale(alpha);
    scroll = gSaveScreen->fileList.scroll;
    fadeBase = gSaveScreen->fileList.fadeBase;
    scrollTarget = gSaveScreen->fileList.scrollTarget;
    scrollPosition = scroll + (scroll - scrollTarget + ((fadeBase - scroll) >> 3));
    gSaveScreen->fileList.scroll = scrollPosition;
    rowY = -82400 - scrollPosition;
    gSaveScreen->fileList.scrollTarget = scrollPosition;
    for (loopIndex = -50; loopIndex < 100; loopIndex++) {
        nextRowY = rowY + 1648;
        if ((u32) nextRowY < 8816) {
            if (rowY < 1008) {
                rowFade = 128 - ((1008 - rowY) >> 3);
            } else if (rowY >= 4305) {
                rowFade = 128 - ((rowY - 4304) >> 3);
            } else {
                rowFade = 128;
            }
            tableRow = (loopIndex + 50) % 50;
            xglFontSetColorScale((u8) (((u32) pagedTableGetSlotByte(fileTable, tableRow) >> 1) + 96));
            if (rowFade > 0) {
                if (gSaveScreen->fadeStage == 0 ||
                    tableRow != gSaveScreen->levelIndex[0]) {
                    saveScreenDrawFileRow(tableRow, rowY >> 4, (u8) rowFade);
                }
            }
        }
        rowY = nextRowY;
    }
    xglFontSetColorScale(128);
    xglFontResetState();
    queuePos[0] = 29320;
    queuePos[1] = 30184;
    for (loopIndex = 0; loopIndex < 3; loopIndex++) {
        fontQueueColorState(
            (alphaMul(statusByte0Get() / 2 + 64,
                      gSaveScreen->ringAlpha[loopIndex])
             << 24) |
            0x71802F);
        fontDrawPanelFrame(queuePos, 6896, 1648);
        queuePos[1] += 1648;
    }
    xglFontSetColorScale((u8) (statusByte1Get() + 64));
    fontQueueColorState(0x80562E2D);
    queuePos[0] = 32632;
    queuePos[1] = 29896;
    fontDrawRotatedSpriteU143(queuePos, 0, 272, 272);
    queuePos[1] = -30392;
    fontDrawRotatedSpriteU143(queuePos, 2, 272, 272);
    xglFontSetColorScale(128);
}

void saveScreenDrawLevel(void) {
    s32 scaled;
    s32 level;

    scaled = (gSaveScreen->fadeLevel - 0x3F0) * gSaveScreen->fadeProgress[0];
    level = (scaled >> 7) + 0x3F0;
    saveScreenDrawFileRow(gSaveScreen->levelIndex[0], level >> 4, 0x80);
}

typedef struct BattleRosterEntry {
    u8 unmodeled_00[0x14];
    signed char stepCount; /* 0x14 */
} BattleRosterEntry;

extern char *gFontEscapeBuffer;
extern char D_00550BC8[];
extern u8 D_004B378B[];
extern BattleRosterEntry *func_A86DF8(s32 entryId);
extern s32 func_001894A0(s32 a, s32 b, s32 c);

typedef struct MovieStatus {
    u32 flags;
    u8 unmodeled_04[4];
    s16 mode;
} MovieStatus;

extern MovieStatus gGameLoopRuntimeStorage;

/* Builds the title texts of the save movie (the roster step count as two digits, the
   label and the title) and calls the movie player. fileBuffer and fileNumber are
   handed on unchanged to func_001894A0. */
s32 saveScreenPlayTitleMovie(s32 fileBuffer, s32 fileNumber) {
    char *movieName;
    char *titleText;
    char *tensText;
    char *onesText;
    char *labelText;
    BattleRosterEntry *rosterEntry;
    MovieStatus *status;
    s32 stepCount;

    movieName = gFontEscapeBuffer;
    titleText = D_00550BC8;
    tensText = titleText;
    rosterEntry = func_A86DF8(battleRosterCursorEntryIdGet(0));
    stepCount = rosterEntry->stepCount;
    onesText = messageTextGet(stepCount % 10 + 631);
    if (stepCount >= 10) {
        tensText = messageTextGet((stepCount / 10) % 10 + 631);
    }
    status = &gGameLoopRuntimeStorage;
    if (status->mode == 7600 || (status->flags & 0x08000000) != 0 || D_004B378B[0] != 0) {
        titleText = messageTextGet(641);
    }
    labelText = messageTextGet(630);
    ((int (*)(char *destination, ...)) sprintf)(movieName, labelText,
                                                      messageTextGet(battleRosterCursorEntryIdGet(0) + 679),
                                                      tensText, onesText, titleText);
    return func_001894A0(fileBuffer, fileNumber, movieName);
}

struct McCardRequestFileRead {
    s32 fileIndex;
    void *screen;
    s32 size;
    void (*callback)(void);
};

extern u8 *gSaveFileBuffer;
void func_00224A80(void);

/* Card state machine of the file screen. setup != 0 enters it. The states read the
   chosen file from the card in two requests, then wait in state 20 for the menu. */
void saveScreenFileReadState(s32 setup) {
    struct McCardRequestFileRead cardRequest;
    struct McFileList *list;

    if (setup != 0) {
        gSaveScreen->uiState = 0;
        func_00221A88(&gSaveScreen->fileList, 0xC6, 0x160);
        saveScreenSetMessage(saveScreenFormatText(
            messageTextGet(0x297), gSaveScreen->port + 1, 0));
        gSaveScreen->cardBusy = 3;
        saveScreenSetMessageText(messageTextGet(0x299));
        mcStateSetupRequest(0, 0);
        saveScreenSetRequestFlag();
        return;
    }
    switch (gSaveScreen->uiState) {
    case 0:
        if (gSaveScreen->cardReadPending != 0) {
            saveScreenStartPendingLoads();
            break;
        }
        {
            u8 *file = gSaveFileBuffer;

            cardRequest.fileIndex = gSaveScreen->levelIndex[0];
            cardRequest.screen = file;
            saveScreenPlayTitleMovie(file, cardRequest.fileIndex + 1);
            xglMcRequest(gSaveScreen->port, 6, &cardRequest,
                         gSaveScreen->scanBuffer);
            gSaveScreen->uiState = 1;
        }
        break;
    case 1:
        list = &gSaveScreen->fileList;
        list->position = 0x50;
        if (list->portResult[gSaveScreen->port] == 0xFF) {
            break;
        }
        gSaveScreen->uiState = 10;
    case 10:
        if (gSaveScreen->cardReadPending != 0) {
            saveScreenStartPendingLoads();
            break;
        }
        cardRequest.fileIndex = gSaveScreen->levelIndex[0];
        cardRequest.screen = &gXglMcAgedEntries;
        cardRequest.size = sizeof(gXglMcAgedEntries);
        func_00224A80();
        xglMcRequest(gSaveScreen->port, 5, &cardRequest,
                     gSaveScreen->scanBuffer);
        gSaveScreen->uiState = 11;
        addClampedCounter(&gSaveScreen->fileList,
                          gSaveScreen->fileList.cursor);
        break;
    case 11:
        if (gSaveScreen
                ->fileList.portResult[gSaveScreen->port] != 0xFF) {
            gSaveScreen->uiState = 12;
        }
        break;
    case 12:
        if (gSaveScreen->fileList.cursor ==
            gSaveScreen->fileList.previousCursor) {
            gSaveScreen
                ->fileList.fileEntries[gSaveScreen->levelIndex[0]]
                .fileState = 2;
            if (gSaveScreen
                    ->fileList.fileEntries[gSaveScreen->levelIndex[0]]
                    .pictureState == 2) {
                saveThumbnailDecode(
                    gSaveScreen
                        ->thumbnails[gSaveScreen
                                         ->fileList
                                         .fileEntries[gSaveScreen->levelIndex[0]]
                                         .thumbnailSlot],
                    &gXglMcAgedEntries);
            } else {
                saveScreenStartPendingLoads();
            }
            saveScreenLoadFileRecord(gSaveScreen->levelIndex[0],
                          (void *) &gXglMcAgedEntries);
            saveScreenSetMessageText(messageTextGet(0x28C));
            gSaveScreen->cardBusy = 0;
            ((void (*)(void)) saveScreenHandleError)();
            saveScreenSetMessage(messageTextGet(0x29B));
            mcClearSetupStatus();
            func_00220B90(0x10, messageTextGet(0x20));
            func_00220B90(0x40, messageTextGet(0x1F));
            func_00220B90(0x20, 0);
            func_00220D28();
            gSaveScreen->uiState = 20;
            gSaveGameHeader.port = gSaveScreen->port;
            gSaveGameHeader.fileIndex = gSaveScreen->levelIndex[0];
            gSaveScreen->fileLoaded = 1;
            seVoiceStartDefault(0x1C);
        }
        break;
    case 20:
        if (gSaveScreen->cardReadPending != 0) {
            saveScreenStartPendingLoads();
            break;
        }
        if (statusHeaderIsReadyMode20() || statusHeaderIsReadyMode40()) {
            if (statusHeaderIsReadyMode20()) {
                seVoiceStartDefault(2);
            } else {
                seVoiceStartDefault(8);
            }
            saveScreenSetMenuAndDraw(saveScreenMenuStep, saveScreenDrawFileList);
        } else if (getStatusHeader()->buttons & 0x10) {
            saveScreenSetMenuAndDraw(0, 0);
        }
        break;
    }
    func_00221B40(&gSaveScreen->fileList, 2);
}

struct McCardRequestEntriesRead {
    s32 fileIndex;
    void *screen;
    s32 size;
    void (*callback)(void);
};

void func_001B1D28(s32 mode);

/* Card state machine of the second file screen: one entries read (command 4), then
   the same wait in state 20. setup != 0 enters it. */
void saveScreenEntriesReadState(s32 setup) {
    struct McCardRequestEntriesRead cardRequest;

    if (setup != 0) {
        gSaveScreen->uiState = 0;
        func_00221A88(&gSaveScreen->fileList, 0xC6, 0xA);
        saveScreenSetMessage(saveScreenFormatText(
            messageTextGet(0x298), gSaveScreen->port + 1, 0));
        gSaveScreen->cardBusy = 3;
        addClampedCounter(&gSaveScreen->fileList,
                          gSaveScreen->fileList.cursor);
        saveScreenSetMessageText(messageTextGet(0x29A));
        mcStateSetupRequest(0, 0);
        saveScreenSetRequestFlag();
        return;
    }
    switch (gSaveScreen->uiState) {
    case 0:
        if (gSaveScreen->cardReadPending != 0) {
            saveScreenStartPendingLoads();
            break;
        }
        cardRequest.fileIndex = gSaveScreen->levelIndex[0];
        cardRequest.screen = &gXglMcAgedEntries;
        cardRequest.size = sizeof(gXglMcAgedEntries);
        xglMcRequest(gSaveScreen->port, 4, &cardRequest,
                     gSaveScreen->scanBuffer);
        gSaveScreen->uiState = 1;
        break;
    case 1:
        if (gSaveScreen
                ->fileList.portResult[gSaveScreen->port] != 0xFF) {
            gSaveScreen->uiState = 2;
        }
        break;
    case 2:
        if (gSaveScreen->fileList.cursor ==
            gSaveScreen->fileList.previousCursor) {
            gSaveScreen->cardBusy = 0;
            ((void (*)(void)) saveScreenHandleError)();
            saveScreenSetMessage(messageTextGet(0x29C));
            mcClearSetupStatus();
            func_00220B90(0x40, messageTextGet(0x9F));
            func_00220B90(0x20, 0);
            func_00220D28();
            gSaveScreen->uiState = 20;
            gSaveScreen->messagePending = 0;
            gSaveGameHeader.port = gSaveScreen->port;
            gSaveGameHeader.fileIndex = gSaveScreen->levelIndex[0];
            func_001B1D28(0);
            seVoiceStartDefault(0x1C);
        }
        break;
    default:
        if (gSaveScreen->uiState >= 0x3D || statusHeaderIsReadyMode20() ||
            statusHeaderIsReadyMode40()) {
            saveScreenSetMenuAndDraw(0, 0);
            gSaveScreen->displayState = 2;
        } else {
            gSaveScreen->uiState++;
        }
        break;
    }
    func_00221B40(&gSaveScreen->fileList, 1);
}

void xglFontFlush(void);
void xglFontSetDigitStep(s32 step);
void fontQueueSpriteReset(void);
void shardFieldUpdate(void *fields);
void tableSelectValue(void *list, s32 a, s32 b);
void xglFontPrintExtendedColorByte(s32 a, s32 b, s32 c, s32 d);
void xglClockReadBcd(void *clock);
void func_0021BB68(s32 x, s32 y, s32 seconds);
void sheetDrawFrame(s32 frame);
s32 sheetLatchedPositionGet(void);
void mcStateSeekEntry(s32 entry);
void func_0021ABB0(s32 a, s32 b, void *text, s32 c);
void func_002215A0(struct McGauge *gauge, s32 visible, s32 y);

static inline s32 saveScreenFadeAlpha(void) {
    if (gSaveScreen->fade < 16) {
        return (u8) (gSaveScreen->fade << 3);
    }
    return 0x80;
}

/* Draws the save screen each frame: the file list and rings, the level and the card
   busy text, the clock, the frame sheet and the status message. */
void saveScreenDraw(void) {
    u8 clock[8];
    s32 text;

    xglFontFlush();
    xglFontSetDigitStep(10);
    xglFontSetAlphaScale(0x80);
    fontQueueSpriteReset();
    xglFontSetAlphaScale(saveScreenFadeAlpha());
    shardFieldUpdate(gSaveScreen->fields);
    xglFontFreeBuffer();
    if (gSaveScreen->drawFading != 0 &&
        gSaveScreen->fileList.fadingAlpha != 0) {
        gSaveScreen
            ->drawFading(gSaveScreen->fileList.fadingAlpha);
        xglFontResetState();
    }
    if (gSaveScreen->draw != 0 &&
        gSaveScreen->fileList.drawAlpha != 0) {
        gSaveScreen
            ->draw(gSaveScreen->fileList.drawAlpha);
        xglFontResetState();
    }
    ((void (*)(s32 alpha)) xglFontSetAlphaScale)(0x80);
    if (gSaveScreen->fadeStage != 0) {
        saveScreenDrawLevel();
        xglFontResetState();
    }
    xglFontSetAlphaScale(0x80);
    func_002215A0(&gSaveScreen->gauge,
                  gSaveScreen->requestBits[0] & 2,
                  gSaveScreen->mode >= 2 ? 0x10C : 0xAE);
    if (gSaveScreen->cardBusy != 0) {
        tableSelectValue(&gSaveScreen->fileList, 0x9F, 0x111);
        if (gSaveScreen->cardBusy & 2) {
            text = messageTextGet(gSaveScreen->mode + 0x271);
            xglFontPrintTitle();
            xglFontPrintEscapeByte(12);
            xglFontSetColor(0x34, 0x59, 0x54);
            xglFontPrintExtendedColorByte(0, 0, 0, 2);
            xglFontPrintIndexed(0x100 - ((xglFontGetFormattedWidth(text) * 12) >> 5), 0x100, text);
        }
    }
    xglClockReadBcd(clock);
    func_0021BB68(0x200, 0x1C0, xglClockDayTime2UInt(clock) - gSaveGameHeader.startTime);
    xglFontSetAlphaScale(saveScreenFadeAlpha());
    sheetDrawFrame(0x19F);
    xglFontSetAlphaScale(0x80);
    xglFontSetAlphaScale(saveScreenFadeAlpha());
    mcStateSeekEntry(0x18E - sheetLatchedPositionGet());
    if (gSaveScreen->message[0] != 0) {
        func_0021ABB0(0, 11, gSaveScreen->message, 0);
    }
    xglFontSetAlphaScale(0x80);
}

struct McScreen * memPoolAlloc();
s32 bufferPointersRewind();
s32 completionBlockClear();
s32 pagedTablePoolInit();

s32 saveScreenCreate(void) {
    struct McScreen *screen;

    completionBlockClear();
    pagedTablePoolInit();
    screen = memPoolAlloc(0x5BB40);
    gSaveScreen = screen;
    memset(screen, 0, 0x5BB40U);
    bufferPointersRewind();
    return 1;
}

extern const char *gSaveImagePath;

/* Completion callback of the save file read: 4 marks the file image ready, -2 and -1 read it again. */
const char D_00538A18[24] = "mnu\\xx\\icon.bin";

void saveFileReadCallback(s32 fileReadState, s32 unusedA, s32 unusedB) {
    switch (fileReadState) {
    case 4:
        gSaveScreen->fileImageReady = 1;
        break;
    case 0:
    case 1:
    case 2:
    case 3:
        break;
    case -2:
    case -1:
        do {
            xglCdReadFile(gSaveImagePath, gSaveFileBuffer, 1, saveFileReadCallback);
        } while (0);
        break;
    }
}

typedef struct EncodedThumbnail {
    unsigned char bytes[0x1000];
} EncodedThumbnail;

/* The five-word command block scratchpadSendCmd23Mode0 and Mode1 send. */
typedef struct ScratchpadCommand {
    s32 words[5];
} ScratchpadCommand;

typedef struct GameLoopRuntime {
    s32 flags;
    unsigned char unmodeled_04[0x170];
    unsigned char slotState[4]; /* +0x174 */
} GameLoopRuntime;

extern EncodedThumbnail gSaveThumbnailEncoded;
void pagedTableInit(struct SaveMenuTable *table, s32 capacity);
void mcModuleReset(void);
void sheetStateReset(void);
void memPoolFree(void *block);
s32 xglCdServiceIfPending(void);
void scratchpadSendCmd23Mode1(ScratchpadCommand *command);
void scratchpadSendCmd23Mode0(ScratchpadCommand *command);
void slotSetupMode0(s32 a, s32 b, void *c);
unsigned char *stateBlockGet(void);
void pxySetColorScale(void *colorScale);
void pxyEnableFlag8(void);
void movieStepIfActive(void);
void func_002912B0(void);
void xglPadSetRepeat(s32 a, s32 b, s32 c, s32 d);

/* Builds the save screen menus, the first thumbnail and the file read for the current mode, then runs the attack sequence. */
void saveScreenSetup(void) {
    struct SaveMenuTable *portMenu = &gSaveScreen->portTable;
    unsigned char *thumbnailBuffer;
    s32 thumbnailQuality;

    pagedTableInit(portMenu, 3);
    pagedTableSetPageSizes(portMenu, 3, 0);
    pagedTableAppend(portMenu, messageTextGet(0x25A), 0x101);
    pagedTableAppend(portMenu, messageTextGet(0x25B), 0x101);
    pagedTableAppend(portMenu, messageTextGet(gGameBootMode == 2 ? 0x9E : 0x25C), 1);
    portMenu->flags |= 0x48;
    portMenu->columnFlags |= 0x100;
    if (gSaveScreen->mode == 0) {
        pagedTableSeek(portMenu, gSaveGameHeader.port & 1);
    } else {
        pagedTableSeek(portMenu, 0);
    }
    pagedTableInit(&gSaveScreen->fileTable, 50);
    gSaveScreen->fileList.portResult[0] =
        gSaveScreen->fileList.portResult[1] = 0;
    gSaveScreen->fade = 0;
    func_002213A8(
        &gSaveScreen->gauge, 0, 0, 0);
    mcModuleReset();
    sheetStateReset();
    if (gSaveScreen->mode == 0) {
        thumbnailBuffer = (unsigned char *) memPoolAlloc(0xE000);
        memset(thumbnailBuffer, 0, 0xE000);
        thumbnailQuality = 75;
        while (saveThumbnailEncode(thumbnailQuality, thumbnailBuffer) > 0x1000) {
            thumbnailQuality -= 25;
        }
        gSaveThumbnailEncoded = *(EncodedThumbnail *) thumbnailBuffer;
        memPoolFree(thumbnailBuffer);
        gSaveFileBuffer = memPoolAlloc(0x4000);
        gSaveScreen->fileImageReady = 0;
        xglGetLanguageCode(gSaveImagePath + 4);
        xglCdReadFile(gSaveImagePath, gSaveFileBuffer, 1, saveFileReadCallback);
        if (gSaveScreen->unmodeled_5BB2D != 0) {
            while (xglCdServiceIfPending()) {
                statusBlockFrameUpdate();
            }
        } else {
            ScratchpadCommand command = {{10, 0, 0, 0, 0x80}};

            scratchpadSendCmd23Mode1(&command);
            while (xglCdServiceIfPending()) {
                if (gGameLoopRuntimeStorage.flags & 0x40) {
                    slotSetupMode0(0xA0, 0x22, ((GameLoopRuntime *) &gGameLoopRuntimeStorage)->slotState);
                }
                pxySetColorScale(stateBlockGet() + 0x850);
                pxyEnableFlag8();
                movieStepIfActive();
                func_002912B0();
                statusBlockFrameUpdate();
            }
            scratchpadSendCmd23Mode0(&command);
        }
    } else {
        gSaveFileBuffer = 0;
    }
    saveScreenSetMenuAndDraw(saveScreenPortMenuUpdate, saveScreenDrawMenu);
    xglPadSetRepeat(0, 0xF000, 10, 3);
}

void xglPadSetDefaults(void);
void Vibration_Stop(void);
void xcBlockFlagsReset(void *block);

/* Tears the save screen down: stops the pad and the vibration, waits for the card, frees the buffers and the screen. */
void saveScreenDestroy(void) {
    xglPadSetDefaults();
    xglMcReset();
    Vibration_Stop();
    while (xglCdServiceIfPending() != 0) {
        statusBlockFrameUpdate();
    }
    xcBlockFlagsReset(gSaveScreen->fields);
    if (gSaveFileBuffer != 0) {
        memPoolFree(gSaveFileBuffer);
    }
    gSaveFileBuffer = 0;
    memPoolFree(gSaveScreen);
    gSaveScreen = 0;
}

int sceMcSync(s32 mode, s32 *cmd, s32 *result);
int slotRecord1CountDiff(void);
void memPoolInit(void *block, s32 size);
void memPoolReset(void);
void pagedTablePoolShutdown(void);
s32 saveScreenSession(s32 mode);

/* One save screen session: syncs the card, optionally runs a frame, then runs the screen with its own pool. */
s32 saveScreenRun(s32 mode) {
    s32 syncCmd;
    s32 syncResult;
    s32 savedVsyncsPerFrame;
    s32 screenResult;

    savedVsyncsPerFrame = gVsyncsPerFrame;
    sceMcSync(0, &syncCmd, &syncResult);
    ((void (*)(void)) xglMcMain)();
    sceMcSync(0, &syncCmd, &syncResult);
    ((void (*)(void)) xglMcMain)();
    sceMcSync(0, &syncCmd, &syncResult);
    if (mode == 0) {
        if (slotRecord1CountDiff() != 0) {
            return 0;
        }
        if (gGameLoopRuntimeStorage.flags & 0x40) {
            slotSetupMode0(0xA0, 0x22, ((GameLoopRuntime *) &gGameLoopRuntimeStorage)->slotState);
        }
        pxySetColorScale(stateBlockGet() + 0x850);
        pxyEnableFlag8();
        movieStepIfActive();
        func_002912B0();
        statusBlockFrameUpdate();
    }
    gVsyncsPerFrame = 1;
    memPoolInit((void *) 0x980000, 0x100000);
    screenResult = saveScreenSession(mode);
    pagedTablePoolShutdown();
    memPoolReset();
    gVsyncsPerFrame = savedVsyncsPerFrame;
    return screenResult;
}

extern u8 gSaveAbortFlag;
void shardFieldInit(unsigned char *field);
void pagedTableSetTextBox(struct SaveMenuTable *table, s32 a, s32 b, s32 c);
int saveConfirmMenu(int param);
void saveChoiceBoxDraw(unsigned char alpha);
void saveFileMenuUpdate(s32 setup);
void saveScreenDrawLabel(s32 alpha);

/* Runs the save screen for mode: creates it, builds its menus, runs the loop and frees it again. */
s32 saveScreenSession(s32 mode) {
    struct SaveMenuTable *fileMenu;
    s32 sessionResult;
    s32 abortRequested;

    gSaveAbortFlag = 0;
    if (saveScreenCreate() == 0) {
        return 0;
    }
    if (mode == 4) {
        mode = 0;
        gSaveScreen->unmodeled_5BB2D = 1;
    }
    gSaveScreen->mode = mode;
    saveScreenSetup();
    shardFieldInit(gSaveScreen->fields);
    if (mode == 2) {
        fileMenu = &gSaveScreen->fileTable;
        pagedTableSetPageSizes(fileMenu, 10, 0);
        pagedTableAppend(fileMenu, 0, 2);
        pagedTableAppend(fileMenu, 1, 2);
        pagedTableAppend(fileMenu, 2, 2);
        pagedTableAppend(fileMenu, 4, 2);
        pagedTableSetTextBox(fileMenu, 0, 192, 22);
        abortRequested = gSaveAbortFlag;
        fileMenu->columnFlags |= 0x180;
        if (abortRequested != 0) {
            saveScreenSetMenuAndDraw(saveFileMenuUpdate, saveScreenDrawLabel);
        } else {
            saveScreenSetMenuAndDraw(saveConfirmMenu, saveChoiceBoxDraw);
        }
    } else {
        seVoiceStartDefault(11);
    }
    sessionResult = saveScreenLoop();
    saveScreenDestroy();
    return sessionResult;
}

typedef struct CardResultWord {
    s32 result;
} CardResultWord;

void stepValueTowardTarget(void *value);
void func_00220E00(void);
int func_002209C0(void);

/* One frame of the save screen: the card result, the list alphas and the fade, then the menu callback. */
int saveScreenStepFrame(void) {
    ((CardResultWord *) &gSaveScreen->fileList.portResult[0])->result =
        ((int (*)(void)) xglMcMain)();
    stepValueTowardTarget(&gSaveScreen->fileList.fadingAlpha);
    stepValueTowardTarget(&gSaveScreen->fileList.drawAlpha);
    if ((short) gSaveScreen->fade < 255) {
        gSaveScreen->fade =
            gSaveScreen->fade +
            (((s32) ((u32) (u8) gVsyncsPerFrame << 24)) >> 23);
    }
    if (gSaveScreen->messagePending != 0) {
        saveScreenHandleCardResult();
    }
    if ((signed char) gSaveScreen->requestBits[0] & 1) {
        saveScreenGaugeStep();
    } else {
        if (gSaveScreen->menu != 0) {
            gSaveScreen->menu(0);
        }
        func_00221440(&gSaveScreen->gauge, 0);
    }
    if (gSaveScreen->fadeStage != 0) {
        saveScreenStepFade();
    }
    func_00220E00();
    return func_002209C0();
}

s32 statusHeaderTestFlags(void);
void sefFlagSetBit26(void);
void swapWordsSendCommand(s32 command);
void swapWordsRestore(void);

/* Runs the save screen frames until a menu leaves; returns 1 when a file was loaded or the screen ends in state 2. */
s32 saveScreenLoop(void) {
    ScratchpadCommand cmd;
    s32 result;

    for (;;) {
        saveScreenStepFrame();
        if (gSaveScreen->messagePending == 0) {
            if (statusHeaderTestFlags() != 0 ||
                (gGameLoopRuntimeStorage.flags & 0x4000000) != 0) {
                sefFlagSetBit26();
                gSaveScreen->displayState = 3;
            }
        }
        if (gSaveScreen->displayState != 0) {
            if (gSaveScreen->mode == 0 ||
                gSaveScreen->displayState == 2) {
                swapWordsSendCommand(5);
            }
        }
        pxyEnableFlag8();
        if (gSaveScreen->displayState != 3) {
            completionCallbackAdd(saveScreenDraw);
        }
        statusBlockFrameUpdate();
        if (gSaveScreen->displayState != 0) {
            if (gSaveScreen->mode == 0 ||
                gSaveScreen->displayState == 2) {
                swapWordsRestore();
            }
            break;
        }
    }
    if (gSaveScreen->displayState == 3) {
        memset(&cmd, 0, sizeof(cmd));
        cmd.words[4] = 0x80;
        scratchpadSendCmd23Mode0(&cmd);
    }
    result = 0;
    if (gSaveScreen->displayState == 2 ||
        gSaveScreen->fileLoaded == 1) {
        result = 1;
    }
    return result;
}

void savePortMenuUpdate(s32 setup);
void saveDialogAnswer(s32 value);

/* The confirm dialog of the save screen: a nonzero setup opens it, otherwise it waits for the answer. */
int saveConfirmMenu(int setup) {
    if (setup != 0) {
        saveScreenSetMessage(messageTextGet(825));
        saveScreenSetFormattedMessage(0, 0);
        saveScreenStartGauge(messageTextGet(824), 0, savePortMenuUpdate, saveDialogAnswer);
        mcClearSetupStatus();
        if (gSaveScreen->mode == 2) {
            func_00220B90(64, messageTextGet(158));
        } else {
            func_00220B90(64, messageTextGet(31));
        }
        func_00220B90(32, messageTextGet(30));
        return ((int (*)(void)) func_00220D28)();
    } else {
        struct McScreen *screen = gSaveScreen;
        int answer = (signed char) screen->requestBits[0];

        if (answer == 0) {
            if (screen->mode == 3) {
                answer = saveScreenSetMenuAndDraw(0, 0);
            } else {
                answer = saveScreenSetMenuAndDraw(saveFileMenuUpdate, saveScreenDrawLabel);
            }
        }
        return answer;
    }
}

struct Pos16 {
    unsigned short x;
    unsigned short y;
};

struct Box16 {
    struct Pos16 origin;
    struct Pos16 end;
};

/* Draws the save options box: one row for each message from 820 up to a blank one (at most four), then their text. */
void saveChoiceBoxDraw(unsigned char alpha) {
    struct Box16 box;
    s32 lineCount;
    s32 i;
    s32 textWidth;
    s32 x;

    lineCount = 0;
    for (i = 0; i < 4; i++) {
        if (*(char *) messageTextGet(820 + i) == ' ') {
            break;
        }
        lineCount++;
    }
    xglFontSetAlphaScale(alpha);
    box.origin.x = 29688;
    box.origin.y = 30856;
    box.end.x = -29688;
    box.end.y = 31112 + lineCount * 352;
    fontQueueColorState(0x73221E1D);
    xglFontDrawBoxBlend(&box, 2, 0, 0);
    box.origin.y = 30968;
    fontQueueColorState(0x73423732);
    if (lineCount + 1 > 0) {
        i = lineCount + 1;
        do {
            fontQueueLine(29720, box.origin.y, 35816,
                                                                       box.origin.y);
            box.origin.y += 352;
        } while (--i != 0);
    }
    xglFontPrintTitle();
    xglFontPrintExtendedColor(0, 0, 0);
    box.origin.y = 112;
    for (i = 0; i < lineCount; i++) {
        textWidth =
            xglFontGetFormattedWidth((const char *) messageTextGet(820 + i));
        x = 256 - (textWidth >> 1);
        xglFontPrintIndexed(
            x, box.origin.y, (const char *) messageTextGet(820 + i));
        box.origin.y += 22;
    }
}

extern s32 D_004B4F8C[];

void saveScreenResetDisplayState(void) {
    D_004B4F8C[0] = 0;
    gSaveScreen->displayState = 2;
}

static inline void saveMenuKeyClearView(struct SaveMenuTable *table, s32 index,
                                           unsigned short flags) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags &= ~flags;
    }
}

static inline void saveMenuKeySetView(struct SaveMenuTable *table, s32 index,
                                         unsigned short flags) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags |= flags;
    }
}

void saveSlotListUpdate(int listLoadRequested);

/* The port and file menu callback: a setup call shows the port message; otherwise it runs the chosen action. */
void savePortMenuUpdate(s32 setup) {
    s32 selection;
    s32 i;
    struct SaveMenuTable *portMenu = &gSaveScreen->portTable;

    for (i = 0; i < 2; i++) {
        if ((u16) (gSaveScreen->fileList.portResult[i] - 1) < 2) {
            saveMenuKeyClearView(portMenu, i, 0x100);
        } else {
            saveMenuKeySetView(portMenu, i, 0x100);
        }
    }
    if (setup != 0) {
        mcStateSetupRequest(6, 0);
        gSaveScreen->messagePending = 0;
        saveScreenSetMessage(messageTextGet(0x33A));
        saveScreenSetFormattedMessage(messageTextGet(0x33B), 0);
        return;
    }
    pagedTableAnimateSlots(portMenu);
    if (pagedTableUpdate(portMenu, &selection)) {
        if ((u32) selection >= 3) {
            seVoiceStartDefault(8);
            saveScreenSetMenuAndDraw(saveConfirmMenu, saveChoiceBoxDraw);
        } else if (selection != 0) {
            seVoiceStartDefault(2);
            gSaveScreen->port = selection - 1;
            switch (gSaveScreen->fileList.portResult[selection - 1]) {
            case 1:
            case 2:
                saveScreenSetMenuAndDraw(saveSlotListUpdate, saveDialogAnswer);
                break;
            }
        } else {
            if (statusHeaderIsReadyMode20()) {
                saveScreenShowSheet(saveScreenFormatText(
                                  messageTextGet(0x28C),
                                  gSaveScreen->portTable.cursor + 1, 0),
                              1, savePortMenuUpdate, saveDialogAnswer);
            }
        }
    } else {
        if (getStatusHeader()->buttons & 0x50) {
            seVoiceStartDefault(8);
            saveScreenSetMenuAndDraw(saveConfirmMenu, saveChoiceBoxDraw);
        }
    }
}

/* Dialog answer callback: forwards the low byte of the answer to saveScreenDrawMenu. */
void saveDialogAnswer(s32 dialogResult) {
    saveScreenDrawMenu(dialogResult & 0xFF);
}

static inline void saveMenuKeyClearAlt(struct SaveMenuTable *table, s32 index,
                                           unsigned short flags) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags &= ~flags;
    }
}

static inline void saveMenuKeySetAlt(struct SaveMenuTable *table, s32 index,
                                         unsigned short flags) {
    if (table->capacity != 0) {
        table->entryKeys[index].flags |= flags;
    }
}

s32 pagedTableGetCurrentEntry(struct SaveMenuTable *table);
void padSetLatchedValue(s32 value);
void Vibration_Set_Strong(s32 a, s32 b, s32 c);
void func_002257F0(s32 direction);
void soundDriverSetStreamMode(s32 mode);

/* The file list menu callback: a setup call shows its messages; otherwise it handles the buttons on the selected row. */
void saveFileMenuUpdate(s32 setup) {
    s32 selection;

    if (setup != 0) {
        saveScreenSetFormattedMessage(messageTextGet(0x341), 0);
        saveMenuKeyClearAlt(&gSaveScreen->fileTable,
                                gSaveScreen->fileTable.cursor, 0x800);
        mcClearSetupStatus();
        func_00220B90(0x40, messageTextGet(0x2B));
        func_00220B90(0x20, messageTextGet(0x1E));
        func_00220D28();
        return;
    }
    pagedTableAnimateSlots(&gSaveScreen->fileTable);
    sheetSetIndex(messageTextGet(
        pagedTableGetCurrentEntry(&gSaveScreen->fileTable) + 0x1A4));
    if (pagedTableUpdate(&gSaveScreen->fileTable, &selection)) {
        seVoiceStartDefault(1);
    } else if (statusHeaderIsReadyMode20() &&
               pagedTableGetCurrentEntry(&gSaveScreen->fileTable) == 4) {
        void *saveFileBuffer = ((struct McScreen * (*) ()) memPoolAlloc)(0x2800);
        seVoiceStartDefault(2);
        statusBlockFrameUpdate();
        func_001B1D28((s32) saveFileBuffer);
        memPoolFree(saveFileBuffer);
    } else if (statusHeaderIsReadyMode40()) {
        saveMenuKeySetAlt(&gSaveScreen->fileTable,
                              gSaveScreen->fileTable.cursor, 0x800);
        sheetSetIndex(messageTextGet(0x340));
        saveScreenStartGauge(messageTextGet(0x33F), 0, saveScreenResetDisplayState, 0);
        seVoiceStartDefault(6);
    } else if (getStatusHeader()->buttons & 0xA000) {
        switch (pagedTableGetCurrentEntry(&gSaveScreen->fileTable)) {
        case 0:
            gXglMcAgedEntries.flag28 ^= 1;
            soundDriverSetStreamMode(gXglMcAgedEntries.flag28);
            seVoiceStartDefault(1);
            break;
        case 1:
            gXglMcAgedEntries.flag3F ^= 1;
            seVoiceStartDefault(1);
            break;
        case 8:
            gXglMcAgedEntries.flag47 ^= 1;
            seVoiceStartDefault(1);
            break;
        case 2:
            gXglMcAgedEntries.flag42 ^= 1;
            if (gXglMcAgedEntries.flag42 == 0) {
                Vibration_Stop();
                padSetLatchedValue(10);
                Vibration_Set_Strong(0x80, 0x10, 0x3C);
            }
            seVoiceStartDefault(1);
            break;
        case 5:
            func_002257F0(getStatusHeader()->buttons & 0x8000 ? -1 : 1);
            seVoiceStartDefault(1);
            break;
        }
    }
}

s32 func_0021E390();

void saveScreenDrawLabel(s32 alpha) {
    xglFontSetAlphaScale(alpha & 0xFF);
    func_0021E390(0x30, 0x78, &gSaveScreen->fileTable, 0);
}

void saveWriteConfirmMenu(s32 setup);

/* The write confirm callback: a setup call shows message 0x342 and sets the abort flag; otherwise it opens the file list. */
void saveWriteConfirmMenu(s32 setup) {
    if (setup != 0) {
        saveScreenSetMessage(messageTextGet(0x342));
        gSaveAbortFlag = 1;
        mcClearSetupStatus();
        func_00220B90(0x40, messageTextGet(0x9F));
        func_00220B90(0x20, 0);
        func_00220D28();
        return;
    }
    if (statusHeaderIsReadyMode20() || statusHeaderIsReadyMode40()) {
        seVoiceStartDefault(2);
        gSaveScreen->mode == 3
            ? saveScreenSetMenuAndDraw(0, 0)
            : saveScreenSetMenuAndDraw(saveFileMenuUpdate,
                                    saveScreenDrawLabel);
    }
}

void saveWriteConfirmDraw(unsigned char alpha);

/* Draws the write confirm box: the save options box, then the two lines of messages 829 and 830. */
const char D_00538A98[8] = { 0x15, 0x02, 0x01, 0x16 };

void saveWriteConfirmDraw(unsigned char alpha) {
    struct Box16 box;
    s32 i;

    saveChoiceBoxDraw(alpha);
    xglFontSetAlphaScale(alpha);
    box.origin.x = 29688;
    box.origin.y = -32072;
    box.end.x = -29688;
    box.end.y = -30792;
    fontQueueColorState(0x73272120);
    xglFontDrawBoxBlend(&box, 2, 0, 0);
    fontQueueColorState(0x73554643);
    fontQueueSprite(
        box.origin.x + 64, box.origin.y + 64, box.end.x + 0xFFC0, box.origin.y + 432);
    box.origin.y = -31592;
    fontQueueColorState(0x73423732);
    for (i = 0; i < 3; i++) {
        fontQueueLine(29720, box.origin.y, 35816, box.origin.y);
        box.origin.y += 352;
    }
    fontQueueColorState(0x80562E2D);
    xglFontDrawStringScaled(29816, 0x8338, (u8 *) messageTextGet(829),
                                                                 16);
    xglFontPrintTitle();
    xglFontPrintExtendedColor(0, 0, 0);
    xglFontPrintEscapeByte(15);
    xglFontPrintIndexed(0, 0, D_00538A98);
    xglFontPrintIndexed(72, 298,
                                                                     (const char *) messageTextGet(830));
}

s32 saveCardRequestCallback(s32 mode, unsigned char *flagPtr);

/* Card request callback: -2 and -3 set the ui state, mode 3 marks the save pending when the changed flag is set. */
s32 saveCardRequestCallback(s32 mode, unsigned char *changedFlag) {
    if (mode == -2) {
        gSaveScreen->uiState = 1;
    } else if (mode == -3) {
        gSaveScreen->uiState = 2;
    } else if (mode == 3) {
        if (*changedFlag != 0) {
            gSaveScreen->writeConfirmPending = 1;
        }
    }
    return 0;
}

void saveScreenSetMessage(int sheetIndex);

void saveSlotListUpdate(int listLoadRequested)
{
    struct McCardRequest cardRequest;

    if (listLoadRequested != 0) {
        gSaveScreen->uiState = 0;
        saveScreenSetMessage(saveScreenFormatText(messageTextGet(0x291), gSaveScreen->port + 1, 0));
        mcStateSetupRequest(0, 0);
        gSaveScreen->cardBusy = 1;
        func_00221A88(&gSaveScreen->fileList, 198, 8);
        addClampedCounter(&gSaveScreen->fileList, gSaveScreen->fileList.cursor);
        gSaveScreen->writeConfirmPending = 0;
        if (gSaveScreen->fileList.portResult[gSaveScreen->port] == 2) {
            gSaveScreen->uiState = 1;
            return;
        }
        cardRequest.screen = gSaveScreen;
        cardRequest.callback = saveCardRequestCallback;
        xglMcRequest(gSaveScreen->port, 8, &cardRequest, gSaveScreen->scanBuffer);
        return;
    }
    switch (gSaveScreen->uiState) {
    case 0: {
        struct McScreen *screen = gSaveScreen;
        unsigned short cardResult = screen->fileList.portResult[screen->port];

        if ((unsigned short) (cardResult - 1) >= 2 && cardResult != 0xFF) {
            screen->messagePending = 0;
            gSaveScreen->cardBusy = 0;
            gSaveScreen->fadeStage = 0;
            saveScreenHandleError(screen, cardResult);
            gSaveScreen->cardReadPending = 0;
            xglMcReset();
            saveScreenShowSheet(saveScreenFormatText(messageTextGet(0x28C), gSaveScreen->port + 1, 0), 1,
                          savePortMenuUpdate, saveDialogAnswer);
        }
        break;
    }
    case 1:
        if (gSaveScreen->fileList.cursor == gSaveScreen->fileList.previousCursor) {
            gSaveScreen->cardBusy = 0;
            if (gSaveScreen->writeConfirmPending != 0) {
                seVoiceStartDefault(9);
                saveScreenSetMenuAndDraw(saveWriteConfirmMenu, saveWriteConfirmDraw);
            } else {
                saveScreenShowSheet(messageTextGet(0x33C), 1, savePortMenuUpdate, saveDialogAnswer);
            }
        }
        break;
    case 2:
        gSaveScreen->cardBusy = 0;
        saveScreenShowSheet(saveScreenFormatText(messageTextGet(0x28C), gSaveScreen->port + 1, 0), 1,
                      savePortMenuUpdate, saveDialogAnswer);
        break;
    }
    func_00221B40(&gSaveScreen->fileList, 1);
}

unsigned char D_003E6E54[12] = { 0 };
struct McScreen *gSaveScreen = 0;
u8 *gSaveFileBuffer = 0;
char D_00550BC8[4] = { 0 };
const char *gSaveImagePath = D_00538A18;
