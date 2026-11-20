#include "common.h"
#include "shared.h"
/* Row-major 4x4 float matrix. */
#include "main/TiDynamicsColl.h"
#include "main/TiGlobalColl.h"
#include "main/TiMRPackage.h"

typedef struct TiCollSphere { int matrixIndex; f32 radius; /* +0x04 */ unsigned int flags; /* +0x08: bit 0x1 disables the record */ unsigned char unmodeled_0C[0x4]; Vector4 src; /* +0x10 */ Vector4 center; /* +0x20 */ } TiCollSphere;

void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void assertFailed(const char *message, const char *file, int line);

/* Transforms each sphere record from src into its centre with its global matrix. */
void tiDynamicsCollTransformVectors(TiMRPackage *pMRPack)
{
    TiCollSphere *entry;
    unsigned int entryIndex;

    entry = (TiCollSphere *) ((unsigned char *) pMRPack->hrcData + pMRPack->hrcData->sphereOffset);
    if (pMRPack->globalMat == 0) {
        assertFailed("pMRPack->pGlobalMat", "TiDynamicsColl.euc.c", 17);
    }
    entryIndex = 0;
    if (pMRPack->hrcData->sphereCount != 0) {
        do {
            /* entry advances only for the entries that are transformed (original shape). */
            if (entry->matrixIndex != -1) {
                xglVectorMulMat(&entry->center, pMRPack->globalMat[entry->matrixIndex], &entry->src);
                entry++;
            }
            entryIndex++;
        } while (entryIndex < pMRPack->hrcData->sphereCount);
    }
}

TiGlobalCollSet *tiGlobalCollGetHandle(void);
void xacReadJointCurves12Padded(f32 *values, u8 **ppJointHeader, f32 curveParam);
u8 *xacReadJointCurves15(f32 *values, u8 **ppJointHeader, f32 curveParam);

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
extern f32 D_005501B4;
/* Scaffold-owned float at 0x005501B8 (small data): the length used when the offset of tiDynamicsCollPushPointOutOfSphere
 * is zero. */
extern f32 D_005501B8;
void xglMatrixInverse(Matrix4 destination, Matrix4 source);
void xglCurMatrixGet(Matrix4 matrix);
void tiVu0LoadIdentityMatrix(void);

/* One matrix record of the hierarchy's matrix table (0xD0 bytes): the index of its placed matrix (-1 for the
 * identity) and the matrix it fills. */
typedef struct TiCollMatrix {
    int index;                  /* +0x00 */
    unsigned int flags;         /* +0x04: bit 0x1 disables the record */
    unsigned char unmodeled_08[0x8];
    Matrix4 matrix10;           /* +0x10 */
    Matrix4 matrix50;           /* +0x50 */
    Matrix4 matrix90;           /* +0x90 */
} TiCollMatrix;

/* Fills each matrix record from its placed matrix; index -1 loads the identity and stores the current matrix
 * instead. */
void tiDynamicsCollBuildInverseMatrices(TiMRPackage *pMRPackage) {
    TiHrcData *hrcData;
    TiCollMatrix *entry;
    unsigned int entryIndex;

    entryIndex = 0;
    hrcData = pMRPackage->hrcData;
    entry = (TiCollMatrix *) ((u8 *) hrcData + hrcData->matrixOffset);
    if (hrcData->matrixCount != 0) {
        do {
            if (entry->index == -1) {
                tiVu0LoadIdentityMatrix();
                xglCurMatrixGet(entry->matrix90);
            } else {
                xglMatrixInverse(entry->matrix90, pMRPackage->globalMat[entry->index]);
            }
            entryIndex++;
            entry++;
        } while (entryIndex < pMRPackage->hrcData->matrixCount);
    }
}

/* One capsule record of the hierarchy's capsule table (0xE0 bytes): the index of its placed matrix (-1 for the
 * identity) and the matrix it fills. */
typedef struct TiCollCapsule {
    int index;                  /* +0x00 */
    unsigned int flags;         /* +0x04: bit 0x1 disables the record */
    unsigned char unmodeled_08[0x8];
    f32 radius;                 /* +0x10 */
    f32 height;                 /* +0x14 */
    unsigned char unmodeled_18[0x8];
    Matrix4 matrix20;           /* +0x20 */
    Matrix4 matrix60;           /* +0x60 */
    Matrix4 matrix;             /* +0xA0 */
} TiCollCapsule;

