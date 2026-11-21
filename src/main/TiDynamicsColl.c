#include "common.h"
#include "shared.h"
/* Row-major 4x4 float matrix. */
#include "main/TiDynamicsColl.h"
#include "main/TiGlobalColl.h"
#include "main/TiMRPackage.h"
/* Address of a table at a byte displacement read from the data at runtime (not a constant field). */
#define BYTE_OFFSET(base, displacement) ((u8 *) (base) + (displacement))

/* The record tables sit at runtime byte offsets from hrcData (hrcData->xxxOffset), as in TiMRPackage.c: no typed
 * field of TiHrcData holds their pointers, so each table is reached through one cast of hrcData + displacement. */
typedef struct TiCollSphere { int matrixIndex; f32 radius; /* +0x04 */ unsigned int disableBits; /* +0x08: bit 0x1 disables the record */ unsigned char unmodeled_0C[0x4]; Vector4 localCenter; /* +0x10 */ Vector4 worldCenter; /* +0x20 */ } TiCollSphere;

void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void assertFailed(const char *message, const char *file, int line);

/* Transforms each sphere record from src into its centre with its global matrix. */
void tiDynamicsCollTransformVectors(TiMRPackage *mrPackage)
{
    TiCollSphere *sphere;
    unsigned int sphereIndex;

    sphere = (TiCollSphere *) BYTE_OFFSET(mrPackage->hrcData, mrPackage->hrcData->sphereOffset);
    if (mrPackage->globalMat == 0) {
        assertFailed("pMRPack->pGlobalMat", "TiDynamicsColl.euc.c", 17);
    }
    sphereIndex = 0;
    if (mrPackage->hrcData->sphereCount != 0) {
        do {
            /* sphere advances only for the entries that are transformed (original shape). */
            if (sphere->matrixIndex != -1) {
                xglVectorMulMat(&sphere->worldCenter, mrPackage->globalMat[sphere->matrixIndex], &sphere->localCenter);
                sphere++;
            }
            sphereIndex++;
        } while (sphereIndex < mrPackage->hrcData->sphereCount);
    }
}

TiGlobalCollSet *tiGlobalCollGetHandle(void);
void xacReadJointCurves12Padded(f32 *jointCurveValues, u8 **ppJointHeader, f32 curveParam);
u8 *xacReadJointCurves15(f32 *jointCurveValues, u8 **ppJointHeader, f32 curveParam);

/* Level from which tiGlobalCollEvalSubobject takes the second sub-object routine. */
#define TI_GLOBAL_COLL_LEVEL_LIMIT 1.5f

/* Offsets used by tiGlobalCollEvalSubobject; gaps are not modeled. */
typedef struct TiGlobalCollObject {
    unsigned char unmodeled_00[0x04];
    f32 level;               /* 0x04: compared with 1.5f in tiGlobalCollEvalSubobject */
    unsigned char unmodeled_08[0x48];
    unsigned char subobject; /* 0x50: its address is taken in tiGlobalCollEvalSubobject */
} TiGlobalCollObject;

/* The current collision handle: set by tiGlobalCollSetHandle, cleared by tiGlobalCollClearHandle
 * and read by tiGlobalCollGetHandle. Zero means that no collision set is active. */
extern TiGlobalCollSet *tiGlobalCollHandle;
/* Scaffold-owned float at 0x005501B4 (small data, gp-relative): the accumulate fraction of tiDynamicsCollClampAbovePlane. */
extern f32 gTiDynCollPlaneFraction;
/* Scaffold-owned float at 0x005501B8 (small data): the length used when the displacement of tiDynamicsCollPushPointOutOfSphere
 * is zero. */
extern f32 D_005501B8;
void xglMatrixInverse(Matrix4 destination, Matrix4 source);
void xglCurMatrixGet(Matrix4 matrix);
void tiVu0LoadIdentityMatrix(void);

/* One matrix record of the hierarchy's matrix table (0xD0 bytes): the index of its placed matrix (-1 for the
 * identity) and the matrix it fills. */
typedef struct TiCollMatrix {
    int placedMatrixIndex;                  /* +0x00 */
    unsigned int disableBits;         /* +0x04: bit 0x1 disables the record */
    unsigned char unmodeled_08[0x8];
    Matrix4 matrix10;           /* +0x10 */
    Matrix4 matrix50;           /* +0x50 */
    Matrix4 inverseGlobal;           /* +0x90 */
} TiCollMatrix;

