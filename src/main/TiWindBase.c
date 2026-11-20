#include "common.h"
#include "shared.h"

#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"




void tiWindApplyAll(TiHrcDynamics *node, TiSolverState *state);
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
 * phaseStep: +1 or -1, the direction of the phase; speed scales each step.
 * 0x4C: upper bound, 0x50: lower bound, 0x54: radius of the falloff.
 */
typedef struct DirWind {
    int flags;
    unsigned char unmodeled_04[0xC];
    /* The vectors stay float[4]: Vector4 members change the code. */
    union {
        Vector4 vector;
        float direction[4];
    };
    float velocity[4];
    float position[4];
    float phase;
    float phaseStep;
    float speed;
    float upper;
    float lower;
    float radius;
    unsigned char unmodeled_58[0x8];
} DirWind;

/*
 * Point wind table entry, 0x40 bytes.
 * flags as DirWind. 0x10: position (w = 1 at 0x1C). 0x24: scalar wave, phase * 0.1.
 * 0x28: phase. phaseStep: +1 or -1. speedRange[0]: speed, [1]: upper bound, [2]: lower bound.
 * 0x3C: radius of the falloff.
 */
typedef struct PointWind {
    int flags;
    unsigned char unmodeled_04[0xC];
    float position[4];
    unsigned char unmodeled_20[0x4];
    float wave;
    float phase;
    float phaseStep;
    float speedRange[3];
    float radius;
} PointWind;

/* The speed setter addresses the directional table through its own symbol at 0x8E5AD0 (0x40 into the DirWind table,
 * stride 0x60): the partial type names only the phase step and speed it stores. */
typedef struct DirWindValue {
    unsigned char unmodeled_00[4];
    float phaseStep;
    float speed;
    unsigned char unmodeled_0C[0x54];
} DirWindValue;

extern DirWind tiWindDirTable[DIR_WIND_MAX];
extern DirWindValue D_008E5AD0[DIR_WIND_MAX];
extern PointWind tiWindPointTable[POINT_WIND_MAX];

/* The range setter addresses the point table through its own symbol at 0x8E5C40 (0x30 into the PointWind table,
 * stride 0x40): the partial type names the speed range triple it stores. */
typedef struct PointWindValues {
    float speedRange[3];
    unsigned char unmodeled_0C[0x34];
} PointWindValues;

extern PointWindValues D_008E5C40[POINT_WIND_MAX];
extern const char tiWindBaseSourceFile[];
extern const char tiWindWindAssertText[];

void tiWindDirApply(DirWind *entry, TiHrcDynamics *step, TiSolverState *state);
void tiWindPointApply(PointWind *entry, TiHrcDynamics *step, TiSolverState *state);
void tiWindDirAdvance(DirWind *entry);
void tiWindPointAdvance(PointWind *entry);

extern const char tiWindBaseSourceFile[24];
extern const char tiWindWindAssertText[8];

/* Clears every directional and point wind entry. */
void tiWindClearTables(void)
{
    u32 index;

    for (index = 0; index < DIR_WIND_MAX; index++) {
        memset(&tiWindDirTable[index], 0, sizeof(DirWind));
    }
    for (index = 0; index < POINT_WIND_MAX; index++) {
        memset(&tiWindPointTable[index], 0, sizeof(PointWind));
    }
}

extern const char D_00542BE0[24];
extern const char D_00542BF8[24];
extern const char D_00542C10[24];
extern const char D_00542C28[];

/* Enables or disables directional wind `index`. */
void tiWindDirSetEnabled(u32 index, int enabled)
{
    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 30);
    }
    if (enabled != 0) {
        tiWindDirTable[index].flags |= WIND_FLAG_ENABLED;
        return;
    }
    tiWindDirTable[index].flags &= ~WIND_FLAG_ENABLED;
}

