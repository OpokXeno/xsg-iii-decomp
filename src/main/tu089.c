#include "common.h"
#include "main/tu118.h"

typedef int s32;

typedef short s16;

void slotEntryMotionFinish(s32 object);

s32 scriptDispModelSlotPairVisibleSet(s32 scriptThread) {
    s32 slotKindSelector;
    s32 slotOptionFlag;
    s32 *dispModelSlot;
    if (VM_PLUGIN_CNT_GET(scriptThread) <= 0) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    slotKindSelector = vmArgGet(scriptThread, 1);
    slotOptionFlag = vmArgTagIsZero(scriptThread, 2) != 0 ? 0 : vmArgGet(scriptThread, 2);
    dispModelSlot = RgDispModelFindByKey(slotKindSelector);
    if (dispModelSlot == 0) {
        return 0;
    }
    if (slotOptionFlag != 0) {
        slotEntryMotionFinish((s32) dispModelSlot);
        slotEntryMotionFinish(dispModelSlot[6]);
    } else {
        slotEntryMotionStart(dispModelSlot, 0, 1);
        slotEntryMotionStart((s32 *) dispModelSlot[6], 0, 0);
    }
    return 1;
}

s32 *RgDispModelFindByKey(s32 slotKindSelector);

s32 slotEntryMotionStart(s32 *dispModelSlot, s32 a, s32 b);

s32 scriptDispModelSlotPairHide(s32 scriptThread) {
    s32 *dispModelSlot;
    if (VM_PLUGIN_CNT_GET(scriptThread) != 1) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    dispModelSlot = RgDispModelFindByKey(vmArgGet(scriptThread, 1));
    if (dispModelSlot == 0) {
        return 0;
    }
    slotEntryMotionStart(dispModelSlot, 1, 1);
    slotEntryMotionStart((s32 *) dispModelSlot[6], 1, 0);
    return 1;
}

s32 sbNativeNoop(void) {
    return 0;
}

s32 VM_PLUGIN_CNT_GET();

s32 sbSlotJumpRequest();

s32 vmArgGet();

void func_001BE470(s32 value, s32 mode);

s32 vmArgTagIsZero(s32 scriptThread, s32 argIndex);

void sefSetEffectParams(s32 effectIndex, s32 effectPrimaryArg, s32 effectSecondaryArg, s32 effectTertiaryArg, s32 effectOption);

s32 scriptEffectParamsSetDefault(s32 scriptThread) {
    s32 effectPrimaryArg;
    s32 effectOption;
    s32 argTwoTagIsZero;
    if (VM_PLUGIN_CNT_GET(scriptThread) <= 0 || VM_PLUGIN_CNT_GET(scriptThread) >= 3) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    effectPrimaryArg = vmArgGet(scriptThread, 1);
    argTwoTagIsZero = vmArgTagIsZero(scriptThread, 2);
    effectOption = argTwoTagIsZero == 0 ? vmArgGet(scriptThread, 2) : 1;
    sefSetEffectParams(0, effectPrimaryArg, 0, 0, effectOption);
    return 1;
}

s32 scriptEffectParamsSetChained(s32 scriptThread) {
    s32 effectIndex;
    s32 effectPrimaryArg;
    s32 effectSecondaryArg;
    s32 effectTertiaryArg;
    s32 effectFinalArg;
    if (VM_PLUGIN_CNT_GET(scriptThread) <= 0 || VM_PLUGIN_CNT_GET(scriptThread) >= 5) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    effectPrimaryArg = vmArgGet(scriptThread, 1);
    effectIndex = 1;
    effectTertiaryArg = 0;
    effectSecondaryArg = 0;
    effectFinalArg = vmArgTagIsZero(scriptThread, 4) != 0 ? 1 : vmArgGet(scriptThread, 4);
    if (vmArgTagIsZero(scriptThread, 2) == 0) {
        effectSecondaryArg = vmArgGet(scriptThread, 2);
        if (effectSecondaryArg != 0) {
            effectIndex = 2;
            if (vmArgTagIsZero(scriptThread, 3) == 0) {
                effectTertiaryArg = vmArgGet(scriptThread, 3);
                if (effectTertiaryArg != 0) {
                    effectIndex = 3;
                }
            }
        }
    }
    sefSetEffectParams(effectIndex, effectPrimaryArg, effectSecondaryArg, effectTertiaryArg, effectFinalArg);
    return 1;
}

s32 sbNativeSlotSet(s32 index, s32 value);

s32 sbNativeSetTableEntry(s32 scriptThread) {
    s32 nativeSlotIndex;
    s32 nativeSlotValue;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    nativeSlotIndex = vmArgGet(scriptThread, 1);
    nativeSlotValue = vmArgGet(scriptThread, 2);
    sbNativeSlotSet(nativeSlotIndex, nativeSlotValue);
    return 1;
}

