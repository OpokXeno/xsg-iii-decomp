#include "common.h"

#include "main/xgl_2.h"
#include "main/tu007.h"
#include "gs.h"
#include "main/xgl_render.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))
typedef unsigned char u8;

typedef struct {
    u8 bytes[40];
} TagTableBytes;

const TagTableBytes D_00539020 = {
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff,
        0xf0, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
    }
};

const TagTableBytes D_00539048 = {
    {
        0x00, 0x00, 0x01, 0xcf, 0x01, 0x00, 0x01, 0xcf,
        0x00, 0x01, 0x01, 0xcf, 0x01, 0x01, 0x01, 0xcf,
        0x02, 0x01, 0x01, 0xcf, 0x00, 0x02, 0x01, 0xcf,
        0x10, 0x02, 0x01, 0xcf, 0x20, 0x02, 0x01, 0xcf,
        0x00, 0x03, 0x01, 0xcf, 0xff, 0xff, 0xff, 0xff
    }
};

const char D_00539070[24] = "\xa5" "\xe1" "\xa5" "\xe2" "\xa5" "\xea" "\xa4" "\xac" "\xa4" "\xbf" "\xa4" "\xea" "\xa4" "\xca" "\xa4" "\xa4" "\xa4" "\xaf" "\xa4" "\xb5" "\xa4" "\xa4" "";

extern u32 D_00550CA8;
void assertFailed(const char *message, const char *file, int line);

typedef int s32;

s32 poolReleaseMatching(s32 fieldNumber, s32 expected, s32 mask);
void func_001B6AB0();
u32 func_001B6840();
s32 func_001B6520(u32);

/* Frees pool tags, last table entry first, until func_001B6840 reports at least
 * `limit` free; then returns func_001B6520 of that free amount (0 when limit is 0). */
s32 func_002437A0(u32 limit) {
    s32 tagMasks[10];
    s32 expectedTags[10];
    s32 *expected;
    s32 *walk;
    s32 tagSlot;
    s32 tagCount;
    s32 fitResult;
    s32 endTag;
    s32 tagOffset;

    /* kept: the expected pointer is a separate variable; the array name decides the register order. */
    expected = expectedTags;
    *(TagTableBytes *) tagMasks = D_00539020;
    *(TagTableBytes *) expected = D_00539048;
    fitResult = 0;
    if (limit != 0) {
        D_00550CA8 = limit;
        tagCount = 0;
        if (expected[0] != -1) {
            endTag = -1;
            walk = expected;
            do {
                walk++;
                tagCount++;
            } while (*walk != endTag);
        }
        if (func_001B6840() >= D_00550CA8) {
        emitResult:
            fitResult = func_001B6520(func_001B6840());
        } else {
            tagSlot = tagCount;
            for (;;) {
                tagSlot--;
                tagOffset = tagSlot * 4;
                poolReleaseMatching(1, *(s32 *) ((u8 *) expected + tagOffset),
                                    *(s32 *) ((u8 *) tagMasks + tagOffset));
                func_001B6AB0();
                if (func_001B6840() >= D_00550CA8) {
                    goto emitResult;
                }
                if (tagSlot == 0) {
                    break;
                }
            }
        }
        if (fitResult == 0) {
            if (&D_00539070 != 0) {
                assertFailed(
                    "nTop!=0 || !\"\245\341\245\342\245\352\244\254\244\277\244\352\244\312\244\244\244\257\244\265\244\244\"",
                    "EventViewer.euc.c", 0x51);
            }
        }
    }
    return fitResult;
}

extern s32 gEventViewerReadItem;
s32 xglCdAddReadyBits(s32);
s32 xglCdGetDriveStatus();
s32 xglCdGetSectorAlignedFileSize();
s32 xglCdDriveStatusLowBitSet();
s32 xenoActorLoadItemStream();
s32 eventViewerSilenceSound();

void eventViewerCdPoll(void) {
    eventViewerSilenceSound();
    if ((xglCdDriveStatusLowBitSet() != 0) && !(xglCdGetDriveStatus() & 8)) {
        if (xglCdGetSectorAlignedFileSize() > 0) {
            gEventViewerReadItem = xenoActorLoadItemStream();
        }
        xglCdAddReadyBits(8);
    }
}

s32 xenoActorEventGetBufferA(void);
s32 xenoActorEventGetBufferC(void);
s32 func_001B2F60(s32, u8 *, s32);
void XenoActorReleaseAll(void);
s32 XenoActorPoolInitDefault();
void XenoActorInitHeap(s32, s32);
void xglStudioReset(void);
void setCurrentStudio(s32);
void xglStudioMainCameraInit(void);
void xglCameraSetPadPort(XglCamera *camera, s32 port);
void xglCdSetReadBuffers(u32, u32, u32, u32);
s32 xglCdDriveStatusFifthBitSet(void);
s32 xglCdGetCachedFileSize(void);
s32 xglCdServiceIfPending();
void xenoActorEventPrefetchBlock(void);
void xenoActorEventReadDispatch(s32);
void xenoActorEventReadNextChunk(void);
void xenoActorPlayAdxSound(s32);
void eventViewerCdPoll(void);

/* Size of the event area at gEventViewerReadItem. */
#define EVENT_AREA_BYTES 0x80000