/* Sets the direction of directional wind `index`: normalized, with the velocity reset to rest. */
void tiWindDirSetDirection(u32 index, float x, float y, float z)
{
    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 43);
    }
    tiWindDirTable[index].direction[0] = x;
    tiWindDirTable[index].direction[1] = y;
    tiWindDirTable[index].direction[2] = z;
    tiWindDirTable[index].direction[3] = 1.0f;
    xglVectorNormal(&tiWindDirTable[index].vector, &tiWindDirTable[index].vector);
    tiWindDirTable[index].velocity[0] = 0.0f;
    tiWindDirTable[index].velocity[1] = 0.0f;
    tiWindDirTable[index].velocity[2] = 0.0f;
    tiWindDirTable[index].velocity[3] = 1.0f;
}

/* Sets the position of directional wind `index`; the fourth component is the homogeneous 1. */
void tiWindDirSetPosition(u32 index, const float *position)
{
    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 61);
    }
    tiWindDirTable[index].position[0] = position[0];
    tiWindDirTable[index].position[1] = position[1];
    tiWindDirTable[index].position[2] = position[2];
    tiWindDirTable[index].position[3] = 1.0f;
}

/* Sets the speed of directional wind `index`. A new speed resets the phase step to +1. */
void tiWindDirSetSpeed(u32 index, float speed)
{
    DirWindValue *entry;

    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 74);
    }
    entry = &D_008E5AD0[index];
    if (entry->speed != speed) {
        entry->speed = speed;
        entry->phaseStep = 1.0f;
    }
}

/* Sets the upper and lower phase bounds of directional wind `index`. */
void tiWindDirSetRange(u32 index, float upper, float lower)
{
    DirWind *entry;

    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 88);
    }
    entry = &tiWindDirTable[index];
    entry->upper = upper;
    tiWindDirTable[index].lower = lower;
}

/* Sets the falloff radius of directional wind `index`; a non-zero `global` sets the global flag (no falloff with distance). */
void tiWindDirSetRadius(u32 index, float radius, int global)
{
    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 99);
    }
    tiWindDirTable[index].radius = radius;
    if (global != 0) {
        tiWindDirTable[index].flags |= WIND_FLAG_GLOBAL;
    } else {
        tiWindDirTable[index].flags &= ~WIND_FLAG_GLOBAL;
    }
}

/* Enables or disables point wind `index`. */
void tiWindPointSetEnabled(u32 index, int enabled)
{
    if (index >= POINT_WIND_MAX) {
        assertFailed(D_00542C10, D_00542BF8, 113);
    }
    if (enabled != 0) {
        tiWindPointTable[index].flags |= WIND_FLAG_ENABLED;
        return;
    }
    tiWindPointTable[index].flags &= ~WIND_FLAG_ENABLED;
}

/* Sets the position of point wind `index`; the fourth component is the homogeneous 1. */
void tiWindPointSetPosition(u32 index, const float *position)
{
    if (index >= POINT_WIND_MAX) {
        assertFailed(D_00542C10, D_00542BF8, 126);
    }
    tiWindPointTable[index].position[0] = position[0];
    tiWindPointTable[index].position[1] = position[1];
    tiWindPointTable[index].position[2] = position[2];
    tiWindPointTable[index].position[3] = 1.0f;
}

/* Sets the speed of point wind `index`, value[0] of its speed and range triple. */
void tiWindPointSetSpeed(u32 index, float speed)
{
    if (index >= POINT_WIND_MAX) {
        assertFailed(D_00542C10, D_00542BF8, 139);
    }
    if (tiWindPointTable[index].speedRange[0] != speed) {
        tiWindPointTable[index].speedRange[0] = speed;
        tiWindPointTable[index].phaseStep = 1.0f;
    }
}

/* Sets the upper and lower phase bounds of point wind `index`. */
void tiWindPointSetRange(u32 index, float upper, float lower)
{
    PointWindValues *entry;

    if (index >= POINT_WIND_MAX) {
        assertFailed(D_00542C10, D_00542BF8, 153);
    }
    entry = &D_008E5C40[index];
    entry->speedRange[1] = upper;
    entry->speedRange[2] = lower;
}

/* Sets the falloff radius of point wind `index`; a non-zero `global` sets the global flag (no falloff with distance). */
void tiWindPointSetRadius(u32 index, float radius, int global)
{
    if (index >= POINT_WIND_MAX) {
        assertFailed(D_00542C10, D_00542BF8, 164);
    }
    tiWindPointTable[index].radius = radius;
    if (global != 0) {
        tiWindPointTable[index].flags |= WIND_FLAG_GLOBAL;
    } else {
        tiWindPointTable[index].flags &= ~WIND_FLAG_GLOBAL;
    }
}

