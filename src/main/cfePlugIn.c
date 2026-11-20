#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

#include "quadword.h"

void cfeRotateVectorXY(float *result, const float *angles);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglMatrixStackUnit(void);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackSave(Matrix4 matrix);
extern const AlignedVector4 cfeForwardVector;

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019E918);

/* The vector cfeForwardVector rotated by angles[0] about X and angles[1] about Y. */
void cfeRotateVectorXY(float *result, const float *angles)
{
    Matrix4 matrix;
    AlignedVector4 vector = cfeForwardVector;

    xglMatrixStackUnit();
    xglMatrixStackRotX(angles[0]);
    xglMatrixStackRotY(angles[1]);
    xglMatrixStackSave(matrix);
    xglVectorMulMat(&vector, matrix, &vector);
    QuadwordCopy(result, &vector);
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019E9A8);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EA60);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EAC0);

/* The word block at 0x8c28b0 that holds the joint and global matrix pointers and the flags word. */
typedef struct PxyGlobalBlock {
    u8 unmodeled_000[0x300];
    int renderStatus; /* +0x300 */
    u8 unmodeled_304[0x308 - 0x304];
    int shadowMode; /* +0x308 */
    u8 unmodeled_30c[0x330 - 0x30c];
    Matrix4 *jointMatrices; /* +0x330 */
    u8 unmodeled_334[0x360 - 0x334];
    float shadowAlpha; /* +0x360 */
    u8 unmodeled_364[0x394 - 0x364];
    int flags; /* +0x394 */
    u8 unmodeled_398[0x39c - 0x398];
    float animTime; /* +0x39c */
    u8 unmodeled_3a0[0x3a4 - 0x3a0];
    float shadowPlaneHeight; /* +0x3a4 */
    u8 unmodeled_3a8[0x3bc - 0x3a8];
    Matrix4 *globalMatrices; /* +0x3bc */
    u8 unmodeled_3c0[0x698 - 0x3c0];
    int pxyFlags; /* +0x698 */
    u8 unmodeled_69c[0x6a8 - 0x69c];
    int freezeAnimation; /* +0x6a8 */
    u8 unmodeled_6ac[0x6e8 - 0x6ac];
    float shadowScale; /* +0x6e8 */
} PxyGlobalBlock;

extern PxyGlobalBlock D_008C28B0;

void pxyRequestTextureCopyPass(void)
{
    D_008C28B0.pxyFlags |= 0x2;
}

void func_0019EAF0(void)
{
    D_008C28B0.pxyFlags |= 0x8;
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EB08);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EB28);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EB40);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EB58);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EB70);

void pxyFreezeAnimation(void)
{
    D_008C28B0.freezeAnimation = 1;
    D_008C28B0.renderStatus |= 0x2;
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EBB0);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EBD0);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EBF0);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EC10);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EC28);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EC40);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EC58);

void func_0019EC70(void)
{
    D_008C28B0.renderStatus |= 0x20000;
    D_008C28B0.pxyFlags |= 0x8000;
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EC98);

void pxySetAnimTime(float time)
{
    D_008C28B0.animTime = time;
    D_008C28B0.renderStatus |= 0x100;
}

void func_0019ECC8(void)
{
    D_008C28B0.renderStatus |= 0x40000;
    D_008C28B0.pxyFlags |= 0x100000;
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019ECF8);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019ED18);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019ED28);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019ED48);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019ED68);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EDF0);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", pxySetDrawCallback);

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EE28);

void pxyAddFlags(int flags)
{
    D_008C28B0.flags |= flags;
}

void pxySetShadowMode(int mode, float alpha, float planeHeight, float scale)
{
    if (alpha <= 0.0f) {
        return;
    }
    D_008C28B0.shadowScale = scale;
    D_008C28B0.shadowAlpha = alpha;
    D_008C28B0.shadowPlaneHeight = planeHeight;
    D_008C28B0.shadowMode = mode;
    if (mode == 4 || mode == 1 || mode == 5) {
        D_008C28B0.renderStatus |= 0x2;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EEB8);

void pxySetJointMatrices(Matrix4 *joints, Matrix4 *global)
{
    D_008C28B0.jointMatrices = joints;
    D_008C28B0.globalMatrices = global;
}
