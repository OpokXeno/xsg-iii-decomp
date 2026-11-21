/*$
 * OV02 original TU 16: 0x00a871b0..0x00a874a0 (3 functions)$
 */
#include "common.h"

typedef int s32;
extern unsigned char D_00ABD2B0[];
void *memset(void *, s32, unsigned int);

void sceneStateReset(void) {
    memset(D_00ABD2B0, 0, 0x6C);
}

void *sceneStateGet(void) {
    return D_00ABD2B0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu016", func_00A871E8);