s32 func_001BED60(s32 effectPrimaryArg, s32 effectSecondaryArg, s32 effectTertiaryArg, AlignedVector4 *offset);

s32 func_001CC678(s32 scriptThread) {
    s16 effectPrimaryArg;
    s16 effectSecondaryArg;
    s16 effectTertiaryArg;
    AlignedVector4 modelPositionOffset;
    if (VM_PLUGIN_CNT_GET(scriptThread) < 3 || VM_PLUGIN_CNT_GET(scriptThread) >= 7) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&modelPositionOffset) : "memory");
    effectPrimaryArg = vmArgGet(scriptThread, 1);
    effectSecondaryArg = vmArgGet(scriptThread, 2);
    effectTertiaryArg = vmArgGet(scriptThread, 3);
    if (vmArgTagIsZero(scriptThread, 4) != 0) {
        return func_001BED60(effectPrimaryArg, effectSecondaryArg, effectTertiaryArg, 0);
    }
    modelPositionOffset.x = vmArgGet(scriptThread, 4) * 0.01f;
    modelPositionOffset.y = vmArgGet(scriptThread, 5) * 0.01f;
    modelPositionOffset.z = vmArgGet(scriptThread, 6) * 0.01f;
    return func_001BED60(effectPrimaryArg, effectSecondaryArg, effectTertiaryArg, &modelPositionOffset);
}

s32 markerPositionGet(s32 *request, s16 value);

s32 SpawnSlotSetPosition(s32 *object, s32 *request);

s32 func_00290548(s32 handle, s32 timeout);

s32 sbNativeQueueObjectRequest(s32 scriptThread)
{
  s32 objectQueueRequest[4];
  s32 queuedRequestValue;
  s32 subobjectHandle;
  s32 *positionObject;
  if (VM_PLUGIN_CNT_GET(scriptThread) != 1)
  {
    sbSlotJumpRequest(scriptThread, 2);
    return 0;
  }
  positionObject = RgDispModelFindByKey(0);
  if (positionObject != 0)
  {
    queuedRequestValue = vmArgGet(scriptThread, 1);
    markerPositionGet(&objectQueueRequest[0], queuedRequestValue);
    SpawnSlotSetPosition(positionObject, &objectQueueRequest[0]);
    subobjectHandle = positionObject[9];
    if (subobjectHandle != 0)
    {
      func_00290548(subobjectHandle, 0x1E);
    }
  }
  return 1;
}

s32 scriptCameraTargetFlagSet(s32 scriptThread) {
    s16 cameraValue;
    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    cameraValue = vmArgGet(scriptThread, 1);
    if (cameraValue >= 0) {
        gGameLoopRuntimeStorage.flags |= 0x400000;
        func_001BE470(cameraValue, 1);
    } else {
        gGameLoopRuntimeStorage.flags &= 0xffbfffff;
    }
    return 1;
}

void valueSlotChangeRequest(s32 effectPrimaryArg, s32 effectSecondaryArg);

s32 scriptDeferredCallRequest(s32 scriptThread) {
    s16 effectPrimaryArg;
    s16 effectSecondaryArg;
    if (VM_PLUGIN_CNT_GET(scriptThread) <= 0 || VM_PLUGIN_CNT_GET(scriptThread) >= 4) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    effectPrimaryArg = vmArgGet(scriptThread, 1);
    effectSecondaryArg = vmArgTagIsZero(scriptThread, 2) != 0 ? -1 : vmArgGet(scriptThread, 2);
    if (effectPrimaryArg <= 0) {
        return 0;
    }
    valueSlotChangeRequest(effectPrimaryArg, effectSecondaryArg);
    return 1;
}

s32 saveStateIndexGet(s32 slot, s32 mode);

s32 sbNativeGetStoredValue(s32 scriptThread) {
    if (VM_PLUGIN_CNT_GET() != 0) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    return saveStateIndexGet(scriptThread, 2);
}

s32 saveStateIndexSet(s32 value);

s32 sbNativeSetStoredValue(s32 scriptThread) {
    s32 storedValue;
    if (VM_PLUGIN_CNT_GET() >= 2) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    storedValue = vmArgGet(scriptThread, 1);
    return saveStateIndexSet(storedValue);
}

s32 bitmapTestBit(s32 bit);

s32 saveFlagByteGet(s32 index);

s32 sbNativeTestBit(s32 scriptThread)
{
  s32 bitIndex;
  int *flagSelectRef;
  int flagSelect;
  if (VM_PLUGIN_CNT_GET() >= 2)
  {
    sbSlotJumpRequest(scriptThread, 2);
    return 0;
  }
  bitIndex = vmArgGet(scriptThread, 1);
  flagSelectRef = &flagSelect;
  flagSelect = bitIndex & 0x1000;
  if (*flagSelectRef)
  {
    return saveFlagByteGet(bitIndex);
  }
  return bitmapTestBit(bitIndex);
}

