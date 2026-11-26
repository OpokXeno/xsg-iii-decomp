#include "common.h"
#include "shared.h"
#include "main/xgl_debug_file_004.h"

char *caption_nextline(char *cursor);
struct XglPacket;

#include "main/Packet.h"
#include "main/xgl_render.h"

XglPacket *xglPacketGetCurrent(void);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4,
                  unsigned int a5);
void pauseRefBackdrop(void);
/* 8 quadwords of direct GIF/VIF data: the pause backdrop state added to the packet. */
extern u8 pauseBackdropPacket[128];

void pauseRefBackdrop(void)
{
    sceVif1PkRef(xglPacketGetCurrent(), pauseBackdropPacket, 8, 0, 0, 0);
}

char *caption_nextline(char *cursor)
{
    char character;

    do {
        if (*cursor != '\r' && *cursor != '\n') {
            if (*cursor != ';') {
                break;
            }
            cursor++;
        }
        for (;;) {
            character = *cursor++;
            if (character == '\r' || character == '\n') {
                if (*cursor != '\r' && *cursor != '\n' && *cursor != ';') {
                    break;
                }
            }
        }
    } while (0);
    return cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B2368);

typedef signed char s8;
typedef int s32;
typedef unsigned int XglQuadword __attribute__((mode(TI)));
typedef union ImageSyncPacket {
    u64 doublewords[4];
    XglQuadword quadwords[2];
} ImageSyncPacket;
typedef union ImageFlushPacket {
    u64 doublewords[6];
    XglQuadword quadwords[3];
} ImageFlushPacket;
typedef struct GameSynopsisPanel {
    void *image;
    int width;
    int height;
    int qwc;
} GameSynopsisPanel;
typedef union ImageBitbltPacket {
    u64 doublewords[12];
    XglQuadword quadwords[6];
} ImageBitbltPacket;
typedef struct GameSynopsisPanelHeader {
    u8 unmodeled_00[0x40];
    u64 extent;                 /* +0x40: width in the low word, height in the high word */
    u8 unmodeled_48[0x18];
    int qwc;                    /* +0x60: the image's quadword count */
    u8 unmodeled_64[0x0C];
} GameSynopsisPanelHeader;
typedef struct GameSynopsisPanelRect {
    u8 unmodeled_00[0x6C];
    int alpha;                  /* +0x6C */
    u8 unmodeled_70[0x10];
    int top_left_x;             /* +0x80 */
    int top_left_y;             /* +0x84 */
    u8 unmodeled_88[0x08];
    int width;                  /* +0x90 */
    int height;                 /* +0x94 */
    u8 unmodeled_98[0x08];
    int bottom_right_x;         /* +0xA0 */
    int bottom_right_y;         /* +0xA4 */
    u8 unmodeled_A8[0x08];
} GameSynopsisPanelRect;
typedef union BackdropPacket {
    u64 doublewords[20];
    XglQuadword quadwords[10];
} BackdropPacket;
typedef struct GameSynopsisWaitState {
    int pending;
    int crossfadeOutValue;
    int crossfadeInValue;
    GameSynopsisPanel panel;
    u8 unmodeled_1C[0x04];
} GameSynopsisWaitState;

char *func_001B2368(char *cursor, int *frames);

s8 *caption_parseFrameArg(s8 *cursor, s32 *frames) {
    if (*cursor != 0x2C) {
        return func_001B2368(cursor, frames);
    }
    return cursor + 1;
}

#include "main/tu007.h"

int xglFontGetStringWidth(const char *text);
extern const char captionTagStart[];
extern const char captionTagSeparator[];
extern const char captionTagEnd[];

