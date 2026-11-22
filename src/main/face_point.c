#include "common.h"
#include "shared.h"
#include "vu0_macro.h"

/*
 * One culling volume: the position, rotation and scale culling_matrix reads,
 * the transform it builds from them, and the four corners it derives.
 */
typedef struct CullingVolume {
    Vector4 position;
    Vector4 rotation;
    Vector4 scale;
    Vector4 normalizedDirection;
    Matrix4 transform;
    Matrix4 inverseTransform;
    Vector4 cullingCorners[4];
} CullingVolume;

void xglMatrixStackUnit(void);
void xglMatrixStackInverse(void);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackSave(Matrix4 matrix);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
extern const AlignedVector4 cullingForwardAxisVector;
extern const AlignedVector4 cullingCornerLowerLeftVector;
extern const AlignedVector4 cullingCornerLowerRightVector;
extern const AlignedVector4 cullingCornerUpperRightVector;
extern const AlignedVector4 cullingCornerUpperLeftVector;

/* xglCullingVolumeSetup: builds the volume transform from its position, rotation and scale, then the four corners. */
void xglCullingVolumeSetup(CullingVolume *volume)
{
    AlignedVector4 forwardAxis;
    AlignedVector4 lowerLeft;
    AlignedVector4 lowerRight;
    AlignedVector4 upperRight;
    AlignedVector4 upperLeft;

    xglMatrixStackUnit();
    xglMatrixStackTrans(&volume->position);
    xglMatrixStackRotZ(volume->rotation.z);
    xglMatrixStackRotY(volume->rotation.y);
    xglMatrixStackRotX(volume->rotation.x);
    xglMatrixStackScale(&volume->scale);
    xglMatrixStackSave(volume->transform);
    xglMatrixStackInverse();
    xglMatrixStackSave(volume->inverseTransform);
    forwardAxis = cullingForwardAxisVector;
    Vu0ApplyMatrix33XYZ(&volume->normalizedDirection, &volume->transform, &forwardAxis);
    xglVectorNormal(&volume->normalizedDirection, &volume->normalizedDirection);
    lowerLeft = cullingCornerLowerLeftVector;
    lowerRight = cullingCornerLowerRightVector;
    upperRight = cullingCornerUpperRightVector;
    upperLeft = cullingCornerUpperLeftVector;
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[0], &volume->transform, &lowerLeft);
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[1], &volume->transform, &lowerRight);
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[2], &volume->transform, &upperRight);
    Vu0ApplyMatrixXYZ(&volume->cullingCorners[3], &volume->transform, &upperLeft);
}

typedef int s32;

void xglCullingVolumeDisplay(void *volumeRecord) {

}

void _FacePoint(void *hitPoint, void *lineStart, void *lineEnd, void *planeNormal, void *planePoint) {
    __asm__ __volatile__("lqc2 vf22, 0(%0)" : : "r"(planeNormal) : "memory");
    __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(lineStart) : "memory");
    __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(lineEnd) : "memory");
    __asm__ __volatile__("lqc2 vf23, 0(%0)" : : "r"(planePoint) : "memory");
    __asm__ __volatile__("vmulw.xyz vf14, vf22, vf22w" : : : "memory");
    __asm__ __volatile__("vmulw.xyz vf14, vf14, vf20w" : : : "memory");
    __asm__ __volatile__("vadd.xyz vf23, vf23, vf14" : : : "memory");
    __asm__ __volatile__("vaddx.x vf10, vf0, vf22x" : : : "memory");
    __asm__ __volatile__("vaddy.x vf11, vf0, vf22y" : : : "memory");
    __asm__ __volatile__("vaddz.x vf12, vf0, vf22z" : : : "memory");
    __asm__ __volatile__("vsub.xyz vf19, vf21, vf20" : : : "memory");
    __asm__ __volatile__("vmulax.x ACC, vf10, vf21x" : : : "memory");
    __asm__ __volatile__("vmadday.x ACC, vf11, vf21y" : : : "memory");
    __asm__ __volatile__("vmaddz.x vf17, vf12, vf21z" : : : "memory");
    __asm__ __volatile__("vmulax.x ACC, vf10, vf19x" : : : "memory");
    __asm__ __volatile__("vmadday.x ACC, vf11, vf19y" : : : "memory");
    __asm__ __volatile__("vmaddz.x vf18, vf12, vf19z" : : : "memory");
    __asm__ __volatile__("vmulax.x ACC, vf10, vf23x" : : : "memory");
    __asm__ __volatile__("vmadday.x ACC, vf11, vf23y" : : : "memory");
    __asm__ __volatile__("vmaddz.x vf16, vf12, vf23z" : : : "memory");
    __asm__ __volatile__("vdiv Q, vf0w, vf18x" : : : "memory");
    __asm__ __volatile__("vsub.x vf16, vf17, vf16" : : : "memory");
    __asm__ __volatile__("vwaitq" : : : "memory");
    __asm__ __volatile__("vmulq.x vf16, vf16, Q" : : : "memory");
    __asm__ __volatile__("vmulx.xyz vf15, vf19, vf16x" : : : "memory");
    __asm__ __volatile__("vsub.xyz vf15, vf21, vf15" : : : "memory");
    __asm__ __volatile__("sqc2 vf15, 0(%0)" : : "r"(hitPoint) : "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/face_point", _CheckLine);

