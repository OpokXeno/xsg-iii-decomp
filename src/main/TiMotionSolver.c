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
    unsigned char jointBlockHead[0x30];
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
extern const unsigned char gTiMotionNullLocalMatMessage[]; /* "pMRPack->pLocalMat" */
extern const unsigned char gTiMotionNullHrcDataMessage[]; /* "pMRPack->pHrcData" */

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

void tiWordSetBits(unsigned int *bits, int count);
void xglCurMulInvScaleXYZ(float *scale);
void xacEvalTransLocator(float *animatedValues, TiHrcLocator *locator, unsigned char **ppCursor, float position);
void xacEvalTransJoint(float *animatedValues, TiHrcJoint *joint, unsigned char **ppCursor, float position);
void xacEvalTransDynamics(float *animatedValues, TiHrcDynamics *dynamics, unsigned char **ppCursor, float position);
unsigned char *xacReadJointValueAndStart(float *sample, void *curveBlock, unsigned char **ppCursor, float position);

/* The curve block that tiMotionSampleCurve samples: its end time at +0x3C. */
typedef struct TiMotionCurveBlock {
    unsigned char curveBlockHead[0x3C];
    f32 endTime;
} TiMotionCurveBlock;

/* The sampled result written by tiMotionSampleCurve: the value at +0x4 and the duration (end minus start) at +0xC. */
typedef struct TiMotionCurveDest {
    f32 untouchedFirstWord;
    f32 sampledValue;
    f32 untouchedThirdWord;
    f32 duration;
} TiMotionCurveDest;

/* The start and value pair that xacReadJointValueAndStart fills. */
typedef struct TiMotionCurveSample {
    f32 sampledValue;
    f32 startTime;
} TiMotionCurveSample;

/* The curve tail of second-format animation data (inside the `tables` union of TiOffsetEntry): the curve count at
 * +0x28 and the curve offset at +0x48. */
typedef struct TiXacCurveRef {
    unsigned char entryHeader[0x10];
    union {
        struct {
            unsigned char jointTables[0x18];
            unsigned int curveCount;
            unsigned char dynamicsTables[0x1C];
            int curveOffset;
        } secondFormat;
    } tables;
} TiXacCurveRef;

/* The curve block of the hierarchy: present when the count at +0x60 is not zero, at the offset at +0x64. */
typedef struct TiHrcCurveRef {
    unsigned char hrcCountsAndTables[0x60];
    int curveCount;
    int curveOffset;
} TiHrcCurveRef;

/* The two sampled curves of the package at +0x110. */
typedef struct TiMRPackageCurves {
    unsigned char packageHead[0x110];
    TiMotionCurveDest curves[2];
} TiMRPackageCurves;

/* One static node of the hierarchy (0x40 bytes): its fixed transform. */
typedef struct TiHrcStatic {
    TiMotionTransform transform;
    unsigned char staticTail[0x18];
} TiHrcStatic;

/* A scale outside [0.999, 1.001) on any axis. */
#define TI_SCALE_NOT_UNIT(scale)                                                          \
    ((scale)[0] < 0.999f || (scale)[0] >= 1.001f || (scale)[1] < 0.999f || (scale)[1] >= 1.001f \
     || (scale)[2] < 0.999f || (scale)[2] >= 1.001f)

/* Translation animatedValues[0..2], applied when any is not zero. */
#define TI_MUL_TRANSLATE(animatedValues)                                                          \
    if ((animatedValues)[0] != 0.0f || (animatedValues)[1] != 0.0f || (animatedValues)[2] != 0.0f) {              \
        xglCurMulTranslate(animatedValues);                                                       \
    }
/* Undo the parent's scale when the node asks for it (flag 2). */
#define TI_MUL_PARENT_INV_SCALE(pMRPack, node)                                            \
    if ((node)->flags & 2) {                                                              \
        float *parentScale = tiMotionFindParentScale(pMRPack, (node)->parent);           \
        if (parentScale != 0                                                              \
            && (parentScale[0] != 1.0f || parentScale[1] != 1.0f || parentScale[2] != 1.0f)) { \
            xglCurMulInvScaleXYZ(parentScale);                                            \
        }                                                                                 \
    }
