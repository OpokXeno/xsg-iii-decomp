/*
 * OV02 original TU 33: 0x00a8d018..0x00a8d0f8 (3 functions)
 */
#include "common.h"

struct SbThread;
int vmArgGet(struct SbThread *thread, int argIndex);
int vmKeyedEntryGet(struct SbThread *thread, int value);
void func_00A8ABC8(int value);

int func_00A8D018(struct SbThread *call)
{
    func_00A8ABC8(vmKeyedEntryGet(call, vmArgGet(call, 1)));
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu033", func_00A8D058);

INCLUDE_ASM("asm/nonmatchings/ov02/tu033", func_00A8D078);
