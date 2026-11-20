#include "common.h"
#include "shared.h"

#include "main/TiDynamicsColl.h"
#include "main/TiMRPackage.h"











typedef struct TiSolverState TiDynamicsOut;














void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglMatrixInverse(Matrix4 destination, Matrix4 source);
void xglMatrixMul(Matrix4 destination, Matrix4 left, Matrix4 right);
void tiMtnBlendLerpMatrix(Matrix4 dst, Matrix4 a, Matrix4 b, float t);
void XrgQuaternionFromVectors(Vector4 *destination, const Vector4 *from, const Vector4 *to);
void XrgQuaternionToMatrix(Matrix4 matrix, const Vector4 *quaternion);
void tiDynamicsCollPushOutSphereList(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state);
void tiDynamicsCollPushOutMatrixList(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state);
void tiDynamicsCollPushOutCapsuleList(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state);
void tiGlobalCollSolveBody(TiHrcDynamics *pNode, TiSolverState *context);
void tiGlobalCollClampScale(TiSolverState *state);
void tiDynamicsCollSolveSpherePair(TiSolverState *first, TiSolverState *second, TiMRPackage *world);
void tiDynamicsCollSolveMatrixPair(TiSolverState *first, TiSolverState *second, TiMRPackage *world);
void tiDynamicsCollSolveCapsulePair(TiSolverState *first, TiSolverState *second, TiMRPackage *world);
void func_00298040(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state);

/* TiSolverObject flag bit tested by tiDynamicsCollSolveNode: clamp the node scale. */
#define TI_OBJECT_FLAG_CLAMP_SCALE 0x80

typedef union { float value; } FloatAlias;

/* A cell header of the link table: first node index and link count. */
typedef struct TiLinkCell {
    int firstNode;
    int linkCount;
    unsigned char unmodeled_08[0x8];
} TiLinkCell;

/* A link of the table: rest length and the node index of the other end. */
typedef struct TiLinkPair {
    float restLength;
    int node;
    unsigned char unmodeled_08[0x8];
} TiLinkPair;


INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_002973D8);

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_002973F0);

/* Collision pass for one node: the dynamics push-outs (sphere list, matrix list, capsule list), the
 * global collision body and, when object flag 0x80 is set, the scale clamp. Called twice per step. */
void tiDynamicsCollSolveNode(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state) {
    tiDynamicsCollPushOutSphereList(object, node, state);
    tiDynamicsCollPushOutMatrixList(object, node, state);
    tiDynamicsCollPushOutCapsuleList(object, node, state);
    tiGlobalCollSolveBody(node, state);
    if (object->state_a.flags & TI_OBJECT_FLAG_CLAMP_SCALE) {
        tiGlobalCollClampScale(state);
    }
}

/* Pulls the node's point toward its matrix-transformed target, weighted by node->pullScale. */
void tiDynamicsNodePullToTarget(float (*matrix)[4], TiHrcDynamics *node, TiSolverState *state) {
    Vector4 transformed;

    if (node->pullScale != 0.0f) {
        xglVectorMulMat(&transformed, matrix, &node->translationDefault);
        state->accumulator.vector.x += (transformed.x - state->position.vector.x) * node->pullScale;
        state->accumulator.vector.y += (transformed.y - state->position.vector.y) * node->pullScale;
        state->accumulator.vector.z += (transformed.z - state->position.vector.z) * node->pullScale;
    }
}

