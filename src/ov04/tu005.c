/*
 * OV04 original TU 5: 0x00a8deb0..0x00a90940 (14 functions)
 */
#include "common.h"

typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
typedef unsigned short u16;
s32 func_00A99788(void);

/* Screen mode control block at gScreenModeControlStorage: screen mode at +0x16, countdown at +0x1A, script step flags at +0x00. Only the fields these functions touch are named. */
typedef struct ScreenModeControl {
    s32 scriptStepFlags;                        /* 0x00 */
    u8 unk_04[0x16 - 0x04];
    s16 screenMode;                          /* 0x16 */
    u8 unk_18[0x1A - 0x18];
    u16 countdownTicks;                        /* 0x1A */
    u8 unk_1C[0x17C - 0x1C];
    s32 unk_17C;                        /* 0x17C */
} ScreenModeControl;

extern unsigned char gScreenModeControlStorage[];
#define gScreenModeControl (*(ScreenModeControl *) gScreenModeControlStorage)

void screenModeClear(void)
{
    gScreenModeControl.screenMode = 0;
    gScreenModeControl.countdownTicks = 0;
    func_00A99788();
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8DED8);

void func_00A989B0(s32 id, s32 row, s32 x, s32 y, s32 size, s32 flags, s32 target, f32 scale);
s32 func_00AA32C8(s32 value);

void screenModeSetup(void)
{
    func_00AA32C8(-1);
    gScreenModeControl.unk_17C = 0;
    func_00A989B0(0x380, 0x2A, 0x20, 0x140, 0x20, 0, -1, 1.0f);
    gScreenModeControl.screenMode = 1;
    gScreenModeControl.scriptStepFlags |= 0x800000;
    gScreenModeControl.countdownTicks = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8DF98);

void screenCountdownStart(s16 countdownTicks)
{
    gScreenModeControl.screenMode = 2;
    gScreenModeControl.countdownTicks = countdownTicks;
}

s32 func_00A8E110(void);
void ov04SetValueNotifyA(s16 value);

void screenCountdownTick(void)
{
    u16 remaining;
    s32 remainingAsSigned;
    unsigned int countdownStep;

    remaining = gScreenModeControl.countdownTicks;
    countdownStep = 1;
    gScreenModeControl.countdownTicks -= countdownStep;
    remainingAsSigned = remaining << 0x10;
    if (remainingAsSigned <= 0)
    {
        func_00A8E110();
        ov04SetValueNotifyA(0x14);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8E110);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8E658);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8F780);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8F928);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8FBD8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8FE30);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A90348);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A90478);
