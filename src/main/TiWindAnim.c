#include "common.h"
#include "shared.h"

void assertFailed(const char *message, const char *file, int line);
void tiWindDirSetEnabled(u32 index, int enabled);
void tiWindDirSetDirection(u32 index, float x, float y, float z);
void tiWindDirSetPosition(u32 index, const float *position);
void tiWindDirSetSpeed(u32 index, float speed);
void tiWindDirSetRange(u32 index, float upper, float lower);
void tiWindDirSetRadius(u32 index, float radius, int global);
void tiWindPointSetEnabled(u32 index, int enabled);
void tiWindPointSetPosition(u32 index, const float *position);
void tiWindPointSetSpeed(u32 index, float speed);
void tiWindPointSetRange(u32 index, float upper, float lower);
void tiWindPointSetRadius(u32 index, float radius, int global);

#include "main/TiGlobalColl.h"

TiGlobalCollSet *tiGlobalCollGetHandle(void);
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurApplyVector(Vector4 *destination, const Vector4 *vector);
void xglCurSetRotateZ(f32 angle);
void xacReadWindDirCurves(f32 *values, u8 **ppJointHeader, f32 curveParam);
void xacReadWindPointCurves(f32 *values, u8 **ppJointHeader, f32 curveParam);
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

void tiWindAnimApplyDir(u32 index, u8 *pHeader, f32 time);
void tiWindAnimApplyPoint(u32 index, u8 *pHeader, f32 time);
extern const char tiWindAnimHeaderAssertText[8];
extern const char tiWindAnimSourceFile[24];
void tiWindDirSetDirection(u32 index, float x, float y, float z);
void tiWindDirSetPosition(u32 index, const float *position);
void tiWindDirSetSpeed(u32 index, float speed);
void tiWindDirSetRange(u32 index, float upper, float lower);
void tiWindDirSetRadius(u32 index, float radius, int global);
void tiWindPointSetPosition(u32 index, const float *position);
void tiWindPointSetSpeed(u32 index, float speed);
void tiWindPointSetRange(u32 index, float upper, float lower);
void tiWindPointSetRadius(u32 index, float radius, int global);
void xacReadWindDirCurves(f32 *values, u8 **ppJointHeader, f32 curveParam);
void xacReadWindPointCurves(f32 *values, u8 **ppJointHeader, f32 curveParam);
extern const char D_00542BB0[8];
extern const char D_00542BB8[24];
extern const DoublewordAlignedVector4 D_00542BA0;

void tiWindAnimApplyDir(u32 index, u8 *pHeader, f32 time)
{
    DoublewordAlignedVector4 dir = D_00542BA0;
    f32 values[WIND_ANIM_FIELD_COUNT];

    if (pHeader == 0) {
        assertFailed(D_00542BB0, D_00542BB8, 16);
    }
    xacReadWindDirCurves(values, &pHeader, time);
    if (values[WIND_ANIM_ENABLED] == 0.0f) {
        tiWindDirSetEnabled(index, 0);
        return;
    }
    tiWindDirSetEnabled(index, 1);
    tiWindDirSetPosition(index, &values[WIND_ANIM_POSITION]);
    xglCurSetRotateZ(values[WIND_ANIM_ROTATE_Z]);
    xglCurMulRotateY(values[WIND_ANIM_ROTATE_Y]);
    xglCurMulRotateX(values[WIND_ANIM_ROTATE_X]);
    xglCurApplyVector(&dir, &dir);
    tiWindDirSetDirection(index, dir.x, dir.y, dir.z);
    tiWindDirSetSpeed(index, values[WIND_ANIM_SPEED]);
    tiWindDirSetRange(index, values[WIND_ANIM_UPPER], values[WIND_ANIM_LOWER]);
    if (values[WIND_ANIM_GLOBAL] != 0.0f) {
        tiWindDirSetRadius(index, 0.0f, WIND_RADIUS_GLOBAL);
    } else {
        tiWindDirSetRadius(index, values[WIND_ANIM_RADIUS], WIND_RADIUS_FALLOFF);
    }
}

void tiWindAnimApplyPoint(u32 index, u8 *pHeader, f32 time)
{
    f32 values[WIND_ANIM_FIELD_COUNT];

    if (pHeader == 0) {
        assertFailed(D_00542BB0, D_00542BB8, 50);
    }
    xacReadWindPointCurves(values, &pHeader, time);
    if (values[WIND_ANIM_ENABLED] == 0.0f) {
        tiWindPointSetEnabled(index, 0);
        return;
    }
    tiWindPointSetEnabled(index, 1);
    tiWindPointSetPosition(index, &values[WIND_ANIM_POSITION]);
    tiWindPointSetSpeed(index, values[WIND_ANIM_SPEED]);
    tiWindPointSetRange(index, values[WIND_ANIM_UPPER], values[WIND_ANIM_LOWER]);
    if (values[WIND_ANIM_GLOBAL] != 0.0f) {
        tiWindPointSetRadius(index, 0.0f, WIND_RADIUS_GLOBAL);
    } else {
        tiWindPointSetRadius(index, values[WIND_ANIM_RADIUS], WIND_RADIUS_FALLOFF);
    }
}

/* The offsets are bytes from the handle, so the entries are reached through a byte base. */
void tiWindAnimUpdate(f32 time)
{
    TiGlobalCollSet *set;
    int index;

    set = tiGlobalCollGetHandle();
    if (set != 0) {
        for (index = 0; index < WIND_ANIM_SLOT_COUNT; index++) {
            tiWindAnimApplyDir(index, (u8 *)set + set->windDirOffsets[index], time);
        }
        for (index = 0; index < WIND_ANIM_SLOT_COUNT; index++) {
            tiWindAnimApplyPoint(index, (u8 *)set + set->windPointOffsets[index], time);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiWindAnim", func_002A1928);

INCLUDE_ASM("asm/main/nonmatchings/TiWindAnim", func_002A1A50);

INCLUDE_ASM("asm/main/nonmatchings/TiWindAnim", func_002A1B30);
