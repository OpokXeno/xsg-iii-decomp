/*
 * OV04 original TU 11: 0x00a9a8d8..0x00a9a9d0 (4 functions)
 */
#include "common.h"

typedef short s16;
typedef int s32;
extern s32 gScratchpadSendValue;
s32 scratchpadSendCmd23Mode0(s32 *);

void ov04SetValueNotifyA(s16 scratchpadValue) {
    gScratchpadSendValue = scratchpadValue;
    scratchpadSendCmd23Mode0(&gScratchpadSendValue);
}

s32 scratchpadSendCmd23Mode1(s32 *);

void ov04SetValueNotifyB(s16 scratchpadValue) {
    gScratchpadSendValue = scratchpadValue;
    scratchpadSendCmd23Mode1(&gScratchpadSendValue);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu011", func_00A9A938);

extern s32 D_00ABCEAC;

s32 ov04CounterValueGet(void) {
    return D_00ABCEAC;
}
