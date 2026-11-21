#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_002146C0);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214748);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_002147D0);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214878);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214900);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214A00);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214AF0);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214B68);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214BC8);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214CB0);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214D10);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214DC8);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214E78);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00214FA8);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215060);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215250);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215440);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215570);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_002157B0);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215A50);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215BC0);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215E60);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00215FD0);

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;
typedef unsigned int u32;
void func_00215FD0(s32 rect, s32 level, s32 table);

void xglFontDrawDefault(s32 rect, s32 level) {
    func_00215FD0(rect, level, 0xA8B234);
}

extern u8 gFontAlphaScale;

void xglFontSetAlphaScale(s8 alphaScale) {
    gFontAlphaScale = alphaScale;
}

u8 xglFontGetAlphaScale(void) {
    return gFontAlphaScale;
}

s32 xglFontScaleAlpha(s32 alpha) {
    s32 scaled;

    scaled = ((alpha & 0xFF) * gFontAlphaScale) >> 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    return scaled & 0xFF;
}

s32 xglFontScaleColorAlpha(u32 color) {
    s32 scaled;

    scaled = (color >> 0x18) * gFontAlphaScale;
    scaled >>= 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    return (color & 0xFFFFFF) | (scaled << 0x18);
}

extern u8 gFontColorScale;

void xglFontSetColorScale(s8 colorScale) {
    gFontColorScale = colorScale;
}

u8 xglFontGetColorScale(void) {
    return gFontColorScale;
}

s32 xglFontScaleColorChannel(s32 channelValue) {
    s32 scaled;

    scaled = ((channelValue & 0xFF) * gFontColorScale) >> 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    return scaled & 0xFF;
}

extern s32 gFontColorIndex;

void xglFontSetColorIndex(s32 colorIndex) {
    gFontColorIndex = colorIndex;
}

void xglFontColorIndexInc(void) {
    gFontColorIndex += 1;
}

void xglFontColorIndexDec(void) {
    gFontColorIndex -= 1;
}

void xglFontColorIndexAdd(s32 delta) {
    gFontColorIndex += delta;
}

s32 xglFontGetColorIndex(void) {
    return gFontColorIndex;
}

void xglFontPrint(s32 x, s32 y, s32 color, s32 text);

void xglFontPrintIndexed(s32 x, s32 y, s32 text) {
    xglFontPrint(x, y, gFontColorIndex + 2, text);
}

extern s32 gFontStateMask;

s32 xglFontGetStateMask(void) {
    return gFontStateMask;
}

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00216478);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_002164C8);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_002165E8);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_002166F8);

INCLUDE_ASM("asm/main/nonmatchings/tu162", func_00216808);
