#include "common.h"
#include "shared.h"
#include "main/TiDynamicsColl.h"
#include "main/TiGlobalColl.h"
#include "main/TiMRPackage.h"

/* Sample block written by func_0029BEE0: the x and z of the scale and the blend, then the translate and the
 * three rotation angles. The tail is written by the callee and not read here. */
typedef struct TiSampleBlock {
    f32 scaleX;                 /* +0x00 */
    f32 scaleZ;                 /* +0x04 */
    f32 blend;                  /* +0x08 */
    TiVector3 translate;        /* +0x0c */
    TiVector3 rotate;           /* +0x18 */
    unsigned char unmodeled_24[0x0c];
} TiSampleBlock;

/* Scale vector passed to xglCurMulScaleXYZ; z is the raw word copied from the samples. */
typedef struct TiScaleVector {
    f32 x;
    f32 y;
    f32 z;
} TiScaleVector;

/* Transform cell written by tiTransformCellBuild: unit weight, blend, zero, then the matrix and its inverse. */
typedef struct TiTransformCell {
    f32 unitWeight;             /* +0x00: set to 1.0f */
    f32 blend;                  /* +0x04 */
    f32 zero;                   /* +0x08 */
    unsigned char unmodeled_0C[0x4];
    Matrix4 matrix;             /* +0x10 */
    Matrix4 inverse;            /* +0x50 */
} TiTransformCell;

void func_0029BEE0(TiSampleBlock *sampledBlock, int *sampleCursor, int sampleMode, int count);
void xglCurSetTranslate(f32 *translate);
void xglCurMulRotateX(f32 angle);
void xglCurMulRotateY(f32 angle);
void xglCurMulRotateZ(f32 angle);
void xglCurMulScaleXYZ(f32 *scale);
void xglCurMatrixGet(Matrix4 matrix);
void xglMatrixInverse(Matrix4 destination, Matrix4 source);
void xglCurMatrixSet(Matrix4 matrix);
void xglCurApplyVector(Vector4 *dst, Vector4 *src);
void xglVectorMulMat(Vector4 *dst, Matrix4 matrix, const Vector4 *src);
void xglCurMulTranslate(f32 *translate);
void tiVu0LoadIdentityMatrix(void);

/* Below this threshold the node slots are indexed by key; otherwise the keyed entries are searched. */
#define TI_NODE_INDEXED_THRESHOLD 1.1f

/* Builds the current matrix of the node entry for key: the entry's global matrix when useGlobalMatrix is set (else the
 * identity), then its translate and its rotations Z, Y and X, stored into nodeMatrix. Below the threshold the slots are
 * indexed by key; otherwise the first keyed entry with that id is used. */
int tiMRPackageGetNodeMatrix(TiMRPackage *pMRPack, u32 key, Matrix4 nodeMatrix, int useGlobalMatrix) {
    struct TiNodeTable *nodePointTable;
    TiNodeSlot *indexedEntries;
    TiNodeKey *keyedEntries;
    TiNodeKey *found;
    u32 keyedNodeIndex;
    int isNodeFound;

    isNodeFound = 0;
    nodePointTable = pMRPack->nodeTable;
    if (nodePointTable == 0) {
        return isNodeFound;
    }
    if (nodePointTable->threshold < TI_NODE_INDEXED_THRESHOLD) {
        indexedEntries = nodePointTable->slots;
        if (key >= nodePointTable->count) {
            return isNodeFound;
        }
        if (useGlobalMatrix != 0) {
            xglCurMatrixSet(pMRPack->globalMat[indexedEntries[key].matrixIndex]);
        } else {
            tiVu0LoadIdentityMatrix();
        }
        if (indexedEntries[key].offset[0] != 0.0f || indexedEntries[key].offset[1] != 0.0f || indexedEntries[key].offset[2] != 0.0f) {
            xglCurMulTranslate(indexedEntries[key].offset);
        }
        if (indexedEntries[key].rotation[2] != 0.0f) {
            xglCurMulRotateZ(indexedEntries[key].rotation[2]);
        }
        if (indexedEntries[key].rotation[1] != 0.0f) {
            xglCurMulRotateY(indexedEntries[key].rotation[1]);
        }
        if (indexedEntries[key].rotation[0] != 0.0f) {
            xglCurMulRotateX(indexedEntries[key].rotation[0]);
        }
        xglCurMatrixGet(nodeMatrix);
        isNodeFound = 1;
    } else {
        keyedEntries = (TiNodeKey *) nodePointTable->slots;
        found = 0;
        for (keyedNodeIndex = 0; keyedNodeIndex < pMRPack->nodeTable->count; keyedNodeIndex++) {
            if (keyedEntries[keyedNodeIndex].id == key) {
                found = &keyedEntries[keyedNodeIndex];
                break;
            }
        }
        if (found == 0) {
            return isNodeFound;
        }
        if (useGlobalMatrix != 0) {
            xglCurMatrixSet(pMRPack->globalMat[found->matrixIndex]);
        } else {
            tiVu0LoadIdentityMatrix();
        }
        if (found->offset[0] != 0.0f || found->offset[1] != 0.0f || found->offset[2] != 0.0f) {
            xglCurMulTranslate(found->offset);
        }
        if (found->rotation[2] != 0.0f) {
            xglCurMulRotateZ(found->rotation[2]);
        }
        if (found->rotation[1] != 0.0f) {
            xglCurMulRotateY(found->rotation[1]);
        }
        if (found->rotation[0] != 0.0f) {
            xglCurMulRotateX(found->rotation[0]);
        }
        xglCurMatrixGet(nodeMatrix);
        isNodeFound = 1;
    }
    return isNodeFound;
}

