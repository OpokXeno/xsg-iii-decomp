#include "common.h"

typedef unsigned char u8;

typedef unsigned int u32;

extern u8 gFontColorScale;
extern u8 gFontAlphaScale;
extern int gFontColorIndex;


/* Scales the red, green and blue bytes of a packed RGBA32 color by gFontColorScale (1/128 units, saturated to 0xFF)
 * and keeps its alpha byte. */
int fontColorModulateRgb(unsigned int color) {
    int red;
    int green;
    int blue;
    unsigned long long packedColor;

    red = ((int) (color & 0xFF) * gFontColorScale) >> 7;
    green = ((int) ((color >> 8) & 0xFF) * gFontColorScale) >> 7;
    blue = ((int) ((color >> 0x10) & 0xFF) * gFontColorScale) >> 7;
    if (red >= 0x100) {
        red = 0xFF;
    }
    if (green >= 0x100) {
        green = 0xFF;
    }
    if (blue >= 0x100) {
        blue = 0xFF;
    }
    packedColor = (unsigned long long) (color & 0xFF000000) | (red & 0xFF);
    packedColor |= ((unsigned long long) (blue & 0xFF) << 0x10) | ((unsigned long long) (green & 0xFF) << 8);
    return (int) packedColor;
}


/* Scales the red, green and blue bytes by gFontColorScale and the alpha byte by gFontAlphaScale (1/128 units,
 * saturated to 0xFF). */
int fontColorModulateRgba(unsigned int color) {
    int red;
    int green;
    int blue;
    int alpha;
    unsigned long long packedColor;

    red = ((int) (color & 0xFF) * gFontColorScale) >> 7;
    green = ((int) ((color >> 8) & 0xFF) * gFontColorScale) >> 7;
    blue = ((int) ((color >> 0x10) & 0xFF) * gFontColorScale) >> 7;
    alpha = ((int) (color >> 0x18) * gFontAlphaScale) >> 7;
    if (red >= 0x100) {
        red = 0xFF;
    }
    if (green >= 0x100) {
        green = 0xFF;
    }
    if (blue >= 0x100) {
        blue = 0xFF;
    }
    if (alpha >= 0x100) {
        alpha = 0xFF;
    }
    packedColor = (unsigned long long) (alpha << 0x18);
    packedColor |= (red & 0xFF);
    packedColor |= ((unsigned long long) (blue & 0xFF) << 0x10) | ((unsigned long long) (green & 0xFF) << 8);
    return (int) packedColor;
}

/* Halves two packed RGBA32 colors and adds them per channel (alpha included). */
u32 colorBlendAverage(u32 colorA, u32 colorB) {
    return (((colorA & 0xFEFEFE) >> 1) + ((colorB & 0xFEFEFE) >> 1)) | ((((colorA >> 0x18) + (colorB >> 0x18)) >> 1) << 0x18);
}

/* Averages the RGB of two packed colors and sets their alpha byte to alpha. */
u32 colorBlendAverageAlpha(u32 colorA, u32 colorB, int alpha) {
    return (((colorA & 0xFEFEFE) >> 1) + ((colorB & 0xFEFEFE) >> 1)) | (alpha << 0x18);
}

/* Adds two packed colors channel by channel with saturation to 0xFF, then sets the alpha byte to alpha. */
u32 colorAddSaturated(u32 colorA, u32 colorB, int alpha) {
    u32 channelSum;
    u32 blended;
    u32 overflowMask;

    channelSum = (colorA & 0xFF) + (colorB & 0xFF);
    overflowMask = ((channelSum & 0x100) - (channelSum >> 8));
    blended = channelSum | overflowMask;
    channelSum = ((colorA >> 8) & 0xFF) + ((colorB >> 8) & 0xFF);
    overflowMask = ((channelSum & 0x100) - (channelSum >> 8));
    blended |= (channelSum | overflowMask) << 8;
    channelSum = ((colorA >> 0x10) & 0xFF) + ((colorB >> 0x10) & 0xFF);
    overflowMask = ((channelSum & 0x100) - (channelSum >> 8));
    blended |= (channelSum | overflowMask) << 0x10;
    return blended | (alpha << 0x18);
}

