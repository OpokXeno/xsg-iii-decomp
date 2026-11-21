/*
 * OV02 original TU 81: 0x00aabe08..0x00aaca68 (26 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned char u8;

typedef struct RoboPartParameters {
    u8 unmodeled_000[0x40];
    s16 parameterValues[1];             /* 0x40, indexed by the per-slot index; the count is not used here */
} RoboPartParameters;

typedef struct RoboMechPart {
    u8 unmodeled_000[0x2B0];
    RoboPartParameters *parameters; /* 0x2B0 */
} RoboMechPart;

typedef struct RoboCommandUnit {
    u8 unmodeled_000[0xA0];
    RoboMechPart *mechPart;     /* 0xA0 */
    u8 unmodeled_A4[0x40];
    s32 primarySlot;                /* 0xE4 */
    u8 unmodeled_E8[0x178];
    s32 commandCount;                 /* 0x260 */
    s32 commandIndex;                 /* 0x264 */
    s32 notEmptyAsserted;                  /* 0x268 */
    u8 unmodeled_26C[0x8];
    s32 commands[20];           /* 0x274 */
    s32 commandTargetIndex;                 /* 0x2C4 */
    s32 secondarySlot;                 /* 0x2C8 */
    u8 unmodeled_2CC[0x8];
    s32 resetSentinelA;           /* 0x2D4 */
    s32 resetSentinelB;           /* 0x2D8 */
} RoboCommandUnit;
s32 func_00AABE48();
s32 memset();

void queueReset(RoboCommandUnit *commandUnit)
{
  memset(&commandUnit->commandCount, 0, 0x7C);
  commandUnit->resetSentinelB = -1;
  commandUnit->resetSentinelA = -1;
  func_00AABE48(commandUnit);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AABE48);

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AABE68);

s32 queueGetCurrent(RoboCommandUnit *commandUnit) {
    s32 commandIndex;
    s32 commandCount;

    commandCount = commandUnit->commandCount;
    if (commandCount > 0) {
        commandIndex = commandUnit->commandIndex;
        if (commandIndex < commandCount) {
            return commandUnit->commands[commandIndex];
        }
    }
    return 0;
}

void queueAdvance(RoboCommandUnit *commandUnit) {
    s32 commandIndex;

    commandIndex = commandUnit->commandIndex;
    if (commandIndex < commandUnit->commandCount) {
        commandUnit->commandIndex = commandIndex + 1;
    }
}

void queueAssertNotEmptyAndFlag(RoboCommandUnit *commandUnit);
s32 func_00AAC210();

void queueRestartWithValue(RoboCommandUnit *commandUnit, s32 commandId) {
    func_00AABE48(commandUnit);
    func_00AAC210(commandUnit, commandId);
    queueAssertNotEmptyAndFlag(commandUnit);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AAC210);

void queueSetPair(RoboCommandUnit *commandUnit, s32 commandTargetIndex, s32 secondarySlot) {
    commandUnit->commandTargetIndex = commandTargetIndex;
    commandUnit->secondarySlot = secondarySlot;
}

void queuePop(RoboCommandUnit *commandUnit) {
    s32 commandCount;

    commandCount = commandUnit->commandCount;
    if (commandCount > 0) {
        commandUnit->commandCount = commandCount - 1;
    }
}

s32 queueIsEmpty(RoboCommandUnit *commandUnit);
extern s32 queueSourceFile;
extern s32 queueNotEmptyAssertText;
s32 func_001936F0(s32 *, s32, s32 *);

void queueAssertNotEmptyAndFlag(RoboCommandUnit *commandUnit) {
    if (queueIsEmpty(commandUnit) != 0) {
        func_001936F0(&queueSourceFile, 0xD3, &queueNotEmptyAssertText);
    }
    commandUnit->notEmptyAsserted = 1;
}

s32 queueIsEmpty(RoboCommandUnit *commandUnit) {
    return commandUnit->commandCount == 0;
}

s32 queueIsAtStart(RoboCommandUnit *commandUnit) {
    s32 isOnFirstCommand;

    isOnFirstCommand = 0;
    if (commandUnit->commandCount > 0) {
        isOnFirstCommand = commandUnit->commandIndex == 0;
    }
    return isOnFirstCommand;
}

