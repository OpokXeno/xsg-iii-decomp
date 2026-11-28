#include "common.h"
#include "shared.h"
#include "main/xgl_2.h"
#include "quadword.h"

const AlignedVector4 MMathUnitVector = { 0.0f, 0.0f, 0.0f, 1.0f };
const Matrix4 MMathUnitMatrix = {
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
};

void MMathRandomSeed(int seed);
/* genrand_int32: regenerates the 624 words when they are used up, then tempers the next one. */
u32 MMathRandomNext(void);
float MMathMakeRandom(void);
float MMathMakeRandomSigned(void);

/*
 * Mersenne Twister (MT19937): seeding, then the tempered 32-bit output of genrand_int32.
 * Parameters: state words, twist offset, word halves, tempering masks.
 */
#define MT_STATE_WORDS 624
#define MT_TWIST_OFFSET 397
#define MT_UPPER_MASK 0x80000000
#define MT_LOWER_MASK 0x7FFFFFFF
#define MT_TEMPERING_MASK_B 0x9D2C5680
#define MT_TEMPERING_MASK_C 0xEFC60000

extern int MMathRandomIndex;
extern int MMathRandomLastSeed;
extern u32 MMathRandomState[MT_STATE_WORDS];

/* init_genrand: a zero seed means the previous seed plus 0x1571. */
void MMathRandomSeed(int seed)
{
    if (seed == 0)
    {
        seed = MMathRandomLastSeed + 0x1571;
    }
    MMathRandomLastSeed = seed;
    MMathRandomState[0] = seed;
    for (MMathRandomIndex = 1; MMathRandomIndex < MT_STATE_WORDS; MMathRandomIndex++)
    {
        MMathRandomState[MMathRandomIndex] =
            1812433253UL * (MMathRandomState[MMathRandomIndex - 1] ^ (MMathRandomState[MMathRandomIndex - 1] >> 30))
            + MMathRandomIndex;
    }
}

extern u32 MMathRandomMag01[];

/* genrand_int32: regenerates the 624 words when they are used up, then tempers the next one. */
u32 MMathRandomNext(void)
{
    u32 y;
    int kk;

    if (MMathRandomIndex >= MT_STATE_WORDS)
    {
        if (MMathRandomIndex == MT_STATE_WORDS + 1)
        {
            MMathRandomSeed(0);
        }
        for (kk = 0; kk < MT_STATE_WORDS - MT_TWIST_OFFSET; kk++)
        {
            y = (MMathRandomState[kk] & MT_UPPER_MASK) | (MMathRandomState[kk + 1] & MT_LOWER_MASK);
            MMathRandomState[kk] = MMathRandomState[kk + MT_TWIST_OFFSET] ^ (y >> 1) ^ MMathRandomMag01[y & 1];
        }
        for (; kk < MT_STATE_WORDS - 1; kk++)
        {
            y = (MMathRandomState[kk] & MT_UPPER_MASK) | (MMathRandomState[kk + 1] & MT_LOWER_MASK);
            MMathRandomState[kk] =
                MMathRandomState[kk + (MT_TWIST_OFFSET - MT_STATE_WORDS)] ^ (y >> 1) ^ MMathRandomMag01[y & 1];
        }
        y = (MMathRandomState[MT_STATE_WORDS - 1] & MT_UPPER_MASK) | (MMathRandomState[0] & MT_LOWER_MASK);
        MMathRandomState[MT_STATE_WORDS - 1] =
            MMathRandomState[MT_TWIST_OFFSET - 1] ^ (y >> 1) ^ MMathRandomMag01[y & 1];
        MMathRandomIndex = 0;
    }
    y = MMathRandomState[MMathRandomIndex++];
    y ^= y >> 11;
    y ^= (y << 7) & MT_TEMPERING_MASK_B;
    y ^= (y << 15) & MT_TEMPERING_MASK_C;
    y ^= y >> 18;
    return y;
}

/* A random integer in [0, 2^31). */
u32 MMathMakeRandomInt(void)
{
    return MMathRandomNext() >> 1;
}

/* A random float in [0, 1): the 32-bit output scaled by 2^-32. */
f32 MMathMakeRandom(void)
{
    u32 bits;

    bits = MMathRandomNext();
    return bits * 2.3283064e-10f;
}

/* A random float in [-1, 1): the 32-bit output scaled by 2^-31, minus 1. */
f32 MMathMakeRandomSigned(void)
{
    u32 bits;

    bits = MMathRandomNext();
    return (bits * 4.656613e-10f) - 1.0f;
}

float MMathMakeRandom2PI(void);
float MMathMakeRandomPI(void);
/*
 * Mersenne Twister (MT19937): seeding, then the tempered 32-bit output of genrand_int32.
 * Parameters: state words, twist offset, word halves, tempering masks.
 */
float fmodf(float, float);

/* A random angle in [0, 2*pi): the 32-bit output scaled by 2*pi / 2^32. */
f32 MMathMakeRandom2PI(void)
{
    u32 bits;

    bits = MMathRandomNext();
    return bits * 1.462918e-09f;
}

/* A random angle in [-pi, pi). */
f32 MMathMakeRandomPI(void)
{
    u32 bits;

    bits = MMathRandomNext();
    return (bits * 1.462918e-09f) - 3.1415927f;
}

/* Shortest signed difference toAngle - fromAngle, wrapped into [-pi, pi]. */
f32 MMathCalcRotNear(f32 fromAngle, f32 toAngle)
{
    f32 twoPi;
    f32 rotDelta;

    twoPi = 6.2831850f;
    rotDelta = fmodf(toAngle - fromAngle, twoPi);
    if (3.14159274f < __builtin_fabsf(rotDelta))
    {
        if (rotDelta < 0.0f)
        {
            rotDelta = rotDelta + twoPi;
        }
        else
        {
            rotDelta = rotDelta - twoPi;
        }
    }
    return rotDelta;
}

/* Difference toAngle - fromAngle wrapped into the other half-turn (|difference| >= pi). */
f32 MMathCalcRotFar(f32 fromAngle, f32 toAngle)
{
    f32 twoPi;
    f32 rotDelta;

    twoPi = 6.2831850f;
    rotDelta = fmodf(toAngle - fromAngle, twoPi);
    if (__builtin_fabsf(rotDelta) < 3.14159274f)
    {
        if (rotDelta < 0.0f)
        {
            rotDelta = rotDelta + twoPi;
        }
        else
        {
            rotDelta = rotDelta - twoPi;
        }
    }
    return rotDelta;
}

typedef Vector4 HermiteVector;

float *MMathCalcHermite(float *outVector, HermiteVector *tangent_first, HermiteVector *tangent_second,
                         HermiteVector *endpoint_first, HermiteVector *endpoint_second, float parameter);
void MMathCalcHermitePrm(HermiteVector *tangent_start, HermiteVector *tangent_end,
                          const HermiteVector *point_prev, const HermiteVector *point_start,
                          const HermiteVector *point_end, const HermiteVector *point_next);
float MMathCalcLength(const Vector4 *vector);
Vector4 *MMathNormalizeVector2(Vector4 *outVector, const Vector4 *inputVector);

/*
 * Point between two Hermite keys: lhs + (rhs - lhs) * scale on x, y and z; w is 1.
 * Callers: MMathEvalHermiteKeys (tu139), the sef and sdv_packet key tracks (progress in scale).
 */
Vector4 *MMathLerpKeyPoint(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float scale) {
    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\t"
                         "lqc2 vf2, 0(%2)\n\t"
                         "mfc1 $9, %3\n\t"
                         "vmove.w vf2, vf0\n\t"
                         "qmtc2 $9, vf3\n\t"
                         "vsub.xyz vf2, vf2, vf1\n\t"
                         "vmulx.xyz vf2, vf2, vf3x\n\t"
                         "vadd.xyz vf2, vf2, vf1\n\t"
                         "sqc2 vf2, 0(%0)"
                         :
                         : "r"(outVector), "r"(lhs), "r"(rhs), "f"(scale)
                         : "$9", "memory");
    return outVector;
}

/*
 * MMathCalcHermite: four-component cubic Hermite/Catmull-Rom evaluation at
 * parameter t. The VU0 macro-mode block builds the four blend weights from t
 * (packed two-per-register across vf2/vf3's x/y lanes), then accumulates
 * point_start*h1 + point_end*h2 + tangent_start*h3 + tangent_end*h4 into vf1
 * and stores it; vf1.w is forced to VF00's w (1.0) by the trailing vmove.w.
 * The prototype was corrected from the stale `void` shared.h declared for it
 * (no caller anywhere reads the return value, so the change is source-visible
 * only) to the `float *` the original object's `daddu v0,a0,zero` lead-in
 * requires, matching every other pointer-returning function of this TU.
 *
 * The parameter goes to VU0 through the library's fixed scratch GPR: $10
 * here (t2), because $8 and $9 would be the fifth and sixth argument
 * registers of this six-argument function.
 */
