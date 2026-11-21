#include "common.h"

typedef int s32;
typedef float f32;

/* Returns 1 when the squared XZ distance between the two vectors exceeds radius squared (radius > 0). */
s32 isOutsideRadiusXZ(void *targetPosition, void *objectPosition, f32 radius) {
    f32 distanceSquared;

    if (radius <= 0.0f) {
        return 0;
    }
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(targetPosition) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(objectPosition) : "memory");
    __asm__ __volatile__("vsub.xyzw vf11, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf11, vf11, vf11" : : : "memory");
    __asm__ __volatile__("vaddz.x vf11, vf11, vf11z" : : : "memory");
    __asm__ __volatile__("qmfc2.ni $8, vf11\n\tmtc1 $8, %0" : "=f"(distanceSquared) :  : "$8", "memory");
    if ((radius * radius) < distanceSquared) {
        return 1;
    }
    return 0;
}

/* Returns 1 when the squared 3D distance between the two vectors exceeds radius squared (radius > 0). */
s32 isOutsideRadius3D(void *targetPosition, void *objectPosition, f32 radius) {
    f32 distanceSquared;

    if (radius <= 0.0f) {
        return 0;
    }
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(targetPosition) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(objectPosition) : "memory");
    __asm__ __volatile__("vsub.xyzw vf11, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf11, vf11, vf11" : : : "memory");
    __asm__ __volatile__("vaddz.x vf11, vf11, vf11z" : : : "memory");
    __asm__ __volatile__("vaddy.x vf11, vf11, vf11y" : : : "memory");
    __asm__ __volatile__("qmfc2.ni $8, vf11\n\tmtc1 $8, %0" : "=f"(distanceSquared) :  : "$8", "memory");
    if ((radius * radius) < distanceSquared) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu304", func_002859E0);

INCLUDE_ASM("asm/main/nonmatchings/tu304", func_00285B08);

INCLUDE_ASM("asm/main/nonmatchings/tu304", func_00285BF8);

f32 atan2f(f32, f32);

void vecDiffAngleXZ(void *vecA, void *vecB) {
    f32 deltaVec[4];

    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(vecA) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(vecB) : "memory");
    __asm__ __volatile__("vsub.xyz vf1, vf1, vf2" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(deltaVec) : "memory");
    atan2f(deltaVec[2], deltaVec[0]);
}
