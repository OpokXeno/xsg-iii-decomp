#include "common.h"
#include "main/xenoActor.h"

typedef int s32;

void xglMatrixMul(Matrix4 destination, Matrix4 left, Matrix4 right);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
extern const Vector4 D_005412B0[];

void xenoActorTransformJointPoint(XenoActor *actor, s32 jointIndex, Vector4 *jointPosition) {
    Matrix4 matrix;

    xglMatrixMul(matrix, actor->localMatrix, actor->jointMatrices[jointIndex].m);
    xglVectorMulMat(jointPosition, matrix, D_005412B0);
}

extern XenoActor **XenoActorActiveTable;
extern s32 XenoActorPoolCount;

void xenoActorSetAttrBit8(s32 bitIsSet) {
    s32 remaining;
    XenoActor **activeSlot;
    XenoActor *activeActor;

    if (XenoActorPoolCount > 0) {
        activeSlot = XenoActorActiveTable;
        remaining = XenoActorPoolCount;
        do {
            activeActor = *activeSlot;
            activeSlot += 1;
            if (activeActor != 0) {
                if (bitIsSet != 0) {
                    activeActor->attr |= 8;
                } else {
                    activeActor->attr &= 0xFFF7;
                }
            }
            remaining -= 1;
        } while (remaining != 0);
    }
}

void func_0029FB88(Vector4 *vector);

void xenoActorUpdateShadowB(XenoActor *actor) {
    func_0029FB88(&actor->shadowB);
}
