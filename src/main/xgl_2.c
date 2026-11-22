#include "common.h"
#include "shared.h"
#include "main/xgl_2.h"

#include "quadword.h"

/* Row-major 4x4 float matrix. */
void xglEulerFromRotation(Vector4 *euler, Vector4 *rotation);
void xglVectorScale(Vector4 *scaledVector, const Vector4 *vector, float scale);
void xglVectorScaleXYZ(Vector4 *scaledVector, const Vector4 *vector, float scale);
void xglVectorInner(float *dotProduct, const Vector4 *left, const Vector4 *right);
void xglVectorOuter(Vector4 *crossProduct, const Vector4 *first, const Vector4 *second);
void xglVectorLength(float *length, const Vector4 *vector);
float xglPointLength(Vector4 *from, Vector4 *to);
float xglPointLengthXZ(Vector4 *from, Vector4 *to);
void xglVectorNormal(Vector4 *normalizedVector, const Vector4 *vector);
void xglVectorMulMat(Vector4 *transformedVector, Matrix4 matrix, const Vector4 *vector);
void xglRotTransPersN(ScreenVector4 *screenPoints, Matrix4 matrix, const Vector4 *points, int pointCount, int cameraId);
void xglMatrixMul(Matrix4 productMatrix, Matrix4 left, Matrix4 right);
void xglMatrixInverse(Matrix *inverseMatrix, Matrix *matrix);
void xglMatrixTrans(Matrix *translatedMatrix, Matrix *matrix, const Vector4 *translation);
void xglMatrixStackUnit(void);
void xglMatrixStackMul(Matrix4 matrix);
void xglMatrixStackInverse(void);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackSave(Matrix4 matrix);
void xglMatrixStackLoad(Matrix4 matrix);
void xglMatrixStackPush(void);
void xglMatrixStackPop(u32 popDepth);
void xglMatrixStackMulVector(Vector4 *transformedVector, const Vector4 *vector);
void xglMatrixStackRTPS(ScreenVector4 *screenPoint, const Vector4 *point, Vector4 *screenScale, Vector4 *screenOffset);
union Matrix;

#include "vu0_macro.h"

struct XglCamera;
struct XglCamera *xglStudioGetCamera2(int camera_index);
float asinf(float value);
float atan2f(float y, float x);
extern char gXglMicroMatrixStackUnit[];
extern char gXglMicroMatrixStackMul[];
extern char gXglMicroMatrixStackReverse[];
extern char gXglMicroMatrixStackInverse[];
extern char gXglMicroMatrixStackScale[];
extern char gXglMicroMatrixStackTrans[];
extern char gXglMicroMatrixStackRotV[];
extern char gXglMicroMatrixStackRotX[];
extern char gXglMicroMatrixStackRotY[];
extern char gXglMicroMatrixStackRotZ[];
extern char gXglMicroMatrixStackPushUnit[];
extern char gXglMicroMatrixStackPush[];
extern char gXglMicroMatrixStackPop[];
extern char gXglMicroMatrixStackMulVector[];
extern char gXglMicroMatrixStackRTPS[];
/* Identity matrix and the unit matrix with w = 1 in every row, copied by xglMatrixUnit / xglMatrixUnit4s. */
extern Matrix4 xglIdentityMatrix;
extern Matrix4 xglUnit4sMatrix;
/* VU0 resident microprogram entries of sine (D_20) and cosine (D_E8), as xglSin / xglCos call them. */
extern char D_20[];
extern char D_E8[];

/* xglSin / xglCos inlined: result = the micro entry evaluated on the angle bits in vf4. The rotation
 * functions move the angle to a GPR once and share it between both calls. */
#define XglMicroSinCos(result, micro, angleBits) \
    __asm__ __volatile__( \
        "ctc2.i %1, $vi27\n\t" \
        "vnop\n\t" \
        "qmtc2.ni %2, $vf4\n\t" \
        "vcallmsr $vi27\n\t" \
        "qmfc2.i %0, $vf1" \
        : "=r"(result) : "r"((u32)(micro) >> 3), "r"(angleBits))

void xglEulerFromRotation(Vector4 *euler, Vector4 *rotation)
{
    Matrix4 matrix;

    xglMatrixStackUnit();
    xglMatrixStackRotZ(rotation->z);
    xglMatrixStackRotY(rotation->y);
    xglMatrixStackRotX(rotation->x);
    xglMatrixStackSave(matrix);

    if (matrix[2][0] < 0.0001f && -0.0001f < matrix[2][0] &&
        matrix[2][2] < 0.0001f && -0.0001f < matrix[2][2]) {
        *euler = *rotation;
    } else {
        euler->x = -asinf(matrix[2][1]);
        euler->y = atan2f(matrix[2][0], matrix[2][2]);
        euler->z = atan2f(matrix[0][1], matrix[1][1]);
    }
}

float xglAtan2(float y, float x)
{
    float ySquared;
    float angle = 0.0028662258f;
    int swapped = 0;
    int quadrant = 0;
    int *yBits = (int *)&y;

    if (y == 0.0f) {
        if (0.0f <= x) {
            return 0.0f;
        }
        if (*yBits & 0x80000000) {
            return -3.1415927f;
        }
        return 3.1415927f;
    }
    if (x == 0.0f) {
        if (y < 0.0f) {
            return -1.5707964f;
        }
        return 1.5707964f;
    }
    if (0.0f <= y) {
        if (0.0f <= x) {
            quadrant = 0;
        } else {
            quadrant = 1;
        }
    } else {
        if (0.0f <= x) {
            quadrant = 2;
        } else {
            quadrant = 3;
        }
    }
    y = __builtin_fabsf(y);
    x = __builtin_fabsf(x);
    if (y > x) {
        y = x / y;
        swapped = 1;
    } else {
        y = y / x;
    }

    ySquared = y * y;
    angle = angle * ySquared;
    angle = angle - 0.016165737f;
    angle = angle * ySquared;
    angle = angle + 0.042909615f;
    angle = angle * ySquared;
    angle = angle - 0.075289637f;
    angle = angle * ySquared;
    angle = angle + 0.10656264f;
    angle = angle * ySquared;
    angle = angle - 0.14208899f;
    angle = angle * ySquared;
    angle = angle + 0.19993551f;
    angle = angle * ySquared;
    angle = angle - 0.33333147f;
    angle = angle * ySquared;
    angle = angle + 1.0f;
    angle = angle * y;

    if (swapped) {
        angle = 1.5707964f - angle;
    }
    if (quadrant == 1) {
        angle = 3.1415927f - angle;
    }
    if (quadrant == 2) {
        angle = -angle;
    }
    if (quadrant == 3) {
        angle = angle - 3.1415927f;
    }
    return angle;
}