void xglVectorOuter(Vector4 *destination, const Vector4 *left, const Vector4 *right);

void plane_from_points(const Vector4 *origin, const Vector4 *firstPoint,
                       const Vector4 *secondPoint, Vector4 *plane)
{
    Vector4 originToFirst;
    Vector4 originToSecond;
    Vector4 normal;

    originToFirst.x = firstPoint->x - origin->x;
    originToFirst.y = firstPoint->y - origin->y;
    originToFirst.z = firstPoint->z - origin->z;
    originToSecond.x = secondPoint->x - origin->x;
    originToSecond.y = secondPoint->y - origin->y;
    originToSecond.z = secondPoint->z - origin->z;
    xglVectorOuter(&normal, &originToFirst, &originToSecond);
    xglVectorNormal(&normal, &normal);
    plane->x = normal.x;
    plane->y = normal.y;
    plane->z = normal.z;
    plane->w = -(normal.x * origin->x + normal.y * origin->y + normal.z * origin->z);
}

INCLUDE_ASM("asm/main/nonmatchings/face_point", setup_occlusion);

INCLUDE_ASM("asm/main/nonmatchings/face_point", check_occlusion);

INCLUDE_ASM("asm/main/nonmatchings/face_point", xglCullingCheck);

extern s32 gCullingSettings[];

s32 xglCullingGetSetting(void) {
    return gCullingSettings[0];
}

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C3C8);

extern s32 gCullingEnabled;

typedef struct CullingMapRecord {
    unsigned char unmodeled_00[0x8];
    s32 volumeFlags;
    unsigned char unmodeled_0C[0x4];
    unsigned char query[0x160 - 0x10];  /* +0x10: the query block func_0030E68C reads */
} CullingMapRecord;

typedef struct CullingMap {
    unsigned char unmodeled_00[0x20];
    unsigned char volumeRecords[0xDE0 - 0x20];
    s32 volumeCount;
    s32 unmodeled_DE4;
} CullingMap;

extern CullingMap gCullingVolumeTable[];

void xglCullingMapClear(void)
{
    CullingMap *map = gCullingVolumeTable;

    gCullingEnabled = 0;
    map->volumeCount = 0;
    map->unmodeled_00[0] = 0;
    map->unmodeled_DE4 = 0;
}

typedef struct CullingMapEntry {
    const char *name;
    /* The loop tests these pointer fields only for nullness. */
    const float *entryCount;
    const s32 *source;
    unsigned char unmodeled_00C[4];
} CullingMapEntry;

/*
 * Declared as an incomplete array, not a scalar: a plain scalar extern would
 * be classified as small data and emit a single gp-relative load, but the
 * original site is two absolute hi/lo instructions.
 */
extern const CullingMapEntry gCullingNameTable[];
s32 func_0030E68C(void *, void *);

/*
 * Finds a named culling-map entry in the current map table and returns its
 * index or minus one. The table ends at the first row with entryCount and
 * source both zero.
 */
s32 check_culling_map(const char *name) {
    s32 foundRowIndex = -1;
    s32 rowIndex;

    for (rowIndex = 0; gCullingNameTable[rowIndex].entryCount != 0 || gCullingNameTable[rowIndex].source != 0; rowIndex++) {
        if (func_0030E68C((void *)gCullingNameTable[rowIndex].name, (void *)name) == 0) {
            foundRowIndex = rowIndex;
            break;
        }
    }
    return foundRowIndex;
}

