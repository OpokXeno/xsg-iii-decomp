#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu263", func_002665A8);

INCLUDE_ASM("asm/main/nonmatchings/tu263", func_00266758);

INCLUDE_ASM("asm/main/nonmatchings/tu263", func_00266800);

typedef int s32;
typedef float f32;
typedef signed char s8;
typedef struct TiRingVertex {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} TiRingVertex;
f32 actorSlotGetRadius();
s32 actorSlotGetVector();
void tiRingEmitVertices(TiRingVertex *vertices, s32 *counter, void *offset, f32 radius);

void tiRingEmitMode2(TiRingVertex *vertices, s32 *emittedVertexCount, s32 actorSlot) {
    s32 offsetVector[4];
    f32 radius;

    radius = actorSlotGetRadius(actorSlot);
    if (actorSlotGetVector(&offsetVector[0], actorSlot, 2) != 0) {
        tiRingEmitVertices(vertices, emittedVertexCount, &offsetVector[0], radius);
        if (actorSlotGetVector(&offsetVector[0], actorSlot, 0) != 0) {
            tiRingEmitVertices(vertices, emittedVertexCount, &offsetVector[0], radius);
        }
    }
}

extern f32 gTiDegToRadians;
void *memset();
void xglMatrixStackMulVector();
void xglMatrixStackRotY(f32);
void xglMatrixStackUnit();

void tiRingEmitVertices(TiRingVertex *vertices, s32 *emittedVertexCount, void *offsetVector, f32 radius) {
    f32 ringPoint[4];
    f32 degToRad;
    s32 nextVertexCount;
    s32 angleStep;
    TiRingVertex *emittedVertex;

    angleStep = 0;
    memset(&ringPoint[0], 0, 0x10);
    ringPoint[2] = radius;
    degToRad = gTiDegToRadians;
    do {
        xglMatrixStackUnit();
        xglMatrixStackRotY(angleStep * 45.0f * degToRad);
        xglMatrixStackMulVector(&vertices[*emittedVertexCount], &ringPoint[0]);
        /* Element address as an integer, emittedVertexCount shift first (addu v0 = emittedVertexCount<<4, base);
           pointer arithmetic on vertices puts the base first, which is a different instruction order. */
        emittedVertex = (TiRingVertex *) ((*emittedVertexCount << 4) + (s32) vertices);
        __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(emittedVertex) : "memory");
        __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(offsetVector) : "memory");
        __asm__ __volatile__("vadd.xyzw vf20, vf20, vf21" : : : "memory");
        __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(emittedVertex) : "memory");
        angleStep += 1;
        nextVertexCount = *emittedVertexCount + 1;
        *emittedVertexCount = nextVertexCount;
    } while (angleStep < 8);
}

void tiRingEmitMode1(TiRingVertex *vertices, s32 *emittedVertexCount, s32 actorSlot) {
    s32 offsetVector[4];
    f32 radius;

    radius = actorSlotGetRadius(actorSlot);
    if (actorSlotGetVector(&offsetVector[0], actorSlot, 1) != 0) {
        tiRingEmitVertices(vertices, emittedVertexCount, &offsetVector[0], radius);
        if (actorSlotGetVector(&offsetVector[0], actorSlot, 0) != 0) {
            tiRingEmitVertices(vertices, emittedVertexCount, &offsetVector[0], radius);
        }
    }
}
