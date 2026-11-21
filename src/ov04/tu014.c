/*
 * OV04 original TU 14: 0x00a9d408..0x00a9dd40 (8 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu014", func_00A9D408);

typedef short s16;
typedef int s32;
extern s16 D_00AA7768[];
extern s16 D_00AA77C8[];

/* Clears the 48 halfwords of D_00AA7768 and the four 11-halfword rows of D_00AA77C8 (row stride 0x16). */
void resetHalfwordTables(void)
{
  s32 i;
  s32 row;
  s32 columnIndex;
  s16 *cell;
  s16 *row_base;

  cell = D_00AA7768;
  cell = cell + 47;
  for (i = 0; i < 48; i++)
  {
    *cell = 0;
    cell -= 1;
  }

  row = 0;
  row_base = D_00AA77C8;
  do
  {
    columnIndex = 0xA;
    cell = row_base + 10;
    do
    {
      columnIndex -= 1;

      *cell = 0;
      cell -= 1;
    }
    while (columnIndex >= 0);
    row += 1;
    row_base += 11;
  }
  while (row < 4);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu014", func_00A9D718);

INCLUDE_ASM("asm/nonmatchings/ov04/tu014", func_00A9D770);

INCLUDE_ASM("asm/nonmatchings/ov04/tu014", func_00A9D938);

INCLUDE_ASM("asm/nonmatchings/ov04/tu014", func_00A9DA20);

INCLUDE_ASM("asm/nonmatchings/ov04/tu014", func_00A9DAC0);

s32 seVoiceStartDriver(s32 handle, s16 value);

void soundCueByKind(s16 cueGroup, s16 voiceOffset) {
    s32 voiceHandle;

    voiceHandle = -1;
    switch (cueGroup) {                                 /* irregular */
    case 0:
        voiceHandle = voiceOffset + 0xBF;
        break;
    case 1:
        voiceHandle = voiceOffset + 0xCF;
        break;
    case 2:
        voiceHandle = voiceOffset + 0xDF;
        break;
    case 3:
        voiceHandle = voiceOffset + 0xEF;
        break;
    }
    if (voiceHandle != -1) {
        seVoiceStartDriver(voiceHandle, voiceOffset);
    }
}
