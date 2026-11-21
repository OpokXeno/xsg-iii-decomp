/*
 * OV02 original TU 78: 0x00aa7890..0x00aab628 (43 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA7890);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;

/* Unit slot block passed to the robo command submitters (fields at their original offsets). */
typedef struct RoboCommandUnit {
    u8 pad_00[0xE4];
    s32 primarySlot;                    /* 0xE4: slot handed to roboBattleUnitGet */
    u8 pad_E8[0x1DC];
    s32 statIndex;                    /* 0x2C4 */
    s32 secondarySlot;                  /* 0x2C8 */
} RoboCommandUnit;

/* Attack sequence state (the global block behind roboAttackSeqGetState). */
typedef struct RoboAttackOwner {
    u8 pad_00[0x2B4];
    s32 soundEffectHandle;                       /* 0x2B4: sound effect entry handed to sefActorFlagsGet */
} RoboAttackOwner;

typedef struct RoboAttackLink {
    u8 pad_00[0xA0];
    RoboAttackOwner *owner;             /* 0xA0 */
} RoboAttackLink;

typedef struct RoboAttackSeq {
    u8 pad_00[0x0C];
    s32 idleFrames;                     /* 0x0C: counts up to 0x14 before the start poll */
    u8 pad_10[0x14];
    s32 startRequested;                 /* 0x24 */
    u8 pad_28[0x18];
    s32 pendingHandle;                  /* 0x40 */
    s32 pendingCount;                   /* 0x44 */
    u8 pad_48[0x4];
    RoboAttackLink *attackLink;               /* 0x4C */
    u8 pad_50[0x4];
    s32 timingValue;                     /* 0x54 */
    s32 timingLimit;                     /* 0x58 */
    s32 targetId;                       /* 0x5C */
} RoboAttackSeq;

/* Entry table of the object returned by battleRecordBlockGet (fields at their original offsets). */
typedef struct RoboActorSlot {
    s32 actorHandle;
    u8 pad_04[0x4C];
} RoboActorSlot;

typedef struct RoboActorTable {
    u8 pad_00[0x10];
    s32 mainActorRecord;                     /* 0x10 */
    u8 pad_14[0x17C];
    s32 actorCount;                     /* 0x190 */
    u8 pad_194[0xC];
    RoboActorSlot actors[0];               /* 0x1A0: stride 0x50, read through a pointer walk */
} RoboActorTable;

void func_001ECE98();
s32 roboParamTouchEntry();
s32 queueGetCurrent();
s32 queueIsAtStart();
s32 func_00AAC730();
void func_00AAC848();
RoboAttackSeq *roboAttackSeqGetState(void);

extern char gRoboSourceFile[];
extern char gRoboActionMissingMessage[];
extern char gRoboStatMissingMessage[];
extern char gRoboAttackTargetMessage[];

void roboCommandSubmitQueued(RoboCommandUnit *commandUnit) {
    extern s32 roboBattleUnitGet();
    extern void func_00AAAAA0();
    extern void func_00AAAF48();
    extern void queueAdvance();
    s32 commandWords[8];
    s32 commandHandle;
    s32 commandIndex;
    s32 queueAtStart;
    s32 unitResource;
    s32 queueIndex;
    s32 queueIndexAgain;

    unitResource = roboBattleUnitGet(commandUnit->primarySlot, gRoboSourceFile, 0xA7);
    queueIndex = queueGetCurrent(commandUnit);
    commandHandle = func_00AAC730(commandUnit, queueIndex);
    queueAtStart = queueIsAtStart(commandUnit);
    commandIndex = roboParamTouchEntry(commandHandle);
    queueIndexAgain = queueGetCurrent(commandUnit);
    func_00AAC848(commandUnit, queueIndexAgain, &commandWords[0]);
    queueAdvance(commandUnit);
    func_00AAAAA0(1, commandUnit, unitResource, commandIndex, commandWords[3], 0, 0, queueAtStart, 0);
    func_001ECE98();
    func_00AAAF48();
}

