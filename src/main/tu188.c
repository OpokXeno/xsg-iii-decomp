#include "common.h"

#include "gs.h"
typedef unsigned char u8;

typedef int s32;

typedef short s16;

typedef unsigned short u16;

typedef unsigned int u32;

extern unsigned char gFontBuffer[];
void xglFontQueueTextureBind(u8 *fontBuffer);

void xglFontFreeBuffer(void) {
    if (gFontBuffer != 0) {
        xglFontQueueTextureBind(gFontBuffer);
    }
}

/* One GIF packet entry: the 64-bit register value and the GS register address it is written to. */
typedef struct QueueRecord {
    unsigned long registerValue;
    unsigned long registerAddress;
} QueueRecord;

/* One-member view of xglDirectQueueCount (see QUEUE_COUNT). */
typedef struct QueueCountView {
    int queueEntryCount;
} QueueCountView;

extern QueueRecord *xglDirectQueueBuffer;
extern int xglDirectQueueCount;

/* One-member views: these functions read the buffer pointer and the count as a structure member. */
typedef struct QueueBufferView {
    QueueRecord *queueEntries;
} QueueBufferView;

#define QUEUE_BUFFER (((QueueBufferView *) &xglDirectQueueBuffer)->queueEntries)

#define QUEUE_COUNT (((QueueCountView *) &xglDirectQueueCount)->queueEntryCount)

/* Font CLUT base page, in units of 32 words: TEX0 CBP of every font sprite is this value shifted by 5. */
#define FONT_CLUT_BASE (gFontClutBase[0] << 5)

void nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
extern u16 gFontClutBase[];
void func_001997D0(void);

/* Queues the bind of the font texture (TEX0 with the font CLUT, then TEX1), then submits the packet. */
void xglFontQueueTextureBind(u8 *fontBuffer) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;

    if (fontBuffer != 0) {
        func_001997D0();
        nmlPacketGsInit();
        queueWriteStart = cursor = &xglDirectQueueBuffer[xglDirectQueueCount];
        cursor->registerAddress = GS_TEXFLUSH;
        cursor->registerValue = 0;
        cursor++;
        cursor->registerValue = ((unsigned long) 0x20000006 << 32) | (0x29340000 | (FONT_CLUT_BASE)) |
                        ((unsigned long) (FONT_CLUT_BASE) << 37);
        cursor->registerAddress = GS_TEX0_1;
        cursor++;
        cursor->registerValue = 96;
        cursor->registerAddress = GS_TEX1_1;
        cursor++;
        QUEUE_COUNT = xglDirectQueueCount + (((unsigned int) cursor - (unsigned int) queueWriteStart) >> 4);
        xglDirectQueueSubmitRef();
    }
}

/* Queues a texture flush (GS_TEXFLUSH) and submits the packet. */
void xglFontQueueTextureFlush(void) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;

    nmlPacketGsInit();
    queueWriteStart = cursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    cursor->registerAddress = GS_TEXFLUSH;
    cursor->registerValue = 0;
    cursor++;
    QUEUE_COUNT = xglDirectQueueCount + (((unsigned int) cursor - (unsigned int) queueWriteStart) >> 4);
    xglDirectQueueSubmitRef();
}

s32 func_00185A20();
s32 fontQueueDepthTest(s32);
s32 xglFontColorIndexAdd(s32);
void xglFontFreeBuffer();

void xglFontResetState(void) {
    func_00185A20();
    xglFontFreeBuffer();
    fontQueueDepthTest(0);
    xglFontColorIndexAdd(2);
}

/* Screen rectangle of one glyph run: screen origin, texel origin, size and the CLUT offset. */
typedef struct FontRect {
    u16 screenX;
    u16 screenY;
    u16 texU;
    u16 texV;
    u16 width;
    u16 height;
    u16 clutOffset;
} FontRect;

#define GS_TEX2_1 0x16

s32 xglFontGetColorIndex(void);

