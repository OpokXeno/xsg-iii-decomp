#include "common.h"
#include "shared.h"

#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"
/* Address of a table at a byte offset read from the data at runtime (not a constant field). */
#define BYTE_OFFSET(base, offset) ((u8 *) (base) + (offset))











typedef struct TiSolverState TiDynamicsOut;














void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglMatrixInverse(Matrix4 destination, Matrix4 source);
void xglMatrixMul(Matrix4 destination, Matrix4 left, Matrix4 right);
void tiMtnBlendLerpMatrix(Matrix4 dst, Matrix4 a, Matrix4 b, float t);
void XrgQuaternionFromVectors(Vector4 *destination, const Vector4 *from, const Vector4 *to);
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion);
void tiDynamicsCollPushOutSphereList(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);
void tiDynamicsCollPushOutMatrixList(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);
void tiDynamicsCollPushOutCapsuleList(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);
void tiGlobalCollSolveBody(TiHrcDynamics *pNode, TiSolverState *context);
void tiGlobalCollClampScale(TiSolverState *solverBody);
void tiDynamicsCollSolveSpherePair(TiSolverState *first, TiSolverState *second, TiMRPackage *world);
void tiDynamicsCollSolveMatrixPair(TiSolverState *first, TiSolverState *second, TiMRPackage *world);
void tiDynamicsCollSolveCapsulePair(TiSolverState *first, TiSolverState *second, TiMRPackage *world);
void tiDynamicsNodeStep(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody);

/* TiSolverObject flag bit tested by tiDynamicsCollSolveNode: clamp the node scale. */
#define TI_OBJECT_FLAG_CLAMP_SCALE 0x80

typedef union { float matrixElement; } FloatAlias;

/* A cell header of the link table: first node index and link count. */
typedef struct TiLinkCell {
    int firstNodeId;
    int linkedNodeCount;
    unsigned char unmodeled_08[0x8];
} TiLinkCell;

/* A link of the table: rest length and the node index of the other end. */
typedef struct TiDynamicsLink {
    float restLength;
    int otherNodeId;
    unsigned char unmodeled_08[0x8];
} TiDynamicsLink;


void tiDynamicsNodeSubtractBias(TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    solverBody->accumulator.vector.y -= dynamicsNode->biasY;
}

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_002973F0);

/* Collision pass for one node: the dynamics push-outs (sphere list, matrix list, capsule list), the
 * global collision body and, when object flag 0x80 is set, the scale clamp. Called twice per step. */
void tiDynamicsCollSolveNode(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    tiDynamicsCollPushOutSphereList(mrPackage, dynamicsNode, solverBody);
    tiDynamicsCollPushOutMatrixList(mrPackage, dynamicsNode, solverBody);
    tiDynamicsCollPushOutCapsuleList(mrPackage, dynamicsNode, solverBody);
    tiGlobalCollSolveBody(dynamicsNode, solverBody);
    if (mrPackage->bodyAnimBlend.motionFlags & TI_OBJECT_FLAG_CLAMP_SCALE) {
        tiGlobalCollClampScale(solverBody);
    }
}

/* Pulls the node's point toward its matrix-transformed target, weighted by node->pullScale. */
void tiDynamicsNodePullToTarget(float (*matrix)[4], TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    Vector4 transformed;

    if (dynamicsNode->pullScale != 0.0f) {
        xglVectorMulMat(&transformed, matrix, &dynamicsNode->translationDefault);
        solverBody->accumulator.vector.x += (transformed.x - solverBody->position.vector.x) * dynamicsNode->pullScale;
        solverBody->accumulator.vector.y += (transformed.y - solverBody->position.vector.y) * dynamicsNode->pullScale;
        solverBody->accumulator.vector.z += (transformed.z - solverBody->position.vector.z) * dynamicsNode->pullScale;
    }
}

/* Moves the node point onto the matrix origin plus normalScale times the unit direction from the origin. */
void tiDynamicsNodeConstrainNormal(Matrix4 matrix, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    Vector4 delta;
    Vector4 normalDirection;
    float positionX;
    float positionY;
    float positionZ;

    delta.x = solverBody->position.vector.x - matrix[3][0];
    delta.y = solverBody->position.vector.y - matrix[3][1];
    delta.z = solverBody->position.vector.z - matrix[3][2];
    xglVectorNormal(&normalDirection, &delta);
    positionX = normalDirection.x * dynamicsNode->normalScale + matrix[3][0];
    positionY = normalDirection.y * dynamicsNode->normalScale + matrix[3][1];
    positionZ = normalDirection.z * dynamicsNode->normalScale + matrix[3][2];
    solverBody->position.vector.x = positionX;
    solverBody->position.vector.y = positionY;
    solverBody->position.vector.z = positionZ;
}

/* Normalizes the xyz part of the first three rows of a matrix in place; a zero length is replaced by 1e-7. */
void tiMatrixNormalizeRows(float (*matrix)[4]) {
    int i = 0;
    float *row_z = matrix[0] + 2;
    while (i < 3) {
        float length;

        length = __builtin_sqrtf(row_z[-2] * row_z[-2] + row_z[-1] * row_z[-1] + row_z[0] * row_z[0]);
        if (length == 0.0f) {
            length = 1.0e-7f;
        }
        length = 1.0f / length;
        row_z[-2] = row_z[-2] * length;
        ((FloatAlias *)row_z)[-1].matrixElement = ((FloatAlias *)row_z)[-1].matrixElement * length;
        ((FloatAlias *)row_z)[0].matrixElement = ((FloatAlias *)row_z)[0].matrixElement * length;
        row_z = row_z + 4;
        i++;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_00297878);

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_002979A0);

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_00297A80);

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_00297BF8);

