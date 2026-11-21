#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272650);

typedef signed char s8;
extern s8 gTextMovieMode;

void textDrawModeSet(s8 movieMode) {
    gTextMovieMode = movieMode;
}

extern s8 gTextSingletonIdDraw;

void textDrawFlagSet(s8 enabled) {
    gTextSingletonIdDraw = enabled;
}

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272680);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272A10);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272B38);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272CD8);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272D18);
