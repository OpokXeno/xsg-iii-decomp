/*
 * OV02 original TU 17: 0x00a874a0..0x00a88a08 (23 functions)
 */
#include "common.h"
#include "main/battle_shared.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A874A0);

typedef int s32;

typedef unsigned char u8;

/* The class flags byte at +0x09 of the slot returned by ov02BattleSlotGet. */
typedef struct Ov02ClassSlot {
    unsigned char unk_00[0x9];
    unsigned char classFlags;
} Ov02ClassSlot;

/* Returns the 0x58-byte record recordNumber (1-based) of gOv02ExpRecords, or null when recordNumber is out of range. */
Ov02ExpRecord *ov02GetExpRecord(s32 recordNumber) {
    if (!(recordNumber < 0x12)) {
        return 0;
    }
    return &gOv02ExpRecords[recordNumber - 1];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87590);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A87608);

INCLUDE_ASM("asm/nonmatchings/ov02/tu017", func_00A876C8);

/* Tests flag number flagNumber (1-based, 1..120) of the flag bits of record recordNumber: returns 1 when that flag is set, 0 when it is clear or out of range. */
s32 ov02ExpRecordFlagIsSet(s32 recordNumber, s32 flagNumber)
{
    Ov02ExpRecord *expRecord;
    s32 flagBitPosition;

    flagNumber = flagNumber - 1;
    if ((unsigned int)flagNumber >= 120) {
        return 0;
    }
    flagBitPosition = flagNumber;
    flagNumber = 0;
    expRecord = ov02GetExpRecord(recordNumber);
    return ((unsigned int)expRecord->flagBits[flagBitPosition / 8] >> (flagBitPosition % 8)) & 1;
}

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

/* Adds pointsDelta to the experience points of record recordNumber and clamps them to 0..99999. */
void ov02AddRecordPoints(s32 recordNumber, s32 pointsDelta) {
    s32 expPoints;
    Ov02ExpRecord *expRecord;

    expRecord = ov02GetExpRecord(recordNumber);
    expPoints = expRecord->expPoints + pointsDelta;
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
    if (!(classFlags & 1))
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