/* Rotation Z, Y then X, each applied when it is not zero. */
#define TI_MUL_ROTATE(rotation)                                                           \
    if ((rotation)[2] != 0.0f) {                                                          \
        xglCurMulRotateZ((rotation)[2]);                                                  \
    }                                                                                     \
    if ((rotation)[1] != 0.0f) {                                                          \
        xglCurMulRotateY((rotation)[1]);                                                  \
    }                                                                                     \
    if ((rotation)[0] != 0.0f) {                                                          \
        xglCurMulRotateX((rotation)[0]);                                                  \
    }

/* Evaluates the locator into `matrix`. */
static inline void tiMotionSolveLocator(TiMRPackage *pMRPack, Matrix4 matrix, TiHrcLocator *locator,
                                        unsigned char **ppCursor)
{
    float animatedValues[9];

    xacEvalTransLocator(animatedValues, locator, ppCursor, pMRPack->bodyAnimBlend.position);
    tiVu0LoadIdentityMatrix();
    TI_MUL_TRANSLATE(animatedValues);
    TI_MUL_PARENT_INV_SCALE(pMRPack, locator);
    TI_MUL_ROTATE(&animatedValues[3]);
    if (TI_SCALE_NOT_UNIT(&animatedValues[6])) {
        tiWordSetBits(&pMRPack->bodyAnimBlend.motionFlags, 32);
        xglCurMulScaleXYZ(&animatedValues[6]);
    }
    locator->scale[0] = animatedValues[6];
    locator->scale[1] = animatedValues[7];
    locator->scale[2] = animatedValues[8];
    xglCurMatrixGet(matrix);
}

/* Evaluates one joint into `matrix`: the joint's orientation, then the animated rotation. */
static inline void tiMotionSolveJoint(TiMRPackage *pMRPack, Matrix4 matrix, TiHrcJoint *joint,
                                      unsigned char **ppCursor)
{
    float animatedValues[9];

    xacEvalTransJoint(animatedValues, joint, ppCursor, pMRPack->bodyAnimBlend.position);
    tiVu0LoadIdentityMatrix();
    TI_MUL_TRANSLATE(animatedValues);
    TI_MUL_PARENT_INV_SCALE(pMRPack, joint);
    TI_MUL_ROTATE(joint->orient);
    TI_MUL_ROTATE(&animatedValues[3]);
    if (TI_SCALE_NOT_UNIT(&animatedValues[6])) {
        tiWordSetBits(&pMRPack->bodyAnimBlend.motionFlags, 32);
        xglCurMulScaleXYZ(&animatedValues[6]);
    }
    joint->scale[0] = animatedValues[6];
    joint->scale[1] = animatedValues[7];
    joint->scale[2] = animatedValues[8];
    xglCurMatrixGet(matrix);
}

/* Evaluates one dynamics node into `matrix`. */
static inline void tiMotionSolveDynamics(TiMRPackage *pMRPack, Matrix4 matrix, TiHrcDynamics *dynamics,
                                         unsigned char **ppCursor)
{
    float animatedValues[9];

    xacEvalTransDynamics(animatedValues, dynamics, ppCursor, pMRPack->bodyAnimBlend.position);
    tiVu0LoadIdentityMatrix();
    TI_MUL_TRANSLATE(animatedValues);
    TI_MUL_ROTATE(dynamics->orient);
    TI_MUL_ROTATE(&animatedValues[3]);
    if (animatedValues[6] != 1.0f || animatedValues[7] != 1.0f || animatedValues[8] != 1.0f) {
        tiWordSetBits(&pMRPack->bodyAnimBlend.motionFlags, 32);
        xglCurMulScaleXYZ(&animatedValues[6]);
    }
    dynamics->scale[0] = animatedValues[6];
    dynamics->scale[1] = animatedValues[7];
    dynamics->scale[2] = animatedValues[8];
    xglCurMatrixGet(matrix);
}

/* Identity, then offset, rotation Z/Y/X and scale, each applied only when it is not zero (or one for scale);
 * stores the result in `matrix`. */
