#include "common.h"

typedef int s32;
typedef float f32;
typedef unsigned short u16;
s32 func_00285508();

s32 hasMotionSolverWork(void) {
    return func_00285508() != 0;
}

typedef struct Vector4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vector4;

s32 isSquaredDistanceOrdered(Vector4 *origin, Vector4 *pointA, Vector4 *pointB, Vector4 *pointC) {
    f32 squaredDistances[4];

    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(origin) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(pointA) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(pointB) : "memory");
    __asm__ __volatile__("lqc2 vf4, 0(%0)" : : "r"(pointC) : "memory");
    __asm__ __volatile__("vsub.xyz vf12, vf2, vf1" : : : "memory");
    __asm__ __volatile__("vsub.xyz vf13, vf4, vf1" : : : "memory");
    __asm__ __volatile__("vsub.xyz vf14, vf3, vf1" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf18, vf12, vf12" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf19, vf13, vf13" : : : "memory");
    __asm__ __volatile__("vmul.xyz vf20, vf14, vf14" : : : "memory");
    __asm__ __volatile__("vaddz.x vf18, vf18, vf18z" : : : "memory");
    __asm__ __volatile__("vaddz.x vf19, vf19, vf19z" : : : "memory");
    __asm__ __volatile__("vaddz.x vf20, vf20, vf20z" : : : "memory");
    __asm__ __volatile__("vaddx.z vf18, vf0, vf19x" : : : "memory");
    __asm__ __volatile__("vaddx.y vf18, vf0, vf20x" : : : "memory");
    __asm__ __volatile__("sqc2 vf18, 0(%0)" : : "r"(squaredDistances) : "memory");
    if (!(!(squaredDistances[0] < squaredDistances[2]) || !(squaredDistances[1] < squaredDistances[0]))) {
        return 1;
    }
    return 0;
}

s32 isSquaredDistanceAtLeast(Vector4 *origin, Vector4 *pointA, Vector4 *pointB)
{
  f32 squaredDistances[4];
  __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(origin) : "memory");
  __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(pointA) : "memory");
  __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(pointB) : "memory");
  __asm__ __volatile__("vsub.xyz vf12, vf2, vf1" : : : "memory");
  __asm__ __volatile__("vsub.xyz vf13, vf3, vf1" : : : "memory");
  __asm__ __volatile__("vmul.xyz vf18, vf12, vf12" : : : "memory");
  __asm__ __volatile__("vmul.xyz vf19, vf13, vf13" : : : "memory");
  __asm__ __volatile__("vaddz.x vf18, vf18, vf18z" : : : "memory");
  __asm__ __volatile__("vaddz.x vf19, vf19, vf19z" : : : "memory");
  __asm__ __volatile__("vaddx.z vf18, vf0, vf19x" : : : "memory");
  __asm__ __volatile__("sqc2 vf18, 0(%0)" : : "r"(squaredDistances) : "memory");
  if (squaredDistances[0] >= squaredDistances[2])
  {
    return 1;
  }
  return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00284B58);

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00284C60);

extern u16 gObjectFlagMask[];

/* The per-object motion record: the angle pair (sin, cos), flags and the extent of the object. */
typedef struct MotionRecord {
    unsigned char unk_00[0x4];
    f32 center_y;           /* 0x04: vertical centre of the object's extent box */
    unsigned char unk_08[0x18];
    f32 half_angle_weight;  /* 0x20: cos(angle / 2) + 1 */
    f32 angle;              /* 0x24 */
    f32 sin_value;          /* 0x28 */
    f32 cos_value;          /* 0x2C */
    f32 extent;             /* 0x30 */
    s32 objectFlagBits;              /* 0x34 */
    s32 motionState;              /* 0x38 */
    s32 cleared;            /* 0x3C */
} MotionRecord;

s32 isObjectFlagged(MotionRecord *motionRecord) {
    return (gObjectFlagMask[0] & motionRecord->objectFlagBits) != 0;
}

s32 isCoordinateInRange(MotionRecord *motionRecord, Vector4 *point)
{
  f32 center;
  f32 radius;
  f32 coord;
  center = motionRecord->center_y;
  radius = motionRecord->extent;
  coord = point->y;
  if ((center + radius) < coord)
  {
    return 0;
  }
  if (!(coord < (center - radius)))
  {
    return 1;
  }
  return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00284E30);

void motionRecordSetState(MotionRecord *motionRecord, s32 motionState) {
    motionRecord->cleared = 0;
    motionRecord->motionState = motionState;
}

void motionRecordSetFlags(MotionRecord *motionRecord, s32 objectFlagBits) {
    motionRecord->objectFlagBits = objectFlagBits;
}

f32 xglCos(f32);

void setRotationAngle(MotionRecord *motionRecord, f32 angle) {
    motionRecord->angle = angle;
    motionRecord->half_angle_weight = xglCos(angle * 0.5f) + 1.0f;
}

f32 xglSin();

void setSinCosFields(MotionRecord *motionRecord, f32 angle) {
    motionRecord->sin_value = xglSin();
    motionRecord->cos_value = xglCos(angle);
}

void motionRecordSetExtent(MotionRecord *motionRecord, f32 extent) {
    motionRecord->extent = extent;
}

extern s32 gScenePair[];
s32 sceneBit1Get();

void storeValueWithSceneBit(s32 sceneValue) {
    s32 secondary_value;
    s32 primary_value;

    primary_value = sceneValue;
    gScenePair[0] = primary_value;
    secondary_value = sceneBit1Get();
    gScenePair[1] = secondary_value;
}

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00284F50);

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00285508);

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00285720);

void projectPointOntoSegment(Vector4 *projected, Vector4 *segmentEndpointA, Vector4 *segmentEndpointB, Vector4 *point)
{
  f32 t;
  f32 point_y;
  f32 delta_x;
  f32 proj_x;
  f32 delta_z;
  f32 proj_z;
  f32 start_z;
  f32 start_x;
  start_x = segmentEndpointA->x;
  delta_x = segmentEndpointB->x - start_x;
  start_z = segmentEndpointA->z;
  delta_z = segmentEndpointB->z - start_z;
  t = ((delta_x * (point->x - start_x)) + (delta_z * (point->z - start_z))) / ((delta_x * delta_x) + (delta_z * delta_z));
  proj_x = (delta_x * t) + start_x;
  projected->x = proj_x;
  proj_z = (delta_z * t) + segmentEndpointA->z;
  projected->z = proj_z;
  point_y = point->y;
  projected->w = 1.0f;
  projected->y = point_y;
}

INCLUDE_ASM("asm/main/nonmatchings/tu303", func_00285860);
