/*
 * OV02 original TU 80: 0x00aabbc8..0x00aabe08 (4 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef float f32;
extern s32 gActiveEntryIndex;
extern s32 gActiveEntryTable[];

s32 activeEntryIndexGet(void) {
    return gActiveEntryIndex;
}

s32 activeEntryGet(void) {
    return gActiveEntryTable[gActiveEntryIndex];
}

/* Only the rotation of the camera view is modeled: rotation.y sits at +0xA4 of the XglCamera travel view. */
typedef struct Ov02CameraView {
    u8 unk00[0xA4];
    f32 rotationY;
} Ov02CameraView;

Ov02CameraView *xglStudioGetCamera2(int cameraIndex);

f32 ov02CameraYawOpposite(void) {
    return xglStudioGetCamera2(0)->rotationY + 3.1415927f;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu080", func_00AABC28);
