#include "common.h"
#include "shared.h"

void xacEvalJointTransformCurves(f32 *jointValues, u8 *pJointHeader, f32 curveParam);

/* A byte offset read from a header (data-driven, not a field): the record starts at base + off bytes. */
#define BYTE_OFFSET(base, off) ((unsigned char *) (base) + (off))
/* The same record addressed as `type *`, so callers need no cast. */
#define BYTE_OFFSET_AS(type, base, off) ((type *) BYTE_OFFSET(base, off))
/* The 32-bit word of a bit set that holds bit `index` (the set is addressed by bytes). */
#define BIT_WORD(bits, nodeIndex) (*BYTE_OFFSET_AS(int, bits, ((nodeIndex) >> 5) << 2))

typedef struct TiMotionOffsetBlock {
    unsigned char unmodeled_00[0x30];
    int joint_header_offset;
} TiMotionOffsetBlock;

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_002998C0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_00299C70);

/* One transform: parent index, offset, rotation and scale. */
typedef struct TiMotionTransform {
    int parent;
    float translation[3];
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
XacCurveKeyTable *xacFindCurveKey(int *pFlag, XacCurveKeyTable *pTable, f32 curveParam);
extern const unsigned char gTiMotionSolverFileName[];
extern const unsigned char gTiMotionNullPackMessage[];
extern const unsigned char gTiMotionNullVisibleMessage[];
extern const unsigned char gParentScaleAssertEnabled[];
extern const unsigned char gParentScaleNotFoundMessage[];

/* Scale lookup for node `index` in the pMRPack table at +0xD0; asserts "can not found parent scale". */
float *tiMotionFindParentScale(TiMRPackage *pMRPack, int scaleNodeIndex)
{
    TiHrcData *hrc;
    TiHrcSkip *skipRecords;
    TiHrcDynamics *dynamics;
    TiCollEntry *collEntries;
    int collisionEntryCount;
    int dynamicsIndex;
    unsigned int skipEnd;
    int dynamicsSteps;
    int locatorCount;
    unsigned int secondJointEnd;
    unsigned int thirdJointEnd;
    unsigned int jointCount;

    hrc = pMRPack->hrcData;
    locatorCount = 2;
    if (hrc->flags & 0x8) {
        locatorCount = 1;
    }
    if (scaleNodeIndex < 0) {
        return 0;
    }
    if (scaleNodeIndex < locatorCount) {
        return hrc->locators[scaleNodeIndex].scale;
    }
    if ((unsigned int) scaleNodeIndex < hrc->jointCount) {
        return hrc->joints[scaleNodeIndex - 2].scale;
    }
    jointCount = hrc->jointCount;
    locatorCount = jointCount;
    if ((unsigned int) scaleNodeIndex < locatorCount + hrc->joint2Count) {
        TiHrcJoint *secondJointTable = BYTE_OFFSET_AS(TiHrcJoint, hrc, hrc->joint2Offset);
        return secondJointTable[scaleNodeIndex - locatorCount].scale;
    } else {
        locatorCount += hrc->joint2Count;
        thirdJointEnd = locatorCount + hrc->joint3Count;
        if (scaleNodeIndex < thirdJointEnd) {
            TiHrcJoint *thirdJointTable = BYTE_OFFSET_AS(TiHrcJoint, hrc, hrc->joint3Offset);
            return thirdJointTable[scaleNodeIndex - locatorCount].scale;
        } else {
            locatorCount = thirdJointEnd;
            skipEnd = locatorCount + hrc->skipCount;
            if (scaleNodeIndex < skipEnd) {
                skipRecords = BYTE_OFFSET_AS(TiHrcSkip, hrc, hrc->skipOffset);
                return skipRecords[scaleNodeIndex - thirdJointEnd].scale;
            }
        }
    }
    locatorCount = skipEnd;
    if ((unsigned int) scaleNodeIndex < locatorCount + hrc->dynamicsCount) {
        skipEnd = locatorCount + hrc->dynamicsCount;
        dynamicsIndex = scaleNodeIndex - locatorCount;
        dynamics = BYTE_OFFSET_AS(TiHrcDynamics, hrc, hrc->dynamicsOffset);
        if (dynamicsIndex > 0) {
            dynamicsSteps = dynamicsIndex;
            do {
                collisionEntryCount = dynamics->sphereCount + dynamics->matrixCount + dynamics->capsuleCount;
                collEntries = dynamics->entries;
                dynamics = (TiHrcDynamics *) (collEntries + collisionEntryCount);
            } while (--dynamicsSteps != 0);
        }
        return dynamics->scale;
    }
    if (gParentScaleAssertEnabled != 0) {
        assertFailed(gParentScaleNotFoundMessage, gTiMotionSolverFileName, 204);
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_00299E98);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029AD70);

void tiMotionChainDynamics(TiMRPackage *pMRPack, TiHrcDynamics *dynamics)
{
    TiHrcData *hrc;
    unsigned int hierarchyNodeIndex;
    unsigned int dynamicsIndex;
    int parent;
    int leadingNodeCount;

    hrc = pMRPack->hrcData;
    leadingNodeCount = hrc->jointCount + hrc->skipCount;
    hierarchyNodeIndex = leadingNodeCount + (hrc->joint3Count + hrc->joint2Count);
    dynamicsIndex = 0;
    if (hrc->dynamicsCount != 0) {
        do {
            parent = dynamics->parent;
            dynamics = func_002980A0(&pMRPack->localMat[hierarchyNodeIndex], pMRPack, dynamics,
                                     &pMRPack->dynMat[dynamicsIndex]);
            if (parent >= 0) {
                xglCurMatrixSet(pMRPack->globalMat[parent]);
            } else {
                tiVu0LoadIdentityMatrix();
            }
            xglCurMatrixMul(pMRPack->localMat[hierarchyNodeIndex]);
            xglCurMatrixGet(pMRPack->globalMat[hierarchyNodeIndex]);
            hierarchyNodeIndex++;
            dynamicsIndex++;
        } while (dynamicsIndex < pMRPack->hrcData->dynamicsCount);
    }
    tiDynamicsSolveCellPairs(pMRPack);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B2C0);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B488);

