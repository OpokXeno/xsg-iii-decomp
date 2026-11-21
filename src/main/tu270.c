#include "common.h"
#include "shared.h"
#include "main/xgl_2.h"

extern Vector4 D_003E3860;
extern Vector4 D_003E3870;
extern Vector4 D_003E3880;
extern Vector4 D_003E3890;
extern Vector4 D_003E38A0;
extern Vector4 D_003E38B0;
void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source);
void xglMatrixStackRotX(f32 angle);
void xglMatrixStackRotY(f32 angle);
void xglMatrixStackRotZ(f32 angle);
void xglMatrixStackUnit(void);

void xglStudioTransformCameraVectors(void) {
    Vector4 *cameraRotation;

    cameraRotation = &xglStudioGetCamera2(0)->travel.rotation;
    xglMatrixStackUnit();
    xglMatrixStackRotY(cameraRotation->y);
    xglMatrixStackRotX(cameraRotation->x);
    xglMatrixStackRotZ(cameraRotation->z);
    xglMatrixStackMulVector(&D_003E3860, &D_003E3890);
    xglMatrixStackMulVector(&D_003E3870, &D_003E38A0);
    xglMatrixStackMulVector(&D_003E3880, &D_003E38B0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu270", func_00268710);

INCLUDE_ASM("asm/main/nonmatchings/tu270", func_00268820);
