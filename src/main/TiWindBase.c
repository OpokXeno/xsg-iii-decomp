#include "common.h"
#include "shared.h"

#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"




void tiWindApplyAll(TiHrcDynamics *dynamicsNode, TiSolverState *solverState);
void tiWindClearTables(void);
void tiWindAdvanceAll(void);
void tiWindDisableAll(void);
void assertFailed(const char *message, const char *file, int line);

unsigned short xglSRand(void);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);

#define DIR_WIND_MAX 4U
#define POINT_WIND_MAX 4U
#define WIND_FLAG_ENABLED 0x1
#define WIND_FLAG_GLOBAL 0x2

/* Gust redraw: xglSRand() % WIND_GUST_STEPS, gust = step * 0.01 + 0.5. */
#define WIND_GUST_STEPS 50U

/*
 * Directional wind table entry, 0x60 bytes.
 * flags: 0x1 enabled, 0x2 global (no falloff with distance).
 * 0x10: unit direction (w = 1 at 0x1C). 0x20: velocity, direction * phase * 0.1.
 * 0x30: position (w = 1 at 0x3C). 0x40: phase, oscillates between upper and lower.
 * oscillationDirection: +1 or -1, the direction of the phase; speed scales each step.
 * 0x4C: upper bound, 0x50: lower bound, 0x54: radius of the falloff.
 */
typedef struct DirWind {
    int windFlags;
    unsigned char unmodeled_04[0xC];
    /* The vectors stay float[4]: Vector4 members change the code. */
    union {
        Vector4 vector;
        float direction[4];
    };
    float velocity[4];
    float position[4];
    float oscillation;
    float oscillationDirection;
    float speed;
    float upper;
    float lower;
    float radius;
    unsigned char unmodeled_58[0x8];
} DirWind;

/*
 * Point wind table entry, 0x40 bytes.
 * flags as DirWind. 0x10: position (w = 1 at 0x1C). 0x24: scalar wave, phase * 0.1.
 * 0x28: phase. oscillationDirection: +1 or -1. speedRange[0]: speed, [1]: upper bound, [2]: lower bound.
 * 0x3C: radius of the falloff.
 */
typedef struct PointWind {
    int windFlags;
    unsigned char unmodeled_04[0xC];
    float position[4];
    unsigned char unmodeled_20[0x4];
    float wave;
    float oscillation;
    float oscillationDirection;
    float speedRange[3];
    float radius;
} PointWind;

/* The speed setter addresses the directional table through its own symbol at 0x8E5AD0 (0x40 into the DirWind table,
 * stride 0x60): the partial type names only the phase step and speed it stores. */
typedef struct DirWindOscillationSpeed {
    unsigned char unmodeled_00[4];
    float oscillationDirection;
    float speed;
    unsigned char unmodeled_0C[0x54];
} DirWindOscillationSpeed;

extern DirWind tiWindDirTable[DIR_WIND_MAX];
extern DirWindOscillationSpeed tiWindDirOscillationSpeedTable[DIR_WIND_MAX];
extern PointWind tiWindPointTable[POINT_WIND_MAX];

/* The range setter addresses the point table through its own symbol at 0x8E5C40 (0x30 into the PointWind table,
 * stride 0x40): the partial type names the speed range triple it stores. */
typedef struct PointWindSpeedRange {
    float speedRange[3];
    unsigned char unmodeled_0C[0x34];
} PointWindSpeedRange;

extern PointWindSpeedRange tiWindPointSpeedRangeTable[POINT_WIND_MAX];
extern const char tiWindBaseSourceFile[];
extern const char tiWindWindAssertText[];

void tiWindDirApply(DirWind *dirWind, TiHrcDynamics *dynamicsNode, TiSolverState *solverState);
void tiWindPointApply(PointWind *pointWind, TiHrcDynamics *dynamicsNode, TiSolverState *solverState);
void tiWindDirAdvance(DirWind *dirWind);
void tiWindPointAdvance(PointWind *pointWind);

