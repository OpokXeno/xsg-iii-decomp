/*
 * OV04 original TU 18: 0x00a9fb30..0x00a9fe40 (5 functions)
 */
#include "common.h"

typedef float f32;
typedef int s32;

/* Moves approachFrom toward approachTo by one stepDivisor-th of the remaining distance (returns approachFrom when equal). */
f32 floatApproach(f32 approachFrom, f32 approachTo, f32 stepDivisor) {
    if (approachFrom == approachTo) {
        return approachFrom;
    }
    return approachFrom + ((approachTo - approachFrom) / stepDivisor);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu018", func_00A9FB60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu018", func_00A9FC88);

s32 intMaxCapped(s32 leftValue, s32 rightValue, s32 limit) {
    s32 larger;

    larger = leftValue;
    if (leftValue < rightValue) {
        larger = rightValue;
    }
    return (limit < larger) ? limit : larger;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu018", OuterProduct);
