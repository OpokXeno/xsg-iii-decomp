#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* The first word of a blend channel: read-modify-written as one s32 in blendStateInit. */
typedef union BlendStateHead {
    s32 word;
    struct {
        u8 blendMode;           /* +0x00 */
        u8 previousBlendMode;   /* +0x01 */
        s8 unk_02;         /* +0x02 */
        u8 unk_03;         /* +0x03 */
    } headBytes;
} BlendStateHead;

/* Blend channel state. */
typedef struct BlendState {
    BlendStateHead head;   /* +0x00 */
    s32 blendLevel;         /* +0x04 */
    s32 previousBlendLevel; /* +0x08 */
    u8 unk_0C[0x38 - 0x0C];
    f32 unk_38;        /* +0x38: 1.0 after init */
} BlendState;

void blendStateInit(BlendState *blendChannel) {
    blendChannel->unk_38 = 1.0f;
    blendChannel->head.word |= 4;
}

void blendStateSetValue(BlendState *blendChannel, u8 blendMode, s32 blendLevel) {
    blendChannel->previousBlendLevel = blendChannel->blendLevel;
    blendChannel->head.headBytes.previousBlendMode = blendChannel->head.headBytes.blendMode;
    blendChannel->blendLevel = blendLevel;
    blendChannel->head.headBytes.blendMode = blendMode;
    blendChannel->head.headBytes.unk_02 = 0;
}

f32 clampedRatio(f32 position, f32 startPosition, f32 travel)
{
  f32 difference;
  f32 ratio;
  f32 quotient;

  difference = startPosition;
  difference = position - difference;
  ratio = 0.0f;
  if (!(difference < 0.0f))
  {
    ratio = 1.0f;
    quotient = difference / travel;
    if (quotient > ratio)
    {
      quotient = ratio;
    }
    ratio = quotient;
  }
  return ratio;
}

INCLUDE_ASM("asm/main/nonmatchings/tu272", func_00268B10);

INCLUDE_ASM("asm/main/nonmatchings/tu272", func_00268BE0);

INCLUDE_ASM("asm/main/nonmatchings/tu272", func_00268D98);

INCLUDE_ASM("asm/main/nonmatchings/tu272", func_00268E50);
