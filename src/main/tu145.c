#include "common.h"

void *vu0NormalizeXyzwTo(void *normalizedVector, void *inputVector) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("vmul.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vaddw.x vf2, vf2, vf2w" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyzw vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(normalizedVector) : "memory");
    return normalizedVector;
}

void *vu0NormalizeXyzTo(void *normalizedVector, void *inputVector) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("vmul.xyz vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(normalizedVector) : "memory");
    return normalizedVector;
}

void *vu0NormalizeNegatedXyzTo(void *normalizedVector, void *inputVector) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("vsub.xyz vf1, vf0, vf1" : : : "memory");
    __asm__ __volatile__("vmul.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vaddw.x vf2, vf2, vf2w" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyzw vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(normalizedVector) : "memory");
    return normalizedVector;
}
