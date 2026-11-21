/*
 * OV04 original TU 15: 0x00a9dd40..0x00a9e368 (10 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9DD40);

typedef short s16;
typedef int s32;
extern s16 D_00AA944A;
s32 soundSlotSetVolume(s32 index, s32 level, s32 extra);

void soundDefaultSlotReset(void) {
    D_00AA944A = -1;
    soundSlotSetVolume(0, 0, 0x1E);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9DE18);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9DEB0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9DF00);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9DF90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9DFE8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9E0C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9E170);

INCLUDE_ASM("asm/nonmatchings/ov04/tu015", func_00A9E228);
