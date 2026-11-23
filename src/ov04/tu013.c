/*
 * OV04 original TU 13: 0x00a9c918..0x00a9d408 (12 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9C918);

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9C980);

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9CAE8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9CCB0);

typedef short s16;
typedef int s32;
extern s32 gOptionWords[];   /* bitmap A: one bit per index, 32 indices per word */
extern s32 D_00AAD624[];   /* bitmap B */

s32 indexBitTest(s16 bitIndex)
{
  s32 bitmapWordIndex;
  s32 bitMask;
  bitmapWordIndex = bitIndex;
  if (bitIndex < 0)
  {
    bitmapWordIndex = bitIndex + 0x1F;
  }
  bitMask = 1 << bitIndex;
  return (gOptionWords[bitmapWordIndex >> 5] & bitMask) != 0;
}

void indexBitSet(s16 bitIndex)
{
  s32 bitmapWordIndex;
  s32 *bitmapWordPtr;
  s32 updatedWord;
  bitmapWordIndex = bitIndex;
  if (bitIndex < 0)
  {
    bitmapWordIndex = bitIndex + 0x1F;
  }
  bitmapWordPtr = &gOptionWords[bitmapWordIndex >> 5];
  updatedWord = *bitmapWordPtr | (1 << bitIndex);
  *bitmapWordPtr = updatedWord;
}

s32 indexBitTestSecond(s16 bitIndex)
{
  s32 bitmapWordIndex;
  s32 bitMask;
  bitmapWordIndex = bitIndex;
  if (bitIndex < 0)
  {
    bitmapWordIndex = bitIndex + 0x1F;
  }
  bitMask = 1 << bitIndex;
  return (D_00AAD624[bitmapWordIndex >> 5] & bitMask) != 0;
}

s32 indexBitTestSecond(s16 bitIndex);

s32 indexBitRangeAllSet(s16 firstBit, s16 lastBit)
{
  s16 limit;
  int hasFirstBit;
  s32 bitIndex;
  s32 allBitsSet;

  limit = lastBit;

  hasFirstBit = firstBit != (-1);
  if (lastBit == (-1))
  {
    limit = 0x63;
  }
  bitIndex = 0;
  if (hasFirstBit)
  {
    bitIndex = firstBit;
  }
  if (limit >= bitIndex)
  {
    testNextBit:
    allBitsSet = indexBitTestSecond(bitIndex);

    bitIndex += 1;

    if (allBitsSet != 0)
    {
      if (limit < bitIndex)
      {
        goto allSetDone;
      }
      goto testNextBit;
    }
  }
  else
  {
    allSetDone:
    allBitsSet = 1;

  }
  return allBitsSet;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9D090);

void indexBitClearSecond(s16 bitIndex)
{
  s32 bitmapWordIndex;
  s32 *bitmapWordPtr;
  s32 updatedWord;
  s32 *secondBitmap;
  bitmapWordIndex = bitIndex;
  if (bitIndex < 0)
  {
    bitmapWordIndex = bitIndex + 0x1F;
  }
  secondBitmap = gOptionWords - -1;
  bitmapWordPtr = &secondBitmap[bitmapWordIndex >> 5];
  updatedWord = *bitmapWordPtr & (~(1 << bitIndex));
  *bitmapWordPtr = updatedWord;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9D128);

INCLUDE_ASM("asm/nonmatchings/ov04/tu013", func_00A9D1A0);
