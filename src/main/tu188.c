#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef short s16;
typedef unsigned short u16;
typedef unsigned int u32;
extern unsigned char gFontBuffer[];
s32 func_0023BC20();

void xglFontFreeBuffer(void) {
    if (gFontBuffer != 0) {
        func_0023BC20(gFontBuffer);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023BC20);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023BCE0);

s32 func_00185A20();
s32 func_002147D0(s32);
s32 xglFontColorIndexAdd(s32);
void xglFontFreeBuffer();

void xglFontResetState(void) {
    func_00185A20();
    xglFontFreeBuffer();
    func_002147D0(0);
    xglFontColorIndexAdd(2);
}

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023BD60);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023BF30);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023C168);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023C210);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023C248);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023C2C0);

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023C588);

s32 func_0023C248(s32, s32);
s32 func_0023C588(s32, s32, s32, s32);

void xglFontDrawRightAligned16(s32 baseX, s32 baseY, s32 glyphKind, s32 glyphIndex) {
    s32 x;
    s32 y;
    s32 savedGlyphKind;
    s32 savedGlyphIndex;

    savedGlyphKind = glyphKind;
    savedGlyphIndex = glyphIndex;
    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    x -= /* M2C_CALL_ABI func_0023C248 stable */ func_0023C248(savedGlyphKind, savedGlyphIndex) * 0x10;
    /* M2C_CALL_ABI func_0023C588 stable */ func_0023C588((x & 0xFFFF), y, savedGlyphKind, savedGlyphIndex);
}

void xglFontDrawRightAlignedHalfCell(s32 baseX, s32 baseY, s32 glyphKind, s32 glyphIndex) {
    s32 x;
    s32 y;
    s32 savedGlyphKind;
    s32 savedGlyphIndex;

    savedGlyphKind = glyphKind;
    savedGlyphIndex = glyphIndex;
    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    x -= /* M2C_CALL_ABI func_0023C248 stable */ func_0023C248(savedGlyphKind, savedGlyphIndex) * 8;
    /* M2C_CALL_ABI func_0023C588 stable */ func_0023C588((x & 0xFFFF), y, savedGlyphKind, savedGlyphIndex);
}

extern u16 gFontDigitStep;

void xglFontSetDigitStep(s16 digitStep) {
    gFontDigitStep = digitStep;
}

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023C6F8);

s32 unsignedDecimalDigits(s32);
s32 xglFontDrawDigits();

s32 xglFontDrawUnsignedNumber(s32 baseX, s32 baseY, s32 number, s32 scale) {
    s32 stepPerDigit;
    s32 scaleSaved;
    s32 y;
    s32 x;
    s32 digitCount;
    s32 digitsWidth;

    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    scaleSaved = scale;
    digitCount = /* M2C_CALL_ABI unsignedDecimalDigits stable */ unsignedDecimalDigits(number);
    digitsWidth = 0;
    if (digitCount > 0) {
        stepPerDigit = gFontDigitStep * scaleSaved;
        do {
            digitCount -= 1;
            digitsWidth += stepPerDigit >> 4;
        } while (digitCount != 0);
    }
    return /* M2C_CALL_ABI xglFontDrawDigits stable */ xglFontDrawDigits(((x + (digitsWidth * 0x10)) & 0xFFFF), y, number, scaleSaved);
}

s32 func_0023C6F8(s32, s32, u32, s32);

s32 xglFontDrawDigits(s32 x, s32 y, u32 number, s32 scale) {
    s32 digitAdvance;
    s32 totalAdvance;
    u32 remaining;

    remaining = number;
    totalAdvance = 0;
    do {
        digitAdvance = func_0023C6F8((((x & 0xFFFF) - totalAdvance) - (gFontDigitStep * scale)) & 0xFFFF, y & 0xFFFF, remaining % 10U, scale) * 0x10;
        remaining = remaining / 10U;
        totalAdvance += digitAdvance;
    } while (remaining != 0 || totalAdvance == 0);
    return totalAdvance >> 4;
}

INCLUDE_ASM("asm/main/nonmatchings/tu188", func_0023CA38);

s32 xglFontDrawDigitsAlt();

void xglFontDrawUnsignedNumberAlt(s32 baseX, s32 baseY, s32 number, s32 scale) {
    s32 stepPerDigit;
    s32 scaleSaved;
    s32 y;
    s32 x;
    s32 digitCount;
    s32 digitsWidth;

    x = baseX & 0xFFFF;
    y = baseY & 0xFFFF;
    scaleSaved = scale;
    digitCount = /* M2C_CALL_ABI unsignedDecimalDigits stable */ unsignedDecimalDigits(number);
    digitsWidth = 0;
    if (digitCount > 0) {
        stepPerDigit = gFontDigitStep * scaleSaved;
        do {
            digitCount -= 1;
            digitsWidth += stepPerDigit >> 4;
        } while (digitCount != 0);
    }
    /* M2C_CALL_ABI xglFontDrawDigitsAlt stable */ xglFontDrawDigitsAlt(((x + (digitsWidth * 0x10)) & 0xFFFF), y, number, scaleSaved);
}

s32 func_0023CA38(s32, s32, u32, s32);

s32 xglFontDrawDigitsAlt(s32 x, s32 y, u32 number, s32 scale) {
    s32 digitAdvance;
    s32 totalAdvance;
    u32 remaining;

    remaining = number;
    totalAdvance = 0;
    do {
        digitAdvance = func_0023CA38((((x & 0xFFFF) - totalAdvance) - (gFontDigitStep * scale)) & 0xFFFF, y & 0xFFFF, remaining % 10U, scale) * 0x10;
        remaining = remaining / 10U;
        totalAdvance += digitAdvance;
    } while (remaining != 0 || totalAdvance == 0);
    return totalAdvance >> 4;
}

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

    digitAdvance = func_0023C6F8((((x & 0xFFFF) - totalAdvance) - (gFontDigitStep * scale)) & 0xFFFF, y & 0xFFFF, remaining % 10U, scale) * 0x10;
    totalAdvance += digitAdvance;
    remaining = remaining / 10U;
  }
  while (remaining != 0 || passes < 2);
  return totalAdvance >> 4;
}
