#include "common.h"
#include "shared.h"
#include "main/res_get_path.h"
#include "main/tu352.h"
#include "main/xenoActor.h"
#include "ov02/tu012.h"
#include "ov02/tu060.h"

struct XenoActor;

/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyModelHeader *), the texture (.txy), the
 * hierarchy (.xhr, XenoActorAllocMatrices builds the matrices from it), the motion tables (.xap: slot n is
 * XAct_Rsrc_XAP + n, slots 0..5 = indices 4..9) and a few entries xenoActor.c reads as pointers. The names of the
 * first seven are those of the original's asserts; no assert names the rest. */
enum XActRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,                            /* entry passed to tiMRPackageSetAnimData */
    XAct_Rsrc_XAP,                          /* OffsetTable * of slot 0 */
    XAct_Rsrc_5,
    XAct_Rsrc_XAP_Ext2,
    XAct_Rsrc_7,
    XAct_Rsrc_SlotEntry,                    /* slot 4, read as an int entry (the motion set's second entry) */
    XAct_Rsrc_SlotTableB,                   /* slot 5, read as another table */
    XAct_Rsrc_PAM,                          /* non-zero when a PAM pose is active */
    XAct_Rsrc_EntryTableC,                  /* tiMRPackage range entry C */
    XAct_Rsrc_Count
};

void XenoActorSetMode(struct XenoActor *actor, int mode);
void XenoActorEnableChannel(struct XenoActor *actor);
void XenoActorEnableChannelB(struct XenoActor *actor);
void XenoActorSetChannelValue(struct XenoActor *actor, float value);
void XenoActorSetChannelValueB(struct XenoActor *actor, float value);
void XenoActorSetSelector(struct XenoActor *actor, int selector);

#define DATA_LOOKUP_CAPACITY 12 /* items in a DataLookup block */

struct BattleUnit;

/* A unit has three child units (and three slot flags, one per child); the command table has 40 handlers. */
#define UNIT_CHILD_COUNT 3

struct FadeTask;
int unitCurrentPhase(BattleUnit *battleUnit);
void unitSetPhase(BattleUnit *battleUnit, int phase);
void unitSetSelector(BattleUnit *battleUnit, int selector);
void unitRefreshPhase(BattleUnit *battleUnit);
struct XenoLight;
int partyUnitHpLow(struct BattleUnit *owner);
int unitCmdFinish(BattleUnit *battleUnit, BattleUnitCommand *command);
int unitTblSet(int side, BattleUnit *battleUnit);
BattleUnit *func_00A992D0(int id, int side);

typedef int s32;

/* Fields of a BattleUnitLookupKeyView (ov02/tu060.h names only lookupKey) that this file reads; the offsets are measured here. */
typedef struct UnitPoolRow {
    unsigned char unmodeled_00[0x1C];
    short gridX;                            /* +0x1C: shifted left by 12 before func_002105B8 */
    unsigned char unmodeled_1E[0x3E - 0x1E];
    short lookupKey;                        /* +0x3E: the unit filter is lookupKey < 0x12 */
    unsigned char unmodeled_40[0x72 - 0x40];
    short tblSlot;                          /* +0x72: the slot unitTblSet returned for the unit */
    unsigned char unmodeled_74[0x78 - 0x74];
    float targetX;                          /* +0x78: first percent / 100 */
    float targetZ;                          /* +0x7C: second percent / 100 */
    short moveState;                        /* +0x80: cleared when the arrive command starts */
} UnitPoolRow;

/* Creates a player unit and enters it in the player side of the unit table; returns the unit or 0. */
BattleUnit *unitPlayerCreate(int id) {
    BattleUnit *playerUnit;

    playerUnit = func_00A992D0(id, 0);
    if (playerUnit != 0) {
        ((UnitPoolRow *) playerUnit->lookupKeyRecord)->tblSlot = unitTblSet(0, playerUnit);
    }
    return playerUnit;
}

