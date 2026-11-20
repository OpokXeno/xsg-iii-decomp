#include "common.h"
#include "shared.h"
#include "main/TiXacAnimCurve.h"

f32 xacGetKeyValue(TiXacKey *pKeyHeader);
void xacGetStartKeyValueAngle(TiXacKey *keyHeader, f32 *first, f32 *second);
void xacGetEndKeyValueAngle(TiXacKey *keyHeader, f32 *first, f32 *second);
f32 xacGetKeyOutTangentAngle(TiXacKey *key);
f32 xacGetKeyInTangentAngle(TiXacKey *key);
f32 xacEvalCubicSegment(f32 value, const TiXacKey *record, const f32 *coeffs);
f32 xacEvalLinearKeys(f32 time, TiXacKey *key, TiXacKey *nextKey);
extern f32 tiXacAnimCurveTimeScale;

INCLUDE_ASM("asm/main/nonmatchings/xac_eval", func_002A49F0);

/* Reads two key values of one key header: the first through xacGetKeyValue, the second through xacGetKeyOutTangentAngle. */
void xacGetStartKeyValueAngle(TiXacKey *keyHeader, f32 *first, f32 *second)
{
    *first = xacGetKeyValue(keyHeader);
    *second = xacGetKeyOutTangentAngle(keyHeader);
}

void xacGetEndKeyValueAngle(TiXacKey *keyHeader, f32 *first, f32 *second)
{
    *first = xacGetKeyValue(keyHeader);
    *second = xacGetKeyInTangentAngle(keyHeader);
}

f32 cosf(f32 x);
f32 sinf(f32 x);
void func_002A49F0(f32 *coeffs, f32 *times, f32 *values);

/* Evaluates the curve of one key pair at time: the two key segments (position and tangent) are built from the key values and the trig of each second value, then the cubic is evaluated. */
f32 func_002A4B78(f32 time, TiXacKey *key, TiXacKey *nextKey)
{
    f32 keyFirst;
    f32 keySecond;
    f32 nextFirst;
    f32 nextSecond;
    f32 times[4];
    f32 values[4];
    f32 coeffs[4];

    xacGetStartKeyValueAngle(key, &keyFirst, &keySecond);
    xacGetEndKeyValueAngle(nextKey, &nextFirst, &nextSecond);
    times[0] = (f32) key->origin * (1.0f / 30.0f);
    values[0] = keyFirst;
    times[1] = times[0] + cosf(keySecond);
    values[1] = values[0] + sinf(keySecond);
    times[3] = (f32) nextKey->origin * (1.0f / 30.0f);
    values[3] = nextFirst;
    times[2] = times[3] - cosf(nextSecond);
    values[2] = values[3] - sinf(nextSecond);
    func_002A49F0(coeffs, times, values);
    return xacEvalCubicSegment(time, key, coeffs);
}

/* Cubic segment of one key: coeffs[0] is the leading term, coeffs[3] the constant (Horner form). */
f32 xacEvalCubicSegment(f32 value, const TiXacKey *record, const f32 *coeffs)
{
    f32 scaled;

    scaled = (value - (f32) record->origin) * tiXacAnimCurveTimeScale;
    return scaled * (scaled * (scaled * coeffs[0] + coeffs[1]) + coeffs[2]) + coeffs[3];
}

/* Blends the value of one key with the next key over the segment between them; a zero span counts as one frame. */
f32 xacEvalLinearKeys(f32 time, TiXacKey *key, TiXacKey *nextKey)
{
    f32 offset;
    f32 span;
    f32 offsetScaled;
    f32 fraction;
    f32 valueA;
    f32 valueB;

    offset = time - (f32) key->origin;
    offsetScaled = offset / 30.0f;
    span = (f32) (nextKey->origin - key->origin);
    if (span == 0.0f) {
        span = 1.0f;
    }
    span = span / 30.0f;
    valueA = xacGetKeyValue(key);
    valueB = xacGetKeyValue(nextKey);
    fraction = offsetScaled / span;
    return valueA + (valueB - valueA) * fraction;
}
