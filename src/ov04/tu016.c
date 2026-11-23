/*
 * OV04 original TU 16: 0x00a9e368..0x00a9f6e0 (22 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9E368);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9E858);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9EA40);

typedef short s16;
typedef unsigned short u16;
typedef int s32;
extern s16 gOv04ScriptWords[];
extern s16 D_00AA7850[]; /* gOv04ScriptWords from its first halfword: the command under the cursor, then its operands */

typedef unsigned char u8;

/* Bit of ScreenModeControl.modeBits: SCREEN_MODE_DEMO_INPUT replays demoButtons instead of the pads. */
#define SCREEN_MODE_DEMO_INPUT 0x10

/* Operand value that turns a script switch off (commands that set or clear one mode flag). */
#define SCRIPT_OPERAND_OFF 0xC

/* Operand value that turns a script switch on. */
#define SCRIPT_SWITCH_ON 10

/* Pad button bits of ScreenModeControl.demoButtons. */
#define PAD_L2 0x0001
#define PAD_R2 0x0002
#define PAD_L1 0x0004
#define PAD_R1 0x0008
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE 0x0020
#define PAD_CROSS 0x0040
#define PAD_SQUARE 0x0080

/* Screen mode control block (shared with tu002, tu004, tu005, tu006 and tu020): the fields the script commands of this TU touch. */
typedef struct ScreenModeControl {
    s32 modeBits;       /* 0x0000: SCREEN_MODE_* */
    u8 unk_0004[0x2286];
    u16 demoButtons;           /* 0x228A */
} ScreenModeControl;
extern ScreenModeControl gScreenModeControlStorage;

/* Script command: sets or clears bit 0x20 of ScreenModeControl.modeBits from the operand under gOv04ScriptCursor (a
 * halfword index into gOv04ScriptWords) and advances the cursor past command and operand. */
void ov04ScriptSetStateBit20(void)
{
    extern s16 gOv04ScriptCursor;
    if (gOv04ScriptWords[gOv04ScriptCursor] == SCRIPT_OPERAND_OFF) {
        gScreenModeControlStorage.modeBits &= ~0x20;
    } else {
        gScreenModeControlStorage.modeBits |= 0x20;
    }
    gOv04ScriptCursor += 2;
}

void ov04ScriptSetStateBit01(void)
{
    extern s16 gOv04ScriptCursor;

    if (gOv04ScriptWords[gOv04ScriptCursor] == 1) {
        gScreenModeControlStorage.modeBits |= 1;
    } else {
        gScreenModeControlStorage.modeBits &= ~1;
    }
    gOv04ScriptCursor += 2;
}

void ov04StepLatchEntry(void)
{
    extern s16 gOv04ScriptCursor;
    extern s16 gOv04ScriptStepPhase;
    extern s16 gOv04LatchedWord;
    u16 scriptWord;

    gOv04ScriptStepPhase = 1;
    scriptWord = gOv04ScriptWords[gOv04ScriptCursor];
    gOv04ScriptCursor += 2;
    gOv04LatchedWord = scriptWord;
}

/* Script command: enters step state 2 (gOv04ScriptStepPhase) with the command's two operands latched. */
void ov04StepLatchPair(void)
{
    extern s16 gOv04ScriptCursor;
    extern s16 gOv04ScriptStepPhase;
    extern s16 D_00AA905A;
    extern s16 D_00AA905C;

    gOv04ScriptStepPhase = 2;
    D_00AA905A = D_00AA7850[gOv04ScriptCursor + 1];
    D_00AA905C = D_00AA7850[gOv04ScriptCursor + 2];
    gOv04ScriptCursor += 3;
}

/* Script command: enters step state 3 (gOv04ScriptStepPhase) with an id and the three operands that follow it latched. */
void ov04StepLatchIdAndOperands(void)
{
    extern s16 gOv04ScriptCursor;
    extern s16 gOv04ScriptStepPhase;
    extern s16 D_00AA905E;
    extern s16 D_00AA9060[];

    gOv04ScriptStepPhase = 3;
    D_00AA905E = D_00AA7850[gOv04ScriptCursor + 1];
    D_00AA9060[0] = D_00AA7850[gOv04ScriptCursor + 2];
    D_00AA9060[1] = D_00AA7850[gOv04ScriptCursor + 3];
    D_00AA9060[2] = D_00AA7850[gOv04ScriptCursor + 4];
    gOv04ScriptCursor += 5;
}