void xglMemCopy64(void *destination, const void *source, int count)
{
    register void *dst = destination;
    register const void *src = source;
    register int remaining = count;

    QuadwordCopyMatrixArray(dst, src, remaining);
}

void xglMemCopy64b(void *destination, const void *source, int count)
{
    __asm__ __volatile__(
        "1:\n\t"
        "lq $2,0(%1)\n\t"
        "sq $2,0(%0)\n\t"
        "lq $2,-16(%1)\n\t"
        "sq $2,-16(%0)\n\t"
        "lq $2,-32(%1)\n\t"
        "sq $2,-32(%0)\n\t"
        "lq $2,-48(%1)\n\t"
        "sq $2,-48(%0)\n\t"
        "addi %2,%2,-1\n\t"
        "addi %0,%0,-64\n\t"
        "addi %1,%1,-64\n\t"
        "bne $0,%2,1b\n\t"
        : "+r"(destination), "+r"(source), "+r"(count)
        :
        : "$2", "memory");
}

void xglMemCopy16(void *destination, const void *source, int count)
{
    register void *dst = destination;
    register const void *src = source;
    register int remaining = count;

    QuadwordCopyArray(dst, src, remaining);
}

void xglMemCopy16b(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "lq $2, 0(%1)\n\t"
        "sq $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, -16\n\t"
        "addi %1, %1, -16\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglMemCopy8(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "ld $2, 0(%1)\n\t"
        "sd $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, 8\n\t"
        "addi %1, %1, 8\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglMemCopy8b(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "ld $2, 0(%1)\n\t"
        "sd $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, -8\n\t"
        "addi %1, %1, -8\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglMemCopy4(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "lw $2, 0(%1)\n\t"
        "sw $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, 4\n\t"
        "addi %1, %1, 4\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglMemCopy4b(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "lw $2, 0(%1)\n\t"
        "sw $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, -4\n\t"
        "addi %1, %1, -4\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglMemCopy2(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "lh $2, 0(%1)\n\t"
        "sh $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, 2\n\t"
        "addi %1, %1, 2\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglMemCopy2b(u8 *destination, const u8 *source, int count)
{
    register u8 *dst = destination;
    register const u8 *src = source;
    register int n = count;
    __asm__ __volatile__(
        "1:\n\t"
        "lh $2, 0(%1)\n\t"
        "sh $2, 0(%0)\n\t"
        "addi %2, %2, -1\n\t"
        "addi %0, %0, -2\n\t"
        "addi %1, %1, -2\n\t"
        "bne $0, %2, 1b"
        : "+r"(dst), "+r"(src), "+r"(n) : : "$2", "memory");
}

void xglVectorDiv(Vector4 *quotientVector, const Vector4 *left, const Vector4 *right)
{
    quotientVector->x = left->x / right->x;
    quotientVector->y = left->y / right->y;
    quotientVector->z = left->z / right->z;
    quotientVector->w = left->w / right->w;
}

void xglVectorDivXYZ(Vector4 *quotientVector, const Vector4 *left, const Vector4 *right)
{
    quotientVector->x = left->x / right->x;
    quotientVector->y = left->y / right->y;
    quotientVector->z = left->z / right->z;
    quotientVector->w = left->w;
}

void xglVectorMulAdd(Vector4 *productSum, const Vector4 *multiplicand, const Vector4 *multiplier, const Vector4 *addend)
{
    productSum->x = multiplicand->x * multiplier->x + addend->x;
    productSum->y = multiplicand->y * multiplier->y + addend->y;
    productSum->z = multiplicand->z * multiplier->z + addend->z;
    productSum->w = multiplicand->w * multiplier->w + addend->w;
}

void xglVectorMulAddXYZ(Vector4 *productSum, const Vector4 *multiplicand, const Vector4 *multiplier, const Vector4 *addend)
{
    productSum->x = multiplicand->x * multiplier->x + addend->x;
    productSum->y = multiplicand->y * multiplier->y + addend->y;
    productSum->z = multiplicand->z * multiplier->z + addend->z;
    productSum->w = multiplicand->w;
}

void xglVectorMulSub(Vector4 *productDifference, const Vector4 *multiplicand, const Vector4 *multiplier, const Vector4 *subtrahend)
{
    productDifference->x = multiplicand->x * multiplier->x - subtrahend->x;
    productDifference->y = multiplicand->y * multiplier->y - subtrahend->y;
    productDifference->z = multiplicand->z * multiplier->z - subtrahend->z;
    productDifference->w = multiplicand->w * multiplier->w - subtrahend->w;
}

void xglVectorMulSubXYZ(Vector4 *productDifference, const Vector4 *multiplicand, const Vector4 *multiplier, const Vector4 *subtrahend)
{
    productDifference->x = multiplicand->x * multiplier->x - subtrahend->x;
    productDifference->y = multiplicand->y * multiplier->y - subtrahend->y;
    productDifference->z = multiplicand->z * multiplier->z - subtrahend->z;
    productDifference->w = multiplicand->w;
}

void xglVectorDivAdd(Vector4 *quotientSum, const Vector4 *left, const Vector4 *right, const Vector4 *addend)
{
    quotientSum->x = left->x / right->x + addend->x;
    quotientSum->y = left->y / right->y + addend->y;
    quotientSum->z = left->z / right->z + addend->z;
    quotientSum->w = left->w / right->w + addend->w;
}

void xglVectorDivAddXYZ(Vector4 *quotientSum, const Vector4 *left, const Vector4 *right, const Vector4 *addend)
{
    quotientSum->x = left->x / right->x + addend->x;
    quotientSum->y = left->y / right->y + addend->y;
    quotientSum->z = left->z / right->z + addend->z;
    quotientSum->w = left->w;
}

void xglVectorDivSub(Vector4 *quotientDifference, const Vector4 *left, const Vector4 *right, const Vector4 *subtrahend)
{
    quotientDifference->x = left->x / right->x - subtrahend->x;
    quotientDifference->y = left->y / right->y - subtrahend->y;
    quotientDifference->z = left->z / right->z - subtrahend->z;
    quotientDifference->w = left->w / right->w - subtrahend->w;
}

void xglVectorDivSubXYZ(Vector4 *quotientDifference, const Vector4 *left, const Vector4 *right, const Vector4 *subtrahend)
{
    quotientDifference->x = left->x / right->x - subtrahend->x;
    quotientDifference->y = left->y / right->y - subtrahend->y;
    quotientDifference->z = left->z / right->z - subtrahend->z;
    quotientDifference->w = left->w;
}

void xglVectorScale(Vector4 *scaledVector, const Vector4 *vector, float scale)
{
    scaledVector->x = vector->x * scale;
    scaledVector->y = vector->y * scale;
    scaledVector->z = vector->z * scale;
    scaledVector->w = vector->w * scale;
}

void xglVectorScaleXYZ(Vector4 *scaledVector, const Vector4 *vector, float scale)
{
    scaledVector->x = vector->x * scale;
    scaledVector->y = vector->y * scale;
    scaledVector->z = vector->z * scale;
    scaledVector->w = vector->w;
}

void xglVectorScaleAdd(Vector4 *scaledSum, const Vector4 *vector, const Vector4 *addend, float scale)
{
    scaledSum->x = vector->x * scale + addend->x;
    scaledSum->y = vector->y * scale + addend->y;
    scaledSum->z = vector->z * scale + addend->z;
    scaledSum->w = vector->w * scale + addend->w;
}

void xglVectorScaleAddXYZ(Vector4 *scaledSum, const Vector4 *vector, const Vector4 *addend, float scale)
{
    scaledSum->x = vector->x * scale + addend->x;
    scaledSum->y = vector->y * scale + addend->y;
    scaledSum->z = vector->z * scale + addend->z;
    scaledSum->w = vector->w;
}

void xglVectorScaleSub(Vector4 *scaledDifference, const Vector4 *vector, const Vector4 *subtrahend, float scale)
{
    scaledDifference->x = vector->x * scale - subtrahend->x;
    scaledDifference->y = vector->y * scale - subtrahend->y;
    scaledDifference->z = vector->z * scale - subtrahend->z;
    scaledDifference->w = vector->w * scale - subtrahend->w;
}

void xglVectorScaleSubXYZ(Vector4 *scaledDifference, const Vector4 *vector, const Vector4 *subtrahend, float scale)
{
    scaledDifference->x = vector->x * scale - subtrahend->x;
    scaledDifference->y = vector->y * scale - subtrahend->y;
    scaledDifference->z = vector->z * scale - subtrahend->z;
    scaledDifference->w = vector->w;
}

void xglVectorInter(Vector4 *interpolated, const Vector4 *startVector, const Vector4 *endVector, float t)
{
    interpolated->x = (endVector->x - startVector->x) * t + startVector->x;
    interpolated->y = (endVector->y - startVector->y) * t + startVector->y;
    interpolated->z = (endVector->z - startVector->z) * t + startVector->z;
    interpolated->w = (endVector->w - startVector->w) * t + startVector->w;
}

void xglVectorInterXYZ(Vector4 *interpolated, const Vector4 *startVector, const Vector4 *endVector, float t)
{
    interpolated->x = (endVector->x - startVector->x) * t + startVector->x;
    interpolated->y = (endVector->y - startVector->y) * t + startVector->y;
    interpolated->z = (endVector->z - startVector->z) * t + startVector->z;
    interpolated->w = endVector->w;
}

void xglVectorInner(float *dotProduct, const Vector4 *left,
                    const Vector4 *right)
{
    *dotProduct = left->x * right->x;
    *dotProduct = *dotProduct + left->y * right->y;
    *dotProduct = *dotProduct + left->z * right->z;
}

float xglVectorInner4(const Vector4 *left, const Vector4 *right)
{
    return left->x * right->x + left->y * right->y + left->z * right->z + left->w * right->w;
}

void xglVectorOuter(Vector4 *crossProduct, const Vector4 *leftVector, const Vector4 *rightVector)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vopmula.xyz ACC, vf1, vf2\n\t"
        "vopmsub.xyz vf1, vf2, vf1\n\t"
        "sqc2 vf1, 0(%0)"
        : : "r"(crossProduct), "r"(leftVector), "r"(rightVector) : "memory");
}

