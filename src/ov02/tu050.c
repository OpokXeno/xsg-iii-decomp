/*
 * OV02 original TU 50: 0x00a93ea0..0x00a947e8 (14 functions)
 */
#include "common.h"

struct BattleUnit;

#include "ov02/tu060.h"

struct UnitBitTable {
    int storedStatDelta;                /* +0x00 */
    unsigned char unmodeled_004[0x8C];
    signed char byBit[2][32];           /* +0x90 */
};


int unitTblGet(int side, BattleUnit ***table);
int statFlagBitIndex(unsigned int mask);
int calcStatGet(BattleUnit *battleUnit, int row, int statMask);
void func_00A940E8(BattleUnit *battleUnit, int row, int statMask);
void func_00A81868(BattleUnit *battleUnit, int delta);
void func_00A91D08(BattleUnit *battleUnit, int delta, int kind);

typedef int s32;

s32 statCodeFromIndex(s32 statNumber) {
    return statNumber + 0x7A;
}

s32 statFlagBitIndex(u32 mask)
{
  u32 flagMask;
  s32 bitIndex;
  short bitStep;
  s32 bitMask;
  bitMask = 1;
  bitIndex = 0;
  bitStep = 1;
  if (!(mask & bitStep))
  {
    for (;;)
    {
      bitIndex += bitStep;
      flagMask = mask;
      bitMask *= 2;
      if (bitIndex < 0x20)
      {
        if (!(flagMask & bitMask))
        {
          continue;
        }
      }
      break;
    }

  }
  return bitIndex;
}

s32 statFlagFromBitIndex(s32 bitIndex) {
    return 1 << bitIndex;
}

int calcStatGet(BattleUnit *battleUnit, int row, int statMask)
{
    int bitIndex;

    if (row >= 2) {
        return 0;
    }
    bitIndex = statFlagBitIndex(statMask);
    return battleUnit->bitTable->byBit[row][bitIndex];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93F58);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93FE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A940E8);

void calcStatClearBit(BattleUnit *battleUnit, s32 row, s32 unused_arg2, s32 bitIndex) {
    battleUnit->bitTable->byBit[row][bitIndex] = 0;
}

s32 calcStatRowMask(BattleUnit *battleUnit, s32 row) {
    s32 flagBit;
    s32 bitIndex;
    s32 mask;

    bitIndex = 0;
    mask = 0;
    do {
        flagBit = 1 << bitIndex;
        bitIndex += 1;
        if (calcStatGet(battleUnit, row, flagBit) != 0) {
            mask = mask | flagBit;
        }
    } while (bitIndex < 0x20);
    return mask;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A94218);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A942F0);

void battleUnitApplyFlagDamage(BattleUnit *battleUnit) {
    s32 damage;

    if (calcStatGet(battleUnit, 0, 0x4000) == 1) {
        damage = *(s32 *) battleUnit->lookupKeyRecord * 0x28;
        func_00A81868(battleUnit, -(damage / 100));
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A943F8);

void battleUnitsApplyStatDelta(void)
{
    BattleUnit **battleUnitPtrs;
    BattleUnit *affectedUnit;
    int battleUnitTotal;
    int i;
    int statDelta;

    battleUnitTotal = unitTblGet(2, &battleUnitPtrs);
    for (i = 0; i < battleUnitTotal; i++) {
        statDelta = 0;
        affectedUnit = battleUnitPtrs[i];
        if (affectedUnit == 0) {
            continue;
        }
        if (affectedUnit->targetingFlags & 0x8000) {
            if (calcStatGet(affectedUnit, 1, 2) != 0) {
                statDelta = -affectedUnit->bitTable->storedStatDelta;
                func_00A940E8(affectedUnit, 1, 2);
            } else if (calcStatGet(affectedUnit, 1, 1) != 0) {
                statDelta = affectedUnit->bitTable->storedStatDelta / 2;
                statDelta = -statDelta;
                func_00A940E8(affectedUnit, 1, 1);
            }
        }
        if (statDelta != 0) {
            affectedUnit->targetingFlags = (affectedUnit->targetingFlags & 0xffff7fff) | 1;
            func_00A81868(affectedUnit, -statDelta);
            func_00A91D08(affectedUnit, statDelta, 3);
        }
    }
}
