#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_00218898);

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_00218A70);

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_00218E48);

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_002193A0);

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_00219578);

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_00219980);

INCLUDE_ASM("asm/main/nonmatchings/tu168", func_00219AC0);

typedef int s32;
s32 func_00214A00();
s32 xglFontSetAlphaScale(s32);
s32 xglFontGetAlphaScale();
s32 func_00218898(s32 drawObjectHandle, s32 pass);
s32 func_00219578(s32 drawObjectHandle, s32 pass);
s32 func_00219AC0(s32 drawObjectHandle);

void xglFontDrawObjectPasses(s32 drawObjectHandle) {
    s32 savedAlphaScale;

    savedAlphaScale = xglFontGetAlphaScale();
    func_00219AC0(drawObjectHandle);
    func_00218898(drawObjectHandle, 0);
    xglFontSetAlphaScale(savedAlphaScale);
    func_00219578(drawObjectHandle, 0);
    func_00218898(drawObjectHandle, 1);
    xglFontSetAlphaScale(savedAlphaScale);
    func_00219578(drawObjectHandle, 1);
    func_00214A00();
}