/* Fills each capsule record from its placed matrix; index -1 loads the identity and stores the current matrix
 * instead. */
void tiDynamicsCollBuildWideInverseMatrices(TiMRPackage *pMRPackage) {
    TiHrcData *hrcData;
    TiCollCapsule *entry;
    unsigned int entryIndex;

    entryIndex = 0;
    hrcData = pMRPackage->hrcData;
    entry = (TiCollCapsule *) ((u8 *) hrcData + hrcData->capsuleOffset);
    if (hrcData->capsuleCount != 0) {
        do {
            if (entry->index == -1) {
                tiVu0LoadIdentityMatrix();
                xglCurMatrixGet(entry->matrix);
            } else {
                xglMatrixInverse(entry->matrix, pMRPackage->globalMat[entry->index]);
            }
            entryIndex++;
            entry++;
        } while (entryIndex < pMRPackage->hrcData->capsuleCount);
    }
}

/* Pushes the solver body out of a sphere record: the offset from the record centre is scaled to the radius when it
 * lies inside the radius on every axis and inside it in distance; the accumulator moves by 0.7f of the change. */
void tiDynamicsCollPushOutSphere(Vector4 *position, Vector4 *accumulator, const TiCollSphere *sphere)
{
    TiVector3 offset;
    TiVector3 original;
    float distance;
    float radius;

    offset.x = position->x - sphere->center.x;
    offset.y = position->y - sphere->center.y;
    offset.z = position->z - sphere->center.z;
    radius = sphere->radius + 0.0f;
    if (sphere->flags & 0x1) {
        return;
    }
    if (!(__builtin_fabsf(offset.x) < radius)) {
        return;
    }
    if (!(__builtin_fabsf(offset.y) < radius)) {
        return;
    }
    if (!(__builtin_fabsf(offset.z) < radius)) {
        return;
    }
    distance = __builtin_sqrtf(offset.x * offset.x + offset.y * offset.y + offset.z * offset.z);
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
    offset.x = offset.x * distance;
    offset.y = offset.y * distance;
    offset.z = offset.z * distance;
    position->x = offset.x * radius + sphere->center.x;
    position->y = offset.y * radius + sphere->center.y;
    position->z = offset.z * radius + sphere->center.z;
    accumulator->x += (position->x - original.x) * 0.7f;
    accumulator->y += (position->y - original.y) * 0.7f;
    accumulator->z += (position->z - original.z) * 0.7f;
}

/* Clamps the point above the record's plane (flags bit 0x1 skips the record). When the transformed point is below
 * the plane, the accumulator moves by 0.2f of the change and the position takes the clamped point. */
void tiDynamicsCollClampAbovePlane(Vector4 *position, Vector4 *accumulator, TiCollMatrix *record, TiMRPackage *object) {
    Vector4 point;

    point.x = position->x;
    point.y = position->y;
    point.z = position->z;
    point.w = 1.0f;
    xglVectorMulMat(&point, record->matrix90, &point);
    xglVectorMulMat(&point, record->matrix50, &point);
    if (record->flags & 0x1) {
        return;
    }
    if (point.y < 0.0f) {
        point.y = 0.0f;
        xglVectorMulMat(&point, record->matrix10, &point);
        xglVectorMulMat(&point, object->globalMat[record->index], &point);
        accumulator->x += (point.x - position->x) * D_005501B4;
        accumulator->y += (point.y - position->y) * D_005501B4;
        accumulator->z += (point.z - position->z) * D_005501B4;
        position->x = point.x;
        position->y = point.y;
        position->z = point.z;
    }
}

/* Pushes the point out of the sphere around the centre when it lies inside the radius on every axis and inside it in
 * distance; a zero distance is replaced by 1.0e-7f. */
void tiDynamicsCollPushPointOutOfSphere(Vector4 *point, const Vector4 *center, float radius)
{
    TiVector3 delta;
    TiVector3 offset;
    float distance;
    int i;

    offset.x = point->x - center->x;
    offset.y = point->y - center->y;
    offset.z = point->z - center->z;
    delta = offset;
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
        float *offset_axis = &delta.x + i;
        const float *center_axis = &center->x + i;
        float *point_axis = &point->x + i;

        *offset_axis = *offset_axis * distance;
        *point_axis = *offset_axis * radius + *center_axis;
    }
}

