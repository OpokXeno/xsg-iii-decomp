#include "common.h"
#include "shared.h"

#include "main/tu031.h"

extern const DoublewordVector4 lightDebugBoxPointTable;
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
    DoublewordVector4 debugBoxPointA;
    DoublewordVector4 debugBoxPointB;
    DoublewordVector4 debugBoxPointC;
    DoublewordVector4 debugBoxPointD;
    DoublewordVector4 debugBoxPointE;

    debugBoxPointA = lightDebugBoxPointTable;
    debugBoxPointB = D_00541390;
    debugBoxPointC = D_005413A0;
    debugBoxPointD = D_005413B0;
    debugBoxPointE = D_005413C0;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&key->position);
    xglMatrixStackRotZ(key->rotation.z);
    xglMatrixStackRotY(key->rotation.y);
    xglMatrixStackRotX(key->rotation.x);
    xglMatrixStackScale(&key->extent);
    xglMatrixStackMulVector(&debugBoxPointA.vector, &debugBoxPointA.vector);
    xglMatrixStackMulVector(&debugBoxPointB.vector, &debugBoxPointB.vector);
    xglMatrixStackMulVector(&debugBoxPointC.vector, &debugBoxPointC.vector);
    xglMatrixStackMulVector(&debugBoxPointD.vector, &debugBoxPointD.vector);
    xglMatrixStackMulVector(&debugBoxPointE.vector, &debugBoxPointE.vector);
}

void xglLightPackDebugEmpty(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00198E48);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199018);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199030);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199148);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199160);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_001994F0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_light_031", func_00199650);