/* Fills each matrix record from its placed matrix; index -1 loads the identity and stores the current matrix
 * instead. */
void tiDynamicsCollBuildInverseMatrices(TiMRPackage *mrPackage) {
    TiHrcData *hrcData;
    TiCollMatrix *matrixRecord;
    unsigned int matrixIndex;

    matrixIndex = 0;
    hrcData = mrPackage->hrcData;
    matrixRecord = (TiCollMatrix *) BYTE_OFFSET(hrcData, hrcData->matrixOffset);
    if (hrcData->matrixCount != 0) {
        do {
            if (matrixRecord->placedMatrixIndex == -1) {
                tiVu0LoadIdentityMatrix();
                xglCurMatrixGet(matrixRecord->inverseGlobal);
            } else {
                xglMatrixInverse(matrixRecord->inverseGlobal, mrPackage->globalMat[matrixRecord->placedMatrixIndex]);
            }
            matrixIndex++;
            matrixRecord++;
        } while (matrixIndex < mrPackage->hrcData->matrixCount);
    }
}

/* One capsule record of the hierarchy's capsule table (0xE0 bytes): the index of its placed matrix (-1 for the
 * identity) and the matrix it fills. */
typedef struct TiCollCapsule {
    int placedMatrixIndex;                  /* +0x00 */
    unsigned int disableBits;         /* +0x04: bit 0x1 disables the record */
    unsigned char unmodeled_08[0x8];
    f32 radius;                 /* +0x10 */
    f32 height;                 /* +0x14 */
    unsigned char unmodeled_18[0x8];
    Matrix4 matrix20;           /* +0x20 */
    Matrix4 matrix60;           /* +0x60 */
    Matrix4 inverseGlobal;             /* +0xA0 */
} TiCollCapsule;

/* Fills each capsule record from its placed matrix; index -1 loads the identity and stores the current matrix
 * instead. */
void tiDynamicsCollBuildWideInverseMatrices(TiMRPackage *mrPackage) {
    TiHrcData *hrcData;
    TiCollCapsule *capsuleRecord;
    unsigned int capsuleIndex;

    capsuleIndex = 0;
    hrcData = mrPackage->hrcData;
    capsuleRecord = (TiCollCapsule *) BYTE_OFFSET(hrcData, hrcData->capsuleOffset);
    if (hrcData->capsuleCount != 0) {
        do {
            if (capsuleRecord->placedMatrixIndex == -1) {
                tiVu0LoadIdentityMatrix();
                xglCurMatrixGet(capsuleRecord->inverseGlobal);
            } else {
                xglMatrixInverse(capsuleRecord->inverseGlobal, mrPackage->globalMat[capsuleRecord->placedMatrixIndex]);
            }
            capsuleIndex++;
            capsuleRecord++;
        } while (capsuleIndex < mrPackage->hrcData->capsuleCount);
    }
}

/* Pushes the solver body out of a sphere record: the displacement from the record centre is scaled to the radius when it
 * lies inside the radius on every axis and inside it in distance; the accumulator moves by 0.7f of the change. */
void tiDynamicsCollPushOutSphere(Vector4 *position, Vector4 *accumulator, const TiCollSphere *sphere)
{
    TiVector3 displacement;
    TiVector3 original;
    float distance;
    float radius;

    displacement.x = position->x - sphere->worldCenter.x;
    displacement.y = position->y - sphere->worldCenter.y;
    displacement.z = position->z - sphere->worldCenter.z;
    radius = sphere->radius + 0.0f;
    if (sphere->disableBits & 0x1) {
        return;
    }
    if (!(__builtin_fabsf(displacement.x) < radius)) {
        return;
    }
    if (!(__builtin_fabsf(displacement.y) < radius)) {
        return;
    }
    if (!(__builtin_fabsf(displacement.z) < radius)) {
        return;
    }
    distance = __builtin_sqrtf(displacement.x * displacement.x + displacement.y * displacement.y + displacement.z * displacement.z);
    if (distance == 0.0f) {
        distance = 1.0e-6f;
    }
    if (!(distance < radius)) {
        return;
    }
    distance = 1.0f / distance;
    original.x = position->x;
    original.y = position->y;
    original.z = position->z;
    displacement.x = displacement.x * distance;
    displacement.y = displacement.y * distance;
    displacement.z = displacement.z * distance;
    position->x = displacement.x * radius + sphere->worldCenter.x;
    position->y = displacement.y * radius + sphere->worldCenter.y;
    position->z = displacement.z * radius + sphere->worldCenter.z;
    accumulator->x += (position->x - original.x) * 0.7f;
    accumulator->y += (position->y - original.y) * 0.7f;
    accumulator->z += (position->z - original.z) * 0.7f;
}

