/*
 * OV02 original TU 77: 0x00aa7010..0x00aa7890 (11 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/robo_attackseq", func_00AA7010);

typedef int s32;

/* Attack-sequence state at gRoboAttackSeq (0xE8 bytes). */
typedef struct RoboAttackSeqState {
    s32 attackPhase;                  /* 0x00: 1 while the callback runs in roboAttackSeqUpdate */
    void (*callback)(void);     /* 0x04 */
    unsigned char pad_08[0x54];
    s32 targetId;               /* 0x5C */
    unsigned char pad_60[0x88];
} RoboAttackSeqState;

extern RoboAttackSeqState gRoboAttackSeq;

RoboAttackSeqState *roboAttackSeqGetState(void) {
    return &gRoboAttackSeq;
}

RoboAttackSeqState *roboAttackSeqGetState(void);
void *memset(void *destination, s32 value, s32 size);

void roboAttackSeqClear(void) {
    RoboAttackSeqState *attackSeq;
    attackSeq = roboAttackSeqGetState();
    memset(attackSeq, 0, sizeof(RoboAttackSeqState));
}

extern char gRoboAttackSeqFileName[];
extern char gRoboAttackSeqCallbackMessage[];
void assertFailed(const char *message, const char *file, int line);
#include "ov02/robo_battle.h"
s32 roboCurrentIsPlayer(s32);
s32 battleActorHasUnitHandle(s32);
void func_00AA6148(s32);
void func_00AA6348(s32);
s32 func_00AA6F08(s32);
void robotEntryUpdateStep(s32);

void roboAttackSeqUpdate(void) {
    s32 targetId;
    s32 robotIndex;
    s32 secondRobotIndex;
    s32 flag;
    RoboAttackSeqState *attackSeq;
    s32 robotUnit;
    s32 secondRobotUnit;

    flag = 0;
    attackSeq = roboAttackSeqGetState();
    robotIndex = 0;
    do {
        robotUnit = roboBattleUnitGet(robotIndex, gRoboAttackSeqFileName, 0x27);
        func_00AA6148(robotUnit);
        robotIndex += 1;
    } while (robotIndex < 0xD);
    if (attackSeq->attackPhase == 1) {
        if (attackSeq->callback == 0) {
            assertFailed(gRoboAttackSeqCallbackMessage, gRoboAttackSeqFileName, 0x2B);
        }
        attackSeq->callback();
        targetId = attackSeq->targetId;
        if ((battleActorHasUnitHandle(targetId) != 0) && (roboCurrentIsPlayer(targetId) != 0)) {
            flag = func_00AA6F08(targetId) == 0;
        }
    }
    robotEntryUpdateStep(flag);
    secondRobotIndex = 0;
    do {
        secondRobotUnit = roboBattleUnitGet(secondRobotIndex, gRoboAttackSeqFileName, 0x3F);
        func_00AA6348(secondRobotUnit);
        secondRobotIndex += 1;
    } while (secondRobotIndex < 0xD);
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_attackseq", func_00AA72A8);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_attackseq", func_00AA7348);

s32 queueIsExhausted(s32);
s32 func_00AB5B08(s32 kind, const char *file, s32 line);

void roboAttackSeqSetState(s32 targetId, s32 flag) {
    RoboAttackSeqState *attackSeq;

    attackSeq = roboAttackSeqGetState();
    if ((queueIsExhausted(targetId) != 0) || (flag != 0)) {
        attackSeq->attackPhase = 3;
        func_00AB5B08(8, gRoboAttackSeqFileName, 0x9F);
        return;
    }
    attackSeq->attackPhase = 2;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_attackseq", func_00AA74F0);

s32 roboAttackSeqIsActive(void) {
    return roboAttackSeqGetState()->attackPhase == 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_attackseq", func_00AA7708);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_attackseq", func_00AA77C8);
