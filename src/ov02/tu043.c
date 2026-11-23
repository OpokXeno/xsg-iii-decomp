/*
 * OV02 original TU 43: 0x00a8ee98..0x00a8fb00 (20 functions)
 */
#include "common.h"

typedef int s32;
extern s32 gSbScriptImageHandle;
s32 sbNativeBufferGet(s32);

void sbScriptImageInit(void) {
    gSbScriptImageHandle = 0;
    gSbScriptImageHandle = sbNativeBufferGet(1);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8EED0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F038);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F0C8);

s32 handleMakeType01(s32 battleHandleIndex) {
    return battleHandleIndex + 0x01010000;
}

s32 func_00A8F1B0();
s32 func_00A8F390();

void actionDispatchByKind(s32 actionKind, s32 actionValue) {
    if (actionKind < 0x12) {
        func_00A8F1B0(actionKind, actionValue);
        return;
    }
    func_00A8F390(actionKind, actionValue);
}

void actionDispatchDefault(void) {
    func_00A8F1B0();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F1B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F390);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F490);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F698);

s32 * func_001B5C68();

s32 RgBxxGetTexBase(void) {
    return *func_001B5C68();
}

s32 RgBxxGetTexNum(void) {
    return func_001B5C68()[1];
}

s32 *gameOverSlotAt();

s32 RgBxxGetAltTexBase(void) {
    return *gameOverSlotAt();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F7D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8F8D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8FA40);

s32 handleMakeType06(s32 battleHandleIndex) {
    return battleHandleIndex + 0x06010000;
}

s32 sbScriptImageGet(void) {
    return gSbScriptImageHandle;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu043", func_00A8FAA0);
