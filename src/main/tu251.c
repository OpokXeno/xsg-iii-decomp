#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu251", func_002616C8);

INCLUDE_ASM("asm/main/nonmatchings/tu251", func_00261808);

INCLUDE_ASM("asm/main/nonmatchings/tu251", func_00261890);

INCLUDE_ASM("asm/main/nonmatchings/tu251", func_002619C0);

INCLUDE_ASM("asm/main/nonmatchings/tu251", func_00261A78);

INCLUDE_ASM("asm/main/nonmatchings/tu251", func_00261BB8);

typedef int s32;
typedef float f32;
extern f32 gCameraBlendWeightScale;
s32 func_00265528(s32, s32, f32);
s32 vmArgGet(s32 vm, s32 argIndex);

s32 sbNativeSetScaledPair(s32 vm) {
    f32 blendWeight;
    s32 cameraHandle;
    s32 blendValue;

    blendWeight = vmArgGet(vm, 1) * gCameraBlendWeightScale;
    cameraHandle = vmArgGet(vm, 2);
    blendValue = vmArgGet(vm, 3);
    func_00265528(cameraHandle, blendValue, blendWeight);
    return 1;
}