float *MMathCalcHermite(float *outVector, HermiteVector *tangent_first, HermiteVector *tangent_second,
                         HermiteVector *endpoint_first, HermiteVector *endpoint_second, float parameter)
{
    __asm__ __volatile__(
        "mfc1 $10, %0\n"
        "qmtc2 $10, vf1\n"
        "vaddx.xyz vf2, vf0, vf1x\n"
        "vmulx.xyz vf2, vf2, vf1x\n"
        "vmulx.y vf2, vf2, vf1x\n"
        "vaddw.xyzw vf3, vf0, vf0w\n"
        "vaddw.xyzw vf3, vf3, vf0w\n"
        "vmr32.xyzw vf3, vf3\n"
        "vmul.xyz vf3, vf3, vf2\n"
        "vsubz.y vf5, vf3, vf3z\n"
        "vaddw.y vf5, vf5, vf0w\n"
        "vsuby.z vf6, vf3, vf3y\n"
        "vsubx.y vf7, vf2, vf3x\n"
        "vaddx.y vf7, vf7, vf1x\n"
        "vsubx.y vf8, vf2, vf2x\n"
        "lqc2 vf1, 0(%2)\n"
        "lqc2 vf2, 0(%3)\n"
        "lqc2 vf3, 0(%4)\n"
        "lqc2 vf4, 0(%5)\n"
        "vmulay.xyz ACC, vf1, vf7y\n"
        "vmadday.xyz ACC, vf2, vf8y\n"
        "vmadday.xyz ACC, vf3, vf5y\n"
        "vmaddz.xyz vf1, vf4, vf6z\n"
        "vmove.w vf1, vf0\n"
        "sqc2 vf1, 0(%1)\n"
        :
        : "f"(parameter), "r"(outVector), "r"(tangent_first),
          "r"(tangent_second), "r"(endpoint_first), "r"(endpoint_second)
        : "$10", "memory");
    return outVector;
}

void MMathCalcHermitePrm(HermiteVector *tangent_start, HermiteVector *tangent_end,
                          const HermiteVector *point_prev, const HermiteVector *point_start,
                          const HermiteVector *point_end, const HermiteVector *point_next)
{
    __asm__ __volatile__(
        "lqc2 vf1,0(%2)\n\t"
        "lqc2 vf2,0(%3)\n\t"
        "lqc2 vf3,0(%4)\n\t"
        "lqc2 vf4,0(%5)\n\t"
        "viaddi vi5,vi0,8\n\t"
        "vmfir.x vf5x,vi5\n\t"
        "vitof4.x vf5x,vf5x\n\t"
        "vsub.xyz vf6xyz,vf3xyz,vf1xyz\n\t"
        "vsub.xyz vf7xyz,vf4xyz,vf2xyz\n\t"
        "vmulx.xyz vf6xyz,vf6xyz,vf5x\n\t"
        "vmulx.xyz vf7xyz,vf7xyz,vf5x\n\t"
        "sqc2 vf6,0(%0)\n\t"
        "sqc2 vf7,0(%1)"
        :
        : "r"(tangent_start), "r"(tangent_end), "r"(point_prev), "r"(point_start),
          "r"(point_end), "r"(point_next)
        : "memory"
    );
}

extern Matrix4 MMathCatmullRomBasis;
Vector4 *MMathApplyMatrix(Vector4 *outVector, Matrix4 *matrix, const Vector4 *vector);

/* kept: the int round trip through the float field decides the emitted order of the basis stores. */
#define FloatBitsRoundTrip(field) { int bits_ = *(int *) &(field); *(int *) &(field) = bits_; }

/*
 * Catmull-Rom point at parameter between startPoint and endPoint (previousPoint, nextPoint are the
 * neighbours): the basis (t^3, t^2, t, 1) times MMathCatmullRomBasis gives the four blend weights, and all four
 * components are written, w included.
 */
Vector4 *MMathEvalCatmullRom(Vector4 *outVector, const Vector4 *previousPoint, const Vector4 *startPoint,
                       const Vector4 *endPoint, const Vector4 *nextPoint, float parameter) {
    Vector4 basis;
    float tSquared;

    tSquared = parameter * parameter;
    basis.x = tSquared * parameter;
    FloatBitsRoundTrip(basis.x);
    basis.y = tSquared;
    basis.z = parameter;
    basis.w = 1.0f;
    MMathApplyMatrix(&basis, &MMathCatmullRomBasis, &basis);
    outVector->x = basis.x * previousPoint->x + basis.y * startPoint->x + basis.z * endPoint->x + basis.w * nextPoint->x;
    outVector->y = basis.x * previousPoint->y + basis.y * startPoint->y + basis.z * endPoint->y + basis.w * nextPoint->y;
    outVector->z = basis.x * previousPoint->z + basis.y * startPoint->z + basis.z * endPoint->z + basis.w * nextPoint->z;
    outVector->w = basis.x * previousPoint->w + basis.y * startPoint->w + basis.z * endPoint->w + basis.w * nextPoint->w;
    return outVector;
}

extern Matrix4 MMathCatmullRomBasisXYZ;

/*
 * The Catmull-Rom point of MMathEvalCatmullRom with the same basis (MMathCatmullRomBasisXYZ), x, y and z only:
 * w is forced to 1.
 */
Vector4 *MMathEvalCatmullRomXYZ(Vector4 *outVector, const Vector4 *previousPoint, const Vector4 *startPoint,
                       const Vector4 *endPoint, const Vector4 *nextPoint, float parameter) {
    Vector4 basis;
    float tSquared;
    tSquared = parameter * parameter;
    basis.w = 1.0f;
    basis.x = tSquared * parameter;
    FloatBitsRoundTrip(basis.x);
    basis.z = parameter;
    basis.y = tSquared;
    FloatBitsRoundTrip(basis.y);
    MMathApplyMatrix(&basis, &MMathCatmullRomBasisXYZ, &basis);
    outVector->x = basis.x * previousPoint->x + basis.y * startPoint->x + basis.z * endPoint->x + basis.w * nextPoint->x;
    outVector->y = basis.x * previousPoint->y + basis.y * startPoint->y + basis.z * endPoint->y + basis.w * nextPoint->y;
    outVector->z = basis.x * previousPoint->z + basis.y * startPoint->z + basis.z * endPoint->z + basis.w * nextPoint->z;
    outVector->w = 1.0f;
    return outVector;
}

extern Matrix4 MMathBezierBasis;

/*
 * Cubic Bezier point at parameter between startPoint and endPoint (controlPoint1, controlPoint2 are
 * the handles): the basis matrix MMathBezierBasis gives the four weights, and all four components are
 * written, w included.
 */
Vector4 *MMathEvalBezier(Vector4 *outVector, const Vector4 *startPoint, const Vector4 *controlPoint1,
                       const Vector4 *controlPoint2, const Vector4 *endPoint, float parameter) {
    Vector4 basis;
    float tSquared;

    tSquared = parameter * parameter;
    basis.x = tSquared * parameter;
    FloatBitsRoundTrip(basis.x);
    basis.y = tSquared;
    basis.z = parameter;
    basis.w = 1.0f;
    MMathApplyMatrix(&basis, &MMathBezierBasis, &basis);
    outVector->x = basis.x * startPoint->x + basis.y * controlPoint1->x + basis.z * controlPoint2->x + basis.w * endPoint->x;
    outVector->y = basis.x * startPoint->y + basis.y * controlPoint1->y + basis.z * controlPoint2->y + basis.w * endPoint->y;
    outVector->z = basis.x * startPoint->z + basis.y * controlPoint1->z + basis.z * controlPoint2->z + basis.w * endPoint->z;
    outVector->w = basis.x * startPoint->w + basis.y * controlPoint1->w + basis.z * controlPoint2->w + basis.w * endPoint->w;
    return outVector;
}

extern float MMathBezierBasisXYZ[13];

/*
 * The Bezier point of MMathEvalBezier with the basis MMathBezierBasisXYZ, x, y and z only: w is forced to 1.
 */
Vector4 *MMathEvalBezierXYZ(Vector4 *outVector, const Vector4 *startPoint, const Vector4 *controlPoint1,
                       const Vector4 *controlPoint2, const Vector4 *endPoint, float parameter) {
    Vector4 basis;
    float tSquared;

    tSquared = parameter * parameter;
    basis.w = 1.0f;
    basis.x = tSquared * parameter;
    FloatBitsRoundTrip(basis.x);
    basis.z = parameter;
    basis.y = tSquared;
    FloatBitsRoundTrip(basis.y);
    MMathApplyMatrix(&basis, (Matrix4 *) MMathBezierBasisXYZ, &basis);
    outVector->x = basis.x * startPoint->x + basis.y * controlPoint1->x + basis.z * controlPoint2->x + basis.w * endPoint->x;
    outVector->y = basis.x * startPoint->y + basis.y * controlPoint1->y + basis.z * controlPoint2->y + basis.w * endPoint->y;
    outVector->z = basis.x * startPoint->z + basis.y * controlPoint1->z + basis.z * controlPoint2->z + basis.w * endPoint->z;
    outVector->w = 1.0f;
    return outVector;
}

INCLUDE_ASM("asm/main/nonmatchings/m_math", func_0025A600);

/* Float vector times the degree-to-radian factor (no int conversion; the counterpart of MMathDeg2RadVectorI). */
Vector4 *MMathDeg2RadVector(Vector4 *outVector, const Vector4 *inputVector)
{
    __asm__ __volatile__(
        "li.s $f8, 0.017453292\n"
        "mfc1 $8, $f8\n"
        "qmtc2 $8, vf1\n"
        "lqc2 vf2, 0(%1)\n"
        "vmulx.xyz vf2, vf2, vf1x\n"
        "sqc2 vf2, 0(%0)\n"
        : : "r"(outVector), "r"(inputVector) : "$8", "$f8", "memory");
    return outVector;
}

/* Int vector converted to float, times the degree-to-radian factor. */
Vector4 *MMathDeg2RadVectorI(Vector4 *outVector, const Vector4 *inputVector)
{
    __asm__ __volatile__(
        "li.s $f8, 0.017453292\n"
        "mfc1 $8, $f8\n"
        "qmtc2 $8, vf1\n"
        "lqc2 vf2, 0(%1)\n"
        "vitof0.xyz vf2, vf2\n"
        "vmulx.xyz vf2, vf2, vf1x\n"
        "sqc2 vf2, 0(%0)\n"
        : : "r"(outVector), "r"(inputVector) : "$8", "$f8", "memory");
    return outVector;
}