void xglVectorOuterNormal(Vector4 *crossNormal, const Vector4 *leftVector, const Vector4 *rightVector)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(leftVector) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(rightVector) : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf1, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(crossNormal) : "memory");
}

void xglTriangleOuter(Vector4 *faceCross, const Vector4 *origin, const Vector4 *firstPoint, const Vector4 *secondPoint)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(origin) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(firstPoint) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(secondPoint) : "memory");
    __asm__ __volatile__("vsub.xyz vf1, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vsub.xyz vf3, vf3, vf2" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf1, vf3" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf1, vf3, vf1" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(faceCross) : "memory");
}

void xglTriangleNormal(Vector4 *faceNormal, const Vector4 *origin, const Vector4 *firstPoint, const Vector4 *secondPoint)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(origin) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(firstPoint) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(secondPoint) : "memory");
    __asm__ __volatile__("vsub.xyz vf1, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vsub.xyz vf3, vf3, vf2" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf1, vf3" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf1, vf3, vf1" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf2, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddz.x vf2, vf2, vf2z" : : : "memory");
    __asm__ __volatile__("vaddy.x vf2, vf2, vf2y" : : : "memory");
    __asm__ __volatile__("vrsqrt Q, vf0w, vf2x" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf1, vf1, Q" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(faceNormal) : "memory");
}

void xglVectorLength(float *length, const Vector4 *vector)
{
    __asm__ __volatile__(
        "lqc2 vf3, 0(%1)\n"
        "vmul.xyz vf3, vf3, vf3\n"
        "vaddy.x vf3, vf3, vf3y\n"
        "vaddz.x vf3, vf3, vf3z\n"
        "vsqrt Q, vf3x\n"
        "vwaitq\n"
        "vaddq.x vf4, vf0, Q\n"
        "qmfc2 $2, vf4\n"
        "sw $2, 0(%0)\n"
        :
        : "r"(length), "r"(vector)
        : "$2", "memory");
}

float xglPointLength(Vector4 *startPoint, Vector4 *endPoint)
{
    Vector4 *f = startPoint;
    __asm__ __volatile__(
        "lqc2 vf2, 0(%0)\n\t"
        "lqc2 vf3, 0(%1)\n\t"
        "vsub.xyz vf2xyz, vf2xyz, vf3xyz\n\t"
        "vmul.xyz vf3xyz, vf2xyz, vf2xyz\n\t"
        "vaddy.x vf3x, vf3x, vf3y\n\t"
        "vaddz.x vf3x, vf3x, vf3z\n\t"
        "vsqrt Q, vf3x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf4x, vf0x, Q\n\t"
        "qmfc2 $2, vf4\n\t"
        "mtc1 $2, $f0"
        : "+r"(f) : "r"(endPoint) : "$2", "$f0", "memory");
}

float xglPointLengthXZ(Vector4 *startPoint, Vector4 *endPoint)
{
    Vector4 *f = startPoint;
    __asm__ __volatile__(
        "lqc2 vf2, 0(%0)\n\t"
        "lqc2 vf3, 0(%1)\n\t"
        "vsub.xyz vf2xyz, vf2xyz, vf3xyz\n\t"
        "vmul.xyz vf3xyz, vf2xyz, vf2xyz\n\t"
        "vaddz.x vf3x, vf3x, vf3z\n\t"
        "vsqrt Q, vf3x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf4x, vf0x, Q\n\t"
        "qmfc2 $2, vf4\n\t"
        "mtc1 $2, $f0"
        : "+r"(f) : "r"(endPoint) : "$2", "$f0", "memory");
}

void xglVectorNormal(Vector4 *normalizedVector, const Vector4 *vector)
{
    __asm__ __volatile__(
        "lqc2 vf2, 0(%1)\n\t"
        "vmul.xyz vf3xyz, vf2xyz, vf2xyz\n\t"
        "vaddy.x vf3x, vf3x, vf3y\n\t"
        "vaddz.x vf3x, vf3x, vf3z\n\t"
        "vrsqrt Q, vf0w, vf3x\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf2xyz, vf2xyz, Q\n\t"
        "sqc2 vf2, 0(%0)"
        :
        : "r"(normalizedVector), "r"(vector)
        : "memory"
    );
}

/* Clamps each component of source to [minimum, maximum] into destination. */
void xglVectorClamp(Vector4 *destination, const Vector4 *source,
                    float minimum, float maximum)
{
    destination->x = source->x < minimum ? minimum
                    : source->x > maximum ? maximum : source->x;
    destination->y = source->y < minimum ? minimum
                    : source->y > maximum ? maximum : source->y;
    destination->z = source->z < minimum ? minimum
                    : source->z > maximum ? maximum : source->z;
    destination->w = source->w < minimum ? minimum
                    : source->w > maximum ? maximum : source->w;
}

/* Like xglVectorClamp, but copies the w component of source unchanged. */
void xglVectorClampXYZ(Vector4 *destination, const Vector4 *source,
                       float minimum, float maximum)
{
    destination->x = source->x < minimum ? minimum
                    : source->x > maximum ? maximum : source->x;
    destination->y = source->y < minimum ? minimum
                    : source->y > maximum ? maximum : source->y;
    destination->z = source->z < minimum ? minimum
                    : source->z > maximum ? maximum : source->z;
    destination->w = source->w;
}

void xglPlaneParameter(Vector4 *plane, const Vector4 *a, const Vector4 *b,
                       const Vector4 *c)
{
    Vector4 edge1;
    Vector4 edge2;

    Vu0SubVectorXYZ(&edge1, b, a);
    Vu0SubVectorXYZ(&edge2, c, b);
    xglVectorOuter(plane, &edge1, &edge2);
    xglVectorNormal(plane, plane);
    xglVectorInner(&plane->w, plane, a);
    plane->w = -plane->w;
}

void xglVectorMulMat(Vector4 *transformedVector, Matrix4 matrix,
                     const Vector4 *vector)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%2)\n\t"
        "lqc2 vf27, 0(%1)\n\t"
        "lqc2 vf28, 16(%1)\n\t"
        "lqc2 vf29, 32(%1)\n\t"
        "lqc2 vf30, 48(%1)\n\t"
        "vmulax.xyzw ACCxyzw, vf27xyzw, vf1x\n\t"
        "vmadday.xyzw ACCxyzw, vf28xyzw, vf1y\n\t"
        "vmaddaz.xyzw ACCxyzw, vf29xyzw, vf1z\n\t"
        "vmaddw.xyzw vf31xyzw, vf30xyzw, vf1w\n\t"
        "sqc2 vf31, 0(%0)"
        :
        : "r"(transformedVector), "r"(matrix), "r"(vector)
        : "memory"
    );
}

