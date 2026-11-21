/*
 * OV02 original TU 95: 0x00ab4ab8..0x00ab6298 (28 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;

/* Battle flow block at gRoboBattleFlowState (0xB0 bytes). */
typedef struct RoboBattleFlowState {
    u8 unk_00[0x14];
    s32 flowPhase;                  /* 0x14 */
    s32 slots[3];               /* 0x18 */
    s32 active;                 /* 0x24 */
    u8 unk_28[0x88];
} RoboBattleFlowState;

extern char gRoboBattleFlowSourceFile[];
extern RoboBattleFlowState gRoboBattleFlowState;

void roboBattleFlowInit(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    extern void *memset(void *dest, s32 value, s32 size);
    memset(&gRoboBattleFlowState, 0, 0xB0);
    gRoboBattleFlowState.active = 1;
    func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x78);
    gRoboBattleFlowState.flowPhase = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB4B10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB4CD0);

void roboBattleFlowStubEnter(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB4D98);

s32 roboAttackSeqIsActive();

void roboBattleFlowTraceAttackIdle(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    if (roboAttackSeqIsActive() == 0) {
        func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x14D);
    }
}

extern char gUnitHandleAssertMessage[];
void assertFailed(const char *message, const char *file, int line);
#include "ov02/robo_battle.h"
s32 func_00AA1928();
s32 battleActorHasUnitHandle(s32);
s32 battleActorIsStateActive(s32);
s32 func_00AABE48(s32 currentBattleUnit);
s32 func_00AAD490(s32 currentBattleUnit);

void roboBattleFlowFinishUnit(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    s32 currentBattleUnit;
    s32 currentUnitId;

    currentUnitId = func_00AA1928();
    currentBattleUnit = roboBattleUnitGet(currentUnitId, &gRoboBattleFlowSourceFile, 0x155);
    if (battleActorHasUnitHandle(currentBattleUnit) == 0) {
        assertFailed(&gUnitHandleAssertMessage, &gRoboBattleFlowSourceFile, 0x157);
    }
    if (battleActorIsStateActive(currentBattleUnit) == 0) {
        return;
    }
    func_00AABE48(currentBattleUnit);
    func_00AAD490(currentBattleUnit);
    func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x15F);
}

void roboBattleFlowStubLeave(void) {

}

extern s32 gMovieScriptState;

void roboBattleFlowSetScriptIfActive(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    s32 currentBattleUnit;
    s32 currentUnitId;

    currentUnitId = func_00AA1928();
    currentBattleUnit = roboBattleUnitGet(currentUnitId, &gRoboBattleFlowSourceFile, 0x16D);
    if (battleActorHasUnitHandle(currentBattleUnit) == 0) {
        assertFailed(&gUnitHandleAssertMessage, &gRoboBattleFlowSourceFile, 0x16F);
    }
    if (battleActorIsStateActive(currentBattleUnit) != 0) {
        gMovieScriptState = 1;
        func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x173);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5270);

s32 entryQueueHasItems();

void roboBattleFlowSetScriptOnIdle(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    if (entryQueueHasItems() == 0) {
        gMovieScriptState = 1;
        func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x1AF);
    }
}

void func_00AA7348();

void roboBattleFlowRunSeqSetScript(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    func_00AA7348();
    gMovieScriptState = 1;
    func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x1BA);
}

s32 func_00A9E4D8();

void roboBattleFlowSetScriptOnFlag(void) {
    extern void func_00AB5B08(s32 level, const char *file, s32 line);
    if (func_00A9E4D8() != 0) {
        gMovieScriptState = 1;
        func_00AB5B08(0, &gRoboBattleFlowSourceFile, 0x1C3);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB54C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5708);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB58D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5A00);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5B08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5BA8);

s32 padGetPortData(s32 port);
s32 padPressedAll(s32 padPortData, s32 mask);

s32 roboBattleFlowSkipRequested(void) {
    s32 padPortData;
    s32 skipRequested;

    padPortData = padGetPortData(0);
    if ((padPressedAll(padPortData, 0x20) != 0) || (padPressedAll(padPortData, 0x40) != 0) || (padPressedAll(padPortData, 0x100) != 0) || (skipRequested = padPressedAll(padPortData, 0x800), (skipRequested != 0))) {
        skipRequested = 1;
    }
    return skipRequested;
}

void roboBattleFlowResetSlots(void) {
    extern void *memset(void *dest, s32 value, s32 size);
    RoboBattleFlowState *flow = &gRoboBattleFlowState;
    memset(flow->slots, 0, 0xC);
    flow->flowPhase = 2;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5D10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5DE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB5EA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu095", func_00AB6010);

void roboBattleEndedSet();
void func_00AA0DC8(s32);
s32 RgDispModelIsBusy();

void roboBattleFlowFinishAllUnits(void) {
    s32 unitId;
    s32 battleUnit;

    if (RgDispModelIsBusy() == 0) {
        for (unitId = 0; unitId < 0xD; unitId++) {
            battleUnit = roboBattleUnitGet(unitId, &gRoboBattleFlowSourceFile, 0x3D2);
            func_00AA0DC8(battleUnit);
        }
        roboBattleEndedSet();
        gMovieScriptState = 7;
    }
}

s32 scGetMovieScriptState(void) {
    return gMovieScriptState;
}

s32 scGetMovieScriptState(void);

s32 scWaitParseMovieScript(void) {
    extern s32 scGetMovieScriptState();
    return scGetMovieScriptState() == 0;
}
