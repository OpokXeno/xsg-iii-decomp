#include "common.h"
#include "shared.h"

#include "main/gifState.h"

extern GifStatusBlock gifStatus;
/* Scaffold data without a config name (splat names): the per-mode table of pointer-sized entries, and the
 * word the selected entry is copied into. */
extern void *D_003EB7C0[];
extern void *D_004C83D8[];

void gifStateSetMode(int mode)
{
    if (mode < 6) {
        D_004C83D8[0] = D_003EB7C0[mode];
        gifStatus.state = mode;
        gifStatus.ratio = 1.0f;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu221", func_00258B78);

INCLUDE_ASM("asm/main/nonmatchings/tu221", func_00258B88);

INCLUDE_ASM("asm/main/nonmatchings/tu221", func_00258B90);
