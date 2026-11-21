/*
 * OV02 original TU 17: 0x00a874a0..0x00a88a08 (23 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A874A0);

typedef int s32;
typedef unsigned char u8;

/* One 0x58-byte record of gOv02ExpRecords; the experience points at +0x08 are the ones ov02AddRecordPoints adds to. */
typedef struct Ov02ExpRecord {
    unsigned char unk_00[0x8];
    s32 expPoints;
    unsigned char unk_0C[0x58 - 0x0C];
} Ov02ExpRecord;

extern Ov02ExpRecord gOv02ExpRecords[];

/* The class flags byte at +0x09 of the slot returned by ov02BattleSlotGet. */
typedef struct Ov02ClassSlot {
    unsigned char unk_00[0x9];
    unsigned char classFlags;
} Ov02ClassSlot;

/* Returns the 0x58-byte record number record_number (1-based) of gOv02ExpRecords, or null when record_number is out of range. */
Ov02ExpRecord *ov02GetExpRecord(s32 record_number) {
    if (!(record_number < 0x12)) {
        return 0;
    }
    return &gOv02ExpRecords[record_number - 1];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87590);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87608);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A876C8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87740);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A877A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87820);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A878E0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87990);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87A88);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87B68);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87C18);

/* Level for the experience points: the thresholds are 6, 15, 27, ... (each step grows by 3 more). Returns
 * the number of thresholds reached, minus one. */
s32 ov02LevelFromPoints(s32 expPoints)
{
  s32 threshold;
  s32 thresholdStep;
  s32 level;

  threshold = 6;
  level = 2;
  if (expPoints >= threshold)
  {
    thresholdStep = 6;
    do
    {
      thresholdStep += 3;
      threshold += thresholdStep;
      level += 1;
    }
    while (expPoints >= threshold);
  }
  return level - 1;
}

s32 ov02LevelFromPoints(s32 expPoints);
s32 func_00A87B68(void);

/* Clamps the level from the experience points of func_00A87B68 to 1..25. */
s32 ov02GetClampedLevel(void)
{
  s32 level;
  s32 expPoints;

  expPoints = func_00A87B68();
  level = ov02LevelFromPoints(expPoints);
  if (level <= 0)
  {
    level = 1;
  }
  else
  {
    if (level > (0x1A - 1))
    {
      level = 0x19;
    }
  }
  return level;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87DC0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87E08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87EE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A88248);

Ov02ExpRecord *ov02GetExpRecord(s32 record_number);

/* Adds arg1 to the experience points at +8 of record arg0 and clamps them to 0..99999. */
void ov02AddRecordPoints(s32 record_number, s32 points_delta) {
    s32 expPoints;
    Ov02ExpRecord *expRecord;

    expRecord = ov02GetExpRecord(record_number);
    expPoints = expRecord->expPoints + points_delta;
    expRecord->expPoints = expPoints;
    if (expPoints < 0) {
        expRecord->expPoints = 0;
        return;
    }
    if (expPoints > 0x1869F) {
        expRecord->expPoints = 0x1869F;
    }
}

Ov02ClassSlot *ov02BattleSlotGet(s32 slotNumber);

s32 ov02GetRecordClass(s32 unused_first, s32 unused_second, s32 slotNumber)
{
  s32 recordClass;
  u8 classFlags;

  classFlags = ov02BattleSlotGet(slotNumber)->classFlags;
  recordClass = 0x802;
  if (!(classFlags & 4))
  {
    recordClass = 2;
    if (!((classFlags & 1) & 0xFF))
    {
      recordClass = 4;
      if (!(classFlags & 2))
      {
        recordClass = (classFlags & 8) ? (6) : (0xB);
      }
    }
  }
  return recordClass;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A88418);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A88538);
