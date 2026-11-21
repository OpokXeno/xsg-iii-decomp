#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6978);

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef struct { unsigned char chars[4]; } FourBytes;
typedef unsigned char u8;
typedef short s16;

/* The string record the njString setters write. Callers pass it as the object that starts 0xD0 bytes into their own record. */
typedef struct NjString {
    f32 x;                              /* 0x00 */
    f32 y;                              /* 0x04 */
    f32 red;                            /* 0x08 */
    f32 green;                          /* 0x0C */
    f32 blue;                           /* 0x10 */
    f32 alpha;                          /* 0x14 */
    s32 styleFlags;                          /* 0x18 */
    s32 textIndex;                         /* 0x1C */
    s32 word20;                         /* 0x20 */
    s32 layoutCount;                         /* 0x24 */
    s32 layoutLimit;                         /* 0x28 */
    unsigned char unmodeled_2C[0xF9 - 0x2C];
    s8 byteF9;                          /* 0xF9 */
    s8 byteFA;                          /* 0xFA */
    unsigned char unmodeled_FB[0xFC - 0xFB];
    s32 wordFC;                         /* 0xFC */
    s16 half100;                        /* 0x100 */
    s16 layoutDirty;                        /* 0x102 */
    s16 half104;                        /* 0x104 */
} NjString;

void njStringSetCounter(NjString *string, s32 limit) {
    string->layoutCount = 0;
    string->layoutLimit = limit;
}

void njStringSetPosition(NjString *string, f32 x, f32 y) {
    string->x = x;
    string->y = y;
}

void njStringSetByteFields(NjString *string, s8 byteF9, s8 byteFA) {
    string->byteF9 = byteF9;
    string->byteFA = byteFA;
}

void njStringSetTextIndexRaw(NjString *string, s32 textIndex) {
    string->textIndex = textIndex;
}

void njStringSetWord20(NjString *string, s32 value) {
    string->word20 = value;
}

void njStringSetColor(NjString *string, s32 red, s32 green, s32 blue) {
    s32 redByte;
    s32 greenByte;
    s32 blueByte;

    redByte = red;
    if (red < 0) {
        redByte = 0;
    }
    greenByte = green;
    if (green < 0) {
        greenByte = 0;
    }
    blueByte = blue;
    if (blue < 0) {
        blueByte = 0;
    }
    if (redByte >= 0x100) {
        redByte = 0xFF;
    }
    string->red = (f32) redByte;
    if (greenByte >= 0x100) {
        greenByte = 0xFF;
    }
    string->green = (f32) greenByte;
    if (blueByte >= 0x100) {
        blueByte = 0xFF;
    }
    string->blue = (f32) blueByte;
}

void njStringSetAlpha(NjString *string, s32 alpha) {
    s32 alphaValue;

    alphaValue = alpha;
    if (alpha < 0) {
        alphaValue = 0;
    }
    if (alphaValue >= 0x80) {
        alphaValue = 0x7F;
    }
    string->alpha = (f32) alphaValue;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6AA8);

void njStringSetFlags(NjString *string, s32 styleFlags) {
    string->styleFlags = styleFlags;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6B28);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6BA0);

extern FourBytes gNjStringTooWidePrefix;
s32 func_0030E3D0();
void *memset(void *destination, int value, unsigned int count);
char *strcpy(char *destination, const char *source);
s32 xglFontGetStringWidth();

/* Prints the 4-byte prefix gNjStringTooWidePrefix followed by the fill character (twice) into a text buffer and returns
 * whether maxWidth is smaller than the width of that text. appendNumber is passed on to func_0030E3D0 unchanged. */
s32 njStringTextTooWide(s32 appendNumber, s32 maxWidth, s32 fillChar)
{
  u8 text[0x400];
  FourBytes prefix;
  if (appendNumber != 0)
  {
    prefix = gNjStringTooWidePrefix;
    prefix.chars[1] = fillChar;
    prefix.chars[2] = fillChar;
    memset(&text[0], 0, 0x400);
    strcpy(&text[0], prefix.chars);
    func_0030E3D0(&text[0], appendNumber);
  }
  return maxWidth < xglFontGetStringWidth(&text[0]);
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D6CA8);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8358);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8458);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D86C8);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D88D8);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8A00);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8B30);

s32 func_002D8D28();

void njStringSetTextIndex(NjString *string, s32 textIndex) {
    s32 layoutScratch[0x200];

    string->textIndex = textIndex;
    string->layoutDirty = 1;
    if ((textIndex != 0) && (string->styleFlags & 0x20)) {
        func_002D8D28(string, &layoutScratch[0]);
    }
}

void njStringSetPositionB(NjString *string, f32 x, f32 y) {
    string->x = x;
    string->y = y;
}

void njStringSetByteFieldsB(NjString *string, s8 byteF9, s8 byteFA) {
    string->byteF9 = byteF9;
    string->byteFA = byteFA;
}

void njStringSetColorB(NjString *string, s32 red, s32 green, s32 blue) {
    s32 redByte;
    s32 greenByte;
    s32 blueByte;

    redByte = red;
    if (red < 0) {
        redByte = 0;
    }
    greenByte = green;
    if (green < 0) {
        greenByte = 0;
    }
    blueByte = blue;
    if (blue < 0) {
        blueByte = 0;
    }
    if (redByte >= 0x100) {
        redByte = 0xFF;
    }
    string->red = (f32) redByte;
    if (greenByte >= 0x100) {
        greenByte = 0xFF;
    }
    string->green = (f32) greenByte;
    if (blueByte >= 0x100) {
        blueByte = 0xFF;
    }
    string->blue = (f32) blueByte;
}

void njStringSetAlphaB(NjString *string, s32 alpha) {
    s32 alphaValue;

    alphaValue = alpha;
    if (alpha < 0) {
        alphaValue = 0;
    }
    if (alphaValue >= 0x80) {
        alphaValue = 0x7F;
    }
    string->alpha = (f32) alphaValue;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8C80);

void njStringSetFlagsB(NjString *string, s32 styleFlags) {
    string->styleFlags = styleFlags;
}

void njStringSetHalf104(NjString *string, s32 value) {
    string->half104 = value & 0xFF;
}

void njStringSetHalf100(NjString *string, s32 value) {
    string->half100 = value & 0xFF;
}

void njStringSetWordFC(NjString *string, s32 value) {
    string->wordFC = value;
}

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002D8D28);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002DAA08);

INCLUDE_ASM("asm/main/nonmatchings/njString", func_002DAEE8);

s32 func_002D6CA8();

s32 njStringLayoutCount(s32 limit, s32 x, s32 y) {
    s32 layoutScratch[0x200];
    s32 layoutStringWords[0x40];

    NjString *layoutString = (NjString *) &layoutStringWords[0];

    njStringSetByteFields(layoutString, 0xF, 0xF);
    njStringSetFlags(layoutString, 8);
    njStringSetCounter(layoutString, limit);
    njStringSetPosition(layoutString, x, y);
    func_002D6CA8(&layoutStringWords[0], limit, &layoutScratch[0]);
    return layoutStringWords[9];
}
