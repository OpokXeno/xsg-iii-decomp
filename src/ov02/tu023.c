/*
 * OV02 original TU 23: 0x00a8abb0..0x00a8ac10 (3 functions)
 */
#include "common.h"

typedef int s32;
s32 sceneStateClear();

void ov02ResetEventFlags(void) {
    sceneStateClear();
}

s32 sceneStateSetup();

void ov02SetEventFlag(s32 eventFlagId) {
    if (eventFlagId != 0) {
        sceneStateSetup(eventFlagId, 0, 0);
    }
}

void ov02ResetEventFlagsNow(void) {
    sceneStateClear();
}