float xglRotTransPers(ScreenVector4 *screenPoint, float matrix[4][4], Vector4 *point, int cameraId)
{
    XglCamera *camera;

    xglMatrixStackPush();
    camera = xglStudioGetCamera2(cameraId);
    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    if (matrix != 0) {
        xglMatrixStackMul(matrix);
    }
    xglMatrixStackRTPS(screenPoint, point, &camera->screen.screenScale, &camera->screen.screenOffset);
    xglMatrixStackPop(1);
    return screenPoint->w;
}

void xglRotTransPersScreen(ScreenVector4 *screenPoint, Matrix4 matrix, Vector4 *point, int cameraId)
{
    int x;
    int y;
    int xRounded;
    int yRounded;
    int xPixel;
    int yPixel;

    xglRotTransPers(screenPoint, matrix, point, cameraId);
    x = screenPoint->x;
    y = screenPoint->y;
    xRounded = x;
    if (x < 0) {
        xRounded = x + 0xF;
    }
    yRounded = y;
    if (y < 0) {
        yRounded = y + 0xF;
    }
    xPixel = (xRounded >> 4) - 0x700;
    yPixel = (yRounded >> 4) - 0x720;
    screenPoint->x = xPixel;
    screenPoint->y = yPixel;
}

void xglRotTransPersN(ScreenVector4 *screenPoints, Matrix4 matrix, const Vector4 *points,
                   int pointCount, int cameraId)
{
    XglCamera *camera;

    xglMatrixStackPush();
    camera = xglStudioGetCamera2(cameraId);
    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    if (matrix != 0) {
        xglMatrixStackMul(matrix);
    }
    {
        while (pointCount > 0) {
            xglMatrixStackRTPS(screenPoints, points, &camera->screen.screenScale,
                               &camera->screen.screenOffset);
            points++;
            screenPoints++;
            pointCount--;
        }
    }
    xglMatrixStackPop(1);
}

void xglMatrixUnit(Matrix4 destination)
{
    QuadwordCopyMatrix(destination, xglIdentityMatrix);
}

void xglMatrixUnit4s(Matrix4 destination)
{
    QuadwordCopyMatrix(destination, xglUnit4sMatrix);
}

