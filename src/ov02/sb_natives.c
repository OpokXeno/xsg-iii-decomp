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

typedef int s32;
void dataVPadSet(int virtualPad);
struct SbThread;
int vmArgGet(struct SbThread *thread, int argIndex);

/* VM native: sets the virtual pad from the first script argument (low 16 bits) and returns 1. */
s32 VPadSet(struct SbThread *thread) {
    s32 virtualPad;

    virtualPad = vmArgGet(thread, 1);
    dataVPadSet(virtualPad & 0xFFFF);
    return 1;
}
