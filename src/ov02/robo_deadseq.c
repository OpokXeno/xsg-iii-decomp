/*
 * OV02 original TU 72: 0x00aa4a50..0x00aa52f8 (14 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;

/* Motion state that a robot target points to at 0x2B0. Bit 0x80 of motionFlags marks the motion as finished. */
typedef struct RoboMotionState {
    u8 unk_00[0x1A];
    u16 motionFlags;
} RoboMotionState;

/* Target object a robot follows: its motion state sits at 0x2B0. */
typedef struct RoboFollowedObject {
    u8 unk_00[0x2B0];
    RoboMotionState *motion;    /* 0x2B0 */
} RoboFollowedObject;

/* The robot record that every function of this file works on. Fields at the offsets the code uses. */
typedef struct RoboDeadseq {
    u8 unk_00[0xA0];
    RoboFollowedObject *followedObject; /* 0xA0 */
    u8 unk_A4[0x8];
    u32 unk_AC;                 /* 0xAC */
    u8 unk_B0[0xC];
    u32 flags_BC;               /* 0xBC */
    u8 unk_C0[0x190];
    s32 sequencePhase;          /* 0x250: 0 idle, 1..3 running, 3 done */
    s32 unk_254;
    s32 unk_258;
    s32 motionEntry;            /* 0x25C: copied from the table at roboMotionIdTable */
} RoboDeadseq;

void roboDeadseqSetMotion(RoboDeadseq *robo, s32 motionIndex);

void roboDeadseqSetFirstMotion(RoboDeadseq *robo) {
    roboDeadseqSetMotion(robo, 0);
}

s32 func_00A871E8(void *);
s32 roboActorModeEnd();

void roboDeadseqStart(RoboDeadseq *robo)
{
    RoboFollowedObject *followedObject;
    RoboMotionState *followedMotion;

    roboActorModeEnd();
    followedObject = robo->followedObject;
    robo->sequencePhase = 1;
    followedMotion = 0;
    if (followedObject != 0) {
        followedMotion = followedObject->motion;
    }
    if (followedMotion->motionFlags & 0x80) {
        roboDeadseqSetMotion(robo, 3);
        do { } while (0);
    } else if (robo->unk_AC < 0x12U) {
        roboDeadseqSetMotion(robo, 1);
    } else {
        roboDeadseqSetMotion(robo, 2);
    }
    func_00A871E8(robo->followedObject);
    robo->unk_254 = 0;
    robo->unk_258 = 0;
}

s32 roboDeadseqIsActive(RoboDeadseq *robo) {
    return robo->sequencePhase - 1 < 3U;
}

s32 roboDeadseqIsDone(RoboDeadseq *robo) {
    return robo->sequencePhase == 3;
}

void roboDeadseqReset(RoboDeadseq *robo) {
    robo->sequencePhase = 0;
}

s32 roboRemainingEnemiesGet();
s32 roboRemainingEnemiesSet(s32);
s32 func_00AA1300(s32);
s32 roboCurrentIsEnemy();
s32 roboOrderClearKey(s32);
void publishEntryRecords(void);
s32 battleSourceRecordRelease(s32);

void roboDeadseqReleaseSlot(s32 releasedIndex) {
    s32 enemyCount;
    s32 orderKey;
    if (roboCurrentIsEnemy() != 0) {
        enemyCount = roboRemainingEnemiesGet();
        roboRemainingEnemiesSet(enemyCount - 1);
    }
    orderKey = func_00AA1300(releasedIndex);
    roboOrderClearKey(orderKey);
    battleSourceRecordRelease(releasedIndex);
    publishEntryRecords();
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_deadseq", func_00AA4B88);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_deadseq", func_00AA4D00);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_deadseq", func_00AA4EB0);

void roboDeadseqNoop(void) {

}

void roboDeadseqFinish(RoboDeadseq *robo) {
    robo->sequencePhase = 3;
    robo->flags_BC |= 0x800;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_deadseq", func_00AA5198);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_deadseq", func_00AA5228);

extern s32 roboMotionIdTable[];

void roboDeadseqSetMotion(RoboDeadseq *robo, s32 motionIndex) {
    s32 motionId;

    motionId = roboMotionIdTable[motionIndex];
    robo->motionEntry = motionId;
}
