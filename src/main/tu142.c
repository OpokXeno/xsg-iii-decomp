#include "common.h"

#include "shared.h"
/* Axis and direction of the segment helpers below: 0 is x, 2 is z; the sign is +1 or -1. */
extern int gSegmentAxisIndex;
extern int gSegmentAxisSign;

/* Picks the segment axis (z when the two ends differ in z, else x) and the sign of the direction along it. */
void xglSegmentSelectAxis(Vector4 *segmentPoints) {
    if (segmentPoints[0].z != segmentPoints[1].z) {
        gSegmentAxisIndex = 2;
        if (segmentPoints[1].z < segmentPoints[0].z) {
            gSegmentAxisSign = 1;
        } else {
            gSegmentAxisSign = -1;
        }
    } else {
        gSegmentAxisIndex = 0;
        if (segmentPoints[1].x < segmentPoints[0].x) {
            gSegmentAxisSign = 1;
        } else {
            gSegmentAxisSign = -1;
        }
    }
}

extern float xglPointLength(Vector4 *a, Vector4 *b);
extern void xglVectorNormal(Vector4 *pickedPoint, Vector4 *in);
extern void xglPointScaleOffsetAlongSegment(Vector4 *pickedPoint, Vector4 *start, Vector4 *dir, Vector4 *origin);
extern void xglVectorScaleAddXYZ(Vector4 *destination, const Vector4 *source, float scale, const Vector4 *addend);

typedef union AxisPoint {
    Vector4 vector;
    float axis[4];
} AxisPoint;

#define POINT_AXIS(point) (((AxisPoint *) &(point))->axis[gSegmentAxisIndex])
#define VECTOR_AXIS(point) ((point).axis[gSegmentAxisIndex])

/* Walks the point pairs (points[i - 1], points[i]). For the first pair whose segment is hit by the sphere of
   radius around origin, writes the hit that lies on the segment to pickedPoint and returns 1. */
int xglSegmentSphereHit(Vector4 *pickedPoint, Vector4 *points, int pointCount, Vector4 *origin, float radius) {
    Vector4 *segmentEnds[2];
    Vector4 direction;
    AxisPoint footPoint;
    AxisPoint hitPoints[2];
    int hitOnSegment[2];
    int i;
    float hitOffset;

    for (i = 1; i < pointCount; i += 2) {
        segmentEnds[0] = &points[i - 1];
        segmentEnds[1] = &points[i];
        __asm__ __volatile__("lqc2 $vf20, 0(%1)\n"
                             "lqc2 $vf21, 0(%2)\n"
                             "vsub.xyzw $vf20, $vf20, $vf21\n"
                             "sqc2 $vf20, 0(%0)\n"
                             :
                             : "r"(&direction), "r"(segmentEnds[1]), "r"(segmentEnds[0])
                             : "memory");
        xglVectorNormal(&direction, &direction);
        xglPointScaleOffsetAlongSegment(&footPoint.vector, segmentEnds[0], &direction, origin);
        /* First the distance from origin to the foot, then the offset of the hits from the foot. */
        hitOffset = xglPointLength(&footPoint.vector, origin);
        if (hitOffset < radius) {
            hitOffset = __builtin_sqrtf(radius * radius - hitOffset * hitOffset);
        } else {
            hitOffset = 1.0e-4f;
        }
        xglVectorScaleAddXYZ(&hitPoints[0].vector, &direction, hitOffset, &footPoint.vector);
        xglVectorScaleAddXYZ(&hitPoints[1].vector, &direction, -hitOffset, &footPoint.vector);
        hitOnSegment[0] = (POINT_AXIS(*segmentEnds[0]) <= VECTOR_AXIS(hitPoints[0]) &&
                           VECTOR_AXIS(hitPoints[0]) <= POINT_AXIS(*segmentEnds[1])) ||
                          (POINT_AXIS(*segmentEnds[1]) <= VECTOR_AXIS(hitPoints[0]) &&
                           VECTOR_AXIS(hitPoints[0]) <= POINT_AXIS(*segmentEnds[0]));
        if (hitOnSegment[0] && (VECTOR_AXIS(hitPoints[0]) - VECTOR_AXIS(footPoint)) * gSegmentAxisSign <= 0.0f) {
            hitOnSegment[0] = 0;
        }
        if (hitOnSegment[0] && (VECTOR_AXIS(hitPoints[0]) - POINT_AXIS(*origin)) * gSegmentAxisSign <= 0.0f) {
            hitOnSegment[0] = 0;
        }
        hitOnSegment[1] = (POINT_AXIS(*segmentEnds[0]) <= VECTOR_AXIS(hitPoints[1]) &&
                           VECTOR_AXIS(hitPoints[1]) <= POINT_AXIS(*segmentEnds[1])) ||
                          (POINT_AXIS(*segmentEnds[1]) <= VECTOR_AXIS(hitPoints[1]) &&
                           VECTOR_AXIS(hitPoints[1]) <= POINT_AXIS(*segmentEnds[0]));
        if (hitOnSegment[1] && (VECTOR_AXIS(hitPoints[1]) - VECTOR_AXIS(footPoint)) * gSegmentAxisSign <= 0.0f) {
            hitOnSegment[1] = 0;
        }
        if (hitOnSegment[1] && (VECTOR_AXIS(hitPoints[1]) - POINT_AXIS(*origin)) * gSegmentAxisSign <= 0.0f) {
            hitOnSegment[1] = 0;
        }
        if (hitOnSegment[0]) {
            *pickedPoint = hitPoints[0].vector;
            return 1;
        }
        if (hitOnSegment[1]) {
            *pickedPoint = hitPoints[1].vector;
            return 1;
        }
    }
    return 0;
}

/* Returns the point whose distance to origin is closest to targetDistance among the points on the axis side of origin
   (on a tie the later point wins); pickedPoint gets it and the result is 1, or 0 when no point qualifies. */
int xglPointListFindDistance(Vector4 *pickedPoint, Vector4 *points, int pointCount, Vector4 *origin, float targetDistance) {
    int bestIndex = -1;
    float bestError = 99999.0f;
    float *originFloats = &origin->x;
    int i;

    for (i = 0; i < pointCount; i++) {
        float *pointFloats = &points[i].x;
        float *pointAxis = pointFloats + gSegmentAxisIndex;
        float *originAxis = originFloats + gSegmentAxisIndex;
        float axisDelta = (*pointAxis - *originAxis) * gSegmentAxisSign;
        float pointDistance;
        float distanceError;

        if (axisDelta < 0.0f) {
            continue;
        }
        pointDistance = xglPointLength(origin, &points[i]);
        distanceError = __builtin_fabsf(pointDistance - targetDistance);
        if (bestError < distanceError) {
            continue;
        }
        bestError = distanceError;
        bestIndex = i;
    }
    if (bestIndex == -1) {
        return 0;
    }
    pickedPoint[0] = points[bestIndex];
    return 1;
}

/* Returns the point with the largest signed value on the segment axis into pickedPoint (result 1), or 0 when there
   is no point. */
int xglPointListFindAxisExtreme(Vector4 *pickedPoint, Vector4 *points, int pointCount) {
    int bestIndex = -1;
    float bestValue = -99999.0f;
    int i;

    for (i = 0; i < pointCount; i++) {
        float *pointFloats = &points[i].x;
        float axisValue = pointFloats[gSegmentAxisIndex] * gSegmentAxisSign;

        if (axisValue < bestValue) {
            continue;
        }
        bestValue = axisValue;
        bestIndex = i;
    }
    if (bestIndex == -1) {
        return 0;
    }
    *pickedPoint = points[bestIndex];
    return 1;
}
