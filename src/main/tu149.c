#include "common.h"

#include "shared.h"
/* Rotates the input vector about the X axis by the angle scale into the output vector. The half angle goes to the VU0 sin/cos
   micro routines at 0xE8 and 0x20, as in vu0RotateCurrentX. */
Vector4 *vu0RotateVectorAboutX(Vector4 *outVector, Vector4 *inputVector, float scale) {
    f32 halfAngle = 0.5f;

    halfAngle *= scale;
    __asm__ __volatile__("mfc1 $2, %0\n\tqmtc2 $2, $vf5"
                         :
                         : "f"(halfAngle)
                         : "$2", "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "memory");
    __asm__ __volatile__("lqc2 vf6, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("vmove.yzw vf7, vf0" : : : "memory");
    __asm__ __volatile__("vaddw.x vf7, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0xE8" : : : "memory");
    __asm__ __volatile__("vmulx.w vf8, vf0, vf1x" : : : "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmulx.xyz vf8, vf7, vf1x" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf6, vf8" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf3, vf8, vf6" : : : "memory");
    __asm__ __volatile__("vmul.xyzw vf4, vf6, vf8" : : : "memory");
    __asm__ __volatile__("vaddy.x vf4, vf4, vf4y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf4, vf4, vf4z" : : : "memory");
    __asm__ __volatile__("vsubx.w vf7, vf4, vf4x" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf5, vf6, vf8w" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf6, vf8, vf6w" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf3, vf5" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf6, vf7" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(outVector) : "memory");
    return outVector;
}

/* Rotates the input vector about the Y axis by the angle scale into the output vector (same VU0 sequence as the X rotation). */
Vector4 *vu0RotateVectorAboutY(Vector4 *outVector, Vector4 *inputVector, float scale) {
    f32 halfAngle = 0.5f;

    halfAngle *= scale;
    __asm__ __volatile__("mfc1 $2, %0\n\tqmtc2 $2, $vf5"
                         :
                         : "f"(halfAngle)
                         : "$2", "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "memory");
    __asm__ __volatile__("lqc2 vf6, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("vmove.xzw vf7, vf0" : : : "memory");
    __asm__ __volatile__("vaddw.y vf7, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0xE8" : : : "memory");
    __asm__ __volatile__("vmulx.w vf8, vf0, vf1x" : : : "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmulx.xyz vf8, vf7, vf1x" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf6, vf8" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf3, vf8, vf6" : : : "memory");
    __asm__ __volatile__("vmul.xyzw vf4, vf6, vf8" : : : "memory");
    __asm__ __volatile__("vaddy.x vf4, vf4, vf4y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf4, vf4, vf4z" : : : "memory");
    __asm__ __volatile__("vsubx.w vf7, vf4, vf4x" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf5, vf6, vf8w" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf6, vf8, vf6w" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf3, vf5" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf6, vf7" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(outVector) : "memory");
    return outVector;
}

/* Rotates the input vector about the Z axis by the angle scale into the output vector (same VU0 sequence as the X rotation). */
Vector4 *vu0RotateVectorAboutZ(Vector4 *outVector, Vector4 *inputVector, float scale) {
    f32 halfAngle = 0.5f;

    halfAngle *= scale;
    __asm__ __volatile__("mfc1 $2, %0\n\tqmtc2 $2, $vf5"
                         :
                         : "f"(halfAngle)
                         : "$2", "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7", "memory");
    __asm__ __volatile__("lqc2 vf6, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("vmove.xyw vf7, vf0" : : : "memory");
    __asm__ __volatile__("vaddw.z vf7, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0xE8" : : : "memory");
    __asm__ __volatile__("vmulx.w vf8, vf0, vf1x" : : : "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmulx.xyz vf8, vf7, vf1x" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf6, vf8" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf3, vf8, vf6" : : : "memory");
    __asm__ __volatile__("vmul.xyzw vf4, vf6, vf8" : : : "memory");
    __asm__ __volatile__("vaddy.x vf4, vf4, vf4y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf4, vf4, vf4z" : : : "memory");
    __asm__ __volatile__("vsubx.w vf7, vf4, vf4x" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf5, vf6, vf8w" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf6, vf8, vf6w" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf3, vf5" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf7, vf6, vf7" : : : "memory");
    __asm__ __volatile__("sqc2 vf7, 0(%0)" : : "r"(outVector) : "memory");
    return outVector;
}

f32 acosf(f32 value);

/* Spherical interpolation from the start vector towards the end vector by blend into the output vector, normalized. Returns 0 when
   the two vectors already match (dot >= 1), otherwise 1. The dot product includes w. */
int vu0QuaternionSlerp(AlignedVector4 *outVector, AlignedVector4 *startVector, AlignedVector4 *endVector, f32 blend) {
    AlignedVector4 nearTarget;
    AlignedVector4 product;
    f32 angle;
    f32 dot;

    /* kept: dot starts from the unset product.x; 0.0f changes the emitted code. */
    dot = product.x;
    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\t"
                         "lqc2 vf2, 0(%2)\n\t"
                         "li.s $f0, 0.0\n\t"
                         "vmul.xyzw vf3, vf1, vf2\n\t"
                         "vaddy.x vf3, vf3, vf3y\n\t"
                         "vaddz.x vf3, vf3, vf3z\n\t"
                         "vaddw.x vf3, vf3, vf3w\n\t"
                         "qmfc2 $2, vf3\n\t"
                         "mtc1 $2, %0\n\t"
                         "nop\n\t"
                         "c.le.s $f0, %0\n\t"
                         "nop\n\t"
                         "bc1t 1f\n\t"
                         "vsubw.x vf4, vf0, vf0w\n\t"
                         "vmulx.xyzw vf2, vf2, vf4x\n\t"
                         "1:\n\t"
                         "sqc2 vf2, 0(%3)"
                         : "+f"(dot)
                         : "r"(startVector), "r"(endVector), "r"(&nearTarget)
                         : "$1", "$2", "$f0");
    if (dot >= 1.0f) {
        return 0;
    }
    angle = acosf(dot);
    __asm__ __volatile__("mfc1 $3, %1\n\tmfc1 $2, %0\n\tqmtc2 $2, $vf5\n\tqmtc2 $3, $vf6"
                         :
                         : "f"(angle), "f"(blend)
                         : "$2", "$3", "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vdiv Q, vf0w, vf1x" : : : "memory");
    __asm__ __volatile__("vaddw.x vf4, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vsubx.x vf4, vf4, vf6x" : : : "memory");
    __asm__ __volatile__("vmulx.x vf4, vf4, vf5x" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmove.x vf7, vf1" : : : "memory");
    __asm__ __volatile__("vmulx.x vf4, vf5, vf6x" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmove.x vf8, vf1" : : : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(startVector) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(&nearTarget) : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.x vf4, vf7, Q" : : : "memory");
    __asm__ __volatile__("vmulq.x vf5, vf8, Q" : : : "memory");
    __asm__ __volatile__("vmulx.xyzw vf2, vf2, vf4x" : : : "memory");
    __asm__ __volatile__("vmulx.xyzw vf3, vf3, vf5x" : : : "memory");
    __asm__ __volatile__("vadd.xyzw vf1, vf2, vf3" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(outVector) : "memory");
    return 1;
}

/* Spherical interpolation from the start vector towards the end vector by blend into the output vector, normalized (dot product on x, y
   and z only). Returns 0 when the two vectors already match, otherwise 1. */
int vu0VectorSlerp(AlignedVector4 *outVector, AlignedVector4 *startVector, AlignedVector4 *endVector, f32 blend) {
    AlignedVector4 nearTarget;
    AlignedVector4 product;
    f32 angle;
    f32 dot;

    dot = product.x;
    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\t"
                         "lqc2 vf2, 0(%2)\n\t"
                         "li.s $f0, 0.0\n\t"
                         "vmul.xyz vf3, vf1, vf2\n\t"
                         "vaddy.x vf3, vf3, vf3y\n\t"
                         "vaddz.x vf3, vf3, vf3z\n\t"
                         "qmfc2 $2, vf3\n\t"
                         "mtc1 $2, %0\n\t"
                         "nop\n\t"
                         "c.le.s $f0, %0\n\t"
                         "sqc2 vf2, 0(%3)"
                         : "+f"(dot)
                         : "r"(startVector), "r"(endVector), "r"(&nearTarget)
                         : "$1", "$2", "$f0");
    if (dot >= 1.0f) {
        return 0;
    }
    /* kept: the empty test decides the emitted order. */
    if (dot <= -1.0) {
    }
    angle = acosf(dot);
    __asm__ __volatile__("mfc1 $3, %1\n\tmfc1 $2, %0\n\tqmtc2 $2, $vf5\n\tqmtc2 $3, $vf6"
                         :
                         : "f"(angle), "f"(blend)
                         : "$2", "$3", "memory");
    __asm__ __volatile__("vmove.x vf4, vf5" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vdiv Q, vf0w, vf1x" : : : "memory");
    __asm__ __volatile__("vaddw.x vf4, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("vsubx.x vf4, vf4, vf6x" : : : "memory");
    __asm__ __volatile__("vmulx.x vf4, vf4, vf5x" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmove.x vf7, vf1" : : : "memory");
    __asm__ __volatile__("vmulx.x vf4, vf5, vf6x" : : : "memory");
    __asm__ __volatile__("vcallms 0x20" : : : "memory");
    __asm__ __volatile__("vmove.x vf8, vf1" : : : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(startVector) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(&nearTarget) : "memory");
    __asm__ __volatile__("vmove.w vf1, vf0" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.x vf4, vf7, Q" : : : "memory");
    __asm__ __volatile__("vmulq.x vf5, vf8, Q" : : : "memory");
    __asm__ __volatile__("vmulx.xyz vf2, vf2, vf4x" : : : "memory");
    __asm__ __volatile__("vmulx.xyz vf3, vf3, vf5x" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf1, vf2, vf3" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(outVector) : "memory");
    return 1;
}