/* Event-viewer boot: resets actors and the studio, sets camera 2 on pad port 1, reads the
 * event file (func_002437A0) when the disc is ready or waits for the CD, then starts the
 * actor heap after the loaded event block. */
void eventViewerBoot(void) {
    XglCamera *camera;
    s32 eventAreaBase;
    s32 heapBase;
    s32 heapUsed;

    camera = xglStudioGetCamera2(0);
    XenoActorReleaseAll();
    XenoActorPoolInitDefault();
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera->cameraState = 1;
    xglCameraSetPadPort(camera, 1);
    xglCdSetReadBuffers(0x980000, 0x9C0000, 0xA00000, 0xA40000);
    if (xglCdDriveStatusFifthBitSet() != 0 || xglCdDriveStatusLowBitSet() == 0) {
        xenoActorEventPrefetchBlock();
        eventAreaBase = func_002437A0(D_00550CA8 - 0xFF);
        gEventViewerReadItem = eventAreaBase;
        if (eventAreaBase == 0) {
            assertFailed("g_pEvAddrTop!=0", "EventViewer.euc.c", 0xA8);
        }
        heapBase = func_001B2F60(gEventViewerReadItem, "evt/", xenoActorEventGetBufferC());
        heapUsed = heapBase - gEventViewerReadItem;
        XenoActorInitHeap(heapBase, EVENT_AREA_BYTES - heapUsed);
        xenoActorEventReadDispatch(gEventViewerReadItem + EVENT_AREA_BYTES);
    } else if (xglCdDriveStatusLowBitSet() != 0) {
        eventViewerCdPoll();
        do {
        } while (xglCdServiceIfPending() != 0);
        xenoActorEventPrefetchBlock();
        if (xglCdGetCachedFileSize() > 0) {
            heapBase = func_001B2F60(gEventViewerReadItem, "evt/", xenoActorEventGetBufferC());
            heapUsed = heapBase - gEventViewerReadItem;
            XenoActorInitHeap(heapBase, EVENT_AREA_BYTES - heapUsed);
            xenoActorEventReadDispatch(gEventViewerReadItem + EVENT_AREA_BYTES);
            func_001B6AB0();
        }
    }
    xenoActorEventReadNextChunk();
    xenoActorPlayAdxSound(xenoActorEventGetBufferA());
    do {
    } while (xglCdServiceIfPending() != 0);
}

s32 xglFontPrintDirect();
s32 xglFontPrint();

