#include "common.h"

typedef float f32;

void *vu0CrossWithWeights(void *crossProduct, void *lhs, void *rhs) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(lhs) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(rhs) : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf3, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vmul.xyzw vf4, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vaddy.x vf4, vf4, vf4y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf4, vf4, vf4z" : : : "memory");
    __asm__ __volatile__("vsubx.w vf7, vf4, vf4x" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf5, vf1, vf2w" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf6, vf2, vf1w" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf3, vf5" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf6, vf7" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(crossProduct) : "memory");
    return crossProduct;
}

f32 vu0Dot(void *lhs, void *rhs) {
    f32 dot;

    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(lhs) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(rhs) : "memory");
    __asm__ __volatile__("vmul.xyzw vf1, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vaddy.x vf1, vf1, vf1y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf1, vf1, vf1z" : : : "memory");
    __asm__ __volatile__("vaddw.x vf1, vf1, vf1w" : : : "memory");
    __asm__ __volatile__("qmfc2.ni $8, vf1\n\tmtc1 $8, %0" : "=f"(dot) :  : "$8", "memory");
    return dot;
}

INCLUDE_ASM("asm/main/nonmatchings/tu146", func_0020EAE0);

INCLUDE_ASM("asm/main/nonmatchings/tu146", func_0020EB20);
