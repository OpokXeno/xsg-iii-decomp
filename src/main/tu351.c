#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
typedef float f32;
extern unsigned char gMapCollisionObjectStorage[];
s32 func_002A6FA8();
void func_002A70E8();

/* The collision object at gMapCollisionObjectStorage: the MapCollisionObject layout of include/main/tu352.h, only the fields this file writes. */
typedef struct MapCollisionObjectView {
    unsigned char unmodeled_00[0xA];
    unsigned short bits;                /* +0x0A: the attribute mask selecting what a probe hits */
    unsigned char unmodeled_0C[0x1C - 0xC];
    s32 collisionBlock;                 /* +0x1C: the handle func_002A6FA8 returns (MapCollisionObject.block) */
} MapCollisionObjectView;

#define gMapCollisionObject (*(MapCollisionObjectView *) gMapCollisionObjectStorage)

void mapProbeInitDefault(void) {
    s32 collisionBlock;

    func_002A70E8(gMapCollisionObjectStorage);
    gMapCollisionObject.bits = 0x4800;
    collisionBlock = func_002A6FA8(0xA00, -1);
    gMapCollisionObject.collisionBlock = collisionBlock;
}

INCLUDE_ASM("asm/main/nonmatchings/tu351", func_002A62E8);

/* A point in world space; xglSegmentProjectRatio reads x and z only. */
typedef struct SegmentPoint {
  f32 x;                                /* +0x0 */
  f32 y;                                /* +0x4 */
  f32 z;                                /* +0x8 */
} SegmentPoint;

/* Ratio t in 0..1 of the projection of point onto the segment start -> end (x and z at +0 and +8). */
f32 xglSegmentProjectRatio(SegmentPoint *segmentStartPoint, SegmentPoint *segmentEndPoint, SegmentPoint *point)
{
  f32 startX;
  f32 startZ;
  f32 dx;
  f32 dz;
  f32 segmentRatio;
  startX = segmentStartPoint->x;
  startZ = segmentStartPoint->z;
  dx = segmentEndPoint->x - startX;
  dz = segmentEndPoint->z - startZ;
  segmentRatio = ((dx * (point->x - startX)) + (dz * (point->z - startZ))) / ((dx * dx) + (dz * dz));
  if (segmentRatio < 0.0f)
  {
    segmentRatio = 0.0f;
  }
  if (segmentRatio > 1.0f)
  {
    segmentRatio = 1.0f;
  }
  return segmentRatio;
}

void mapProbeInitTimeout(s32 timeout) {
    s32 collisionBlock;

    func_002A70E8(gMapCollisionObjectStorage);
    gMapCollisionObject.bits = 0x4800;
    collisionBlock = func_002A6FA8(0xA00, timeout);
    gMapCollisionObject.collisionBlock = collisionBlock;
}

INCLUDE_ASM("asm/main/nonmatchings/tu351", func_002A64D8);
