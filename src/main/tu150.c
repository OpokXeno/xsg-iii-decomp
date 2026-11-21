#include "common.h"

void *vu0QuatToMatrix(void *matrix, void *quaternion) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(quaternion) : "memory");
    __asm__ __volatile__("vadd.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddw.xyzw vf9, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf8, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vmulz.xyzw vf5, vf2, vf1z" : : : "memory");
    __asm__ __volatile__("vmuly.xyzw vf4, vf2, vf1y" : : : "memory");
    __asm__ __volatile__("vmulx.xyzw vf3, vf2, vf1x" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf7, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf6, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vaddz.y vf10, vf4, vf5z" : : : "memory");
    __asm__ __volatile__("vaddx.z vf11, vf5, vf3x" : : : "memory");
    __asm__ __volatile__("vaddy.x vf12, vf3, vf4y" : : : "memory");
    __asm__ __volatile__("vaddw.x vf8, vf5, vf4w" : : : "memory");
    __asm__ __volatile__("vsubw.x vf7, vf4, vf5w" : : : "memory");
    __asm__ __volatile__("vsuby.x vf6, vf9, vf10y" : : : "memory");
    __asm__ __volatile__("vsubw.y vf8, vf5, vf3w" : : : "memory");
    __asm__ __volatile__("vsubz.y vf7, vf9, vf11z" : : : "memory");
    __asm__ __volatile__("vaddw.y vf6, vf3, vf5w" : : : "memory");
    __asm__ __volatile__("vsubx.z vf8, vf9, vf12x" : : : "memory");
    __asm__ __volatile__("vaddw.z vf7, vf4, vf3w" : : : "memory");
    __asm__ __volatile__("vsubw.z vf6, vf3, vf4w" : : : "memory");
    __asm__ __volatile__("sqc2 vf0, 48(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("sqc2 vf8, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("sqc2 vf7, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("sqc2 vf6, 0(%0)" : : "r"(matrix) : "memory");
    return matrix;
}

INCLUDE_ASM("asm/main/nonmatchings/tu150", func_0020F2E8);

INCLUDE_ASM("asm/main/nonmatchings/tu150", func_0020F598);
