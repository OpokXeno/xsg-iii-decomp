#include "common.h"
#include "shared.h"
#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */
void xglEulerFromRotation(Vector4 *euler, Vector4 *rotation);
void xglVectorScale(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorInner(float *destination, const Vector4 *left, const Vector4 *right);
void xglVectorOuter(Vector4 *destination, const Vector4 *first, const Vector4 *second);
void xglVectorLength(float *destination, const Vector4 *vector);
float xglPointLength(Vector4 *from, Vector4 *to);
float xglPointLengthXZ(Vector4 *from, Vector4 *to);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglRotTransPersN(ScreenVector4 *destination, Matrix4 matrix, const Vector4 *source, int count, int cameraId);
void xglMatrixMul(Matrix4 destination, Matrix4 left, Matrix4 right);
void xglMatrixInverse(Matrix *destination, Matrix *source);
void xglMatrixTrans(Matrix *destination, Matrix *source, const Vector4 *translation);
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
void xglMatrixStackPop(u32 value);
void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source);
void xglMatrixStackRTPS(ScreenVector4 *destination, const Vector4 *point, Vector4 *screenScale, Vector4 *screenOffset);
union Matrix;

#include "vu0_macro.h"

struct XglCamera;
struct XglCamera *xglStudioGetCamera2(int camera_index);
float asinf(float value);
float atan2f(float y, float x);
extern char D_00000108[];
extern char D_00000130[];
extern char D_000001B8[];
extern char D_00000218[];
extern char D_000002E8[];
extern char D_00000310[];
extern char D_00000338[];
extern char D_000003D0[];
extern char D_00000418[];
extern char D_00000460[];
extern char D_00000538[];
extern char D_00000580[];
extern char D_000005C8[];
extern char D_00000620[];
extern char D_00000648[];

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
    float s;
    float r = 0.0028662258f;
    int swap = 0;
    int quadrant = 0;
    int *bits = (int *)&y;

    if (y == 0.0f) {
        if (0.0f <= x) {
            return 0.0f;
        }
        if (*bits & 0x80000000) {
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
        swap = 1;
    } else {
        y = y / x;
    }

    s = y * y;
    r = r * s;
    r = r - 0.016165737f;
    r = r * s;
    r = r + 0.042909615f;
    r = r * s;
    r = r - 0.075289637f;
    r = r * s;
    r = r + 0.10656264f;
    r = r * s;
    r = r - 0.14208899f;
    r = r * s;
    r = r + 0.19993551f;
    r = r * s;
    r = r - 0.33333147f;
    r = r * s;
    r = r + 1.0f;
    r = r * y;

    if (swap) {
        r = 1.5707964f - r;
    }
    if (quadrant == 1) {
        r = 3.1415927f - r;
    }
    if (quadrant == 2) {
        r = -r;
    }
    if (quadrant == 3) {
        r = r - 3.1415927f;
    }
    return r;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy64);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy64b);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy16);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy16b);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy8b);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy4);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy4b);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy2);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMemCopy2b);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorDiv);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorDivXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorMulAdd);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorMulAddXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorMulSub);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorMulSubXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorDivAdd);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorDivAddXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorDivSub);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorDivSubXYZ);

void xglVectorScale(Vector4 *destination, const Vector4 *source, float scale)
{
    destination->x = source->x * scale;
    destination->y = source->y * scale;
    destination->z = source->z * scale;
    destination->w = source->w * scale;
}

void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale)
{
    destination->x = source->x * scale;
    destination->y = source->y * scale;
    destination->z = source->z * scale;
    destination->w = source->w;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorScaleAdd);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorScaleAddXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorScaleSub);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorScaleSubXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorInter);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorInterXYZ);

void xglVectorInner(float *destination, const Vector4 *left,
                    const Vector4 *right)
{
    *destination = left->x * right->x;
    *destination = *destination + left->y * right->y;
    *destination = *destination + left->z * right->z;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00195C88);

void xglVectorOuter(Vector4 *destination, const Vector4 *first, const Vector4 *second)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vopmula.xyz ACC, vf1, vf2\n\t"
        "vopmsub.xyz vf1, vf2, vf1\n\t"
        "sqc2 vf1, 0(%0)"
        : : "r"(destination), "r"(first), "r"(second) : "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00195CE8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00195D20);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00195D48);

void xglVectorLength(float *destination, const Vector4 *vector)
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
        : "r"(destination), "r"(vector)
        : "$2", "memory");
}

float xglPointLength(Vector4 *from, Vector4 *to)
{
    register Vector4 *f = from;
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
        : "+r"(f) : "r"(to) : "$2", "$f0", "memory");
}

float xglPointLengthXZ(Vector4 *from, Vector4 *to)
{
    register Vector4 *f = from;
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
        : "+r"(f) : "r"(to) : "$2", "$f0", "memory");
}

void xglVectorNormal(Vector4 *destination, const Vector4 *source)
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
        : "r"(destination), "r"(source)
        : "memory"
    );
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorClamp);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglVectorClampXYZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglPlaneParameter);

