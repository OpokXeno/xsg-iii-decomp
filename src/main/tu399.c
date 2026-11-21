#include "common.h"

typedef int s32;
extern s32 gTimingTable[];

s32 timingTableGet(s32 timingIndex) {
    return gTimingTable[timingIndex];
}

struct TimingState;
extern struct TimingState *gTimingStateHandle[];

void timingStateSetHandle(struct TimingState *timingState) {
    gTimingStateHandle[0] = timingState;
}

/* The timing state that gTimingStateHandle points to; the fields are the last sampled total, the total before it and their difference. */
typedef struct TimingState {
    unsigned char unk_0000[0x44C];
    s32 sampledTotal;
    unsigned char unk_0450[0x8];
    s32 previousSampledTotal;
    s32 lastSampledTotal;
    s32 sampledDelta;
} TimingState;

INCLUDE_ASM("asm/main/nonmatchings/tu399", func_002E28C8);

INCLUDE_ASM("asm/main/nonmatchings/tu399", func_002E2C08);

INCLUDE_ASM("asm/main/nonmatchings/tu399", func_002E2E50);

INCLUDE_ASM("asm/main/nonmatchings/tu399", func_002E2EC0);

INCLUDE_ASM("asm/main/nonmatchings/tu399", func_002E32D0);

void timingStateUpdate(void)
{
    TimingState *timingState;
    s32 sampledTotal;
    s32 sampledDelta;
    s32 previousSampledTotal;
    s32 lastSampledTotal;

    timingState = gTimingStateHandle[0];
    sampledTotal = timingState->sampledTotal;
    sampledDelta = sampledTotal - timingState->lastSampledTotal;
    previousSampledTotal = sampledTotal - sampledDelta;
    timingState->sampledDelta = sampledDelta;
    timingState->previousSampledTotal = previousSampledTotal;
    lastSampledTotal = previousSampledTotal + sampledDelta;
    timingState->lastSampledTotal = lastSampledTotal;
}
