/*
 * OV02 original TU 10: 0x00a84998..0x00a852d8 (19 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84998);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84A10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84A90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84B08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84B78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84BF0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84C68);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84CE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84D58);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84DD0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84E48);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84EC0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84F38);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A84FB0);

typedef unsigned char u8;
typedef short s16;
typedef int s32;
u8 *ov02DataSectionGet(s32 offset);

/* Returns the string with the given id (0 for id 0); the table of s16 offsets sits at base + id * 2 - 2. */
u8 *stringTableGet(s32 stringId)
{
  u8 *stringTableBase;
  if (stringId == 0)
  {
    return 0;
  }
  stringTableBase = ov02DataSectionGet(0x64);
  return stringTableBase + ((s16 *) stringTableBase)[stringId - 1];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A85070);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A850F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A85160);

INCLUDE_ASM("asm/nonmatchings/ov02/tu010", func_00A85208);
