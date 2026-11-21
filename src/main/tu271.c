#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu271", func_00268938);

typedef unsigned char u8;
typedef int s32;

/* Font state block: redrawFlags at +0x00, pending count at +0x38. */
typedef struct FontRecord {
    s32 redrawFlags;
    u8 unk_04[0x38 - 0x04];
    s32 pendingCount;
} FontRecord;

s32 func_00272680();
s32 func_00272D18();
s32 xglFontGetStringWidth();

s32 fontMeasureString(void *text) {
    s32 textEnd;
    textEnd = func_00272D18(text, 1);
    func_00272680(0x70000000, text, textEnd, 0);
    return xglFontGetStringWidth(0x70000000);
}

void fontStateMarkDirty(FontRecord *font) {
    font->pendingCount = 0;
    font->redrawFlags |= 2;
}