extern const char tiWindBaseSourceFile[24];
extern const char tiWindWindAssertText[8];

/* Clears every directional and point wind entry. */
void tiWindClearTables(void)
{
    u32 windIndex;

    for (windIndex = 0; windIndex < DIR_WIND_MAX; windIndex++) {
        memset(&tiWindDirTable[windIndex], 0, sizeof(DirWind));
    }
    for (windIndex = 0; windIndex < POINT_WIND_MAX; windIndex++) {
        memset(&tiWindPointTable[windIndex], 0, sizeof(PointWind));
    }
}

extern const char tiWindDirIndexAssertText[24];
extern const char D_00542BF8[24];
extern const char tiWindPointIndexAssertText[24];
extern const char D_00542C28[];

/* Enables or disables directional wind `index`. */
void tiWindDirSetEnabled(u32 windIndex, int enabled)
{
    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 30);
    }
    if (enabled != 0) {
        tiWindDirTable[windIndex].windFlags |= WIND_FLAG_ENABLED;
        return;
    }
    tiWindDirTable[windIndex].windFlags &= ~WIND_FLAG_ENABLED;
}

/* Sets the direction of directional wind `index`: normalized, with the velocity reset to rest. */
void tiWindDirSetDirection(u32 windIndex, float x, float y, float z)
{
    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 43);
    }
    tiWindDirTable[windIndex].direction[0] = x;
    tiWindDirTable[windIndex].direction[1] = y;
    tiWindDirTable[windIndex].direction[2] = z;
    tiWindDirTable[windIndex].direction[3] = 1.0f;
    xglVectorNormal(&tiWindDirTable[windIndex].vector, &tiWindDirTable[windIndex].vector);
    tiWindDirTable[windIndex].velocity[0] = 0.0f;
    tiWindDirTable[windIndex].velocity[1] = 0.0f;
    tiWindDirTable[windIndex].velocity[2] = 0.0f;
    tiWindDirTable[windIndex].velocity[3] = 1.0f;
}

/* Sets the position of directional wind `index`; the fourth component is the homogeneous 1. */
void tiWindDirSetPosition(u32 windIndex, const float *position)
{
    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 61);
    }
    tiWindDirTable[windIndex].position[0] = position[0];
    tiWindDirTable[windIndex].position[1] = position[1];
    tiWindDirTable[windIndex].position[2] = position[2];
    tiWindDirTable[windIndex].position[3] = 1.0f;
}

/* Sets the speed of directional wind `index`. A new speed resets the phase step to +1. */
void tiWindDirSetSpeed(u32 windIndex, float speed)
{
    DirWindOscillationSpeed *dirSpeedEntry;

    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 74);
    }
    dirSpeedEntry = &tiWindDirOscillationSpeedTable[windIndex];
    if (dirSpeedEntry->speed != speed) {
        dirSpeedEntry->speed = speed;
        dirSpeedEntry->oscillationDirection = 1.0f;
    }
}

/* Sets the upper and lower phase bounds of directional wind `index`. */
void tiWindDirSetRange(u32 windIndex, float upper, float lower)
{
    DirWind *dirWind;

    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 88);
    }
    dirWind = &tiWindDirTable[windIndex];
    dirWind->upper = upper;
    tiWindDirTable[windIndex].lower = lower;
}

/* Sets the falloff radius of directional wind `index`; a non-zero `global` sets the global flag (no falloff with distance). */
void tiWindDirSetRadius(u32 windIndex, float radius, int isGlobalWind)
{
    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 99);
    }
    tiWindDirTable[windIndex].radius = radius;
    if (isGlobalWind != 0) {
        tiWindDirTable[windIndex].windFlags |= WIND_FLAG_GLOBAL;
    } else {
        tiWindDirTable[windIndex].windFlags &= ~WIND_FLAG_GLOBAL;
    }
}

