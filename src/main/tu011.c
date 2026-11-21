#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef float f32;

typedef struct PadRightStick {
    u8 unk_00[0x64];
    s8 rightX;
    s8 rightY;
} PadRightStick;

void padGetRightStick(PadRightStick *controllerPort, f32 *stickX, f32 *stickY) {
    *stickX = controllerPort->rightX * 0.0078125f;
    *stickY = controllerPort->rightY * 0.0078125f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu011", func_00186FB8);

extern unsigned char gPadLatchBlocked[];
extern s16 gPadLatchedValue;

void padSetLatchedValue(s32 latchedValue) {
    if (gPadLatchBlocked[0] == 0) {
        gPadLatchedValue = latchedValue;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu011", func_00186FF0);

INCLUDE_ASM("asm/main/nonmatchings/tu011", func_00187020);