/* Centre of the capsule end spheres (scaffold-owned, initialised 0, 0, 0, 1); its y is set to the capsule height. */
extern Vector4 D_004CAB10;

/* Pushes the solver body out of a capsule: the point is moved into capsule space, pushed out of both end spheres and,
 * when its height lies within the capsule, pushed out radially; the accumulator moves by 0.7f of the change. */
void tiDynamicsCollPushOutCapsule(Vector4 *position, Vector4 *accumulator, TiCollCapsule *capsule, TiMRPackage *object)
{
    Vector4 point;
    float distance;
    float radius;

    point.w = 1.0f;
    point.x = position->x;
    point.y = position->y;
    point.z = position->z;
    xglVectorMulMat(&point, capsule->matrix, &point);
    xglVectorMulMat(&point, capsule->matrix60, &point);
    if (capsule->flags & 0x1) {
        return;
    }
    D_004CAB10.y = capsule->height;
    tiDynamicsCollPushPointOutOfSphere(&point, &D_004CAB10, capsule->radius);
    D_004CAB10.y = -capsule->height;
    tiDynamicsCollPushPointOutOfSphere(&point, &D_004CAB10, capsule->radius);
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
    xglVectorMulMat(&point, object->globalMat[capsule->index], &point);
    accumulator->x += (point.x - position->x) * 0.7f;
    accumulator->y += (point.y - position->y) * 0.7f;
    accumulator->z += (point.z - position->z) * 0.7f;
    position->x = point.x;
    position->y = point.y;
    position->z = point.z;
}

INCLUDE_ASM("asm/main/nonmatchings/TiDynamicsColl", tiDynamicsCollPushOutSphereList);

/* One collision entry of a dynamics node (16 bytes): the index of a record in the hierarchy's table. */
/* Pushes the solver body out of every matrix record of the node, after the sphere entries. */
void tiDynamicsCollPushOutMatrixList(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state) {
    TiCollMatrix *records;
    TiCollEntry *entry;
    unsigned int i;

    if (node->matrixCount == 0) {
        return;
    }
    records = (TiCollMatrix *)((u8 *)object->hrcData + object->hrcData->matrixOffset);
    entry = (TiCollEntry *)((u8 *)node + 0xD0) + node->sphereCount;
    for (i = 0; i < node->matrixCount; i++, entry++) {
        tiDynamicsCollClampAbovePlane(&state->position.vector, &state->accumulator.vector, &records[entry->recordIndex], object);
    }
}

/* Pushes the solver body out of a capsule: the point is moved into capsule space, pushed out of both end spheres and, * when its height lies within the capsule, pushed out radially; the accumulator moves by 0.7f of the change. */ void tiDynamicsCollPushOutCapsule(Vector4 *position, Vector4 *accumulator, TiCollCapsule *capsule, TiMRPackage *object);

/* Pushes the solver body out of every capsule record of the node, after its sphere and matrix entries. */
void tiDynamicsCollPushOutCapsuleList(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state)
{
    TiCollCapsule *records;
    TiHrcData *record_set;
    TiCollEntry *entry;
    unsigned int i;

    if (node->capsuleCount == 0) {
        return;
    }
    record_set = object->hrcData;
    records = (TiCollCapsule *) ((unsigned char *) record_set + record_set->capsuleOffset);
    entry = node->entries + node->sphereCount + node->matrixCount;
    for (i = 0; i < node->capsuleCount; i++, entry++) {
        tiDynamicsCollPushOutCapsule(&state->position.vector, &state->accumulator.vector, &records[entry->recordIndex], object);
    }
}

/* Pushes the solver body out of a sphere record: the offset from the record centre is scaled to the radius when it * lies inside the radius on every axis and inside it in distance; the accumulator moves by 0.7f of the change. */ void tiDynamicsCollPushOutSphere(Vector4 *position, Vector4 *accumulator, const TiCollSphere *sphere);