void assertFailed(const char *message, const char *file, int line);
s32 ov02ActionEntryGet(s32);
s32 func_00A84B78(s32, s32);
s32 roboKeyRecordStat(void *, s32);

void roboCommandSubmitLookup(RoboCommandUnit *commandUnit, s32 paramIndex) {
    extern s32 roboBattleUnitGet(s32, const char *, s32);
    extern s32 func_00AAAAA0(s32, void *, s32, s32, s32, s32, s32, s32, s32);
    extern s32 queueAdvance(void *);
    s32 submitExtra;
    s32 unitResource;
    s32 paramObject;
    s32 actionHandle;

    unitResource = roboBattleUnitGet(commandUnit->primarySlot, gRoboSourceFile, 0xBC);
    paramObject = roboKeyRecordStat(commandUnit, paramIndex);
    submitExtra = func_00A84B78(paramObject, 0);
    actionHandle = ov02ActionEntryGet(paramObject);
    if (actionHandle == 0) {
        assertFailed(gRoboActionMissingMessage, gRoboSourceFile, 0xC1);
    }
    queueAdvance(commandUnit);
    func_00AAAAA0(2, commandUnit, unitResource, actionHandle, submitExtra, 0, paramObject, 0, 0);
}

u8 *ov02BattleSlotGet(s32 index);
s32 func_00A84BF0(s32, s32);

void roboCommandSubmitSlot(RoboCommandUnit *commandUnit) {
    extern s32 roboBattleUnitGet(s32, const char *, s32);
    extern s32 func_00AAAAA0(s32, void *, s32, void *, s32, s32, s32, s32, s32);
    extern s32 func_00AAAF48();
    extern s32 queueAdvance(void *);
    u8 *statEntry;
    s32 submitExtra;
    s32 unitResource;

    unitResource = roboBattleUnitGet(commandUnit->secondarySlot, gRoboSourceFile, 0xCB);
    statEntry = ov02BattleSlotGet(commandUnit->statIndex);
    submitExtra = func_00A84BF0(commandUnit->statIndex, 0);
    if (statEntry == 0) {
        assertFailed(gRoboStatMissingMessage, gRoboSourceFile, 0xCF);
    }
    queueAdvance(commandUnit);
    func_00AAAAA0(3, commandUnit, unitResource, statEntry, submitExtra, commandUnit->statIndex, 0, 0, 0);
    func_00AAAF48();
}

void roboEmptyCallback(void) {
}

s32 roboStatusNone(void) {
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA7C28);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA7CA0);

s32 func_00A81B48(s32);

s32 roboStateIsPrimaryReady(void) {
    extern s32 battleRecordBlockGet();
    s32 battleRecordBlock;
    battleRecordBlock = battleRecordBlockGet();
    return func_00A81B48(battleRecordBlock) != 0;
}

s32 func_00A81E40(s32);

s32 roboStateIsSecondaryReady(void) {
    extern s32 battleRecordBlockGet();
    s32 battleRecordBlock;
    battleRecordBlock = battleRecordBlockGet();
    return func_00A81E40(battleRecordBlock) != 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA7DC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA7ED0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA80F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8180);

s32 func_00A9FED8(s32);
s32 func_00AAB1C0(s32, s32);

void roboAttackSeqApplyParams(void) {
    RoboAttackSeq *attackSeq;
    s32 timingValue;
    s32 timingLimit;

    attackSeq = roboAttackSeqGetState();
    timingValue = attackSeq->timingValue;
    timingLimit = func_00A9FED8(attackSeq->timingLimit);
    func_00AAB1C0(timingValue, timingLimit);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8210);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8348);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8390);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA84C8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8620);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA88C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8C40);

s32 roboSetActive(s32);