static inline void tiMotionApplyTransform(Matrix4 matrix, TiMotionTransform *transform)
{
    tiVu0LoadIdentityMatrix();
    TI_MUL_TRANSLATE(transform->translation);
    TI_MUL_ROTATE(transform->rotation);
    if (transform->scale[0] != 1.0f || transform->scale[1] != 1.0f || transform->scale[2] != 1.0f) {
        xglCurMulScaleXYZ(transform->scale);
    }
    xglCurMatrixGet(matrix);
}

/* Samples one curve of the block at the package position: the value and the time left to the block's end. */
static inline void tiMotionSampleCurveAt(TiMRPackage *pMRPack, TiMotionCurveBlock *curveBlock,
                                         unsigned char **ppCursor, TiMotionCurveDest *sampledCurve)
{
    TiMotionCurveSample sample;
    f32 curveParam;
    f32 duration;

    xacReadJointValueAndStart(&sample.sampledValue, curveBlock, ppCursor, pMRPack->bodyAnimBlend.position);
    curveParam = sample.sampledValue;
    sampledCurve->sampledValue = curveParam;
    duration = curveBlock->endTime - sample.startTime;
    sampledCurve->duration = duration;
}

/* Evaluates the animation of every node of the hierarchy (one locator here) and stores its local matrix in
 * pMRPack->localMat, in node order: the locator, the joints, the second and third joint tables, the dynamics
 * nodes after the skip nodes, and the static nodes; then samples the two package curves. */