s32 queueIsExhausted(RoboCommandUnit *commandUnit)
{
  return commandUnit->commandCount <= commandUnit->commandIndex;
}

s32 roboCommandSubmitQueued();

void queueRunPrimaryAction(void) {
    extern s32 func_00AB5B08(s32, s32 *, s32);
    roboCommandSubmitQueued();
    func_00AB5B08(2, &queueSourceFile, 0xF2);
}

s32 roboCommandSubmitLookup();

void queueRunSecondaryAction(void) {
    extern s32 func_00AB5B08(s32, s32 *, s32);
    roboCommandSubmitLookup();
    func_00AB5B08(2, &queueSourceFile, 0xFA);
}

void queueAdvance(RoboCommandUnit *commandUnit);
void *roboBattleUnitGet(u32 unitId, void *file, s32 line);
void roboActorModeStart(void *, s32);
void func_00AAB820(s32, void *, void *, s32 *, s32);
void publishEntryRecords(void);
void seStartDefaultVoice(s32);

void queueCommitWithMode(RoboCommandUnit *commandUnit, s32 commitMode) {
    extern void func_00AB5B08(s32, s32 *, s32);
    void *primaryUnit;
    primaryUnit = roboBattleUnitGet(commandUnit->primarySlot, &queueSourceFile, 0x100);
    func_00AAB820(0x16, commandUnit, primaryUnit, &queueSourceFile, 0x100);
    seStartDefaultVoice(0x24);
    roboActorModeStart(commandUnit, commitMode);
    publishEntryRecords();
    func_00AB5B08(3, &queueSourceFile, 0x108);
    queueAdvance(commandUnit);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AAC500);

void queueSkipCurrent(RoboCommandUnit *commandUnit) {
    extern s32 func_00AB5B08(s32, s32 *, s32);
    func_00AB5B08(5, &queueSourceFile, 0x123);
    queueAdvance(commandUnit);
}

s32 ov02AdjustRollDown(s32, s32);
s32 roboCommandSubmitSlot(void *);

void queueRunTertiaryAction(RoboCommandUnit *commandUnit) {
    extern s32 func_00AB5B08(s32, s32 *, s32);
    ov02AdjustRollDown(1, commandUnit->commandTargetIndex);
    roboCommandSubmitSlot(commandUnit);
    func_00AB5B08(2, &queueSourceFile, 0x131);
}

void battleActorSetDispatchMode(void *, s32);

void queueCommitAndActivate(RoboCommandUnit *commandUnit) {
    extern void func_00AB5B08(s32, s32 *, s32);
    void *primaryUnit;
    primaryUnit = roboBattleUnitGet(commandUnit->primarySlot, &queueSourceFile, 0x137);
    func_00AAB820(0x1B, commandUnit, primaryUnit, &queueSourceFile, 0x137);
    battleActorSetDispatchMode(commandUnit, 1);
    func_00AB5B08(0xC, &queueSourceFile, 0x13B);
    queueAdvance(commandUnit);
}

s32 queueIsValidSlot(s32 commandIndex, u32 slotNumber) {
    s32 isValid;

    if ((commandIndex >= 0x12) || (isValid = 1, ((slotNumber < 8U) == 0))) {
        isValid = 0;
    }
    return isValid;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AAC6C0);

extern s32 queueSlotValueIndices[];

s16 queueGetSlotValue(RoboCommandUnit *commandUnit, s32 slotId) {
    RoboMechPart *mechPart;
    RoboPartParameters *partParameters;
    u32 parameterIndex;

    parameterIndex = slotId - 6;
    if (!(parameterIndex < 6U)) {
        return 0;
    }
    mechPart = commandUnit->mechPart;
    partParameters = 0;
    if (mechPart != 0) {
        partParameters = mechPart->parameters;
    }
    return partParameters->parameterValues[queueSlotValueIndices[parameterIndex]];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AAC730);

s32 queueGetSlotCount(void) {
    return 6;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu081", func_00AAC848);
