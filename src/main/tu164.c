#include "common.h"

typedef signed char s8;

typedef int s32;

extern void *xglCdPreloadBuffer;
extern s8 xglCdPreloadState;
s32 xglCdGetAlignedFileSize(s32 *cdFileInfo);
s32 func_001B6F18(s32 *cdFileInfo, s32 flags);
void *memPoolAlloc(s32 size);
s32 completionRequestStart(void *preloadBuffer, s32 flags, s32 size);

void xglCdPreloadStart(void) {
    s32 cdFileInfo[0x20];
    s32 alignedSize;
    void *preloadBuffer;

    if ((xglCdPreloadState <= 0) && (func_001B6F18(&cdFileInfo[0], 0x09010005) != 0)) {
        alignedSize = xglCdGetAlignedFileSize(&cdFileInfo[0]);
        if (alignedSize >= 0) {
            preloadBuffer = memPoolAlloc(alignedSize);
            xglCdPreloadBuffer = preloadBuffer;
            if ((preloadBuffer != 0) && (completionRequestStart(preloadBuffer, 0x09010005, 0x1000) != 0)) {
                xglCdPreloadState = 1;
            }
        }
    }
}

s32 memPoolFree(void *block);
s32 completionFlagsTest(s32);
s32 statusBlockFrameUpdate();

void xglCdPreloadRelease(void) {
    if (xglCdPreloadState == 1) {
        while (completionFlagsTest(0x1000) != 0) {
            statusBlockFrameUpdate();
        }
    }
    if (xglCdPreloadBuffer != 0) {
        memPoolFree(xglCdPreloadBuffer);
        xglCdPreloadBuffer = 0;
    }
    xglCdPreloadState = 0;
}

/* Preload poll: while the preload is in flight (state 1) and its completion flag is still set this returns 0; once the flag
 * clears it moves the state to 2 and returns 1. Any other state returns whether the preload is done (state 2 or more). */
s32 xglCdPreloadPoll(void) {
    if (xglCdPreloadState == 1) {
        if (completionFlagsTest(0x1000) == 0) {
            xglCdPreloadState = 2;
            return 1;
        }
        return 0;
    }
    return xglCdPreloadState >= 2;
}

s32 fontQueueOutline(s32, s32, s32, s32);

void xglDrawCornerLines(s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 cornerY;
    s32 cornerX;
    s32 bottomY;

    cornerY = startY & 0xFFFF;
    cornerX = endX & 0xFFFF;
    bottomY = endY & 0xFFFF;
    fontQueueOutline(startX & 0xFFFF, cornerY, cornerX, cornerY);
    fontQueueOutline(cornerX, cornerY, cornerX, bottomY);
}

struct Pos16 {
    unsigned short x;
    unsigned short y;
};

struct Box16 {
    struct Pos16 origin;
    struct Pos16 end;
};

void fontQueueColorState(s32 value);
void xglFontDrawBoxBlend(struct Box16 *box, s32 mode, s32 a2, s32 a3);
void fontQueueSprite(s32 x0, s32 y0, s32 x1, s32 y1);
void xglFontQueueFlaggedRow(s32 x, s32 y, void *image);
s32 xglFontGetFormattedWidth(const char *text);
void xglFontPrintIndexed(s32 x, s32 y, const char *text);
void xglFontPrintTitle(void);
void xglFontPrintEscapeByte(s32 value);
void xglFontPrintExtendedColor(s32 r, s32 g, s32 b);
void xglFontSetColor(s32 r, s32 g, s32 b);

/* Draws one titled card at origin: the title (and the icon, when there is one) above the subtitle line. The card is a
 * 1952 x 752 box, and its text is centred from the measured width of each string. */
void loadingCardDraw(struct Pos16 *origin, const char *title, const char *subtitle, void *icon) {
    struct Box16 box;
    s32 textWidth;
    s32 textX;

    box.origin = *origin;
    box.end.x = origin->x + 1952;
    box.end.y = origin->y + 752;
    fontQueueColorState(0x73221E1D);
    xglFontDrawBoxBlend(&box, 2, 0, 0);
    fontQueueColorState(0x733A2827);
    fontQueueSprite((origin->x + 48) & 0xFFFF, (origin->y + 64) & 0xFFFF, (origin->x + 1920) & 0xFFFF,
                  (origin->y + 352) & 0xFFFF);
    xglFontPrintTitle();
    xglFontPrintEscapeByte(14);
    xglFontPrintExtendedColor(0, 0, 0);
    xglFontSetColor(52, 89, 84);
    if (title != 0) {
        textWidth = xglFontGetFormattedWidth(title);
        textX = ((origin->x - 28664) >> 4) - ((textWidth * 7) >> 4);
        xglFontPrintIndexed(textX + 61, ((origin->y - 29176) >> 4) + 2, title);
        if (icon != 0) {
            fontQueueColorState(0x80808080);
            xglFontQueueFlaggedRow(((textX << 4) + 29352) & 0xFFF8, (origin->y + 80) & 0xFFFF, icon);
        }
    }
    xglFontSetColor(98, 98, 98);
    if (subtitle != 0) {
        textWidth = xglFontGetFormattedWidth(subtitle);
        xglFontPrintIndexed(((origin->x - 28664) >> 4) - ((textWidth * 7) >> 4) + 61,
                            ((origin->y - 29176) >> 4) + 23, subtitle);
    }
}

