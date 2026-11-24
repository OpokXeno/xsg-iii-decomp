#include "common.h"

typedef int s32;

s32 func_002BB450(s32);

s32 effectPoolSetTails(s32);

void stateApplyEnableLevel(s32 ownerHandle, s32 level) {
    effectPoolSetTails(level);
    func_002BB450(ownerHandle);
}

s32 func_002B9050();

s32 func_002BD230();

void stateResetSystems(void) {
    func_002B9050();
    func_002BD230();
}

s32 sceneCounterLimitReset();

void effectPoolResetCounters(void) {
    sceneCounterLimitReset();
}

INCLUDE_ASM("asm/main/nonmatchings/tu402", func_002E63B0);

extern s32 gPendingCounterValue[];

s32 counterSet(void);

void pendingValueCommit(s32 pendingCounterValue) {
    gPendingCounterValue[0] = pendingCounterValue;
    counterSet();
}

s32 valueTableSet(void);

void slotEntryClear(void) {
    valueTableSet();
}

s32 func_002BA060();

void recordListFlush(void) {
    func_002BA060();
}

s32 func_002C1E08();

void recordQueueSetup(void) {
    func_002C1E08();
}

s32 func_002BD2D0();

void recordSubmitPlain(s32 id, s32 submitValue, s32 submitFlags) {
    func_002BD2D0(id, submitValue, 0, submitFlags);
}

void recordSubmitRelease(s32 id, s32 submitValue, s32 submitFlags) {
    func_002BD2D0(id, submitValue, 2, submitFlags);
}

s32 func_002C1E28();

void recordQueueService(void) {
    func_002C1E28();
}

s32 func_002C1ED0();

void recordSubmitQueued(s32 id, s32 submitValue, s32 submitFlags) {
    func_002C1ED0(id, submitValue, 1, submitFlags);
}

s32 stateTailReset();

void seVoiceStateReset(void) {
    stateTailReset();
}

s32 func_002BC690();

void ownerModeRequest(void) {
    func_002BC690();
}