char *caption_getparam(char *cursor, int *captionFrames, char *outCursor)
{
    int frames;
    char ch;
    char *textStart;
    char *afterFrameArg;
    int width;

    if ((*cursor >= '0' && *cursor <= '9') || *cursor == '+') {
        frames = *captionFrames;
        afterFrameArg = func_001B2368(cursor, &frames);
        cursor = afterFrameArg;
        for (;;) {
            ch = *cursor++;
            outCursor += 2;
            textStart = outCursor;
            while (ch != '\\' && ch != '\r' && ch != '\n') {
                if (ch == '[') {
                    __builtin_memcpy(outCursor, captionTagStart, 8);
                    while (*outCursor != 0) {
                        outCursor++;
                    }
                    ch = *cursor++;
                    while (ch != '/') {
                        *outCursor++ = ch;
                        ch = *cursor++;
                    }
                    __builtin_memcpy(outCursor, captionTagSeparator, 8);
                    while (*outCursor != 0) {
                        outCursor++;
                    }
                    ch = *cursor++;
                    while (ch != ']') {
                        *outCursor++ = ch;
                        ch = *cursor++;
                    }
                    __builtin_memcpy(outCursor, captionTagEnd, 8);
                    while (*outCursor != 0) {
                        outCursor++;
                    }
                } else {
                    *outCursor++ = ch;
                }
                ch = *cursor++;
            }
            *outCursor = 0;
            width = xglFontGetStringWidth(textStart);
            textStart[-2] = 1;
            textStart[-1] = ~(width / 2);
            if (ch != '\\') {
                break;
            }
            *outCursor++ = '\n';
            cursor++;
        }
    } else {
        frames = -1;
    }
    *captionFrames = frames;
    *outCursor = 0;
    return caption_nextline(cursor);
}

char *caption_getparam(char *cursor, int *captionFrames, char *outCursor);
extern unsigned char captionPlayMode[];
extern const char D_0053F5A0[];

int caption_convert(char *text)
{
    char *writeCursor;
    char *readCursor;
    char *nextLine;
    int captionFrames;

    writeCursor = text;
    captionPlayMode[0] = 0;
    xglFontGetStringWidth(D_0053F5A0);
    readCursor = caption_nextline(text);
    captionFrames = 0;
    nextLine = caption_getparam(readCursor, &captionFrames, text);
    if (captionFrames == 0) {
        while (*writeCursor++ != 0) {
        }
        readCursor = nextLine;
    } else {
        *text = 0;
        writeCursor = text + 1;
    }
    captionFrames = 0;
    do {
        readCursor = caption_getparam(readCursor, &captionFrames, writeCursor + 2);
        writeCursor[0] = captionFrames;
        writeCursor[1] = captionFrames >> 8;
        writeCursor += 4;
        while (*writeCursor++ != 0) {
        }
    } while (captionFrames >= 0);
    return writeCursor - text;
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B28C0);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B2E18);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B2F60);

extern s8 *captionTextBegin;
extern s8 *captionTextCursor;

void caption_begin(s8 *text)
{
    if (text != 0) {
        captionTextBegin = text;
        captionTextCursor = text;
        caption_convert(text);
    }
}

void caption_rewind(void) {
    captionTextCursor = captionTextBegin;
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3120);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3200);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B32F8);

s32 func_001B3120();
s32 func_001B32F8();

void captionPlayIndex(s32 captionIndex) {
    if (captionPlayMode[0] == 1) {
        func_001B32F8(captionIndex);
        return;
    }
    func_001B3120(captionIndex, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3548);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3670);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3858);












/* An image of the synopsis screen and its size in pixels and in quadwords. */







void DrawImage(u8 *framebuffer);
void GameSynopsisDrawPanel(GameSynopsisPanel *panel, int x, int y, int alpha);
extern XglRenderState sRender;
void xglSleep(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddDataN(XglPacket *packet, const void *data, unsigned int qwc);






void redraw_frame(void *framebuffer);
extern ImageBitbltPacket TestEnv_0_002BCD00;
extern ImageSyncPacket TransEnv_1_002BCD60;
extern ImageFlushPacket FlushEnv_2_002BCD80;

/* The 28-word VIF payload at 0x002BCDB0 that GameSynopsisDrawPanel sends first. */








/* The 44-word VIF payload at 0x002BCE20 that GameSynopsisDrawPanel sends second. */















extern GameSynopsisPanelHeader gameSynopsisPanelHeaderPacket;
extern GameSynopsisPanelRect gameSynopsisPanelRectPacket;

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3D50);

