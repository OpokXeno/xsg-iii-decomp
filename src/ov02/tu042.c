/*
 * OV02 original TU 42: 0x00a8ed30..0x00a8ee98 (8 functions)
 */
#include "common.h"

typedef short s16;
typedef int s32;
typedef unsigned int u32;

/* The info record behind a unit: only the target id at +0x72 is used here. */
typedef struct BattleStats {
    unsigned char unmodeled_00[0x72];
    s16 targetId;                       /* +0x72 */
} BattleStats;

/* The unit record that the unit table points at. */
typedef struct BattleUnit {
    unsigned char unmodeled_00[0x0C];
    u32 battleUnitFlags;                /* +0x0C: bit 0x4000 is tested by unitTableUpdateFlagged */
    unsigned char unmodeled_10[0x2B0 - 0x10];
    BattleStats *battleStats;                   /* +0x2B0 */
} BattleUnit;

s32 textSubmitEntry0408(s16 targetId);

void unitTargetApplyPrimary(BattleUnit *targetBattleUnit) {
    textSubmitEntry0408(targetBattleUnit->battleStats->targetId);
}

s32 textSubmitTargetSecondary(s16 targetId);

void unitTargetApplySecondary(BattleUnit *targetBattleUnit) {
    textSubmitTargetSecondary(targetBattleUnit->battleStats->targetId);
}

s32 textSubmitEntry1100(s16 targetId);

void unitTargetApplyLatched(BattleUnit *targetBattleUnit) {
    textSubmitEntry1100(targetBattleUnit->battleStats->targetId);
}

s32 textSubmitTargetLatchedSecond(s16 targetId);

void unitTargetApplyLatchedSecond(BattleUnit *targetBattleUnit) {
    textSubmitTargetLatchedSecond(targetBattleUnit->battleStats->targetId);
}

s32 textSubmitTargetIfSet(s16 targetId);

void unitTargetApplyIfSet(BattleUnit *targetBattleUnit) {
    if (targetBattleUnit != 0) {
        textSubmitTargetIfSet(targetBattleUnit->battleStats->targetId);
    }
}

s32 textSubmitNotAllFlagged(void);
s32 textSubmitAllFlagged(void);
s32 unitTblGet();

void unitTableUpdateFlagged(void) {
    BattleUnit **units;
    s32 battleUnitCount;
    s32 allFlagged;
    BattleUnit **cursor;
    s32 remaining;
    BattleUnit *battleUnit;

    battleUnitCount = unitTblGet(0, &units);
    allFlagged = 1;
    if (battleUnitCount > 0) {
        cursor = units;
        remaining = battleUnitCount;
        do {
            battleUnit = *cursor;
            cursor += 1;
            if (battleUnit != 0) {
                allFlagged = !(battleUnit->battleUnitFlags & 0x4000) ? 0 : allFlagged;
            }
            remaining -= 1;
        } while (remaining != 0);
    }
    if (allFlagged != 0) {
        textSubmitAllFlagged();
        return;
    }
    textSubmitNotAllFlagged();
}

s32 textFinishScene(void);

void unitTargetFinishScene(void) {
    textFinishScene();
}

s32 textSubmitTargetTertiary(s16 targetId);

void unitTargetApplyTertiary(BattleUnit *targetBattleUnit) {
    textSubmitTargetTertiary(targetBattleUnit->battleStats->targetId);
}