/* Clamps the point above the record's plane (flags bit 0x1 skips the record). When the transformed point is below
 * the plane, the accumulator moves by 0.2f of the change and the position takes the clamped point. */
void tiDynamicsCollClampAbovePlane(Vector4 *position, Vector4 *accumulator, TiCollMatrix *matrixRecord, TiMRPackage *mrPackage) {
    Vector4 point;

    point.x = position->x;
    point.y = position->y;
    point.z = position->z;
    point.w = 1.0f;
    xglVectorMulMat(&point, matrixRecord->inverseGlobal, &point);
    xglVectorMulMat(&point, matrixRecord->matrix50, &point);
    if (matrixRecord->disableBits & 0x1) {
        return;
    }
    if (point.y < 0.0f) {
        point.y = 0.0f;
        xglVectorMulMat(&point, matrixRecord->matrix10, &point);
        xglVectorMulMat(&point, mrPackage->globalMat[matrixRecord->placedMatrixIndex], &point);
        accumulator->x += (point.x - position->x) * gTiDynCollPlaneFraction;
        accumulator->y += (point.y - position->y) * gTiDynCollPlaneFraction;
        accumulator->z += (point.z - position->z) * gTiDynCollPlaneFraction;
        position->x = point.x;
        position->y = point.y;
        position->z = point.z;
    }
}

/* Pushes the point out of the sphere around the centre when it lies inside the radius on every axis and inside it in
 * distance; a zero distance is replaced by 1.0e-7f. */
void tiDynamicsCollPushPointOutOfSphere(Vector4 *point, const Vector4 *worldCenter, float radius)
{
    TiVector3 delta;
    TiVector3 displacement;
    float distance;
    int i;

    displacement.x = point->x - worldCenter->x;
    displacement.y = point->y - worldCenter->y;
    displacement.z = point->z - worldCenter->z;
    delta = displacement;
    if (!(delta.x < radius)) {
        return;
    }
    if (!(delta.y < radius)) {
        return;
    }
    if (!(delta.z < radius)) {
        return;
    }
    distance = __builtin_sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (!(distance < radius)) {
        return;
    }
    if (distance == 0.0f) {
        distance = 1.0e-7f;
    }
    distance = 1.0f / distance;
    for (i = 0; i < 3; i++) {
        float *offsetAxis = &delta.x + i;
        const float *centerAxis = &worldCenter->x + i;
        float *pointAxis = &point->x + i;

        *offsetAxis = *offsetAxis * distance;
        *pointAxis = *offsetAxis * radius + *centerAxis;
    }
}

/* Centre of the capsule end spheres (scaffold-owned, initialised 0, 0, 0, 1); its y is set to the capsule height. */
extern Vector4 gCapsuleEndCenter;

/* Pushes the solver body out of a capsule: the point is moved into capsule space, pushed out of both end spheres and,
 * when its height lies within the capsule, pushed out radially; the accumulator moves by 0.7f of the change. */
void tiDynamicsCollPushOutCapsule(Vector4 *position, Vector4 *accumulator, TiCollCapsule *capsule, TiMRPackage *mrPackage)
{
    Vector4 point;
    float distance;
    float radius;

    point.w = 1.0f;
    point.x = position->x;
    point.y = position->y;
    point.z = position->z;
    xglVectorMulMat(&point, capsule->inverseGlobal, &point);
    xglVectorMulMat(&point, capsule->matrix60, &point);
    if (capsule->disableBits & 0x1) {
        return;
    }
    gCapsuleEndCenter.y = capsule->height;
    tiDynamicsCollPushPointOutOfSphere(&point, &gCapsuleEndCenter, capsule->radius);
    gCapsuleEndCenter.y = -capsule->height;
    tiDynamicsCollPushPointOutOfSphere(&point, &gCapsuleEndCenter, capsule->radius);
    if (!(__builtin_fabsf(point.y) < capsule->height + 0.0f)) {
        return;
    }
    distance = __builtin_sqrtf(point.x * point.x + point.z * point.z);
    radius = capsule->radius + 0.0f;
    if (!(distance < radius)) {
        return;
    }
    if (distance == 0.0f) {
        distance = 1.0e-7f;
    }
    distance = 1.0f / distance;
    point.x = point.x * distance * radius;
    point.z = point.z * distance * radius;
    xglVectorMulMat(&point, capsule->matrix20, &point);
    xglVectorMulMat(&point, mrPackage->globalMat[capsule->placedMatrixIndex], &point);
    accumulator->x += (point.x - position->x) * 0.7f;
    accumulator->y += (point.y - position->y) * 0.7f;
    accumulator->z += (point.z - position->z) * 0.7f;
    position->x = point.x;
    position->y = point.y;
    position->z = point.z;
}

