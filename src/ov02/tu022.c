/*
 * OV02 original TU 22: 0x00a8a110..0x00a8abb0 (19 functions)
 */
#include "common.h"
#include "ov02/tu060.h"

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;
extern s8 D_00ABF8C8[];
s32 func_002AC3B0();
s32 func_002AC498();
s32 sceneStateDispatch();
s32 entryCostGet();
s32 unitSlotUsable();
s32 ov02ItemEntryGet();
s32 func_00A84A90();
s32 func_00A878E0();
s32 * func_00A88F60();

/* One 0x10-byte record of the slot tables built by buildActiveSlotTable and buildSmallSlotTable. */
typedef struct BattleSlotRecord {
    s32 primaryHandle;             /* 0x0 */
    s32 secondaryHandle;   /* 0x4 */
    s32 slotCost;              /* 0x8 */
    s32 isUsable;              /* 0xC */
} BattleSlotRecord;

extern BattleSlotRecord battleSlotRecords[];

void buildActiveSlotTable(BattleUnit *battleUnit)
{
  s32 battleSlotNumber;
  s32 previousSlot;
  s32 activeSlotCount;
  s32 primaryHandle;
  s32 secondaryHandle;
  s32 slotCost;
  s32 isUsable;
  s32 battleSlotItem;
  BattleSlotRecord *battleSlotRecord;
  s32 *sceneStateTable;
  activeSlotCount = 0;
  previousSlot = 0;
  battleSlotRecord = battleSlotRecords;
  do
  {
    battleSlotNumber = previousSlot + 1;
    if (func_00A878E0(battleUnit->lookupKeyRecord->lookupKey, battleSlotNumber) != 0)
    {
      battleSlotItem = ov02ItemEntryGet(battleSlotNumber);
      slotCost = entryCostGet(battleUnit, battleSlotItem);
      primaryHandle = func_00A84A90(battleSlotNumber, 0);
      battleSlotRecord->primaryHandle = primaryHandle;
      secondaryHandle = func_00A84A90(battleSlotNumber, 1);
      battleSlotRecord->slotCost = slotCost;
      battleSlotRecord->secondaryHandle = secondaryHandle;
      isUsable = unitSlotUsable(battleUnit, battleSlotNumber);
      D_00ABF8C8[activeSlotCount] = battleSlotNumber;
      activeSlotCount += 1;
      battleSlotRecord->isUsable = isUsable;
      battleSlotRecord += 1;
    }
    previousSlot = battleSlotNumber;
  }
  while (previousSlot < 0x58);
  func_002AC498(battleSlotRecords, activeSlotCount);
  func_002AC3B0(2);
  sceneStateTable = func_00A88F60(battleUnit, 1);
  sceneStateDispatch(*sceneStateTable);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A228);

s32 stateFlag0002ClearAndReset();

void battleFlagClearA(void) {
    stateFlag0002ClearAndReset();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A2B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A3C8);

void battleFlagClearB(void) {
    stateFlag0002ClearAndReset();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A450);

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A570);

void battleFlagClearC(void) {
    stateFlag0002ClearAndReset();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A5F8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A778);

void battleFlagClearD(void) {
    stateFlag0002ClearAndReset();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8A800);

extern s32 battleSlotValues[];
s32 func_00A89128();

s32 activeSlotGetValue(void) {
    s32 battleSlotIndex;

    battleSlotIndex = func_00A89128();
    return battleSlotValues[battleSlotIndex];
}

void battleFlagClearE(void) {
    stateFlag0002ClearAndReset();
}

u8 *roboKeyTableGet();
s32 func_00A84A10();

void buildSmallSlotTable(BattleUnit *battleUnit)
{
  s16 slot_id;
  s32 primaryHandle;
  s32 battleSlotIndex;
  s32 smallSlotCount;
  s8 *slot_ids_entry;
  s16 *slotIdSource;
  BattleSlotRecord *battleSlotRecord;
  battleSlotRecord = battleSlotRecords;
  battleSlotIndex = 0;
  smallSlotCount = 0;
  do
  {
    /* element address kept in a pointer local: base first, as in the original */
    slotIdSource = (s16 *) roboKeyTableGet(battleUnit->lookupKeyRecord->lookupKey) + battleSlotIndex;
    slot_id = *slotIdSource;
    if (slot_id != 0)
    {
      primaryHandle = func_00A84A10(slot_id, 0);
      battleSlotRecord->secondaryHandle = 0;
      battleSlotRecord->primaryHandle = primaryHandle;
      slot_ids_entry = &D_00ABF8C8[smallSlotCount];
      battleSlotRecord->slotCost = 0;
      smallSlotCount += 1;
      battleSlotRecord->isUsable = 1;
      battleSlotRecord += 1;
      *slot_ids_entry = battleSlotIndex;
    }
    battleSlotIndex += 1;
  }
  while (battleSlotIndex < 7);
  func_002AC498(battleSlotRecords, smallSlotCount);
  func_002AC3B0(4);
}

extern BattleSlotRecord D_00ABD4D4[];

s32 activeSlotGetNumber(void) {
    s32 battleSlotIndex;

    battleSlotIndex = func_00A89128();
    if (D_00ABD4D4[battleSlotIndex].primaryHandle == 0) {
        return 0;
    }
    return D_00ABF8C8[battleSlotIndex] + 1;
}

void battleFlagClearF(void) {
    stateFlag0002ClearAndReset();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu022", func_00A8AA58);
