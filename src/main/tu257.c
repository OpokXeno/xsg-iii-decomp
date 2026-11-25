#include "common.h"
#include "shared.h"

#include "main/tu105.h"

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

int scriptCameraSetSlot();
int scriptCameraSetSlotById();
int scriptObjectActivate();
int scriptObjectClear();
int scriptObjectSetIndex();
int scriptObjectSetCount();
int func_00263368();
int func_00263478();
int func_00263538();
int func_00263648();
int func_002636A8();
int func_00263708();
int func_00263768();
int func_002637C8();
int func_002638D8();
int func_00263A30();
int func_00263A90();
int sbNativeObjectSetStateByte();
int func_00263B18();
int func_00263C30();
int sbNativeCameraLoadResource();
int func_00263D70();
int func_00263DD8();

SbNativeMethod gCameraNativeTable[28] = {
    { "DefMov", scriptCameraSetSlot, { 2, 0, 0, 0 } },
    { "Mov", scriptCameraSetSlotById, { 4, 0, 0, 0 } },
    { "Start", scriptObjectActivate, { 2, 0, 0, 0 } },
    { "Exit", scriptObjectClear, { 0, 0, 0, 0 } },
    { "Line", scriptObjectSetIndex, { 1, 0, 0, 0 } },
    { "CamID", scriptObjectSetCount, { 1, 0, 0, 0 } },
    { "Pos", func_00263368, { 3, 0, 0, 0 } },
    { "AngYZ", func_00263478, { 2, 0, 0, 0 } },
    { "AngXYZ", func_00263538, { 3, 0, 0, 0 } },
    { "Len", func_00263648, { 1, 0, 0, 0 } },
    { "LockX", func_002636A8, { 1, 0, 0, 0 } },
    { "LockY", func_00263708, { 1, 0, 0, 0 } },
    { "LockZ", func_00263768, { 1, 0, 0, 0 } },
    { "Act", func_002637C8, { 3, 0, 0, 0 } },
    { "Limit", func_002638D8, { 4, 0, 0, 0 } },
    { "Pers", func_00263A30, { 1, 0, 0, 0 } },
    { "Front", func_00263A90, { 1, 0, 0, 0 } },
    { "Mode", sbNativeObjectSetStateByte, { 1, 0, 0, 0 } },
    { "FocusA", func_00263B18, { 5, 0, 0, 0 } },
    { "FocusB", func_00263C30, { 5, 0, 0, 0 } },
    { "Chgid", sbNativeCameraLoadResource, { 1, 0, 0, 0 } },
    { "RotZ", func_00263D70, { 1, 0, 0, 0 } },
    { "Cfcam", func_00263DD8, { 0, 0, 0, 0 } },
    { "MovSignal", cameraSlotReset, { 0, 0, 0, 0 } },
    { "MovBlur", cameraStateIsReady, { 2, 0, 0, 0 } },
    { "Focus", cameraStateIsDone, { 0, 0, 0, 0 } },
    { 0, 0, { 0, 0, 0, 0 } },
    { 0, 0, { 0, 0, 0, 0 } },
};

void cameraRegisterNativeOwner(void) {
    sbNativeOwnerAdd("cam", gCameraNativeTable);
}
