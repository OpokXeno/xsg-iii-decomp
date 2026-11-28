#include "common.h"
#include "shared.h"

#include "main/tu105.h"

typedef int s32;
void textSetAvailable(void);

int vmOpCallWithTwoArgs();
int bcamStart();
int bcamPos();
int bcamLook();
int bcamCvr();
int bcamRotZ();
int bcamWait();
int bcamOver();
int bcamOvercam();
int bcamDef();

s32 cameraArmPendingFlag(void) {
    textSetAvailable();
    return 1;
}

SbNativeMethod gBcamNativeMethods[12] = {
    { "AB", vmOpCallWithTwoArgs, { 2, 0, 0, 0 } },
    { "Start", bcamStart, { 1, 0, 0, 0 } },
    { "Pos", bcamPos, { 4, 0, 0, 0 } },
    { "Look", bcamLook, { 4, 0, 0, 0 } },
    { "Cvr", bcamCvr, { 2, 0, 0, 0 } },
    { "RotZ", bcamRotZ, { 1, 0, 0, 0 } },
    { "Wait", bcamWait, { 1, 0, 0, 0 } },
    { "Over", bcamOver, { 6, 0, 0, 0 } },
    { "Overcam", bcamOvercam, { 0, 0, 0, 0 } },
    { "Def", bcamDef, { 2, 0, 0, 0 } },
    { "SoroMode", cameraArmPendingFlag, { 0, 0, 0, 0 } },
    { 0, 0, { 0, 0, 0, 0 } },
};

s32 sbNativeOwnerAdd(const char *name, SbNativeMethod *methods);

void cameraRegisterBcamNatives(void) {
    sbNativeOwnerAdd("bcam", gBcamNativeMethods);
}

extern s32 gBattleRecordBufferBaseStorage[];

s32 cameraGetBaseValue(void) {
    return gBattleRecordBufferBaseStorage[0];
}