s32 func_00A83C20(s16 index, s32 enabled);

void ov04ScriptEnableTableEntry(void)
{
    extern s16 gOv04ScriptCursor;

    func_00A83C20(gOv04ScriptWords[gOv04ScriptCursor], 1);
    gOv04ScriptCursor += 2;
}

s32 actorTableExpireLongest(void);

void ov04StepExpireActor(void)
{
    extern u16 gOv04ScriptCursor;

    actorTableExpireLongest();
    gOv04ScriptCursor += 1;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9EDD8);

/* Script command: switches the pads to demo input and presses the demo buttons whose operand is on (circle, cross,
 * triangle, square, L1, L2, R1, R2 in operand order). */
void ov04ScriptSetDemoButtons(void)
{
    extern s16 gOv04ScriptCursor;

    gScreenModeControlStorage.modeBits |= SCREEN_MODE_DEMO_INPUT;
    gScreenModeControlStorage.demoButtons = 0;
    if (D_00AA7850[gOv04ScriptCursor + 1] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons = PAD_CIRCLE;
    }
    if (D_00AA7850[gOv04ScriptCursor + 2] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_CROSS;
    }
    if (D_00AA7850[gOv04ScriptCursor + 3] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_TRIANGLE;
    }
    if (D_00AA7850[gOv04ScriptCursor + 4] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_SQUARE;
    }
    if (D_00AA7850[gOv04ScriptCursor + 5] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_L1;
    }
    if (D_00AA7850[gOv04ScriptCursor + 6] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_L2;
    }
    if (D_00AA7850[gOv04ScriptCursor + 7] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_R1;
    }
    if (D_00AA7850[gOv04ScriptCursor + 8] == SCRIPT_SWITCH_ON) {
        gScreenModeControlStorage.demoButtons |= PAD_R2;
    }
    gOv04ScriptCursor += 9;
}

s32 gsFlagBlockASet(s32 enabled);

void ov04ScriptSetDemoInput(void)
{
    extern s16 gOv04ScriptCursor;

    if (gOv04ScriptWords[gOv04ScriptCursor] == SCRIPT_OPERAND_OFF) {
        gScreenModeControlStorage.modeBits &= ~SCREEN_MODE_DEMO_INPUT;
        gsFlagBlockASet(0);
    } else {
        gScreenModeControlStorage.modeBits |= SCREEN_MODE_DEMO_INPUT;
        gsFlagBlockASet(1);
    }
    gOv04ScriptCursor += 2;
}

s32 ov04CallEntryRoutine(s16 index);

s32 ov04ScriptApplyNextEntry(void)
{
    extern s16 gOv04ScriptCursor;
    s32 newScriptCursor;

    ov04CallEntryRoutine(gOv04ScriptWords[gOv04ScriptCursor]);
    newScriptCursor = (u16) gOv04ScriptCursor + 2;
    gOv04ScriptCursor = newScriptCursor;
    return newScriptCursor;
}

s32 func_00A9D718(s32 actorIndex, s32 mode, s32 value);
s32 func_00AA30D0(s32, s32, s32, s32, s32, s32, s32, s32, float, s32, s32);

/* Script command: plays a sound, then draws the message whose id is operand 1 at the position of operands 2 and 3;
 * operand 4 is 0xF for the flagOperandSet variant. */
void ov04ScriptShowMessage(void)
{
    extern s16 gOv04ScriptCursor;
    s32 flagOperandSet = D_00AA7850[gOv04ScriptCursor + 4] == 0xF;

    func_00A9D718(-1, 5, 4);
    func_00AA30D0(D_00AA7850[gOv04ScriptCursor + 1], 0x1D, D_00AA7850[gOv04ScriptCursor + 2],
                  D_00AA7850[gOv04ScriptCursor + 3], -1, -1, 0, 0, 2.5f, flagOperandSet, 0x802F4F5F);
    gOv04ScriptCursor += 5;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9F1D8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9F320);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9F400);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9F508);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9F608);

void ov04ScriptNoop(void)
{

}

void ov04ScriptClearStateBit08(void)
{
    extern u16 gOv04ScriptCursor;
    extern s16 gOv04ScriptStepPhase;

    gOv04ScriptStepPhase = 4;
    gScreenModeControlStorage.modeBits &= ~8;
    gOv04ScriptCursor += 1;
}
