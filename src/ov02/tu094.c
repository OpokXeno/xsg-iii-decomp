/*
 * OV02 original TU 94: 0x00ab3200..0x00ab4ab8 (29 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef float f32;
typedef unsigned int u32;
typedef short s16;

typedef struct BattleStats BattleStats;
typedef struct BattleUnit BattleUnit;
typedef struct RoboActor RoboActor;

/* The stats record that a battle unit points to at +0x2B0. Only the claimed fields are named. */
struct BattleStats {
    s32 percentDivisor;                            /* +0x00: the divisor of the percent in roboUnitCalcHitThreshold */
    unsigned char unmodeled_04[0x38 - 0x04];
    s32 percentValue;                        /* +0x38: the value the percent is taken from */
    unsigned char unmodeled_3C[0x8A - 0x3C];
    s16 statusEffectIdA;                        /* +0x8A */
    s16 statusEffectIdB;                        /* +0x8C */
};

/* The battle unit a robo unit points to at +0xA0. Only the stats pointer is claimed. */
struct BattleUnit {
    unsigned char unmodeled_00[0x2B0];
    BattleStats *battleStats;                 /* +0x2B0 */
};

/* A robo unit as returned by roboBattleUnitGet. */
struct RoboActor {
    unsigned char unmodeled_00[0xA0];
    BattleUnit *battle;                 /* +0xA0 */
    unsigned char unmodeled_A4[0xB8 - 0xA4];
    s32 messageParam;                   /* +0xB8: the last argument of func_001936F0 */
};

s32 func_00AAB628();
extern s32 D_00AB81F8;
extern s32 D_00ABB9B0;
s32 sbNativeOwnerAdd(s32 *, s32 *);

void roboStateTableInit(void) {
    func_00AAB628();
}

void sbRegisterNativeOwner(void) {
    sbNativeOwnerAdd(&D_00ABB9B0, &D_00AB81F8);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3240);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB32C0);

void roboNoOp(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB34E8);

s32 func_00AB34E8();

s32 roboStartCommandFlagged(s32 actorId, s32 targetId, s32 commandValue) {
    return func_00AB34E8(actorId, 1, targetId, commandValue);
}

s32 roboStartCommandPlain(s32 actorId, s32 targetId, s32 commandValue) {
    return func_00AB34E8(actorId, 0, targetId, commandValue);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3618);

extern unsigned char D_00ABBBE0[];
void *roboBattleUnitGet(u32 battleSlotIndex, const char *caller_file, s32 caller_line);

s32 roboPairStatDelta(s32 *leftId, s32 *rightId) {
    RoboActor *left_unit;
    RoboActor *right_unit;
    BattleUnit *right_battle;
    BattleUnit *left_battle;
    BattleStats *left_stats;
    BattleStats *right_stats;

    left_unit = roboBattleUnitGet(*leftId, D_00ABBBE0, 0x15B);
    right_unit = roboBattleUnitGet(*rightId, D_00ABBBE0, 0x15C);
    right_stats = 0;
    left_battle = left_unit->battle;
    left_stats = 0;
    if (left_battle != 0) {
        left_stats = left_battle->battleStats;
    }
    right_battle = right_unit->battle;
    if (right_battle != 0) {
        right_stats = right_battle->battleStats;
    }
    return left_stats->percentValue - right_stats->percentValue;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3780);

s32 unitStatUsable(BattleUnit *);
u8 *ov02BattleSlotGet(s32 entryNumber);
s32 func_00A86530(s32);

s32 roboUnitOwnedStat(RoboActor *roboActor, s32 statId) {
    s32 owner_ok;
    s32 ownedStat;

    owner_ok = unitStatUsable(roboActor->battle);
    ownedStat = ov02BattleSlotGet(statId);
    if ((owner_ok != 0) && (func_00A86530(statId) > 0)) {
        return ownedStat;
    }
    return 0;
}

extern s32 gRoboStatIds[];
s32 calcStatGet();

s32 roboUnitOwnedStatMask(RoboActor *roboActor) {
    s32 statIdIndex;
    s32 ownedStatFlags;
    s32 *statIdCursor;

    statIdCursor = gRoboStatIds;
    ownedStatFlags = 0;
    for (statIdIndex = 0; statIdIndex < 6; statIdIndex++) {
        if (calcStatGet(roboActor->battle, 0, *statIdCursor) != 0) {
            ownedStatFlags |= *statIdCursor;
        }
        statIdCursor += 1;
    }
    return ownedStatFlags;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3948);

s32 func_00AB3A30();
s32 func_00AB3CC8();

void roboStartModeAction(s32 actorId, s32 actionMode) {
    if (actionMode == 0) {
        func_00AB3A30(actorId, actionMode);
        return;
    }
    if (actionMode == 1) {
        func_00AB3A30(actorId, actionMode);
        return;
    }
    func_00AB3CC8(actorId);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3A30);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3BE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3CC8);

extern unsigned char gRoboHitThresholdMessage[];
s32 func_001936F0();
s32 func_00A9B8D8(s32, s32, f32 *, f32);

