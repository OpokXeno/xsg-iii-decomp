#include "common.h"
#include "shared.h"

#include "main/tu105.h"



/* Messages in a thread's queue (sbQueueReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF



void sbSlotJumpRequest(SbThread *thread, int key);

struct SbThread;

int VM_PLUGIN_CNT_GET(struct SbThread *thread);
int vmArgGet(struct SbThread *thread, int argIndex);
int Get_Rnd_001B8CD0(int min, int max);
int mathRandom(SbThread *thread);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CCF28);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CCFF0);

typedef signed char s8;
typedef int s32;
/* The argument block at gGameLoopRuntimeStorage: the two values sbDeferredCallStore stores (+0x10 and +0x12). */
typedef struct GameLoopRuntime {
    u8 unk_00[0x10];
    s16 firstVmArg;               /* +0x10 */
    s16 secondVmArg;              /* +0x12 */
} GameLoopRuntime;
extern GameLoopRuntime gGameLoopRuntimeStorage[];
s32 vmArgTagIsZero(SbThread *thread, s32 argIndex);

s32 sbDeferredCallStore(SbThread *thread)
{
  s16 firstVmArg;
  s16 secondVmArg;
  s16 storedSecondVmArg;
    if ((VM_PLUGIN_CNT_GET(thread) <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3))
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  firstVmArg = vmArgGet(thread, 1);
  if (vmArgTagIsZero(thread, 2) == 0)
  {
    secondVmArg = vmArgGet(thread, 2);
    storedSecondVmArg = secondVmArg;
  }
  else
  {
    storedSecondVmArg = -1;
  }
  gGameLoopRuntimeStorage->firstVmArg = firstVmArg;
  gGameLoopRuntimeStorage->secondVmArg = storedSecondVmArg;
  return 1;
}

s32 func_001BBCB8(s16 first, s16 second);

s32 sbDeferredCallRun(void)
{
  GameLoopRuntime *deferredCall;
  s16 secondVmArg;
  deferredCall = gGameLoopRuntimeStorage;
  secondVmArg = deferredCall->secondVmArg;
  if (((deferredCall->firstVmArg) > 0) == 0U)
  {
    return 0;
  }
  func_001BBCB8(deferredCall->firstVmArg, secondVmArg);
  return 1;
}

/* Script native "math.random": two arguments, min and max, passed to Get_Rnd. */
int mathRandom(SbThread *thread)
{
    int randomMin;
    int randomMax;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    randomMin = vmArgGet(thread, 1);
    randomMax = vmArgGet(thread, 2);
    return Get_Rnd_001B8CD0(randomMin, randomMax);
}

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD248);

s32 func_001B5C38(s32, s32);

s32 mathArgsTest(SbThread *thread) {
    s32 firstVmArg;
    int secondVmArg;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    firstVmArg = vmArgGet(thread, 1);
    secondVmArg = vmArgGet(thread, 2);
    return func_001B5C38(firstVmArg, secondVmArg) != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD410);

INCLUDE_ASM("asm/main/nonmatchings/tu091", func_001CD490);