/* Creates an enemy unit, sets the 0xC bits of its targetingFlags word, then enters it in the enemy side of the unit
 * table; returns the unit or 0. */
BattleUnit *unitEnemyCreate(int id) {
    BattleUnit *enemyUnit;

    enemyUnit = func_00A992D0(id, 1);
    enemyUnit->targetingFlags |= 0xC;
    if (enemyUnit != 0) {
        ((UnitPoolRow *) enemyUnit->lookupKeyRecord)->tblSlot = unitTblSet(1, enemyUnit);
    }
    return enemyUnit;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A992D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99390);

/* The step counter and the timer of a unit command. They sit in the opaque commandQueue block of BattleUnit
 * (ov02/tu060.h, +0x10 for 0x12C bytes); the offsets are measured here. */
typedef struct UnitCommandState {
    unsigned char unmodeled_00[0x20];
    s32 stepsTaken;                            /* +0x20: 0 until the one-shot start has run, then the steps taken */
    s32 framesWaited;                      /* +0x24: frames the command has waited */
} UnitCommandState;

/* Callees of this file. The func_ names are the original's; the others are declared where the file needs them. */
void unitReleaseResources(BattleUnit *battleUnit);
s32 unitSoundSlotRelease();
void XenoActorRelease(XenoActor *actor);
s32 func_002105B8(s32 gridX, s32 flag);
void func_00A857F0(BattleUnit *battleUnit);
void unitSideRelease(BattleUnit *battleUnit);
int unitTblRemove(BattleUnit *battleUnit);
s32 scKeyRangeSample(XenoActor *actor, s16 lookupKey);
void XenoActorWrapRotation(XenoActor *actor);
s32 func_00A999A8(void);
s32 func_00A83BF0(s32 queue);
s32 commandQueueReset(s32 queue);
int commandQueueDone(void *p);
void func_00A8AD20(BattleUnit *battleUnit, s32 command);
s32 unitIsActorDone(BattleUnit *battleUnit);
void unitSoundSlotStart(BattleUnit *battleUnit, s32 value);
void battleUnitRefreshActionCode(BattleUnit *battleUnit);
s32 func_00A9AB70(BattleUnit *battleUnit);
s32 func_00A818D0(BattleUnit *battleUnit);
u32 unitActorKeyNext(void);
void func_00A940E8(BattleUnit *battleUnit, s32 slot, s32 value);
void func_00A854D8(BattleUnit *battleUnit, s32 a, s32 b);
void unitSoundPlay(BattleUnit *battleUnit, s32 sound);
void unitSetupPhaseActor(BattleUnit *battleUnit, s32 phase);

void unitReleaseSoundAndResources(BattleUnit *battleUnit) {
    UnitPoolRow *poolRow;

    poolRow = (UnitPoolRow *) battleUnit->lookupKeyRecord;
    if (poolRow->lookupKey < 0x12) {
        unitSoundSlotRelease();
    }
    unitReleaseResources(battleUnit);
}

void unitReleaseResources(BattleUnit *battleUnit) {
    int childIndex;
    UnitPoolRow *poolRow;

    func_00A857F0(battleUnit);
    for (childIndex = 0; childIndex < 4; childIndex++) {
        if (battleUnit->children[childIndex] != 0) {
            XenoActorRelease(battleUnit->children[childIndex]);
        }
    }
    XenoActorRelease(battleUnit->motionActor);
    poolRow = (UnitPoolRow *) battleUnit->lookupKeyRecord;
    if (poolRow->lookupKey < 0x12) {
        func_002105B8(poolRow->gridX << 0xC, 1);
    }
    unitSideRelease(battleUnit);
    unitTblRemove(battleUnit);
}

void unitTickCommandQueues(void) {
    func_00A999A8();
}

