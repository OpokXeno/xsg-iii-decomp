#include "common.h"
#include "shared.h"

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

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B24E8);

#include "main/tu007.h"

char *func_001B2368(char *cursor, int *frames);
int xglFontGetStringWidth(const char *text);
extern const char D_0053F588[];
extern const char D_0053F590[];
extern const char D_0053F598[];

char *caption_getparam(char *cursor, int *time, char *out)
{
    int frames;
    char ch;
    char *text;
    char *next;
    int width;

    if ((*cursor >= '0' && *cursor <= '9') || *cursor == '+') {
        frames = *time;
        next = func_001B2368(cursor, &frames);
        cursor = next;
        for (;;) {
            ch = *cursor++;
            out += 2;
            text = out;
            while (ch != '\\' && ch != '\r' && ch != '\n') {
                if (ch == '[') {
                    __builtin_memcpy(out, D_0053F588, 8);
                    while (*out != 0) {
                        out++;
                    }
                    ch = *cursor++;
                    while (ch != '/') {
                        *out++ = ch;
                        ch = *cursor++;
                    }
                    __builtin_memcpy(out, D_0053F590, 8);
                    while (*out != 0) {
                        out++;
                    }
                    ch = *cursor++;
                    while (ch != ']') {
                        *out++ = ch;
                        ch = *cursor++;
                    }
                    __builtin_memcpy(out, D_0053F598, 8);
                    while (*out != 0) {
                        out++;
                    }
                } else {
                    *out++ = ch;
                }
                ch = *cursor++;
            }
            *out = 0;
            width = xglFontGetStringWidth(text);
            text[-2] = 1;
            text[-1] = ~(width / 2);
            if (ch != '\\') {
                break;
            }
            *out++ = '\n';
            cursor++;
        }
    } else {
        frames = -1;
    }
    *time = frames;
    *out = 0;
    return caption_nextline(cursor);
}

char *caption_getparam(char *cursor, int *time, char *out);
extern unsigned char D_00771240[];
extern const char D_0053F5A0[];

int caption_convert(char *text)
{
    char *dst;
    char *src;
    char *next;
    int time;

    dst = text;
    D_00771240[0] = 0;
    xglFontGetStringWidth(D_0053F5A0);
    src = caption_nextline(text);
    time = 0;
    next = caption_getparam(src, &time, text);
    if (time == 0) {
        while (*dst++ != 0) {
        }
        src = next;
    } else {
        *text = 0;
        dst = text + 1;
    }
    time = 0;
    do {
        src = caption_getparam(src, &time, dst + 2);
        dst[0] = time;
        dst[1] = time >> 8;
        dst += 4;
        while (*dst++ != 0) {
        }
    } while (time >= 0);
    return dst - text;
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B28C0);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B2E18);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B2F60);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B30D8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3110);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3120);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3200);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B32F8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3510);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3548);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3670);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3858);

/* Quadword (128-bit) type of the DMA packet unions. */
typedef unsigned int XglQuadword __attribute__((mode(TI)));

typedef union GameOverDmaPacket4 {
    u64 words[4];
    XglQuadword quadwords[2];
} GameOverDmaPacket4;

typedef union GameOverDmaPacket6 {
    u64 words[6];
    XglQuadword quadwords[3];
} GameOverDmaPacket6;

/* An image of the synopsis screen and its size in pixels and in quadwords. */
typedef struct GameSynopsisPanel {
    void *image;
    int width;
    int height;
    int qwc;
} GameSynopsisPanel;

void DrawImage(void *framebuffer);
void GameSynopsisDrawPanel(GameSynopsisPanel *panel, int x, int y, int alpha);
extern XglRenderState sRender;
void xglSleep(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddDataN(XglPacket *packet, const void *data, unsigned int qwc);

typedef union GameOverDmaPacket12 {
    u64 words[12];
    XglQuadword quadwords[6];
} GameOverDmaPacket12;

void redraw_frame(void *framebuffer);
extern GameOverDmaPacket12 TestEnv_0_002BCD00;
extern GameOverDmaPacket4 TransEnv_1_002BCD60;
extern GameOverDmaPacket6 FlushEnv_2_002BCD80;

/* The 28-word VIF payload at 0x002BCDB0 that GameSynopsisDrawPanel sends first. */
typedef struct GameSynopsisPanelHeader {
    u8 unmodeled_00[0x40];
    u64 extent;                 /* +0x40: width in the low word, height in the high word */
    u8 unmodeled_48[0x18];
    int qwc;                    /* +0x60: the image's quadword count */
    u8 unmodeled_64[0x0C];
} GameSynopsisPanelHeader;

/* The 44-word VIF payload at 0x002BCE20 that GameSynopsisDrawPanel sends second. */
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

extern GameSynopsisPanelHeader gameSynopsisPanelHeaderPacket;
extern GameSynopsisPanelRect gameSynopsisPanelRectPacket;

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B3D50);