/* Writes the position of the node entry for key into nodePoint with w = 1. Below the threshold the slots are indexed by
 * key; otherwise the first keyed entry with that id is used. With useGlobalMatrix set, nodePoint is then transformed by the
 * global matrix that the entry names. */
int tiMRPackageGetNodePoint(TiMRPackage *pMRPack, u32 key, Vector4 *nodePoint, int useGlobalMatrix) {
    struct TiNodeTable *nodePointTable;
    TiNodeSlot *indexedEntries;
    TiNodeKey *keyedEntries;
    TiNodeKey *found;
    u32 keyedNodeIndex;

    nodePointTable = pMRPack->nodeTable;
    if (nodePointTable == 0) {
        return 0;
    }
    if (nodePointTable->threshold < TI_NODE_INDEXED_THRESHOLD) {
        indexedEntries = nodePointTable->slots;
        if (key >= nodePointTable->count) {
            return 0;
        }
        nodePoint->x = indexedEntries[key].offset[0];
        nodePoint->y = indexedEntries[key].offset[1];
        nodePoint->z = indexedEntries[key].offset[2];
        nodePoint->w = 1.0f;
        if (useGlobalMatrix != 0) {
            xglCurMatrixSet(pMRPack->globalMat[indexedEntries[key].matrixIndex]);
            xglCurApplyVector(nodePoint, nodePoint);
            return 1;
        }
        return 1;
    } else {
        keyedEntries = (TiNodeKey *) nodePointTable->slots;
        found = 0;
        for (keyedNodeIndex = 0; keyedNodeIndex < pMRPack->nodeTable->count; keyedNodeIndex++) {
            if (keyedEntries[keyedNodeIndex].id == key) {
                found = &keyedEntries[keyedNodeIndex];
                break;
            }
        }
        if (found == 0) {
            return 0;
        }
        nodePoint->x = found->offset[0];
        nodePoint->y = found->offset[1];
        nodePoint->z = found->offset[2];
        nodePoint->w = 1.0f;
        if (useGlobalMatrix != 0) {
            xglCurMatrixSet(pMRPack->globalMat[found->matrixIndex]);
            xglCurApplyVector(nodePoint, nodePoint);
        }
        return 1;
    }
}

/* Sphere entry of a global collision set (0x20 bytes): the radius at +0x00 and the centre at +0x10. */
typedef struct TiGlobalCollSphere {
    float radius;               /* +0x00 */
    unsigned char unmodeled_04[12];
    Vector4 centre;             /* +0x10 */
} TiGlobalCollSphere;
typedef int s32;
extern s32 D_004C9A00[];

void tiGlobalCollClearCounters(void) {
    D_004C9A00[0] = 0;
    D_004C9A00[1] = 0;
    D_004C9A00[2] = 0;
}

void assertFailed(const char *message, const char *file, int line);
TiGlobalCollSet *tiGlobalCollGetHandle(void);
extern const char tiGlobalCollAssertMessage[];
extern const char tiGlobalCollAssertFile[];
void func_00296B80(TiGlobalCollSet *collisionSet, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);
void func_00296D28(TiGlobalCollSet *collisionSet, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);
void func_00296F48(TiGlobalCollSet *collisionSet, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);

/* Pushes the body out of one sphere: inside the sphere the body moves to its surface along the unit direction
 * from the centre, and its accumulator gains the push scaled by the penetration depth. */
