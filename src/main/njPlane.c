#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef signed char s8;
typedef s32 s32;

/* Plane record: the 0x80-byte block this TU initialises; func_00196200 takes the sub-record at +0x80. */
typedef struct NjPlane {
    f32 offset_x;           /* 0x00 */
    f32 offset_y;           /* 0x04 */
    f32 tint[3];            /* 0x08, passed as a vector to func_002C6F78 */
    f32 level;              /* 0x14, clamped to 0..127 */
    f32 origin_x;           /* 0x18 */
    f32 origin_y;           /* 0x1C */
    s32 color;              /* 0x20 */
    s32 mode;               /* 0x24 */
    s32 bounds[4];          /* 0x28..0x37 */
    u8 unk_38[0x48];
} NjPlane;

/* Stack buffer for one plane and the sub-record that njPlaneInit passes on (the 0xC0-byte block). */
typedef struct NjPlaneBuffer {
    NjPlane plane;
    u8 planeMatrix[0x40];    /* +0x80 */
} NjPlaneBuffer;

extern unsigned char gNjPlaneAssertCondition[];
extern unsigned char gNjPlaneSourceFileName[];
extern unsigned char gNjPlaneInitAssertMessage[];
s32 func_001936A8();
s32 func_00196200(void *planeMatrix);
void njPlaneSetOffset(NjPlane *plane, f32 x, f32 y);
void njPlaneSetColor(NjPlane *plane, s32 color);
void njPlaneSetOrigin(NjPlane *plane, f32 x, f32 y);
void njPlaneSetTint(NjPlane *plane, f32 r, f32 g, f32 b);
void njPlaneSetLevel(NjPlane *plane, f32 level);
void njPlaneSetMode(NjPlane *plane, s32 mode);

void njPlaneInit(NjPlane *plane) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x22D, gNjPlaneInitAssertMessage);
    }
    njPlaneSetOffset(plane, 0.0f, 0.0f);
    njPlaneSetColor(plane, 0xFFFF00);
    njPlaneSetOrigin(plane, 0.0f, 0.0f);
    njPlaneSetTint(plane, 128.0f, 128.0f, 128.0f);
    njPlaneSetLevel(plane, 128.0f);
    njPlaneSetMode(plane, 0);
    func_00196200(plane + 1);
}

extern unsigned char gNjPlaneOffsetColorAssertMessage[];

void njPlaneSetOffset(NjPlane *plane, f32 x, f32 y) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x242, gNjPlaneOffsetColorAssertMessage);
    }
    plane->offset_x = x;
    plane->offset_y = y;
}

void njPlaneSetColor(NjPlane *plane, s32 color) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x252, gNjPlaneOffsetColorAssertMessage);
    }
    plane->color = color;
}

extern unsigned char gNjPlaneSetOriginAssertMessage[];

void njPlaneSetOrigin(NjPlane *plane, f32 x, f32 y) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x261, gNjPlaneSetOriginAssertMessage);
    }
    plane->origin_x = x;
    plane->origin_y = y;
}

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002CFC78);

extern unsigned char gNjPlaneSetTintAssertMessage[];
s32 func_002C6F78(f32 *tint, s32 r, s32 g, s32 b);

void njPlaneSetTint(NjPlane *plane, f32 r, f32 g, f32 b) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x282, gNjPlaneSetTintAssertMessage);
    }
    func_002C6F78(plane->tint, r, g, b);
}

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002CFDD0);

extern unsigned char gNjPlaneSetLevelAssertMessage[];

void njPlaneSetLevel(NjPlane *plane, f32 level) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x2A5, gNjPlaneSetLevelAssertMessage);
    }
    if (level < 0.0f) {
        level = 0.0f;
    }
    if (level > 127.0f) {
        level = 127.0f;
    }
    plane->level = level;
}

extern unsigned char gNjPlaneSetModeAssertMessage[];

void njPlaneSetMode(NjPlane *plane, s32 mode) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x2B8, gNjPlaneSetModeAssertMessage);
    }
    plane->mode = mode;
}

extern unsigned char gNjPlaneSetBoundsAssertMessage[];

void njPlaneSetBounds(NjPlane *plane, s32 bound0, s32 bound1, s32 bound2, s32 bound3) {
    if (plane == 0) {
        func_001936A8(gNjPlaneAssertCondition, gNjPlaneSourceFileName, 0x2C7, gNjPlaneSetBoundsAssertMessage);
    }
    plane->bounds[0] = bound0;
    plane->bounds[1] = bound1;
    plane->bounds[2] = bound2;
    plane->bounds[3] = bound3;
}

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002CFFF0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D04F0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D0828);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D0D88);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D0F50);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D10D0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D1358);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D15E8);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D17D0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D19C8);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D1D10);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D2000);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D22E0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D24C0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D26D0);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D2920);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D2AB8);

extern u8 *D_004C8390[];
extern s32 D_004C83D4[];
extern unsigned char func_002D2FA8[];

void njPlaneSetHook(s32 installHook)
{
  u8 **hookTable;
  if (installHook != 0)
  {
    hookTable = D_004C8390;
    if (hookTable[0x44 / 4] != func_002D2FA8)
    {
      hookTable[0x44 / 4] = func_002D2FA8;
    }
  }
  else
  {
    D_004C83D4[0] = 0;
  }
}

extern s32 gNjPlaneRequestedRange;
extern s32 gNjPlaneClampedRange;
extern s32 gNjPlaneRangeExtent;

void njPlaneSetClampedRange(s32 requestedRange, s32 rangeLimit, s32 rangeExtent) {
    gNjPlaneRequestedRange = requestedRange;
    gNjPlaneRangeExtent = rangeExtent;
    gNjPlaneClampedRange = requestedRange;
    if (requestedRange >= rangeLimit) {
        gNjPlaneClampedRange = rangeLimit;
    }
}

void njPlaneSetRange(s32 requestedRange, s32 rangeExtent) {
    gNjPlaneRequestedRange = requestedRange;
    gNjPlaneRangeExtent = rangeExtent;
}

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D2C78);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D2FA8);

void njPlaneInit(NjPlane *plane);
s32 func_002CFFF0();

void njPlaneSpawnPair(s32 baseX, s32 baseY, s32 layer) {
    NjPlaneBuffer firstPlane;
    NjPlaneBuffer secondPlane;
    f32 x;
    f32 y;

    njPlaneInit(&firstPlane.plane);
    if (layer < 0x10) {
        x = baseX;
        y = baseY;
        njPlaneSetOffset(&firstPlane.plane, x, baseY - 4);
    } else {
        x = baseX;
        y = baseY;
        njPlaneSetOffset(&firstPlane.plane, x, y);
    }
    njPlaneSetOrigin(&firstPlane.plane, 12.0f, 12.0f);
    njPlaneInit(&secondPlane.plane);
    njPlaneSetOffset(&secondPlane.plane, x, y);
    njPlaneSetOrigin(&secondPlane.plane, 12.0f, 12.0f);
    func_002CFFF0(&firstPlane.plane, 0x87, 0);
    func_002CFFF0(&secondPlane.plane, 0x88, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D3548);

INCLUDE_ASM("asm/main/nonmatchings/njPlane", func_002D36E0);