INCLUDE_ASM("asm/main/nonmatchings/TiDynamicsColl", tiDynamicsCollPushOutSphereList);

/* Pushes the solver body out of every matrix record of the node, after the sphere entries. */
void tiDynamicsCollPushOutMatrixList(TiMRPackage *mrPackage, TiHrcDynamics *hrcNode, TiSolverState *solverState) {
    TiCollMatrix *matrixRecords;
    TiCollEntry *collEntry;
    unsigned int i;

    if (hrcNode->matrixCount == 0) {
        return;
    }
    matrixRecords = (TiCollMatrix *)BYTE_OFFSET(mrPackage->hrcData, mrPackage->hrcData->matrixOffset);
    collEntry = hrcNode->entries + hrcNode->sphereCount;
    for (i = 0; i < hrcNode->matrixCount; i++, collEntry++) {
        tiDynamicsCollClampAbovePlane(&solverState->position.vector, &solverState->accumulator.vector, &matrixRecords[collEntry->recordIndex], mrPackage);
    }
}

/* Pushes the solver body out of every capsule record of the node, after its sphere and matrix entries. */
void tiDynamicsCollPushOutCapsuleList(TiMRPackage *mrPackage, TiHrcDynamics *hrcNode, TiSolverState *solverState)
{
    TiCollCapsule *capsuleRecords;
    TiHrcData *hrcData;
    TiCollEntry *collEntry;
    unsigned int i;

    if (hrcNode->capsuleCount == 0) {
        return;
    }
    hrcData = mrPackage->hrcData;
    capsuleRecords = (TiCollCapsule *) BYTE_OFFSET(hrcData, hrcData->capsuleOffset);
    collEntry = hrcNode->entries + hrcNode->sphereCount + hrcNode->matrixCount;
    for (i = 0; i < hrcNode->capsuleCount; i++, collEntry++) {
        tiDynamicsCollPushOutCapsule(&solverState->position.vector, &solverState->accumulator.vector, &capsuleRecords[collEntry->recordIndex], mrPackage);
    }
}

/* Solves one contact pair of sphere records: the sphere entries of the first node push the pair's midpoint out, and
 * each correction moves both bodies and both accumulators. */
void tiDynamicsCollSolveSpherePair(TiSolverState *firstSolverBody, TiSolverState *secondSolverBody, TiMRPackage *mrPackage)
{
    DoublewordVector4 midpoint;
    DoublewordVector4 correction;
    TiCollSphere *sphereRecords;
    TiCollEntry *collEntry;
    unsigned int i;
    int contactRecordIndex;
    TiHrcData *hrcData;

    if (firstSolverBody->node->sphereCount == 0) {
        return;
    }
    collEntry = firstSolverBody->node->entries;
    hrcData = mrPackage->hrcData;
    sphereRecords = (TiCollSphere *) BYTE_OFFSET(hrcData, hrcData->sphereOffset);
    midpoint.vector.w = 1.0f;
    correction.vector.w = 1.0f;
    midpoint.vector.x = (secondSolverBody->position.vector.x - firstSolverBody->position.vector.x) * 0.5f + firstSolverBody->position.vector.x;
    correction.vector.x = 0.0f;
    midpoint.vector.y = (secondSolverBody->position.vector.y - firstSolverBody->position.vector.y) * 0.5f + firstSolverBody->position.vector.y;
    correction.vector.y = 0.0f;
    midpoint.vector.z = (secondSolverBody->position.vector.z - firstSolverBody->position.vector.z) * 0.5f + firstSolverBody->position.vector.z;
    correction.vector.z = 0.0f;
    for (i = 0; i < firstSolverBody->node->sphereCount; i++) {
        correction.vector.x = 0.0f;
        correction.vector.y = 0.0f;
        correction.vector.z = 0.0f;
        contactRecordIndex = collEntry->recordIndex;
        collEntry++;
        tiDynamicsCollPushOutSphere(&midpoint.vector, &correction.vector, &sphereRecords[contactRecordIndex]);
        firstSolverBody->position.vector.x += correction.vector.x;
        firstSolverBody->position.vector.y += correction.vector.y;
        firstSolverBody->position.vector.z += correction.vector.z;
        firstSolverBody->accumulator.vector.x += correction.vector.x;
        firstSolverBody->accumulator.vector.y += correction.vector.y;
        firstSolverBody->accumulator.vector.z += correction.vector.z;
        secondSolverBody->position.vector.x += correction.vector.x;
        secondSolverBody->position.vector.y += correction.vector.y;
        secondSolverBody->position.vector.z += correction.vector.z;
        secondSolverBody->accumulator.vector.x += correction.vector.x;
        secondSolverBody->accumulator.vector.y += correction.vector.y;
        secondSolverBody->accumulator.vector.z += correction.vector.z;
    }
}

