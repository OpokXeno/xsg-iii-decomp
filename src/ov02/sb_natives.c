#include "common.h"
#include "shared.h"

void dataVPadModeSet(int mode);

int VPadEnable(void);
int VPadDisable(void);

int VPadEnable(void)
{
    dataVPadModeSet(1);
    return 1;
}

int VPadDisable(void)
{
    dataVPadModeSet(0);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives", func_00A8CFF0);