void unitActorSetKey(BattleUnit *battleUnit) {
    scKeyRangeSample(battleUnit->motionActor, ((UnitPoolRow *) battleUnit->lookupKeyRecord)->lookupKey);
}

void unitSetPositionPercent(BattleUnit *battleUnit, s32 xPercent, s32 zPercent) {
    XenoActor *actor;

    actor = battleUnit->motionActor;
    actor->position.y = 0;
    actor->position.w = 1.0f;
    actor->position.x = xPercent / 100.0f;
    actor->position.z = zPercent / 100.0f;
    XenoActorWrapRotation(actor);
}

void unitSetTargetPercent(BattleUnit *battleUnit, s32 targetXPercent, s32 targetZPercent) {
    UnitPoolRow *poolRow;

    poolRow = (UnitPoolRow *) battleUnit->lookupKeyRecord;
    poolRow->targetX = targetXPercent / 100.0f;
    poolRow->targetZ = targetZPercent / 100.0f;
}

void unitSetFacing(BattleUnit *battleUnit, f32 rotationY) {
    XenoActor *actor;

    actor = battleUnit->motionActor;
    actor->rotation.y = rotationY;
    actor->rotation.x = 0;
    actor->rotation.z = 0;
    actor->rotation.w = 1.0f;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99638);

void unitSetChildFlags(BattleUnit *battleUnit);

void unitSetDrawFlagTree(BattleUnit *battleUnit) {
    XenoActor *actor;

    actor = battleUnit->motionActor;
    actor->drawFlags |= 4;
    battleUnit->targetingFlags |= 2;
    unitSetChildFlags(battleUnit);
}