typedef struct TiMotionVectors {
    float translation[3];
    float rotation[3];
    float scale[3];
} TiMotionVectors;

typedef struct TiMotionSlot {
    unsigned char unmodeled_00[0x40];
    float rotation[3];
    unsigned char unmodeled_4C[0x24];
    float resultScale[3];
} TiMotionSlot;

typedef struct TiMotionSource {
    unsigned char unmodeled_00[0x10];
    unsigned int bits[2];
    float weight;
} TiMotionSource;

typedef int s32;

TiHrcDynamics *tiDynamicsNodeSolveNext(TiMRPackage *object, TiHrcDynamics *solveNode, TiSolverState *state);

/* The solver bodies are 0x40 bytes each, so body i is pMRPack->dynMat + i. */
void tiMotionSolveDynamicNodes(TiMRPackage *pMRPack, TiHrcDynamics *firstDynamicsNode) {
    s32 dynamicsCount;
    TiHrcDynamics *solveNode;
    u32 dynamicsIndex;

    solveNode = firstDynamicsNode;
    dynamicsIndex = 0;
    if (pMRPack->hrcData->dynamicsCount != 0) {
        do {
            solveNode = tiDynamicsNodeSolveNext(pMRPack, solveNode, pMRPack->dynMat + dynamicsIndex);
            dynamicsIndex += 1;
            dynamicsCount = pMRPack->hrcData->dynamicsCount;
        } while (dynamicsIndex < dynamicsCount);
    }
    tiDynamicsSolveCellPairs(pMRPack);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B650);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B7D8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B830);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029B8C0);

void tiMotionUpdateVisibility(TiMRPackage *pMRPack, unsigned char *visibilityBits)
{
    TiOffsetEntry *bodyAnimEntry;
    XacCurveKeyTable *visibilityTable;
    int isHidden;
    int curveIndex;

    if (pMRPack == 0) {
        assertFailed(gTiMotionNullPackMessage, gTiMotionSolverFileName, 993);
    }
    if (visibilityBits == 0) {
        assertFailed(gTiMotionNullVisibleMessage, gTiMotionSolverFileName, 994);
    }
    bodyAnimEntry = pMRPack->bodyAnimEntry;
    if (bodyAnimEntry == 0 || bodyAnimEntry->version < 1.4f) {
        return;
    }
    curveIndex = 0;
    visibilityTable = BYTE_OFFSET_AS(XacCurveKeyTable, bodyAnimEntry, bodyAnimEntry->tables.second.visibilityOffset);
    if (bodyAnimEntry->tables.second.visibilityCount != 0) {
        do {
            /* The next table follows the keys of this one. */
            visibilityTable = xacFindCurveKey(&isHidden, visibilityTable, pMRPack->bodyAnimBlend.position);
            if (isHidden != 0) {
                BIT_WORD(visibilityBits, curveIndex) &= ~(1 << (curveIndex & 0x1F));
            } else {
                BIT_WORD(visibilityBits, curveIndex) |= 1 << (curveIndex & 0x1F);
            }
            curveIndex++;
        /* Reloads the count through pMRPack (anim->tables.second.visibilityCount changes the code). */
        } while (curveIndex < pMRPack->bodyAnimEntry->tables.second.visibilityCount);
    }
}

/* Thin wrapper: the joint header sits at block + block->joint_header_offset. */
void tiMotionSampleJoint(float *jointValues, TiMotionOffsetBlock *jointBlock, float curveParam)
{
    xacEvalJointTransformCurves(jointValues, BYTE_OFFSET(jointBlock, jointBlock->joint_header_offset), curveParam);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029BEE0);

void tiWordSetBits(unsigned int *bits, int count);
void func_0029BEE0(float *values, unsigned char **ppCursor, int unmodeled_e8, int valueCount);

