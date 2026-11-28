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
    s8 battleMode;      /* +0x00: 0, 1 or 2 (battleEnterModeZero/0C130/0C180) */
    s8 firstActorSlot;  /* +0x01 */
    s8 secondActorSlot; /* +0x02 */
    u8 nextSyncMode;    /* +0x03 (D_003E0173): copied to syncMode before cameraSyncMove */
    u8 syncMode;        /* +0x04 (D_003E0174) */
    s8 suspended;       /* +0x05: nonzero skips the battle update */
    u8 unk_06[10];
} BattleModeState;

extern BattleModeState gBattleModeActors;
extern s8 D_003E0173[];
extern s8 D_003E0174[];
static s8 gBattleRecord[0xA0];
extern s32 gCameraSourcePointers[];
void *memset(void *, s32, s32);

typedef struct BattleProbeRecord {
    float distance; /* +0x00 */
    s32 unk04;
    s32 *actorRef; /* +0x08 */
    s32 unk0C;
} BattleProbeRecord;

void func_A86CB8(BattleProbeRecord *probe, s32 actor);
s32 func_AA16D8(s32 actor);

/* Returns the value of the probe record of the actor in battle mode 0 (func_A86CB8 fills it), or the result of
 * func_AA16D8 in mode 1; 0 otherwise. Called by the battle actor code. */
s32 battleSlotActorGet(s32 actor) {
    BattleProbeRecord probe;

    if (gBattleModeActors.battleMode == 0) {
        func_A86CB8(&probe, actor);
        if (probe.actorRef != 0 && *probe.actorRef != 0) {
            return *probe.actorRef;
        }
        return 0;
    }
    if (gBattleModeActors.battleMode == 1) {
        return func_AA16D8(actor);
    }
    return 0;
}

s32 sefReadProbePoint(s32, s32, s32, s32);

s32 battleActorMatrixLoad(s32 outMatrix, s32 actor, s32 pose) {
    return sefReadProbePoint(actor, pose, outMatrix, 1) != 0;
}

void func_AA1738(s32 actor, float *distance, float *height);

/* Returns the distance of the slot probe (func_A86CB8) in battle mode 0, or the distance from func_AA1738 in
 * mode 1; 0.5 otherwise. tu236 stores it as the slot radius. */
float battleSlotRadiusGet(s32 actor) {
    BattleProbeRecord probe;
    float distance;
    float height;

    if (gBattleModeActors.battleMode == 0) {
        func_A86CB8(&probe, actor);
        return probe.distance;
    }
    if (gBattleModeActors.battleMode == 1) {
        func_AA1738(actor, &distance, &height);
        return distance;
    }
    return 0.5f;
}

s32 func_A86D98();

s32 battleCheckModeZero(void) {
    if (gBattleModeActors.battleMode == 0) {
        return func_A86D98();
    }
    return 0;
}

s32 func_AA1710();

s32 battleCheckModeOne(s32 actorSlotIndex) {
    if (gBattleModeActors.battleMode == 1) {
        return func_AA1710(actorSlotIndex, 1);
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
s32 textResetRecord();
s32 battleRecordReset();
s32 textCommandStateReset();
s32 cameraSlotsSelect();
s32 cameraPendingClear();
s32 cameraSlotsClear();
s32 battleClearFlag();
struct XglCamera *xglStudioGetCamera2(s32);

/* Common reset of the battle state, called by the battleEnterModeZero/0C130/0C180 entries. */
void battleResetState(void) {
    D_003E0174[0] = -1;
    xglStudioGetCamera2(0)->cameraState = 4;
    cameraSlotsSelect();
    battleResultReset();
    textResetRecord();
    cameraSlotsClear();
    battleClearFlag();
    cameraPendingClear();
    battleRecordReset();
    textCommandStateReset();
    actorSlotsInit();
}

s32 func_00265FA8();
s32 actorSlotsLoadAll();
s32 textStateResume();
s32 battleTextCommandUpdate();
void battlePhaseDispatch(void);
void cameraSlotsUpdate(void);
s32 cameraSyncMove();

/* Battle update step: does nothing while the battle block is suspended. Otherwise it runs the update callees in
 * order and copies nextSyncMode to syncMode before cameraSyncMove. */
void battleUpdateStep(void) {
    if (gBattleModeActors.suspended != 0) {
        return;
    }
    cameraSlotsSelect();
    func_00265FA8();
    actorSlotsLoadAll();
    textStateResume();
    battleTextCommandUpdate();
    battlePhaseDispatch();
    cameraSlotsUpdate();
    gBattleModeActors.syncMode =
        gBattleModeActors.nextSyncMode;
    cameraSyncMove();
}

void battleSetPendingFlag(s8 isPending) {
    D_003E0174[1] = isPending;
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

s32 func_00264E60();
s32 func_00267AD8(s8 *battleRecord);
s32 func_002665A8(s8 *battleRecord);

/* Dispatches the battle phase byte D_003E0173[0]: phases 1 and 2 run func_00264E60, phase 3 runs func_00267AD8
 * on gBattleRecord and phase 4 runs func_002665A8 (its result is unused). */
void battlePhaseDispatch(void) {
    switch (D_003E0173[0]) {
    case 1:
    case 2:
        func_00264E60();
        break;
    case 3:
        func_00267AD8(gBattleRecord);
        break;
    case 4:
        if (func_002665A8(gBattleRecord) != 0) {
            /* the finished step has no handler here (a compiled-out report) */
        }
        break;
    }
}

s32 func_0025C408(s32, s32);

void battleStartFromPair(void) {
    func_0025C408(gCameraSourcePointers[0], gCameraSourcePointers[1]);
}

BattleModeState gBattleModeActors = { 0 };
