/*
 * OV04 original TU 20: 0x00aa0c00..0x00aa3d40 (17 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef short s16;
typedef float f32;
s32 xglCdReadQueueClear();
s32 func_001B64D8(s32, s32);
s32 func_001B6BD8();
s32 func_00AA0C30();

void ov04CdReadQueueInit(void) {
    func_001B64D8(0xE00000, 0x01200000);
    xglCdReadQueueClear();
    func_001B6BD8();
    func_00AA0C30();
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA0C30);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA17B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA22F0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA23C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA2B50);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA2D28);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA30D0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA32C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA3380);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA3428);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA3488);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA34E0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA3998);

extern s32 *D_00ABCE90;
extern s32 D_00ABCEA0;
s32 func_00AA3998(s32, s32 *);

void ov04CallEntryRoutine(s16 entryId) {
    func_00AA3998(D_00ABCEA0 + D_00ABCE90[entryId], D_00ABCE90);
}

/* Screen mode control block at gScreenModeControlStorage (shared with tu004, tu005, tu006, tu016); only the defaults written here are viewed. */
typedef struct ScreenModeControl {
    s32 scriptStepFlags;
    u8 unk04[0x15C];
    f32 unk_160;
    f32 unk_164;
    u8 unk168[0x18];
    s16 unk_180;
} ScreenModeControl;

extern ScreenModeControl gScreenModeControlStorage;

void screenDefaultsSet(void)
{
    gScreenModeControlStorage.unk_180 = 0x12C;
    gScreenModeControlStorage.unk_160 = 8.0f;
    gScreenModeControlStorage.unk_164 = 408.0f;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu020", func_00AA3C68);
