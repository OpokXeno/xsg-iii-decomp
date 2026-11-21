/*
 * OV02 original TU 55: 0x00a964f0..0x00a96d38 (13 functions)
 */
#include "common.h"

typedef int s32;
typedef short s16;
typedef unsigned char u8;
typedef unsigned int u32;

typedef struct BattleUnitLookup BattleUnitLookup;
typedef struct BattleUnit BattleUnit;
typedef struct BattleTargetResult BattleTargetResult;

/* The battle unit record that the target tables point at. */
struct BattleUnit {
    u8 unk_00[0x0C];
    u32 targetingFlags;                          /* +0x0C: 0x8000 excludes the motionActor from targeting */
    u8 unk_10[0x2B0 - 0x10];
    BattleUnitLookup *lookupKeyRecord;                   /* +0x2B0 */
};

/* The lookup key record that BattleUnit.lookupKeyRecord points at. */
struct BattleUnitLookup {
    u8 unk_00[0x38];
    s32 unk_38;                         /* +0x38: compared by the 0x38 pickers */
    u8 unk_3C[0x3E - 0x3C];
    s16 lookupKey;                      /* +0x3E: the unit filter is lookupKey < 0x12 */
    u8 unk_40[0x72 - 0x40];
    s16 targetId;                       /* +0x72: the id written to the result */
    u8 unk_74[0x80 - 0x74];
    s16 unk_80;                         /* +0x80: compared by the 0x80 pickers */
};

/* The choice record: the chosen target id goes to +0x0C. */
struct BattleTargetResult {
    u8 unk_00[0x0C];
    s32 targetId;                       /* +0x0C */
};

s32 rnd(s32 max);
void battleTargetPickRandom(BattleUnit *attacker, BattleTargetResult *targetChoice);
void battleTargetPickHighest38(BattleUnit *attacker, BattleTargetResult *targetChoice);

/* 30% a random target, otherwise the highest 0x38 (battleTargetPickHighest38). */
void battleTargetPickRandomOrHighest38(BattleUnit *attacker, BattleTargetResult *targetChoice) {
    if (rnd(0x64) < 0x1E) {
        battleTargetPickRandom(attacker, targetChoice);
        return;
    }
    battleTargetPickHighest38(attacker, targetChoice);
}

void battleTargetPickHighest80(BattleUnit *attacker, BattleTargetResult *targetChoice);

