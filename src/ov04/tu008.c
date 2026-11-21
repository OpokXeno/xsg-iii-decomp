/*
 * OV04 original TU 8: 0x00a98450..0x00a9a2a0 (26 functions)
 */
#include "common.h"

typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

typedef struct PlacedRecord {
    s16 unk_00;
    u8 unk_02[0x2];
    f32 unk_04;
    f32 unk_08;
    u8 unk_0C[0x4];
    f32 posX;
    f32 posY;
    f32 valueX;
    f32 valueY;
    u8 unk_20[0x4];
    f32 offsetX;
    f32 offsetY;
    u8 unk_2C[0x4];
    f32 scaleX;
    f32 scaleY;
    u8 unk_38[0x38];
    s16 unk_70;
    u8 unk_72[0x2];
    s32 unk_74;
    u8 unk_78[0x8];
} PlacedRecord;

extern PlacedRecord *gPlacedRecordTable;

void initPlacedRecordTable(void)
{
  s32 recordNumber;
  PlacedRecord *placedRecord;
  placedRecord = gPlacedRecordTable;
  for (recordNumber = 0; recordNumber < 126; recordNumber++)
  {
    placedRecord->unk_00 = -1;
    placedRecord->unk_04 = placedRecord->unk_08 = 0.0f;
    placedRecord->unk_74 = 0;
    placedRecord->unk_70 = -1;
    placedRecord++;
  }

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98490);

void setRecordPosition(s16 recordNumber, s16 posX, s16 posY, f32 valueX, f32 valueY) {
    PlacedRecord *placedRecord;

    placedRecord = &gPlacedRecordTable[recordNumber];
    placedRecord->posX = posX;
    placedRecord->posY = posY;
    placedRecord->valueX = valueX;
    placedRecord->valueY = valueY;
}

void recordSetScaleXY(s16 recordNumber, f32 scaleX, f32 scaleY) {
    PlacedRecord *placedRecord;

    placedRecord = &gPlacedRecordTable[recordNumber];
    placedRecord->scaleX = scaleX;
    placedRecord->scaleY = scaleY;
}

void recordSetOffsetXY(s16 recordNumber, f32 offsetX, f32 offsetY) {
    PlacedRecord *placedRecord;

    placedRecord = &gPlacedRecordTable[recordNumber];
    placedRecord->offsetX = offsetX;
    placedRecord->offsetY = offsetY;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98608);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98678);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A986D0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98730);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A989B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98AD8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98CC0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A98E10);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A990B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A992A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A995E8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99728);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99788);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99890);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99A30);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99B60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99DB8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A99F98);

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A9A1D8);

extern f32 D_00AA5B80[];

void setGlobalVec3(f32 x, f32 y, f32 z) {
    D_00AA5B80[0] = x;
    D_00AA5B80[1] = y;
    D_00AA5B80[2] = z;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu008", func_00A9A230);