void tiGlobalCollPushOutSphere(TiGlobalCollSphere *sphere, void *unused1, TiSolverState *solverBody) {
    Vector4 centreToBody;
    float lengthSquared;
    float distance;
    float depth;

    centreToBody.x = solverBody->position.vector.x - sphere->centre.x;
    centreToBody.y = solverBody->position.vector.y - sphere->centre.y;
    centreToBody.z = solverBody->position.vector.z - sphere->centre.z;
    if (__builtin_fabsf(centreToBody.x) < sphere->radius) {
        if (__builtin_fabsf(centreToBody.y) < sphere->radius) {
            if (__builtin_fabsf(centreToBody.z) < sphere->radius) {
                lengthSquared = centreToBody.x * centreToBody.x + centreToBody.y * centreToBody.y + centreToBody.z * centreToBody.z;
                distance = __builtin_sqrtf(lengthSquared);
                if (distance == 0.0f) {
                    distance = 0.000001f;
                }
                if (distance < sphere->radius) {
                    depth = sphere->radius - distance;
                    distance = 1.0f / distance;
                    centreToBody.x = centreToBody.x * distance;
                    centreToBody.y = centreToBody.y * distance;
                    centreToBody.z = centreToBody.z * distance;
                    solverBody->accumulator.vector.x = solverBody->accumulator.vector.x + centreToBody.x * sphere->radius * depth;
                    solverBody->accumulator.vector.y = solverBody->accumulator.vector.y + centreToBody.y * sphere->radius * depth;
                    solverBody->accumulator.vector.z = solverBody->accumulator.vector.z + centreToBody.z * sphere->radius * depth;
                    solverBody->position.vector.x = centreToBody.x * sphere->radius + sphere->centre.x;
                    solverBody->position.vector.y = centreToBody.y * sphere->radius + sphere->centre.y;
                    solverBody->position.vector.z = centreToBody.z * sphere->radius + sphere->centre.z;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00296B80);

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00296C58);

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00296D28);

/* Pushes the body out of one cylinder: its position is taken into the cylinder space with toLocal; inside the half
 * height and the radius, the horizontal distance is pulled back to the radius and the point is mapped back with
 * toWorld. */
void tiGlobalCollPushOutCylinder(TiGlobalCollCylinder *cylinder, void *unused1, TiSolverState *solverBody) {
    Vector4 localPoint;
    float radialDistance;

    localPoint.x = solverBody->position.vector.x;
    localPoint.y = solverBody->position.vector.y;
    localPoint.z = solverBody->position.vector.z;
    localPoint.w = 1.0f;
    xglVectorMulMat(&localPoint, cylinder->toLocal, &localPoint);
    if (__builtin_fabsf(localPoint.y) < cylinder->halfHeight) {
        radialDistance = localPoint.x;
        radialDistance = radialDistance * localPoint.x;
        radialDistance = radialDistance + localPoint.z * localPoint.z;
        radialDistance = __builtin_sqrtf(radialDistance);
        if (radialDistance < cylinder->radius) {
            if (radialDistance == 0.0f) {
                radialDistance = 0.0000001f;
            }
            radialDistance = 1.0f / radialDistance;
            localPoint.x = localPoint.x * radialDistance * cylinder->radius;
            localPoint.z = localPoint.z * radialDistance * cylinder->radius;
            xglVectorMulMat(&localPoint, cylinder->toWorld, &localPoint);
            solverBody->position.vector.x = localPoint.x;
            solverBody->position.vector.y = localPoint.y;
            solverBody->position.vector.z = localPoint.z;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00296F48);

/* Solves one body against the shapes of the current collision set: the three solver passes take the handle, the
 * node and the body in turn. */
void tiGlobalCollSolveBody(TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    TiGlobalCollSet *collisionSet;

    collisionSet = tiGlobalCollGetHandle();
    if (dynamicsNode == 0) {
        assertFailed(tiGlobalCollAssertMessage, tiGlobalCollAssertFile, 197);
    }
    func_00296B80(collisionSet, dynamicsNode, solverBody);
    func_00296D28(collisionSet, dynamicsNode, solverBody);
    func_00296F48(collisionSet, dynamicsNode, solverBody);
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_002970A8);

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00297100);

/* Builds the cell's current matrix from the sampled translate, rotation and scale, then its inverse. */
void tiTransformCellBuild(void *unused0, int sampleMode, void *unused2, int sampleCursor, TiTransformCell *cell) {
    TiSampleBlock sampledBlock;
    TiScaleVector scale;
    TiScaleVector sample;
    int sampleParam;

    sampleParam = sampleCursor;
    func_0029BEE0(&sampledBlock, &sampleParam, sampleMode, 9);
    xglCurSetTranslate(&sampledBlock.translate.x);
    xglCurMulRotateZ(sampledBlock.rotate.z);
    xglCurMulRotateY(sampledBlock.rotate.y);
    xglCurMulRotateX(sampledBlock.rotate.x);
    sample.x = sampledBlock.scaleX;
    sample.y = 1.0f;
    sample.z = sampledBlock.scaleZ;
    scale = sample;
    xglCurMulScaleXYZ(&scale.x);
    xglCurMatrixGet(cell->matrix);
    xglMatrixInverse(cell->inverse, cell->matrix);
    cell->unitWeight = 1.0f;
    cell->zero = 0.0f;
    cell->blend = sampledBlock.blend;
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00297240);
