#include "common.h"
#include "shared.h"

#include "main/tu105.h"

typedef int s32;
void textSetAvailable(void);

int vmOpCallWithTwoArgs();
int func_0025DBD8();
int func_0025DCE0();
int func_0025DE40();
int func_0025DFA0();
int func_0025E028();
int vmOpRecordAppend();
int func_0025E130();
int vmOpBattleStep();
int func_0025E340();

s32 cameraArmPendingFlag(void) {
    textSetAvailable();
    return 1;
}

SbNativeMethod gBcamNativeMethods[12] = {
    { "AB", vmOpCallWithTwoArgs, { 2, 0, 0, 0 } },
    { "Start", func_0025DBD8, { 1, 0, 0, 0 } },
    { "Pos", func_0025DCE0, { 4, 0, 0, 0 } },
    { "Look", func_0025DE40, { 4, 0, 0, 0 } },
    { "Cvr", func_0025DFA0, { 2, 0, 0, 0 } },
    { "RotZ", func_0025E028, { 1, 0, 0, 0 } },
    { "Wait", vmOpRecordAppend, { 1, 0, 0, 0 } },
    { "Over", func_0025E130, { 6, 0, 0, 0 } },
    { "Overcam", vmOpBattleStep, { 0, 0, 0, 0 } },
    { "Def", func_0025E340, { 2, 0, 0, 0 } },
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
