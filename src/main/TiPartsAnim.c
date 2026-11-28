#include "common.h"
#include "shared.h"

float XrgInnerVector(Vector4 *first, Vector4 *second);
void XrgOuterVector(Vector4 *destination, Vector4 *first, Vector4 *second);
f32 cosf(f32);
f32 sinf(f32);
f32 acosf(f32);
/* Row-major 4x4 float matrix. */
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion);

const char D_00542B48[16] = "pPack->pPamData";
const char D_00542B58[24] = "TiPartsAnim.euc.c";
const char D_00542B70[32] = "not found parts anim id %d\n";

INCLUDE_ASM("asm/main/nonmatchings/TiPartsAnim", func_0029F548);

INCLUDE_ASM("asm/main/nonmatchings/TiPartsAnim", func_0029F9A0);

typedef int s32;

#include "quadword.h"

/* Parts state returned by func_0029F9A0: flagBits at 0, a float at 4 and a float at 8 (set by XrgPartsSetTime and XrgPartsSetWeight). */
typedef struct XrgPartsState {
    s32 flagBits;
    f32 time;
    f32 weight;
} XrgPartsState;

XrgPartsState *func_0029F9A0();

void XrgPartsSetFlags(s32 unused_arg0, s32 unused_arg1, s32 bitsToSet) {
    XrgPartsState *animPartsState;

    animPartsState = func_0029F9A0();
    if (animPartsState != 0) {
        animPartsState->flagBits |= bitsToSet;
    }
}

void XrgPartsClearFlags(s32 unused_arg0, s32 unused_arg1, s32 bitsToClear) {
    XrgPartsState *animPartsState;

    animPartsState = func_0029F9A0();
    if (animPartsState != 0) {
        animPartsState->flagBits &= ~bitsToClear;
    }
}

void XrgPartsResetFlags(void) {
    XrgPartsState *animPartsState;

    animPartsState = func_0029F9A0();
    if (animPartsState != 0) {
        animPartsState->flagBits = 0;
    }
}

void XrgPartsSetWeight(f32 newWeight) {
    XrgPartsState *animPartsState;

    animPartsState = func_0029F9A0();
    if (animPartsState != 0) {
        animPartsState->weight = newWeight;
    }
}