void eventViewerDrawCaption(u8 *caption) {
    xglFontPrint(0xA, 0x1E, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col60c000;");
    xglFontPrintDirect(caption);
    xglFontPrint(0x12C, 0x15E, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col304070;");
    xglFontPrintDirect("NOW LOADING...");
}

static char **gEventViewerLineTable;
s32 xglCdGetFileSize(const char *);
s32 xglCdReadFile(const char *, void *, s32, void *);
void *memset(void *, s32, unsigned int);

/* Reads the text file `path` into buffer and splits it into 128-byte lines (CR LF or LF)
 * in the table at 0x1F97000; returns the line count, or -1 when the read fails. */
s32 func_00243C00(char *buffer, const char *path) {
    s32 fileSize;
    s32 i;
    s32 lineCount;
    s32 column;
    char **lineTable;
    char *lineBuffer;

    fileSize = xglCdGetFileSize(path);
    memset(buffer, 0, fileSize);
    if (xglCdReadFile(path, buffer, 0, 0) <= 0) {
        return -1;
    }
    memset((void *) 0x1F97000, 0, 0x19000);
    lineTable = (char **) 0x1F97000;
    gEventViewerLineTable = lineTable;
    lineBuffer = (char *) 0x1F97400;
    for (i = 0; i < 256; i++) {
        lineTable[i] = lineBuffer;
        lineBuffer += 128;
    }
    lineCount = 0;
    column = 0;
    for (i = 0; i < fileSize; i++) {
        if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
            lineTable[lineCount][column] = 0;
            lineCount++;
            column = 0;
            lineTable = gEventViewerLineTable;
        } else if (buffer[i] != '\n') {
            lineTable[lineCount][column] = buffer[i];
            column++;
            lineTable = gEventViewerLineTable;
        }
    }
    lineTable[lineCount][column] = 0;
    return lineCount;
}

extern PadRecord PadData[2];
extern u8 D_004B377F[];
extern s32 D_00550CB0;
extern char *gEventViewerTextBuffer;
void xglSleep();
void xglRenderClearFrame();
void xglCameraSetWindow(s32, s32, s32, s32, s32);
s32 xglStudioSelectGetActiveCamera(s32);
void studioCameraAt();
void pxyEnableFlag8();
void xglRenderIoBegin();
s32 sprintf(char *dst, const char *fmt, ...);
u32 func_0018F7E8();
void func_001B64D8(s32, s32);
void soundWorkSource();
void xglCdSetDriveStatus();
void eventViewerResetStart(s32 mode);
void eventViewerRunEvent(void *arg);

/* Event file menu: draws the lines of evt.flist (15 per page) with the selected one
 * highlighted, moves the selection with the pad, and on pressed & 0xA0 loads the chosen
 * event and calls eventViewerRunEvent with its name; returns to the actor reset at the end. */
void eventViewerFileMenu(void) {
    XglCamera *camera;
    char *listText;
    s32 pathBuffer[0x40];
    s32 lineCount;
    s32 selectedLine;
    s32 pageIndex;
    s32 pageStart;
    s32 pageEnd;
    s32 lastLine;
    s32 line;
    u32 reservedSize;

    selectedLine = 0;
    camera = xglStudioGetCamera2(0);
    listText = gEventViewerTextBuffer;
    xglSleep();
    xglSleep();
    lineCount = func_00243C00(listText, "evt\\.flist");
    if (lineCount >= 0) {
        D_004B377F[0] = 1;
        if ((PadData[0].heldAndPressed & 0x08000100) != 0x08000100) {
            lastLine = lineCount - 1;
            do {
                studioCameraAt(&camera, 0);
                pxyEnableFlag8();
                xglRenderIoBegin();
                pageIndex = selectedLine / 15;
                pageStart = pageIndex * 15;
                pageEnd = (pageIndex + 1) * 15;
                if (PadData[0].directionRepeated & 0x1000) {
                    selectedLine--;
                    if (selectedLine < pageStart) {
                        selectedLine = pageStart + 14;
                    }
                    if (selectedLine >= lineCount) {
                        selectedLine = lastLine;
                    }
                }
                if (PadData[0].directionRepeated & 0x4000) {
                    selectedLine++;
                    if (selectedLine >= pageEnd) {
                        selectedLine = pageStart;
                    }
                    if (selectedLine >= lineCount) {
                        selectedLine = pageStart;
                    }
                }
                if (PadData[0].directionRepeated & 0x8000) {
                    selectedLine -= selectedLine % 15;
                    selectedLine -= 15;
                    if (selectedLine < 0) {
                        selectedLine = lastLine;
                    }
                }
                if (PadData[0].directionRepeated & 0x2000) {
                    selectedLine -= selectedLine % 15;
                    selectedLine += 15;
                    if (selectedLine >= lineCount) {
                        selectedLine = 0;
                    }
                }
                line = pageStart;
                sprintf(&SEF_SCRATCHPAD(void, 0x0), "%2d / %2d", pageIndex + 1, lastLine / 15 + 1, pageStart);
                xglFontPrint(0xA, 5, 0xFFFFFF, "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
                /* kept: the cast matches the unprototyped xglFontPrintDirect call of the original. */
        ((void (*)()) xglFontPrintDirect)(&SEF_SCRATCHPAD(char, 0x0));
                for (; line < pageEnd && line < lineCount; line++) {
                    if (line == selectedLine) {
                        xglFontPrint(0xA, (selectedLine - pageStart) * 20 + 0x1E, 0xFFFFFF,
                                     "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col60c000;");
                    } else {
                        xglFontPrint(0xA, (line - pageStart) * 0x14 + 0x1E, 0xFFFFFF,
                                     "$bg3;$prop1;$han1;$xx0,36;$xx1,8;$xx2,0;$col707070;");
                    }
                    xglFontPrintDirect(gEventViewerLineTable[line]);
                }
                if (PadData[0].pressed & 0xA0) {
                    sprintf(pathBuffer, "evt\\%s", gEventViewerLineTable[selectedLine]);
                    {
                        gEventViewerTextBuffer = (char *) 0xE00000;
                        D_00550CB0 = 0x01FB0000;
                        D_00550CA8 = 0x011B0000;
                        func_001B64D8(0xE00000, 0x011B0000);
                    }
                    reservedSize = func_0018F7E8(8);
                    soundWorkSource(1, func_001B6520(reservedSize), reservedSize);
                    D_00550CA8 = D_00550CA8 - reservedSize;
                    if (PadData[0].pressed & 0x80) {
                        xglCdSetDriveStatus(0x32);
                    } else if (PadData[0].held & 1) {
                        xglCdSetDriveStatus(0x13);
                    } else {
                        xglCdSetDriveStatus(0x12);
                    }
                    eventViewerDrawCaption(gEventViewerLineTable[selectedLine]);
                    xglSleep();
                    eventViewerDrawCaption(gEventViewerLineTable[selectedLine]);
                    xglSleep();
                    eventViewerDrawCaption(gEventViewerLineTable[selectedLine]);
                    xglSleep();
                    eventViewerResetStart((s32) gEventViewerLineTable[selectedLine]);
                    eventViewerCdPoll();
                    eventViewerRunEvent(gEventViewerLineTable[selectedLine]);
                }
                camera->cameraState = 1;
                xglRenderClearFrame();
                xglCameraSetWindow(xglStudioSelectGetActiveCamera(0), 0, 0x1C, 0x200, 0x15C);
                xglSleep();
            } while ((PadData[0].heldAndPressed & 0x08000100) != 0x08000100);
        }
        XenoActorReleaseAll();
        xglRenderClearFrame();
        xglCameraSetWindow(xglStudioSelectGetActiveCamera(0), 0, 0, 0x200, 0x1C0);
    }
}

typedef long long s64;

extern s32 gEventViewerExchangeSuspended;
s64 xenoActorEventGetCurrentRangeLength(void);
s32 xglCdDriveReadyStatusLowBitSet(void);
s32 xenoActorEventFindTableInterval(void);
s32 xenoActorEventGetCurrentDataBase(void);
s32 xenoActorEventGetStatus23C(void);
s32 xenoActorEventGetStatus240(void);
s32 xenoActorEventGetStatus244(void);
s32 xenoActorEventGetStatus248(void);
s32 xenoActorEventCountInactive(void);
s32 xenoActorEventCountActive(void);
s32 xenoActorEventGetStatus210(void);
s32 xenoActorEventGetCurrentField38(void);
s32 xenoActorEventGetCurrentField18(void);
s32 xenoActorEventGetCurrentField30(void);
s32 xenoActorEventGetStatus234(void);
s32 func_0019D4E8(const char *text, s32 x, s32 y, f32 weight, s32 kind, s32 count, s32 mode);

#define EV_SCRATCH (&SEF_SCRATCHPAD(char, 0x0))

static __inline__ s32 evBcdHour(s32 status) {
    s32 h = status / 1800;
    return ((h / 10) << 4) + (h % 10);
}

static __inline__ s32 evBcdMin(s32 status, s32 sixty) {
    s32 m = (status / 30) % sixty;
    return ((m / 10) << 4) + (m % 10);
}

static __inline__ s32 evBcdSec(s32 status) {
    s32 x = status % 30;
    return ((x / 10) << 4) + (x % 10);
}

static __inline__ double dbits_to_f64(s64 vector) {
    union {
        double d;
        s64 i;
    } u;
    u.i = vector;
    return u.d;
}

static __inline__ s64 f64_to_dbits(double d) {
    union {
        double d;
        s64 i;
    } u;
    u.d = d;
    return u.i;
}

s32 xenoActorEventGetStatus224(void);

/* Event-viewer debug HUD: prints the frame, CUT, TIME, DISP and the MB figures of the
 * CD stream (memStat*) or of the heap (mapCount, objCount, effCount), each in a fixed screen
 * position; the warning line colours by the memory left under memLimitMB. */
void eventViewerDebugHud(void) {
    s32 allCapacity;
    s32 frameNumber;
    s64 rangeLength;
    s32 readyBitSet;
    s32 driveBitSet;
    s32 cutCount;
    s32 streamDataBase;
    s32 eventBufferC;
    s32 mapCapacity;
    s32 objCapacity;
    s32 effCapacity;
    s32 mapUsed;
    s32 effUsed;
    s64 memBits;
    f32 memUsedMB;
    f32 textScale;
    f32 memStat248;
    f32 memStat244;
    f32 memStat240;
    f32 memStat23C;
    f32 memLimitMB;
    f32 roundHalf;
    const char *memFormat;

    frameNumber = xenoActorEventGetStatus224();
    rangeLength = xenoActorEventGetCurrentRangeLength();
    readyBitSet = xglCdDriveReadyStatusLowBitSet();
    driveBitSet = xglCdDriveStatusLowBitSet();
    cutCount = xenoActorEventFindTableInterval();
    streamDataBase = (s32) (f32) xenoActorEventGetCurrentDataBase();
    roundHalf = 0.5f;
    memStat23C = (f32) xenoActorEventGetStatus23C() * 9.5367431640625e-07f;
    memLimitMB = 17.6875f;
    memStat240 = (f32) xenoActorEventGetStatus240() * 9.5367431640625e-07f;
    memStat244 = (f32) xenoActorEventGetStatus244() * 9.5367431640625e-07f;
    memStat248 = (f32) xenoActorEventGetStatus248() * 9.5367431640625e-07f;
    eventBufferC = xenoActorEventGetBufferC();
    sprintf(EV_SCRATCH, "%s", eventBufferC);
    func_0019D4E8(EV_SCRATCH, 10, 2, 1.7f, 0xFFF06060, 0xFFFFFF, 1);
    if (cutCount < 0) {
        cutCount = 0;
    }
    if (cutCount >= 0) {
        sprintf(EV_SCRATCH, "CUT %d", cutCount);
        func_0019D4E8(EV_SCRATCH, 10, 20, 1.2f, 0xFFF06060, 0xFFFFFF, 1);
    }
    memUsedMB = 1.2f;
    sprintf(EV_SCRATCH, "FRAME %4d / %4d", frameNumber, rangeLength);
    func_0019D4E8(EV_SCRATCH, 100, 2, memUsedMB, -1, 0xFFFFFF, 1);
    sprintf(EV_SCRATCH, "TIME %2x/%2x/%2x", evBcdHour(frameNumber), evBcdMin(frameNumber, 60), evBcdSec(frameNumber));
    func_0019D4E8(EV_SCRATCH, 250, 15, memUsedMB, -1, 0xFFFFFF, 1);
    sprintf(EV_SCRATCH, "DISP   %3d / 30", 30 - xenoActorEventGetStatus234());
    func_0019D4E8(EV_SCRATCH, 100, 15, memUsedMB, -1, 0xFFFFFF, 1);
    if (gEventViewerExchangeSuspended == 2) {
        if (readyBitSet != 0) {
            sprintf(EV_SCRATCH, "LIGHT EVENT");
        } else {
            sprintf(EV_SCRATCH, "LIGHT CF");
        }
        func_0019D4E8(EV_SCRATCH, 330, 380, 1.2f, -1, 0xFFFFFF, 1);
        if (driveBitSet != 0) {
            sprintf(EV_SCRATCH, "READ VCD");
        } else {
            sprintf(EV_SCRATCH, "READ EV_EP3");
        }
        textScale = 1.2f;
        func_0019D4E8(EV_SCRATCH, 330, 395, textScale, -1, 0xFFFFFF, 1);
        sprintf(EV_SCRATCH, "STREAM %6d Byte", streamDataBase);
        func_0019D4E8(EV_SCRATCH, 330, 410, textScale, -1, 0xFFFFFF, 1);
        if (xglCdDriveStatusLowBitSet() != 0) {
            memUsedMB = memStat248 + roundHalf;
            if (memLimitMB < memStat248 + 2.0f) {
                {
                    const char *allFormat = "ALL %2.2f / %2.2fMB";
                    sprintf(EV_SCRATCH, allFormat, memUsedMB, 17.6875);
                }
                func_0019D4E8(EV_SCRATCH, 150, 380, textScale, 0xFF0000FF, 0xFFFFFF, 1);
            } else if (memLimitMB < memStat248 + 5.0f) {
                {
                    const char *allFormat = "ALL %2.2f / %2.2fMB";
                    sprintf(EV_SCRATCH, allFormat, memUsedMB, 17.6875);
                }
                func_0019D4E8(EV_SCRATCH, 150, 380, textScale, 0xFF00FFFF, 0xFFFFFF, 1);
            } else {
                {
                    const char *allFormat = "ALL %2.2f / %2.2fMB";
                    sprintf(EV_SCRATCH, allFormat, memUsedMB, 17.6875);
                }
                func_0019D4E8(EV_SCRATCH, 150, 380, textScale, -1, 0xFFFFFF, 1);
            }
            {
                const char *matFormat;
                matFormat = "MAT %2.2f / %2.2fMB";
                memBits = f64_to_dbits(memUsedMB);
                memUsedMB = 1.2f;
                sprintf(EV_SCRATCH, matFormat, 0.5, dbits_to_f64(memBits));
            }
            {
                const char *xepFormat = "XEP %2.2f / %2.2fMB";
                func_0019D4E8(EV_SCRATCH, 150, 395, memUsedMB, -1, 0xFFFFFF, 1);
                sprintf(EV_SCRATCH, xepFormat, memStat248, dbits_to_f64(memBits));
            }
            func_0019D4E8(EV_SCRATCH, 150, 410, memUsedMB, -1, 0xFFFFFF, 1);
        } else {
            s32 objUsed;
            mapCapacity = xenoActorEventCountInactive();
            objCapacity = xenoActorEventCountActive();
            effCapacity = xenoActorEventGetStatus210();
            allCapacity = mapCapacity + objCapacity + effCapacity;
            mapUsed = xenoActorEventGetCurrentField38();
            objUsed = xenoActorEventGetCurrentField18();
            effUsed = xenoActorEventGetCurrentField30();
            memUsedMB = memStat23C + memStat240 + memStat244 + roundHalf;
            sprintf(EV_SCRATCH, "ALL %3d / %3d", objUsed + mapUsed + effUsed, allCapacity);
            func_0019D4E8(EV_SCRATCH, 20, 380, textScale, -1, 0xFFFFFF, 1);
            sprintf(EV_SCRATCH, "OBJ %3d / %3d", objUsed, objCapacity);
            func_0019D4E8(EV_SCRATCH, 20, 395, textScale, -1, 0xFFFFFF, 1);
            sprintf(EV_SCRATCH, "MAP %3d / %3d", mapUsed, mapCapacity);
            func_0019D4E8(EV_SCRATCH, 20, 410, textScale, -1, 0xFFFFFF, 1);
            sprintf(EV_SCRATCH, "EFF %3d / %3d", effUsed, effCapacity);
            func_0019D4E8(EV_SCRATCH, 20, 425, textScale, -1, 0xFFFFFF, 1);
            if (memLimitMB < memUsedMB + 2.0f) {
                double limitMB = 17.6875;
                memFormat = "MEM %2.2f / %2.2fMB";
                sprintf(EV_SCRATCH, memFormat, memUsedMB, limitMB);
                func_0019D4E8(EV_SCRATCH, 150, 380, textScale, 0xFF0000FF, 0xFFFFFF, 1);
            } else if (memLimitMB < memUsedMB + 5.0f) {
                double limitMB = 17.6875;
                {
                    const char *memLineFormat = "MEM %2.2f / %2.2fMB";
                    memBits = f64_to_dbits(memUsedMB);
                    sprintf(EV_SCRATCH, memLineFormat, dbits_to_f64(memBits), limitMB);
                }
                func_0019D4E8(EV_SCRATCH, 150, 380, textScale, 0xFF00FFFF, 0xFFFFFF, 1);
            } else {
                {
                    const char *memLineFormat = "MEM %2.2f / %2.2fMB";
                    memBits = f64_to_dbits(memUsedMB);
                    sprintf(EV_SCRATCH, memLineFormat, dbits_to_f64(memBits), 17.6875);
                }
                func_0019D4E8(EV_SCRATCH, 150, 380, textScale, -1, 0xFFFFFF, 1);
            }
            {
                float lineScale;

                sprintf(EV_SCRATCH, "    %2.2f / %2.2fMB", (double) memStat23C, (double) memUsedMB);
                lineScale = 1.2f;
                func_0019D4E8(EV_SCRATCH, 150, 395, lineScale, -1, 0xFFFFFF, 1);
                sprintf(EV_SCRATCH, "    %2.2f / %2.2fMB", memStat240, (double) memUsedMB);
                func_0019D4E8(EV_SCRATCH, 150, 410, lineScale, -1, 0xFFFFFF, 1);
                sprintf(EV_SCRATCH, "    %2.2f / %2.2fMB", memStat244, (double) memUsedMB);
                func_0019D4E8(EV_SCRATCH, 150, 425, lineScale, -1, 0xFFFFFF, 1);
            }
        }
    }
    if (xglCdDriveStatusLowBitSet() != 0) {
        memUsedMB = memStat248 + roundHalf;
    } else {
        memUsedMB = memStat23C + memStat240 + memStat244 + roundHalf;
    }
    if (memLimitMB < memUsedMB + 2.0f) {
        sprintf(EV_SCRATCH, "WARNING BREAK CF MAP MEM");
        func_0019D4E8(EV_SCRATCH, 100, 30, 1.2f, 0xFF0000FF, 0xFFFFFF, 1);
    } else if (memLimitMB < memUsedMB + 5.0f) {
        sprintf(EV_SCRATCH, "WARNING BREAK CF MEM");
        func_0019D4E8(EV_SCRATCH, 100, 30, 1.2f, 0xFF00FFFF, 0xFFFFFF, 1);
    }
}

void sceVif1PkCnt(void *packet, unsigned int count);
void sceVif1PkOpenDirectHLCode(void *packet, unsigned int count);
void sceVif1PkCloseDirectHLCode(void *packet);
void gifStateReset(void);
void gifStateAddReg(int gifAddress, u64 gifData);
void gifStateFlush(void);
void gameClearRuntimeFlag(void);
void func_002D3890(s32 flag);
void func_001B3120(s32 status, s32 flag);
void xglFontFlush(void);
extern s32 gEventViewerPaused;
void eventViewerFrameCallback(void *packet);

/* Writes one GIF packet that sets the scissor to 0..512 x 0..448, then leaves the
 * exchange: func_002D3890 while the exchange runs, else gameClearRuntimeFlag. */
void eventViewerFrameCallback(void *packet) {
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, 512, 0, 448));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
    if (gEventViewerPaused != 0 && gEventViewerExchangeSuspended != 2) {
        func_002D3890(1);
    } else {
        gameClearRuntimeFlag();
    }
    func_001B3120(xenoActorEventGetStatus224(), gEventViewerExchangeSuspended != 2);
    xglFontFlush();
}

