#include "common.h"
#include "shared.h"

#include "main/tu031.h"

extern const DoublewordVector4 D_00541380;
extern const DoublewordVector4 D_00541390;
extern const DoublewordVector4 D_005413A0;
extern const DoublewordVector4 D_005413B0;
extern const DoublewordVector4 D_005413C0;
void xglMatrixStackUnit(void);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackScale(const Vector4 *vector);
void xglMatrixStackMulVector(Vector4 *destination, const Vector4 *source);

void xglLightPackDebugBox(XglLightKey *key)
{
    int scaled[3];
    DoublewordVector4 point0;
    DoublewordVector4 point1;
    DoublewordVector4 point2;
    DoublewordVector4 point3;
    DoublewordVector4 point4;

    point0 = D_00541380;
    point1 = D_00541390;
    point2 = D_005413A0;
    point3 = D_005413B0;
    point4 = D_005413C0;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    xglMatrixStackRotZ(key->rotation.z);
    xglMatrixStackRotY(key->rotation.y);
    xglMatrixStackRotX(key->rotation.x);
    xglMatrixStackScale(&key->extent);
    xglMatrixStackMulVector(&point0.vector, &point0.vector);
    xglMatrixStackMulVector(&point1.vector, &point1.vector);
    xglMatrixStackMulVector(&point2.vector, &point2.vector);
    xglMatrixStackMulVector(&point3.vector, &point3.vector);
    xglMatrixStackMulVector(&point4.vector, &point4.vector);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00198E40);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00198E48);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199018);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199030);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199148);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199160);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_001994F0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199650);
