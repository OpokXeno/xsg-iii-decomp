/*
 * OV02 original TU 69: 0x00aa00f8..0x00aa1908 (41 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA00F8);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA0650);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA07D8);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA0910);

typedef int s32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef short s16;
typedef float f32;

/* The robo battle table at gRoboBattleTable: one 0x2F0-byte block per slot, then the battle unit list and counters. */
typedef struct RoboBattleTable {
    u8 units[0xD][0x2F0];           /* +0x0000 */
    void *battleUnits[0xD];           /* +0x2630: filled by roboBattleUnitRegister */
    s32 battleUnitCount;                 /* +0x2664 */
    u8 unk_2668[0x2850 - 0x2668];
    s32 serial;                     /* +0x2850: bumped by roboUnitTableSerialBump */
} RoboBattleTable;

typedef struct RoboPartPercentBlock {
    u8 unk_00[0x18];
    s16 percent;                    /* +0x18 */
} RoboPartPercentBlock;

typedef struct RoboBodyPart {
    u8 unk_00[0x2B0];
    RoboPartPercentBlock *percentBlock; /* +0x2B0 */
} RoboBodyPart;

typedef struct RoboBattleUnit {
    u8 unk_00[0xA0];
    RoboBodyPart *percentPart;              /* +0xA0 */
    u8 unk_A4[0xAC - 0xA4];
    void *link;                     /* +0xAC */
    u8 unk_B0[0xB8 - 0xB0];
    s32 debugValue;                 /* +0xB8: the value argument printed by roboBattleUnitRegister */
} RoboBattleUnit;

extern RoboBattleTable gRoboBattleTable;
extern const char gRoboBattleSourceFile[];
extern const char gRoboUnitListFullMessage[];
extern const char gRoboUnitAddMessage[];
extern const char D_00AB9E18[];
extern const char D_00AB9E40[];

void assertFailed(const char *message, const char *file, int line);
void func_001936F0(const char *file, s32 line, const char *message, s32 value);
void func_001936A8();
void func_00A9DD70(void);
void *func_00AA1370(s32 entry);
s32 unitTblEntryGet();
u32 func_00AA1300(void);
s32 roboCurrentIsPlayer(void);
s32 battleActorHasUnitHandle(void *unit);
void func_001E8E70();
void func_001E9100();

void roboSceneHookCall(void) {
    func_00A9DD70();
}

extern s32 gRoboRemainingEnemies;

s32 roboRemainingEnemiesGet(void) {
    return gRoboRemainingEnemies;
}

void roboRemainingEnemiesSet(s32 remainingEnemies) {
    gRoboRemainingEnemies = remainingEnemies;
    if (remainingEnemies < 0) {
        gRoboRemainingEnemies = 0;
    }
}

extern s32 gRoboSecondaryCounter;

s32 roboSecondaryCounterGet(void) {
    return gRoboSecondaryCounter;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA0C00);

extern s32 gRoboBattleEnded;

void roboBattleEndedSet(void) {
    gRoboBattleEnded = 1;
}

void roboBattleEndedClear(void) {
    gRoboBattleEnded = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA0C30);

void roboBattleUnitRegister(RoboBattleUnit *battleUnit)
{
    RoboBattleTable *battleTable = &gRoboBattleTable;
    s32 *battleUnitCounter;

    if (battleTable->battleUnitCount >= 0xD) {
        assertFailed(gRoboUnitListFullMessage, gRoboBattleSourceFile, 0x2C7);
    }
    func_001936F0(gRoboBattleSourceFile, 0x2C9, gRoboUnitAddMessage, battleUnit->debugValue);
    battleUnitCounter = &battleTable->battleUnitCount;
    battleTable->battleUnits[*battleUnitCounter] = battleUnit;
    *battleUnitCounter += 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA0DC8);

void roboNoopStep(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA0FC0);

void *roboBattleUnitGet(u32 battleSlotIndex, const char *caller_file, s32 caller_line) {
    if (battleSlotIndex >= 0xDU) {
        func_001936A8(D_00AB9E18, gRoboBattleSourceFile, 0x393, D_00AB9E40, caller_file, caller_line, battleSlotIndex);
    }
    return gRoboBattleTable.units[battleSlotIndex];
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA1300);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA1370);