/* Float vector times the radian-to-degree factor (no int conversion; the counterpart of MMathRad2DegVectorI). */
Vector4 *MMathRad2DegVector(Vector4 *outVector, const Vector4 *inputVector)
{
    __asm__ __volatile__(
        "li.s $f8, 57.29578\n"
        "mfc1 $8, $f8\n"
        "qmtc2 $8, vf1\n"
        "lqc2 vf2, 0(%1)\n"
        "vmulx.xyz vf2, vf2, vf1x\n"
        "sqc2 vf2, 0(%0)\n"
        : : "r"(outVector), "r"(inputVector) : "$8", "$f8", "memory");
    return outVector;
}

/* Float vector times the radian-to-degree factor, converted to int. */
Vector4 *MMathRad2DegVectorI(Vector4 *outVector, const Vector4 *inputVector)
{
    __asm__ __volatile__(
        "li.s $f8, 57.29578\n"
        "mfc1 $8, $f8\n"
        "qmtc2 $8, vf1\n"
        "lqc2 vf2, 0(%1)\n"
        "vmulx.xyz vf2, vf2, vf1x\n"
        "vftoi0.xyz vf2, vf2\n"
        "sqc2 vf2, 0(%0)\n"
        : : "r"(outVector), "r"(inputVector) : "$8", "$f8", "memory");
    return outVector;
}

Vector4 *MMathNormalizeVector2(Vector4 *outVector, const Vector4 *inputVector)
{
    __asm__ __volatile__(
        "lqc2 vf1,0(%1)\n\t"
        "vmul.xyz vf2xyz,vf1xyz,vf1xyz\n\t"
        "vaddy.x vf2x,vf2x,vf2y\n\t"
        "vaddz.x vf2x,vf2x,vf2z\n\t"
        "vrsqrt Q,vf0w,vf2x\n\t"
        "vmove.w vf2w,vf0w\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf2xyz,vf1xyz,Q\n\t"
        "sqc2 vf2,0(%0)"
        : "+r"(outVector)
        : "r"(inputVector)
        : "memory"
    );
    return outVector;
}

/* Vector length over x, y and z (VU0 square, sum, square root; the result is a float returned in $f0).
 * The asm's float output stays in a register; it is bound to the x field of a Vector4 local only so
 * the function keeps the original's 16-byte frame, which no instruction reads or writes. */
float MMathCalcLength(const Vector4 *vector)
{
    Vector4 length;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vmul.xyz vf1, vf1, vf1\n\t"
        "vaddy.x vf1, vf1, vf1y\n\t"
        "vaddz.x vf1, vf1, vf1z\n\t"
        "vsqrt Q, vf1x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(length.x) : "r"(vector) : "$8", "memory");
    return length.x;
}

float MMathCalcDist(const Vector4 *fromPoint, const Vector4 *toPoint);

/* Squared length of vector over x, y and z (no square root; sef compares it with squared radii). */
float MMathCalcLengthSquared(const Vector4 *vector) {
    Vector4 squared;

    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\t"
                         "vmul.xyz vf1, vf1, vf1\n\t"
                         "vaddy.x vf1, vf1, vf1y\n\t"
                         "vaddz.x vf1, vf1, vf1z\n\t"
                         "qmfc2 $8, vf1\n\t"
                         "mtc1 $8, %0"
                         : "=f"(squared.x)
                         : "r"(vector)
                         : "$8", "memory");
    return squared.x;
}

/* Vector length over x and y only. */
float MMathCalcLengthXY(const Vector4 *vector)
{
    Vector4 length;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vmul.xy vf1, vf1, vf1\n\t"
        "vaddy.x vf1, vf1, vf1y\n\t"
        "vsqrt Q, vf1x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(length.x) : "r"(vector) : "$8", "memory");
    return length.x;
}

/* Vector length over x and z only. */
float MMathCalcLengthXZ(const Vector4 *vector)
{
    Vector4 length;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vmul.xz vf1, vf1, vf1\n\t"
        "vaddz.x vf1, vf1, vf1z\n\t"
        "vsqrt Q, vf1x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(length.x) : "r"(vector) : "$8", "memory");
    return length.x;
}

/* Vector length over y and z only. */
float MMathCalcLengthYZ(const Vector4 *vector)
{
    Vector4 length;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vmul.yz vf1, vf1, vf1\n\t"
        "vaddz.y vf1, vf1, vf1z\n\t"
        "vsqrt Q, vf1y\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(length.x) : "r"(vector) : "$8", "memory");
    return length.x;
}

/* Distance from first to second (MMathCalcDist in X1). The original reserves a 16-byte frame
 * that no instruction touches: without the unused `difference` local (the vector VU0 keeps in
 * vf1) gcc emits no stack adjustment. */
float MMathCalcDist(const Vector4 *fromPoint, const Vector4 *toPoint)
{
    Vector4 difference;
    float distance;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vsub.xyz vf1, vf2, vf1\n\t"
        "vmul.xyz vf1, vf1, vf1\n\t"
        "vaddy.x vf1, vf1, vf1y\n\t"
        "vaddz.x vf1, vf1, vf1z\n\t"
        "vsqrt Q, vf1x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(distance)
        : "r"(fromPoint), "r"(toPoint)
        : "$8", "memory");
    return distance;
}

float MMathCalcDistXZ(const Vector4 *fromPoint, const Vector4 *toPoint);
Vector4 *MMathCalcDirVector(Vector4 *outVector, const Vector4 *fromPoint, const Vector4 *toPoint);
Vector4 *MMathDivVectorS(Vector4 *outVector, const Vector4 *inputVector, float scale);
Vector4 *MMathCalcAngle(Vector4 *outAngles, const Vector4 *fromPoint, const Vector4 *toPoint);
Vector4 *MMathSubVectorMulS(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float scale);
float MMathVectorDotProduct(const Vector4 *lhs, const Vector4 *rhs);
Vector4 *MMathVectorCrossProduct(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs);
Vector4 *MMathVectorInterpolation(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float parameter);
float srsAtan2(float x, float y);

/* Squared distance between two points over x, y and z (sef compares it with a squared step). */
float MMathCalcDistSquared(const Vector4 *fromPoint, const Vector4 *toPoint) {
    Vector4 difference;

    __asm__ __volatile__("lqc2 vf1, 0(%1)\n\t"
                         "lqc2 vf2, 0(%2)\n\t"
                         "vsub.xyz vf1, vf2, vf1\n\t"
                         "vmul.xyz vf1, vf1, vf1\n\t"
                         "vaddy.x vf1, vf1, vf1y\n\t"
                         "vaddz.x vf1, vf1, vf1z\n\t"
                         "qmfc2 $8, vf1\n\t"
                         "mtc1 $8, %0"
                         : "=f"(difference.x)
                         : "r"(fromPoint), "r"(toPoint)
                         : "$8", "memory");
    return difference.x;
}

/* Distance from first to second in the XZ plane (MMathCalcDistXZ in X1); the unused
 * `difference` local reproduces the 16-byte frame, as in MMathCalcDist. */
float MMathCalcDistXZ(const Vector4 *fromPoint, const Vector4 *toPoint)
{
    Vector4 difference;
    float distanceXZ;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vsub.xz vf1, vf2, vf1\n\t"
        "vmul.xz vf1, vf1, vf1\n\t"
        "vaddz.x vf1, vf1, vf1z\n\t"
        "vsqrt Q, vf1x\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(distanceXZ)
        : "r"(fromPoint), "r"(toPoint)
        : "$8", "memory");
    return distanceXZ;
}

/* Unit vector from source towards target, stored at destination (MMathCalcDirVector in X1).
 * A plain `return MMathNormalizeVector2(...)` compiles to a sibling call (`j`); the original
 * calls it with `jal` and returns its result, which the do { } while (0) reproduces. */
Vector4 *MMathCalcDirVector(Vector4 *outVector, const Vector4 *fromPoint, const Vector4 *toPoint)
{
    Vector4 *difference = outVector;
    Vector4 *normalized;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vsub.xyz vf2, vf2, vf1\n\t"
        "sqc2 vf2, 0(%0)"
        : "+r"(difference)
        : "r"(fromPoint), "r"(toPoint)
        : "memory");
    do {
        normalized = MMathNormalizeVector2(outVector, difference);
    } while (0);
    return normalized;
}

/* Heading in the XZ plane from first to second (MMathCalcDir in X1). The srsAtan2 result is
 * left in $f0 by the last call; the declared void return does not change that. */
void MMathCalcDir(const Vector4 *fromPoint, const Vector4 *toPoint)
{
    const Vector4 **origin;

    origin = &fromPoint;
    srsAtan2(toPoint->x - (*origin)->x, toPoint->z - (*origin)->z);
}

/* Pitch from source to target to out.x, yaw to out.y; a null source or target defaults to
 * MMathUnitVector (the library's own lui/addiu/movz null-default group inside the VU0 block). */
Vector4 *MMathCalcAngle(Vector4 *outAngles, const Vector4 *fromPoint, const Vector4 *toPoint)
{
    Vector4 delta;
    float horizontalLength = 0.0f;

    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitVector)\n\t"
        "addiu $8, $8, %%lo(MMathUnitVector)\n\t"
        "movz %2, $8, %2\n\t"
        "movz %3, $8, %3\n\t"
        "lqc2 vf1, 0(%2)\n\t"
        "lqc2 vf2, 0(%3)\n\t"
        "vsub.xz vf3, vf2, vf1\n\t"
        "vmul.xz vf4, vf3, vf3\n\t"
        "vaddz.x vf4, vf4, vf4z\n\t"
        "vsqrt Q, vf4x\n\t"
        "vsub.y vf3, vf1, vf2\n\t"
        "sqc2 vf3, %1\n\t"
        "sqc2 vf0, 0(%4)\n\t"
        "vwaitq\n\t"
        "vaddq.x vf1, vf0, Q\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "+f"(horizontalLength), "=m"(delta), "+r"(fromPoint), "+r"(toPoint)
        : "r"(outAngles)
        : "$8", "memory");
    outAngles->x = srsAtan2(delta.y, horizontalLength);
    outAngles->y = srsAtan2(delta.x, delta.z);
    return outAngles;
}

