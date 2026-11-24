#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu147", func_0020EC38);

INCLUDE_ASM("asm/main/nonmatchings/tu147", func_0020EC78);

INCLUDE_ASM("asm/main/nonmatchings/tu147", func_0020ECB8);

INCLUDE_ASM("asm/main/nonmatchings/tu147", func_0020ECF8);

INCLUDE_ASM("asm/main/nonmatchings/tu147", func_0020ED98);

void *quatRotationColumnX(void *rotationColumn, void *quaternion) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(quaternion) : "memory");
    __asm__ __volatile__("vadd.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddw.x vf7, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vmove.yzw vf7, vf0" : : : "memory");
    __asm__ __volatile__("vmul.yz vf3, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vmulx.yz vf4, vf2, vf1x" : : : "memory");
    __asm__ __volatile__("vmulw.yz vf5, vf2, vf1w" : : : "memory");
    __asm__ __volatile__("vadd.yz vf7, vf7, vf4" : : : "memory");
    __asm__ __volatile__("vaddz.y vf7, vf7, vf5z" : : : "memory");
    __asm__ __volatile__("vsuby.z vf7, vf7, vf5y" : : : "memory");
    __asm__ __volatile__("vsuby.x vf7, vf7, vf3y" : : : "memory");
    __asm__ __volatile__("vsubz.x vf7, vf7, vf3z" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(rotationColumn) : "memory");
    return rotationColumn;
}

void *quatRotationColumnY(void *rotationColumn, void *quaternion) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(quaternion) : "memory");
    __asm__ __volatile__("vadd.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddw.y vf7, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vmove.xzw vf7, vf0" : : : "memory");
    __asm__ __volatile__("vmul.xz vf3, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vmuly.xz vf4, vf2, vf1y" : : : "memory");
    __asm__ __volatile__("vmulw.xz vf5, vf2, vf1w" : : : "memory");
    __asm__ __volatile__("vadd.xz vf7, vf7, vf4" : : : "memory");
    __asm__ __volatile__("vaddx.z vf7, vf7, vf5x" : : : "memory");
    __asm__ __volatile__("vsubz.x vf7, vf7, vf5z" : : : "memory");
    __asm__ __volatile__("vsubx.y vf7, vf7, vf3x" : : : "memory");
    __asm__ __volatile__("vsubz.y vf7, vf7, vf3z" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(rotationColumn) : "memory");
    return rotationColumn;
}

void *quatRotationColumnZ(void *rotationColumn, void *quaternion) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(quaternion) : "memory");
    __asm__ __volatile__("vadd.xyzw vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddw.z vf7, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vmove.xyw vf7, vf0" : : : "memory");
    __asm__ __volatile__("vmul.xy vf3, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vmulz.xy vf4, vf2, vf1z" : : : "memory");
    __asm__ __volatile__("vmulw.xy vf5, vf2, vf1w" : : : "memory");
    __asm__ __volatile__("vadd.xy vf7, vf7, vf4" : : : "memory");
    __asm__ __volatile__("vaddy.x vf7, vf7, vf5y" : : : "memory");
    __asm__ __volatile__("vsubx.y vf7, vf7, vf5x" : : : "memory");
    __asm__ __volatile__("vsubx.z vf7, vf7, vf3x" : : : "memory");
    __asm__ __volatile__("vsuby.z vf7, vf7, vf3y" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(rotationColumn) : "memory");
    return rotationColumn;
}
