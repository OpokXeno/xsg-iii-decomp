#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC2B8);

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC390);

typedef int s32;
typedef short s16;

s32 sbNativeNoop(void) {
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC420);

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC4D0);

s32 VM_PLUGIN_CNT_GET();
s32 sbNativeSlotSet(s32 index, s32 value);
s32 sbSlotJumpRequest();
s32 vmArgGet();

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

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC678);

s32 func_001BEC28(s32 *request, s16 value);
s32 *func_001C0570(s32 handle);
s32 func_001C0B38(s32 *object, s32 *request);
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
  positionObject = func_001C0570(0);
  if (positionObject != 0)
  {
    queuedRequestValue = vmArgGet(scriptThread, 1);
    func_001BEC28(&objectQueueRequest[0], queuedRequestValue);
    func_001C0B38(positionObject, &objectQueueRequest[0]);
    subobjectHandle = positionObject[9];
    if (subobjectHandle != 0)
    {
      func_00290548(subobjectHandle, 0x1E);
    }
  }
  return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC880);

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CC928);

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

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CCBA8);

INCLUDE_ASM("asm/main/nonmatchings/tu089", func_001CCC70);
