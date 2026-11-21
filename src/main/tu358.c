#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu358", func_002ACED0);

typedef int s32;
s32 func_002B9050();

void menuWepSpPanelReset(void) {
    func_002B9050();
}

INCLUDE_ASM("asm/main/nonmatchings/tu358", func_002AD3D0);

s32 valueTableSet();

void menuWepSpPanelResetTable(void) {
    valueTableSet();
}

s32 func_002B9468();
s32 slotSetFlagPositive(s32, s32);

s32 menuWepSpPanelSetActive(s32 battleSlot, s32 unused_arg1, s32 active) {
    s32 result;

    result = func_002B9468();
    slotSetFlagPositive(battleSlot, active);
    return result;
}

s32 slotActivityModeSet();
s32 func_002B95A0(s32);

void menuWepSpPanelSetLimitState(s32 limitReached) {
    s32 clamped;

    clamped = limitReached;
    if (clamped < 0) {
        clamped = 0;
    }
    if (clamped >= 2) {
        clamped = 1;
    }
    slotActivityModeSet(clamped, 3);
    func_002B95A0(clamped);
}

void menuWepSpPanelSetAltState(s32 flag) {
    s32 clamped;

    clamped = flag;
    if (flag < 0) {
        clamped = 0;
    }
    if (clamped >= 2) {
        clamped = 1;
    }
    slotActivityModeSet(clamped, 4);
}

s32 func_002B99B8();

void menuWepSpPanelSetLimitIndex(s32 limitReached) {
    s32 clamped;

    clamped = limitReached;
    if (limitReached < 0) {
        clamped = 0;
    }
    if (clamped >= 2) {
        clamped = 1;
    }
    func_002B99B8(clamped);
}
