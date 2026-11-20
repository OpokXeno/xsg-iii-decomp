#include "common.h"
#include "shared.h"
#include "quadword.h"
#include "vu0_macro.h"
#include "main/tu031.h"

float xglFRand(void);
int F2I(float value);
float I2F(int value);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglVectorScale(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorInner(float *destination, const Vector4 *left, const Vector4 *right);
void xglMatrixStackUnit(void);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackInverse(void);
float acosf(float);

/* Next VU0 random value as a float: vrnext advances R, and the low 32 bits of vf1 are the result. */
float xglFRand(void)
{
    __asm__ __volatile__("vrnext.x vf1x, R\n\tqmfc2 $2, vf1\n\tmtc1 $2, $f0" : : : "$2", "$f0", "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197500);

int F2I(register f32 value)
{
    register int result;

    __asm__ __volatile__(
        "mtc1 %1,$f0\n\t"
        "cvt.w.s $f0,$f0\n\t"
        "mfc1 %0,$f0"
        : "=r"(result)
        : "r"(value)
        : "$f0");
    return result;
}

f32 I2F(register int value)
{
    return (f32)value;
}

extern char D_20[];

float xglSin(float angle)
{
    u32 angleBits;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    __asm__ __volatile__(
        "ctc2.i %1, $vi27\n\t"
        "vnop\n\t"
        "qmtc2 %0, $vf4\n\t"
        "vcallmsr $vi27\n\t"
        "qmfc2.i %0, $vf1\n\t"
        "mtc1 %0, $f0"
        : "+r"(angleBits) : "r"((u32)&D_20 >> 3) : "$f0", "memory");
}

extern char D_E8[];

float xglCos(float angle)
{
    u32 angleBits;

    __asm__("mfc1 %0, %1" : "=r"(angleBits) : "f"(angle));
    __asm__ __volatile__(
        "ctc2.i %1, $vi27\n\t"
        "vnop\n\t"
        "qmtc2 %0, $vf4\n\t"
        "vcallmsr $vi27\n\t"
        "qmfc2.i %0, $vf1\n\t"
        "mtc1 %0, $f0"
        : "+r"(angleBits) : "r"((u32)&D_E8 >> 3) : "$f0", "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_001975B0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_001975C8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197880);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_001978B8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197970);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197A00);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197B10);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197C98);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00197DB0);

int xglLightSpotColor(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Vector4 *receiver)
{
    DoublewordAlignedVector4 coneAxis = { 0.0f, 0.0f, -1.0f, 1.0f };
    float angle;
    float halfCone;

    halfCone = key->coneAngle * 0.5f;
    Vu0SubVector(lightDirection, receiver, &key->position);
    xglVectorNormal(lightDirection, lightDirection);
    xglMatrixStackUnit();
    xglMatrixStackRotZ(key->rotation.z);
    xglMatrixStackRotY(key->rotation.y);
    xglMatrixStackRotX(key->rotation.x);
    xglMatrixStackMulVector(&coneAxis, &coneAxis);
    xglVectorInner(&angle, &coneAxis, lightDirection);
    angle = acosf(angle);
    if (angle > halfCone) {
        return 0;
    }
    QuadwordCopy(color, &key->color);
    if (halfCone - __builtin_fabsf(key->penumbra) < angle) {
        halfCone = (halfCone - angle) / __builtin_fabsf(key->penumbra);
        xglVectorScale(color, color, halfCone);
    }
    xglVectorScale(lightDirection, lightDirection, -1.0f);
    return 1;
}

int xglLightBoxColor(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Vector4 *receiver)
{
    DoublewordAlignedVector4 boxAxis = {0};
    float edgeDistance;

    boxAxis.z = 1.0f;
    edgeDistance = 1000000.0f;
    *lightDirection = *receiver;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    if (key->rotation.z != 0.0f) {
        xglMatrixStackRotZ(key->rotation.z);
    }
    if (key->rotation.y != 0.0f) {
        xglMatrixStackRotY(key->rotation.y);
    }
    if (key->rotation.x != 0.0f) {
        xglMatrixStackRotX(key->rotation.x);
    }
    xglMatrixStackMulVector(&boxAxis, &boxAxis);
    xglMatrixStackScale(&key->extent);
    xglMatrixStackInverse();
    lightDirection->w = 1.0f;
    xglMatrixStackMulVector(lightDirection, lightDirection);
    if (lightDirection->x < -1.0f || lightDirection->x > 1.0f || lightDirection->y < -1.0f || lightDirection->y > 1.0f || lightDirection->z > 0.0f) {
        return 0;
    }
    QuadwordCopy(color, &key->color);
    if (__builtin_fabsf((-1.0f - lightDirection->x) * key->extent.x) < edgeDistance) {
        edgeDistance = __builtin_fabsf((-1.0f - lightDirection->x) * key->extent.x);
    }
    if (__builtin_fabsf((1.0f - lightDirection->x) * key->extent.x) < edgeDistance) {
        edgeDistance = __builtin_fabsf((1.0f - lightDirection->x) * key->extent.x);
    }
    if (__builtin_fabsf((-1.0f - lightDirection->y) * key->extent.y) < edgeDistance) {
        edgeDistance = __builtin_fabsf((-1.0f - lightDirection->y) * key->extent.y);
    }
    if (__builtin_fabsf((1.0f - lightDirection->y) * key->extent.y) < edgeDistance) {
        edgeDistance = __builtin_fabsf((1.0f - lightDirection->y) * key->extent.y);
    }
    if (edgeDistance < 1.0f) {
        xglVectorScale(color, color, edgeDistance);
    }
    *lightDirection = boxAxis;
    return 1;
}

#include "main/tu141.h"

void xglMatrixStackSave(Matrix4 matrix);
void xglMatrixStackLoad(Matrix4 matrix);
float xglPointLength(Vector4 *from, Vector4 *to);
extern const DoublewordAlignedVector4 D_005412E0;

static inline void xglLightConeFinish(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Matrix4 keyMatrix, float fade)
{
    xglMatrixStackLoad(keyMatrix);
    lightDirection->w = 0.0f;
    xglMatrixStackMulVector(lightDirection, lightDirection);
    xglVectorNormal(lightDirection, lightDirection);
    xglVectorScale(lightDirection, lightDirection, -1.0f);
    QuadwordCopy(color, &key->color);
    xglVectorScale(color, color, fade);
}

int xglLightConeColor(Vector4 *color, Vector4 *lightDirection, XglLightKey *key, Vector4 *receiver)
{
    Matrix4 keyMatrix;
    Vector4 coneScale;
    float innerSlope;
    float outerSlope;
    float fade;
    float coneRatio;
    DoublewordAlignedVector4 edgePoint;

    innerSlope = key->extent.x / key->extent.z;
    outerSlope = key->extent.y / key->extent.z;
    fade = 1.0f;
    coneRatio = 1.0f;
    coneScale.x = key->extent.y;
    coneScale.y = key->extent.z;
    coneScale.z = key->extent.y;
    coneScale.w = 1.0f;

    *lightDirection = *receiver;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    if (key->rotation.z != 0.0f) {
        xglMatrixStackRotZ(key->rotation.z);
    }
    if (key->rotation.y != 0.0f) {
        xglMatrixStackRotY(key->rotation.y);
    }
    if (key->rotation.x != 0.0f) {
        xglMatrixStackRotX(key->rotation.x);
    }
    xglMatrixStackSave(keyMatrix);
    xglMatrixStackScale(&coneScale);
    xglMatrixStackInverse();
    lightDirection->w = 1.0f;
    xglMatrixStackMulVector(lightDirection, lightDirection);

    if (lightDirection->y > 1.0f || lightDirection->y < -1.0f) {
        edgePoint = D_005412E0;
        if (lightDirection->y < -1.0f) {
            edgePoint.y = -1.0f;
        }
        coneRatio = xglPointLength(lightDirection, &edgePoint) * outerSlope;
        if (coneRatio >= outerSlope) {
            return 0;
        }
        if (coneRatio > innerSlope) {
            fade = 1.0f - (coneRatio - innerSlope) / (outerSlope - innerSlope);
        }
        Vu0SubVectorXYZ(lightDirection, lightDirection, &edgePoint);
        xglLightConeFinish(color, lightDirection, key, keyMatrix, fade);
        return 1;
    } else {
        if (lightDirection->x > -1.0f && lightDirection->x < 1.0f && lightDirection->z > -1.0f && lightDirection->z < 1.0f) {
            coneRatio = __builtin_sqrtf(lightDirection->x * lightDirection->x + lightDirection->z * lightDirection->z) * outerSlope;
            if (coneRatio >= outerSlope) {
                return 0;
            }
            if (coneRatio > innerSlope) {
                fade = 1.0f - (coneRatio - innerSlope) / (outerSlope - innerSlope);
            }
            lightDirection->y = 0.0f;
            xglLightConeFinish(color, lightDirection, key, keyMatrix, fade);
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198548);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198568);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198570);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198C08);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198CA8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_030", func_00198CE0);
