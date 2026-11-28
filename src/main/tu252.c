#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;

typedef struct PointRef {
    void *targetPoint;
    u8 pad_04[4];
    f32 *scale;
} PointRef;

extern unsigned char gEffcamNativeTable[];
extern unsigned char gEffcamOwnerName[];
s32 sbNativeOwnerAdd(const char *name, void *methods);

void cameraRegisterEffcamNatives(void) {
    sbNativeOwnerAdd(gEffcamOwnerName, gEffcamNativeTable);
}

INCLUDE_ASM("asm/main/nonmatchings/tu252", func_00261DA0);

extern unsigned char gCameraSourcePointers[];
#define gPointRef (*(PointRef *) gCameraSourcePointers)
extern f32 gCameraRefDistance;
extern f32 gCameraRefScale;
f32 xglPointLength(void *cameraPoint, void *to);

void cameraUpdateRefDistance(void *cameraPoint) {
    f32 length;
    f32 scale;

    length = xglPointLength(cameraPoint, gPointRef.targetPoint);
    gCameraRefDistance = length;
    scale = *gPointRef.scale;
    gCameraRefScale = scale;
}

INCLUDE_ASM("asm/main/nonmatchings/tu252", func_00261F20);

INCLUDE_ASM("asm/main/nonmatchings/tu252", func_00261FB8);

INCLUDE_ASM("asm/main/nonmatchings/tu252", func_002621F8);
