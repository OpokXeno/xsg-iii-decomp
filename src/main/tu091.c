#include "common.h"
#include "shared.h"

#include "main/tu105.h"



/* Messages in a thread's queue (sbChainReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF



void sbSlotJumpRequest(SbThread *thread, int key);

struct SbThread;

int VM_PLUGIN_CNT_GET(struct SbThread *thread);
int vmArgGet(struct SbThread *thread, int argIndex);
int Get_Rnd_001B8CD0(int min, int max);
int mathRandom(SbThread *thread);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CCF28);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CCFF0);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD0D0);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD190);

/* Script native "math.random": two arguments, min and max, passed to Get_Rnd. */
int mathRandom(SbThread *context)
{
    int min;
    int max;

    if (VM_PLUGIN_CNT_GET(context) != 2) {
        sbSlotJumpRequest(context, 2);
        return 0;
    }
    min = vmArgGet(context, 1);
    max = vmArgGet(context, 2);
    return Get_Rnd_001B8CD0(min, max);
}

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD248);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD398);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD410);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD490);
