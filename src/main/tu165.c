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

typedef long s64;

typedef struct Position8 {
    s64 xy;
    s64 zw;
} Position8;

extern const Position8 D_00538EF0;
void xglMatrixStackUnit(void);
void xglMatrixStackTrans(Position8 *translation);
void xglMatrixStackRotX(float angle);
void xglMatrixStackSave(float *matrix);
void nmlModelSetDrawMatrix(float *matrix);
void nmlModelSetStencil(s32 stencil);
s32 xglFontGetAlphaScale(void);
void nmlModelSetTransparency(float transparency);
void nmlModelSetTexture(void *texture);
void pxySubmitModel(void *model);
void func_001AA580(void);
void fontQueueSpriteBlack(void);

/* Draws the optional model when the blocks are loaded: the primary block is the model, the secondary block its
 * texture, drawn with a half-turn spin of -0.6283 rad and alpha from the font fade. fontQueueSpriteBlack ends the frame. */
void optionalBlocksDraw(void) {
    float matrix[16];
    Position8 position = D_00538EF0;

    if (gOptionalBlocksLoadState == 3) {
        xglMatrixStackUnit();
        xglMatrixStackTrans(&position);
        xglMatrixStackRotX(-0.62831855f);
        xglMatrixStackSave(matrix);
        nmlModelSetDrawMatrix(matrix);
        nmlModelSetStencil(1);
        nmlModelSetTransparency(xglFontGetAlphaScale() * 0.0078125f);
        nmlModelSetTexture(gOptionalSecondaryBlock);
        pxySubmitModel(gOptionalPrimaryBlock);
        func_001AA580();
    }
    fontQueueSpriteBlack();
}

const Position8 D_00538EF0 = { 0xBF80000000000000, 0x3F80000041800000 };

unsigned char gOptionalBlocksBlob[45056] = { 0 };

u8 *gOptionalBlocksSource = 0;

void *gOptionalSecondaryBlock = 0;

void *gOptionalPrimaryBlock = 0;

s8 gOptionalBlocksLoadState = 0;
