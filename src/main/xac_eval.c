#include "common.h"
#include "shared.h"
#include "main/TiXacAnimCurve.h"

f32 xacGetKeyValue(TiXacKey *pKeyHeader);
void xacGetStartKeyValueAngle(TiXacKey *keyHeader, f32 *keyValue, f32 *outTangentAngle);
void xacGetEndKeyValueAngle(TiXacKey *keyHeader, f32 *keyValue, f32 *inTangentAngle);
f32 xacGetKeyOutTangentAngle(TiXacKey *key);
f32 xacGetKeyInTangentAngle(TiXacKey *key);
f32 xacEvalCubicSegment(f32 animTime, const TiXacKey *key, const f32 *coeffs);
f32 xacEvalLinearKeys(f32 animTime, TiXacKey *key, TiXacKey *nextKey);
extern f32 tiXacAnimCurveTimeScale;

INCLUDE_ASM("asm/main/nonmatchings/xac_eval", func_002A49F0);

/* Reads two key values of one key header: the first through xacGetKeyValue, the second through xacGetKeyOutTangentAngle. */
void xacGetStartKeyValueAngle(TiXacKey *keyHeader, f32 *keyValue, f32 *outTangentAngle)
{
    *keyValue = xacGetKeyValue(keyHeader);
    *outTangentAngle = xacGetKeyOutTangentAngle(keyHeader);
}

void xacGetEndKeyValueAngle(TiXacKey *keyHeader, f32 *keyValue, f32 *inTangentAngle)
{
    *keyValue = xacGetKeyValue(keyHeader);
    *inTangentAngle = xacGetKeyInTangentAngle(keyHeader);
}

f32 cosf(f32 x);
f32 sinf(f32 x);
void func_002A49F0(f32 *coeffs, f32 *times, f32 *values);

/* Evaluates the curve of one key pair at time: the two key segments (position and tangent) are built from the key values and the trig of each second value, then the cubic is evaluated. */
f32 xacEvalCubicKeys(f32 animTime, TiXacKey *key, TiXacKey *nextKey)
{
    f32 keyValue;
    f32 keyOutTangent;
    f32 nextKeyValue;
    f32 nextKeyInTangent;
    f32 bezierTimes[4];
    f32 bezierValues[4];
    f32 coeffs[4];

    xacGetStartKeyValueAngle(key, &keyValue, &keyOutTangent);
    xacGetEndKeyValueAngle(nextKey, &nextKeyValue, &nextKeyInTangent);
    bezierTimes[0] = key->origin * (1.0f / 30.0f);
    bezierValues[0] = keyValue;
    bezierTimes[1] = bezierTimes[0] + cosf(keyOutTangent);
    bezierValues[1] = bezierValues[0] + sinf(keyOutTangent);
    bezierTimes[3] = nextKey->origin * (1.0f / 30.0f);
    bezierValues[3] = nextKeyValue;
    bezierTimes[2] = bezierTimes[3] - cosf(nextKeyInTangent);
    bezierValues[2] = bezierValues[3] - sinf(nextKeyInTangent);
    func_002A49F0(coeffs, bezierTimes, bezierValues);
    return xacEvalCubicSegment(animTime, key, coeffs);
}

/* Cubic segment of one key: coeffs[0] is the leading term, coeffs[3] the constant (Horner form). */
f32 xacEvalCubicSegment(f32 animTime, const TiXacKey *key, const f32 *coeffs)
{
    f32 segmentTime;

    segmentTime = (animTime - key->origin) * tiXacAnimCurveTimeScale;
    return segmentTime * (segmentTime * (segmentTime * coeffs[0] + coeffs[1]) + coeffs[2]) + coeffs[3];
}

/* Blends the value of one key with the next key over the segment between them; a zero span counts as one frame. */
f32 xacEvalLinearKeys(f32 animTime, TiXacKey *key, TiXacKey *nextKey)
{
    f32 elapsedFrames;
    f32 segmentSeconds;
    f32 elapsedSeconds;
    f32 fraction;
    f32 startKeyValue;
    f32 endKeyValue;

    elapsedFrames = animTime - key->origin;
    elapsedSeconds = elapsedFrames / 30.0f;
    segmentSeconds = nextKey->origin - key->origin;
    if (segmentSeconds == 0.0f) {
        segmentSeconds = 1.0f;
    }
    segmentSeconds = segmentSeconds / 30.0f;
    startKeyValue = xacGetKeyValue(key);
    endKeyValue = xacGetKeyValue(nextKey);
    fraction = elapsedSeconds / segmentSeconds;
    return startKeyValue + (endKeyValue - startKeyValue) * fraction;
}