s32 xglCullingRecordQuery(CullingMapRecord *volumeRecord) {
    s32 queryResult;

    queryResult = volumeRecord->volumeFlags & 0x81;
    if (queryResult != 0) {
        queryResult = func_0030E68C(gCullingVolumeTable, volumeRecord->query);
        if (queryResult == 0) {
            /* xglCullingMapClear returns void; the call through this type is what the original emits. */
            queryResult = ((s32 (*)()) xglCullingMapClear)();
        }
    }
    return queryResult;
}

INCLUDE_ASM("asm/main/nonmatchings/face_point", func_0019C538);

INCLUDE_ASM("asm/main/nonmatchings/face_point", xglCullingMapCreate);

void xglCullingMapDisp(void)
{
    CullingMap *map = gCullingVolumeTable;
    unsigned char *volumeRecord;
    s32 remaining;

    if (map->volumeCount > 0) {
        volumeRecord = map->volumeRecords;
        for (remaining = map->volumeCount; remaining != 0; remaining--) {
            xglCullingVolumeDisplay(volumeRecord);
            volumeRecord += 0x160;
        }
    }
}

void xglCullingEnable(void) {
    gCullingEnabled = 1;
}

void xglCullingDisable(void) {
    gCullingEnabled = 0;
}

void xglCullingIdleHook(void) {

}

s32 xglCullingCheck();

void xglCullingPollCheck(void)
{
    s32 new_var;

    xglCullingCheck();
    new_var = 0;
    if (new_var) {
    }
}

void _ApplyMatrix33(void *rotatedVector, void *matrix, void *vector) {
    __asm__ __volatile__("lqc2 vf31, 0(%0)" : : "r"(vector) : "memory");
    __asm__ __volatile__("lqc2 vf27, 0(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf28, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf29, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf30, 48(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("vmulax.xyz ACC, vf27, vf31x" : : : "memory");
    __asm__ __volatile__("vmadday.xyz ACC, vf28, vf31y" : : : "memory");
    __asm__ __volatile__("vmaddz.xyz vf31, vf29, vf31z" : : : "memory");
    __asm__ __volatile__("sqc2 vf31, 0(%0)" : : "r"(rotatedVector) : "memory");
}

void applyMatrixPoint(void *transformedPoint, void *matrix, void *vector) {
    __asm__ __volatile__("lqc2 vf31, 0(%0)" : : "r"(vector) : "memory");
    __asm__ __volatile__("lqc2 vf27, 0(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf28, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf29, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf30, 48(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("vmulax.xyz ACC, vf27, vf31x" : : : "memory");
    __asm__ __volatile__("vmadday.xyz ACC, vf28, vf31y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyz ACC, vf29, vf31z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyz vf31, vf30, vf0w" : : : "memory");
    __asm__ __volatile__("sqc2 vf31, 0(%0)" : : "r"(transformedPoint) : "memory");
}

void _ApplyMatrix2Mat(void *transformedPoint, void *firstMatrix, void *secondMatrix, void *vector) {
    __asm__ __volatile__("lqc2 vf31, 0(%0)" : : "r"(vector) : "memory");
    __asm__ __volatile__("lqc2 vf27, 0(%0)" : : "r"(firstMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf28, 16(%0)" : : "r"(firstMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf29, 32(%0)" : : "r"(firstMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf30, 48(%0)" : : "r"(firstMatrix) : "memory");
    __asm__ __volatile__("vmulax.xyz ACC, vf27, vf31x" : : : "memory");
    __asm__ __volatile__("vmadday.xyz ACC, vf28, vf31y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyz ACC, vf29, vf31z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyz vf31, vf30, vf0w" : : : "memory");
    __asm__ __volatile__("lqc2 vf27, 0(%0)" : : "r"(secondMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf28, 16(%0)" : : "r"(secondMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf29, 32(%0)" : : "r"(secondMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf30, 48(%0)" : : "r"(secondMatrix) : "memory");
    __asm__ __volatile__("vmulax.xyz ACC, vf27, vf31x" : : : "memory");
    __asm__ __volatile__("vmadday.xyz ACC, vf28, vf31y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyz ACC, vf29, vf31z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyz vf31, vf30, vf0w" : : : "memory");
    __asm__ __volatile__("sqc2 vf31, 0(%0)" : : "r"(transformedPoint) : "memory");
}
