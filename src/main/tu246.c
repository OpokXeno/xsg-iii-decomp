#include "common.h"

#include "shared.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))
typedef float f32;

#define ANGLE_PI 3.1415904f
#define ANGLE_TAU 6.2831807f

/* Wraps the x and y angles of pos into [-pi, pi] by steps of 2*pi; called from the camera code of tu244. */
void cameraAnglesWrapPi(f32 *pos) {
    f32 angleX;
    f32 angleY;
    f32 xRunning;
    f32 xStepped;
    f32 yRunning;
    f32 yStepped;

    angleX = pos[0];
    xRunning = angleX;
    if (ANGLE_PI < angleX) {
        do {
            xStepped = xRunning - ANGLE_TAU;
            xRunning = xStepped;
        } while (ANGLE_PI < xStepped);
        angleX = xStepped;
        pos[0] = angleX;
    }
    xRunning = angleX;
    if (angleX < -ANGLE_PI) {
        do {
            xStepped = xRunning + ANGLE_TAU;
            xRunning = xStepped;
        } while (xStepped < -ANGLE_PI);
        angleX = xStepped;
        pos[0] = angleX;
    }
    angleY = pos[1];
    yRunning = angleY;
    if (ANGLE_PI < angleY) {
        do {
            yStepped = yRunning - ANGLE_TAU;
            yRunning = yStepped;
        } while (ANGLE_PI < yStepped);
        angleY = yStepped;
        pos[1] = angleY;
    }
    yRunning = angleY;
    if (angleY < -ANGLE_PI) {
        do {
            yStepped = yRunning + ANGLE_TAU;
            yRunning = yStepped;
        } while (yStepped < -ANGLE_PI);
        angleY = yStepped;
        pos[1] = angleY;
    }
}

typedef int s32;

static unsigned char gMapProbeObjectStorage[48];

/* The probe object at gMapProbeObjectStorage, the MapCollisionObject layout of include/main/tu352.h; only the fields this file writes are claimed. */
typedef struct MapProbeObject {
    s32 hitRecord;                         /* +0x00: MapRecord * (0 for none) */
    s32 previousRecord;                 /* +0x04: MapRecord * */
    unsigned char unmodeled_08[0xA - 0x8];
    unsigned short bits;                /* +0x0A: the attribute mask selecting what a probe hits */
    unsigned char unmodeled_0C[0x14 - 0xC];
    float heightTolerance;              /* +0x14 */
    unsigned char unmodeled_18[0x1C - 0x18];
    s32 probeBlocks[(48 - 0x1C) / 4];    /* +0x1C: the block slots that follow the probe object */
} MapProbeObject;

#define gMapProbeObject (*(MapProbeObject *) gMapProbeObjectStorage)

s32 func_002A70E8();

void mapProbeObjectInit(void) {
    func_002A70E8(gMapProbeObjectStorage);
    gMapProbeObject.previousRecord = 0;
    gMapProbeObject.bits = 0x803;
    gMapProbeObject.heightTolerance = 1000.0f;
    gMapProbeObject.hitRecord = 0;
}

s32 func_002A6FA8();

/* Allocates the 512-byte probe block for recordId through func_002A6FA8 and stores it in gMapProbeObject.probeBlocks[0]; returns 0 when none is granted. */
s32 mapProbeAllocBlock(void *recordId) {
    s32 probeBlock = func_002A6FA8(512, recordId);

    if (probeBlock == 0) {
        return 0;
    }
    gMapProbeObject.probeBlocks[0] = probeBlock;
    return 1;
}

s32 func_002755E8();

/* Tests the probe segment against gMapProbeObject. It moves both ends by -0.001 and tests; with fewer than two hits it moves them to +0.001 and tests again. Returns the hit count, or 0. */
s32 mapProbeSegmentHits(void *hitBuffer, Vector4 *probeSegment, void *hitLimit) {
    s32 hits;

    probeSegment[0].x -= 0.001f;
    probeSegment[1].x -= 0.001f;
    hits = func_002755E8(probeSegment, probeSegment + 1, gMapProbeObjectStorage, hitBuffer, hitLimit);
    if (hits < 2) {
        probeSegment[0].x += 0.002f;
        probeSegment[1].x += 0.002f;
        hits = func_002755E8(probeSegment, probeSegment + 1, gMapProbeObjectStorage, hitBuffer, hitLimit);
        if (hits < 2) {
            return 0;
        }
    }
    return hits;
}

typedef struct PointRange {
    unsigned char unmodeled_00[0x4];
    s32 pointsBegin;
    s32 pointsEnd;
} PointRange;

f32 sinf(f32);

/* Copies into nearestPoint the point of the record's point list whose x is closest to the centre-based line at angle. Returns 0 when the record is not loaded or the list is empty. */
s32 mapProbeNearestPoint(Vector4 *nearestPoint, void *recordId, Vector4 *centre, f32 angle) {
    PointRange *pointRange;
    s32 pointBlock;
    Vector4 *firstPoint;
    Vector4 *pointCursor;
    s32 pointCount;
    s32 i;
    s32 nearestIndex;
    f32 minDistance;
    f32 distance;

    nearestIndex = 0;
    minDistance = 99999.0f;
    pointBlock = func_002A6FA8(512, recordId);
    if (pointBlock == 0) {
        return 0;
    }
    pointRange = (PointRange *) pointBlock;
    firstPoint = (Vector4 *) pointRange->pointsBegin;
    pointCount = ((s32) pointRange->pointsEnd - (s32) firstPoint) >> 4;
    i = 0;
    if (pointCount > 0) {
        pointCursor = firstPoint;
        for (; i < pointCount; i++) {
            distance = __builtin_fabsf(centre->x + sinf(angle) * (pointCursor->z - centre->z) - pointCursor->x);
            /* kept: the empty then-branch is the shape the match needs. */
            if (minDistance <= distance) {
            } else {
                minDistance = distance;
                nearestIndex = i;
            }
            pointCursor++;
        }
    }
    if (minDistance == 99999.0) {
        return 0;
    }
    *nearestPoint = firstPoint[nearestIndex];
    return 1;
}