/* Solves one contact pair of matrix records: the matrix entries of the first node (after its sphere entries) clamp the
 * midpoint above their planes, and each correction moves both bodies and both accumulators. */
void tiDynamicsCollSolveMatrixPair(TiSolverState *firstSolverBody, TiSolverState *secondSolverBody, TiMRPackage *mrPackage)
{
    DoublewordVector4 midpoint;
    DoublewordVector4 correction;
    TiCollEntry *collEntry;
    TiCollMatrix *matrixRecords;
    int matrixIndex;
    unsigned int sphereCount;
    TiHrcData *hrcData;
    unsigned int contactRecordIndex;

    if (firstSolverBody->node->matrixCount == 0) {
        return;
    }
    matrixIndex = 0;
    hrcData = mrPackage->hrcData;
    matrixRecords = (TiCollMatrix *) BYTE_OFFSET(hrcData, hrcData->matrixOffset);
    sphereCount = firstSolverBody->node->sphereCount;
    collEntry = &firstSolverBody->node->entries[sphereCount];
    midpoint.vector.w = 1.0f;
    correction.vector.w = 1.0f;
    midpoint.vector.x = (secondSolverBody->position.vector.x - firstSolverBody->position.vector.x) * 0.5f + firstSolverBody->position.vector.x;
    midpoint.vector.y = (secondSolverBody->position.vector.y - firstSolverBody->position.vector.y) * 0.5f + firstSolverBody->position.vector.y;
    midpoint.vector.z = (secondSolverBody->position.vector.z - firstSolverBody->position.vector.z) * 0.5f + firstSolverBody->position.vector.z;
    correction.vector.x = 0.0f;
    correction.vector.y = 0.0f;
    correction.vector.z = 0.0f;
    for (matrixIndex = 0; matrixIndex < firstSolverBody->node->matrixCount; matrixIndex++) {
        correction.vector.x = 0.0f;
        correction.vector.y = 0.0f;
        correction.vector.z = 0.0f;
        contactRecordIndex = collEntry->recordIndex;
        collEntry++;
        tiDynamicsCollClampAbovePlane(&midpoint.vector, &correction.vector, &matrixRecords[contactRecordIndex], mrPackage);
        firstSolverBody->position.vector.x = firstSolverBody->position.vector.x + correction.vector.x;
        firstSolverBody->position.vector.y = firstSolverBody->position.vector.y + correction.vector.y;
        firstSolverBody->position.vector.z = firstSolverBody->position.vector.z + correction.vector.z;
        firstSolverBody->accumulator.vector.x = firstSolverBody->accumulator.vector.x + correction.vector.x;
        firstSolverBody->accumulator.vector.y = firstSolverBody->accumulator.vector.y + correction.vector.y;
        firstSolverBody->accumulator.vector.z = firstSolverBody->accumulator.vector.z + correction.vector.z;
        secondSolverBody->position.vector.x = secondSolverBody->position.vector.x + correction.vector.x;
        secondSolverBody->position.vector.y = secondSolverBody->position.vector.y + correction.vector.y;
        secondSolverBody->position.vector.z = secondSolverBody->position.vector.z + correction.vector.z;
        secondSolverBody->accumulator.vector.x = secondSolverBody->accumulator.vector.x + correction.vector.x;
        secondSolverBody->accumulator.vector.y = secondSolverBody->accumulator.vector.y + correction.vector.y;
        secondSolverBody->accumulator.vector.z = secondSolverBody->accumulator.vector.z + correction.vector.z;
    }
}

