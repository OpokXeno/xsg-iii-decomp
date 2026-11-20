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

void func_0029BEE0(TiSampleBlock *block, int *param, int mode, int count);
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

/* Builds the current matrix of the node entry for key: the entry's global matrix when apply is set (else the
 * identity), then its translate and its rotations Z, Y and X, stored into dst. Below the threshold the slots are
 * indexed by key; otherwise the first keyed entry with that id is used. */
int tiMRPackageGetNodeMatrix(TiMRPackage *pMRPack, u32 key, Matrix4 dst, int apply) {
    struct TiNodeTable *pStatus;
    TiNodeSlot *indexedEntries;
    TiNodeKey *keyedEntries;
    TiNodeKey *found;
    u32 entryIndex;
    int result;

    result = 0;
    pStatus = pMRPack->nodeTable;
    if (pStatus == 0) {
        return result;
    }
    if (pStatus->threshold < TI_NODE_INDEXED_THRESHOLD) {
        indexedEntries = pStatus->slots;
        if (key >= pStatus->count) {
            return result;
        }
        if (apply != 0) {
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
        xglCurMatrixGet(dst);
        result = 1;
    } else {
        keyedEntries = (TiNodeKey *) pStatus->slots;
        found = 0;
        for (entryIndex = 0; entryIndex < pMRPack->nodeTable->count; entryIndex++) {
            if (keyedEntries[entryIndex].id == key) {
                found = &keyedEntries[entryIndex];
                break;
            }
        }
        if (found == 0) {
            return result;
        }
        if (apply != 0) {
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
        xglCurMatrixGet(dst);
        result = 1;
    }
    return result;
}

/* Writes the position of the node entry for key into dst with w = 1. Below the threshold the slots are indexed by
 * key; otherwise the first keyed entry with that id is used. With apply set, dst is then transformed by the
 * global matrix that the entry names. */
int tiMRPackageGetNodePoint(TiMRPackage *pMRPack, u32 key, Vector4 *dst, int apply) {
    struct TiNodeTable *pStatus;
    TiNodeSlot *indexedEntries;
    TiNodeKey *keyedEntries;
    TiNodeKey *found;
    u32 entryIndex;

    pStatus = pMRPack->nodeTable;
    if (pStatus == 0) {
        return 0;
    }
    if (pStatus->threshold < TI_NODE_INDEXED_THRESHOLD) {
        indexedEntries = pStatus->slots;
        if (key >= pStatus->count) {
            return 0;
        }
        dst->x = indexedEntries[key].offset[0];
        dst->y = indexedEntries[key].offset[1];
        dst->z = indexedEntries[key].offset[2];
        dst->w = 1.0f;
        if (apply != 0) {
            xglCurMatrixSet(pMRPack->globalMat[indexedEntries[key].matrixIndex]);
            xglCurApplyVector(dst, dst);
            return 1;
        }
        return 1;
    } else {
        keyedEntries = (TiNodeKey *) pStatus->slots;
        found = 0;
        for (entryIndex = 0; entryIndex < pMRPack->nodeTable->count; entryIndex++) {
            if (keyedEntries[entryIndex].id == key) {
                found = &keyedEntries[entryIndex];
                break;
            }
        }
        if (found == 0) {
            return 0;
        }
        dst->x = found->offset[0];
        dst->y = found->offset[1];
        dst->z = found->offset[2];
        dst->w = 1.0f;
        if (apply != 0) {
            xglCurMatrixSet(pMRPack->globalMat[found->matrixIndex]);
            xglCurApplyVector(dst, dst);
        }
        return 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_002969A8);

/* Sphere entry of a global collision set (0x20 bytes): the radius at +0x00 and the centre at +0x10. */
typedef struct TiGlobalCollSphere {
    float radius;               /* +0x00 */
    unsigned char unmodeled_04[12];
    Vector4 centre;             /* +0x10 */
} TiGlobalCollSphere;

void assertFailed(const char *message, const char *file, int line);
TiGlobalCollSet *tiGlobalCollGetHandle(void);
extern const char D_005427F8[];
extern const char D_00542800[];
void func_00296B80(TiGlobalCollSet *handle, TiHrcDynamics *pNode, TiSolverState *body);
void func_00296D28(TiGlobalCollSet *handle, TiHrcDynamics *pNode, TiSolverState *body);
void func_00296F48(TiGlobalCollSet *handle, TiHrcDynamics *pNode, TiSolverState *body);

/* Pushes the body out of one sphere: inside the sphere the body moves to its surface along the unit direction
 * from the centre, and its accumulator gains the push scaled by the penetration depth. */
void tiGlobalCollPushOutSphere(TiGlobalCollSphere *sphere, void *unused1, TiSolverState *body) {
    Vector4 offset;
    float lengthSquared;
    float distance;
    float depth;

    offset.x = body->position.vector.x - sphere->centre.x;
    offset.y = body->position.vector.y - sphere->centre.y;
    offset.z = body->position.vector.z - sphere->centre.z;
    if (__builtin_fabsf(offset.x) < sphere->radius) {
        if (__builtin_fabsf(offset.y) < sphere->radius) {
            if (__builtin_fabsf(offset.z) < sphere->radius) {
                lengthSquared = offset.x * offset.x + offset.y * offset.y + offset.z * offset.z;
                distance = __builtin_sqrtf(lengthSquared);
                if (distance == 0.0f) {
                    distance = 0.000001f;
                }
                if (distance < sphere->radius) {
                    depth = sphere->radius - distance;
                    distance = 1.0f / distance;
                    offset.x = offset.x * distance;
                    offset.y = offset.y * distance;
                    offset.z = offset.z * distance;
                    body->accumulator.vector.x = body->accumulator.vector.x + offset.x * sphere->radius * depth;
                    body->accumulator.vector.y = body->accumulator.vector.y + offset.y * sphere->radius * depth;
                    body->accumulator.vector.z = body->accumulator.vector.z + offset.z * sphere->radius * depth;
                    body->position.vector.x = offset.x * sphere->radius + sphere->centre.x;
                    body->position.vector.y = offset.y * sphere->radius + sphere->centre.y;
                    body->position.vector.z = offset.z * sphere->radius + sphere->centre.z;
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
void tiGlobalCollPushOutCylinder(TiGlobalCollCylinder *cylinder, void *unused1, TiSolverState *body) {
    Vector4 localPoint;
    float radialDistance;

    localPoint.x = body->position.vector.x;
    localPoint.y = body->position.vector.y;
    localPoint.z = body->position.vector.z;
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
            body->position.vector.x = localPoint.x;
            body->position.vector.y = localPoint.y;
            body->position.vector.z = localPoint.z;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00296F48);

/* Solves one body against the shapes of the current collision set: the three solver passes take the handle, the
 * node and the body in turn. */
void tiGlobalCollSolveBody(TiHrcDynamics *pNode, TiSolverState *body) {
    TiGlobalCollSet *handle;

    handle = tiGlobalCollGetHandle();
    if (pNode == 0) {
        assertFailed(D_005427F8, D_00542800, 197);
    }
    func_00296B80(handle, pNode, body);
    func_00296D28(handle, pNode, body);
    func_00296F48(handle, pNode, body);
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_002970A8);

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00297100);

/* Builds the cell's current matrix from the sampled translate, rotation and scale, then its inverse. */
void tiTransformCellBuild(void *unused0, int mode, void *unused2, int param, TiTransformCell *cell) {
    TiSampleBlock block;
    TiScaleVector scale;
    TiScaleVector sample;
    int sampleParam;

    sampleParam = param;
    func_0029BEE0(&block, &sampleParam, mode, 9);
    xglCurSetTranslate(&block.translate.x);
    xglCurMulRotateZ(block.rotate.z);
    xglCurMulRotateY(block.rotate.y);
    xglCurMulRotateX(block.rotate.x);
    sample.x = block.scaleX;
    sample.y = 1.0f;
    sample.z = block.scaleZ;
    scale = sample;
    xglCurMulScaleXYZ(&scale.x);
    xglCurMatrixGet(cell->matrix);
    xglMatrixInverse(cell->inverse, cell->matrix);
    cell->unitWeight = 1.0f;
    cell->zero = 0.0f;
    cell->blend = block.blend;
}

INCLUDE_ASM("asm/main/nonmatchings/TiGlobalColl", func_00297240);