/* Enables or disables point wind `index`. */
void tiWindPointSetEnabled(u32 windIndex, int enabled)
{
    if (windIndex >= POINT_WIND_MAX) {
        assertFailed(tiWindPointIndexAssertText, D_00542BF8, 113);
    }
    if (enabled != 0) {
        tiWindPointTable[windIndex].windFlags |= WIND_FLAG_ENABLED;
        return;
    }
    tiWindPointTable[windIndex].windFlags &= ~WIND_FLAG_ENABLED;
}

/* Sets the position of point wind `index`; the fourth component is the homogeneous 1. */
void tiWindPointSetPosition(u32 windIndex, const float *position)
{
    if (windIndex >= POINT_WIND_MAX) {
        assertFailed(tiWindPointIndexAssertText, D_00542BF8, 126);
    }
    tiWindPointTable[windIndex].position[0] = position[0];
    tiWindPointTable[windIndex].position[1] = position[1];
    tiWindPointTable[windIndex].position[2] = position[2];
    tiWindPointTable[windIndex].position[3] = 1.0f;
}

/* Sets the speed of point wind `index`, value[0] of its speed and range triple. */
void tiWindPointSetSpeed(u32 windIndex, float speed)
{
    if (windIndex >= POINT_WIND_MAX) {
        assertFailed(tiWindPointIndexAssertText, D_00542BF8, 139);
    }
    if (tiWindPointTable[windIndex].speedRange[0] != speed) {
        tiWindPointTable[windIndex].speedRange[0] = speed;
        tiWindPointTable[windIndex].oscillationDirection = 1.0f;
    }
}

/* Sets the upper and lower phase bounds of point wind `index`. */
void tiWindPointSetRange(u32 windIndex, float upper, float lower)
{
    PointWindSpeedRange *pointRangeEntry;

    if (windIndex >= POINT_WIND_MAX) {
        assertFailed(tiWindPointIndexAssertText, D_00542BF8, 153);
    }
    pointRangeEntry = &tiWindPointSpeedRangeTable[windIndex];
    pointRangeEntry->speedRange[1] = upper;
    pointRangeEntry->speedRange[2] = lower;
}

/* Sets the falloff radius of point wind `index`; a non-zero `global` sets the global flag (no falloff with distance). */
void tiWindPointSetRadius(u32 windIndex, float radius, int isGlobalWind)
{
    if (windIndex >= POINT_WIND_MAX) {
        assertFailed(tiWindPointIndexAssertText, D_00542BF8, 164);
    }
    tiWindPointTable[windIndex].radius = radius;
    if (isGlobalWind != 0) {
        tiWindPointTable[windIndex].windFlags |= WIND_FLAG_GLOBAL;
    } else {
        tiWindPointTable[windIndex].windFlags &= ~WIND_FLAG_GLOBAL;
    }
}

/* Pushes the state with a directional wind: a global wind moves it by velocity * time step * gust; otherwise the push
 * falls off with distance over the radius. The gust is redrawn when the phase reaches its upper bound. */
void tiWindDirApply(DirWind *dirWind, TiHrcDynamics *dynamicsNode, TiSolverState *solverState)
{
    float delta[3];
    float dist;
    float falloff;
    float gust;
    int gustRoll;

    if (dirWind == 0) {
        assertFailed(D_00542C28, D_00542BF8, 178);
    }
    if (dirWind->oscillation == dirWind->upper) {
        gustRoll = xglSRand() % WIND_GUST_STEPS;
        gust = gustRoll * 0.01f + 0.1f;
        solverState->gust = gust * gust;
    }
    if (dirWind->windFlags & WIND_FLAG_GLOBAL) {
        solverState->accumulator.vector.x += dirWind->velocity[0] * dynamicsNode->timeStep * solverState->gust;
        solverState->accumulator.vector.y += dirWind->velocity[1] * dynamicsNode->timeStep * solverState->gust;
        solverState->accumulator.vector.z += dirWind->velocity[2] * dynamicsNode->timeStep * solverState->gust;
        return;
    }
    delta[0] = solverState->position.vector.x - dirWind->position[0];
    delta[1] = solverState->position.vector.y - dirWind->position[1];
    delta[2] = solverState->position.vector.z - dirWind->position[2];
    dist = __builtin_sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
    if (dirWind->radius == 0.0f || !(dist <= dirWind->radius)) {
        return;
    }
    falloff = dist / dirWind->radius;
    solverState->accumulator.vector.x += dirWind->velocity[0] * dynamicsNode->timeStep * falloff * solverState->gust;
    solverState->accumulator.vector.y += dirWind->velocity[1] * dynamicsNode->timeStep * falloff * solverState->gust;
    solverState->accumulator.vector.z += dirWind->velocity[2] * dynamicsNode->timeStep * falloff * solverState->gust;
}

