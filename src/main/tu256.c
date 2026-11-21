#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263368);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263478);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263538);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263648);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_002636A8);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263708);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263768);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_002637C8);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_002638D8);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263A30);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263A90);

typedef int s32;
void objectSetActiveState(s32 value);
s32 vmArgGet(s32 vm, s32 argIndex);

s32 sbNativeObjectSetStateByte(s32 vm) {
    s32 scriptStateCode;

    scriptStateCode = vmArgGet(vm, 1);
    objectSetActiveState(scriptStateCode);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263B18);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263C30);

void cameraLoadResource(s32 resource_id);

s32 sbNativeCameraLoadResource(s32 vm) {
    s32 cameraResourceId;

    cameraResourceId = vmArgGet(vm, 1);
    cameraLoadResource(cameraResourceId);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263D70);

INCLUDE_ASM("asm/main/nonmatchings/tu256", func_00263DD8);
