/* header_harvest: hand-maintained (owner header of main/tu173; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU173_H
#define INCLUDE_MAIN_TU173_H

/* Types owned by main/tu173 and shared with other TUs. */

#include "shared.h"

/* Screen position a sheet frame is drawn at (sheetFramePosition). */
typedef struct TiFramePos {
    u16 x;                          /* +0x00 */
    u16 y;                          /* +0x02 */
} TiFramePos;

/* Frame sheet header: a grid of frames drawn from a sheet's origin, with per-column cell widths. */
typedef struct TiFrameSheet {
    unsigned char unmodeled_00[0x0C - 0x00];
    signed char columns;            /* +0x0C */
    signed char rows;               /* +0x0D */
    u8 valueColumn;                 /* +0x0E: scales the x offset by 0x50 */
    unsigned char unmodeled_0F[0x10 - 0x0F];
    u16 originX;                    /* +0x10 */
    u16 originY;                    /* +0x12 */
    u16 cellWidthA;                 /* +0x14 */
    u16 cellWidthB;                 /* +0x16 */
    u16 cellWidthC;                 /* +0x18 */
    s16 entryCount;                 /* +0x1A: passed to the box draw */
    s16 frameBase;                  /* +0x1C: subtracted from the tick */
} TiFrameSheet;

#endif /* INCLUDE_MAIN_TU173_H */
