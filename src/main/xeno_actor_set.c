#include "common.h"
#include "shared.h"

#include "main/xenoActor.h"



void xglCurSetTranslate(const float *translation);
void xglCurMulRotateZ(float angle);
void xglCurMulRotateY(float angle);
void xglCurMulRotateX(float angle);
void xglCurMulScaleXYZ(const float *scale);
/* int: the tail jump forwards the callee's v0, which the function returns. */
int xglCurApplyVector(Vector4 *destination, const Vector4 *vector);
/* Direction table at 0x4ADF80 (scaffold label gFocusDirectionOffsets): one float pair per direction. */
extern float gFocusDirectionOffsets[][2];

void xenoActorSetFocusDirection(struct XenoActor *actor, int direction) {
    if (direction < 9) {
        actor->focusX = actor->focusX * 0.9f + gFocusDirectionOffsets[direction][0] * 0.1f;
        actor->focusY = actor->focusY * 0.9f + gFocusDirectionOffsets[direction][1] * 0.1f;
        if (actor->focusX != 0.0f || actor->focusY != 0.0f) {
            actor->drawFlags |= 0x100;
        } else {
            actor->drawFlags &= ~0x100;
        }
    }
}

int xenoActorTransformPoint(struct XenoActor *actor, Vector4 *transformedPoint) {
    XenoHrc *hrc = actor->pHrcData;
    XenoTransform *transform;

    if (hrc == 0) {
        return 0;
    }
    transform = &hrc->transform;
    xglCurSetTranslate(transform->translation);
    xglCurMulRotateZ(transform->rotationZ);
    xglCurMulRotateY(transform->rotationY);
    xglCurMulRotateX(transform->rotationX);
    xglCurMulScaleXYZ(transform->scale);
    transformedPoint->x = transform->point[0];
    transformedPoint->y = transform->point[1];
    transformedPoint->z = transform->point[2];
    transformedPoint->w = 1.0f;
    return xglCurApplyVector(transformedPoint, transformedPoint);
}

void xenoActorSetShadowB2Flag(struct XenoActor *actor) {
    *(int *)actor->shadowB2 |= 0x80;
}

float gFocusDirectionOffsets[][2] = {
    { 0.0f, 0.0f },
    { 0.0f, -0.100000001f },
    { 0.0f, 0.150000006f },
    { 0.100000001f, 0.0f },
    { -0.100000001f, 0.0f },
    { 0.100000001f, -0.100000001f },
    { -0.100000001f, -0.100000001f },
    { 0.100000001f, 0.150000006f },
    { -0.100000001f, 0.150000006f },
    { 0.0f, 0.0f },
};
