#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu379", func_002C7A98);

typedef int s32;
extern s32 gGameLoopRuntimeStorage;
s32 bitmapTestBit(s32 bit);

s32 masterFlagTest(void)
{
  s32 masterFlagBit;
  s32 isEnabled;
  int overrideBitSet;
  masterFlagBit = gGameLoopRuntimeStorage;
  isEnabled = 1;
  masterFlagBit &= 0x08000000;
  masterFlagBit = masterFlagBit != 0;
  overrideBitSet = bitmapTestBit(0xC7);
  isEnabled = 1;
  if (overrideBitSet == 0)
  {
    isEnabled = masterFlagBit;
  }
  return isEnabled;
}

s32 saveStateIndexGet();
s32 masterFlagTest();

s32 masterFlagValueGet(void)
{
  s32 overrideValue;
  s32 isEnabled;
  isEnabled = masterFlagTest();
  overrideValue = 0x189;
  if (isEnabled == 0)
  {
    overrideValue = saveStateIndexGet();
  }
  return overrideValue;
}