/* Pushes the state with a directional wind: a global wind moves it by velocity * time step * gust; otherwise the push
 * falls off with distance over the radius. The gust is redrawn when the phase reaches its upper bound. */
void tiWindDirApply(DirWind *entry, TiHrcDynamics *step, TiSolverState *state)
{
    float delta[3];
    float dist;
    float falloff;
    float gust;
    int gustRoll;

    if (entry == 0) {
        assertFailed(D_00542C28, D_00542BF8, 178);
    }
    if (entry->phase == entry->upper) {
        gustRoll = xglSRand() % WIND_GUST_STEPS;
        gust = (float)gustRoll * 0.01f + 0.1f;
        state->gust = gust * gust;
    }
    if (entry->flags & WIND_FLAG_GLOBAL) {
        state->accumulator.vector.x += entry->velocity[0] * step->timeStep * state->gust;
        state->accumulator.vector.y += entry->velocity[1] * step->timeStep * state->gust;
        state->accumulator.vector.z += entry->velocity[2] * step->timeStep * state->gust;
        return;
    }
    delta[0] = state->position.vector.x - entry->position[0];
    delta[1] = state->position.vector.y - entry->position[1];
    delta[2] = state->position.vector.z - entry->position[2];
    dist = __builtin_sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
    if (entry->radius == 0.0f || !(dist <= entry->radius)) {
        return;
    }
    falloff = dist / entry->radius;
    state->accumulator.vector.x += entry->velocity[0] * step->timeStep * falloff * state->gust;
    state->accumulator.vector.y += entry->velocity[1] * step->timeStep * falloff * state->gust;
    state->accumulator.vector.z += entry->velocity[2] * step->timeStep * falloff * state->gust;
}

/* Pushes the state from a point wind along the unit direction to the state: a global wind adds the push on all three
 * axes; otherwise the push falls off with distance over the radius and only x and z change. The gust is redrawn when
 * the phase reaches its speed range upper bound. */
void tiWindPointApply(PointWind *entry, TiHrcDynamics *step, TiSolverState *state)
{
    float delta[4];
    float dist;
    float scale;
    float gust;
    int gustRoll;

    if (entry == 0) {
        assertFailed(D_00542C28, D_00542BF8, 228);
    }
    delta[0] = state->position.vector.x - entry->position[0];
    delta[1] = state->position.vector.y - entry->position[1];
    delta[2] = state->position.vector.z - entry->position[2];
    delta[3] = 1.0f;
    if (entry->phase == entry->speedRange[1]) {
        gustRoll = xglSRand() % WIND_GUST_STEPS;
        gust = (float)gustRoll * 0.01f + 0.1f;
        state->gust = gust * gust;
    }
    dist = __builtin_sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
    if (dist == 0.0f) {
        dist = 1e-7f;
    }
    scale = 1.0f / dist;
    delta[0] *= scale;
    delta[1] *= scale;
    delta[2] *= scale;
    if (entry->flags & WIND_FLAG_GLOBAL) {
        state->accumulator.vector.x += delta[0] * entry->wave * step->timeStep * state->gust;
        state->accumulator.vector.y += delta[1] * entry->wave * step->timeStep * state->gust;
        state->accumulator.vector.z += delta[2] * entry->wave * step->timeStep * state->gust;
        return;
    }
    if (entry->radius == 0.0f || !(dist <= entry->radius)) {
        return;
    }
    scale = dist / entry->radius;
    state->accumulator.vector.x += delta[0] * entry->wave * step->timeStep * scale * state->gust;
    state->accumulator.vector.z += delta[2] * entry->wave * step->timeStep * scale * state->gust;
}

