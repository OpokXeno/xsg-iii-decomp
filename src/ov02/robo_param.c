/*
 * OV02 original TU 76: 0x00aa6518..0x00aa7010 (24 functions)
 */
#include "common.h"
#include "main/tu176.h"

typedef struct RoboBattleStats RoboBattleStats;
typedef struct RoboBattleUnit RoboBattleUnit;
typedef struct RoboActor RoboActor;

/* The stats block behind RoboBattleUnit.battleStats (+0x2B0 of the unit). */
struct RoboBattleStats {
    s32 scalingBase;                           /* +0x00 */
    u8 unk_04[0x0E];
    s8 rollBias;                        /* +0x12 */
    u8 unk_13[0x34 - 0x13];
    u8 fallbackValue;                   /* +0x34 */
    u8 unk_35[0x76 - 0x35];
    u8 unk_76[0x88 - 0x76];
    s16 currentPoints;                   /* +0x88 */
    u8 unk_8A[0x8C - 0x8A];
    s16 activeMode;                           /* +0x8C: 1..3, positive while a activeMode runs */
};

/* The battle unit that RoboActor.battleUnit points at (+0xA0 of the actor). */
struct RoboBattleUnit {
    u8 unk_000[0x2B0];
    RoboBattleStats *battleStats;                   /* +0x2B0 */
    void *motionActor;                  /* +0x2B4: zero when the unit has no motion actor */
};

/* The robo actor passed to the functions of this file. */
struct RoboActor {
    u8 unk_00[0xA0];
    RoboBattleUnit *battleUnit;                     /* +0xA0 */
    u8 unk_A4[0xAC - 0xA4];
    s32 key;                            /* +0xAC */
    u8 unk_B0[0xB8 - 0xB0];
    s32 debugValue;                         /* +0xB8 */
    s32 behaviorFlags;                          /* +0xBC */
    u8 unk_C0[0xDC - 0xC0];
    s32 active;                         /* +0xDC */
    s32 gauge;                          /* +0xE0: 0..100 */
    u8 unk_E4[0x244 - 0xE4];
    s32 modeInitialState;                      /* +0x244 */
    s32 modeEffectHandle;                     /* +0x248 */
    s32 modeActive;                       /* +0x24C */
};

s16 func_00AA65F0();
s32 func_00AA6A08();