/* Multiplies two bytes (scaled by 1/128) and saturates the result to a byte. */
int colorModulateByte(int sourceChannel, int scaleByte) {
    int alphaProduct;

    alphaProduct = ((sourceChannel & 0xFF) * (scaleByte & 0xFF)) >> 7;
    if (alphaProduct >= 0x100) {
        alphaProduct = 0xFF;
    }
    return alphaProduct & 0xFF;
}

extern char *gFontEscapeBuffer;
void xglFontPrint(int x, int y, unsigned int color, const char *text);

/* Writes a color escape (0xC) with the scaled red, green and blue into the text buffer at
 * gFontEscapeBuffer and prints it through xglFontPrint at the origin. */
void xglFontSetColor(int red, int green, int blue) {
    int redProduct;
    int redScaled;
    int blueProduct;
    int blueScaled;
    int greenProduct;
    int greenScaled;

    gFontEscapeBuffer[0] = 0xC;
    redProduct = (red & 0xFF) * gFontColorScale;
    redScaled = redProduct >> 7;
    if (redScaled >= 0x100) {
        redScaled = 0xFF;
    }
    gFontEscapeBuffer[1] = redScaled;
    greenProduct = (green & 0xFF) * gFontColorScale;
    greenScaled = greenProduct >> 7;
    if (greenScaled >= 0x100) {
        greenScaled = 0xFF;
    }
    gFontEscapeBuffer[2] = greenScaled;
    blueProduct = (blue & 0xFF) * gFontColorScale;
    blueScaled = blueProduct >> 7;
    if (blueScaled >= 0x100) {
        blueScaled = 0xFF;
    }
    gFontEscapeBuffer[3] = blueScaled;
    gFontEscapeBuffer[4] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

/* Writes the 0x12 escape with the byte value at bytes 1 and 2 (stored at 2 first) into the text
 * buffer and prints it through xglFontPrint at the origin. */
void xglFontPrintEscapeByte(int escapeArgument)
{
    int escapeByte;

    gFontEscapeBuffer[0] = 0x12;
    escapeByte = escapeArgument & 0xFF;
    gFontEscapeBuffer[1] = gFontEscapeBuffer[2] = escapeByte;
    gFontEscapeBuffer[3] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

/* Writes the 0xD/0xE escape header with three scaled color bytes into the text buffer and prints
 * it through xglFontPrint at the origin. */
void xglFontPrintExtendedColor(int red, int green, int blue)
{
    int blueProduct;
    int blueScaled;
    int redProduct;
    int redScaled;
    int greenProduct;
    int greenScaled;

    gFontEscapeBuffer[0] = 0xD;
    gFontEscapeBuffer[1] = 2;
    gFontEscapeBuffer[2] = 0xE;
    gFontEscapeBuffer[3] = 2;
    gFontEscapeBuffer[4] = 2;
    redProduct = (red & 0xFF) * gFontColorScale;
    redScaled = redProduct >> 7;
    if (redScaled >= 0x100) {
        redScaled = 0xFF;
    }
    gFontEscapeBuffer[5] = redScaled;
    greenProduct = (green & 0xFF) * gFontColorScale;
    greenScaled = greenProduct >> 7;
    if (greenScaled >= 0x100) {
        greenScaled = 0xFF;
    }
    gFontEscapeBuffer[6] = greenScaled;
    blueProduct = (blue & 0xFF) * gFontColorScale;
    blueScaled = blueProduct >> 7;
    if (blueScaled >= 0x100) {
        blueScaled = 0xFF;
    }
    gFontEscapeBuffer[7] = blueScaled;
    gFontEscapeBuffer[8] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

/* Writes the 0xD/0xE escape with the byte value at bytes 3 and 4, the scaled red, green and blue, and a
 * terminator, then prints it through xglFontPrint at the origin. */
void xglFontPrintExtendedColorByte(int red, int green, int blue, int escapeByte) {
    int escapeValue;
    int redScaled;
    int greenScaled;
    int blueScaled;

    gFontEscapeBuffer[0] = 0xD;
    gFontEscapeBuffer[1] = 3;
    gFontEscapeBuffer[2] = 0xE;
    escapeValue = escapeByte & 0xFF;
    gFontEscapeBuffer[3] = escapeValue;
    gFontEscapeBuffer[4] = escapeValue;
    redScaled = ((red & 0xFF) * gFontColorScale) >> 7;
    if (redScaled >= 0x100) {
        redScaled = 0xFF;
    }
    gFontEscapeBuffer[5] = redScaled;
    greenScaled = ((green & 0xFF) * gFontColorScale) >> 7;
    if (greenScaled >= 0x100) {
        greenScaled = 0xFF;
    }
    gFontEscapeBuffer[6] = greenScaled;
    blueScaled = ((blue & 0xFF) * gFontColorScale) >> 7;
    if (blueScaled >= 0x100) {
        blueScaled = 0xFF;
    }
    gFontEscapeBuffer[7] = blueScaled;
    gFontEscapeBuffer[8] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

void xglFontPrintColorReset(void) {
    gFontEscapeBuffer[0] = 0xD;
    gFontEscapeBuffer[1] = 0;
    gFontEscapeBuffer[2] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

/* Writes the 0xF escape with two bytes and the alpha byte scaled by the alpha scale, then prints it. */
void xglFontPrintAlphaCommand(int alphaMode, int alphaIndex, int alpha) {
    int alphaProduct;
    int scaledAlpha;

    gFontEscapeBuffer[0] = 0xF;
    gFontEscapeBuffer[1] = alphaMode;
    gFontEscapeBuffer[2] = alphaIndex;
    alphaProduct = (alpha & 0xFF) * gFontAlphaScale;
    scaledAlpha = alphaProduct >> 7;
    if (scaledAlpha >= 0x100) {
        scaledAlpha = 0xFF;
    }
    gFontEscapeBuffer[3] = scaledAlpha;
    gFontEscapeBuffer[4] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

extern const unsigned char gFontTitleText[];

void xglFontPrintTitle(void) {
    xglFontPrint(0, 0, gFontColorIndex, gFontTitleText);
    xglFontPrintAlphaCommand(0x44, 0, 0x80);
}

extern const unsigned char gFontFormatString[];
int GameMpeg2Play();

void xglFontPrintLabel(int x, int y) {
    GameMpeg2Play(gFontEscapeBuffer, gFontFormatString);
    xglFontPrint(x, y, gFontColorIndex + 2, gFontEscapeBuffer);
}

extern char *gFontNumberBuffer;
int unsignedDecimalDigits();

void xglFontPrintUnsignedRight(int x, int y, int charWidth, int unsignedValue) {
    int advance;

    GameMpeg2Play(gFontNumberBuffer, gFontFormatString, unsignedValue);
    advance = unsignedDecimalDigits(unsignedValue);
    xglFontPrint(x - (charWidth * advance), y, gFontColorIndex + 2, gFontNumberBuffer);
}

/* Font command bytes and the format string (a 0 ends each), read-only. */
const unsigned char gFontTitleText[16] = {
    0x0B, 0x0D, 0x00, 0x19, 0x03, 0x0C, 0x62, 0x62, 0x62, 0x15, 0x02, 0x01, 0x18, 0x00, 0x00, 0x00,
};
const unsigned char gFontFormatString[8] = { 0x15, 0x02, 0x04, 0x01, 0x25, 0x75, 0x00, 0x00 };

/* Alpha and color scales (1/128 units) and the color index of the font printer. */
u8 gFontAlphaScale = 0x80;
u8 gFontColorScale = 0x80;
int gFontColorIndex = 1;
