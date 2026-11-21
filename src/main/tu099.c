#include "common.h"

typedef int s32;
typedef signed char s8;
void sbNativeSlotsClear(void);
s32 func_001D1250();
s32 memset(s32 *, s32, s32);

void sbNativeOwnersReset(void) {
    extern s32 gSbNativeOwnerFlags;
    memset(&gSbNativeOwnerFlags, 0, 8);
    sbNativeSlotsClear();
    func_001D1250();
}

extern s32 gSbCfmapNativeTable;
extern s32 gSbDebugNativeTable;
extern s32 gSbMathNativeTable;
extern s32 gSbObjxNativeTable;
extern s32 gSbRuntimeNativeTable;
extern s32 gSbSoundNativeTable;
extern s32 gSbRuntimeModuleName;
extern s32 gSbObjxModuleName;
extern s32 gSbCfmapModuleName;
extern s32 gSbMathModuleName;
extern s32 gSbSoundModuleName;
extern s32 gSbDebugModuleName;
s32 sbNativeOwnerAdd(s32 *, s32 *);

void sbNativeOwnersInit(void) {
    sbNativeSlotsClear();
    func_001D1250();
    sbNativeOwnerAdd(&gSbDebugModuleName, &gSbDebugNativeTable);
    sbNativeOwnerAdd(&gSbRuntimeModuleName, &gSbRuntimeNativeTable);
    sbNativeOwnerAdd(&gSbObjxModuleName, &gSbObjxNativeTable);
    sbNativeOwnerAdd(&gSbCfmapModuleName, &gSbCfmapNativeTable);
    sbNativeOwnerAdd(&gSbMathModuleName, &gSbMathNativeTable);
    sbNativeOwnerAdd(&gSbSoundModuleName, &gSbSoundNativeTable);
}

void sbNativeOwnersAddExtra(void) {
    sbNativeOwnerAdd(&gSbDebugModuleName, &gSbDebugNativeTable);
    sbNativeOwnerAdd(&gSbSoundModuleName, &gSbSoundNativeTable);
}

s32 sbNativeOwnerGetValue(s32 ownerIndex) {
    extern s32 gSbNativeOwnerFlags[];
    return gSbNativeOwnerFlags[ownerIndex];
}

INCLUDE_ASM("asm/main/nonmatchings/tu099", func_001CE990);

INCLUDE_ASM("asm/main/nonmatchings/tu099", func_001CE9C0);

INCLUDE_ASM("asm/main/nonmatchings/tu099", func_001CEA70);

void sbNativeSlotsClear(void) {
    extern s32 gSbNativeTableWords;
    memset(&gSbNativeTableWords, 0, 0x1C);
}

s32 sbNativeSlotSet(s32 nativeSlotIndex, s32 nativeSlotWord) {
    extern s32 gSbNativeTableWords[];

    gSbNativeTableWords[nativeSlotIndex] = nativeSlotWord;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu099", func_001CEB20);
