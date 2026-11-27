#ifndef INCLUDE_MAIN_TU162_H
#define INCLUDE_MAIN_TU162_H

#include "shared.h"

typedef signed char s8;

typedef int s32;

/* Screen rectangle in GS coordinates: left/top corner and right/bottom corner. */
typedef struct FontRect {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
} FontRect;

/* Draws the tinted box of rect, then either its inner rect (blendMode set) or a sprite fill of the same rect.
   inset packs the x inset (high bits) and the y inset (low nibble). */
void xglFontDrawBoxBlend(FontRect *rect, int inset, u32 colour, u32 blendMode);

/* Same queue as fontQueueSpriteReset with another TEST record (ZTST 1, ALPHA 2 blend), an RGBAQ of 0 and one sprite. */
void fontQueueSpriteBlack(void);

void xglFontSetAlphaScale(s32 alphaScale);

#endif /* INCLUDE_MAIN_TU162_H */
