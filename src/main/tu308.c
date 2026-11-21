#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu308", func_00286E48);

typedef int s32;
typedef float f32;

typedef struct XzPoint {
    f32 x;
    f32 unk_04;
    f32 z;
} XzPoint;

s32 xzDotWithinLimit(XzPoint *firstDirection, XzPoint *secondDirection, f32 threshold) {
    return threshold <= (firstDirection->x * secondDirection->x) + (firstDirection->z * secondDirection->z) + 1.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu308", func_00286F40);

INCLUDE_ASM("asm/main/nonmatchings/tu308", func_00287000);