extern s32 gEventViewerWindowTop;
s32 func_00185CB0(s32, s32);
s32 xglCdSetReadyFlag();
s32 gifStateResetTimings();

void eventViewerReset(void) {
    func_00185CB0(0, 0);
    do {

    } while (xglCdServiceIfPending() != 0);
    gEventViewerWindowTop = 0x37;
    gifStateResetTimings();
    XenoActorPoolInitDefault();
    xglCdSetReadyFlag();
    gEventViewerExchangeSuspended = 0;
    gEventViewerPaused = 0;
    gEventViewerReadItem = 0;
}

void stateExchangeOff();
void stateExchangeOn();
void stateExchangeOverrideSet();

void eventViewerRender(void) {
    s32 camera;
    s32 windowTop;
    xglRenderClearFrame();
    if (gEventViewerExchangeSuspended == 0) {
        stateExchangeOn();
    } else {
        stateExchangeOff();
    }
    stateExchangeOverrideSet();
    camera = xglStudioSelectGetActiveCamera(0);
    windowTop = gEventViewerWindowTop;
    xglCameraSetWindow(camera, 0, windowTop, 0x200, windowTop + 0x140);
}

void eventViewerNoop(void) {

}

s32 xenoActorEventIsReadyFlagSet(void);
void xenoActorEventAdvanceCounters(s32);
void xenoActorEventFrameUpdate(s32);
extern signed char gVsyncsPerFrame;
extern s32 D_00550C98;
s32 xglCdGetReadyStatus();