extern u16 movieFramebufferBasePage[];
extern ImageBitbltPacket movieImageBitbltPacket;
extern ImageSyncPacket movieImageSyncPacket;
extern ImageFlushPacket movieImageFlushPacket;

void DrawImage(u8 *framebuffer)
{
    XglPacket *packet;
    u64 msb;

    if (framebuffer == 0) {
        return;
    }

    packet = xglPacketGetCurrent();
    /* BITBLTBUF: destination page buffer_select << 5, destination width 8
     * (512 pixels) in bits 48-53. */
    msb = 0x8000;
    movieImageBitbltPacket.doublewords[4] = ((u64)movieFramebufferBasePage[0] << 37) | (msb << 36);

    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, movieImageBitbltPacket.doublewords, 0x18);
    sceVif1PkRef(packet, movieImageSyncPacket.doublewords, 2, 0, 0, 0);
    sceVif1PkRef(packet, framebuffer, 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, movieImageSyncPacket.doublewords, 2, 0, 0, 0);
    sceVif1PkRef(packet, &framebuffer[0x70000], 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, movieImageFlushPacket.doublewords, 3, 0, 0, 0);
}

void GameSynopsisDrawPanel(GameSynopsisPanel *panel, int x, int y, int alpha)
{
    XglPacket *packet;
    u64 extent;
    int origin_x;
    int origin_y;

    if (panel->image == 0) {
        return;
    }
    if (alpha == 0) {
        return;
    }
    packet = xglPacketGetCurrent();
    origin_x = 0x6FF8;
    origin_y = 0x71F7;
    extent = (u64)panel->width | ((u64)panel->height << 32);
    gameSynopsisPanelHeaderPacket.qwc = panel->qwc;
    gameSynopsisPanelRectPacket.alpha = alpha;
    gameSynopsisPanelHeaderPacket.extent = extent;
    gameSynopsisPanelRectPacket.width = panel->width * 16;
    gameSynopsisPanelRectPacket.height = panel->height * 16;
    gameSynopsisPanelRectPacket.top_left_x = x * 16 + origin_x;
    gameSynopsisPanelRectPacket.top_left_y = y * 16 + origin_y;
    gameSynopsisPanelRectPacket.bottom_right_x = (x + panel->width) * 16 + origin_x;
    gameSynopsisPanelRectPacket.bottom_right_y = (y + panel->height) * 16 + origin_y;
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, &gameSynopsisPanelHeaderPacket, 28);
    sceVif1PkRef(packet, panel->image, panel->qwc, 0, 0x51000000 | panel->qwc, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, &gameSynopsisPanelRectPacket, 44);
}

extern XglRenderStateBlock D_004C8390;






extern BackdropPacket movieBackdropPacket;
void FlushCache(int mode);

void DrawBack(int alpha)
{
    u64 nonnegative_alpha;
    u64 gs_frame_address_bits;

    nonnegative_alpha = alpha < 0 ? 0 : alpha;
    gs_frame_address_bits = 0xc800;
    movieBackdropPacket.doublewords[6] =
        (0x24120000 | (D_004C8390.work_buffer_base << 5)) |
        (gs_frame_address_bits << 0x13);
    movieBackdropPacket.doublewords[8] = (nonnegative_alpha << 32) | 100;
    FlushCache(0);
    sceVif1PkRef(xglPacketGetCurrent(), movieBackdropPacket.doublewords, 10, 0, 0, 0);
}

void redraw_frame(void *framebuffer) {
    xglSleep();
    DrawImage(framebuffer);
    xglSleep();
    DrawImage(framebuffer);
    xglSleep();
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B41E8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B43C8);

extern unsigned char movieCdReadBuffer[];
s32 func_001B43C8();
s32 xglCdReadFile();