/* Queues one textured glyph rectangle (font texture, two sprite vertices) and submits the packet. */
void xglFontQueueGlyphRect(FontRect *rect) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;
    QueueRecord *queueBase;
    s32 queueCount;
    u64 texState;

    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    texState = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, FONT_CLUT_BASE + rect->clutOffset, 0, 0, 0, 1);
    queueBase = QUEUE_BUFFER;
    queueWriteStart = cursor = &queueBase[queueCount];
    cursor->registerValue = texState;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_PRIM_TEXTURED_SPRITE;
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue =
        GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, rect->texU >> 4, ((rect->texU + rect->width) >> 4) - 1,
                     rect->texV >> 4, ((rect->texV + rect->height) >> 4) - 1);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(rect->texU, rect->texV);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(rect->screenX, rect->screenY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(rect->texU + rect->width, rect->texV + rect->height);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(rect->screenX + rect->width, rect->screenY + rect->height, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    QUEUE_COUNT = xglDirectQueueCount + (((unsigned int) cursor - (unsigned int) queueWriteStart) >> 4);
    xglDirectQueueSubmitRef();
}

const unsigned short gFontRowFlagTable[8] = {
    32, 64, 16, 128, 4, 1, 8, 2
};

/* Draws a row of the menu frame: the row is the first of the 8 flag entries of gFontRowFlagTable that matches the mask. */
void xglFontQueueFlaggedRow(u16 x, u16 y, s32 rowFlagMask) {
    s32 rowIndex;
    u16 *rowFlagEntry;
    s32 rowHeight;
    s32 rowTop;
    s32 rowBottom;
    s32 colorIndex;
    s32 queueCount;
    QueueRecord *queueBase;
    QueueRecord *queueWriteStart;
    QueueRecord *cursor;
    s32 *queueCountPtr;
    u64 texState;

    rowFlagEntry = gFontRowFlagTable;
    for (rowIndex = 0; rowIndex < 8; rowIndex++) {
        if ((rowFlagMask & *rowFlagEntry) != 0) {
            break;
        }
        rowFlagEntry++;
    }
    if (rowIndex == 8) {
        return;
    }
    rowHeight = 352;
    if (rowIndex < 4) {
        rowHeight = 272;
    }
    rowTop = rowIndex * 288 + (rowHeight - 272) * (rowIndex - 4) + 16;
    nmlPacketGsInit();
    rowBottom = rowTop + rowHeight;
    queueCountPtr = &xglDirectQueueCount;
    queueCount = *queueCountPtr;
    queueBase = QUEUE_BUFFER;
    cursor = &queueBase[queueCount];
    queueWriteStart = cursor;
    texState = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, FONT_CLUT_BASE + 820, 0, 0, 0, 1);
    cursor->registerValue = texState;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_PRIM_TEXTURED_SPRITE;
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, rowTop >> 4, (rowBottom >> 4) - 1, 151, 167);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(rowTop, 0x970);
    cursor->registerAddress = GS_UV;
    cursor++;
    colorIndex = xglFontGetColorIndex();
    cursor->registerValue = GS_SET_XYZ(x, y, colorIndex);
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(rowBottom, 0xa80);
    cursor->registerAddress = GS_UV;
    cursor++;
    colorIndex = xglFontGetColorIndex();
    cursor->registerValue = GS_SET_XYZ(x + rowHeight, y + 272, colorIndex);
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    xglDirectQueueCount += ((unsigned int) cursor - (unsigned int) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Maps a character to its glyph index: A-Z 0x00-0x19 (W is 0x1E), a-z 0x00-0x19, symbols 0x16-0x1F, else 0x40. */
typedef signed char s8;

s32 xglFontGlyphIndex(s8 character) {
    s32 upperOffset;
    s32 glyphIndex;

    upperOffset = character - 0x41;
    if ((u32) (upperOffset & 0xFF) < 0x1AU) {
        glyphIndex = (character != 0x57) ? upperOffset : 0x1E;
    } else if ((u32) (character - 0x61) < 0x1AU) {
        glyphIndex = upperOffset;
    } else {
        switch (character) {
        case 0x2A:
            glyphIndex = 0x16;
            break;
        case 0x2F:
            glyphIndex = 0x1A;
            break;
        case 0x2D:
            glyphIndex = 0x1B;
            break;
        case 0x3A:
            glyphIndex = 0x1C;
            break;
        case 0x2E:
            glyphIndex = 0x1D;
            break;
        case 0x21:
            glyphIndex = 0x1F;
            break;
        default:
            glyphIndex = 0x40;
            break;
        }
    }
    return glyphIndex;
}

/* Width table of the font glyphs, indexed by glyph index. */
const signed char gFontGlyphWidthTable[72] = {
    15, 13, 13, 14, 12, 12, 14, 13,
    8, 14, 13, 12, 15, 13, 14, 13,
    15, 14, 13, 14, 13, 14, 14, 14,
    14, 13, 11, 10, 8, 8, 17, 8,
    14, 13, 13, 13, 14, 12, 13, 14,
    8, 13, 12, 9, 14, 13, 13, 13,
    13, 13, 13, 11, 14, 13, 14, 13,
    13, 13, 0, 0, 0, 0, 0, 0,
    8, 0, 0, 0, 0, 0, 0, 0
};

s8 xglFontGlyphWidth(s8 character) {
    return gFontGlyphWidthTable[xglFontGlyphIndex(character)];
}

s32 xglFontTextWidth(s8 *text, s32 scale);

/* Width in pixels of a string at the given scale (sum of the glyph advances). */
s32 xglFontTextWidth(s8 *text, s32 scale) {
    s32 totalWidth;
    s32 glyphWidth;
    s8 *cursor;

    totalWidth = 0;
    cursor = text;
    while (*cursor != 0) {
        glyphWidth = xglFontGlyphWidth(*cursor++);
        totalWidth += ((glyphWidth * scale) - scale) >> 4;
    }
    return totalWidth;
}

/* Same layout as QueueRecord, kept apart: the field types decide the emitted order. */
typedef struct SpriteRecord {
    u64 registerValue;
    u64 registerAddress;
} SpriteRecord;

s32 xglFontDrawDigitScaled(s32 x, s32 y, u32 digit, s32 scale);

#define GIF_PUT(cursor, gsValue, gsRegister) \
    (cursor)->registerValue = (gsValue);     \
    (cursor)->registerAddress = (gsRegister); \
    (cursor)++
#define GS_XYZ(x, y, z) GS_SET_XYZ(x, y, (u32) (z))
#define FONT_TEX0(clutOffset) \
    GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, FONT_CLUT_BASE + (clutOffset), GS_PSMCT32, 0, 0, 1)

