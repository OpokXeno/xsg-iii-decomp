/*
 * OV02 original TU 66: 0x00a9e668..0x00a9e738 (3 functions)
 */
#include "common.h"

typedef int s32;
typedef float f32;

/* Four 32-bit offsets, each relative to the block they are stored in. */
typedef struct ResolvedBlockOffsets {
    s32 firstRelOffset;
    s32 secondRelOffset;
    s32 thirdRelOffset;
    s32 fourthRelOffset;
} ResolvedBlockOffsets;

void blockResolveFourOffsets(void *relativeBase, void **resolvedFirstBlock, void **resolvedSecondBlock, void **resolvedThirdBlock, void **resolvedFourthBlock) {
    ResolvedBlockOffsets *resolvedOffsets;

    resolvedOffsets = relativeBase;
    *resolvedFirstBlock = relativeBase + resolvedOffsets->firstRelOffset;
    *resolvedSecondBlock = relativeBase + resolvedOffsets->secondRelOffset;
    *resolvedThirdBlock = relativeBase + resolvedOffsets->thirdRelOffset;
    *resolvedFourthBlock = relativeBase + resolvedOffsets->fourthRelOffset;
}

void floatLerpStore(f32 *lerpResult, f32 blendFactor, f32 lerpFrom, f32 lerpTo) {
    *lerpResult = ((lerpTo - lerpFrom) * blendFactor) + lerpFrom;
}

f32 floatCubicEvaluate(f32 outerLeftValue, f32 tSquared, f32 tCubed, f32 startNeighbor, f32 hermiteStart, f32 hermiteEnd, f32 endNeighbor)
{
  f32 coefficientB;
  f32 minusTwo;
  f32 hermiteStartCopy;
  f32 threeTSquared;
  f32 zeroWeight;
  threeTSquared = tSquared * 3.0f;
  zeroWeight = (((hermiteStart - startNeighbor) + hermiteEnd) - (hermiteStartCopy = hermiteStart)) * 0.0f;
  return ((((((2.0f * tCubed) - threeTSquared) * hermiteStart) + hermiteStart) + (((coefficientB = tCubed - (2.0f * tSquared)) + outerLeftValue) * zeroWeight)) + ((tCubed - tSquared) * ((((hermiteEnd - hermiteStartCopy) + endNeighbor) - hermiteEnd) * 0.0f))) + (((tCubed * (minusTwo = -2.0f)) + threeTSquared) * hermiteEnd);
}