void movieCdLoadAndStart(s32 location, s32 mode) {
    xglGetLanguageCode(movieCdReadBuffer);
    xglCdReadFile(movieCdReadBuffer - 4, location, 0, 0);
    func_001B43C8(location, mode);
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4618);

extern unsigned char movieTextHeaderString[];
s32 xglFontPrintDirect();

void movieTextPrintEntry(u8 **textTable, s32 textIndex) {
    xglFontPrintDirect(movieTextHeaderString);
    xglFontPrintDirect(textTable[10]);
    xglFontPrintDirect(textTable[textIndex]);
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4810);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4B88);

void fontPrintOptionalTriple(void *prefix, char *text, s32 suffix) {
    if (prefix != 0) {
        xglFontPrintDirect();
    }
    if (text != 0) {
        xglFontPrintDirect(text);
    }
    if (suffix != 0) {
        xglFontPrintDirect(suffix);
    }
}

extern char movieLabelPrefix[];
void fontPrintOptionalTriple();
void moviePrintLabelValue(char *labelText, int labelValue);

void moviePrintLabelValue(char *labelText, int labelValue) {
    fontPrintOptionalTriple(movieLabelPrefix, labelText, labelValue);
}

extern char movieStatusTitle[];
extern char D_0053F750[];
extern char D_0053F758[];
extern char D_0053F760[];
extern char D_0053F768[];
extern char D_0053F770[];

void moviePrintStatusRows(void *prefix, s32 statusRowValue) {
    moviePrintLabelValue(movieStatusTitle, 0);
    fontPrintOptionalTriple(prefix, D_0053F750, statusRowValue);
    fontPrintOptionalTriple(prefix, D_0053F758, statusRowValue);
    fontPrintOptionalTriple(prefix, D_0053F760, statusRowValue);
    fontPrintOptionalTriple(prefix, D_0053F768, statusRowValue);
    fontPrintOptionalTriple(prefix, D_0053F770, statusRowValue);
}


INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B50D8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B5618);

/* Pending wait state at synopsisCrossfadeState: the flag starts the wait, the panel is the synopsis image. */








extern GameSynopsisWaitState synopsisCrossfadeState;
extern const char synopsisAlphaLabelFormat[];
extern PadRecord PadData[2];
int sprintf(char *destination, const char *format, ...);
void seVoiceStartDefault(unsigned int handle);
void sefFlagSetBit26(void);

int GameSynopsisCrossfadeWait(void)
{
    char labelBuffer[256];
    int alpha;

    alpha = 0;
    if (synopsisCrossfadeState.pending == 0) {
        return 0;
    }
    synopsisCrossfadeState.pending = 0;
    seVoiceStartDefault(4);
    for (;;) {
        GameSynopsisDrawPanel(&synopsisCrossfadeState.panel, 512 - synopsisCrossfadeState.panel.width, 448 - synopsisCrossfadeState.panel.height, 128);
        alpha += 4;
        if (alpha > 128) {
            alpha = 128;
        }
        sprintf(labelBuffer, synopsisAlphaLabelFormat, 128 - alpha);
        moviePrintLabelValue(labelBuffer, synopsisCrossfadeState.crossfadeOutValue);
        sprintf(labelBuffer, synopsisAlphaLabelFormat, alpha);
        moviePrintLabelValue(labelBuffer, synopsisCrossfadeState.crossfadeInValue);
        if ((PadData[0].heldAndPressed & 0x800010c) == 0x800010c) {
            sefFlagSetBit26();
            return 1;
        }
        if ((PadData[0].pressed & 0x60) == 0) {
            xglSleep();
        } else {
            break;
        }
    }
    seVoiceStartDefault(2);
    GameSynopsisDrawPanel(&synopsisCrossfadeState.panel, 512 - synopsisCrossfadeState.panel.width, 448 - synopsisCrossfadeState.panel.height, 128);
    xglSleep();
    GameSynopsisDrawPanel(&synopsisCrossfadeState.panel, 512 - synopsisCrossfadeState.panel.width, 448 - synopsisCrossfadeState.panel.height, 128);
    xglSleep();
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B5970);