/* Solves one contact pair of sphere records: the sphere entries of the first node push the pair's midpoint out, and
 * each correction moves both bodies and both accumulators. */
void tiDynamicsCollSolveSpherePair(TiSolverState *first, TiSolverState *second, TiMRPackage *object)
{
    DoublewordVector4 midpoint;
    DoublewordVector4 correction;
    TiCollSphere *records;
    TiCollEntry *entry;
    unsigned int i;
    int record_index;
    TiHrcData *record_set;

    if (first->node->sphereCount == 0) {
        return;
    }
    entry = first->node->entries;
    record_set = object->hrcData;
    records = (TiCollSphere *) ((u8 *) record_set + record_set->sphereOffset);
    midpoint.vector.w = 1.0f;
    correction.vector.w = 1.0f;
    midpoint.vector.x = (second->position.vector.x - first->position.vector.x) * 0.5f + first->position.vector.x;
    correction.vector.x = 0.0f;
    midpoint.vector.y = (second->position.vector.y - first->position.vector.y) * 0.5f + first->position.vector.y;
    correction.vector.y = 0.0f;
    midpoint.vector.z = (second->position.vector.z - first->position.vector.z) * 0.5f + first->position.vector.z;
    correction.vector.z = 0.0f;
    for (i = 0; i < first->node->sphereCount; i++) {
        correction.vector.x = 0.0f;
        correction.vector.y = 0.0f;
        correction.vector.z = 0.0f;
        record_index = entry->recordIndex;
        entry++;
        tiDynamicsCollPushOutSphere(&midpoint.vector, &correction.vector, &records[record_index]);
        first->position.vector.x += correction.vector.x;
        first->position.vector.y += correction.vector.y;
        first->position.vector.z += correction.vector.z;
        first->accumulator.vector.x += correction.vector.x;
        first->accumulator.vector.y += correction.vector.y;
        first->accumulator.vector.z += correction.vector.z;
        second->position.vector.x += correction.vector.x;
        second->position.vector.y += correction.vector.y;
        second->position.vector.z += correction.vector.z;
        second->accumulator.vector.x += correction.vector.x;
        second->accumulator.vector.y += correction.vector.y;
        second->accumulator.vector.z += correction.vector.z;
    }
}

/* Solves one contact pair of matrix records: the matrix entries of the first node (after its sphere entries) clamp the
 * midpoint above their planes, and each correction moves both bodies and both accumulators. */
void tiDynamicsCollSolveMatrixPair(TiSolverState *first, TiSolverState *second, TiMRPackage *object)
{
    DoublewordVector4 midpoint;
    DoublewordVector4 correction;
    TiCollEntry *entry;
    TiCollMatrix *records;
    int processed;
    unsigned int sphereCount;
    TiHrcData *record_set;
    unsigned int record_index;

    if (first->node->matrixCount == 0) {
        return;
    }
    processed = 0;
    record_set = object->hrcData;
    records = (TiCollMatrix *) ((u8 *) record_set + record_set->matrixOffset);
    sphereCount = first->node->sphereCount;
    entry = &first->node->entries[sphereCount];
    midpoint.vector.w = 1.0f;
    correction.vector.w = 1.0f;
    midpoint.vector.x = (second->position.vector.x - first->position.vector.x) * 0.5f + first->position.vector.x;
    midpoint.vector.y = (second->position.vector.y - first->position.vector.y) * 0.5f + first->position.vector.y;
    midpoint.vector.z = (second->position.vector.z - first->position.vector.z) * 0.5f + first->position.vector.z;
    correction.vector.x = 0.0f;
    correction.vector.y = 0.0f;
    correction.vector.z = 0.0f;
    if (first->node->matrixCount != 0) {
        do {
            processed++;
            correction.vector.x = 0.0f;
            correction.vector.y = 0.0f;
            correction.vector.z = 0.0f;
            record_index = entry->recordIndex;
            entry++;
            tiDynamicsCollClampAbovePlane(&midpoint.vector, &correction.vector, &records[record_index], object);
            first->position.vector.x = first->position.vector.x + correction.vector.x;
            first->position.vector.y = first->position.vector.y + correction.vector.y;
            first->position.vector.z = first->position.vector.z + correction.vector.z;
            first->accumulator.vector.x = first->accumulator.vector.x + correction.vector.x;
            first->accumulator.vector.y = first->accumulator.vector.y + correction.vector.y;
            first->accumulator.vector.z = first->accumulator.vector.z + correction.vector.z;
            second->position.vector.x = second->position.vector.x + correction.vector.x;
            second->position.vector.y = second->position.vector.y + correction.vector.y;
            second->position.vector.z = second->position.vector.z + correction.vector.z;
            second->accumulator.vector.x = second->accumulator.vector.x + correction.vector.x;
            second->accumulator.vector.y = second->accumulator.vector.y + correction.vector.y;
            second->accumulator.vector.z = second->accumulator.vector.z + correction.vector.z;
        } while (processed < first->node->matrixCount);
    }
}

