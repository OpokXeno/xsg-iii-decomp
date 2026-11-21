#include "common.h"
#include "shared.h"

void assertFailed(const char *message, const char *file, int line);
void tiWindDirSetEnabled(u32 windSlot, int enabled);
void tiWindDirSetDirection(u32 windSlot, float x, float y, float z);
void tiWindDirSetPosition(u32 windSlot, const float *position);
void tiWindDirSetSpeed(u32 windSlot, float speed);
void tiWindDirSetRange(u32 windSlot, float upper, float lower);
void tiWindDirSetRadius(u32 windSlot, float radius, int global);
void tiWindPointSetEnabled(u32 windSlot, int enabled);
void tiWindPointSetPosition(u32 windSlot, const float *position);
void tiWindPointSetSpeed(u32 windSlot, float speed);
void tiWindPointSetRange(u32 windSlot, float upper, float lower);
void tiWindPointSetRadius(u32 windSlot, float radius, int global);

#include "main/TiGlobalColl.h"

TiGlobalCollSet *tiGlobalCollGetHandle(void);
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurApplyVector(Vector4 *destination, const Vector4 *vector);
void xglCurSetRotateZ(f32 angle);
void xacReadWindDirCurves(f32 *windFields, u8 **ppJointHeader, f32 curveParam);
void xacReadWindPointCurves(f32 *windFields, u8 **ppJointHeader, f32 curveParam);
extern const char tiWindAnimHeaderAssertText[];
extern const char tiWindAnimSourceFile[];
extern const DoublewordAlignedVector4 tiWindAnimDefaultDirVector;

/* The 12 floats that xacReadWindDirCurves and xacReadWindPointCurves fill from a wind header. */
enum WindAnimField {
    WIND_ANIM_ENABLED = 0,
    WIND_ANIM_POSITION = 1,   /* 1..3: x, y, z */
    WIND_ANIM_ROTATE_X = 4,
    WIND_ANIM_ROTATE_Y = 5,
    WIND_ANIM_ROTATE_Z = 6,
    WIND_ANIM_SPEED = 7,
    WIND_ANIM_UPPER = 8,
    WIND_ANIM_LOWER = 9,
    WIND_ANIM_GLOBAL = 10,    /* nonzero: no falloff with distance */
    WIND_ANIM_RADIUS = 11,
    WIND_ANIM_FIELD_COUNT = 12
};

/* Directional and point slots per collision set. */
#define WIND_ANIM_SLOT_COUNT 4
/* The radius mode argument of tiWindDirSetRadius and tiWindPointSetRadius: nonzero is global (no falloff). */
#define WIND_RADIUS_GLOBAL 1
#define WIND_RADIUS_FALLOFF 0

void tiWindAnimApplyDir(u32 windSlot, u8 *windHeader, f32 time);
void tiWindAnimApplyPoint(u32 windSlot, u8 *windHeader, f32 time);
extern const char D_00542BB0[8];
extern const char D_00542BB8[24];
extern const DoublewordAlignedVector4 D_00542BA0;

void tiWindAnimApplyDir(u32 windSlot, u8 *windHeader, f32 time)
{
    DoublewordAlignedVector4 windDirection = D_00542BA0;
    f32 windFields[WIND_ANIM_FIELD_COUNT];

    if (windHeader == 0) {
        assertFailed(D_00542BB0, D_00542BB8, 16);
    }
    xacReadWindDirCurves(windFields, &windHeader, time);
    if (windFields[WIND_ANIM_ENABLED] == 0.0f) {
        tiWindDirSetEnabled(windSlot, 0);
        return;
    }
    tiWindDirSetEnabled(windSlot, 1);
    tiWindDirSetPosition(windSlot, &windFields[WIND_ANIM_POSITION]);
    xglCurSetRotateZ(windFields[WIND_ANIM_ROTATE_Z]);
    xglCurMulRotateY(windFields[WIND_ANIM_ROTATE_Y]);
    xglCurMulRotateX(windFields[WIND_ANIM_ROTATE_X]);
    xglCurApplyVector(&windDirection, &windDirection);
    tiWindDirSetDirection(windSlot, windDirection.x, windDirection.y, windDirection.z);
    tiWindDirSetSpeed(windSlot, windFields[WIND_ANIM_SPEED]);
    tiWindDirSetRange(windSlot, windFields[WIND_ANIM_UPPER], windFields[WIND_ANIM_LOWER]);
    if (windFields[WIND_ANIM_GLOBAL] != 0.0f) {
        tiWindDirSetRadius(windSlot, 0.0f, WIND_RADIUS_GLOBAL);
    } else {
        tiWindDirSetRadius(windSlot, windFields[WIND_ANIM_RADIUS], WIND_RADIUS_FALLOFF);
    }
}

void tiWindAnimApplyPoint(u32 windSlot, u8 *windHeader, f32 time)
{
    f32 windFields[WIND_ANIM_FIELD_COUNT];

    if (windHeader == 0) {
        assertFailed(D_00542BB0, D_00542BB8, 50);
    }
    xacReadWindPointCurves(windFields, &windHeader, time);
    if (windFields[WIND_ANIM_ENABLED] == 0.0f) {
        tiWindPointSetEnabled(windSlot, 0);
        return;
    }
    tiWindPointSetEnabled(windSlot, 1);
    tiWindPointSetPosition(windSlot, &windFields[WIND_ANIM_POSITION]);
    tiWindPointSetSpeed(windSlot, windFields[WIND_ANIM_SPEED]);
    tiWindPointSetRange(windSlot, windFields[WIND_ANIM_UPPER], windFields[WIND_ANIM_LOWER]);
    if (windFields[WIND_ANIM_GLOBAL] != 0.0f) {
        tiWindPointSetRadius(windSlot, 0.0f, WIND_RADIUS_GLOBAL);
    } else {
        tiWindPointSetRadius(windSlot, windFields[WIND_ANIM_RADIUS], WIND_RADIUS_FALLOFF);
    }
}

/* The offsets are bytes from the handle, so the entries are reached through a byte base. */
void tiWindAnimUpdate(f32 time)
{
    TiGlobalCollSet *collisionSet;
    int windSlot;

    collisionSet = tiGlobalCollGetHandle();
    if (collisionSet != 0) {
        for (windSlot = 0; windSlot < WIND_ANIM_SLOT_COUNT; windSlot++) {
            tiWindAnimApplyDir(windSlot, (u8 *)collisionSet + collisionSet->windDirOffsets[windSlot], time);
        }
        for (windSlot = 0; windSlot < WIND_ANIM_SLOT_COUNT; windSlot++) {
            tiWindAnimApplyPoint(windSlot, (u8 *)collisionSet + collisionSet->windPointOffsets[windSlot], time);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiWindAnim", func_002A1928);

INCLUDE_ASM("asm/main/nonmatchings/TiWindAnim", func_002A1A50);

INCLUDE_ASM("asm/main/nonmatchings/TiWindAnim", func_002A1B30);