void xglMatrixMul(Matrix4 productMatrix, Matrix4 left, Matrix4 right)
{
    __asm__ __volatile__(
        "lqc2 vf2, 0(%2)\n\t"
        "lqc2 vf3, 16(%2)\n\t"
        "lqc2 vf4, 32(%2)\n\t"
        "lqc2 vf5, 48(%2)\n\t"
        "lqc2 vf27, 0(%1)\n\t"
        "lqc2 vf28, 16(%1)\n\t"
        "lqc2 vf29, 32(%1)\n\t"
        "lqc2 vf30, 48(%1)\n\t"
        "vmulax.xyzw ACCxyzw, vf27xyzw, vf2x\n\t"
        "vmadday.xyzw ACCxyzw, vf28xyzw, vf2y\n\t"
        "vmaddaz.xyzw ACCxyzw, vf29xyzw, vf2z\n\t"
        "vmaddw.xyzw vf2xyzw, vf30xyzw, vf2w\n\t"
        "vmulax.xyzw ACCxyzw, vf27xyzw, vf3x\n\t"
        "vmadday.xyzw ACCxyzw, vf28xyzw, vf3y\n\t"
        "vmaddaz.xyzw ACCxyzw, vf29xyzw, vf3z\n\t"
        "vmaddw.xyzw vf3xyzw, vf30xyzw, vf3w\n\t"
        "vmulax.xyzw ACCxyzw, vf27xyzw, vf4x\n\t"
        "vmadday.xyzw ACCxyzw, vf28xyzw, vf4y\n\t"
        "vmaddaz.xyzw ACCxyzw, vf29xyzw, vf4z\n\t"
        "vmaddw.xyzw vf4xyzw, vf30xyzw, vf4w\n\t"
        "vmulax.xyzw ACCxyzw, vf27xyzw, vf5x\n\t"
        "vmadday.xyzw ACCxyzw, vf28xyzw, vf5y\n\t"
        "vmaddaz.xyzw ACCxyzw, vf29xyzw, vf5z\n\t"
        "vmaddw.xyzw vf30xyzw, vf30xyzw, vf5w\n\t"
        "sqc2 vf2, 0(%0)\n\t"
        "sqc2 vf3, 16(%0)\n\t"
        "sqc2 vf4, 32(%0)\n\t"
        "sqc2 vf30, 48(%0)"
        :
        : "r"(productMatrix), "r"(left), "r"(right)
        : "memory"
    );
}

/* destination = the transpose of source. */
void xglMatrixReverse(Matrix4 destination, const Matrix *source)
{
    int i;
    int j;
    Matrix4 work;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            work[i][j] = source->rows[j][i];
        }
    }
    QuadwordCopyMatrix(destination, work);
}

void xglMatrixInverse(Matrix *inverseMatrix, Matrix *matrix)
{
    float cofactor[16];
    float inverseDeterminant;

    cofactor[0] = matrix->elements[5] * matrix->elements[10] - matrix->elements[6] * matrix->elements[9];
    cofactor[1] = matrix->elements[9] * matrix->elements[2] - matrix->elements[10] * matrix->elements[1];
    cofactor[2] = matrix->elements[1] * matrix->elements[6] - matrix->elements[2] * matrix->elements[5];
    cofactor[3] = 0.0f;
    cofactor[4] = matrix->elements[6] * matrix->elements[8] - matrix->elements[4] * matrix->elements[10];
    cofactor[5] = matrix->elements[10] * matrix->elements[0] - matrix->elements[8] * matrix->elements[2];
    cofactor[6] = matrix->elements[2] * matrix->elements[4] - matrix->elements[0] * matrix->elements[6];
    cofactor[7] = 0.0f;
    cofactor[8] = matrix->elements[4] * matrix->elements[9] - matrix->elements[5] * matrix->elements[8];
    cofactor[9] = matrix->elements[8] * matrix->elements[1] - matrix->elements[9] * matrix->elements[0];
    cofactor[10] = matrix->elements[0] * matrix->elements[5] - matrix->elements[1] * matrix->elements[4];
    cofactor[11] = 0.0f;
    cofactor[12] = cofactor[0] * matrix->elements[12] + cofactor[4] * matrix->elements[13]
        + cofactor[8] * matrix->elements[14];
    cofactor[13] = cofactor[1] * matrix->elements[12] + cofactor[5] * matrix->elements[13]
        + cofactor[9] * matrix->elements[14];
    cofactor[14] = cofactor[2] * matrix->elements[12] + cofactor[6] * matrix->elements[13]
        + cofactor[10] * matrix->elements[14];
    cofactor[15] = 1.0f;
    inverseDeterminant = matrix->elements[0] * cofactor[0];
    inverseDeterminant += matrix->elements[1] * cofactor[4];
    inverseDeterminant += matrix->elements[2] * cofactor[8];
    inverseDeterminant = 1.0f / inverseDeterminant;

    inverseMatrix->elements[0] = cofactor[0] * inverseDeterminant;
    inverseMatrix->elements[1] = cofactor[1] * inverseDeterminant;
    inverseMatrix->elements[2] = cofactor[2] * inverseDeterminant;
    inverseMatrix->elements[3] = 0.0f;
    inverseMatrix->elements[4] = cofactor[4] * inverseDeterminant;
    inverseMatrix->elements[5] = cofactor[5] * inverseDeterminant;
    inverseMatrix->elements[6] = cofactor[6] * inverseDeterminant;
    inverseMatrix->elements[7] = 0.0f;
    inverseMatrix->elements[8] = cofactor[8] * inverseDeterminant;
    inverseMatrix->elements[9] = cofactor[9] * inverseDeterminant;
    inverseMatrix->elements[10] = cofactor[10] * inverseDeterminant;
    inverseMatrix->elements[11] = 0.0f;

    inverseDeterminant = -inverseDeterminant;
    inverseMatrix->elements[12] = cofactor[12] * inverseDeterminant;
    inverseMatrix->elements[13] = cofactor[13] * inverseDeterminant;
    inverseMatrix->elements[14] = cofactor[14] * inverseDeterminant;
    inverseMatrix->elements[15] = 1.0f;
}

void xglMatrixScale(Matrix *scaledMatrix, Matrix *matrix, const Vector4 *scale)
{
    int column;

    for (column = 0; column < 4; column++) {
        scaledMatrix->rows[0][column] = matrix->rows[0][column] * scale->x;
        scaledMatrix->rows[1][column] = matrix->rows[1][column] * scale->y;
        scaledMatrix->rows[2][column] = matrix->rows[2][column] * scale->z;
        scaledMatrix->rows[3][column] = matrix->rows[3][column] * scale->w;
    }
}

