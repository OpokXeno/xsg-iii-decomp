/*
 * OV02 original TU 84: 0x00aad1c8..0x00aad248 (3 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;
s32 pxyModelRenderStateLoadLight(s32);
s32 lightBlockGet();

/* Object with a word at 0x1B4 that the clear function resets. */
typedef struct PxyObject {
    u8 unk_00[0x1B4];
    s32 unk_1B4;
} PxyObject;

void pxyLoadCurrentLight(void) {
    s32 currentLight;
    currentLight = lightBlockGet();
    pxyModelRenderStateLoadLight(currentLight);
}

void pxyObjectResetWord1B4(PxyObject *object) {
    object->unk_1B4 = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu084", func_00AAD1F0);
