#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_00260F70);

typedef int s32;
typedef float f32;
extern s32 D_0055A810[];
void *memset(void *s, int c, unsigned int n);

void cameraBlockClear(void) {
    memset(D_0055A810, 0, 8);
}

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_00260FD0);

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_00261050);

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_00261188);

typedef struct Vector3f {
    f32 x;
    f32 y;
    f32 z;
} Vector3f;

void vector3fLerp(Vector3f *outVector, Vector3f *startVector, Vector3f *endVector, f32 t) {
    f32 startX;
    f32 startY;
    f32 startZ;

    startX = startVector->x;
    outVector->x = ((endVector->x - startX) * t) + startX;
    startY = startVector->y;
    outVector->y = ((endVector->y - startY) * t) + startY;
    startZ = startVector->z;
    outVector->z = ((endVector->z - startZ) * t) + startZ;
}

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_002612F0);

void floatLerp(f32 *outScalar, f32 startScalar, f32 endScalar, f32 t) {
    *outScalar = ((endScalar - startScalar) * t) + startScalar;
}

s32 cameraBlockGetFirst(void) {
    return D_0055A810[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_002613A0);

INCLUDE_ASM("asm/main/nonmatchings/tu249", func_00261448);
