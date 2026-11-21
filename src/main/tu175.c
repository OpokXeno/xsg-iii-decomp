#include "common.h"

typedef short s16;
typedef int s32;
extern s16 D_00551458;
extern s16 gMcSetupStatus;
extern s16 D_0055145C;
extern s32 gMcSeekCurrentPos;
extern s32 gMcSeekTargetPos;
extern s32 D_00551468;

void mcModuleReset(void) {
    D_00551458 = 0;
    gMcSetupStatus = 0;
    D_0055145C = 0;
    gMcSeekCurrentPos = 0;
    gMcSeekTargetPos = 0;
    D_00551468 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu175", func_00220B90);

INCLUDE_ASM("asm/main/nonmatchings/tu175", func_00220C38);

void mcClearSetupStatus(void) {
    gMcSetupStatus = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu175", func_00220D28);

INCLUDE_ASM("asm/main/nonmatchings/tu175", func_00220E00);

INCLUDE_ASM("asm/main/nonmatchings/tu175", func_00220ED8);

INCLUDE_ASM("asm/main/nonmatchings/tu175", func_00220FB0);

s32 func_00220FB0(s32 offset, s32 seekMode);

void mcStateSeekEntry(s32 seekMode) {
    func_00220FB0(-gMcSeekCurrentPos, seekMode);
    if (gMcSeekCurrentPos != 0) {
        func_00220FB0(gMcSeekTargetPos - gMcSeekCurrentPos, seekMode);
    }
}

s32 func_00213700(s32 textId);
void func_00220B90(s32 field, s32 value);
void mcClearSetupStatus(void);
void func_00220D28(void);

void mcStateSetupRequest(s32 setupFlags, s32 id) {
    s32 resolvedText;
    mcClearSetupStatus();
    if (setupFlags & 1) {
        resolvedText = func_00213700(0x20);
        func_00220B90(0x10, resolvedText);
    }
    if (id != 0) {
        func_00220B90(0x80, id);
    }
    if (setupFlags & 2) {
        resolvedText = func_00213700(0x1F);
        func_00220B90(0x40, resolvedText);
    }
    if (setupFlags & 4) {
        resolvedText = func_00213700(0x1E);
        func_00220B90(0x20, resolvedText);
    }
    if (setupFlags & 8) {
        resolvedText = func_00213700(0x22);
        func_00220B90(8, resolvedText);
        func_00220B90(4, 0);
    }
    if (setupFlags & 0x10) {
        resolvedText = func_00213700(0x23);
        func_00220B90(2, resolvedText);
        func_00220B90(1, 0);
    }
    if (setupFlags & 0x20) {
        resolvedText = func_00213700(0x2C);
        func_00220B90(2, resolvedText);
        func_00220B90(1, 0);
    }
    func_00220D28();
}