/* Rolled: under 20 the highest 0x38, under 50 a random target, otherwise the highest 0x80 (battleTargetPickHighest80). */
void battleTargetPickRolledHighest(BattleUnit *attacker, BattleTargetResult *targetChoice) {
    s32 roll;

    roll = rnd(0x64);
    if (roll < 0x14) {
        battleTargetPickHighest38(attacker, targetChoice);
        return;
    }
    if (roll < 0x32) {
        battleTargetPickRandom(attacker, targetChoice);
        return;
    }
    battleTargetPickHighest80(attacker, targetChoice);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu055", func_00A965D0);

void battleTargetPickLowest38(BattleUnit *attacker, BattleTargetResult *targetChoice);
s32 func_00A96A08(BattleUnit *attacker, BattleTargetResult *targetChoice);

/* Rolled: under 20 the highest 0x80 (battleTargetPickHighest80), under 50 func_00A96A08, otherwise the lowest 0x38. */
void battleTargetPickRolledHighestOrLowest(BattleUnit *attacker, BattleTargetResult *targetChoice) {
    s32 roll;

    roll = rnd(0x64);
    if (roll < 0x14) {
        battleTargetPickHighest80(attacker, targetChoice);
        return;
    }
    if (roll < 0x32) {
        func_00A96A08(attacker, targetChoice);
        return;
    }
    battleTargetPickLowest38(attacker, targetChoice);
}

s32 unitTblGet(s32 side, BattleUnit ***table);

/* A random target among the unit table entries of the attacker's filter, skipping empty and excluded units. */
void battleTargetPickRandom(BattleUnit *attacker, BattleTargetResult *targetChoice) {
    BattleUnit **units;
    s32 candidateCount;
    BattleUnit *candidateUnit;
    s32 targetId;

    candidateCount = unitTblGet(attacker->lookupKeyRecord->lookupKey < 0x12, &units) - 1;
    do {
        candidateUnit = units[rnd(candidateCount)];
    } while ((candidateUnit == 0) || (candidateUnit->targetingFlags & 0x8000));
    targetId = candidateUnit->lookupKeyRecord->targetId;
    targetChoice->targetId = targetId;
}

/* The highest unk_80 of the filtered units, skipping empty and excluded ones. */
void battleTargetPickHighest80(BattleUnit *attacker, BattleTargetResult *targetChoice)
{
  BattleUnit **units;
  s32 targetId;
  s32 candidateCount;
  BattleUnit **cursor;
  s32 remaining;
  BattleUnit *candidateUnit;
  BattleUnit *best;
  best = 0;
  candidateCount = unitTblGet((attacker->lookupKeyRecord->lookupKey) < 0x12, &units);
  if (candidateCount > 0)
  {
    cursor = units;
    remaining = candidateCount;
    do
    {
      candidateUnit = *cursor;
      cursor += 1;
      if ((candidateUnit != 0) && !(candidateUnit->targetingFlags & 0x8000))
      {
        if (best == 0)
        {
          best = candidateUnit;
        }
        else
        {
          if ((best->lookupKeyRecord->unk_80) < (candidateUnit->lookupKeyRecord->unk_80))
          {
            best = candidateUnit;
          }
        }
      }
      remaining -= 1;
    }
    while (remaining != 0);
  }
  targetId = best->lookupKeyRecord->targetId;
  targetChoice->targetId = targetId;
}

/* The highest 0x38 of the filtered units, skipping empty and excluded ones. */
void battleTargetPickHighest38(BattleUnit *attacker, BattleTargetResult *targetChoice)
{
  BattleUnit **units;
  s32 targetId;
  s32 candidateCount;
  BattleUnit **cursor;
  s32 remaining;
  BattleUnit *candidateUnit;
  BattleUnit *best;
  best = 0;
  candidateCount = unitTblGet((attacker->lookupKeyRecord->lookupKey) < 0x12, &units);
  if (candidateCount > 0)
  {
    cursor = units;
    remaining = candidateCount;
    do
    {
      candidateUnit = *cursor;
      cursor += 1;
      if ((candidateUnit != 0) && !(candidateUnit->targetingFlags & 0x8000))
      {
        if (best == 0)
        {
          best = candidateUnit;
        }
        else
        {
          if ((best->lookupKeyRecord->unk_38) < (candidateUnit->lookupKeyRecord->unk_38))
          {
            best = candidateUnit;
          }
        }
      }
      remaining -= 1;
    }
    while (remaining != 0);
  }
  targetId = best->lookupKeyRecord->targetId;
  targetChoice->targetId = targetId;
}

/* The lowest 0x38 of the filtered units, skipping empty and excluded ones. */
void battleTargetPickLowest38(BattleUnit *attacker, BattleTargetResult *targetChoice)
{
  BattleUnit **units;
  s32 targetId;
  s32 candidateCount;
  BattleUnit **cursor;
  s32 remaining;
  BattleUnit *candidateUnit;
  BattleUnit *best;
  best = 0;
  candidateCount = unitTblGet((attacker->lookupKeyRecord->lookupKey) < 0x12, &units);
  if (candidateCount > 0)
  {
    cursor = units;
    remaining = candidateCount;
    do
    {
      candidateUnit = *cursor;
      cursor += 1;
      if ((candidateUnit != 0) && !(candidateUnit->targetingFlags & 0x8000))
      {
        if (best == 0)
        {
          best = candidateUnit;
        }
        else
        {
          if ((candidateUnit->lookupKeyRecord->unk_38) < (best->lookupKeyRecord->unk_38))
          {
            best = candidateUnit;
          }
        }
      }
      remaining -= 1;
    }
    while (remaining != 0);
  }
  targetId = best->lookupKeyRecord->targetId;
  targetChoice->targetId = targetId;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu055", func_00A96978);

INCLUDE_ASM("asm/nonmatchings/ov02/tu055", func_00A96A08);

s32 func_00A819D8(BattleUnit *candidateUnit);

/* The last filtered unit that func_00A819D8 accepts; the target id, or -1 when none. */
void battleTargetPickLastPassing(BattleUnit *attacker, BattleTargetResult *targetChoice)
{
  BattleUnit **units;
  s32 targetId;
  s32 candidateCount;
  s32 candidateIndex;
  BattleUnit *candidateUnit;
  BattleUnit *chosen;
  candidateIndex = 0;
  chosen = 0;
  candidateCount = unitTblGet((attacker->lookupKeyRecord->lookupKey) < 0x12, &units);
  if (candidateCount > 0)
  {
    do
    {
      candidateUnit = units[candidateIndex];
      if (((candidateUnit != 0) && (!(candidateUnit->targetingFlags & 0x8000))) && (func_00A819D8(candidateUnit) != 0))
      {
        chosen = units[candidateIndex];
      }
      candidateIndex++;
    }
    while (candidateIndex < candidateCount);
  }
  if (chosen != 0)
  {
    targetId = chosen->lookupKeyRecord->targetId;
  }
  else
  {
    targetId = -1;
  }
  targetChoice->targetId = targetId;
}

s32 func_00A818D0(BattleUnit *candidateUnit);

/* The last filtered unit that func_00A818D0 accepts; the target id, or -1 when none. */
void battleTargetPickLastPassingAlt(BattleUnit *attacker, BattleTargetResult *targetChoice)
{
  BattleUnit **units;
  s32 targetId;
  s32 candidateCount;
  s32 candidateIndex;
  BattleUnit *candidateUnit;
  BattleUnit *chosen;
  candidateIndex = 0;
  chosen = 0;
  candidateCount = unitTblGet((attacker->lookupKeyRecord->lookupKey) < 0x12, &units);
  if (candidateCount > 0)
  {
    do
    {
      candidateUnit = units[candidateIndex];
      if (((candidateUnit != 0) && (!(candidateUnit->targetingFlags & 0x8000))) && (func_00A818D0(candidateUnit) != 0))
      {
        chosen = units[candidateIndex];
      }
      candidateIndex++;
    }
    while (candidateIndex < candidateCount);
  }
  if (chosen != 0)
  {
    targetId = chosen->lookupKeyRecord->targetId;
  }
  else
  {
    targetId = -1;
  }
  targetChoice->targetId = targetId;
}

/* The lowest unk_80 of the filtered units, skipping empty and excluded ones. */
void battleTargetPickLowest80(BattleUnit *attacker, BattleTargetResult *targetChoice)
{
  BattleUnit **units;
  s32 targetId;
  s32 candidateCount;
  BattleUnit **cursor;
  s32 remaining;
  BattleUnit *candidateUnit;
  BattleUnit *best;
  best = 0;
  candidateCount = unitTblGet((attacker->lookupKeyRecord->lookupKey) < 0x12, &units);
  if (candidateCount > 0)
  {
    cursor = units;
    remaining = candidateCount;
    do
    {
      candidateUnit = *cursor;
      cursor += 1;
      if ((candidateUnit != 0) && !(candidateUnit->targetingFlags & 0x8000))
      {
        if (best == 0)
        {
          best = candidateUnit;
        }
        else
        {
          if ((candidateUnit->lookupKeyRecord->unk_80) < (best->lookupKeyRecord->unk_80))
          {
            best = candidateUnit;
          }
        }
      }
      remaining -= 1;
    }
    while (remaining != 0);
  }
  targetId = best->lookupKeyRecord->targetId;
  targetChoice->targetId = targetId;
}
