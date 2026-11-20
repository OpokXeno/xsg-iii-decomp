#include "common.h"
#include "shared.h"

void xacEvalJointCurves9(f32 *values, u8 *pJointHeader, f32 curveParam);

typedef struct TiMotionOffsetBlock {
    unsigned char unmodeled_00[0x30];
    int joint_header_offset;
} TiMotionOffsetBlock;

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_002998C0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_00299C70);


/* One transform: parent index, offset, rotation and scale. */
typedef struct TiMotionTransform {
    int parent;
    float offset[3];
    float rotation[3];
    float scale[3];
} TiMotionTransform;

void tiVu0LoadIdentityMatrix(void);
void xglCurMulTranslate(float *translation);
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurMulRotateZ(float angle);
void xglCurMulScaleXYZ(const f32 scale[3]);
void xglCurMatrixGet(Matrix4 matrix);

#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"
#include "main/TiXacAnimCurve.h"













void assertFailed(const char *message, const char *file, int line);
void xglCurMatrixSet(Matrix4 matrix);
void xglCurMatrixMul(Matrix4 matrix);
TiHrcDynamics *func_002980A0(Matrix4 *localMat, TiMRPackage *pMRPack, TiHrcDynamics *record, TiSolverState *state);
void tiDynamicsSolveCellPairs(TiMRPackage *pMRPack);
f32 *xacFindCurveKey(int *pFlag, XacCurveKeyTable *pTable, f32 curveParam);
extern const unsigned char D_00542A50[];
extern const unsigned char D_00542AB0[];
extern const unsigned char D_00542AB8[];
extern const unsigned char D_00542A10[];
extern const unsigned char D_00542A30[];

/* Scale lookup for node `index` in the pMRPack table at +0xD0; asserts "can not found parent scale". */
float *tiMotionFindParentScale(TiMRPackage *pMRPack, int index)
{
    TiHrcData *hrc;
    TiHrcSkip *skips;
    TiHrcDynamics *dynamics;
    unsigned char *vectors;
    int vectorBytes;
    int dynamicsIndex;
    unsigned int skipEnd;
    int steps;
    unsigned int base;
    int locatorCount;
    unsigned int joint2End;
    unsigned int joint3End;
    unsigned int jointCount;

    hrc = pMRPack->hrcData;
    locatorCount = 2;
    if (hrc->flags & 0x8) {
        locatorCount = 1;
    }
    if (index < 0) {
        return 0;
    }
    if (index < locatorCount) {
        return hrc->locators[index].scale;
    }
    if ((unsigned int) index < hrc->jointCount) {
        return hrc->joints[index - 2].scale;
    }
    jointCount = hrc->jointCount;
    locatorCount = jointCount;
    if ((unsigned int) index < locatorCount + hrc->joint2Count) {
        TiHrcJoint *joint2 = (TiHrcJoint *) ((unsigned char *) hrc + hrc->joint2Offset);
        return joint2[index - locatorCount].scale;
    } else {
        locatorCount += hrc->joint2Count;
        joint3End = locatorCount + hrc->joint3Count;
        if (index < joint3End) {
            TiHrcJoint *joint3 = (TiHrcJoint *) ((unsigned char *) hrc + hrc->joint3Offset);
            return joint3[index - locatorCount].scale;
        } else {
            locatorCount = joint3End;
            skipEnd = locatorCount + hrc->skipCount;
            if (index < skipEnd) {
                skips = (TiHrcSkip *) ((unsigned char *) hrc + hrc->skipOffset);
                return skips[index - joint3End].scale;
            }
        }
    }
    locatorCount = skipEnd;
    if ((unsigned int) index < locatorCount + hrc->dynamicsCount) {
        skipEnd = locatorCount + hrc->dynamicsCount;
        dynamicsIndex = index - locatorCount;
        dynamics = (TiHrcDynamics *) ((unsigned char *) hrc + hrc->dynamicsOffset);
        if (dynamicsIndex > 0) {
            steps = dynamicsIndex;
            do {
                vectorBytes = (dynamics->sphereCount + dynamics->matrixCount
                               + dynamics->capsuleCount) * 16;
                vectors = (unsigned char *) dynamics->entries;
                dynamics = (TiHrcDynamics *) (vectors + vectorBytes);
            } while (--steps != 0);
        }
        return dynamics->scale;
    }
    if (D_00542A10 != 0) {
        assertFailed(D_00542A30, D_00542A50, 204);
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_00299E98);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029AD70);

void tiMotionChainDynamics(TiMRPackage *pMRPack, TiHrcDynamics *dynamics)
{
    TiHrcData *hrc;
    unsigned int nodeIndex;
    unsigned int dynamicsIndex;
    int parent;
    int leading;

    hrc = pMRPack->hrcData;
    leading = hrc->jointCount + hrc->skipCount;
    nodeIndex = leading + (hrc->joint3Count + hrc->joint2Count);
    dynamicsIndex = 0;
    if (hrc->dynamicsCount != 0) {
        do {
            parent = dynamics->parent;
            dynamics = func_002980A0(&pMRPack->localMat[nodeIndex], pMRPack, dynamics,
                                     &pMRPack->dynMat[dynamicsIndex]);
            if (parent >= 0) {
                xglCurMatrixSet(pMRPack->globalMat[parent]);
            } else {
                tiVu0LoadIdentityMatrix();
            }
            xglCurMatrixMul(pMRPack->localMat[nodeIndex]);
            xglCurMatrixGet(pMRPack->globalMat[nodeIndex]);
            nodeIndex++;
            dynamicsIndex++;
        } while (dynamicsIndex < pMRPack->hrcData->dynamicsCount);
    }
    tiDynamicsSolveCellPairs(pMRPack);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B2C0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B488);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B5E0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B650);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B7D8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B830);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B8C0);