void roboUnitCalcHitThreshold(RoboActor *roboActor, s32 id, s32 actionMode, s32 *thresholdOut, s32 *percentOut) {
    f32 hitThreshold;
    BattleStats *owner_stats;
    BattleStats *owner_base;
    s32 percent;
    s32 divisor;
    s32 scaled_total;
    s32 hitThresholdInt;
    BattleUnit *owner;

    owner_stats = 0;
    owner = roboActor->battle;
    if (owner != 0) {
        owner_stats = owner->battleStats;
    }
    owner_base = 0;
    scaled_total = owner_stats->percentValue * 0x64;
    if (owner != 0) {
        owner_base = owner->battleStats;
    }
    divisor = owner_base->percentDivisor;
    percent = scaled_total / divisor;
    if (func_00A9B8D8(id, actionMode, &hitThreshold, (0x64 - percent)) == 0) {
        func_001936F0(D_00ABBBE0, 0x253, gRoboHitThresholdMessage, roboActor->messageParam);
        return;
    }
    hitThresholdInt = (s32) hitThreshold;
    *thresholdOut = hitThresholdInt;
    *percentOut = percent;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB3FE0);

s32 func_00AA1300(s32);
s32 queueRestartWithValue(RoboActor *, s32);
s32 queueSetPair(RoboActor *, s32, s32);
s32 roboUnitOwnedStat();

s32 roboUnitApplyFirstOwned(RoboActor *roboActor, s32 targetId, s32 *statIdList, s32 statIdCount) {
    s32 *statIdCursor;
    s32 statIdIndex;
    s32 restartResult;
    s32 statId;
    s32 converted;

    statIdCursor = statIdList;
    statIdIndex = 0;
    for (;;) {
        restartResult = statIdIndex < statIdCount;
        statIdIndex += 1;
        if (restartResult == 0) {
            break;
        }
        statId = *statIdCursor;
        statIdCursor += 1;
        if (roboUnitOwnedStat(roboActor, statId) != 0) {
            converted = func_00AA1300(targetId);
            queueSetPair(roboActor, statId, converted);
            restartResult = queueRestartWithValue(roboActor, 0x12);
            break;
        }
    }
    return restartResult;
}

extern s32 gRoboOwnedStatIdsA[];
extern s32 gRoboOwnedStatIdsB[];
extern unsigned char gRoboRollMessage[];
s32 queueIsEmpty();
s32 func_00AB3240();
s32 func_00AB3FE0();
s32 roboUnitApplyFirstOwned();
u32 xglSRand();

void roboUnitRollAction(RoboActor *roboActor, s32 targetId, s32 actionMode) {
    s32 hitThreshold;
    s32 percent;
    s32 hit;
    s32 limit;
    u32 firstRoll;
    u32 secondRoll;
    u32 firstPercent;

    if ((queueIsEmpty(roboActor) != 0) && (func_00AB3FE0(roboActor, targetId, actionMode, &hitThreshold, &percent) >= 2) && (func_00AB3240(D_00ABBBE0, 0x299, gRoboRollMessage, hitThreshold, percent), firstRoll = xglSRand(), firstPercent = firstRoll % 100U, hit = (s32) firstPercent < hitThreshold, (hit != 0))) {
        secondRoll = xglSRand();
        limit = (secondRoll % 10U) + 0x2D;
        if (percent >= limit) {
            roboUnitApplyFirstOwned(roboActor, (s32) roboActor, gRoboOwnedStatIdsA, 2);
            return;
        }
        roboUnitApplyFirstOwned(roboActor, (s32) roboActor, gRoboOwnedStatIdsB, 2);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB42C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB44D8);

s32 func_00AA6A58(void *, s16);
s32 func_00AA6B48(void *, s16);

void roboUnitTickStatusEffects(RoboActor *roboActor, s32 chance)
{
    BattleUnit *hitBattle;
    BattleUnit *statusBBattle;
    BattleUnit *battle;
    BattleStats *battleStats;
    BattleStats *checkedBattleStats;
    BattleStats *battleStatsA;
    BattleStats *battleStatsB;
    BattleStats *hitStatsA;
    BattleStats *hitStatsB;

    if (!(queueIsEmpty() != 0))
    {
        return;
    }
    battle = roboActor->battle;
    battleStats = 0;
    if (battle != 0)
    {
        battleStats = battle->battleStats;
    }
    if ((battleStats->statusEffectIdA) != 0)
    {
        checkedBattleStats = 0;
        if (battle != 0)
        {
            checkedBattleStats = battle->battleStats;
        }
        if ((checkedBattleStats->statusEffectIdB) == 0)
        {
            battleStatsA = 0;
            if (battle != 0)
            {
                battleStatsA = battle->battleStats;
            }
            if ((func_00AA6A58(roboActor, battleStatsA->statusEffectIdA) != 0) && (((s32) (xglSRand() % 100U)) < chance))
            {
                hitBattle = roboActor->battle;
                hitStatsA = 0;
                if (hitBattle != 0)
                {
                    hitStatsA = hitBattle->battleStats;
                }
                queueRestartWithValue(roboActor, (hitStatsA->statusEffectIdA) + 0xB);
            }
        }
        else
        {
            battleStatsB = 0;
            if (battle != 0)
            {
                battleStatsB = battle->battleStats;
            }
            if (func_00AA6B48(roboActor, battleStatsB->statusEffectIdB) != 0)
            {
                statusBBattle = roboActor->battle;
                hitStatsB = 0;
                if (statusBBattle != 0)
                {
                    hitStatsB = statusBBattle->battleStats;
                }
                queueRestartWithValue(roboActor, (hitStatsB->statusEffectIdB) + 0xE);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB4738);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB47E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB48D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu094", func_00AB49A0);