void roboParamAdjustCurrent(RoboActor *actor, s32 delta) {
    s32 clampedStat;
    s32 statCap;
    RoboBattleUnit *actorUnit;
    RoboBattleUnit *capUnit;
    RoboBattleUnit *clampedUnit;
    RoboBattleStats *modifiedStats;
    RoboBattleStats *originalStats;
    RoboBattleStats *checkedStats;
    RoboBattleStats *capStats;
    RoboBattleStats *zeroedStats;
    RoboBattleStats *cappedStats;

    if (func_00AA6A08() == 0) {
        actorUnit = actor->battleUnit;
        modifiedStats = 0;
        if (actorUnit != 0) {
            modifiedStats = actorUnit->battleStats;
        }
        originalStats = 0;
        if (actorUnit != 0) {
            originalStats = actorUnit->battleStats;
        }
        checkedStats = 0;
        modifiedStats->currentPoints = originalStats->currentPoints + delta;
        if (actorUnit != 0) {
            checkedStats = actorUnit->battleStats;
        }
        if (checkedStats->currentPoints < 0) {
            zeroedStats = 0;
            if (actorUnit != 0) {
                zeroedStats = actorUnit->battleStats;
            }
            zeroedStats->currentPoints = 0;
            return;
        }
        statCap = func_00AA65F0(actor, modifiedStats);
        capUnit = actor->battleUnit;
        capStats = 0;
        if (capUnit != 0) {
            capStats = capUnit->battleStats;
        }
        if (statCap < capStats->currentPoints) {
            clampedStat = func_00AA65F0(actor);
            clampedUnit = actor->battleUnit;
            cappedStats = 0;
            if (clampedUnit != 0) {
                cappedStats = clampedUnit->battleStats;
            }
            cappedStats->currentPoints = clampedStat;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA65F0);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA6620);

void roboParamAddGauge(RoboActor *actor, s32 delta) {
    s32 gauge;

    gauge = actor->gauge + delta;
    actor->gauge = gauge;
    if (gauge < 0) {
        actor->gauge = 0;
        return;
    }
    if (gauge >= 0x65) {
        actor->gauge = 0x64;
    }
}

s32 roboKeyTableGet(s32);

void roboParamTouchKey(RoboActor *actor) {
    roboKeyTableGet(actor->key);
}

s16 *ov02GetExpRecord(s32);

s16 roboKeyRecordStat(RoboActor *actor, s32 statNumber) {
    s16 *keyRecord;
    RoboBattleUnit *actorUnit;
    RoboBattleStats *battleStats;

    keyRecord = ov02GetExpRecord(actor->key);
    if (keyRecord != 0) {
        return keyRecord[statNumber + 0x24];
    }
    actorUnit = actor->battleUnit;
    battleStats = 0;
    if (actorUnit != 0) {
        battleStats = actorUnit->battleStats;
    }
    return battleStats->fallbackValue;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA68D8);

s32 ov02ChunkedEntryGet();

void roboParamTouchEntry(void) {
    ov02ChunkedEntryGet();
}

s32 func_00A92900(RoboBattleUnit *actorUnit, s32 kind, s32 subKind, s32 *bonus);

s32 roboParamGetScaledValue(RoboActor *actor) {
    s32 bonus;
    s32 scale;
    RoboBattleStats *battleStats;
    s32 scaled;
    RoboBattleUnit *actorUnit;

    scale = 0xF;
    if (func_00A92900(actor->battleUnit, 0xD, 2, &bonus) != 0) {
        scale = bonus + 0xF;
    }
    actorUnit = actor->battleUnit;
    battleStats = 0;
    if (actorUnit != 0) {
        battleStats = actorUnit->battleStats;
    }
    scaled = battleStats->scalingBase * scale;
    return scaled / 100;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA69A8);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA6A08);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA6A58);

extern s32 gRoboModeCostTable[];

s32 roboModeCostGet(RoboActor *unusedActor, s32 modeNumber) {
    return gRoboModeCostTable[modeNumber];
}

s32 ov02ActionEntryGet(s32);
s32 func_00A87C18(s32);
s32 func_00A97168(RoboBattleUnit *actorUnit, s32 value, s32 id);

void roboParamApplyId(RoboActor *actor, s32 id) {
    s32 actionValue;

    actor->modeActive = 0;
    func_00A87C18(actor->key);
    actionValue = ov02ActionEntryGet(id);
    func_00A97168(actor->battleUnit, actionValue, id);
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA6B48);

u32 xglSRand();

s32 roboParamRollScore(RoboActor *actor) {
    u32 random;
    RoboBattleUnit *actorUnit;
    RoboBattleStats *battleStats;

    random = xglSRand();
    actorUnit = actor->battleUnit;
    battleStats = 0;
    if (actorUnit != 0) {
        battleStats = actorUnit->battleStats;
    }
    return ((0x64 - battleStats->rollBias) * actor->gauge) + (random % 9U) + 1;
}

void roboParamSetActive(RoboActor *actor, s32 isActive) {
    s32 effectHandle;

    if (isActive != 0) {
        actor->active = 1;
        effectHandle = battleRosterEntryIdIndexGet(actor->key);
        battleRosterCursorSet(effectHandle, 1);
        return;
    }
    actor->active = 0;
}

void *memset(void *destination, s32 value, u32 count);

void roboActorModeClear(RoboActor *actor) {
    RoboBattleUnit *actorUnit;
    RoboBattleStats *battleStats;

    memset(&actor->modeInitialState, 0, 0xC);
    actorUnit = actor->battleUnit;
    battleStats = 0;
    if (actorUnit != 0) {
        battleStats = actorUnit->battleStats;
    }
    battleStats->activeMode = 0;
}

/* One 8-byte entry of the mode table gRoboModeInitialTable: the state the mode starts from, then four bytes read nowhere. */
typedef struct RoboModeInitial {
    s32 initialState;                          /* +0x00 */
    s32 unk_04;                         /* +0x04 */
} RoboModeInitial;