void XrgPartsSetTime(f32 newTime) {
    XrgPartsState *animPartsState;

    animPartsState = func_0029F9A0();
    if (animPartsState != 0) {
        animPartsState->time = newTime;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiPartsAnim", func_0029FB88);

INCLUDE_ASM("asm/main/nonmatchings/TiPartsAnim", func_0029FC60);

s32 xacReadJointCurves24(s32 *, s32, f32);

void XrgReadJointCurves(s32 curveSetIndex) {
    s32 jointCurveWords[0x18];

    xacReadJointCurves24(&jointCurveWords[0], curveSetIndex, 0.0f);
}

/* Four-component scale: dst = src * scale (same body as XrgScaleVector in X1 ov12). */
void XrgScaleVector(Vector4 *scaledVector, Vector4 *inputVector, float scale) {
    scaledVector->x = inputVector->x * scale;
    scaledVector->y = inputVector->y * scale;
    scaledVector->z = inputVector->z * scale;
    scaledVector->w = inputVector->w * scale;
}

/* Componentwise product of two vectors, all four components. */
void XrgMultiplyVector(Vector4 *productVector, Vector4 *leftVector, Vector4 *rightVector) {
    productVector->x = leftVector->x * rightVector->x;
    productVector->y = leftVector->y * rightVector->y;
    productVector->z = leftVector->z * rightVector->z;
    productVector->w = leftVector->w * rightVector->w;
}

/* In-place accumulate: dst += src, all four components. */
void XrgAccumulateVector(Vector4 *accumulator, Vector4 *addendVector) {
    accumulator->x += addendVector->x;
    accumulator->y += addendVector->y;
    accumulator->z += addendVector->z;
    accumulator->w += addendVector->w;
}

/* Scales a quaternion to unit length into destination; a zero length is replaced by 1e-8f before the division. */
void XrgQuaternionNormalize(Vector4 *normalizedQuat, Vector4 *inputQuat)
{
    f32 length;

    length = __builtin_sqrtf(inputQuat->x * inputQuat->x + inputQuat->y * inputQuat->y + inputQuat->z * inputQuat->z + inputQuat->w * inputQuat->w);
    if (length == 0.0f)
    {
        length = 0.00000001f;
    }
    length = 1.0f / length;
    normalizedQuat->x = inputQuat->x * length;
    normalizedQuat->y = inputQuat->y * length;
    normalizedQuat->z = inputQuat->z * length;
    normalizedQuat->w = inputQuat->w * length;
}

/* Quaternion from Euler angles (x, y, z in radians), halved; the product of the three axis rotations, normalised in place. */
void XrgEulerToQuaternion(Vector4 *quaternion, Vector4 *euler)
{
    Vector4 half_angle;
    Vector4 cosine;
    Vector4 sine;
    f32 cos_x_cos_z;
    f32 cos_x_sin_z;
    f32 sin_x_cos_z;
    f32 sin_x_sin_z;

    half_angle.y = euler->y / 2.0f;
    half_angle.x = euler->x / 2.0f;
    half_angle.z = euler->z / 2.0f;
    cosine.x = cosf(half_angle.x);
    cosine.y = cosf(half_angle.y);
    cosine.z = cosf(half_angle.z);
    sine.x = sinf(half_angle.x);
    sine.y = sinf(half_angle.y);
    sine.z = sinf(half_angle.z);
    cos_x_cos_z = cosine.x * cosine.z;
    cos_x_sin_z = cosine.x * sine.z;
    sin_x_cos_z = sine.x * cosine.z;
    sin_x_sin_z = sine.x * sine.z;
    quaternion->x = cosine.y * sin_x_cos_z - sine.y * cos_x_sin_z;
    quaternion->y = -(cosine.y * sin_x_sin_z - sine.y * cos_x_cos_z);
    quaternion->z = cosine.y * cos_x_sin_z - sine.y * sin_x_cos_z;
    quaternion->w = cosine.y * cos_x_cos_z - sine.y * sin_x_sin_z;
    XrgQuaternionNormalize(quaternion, quaternion);
}

f32 atan2f(f32, f32);

/* Euler angles (radians) of a quaternion, through its rotation matrix; X2 homolog XrgQuaternionToEuler (xrg_rand_int.c) with the
 * 1/cosine division and the zero guard. */
void XrgQuaternionToEuler(Vector4 *euler, Vector4 *quaternion) {
    f32 matrix[3][3];
    Vector4 cosine;
    Vector4 sine;
    f32 inverseCosine;

    matrix[0][0] = 1.0f - 2.0f * quaternion->y * quaternion->y - 2.0f * quaternion->z * quaternion->z;
    matrix[1][0] = 2.0f * quaternion->x * quaternion->y + 2.0f * quaternion->w * quaternion->z;
    matrix[2][0] = 2.0f * quaternion->x * quaternion->z - 2.0f * quaternion->w * quaternion->y;
    sine.y = -matrix[2][0];
    matrix[2][1] = 2.0f * quaternion->y * quaternion->z + 2.0f * quaternion->w * quaternion->x;
    matrix[2][2] = 1.0f - 2.0f * quaternion->x * quaternion->x + 2.0f * quaternion->y * quaternion->y;
    cosine.y = __builtin_sqrtf(1.0f - sine.y * sine.y);
    euler->y = atan2f(sine.y, cosine.y);
    if (__builtin_fabsf(cosine.y) > 0.0f) {
        inverseCosine = 1.0f / cosine.y;
        if (inverseCosine == 0.0f) {
            inverseCosine = 0.00001f;
        }
        sine.x = matrix[2][1] * inverseCosine;
        cosine.x = matrix[2][2] * inverseCosine;
        euler->x = atan2f(sine.x, cosine.x);
        sine.z = matrix[1][0] * inverseCosine;
        cosine.z = matrix[0][0] * inverseCosine;
        euler->z = atan2f(sine.z, cosine.z);
    } else {
        matrix[1][1] = 1.0f - 2.0f * quaternion->x * quaternion->x - 2.0f * quaternion->z * quaternion->z;
        matrix[1][2] = 2.0f * quaternion->y * quaternion->z - 2.0f * quaternion->w * quaternion->x;
        euler->x = atan2f(-matrix[1][2], matrix[1][1]);
        euler->z = atan2f(0.0f, 1.0f);
    }
}

/* Axis-angle from a quaternion: angle is twice acosf(w); the axis is xyz divided by sin(angle / 2), with a zero
 * sine clamped to 1e-6. */
void XrgQuaternionToAxisAngle(Vector4 *axis_angle, Vector4 *quaternion)
{
    f32 angle;
    f32 divisor;

    angle = 2.0f * acosf(quaternion->w);
    divisor = sinf(angle * 0.5f);
    if (divisor == 0.0f)
    {
        divisor = 0.000001f;
    }
    axis_angle->x = quaternion->x / divisor;
    axis_angle->y = quaternion->y / divisor;
    axis_angle->z = quaternion->z / divisor;
    axis_angle->w = angle;
}

/* Quaternion from axis-angle: w = cos(angle / 2), xyz = axis * sin(angle / 2). */
void XrgAxisAngleToQuaternion(Vector4 *quaternion, Vector4 *axis_angle)
{
    f32 half_angle;
    f32 sin_half_angle;

    half_angle = axis_angle->w * 0.5f;
    quaternion->w = cosf(half_angle);
    sin_half_angle = sinf(half_angle);
    quaternion->x = axis_angle->x * sin_half_angle;
    quaternion->y = axis_angle->y * sin_half_angle;
    quaternion->z = axis_angle->z * sin_half_angle;
}

float XrgInnerVector(Vector4 *leftVector, Vector4 *rightVector)
{
    return leftVector->x * rightVector->x + leftVector->y * rightVector->y + leftVector->z * rightVector->z;
}

void XrgOuterVector(Vector4 *crossVector, Vector4 *leftVector, Vector4 *rightVector)
{
    crossVector->x = leftVector->y * rightVector->z - leftVector->z * rightVector->y;
    crossVector->y = leftVector->z * rightVector->x - leftVector->x * rightVector->z;
    crossVector->z = leftVector->x * rightVector->y - leftVector->y * rightVector->x;
}

void XrgQuaternionCombine(Vector4 *combinedQuat, Vector4 *leftQuat, Vector4 *rightQuat)
{
    Vector4 scaledLeft;
    Vector4 scaledRight;
    Vector4 crossVector;
    Vector4 sumVector;
    f32 dot;

    scaledLeft = *leftQuat;
    XrgScaleVector(&scaledLeft, &scaledLeft, leftQuat->w);
    scaledRight = *rightQuat;
    XrgScaleVector(&scaledRight, &scaledRight, rightQuat->w);
    XrgOuterVector(&crossVector, rightQuat, leftQuat);
    sumVector.x = scaledLeft.x + scaledRight.x + crossVector.x;
    sumVector.y = scaledLeft.y + scaledRight.y + crossVector.y;
    sumVector.z = scaledLeft.z + scaledRight.z + crossVector.z;
    sumVector.w = scaledLeft.w + scaledRight.w + crossVector.w;
    dot = XrgInnerVector(leftQuat, rightQuat);
    {
        f32 w = leftQuat->w * rightQuat->w - dot;
        combinedQuat->x = sumVector.x;
        combinedQuat->y = sumVector.y;
        combinedQuat->z = sumVector.z;
        combinedQuat->w = w;
    }
}

/* Hamilton product of two quaternions into result; both operands are copied before the first store, since result may alias one of them. */
void XrgQuaternionMultiply(Vector4 *productQuat, Vector4 *leftQuat, Vector4 *rightQuat)
{
    Vector4 leftCopy;
    Vector4 rightCopy;

    leftCopy = *leftQuat;
    rightCopy = *rightQuat;
    productQuat->x = leftCopy.w * rightCopy.x + leftCopy.x * rightCopy.w + leftCopy.y * rightCopy.z - leftCopy.z * rightCopy.y;
    productQuat->y = leftCopy.w * rightCopy.y - leftCopy.x * rightCopy.z + leftCopy.y * rightCopy.w + leftCopy.z * rightCopy.x;
    productQuat->z = leftCopy.w * rightCopy.z + leftCopy.x * rightCopy.y - leftCopy.y * rightCopy.x + leftCopy.z * rightCopy.w;
    productQuat->w = leftCopy.w * rightCopy.w - leftCopy.x * rightCopy.x - leftCopy.y * rightCopy.y - leftCopy.z * rightCopy.z;
}

#include "quadword.h"

void xglVectorInner(float *destination, const Vector4 *left, const Vector4 *right);

/* Spherical interpolation from one quaternion toward another, by the angle between them, falling back to a copy of from when they are nearly equal. */
void XrgQuaternionSlerp(Vector4 *slerpedQuat, Vector4 *startQuat, Vector4 *endQuat, f32 blend)
{
    f32 dot;
    f32 theta;
    f32 endWeight;
    f32 startWeight;
    f32 inv_sin;

    xglVectorInner(&dot, startQuat, endQuat);
    XrgQuaternionNormalize(startQuat, startQuat);
    XrgQuaternionNormalize(endQuat, endQuat);
    if (dot > 0.99999f)
    {
        QuadwordCopy(slerpedQuat, startQuat);
        return;
    }
    if (dot < -0.2f)
    {
        dot = -dot;
        startQuat->x = -startQuat->x;
        startQuat->y = -startQuat->y;
        startQuat->z = -startQuat->z;
        startQuat->w = -startQuat->w;
    }
    theta = acosf(dot);
    inv_sin = sinf(theta);
    endWeight = sinf(theta * blend);
    startWeight = sinf(theta * (1.0f - blend));
    inv_sin = 1.0f / inv_sin;
    slerpedQuat->x = (startQuat->x * startWeight + endQuat->x * endWeight) * inv_sin;
    slerpedQuat->y = (startQuat->y * startWeight + endQuat->y * endWeight) * inv_sin;
    slerpedQuat->z = (startQuat->z * startWeight + endQuat->z * endWeight) * inv_sin;
    slerpedQuat->w = (startQuat->w * startWeight + endQuat->w * endWeight) * inv_sin;
    XrgQuaternionNormalize(slerpedQuat, slerpedQuat);
}

/* Normalised linear interpolation: result = from + blend * (to - from) on all four components,
 * then renormalised by XrgQuaternionNormalize. */
void XrgQuaternionLerpNormalized(Vector4 *lerpedQuat, Vector4 *startQuat, Vector4 *endQuat, f32 blend)
{
    lerpedQuat->x = startQuat->x + blend * (endQuat->x - startQuat->x);
    lerpedQuat->y = startQuat->y + blend * (endQuat->y - startQuat->y);
    lerpedQuat->z = startQuat->z + blend * (endQuat->z - startQuat->z);
    lerpedQuat->w = startQuat->w + blend * (endQuat->w - startQuat->w);
    XrgQuaternionNormalize(lerpedQuat, lerpedQuat);
}

/* Unit quaternion (x, y, z, w) to a row-major rotation matrix; the last column is zero except m[3][3] = 1. */
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion) {
    matrix[0][0] = 1.0f - 2.0f * (quaternion->y * quaternion->y + quaternion->z * quaternion->z);
    matrix[0][1] = 2.0f * (quaternion->x * quaternion->y + quaternion->w * quaternion->z);
    matrix[0][2] = 2.0f * (quaternion->x * quaternion->z - quaternion->w * quaternion->y);
    matrix[0][3] = 0.0f;
    matrix[1][0] = 2.0f * (quaternion->x * quaternion->y - quaternion->w * quaternion->z);
    matrix[1][1] = 1.0f - 2.0f * (quaternion->x * quaternion->x + quaternion->z * quaternion->z);
    matrix[1][2] = 2.0f * (quaternion->y * quaternion->z + quaternion->w * quaternion->x);
    matrix[1][3] = 0.0f;
    matrix[2][0] = 2.0f * (quaternion->x * quaternion->z + quaternion->w * quaternion->y);
    matrix[2][1] = 2.0f * (quaternion->y * quaternion->z - quaternion->w * quaternion->x);
    matrix[2][2] = 1.0f - 2.0f * (quaternion->x * quaternion->x + quaternion->y * quaternion->y);
    matrix[2][3] = 0.0f;
    matrix[3][0] = 0.0f;
    matrix[3][1] = 0.0f;
    matrix[3][2] = 0.0f;
    matrix[3][3] = 1.0f;
}
