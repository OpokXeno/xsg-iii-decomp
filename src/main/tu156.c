#include "common.h"

typedef int s32;
extern char *gFontEscapeBuffer;
s32 func_0030E3D0(void *text, s32 value);
s32 xglFontGetStringWidth(const char *text);

s32 xglFontGetFormattedWidth(s32 formattedNumber) {
    gFontEscapeBuffer[0] = 0x19;
    gFontEscapeBuffer[1] = 3;
    gFontEscapeBuffer[2] = 0;
    func_0030E3D0(gFontEscapeBuffer, formattedNumber);
    return xglFontGetStringWidth(gFontEscapeBuffer);
}

INCLUDE_ASM("asm/main/nonmatchings/tu156", func_00213268);

INCLUDE_ASM("asm/main/nonmatchings/tu156", func_002132C8);