Vector4 *MMathCalcOffset(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs)
{
    __asm__ __volatile__(
        "lqc2 vf5, 0(%0)\n\t"
        "lqc2 vf6, 0(%1)\n\t"
        "vaddy.w vf7, vf5, vf6y\n\t"
        "vaddw.x vf4, vf0, vf7w\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.z vf7, vf0, vf1x\n\t"
        "vaddw.x vf4, vf0, vf7w\n\t"
        "vcallms 0x20\n\t"
        "vmove.x vf7, vf1\n\t"
        "vmove.x vf4, vf6\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf7, vf0, vf1x\n\t"
        "vmove.x vf4, vf6\n\t"
        "vcallms 0xE8\n\t"
        "vmulx.xz vf7, vf7, vf1x\n\t"
        "vmulaw.xyz ACC, vf7, vf6w\n\t"
        "vmaddw.xyz vf5, vf5, vf0w\n\t"
        "sqc2 vf5, 0(%2)"
        : : "r"(lhs), "r"(rhs), "r"(outVector) : "memory");
    return outVector;
}

Vector4 *MMathCalcOffsetXYZ(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs)
{
    __asm__ __volatile__(
        "lqc2 vf5, 0(%0)\n\t"
        "lqc2 vf6, 0(%1)\n\t"
        "vaddw.x vf4, vf0, vf5w\n\t"
        "vcallms 0xE8\n\t"
        "vmove.x vf7, vf1\n\t"
        "vaddw.x vf4, vf0, vf5w\n\t"
        "vcallms 0x20\n\t"
        "vaddz.x vf2, vf0, vf6z\n\t"
        "vsubx.z vf2, vf0, vf6x\n\t"
        "vmulax.xz ACC, vf6, vf7x\n\t"
        "vmaddx.xz vf6, vf2, vf1x\n\t"
        "vadd.xyz vf5, vf5, vf6\n\t"
        "sqc2 vf5, 0(%2)"
        : : "r"(lhs), "r"(rhs), "r"(outVector) : "memory");
    return outVector;
}

Vector4 *MMathSubVectorMulS(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float scale)
{
    __asm__ __volatile__(
        "mfc1 $8, %0\n\t"
        "qmtc2.ni $8, vf3\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vsub.xyz vf1, vf1, vf2\n\t"
        "vmulx.xyz vf1, vf1, vf3x\n\t"
        "sqc2 vf1, 0(%3)"
        : : "f"(scale), "r"(lhs), "r"(rhs), "r"(outVector) : "$8", "memory");
    return outVector;
}

Vector4 *MMathSubVectorDivS(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float scale)
{
    __asm__ __volatile__(
        "mfc1 $8, %0\n\t"
        "qmtc2.ni $8, vf3\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "vdiv Q, vf0w, vf3x\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vsub.xyz vf1, vf1, vf2\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf1, vf1, Q\n\t"
        "sqc2 vf1, 0(%3)"
        : : "f"(scale), "r"(lhs), "r"(rhs), "r"(outVector) : "$8", "memory");
    return outVector;
}

Vector4 *MMathDivVector(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 0(%1)\n\t"
        "vdiv Q, vf1x, vf2x\n\t"
        "vmove.w vf3, vf1\n\t"
        "vwaitq\n\t"
        "vaddq.x vf3, vf0, Q\n\t"
        "vdiv Q, vf1y, vf2y\n\t"
        "vwaitq\n\t"
        "vaddq.y vf3, vf0, Q\n\t"
        "vdiv Q, vf1z, vf2z\n\t"
        "vwaitq\n\t"
        "vaddq.z vf3, vf0, Q\n\t"
        "sqc2 vf3, 0(%2)"
        : : "r"(lhs), "r"(rhs), "r"(outVector) : "memory");
    return outVector;
}

Vector4 *MMathDivVectorS(Vector4 *outVector, const Vector4 *inputVector, float scale)
{
    __asm__ __volatile__(
        "mfc1 $8, %0\n\t"
        "qmtc2.ni $8, vf2\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "vdiv Q, vf0w, vf2x\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf1, vf1, Q\n\t"
        "sqc2 vf1, 0(%2)"
        : : "f"(scale), "r"(inputVector), "r"(outVector) : "$8", "memory");
    return outVector;
}

Vector4 *MMathDivVector4(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 0(%1)\n\t"
        "vdiv Q, vf1x, vf2x\n\t"
        "vsub.w vf3, vf3, vf3\n\t"
        "vwaitq\n\t"
        "vaddq.x vf3, vf0, Q\n\t"
        "vdiv Q, vf1y, vf2y\n\t"
        "vwaitq\n\t"
        "vaddq.y vf3, vf0, Q\n\t"
        "vdiv Q, vf1z, vf2z\n\t"
        "vwaitq\n\t"
        "vaddq.z vf3, vf0, Q\n\t"
        "vdiv Q, vf1w, vf2w\n\t"
        "vwaitq\n\t"
        "vaddq.w vf3, vf3, Q\n\t"
        "sqc2 vf3, 0(%2)"
        : : "r"(lhs), "r"(rhs), "r"(outVector) : "memory");
    return outVector;
}

Vector4 *MMathDivVectorS4(Vector4 *outVector, const Vector4 *inputVector, float scale)
{
    __asm__ __volatile__(
        "mfc1 $8, %0\n\t"
        "qmtc2.ni $8, vf2\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "vdiv Q, vf0w, vf2x\n\t"
        "vwaitq\n\t"
        "vmulq.xyzw vf1, vf1, Q\n\t"
        "sqc2 vf1, 0(%2)"
        : : "f"(scale), "r"(inputVector), "r"(outVector) : "$8", "memory");
    return outVector;
}

/* 128-bit equality test: 1 when the two quadwords are identical, else 0. The hand-written MMI block
 * compares both doubleword halves of the xor mask; the sibling below differs only in the permute
 * before the second movn. The original keeps an untouched 16-byte frame: the unused scratch quadword. */
int MMathIsVectorEqual(const Vector4 *lhs, const Vector4 *rhs)
{
    int equal;
    Vector4 frameQuad;

    __asm__ __volatile__(
        "lq $8, 0(%1)\n\t"
        "lq $9, 0(%2)\n\t"
        "pxor $10, $8, $9\n\t"
        "addi %0, $0, 1\n\t"
        "movn %0, $0, $10\n\t"
        "pexew $10, $10\n\t"
        "movn %0, $0, $10"
        : "=&r"(equal)
        : "r"(lhs), "r"(rhs)
        : "$8", "$9", "$10");
    return equal;
}

int MMathIsVectorEqual4(const Vector4 *lhs, const Vector4 *rhs)
{
    int equal;
    Vector4 frameQuad;

    __asm__ __volatile__(
        "lq $8, 0(%1)\n\t"
        "lq $9, 0(%2)\n\t"
        "pxor $10, $8, $9\n\t"
        "addi %0, $0, 1\n\t"
        "movn %0, $0, $10\n\t"
        "pcpyud $10, $10, $0\n\t"
        "movn %0, $0, $10"
        : "=&r"(equal)
        : "r"(lhs), "r"(rhs)
        : "$8", "$9", "$10");
    return equal;
}

/* Dot product of the xyz lanes, returned in $f0. The asm's float output stays in a register; it is
 * bound to the x field of a Vector4 local only so the function keeps the original's 16-byte frame,
 * which no instruction reads or writes. */
float MMathVectorDotProduct(const Vector4 *lhs, const Vector4 *rhs)
{
    Vector4 product;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vmul.xyz vf1, vf1, vf2\n\t"
        "vaddy.x vf1, vf1, vf1y\n\t"
        "vaddz.x vf1, vf1, vf1z\n\t"
        "qmfc2.ni $8, vf1\n\t"
        "mtc1 $8, %0"
        : "=f"(product.x) : "r"(lhs), "r"(rhs) : "$8", "memory");
    return product.x;
}

Vector4 *MMathVectorCrossProduct(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 0(%1)\n\t"
        "vopmula.xyz ACC, vf1, vf2\n\t"
        "vopmsub.xyz vf1, vf2, vf1\n\t"
        "vmove.w vf1, vf0\n\t"
        "sqc2 vf1, 0(%2)"
        : : "r"(lhs), "r"(rhs), "r"(outVector) : "memory");
    return outVector;
}

Vector4 *MMathVectorInterpolation(Vector4 *outVector, const Vector4 *lhs, const Vector4 *rhs, float parameter)
{
    __asm__ __volatile__(
        "mfc1 $8, %0\n\t"
        "qmtc2.ni $8, vf3\n\t"
        "vsubx.w vf3, vf0, vf3x\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "vmulaw.xyz ACC, vf1, vf3w\n\t"
        "vmaddx.xyz vf1, vf2, vf3x\n\t"
        "sqc2 vf1, 0(%3)"
        : : "f"(parameter), "r"(lhs), "r"(rhs), "r"(outVector) : "$8", "memory");
    return outVector;
}

