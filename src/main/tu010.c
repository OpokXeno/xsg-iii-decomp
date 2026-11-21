#include "common.h"

typedef signed char s8;
typedef unsigned short u16;
typedef int s32;
typedef float f32;

typedef struct PadPortRecord {
    unsigned char unk00[0x28];
    unsigned short heldButtons;
    unsigned char unk2A[0xA];
    unsigned short buttons34;
    unsigned short buttons36;
    unsigned char unk38[0x2E];
    signed char stickX;
    signed char stickY;
} PadPortRecord;

s32 padIsMaskHeld36(PadPortRecord *pad, s32 mask) {
    return (~pad->buttons36 & mask) == 0;
}

s32 padIsMaskHeld34(PadPortRecord *pad, s32 mask) {
    return (~pad->buttons34 & mask) == 0;
}

void xglPadGetAnalog(PadPortRecord *pad, f32 *axisX, f32 *axisY, s32 useDpad) {
    s32 heldButtons;

    *axisX = pad->stickX * 0.0078125f;
    *axisY = pad->stickY * 0.0078125f;
    if (useDpad != 0) {
        heldButtons = pad->heldButtons;
        if (heldButtons & 0x8000) {
            *axisX = -1.0f;
        }
        if (heldButtons & 0x2000) {
            *axisX = 1.0f;
        }
        if (heldButtons & 0x1000) {
            *axisY = -1.0f;
        }
        if (heldButtons & 0x4000) {
            *axisY = 1.0f;
        }
    }
}
