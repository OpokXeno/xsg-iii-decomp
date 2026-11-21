#include "common.h"

typedef int s32;

void sbNativeIgnoredCall(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AABC8);

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AAC60);

s32 VM_PLUGIN_CNT_GET();
s32 func_002A5308();
s32 func_002A5AF8();
s32 sbSlotJumpRequest();
s32 vmArgGet();
s32 vmArgTagGet();
/* Argument block of sbNativeSlotStartWithArgs, passed to func_002A5308 as a whole. */
struct VmArgRecord {
    /* 0x00 */ s32 hasVmArgSecond;
    /* 0x04 */ s32 vmArgSecond;
    /* 0x08 */ s32 hasVmArgThird;
    /* 0x0C */ s32 vmArgThird;
    /* 0x10 */ s32 hasVmArgFourth;
    /* 0x14 */ s32 vmArgFourth;
    /* 0x18 */ s32 vmArgFifth;
    /* 0x1C */ s32 slotId;
};

s32 sbNativeSlotStartWithArgs(s32 vm) {
    struct VmArgRecord startCallArgs;
    s32 slotId;
    s32 slotNumber;

    if ((VM_PLUGIN_CNT_GET(vm) != 5) || (slotId = vmArgGet(vm, 1), slotNumber = func_002A5AF8(slotId), (slotNumber == -1))) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    if (vmArgTagGet(vm, 2) != 0) {
        startCallArgs.hasVmArgSecond = 1;
        startCallArgs.vmArgSecond = vmArgGet(vm, 2);
    } else {
        startCallArgs.hasVmArgSecond = 0;
        startCallArgs.vmArgSecond = 0;
    }
    if (vmArgTagGet(vm, 3) != 0) {
        startCallArgs.hasVmArgThird = 1;
        startCallArgs.vmArgThird = vmArgGet(vm, 3);
    } else {
        startCallArgs.hasVmArgThird = 0;
        startCallArgs.vmArgThird = 0;
    }
    if (vmArgTagGet(vm, 4) != 0) {
        startCallArgs.hasVmArgFourth = 1;
        startCallArgs.vmArgFourth = vmArgGet(vm, 4);
    } else {
        startCallArgs.hasVmArgFourth = 0;
        startCallArgs.vmArgFourth = 0;
    }
    startCallArgs.vmArgFifth = vmArgGet(vm, 5);
    startCallArgs.slotId = slotId;
    func_002A5308(slotNumber, &startCallArgs.hasVmArgSecond);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AAE38);

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AAEE0);

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AAF90);

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AB040);

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AB0D0);

INCLUDE_ASM("asm/main/nonmatchings/tu353", func_002AB168);

s32 mapStaticStoreProbe(s32);

s32 undulateNativeSetIndex(s32 vm) {
    s32 mapGroupId;
    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    mapGroupId = vmArgGet(vm, 1);
    mapStaticStoreProbe(mapGroupId);
    return 1;
}