/* Evaluates one locator into `matrix`: its values, then translate, rotate Z/Y/X and scale. */
void tiMotionSolveLocatorOutOfLine(Matrix4 matrix, TiMRPackage *pMRPack, TiHrcLocator *locator, unsigned char **ppCursor)
{
    float locatorValues[9];

    func_0029BEE0(locatorValues, ppCursor, pMRPack->unmodeled_e8, 9);
    tiVu0LoadIdentityMatrix();
    if (locatorValues[0] != 0.0f || locatorValues[1] != 0.0f || locatorValues[2] != 0.0f) {
        xglCurMulTranslate(locatorValues);
    }
    if (locatorValues[5] != 0.0f) {
        xglCurMulRotateZ(locatorValues[5]);
    }
    if (locatorValues[4] != 0.0f) {
        xglCurMulRotateY(locatorValues[4]);
    }
    if (locatorValues[3] != 0.0f) {
        xglCurMulRotateX(locatorValues[3]);
    }
    if (locatorValues[6] != 1.0f || locatorValues[7] != 1.0f || locatorValues[8] != 1.0f) {
        tiWordSetBits(&pMRPack->bodyAnimBlend.motionFlags, 32);
        xglCurMulScaleXYZ(&locatorValues[6]);
    }
    locator->scale[0] = locatorValues[6];
    locator->scale[1] = locatorValues[7];
    locator->scale[2] = locatorValues[8];
    xglCurMatrixGet(matrix);
}

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
/* One transform slot: its rotation at +0x40 and the result scale at +0x70. */
/* The source record: its bit words at +0x10 and a weight at +0x18. */
void xacEvalTransDynamics(TiMotionVectors *out, TiMotionSlot *motionSlot, int curveParam, float weight);

void tiMotionSlotToMatrix(Matrix4 matrix, TiMotionSource *motionSource, TiMotionSlot *motionSlot, int curveParam)
{
    TiMotionVectors localVectors;

    xacEvalTransDynamics(&localVectors, motionSlot, curveParam, motionSource->weight);
    tiVu0LoadIdentityMatrix();
    if (localVectors.translation[0] != 0.0f || localVectors.translation[1] != 0.0f || localVectors.translation[2] != 0.0f) {
        xglCurMulTranslate(localVectors.translation);
    }
    if (motionSlot->rotation[2] != 0.0f) {
        xglCurMulRotateZ(motionSlot->rotation[2]);
    }
    if (motionSlot->rotation[1] != 0.0f) {
        xglCurMulRotateY(motionSlot->rotation[1]);
    }
    if (motionSlot->rotation[0] != 0.0f) {
        xglCurMulRotateX(motionSlot->rotation[0]);
    }
    if (localVectors.rotation[2] != 0.0f) {
        xglCurMulRotateZ(localVectors.rotation[2]);
    }
    if (localVectors.rotation[1] != 0.0f) {
        xglCurMulRotateY(localVectors.rotation[1]);
    }
    if (localVectors.rotation[0] != 0.0f) {
        xglCurMulRotateX(localVectors.rotation[0]);
    }
    if (localVectors.scale[0] != 1.0f || localVectors.scale[1] != 1.0f || localVectors.scale[2] != 1.0f) {
        tiWordSetBits(motionSource->bits, 32);
        xglCurMulScaleXYZ(localVectors.scale);
    }
    motionSlot->resultScale[0] = localVectors.scale[0];
    motionSlot->resultScale[1] = localVectors.scale[1];
    motionSlot->resultScale[2] = localVectors.scale[2];
    xglCurMatrixGet(matrix);
}

/* Identity, then offset, rotation Z/Y/X and scale, each applied only when it is not zero (or one for scale);
 * stores the result in `matrix`. */
void tiMotionTransformToMatrix(Matrix4 matrix, TiMotionTransform *transform)
{
    tiVu0LoadIdentityMatrix();
    if (transform->translation[0] != 0.0f || transform->translation[1] != 0.0f || transform->translation[2] != 0.0f) {
        xglCurMulTranslate(transform->translation);
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

unsigned char *xacReadJointValueAndStart(f32 *, void *, s32, f32);

/* The curve block that tiMotionSampleCurve samples: its end time at +0x3C. */
typedef struct TiMotionCurveBlock {
    unsigned char unmodeled_00[0x3C];
    f32 endTime;
} TiMotionCurveBlock;

/* The sampled result written by tiMotionSampleCurve: the value at +0x4 and the duration (end minus start) at +0xC. */
typedef struct TiMotionCurveDest {
    f32 unk_00;
    f32 sampledValue;
    f32 unk_08;
    f32 duration;
} TiMotionCurveDest;

/* The start and value pair that xacReadJointValueAndStart fills. */
typedef struct TiMotionCurveSample {
    f32 sampledValue;
    f32 startTime;
} TiMotionCurveSample;

void tiMotionSampleCurve(TiMotionSource *motionSource, TiMotionCurveBlock *curveBlock, s32 jointIndex, TiMotionCurveDest *sampledCurve) {
    TiMotionCurveSample sample;
    f32 curveParam;
    f32 duration;

    xacReadJointValueAndStart(&sample.sampledValue, curveBlock, jointIndex, motionSource->weight);
    curveParam = sample.sampledValue;
    sampledCurve->sampledValue = curveParam;
    duration = curveBlock->endTime - sample.startTime;
    sampledCurve->duration = duration;
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E2C8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E570);