void func_00185A20(void);
void xglFontQueueTextureBind(void *buffer);
void fontQueueDepthTest(s32 value);
void xglFontQueueGlyphRect(unsigned short *frame);
void xglFontFreeBuffer(void);
void *messageTextGet(s32 id);
s32 xglFontGetAlphaScale(void);
void xglFontColorIndexAdd(s32 index);

typedef struct LoadingFrame {
    unsigned short x;
    unsigned short y;
    unsigned short unk_04;
    unsigned short unk_06;
    unsigned short unk_08;
    unsigned short unk_0A;
    unsigned short unk_0C;
} LoadingFrame;

/* Loading screen: draws the preload image (when there is one), the frame and the corner lines, then the eight cards of
 * text ids 0x173-0x179 (the message table of messageTextGet). Nothing is drawn unless the preload is done or the font fades in. */
void loadingScreenDraw(void) {
    LoadingFrame frame;
    struct Pos16 pos;
    void *image;

    if (xglCdPreloadState != 2 && xglFontGetAlphaScale() == 0) {
        return;
    }
    image = xglCdPreloadBuffer;
    func_00185A20();
    if (image != 0) {
        xglFontQueueTextureBind(image);
    }
    fontQueueDepthTest(0);
    xglFontColorIndexAdd(2);
    fontQueueColorState(0x80808080);
    frame.x = 0x78A8;
    frame.y = 0x7A18;
    frame.unk_04 = 0;
    frame.unk_06 = 0;
    frame.unk_08 = 0x1000;
    frame.unk_0A = 0xBB0;
    frame.unk_0C = 0x308;
    xglFontQueueGlyphRect(&frame.x);
    xglFontFreeBuffer();
    fontQueueDepthTest(1);
    fontQueueColorState(0x80EC9B9A);
    xglDrawCornerLines(0x7898, 0x7778, 0x7C38, 0x7B08);
    xglDrawCornerLines(0x7898, 0x7A68, 0x7B88, 0x7BD8);
    xglDrawCornerLines(0x7898, 0x8048, 0x7BD8, 0x7FB8);
    xglDrawCornerLines(0x7898, 0x8628, 0x7DC8, 0x81E8);
    xglDrawCornerLines(0x7898, 0x8918, 0x8208, 0x81E8);
    xglDrawCornerLines(0x8758, 0x7778, 0x8378, 0x7B08);
    xglDrawCornerLines(0x8758, 0x7A68, 0x8438, 0x7BD8);
    xglDrawCornerLines(0x8758, 0x84A8, 0x8408, 0x7FC8);
    fontQueueDepthTest(0);
    xglFontColorIndexAdd(2);
    pos.x = 0x70F8;
    pos.y = 0x75F8;
    loadingCardDraw(&pos, messageTextGet(0x173), messageTextGet(0x23), 0);
    pos.y = 0x78E8;
    loadingCardDraw(&pos, messageTextGet(0x172), messageTextGet(0x22), 0);
    pos.y = 0x7EC8;
    loadingCardDraw(&pos, messageTextGet(0x176), messageTextGet(0x27), 0);
    pos.y = 0x84A8;
    loadingCardDraw(&pos, messageTextGet(0x177), messageTextGet(0x27), 0);
    pos.y = 0x8798;
    loadingCardDraw(&pos, messageTextGet(0x178), messageTextGet(0x28), 0);
    pos.x = 0x8758;
    pos.y = 0x75F8;
    loadingCardDraw(&pos, messageTextGet(0x175), messageTextGet(0x23), 0);
    pos.y = 0x78E8;
    loadingCardDraw(&pos, messageTextGet(0x174), messageTextGet(0x22), 0);
    pos.y = 0x7EC8;
    loadingCardDraw(&pos, messageTextGet(0x179), messageTextGet(0x1E), (void *) 0x20);
    pos.y = 0x81B8;
    loadingCardDraw(&pos, messageTextGet(0x179), messageTextGet(0x1F), (void *) 0x40);
    pos.y = 0x84A8;
    loadingCardDraw(&pos, messageTextGet(0x179), messageTextGet(0x20), (void *) 0x10);
    pos.y = 0x8798;
    loadingCardDraw(&pos, messageTextGet(0x179), messageTextGet(0x29), (void *) 0x80);
}