void xglVectorMulMat(Vector4 *destination, Matrix4 matrix,
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
        : "r"(destination), "r"(matrix), "r"(vector)
        : "memory"
    );
}

float xglRotTransPers(ScreenVector4 *destination, float matrix[4][4], Vector4 *point, int cameraId)
{
    XglCamera *camera;

    xglMatrixStackPush();
    camera = xglStudioGetCamera2(cameraId);
    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    if (matrix != 0) {
        xglMatrixStackMul(matrix);
    }
    xglMatrixStackRTPS(destination, point, &camera->screen.screenScale, &camera->screen.screenOffset);
    xglMatrixStackPop(1);
    return *(&destination->w);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_001960E8);

void xglRotTransPersN(ScreenVector4 *destination, Matrix4 matrix, const Vector4 *source,
                   int count, int cameraId)
{
    XglCamera *camera;

    xglMatrixStackPush();
    camera = xglStudioGetCamera2(cameraId);
    xglMatrixStackLoad(camera->volume[0].viewMatrix);
    if (matrix != 0) {
        xglMatrixStackMul(matrix);
    }
    {
        while (count > 0) {
            xglMatrixStackRTPS(destination, source, &camera->screen.screenScale,
                               &camera->screen.screenOffset);
            source++;
            destination++;
            count--;
        }
    }
    xglMatrixStackPop(1);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00196200);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00196230);

void xglMatrixMul(Matrix4 destination, Matrix4 left, Matrix4 right)
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
        : "r"(destination), "r"(left), "r"(right)
        : "memory"
    );
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixReverse);

void xglMatrixInverse(Matrix *destination, Matrix *source)
{
    float cofactor[16];
    float inverseDeterminant;

    cofactor[0] = source->elements[5] * source->elements[10] - source->elements[6] * source->elements[9];
    cofactor[1] = source->elements[9] * source->elements[2] - source->elements[10] * source->elements[1];
    cofactor[2] = source->elements[1] * source->elements[6] - source->elements[2] * source->elements[5];
    cofactor[3] = 0.0f;
    cofactor[4] = source->elements[6] * source->elements[8] - source->elements[4] * source->elements[10];
    cofactor[5] = source->elements[10] * source->elements[0] - source->elements[8] * source->elements[2];
    cofactor[6] = source->elements[2] * source->elements[4] - source->elements[0] * source->elements[6];
    cofactor[7] = 0.0f;
    cofactor[8] = source->elements[4] * source->elements[9] - source->elements[5] * source->elements[8];
    cofactor[9] = source->elements[8] * source->elements[1] - source->elements[9] * source->elements[0];
    cofactor[10] = source->elements[0] * source->elements[5] - source->elements[1] * source->elements[4];
    cofactor[11] = 0.0f;
    cofactor[12] = cofactor[0] * source->elements[12] + cofactor[4] * source->elements[13]
        + cofactor[8] * source->elements[14];
    cofactor[13] = cofactor[1] * source->elements[12] + cofactor[5] * source->elements[13]
        + cofactor[9] * source->elements[14];
    cofactor[14] = cofactor[2] * source->elements[12] + cofactor[6] * source->elements[13]
        + cofactor[10] * source->elements[14];
    cofactor[15] = 1.0f;
    inverseDeterminant = source->elements[0] * cofactor[0];
    inverseDeterminant += source->elements[1] * cofactor[4];
    inverseDeterminant += source->elements[2] * cofactor[8];
    inverseDeterminant = 1.0f / inverseDeterminant;

    destination->elements[0] = cofactor[0] * inverseDeterminant;
    destination->elements[1] = cofactor[1] * inverseDeterminant;
    destination->elements[2] = cofactor[2] * inverseDeterminant;
    destination->elements[3] = 0.0f;
    destination->elements[4] = cofactor[4] * inverseDeterminant;
    destination->elements[5] = cofactor[5] * inverseDeterminant;
    destination->elements[6] = cofactor[6] * inverseDeterminant;
    destination->elements[7] = 0.0f;
    destination->elements[8] = cofactor[8] * inverseDeterminant;
    destination->elements[9] = cofactor[9] * inverseDeterminant;
    destination->elements[10] = cofactor[10] * inverseDeterminant;
    destination->elements[11] = 0.0f;

    inverseDeterminant = -inverseDeterminant;
    destination->elements[12] = cofactor[12] * inverseDeterminant;
    destination->elements[13] = cofactor[13] * inverseDeterminant;
    destination->elements[14] = cofactor[14] * inverseDeterminant;
    destination->elements[15] = 1.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixScale);

void xglMatrixTrans(Matrix *destination, Matrix *source, const Vector4 *translation)
{
    int column;

    for (column = 0; column < 4; column++) {
        destination->rows[0][column] = source->rows[0][column];
        destination->rows[1][column] = source->rows[1][column];
        destination->rows[2][column] = source->rows[2][column];
        destination->rows[3][column] = source->rows[0][column] * translation->x + source->rows[1][column] * translation->y + source->rows[2][column] * translation->z + source->rows[3][column] * translation->w;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixRotV);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixRotX);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixRotY);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixRotZ);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglMatrixFrustum);