/* Pushes the state from a point wind along the unit direction to the state: a global wind adds the push on all three
 * axes; otherwise the push falls off with distance over the radius and only x and z change. The gust is redrawn when
 * the phase reaches its speed range upper bound. */
void tiWindPointApply(PointWind *pointWind, TiHrcDynamics *dynamicsNode, TiSolverState *solverState)
{
    float delta[4];
    float dist;
    float scale;
    float gust;
    int gustRoll;

    if (pointWind == 0) {
        assertFailed(D_00542C28, D_00542BF8, 228);
    }
    delta[0] = solverState->position.vector.x - pointWind->position[0];
    delta[1] = solverState->position.vector.y - pointWind->position[1];
    delta[2] = solverState->position.vector.z - pointWind->position[2];
    delta[3] = 1.0f;
    if (pointWind->oscillation == pointWind->speedRange[1]) {
        gustRoll = xglSRand() % WIND_GUST_STEPS;
        gust = gustRoll * 0.01f + 0.1f;
        solverState->gust = gust * gust;
    }
    dist = __builtin_sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
    if (dist == 0.0f) {
        dist = 1e-7f;
    }
    scale = 1.0f / dist;
    delta[0] *= scale;
    delta[1] *= scale;
    delta[2] *= scale;
    if (pointWind->windFlags & WIND_FLAG_GLOBAL) {
        solverState->accumulator.vector.x += delta[0] * pointWind->wave * dynamicsNode->timeStep * solverState->gust;
        solverState->accumulator.vector.y += delta[1] * pointWind->wave * dynamicsNode->timeStep * solverState->gust;
        solverState->accumulator.vector.z += delta[2] * pointWind->wave * dynamicsNode->timeStep * solverState->gust;
        return;
    }
    if (pointWind->radius == 0.0f || !(dist <= pointWind->radius)) {
        return;
    }
    scale = dist / pointWind->radius;
    solverState->accumulator.vector.x += delta[0] * pointWind->wave * dynamicsNode->timeStep * scale * solverState->gust;
    solverState->accumulator.vector.z += delta[2] * pointWind->wave * dynamicsNode->timeStep * scale * solverState->gust;
}

/* Applies every enabled wind to the solver state; the parameters are the dynamics node and the solver state. */
void tiWindApplyAll(TiHrcDynamics *dynamicsNode, TiSolverState *solverState)
{
    u32 windIndex;

    for (windIndex = 0; windIndex < DIR_WIND_MAX; windIndex++) {
        if (tiWindDirTable[windIndex].windFlags & WIND_FLAG_ENABLED) {
            tiWindDirApply(&tiWindDirTable[windIndex], dynamicsNode, solverState);
        }
    }
    for (windIndex = 0; windIndex < POINT_WIND_MAX; windIndex++) {
        if (tiWindPointTable[windIndex].windFlags & WIND_FLAG_ENABLED) {
            tiWindPointApply(&tiWindPointTable[windIndex], dynamicsNode, solverState);
        }
    }
}

/* Advances the phase of directional wind `entry`: equal bounds or zero speed hold the phase at the lower bound. The
 * velocity is direction * phase * 0.1. */
