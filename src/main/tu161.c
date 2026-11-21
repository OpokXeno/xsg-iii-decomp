#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu161", func_00214060);

INCLUDE_ASM("asm/main/nonmatchings/tu161", func_002140F0);

typedef unsigned int u32;
typedef unsigned char u8;

/* Halves two packed RGBA32 colors and adds them per channel (alpha included). */
u32 colorBlendAverage(u32 colorA, u32 colorB) {
    return (((colorA & 0xFEFEFE) >> 1) + ((colorB & 0xFEFEFE) >> 1)) | ((((colorA >> 0x18) + (colorB >> 0x18)) >> 1) << 0x18);
}

/* Averages the RGB of two packed colors and sets their alpha byte to alpha. */
u32 colorBlendAverageAlpha(u32 colorA, u32 colorB, int alpha) {
    return (((colorA & 0xFEFEFE) >> 1) + ((colorB & 0xFEFEFE) >> 1)) | (alpha << 0x18);
}

INCLUDE_ASM("asm/main/nonmatchings/tu161", func_002141F0);

/* Multiplies two bytes (scaled by 1/128) and saturates the result to a byte. */
int colorModulateByte(int sourceChannel, int scaleByte) {
    int product;

    product = ((sourceChannel & 0xFF) * (scaleByte & 0xFF)) >> 7;
    if (product >= 0x100) {
        product = 0xFF;
    }
    return product & 0xFF;
}

extern char *gFontEscapeBuffer;
extern u8 gFontColorScale;
extern int gFontColorIndex;
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
    int greenShifted;
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
    greenShifted = (greenScaled = greenProduct >> 7);
    if (greenShifted >= 0x100) {
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

INCLUDE_ASM("asm/main/nonmatchings/tu161", func_00214448);

void xglFontPrintColorReset(void) {
    gFontEscapeBuffer[0] = 0xD;
    gFontEscapeBuffer[1] = 0;
    gFontEscapeBuffer[2] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

extern u8 gFontAlphaScale;

/* Writes the 0xF escape with two bytes and the alpha byte scaled by the alpha scale, then prints it. */
void xglFontPrintAlphaCommand(int alphaMode, int alphaIndex, int alpha) {
    int product;
    int scaled;

    gFontEscapeBuffer[0] = 0xF;
    gFontEscapeBuffer[1] = alphaMode;
    gFontEscapeBuffer[2] = alphaIndex;
    product = (alpha & 0xFF) * gFontAlphaScale;
    scaled = product >> 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    gFontEscapeBuffer[3] = scaled;
    gFontEscapeBuffer[4] = 0;
    xglFontPrint(0, 0, gFontColorIndex, gFontEscapeBuffer);
}

extern unsigned char gFontTitleText[];

void xglFontPrintTitle(void) {
    xglFontPrint(0, 0, gFontColorIndex, gFontTitleText);
    xglFontPrintAlphaCommand(0x44, 0, 0x80);
}

extern unsigned char gFontFormatString[];
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