/* Moves the node point onto the matrix origin plus normalScale times the unit direction from the origin. */
void tiDynamicsNodeConstrainNormal(Matrix4 matrix, TiHrcDynamics *node, TiSolverState *state) {
    Vector4 delta;
    Vector4 unit;
    float positionX;
    float positionY;
    float positionZ;

    delta.x = state->position.vector.x - matrix[3][0];
    delta.y = state->position.vector.y - matrix[3][1];
    delta.z = state->position.vector.z - matrix[3][2];
    xglVectorNormal(&unit, &delta);
    positionX = unit.x * node->normalScale + matrix[3][0];
    positionY = unit.y * node->normalScale + matrix[3][1];
    positionZ = unit.z * node->normalScale + matrix[3][2];
    state->position.vector.x = positionX;
    state->position.vector.y = positionY;
    state->position.vector.z = positionZ;
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
        ((FloatAlias *)row_z)[-1].value = ((FloatAlias *)row_z)[-1].value * length;
        ((FloatAlias *)row_z)[0].value = ((FloatAlias *)row_z)[0].value * length;
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
void tiDynamicsNodeSolvePalette(TiMRPackage *unused_object, Matrix4 matrix, TiHrcDynamics *node, TiSolverState *state) {
    Matrix4 blended;
    Matrix4 rotation;
    Vector4 local_position;
    Vector4 local_direction;
    Vector4 target_direction;
    Matrix4 inverse;
    Vector4 align_quat;

    if (1.0f <= node->weight) {
        return;
    }
    xglMatrixInverse(inverse, matrix);
    xglVectorMulMat(&local_position, inverse, &state->position.vector);
    xglVectorNormal(&local_direction, &local_position);
    xglVectorNormal(&target_direction, &node->translationDefault);
    XrgQuaternionFromVectors(&align_quat, &target_direction, &local_direction);
    XrgQuaternionToMatrix(rotation, &align_quat);
    if (node->weight != 0.0f) {
        xglMatrixMul(blended, matrix, rotation);
        tiMtnBlendLerpMatrix(matrix, blended, matrix, node->weight);
        tiMatrixNormalizeRows(matrix);
    } else {
        xglMatrixMul(matrix, matrix, rotation);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_00298040);

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_002980A0);

TiHrcDynamics *tiDynamicsNodeSolveNext(TiMRPackage *object, TiHrcDynamics *node, TiSolverState *state) {
    unsigned int stepCount;
    TiCollEntry *entries;

    object->timeScale = 1.0f;
    stepCount = 0;
    do {
        stepCount++;
        func_00298040(object, node, state);
    } while (stepCount == 0);
    entries = node->entries;
    return (TiHrcDynamics *)(entries + (node->sphereCount + node->matrixCount + node->capsuleCount));
}

INCLUDE_ASM("asm/main/nonmatchings/ti_dynamics_node", func_00298290);

/* Moves the solver body toward the rest length of its link to the matrix other (its translation row). */
void tiDynamicsSolveLink(TiSolverState *state, Matrix4 other, const float *restLength) {
    TiVector3 dir;
    float length;
    float correction;

    dir = (TiVector3) { other[3][0] - state->position.vector.x, other[3][1] - state->position.vector.y, other[3][2] - state->position.vector.z };
    length = __builtin_sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    correction = (*restLength - length) * 0.9f;
    state->accumulator.vector.x -= dir.x * correction;
    state->accumulator.vector.y -= dir.y * correction;
    state->accumulator.vector.z -= dir.z * correction;
    state->position.vector.x -= dir.x * correction * 0.1f;
    state->position.vector.y -= dir.y * correction * 0.1f;
    state->position.vector.z -= dir.z * correction * 0.1f;
}

void tiDynamicsSolveCellPairs(TiMRPackage *world) {
    TiSolverState *nodes;
    int leftStart;
    int rightStart;
    int nodeBase;
    TiLinkCell *cell;
    TiLinkPair *pair;
    unsigned int pairIndex;
    unsigned int cellIndex;

    nodes = world->dynMat;
    leftStart = world->hrcData->jointCount + world->hrcData->skipCount;
    rightStart = world->hrcData->joint3Count + world->hrcData->joint2Count;
    nodeBase = leftStart + rightStart;
    cell = (TiLinkCell *)((char *)world->hrcData + world->hrcData->linkCellOffset);
    cellIndex = 0;
    if (world->hrcData->linkCellCount != 0) {
        do {
            pair = (TiLinkPair *)(cell + 1);
            pairIndex = 0;
            if (cell->linkCount != 0) {
                do {
                    pairIndex++;
                    tiDynamicsSolveLink(&nodes[cell->firstNode - nodeBase], world->globalMat[pair->node], &pair->restLength);
                    tiDynamicsCollSolveSpherePair(&nodes[cell->firstNode - nodeBase], &nodes[pair->node - nodeBase], world);
                    tiDynamicsCollSolveMatrixPair(&nodes[cell->firstNode - nodeBase], &nodes[pair->node - nodeBase], world);
                    tiDynamicsCollSolveCapsulePair(&nodes[cell->firstNode - nodeBase], &nodes[pair->node - nodeBase], world);
                    pair++;
                } while (pairIndex < cell->linkCount);
            }
            cell = (TiLinkCell *)pair;
        } while (++cellIndex < world->hrcData->linkCellCount);
    }
}
