#include "common.h"

typedef int s32;
s32 memset();

/* Clears the 0x110-byte block at block. */
void blockClear110(s32 block) {
    memset(block, 0, 0x110);
}

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BD230);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BD2D0);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BD698);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BD6E0);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BD7A8);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BDC90);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BDEA8);

INCLUDE_ASM("asm/main/nonmatchings/tu369", func_002BE2E0);

s32 func_002146C0(s32);
s32 xglFontSetColorIndex(s32);
s32 xglFontGetColorIndex();
s32 xglFontGetStateMask();
s32 sheetDrawFrame(s32);

/* Saves the two state words, sets state 0xFFFF01 and mask 0x80808080, runs the body, restores. */
void xglFontDrawMaskedDefault(void) {
    s32 savedStateMask;
    s32 savedColorIndex;

    savedColorIndex = xglFontGetColorIndex();
    savedStateMask = xglFontGetStateMask();
    xglFontSetColorIndex(0xFFFF01);
    func_002146C0(0x80808080);
    sheetDrawFrame(0x19F);
    func_002146C0(savedStateMask);
    xglFontSetColorIndex(savedColorIndex);
}

s32 mcStateSeekEntry(s32);

/* Same save/restore as xglFontDrawMaskedDefault, with mcStateSeekEntry(value) as the body. */
void xglFontDrawMaskedWith(s32 seekMode) {
    s32 savedStateMask;
    s32 savedColorIndex;

    savedColorIndex = xglFontGetColorIndex();
    savedStateMask = xglFontGetStateMask();
    xglFontSetColorIndex(0xFFFF01);
    func_002146C0(0x80808080);
    mcStateSeekEntry(seekMode);
    func_002146C0(savedStateMask);
    xglFontSetColorIndex(savedColorIndex);
}
