/*
 * OV02 original TU 3: 0x00a81b48..0x00a837c0 (26 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A81B48);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A81CD0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A81E40);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A81FC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A821D8);

typedef int s32;
typedef short s16;

/* Link record of a unit: its handler kind byte (+8) selects the handler. */
typedef struct BattleUnitLink {
    unsigned char unk_00[8];
    signed char battleHandlerKind;
} BattleUnitLink;

/* Per-unit stats block at BattleUnit +0x2B0. */
typedef struct BattleUnitStats {
    s32 unk_00;
    unsigned short unk_04;
    unsigned char unk_06[2];
    s16 bonusForKindZero;
    unsigned char unk_0A[2];
    s16 bonusForOtherKinds;
    unsigned char unk_0E[5];
    signed char unk_13;
} BattleUnitStats;

typedef struct BattleUnit {
    unsigned char unk_00[8];
    BattleUnitLink *battleHandlerLink;
    s32 battleStatusFlags;
    unsigned char unk_10[0x2A0];
    BattleUnitStats *battleStats;
} BattleUnit;

/* Battle entry slot; the unit stats are copied into battleStatValue and battleStatShortValue. */
typedef struct BattleEntry {
    unsigned char unk_00[0x38];
    s32 battleStatValue;
    s16 battleStatShortValue;
} BattleEntry;

void func_00A81FC8(BattleUnit *battleUnit, s32 value);
void func_00A821D8(BattleUnit *battleUnit, s32 value);

void battleUnitSetValueByKind(BattleUnit *battleUnit, s32 value) {
    if (battleUnit->battleHandlerLink->battleHandlerKind == 2) {
        func_00A821D8(battleUnit, value);
        return;
    }
    func_00A81FC8(battleUnit, value);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A82260);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A82308);

signed char battleUnitGetStatsByte13(BattleUnit *battleUnit) {
    return battleUnit->battleStats->unk_13;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A823C0);

s32 battleUnitPairFlagTest(BattleUnit **firstBattleUnit, BattleUnit **secondBattleUnit)
{
    s32 pairFlagsMatch;

    pairFlagsMatch = 1;
    if (!((*firstBattleUnit)->battleStatusFlags & 0x80))
    {
        pairFlagsMatch = 0x400;
        pairFlagsMatch = ((*secondBattleUnit)->battleStatusFlags & pairFlagsMatch) != 0;
    }
    return pairFlagsMatch;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A82668);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A82800);

void func_00A82668(BattleUnit *battleUnit, s32 value);
void func_00A82800(BattleUnit *battleUnit, s32 value);

void battleUnitSetValueByKindSecondary(BattleUnit *battleUnit, s32 value) {
    if (battleUnit->battleHandlerLink->battleHandlerKind == 2) {
        func_00A82800(battleUnit, value);
        return;
    }
    func_00A82668(battleUnit, value);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A828D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A829B0);

s32 battleUnitStatBonus(BattleUnit *bonusSourceUnit, BattleUnit **bonusTargetHandle) {
    s32 bonusBase;
    BattleUnit *bonusTarget;

    bonusTarget = *bonusTargetHandle;
    if (bonusSourceUnit->battleHandlerLink->battleHandlerKind == 0) {
        bonusBase = bonusTarget->battleStats->bonusForKindZero;
    } else {
        bonusBase = bonusTarget->battleStats->bonusForOtherKinds;
    }
    return bonusBase * 3;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A82A98);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A831C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A83258);

s32 flagsFirstSetBit(s32 bitField)
{
  s32 bitIndex;
  s32 bitMask;
  unsigned int lowBit;
  bitMask = 1;
  bitIndex = 0;
  lowBit = 1;
  if (!(bitField & lowBit))
  {
    for (;;)
    {
      bitIndex += 1;
      bitMask *= 2;
      if (bitIndex < 5)
      {
        if (!(bitField & bitMask))
        {
          continue;
        }
      }
      break;
    }

  }
  return bitIndex;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A83310);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A83418);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A834A0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu003", func_00A83618);

BattleUnitStats *unitGetValueById(s32 slot);
BattleEntry *battleEntrySlot(s32 key);

void battleEntriesCopyStats(s32 reserved0, s32 reserved1) {
    s32 copiedStatValue;
    s32 hasMoreSlots;
    s32 battleEntryKey;
    BattleEntry *battleEntry;
    BattleUnitStats *sourceBattleStats;

    battleEntryKey = 1;
    do {
        battleEntry = battleEntrySlot(battleEntryKey);
        sourceBattleStats = unitGetValueById(battleEntryKey);
        battleEntryKey += 1;
        copiedStatValue = sourceBattleStats->unk_00;
        hasMoreSlots = battleEntryKey < 0x12;
        battleEntry->battleStatShortValue = sourceBattleStats->unk_04;
        battleEntry->battleStatValue = copiedStatValue;
    } while (hasMoreSlots != 0);
}
