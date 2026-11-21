/*
 * OV04 original TU 10: 0x00a9a640..0x00a9a8d8 (4 functions)
 */
#include "common.h"

typedef short s16;
typedef int s32;
typedef float f32;

typedef struct LineSegment {
    f32 startX;
    f32 startY;
    f32 startZ;
    f32 endX;
    f32 endY;
    f32 endZ;
} LineSegment;

typedef struct EdgeTriple {
    LineSegment lineSegments[3];
} EdgeTriple;

typedef struct EdgeEndpointVertex {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} EdgeEndpointVertex;

extern EdgeTriple gEdgeTripleTable[];
extern EdgeEndpointVertex gEdgeVertexBuffers[];

INCLUDE_ASM("asm/nonmatchings/ov04/tu010", func_00A9A640);

INCLUDE_ASM("asm/nonmatchings/ov04/tu010", func_00A9A768);

INCLUDE_ASM("asm/nonmatchings/ov04/tu010", func_00A9A810);

void segmentTableLoadPoints(s16 edgeTripleIndex) {
    f32 startX;
    f32 startY;
    f32 startZ;
    f32 endX;
    f32 endY;
    f32 endZ;
    EdgeEndpointVertex *vertexOut;
    LineSegment *edgeSegment;
    s32 i;

    vertexOut = gEdgeVertexBuffers;
    edgeSegment = gEdgeTripleTable[edgeTripleIndex].lineSegments;
    for (i = 0; i < 3; i++) {
        startX = edgeSegment->startX;
        startY = edgeSegment->startY;
        startZ = edgeSegment->startZ;
        endX = edgeSegment->endX;
        endY = edgeSegment->endY;
        endZ = edgeSegment->endZ;
        edgeSegment++;
        vertexOut->x = startX;
        vertexOut->y = startY;
        vertexOut->z = startZ;
        vertexOut[7].x = endX;
        vertexOut[7].y = endY;
        vertexOut[7].z = endZ;
        vertexOut->w = 1.0f;
        vertexOut[7].w = 1.0f;
        vertexOut++;
    }
}