void tiMotionUpdateVisibility(TiMRPackage *pMRPack, unsigned char *pVisible)
{
    TiOffsetEntry *anim;
    XacCurveKeyTable *table;
    int hidden;
    int curveIndex;

    if (pMRPack == 0) {
        assertFailed(D_00542AB0, D_00542A50, 993);
    }
    if (pVisible == 0) {
        assertFailed(D_00542AB8, D_00542A50, 994);
    }
    anim = pMRPack->entryA;
    if (anim == 0 || anim->version < 1.4f) {
        return;
    }
    curveIndex = 0;
    table = (XacCurveKeyTable *) ((unsigned char *) anim + anim->tables.second.visibilityOffset);
    if (anim->tables.second.visibilityCount != 0) {
        do {
            /* The next table follows the keys of this one. */
            table = (XacCurveKeyTable *) xacFindCurveKey(&hidden, table, pMRPack->state_a.position);
            if (hidden != 0) {
                /* Word access through the byte pointer: an int array index changes the code. */
                *(int *) (pVisible + ((curveIndex >> 5) << 2)) &= ~(1 << (curveIndex & 0x1F));
            } else {
                *(int *) (pVisible + ((curveIndex >> 5) << 2)) |= 1 << (curveIndex & 0x1F);
            }
            curveIndex++;
        /* Reloads the count through pMRPack (anim->tables.second.visibilityCount changes the code). */
        } while (curveIndex < pMRPack->entryA->tables.second.visibilityCount);
    }
}

/* Thin wrapper: the joint header sits at block + block->joint_header_offset. */
void tiMotionSampleJoint(float *values, TiMotionOffsetBlock *block, float value)
{
    xacEvalJointCurves9(values, (unsigned char *) block + block->joint_header_offset, value);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029BEE0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029BF48);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029C0C0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029C268);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029C430);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029C628);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029C820);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029C9A8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029D7B8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029D9D0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029DC40);

/* Transform vectors filled by xacEvalTransDynamics: offset, rotation and scale (the local vectors, sp+0 to sp+32). */
typedef struct TiMotionVectors {
    float offset[3];
    float rotation[3];
    float scale[3];
} TiMotionVectors;

/* One transform slot: its rotation at +0x40 and the result scale at +0x70. */
typedef struct TiMotionSlot {
    unsigned char unmodeled_00[0x40];
    float rotation[3];
    unsigned char unmodeled_4C[0x24];
    float result[3];
} TiMotionSlot;

/* The source record: its bit words at +0x10 and a weight at +0x18. */
typedef struct TiMotionSource {
    unsigned char unmodeled_00[0x10];
    unsigned int bits[2];
    float weight;
} TiMotionSource;

void xacEvalTransDynamics(TiMotionVectors *out, TiMotionSlot *slot, int value, float weight);
void tiWordSetBits(unsigned int *bits, int count);

void func_0029DF78(Matrix4 matrix, TiMotionSource *source, TiMotionSlot *slot, int value)
{
    TiMotionVectors local;

    xacEvalTransDynamics(&local, slot, value, source->weight);
    tiVu0LoadIdentityMatrix();
    if (local.offset[0] != 0.0f || local.offset[1] != 0.0f || local.offset[2] != 0.0f) {
        xglCurMulTranslate(local.offset);
    }
    if (slot->rotation[2] != 0.0f) {
        xglCurMulRotateZ(slot->rotation[2]);
    }
    if (slot->rotation[1] != 0.0f) {
        xglCurMulRotateY(slot->rotation[1]);
    }
    if (slot->rotation[0] != 0.0f) {
        xglCurMulRotateX(slot->rotation[0]);
    }
    if (local.rotation[2] != 0.0f) {
        xglCurMulRotateZ(local.rotation[2]);
    }
    if (local.rotation[1] != 0.0f) {
        xglCurMulRotateY(local.rotation[1]);
    }
    if (local.rotation[0] != 0.0f) {
        xglCurMulRotateX(local.rotation[0]);
    }
    if (local.scale[0] != 1.0f || local.scale[1] != 1.0f || local.scale[2] != 1.0f) {
        tiWordSetBits(source->bits, 32);
        xglCurMulScaleXYZ(local.scale);
    }
    slot->result[0] = local.scale[0];
    slot->result[1] = local.scale[1];
    slot->result[2] = local.scale[2];
    xglCurMatrixGet(matrix);
}

/* Identity, then offset, rotation Z/Y/X and scale, each applied only when it is not zero (or one for scale);
 * stores the result in `matrix`. */
void tiMotionTransformToMatrix(Matrix4 matrix, TiMotionTransform *transform)
{
    tiVu0LoadIdentityMatrix();
    if (transform->offset[0] != 0.0f || transform->offset[1] != 0.0f || transform->offset[2] != 0.0f) {
        xglCurMulTranslate(transform->offset);
    }
    if (transform->rotation[2] != 0.0f) {
        xglCurMulRotateZ(transform->rotation[2]);
    }
    if (transform->rotation[1] != 0.0f) {
        xglCurMulRotateY(transform->rotation[1]);
    }
    if (transform->rotation[0] != 0.0f) {
        xglCurMulRotateX(transform->rotation[0]);
    }
    if (transform->scale[0] != 1.0f || transform->scale[1] != 1.0f || transform->scale[2] != 1.0f) {
        xglCurMulScaleXYZ(transform->scale);
    }
    xglCurMatrixGet(matrix);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E278);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E2C8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E570);
