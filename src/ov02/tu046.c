/*
 * OV02 original TU 46: 0x00a92030..0x00a92900 (10 functions)
 */
#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;

/* Action record read by the unitStep functions: the action kind and value at +4 and +8, the actor id at +0xC. */
typedef struct UnitActionRecord {
    u8 unk_00[4];
    s32 actionKind;                     /* +0x04 */
    s32 actionValue;                    /* +0x08 */
    s32 actorId;                        /* +0x0C */
} UnitActionRecord;

/* Placement record read by unitPlaceAt: placed unit slot and two position floats. */
typedef struct UnitPlaceRecord {
    u8 unk_00[4];
    s32 placedUnitSlot;                 /* +0x04 */
    u8 unk_08[4];
    f32 positionX;                      /* +0x0C */
    f32 positionY;                      /* +0x10 */
} UnitPlaceRecord;

s32 actionDispatchDefault(s32 kind, s32 value);

s32 unitStepSendPairA(s32 unused_arg0, UnitActionRecord *actionRecord) {
    actionDispatchDefault(actionRecord->actionKind, actionRecord->actionValue);
    return 1;
}

s32 battleClearEntryFlags(void);
s32 func_00A8F490(s32 unit, s32 unitIndex);
s32 soundSlotSystemInit(void);
s32 func_00A99230(s32 unitIndex);
s32 unitSetPositionPercent(s32 unit, s32 firstPercent, s32 secondPercent);
s32 unitSetTargetPercent(s32 unit, s32 firstPercent, s32 secondPercent);
s32 unitSetFacing(s32 unit, f32 angle);
s32 unitSetDrawFlagTree(s32 unit);
s32 unitRefreshPhase(s32 unit);

s32 unitPlaceAt(s32 unused_arg0, UnitPlaceRecord *placeRecord)
{
    f32 xScaled;
    f32 yScaled;
    s32 placedUnitIndex;
    s32 battleUnit;
    s32 yInt;
    s32 xInt;

    placedUnitIndex = placeRecord->placedUnitSlot;
    xScaled = placeRecord->positionX * 100.0f;
    yScaled = placeRecord->positionY * 100.0f;
    battleUnit = func_00A99230(placedUnitIndex);
    yInt = yScaled;
    xInt = xScaled;
    unitSetTargetPercent(battleUnit, xInt, yInt);
    unitSetPositionPercent(battleUnit, xInt, yInt);
    unitSetFacing(battleUnit, 3.1415927f);
    soundSlotSystemInit();
    if (func_00A8F490(battleUnit, placedUnitIndex) != 0)
    {
        unitRefreshPhase(battleUnit);
        unitSetDrawFlagTree(battleUnit);
        battleClearEntryFlags();
    }
    do { return 1; } while (0);
}

s32 func_001B60B8(s32 slot);
s32 soundSlotPrimaryActive(void);

s32 unitStepCanAdvance(void) {
    if (soundSlotPrimaryActive() != 0) {
        return 0;
    }
    return func_001B60B8(3) == 0;
}

s32 func_00A8F7D8(s32 kind, s32 value);

s32 unitStepSendPairB(s32 unused_arg0, UnitActionRecord *actionRecord) {
    func_00A8F7D8(actionRecord->actionKind, actionRecord->actionValue);
    return 1;
}

s32 func_00A8F8D0(s32 id, s32 kind, s32 value);
s32 monsAttachActor(s32 id, s32 value);

s32 unitActionApply(s32 unused_arg0, UnitActionRecord *actionRecord)
{
    s32 actorId;
    s32 actionValue;
    s32 actionKind;

    actorId = actionRecord->actorId;
    actionValue = actionRecord->actionValue;
    actionKind = actionRecord->actionKind;
    if (actionKind == 5)
    {
        monsAttachActor(actorId, actionValue);
    }
    func_00A8F8D0(actorId, actionKind, actionValue);
    return 1;
}

s32 func_00A92FC8(void);

s32 unitStepHasPending(void) {
    return func_00A92FC8() != 0;
}

s32 battleUnitRefreshActionCode(s32 unit);
s32 func_00A99638(s32 unit);
s32 unitTblGet(s32 side, s32 **table);

s32 unitActionInitFromTable(s32 unused_arg0, UnitActionRecord *actionRecord) {
    s32 *battleUnitTable;
    s32 battleUnit;

    unitTblGet(2, &battleUnitTable);
    battleUnit = battleUnitTable[actionRecord->actionValue];
    battleUnitRefreshActionCode(battleUnit);
    func_00A99638(battleUnit);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu046", func_00A92278);

INCLUDE_ASM("asm/nonmatchings/ov02/tu046", func_00A92318);

INCLUDE_ASM("asm/nonmatchings/ov02/tu046", func_00A924D8);