/* Polls the CD ready word once or more per frame (bit 0x2 stops the poll, bit 0x100
 * sets D_00550C98, 0x40 and readyMask 0x24 gate the extra polls) and advances the event
 * counters; gVsyncsPerFrame becomes 4 while the ready flag is set, else 2. */
void eventViewerPollReady(void) {
    s32 readyMask = 0x24;
    s32 i;

    D_00550C98 = 0;
    if (xglCdGetReadyStatus() & 2) {
        return;
    }
    xenoActorEventFrameUpdate(gEventViewerPaused);
    if (xglCdGetReadyStatus() & 0x100) {
        D_00550C98 = 1;
    }
    if (xenoActorEventIsReadyFlagSet()) {
        gVsyncsPerFrame = 4;
        xenoActorEventAdvanceCounters(gEventViewerPaused);
        if (xglCdGetReadyStatus() & 2) {
            return;
        }
        xenoActorEventFrameUpdate(gEventViewerPaused);
        if (xglCdGetReadyStatus() & 0x100) {
            D_00550C98 = 1;
        }
    } else {
        gVsyncsPerFrame = 2;
    }
    if (xglCdGetReadyStatus() & 0x40) {
        if (xglCdGetReadyStatus() & 2) {
            return;
        }
        xenoActorEventFrameUpdate(gEventViewerPaused);
        if (xglCdGetReadyStatus() & 0x100) {
            D_00550C98 = 1;
        }
        if (xenoActorEventIsReadyFlagSet()) {
            xenoActorEventAdvanceCounters(gEventViewerPaused);
            if (xglCdGetReadyStatus() & 2) {
                return;
            }
            xenoActorEventFrameUpdate(gEventViewerPaused);
            if (xglCdGetReadyStatus() & 0x100) {
                D_00550C98 = 1;
            }
        }
    }
    if ((xglCdGetReadyStatus() & readyMask) != readyMask) {
        return;
    }
    for (i = 0; i < 3; i++) {
        if (xglCdGetReadyStatus() & 2) {
            return;
        }
        xenoActorEventFrameUpdate(gEventViewerPaused);
        if (xglCdGetReadyStatus() & 0x100) {
            D_00550C98 = 1;
        }
    }
}

