#include "common.h"
#include "shared.h"

float srsAtan2(float x, float y);

/* Row-major 4x4 float matrix. */

float vu0CopyMatrixWithTranslation(Vector4 *dstMatrix, Matrix4 srcMatrix, Vector4 *translationRow, Vector4 *direction);
float MMathCalcDist(const Vector4 *first, const Vector4 *second);

/* Copies a matrix: rows 0-2 from srcMatrix, row 3 from the translation vector translationRow. Returns the distance from
 * translationRow to direction. */
float vu0CopyMatrixWithTranslation(Vector4 *dstMatrix, Matrix4 srcMatrix, Vector4 *translationRow, Vector4 *direction)
{
    float (*source)[4];
    Vector4 *translation = translationRow;
    Vector4 *dstRows = dstMatrix;
    float distance = MMathCalcDist(translation, direction);
    source = srcMatrix;
    __asm__ __volatile__("lqc2 vf26, 0(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("lqc2 vf27, 16(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("lqc2 vf28, 32(%0)" : : "r"(source) : "memory");
    __asm__ __volatile__("lqc2 vf29, 0(%0)" : : "r"(translation) : "memory");
    __asm__ __volatile__("sqc2 vf26, 0(%0)" : : "r"(dstRows) : "memory");
    __asm__ __volatile__("sqc2 vf27, 16(%0)" : : "r"(dstRows) : "memory");
    __asm__ __volatile__("sqc2 vf28, 32(%0)" : : "r"(dstRows) : "memory");
    __asm__ __volatile__("sqc2 vf29, 48(%0)" : : "r"(dstRows) : "memory");
    return distance;
}

#include "vu0_macro.h"

/* A key of a Hermite track: the point, then the key time (signed short at +12) and the curve-and-ease byte (curve type in the
 * low nibble, ease index in the high nibble) at +15. A time of MMATH_HERMITE_END ends the track. */
typedef struct MMathHermiteKey {
    float x;
    float y;
    float z;
    short time;
    unsigned char unmodeled_0E;
    unsigned char curveAndEase;
} MMathHermiteKey;

#define MMATH_HERMITE_END 0x1000

float sefEaseCurve(float offset, float span, int ease);
void MMathHermiteKeyNeighbors(const MMathHermiteKey *key, Vector4 **first, Vector4 **second, Vector4 **third, Vector4 **fourth);
void MMathLerpKeyPoint(Vector4 *outPoint, const Vector4 *start, const Vector4 *next, float parameter);
void MMathCalcHermitePrm(Vector4 *tangent_start, Vector4 *tangent_end, const Vector4 *point_prev,
                         const Vector4 *point_start, const Vector4 *point_end, const Vector4 *point_next);
float *MMathCalcHermite(float *outPoint, Vector4 *tangent_first, Vector4 *tangent_second,
                        Vector4 *endpoint_first, Vector4 *endpoint_second, float parameter);

/* The point of the Hermite track `keys` at `time`: the origin without keys, the first or last key's point outside
 * the keys, else the curve between the two keys around `time`. */
void MMathEvalHermiteKeys(Vector4 *outPoint, MMathHermiteKey *keys, float time)
{
    Vector4 tangent[2];
    Vector4 *points[4];
    MMathHermiteKey *trackStart;
    MMathHermiteKey *firstKey;
    MMathHermiteKey *nextKey;
    float parameter;
    int frame;
    int keySegment;
    int keyCount;
    int keyIndex;

    frame = time;
    trackStart = keys;
    keySegment = -1;
    keyIndex = 0;
    keyCount = 0;
    for (; *&keys[keyIndex].time != MMATH_HERMITE_END; keyIndex++) {
        if (frame < keys[keyIndex].time && keySegment < 0) {
            keySegment = keyIndex - 1;
        }
        keyCount++;
    }
    if (keyCount <= 0) {
        Vu0SetOrigin(outPoint);
        return;
    }
    if (keyCount == 1) {
        Vu0MMathCopyPoint(outPoint, trackStart);
        return;
    }
    if (frame < keys[0].time) {
        keyCount = 1;
    }
    if (keySegment < 0 || keySegment == keyCount - 1) {
        Vu0MMathCopyPoint(outPoint, &keys[keyCount - 1]);
        return;
    }
    parameter = sefEaseCurve(time - keys[keySegment].time, keys[keySegment + 1].time - keys[keySegment].time, keys[keySegment].curveAndEase >> 4);
    firstKey = &keys[keySegment];
    nextKey = firstKey + 1;
    switch (firstKey->curveAndEase & 0xf) {
    case 0:
    default:
        MMathLerpKeyPoint(outPoint, (const Vector4 *) firstKey, (const Vector4 *) nextKey, parameter);
        break;
    case 1:
        MMathHermiteKeyNeighbors(firstKey, &points[0], &points[1], &points[2], &points[3]);
        MMathCalcHermitePrm(&tangent[0], &tangent[1], points[0], points[1], points[2], points[3]);
        MMathCalcHermite(&outPoint->x, &tangent[0], &tangent[1], points[1], points[2], parameter);
        break;
    }
}

/* Converts a direction into pitch (angles->x) and yaw (angles->y). */
void vu0DirectionToAngles(Vector4 *angles, Vector4 *direction, int unusedArg2, int unusedArg3) {
    float negatedY;
    float horizontalLength;
    float pitch;
    float yaw;

    negatedY = -direction->y;
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(direction) : "memory");
    __asm__ __volatile__("vmul.xz vf1, vf1, vf1" : : : "memory");
    __asm__ __volatile__("vaddz.x vf1, vf1, vf1z" : : : "memory");
    __asm__ __volatile__("vsqrt Q, vf1x" : : : "memory");
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(angles) : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vaddq.x vf1, vf0, Q" : : : "memory");
    __asm__ __volatile__("qmfc2.ni $8, vf1\n\tmtc1 $8, %0" : "=f"(horizontalLength) : : "$8", "memory");
    pitch = srsAtan2(negatedY, horizontalLength);
    angles->x = pitch;
    yaw = srsAtan2(direction->x, direction->z);
    angles->y = yaw;
}