/* Applies every enabled wind to the solver state; the parameters are the dynamics node and the solver state. */
void tiWindApplyAll(TiHrcDynamics *first_input, TiSolverState *second_input)
{
    u32 index;

    for (index = 0; index < DIR_WIND_MAX; index++) {
        if (tiWindDirTable[index].flags & WIND_FLAG_ENABLED) {
            tiWindDirApply(&tiWindDirTable[index], first_input, second_input);
        }
    }
    for (index = 0; index < POINT_WIND_MAX; index++) {
        if (tiWindPointTable[index].flags & WIND_FLAG_ENABLED) {
            tiWindPointApply(&tiWindPointTable[index], first_input, second_input);
        }
    }
}

/* Advances the phase of directional wind `entry`: equal bounds or zero speed hold the phase at the lower bound. The
 * velocity is direction * phase * 0.1. */
void tiWindDirAdvance(DirWind *entry)
{
    float nextPhase;

    if (entry == 0) {
        assertFailed(D_00542C28, D_00542BF8, 300);
    }
    if (entry->lower == entry->upper || entry->speed == 0.0f) {
        entry->phase = entry->lower;
    } else {
        nextPhase = entry->phase + entry->speed * (entry->lower - entry->upper) * entry->phaseStep;
        entry->phase = nextPhase;
        if (nextPhase <= entry->upper) {
            entry->phase = entry->upper;
            entry->phaseStep = 1.0f;
        } else if (entry->lower <= nextPhase) {
            entry->phase = entry->lower;
            entry->phaseStep = -1.0f;
        }
    }
    entry->velocity[0] = entry->direction[0] * entry->phase * 0.1f;
    entry->velocity[1] = entry->direction[1] * entry->phase * 0.1f;
    entry->velocity[2] = entry->direction[2] * entry->phase * 0.1f;
}

/* Advances the phase of point wind `entry`; equal bounds or zero speed hold the phase at the lower bound. */
void tiWindPointAdvance(PointWind *entry)
{
    float nextPhase;

    if (entry == 0) {
        assertFailed(D_00542C28, D_00542BF8, 331);
    }
    if (entry->speedRange[2] == entry->speedRange[1] || entry->speedRange[0] == 0.0f) {
        entry->phase = entry->speedRange[2];
    } else {
        nextPhase = entry->phase + entry->speedRange[0] * (entry->speedRange[2] - entry->speedRange[1]) * entry->phaseStep;
        entry->phase = nextPhase;
        if (nextPhase <= entry->speedRange[1]) {
            entry->phase = entry->speedRange[1];
            entry->phaseStep = 1.0f;
        } else if (entry->speedRange[2] <= nextPhase) {
            entry->phase = entry->speedRange[2];
            entry->phaseStep = -1.0f;
        }
    }
    entry->wave = entry->phase * 0.1f;
}

/* Advances the phase of every enabled wind. */
void tiWindAdvanceAll(void)
{
    u32 index;

    for (index = 0; index < DIR_WIND_MAX; index++) {
        if (tiWindDirTable[index].flags & WIND_FLAG_ENABLED) {
            tiWindDirAdvance(&tiWindDirTable[index]);
        }
    }
    for (index = 0; index < POINT_WIND_MAX; index++) {
        if (tiWindPointTable[index].flags & WIND_FLAG_ENABLED) {
            tiWindPointAdvance(&tiWindPointTable[index]);
        }
    }
}

/* Disables every wind; the entries keep their data. */
void tiWindDisableAll(void)
{
    u32 index;

    for (index = 0; index < DIR_WIND_MAX; index++) {
        tiWindDirTable[index].flags &= ~WIND_FLAG_ENABLED;
    }
    for (index = 0; index < POINT_WIND_MAX; index++) {
        tiWindPointTable[index].flags &= ~WIND_FLAG_ENABLED;
    }
}

/* Returns directional wind `index`. */
DirWind *tiWindDirGet(u32 index)
{
    if (index >= DIR_WIND_MAX) {
        assertFailed(D_00542BE0, D_00542BF8, 391);
    }
    return &tiWindDirTable[index];
}

/* Returns point wind `index`. */
PointWind *tiWindPointGet(u32 index)
{
    if (index >= POINT_WIND_MAX) {
        assertFailed(D_00542C10, D_00542BF8, 401);
    }
    return &tiWindPointTable[index];
}