void tiMotionSolveLocalMatrices(TiMRPackage *pMRPack)
{
    unsigned char *cursor = 0;
    TiHrcData *hrc;
    TiHrcLocator *locators;
    TiHrcJoint *joints;
    TiHrcJoint *secondJoints;
    TiHrcJoint *thirdJoints;
    TiHrcDynamics *dynamics;
    TiHrcStatic *statics;
    TiMotionCurveBlock *curveBlock;
    TiOffsetEntry *anim;
    unsigned int transformIndex;
    unsigned int i;
    int collisionBytes;
    unsigned char *collisionEntries;
    TiMRPackageCurves *curveSlots;

    hrc = pMRPack->hrcData;
    locators = hrc->locators;
    joints = (TiHrcJoint *) &hrc->locators[1];
    secondJoints = BYTE_OFFSET_AS(TiHrcJoint, hrc, hrc->joint2Offset);
    thirdJoints = BYTE_OFFSET_AS(TiHrcJoint, hrc, hrc->joint3Offset);
    dynamics = BYTE_OFFSET_AS(TiHrcDynamics, hrc, hrc->dynamicsOffset);
    statics = BYTE_OFFSET_AS(TiHrcStatic, hrc, hrc->staticOffset);
    curveBlock = BYTE_OFFSET_AS(TiMotionCurveBlock, hrc, ((TiHrcCurveRef *) hrc)->curveOffset);
    anim = pMRPack->bodyAnimEntry;
    if (pMRPack->localMat == 0) {
        assertFailed(gTiMotionNullLocalMatMessage, gTiMotionSolverFileName, 0x1F2);
    }
    if (pMRPack->hrcData == 0) {
        assertFailed(gTiMotionNullHrcDataMessage, gTiMotionSolverFileName, 0x1F3);
    }

    transformIndex = 0;
    if (anim != 0) {
        cursor = BYTE_OFFSET(anim, anim->tables.second.jointOffset);
    } else {
        cursor = 0;
    }
    for (i = 0; i < 1; i++, transformIndex++) {
        tiMotionSolveLocator(pMRPack, pMRPack->localMat[transformIndex], &locators[i], &cursor);
    }
    pMRPack->localMat[1][3][0] *= pMRPack->scale[0];
    pMRPack->localMat[1][3][1] *= pMRPack->scale[1];
    pMRPack->localMat[1][3][2] *= pMRPack->scale[2];

    for (i = 0; i < pMRPack->hrcData->jointCount - 1; i++, transformIndex++) {
        if (cursor != 0 && i >= anim->tables.second.jointCount - 1) {
            cursor = 0;
        }
        tiMotionSolveJoint(pMRPack, pMRPack->localMat[transformIndex], &joints[i], &cursor);
    }

    if (anim != 0) {
        cursor = BYTE_OFFSET(anim, anim->tables.second.joint2Offset);
    } else {
        cursor = 0;
    }
    for (i = 0; i < pMRPack->hrcData->joint2Count; i++, transformIndex++) {
        if (cursor != 0 && i >= anim->tables.second.joint2Count) {
            cursor = 0;
        }
        tiMotionSolveJoint(pMRPack, pMRPack->localMat[transformIndex], &secondJoints[i], &cursor);
    }

    if (anim != 0) {
        cursor = BYTE_OFFSET(anim, anim->tables.second.joint3Offset);
    } else {
        cursor = 0;
    }
    for (i = 0; i < pMRPack->hrcData->joint3Count; i++, transformIndex++) {
        if (cursor != 0 && i >= anim->tables.second.joint3Count) {
            cursor = 0;
        }
        tiMotionSolveJoint(pMRPack, pMRPack->localMat[transformIndex], &thirdJoints[i], &cursor);
    }

    transformIndex += pMRPack->hrcData->skipCount;
    if (anim != 0) {
        cursor = BYTE_OFFSET(anim, anim->tables.second.dynamicsOffset);
    } else {
        cursor = 0;
    }
    for (i = 0; i < pMRPack->hrcData->dynamicsCount; i++) {
        collisionBytes = (dynamics->sphereCount + dynamics->matrixCount + dynamics->capsuleCount) * 16;
        if (cursor != 0 && i >= anim->tables.second.dynamicsCount) {
            cursor = 0;
        }
        tiMotionSolveDynamics(pMRPack, pMRPack->localMat[transformIndex], dynamics, &cursor);
        transformIndex++;
        collisionEntries = (unsigned char *) dynamics->entries;
        dynamics = (TiHrcDynamics *) (collisionEntries + collisionBytes);
    }

    if (pMRPack->hrcData->staticCount != 0) {
        for (i = 0; i < pMRPack->hrcData->staticCount; i++, transformIndex++) {
            tiMotionApplyTransform(pMRPack->localMat[transformIndex], &statics[i].transform);
        }
    }

    if (anim != 0) {
        if (((TiXacCurveRef *) anim)->tables.secondFormat.curveCount != 0 && ((TiHrcCurveRef *) pMRPack->hrcData)->curveCount != 0
            && ((TiXacCurveRef *) anim)->tables.secondFormat.curveCount == 2) {
            cursor = BYTE_OFFSET(anim, ((TiXacCurveRef *) anim)->tables.secondFormat.curveOffset);
            i = 0;
            /* The curve slot is reached by stepping the package base one curve at a time. */
            curveSlots = (TiMRPackageCurves *) pMRPack;
            do {
                i++;
                tiMotionSampleCurveAt(pMRPack, curveBlock, &cursor, curveSlots->curves);
                curveSlots = (TiMRPackageCurves *) ((TiMotionCurveDest *) curveSlots + 1);
            } while (i < ((TiXacCurveRef *) anim)->tables.secondFormat.curveCount);
        }
    } else {
        cursor = 0;
    }
}

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
void func_0029BEE0(float *animatedValues, unsigned char **ppCursor, int unmodeled_e8, int valueCount);

/* Evaluates one locator into `matrix`: its animatedValues, then translate, rotate Z/Y/X and scale. */
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

/* Out-of-line copy of tiMotionSolveDynamics. */
void tiMotionSlotToMatrix(Matrix4 matrix, TiMRPackage *pMRPack, TiHrcDynamics *dynamics, unsigned char **ppCursor)
{
    tiMotionSolveDynamics(pMRPack, matrix, dynamics, ppCursor);
}

/* Identity, then offset, rotation Z/Y/X and scale, each applied only when it is not zero (or one for scale);
 * stores the result in `matrix`. */
void tiMotionTransformToMatrix(Matrix4 matrix, TiMotionTransform *transform)
{
    tiMotionApplyTransform(matrix, transform);
}

void tiMotionSampleCurve(TiMRPackage *pMRPack, TiMotionCurveBlock *curveBlock, unsigned char **ppCursor,
                         TiMotionCurveDest *sampledCurve)
{
    tiMotionSampleCurveAt(pMRPack, curveBlock, ppCursor, sampledCurve);
}

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E2C8);

INCLUDE_ASM("asm/main/nonmatchings/TiMotionSolver", func_0029E570);