/* Solves one contact pair of capsule records: the capsule entries of the first node (after its sphere and matrix
 * entries) push the pair's midpoint out, and each correction moves both bodies and both accumulators. */
void tiDynamicsCollSolveCapsulePair(TiSolverState *first, TiSolverState *second, TiMRPackage *object)
{
    DoublewordVector4 midpoint;
    DoublewordVector4 correction;
    TiCollCapsule *records;
    TiCollEntry *entry;
    unsigned int i;
    int record_index;
    TiHrcData *record_set;

    if (first->node->capsuleCount == 0) {
        return;
    }
    entry = &(&first->node->entries[first->node->sphereCount])[first->node->matrixCount];
    record_set = object->hrcData;
    records = (TiCollCapsule *) ((u8 *) record_set + record_set->capsuleOffset);
    midpoint.vector.w = 1.0f;
    correction.vector.w = 1.0f;
    midpoint.vector.x = (second->position.vector.x - first->position.vector.x) * 0.5f + first->position.vector.x;
    correction.vector.x = 0.0f;
    midpoint.vector.y = (second->position.vector.y - first->position.vector.y) * 0.5f + first->position.vector.y;
    correction.vector.y = 0.0f;
    midpoint.vector.z = (second->position.vector.z - first->position.vector.z) * 0.5f + first->position.vector.z;
    correction.vector.z = 0.0f;
    for (i = 0; i < first->node->capsuleCount; i++) {
        correction.vector.x = 0.0f;
        correction.vector.y = 0.0f;
        correction.vector.z = 0.0f;
        record_index = entry->recordIndex;
        entry++;
        tiDynamicsCollPushOutCapsule(&midpoint.vector, &correction.vector, &records[record_index], object);
        first->position.vector.x += correction.vector.x;
        first->position.vector.y += correction.vector.y;
        first->position.vector.z += correction.vector.z;
        first->accumulator.vector.x += correction.vector.x;
        first->accumulator.vector.y += correction.vector.y;
        first->accumulator.vector.z += correction.vector.z;
        second->position.vector.x += correction.vector.x;
        second->position.vector.y += correction.vector.y;
        second->position.vector.z += correction.vector.z;
        second->accumulator.vector.x += correction.vector.x;
        second->accumulator.vector.y += correction.vector.y;
        second->accumulator.vector.z += correction.vector.z;
    }
}

/* Floors the height of a solver body at 0.03f; the height lost (0.7f of the old height) is removed from its accumulator. */
void tiGlobalCollClampScale(TiSolverState *state)
{
    f32 previousHeight;

    previousHeight = state->position.vector.y;
    if (previousHeight < 0.03f) {
        state->position.vector.y = 0.03f;
        state->accumulator.vector.y -= previousHeight * 0.7f;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiDynamicsColl", func_00296528);

TiGlobalCollSet *tiGlobalCollGetHandle(void) {
    return tiGlobalCollHandle;
}

/* Chooses the sub-object routine by obj->level: below 1.5f or not. */
void tiGlobalCollEvalSubobject(f32 *values, TiGlobalCollObject *obj, f32 amount) {
    u8 *subobjectPtr;

    subobjectPtr = &obj->subobject;
    if (obj->level < TI_GLOBAL_COLL_LEVEL_LIMIT) {
        xacReadJointCurves12Padded(values, &subobjectPtr, amount);
        return;
    }
    xacReadJointCurves15(values, &subobjectPtr, amount);
}
