#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
extern unsigned char gOptionalBlocksBlob[];
extern u8 *gOptionalBlocksSource;
extern void *gOptionalSecondaryBlock;
extern void *gOptionalPrimaryBlock;
extern s8 gOptionalBlocksLoadState;
/* Optional sub-block offsets that xcLocatePayload returns for gOptionalBlocksBlob. */
typedef struct XcOptionalSubBlocks {
    s32 primaryOffset;   /* +0x00 */
    s32 hasPrimary;      /* +0x04 */
    s32 secondaryOffset; /* +0x08 */
    s32 hasSecondary;    /* +0x0C */
} XcOptionalSubBlocks;

s32 xcLocatePayload(XcOptionalSubBlocks **optionalBlocks, u8 *source);

void xcLoadOptionalBlockPointers(void)
{
  XcOptionalSubBlocks *optionalBlocks;
  u8 *optionalBase;
  u8 *primaryBlock;
  u8 *secondaryBlock;
  s32 hasSecondary;

  gOptionalBlocksSource = gOptionalBlocksBlob;
  xcLocatePayload(&optionalBlocks, gOptionalBlocksBlob);
  optionalBase = (u8 *) optionalBlocks;
  primaryBlock = 0;
  secondaryBlock = 0;
  if (optionalBlocks->hasPrimary != 0)
  {
    primaryBlock = optionalBase + optionalBlocks->primaryOffset;
  }
  hasSecondary = optionalBlocks->hasSecondary != 0;
  gOptionalPrimaryBlock = primaryBlock;
  if (hasSecondary)
  {
    secondaryBlock = optionalBase + optionalBlocks->secondaryOffset;
  }
  gOptionalSecondaryBlock = secondaryBlock;
  gOptionalBlocksLoadState = 3;
}

void recordClearEmpty(void) {

}

void recordFlushEmpty(void) {

}

s32 recordIsLoaded(void) {
    return gOptionalBlocksLoadState >= 3;
}

INCLUDE_ASM("asm/main/nonmatchings/tu165", func_002184E0);
