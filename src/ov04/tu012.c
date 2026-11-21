/*
 * OV04 original TU 12: 0x00a9a9d0..0x00a9c918 (14 functions)
 */
#include "common.h"

typedef signed char s8;
typedef short s16;
typedef int s32;
extern s16 ov04LastFoundIndex;
extern s32 D_00AA9088;
extern s8 *ov04NotFoundFlagPtr;
s16 func_00A9AA50(s16, s32);
s32 func_00A9B110(s32, s16, s32);

void ov04ProcessEntry(s16 searchKey, s32 mode)
{
  s32 foundIndex;
  foundIndex = (ov04LastFoundIndex = func_00A9AA50(searchKey, D_00AA9088));
  if (foundIndex < 0)
  {
    *ov04NotFoundFlagPtr = 0;
    return;
  }
  func_00A9B110(D_00AA9088, foundIndex, mode);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9AA50);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9B110);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9B450);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9B568);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9BC30);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9BCE8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C120);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C250);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C388);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C468);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C560);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C660);

INCLUDE_ASM("asm/nonmatchings/ov04/tu012", func_00A9C6D8);