/* Draws one character: a digit goes to xglFontDrawDigitScaled, any other glyph is a textured sprite. Returns the advance. */
s32 xglFontDrawCharScaled(u16 x, u16 y, s8 character, s32 scale) {
    s32 glyphWidth;
    s32 advance;
    s32 glyphIndex;
    s32 texU;
    s32 texV;
    s32 glyphHeight;
    SpriteRecord *packetStart;
    SpriteRecord *cursor;

    if ((u8) (character - '0') < 10) {
        return ((s32 (*)(u16 x, u16 y, s8 digit, s32 scale)) xglFontDrawDigitScaled)(x, y - 0x10, character - '0', scale);
    }
    glyphHeight = scale << 4;
    glyphWidth = xglFontGlyphWidth(character) << 4;
    advance = (glyphWidth * scale) >> 4;
    glyphIndex = xglFontGlyphIndex(character);
    if (glyphIndex < 0x40) {
        nmlPacketGsInit();
        packetStart = cursor = &((SpriteRecord *) xglDirectQueueBuffer)[xglDirectQueueCount];
        if (glyphIndex >= 0x20) {
            glyphIndex &= 0x1F;
            texU = ((glyphIndex & 0xF) << 8) | 0x3000;
            texV = (glyphIndex >> 4) << 8;
            GIF_PUT(cursor, FONT_TEX0(968), GS_TEX2_1);
        } else {
            texU = (glyphIndex & 0xF) << 8;
            if (glyphIndex == 0x1F) {
                texU += 0x40;
            }
            texV = (glyphIndex >> 4) << 8;
            GIF_PUT(cursor, FONT_TEX0(776), GS_TEX2_1);
        }
        GIF_PUT(cursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
        GIF_PUT(cursor,
                GS_SET_CLAMP(2, 2, texU >> 4, ((texU + glyphWidth) >> 4) - 1, texV >> 4,
                             ((texV + 0x100) >> 4) - 1),
                GS_CLAMP_1);
        GIF_PUT(cursor, GS_SET_UV(texU, texV), GS_UV);
        GIF_PUT(cursor, GS_XYZ(x, y, xglFontGetColorIndex()), GS_XYZ2);
        GIF_PUT(cursor, GS_SET_UV(texU + glyphWidth, texV + 0x100), GS_UV);
        GIF_PUT(cursor, GS_XYZ(x + advance, y + glyphHeight, xglFontGetColorIndex()), GS_XYZ2);
        xglDirectQueueCount += ((u32) cursor - (u32) packetStart) >> 4;
        xglDirectQueueSubmitRef();
    }
    return (advance - scale) >> 4;
}

s32 xglFontDrawStringScaled(s32 startX, s32 startY, s8 *text, s32 scale);

/* Draws a string from startX/startY at the given scale and returns its width in pixels. */
s32 xglFontDrawStringScaled(s32 startX, s32 startY, s8 *text, s32 scale) {
    s32 x;
    s32 y;
    s32 advanceTotal;
    s8 *cursor;

    x = startX & 0xFFFF;
    y = startY & 0xFFFF;
    advanceTotal = 0;
    cursor = text;
    while (*cursor != 0) {
        advanceTotal += ((s32 (*)(s32 x, s32 y, s8 digit, s32 scale)) xglFontDrawCharScaled)((x + advanceTotal) & 0xFFFF, y,
                                                                                     *cursor, scale)
                        << 4;
        cursor++;
    }
    return advanceTotal >> 4;
}

/* Draws the string right-aligned at baseX: it starts 16 times its width to the left. */
void xglFontDrawRightAligned16(s32 baseX, s32 baseY, s8 *text, s32 scale) {
    s32 x;
    s32 y;

    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    x -= /* M2C_CALL_ABI xglFontTextWidth stable */ xglFontTextWidth(text, scale) * 0x10;
    /* M2C_CALL_ABI xglFontDrawStringScaled stable */ xglFontDrawStringScaled((x & 0xFFFF), y, text, scale);
}

/* Same as xglFontDrawRightAligned16 with half a glyph cell of shift (8 instead of 16). */
void xglFontDrawRightAlignedHalfCell(s32 baseX, s32 baseY, s8 *text, s32 scale) {
    s32 x;
    s32 y;

    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    x -= /* M2C_CALL_ABI xglFontTextWidth stable */ xglFontTextWidth(text, scale) * 8;
    /* M2C_CALL_ABI xglFontDrawStringScaled stable */ xglFontDrawStringScaled((x & 0xFFFF), y, text, scale);
}

/* Horizontal step of one digit at scale 1, in 1/16 pixel units. */
extern u16 gFontDigitStep;


void xglFontSetDigitStep(s16 digitStep) {
    gFontDigitStep = digitStep;
}

/* Queues one digit sprite (font digits 0-9 sit in texture columns 9,0,1,...,8) and returns its advance. */
s32 xglFontDrawDigitScaled(s32 x, s32 y, u32 digit, s32 scale) {
    SpriteRecord *cursor;
    SpriteRecord *packetStart;
    s32 glyphX;
    s32 glyphY;
    s32 glyphW;
    s32 glyphH;
    s32 texU;

    texU = ((((s8) digit + 9) % 10) * 13) << 4;
    glyphX = x & 0xFFFF;
    glyphY = y & 0xFFFF;
    glyphW = 13 * scale;
    glyphH = 17 * scale;
    nmlPacketGsInit();
    cursor = ((SpriteRecord *) xglDirectQueueBuffer) + xglDirectQueueCount;
    packetStart = cursor;
    cursor->registerValue = ((u64) ((gFontClutBase[0] << 5) + 780) << 37) | 0x2000000001300000ULL;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_PRIM_TEXTURED_SPRITE;
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(2, 2, texU >> 4, (texU >> 4) + 12, 32, 48);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(texU, 512);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(glyphX, glyphY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(texU + 208, 784);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(glyphX + glyphW, glyphY + glyphH, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    xglDirectQueueCount += ((u32) cursor - (u32) packetStart) >> 4;
    xglDirectQueueSubmitRef();
    return (gFontDigitStep * scale) >> 4;
}

s32 unsignedDecimalDigits(s32);
s32 xglFontDrawDigits();

/* Draws an unsigned number right-aligned at baseX; the width of its digits is subtracted first. */
s32 xglFontDrawUnsignedNumber(s32 baseX, s32 baseY, s32 number, s32 scale) {
    s32 stepPerDigit;
    s32 y;
    s32 x;
    s32 digitCount;
    s32 digitsWidth;

    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    digitCount = /* M2C_CALL_ABI unsignedDecimalDigits stable */ unsignedDecimalDigits(number);
    digitsWidth = 0;
    if (digitCount > 0) {
        stepPerDigit = gFontDigitStep * scale;
        do {
            digitCount -= 1;
            digitsWidth += stepPerDigit >> 4;
        } while (digitCount != 0);
    }
    return /* M2C_CALL_ABI xglFontDrawDigits stable */ xglFontDrawDigits(((x + (digitsWidth * 0x10)) & 0xFFFF), y, number, scale);
}

/* Draws the digits of number from the least significant one leftwards; returns the total advance. */
s32 xglFontDrawDigits(s32 x, s32 y, u32 number, s32 scale) {
    s32 digitAdvance;
    s32 totalAdvance;
    u32 remaining;

    remaining = number;
    totalAdvance = 0;
    do {
        digitAdvance = xglFontDrawDigitScaled((((x & 0xFFFF) - totalAdvance) - (gFontDigitStep * scale)) & 0xFFFF, y & 0xFFFF, remaining % 10U, scale) * 0x10;
        remaining = remaining / 10U;
        totalAdvance += digitAdvance;
    } while (remaining != 0 || totalAdvance == 0);
    return totalAdvance >> 4;
}

s32 xglFontDrawSecondDigitScaled(s32 x, s32 y, u32 digit, s32 scale);

/* Same as xglFontDrawDigitScaled for the second digit row of the texture (CLUT 804, rows 0xA90-0xBB0). */
s32 xglFontDrawSecondDigitScaled(s32 x, s32 y, u32 digit, s32 scale) {
    SpriteRecord *cursor;
    SpriteRecord *packetStart;
    s32 glyphX;
    s32 glyphY;
    s32 glyphW;
    s32 glyphH;
    s32 texU;

    texU = ((((s8) digit + 9) % 10) * 13) << 4;
    glyphX = x & 0xFFFF;
    glyphY = y & 0xFFFF;
    glyphW = 13 * scale;
    glyphH = 17 * scale;
    nmlPacketGsInit();
    cursor = ((SpriteRecord *) xglDirectQueueBuffer) + xglDirectQueueCount;
    packetStart = cursor;
    cursor->registerValue = ((u64) ((gFontClutBase[0] << 5) + 804) << 37) | 0x2000000001300000ULL;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_PRIM_TEXTURED_SPRITE;
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(2, 2, texU >> 4, (texU >> 4) + 12, 169, 186);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(texU, 0xA90);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(glyphX, glyphY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(texU + 208, 0xBB0);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(glyphX + glyphW, glyphY + glyphH, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    xglDirectQueueCount += ((u32) cursor - (u32) packetStart) >> 4;
    xglDirectQueueSubmitRef();
    return (gFontDigitStep * scale) >> 4;
}

s32 xglFontDrawDigitsLower();

/* Alternate of xglFontDrawUnsignedNumber: draws the digits through xglFontDrawSecondDigitScaled. */
void xglFontDrawUnsignedNumberLower(s32 baseX, s32 baseY, s32 number, s32 scale) {
    s32 stepPerDigit;
    s32 y;
    s32 x;
    s32 digitCount;
    s32 digitsWidth;

    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    digitCount = /* M2C_CALL_ABI unsignedDecimalDigits stable */ unsignedDecimalDigits(number);
    digitsWidth = 0;
    if (digitCount > 0) {
        stepPerDigit = gFontDigitStep * scale;
        do {
            digitCount -= 1;
            digitsWidth += stepPerDigit >> 4;
        } while (digitCount != 0);
    }
    /* M2C_CALL_ABI xglFontDrawDigitsLower stable */ xglFontDrawDigitsLower(((x + (digitsWidth * 0x10)) & 0xFFFF), y, number, scale);
}

/* Alternate of xglFontDrawDigits: draws the digits through xglFontDrawSecondDigitScaled. */
s32 xglFontDrawDigitsLower(s32 x, s32 y, u32 number, s32 scale) {
    s32 digitAdvance;
    s32 totalAdvance;
    u32 remaining;

    remaining = number;
    totalAdvance = 0;
    do {
        digitAdvance = xglFontDrawSecondDigitScaled((((x & 0xFFFF) - totalAdvance) - (gFontDigitStep * scale)) & 0xFFFF, y & 0xFFFF, remaining % 10U, scale) * 0x10;
        remaining = remaining / 10U;
        totalAdvance += digitAdvance;
    } while (remaining != 0 || totalAdvance == 0);
    return totalAdvance >> 4;
}

/* Like xglFontDrawDigits, but always draws at least two digit positions (a leading zero is drawn). */
s32 xglFontDrawDigitsMinTwo(s32 x, s32 y, u32 number, s32 scale)
{
  s32 digitAdvance;
  s32 totalAdvance;
  s32 passes;
  u32 remaining;

  remaining = number;
  totalAdvance = 0;
  passes = 0;
  do
  {
    passes += 1;

    digitAdvance = xglFontDrawDigitScaled((((x & 0xFFFF) - totalAdvance) - (gFontDigitStep * scale)) & 0xFFFF, y & 0xFFFF, remaining % 10U, scale) * 0x10;
    totalAdvance += digitAdvance;
    remaining = remaining / 10U;
  }
  while (remaining != 0 || passes < 2);
  return totalAdvance >> 4;
}

u16 gFontDigitStep = 10;