/* Solves one node's palette matrix: the node point in palette space, the rotation that aligns the node's rest
 * direction with it, and the palette multiplied by that rotation, blended by node->weight below 1. */
void tiDynamicsNodeSolvePalette(TiMRPackage *unused_object, Matrix4 matrix, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    Matrix4 blended;
    Matrix4 rotation;
    Vector4 local_position;
    Vector4 local_direction;
    Vector4 target_direction;
    Matrix4 inverse;
    Vector4 align_quat;

    if (1.0f <= dynamicsNode->weight) {
        return;
    }
    xglMatrixInverse(inverse, matrix);
    xglVectorMulMat(&local_position, inverse, &solverBody->position.vector);
    xglVectorNormal(&local_direction, &local_position);
    xglVectorNormal(&target_direction, &dynamicsNode->translationDefault);
    XrgQuaternionFromVectors(&align_quat, &target_direction, &local_direction);
    XrgQuaternionToMatrix(rotation, &align_quat);
    if (dynamicsNode->weight != 0.0f) {
        xglMatrixMul(blended, matrix, rotation);
        tiMtnBlendLerpMatrix(matrix, blended, matrix, dynamicsNode->weight);
        tiMatrixNormalizeRows(matrix);
    } else {
        xglMatrixMul(matrix, matrix, rotation);
    }
}

void func_002973F0(TiHrcDynamics *, TiDynamicsOut *, f32);
void tiWindApplyAll(TiHrcDynamics *, TiDynamicsOut *);

void tiDynamicsNodeStep(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiDynamicsOut *solverBody) {
    tiWindApplyAll(dynamicsNode, solverBody);
    tiDynamicsNodeSubtractBias(dynamicsNode, solverBody);
    func_002973F0(dynamicsNode, solverBody, mrPackage->timeScale);
}

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_002980A0);

TiHrcDynamics *tiDynamicsNodeSolveNext(TiMRPackage *mrPackage, TiHrcDynamics *dynamicsNode, TiSolverState *solverBody) {
    unsigned int stepsRun;
    TiCollEntry *collisionEntries;

    mrPackage->timeScale = 1.0f;
    stepsRun = 0;
    do {
        stepsRun++;
        tiDynamicsNodeStep(mrPackage, dynamicsNode, solverBody);
    } while (stepsRun == 0);
    collisionEntries = dynamicsNode->entries;
    return (TiHrcDynamics *)(collisionEntries + (dynamicsNode->sphereCount + dynamicsNode->matrixCount + dynamicsNode->capsuleCount));
}

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_00298290);

/* Moves the solver body toward the rest length of its link to the matrix other (its translation row). */
void tiDynamicsSolveLink(TiSolverState *solverBody, Matrix4 linkedMatrix, const float *restLength) {
    TiVector3 dir;
    float length;
    float correction;

    dir = (TiVector3) { linkedMatrix[3][0] - solverBody->position.vector.x, linkedMatrix[3][1] - solverBody->position.vector.y, linkedMatrix[3][2] - solverBody->position.vector.z };
    length = __builtin_sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    correction = (*restLength - length) * 0.9f;
    solverBody->accumulator.vector.x -= dir.x * correction;
    solverBody->accumulator.vector.y -= dir.y * correction;
    solverBody->accumulator.vector.z -= dir.z * correction;
    solverBody->position.vector.x -= dir.x * correction * 0.1f;
    solverBody->position.vector.y -= dir.y * correction * 0.1f;
    solverBody->position.vector.z -= dir.z * correction * 0.1f;
}

void tiDynamicsSolveCellPairs(TiMRPackage *world) {
    TiSolverState *solverBodies;
    int leftStart;
    int rightStart;
    int dynamicsNodeBase;
    TiLinkCell *cell;
    TiDynamicsLink *restLink;
    unsigned int linksDone;
    unsigned int cellIndex;

    solverBodies = world->dynMat;
    leftStart = world->hrcData->jointCount + world->hrcData->skipCount;
    rightStart = world->hrcData->joint3Count + world->hrcData->joint2Count;
    dynamicsNodeBase = leftStart + rightStart;
    /* the link table sits at a byte offset read from the hrc data at runtime */
    cell = (TiLinkCell *)BYTE_OFFSET(world->hrcData, world->hrcData->linkCellOffset);
    cellIndex = 0;
    if (world->hrcData->linkCellCount != 0) {
        do {
            restLink = (TiDynamicsLink *)(cell + 1);
            linksDone = 0;
            if (cell->linkedNodeCount != 0) {
                do {
                    linksDone++;
                    tiDynamicsSolveLink(&solverBodies[cell->firstNodeId - dynamicsNodeBase], world->globalMat[restLink->otherNodeId], &restLink->restLength);
                    tiDynamicsCollSolveSpherePair(&solverBodies[cell->firstNodeId - dynamicsNodeBase], &solverBodies[restLink->otherNodeId - dynamicsNodeBase], world);
                    tiDynamicsCollSolveMatrixPair(&solverBodies[cell->firstNodeId - dynamicsNodeBase], &solverBodies[restLink->otherNodeId - dynamicsNodeBase], world);
                    tiDynamicsCollSolveCapsulePair(&solverBodies[cell->firstNodeId - dynamicsNodeBase], &solverBodies[restLink->otherNodeId - dynamicsNodeBase], world);
                    restLink++;
                } while (linksDone < cell->linkedNodeCount);
            }
            cell = (TiLinkCell *)restLink;
        } while (++cellIndex < world->hrcData->linkCellCount);
    }
}
