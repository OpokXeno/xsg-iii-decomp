#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu281", func_0026B818);

typedef signed char s8;
typedef float f32;
typedef unsigned char u8;
typedef int s32;

/* rect holds x, y, z, w floats: the midpoint of the (x, y) and (z, w) corners. */
void rectCenter(f32 *center, const f32 *rect) {
    center[0] = (rect[0] + rect[2]) * 0.5f;
    center[1] = (rect[1] + rect[3]) * 0.5f;
}

s32 stepStateSetOrigin(void *, f32, f32);
s32 stepStateSetRectFixed(void *, f32, f32, f32, f32);

/* Step cursor state. Fields not yet named keep their offsets. */
typedef struct TextStepCursor {
    u8 unk_00[0x14];
    f32 scale;      /* +0x14: divisor that turns a position into a ratio */
    u8 unk_18[0x20 - 0x18];
    f32 originX;     /* +0x20 */
    f32 originY;     /* +0x24 */
    f32 rectEndX;     /* +0x28 */
    f32 rectEndY;     /* +0x2C */
    u8 unk_30[0x44 - 0x30];
    s32 interpolationFlags;      /* +0x44: bit 0 selects the interpolated path */
    s32 extraOffsetEnabled;     /* +0x48: adds 25.0 to the position when set */
    u8 unk_4C[0xA4 - 0x4C];
    u8 textStepIndex;       /* +0xA4: current step index */
    u8 unk_A5[0xA7 - 0xA5];
    s8 stepDelta;      /* +0xA7: position minus the previous position, written on each step */
    u8 unk_A8;
    s8 positionBias;      /* +0xA9: adds twice its value to the position */
} TextStepCursor;

void textStepSetIndex(TextStepCursor *cursor, s32 newTextStepIndex)
{
  f32 scale;
  f32 rectEndY;
  f32 position;
  s32 interpolated;

  if (newTextStepIndex == cursor->textStepIndex)
  {
    cursor->stepDelta = 0;
    cursor->textStepIndex = newTextStepIndex;
    return;
  }
  interpolated = cursor->interpolationFlags & 1;
  position = (newTextStepIndex * 0x1B) + (cursor->positionBias * 2);
  if (!(interpolated == 0))
  {
    if (cursor->extraOffsetEnabled != 0)
    {
      position += 25.0f;
    }
    scale = cursor->scale;
    rectEndY = cursor->rectEndY;
    cursor->stepDelta = position - scale;
    stepStateSetRectFixed(cursor, cursor->originX, rectEndY - ((rectEndY - cursor->originY) * (position / scale)), cursor->rectEndX, rectEndY);
  }
  else
  {
    stepStateSetOrigin(cursor, cursor->originX, cursor->originY);
  }
  cursor->scale = position;
  cursor->textStepIndex = newTextStepIndex;
}
