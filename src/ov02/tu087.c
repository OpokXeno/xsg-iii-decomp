/*
 * OV02 original TU 87: 0x00ab0868..0x00ab12b8 (9 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB0868);

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB0B30);

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB0BE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB0CA8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB0D88);

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB0F20);

typedef int s32;
s32 xglSRand();

void shuffleWordTable(s32 *shuffledWords, s32 shuffleWordCount) {
    s32 firstSwapIndex;
    s32 secondSwapIndex;
    s32 remaining;
    s32 swapWord;

    swapWord = (s32) shuffledWords;
    if (shuffleWordCount > 0) {
        remaining = shuffleWordCount;
        do {
            firstSwapIndex = xglSRand(swapWord) % shuffleWordCount;
            remaining -= 1;
            secondSwapIndex = xglSRand() % shuffleWordCount;
            swapWord = shuffledWords[secondSwapIndex];
            shuffledWords[secondSwapIndex] = shuffledWords[firstSwapIndex];
            shuffledWords[firstSwapIndex] = swapWord;
        } while (remaining != 0);
    }
}
INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB1010);

INCLUDE_ASM("asm/nonmatchings/ov02/tu087", func_00AB1100);