extern u16 D_004C83A6[];
extern GameOverDmaPacket12 D_004ADC20;
extern GameOverDmaPacket4 D_004ADC80;
extern GameOverDmaPacket6 D_004ADCA0;

void DrawImage(void *framebuffer)
{
    XglPacket *packet;
    u16 msb;

    if (framebuffer == 0) {
        return;
    }

    packet = xglPacketGetCurrent();
    /* BITBLTBUF: destination page buffer_select << 5, destination width 8
     * (512 pixels) in bits 48-53. */
    msb = 0x8000;
    D_004ADC20.words[4] = ((u64)D_004C83A6[0] << 37) | ((u64)msb << 36);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, D_004ADC20.words, 0x18);
    sceVif1PkRef(packet, D_004ADC80.words, 2, 0, 0, 0);
    sceVif1PkRef(packet, framebuffer, 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_004ADC80.words, 2, 0, 0, 0);
    sceVif1PkRef(packet, (u8 *)framebuffer + 0x70000, 0x7000, 0, 0x51007000, 0);
    sceVif1PkRef(packet, D_004ADCA0.words, 3, 0, 0, 0);
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

extern u16 D_004C839E[];

typedef union GameOverDmaPacket20 {
    u64 words[20];
    XglQuadword quadwords[10];
} GameOverDmaPacket20;

extern GameOverDmaPacket20 D_004ADDF0;
void FlushCache(int mode);

void DrawBack(int alpha)
{
    u64 nonnegative_alpha;
    u64 gs_frame_address_bits;

    nonnegative_alpha = alpha < 0 ? 0 : alpha;
    gs_frame_address_bits = 0xc800;
    D_004ADDF0.words[6] =
        (0x24120000 | (D_004C839E[0] << 5)) |
        (gs_frame_address_bits << 0x13);
    D_004ADDF0.words[8] = (nonnegative_alpha << 32) | 100;
    FlushCache(0);
    sceVif1PkRef(xglPacketGetCurrent(), D_004ADDF0.words, 10, 0, 0, 0);
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

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B45B8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4618);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B47C0);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4810);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4B88);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B4FA8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B5010);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B5038);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B50D8);

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B5618);

/* Pending wait state at D_00771260: the flag starts the wait, the panel is the synopsis image. */
typedef struct GameSynopsisWaitState {
    int pending;
    int unmodeled_04;
    int unmodeled_08;
    GameSynopsisPanel panel;
    u8 unmodeled_1C[0x04];
} GameSynopsisWaitState;

extern GameSynopsisWaitState D_00771260;
extern const char D_0053F8D0[];
extern PadRecord PadData[2];
int GameMpeg2Play(char *destination, const char *format, ...);
void seVoiceStartDefault(int);
void func_001B5010(char *buffer, int value);
void func_001BBEA8(void);

int GameSynopsisCrossfadeWait(void)
{
    char buffer[256];
    int alpha;

    alpha = 0;
    if (D_00771260.pending == 0) {
        return 0;
    }
    D_00771260.pending = 0;
    seVoiceStartDefault(4);
    for (;;) {
        GameSynopsisDrawPanel(&D_00771260.panel, 512 - D_00771260.panel.width, 448 - D_00771260.panel.height, 128);
        alpha += 4;
        if (alpha > 128) {
            alpha = 128;
        }
        GameMpeg2Play(buffer, D_0053F8D0, 128 - alpha);
        func_001B5010(buffer, D_00771260.unmodeled_04);
        GameMpeg2Play(buffer, D_0053F8D0, alpha);
        func_001B5010(buffer, D_00771260.unmodeled_08);
        if ((PadData[0].heldAndPressed & 0x800010c) == 0x800010c) {
            func_001BBEA8();
            return 1;
        }
        if ((PadData[0].pressed & 0x60) == 0) {
            xglSleep();
        } else {
            break;
        }
    }
    seVoiceStartDefault(2);
    GameSynopsisDrawPanel(&D_00771260.panel, 512 - D_00771260.panel.width, 448 - D_00771260.panel.height, 128);
    xglSleep();
    GameSynopsisDrawPanel(&D_00771260.panel, 512 - D_00771260.panel.width, 448 - D_00771260.panel.height, 128);
    xglSleep();
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/game_movie", func_001B5970);
