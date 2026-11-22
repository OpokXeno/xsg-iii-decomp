#include "common.h"

typedef float f32;

void *memset(void *destination, int value, unsigned int size);
void xglMatrixUnit(f32 *matrix);
void xglMatrixRotX(f32 *matrix, f32 *source, f32 angle);
void xglMatrixRotY(f32 *matrix, f32 *source, f32 angle);
void xglVectorMulMat(f32 *destination, f32 *matrix, f32 *vector);

void xglPointAddRotatedOffset(void *resultPoint, void *basePoint, f32 *angles, f32 offsetZ) {
    f32 unrotatedOffset[4];
    f32 rotatedOffset[4];
    f32 rotation[0x10];
    f32 rotationY[0x10];

    memset(&unrotatedOffset[0], 0, 0x10);
    unrotatedOffset[2] = offsetZ;
    xglMatrixUnit(&rotation[0]);
    xglMatrixRotY(&rotationY[0], &rotation[0], angles[1]);
    xglMatrixRotX(&rotation[0], &rotationY[0], angles[0]);
    xglVectorMulMat(&rotatedOffset[0], &rotation[0], &unrotatedOffset[0]);
    __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(basePoint) : "memory");
    __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(&rotatedOffset[0]) : "memory");
    __asm__ __volatile__("vadd.xyzw vf20, vf20, vf21" : : : "memory");
    __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(resultPoint) : "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/tu247", func_00260550);

f32 xglPointLength(f32 *segmentStartPoint, f32 *segmentEndPoint);
void xglVectorInner(f32 *destination, f32 *left, f32 *right);
void xglVectorNormal(f32 *destination, f32 *source);
void xglVectorScaleAddXYZ(f32 *destination, f32 *first, f32 *addend, f32 scale);

void xglPointScaleOffsetAlongSegment(f32 *resultPoint, f32 *segmentStartPoint, f32 *offsetVector, f32 *segmentEndPoint) {
    f32 direction[4];
    f32 offsetAlongSegment;
    f32 segmentLength;

    __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(segmentEndPoint) : "memory");
    __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(segmentStartPoint) : "memory");
    __asm__ __volatile__("vsub.xyzw vf20, vf20, vf21" : : : "memory");
    __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(direction) : "memory");
    xglVectorNormal(&direction[0], &direction[0]);
    segmentLength = xglPointLength(segmentStartPoint, segmentEndPoint);
    xglVectorInner(&offsetAlongSegment, offsetVector, &direction[0]);
    xglVectorScaleAddXYZ(resultPoint, offsetVector, segmentStartPoint, segmentLength * offsetAlongSegment);
}