void XenoActorSetHoldFlag(void);
void XenoActorClearHoldFlag(void);
s32 seVoiceStartDriver(s32 voice);
void soundSlotSetPause(int soundSlotNumber, int level);
void func_00225B90(void);
void xglClockSampleDayTime(void);
void xglRenderIoEnd(void);
extern s32 D_00550C94;

/* Per-frame pause toggle: PAD pressed & 0x800 flips gEventViewerPaused between running (0) and
 * paused (1) with the sound pause; draws the render IO begin or end by that state. */
void eventViewerPauseToggle(void) {
    if (gEventViewerPaused != 0) {
        XenoActorSetHoldFlag();
        D_00550C94 = 1;
    } else {
        XenoActorClearHoldFlag();
        D_00550C94 = 0;
    }
    if (D_00550C98 == 0) {
        XenoActorSetHoldFlag();
    }
    if (gEventViewerPaused == 2) {
        gEventViewerPaused = 1;
    }
    if (PadData[0].pressed & 0x800) {
        seVoiceStartDriver(26);
        if (gEventViewerPaused == 1) {
            gEventViewerPaused = 0;
            func_00225B90();
            soundSlotSetPause(1, 0);
        } else {
            gEventViewerPaused = 1;
            xglClockSampleDayTime();
            soundSlotSetPause(1, 1);
        }
    }
    if (gEventViewerPaused == 0) {
        xglRenderIoBegin();
    } else {
        xglRenderIoEnd();
    }
}