void xglMatrixTrans(Matrix *translatedMatrix, Matrix *matrix, const Vector4 *translation)
{
    int column;

    for (column = 0; column < 4; column++) {
        translatedMatrix->rows[0][column] = matrix->rows[0][column];
        translatedMatrix->rows[1][column] = matrix->rows[1][column];
        translatedMatrix->rows[2][column] = matrix->rows[2][column];
        translatedMatrix->rows[3][column] = matrix->rows[0][column] * translation->x + matrix->rows[1][column] * translation->y + matrix->rows[2][column] * translation->z + matrix->rows[3][column] * translation->w;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixRotV);

void xglMatrixRotX(Matrix4 destination, Matrix4 source, float angle)
{
    AlignedVector4 sincos;
    AlignedVector4 *pSincos;
    u32 angleBits;
    float sine;
    float cosine;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    pSincos = &sincos;
    XglMicroSinCos(sine, D_20, angleBits);
    sincos.y = sine;
    XglMicroSinCos(cosine, D_E8, angleBits);
    sincos.x = cosine;
    __asm__ __volatile__(
        "lq $2, 0(%1)\n"
        "lqc2 vf28, 16(%1)\n"
        "lqc2 vf29, 32(%1)\n"
        "lqc2 vf1, 0(%2)\n"
        "sq $2, 0(%0)\n"
        "lq $2, 48(%1)\n"
        "sq $2, 48(%0)\n"
        "vmulax.xyzw ACC, vf28, vf1x\n"
        "vmaddy.xyzw vf27, vf29, vf1y\n"
        "vmulax.xyzw ACC, vf29, vf1x\n"
        "vmsuby.xyzw vf30, vf28, vf1y\n"
        "sqc2 vf27, 16(%0)\n"
        "sqc2 vf30, 32(%0)\n"
        :
        : "r" (destination), "r" (source), "r" (pSincos)
        : "memory", "$2");
}

void xglMatrixRotY(Matrix4 destination, Matrix4 source, float angle)
{
    AlignedVector4 sincos;
    AlignedVector4 *pSincos;
    u32 angleBits;
    float sine;
    float cosine;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    pSincos = &sincos;
    XglMicroSinCos(sine, D_20, angleBits);
    sincos.y = sine;
    XglMicroSinCos(cosine, D_E8, angleBits);
    sincos.x = cosine;
    __asm__ __volatile__(
        "lq $2, 16(%1)\n"
        "lqc2 vf27, 0(%1)\n"
        "lqc2 vf29, 32(%1)\n"
        "lqc2 vf1, 0(%2)\n"
        "sq $2, 16(%0)\n"
        "lq $2, 48(%1)\n"
        "sq $2, 48(%0)\n"
        "vmulax.xyzw ACC, vf27, vf1x\n"
        "vmsuby.xyzw vf28, vf29, vf1y\n"
        "vmulax.xyzw ACC, vf29, vf1x\n"
        "vmaddy.xyzw vf30, vf27, vf1y\n"
        "sqc2 vf28, 0(%0)\n"
        "sqc2 vf30, 32(%0)\n"
        :
        : "r" (destination), "r" (source), "r" (pSincos)
        : "memory", "$2");
}

void xglMatrixRotZ(Matrix4 destination, Matrix4 source, float angle)
{
    AlignedVector4 sincos;
    AlignedVector4 *pSincos;
    u32 angleBits;
    float sine;
    float cosine;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    pSincos = &sincos;
    XglMicroSinCos(sine, D_20, angleBits);
    sincos.y = sine;
    XglMicroSinCos(cosine, D_E8, angleBits);
    sincos.x = cosine;
    __asm__ __volatile__(
        "lq $2, 32(%1)\n"
        "lqc2 vf27, 0(%1)\n"
        "lqc2 vf28, 16(%1)\n"
        "lqc2 vf1, 0(%2)\n"
        "sq $2, 32(%0)\n"
        "lq $2, 48(%1)\n"
        "sq $2, 48(%0)\n"
        "vmulax.xyzw ACC, vf27, vf1x\n"
        "vmaddy.xyzw vf29, vf28, vf1y\n"
        "vmulax.xyzw ACC, vf28, vf1x\n"
        "vmsuby.xyzw vf30, vf27, vf1y\n"
        "sqc2 vf29, 0(%0)\n"
        "sqc2 vf30, 16(%0)\n"
        :
        : "r" (destination), "r" (source), "r" (pSincos)
        : "memory", "$2");
}

void xglMatrixFrustum(Matrix *destination, Matrix *source,
                      float left, float right, float bottom, float top,
                      float nearPlane, float farPlane)
{
    Matrix projection;

    projection.elements[0] = (2.0f * nearPlane) / (right - left);
    projection.elements[1] = 0.0f;
    projection.elements[2] = 0.0f;
    projection.elements[3] = 0.0f;
    projection.elements[4] = 0.0f;
    projection.elements[5] = (2.0f * nearPlane) / (top - bottom);
    projection.elements[6] = 0.0f;
    projection.elements[7] = 0.0f;
    projection.elements[8] = (right + left) / (right - left);
    projection.elements[9] = (top + bottom) / (top - bottom);
    projection.elements[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    projection.elements[11] = -1.0f;
    projection.elements[12] = 0.0f;
    projection.elements[13] = 0.0f;
    projection.elements[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    projection.elements[15] = 0.0f;
    xglMatrixMul(destination->rows, source->rows, projection.rows);
}

void xglMatrixStackUnit(void)
{
    Vu0CallMicro((u32)&gXglMicroMatrixStackUnit >> 3);
}

void xglMatrixStackMul(Matrix4 matrix)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "lqc2 vf5, 16(%1)\n\t"
        "lqc2 vf6, 32(%1)\n\t"
        "lqc2 vf7, 48(%1)\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackMul >> 3), "r"(matrix) : "memory");
}

void xglMatrixStackReverse(void)
{
    Vu0CallMicro((u32)&gXglMicroMatrixStackReverse >> 3);
}

void xglMatrixStackInverse(void)
{
    Vu0CallMicro((u32)&gXglMicroMatrixStackInverse >> 3);
}

void xglMatrixStackScale(const Vector4 *vector)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackScale >> 3), "r"(vector) : "memory");
}

void xglMatrixStackTrans(const Vector4 *vector)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackTrans >> 3), "r"(vector) : "memory");
}

void xglMatrixStackRotV(const Vector4 *vector, float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "lqc2 vf5, 0(%1)\n\t"
        "qmtc2 %2,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackRotV >> 3), "r"(vector), "r"(angle) : "memory");
}

void xglMatrixStackRotX(float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "qmtc2 %1,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackRotX >> 3), "r"(angle) : "memory");
}

void xglMatrixStackRotY(float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "qmtc2 %1,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackRotY >> 3), "r"(angle) : "memory");
}

void xglMatrixStackRotZ(float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "qmtc2 %1,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackRotZ >> 3), "r"(angle) : "memory");
}

extern char D_4A8[];

