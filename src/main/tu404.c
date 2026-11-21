#include "common.h"

typedef int s32;
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
