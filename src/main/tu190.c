#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;
extern s8 glyphIndexTable[];

s8 glyphIndexGet(u32 glyphCode) {
    if (!(glyphCode < 0x16U)) {
        return -1;
    }
    return glyphIndexTable[glyphCode];

}

extern unsigned char glyphPairOffsetTable[];

s32 glyphPairOffsetGet(s32 glyphIndex) {
    return glyphPairOffsetTable[glyphIndex >> 1] + 0x320;
}

extern unsigned char glyphAttrTable[];

u8 glyphAttrGet(s32 glyphIndex) {
    return glyphAttrTable[glyphIndex];
}

extern unsigned char D_00538E40[];

u8 glyphAltAttrGet(s32 glyphIndex) {
    return D_00538E40[glyphIndex];
}

INCLUDE_ASM("asm/main/nonmatchings/tu190", func_0023DD18);

s8 glyphIndexGet();
void func_0023DD18(s32 drawTarget, s32 index, s32 attr, s32 mode);

void glyphDrawByCode(s32 drawTarget, s32 glyphCode) {
    s32 glyphIndex;
    glyphIndex = glyphIndexGet(glyphCode);
    func_0023DD18(drawTarget, glyphIndex, 0, 0x40);
}

u8 glyphAttrGet();

void glyphDrawWithAttr(s32 drawTarget, s32 glyphCode) {
    s32 glyphIndex;
    s32 glyphAttr;

    glyphIndex = glyphIndexGet(glyphCode);
    glyphAttr = glyphAttrGet(glyphIndex);
    func_0023DD18(drawTarget, glyphIndex, glyphAttr, 0x30);
}

u8 glyphAltAttrGet();

void glyphDrawWithAltAttr(s32 drawTarget, s32 glyphCode) {
    s32 glyphIndex;
    s32 attr;

    glyphIndex = glyphIndexGet(glyphCode);
    attr = glyphAltAttrGet(glyphIndex);
    func_0023DD18(drawTarget, glyphIndex, attr, 0x2B);
}

void glyphDrawPairSlot(s32 drawTarget, s32 pairNumber) {
    s32 zeroBasedPair;

    zeroBasedPair = pairNumber - 1;
    if (pairNumber != 0) {
        func_0023DD18(drawTarget, (zeroBasedPair >> 1) + 0xE, (~pairNumber & 1) << 5, 0x20);
    }
}
