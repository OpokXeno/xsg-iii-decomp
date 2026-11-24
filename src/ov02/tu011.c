/*
 * OV02 original TU 11: 0x00a852d8..0x00a859a0 (12 functions)
 */
#include "common.h"

typedef int s32;
typedef signed char s8;
typedef short s16;

struct BattleStats {
    unsigned char unmodeled_00[0x1C];
    s16 actionKind;                       /* +0x1C: the action kind passed back to func_00A854D8 */
};

struct BattleUnit {
    unsigned char unmodeled_00[0x2B0];
    struct BattleStats *battleStats;          /* +0x2B0 */
    struct XenoActor *actor;            /* +0x2B4: the motionActor's motion actor */
};

typedef struct BattleUnit BattleUnit;
struct XenoActor;

void battleNoOpCallback(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A852E0);

s32 battleAttackCodeFromFlags(s32 attackFlags) {
    s32 attackCode;

    attackCode = 0xF;
    if (!(attackFlags & 2)) {
        attackCode = 0x10;
        if (!(attackFlags & 4)) {
            attackCode = 0x11;
            if (!(attackFlags & 8)) {
                attackCode = !(attackFlags & 0x10) ? 0 : 0x12;
            }
        }
    }
    return attackCode;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A85398);

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A854D8);

s32 sefReleaseGroupMatches(struct XenoActor *, s32);

void battleUnitSendSlotValue(BattleUnit *battleUnit, s32 category, s32 event) {
    sefReleaseGroupMatches(battleUnit->actor, (category << 0xC) | event);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A85570);

extern s32 D_00AB7950[];

s32 battleWordTableGet(s32 battleWordIndex) {
    return D_00AB7950[battleWordIndex];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A855C8);

s32 battleAttackCodeFromFlags();
s32 func_00A854D8();
s32 func_00A92900();

void battleUnitRefreshActionCode(BattleUnit *battleUnit) {
    s32 actionFlags;
    s32 actionCode;

    func_00A854D8(battleUnit, battleUnit->battleStats->actionKind, 8);
    if (func_00A92900(battleUnit, 3, 1, &actionFlags) != 0) {
        actionCode = battleAttackCodeFromFlags(actionFlags);
        if (actionCode != 0) {
            func_00A854D8(battleUnit, battleUnit->battleStats->actionKind, actionCode);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A857F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu011", func_00A85898);
