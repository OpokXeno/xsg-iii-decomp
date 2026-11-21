#include "common.h"

typedef int s32;
void textSetAvailable(void);

s32 cameraArmPendingFlag(void) {
    textSetAvailable();
    return 1;
}

extern unsigned char gBcamNativeMethods[];
extern char gBcamNativeOwnerName[];
s32 sbNativeOwnerAdd(const char *name, void *methods);

void cameraRegisterBcamNatives(void) {
    sbNativeOwnerAdd(gBcamNativeOwnerName, gBcamNativeMethods);
}

extern s32 gBattleRecordBufferBaseStorage;

s32 cameraGetBaseValue(void) {
    return gBattleRecordBufferBaseStorage;
}
