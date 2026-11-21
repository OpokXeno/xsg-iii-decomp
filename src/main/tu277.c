#include "common.h"

typedef int s32;
extern s32 gMovieCurrentIndex;

void movieResetCurrentIndex(void) {
    gMovieCurrentIndex = -1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu277", func_00269BA8);

INCLUDE_ASM("asm/main/nonmatchings/tu277", func_00269BD8);

s32 movieGetCurrentIndex(void) {
    return gMovieCurrentIndex;
}

void followSlotsClear(void);

void movieClearWorkBuffer(void) {
    followSlotsClear();
}