void caption_rewind();
void xglCdInitReadRing();
void xenoActorEventTableInit();

/* Returns 1 when the CD is not reading; otherwise rewinds the caption and the read ring. */
s32 func_00245040(void) {
    if (xglCdGetReadyStatus() & 2) {
        if ((xglCdGetDriveStatus() & 2) == 0) {
            return 1;
        }
        caption_rewind();
        xglCdInitReadRing();
        xenoActorEventTableInit();
    }
    return 0;
}

extern XglRenderStateBlock D_004C8390;
s32 FlushCache(s32);
void xglDmaDirectNormal(u32, u32, u32);

/* One 64-bit GIF/GS word of a quadword in the scratchpad (value at [0], register at [1]). */
typedef union {
    u64 bits;
} GsDoubleword;

/* Sends a GIF DMA chain from the scratchpad: a tag, TEXFLUSH, a BITBLTBUF for the
 * work buffer, TRXPOS/TRXREG/TRXDIR, then the sync and the DMA at 0x70000000. */
void eventViewerSendBitbltChain(void) {
    GsDoubleword *gifTag;
    GsDoubleword *texFlushQword;
    GsDoubleword *bitbltQword;
    GsDoubleword *trxPosQword;
    GsDoubleword *trxRegQword;
    GsDoubleword *trxDirQword;
    u64 bitbltValue;

    gifTag = &SEF_SCRATCHPAD(GsDoubleword, 0x0);
    gifTag[1].bits = GIF_AD;
    texFlushQword = &SEF_SCRATCHPAD(GsDoubleword, 0x10);
    texFlushQword[1].bits = GS_TEXFLUSH;
    texFlushQword[0].bits = 0;
    bitbltValue = (((s32) D_004C8390.work_buffer_base << 5) | 0x80000) |
             ((u64) D_004C8390.display_buffer_base << 37) | (1ULL << 51) |
             ((u64) (s16) D_004C8390.psm << 24) | ((u64) (s16) D_004C8390.psm << 56);
    bitbltQword = &SEF_SCRATCHPAD(GsDoubleword, 0x20);
    bitbltQword[0].bits = bitbltValue;
    bitbltQword[1].bits = GS_BITBLTBUF;
    trxPosQword = &SEF_SCRATCHPAD(GsDoubleword, 0x30);
    trxPosQword[1].bits = GS_TRXPOS;
    trxPosQword[0].bits = 0;
    trxRegQword = &SEF_SCRATCHPAD(GsDoubleword, 0x40);
    trxRegQword[0].bits = GS_SET_TRXREG(0x200, 448);
    trxRegQword[1].bits = GS_TRXREG;
    trxDirQword = &SEF_SCRATCHPAD(GsDoubleword, 0x50);
    trxDirQword[0].bits = 2;
    trxDirQword[1].bits = GS_TRXDIR;
    gifTag[0].bits = 0x1000000000008005ULL;
    /* sync: wait until the GS bitbltValue stores above reach memory before the DMA reads them */
    __asm__ volatile("sync" ::: "memory");
    FlushCache(0);
    xglDmaDirectNormal(2, 0x70000000, 6);
}

