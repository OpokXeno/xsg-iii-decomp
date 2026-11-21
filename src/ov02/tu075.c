/*
 * OV02 original TU 75: 0x00aa6348..0x00aa6518 (4 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu075", func_00AA6348);

typedef int s32;
typedef short s16;
typedef unsigned short u16;
typedef unsigned char u8;

typedef struct BattleStats BattleStats;
typedef struct BattleUnit BattleUnit;
typedef struct BattleActor BattleActor;

/* The stats record of a battle unit (at BattleUnit +0x2B0). */
struct BattleStats {
    s32 currentCap;                         /* +0x00: the upper clamp of currentAmount (battleStatsAddClamped38) */
    s16 secondaryCap;                       /* +0x04: the upper clamp of secondaryAmount (battleSecondaryAddClamped) */
    u8 unk_06[0x38 - 0x06];
    s32 currentAmount;                      /* +0x38: the value battleStatsAddClamped38 adds to and clamps */
    s16 secondaryAmount;                    /* +0x3C: the value battleSecondaryAddClamped adds to and clamps */
};

/* The battle unit record that BattleActor.owner points at. */
struct BattleUnit {
    u8 unk_000[0x2B0];
    BattleStats *battleStats;               /* +0x2B0 */
};

/* The battle actor. The unit record at +0xA0 is its owner (BattleActor.unit_side in ov02/tu071). */
struct BattleActor {
    u8 unk_00[0xA0];
    BattleUnit *owner;                      /* +0xA0 */
    u8 unk_A4[0xE4 - 0xA4];
    s32 refreshValue;                       /* +0xE4: written by battleActorStoreAndRefresh */
};

s32 func_00AACB10();

void battleActorStoreAndRefresh(BattleActor *actor, s32 storedValue) {
    actor->refreshValue = storedValue;
    func_00AACB10();
}

void battleStatsAddClamped38(BattleActor *actor, s32 delta) {
    BattleUnit *owner;
    BattleStats *afterAddStats;
    BattleStats *beforeAddStats;
    BattleStats *capCompareStats;
    BattleStats *clampCheck;
    BattleStats *amountStats;
    BattleStats *clampedStats;
    BattleStats *capValueStats;
    BattleStats *clampTarget;

    owner = actor->owner;
    afterAddStats = 0;
    if (owner != 0) {
        afterAddStats = owner->battleStats;
    }
    beforeAddStats = 0;
    if (owner != 0) {
        beforeAddStats = owner->battleStats;
    }
    clampCheck = 0;
    afterAddStats->currentAmount = beforeAddStats->currentAmount + delta;
    if (owner != 0) {
        clampCheck = owner->battleStats;
    }
    if (clampCheck->currentAmount < 0) {
        clampTarget = 0;
        if (owner != 0) {
            clampTarget = owner->battleStats;
        }
        clampTarget->currentAmount = 0;
        return;
    }
    capCompareStats = 0;
    if (owner != 0) {
        capCompareStats = owner->battleStats;
    }
    amountStats = 0;
    if (owner != 0) {
        amountStats = owner->battleStats;
    }
    if (capCompareStats->currentCap < amountStats->currentAmount) {
        clampedStats = 0;
        if (owner != 0) {
            clampedStats = owner->battleStats;
        }
        capValueStats = 0;
        if (owner != 0) {
            capValueStats = owner->battleStats;
        }
        clampedStats->currentAmount = capValueStats->currentCap;
    }
}

void battleSecondaryAddClamped(BattleActor *actor, s32 delta) {
    BattleUnit *owner;
    BattleStats *afterAddStats;
    BattleStats *beforeAddStats;
    BattleStats *capCompareStats;
    BattleStats *clampCheck;
    BattleStats *amountStats;
    BattleStats *clampedStats;
    BattleStats *capValueStats;
    BattleStats *clampTarget;

    owner = actor->owner;
    afterAddStats = 0;
    if (owner != 0) {
        afterAddStats = owner->battleStats;
    }
    beforeAddStats = 0;
    if (owner != 0) {
        beforeAddStats = owner->battleStats;
    }
    clampCheck = 0;
    afterAddStats->secondaryAmount = beforeAddStats->secondaryAmount + delta;
    if (owner != 0) {
        clampCheck = owner->battleStats;
    }
    if (clampCheck->secondaryAmount < 0) {
        clampTarget = 0;
        if (owner != 0) {
            clampTarget = owner->battleStats;
        }
        clampTarget->secondaryAmount = 0;
        return;
    }
    capCompareStats = 0;
    if (owner != 0) {
        capCompareStats = owner->battleStats;
    }
    amountStats = 0;
    if (owner != 0) {
        amountStats = owner->battleStats;
    }
    if (capCompareStats->secondaryCap < amountStats->secondaryAmount) {
        clampedStats = 0;
        if (owner != 0) {
            clampedStats = owner->battleStats;
        }
        capValueStats = 0;
        if (owner != 0) {
            capValueStats = owner->battleStats;
        }
        clampedStats->secondaryAmount = capValueStats->secondaryCap;
    }
}
