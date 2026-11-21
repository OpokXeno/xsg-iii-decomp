#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu192", func_0023F860);

typedef short s16;
typedef int s32;
typedef float f32;
extern f32 D_0054F604;
extern f32 D_0054F608;
extern f32 D_0054F60C;
s32 func_0023F860(s16 *, s32, s32, f32);

/* Two 16-bit screen coordinates passed to func_0023F860 as a pair. */
typedef struct ShortPoint {
    s16 x; /* 0x0 */
    s16 y; /* 0x2 */
} ShortPoint;

void drawScreenPointLayers(s32 x, s32 y, s32 drawFlags) {
    ShortPoint point;
    s32 lowDrawFlags;

    lowDrawFlags = drawFlags & 0xFFFF;
    point.x = x;
    point.y = y;
    func_0023F860(&point.x, lowDrawFlags, 0, 0.0f);
    func_0023F860(&point.x, lowDrawFlags, 0, D_0054F604);
    func_0023F860(&point.x, lowDrawFlags, 0, D_0054F608);
    func_0023F860(&point.x, lowDrawFlags, 0, D_0054F60C);
}

INCLUDE_ASM("asm/main/nonmatchings/tu192", func_0023FC08);
