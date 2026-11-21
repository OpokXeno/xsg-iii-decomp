#include "common.h"

typedef signed char s8;
typedef int s32;
s32 sceneLinkStateSetEnabled();
s32 sceneLinkSetFlag44(s8);

void setValueAndFlush(s8 sceneLinkFlag) {
    sceneLinkSetFlag44(sceneLinkFlag);
    sceneLinkStateSetEnabled();
}

s32 sceneLinkStateClearCounters();

void flushPending(void) {
    sceneLinkStateClearCounters();
}

extern s8 gSceneSharedMode;

void sceneSharedModeSet(s8 sceneSharedMode) {
    gSceneSharedMode = sceneSharedMode;
}