void unitSetChildFlags(BattleUnit *battleUnit) {
    s32 childIndex;
    XenoActor *child;

    for (childIndex = 0; childIndex < 4; childIndex++) {
        child = battleUnit->children[childIndex];
        if (child != 0) {
            child->drawFlags |= 4;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99730);

void unitSnapActorToTarget(BattleUnit *battleUnit) {
    if (unitActorKeyNext() - 0xF < 2U) {
        func_00A8AD20(battleUnit, 0x13);
    }
    battleUnit->motionActor->position.z = ((UnitPoolRow *) battleUnit->lookupKeyRecord)->targetZ;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A997E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99860);

/* The command queue is the block at +0x10 of the unit. */
void unitCommandQueueClear(s32 battleUnitAddress) {
    func_00A83BF0(battleUnitAddress + 0x10);
}

void unitCommandQueueReset(s32 battleUnitAddress) {
    commandQueueReset(battleUnitAddress + 0x10);
}

/* Tail-calls commandQueueDone on the command queue at +0x10 of the unit. */
int unitCommandQueueDone(BattleUnit *battleUnit)
{
    return commandQueueDone(&battleUnit->commandQueue);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A999A8);

s32 unitCmdNoop(void) {
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99A90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99B10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99B90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99BE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99C40);

s32 unitCmdRelease(BattleUnit *battleUnit) {
    UnitCommandState *commandState;

    commandState = (UnitCommandState *) battleUnit;
    if (commandState->stepsTaken == 0) {
        func_00A8AD20(battleUnit, 0xB);
        func_00A9AB70(battleUnit);
        commandState->stepsTaken += 1;
    } else if (unitIsActorDone(battleUnit) != 0) {
        unitReleaseSoundAndResources(battleUnit);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99D18);

/* Finishes at once: unitRefreshPhase, clears the active flag, reports done. */
int unitCmdFinish(BattleUnit *battleUnit, BattleUnitCommand *unusedStep)
{
    unitRefreshPhase(battleUnit);
    battleUnit->targetingFlags &= ~1;
    return 1;
}

void unitCmdFinishFlagged(BattleUnit *battleUnit, BattleUnitCommand *command) {
    battleUnit->targetingFlags |= 0x10;
    unitCmdFinish(battleUnit, command);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99E78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99ED8);

s32 unitCmdWaitClearActive(BattleUnit *battleUnit) {
    UnitCommandState *commandState;

    commandState = (UnitCommandState *) battleUnit;
    if (commandState->stepsTaken == 0) {
        func_00A8AD20(battleUnit, 6);
        unitSoundSlotStart(battleUnit, 1);
        commandState->stepsTaken += 1;
    } else if (unitIsActorDone(battleUnit) != 0) {
        battleUnit->targetingFlags &= ~1;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A100);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A210);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A4A0);

s32 unitCmdPoseArrive(BattleUnit *battleUnit) {
    UnitCommandState *commandState;
    UnitPoolRow *poolRow;
    s32 arriveFrameLimit;

    poolRow = (UnitPoolRow *) battleUnit->lookupKeyRecord;
    commandState = (UnitCommandState *) battleUnit;
    if (commandState->stepsTaken == 0) {
        if (poolRow->lookupKey < 0x12) {
            if (func_00A818D0(battleUnit) == 0) {
                unitSoundSlotStart(battleUnit, 9);
                func_00A8AD20(battleUnit, 0x14);
            }
        } else {
            func_00A8AD20(battleUnit, 1);
        }
        func_00A857F0(battleUnit);
        func_00A940E8(battleUnit, 0, -1);
        func_00A940E8(battleUnit, 1, -1);
        ((UnitPoolRow *) battleUnit->lookupKeyRecord)->moveState = 0;
        commandState->stepsTaken += 1;
    } else {
        arriveFrameLimit = 0x46;
        if (poolRow->lookupKey < 0x12) {
            if (func_00A818D0(battleUnit) != 0) {
                arriveFrameLimit = 0x64;
            }
        }
        if (arriveFrameLimit < commandState->framesWaited) {
            battleUnit->targetingFlags &= ~1;
            return 1;
        }
    }
    return 0;
}

s32 unitCmdClearActive(BattleUnit *battleUnit) {
    battleUnit->targetingFlags &= ~1;
    return 1;
}

s32 unitCmdSoundWaitRefresh(BattleUnit *battleUnit) {
    UnitCommandState *commandState;
    u32 keepFlagsMask;

    commandState = (UnitCommandState *) battleUnit;
    if (commandState->stepsTaken == 0) {
        func_00A8AD20(battleUnit, 0x12);
        unitSoundPlay(battleUnit, 0xB);
        commandState->stepsTaken += 1;
    } else if (unitIsActorDone(battleUnit) != 0) {
        keepFlagsMask = 0xFFFF7FFF;
        battleUnit->targetingFlags = (battleUnit->targetingFlags & keepFlagsMask) & ~1;
        battleUnitRefreshActionCode(battleUnit);
        unitRefreshPhase(battleUnit);
        return 1;
    }
    return 0;
}

s32 unitCmdSetupPhaseActor(BattleUnit *battleUnit) {
    UnitCommandState *commandState;

    commandState = (UnitCommandState *) battleUnit;
    if (commandState->stepsTaken == 0) {
        func_00A8AD20(battleUnit, 9);
        unitSetupPhaseActor(battleUnit, 4);
        func_00A854D8(battleUnit, 0x136, 0x11);
        unitSoundSlotStart(battleUnit, 3);
        commandState->stepsTaken += 1;
    } else if (unitIsActorDone(battleUnit) != 0) {
        unitSetupPhaseActor(battleUnit, 0);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A760);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A820);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A8C0);

s32 unitCmdResetPlacement(BattleUnit *battleUnit) {
    XenoActor *actor;
    s32 flagsWithBit80;

    actor = battleUnit->motionActor;
    actor->position.z = -2.0f;
    actor->rotation.y = 0;
    func_00A8AD20(battleUnit, 1);
    flagsWithBit80 = battleUnit->targetingFlags | 0x80;
    battleUnit->targetingFlags = flagsWithBit80;
    return 1;
}