extern RoboModeInitial gRoboModeInitialTable[];
extern unsigned char gRoboParamSourceFile[];
extern unsigned char D_00ABA798[];
extern unsigned char D_00ABA7B0[];
s32 func_001936A8();
s32 func_00A940E8();
s32 func_00AA1300();
s32 func_00AA4780();
s32 battleActorPostResetEvents();
s32 func_00AA5B20();
void roboParamAdjustCurrent();
s32 roboModeCostGet(RoboActor *unusedActor, s32 modeNumber);

void roboActorModeStart(RoboActor *actor, s32 modeNumber) {
    s32 initialState;
    s32 actorId;
    u32 zeroBasedMode;
    RoboBattleUnit *actorUnit;
    RoboBattleStats *battleStats;
    s32 delta;
    s32 effectHandle;

    zeroBasedMode = modeNumber - 1;
    if (zeroBasedMode >= 3U) {
        actorId = func_00AA1300(actor);
        func_001936A8(D_00ABA798, gRoboParamSourceFile, 0x125, D_00ABA7B0, actorId, actor->debugValue, modeNumber);
    }
    actorUnit = actor->battleUnit;
    battleStats = 0;
    if (actorUnit != 0) {
        battleStats = actorUnit->battleStats;
    }
    battleStats->activeMode = modeNumber;
    initialState = gRoboModeInitialTable[modeNumber].initialState;
    actor->modeActive = 1;
    actor->modeInitialState = initialState;
    func_00A940E8(actorUnit, 0, -1);
    func_00A940E8(actor->battleUnit, 1, -1);
    battleActorPostResetEvents(actor);
    effectHandle = func_00AA4780(actor, 0, 0x136, 0x14);
    actor->modeEffectHandle = effectHandle;
    if (!(actor->behaviorFlags & 0x40000)) {
        delta = roboModeCostGet(actor, modeNumber);
        roboParamAdjustCurrent(actor, -delta);
    }
    func_00AA5B20(actor, 0x1C, 0, 0);
}

s32 sefEffectHandleRelease(s32);
void roboActorModeClear();

void roboActorModeEnd(RoboActor *actor) {
    s32 effectHandle;
    RoboBattleUnit *actorUnit;
    RoboBattleStats *battleStats;

    actorUnit = actor->battleUnit;
    battleStats = 0;
    if (actorUnit != 0) {
        battleStats = actorUnit->battleStats;
    }
    if (battleStats->activeMode <= 0) {
        return;
    }
    effectHandle = actor->modeEffectHandle;
    if (effectHandle != 0) {
        sefEffectHandleRelease(effectHandle);
    }
    roboActorModeClear(actor);
}

void roboModeEffectRelease(RoboActor *actor) {
    s32 effectHandle;
    RoboBattleUnit *actorUnit;

    actorUnit = actor->battleUnit;
    if ((actorUnit != 0) && (actorUnit->motionActor != 0)) {
        effectHandle = actor->modeEffectHandle;
        if (effectHandle != 0) {
            sefEffectHandleRelease(effectHandle);
            actor->modeEffectHandle = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA6E88);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_param", func_00AA6F08);

void *roboBattleUnitGet(u32 index, const char *caller_file, s32 caller_line);
s32 battleActorHasUnitHandle(RoboActor *actor);

s32 roboParamSumMatching(s32 bonusKind, s32 bonusSubKind, s32 *bonusTotal) {
    s32 bonus;
    s32 playerIndex;
    s32 visitedPlayers;
    s32 bonusSum;
    s32 anyMatch;
    RoboActor *actor;

    visitedPlayers = 0;
    bonusSum = 0;
    anyMatch = 0;
    do {
        playerIndex = visitedPlayers;
        visitedPlayers += 1;
        actor = roboBattleUnitGet(playerIndex, gRoboParamSourceFile, 0x185);
        if ((battleActorHasUnitHandle(actor) != 0) && (func_00A92900(actor->battleUnit, bonusKind, bonusSubKind, &bonus) != 0)) {
            anyMatch = 1;
            bonusSum += bonus;
        }
    } while (visitedPlayers < 4);
    if (bonusTotal != 0) {
        *bonusTotal = bonusSum;
    }
    return anyMatch;
}
