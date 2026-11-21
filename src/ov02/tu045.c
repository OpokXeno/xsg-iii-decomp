/*
 * OV02 original TU 45: 0x00a914d0..0x00a92030 (13 functions)
 */
#include "common.h"

struct BattleUnit;

typedef struct BattleUnit BattleUnit;

int unitTblGet(int side, BattleUnit ***table);
int calcStatGet(BattleUnit *unit, int row, int flag);
int unitCommandQueueDone(BattleUnit *unit);
void battleUnitsApplyStatDelta(void);

typedef signed char s8;
typedef int s32;

/* Record whose +0x04 word is the unit the queued request is for. */
typedef struct BattleTarget {
    unsigned char pad_00[0x4];
    BattleUnit *queuedUnit;                   /* +0x04 */
} BattleTarget;

/* Two words of an action record: the id at +0x00 and the argument at +0x08 of the record. */
typedef struct BattleActionArgs {
    s32 actionId;                       /* +0x00 */
    unsigned char pad_04[0x4];
    s32 actionArg;                      /* +0x08 */
} BattleActionArgs;

/* Record whose +0x10 holds the BattleActionArgs passed to unitSoundStartRecord. */
typedef struct BattleActionBlock {
    unsigned char pad_00[0x10];
    BattleActionArgs actionArgs;              /* +0x10 */
} BattleActionBlock;

void battleTargetUnitStep(s32 unused_arg0, BattleTarget *queuedTarget) {
    unitCommandQueueDone(queuedTarget->queuedUnit);
}

s32 func_00A85398(void *);
s32 unitSoundStartRecord(s32, s32);
s32 soundToggleReset();

void battleActionBlockStart(BattleActionBlock *actionBlock) {
    BattleActionArgs *actionArgs;

    actionArgs = &actionBlock->actionArgs;
    func_00A85398(actionBlock);
    soundToggleReset();
    unitSoundStartRecord(actionArgs->actionId, actionArgs->actionArg);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91520);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91630);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91928);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91A10);

s32 func_00A91A10();

void battleStateEnterStep2(s32 *battleStateStep) {
    *battleStateStep = 2;
    func_00A91A10();
}

void battleStateEnterStep3(s32 *battleStateStep) {
    *battleStateStep = 3;
    func_00A91A10();
}

s32 battleSetPendingFlag(s32);
s32 unitCountFlagged(s32);

s32 battleStatusQueueTryClose(void) {
    if (unitCountFlagged(2) == 0) {
        battleSetPendingFlag(0);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91C18);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91D08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91E78);

int battleStatusQueueWait(void)
{
    BattleUnit **units;
    BattleUnit *battleUnit;
    int sideUnitCount;
    int i;

    sideUnitCount = unitTblGet(2, &units);
    for (i = 0; i < sideUnitCount; i++) {
        battleUnit = units[i];
        if (battleUnit != 0) {
            if (calcStatGet(battleUnit, 1, 1) != 0 || calcStatGet(battleUnit, 1, 2) != 0) {
                if (unitCommandQueueDone(battleUnit) == 0) {
                    return 0;
                }
            }
        }
    }
    battleUnitsApplyStatDelta();
    return 1;
}
