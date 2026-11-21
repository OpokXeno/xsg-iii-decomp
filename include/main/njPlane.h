#ifndef INCLUDE_MAIN_NJPLANE_H
#define INCLUDE_MAIN_NJPLANE_H

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

void njPlaneSetTint(NjPlane *plane, f32 r, f32 g, f32 b);

#endif /* INCLUDE_MAIN_NJPLANE_H */