f32 cosf(f32);

/* Fills two points 1000 units from centre along the direction at angle (the sin and cos of angle), with w = 1. */
void mapProbeSegmentEndpoints(Vector4 *points, const Vector4 *centre, f32 angle) {
    points[0].x = sinf(angle) * 1000.0f + centre->x;
    points[0].y = centre->y;
    points[0].z = cosf(angle) * 1000.0f + centre->z;
    points[0].w = 1.0f;
    points[1].x = sinf(angle) * -1000.0f + centre->x;
    points[1].y = centre->y;
    points[1].z = cosf(angle) * -1000.0f + centre->z;
    points[1].w = 1.0f;
}

void mapProbeObjectInit();

void mapProbeObjectReset(void) {
    mapProbeObjectInit();
}

s32 mapProbeSegmentTest(Vector4 *pickedPoint, void *recordId, f32 radius, Vector4 *probeSegment, Vector4 *origin, f32 angle);
void xglPointScaleOffsetAlongSegment(Vector4 *pickedPoint, Vector4 *pointsBegin, Vector4 *dir, Vector4 *origin);
void *memset();
extern Vector4 gMapProbeForwardAxis;

typedef struct ProbeSpan {
    unsigned char unmodeled_00[0x4];
    f32 angle;
} ProbeSpan;

/* Span hit test: tests the picked point along the span angle (mapProbeSegmentTest) and, when the first test misses, again from the centre at angle 0 with the origin moved along gMapProbeForwardAxis. Returns 1 on a hit, 0 otherwise. */
s32 mapProbeSpanHit(Vector4 *centre, void *recordId, Vector4 *origin, ProbeSpan *probeSpan, f32 radius) {
    Vector4 points[2];
    s32 hitResults[2];
    Vector4 scaledOrigin;
    f32 angle;
    s32 firstHit;
    s32 secondHit;

    angle = probeSpan->angle;
    memset(hitResults, 0, sizeof(hitResults));
    mapProbeSegmentEndpoints(points, origin, angle);
    hitResults[0] = mapProbeSegmentTest(centre, recordId, radius, points, origin, angle);
    firstHit = hitResults[0];
    if (firstHit == 2) {
        return 1;
    }
    if (firstHit == -2) {
        return 1;
    }
    if (firstHit == -1) {
        if (mapProbeNearestPoint(centre, recordId, origin, angle) == 0) {
            return 0;
        }
    }
    radius = radius * cosf(angle);
    angle = 0.0f;
    mapProbeSegmentEndpoints(points, centre, angle);
    xglPointScaleOffsetAlongSegment(&scaledOrigin, points, &gMapProbeForwardAxis, origin);
    scaledOrigin.y = origin->y;
    scaledOrigin.z = origin->z;
    secondHit = mapProbeSegmentTest(centre, recordId, radius, points, &scaledOrigin, angle);
    /* kept: the store to hitResults[1] is part of the match. */
    hitResults[1] = secondHit;
    if (secondHit > 0 || hitResults[0] > 0) {
        return 1;
    }
    return 0;
}

#define PROBE_HIT_BUFFER (&SEF_SCRATCHPAD(Vector4, 0x0))

void xglSegmentSelectAxis(Vector4 *probeSegment);
s32 xglSegmentSphereHit(Vector4 *pickedPoint, Vector4 *hitBuffer, s32 hits, Vector4 *origin, f32 radius);
s32 xglPointListFindDistance(Vector4 *pickedPoint, Vector4 *hitBuffer, s32 hits, Vector4 *origin, f32 radius);
s32 xglPointListFindAxisExtreme(Vector4 *pickedPoint, Vector4 *hitBuffer, s32 hits);

/* Probe test of one segment against the record. Returns 2 for a sphere hit, 1 for a distance or axis-extreme hit, -1 when the probe finds no hit and -2 when the record cannot be loaded. */
s32 mapProbeSegmentTest(Vector4 *pickedPoint, void *recordId, f32 radius, Vector4 *probeSegment, Vector4 *origin,
                  f32 angle) {
    s32 hitCount;
    s32 sphereHit;

    if (mapProbeAllocBlock(recordId) == 0) {
        return -2;
    }
    /* kept: 80 is the hitLimit argument of mapProbeSegmentHits. */
    hitCount = mapProbeSegmentHits(PROBE_HIT_BUFFER, probeSegment, (void *) 80);
    if (hitCount == 0) {
        return -1;
    }
    xglSegmentSelectAxis(probeSegment);
    sphereHit = xglSegmentSphereHit(pickedPoint, PROBE_HIT_BUFFER, hitCount, origin, radius);
    if (sphereHit != 0) {
        return 2;
    }
    if (xglPointListFindDistance(pickedPoint, PROBE_HIT_BUFFER, hitCount, origin, radius) != 0) {
        return 1;
    }
    if (xglPointListFindAxisExtreme(pickedPoint, PROBE_HIT_BUFFER, hitCount) != 0) {
        return 1;
    }
    return -1;
}

Vector4 gMapProbeForwardAxis = { 0.0f, 0.0f, 1.0f, 1.0f };
