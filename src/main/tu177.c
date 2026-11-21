#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222598);

typedef unsigned char u8;
typedef int s32;
typedef short s16;
extern unsigned char gPagedEntryKindTable[];

u8 slotKindResolve(s32 pagedEntryId) {
    if (!(pagedEntryId < 0xB)) {
        return 7;
    }
    return gPagedEntryKindTable[pagedEntryId];
}

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222630);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222710);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222768);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222848);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_002228E8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222AC0);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222BF8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222C60);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222D18);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222D48);

s32 Get_Rnd_001B8CD0();

void rndSkipOne(void) {
    Get_Rnd_001B8CD0();
}

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222E30);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222EA0);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00222FB8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223228);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_002232B8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_002233A8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_002234F8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223778);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223830);

extern unsigned char gRecordSlotBitTable[];

s32 recordSlotFieldBitTest(s32 recordNumber, s32 slotNumber, s32 fieldDisplacement, s32 bitNumber)
{
  s32 recordStrideOffset;
  s32 selectedBits;
  u8 *packedSlotByte;
  recordStrideOffset = recordNumber * 0x24;
  packedSlotByte = &gRecordSlotBitTable[((slotNumber >> 1) * 8) + (fieldDisplacement + recordStrideOffset)];
  selectedBits = (*packedSlotByte) & (1 << (bitNumber + ((slotNumber & 1) * 4)));
  return selectedBits != 0;
}

void func_00223DC0(s32 record, s32 slot, s32 field, s32 bit);

void recordSlotFlagApply(s32 recordNumber, s32 slotNumber) {
    func_00223DC0(recordNumber, slotNumber, 7, 0);
}

s32 recordSlotFieldBitTest();

s32 recordSlotFlagTest(s32 recordNumber, s32 slotNumber) {
    return recordSlotFieldBitTest(recordNumber, slotNumber, 7, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223970);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_002239B0);

s32 slotKindFromMode(s32 unused, s32 mode)
{
  return (!(mode < 2)) ? (7) : (8);
}

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223B08);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223BC8);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223C58);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223CE0);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223D58);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223DC0);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223F70);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00223F98);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00224058);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00224118);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_002241A0);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00224310);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00224420);

INCLUDE_ASM("asm/main/nonmatchings/tu177", func_00224508);

s16 *func_00222768(s32, s32);

s32 recordActiveSlotCount(s32 recordNumber)
{
  s32 slotNumber;
  s32 activeSlotIndex;
  s32 activeSlotCount;
  activeSlotIndex = 0;
  activeSlotCount = 0;
  do
  {
    slotNumber = activeSlotIndex + 0xA;
    if ((*func_00222768(recordNumber, slotNumber)) != 0)
    {
      activeSlotCount = activeSlotCount + 1;
    }
    activeSlotIndex += 1;
  }
  while (activeSlotIndex < 0xA);
  return activeSlotCount;
}
