#include "common.h"

typedef int s32;
typedef short s16;
typedef float f32;
extern s32 gRgSingletonIdEnabled;

void rgSingletonClearFlag(void) {
    gRgSingletonIdEnabled = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu288", func_0026EED0);

f32 rgSingletonWriteRect(s16 *rectOut, f32 *coords, f32 yOffset)
{
  f32 rectTopY;
  rectTopY = coords[1] + yOffset;
  rectOut[0] = 0x6F88;
  rectOut[2] = -0x6F98;
  rectOut[3] = ((((s32) (coords[3] - yOffset)) * 0x10) + 0x71F8);
  rectOut[1] = ((((s32) rectTopY) * 0x10) + 0x71F8);
  return rectTopY;
}

INCLUDE_ASM("asm/main/nonmatchings/tu288", func_0026F160);

INCLUDE_ASM("asm/main/nonmatchings/tu288", func_0026F220);
