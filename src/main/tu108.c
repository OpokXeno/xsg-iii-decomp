#include "common.h"

typedef int s32;
extern s32 gDrawArea;
void drawAreaSetWindow(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

s32 *drawEnvGetRecord(void) {
    return &gDrawArea;
}

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D4C40);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5398);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D53E0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5440);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D54C8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5528);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5588);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D55E8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5630);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5678);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D56D8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5738);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5798);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D57E0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5840);

void func_001D5398();

void drawEnvSetLarge(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x200, 0x1C0);
    func_001D5398();
}

void func_001D53E0();

void drawEnvSetHalfDraw(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D53E0();
}

s32 xglRenderMovieIsActive();
s32 func_001D5440();

void drawEnvSetField(void) {
    if (xglRenderMovieIsActive() != 0) {
        drawAreaSetWindow(0x80, 3, 0, 0, 0x280, 0x1BF, 0, 0, 0x200, 0x1BF);
    } else {
        drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1BF, 0, 0, 0x200, 0x1BF);
    }
    func_001D5440();
}

void func_001D56D8();

void drawEnvSetHalfDisplay(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D56D8();
}

void func_001D5738();

void drawEnvSetHalfDisplayNext(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D5738();
}

void func_001D54C8();

void drawEnvSetHalfDrawNext(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D54C8();
}

void func_001D5528();

void drawEnvSetHalfDrawLast(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D5528();
}

void drawEnvSetHalfDrawRepeat(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D5528();
}