void xglMatrixStackFrustum(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    float *window = (float *)0x11004800;

    __asm__ __volatile__("ctc2.i %0,$vi27" : : "r"((u32)&D_4A8 >> 3) : "memory");
    window[0] = left;
    window[1] = bottom;
    window[2] = nearPlane;
    window[3] = 0.0f;
    window[4] = right;
    window[5] = top;
    window[6] = farPlane;
    window[7] = 0.0f;
    __asm__ __volatile__("sync\n\tvcallmsr $vi27" : : : "memory");
}

void xglMatrixStackSave(float matrix[4][4])
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi0\n\t"
        "sqc2 $vf28,0(%0)\n\t"
        "sqc2 $vf29,16(%0)\n\t"
        "sqc2 $vf30,32(%0)\n\t"
        "sqc2 $vf31,48(%0)"
        :
        : "r"(matrix)
        : "memory"
    );
}

void xglMatrixStackLoad(float matrix[4][4])
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi0\n\t"
        "lqc2 $vf28,0(%0)\n\t"
        "lqc2 $vf29,16(%0)\n\t"
        "lqc2 $vf30,32(%0)\n\t"
        "lqc2 $vf31,48(%0)"
        :
        : "r"(matrix)
        : "memory"
    );
}

void xglMatrixStackPushUnit(void)
{
    Vu0CallMicro((u32)&gXglMicroMatrixStackPushUnit >> 3);
}

void xglMatrixStackPush(void)
{
    Vu0CallMicro((u32)&gXglMicroMatrixStackPush >> 3);
}

void xglMatrixStackPop(u32 popDepth)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "ctc2 %1,$vi1\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&gXglMicroMatrixStackPop >> 3), "r"(popDepth) : "memory");
}

void xglMatrixStackMulVector(Vector4 *transformedVector, const Vector4 *vector)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "vcallmsr $vi27\n\t"
        "cfc2.i $0,$vi0\n\t"
        "sqc2 vf5, 0(%2)"
        : : "r"((u32)&gXglMicroMatrixStackMulVector >> 3), "r"(vector), "r"(transformedVector) : "memory");
}

void xglMatrixStackRTPS(ScreenVector4 *screenPoint, const Vector4 *point, Vector4 *screenScale, Vector4 *screenOffset)
{
    __asm__ __volatile__(
        "ctc2.i %0, $vi27\n"
        "lqc2 vf4, 0(%2)\n"
        "lqc2 vf5, 0(%3)\n"
        "lqc2 vf6, 0(%4)\n"
        "vcallmsr $vi27\n"
        "cfc2.i $0, $vi0\n"
        "sqc2 vf7, 0(%1)\n"
        :
        : "r" ((u32)gXglMicroMatrixStackRTPS >> 3), "r" (screenPoint), "r" (point), "r" (screenScale), "r" (screenOffset)
        : "memory");
}

void xglMatrix2Quaternion(Matrix *quaternion, Matrix *rotationMatrix)
{
    float trace;
    float w;

    trace = rotationMatrix->elements[0] + rotationMatrix->elements[5] + rotationMatrix->elements[10];
    if (trace > 0.0f) {
        w = __builtin_sqrtf(trace + 1.0f) * 0.5f;
        quaternion->elements[3] = w;
        trace = 0.5f / (w + w);
        quaternion->elements[0] = (rotationMatrix->elements[6] - rotationMatrix->elements[9]) * trace;
        quaternion->elements[1] = (rotationMatrix->elements[8] - rotationMatrix->elements[2]) * trace;
        quaternion->elements[2] = (rotationMatrix->elements[1] - rotationMatrix->elements[4]) * trace;
    } else {
        int nextAxisTable[3] = {1, 2, 0};
        int largestAxis = 0;
        int nextAxis;
        int lastAxis;
        float *largestComponent;

        if (rotationMatrix->elements[0] < rotationMatrix->elements[5]) {
            largestAxis = 1;
        }
        if (rotationMatrix->elements[largestAxis * 5] < rotationMatrix->elements[10]) {
            largestAxis = 2;
        }
        nextAxis = nextAxisTable[largestAxis];
        lastAxis = nextAxisTable[nextAxis];
        largestComponent = quaternion->elements + largestAxis;
        w = __builtin_sqrtf(rotationMatrix->elements[largestAxis * 5]
                  - (rotationMatrix->elements[nextAxis * 5] + rotationMatrix->elements[lastAxis * 5]) + 1.0f)
            * 0.5f;
        *largestComponent = w;
        trace = 0.5f / (w + w);
        quaternion->elements[3] = (rotationMatrix->elements[nextAxis * 4 + lastAxis] - rotationMatrix->elements[lastAxis * 4 + nextAxis]) * trace;
        quaternion->elements[nextAxis] = (rotationMatrix->elements[largestAxis * 4 + nextAxis] + rotationMatrix->elements[nextAxis * 4 + largestAxis]) * trace;
        quaternion->elements[lastAxis] = (rotationMatrix->elements[largestAxis * 4 + lastAxis] + rotationMatrix->elements[lastAxis * 4 + largestAxis]) * trace;
    }
}

void xglQuaternion2Matrix(Matrix4 rotationMatrix, const Vector4 *quaternion)
{
    __asm__ __volatile__(
        "lqc2 vf31, 0(%1)\n\t"
        "vaddw.x vf27x, vf0x, vf0w\n\t"
        "vaddw.y vf28y, vf0y, vf0w\n\t"
        "vaddw.z vf29z, vf0z, vf0w\n\t"
        "vadd.xyzw vf14xyzw, vf31xyzw, vf31xyzw\n\t"
        "vsub.w vf27w, vf27w, vf27w\n\t"
        "vsub.w vf28w, vf28w, vf28w\n\t"
        "vsub.w vf29w, vf29w, vf29w\n\t"
        "vmulx.xyzw vf15xyzw, vf31xyzw, vf14x\n\t"
        "vmuly.xyzw vf16xyzw, vf31xyzw, vf14y\n\t"
        "vmulz.xyzw vf17xyzw, vf31xyzw, vf14z\n\t"
        "vnop\n\t"
        "vnop\n\t"
        "vaddy.x vf20x, vf15x, vf16y\n\t"
        "vaddz.y vf20y, vf16y, vf17z\n\t"
        "vaddx.z vf20z, vf17z, vf15x\n\t"
        "vnop\n\t"
        "vsubw.z vf27z, vf15z, vf16w\n\t"
        "vaddw.y vf27y, vf15y, vf17w\n\t"
        "vsuby.x vf27x, vf27x, vf20y\n\t"
        "vsubw.x vf28x, vf16x, vf17w\n\t"
        "vaddw.z vf28z, vf16z, vf15w\n\t"
        "vsubz.y vf28y, vf28y, vf20z\n\t"
        "vsubw.y vf29y, vf17y, vf15w\n\t"
        "vaddw.x vf29x, vf17x, vf16w\n\t"
        "vsubx.z vf29z, vf29z, vf20x\n\t"
        "sqc2 vf0, 48(%0)\n\t"
        "sqc2 vf27, 0(%0)\n\t"
        "sqc2 vf28, 16(%0)\n\t"
        "sqc2 vf29, 32(%0)"
        :
        : "r"(rotationMatrix), "r"(quaternion)
        : "memory"
    );
}