void *roboCurrentBattleUnitGet(void) {
    s32 battleSlot;
    battleSlot = unitTblEntryGet();
    return func_00AA1370(battleSlot);
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA1410);

extern u8 gRoboBattleStateBlock[];

void *roboBattleStateBlockGet(void) {
    return gRoboBattleStateBlock;
}

s32 roboCurrentIsPlayer(void) {
    return func_00AA1300() < 4U;
}

s32 roboCurrentIsEnemy(void) {
    return func_00AA1300() - 4 < 9U;
}

s32 roboFirstSlotForSide(s32 isPlayer) {
    return (isPlayer != 0) ? 0 : 4;
}

s32 roboEndSlotForSide(s32 isPlayer) {
    return (isPlayer != 0) ? 4 : 0xD;
}

s32 roboSlotCountForSide(s32 isPlayer) {
    return (isPlayer != 0) ? 4 : 9;
}

void roboGetSideRange(s32 unused_arg0, s32 sameSide, s32 *sideFirstSlot, s32 *sideEndSlot) {
    if (roboCurrentIsPlayer() != 0) {
        if (sameSide != 0) {
            *sideFirstSlot = 0;
            *sideEndSlot = 4;
        } else {
            *sideFirstSlot = 4;
            *sideEndSlot = 0xD;
        }
    } else if (sameSide != 0) {
        *sideFirstSlot = 4;
        *sideEndSlot = 0xD;
    } else {
        *sideFirstSlot = 0;
        *sideEndSlot = 4;
    }
}

s32 roboQueryDisabled(void) {
    return 0;
}

extern s32 D_00AC7DBC;

s32 roboFlagGet(void) {
    return D_00AC7DBC;
}

extern s32 D_00AC7DC0;

s32 roboModeGet(void) {
    return D_00AC7DC0;
}

void roboUnitTableSerialBump(void)
{
    RoboBattleTable *battleTable = &gRoboBattleTable;

    battleTable->serial += 1;
}

extern s32 D_00AC7DA0;

s32 roboPlayerAliveScan(s32 *aliveCount) {
    s32 playerIndex;
    void *battleUnit;

    if (aliveCount != 0) {
        *aliveCount = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            battleUnit = roboBattleUnitGet(playerIndex, gRoboBattleSourceFile, 0x427);
            if (battleActorHasUnitHandle(battleUnit) != 0) {
                *aliveCount += 1;
            }
        }
    }
    return D_00AC7DA0;
}

extern s32 D_00AC7DA4;

s32 roboEnemyAliveScan(s32 *aliveCount) {
    s32 enemyIndex;
    void *battleUnit;

    if (aliveCount != 0) {
        *aliveCount = 0;
        for (enemyIndex = 4; enemyIndex < 0xD; enemyIndex++) {
            battleUnit = roboBattleUnitGet(enemyIndex, gRoboBattleSourceFile, 0x438);
            if (battleActorHasUnitHandle(battleUnit) != 0) {
                *aliveCount += 1;
            }
        }
    }
    return D_00AC7DA4;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA16D8);

void *roboCurrentBattleUnitLinkGet(void)
{
    RoboBattleUnit *currentBattleUnit;

    currentBattleUnit = roboCurrentBattleUnitGet();
    if (currentBattleUnit != 0) {
        return currentBattleUnit->link;
    }
}

void roboCurrentPercentRatio(s32 unused_arg0, f32 *ratio, f32 *limit) {
    RoboBattleUnit *currentBattleUnit;
    RoboBodyPart *percentPart;
    RoboPartPercentBlock *partPercentBlock;

    currentBattleUnit = roboCurrentBattleUnitGet();
    if ((currentBattleUnit != 0) && (percentPart = currentBattleUnit->percentPart, (percentPart != 0)) && (partPercentBlock = percentPart->percentBlock, (partPercentBlock != 0))) {
        if (ratio != 0) {
            *ratio = partPercentBlock->percent / 100.0f;
        }
    } else if (ratio != 0) {
        *ratio = 5.0f;
    }
    if (limit != 0) {
        *limit = 10.0f;
    }
}

void roboSetActive(s32 active) {
    if (active != 0) {
        func_001E8E70();
        return;
    }
    func_001E9100();
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA1810);

s32 roboIsEnabled(void) {
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_battle", func_00AA1830);