extern unsigned char D_004C83F0[];

#define gStateFrameCallback (*(u8 **) D_004C83F0)

void func_001E7268(void);
void func_001B6798(s32);
void xglRenderLockSet(void);
void xglRenderHoldSet(void);
void xglRenderHoldClear(void);
void xglRenderLockClear(void);
void stateExchangeOverrideClear(void);
s32 func_0019AC80();

/* Leaves the event viewer: waits for the CD, releases the actors and resets the callbacks,
 * read buffers and studio state, then restores the lock, hold and exchange overrides. */
void eventViewerStop(void) {
    do {
    } while (xglCdServiceIfPending() != 0);
    xglSleep();
    XenoActorReleaseAll();
    xglCdSetReadyFlag();
    xglRenderClearFrame();
    gStateFrameCallback = 0;
    xglCameraSetWindow(xglStudioSelectGetActiveCamera(0), 0, 0, 0x200, 0x1C0);
    func_001E7268();
    /* kept: the cast calls gifStateResetTimings through a void pointer (the original shape). */
    ((void (*)()) gifStateResetTimings)();
    gVsyncsPerFrame = 2;
    eventViewerSilenceSound();
    func_001B6798(gEventViewerReadItem);
    gEventViewerReadItem = 0;
    gameClearRuntimeFlag();
    D_00550CA8 = 0x011B0000;
    gEventViewerTextBuffer = 0x00E00000;
    D_00550CB0 = 0x01FB0000;
    /* kept: the empty do/while(0) is needed by the match (removing it changes the code). */
    do {
    } while (0);
    xglFontFlush();
    xglRenderLockSet();
    xglRenderHoldSet();
    func_0019AC80(1);
    xglRenderHoldClear();
    xglRenderLockClear();
    stateExchangeOverrideClear();
    func_00185CB0(1, 0);
}

void eventViewerReset();
s32 xenoActorEventOpenFile(s32);

void eventViewerResetStart(s32 mode) {
    eventViewerReset();
    xenoActorEventOpenFile(mode);
}

s32 sefResetEffectSystem();
s32 sefSetSceneMode();
s32 sefResetEffectWorld();
s32 soundSlotSetVolume();

/* Event viewer start: disables func_0019AC80, installs eventViewerFrameCallback as the frame callback,
 * resets the effect world and system, sets scene mode 2 and the sound slot 1 volume. */
void eventViewerInit(void) {
    func_0019AC80(0);
    gStateFrameCallback = eventViewerFrameCallback;
    gifStateResetTimings();
    sefResetEffectWorld();
    sefResetEffectSystem();
    sefSetSceneMode(2);
    soundSlotSetVolume(1, 0x7F, 0);
}

s32 strcmp(void *, void *);
void func_0030C160(s32);
void sefFlagSetBit26(void);
void func_002911C0(void);
void func_002912B0(void);
void sefFrameUpdate(void);

const char eventViewerInitText[16] = "$init;";

/* Runs one event `arg`: starts it through strcmp when the drive is idle, else runs
 * the event loop (START+SELECT or the sound toggle ends it) and then the reset in eventViewerStop. */
void eventViewerRunEvent(void *eventName) {
    void *camera;
    s32 syncFlag;

    camera = xglStudioGetCamera2(0);
    func_001B6AB0();
    do {
        if ((xglCdGetDriveStatus() & 0x10) == 0 &&
            strcmp((void *) xenoActorEventGetBufferC(), eventName) != 0) {
            func_0030C160(1);
            sefFlagSetBit26();
        } else {
            syncFlag = 1;
            eventViewerInit();
            eventViewerBoot();
                    for (;;) {
                if ((PadData[0].heldAndPressed & 0x0800010CULL) == 0x0800010CULL) {
                    sefFlagSetBit26();
                    break;
                }
                if (gEventViewerPaused != 0 && (PadData[0].pressed & 0x10)) {
                    func_00225B90();
                    seVoiceStartDriver(2);
                    break;
                }
                eventViewerRender();
                eventViewerNoop();
                /* kept: the call passes camera although eventViewerPauseToggle takes no argument. */
                ((void (*)(void *camera)) eventViewerPauseToggle)(camera);
                eventViewerPollReady();
                func_002911C0();
                func_002912B0();
                if (D_00550C94 != syncFlag) {
                    sefFrameUpdate();
                }
                xglSleep();
                if (func_00245040() != 0) {
                    break;
                }
            }
        }
        eventViewerStop();
    } while (0);
    xglFontPrintDirect(eventViewerInitText);
    func_001B6AB0();
}

s32 gEventViewerReadItem = 0;
s32 gEventViewerExchangeSuspended = 1;
s32 gEventViewerPaused = 0;
s32 D_00550C94 = 0;
s32 D_00550C98 = 0;
s32 gEventViewerWindowTop = 0x37;
u32 D_00550CA8 = 0x011B0000;
char *gEventViewerTextBuffer = (char *) 0x00E00000;
s32 D_00550CB0 = 0x01FB0000;
