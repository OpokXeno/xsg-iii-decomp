#include "common.h"

#include "shared.h"
typedef int s32;

void vec2Scale(Vector4 *outVec, const Vector4 *inVec, f32 scale) {
    outVec->x = inVec->x * scale;
    outVec->y = inVec->y * scale;
}

float CheckDist2DXY(const Vector4 *fromPos, const Vector4 *toPos)
{
    return __builtin_sqrtf((toPos->x - fromPos->x) * (toPos->x - fromPos->x)
                           + (toPos->y - fromPos->y) * (toPos->y - fromPos->y));
}

void vec2Add(Vector4 *outVec, const Vector4 *lhsVec, const Vector4 *rhsVec) {
    outVec->x = lhsVec->x + rhsVec->x;
    outVec->y = lhsVec->y + rhsVec->y;
}

void vec2Sub(Vector4 *outVec, const Vector4 *lhsVec, const Vector4 *rhsVec) {
    outVec->x = lhsVec->x - rhsVec->x;
    outVec->y = lhsVec->y - rhsVec->y;
}

void vec2Scale(Vector4 *outVec, const Vector4 *inVec, f32 scale);
/* Writes the length of the XY part of *inVec to *length (INCLUDE_ASM). */
void func_00264DD8(f32 *length, Vector4 *inVec);

void vec2Normalize(Vector4 *outVec, Vector4 *inVec) {
    f32 length;

    func_00264DD8(&length, inVec);
    vec2Scale(outVec, inVec, 1.0f / length);
}

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264DD8);

void vec2ScaleAdd(Vector4 *outVec, const Vector4 *scaledVec, const Vector4 *addVec, f32 scale) {
    outVec->x = scaledVec->x * scale + addVec->x;
    outVec->y = scaledVec->y * scale + addVec->y;
}