void tiWindDirAdvance(DirWind *dirWind)
{
    float nextOscillation;

    if (dirWind == 0) {
        assertFailed(D_00542C28, D_00542BF8, 300);
    }
    if (dirWind->lower == dirWind->upper || dirWind->speed == 0.0f) {
        dirWind->oscillation = dirWind->lower;
    } else {
        nextOscillation = dirWind->oscillation + dirWind->speed * (dirWind->lower - dirWind->upper) * dirWind->oscillationDirection;
        dirWind->oscillation = nextOscillation;
        if (nextOscillation <= dirWind->upper) {
            dirWind->oscillation = dirWind->upper;
            dirWind->oscillationDirection = 1.0f;
        } else if (dirWind->lower <= nextOscillation) {
            dirWind->oscillation = dirWind->lower;
            dirWind->oscillationDirection = -1.0f;
        }
    }
    dirWind->velocity[0] = dirWind->direction[0] * dirWind->oscillation * 0.1f;
    dirWind->velocity[1] = dirWind->direction[1] * dirWind->oscillation * 0.1f;
    dirWind->velocity[2] = dirWind->direction[2] * dirWind->oscillation * 0.1f;
}

/* Advances the phase of point wind `entry`; equal bounds or zero speed hold the phase at the lower bound. */
void tiWindPointAdvance(PointWind *pointWind)
{
    float nextOscillation;

    if (pointWind == 0) {
        assertFailed(D_00542C28, D_00542BF8, 331);
    }
    if (pointWind->speedRange[2] == pointWind->speedRange[1] || pointWind->speedRange[0] == 0.0f) {
        pointWind->oscillation = pointWind->speedRange[2];
    } else {
        nextOscillation = pointWind->oscillation + pointWind->speedRange[0] * (pointWind->speedRange[2] - pointWind->speedRange[1]) * pointWind->oscillationDirection;
        pointWind->oscillation = nextOscillation;
        if (nextOscillation <= pointWind->speedRange[1]) {
            pointWind->oscillation = pointWind->speedRange[1];
            pointWind->oscillationDirection = 1.0f;
        } else if (pointWind->speedRange[2] <= nextOscillation) {
            pointWind->oscillation = pointWind->speedRange[2];
            pointWind->oscillationDirection = -1.0f;
        }
    }
    pointWind->wave = pointWind->oscillation * 0.1f;
}

/* Advances the phase of every enabled wind. */
void tiWindAdvanceAll(void)
{
    u32 windIndex;

    for (windIndex = 0; windIndex < DIR_WIND_MAX; windIndex++) {
        if (tiWindDirTable[windIndex].windFlags & WIND_FLAG_ENABLED) {
            tiWindDirAdvance(&tiWindDirTable[windIndex]);
        }
    }
    for (windIndex = 0; windIndex < POINT_WIND_MAX; windIndex++) {
        if (tiWindPointTable[windIndex].windFlags & WIND_FLAG_ENABLED) {
            tiWindPointAdvance(&tiWindPointTable[windIndex]);
        }
    }
}

/* Disables every wind; the entries keep their data. */
void tiWindDisableAll(void)
{
    u32 windIndex;

    for (windIndex = 0; windIndex < DIR_WIND_MAX; windIndex++) {
        tiWindDirTable[windIndex].windFlags &= ~WIND_FLAG_ENABLED;
    }
    for (windIndex = 0; windIndex < POINT_WIND_MAX; windIndex++) {
        tiWindPointTable[windIndex].windFlags &= ~WIND_FLAG_ENABLED;
    }
}

/* Returns directional wind `index`. */
DirWind *tiWindDirGet(u32 windIndex)
{
    if (windIndex >= DIR_WIND_MAX) {
        assertFailed(tiWindDirIndexAssertText, D_00542BF8, 391);
    }
    return &tiWindDirTable[windIndex];
}

/* Returns point wind `index`. */
PointWind *tiWindPointGet(u32 windIndex)
{
    if (windIndex >= POINT_WIND_MAX) {
        assertFailed(tiWindPointIndexAssertText, D_00542BF8, 401);
    }
    return &tiWindPointTable[windIndex];
}
