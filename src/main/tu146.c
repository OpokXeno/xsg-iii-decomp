#include "common.h"
#include "shared.h"

/* Quaternion product of lhs and rhs on VU0, stored in crossProduct: w = lhs.w * rhs.w - dot(lhs.xyz, rhs.xyz); xyz is the
 * cross product plus lhs.w * rhs.xyz + rhs.w * lhs.xyz. */
AlignedVector4 *vu0CrossWithWeights(AlignedVector4 *crossProduct, AlignedVector4 *lhs, AlignedVector4 *rhs) {
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

/* Dot product of lhs and rhs on VU0 (xyzw), returned as a float. */
f32 vu0Dot(AlignedVector4 *lhs, AlignedVector4 *rhs) {
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

/* Quaternion from axis and angle on VU0: w = cos(angle / 2), xyz = axis * sin(angle / 2). The VU0 micro
 * routines at 0xE8 and 0x20 return the cosine and the sine of vf4x in vf1x. */
AlignedVector4 *vu0AxisAngleToQuaternion(AlignedVector4 *quaternion, AlignedVector4 *axis, f32 angle) {
    __asm__ __volatile__("li.s $f8, 0.5\n\t"
                         "mul.s $f8, %1, $f8\n\t"
                         "mfc1 $2, $f8\n\t"
                         "qmtc2.ni $2, vf5\n\t"
                         "lqc2 vf6, 0(%2)\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0xE8\n\t"
                         "vmulx.w vf7, vf0, vf1x\n\t"
                         "vmove.x vf4, vf5\n\t"
                         "vcallms 0x20\n\t"
                         "vmulx.xyz vf7, vf6, vf1x\n\t"
                         "sqc2 vf7, 0(%0)"
                         :
                         : "r"(quaternion), "f"(angle), "r"(axis)
                         : "$2", "$f8", "memory");
    return quaternion;
}

f32 acosf(f32 value);

/* Axis and angle of a unit quaternion: angle = 2 * acos(w); the axis is xyz divided by its length. A zero
 * length gives the axis (0, 1, 0) and angle 0. */
void quaternionToAxisAngle(AlignedVector4 *axis, f32 *angle, AlignedVector4 *quaternion) {
    f32 twiceAcosW = acosf(quaternion->w) * 2.0f;
    f32 xyzLength = __builtin_sqrtf(quaternion->x * quaternion->x + quaternion->y * quaternion->y +
                                    quaternion->z * quaternion->z);

    if (xyzLength != 0.0f) {
        axis->x = quaternion->x / xyzLength;
        axis->y = quaternion->y / xyzLength;
        axis->z = quaternion->z / xyzLength;
        axis->w = 1.0f;
        *angle = twiceAcosW;
    } else {
        axis->x = 0.0f;
        axis->y = 1.0f;
        axis->z = 0.0f;
        axis->w = 1.0f;
        *angle = 0.0f;
    }
}
