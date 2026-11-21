/*
 * OV02 original TU 38: 0x00a8ddb8..0x00a8dee8 (4 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu038", func_00A8DDB8);

typedef int s32;
s32 func_001B60B8(s32 channel);

s32 cdChannelPollFlag(s32 unused, s32 *channelBusyFlag) {
    if (func_001B60B8(3) != 0) {
        *channelBusyFlag = 1;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu038", func_00A8DE58);

s32 func_002107B8(s32 address, s32 enable);

s32 overlayBlockEnable(void) {
    func_002107B8(0xA7C000, 1);
    return 1;
}
