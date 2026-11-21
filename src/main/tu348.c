#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef struct UndulateVector {
  s32 x;
  s32 y;
  s32 z;
  f32 w;
} UndulateVector;

typedef struct UndulatePointSet {
  UndulateVector points[16];
  u8 vectorFlags[16];
} UndulatePointSet;

extern UndulatePointSet gUndulatePointSet[];

void undulateResetVectors(void)
{
  s32 i;
  UndulatePointSet *pointSet;
  u8 *flagCursor;
  UndulateVector *pointCursor;
  pointSet = gUndulatePointSet;
  flagCursor = pointSet->vectorFlags;
  pointCursor = pointSet->points;
  for (i = 0; i < 16; i++)
  {
    pointCursor->x = 0;
    pointCursor->y = 0;
    pointCursor->z = 0;
    pointCursor->w = 1.0f;
    pointCursor += 1;
    *flagCursor = 0;
    flagCursor += 1;
  }

}

INCLUDE_ASM("asm/main/nonmatchings/tu348", func_002A4EB0);

INCLUDE_ASM("asm/main/nonmatchings/tu348", func_002A4F88);

extern unsigned char gUndulateVectorFlags[];

void undulateSetVectorEnabled(s32 vectorIndex, s32 isEnabled) {
    u8 *enabledFlag;
    u8 *disabledFlag;

    if (isEnabled != 0) {
        enabledFlag = &gUndulateVectorFlags[vectorIndex];
        *enabledFlag &= 0xFB;
        return;
    }
    disabledFlag = &gUndulateVectorFlags[vectorIndex];
    *disabledFlag |= 4;
}

INCLUDE_ASM("asm/main/nonmatchings/tu348", func_002A50C0);

INCLUDE_ASM("asm/main/nonmatchings/tu348", func_002A5200);
