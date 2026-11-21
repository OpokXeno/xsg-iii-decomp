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

/* Steps the cursor gOv04ScriptCursor (halfword index into gOv04ScriptWords, advanced by 2) and sets or clears bit 0x20 of the
 * script step flags word gScreenModeControlStorage (ScreenModeControl.scriptStepFlags) from the entry under the cursor. */
void ov04ScriptSetStateBit20(void)
{
    extern s16 gOv04ScriptCursor;
    extern s32 gScreenModeControlStorage;
    if (gOv04ScriptWords[gOv04ScriptCursor] == 0xC) {
        gScreenModeControlStorage &= ~0x20;
    } else {
        gScreenModeControlStorage |= 0x20;
    }
    gOv04ScriptCursor += 2;
}

void ov04ScriptSetStateBit01(void)
{
    extern s16 gOv04ScriptCursor;
    extern s32 gScreenModeControlStorage;

    if (gOv04ScriptWords[gOv04ScriptCursor] == 1) {
        gScreenModeControlStorage |= 1;
    } else {
        gScreenModeControlStorage &= ~1;
    }
    gOv04ScriptCursor += 2;
}

void ov04StepLatchEntry(void)
{
    extern s16 gOv04ScriptCursor;
    extern s16 D_00AA9056;
    extern s16 gOv04LatchedWord;
    u16 scriptWord;

    D_00AA9056 = 1;
    scriptWord = gOv04ScriptWords[gOv04ScriptCursor];
    gOv04ScriptCursor += 2;
    gOv04LatchedWord = scriptWord;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9EC90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9ECE8);

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

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9EF18);

s32 gsFlagBlockASet(s32 enabled);

void ov04ScriptSetStateBit10(void)
{
    extern s16 gOv04ScriptCursor;
    extern s32 gScreenModeControlStorage;

    if (gOv04ScriptWords[gOv04ScriptCursor] == 0xC) {
        gScreenModeControlStorage &= ~0x10;
        gsFlagBlockASet(0);
    } else {
        gScreenModeControlStorage |= 0x10;
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

INCLUDE_ASM("asm/nonmatchings/ov04/tu016", func_00A9F118);

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
    extern s16 D_00AA9056;
    extern s32 gScreenModeControlStorage;

    D_00AA9056 = 4;
    gScreenModeControlStorage &= ~8;
    gOv04ScriptCursor += 1;
}