void xglMatrixStackUnit(void)
{
    Vu0CallMicro((u32)&D_00000108 >> 3);
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
        : : "r"((u32)&D_00000130 >> 3), "r"(matrix) : "memory");
}

void xglMatrixStackReverse(void)
{
    Vu0CallMicro((u32)&D_000001B8 >> 3);
}

void xglMatrixStackInverse(void)
{
    Vu0CallMicro((u32)&D_00000218 >> 3);
}

void xglMatrixStackScale(const Vector4 *vector)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_000002E8 >> 3), "r"(vector) : "memory");
}

void xglMatrixStackTrans(const Vector4 *vector)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_00000310 >> 3), "r"(vector) : "memory");
}

void xglMatrixStackRotV(const Vector4 *vector, float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "lqc2 vf5, 0(%1)\n\t"
        "qmtc2 %2,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_00000338 >> 3), "r"(vector), "r"(angle) : "memory");
}

void xglMatrixStackRotX(float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "qmtc2 %1,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_000003D0 >> 3), "r"(angle) : "memory");
}

void xglMatrixStackRotY(float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "qmtc2 %1,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_00000418 >> 3), "r"(angle) : "memory");
}

void xglMatrixStackRotZ(float angle)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "qmtc2 %1,$vf4\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_00000460 >> 3), "r"(angle) : "memory");
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
    Vu0CallMicro((u32)&D_00000538 >> 3);
}

void xglMatrixStackPush(void)
{
    Vu0CallMicro((u32)&D_00000580 >> 3);
}

void xglMatrixStackPop(u32 value)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "ctc2 %1,$vi1\n\t"
        "vcallmsr $vi27"
        : : "r"((u32)&D_000005C8 >> 3), "r"(value) : "memory");
}

void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source)
{
    __asm__ __volatile__(
        "ctc2.i %0,$vi27\n\t"
        "vnop\n\t"
        "lqc2 vf4, 0(%1)\n\t"
        "vcallmsr $vi27\n\t"
        "cfc2.i $0,$vi0\n\t"
        "sqc2 vf5, 0(%2)"
        : : "r"((u32)&D_00000620 >> 3), "r"(source), "r"(destination) : "memory");
}

void xglMatrixStackRTPS(ScreenVector4 *destination, const Vector4 *point, Vector4 *screenScale, Vector4 *screenOffset)
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
        : "r" ((u32)D_00000648 >> 3), "r" (destination), "r" (point), "r" (screenScale), "r" (screenOffset)
        : "memory");
}

void xglMatrix2Quaternion(Matrix *quaternion, Matrix *source)
{
    float trace;
    float w;

    trace = source->elements[0] + source->elements[5] + source->elements[10];
    if (trace > 0.0f) {
        w = __builtin_sqrtf(trace + 1.0f) * 0.5f;
        quaternion->elements[3] = w;
        trace = 0.5f / (w + w);
        quaternion->elements[0] = (source->elements[6] - source->elements[9]) * trace;
        quaternion->elements[1] = (source->elements[8] - source->elements[2]) * trace;
        quaternion->elements[2] = (source->elements[1] - source->elements[4]) * trace;
    } else {
        int nextAxisTable[3] = {1, 2, 0};
        int largestAxis = 0;
        int nextAxis;
        int lastAxis;
        float *largestComponent;

        if (source->elements[0] < source->elements[5]) {
            largestAxis = 1;
        }
        if (source->elements[largestAxis * 5] < source->elements[10]) {
            largestAxis = 2;
        }
        nextAxis = nextAxisTable[largestAxis];
        lastAxis = nextAxisTable[nextAxis];
        largestComponent = quaternion->elements + largestAxis;
        w = __builtin_sqrtf(source->elements[largestAxis * 5]
                  - (source->elements[nextAxis * 5] + source->elements[lastAxis * 5]) + 1.0f)
            * 0.5f;
        *largestComponent = w;
        trace = 0.5f / (w + w);
        quaternion->elements[3] = (source->elements[nextAxis * 4 + lastAxis] - source->elements[lastAxis * 4 + nextAxis]) * trace;
        quaternion->elements[nextAxis] = (source->elements[largestAxis * 4 + nextAxis] + source->elements[nextAxis * 4 + largestAxis]) * trace;
        quaternion->elements[lastAxis] = (source->elements[largestAxis * 4 + lastAxis] + source->elements[lastAxis * 4 + largestAxis]) * trace;
    }
}

void xglQuaternion2Matrix(Matrix4 destination, const Vector4 *quaternion)
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
        : "r"(destination), "r"(quaternion)
        : "memory"
    );
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", xglQuaternionInterpolateLinear);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00197110);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00197128);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00197168);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_001971E8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00197268);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00197328);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_001973D0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_001973F8);

unsigned long long func_00307E48(unsigned long long multiplicand, unsigned long long multiplier);
extern unsigned long long D_008BBFC0[];

unsigned short xglSRand(void)
{
    D_008BBFC0[0] = func_00307E48(D_008BBFC0[0], 1103515245ULL) + 12345ULL;
    return (D_008BBFC0[0] >> 16) & 0x7fff;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_2", func_00197480);