/* Solves one contact pair of capsule records: the capsule entries of the first node (after its sphere and matrix
 * entries) push the pair's midpoint out, and each correction moves both bodies and both accumulators. */
void tiDynamicsCollSolveCapsulePair(TiSolverState *firstSolverBody, TiSolverState *secondSolverBody, TiMRPackage *mrPackage)
{
    DoublewordVector4 midpoint;
    DoublewordVector4 correction;
    TiCollCapsule *capsuleRecords;
    TiCollEntry *collEntry;
    unsigned int i;
    int contactRecordIndex;
    TiHrcData *hrcData;

    if (firstSolverBody->node->capsuleCount == 0) {
        return;
    }
    collEntry = &(&firstSolverBody->node->entries[firstSolverBody->node->sphereCount])[firstSolverBody->node->matrixCount];
    hrcData = mrPackage->hrcData;
    capsuleRecords = (TiCollCapsule *) BYTE_OFFSET(hrcData, hrcData->capsuleOffset);
    midpoint.vector.w = 1.0f;
    correction.vector.w = 1.0f;
    midpoint.vector.x = (secondSolverBody->position.vector.x - firstSolverBody->position.vector.x) * 0.5f + firstSolverBody->position.vector.x;
    correction.vector.x = 0.0f;
    midpoint.vector.y = (secondSolverBody->position.vector.y - firstSolverBody->position.vector.y) * 0.5f + firstSolverBody->position.vector.y;
    correction.vector.y = 0.0f;
    midpoint.vector.z = (secondSolverBody->position.vector.z - firstSolverBody->position.vector.z) * 0.5f + firstSolverBody->position.vector.z;
    correction.vector.z = 0.0f;
    for (i = 0; i < firstSolverBody->node->capsuleCount; i++) {
        correction.vector.x = 0.0f;
        correction.vector.y = 0.0f;
        correction.vector.z = 0.0f;
        contactRecordIndex = collEntry->recordIndex;
        collEntry++;
        tiDynamicsCollPushOutCapsule(&midpoint.vector, &correction.vector, &capsuleRecords[contactRecordIndex], mrPackage);
        firstSolverBody->position.vector.x += correction.vector.x;
        firstSolverBody->position.vector.y += correction.vector.y;
        firstSolverBody->position.vector.z += correction.vector.z;
        firstSolverBody->accumulator.vector.x += correction.vector.x;
        firstSolverBody->accumulator.vector.y += correction.vector.y;
        firstSolverBody->accumulator.vector.z += correction.vector.z;
        secondSolverBody->position.vector.x += correction.vector.x;
        secondSolverBody->position.vector.y += correction.vector.y;
        secondSolverBody->position.vector.z += correction.vector.z;
        secondSolverBody->accumulator.vector.x += correction.vector.x;
        secondSolverBody->accumulator.vector.y += correction.vector.y;
        secondSolverBody->accumulator.vector.z += correction.vector.z;
    }
}

/* Floors the height of a solver body at 0.03f; the height lost (0.7f of the old height) is removed from its accumulator. */
void tiGlobalCollClampScale(TiSolverState *solverState)
{
    f32 previousHeight;

    previousHeight = solverState->position.vector.y;
    if (previousHeight < 0.03f) {
        solverState->position.vector.y = 0.03f;
        solverState->accumulator.vector.y -= previousHeight * 0.7f;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiDynamicsColl", func_00296528);

TiGlobalCollSet *tiGlobalCollGetHandle(void) {
    return tiGlobalCollHandle;
}

/* Chooses the sub-object routine by obj->level: below 1.5f or not. */
void tiGlobalCollEvalSubobject(f32 *jointCurveValues, TiGlobalCollObject *collObject, f32 amount) {
    u8 *subobjectPtr;

    subobjectPtr = &collObject->subobject;
    if (collObject->level < TI_GLOBAL_COLL_LEVEL_LIMIT) {
        xacReadJointCurves12Padded(jointCurveValues, &subobjectPtr, amount);
        return;
    }
    xacReadJointCurves15(jointCurveValues, &subobjectPtr, amount);
}
