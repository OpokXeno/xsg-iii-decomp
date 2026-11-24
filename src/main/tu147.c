#include "common.h"
#include "shared.h"


/* Quaternion of a rotation by angle about the X axis: w = cos(angle / 2), x = sin(angle / 2). */
AlignedVector4 *quaternionFromAngleX(AlignedVector4 *quaternion, f32 angle) {
    __asm__ __volatile__("li.s $f8, 0.5\n\t"
                         "mul.s $f8, %1, $f8\n\t"
                         "mfc1 $2, $f8\n\t"
                         "qmtc2.ni $2, vf5\n\t"
                         "vmove.xyz vf7, vf0\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0xE8\n\t"
                         "vmulx.w vf7, vf0, vf1x\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0x20\n\t"
                         "vaddx.x vf7, vf0, vf1x\n\t"
                         "sqc2 vf7, 0(%0)"
                         :
                         : "r"(quaternion), "f"(angle)
                         : "$2", "$f8", "memory");
    return quaternion;
}

/* Quaternion of a rotation by angle about the Y axis: w = cos(angle / 2), y = sin(angle / 2). */
AlignedVector4 *quaternionFromAngleY(AlignedVector4 *quaternion, f32 angle) {
    __asm__ __volatile__("li.s $f8, 0.5\n\t"
                         "mul.s $f8, %1, $f8\n\t"
                         "mfc1 $2, $f8\n\t"
                         "qmtc2.ni $2, vf5\n\t"
                         "vmove.xyz vf7, vf0\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0xE8\n\t"
                         "vmulx.w vf7, vf0, vf1x\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0x20\n\t"
                         "vaddx.y vf7, vf0, vf1x\n\t"
                         "sqc2 vf7, 0(%0)"
                         :
                         : "r"(quaternion), "f"(angle)
                         : "$2", "$f8", "memory");
    return quaternion;
}

/* Quaternion of a rotation by angle about the Z axis: w = cos(angle / 2), z = sin(angle / 2). */
AlignedVector4 *quaternionFromAngleZ(AlignedVector4 *quaternion, f32 angle) {
    __asm__ __volatile__("li.s $f8, 0.5\n\t"
                         "mul.s $f8, %1, $f8\n\t"
                         "mfc1 $2, $f8\n\t"
                         "qmtc2.ni $2, vf5\n\t"
                         "vmove.xyz vf7, vf0\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0xE8\n\t"
                         "vmulx.w vf7, vf0, vf1x\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0x20\n\t"
                         "vaddx.z vf7, vf0, vf1x\n\t"
                         "sqc2 vf7, 0(%0)"
                         :
                         : "r"(quaternion), "f"(angle)
                         : "$2", "$f8", "memory");
    return quaternion;
}

/* Shortest-arc quaternion that turns the unit vector from into to: xyz = cross(from, to) / |from + to| and
 * w = |from + to| / 2. When from and to are opposite (|from + to| under 0.01) it stores (0, 1, 0, 0) and returns 0. */
int quaternionFromToRotation(AlignedVector4 *quaternion, AlignedVector4 *fromVector, AlignedVector4 *toVector) {
    f32 sumLength;
    /* kept: the four-float array decides the emitted code; a scalar does not match. Only [0] is used. */
    f32 magnitude[4];

    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\t"
                         "lqc2 vf2, 0(%2)\n\t"
                         "vmul.xyz vf3, vf1, vf2\n\t"
                         "vaddy.x vf3, vf3, vf3y\n\t"
                         "vaddz.x vf3, vf3, vf3z\n\t"
                         "vaddw.x vf3, vf3, vf0w\n\t"
                         "vadd.x vf3, vf3, vf3\n\t"
                         "vsqrt Q, vf3x\n\t"
                         "vopmula.xyz ACC, vf1, vf2\n\t"
                         "vopmsub.xyz vf5, vf2, vf1\n\t"
                         "vwaitq\n\t"
                         "vaddq.x vf3, vf0, Q\n\t"
                         "qmfc2.ni $8, vf3\n\t"
                         "mtc1 $8, %0"
                         : "=f"(sumLength)
                         : "r"(fromVector), "r"(toVector)
                         : "$8", "memory");
    magnitude[0] = __builtin_fabsf(sumLength);
    if (magnitude[0] < 0.01f) {
        __asm__ __volatile__("vsub.xyzw vf4, vf0, vf0\n\t"
                             "vaddw.y vf4, vf0, vf0w\n\t"
                             "sqc2 vf4, 0(%0)"
                             :
                             : "r"(quaternion)
                             : "memory");
        return 0;
    }
    __asm__ __volatile__("viaddi vi1, vi0, 8\n\t"
                         "vmfir.w vf6, vi1\n\t"
                         "vdiv Q, vf0w, vf3x\n\t"
                         "vitof4.w vf6, vf6\n\t"
                         "vmulx.w vf5, vf6, vf3x\n\t"
                         "vwaitq\n\t"
                         "vmulq.xyz vf5, vf5, Q\n\t"
                         "sqc2 vf5, 0(%0)"
                         :
                         : "r"(quaternion)
                         : "memory");
    return 1;
}

/* quaternion = left * q(axis, angle), where q(axis, angle) = (axis * sin(angle / 2), cos(angle / 2)). */
AlignedVector4 *quaternionMulAxisAngle(AlignedVector4 *quaternion, AlignedVector4 *left, AlignedVector4 *axis, f32 angle) {
    __asm__ __volatile__("li.s $f8, 0.5\n\t"
                         "mul.s $f8, %1, $f8\n\t"
                         "mfc1 $2, $f8\n\t"
                         "qmtc2.ni $2, vf5\n\t"
                         "lqc2 vf6, 0(%2)\n\t"
                         "lqc2 vf7, 0(%3)\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0xE8\n\t"
                         "vmulx.w vf8, vf0, vf1x\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0x20\n\t"
                         "vmulx.xyz vf8, vf7, vf1x\n\t"
                         "vopmula.xyz ACC, vf6, vf8\n\t"
                         "vopmsub.xyz vf3, vf8, vf6\n\t"
                         "vmul.xyzw vf4, vf6, vf8\n\t"
                         "vaddy.x vf4, vf4, vf4y\n\t"
                         "vaddz.x vf4, vf4, vf4z\n\t"
                         "vsubx.w vf7, vf4, vf4x\n\t"
                         "vmulw.xyz vf5, vf6, vf8w\n\t"
                         "vmulw.xyz vf6, vf8, vf6w\n\t"
                         "vadd.xyz vf7, vf3, vf5\n\t"
                         "vadd.xyz vf7, vf6, vf7\n\t"
                         "sqc2 vf7, 0(%0)"
                         :
                         : "r"(quaternion), "f"(angle), "r"(left), "r"(axis)
                         : "$2", "$f8", "memory");
    return quaternion;
}

AlignedVector4 *quatRotationColumnX(AlignedVector4 *rotationColumn, AlignedVector4 *quaternion) {
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

AlignedVector4 *quatRotationColumnY(AlignedVector4 *rotationColumn, AlignedVector4 *quaternion) {
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

AlignedVector4 *quatRotationColumnZ(AlignedVector4 *rotationColumn, AlignedVector4 *quaternion) {
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
