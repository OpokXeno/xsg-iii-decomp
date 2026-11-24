#include "common.h"

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;

/* The xgl camera fields this file touches (the full struct is in main/xgl_2.h). */
struct XglCamera {
    unsigned int active;
    unsigned int cameraState;
};

/* Battle mode block at 0x3E0170: the mode byte, then the two actor slots set by battleSetActorSlots. */
typedef struct BattleModeState {
    s8 battleMode;                /* +0x00: 0, 1 or 2 (battleEnterModeZero/0C130/0C180) */
    s8 firstActorSlot;               /* +0x01 */
    s8 secondActorSlot;               /* +0x02 */
    u8 unk_03[3];
} BattleModeState;

extern BattleModeState gBattleModeActors;
extern s8 D_003E0173[];
extern s8 D_003E0174[];
extern s8 gBattlePendingFlag[];
extern s8 gBattleRecord[0xA0];
extern s32 D_003E3350[];

void *memset(void *, s32, s32);

INCLUDE_ASM("asm/main/nonmatchings/tu237", func_0025BF88);

s32 func_00203048(s32, s32, s32, s32);

s32 battleActorMatrixLoad(s32 outMatrix, s32 actor, s32 pose) {
    return func_00203048(actor, pose, outMatrix, 1) != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu237", func_0025C018);

s32 func_A86D98();

s32 battleCheckModeZero(void) {
    if (gBattleModeActors.battleMode == 0) {
        return func_A86D98();
    }
    return 0;
}

s32 func_AA1710();

s32 battleCheckModeOne(s32 value) {
    if (gBattleModeActors.battleMode == 1) {
        return func_AA1710(value, 1);
    }
    return 0;
}

void battleResetState(void);

void battleEnterModeZero(void) {
    memset(&gBattleModeActors, 0, 6);
    memset(gBattleRecord, 0, 0xA0);
    gBattleModeActors.battleMode = 0;
    battleResetState();
}

void battleEnterModeOne(void) {
    memset(&gBattleModeActors, 0, 6);
    memset(gBattleRecord, 0, 0xA0);
    gBattleModeActors.battleMode = 1;
    battleResetState();
}

void battleEnterModeTwo(void) {
    memset(&gBattleModeActors, 0, 6);
    memset(gBattleRecord, 0, 0xA0);
    gBattleModeActors.battleMode = 2;
    battleResetState();
}

s32 battleResultReset();
s32 actorSlotsInit();
s32 func_0025CA10();
s32 battleRecordReset();
s32 textCommandStateReset();
s32 func_0025F3C0();
s32 cameraPendingClear();
s32 cameraSlotsClear();
s32 battleClearFlag();
struct XglCamera *xglStudioGetCamera2(s32);

/* Common reset of the battle state, called by the battleEnterModeZero/0C130/0C180 entries. */
void battleResetState(void) {
    D_003E0174[0] = -1;
    xglStudioGetCamera2(0)->cameraState = 4;
    func_0025F3C0();
    battleResultReset();
    func_0025CA10();
    cameraSlotsClear();
    battleClearFlag();
    cameraPendingClear();
    battleRecordReset();
    textCommandStateReset();
    actorSlotsInit();
}

INCLUDE_ASM("asm/main/nonmatchings/tu237", func_0025C240);

void battleSetPendingFlag(s8 isPending) {
    gBattlePendingFlag[0] = isPending;
}

void actorSlotsClearFlags();

void battleClearEntryFlags(void) {
    actorSlotsClearFlags();
}

void actorSlotsSetReady();
void textStateSuspend();
void textStateReset();

void battleSetPhase(s32 nextBattlePhase) {
    D_003E0173[0] = nextBattlePhase;
    actorSlotsSetReady();
    if (nextBattlePhase != 1) {
        return;
    }
    textStateSuspend();
    textStateReset();
}

void battleSetActorSlots(s8 firstActorSlot, s8 secondActorSlot) {
    gBattleModeActors.firstActorSlot = firstActorSlot;
    gBattleModeActors.secondActorSlot = secondActorSlot;
}

void *battleGetRecord(void) {
    return gBattleRecord;
}

INCLUDE_ASM("asm/main/nonmatchings/tu237", func_0025C368);

s32 func_0025C408(s32, s32);

void battleStartFromPair(void) {
    func_0025C408(D_003E3350[0], D_003E3350[1]);
}
