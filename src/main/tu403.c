#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu403", func_002E63B0);

typedef int s32;
extern s32 gPendingCounterValue;
s32 counterSet(void);

void pendingValueCommit(s32 pendingCounterValue) {
    gPendingCounterValue = pendingCounterValue;
    counterSet();
}

s32 valueTableSet(void);

void slotEntryClear(void) {
    valueTableSet();
}
