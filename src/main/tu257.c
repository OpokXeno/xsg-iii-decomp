#include "common.h"

typedef int s32;
s32 objectMarkPending();

s32 cameraSlotReset(void) {
    objectMarkPending();
    return 1;
}

s32 cameraStateIsReady(void) {
    return 1;
}

s32 cameraStateIsDone(void) {
    return 1;
}

extern s32 gCameraNativeTable[];
extern s32 gCameraModuleName[];
s32 sbNativeOwnerAdd(s32 *, s32 *);

void cameraRegisterNativeOwner(void) {
    sbNativeOwnerAdd(gCameraModuleName, gCameraNativeTable);
}