void xglQuaternionInterpolateLinear(Vector4 *destination, const Vector4 *first, const Vector4 *second,
                                    float weightFirst, float weightSecond)
{
    __asm__ __volatile__(
        "mfc1 $3, %3\n"
        "mfc1 $7, %4\n"
        "qmtc2.ni $3, vf10\n"
        "qmtc2.ni $7, vf11\n"
        "lqc2 vf20, 0(%1)\n"
        "lqc2 vf21, 0(%2)\n"
        "vmul.xyzw vf18, vf20, vf21\n"
        ".set noat\n"
        "addiu $1, $0, 0\n"
        "mtc1 $1, $f0\n"
        ".set at\n"
        "vsubw.x vf1x, vf0x, vf0w\n"
        "vaddy.x vf23x, vf0x, vf18y\n"
        "vaddz.x vf24x, vf0x, vf18z\n"
        "vaddw.x vf25x, vf0x, vf18w\n"
        "vmulaw.x ACCx, vf18x, vf0w\n"
        "vmaddaw.x ACCx, vf23x, vf0w\n"
        "vmaddaw.x ACCx, vf24x, vf0w\n"
        "vmaddw.x vf26x, vf25x, vf0w\n"
        "qmfc2.ni $2, vf26\n"
        "mtc1 $2, $f1\n"
        "c.lt.s $f0, $f1\n"
        "bc1t 1f\n"
        "vmulx.xyzw vf21, vf21, vf1x\n"
        "1:\n"
        "vmulx.xyzw vf20, vf20, vf10x\n"
        "vmulx.xyzw vf21, vf21, vf11x\n"
        "vadd.xyzw vf22, vf20, vf21\n"
        "vmul.xyzw vf19, vf22, vf22\n"
        "vaddy.x vf23x, vf0x, vf19y\n"
        "vaddz.x vf24x, vf0x, vf19z\n"
        "vaddw.x vf25x, vf0x, vf19w\n"
        "vmulaw.x ACCx, vf19x, vf0w\n"
        "vmaddaw.x ACCx, vf23x, vf0w\n"
        "vmaddaw.x ACCx, vf24x, vf0w\n"
        "vmaddw.x vf26x, vf25x, vf0w\n"
        "vrsqrt Q, vf0w, vf26x\n"
        "vwaitq\n"
        "vmulq.xyzw vf22, vf22, Q\n"
        "sqc2 vf22, 0(%0)\n"
        :
        : "r" (destination), "r" (first), "r" (second), "f" (weightFirst), "f" (weightSecond)
        : "memory", "$2", "$3", "$7", "$f0", "$f1");
}

extern unsigned long long gXglRandomSeed[];

void xglSRandInit(void) {
    gXglRandomSeed[0] = 0x12345678;
}

void xglDmaDirectSrcChain(u32 channel, u32 address);
extern char xglVu0ResidentMicroprogram[];

/* Geometry start-up: seeds the VU0 random generator and xglRand, then sends the VU0 resident microprogram. */
void func_00197128(void)
{
    float randomSeed = 0.1f;

    __asm__ __volatile__(
        "qmtc2.ni %0, vf1\n\t"
        "vrinit R, vf1x"
        :
        : "r"(randomSeed)
        : "memory");
    gXglRandomSeed[0] = 0x12345678;
    xglDmaDirectSrcChain(0, (u32)xglVu0ResidentMicroprogram);
}

/* Wraps an angle into [-pi, pi). */
float func_00197168(float angle)
{
    while (angle < -3.1415927f) {
        angle += 6.2831855f;
    }
    while (angle >= 3.1415927f) {
        angle -= 6.2831855f;
    }
    return angle;
}

/* Wraps an angle into [0, 2 pi). */
float func_001971E8(float angle)
{
    while (angle < 0.0f) {
        angle += 6.2831855f;
    }
    while (angle >= 6.2831855f) {
        angle -= 6.2831855f;
    }
    return angle;
}

/* The rotation that turns the z axis from from towards to (see func_00197328). */
void func_00197268(Vector4 *rotation, const Vector4 *from, const Vector4 *to)
{
    float dx;
    float dz;
    float dy;

    dz = to->z - from->z;
    dx = to->x - from->x;
    dy = to->y - from->y;
    rotation->y = xglAtan2(dx, dz);
    rotation->x = -xglAtan2(dy, __builtin_sqrtf(dx * dx + dz * dz));
    rotation->w = 1.0f;
}


/* The rotation that turns the z axis towards direction: y angle around, x angle up (negated), w = 1. */
void func_00197328(Vector4 *rotation, const Vector4 *direction)
{
    float x;
    float z;
    float y;

    x = direction->x;
    z = direction->z;
    y = direction->y;
    rotation->y = xglAtan2(x, z);
    rotation->x = -xglAtan2(y, __builtin_sqrtf(x * x + z * z));
    rotation->w = 1.0f;
}

int func_001973D0(float a, float b, float tolerance)
{
    return __builtin_fabsf(a - b) < __builtin_fabsf(tolerance);
}

int func_001973F8(float radius, Vector4 *from, Vector4 *to)
{
    return xglPointLength(from, to) < __builtin_fabsf(radius);
}

unsigned long long func_00307E48(unsigned long long multiplicand, unsigned long long multiplier);

unsigned short xglSRand(void)
{
    gXglRandomSeed[0] = func_00307E48(gXglRandomSeed[0], 1103515245ULL) + 12345ULL;
    return (gXglRandomSeed[0] >> 16) & 0x7fff;
}

/* Two steps of the gXglRandomSeed generator; returns the first step shifted left 16 plus the second state shifted
 * right 16 (X2 xglLRand). */
unsigned int func_00197480(void)
{
    unsigned int first;

    first = (unsigned int)(func_00307E48(gXglRandomSeed[0], 1103515245ULL) + 12345ULL);
    gXglRandomSeed[0] = first * 1103515245U + 12345U;
    return (first << 16) + (unsigned int)(gXglRandomSeed[0] >> 16);
}