/* Row-major 4x4 float matrix. */
Matrix4 *MMathRotateMatrixX(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixY(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixZ(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixXY(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathRotateMatrixYX(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathRotateMatrixXYZ(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathRotateMatrixYXZ(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathRotateMatrixZXY(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathRotateMatrixZYX(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles);

/* destination = source with x, y and z clamped to [minimum, maximum]; w is kept. */
Vector4 *MMathClampVectorXYZ(Vector4 *outVector, const Vector4 *inputVector, float minimum, float maximum) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(inputVector) : "memory");
    __asm__ __volatile__("mfc1 $8, %0\n\tqmtc2.ni $8, vf2" :  : "f"(minimum) : "$8", "memory");
    __asm__ __volatile__("mfc1 $8, %0\n\tqmtc2.ni $8, vf3" :  : "f"(maximum) : "$8", "memory");
    __asm__ __volatile__("vmaxx.xyz vf1, vf1, vf2x" : : : "memory");
    __asm__ __volatile__("vminix.xyz vf1, vf1, vf3x" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(outVector) : "memory");
    return outVector;
}

/* The rotation helpers rotate matrix (MMathUnitMatrix when null) about the axes in their name,
 * applied in name order, and store the result in out. vcallms 0xE8 and 0x20 leave the cosine and
 * sine of vf4x in vf1x; the vector forms take the x, y and z angles from angles. */
Matrix4 *MMathRotateMatrixX(Matrix4 *outMatrix, Matrix4 *matrix, float angle)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "mfc1 $8, %2\n\t"
        "qmtc2.ni $8, vf4\n\t"
        "vcallms 0xE8\n\t"
        "vmove.x vf5, vf1\n\t"
        "qmtc2.ni $8, vf4\n\t"
        "vcallms 0x20\n\t"
        "vmove.x vf6, vf1\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmaddx.xyzw vf7, vf3, vf6x\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmsubx.xyzw vf8, vf2, vf6x\n\t"
        "sqc2 vf1, 0(%1)\n\t"
        "sqc2 vf7, 16(%1)\n\t"
        "sqc2 vf8, 32(%1)\n\t"
        "sqc2 vf4, 48(%1)"
        : "+r"(matrix) : "r"(outMatrix), "f"(angle) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixY(Matrix4 *outMatrix, Matrix4 *matrix, float angle)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "mfc1 $8, %2\n\t"
        "qmtc2.ni $8, vf4\n\t"
        "vcallms 0xE8\n\t"
        "vmove.x vf5, vf1\n\t"
        "qmtc2.ni $8, vf4\n\t"
        "vcallms 0x20\n\t"
        "vmove.x vf6, vf1\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulax.xyzw ACC, vf1, vf5x\n\t"
        "vmsubx.xyzw vf7, vf3, vf6x\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmaddx.xyzw vf8, vf1, vf6x\n\t"
        "sqc2 vf7, 0(%1)\n\t"
        "sqc2 vf2, 16(%1)\n\t"
        "sqc2 vf8, 32(%1)\n\t"
        "sqc2 vf4, 48(%1)"
        : "+r"(matrix) : "r"(outMatrix), "f"(angle) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixZ(Matrix4 *outMatrix, Matrix4 *matrix, float angle)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "mfc1 $8, %2\n\t"
        "qmtc2.ni $8, vf4\n\t"
        "vcallms 0xE8\n\t"
        "vmove.x vf5, vf1\n\t"
        "qmtc2.ni $8, vf4\n\t"
        "vcallms 0x20\n\t"
        "vmove.x vf6, vf1\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulax.xyzw ACC, vf1, vf5x\n\t"
        "vmaddx.xyzw vf7, vf2, vf6x\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmsubx.xyzw vf8, vf1, vf6x\n\t"
        "sqc2 vf7, 0(%1)\n\t"
        "sqc2 vf8, 16(%1)\n\t"
        "sqc2 vf3, 32(%1)\n\t"
        "sqc2 vf4, 48(%1)"
        : "+r"(matrix) : "r"(outMatrix), "f"(angle) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixXY(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "lqc2 vf7, 0(%2)\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.x vf5, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.y vf5, vf0, vf1x\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0x20\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf6, vf0, vf1x\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmaddx.xyzw vf7, vf3, vf6x\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmsubx.xyzw vf3, vf2, vf6x\n\t"
        "vmulay.xyzw ACC, vf1, vf5y\n\t"
        "vmsuby.xyzw vf8, vf3, vf6y\n\t"
        "vmulay.xyzw ACC, vf3, vf5y\n\t"
        "vmaddy.xyzw vf3, vf1, vf6y\n\t"
        "sqc2 vf4, 48(%1)\n\t"
        "sqc2 vf7, 16(%1)\n\t"
        "sqc2 vf8, 0(%1)\n\t"
        "sqc2 vf3, 32(%1)"
        : "+r"(matrix) : "r"(outMatrix), "r"(angles) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixYX(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "lqc2 vf7, 0(%2)\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.x vf5, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.y vf5, vf0, vf1x\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0x20\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf6, vf0, vf1x\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulay.xyzw ACC, vf1, vf5y\n\t"
        "vmsuby.xyzw vf7, vf3, vf6y\n\t"
        "vmulay.xyzw ACC, vf3, vf5y\n\t"
        "vmaddy.xyzw vf3, vf1, vf6y\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmaddx.xyzw vf8, vf3, vf6x\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmsubx.xyzw vf3, vf2, vf6x\n\t"
        "sqc2 vf4, 48(%1)\n\t"
        "sqc2 vf7, 0(%1)\n\t"
        "sqc2 vf8, 16(%1)\n\t"
        "sqc2 vf3, 32(%1)"
        : "+r"(matrix) : "r"(outMatrix), "r"(angles) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixXYZ(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "lqc2 vf7, 0(%2)\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.x vf5, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.y vf5, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.z vf5, vf0, vf1x\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0x20\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf6, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0x20\n\t"
        "vaddx.z vf6, vf0, vf1x\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmaddx.xyzw vf7, vf3, vf6x\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmsubx.xyzw vf3, vf2, vf6x\n\t"
        "vmulay.xyzw ACC, vf1, vf5y\n\t"
        "vmsuby.xyzw vf8, vf3, vf6y\n\t"
        "vmulay.xyzw ACC, vf3, vf5y\n\t"
        "vmaddy.xyzw vf3, vf1, vf6y\n\t"
        "vmulaz.xyzw ACC, vf8, vf5z\n\t"
        "vmaddz.xyzw vf1, vf7, vf6z\n\t"
        "vmulaz.xyzw ACC, vf7, vf5z\n\t"
        "vmsubz.xyzw vf2, vf8, vf6z\n\t"
        "sqc2 vf4, 48(%1)\n\t"
        "sqc2 vf3, 32(%1)\n\t"
        "sqc2 vf1, 0(%1)\n\t"
        "sqc2 vf2, 16(%1)"
        : "+r"(matrix) : "r"(outMatrix), "r"(angles) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixYXZ(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "lqc2 vf7, 0(%2)\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.x vf5, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.y vf5, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.z vf5, vf0, vf1x\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0x20\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf6, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0x20\n\t"
        "vaddx.z vf6, vf0, vf1x\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulay.xyzw ACC, vf1, vf5y\n\t"
        "vmsuby.xyzw vf7, vf3, vf6y\n\t"
        "vmulay.xyzw ACC, vf3, vf5y\n\t"
        "vmaddy.xyzw vf3, vf1, vf6y\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmaddx.xyzw vf8, vf3, vf6x\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmsubx.xyzw vf3, vf2, vf6x\n\t"
        "vmulaz.xyzw ACC, vf7, vf5z\n\t"
        "vmaddz.xyzw vf1, vf8, vf6z\n\t"
        "vmulaz.xyzw ACC, vf8, vf5z\n\t"
        "vmsubz.xyzw vf2, vf7, vf6z\n\t"
        "sqc2 vf4, 48(%1)\n\t"
        "sqc2 vf3, 32(%1)\n\t"
        "sqc2 vf1, 0(%1)\n\t"
        "sqc2 vf2, 16(%1)"
        : "+r"(matrix) : "r"(outMatrix), "r"(angles) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixZXY(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "lqc2 vf7, 0(%2)\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.x vf5, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.y vf5, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.z vf5, vf0, vf1x\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0x20\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf6, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0x20\n\t"
        "vaddx.z vf6, vf0, vf1x\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulaz.xyzw ACC, vf1, vf5z\n\t"
        "vmaddz.xyzw vf7, vf2, vf6z\n\t"
        "vmulaz.xyzw ACC, vf2, vf5z\n\t"
        "vmsubz.xyzw vf2, vf1, vf6z\n\t"
        "vmulax.xyzw ACC, vf3, vf5x\n\t"
        "vmsubx.xyzw vf8, vf2, vf6x\n\t"
        "vmulax.xyzw ACC, vf2, vf5x\n\t"
        "vmaddx.xyzw vf2, vf3, vf6x\n\t"
        "vmulay.xyzw ACC, vf7, vf5y\n\t"
        "vmsuby.xyzw vf1, vf8, vf6y\n\t"
        "vmulay.xyzw ACC, vf8, vf5y\n\t"
        "vmaddy.xyzw vf3, vf7, vf6y\n\t"
        "sqc2 vf4, 48(%1)\n\t"
        "sqc2 vf2, 16(%1)\n\t"
        "sqc2 vf1, 0(%1)\n\t"
        "sqc2 vf3, 32(%1)"
        : "+r"(matrix) : "r"(outMatrix), "r"(angles) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathRotateMatrixZYX(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *angles)
{
    __asm__ __volatile__(
        "lui $8, %%hi(MMathUnitMatrix)\n\t"
        "addiu $8, $8, %%lo(MMathUnitMatrix)\n\t"
        "movz %0, $8, %0\n\t"
        "lqc2 vf7, 0(%2)\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.x vf5, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.y vf5, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0xE8\n\t"
        "vaddx.z vf5, vf0, vf1x\n\t"
        "vaddx.x vf4, vf0, vf7x\n\t"
        "vcallms 0x20\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf4, vf0, vf7y\n\t"
        "vcallms 0x20\n\t"
        "vaddx.y vf6, vf0, vf1x\n\t"
        "vaddz.x vf4, vf0, vf7z\n\t"
        "vcallms 0x20\n\t"
        "vaddx.z vf6, vf0, vf1x\n\t"
        "lqc2 vf2, 16(%0)\n\t"
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf3, 32(%0)\n\t"
        "lqc2 vf4, 48(%0)\n\t"
        "vmulaz.xyzw ACC, vf2, vf5z\n\t"
        "vmsubz.xyzw vf7, vf1, vf6z\n\t"
        "vmulaz.xyzw ACC, vf1, vf5z\n\t"
        "vmaddz.xyzw vf1, vf2, vf6z\n\t"
        "vmulay.xyzw ACC, vf3, vf5y\n\t"
        "vmaddy.xyzw vf8, vf1, vf6y\n\t"
        "vmulay.xyzw ACC, vf1, vf5y\n\t"
        "vmsuby.xyzw vf1, vf3, vf6y\n\t"
        "vmulax.xyzw ACC, vf7, vf5x\n\t"
        "vmaddx.xyzw vf2, vf8, vf6x\n\t"
        "vmulax.xyzw ACC, vf8, vf5x\n\t"
        "vmsubx.xyzw vf3, vf7, vf6x\n\t"
        "sqc2 vf4, 48(%1)\n\t"
        "sqc2 vf1, 0(%1)\n\t"
        "sqc2 vf2, 16(%1)\n\t"
        "sqc2 vf3, 32(%1)"
        : "+r"(matrix) : "r"(outMatrix), "r"(angles) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathMulMatrix(Matrix4 *outMatrix, Matrix4 *rhs, Matrix4 *lhs);
Matrix4 *MMathScaleMatrix(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *scale);

Vector4 *MMathApplyMatrix(Vector4 *outVector, Matrix4 *matrix, const Vector4 *vector)
{
    __asm__ __volatile__(
        "lqc2 $vf1,0(%2)\n\t"
        "lqc2 $vf2,0(%1)\n\t"
        "lqc2 $vf3,16(%1)\n\t"
        "lqc2 $vf4,32(%1)\n\t"
        "lqc2 $vf5,48(%1)\n\t"
        "vmulax.xyzw ACCxyzw,$vf2xyzw,$vf1x\n\t"
        "vmadday.xyzw ACCxyzw,$vf3xyzw,$vf1y\n\t"
        "vmaddaz.xyzw ACCxyzw,$vf4xyzw,$vf1z\n\t"
        "vmaddw.xyzw $vf1xyzw,$vf5xyzw,$vf1w\n\t"
        "sqc2 $vf1,0(%0)"
        : : "r"(outVector), "r"(matrix), "r"(vector) : "memory");
    return outVector;
}

/*
 * MMathMulMatrix (X1 computes the same 4x4 product): out receives the product lhs x rhs in the
 * row-vector convention of MMathApplyMatrix (each row of lhs times rhs). Returns out.
 */
Matrix4 *MMathMulMatrix(Matrix4 *outMatrix, Matrix4 *rhs, Matrix4 *lhs) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(rhs) : "memory");
    __asm__ __volatile__("lqc2 vf2, 16(%0)" : : "r"(rhs) : "memory");
    __asm__ __volatile__("lqc2 vf3, 32(%0)" : : "r"(rhs) : "memory");
    __asm__ __volatile__("lqc2 vf4, 48(%0)" : : "r"(rhs) : "memory");
    __asm__ __volatile__("lqc2 vf5, 0(%0)" : : "r"(lhs) : "memory");
    __asm__ __volatile__("lqc2 vf6, 16(%0)" : : "r"(lhs) : "memory");
    __asm__ __volatile__("lqc2 vf7, 32(%0)" : : "r"(lhs) : "memory");
    __asm__ __volatile__("lqc2 vf8, 48(%0)" : : "r"(lhs) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf5x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf5y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf5z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf5, vf4, vf5w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf6x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf6y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf6z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf6, vf4, vf6w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf7x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf7y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf7z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf7, vf4, vf7w" : : : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf1, vf8x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf2, vf8y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf3, vf8z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf8, vf4, vf8w" : : : "memory");
    __asm__ __volatile__("sqc2 vf5, 0(%0)" : : "r"(outMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf6, 16(%0)" : : "r"(outMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf7, 32(%0)" : : "r"(outMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf8, 48(%0)" : : "r"(outMatrix) : "memory");
    return outMatrix;
}

Matrix4 *MMathScaleMatrix(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *scale)
{
    __asm__ __volatile__(
        "lqc2 $vf5,0(%2)\n\t"
        "lqc2 $vf1,0(%1)\n\t"
        "lqc2 $vf2,16(%1)\n\t"
        "lqc2 $vf3,32(%1)\n\t"
        "lqc2 $vf4,48(%1)\n\t"
        "vmulx.xyzw $vf1xyzw,$vf1xyzw,$vf5x\n\t"
        "vmuly.xyzw $vf2xyzw,$vf2xyzw,$vf5y\n\t"
        "vmulz.xyzw $vf3xyzw,$vf3xyzw,$vf5z\n\t"
        "vmulw.xyzw $vf4xyzw,$vf4xyzw,$vf5w\n\t"
        "sqc2 $vf1,0(%0)\n\t"
        "sqc2 $vf2,16(%0)\n\t"
        "sqc2 $vf3,32(%0)\n\t"
        "sqc2 $vf4,48(%0)"
        : : "r"(outMatrix), "r"(matrix), "r"(scale) : "memory");
    return outMatrix;
}

Matrix4 *MMathCalcLookAtMatrix(Matrix4 *outMatrix, const Vector4 *fromPoint, const Vector4 *toPoint, const Vector4 *position, float roll);
Matrix4 *MMathScaleMatrixRotS(Matrix4 *outMatrix, Matrix4 *matrix, float scale);
Matrix4 *MMathTranslateMatrix(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *translation);
struct XglCamera;
/* out = the transformed +Z point: (0, 0, 1, 1) times the matrix with its row 3 replaced by MMathUnitVector. */
Vector4 *MMathCalcVectorMatrixZ(Vector4 *out, Matrix4 *matrix);

/* out = matrix with rows 0..2 scaled by scale; row 3, the translation row, is copied unchanged. */
Matrix4 *MMathScaleMatrixRotS(Matrix4 *outMatrix, Matrix4 *matrix, float scale)
{
    __asm__ __volatile__(
        "mfc1 $8, %2\n\t"
        "qmtc2.ni $8, vf5\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 16(%1)\n\t"
        "lqc2 vf3, 32(%1)\n\t"
        "lqc2 vf4, 48(%1)\n\t"
        "vmulx.xyzw vf1, vf1, vf5x\n\t"
        "vmulx.xyzw vf2, vf2, vf5x\n\t"
        "vmulx.xyzw vf3, vf3, vf5x\n\t"
        "sqc2 vf1, 0(%0)\n\t"
        "sqc2 vf2, 16(%0)\n\t"
        "sqc2 vf3, 32(%0)\n\t"
        "sqc2 vf4, 48(%0)"
        : : "r"(outMatrix), "r"(matrix), "f"(scale) : "$8", "memory");
    return outMatrix;
}

/* out = matrix scaled by scale in every row; the w of row 3 is kept. */
Matrix4 *MMathScaleMatrixS(Matrix4 *outMatrix, const Matrix4 *matrix, float scale)
{
    __asm__ __volatile__(
        "mfc1 $8, %2\n\t"
        "qmtc2.ni $8, vf5\n\t"
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 16(%1)\n\t"
        "lqc2 vf3, 32(%1)\n\t"
        "lqc2 vf4, 48(%1)\n\t"
        "vmulx.xyzw vf1, vf1, vf5x\n\t"
        "vmulx.xyzw vf2, vf2, vf5x\n\t"
        "vmulx.xyzw vf3, vf3, vf5x\n\t"
        "vmulx.xyz vf4, vf4, vf5x\n\t"
        "sqc2 vf1, 0(%0)\n\t"
        "sqc2 vf2, 16(%0)\n\t"
        "sqc2 vf3, 32(%0)\n\t"
        "sqc2 vf4, 48(%0)"
        : : "r"(outMatrix), "r"(matrix), "f"(scale) : "$8", "memory");
    return outMatrix;
}

Matrix4 *MMathTranslateMatrix(Matrix4 *outMatrix, Matrix4 *matrix, const Vector4 *translation)
{
    __asm__ __volatile__(
        "lqc2 $vf5,0(%2)\n\t"
        "lqc2 $vf1,0(%1)\n\t"
        "lqc2 $vf2,16(%1)\n\t"
        "lqc2 $vf3,32(%1)\n\t"
        "lqc2 $vf4,48(%1)\n\t"
        "vmulax.xyzw ACCxyzw,$vf1xyzw,$vf5x\n\t"
        "vmadday.xyzw ACCxyzw,$vf2xyzw,$vf5y\n\t"
        "vmaddaz.xyzw ACCxyzw,$vf3xyzw,$vf5z\n\t"
        "vmaddw.xyzw $vf4xyzw,$vf4xyzw,$vf0w\n\t"
        "sqc2 $vf1,0(%0)\n\t"
        "sqc2 $vf2,16(%0)\n\t"
        "sqc2 $vf3,32(%0)\n\t"
        "sqc2 $vf4,48(%0)"
        : : "r"(outMatrix), "r"(matrix), "r"(translation) : "memory");
    return outMatrix;
}

/* Pitch and yaw from source to target, roll as z of the angles, YXZ rotation over the identity;
 * a non-null position goes to row 3 with w = 1. */
Matrix4 *MMathCalcLookAtMatrix(Matrix4 *outMatrix, const Vector4 *fromPoint, const Vector4 *toPoint,
                               const Vector4 *position, float roll)
{
    Vector4 angles;

    MMathCalcAngle(&angles, fromPoint, toPoint);
    angles.z = roll;
    MMathRotateMatrixYXZ(outMatrix, 0, &angles);
    /* An early return: the nested if (position != 0) form swaps out and position between $16 and $17. */
    if (position == 0) {
        return outMatrix;
    }
    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vmove.w vf1, vf0\n\t"
        "sqc2 vf1, 48(%0)"
        : : "r"(outMatrix), "r"(position) : "memory");
    return outMatrix;
}

/* out = matrix with its 3x3 block transposed; the fourth column is kept. */
Matrix4 *MMathTransposeMatrix3(Matrix4 *outMatrix, const Matrix4 *matrix)
{
    __asm__ __volatile__(
        "lqc2 vf6, 0(%1)\n\t"
        "lqc2 vf7, 16(%1)\n\t"
        "lqc2 vf8, 32(%1)\n\t"
        "lqc2 vf4, 48(%1)\n\t"
        "vmove.xw vf1, vf6\n\t"
        "vaddy.x vf2, vf0, vf6y\n\t"
        "vaddz.x vf3, vf0, vf6z\n\t"
        "vaddx.y vf1, vf0, vf7x\n\t"
        "vmove.yw vf2, vf7\n\t"
        "vaddz.y vf3, vf0, vf7z\n\t"
        "vaddx.z vf1, vf0, vf8x\n\t"
        "vaddy.z vf2, vf0, vf8y\n\t"
        "vmove.zw vf3, vf8\n\t"
        "sqc2 vf1, 0(%0)\n\t"
        "sqc2 vf2, 16(%0)\n\t"
        "sqc2 vf3, 32(%0)\n\t"
        "sqc2 vf4, 48(%0)"
        : : "r"(outMatrix), "r"(matrix) : "memory");
    return outMatrix;
}

/* out = the full 4x4 transpose of matrix. */
Matrix4 *MMathTransposeMatrix(Matrix4 *outMatrix, const Matrix4 *matrix)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 16(%1)\n\t"
        "lqc2 vf3, 32(%1)\n\t"
        "lqc2 vf4, 48(%1)\n\t"
        "vsub.xyzw vf5, vf0, vf0\n\t"
        "vaddx.x vf6, vf0, vf1x\n\t"
        "vaddy.x vf7, vf0, vf1y\n\t"
        "vaddz.x vf8, vf0, vf1z\n\t"
        "vaddw.x vf9, vf0, vf1w\n\t"
        "vaddx.y vf6, vf0, vf2x\n\t"
        "vaddy.y vf7, vf0, vf2y\n\t"
        "vaddz.y vf8, vf0, vf2z\n\t"
        "vaddw.y vf9, vf0, vf2w\n\t"
        "vaddx.z vf6, vf0, vf3x\n\t"
        "vaddy.z vf7, vf0, vf3y\n\t"
        "vaddz.z vf8, vf0, vf3z\n\t"
        "vaddw.z vf9, vf0, vf3w\n\t"
        "vaddx.w vf6, vf5, vf4x\n\t"
        "vaddy.w vf7, vf5, vf4y\n\t"
        "vaddz.w vf8, vf5, vf4z\n\t"
        "vaddw.w vf9, vf5, vf4w\n\t"
        "sqc2 vf6, 0(%0)\n\t"
        "sqc2 vf7, 16(%0)\n\t"
        "sqc2 vf8, 32(%0)\n\t"
        "sqc2 vf9, 48(%0)"
        : : "r"(outMatrix), "r"(matrix) : "memory");
    return outMatrix;
}

/* out = inverse of the affine matrix src: transposed 3x3 cofactors over the determinant, translation -(inv3 * t). */
Matrix4 *MMathInvertAffineMatrix(Matrix4 *outMatrix, float *inputMatrix) {
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(inputMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf2, 16(%0)" : : "r"(inputMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 32(%0)" : : "r"(inputMatrix) : "memory");
    __asm__ __volatile__("vsub.xyzw vf5, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf6, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vsub.xyzw vf7, vf0, vf0" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf2, vf3" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf9, vf3, vf2" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf3, vf1" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf10, vf1, vf3" : : : "memory");
    __asm__ __volatile__("vopmula.xyz ACC, vf1, vf2" : : : "memory");
    __asm__ __volatile__("vopmsub.xyz vf11, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf12, vf1, vf9" : : : "memory");
    __asm__ __volatile__("vaddx.x vf5, vf0, vf9x" : : : "memory");
    __asm__ __volatile__("vaddy.x vf6, vf0, vf9y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf7, vf0, vf9z" : : : "memory");
    __asm__ __volatile__("vaddy.x vf12, vf12, vf12y" : : : "memory");
    __asm__ __volatile__("vaddx.y vf5, vf0, vf10x" : : : "memory");
    __asm__ __volatile__("vaddy.y vf6, vf0, vf10y" : : : "memory");
    __asm__ __volatile__("vaddz.y vf7, vf0, vf10z" : : : "memory");
    __asm__ __volatile__("vaddz.x vf12, vf12, vf12z" : : : "memory");
    __asm__ __volatile__("vaddx.z vf5, vf0, vf11x" : : : "memory");
    __asm__ __volatile__("vaddy.z vf6, vf0, vf11y" : : : "memory");
    __asm__ __volatile__("vaddz.z vf7, vf0, vf11z" : : : "memory");
    __asm__ __volatile__("vdiv Q, vf0w, vf12x" : : : "memory");
    __asm__ __volatile__("lqc2 vf4, 48(%0)" : : "r"(inputMatrix) : "memory");
    __asm__ __volatile__("vmove.xyzw vf8, vf0" : : : "memory");
    __asm__ __volatile__("vmulax.xyz ACC, vf5, vf4x" : : : "memory");
    __asm__ __volatile__("vmadday.xyz ACC, vf6, vf4y" : : : "memory");
    __asm__ __volatile__("vmaddz.xyz vf8, vf7, vf4z" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vsubq.y vf12, vf0, Q" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf5, vf5, Q" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf6, vf6, Q" : : : "memory");
    __asm__ __volatile__("vmulq.xyz vf7, vf7, Q" : : : "memory");
    __asm__ __volatile__("vmuly.xyz vf8, vf8, vf12y" : : : "memory");
    __asm__ __volatile__("sqc2 vf5, 0(%0)" : : "r"(outMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf6, 16(%0)" : : "r"(outMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf7, 32(%0)" : : "r"(outMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf8, 48(%0)" : : "r"(outMatrix) : "memory");
    return outMatrix;
}

/* out = rotation (YX order) that turns the default direction toward target. */
Matrix4 *MMathCalcMatrixVector(Matrix4 *outMatrix, const Vector4 *toPoint)
{
    Vector4 angles;

    MMathCalcAngle(&angles, 0, toPoint);
    MMathRotateMatrixYX(outMatrix, 0, &angles);
    return outMatrix;
}

/* out = the transformed +Z point: (0, 0, 1, 1) times the matrix with its row 3 replaced by MMathUnitVector. */
Vector4 *MMathCalcVectorMatrixZ(Vector4 *outVector, Matrix4 *matrix)
{
    Matrix4 matrixCopy;

    QuadwordCopyMatrixT0(matrixCopy, matrix);
    __asm__ __volatile__(
        "lui $2, %%hi(MMathUnitVector)\n\t"
        "addiu $2, $2, %%lo(MMathUnitVector)\n\t"
        "lqc2 vf1, 0($2)\n\t"
        "vmove.w vf1, vf0\n\t"
        "sqc2 vf1, 48(%0)\n\t"
        "vmove.xyw vf1, vf0\n\t"
        "vaddw.z vf1, vf0, vf0w\n\t"
        "sqc2 vf1, 0(%1)"
        : : "r"(matrixCopy), "r"(outVector) : "$2", "memory");
    MMathApplyMatrix(outVector, &matrixCopy, outVector);
    return outVector;
}

/* out = angles from the default direction to the transformed +Z point of matrix. */
Vector4 *MMathCalcAngleMatrixZ(Vector4 *outAngles, Matrix4 *matrix)
{
    Vector4 point;

    MMathCalcVectorMatrixZ(&point, matrix);
    MMathCalcAngle(outAngles, 0, &point);
    return outAngles;
}

void MMathRotTransPers(Vector4 *outVector, const XglCamera *camera, const Matrix4 *matrix, const Vector4 *point)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%0)\n\t"
        "lqc2 vf2, 0(%1)\n\t"
        "lqc2 vf3, 16(%1)\n\t"
        "lqc2 vf4, 32(%1)\n\t"
        "lqc2 vf5, 48(%1)\n\t"
        "vmulax.xyzw ACC, vf2, vf1x\n\t"
        "vmadday.xyzw ACC, vf3, vf1y\n\t"
        "vmaddaz.xyzw ACC, vf4, vf1z\n\t"
        "vmaddw.xyzw vf1, vf5, vf1w\n\t"
        "vdiv Q, vf0w, vf1w\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "lqc2 vf3, 0(%3)\n\t"
        "vsub.xyzw vf4, vf0, vf0\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf1, vf1, Q\n\t"
        "vmula.xyz ACC, vf1, vf2\n\t"
        "vmaddw.xyz vf4, vf3, vf0w\n\t"
        "vftoi4.xy vf1, vf4\n\t"
        "vftoi0.zw vf1, vf4\n\t"
        "sqc2 vf1, 0(%4)"
        : : "r"(point), "r"(matrix), "r"(&camera->screen.screenScale), "r"(&camera->screen.screenOffset), "r"(outVector) : "memory");
}

int MMathRotTransPersClip(Vector4 *outVector, const struct XglCamera *camera, Matrix4 *matrix, const Vector4 *point);
int MMathQuatBetweenVectors(Vector4 *outQuat, const Vector4 *fromUnit, const Vector4 *toUnit);
Vector4 *MMathQuatToMatrix(Vector4 *outVector, const Vector4 *inputQuat);

/*
 * Transforms point by matrix, projects it with the camera's screen scale and offset, and stores
 * the fixed-point screen position in destination with the clip mark in w: 0x8000 when the VU0 clip
 * flags are set, else 0. Returns that mark. The ori/cfc2/nop/movn lines are the library's clip-flag
 * read-out. The original keeps an untouched 16-byte frame: the unused scratch quadword.
 */
int MMathRotTransPersClip(Vector4 *outVector, const XglCamera *camera, Matrix4 *matrix, const Vector4 *point)
{
    Vector4 frameQuad;
    int clipped;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "lqc2 vf2, 0(%2)\n\t"
        "lqc2 vf3, 16(%2)\n\t"
        "lqc2 vf4, 32(%2)\n\t"
        "lqc2 vf5, 48(%2)\n\t"
        "vmulax.xyzw ACC, vf2, vf1x\n\t"
        "vmadday.xyzw ACC, vf3, vf1y\n\t"
        "vmaddaz.xyzw ACC, vf4, vf1z\n\t"
        "vmaddw.xyzw vf1, vf5, vf1w\n\t"
        "vdiv Q, vf0w, vf1w\n\t"
        "ctc2.ni $0, $vi18\n\t"
        "lqc2 vf2, 0(%3)\n\t"
        "lqc2 vf3, 0(%4)\n\t"
        "ori $9, $0, 0x8000\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf4, vf1, Q\n\t"
        "vclipw.xyz vf4xyz, vf0w\n\t"
        "vmula.xyz ACC, vf4, vf2\n\t"
        "vmaddw.xyz vf5, vf3, vf0w\n\t"
        "vftoi4.xy vf1, vf5\n\t"
        "vftoi0.z vf1, vf5\n\t"
        "cfc2.ni %0, $vi18\n\t"
        "nop\n\t"
        "movn %0, $9, %0\n\t"
        "qmtc2.ni %0, vf2\n\t"
        "vmr32.w vf1, vf2\n\t"
        "sqc2 vf1, 0(%5)"
        : "=r"(clipped)
        : "r"(point), "r"(matrix), "r"(&camera->screen.screenScale), "r"(&camera->screen.screenOffset), "r"(outVector)
        : "$9", "memory");
    return clipped;
}

/*
 * Shortest-arc rotation quaternion from the unit vector from to the unit vector to:
 * s = sqrt(2 (1 + from.to)), xyz = (from x to) / s, w = s / 2. Nearly opposite vectors
 * (|s| < 0.01) give (0, 1, 0, 0) and return 0; otherwise returns 1.
 */
int MMathQuatBetweenVectors(Vector4 *outQuat, const Vector4 *fromUnit, const Vector4 *toUnit)
{
    Vector4 arc;

    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
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
        : "=f"(arc.x) : "r"(fromUnit), "r"(toUnit) : "$8", "memory");
    arc.x = __builtin_fabsf(arc.x);
    if (arc.x < 0.01f) {
        __asm__ __volatile__(
            "vsub.xyzw vf4, vf0, vf0\n\t"
            "vaddw.y vf4, vf0, vf0w\n\t"
            "sqc2 vf4, 0(%0)"
            : : "r"(outQuat) : "memory");
        return 0;
    }
    __asm__ __volatile__(
        "viaddi vi1, vi0, 8\n\t"
        "vmfir.w vf6, vi1\n\t"
        "vdiv Q, vf0w, vf3x\n\t"
        "vitof4.w vf6, vf6\n\t"
        "vmulx.w vf5, vf6, vf3x\n\t"
        "vwaitq\n\t"
        "vmulq.xyz vf5, vf5, Q\n\t"
        "sqc2 vf5, 0(%0)"
        : : "r"(outQuat) : "memory");
    return 1;
}

/*
 * VU0 block: loads the quaternion at source, forms its lane products and sums
 * in vf3..vf12, and stores four quadword rows of destination: a 4x4 rotation
 * matrix (row 0 holds 1 - 2(yy+zz)); the last row is vf0 = (0, 0, 0, 1). The
 * only caller, sdvDrawAlter, passes a row block and ignores the returned
 * pointer.
 */
Vector4 *MMathQuatToMatrix(Vector4 *outVector, const Vector4 *inputQuat)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vadd.xyzw vf2, vf1, vf1\n\t"
        "vaddw.xyzw vf9, vf0, vf0w\n\t"
        "vsub.xyzw vf8, vf0, vf0\n\t"
        "vmulz.xyzw vf5, vf2, vf1z\n\t"
        "vmuly.xyzw vf4, vf2, vf1y\n\t"
        "vmulx.xyzw vf3, vf2, vf1x\n\t"
        "vsub.xyzw vf7, vf0, vf0\n\t"
        "vsub.xyzw vf6, vf0, vf0\n\t"
        "vaddz.y vf10, vf4, vf5z\n\t"
        "vaddx.z vf11, vf5, vf3x\n\t"
        "vaddy.x vf12, vf3, vf4y\n\t"
        "vaddw.x vf8, vf5, vf4w\n\t"
        "vsubw.x vf7, vf4, vf5w\n\t"
        "vsuby.x vf6, vf9, vf10y\n\t"
        "vsubw.y vf8, vf5, vf3w\n\t"
        "vsubz.y vf7, vf9, vf11z\n\t"
        "vaddw.y vf6, vf3, vf5w\n\t"
        "vsubx.z vf8, vf9, vf12x\n\t"
        "vaddw.z vf7, vf4, vf3w\n\t"
        "vsubw.z vf6, vf3, vf4w\n\t"
        "sqc2 vf0, 48(%0)\n\t"
        "sqc2 vf8, 32(%0)\n\t"
        "sqc2 vf7, 16(%0)\n\t"
        "sqc2 vf6, 0(%0)"
        : : "r"(outVector), "r"(inputQuat) : "memory");
    return outVector;
}

/*
 * The same VU0 operations as MMathQuatToMatrix, with the three row zeroings and
 * the vf9 sum scheduled differently. No caller in this unit.
 */
Vector4 *MMathQuatToMatrixReordered(Vector4 *outVector, const Vector4 *inputQuat)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%1)\n\t"
        "vadd.xyzw vf2, vf1, vf1\n\t"
        "vsub.xyzw vf8, vf0, vf0\n\t"
        "vsub.xyzw vf7, vf0, vf0\n\t"
        "vsub.xyzw vf6, vf0, vf0\n\t"
        "vmulz.xyzw vf5, vf2, vf1z\n\t"
        "vmuly.xyzw vf4, vf2, vf1y\n\t"
        "vmulx.xyzw vf3, vf2, vf1x\n\t"
        "vaddw.xyzw vf9, vf0, vf0w\n\t"
        "vaddz.y vf10, vf4, vf5z\n\t"
        "vaddx.z vf11, vf5, vf3x\n\t"
        "vaddy.x vf12, vf3, vf4y\n\t"
        "vaddw.x vf8, vf5, vf4w\n\t"
        "vsubw.x vf7, vf4, vf5w\n\t"
        "vsuby.x vf6, vf9, vf10y\n\t"
        "vsubw.y vf8, vf5, vf3w\n\t"
        "vsubz.y vf7, vf9, vf11z\n\t"
        "vaddw.y vf6, vf3, vf5w\n\t"
        "vsubx.z vf8, vf9, vf12x\n\t"
        "vaddw.z vf7, vf4, vf3w\n\t"
        "vsubw.z vf6, vf3, vf4w\n\t"
        "sqc2 vf0, 48(%0)\n\t"
        "sqc2 vf8, 32(%0)\n\t"
        "sqc2 vf7, 16(%0)\n\t"
        "sqc2 vf6, 0(%0)"
        : : "r"(outVector), "r"(inputQuat) : "memory");
    return outVector;
}

int MMathRandomIndex = 625;
int MMathRandomLastSeed = 0;
u32 MMathRandomState[MT_STATE_WORDS];

u32 MMathRandomMag01[4] = { 0, 0x9908B0DFu, 0, 0 };
Matrix4 MMathCatmullRomBasis = {
    { -0.5f, 1.5f, -1.5f, 0.5f },
    { 1.0f, -2.5f, 2.0f, -0.5f },
    { -0.5f, 0.0f, 0.5f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
};
Matrix4 MMathCatmullRomBasisXYZ = {
    { -0.5f, 1.5f, -1.5f, 0.5f },
    { 1.0f, -2.5f, 2.0f, -0.5f },
    { -0.5f, 0.0f, 0.5f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
};
Matrix4 MMathBezierBasis = {
    { -1.0f, 3.0f, -3.0f, 1.0f },
    { 3.0f, -6.0f, 3.0f, 0.0f },
    { -3.0f, 3.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0.0f },
};
float MMathBezierBasisXYZ[13] = {
    -1.0f, 3.0f, -3.0f, 1.0f, 3.0f, -6.0f, 3.0f, 0.0f, -3.0f, 3.0f, 0.0f, 0.0f, 1.0f,
};
