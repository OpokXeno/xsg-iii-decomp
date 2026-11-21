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