s32 bitmapSetBit(s32 bit, s32 value);

s32 saveFlagByteSet(s32 index, s32 value);

s32 sbNativeSetBit(s32 scriptThread)
{
  s32 bitIndex;
  s32 bitValue;
  s32 *bitValueRef;
  s32 bitValueCopy;
  if (VM_PLUGIN_CNT_GET() >= 3)
  {
    sbSlotJumpRequest(scriptThread, 2);
    return 0;
  }
  bitIndex = vmArgGet(scriptThread, 1);
  bitValue = vmArgGet(scriptThread, 2);
  bitValueRef = &bitValue;
  bitValueCopy = *bitValueRef;
  if (bitIndex & 0x1000)
  {
    return saveFlagByteSet(bitIndex, bitValueCopy);
  }
  return bitmapSetBit(bitIndex, bitValue);
}

extern s16 gSaveTags[];

extern s16 D_003E46BA[];

extern s16 D_003E46BC[];

extern s16 gGameBootMode;

s32 scriptGameStateQuery(s32 scriptThread) {
    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    switch (vmArgGet(scriptThread, 1)) {
    case 0:
        return gSaveTags[0];
    case 1:
        return D_003E46BA[0];
    case 2:
        return D_003E46BC[0];
    case 3:
        return (gGameLoopRuntimeStorage.flags >> 21) & 1;
    case 4:
        return gGameBootMode == 2;
    default:
        return 0;
    }
}

/* Names of the seven entries of the scene table: space0000 to space0006. */
const char D_005379F8[7][16] = {
    "space0000",
    "space0001",
    "space0002",
    "space0003",
    "space0004",
    "space0005",
    "space0006"
};

s32 scriptRuntimeFlagToggle(s32 scriptThread) {
    s32 *dispModelSlot;
    s32 flagToggleKind;
    s32 flagToggleOn;
    dispModelSlot = RgDispModelFindByKey(0);
    if (VM_PLUGIN_CNT_GET(scriptThread) != 2) {
        sbSlotJumpRequest(scriptThread, 2);
        return 0;
    }
    flagToggleKind = vmArgGet(scriptThread, 1);
    flagToggleOn = vmArgGet(scriptThread, 2);
    switch (flagToggleKind) {
    case 0:
        if (flagToggleOn != 0) {
            gGameLoopRuntimeStorage.flags |= 0x20;
            if (dispModelSlot != 0) {
                dispModelSlot[0] |= 0x100000;
            }
        } else {
            gGameLoopRuntimeStorage.flags &= 0xffffffdf;
            if (dispModelSlot != 0) {
                dispModelSlot[0] &= 0xffefffff;
            }
        }
        break;
    case 1:
        if (flagToggleOn != 0) {
            gGameLoopRuntimeStorage.flags |= 0x40000;
        } else {
            gGameLoopRuntimeStorage.flags &= 0xfffbffff;
        }
        break;
    case 2:
        if (flagToggleOn != 0) {
            gGameLoopRuntimeStorage.flags |= 0x80000;
        } else {
            gGameLoopRuntimeStorage.flags &= 0xfff7ffff;
        }
        break;
    default:
        return 0;
    }
    return 1;
}

s32 func_00245308();

s32 xglCdAddReadyBits();

s32 vmArgTagGet();

s32 vmKeyedEntryGet();

s32 sbNativeJumpToKeyedEntry(s32 vm) {
    s32 keyedEntryKey;
    s32 keyedEntryText;

    if (VM_PLUGIN_CNT_GET(vm) >= 2) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    if (!((vmArgTagGet(vm, 1) & 0xFF) == 2)) {
        return 0;
    }
    keyedEntryKey = vmArgGet(vm, 1);
    keyedEntryText = vmKeyedEntryGet(vm, keyedEntryKey);
    xglCdAddReadyBits(1);
    func_00245308(keyedEntryText);
    return 1;
}

s32 sbNativeAlwaysSucceed(void) {
    return 1;
}

s32 func_001B3858();

s32 sbNativeCallKeyedEntry(s32 vm) {
    s32 keyedEntryKey;
    s32 keyedEntryText;

    if (VM_PLUGIN_CNT_GET(vm) >= 2) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    if (!((vmArgTagGet(vm, 1) & 0xFF) == 2)) {
        return 0;
    }
    keyedEntryKey = vmArgGet(vm, 1);
    keyedEntryText = vmKeyedEntryGet(vm, keyedEntryKey);
    func_001B3858(keyedEntryText);
    return 1;
}

s16 gSaveTags[] = { 0 };
s16 D_003E46BA[] = { 0 };
s16 D_003E46BC[14] = { 0 };
void *gRgSceneRoot[54] = { 0 };
