/*
 * OV02 original TU 31: 0x00a8c5e8..0x00a8cfb0 (21 functions)
 */
#include "common.h"
#include "ov02/tu060.h"

typedef int s32;

/* The stats record at +0x2B0 of a battle unit: its flag word is at +0x24. */
typedef struct BattleStats {
    unsigned char unmodeled_00[0x24];   /* +0x00 */
    s32 statBits;                       /* +0x24 */
} BattleStats;

s32 vmArgGet(s32 vm, s32 index);
BattleUnit *unitTblEntryGet(s32 index);
void func_00A95288(BattleUnit *unit);

s32 sbNativeUnitRefresh(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    func_00A95288(battleUnit);
    return 1;
}

void func_00A95300(BattleUnit *unit);

s32 sbNativeUnitReset(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    func_00A95300(battleUnit);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu031", func_00A8C648);

INCLUDE_ASM("asm/nonmatchings/ov02/tu031", func_00A8C720);

INCLUDE_ASM("asm/nonmatchings/ov02/tu031", func_00A8C850);

INCLUDE_ASM("asm/nonmatchings/ov02/tu031", func_00A8C9A8);

void func_00A95350(s32 firstArg, s32 secondArg);

void sbNativeArgPairCall(s32 vm) {
    s32 firstArg;
    s32 secondArg;

    firstArg = vmArgGet(vm, 1);
    secondArg = vmArgGet(vm, 2);
    func_00A95350(firstArg, secondArg);
}

s32 func_00A95470(s32 firstArg, s32 secondArg, s32 thirdArg);

s32 sbNativeArgTripleCall(s32 vm) {
    s32 firstArg;
    s32 secondArg;
    s32 thirdArg;

    firstArg = vmArgGet(vm, 1);
    secondArg = vmArgGet(vm, 2);
    thirdArg = vmArgGet(vm, 3);
    func_00A95470(firstArg, secondArg, thirdArg);
    return 1;
}

void func_00A95580(s32 firstArg, s32 secondArg);

void sbNativeArgPairApply(s32 vm) {
    s32 firstArg;
    s32 secondArg;

    firstArg = vmArgGet(vm, 1);
    secondArg = vmArgGet(vm, 2);
    func_00A95580(firstArg, secondArg);
}

void func_00A955D8(s32 firstArg, s32 secondArg);

void sbNativeArgPairApplyAlt(s32 vm) {
    s32 firstArg;
    s32 secondArg;

    firstArg = vmArgGet(vm, 1);
    secondArg = vmArgGet(vm, 2);
    func_00A955D8(firstArg, secondArg);
}

void boostGaugeSetField(s32 index, s32 offset, s32 value);

s32 sbNativeBoostGaugeByteSet(s32 vm) {
    s32 gaugeIndex;
    s32 gaugeFieldId;
    s32 gaugeByteValue;

    gaugeIndex = vmArgGet(vm, 1);
    gaugeFieldId = vmArgGet(vm, 2);
    gaugeByteValue = vmArgGet(vm, 3);
    boostGaugeSetField(gaugeIndex, gaugeFieldId, gaugeByteValue);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu031", func_00A8CC08);

s32 calcStatRowMask(BattleUnit *unit, s32 row);

void sbNativeUnitStatRowMask(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;
    s32 statRow;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    statRow = vmArgGet(vm, 2);
    calcStatRowMask(battleUnit, statRow);
}

void func_00A93F58(BattleUnit *unit, s32 value, s32 thirdArg);

s32 sbNativeUnitStatApply(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;
    s32 value;
    s32 thirdArg;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    value = vmArgGet(vm, 2);
    thirdArg = vmArgGet(vm, 3);
    func_00A93F58(battleUnit, value, thirdArg);
    return 1;
}

void func_00A940E8(BattleUnit *unit, s32 row, s32 flag);

s32 sbNativeUnitStatFlagSet(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;
    s32 statRow;
    s32 statRowFlag;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    statRow = vmArgGet(vm, 2);
    statRowFlag = vmArgGet(vm, 3);
    func_00A940E8(battleUnit, statRow, statRowFlag);
    return 1;
}

void actorApplyLevelMode(void *owner, s32 mode);
s32 func_00A8C2C8(BattleUnit *unit, s32 value);

s32 sbNativeUnitTryEffect(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;
    s32 levelMode;
    s32 effectApplied;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    levelMode = vmArgGet(vm, 2);
    effectApplied = func_00A8C2C8(battleUnit, levelMode);
    if (effectApplied != 0) {
        actorApplyLevelMode(battleUnit, levelMode);
    }
    return effectApplied;
}

void sbNativeUnitTryEffectQuiet(s32 vm) {
    s32 battleUnitIndex;
    BattleUnit *battleUnit;
    s32 levelMode;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    levelMode = vmArgGet(vm, 2);
    func_00A8C2C8(battleUnit, levelMode);
}

s32 sbNativeUnitFlagTest(s32 vm)
{
    s32 battleUnitIndex;

    battleUnitIndex = vmArgGet(vm, 1);
    if ((unitTblEntryGet(battleUnitIndex)->targetingFlags & 0x800) != 0)
    {
        return 1;
    }
    return 0;
}

s32 sbNativeUnitStatBitsUpdate(s32 vm)
{
    s32 battleUnitIndex;
    BattleUnit *battleUnit;
    s32 statBitMask;

    battleUnitIndex = vmArgGet(vm, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    statBitMask = vmArgGet(vm, 2);
    if (vmArgGet(vm, 3) == 0)
    {
        ((BattleStats *) battleUnit->stats)->statBits &= ~statBitMask;
    }
    else
    {
        ((BattleStats *) battleUnit->stats)->statBits |= statBitMask;
    }
    return 1;
}

void ov02AddRecordPoints(s32 unused_arg0, s32 points);

s32 sbNativeRecordPointsAdd(s32 vm) {
    s32 unusedArg;
    s32 points;

    unusedArg = vmArgGet(vm, 1);
    points = vmArgGet(vm, 2);
    ov02AddRecordPoints(unusedArg, points);
    return 1;
}

int rnd(int max);

void sbNativeRandom(s32 vm) {
    s32 randomLimit;

    randomLimit = vmArgGet(vm, 1);
    rnd(randomLimit);
}