void roboBattleClearMode(void) {
    roboSetActive(0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8D28);

s32 func_00AABE48(s32);

void roboAttackSeqResetPending(void) {
    RoboAttackSeq *attackSeq;

    attackSeq = roboAttackSeqGetState();
    func_00AABE48(attackSeq->targetId);
    attackSeq->pendingCount = 0;
    attackSeq->pendingHandle = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA8E30);

s32 func_00AA1370(s32);
s32 battleActorRequestSlotState(s32);
s32 func_00AA5B20(s32, s32, s32, s32);

s32 roboActorTableRelease(void) {
    extern RoboActorTable *battleRecordBlockGet();
    RoboActorTable *actorTable;
    s32 mainActorHandle;
    RoboActorSlot *actorSlot;
    s32 actorHandle;
    s32 slotActorHandle;
    s32 loopCount;
    s32 actorCount;

    loopCount = 0;
    actorTable = battleRecordBlockGet();
    mainActorHandle = func_00AA1370(actorTable->mainActorRecord);
    func_00AA5B20(mainActorHandle, 1, 0, 0);
    battleActorRequestSlotState(mainActorHandle);
    actorCount = actorTable->actorCount;
    if (actorCount > 0) {
        actorSlot = actorTable->actors;
        do {
            slotActorHandle = actorSlot->actorHandle;
            actorSlot += 1;
            loopCount += 1;
            actorHandle = func_00AA1370(slotActorHandle);
            func_00AA5B20(actorHandle, 1, 0, 0);
            battleActorRequestSlotState(actorHandle);
            actorCount = actorTable->actorCount;
            actorCount = loopCount < actorCount;
        } while (actorCount != 0);
    }
    return actorCount;
}

void roboAttackSeqPoll(void) {
    roboAttackSeqGetState();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA9608);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA98D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AA9BF0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAA0F8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAA538);

s32 padGetPortData(s32);
s32 padPressedAll(s32, s32);
s32 sefActorFlagsGet(s32);
void seVoiceCursorSetEnable(s32);
void roboAttackSeqSetState(s32, s32);
void func_00AAAA00(s32, s32, const char *, s32);

void roboAttackSeqUpdateIdle(void) {
    s32 padPort;
    s32 idleFrame;
    s32 soundEffectHandle;
    RoboAttackSeq *attackSeq;
    RoboAttackOwner *attackOwner;

    attackSeq = roboAttackSeqGetState();
    idleFrame = attackSeq->idleFrames;
    if (idleFrame == 0) {
        attackOwner = attackSeq->attackLink->owner;
        soundEffectHandle = 0;
        if (attackOwner != 0) {
            soundEffectHandle = attackOwner->soundEffectHandle;
        }
        if (sefActorFlagsGet(soundEffectHandle) & 2) {
            seVoiceCursorSetEnable(1);
            attackSeq->idleFrames += 1;
        }
    } else if (idleFrame < 0x14) {
        attackSeq->idleFrames = idleFrame + 1;
    } else {
        padPort = padGetPortData(0);
        if ((padPressedAll(padPort, 0x20) != 0) || (padPressedAll(padPort, 0x40) != 0)) {
            attackSeq->startRequested = 1;
        }
    }
    if (attackSeq->startRequested == 0) {
        return;
    }
    seVoiceCursorSetEnable(0);
    roboAttackSeqSetState(attackSeq->targetId, 1);
    func_00AAAA00(0, 0, gRoboSourceFile, 0x6E4);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAA690);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAAA00);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAAAA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAAF48);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAB060);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAB1C0);

s32 battleActorReset(s32);
s32 battleActorStateReset(s32, s32);

void roboAttackSeqStart(s32 targetId) {
    if (targetId == 0) {
        assertFailed(gRoboAttackTargetMessage, gRoboSourceFile, 0x856);
    }
    battleActorReset(targetId);
    battleActorStateReset(targetId, 0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAB308);

INCLUDE_ASM("asm/nonmatchings/ov02/tu078", func_00AAB400);
