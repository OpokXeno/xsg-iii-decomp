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
/* Direction table at 0x4ADF80 (scaffold label D_004ADF80): one float pair per direction. */
extern float D_004ADF80[][2];

void func_00291D88(struct XenoActor *actor, int dir) {
    if (dir < 9) {
        actor->focusX = actor->focusX * 0.9f + D_004ADF80[dir][0] * 0.1f;
        actor->focusY = actor->focusY * 0.9f + D_004ADF80[dir][1] * 0.1f;
        if (actor->focusX != 0.0f || actor->focusY != 0.0f) {
            actor->drawFlags |= 0x100;
        } else {
            actor->drawFlags &= ~0x100;
        }
    }
}

int func_00291E20(struct XenoActor *actor, Vector4 *out) {
    XenoHrc *table = actor->pHrcData;
    XenoTransform *transform;

    if (table == 0) {
        return 0;
    }
    transform = &table->transform;
    xglCurSetTranslate(transform->translation);
    xglCurMulRotateZ(transform->rotationZ);
    xglCurMulRotateY(transform->rotationY);
    xglCurMulRotateX(transform->rotationX);
    xglCurMulScaleXYZ(transform->scale);
    out->x = transform->point[0];
    out->y = transform->point[1];
    out->z = transform->point[2];
    out->w = 1.0f;
    return xglCurApplyVector(out, out);
}

INCLUDE_ASM("asm/main/nonmatchings/xeno_actor_set", func_00291ED0);
