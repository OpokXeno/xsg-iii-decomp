#include "common.h"
#include "shared.h"
/* Row-major 4x4 float matrix. */
#include "quadword.h"

void cfeDirectionFromAngles(float *direction, const float *angles);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglMatrixStackUnit(void);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackSave(Matrix4 matrix);
extern const AlignedVector4 cfeForwardVector;

typedef int s32;

typedef struct PxyModelRenderState {
    u8 pad_000[0x300];
    int renderStatus; /* +0x300 */
    u8 pad_304[0x308 - 0x304];
    int shadowMode; /* +0x308 */
    u8 pad_30c[0x330 - 0x30c];
    Matrix4 *jointMatrices; /* +0x330 */
    u8 pad_334[0x360 - 0x334];
    float shadowAlpha; /* +0x360 */
    u8 pad_364[0x368 - 0x364];
    float shadowOffsetX; /* +0x368 */
    float shadowOffsetY; /* +0x36c */
    u8 pad_370[0x394 - 0x370];
    int renderLevel; /* +0x394 */
    u8 pad_398[0x39c - 0x398];
    float animTime; /* +0x39c */
    u8 pad_3a0[0x3a4 - 0x3a0];
    float shadowPlaneHeight; /* +0x3a4 */
    u8 pad_3a8[0x3bc - 0x3a8];
    Matrix4 *globalMatrices; /* +0x3bc */
    u8 pad_3c0[0x3d0 - 0x3c0];
    float planeParamY; /* +0x3d0 */
    u8 pad_3d4[0x3d8 - 0x3d4];
    float planeParamX; /* +0x3d8 */
    u8 pad_3dc[0x3e4 - 0x3dc];
    int renderStateFlags; /* +0x3e4 */
    u8 pad_3e8[0x4d0 - 0x3e8];
    int latchedValueA; /* +0x4d0 */
    u8 pad_4d4[0x4e0 - 0x4d4];
    int latchedValueB; /* +0x4e0 */
    u8 pad_4e4[0x698 - 0x4e4];
    int drawFlags; /* +0x698 */
    u8 pad_69c[0x6a8 - 0x69c];
    int freezeAnimation; /* +0x6a8 */
    u8 pad_6ac[0x6e8 - 0x6ac];
    float shadowScale; /* +0x6e8 */
} PxyModelRenderState;

extern s32 gPxySyncMode;
extern PxyModelRenderState gPxyModelRenderStateStorage;

void pxySetSyncMode(s32 syncMode) {
    gPxySyncMode = syncMode;
}

/* The vector cfeForwardVector rotated by angles[0] about X and angles[1] about Y. */
void cfeDirectionFromAngles(float *direction, const float *angles)
{
    Matrix4 matrix;
    AlignedVector4 vector = cfeForwardVector;

    xglMatrixStackUnit();
    xglMatrixStackRotX(angles[0]);
    xglMatrixStackRotY(angles[1]);
    xglMatrixStackSave(matrix);
    xglVectorMulMat(&vector, matrix, &vector);
    QuadwordCopy(direction, &vector);
}

#include "main/tu045.h"

int func_0030E68C(unsigned char *geometryEntry, const char *name);

/* Index of the first geometry geometryEntry (112 bytes each) of a "pxy" pack whose name matches partName; -1 when none does. */
int cfeModelFindPart(PxyModelHeader *pack, const char *partName) {
    unsigned char *geometryEntry;
    int i;

    if (partName != 0 && pack != 0 && (*(u64 *)pack & 0xFFFFFF) == 0x797870) {
        geometryEntry = (unsigned char *)pack + pack->geometryOffset;
        for (i = 0; i < pack->geometryCount; i++) {
            if (func_0030E68C(geometryEntry, partName) == 0) {
                return i;
            }
            geometryEntry += 112;
        }
    }
    return -1;
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019EA60);

void pxySetShadowOffset(f32 offsetX, f32 offsetY)
{
    gPxyModelRenderStateStorage.shadowOffsetX = offsetX;
    gPxyModelRenderStateStorage.shadowOffsetY = offsetY;
}

void pxyRequestTextureCopyPass(void)
{
    gPxyModelRenderStateStorage.drawFlags |= 0x2;
}

void pxyEnableFlag8(void)
{
    gPxyModelRenderStateStorage.drawFlags |= 0x8;
}

void pxyEnableFlag800000(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x800000;
}

void pxySetOverlayAlphaMaskMode(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x2000;
}

void pxySkipOverlayDraw(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x200;
}

void pxyRequestFrameClearOnly(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x4000;
}

void pxySetShadowUploadFlag(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x20000;
}

void pxyFreezeAnimation(void)
{
    gPxyModelRenderStateStorage.freezeAnimation = 1;
    gPxyModelRenderStateStorage.renderStatus |= 0x2;
}

void pxySetHeaderScanEnabled(s32 enabled) {
    if (enabled != 0) {
        gPxyModelRenderStateStorage.drawFlags |= 0x10;
    }
}

void pxyEnableFlag80000(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x80000;
}

void pxySetShadowProjectOff(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x40000;
}

void pxySkipTextureUpload(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x40;
}

void pxySetBlackFog(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x80;
}

void pxySetStateFlag4(void)
{
    gPxyModelRenderStateStorage.renderStateFlags |= 0x4;
}

void pxyEnableRenderFlag800(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x800;
}

void pxyEnableRenderFlag20000(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x20000;
    gPxyModelRenderStateStorage.drawFlags |= 0x8000;
}

extern s32 pxyStoredValue;

void pxySetStoredValue(s32 storedValue) {
    pxyStoredValue = storedValue;
}

void pxySetAnimTime(float time)
{
    gPxyModelRenderStateStorage.animTime = time;
    gPxyModelRenderStateStorage.renderStatus |= 0x100;
}

void pxyEnableRenderFlag40000(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x40000;
    gPxyModelRenderStateStorage.drawFlags |= 0x100000;
}

void pxyEnableRenderFlag10000(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x10000;
}

extern s32 pxyPendingFlag;

void pxySetPendingFlag(void) {
    pxyPendingFlag = 1;
}

void pxyLatchValueA(s32 valueToLatch)
{
    if (gPxyModelRenderStateStorage.latchedValueA == 0)
    {
        gPxyModelRenderStateStorage.latchedValueA = valueToLatch;
    }
}

void pxyLatchValueB(s32 valueToLatch)
{
    if (gPxyModelRenderStateStorage.latchedValueB == 0)
    {
        gPxyModelRenderStateStorage.latchedValueB = valueToLatch;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/cfePlugIn", func_0019ED68);

void pxyEnableFlag400000(s32 enabled) {
    if (enabled != 0) {
        gPxyModelRenderStateStorage.drawFlags |= 0x400000;
    }
}

extern s32 pxyDrawCallback;

void pxySetDrawCallback(s32 drawCallback) {
    pxyDrawCallback = drawCallback;
}

extern s32 pxyCameraEnabled[];

void pxySetCameraEnabled(s32 camera, s32 enabled) {
    pxyCameraEnabled[camera] = enabled;
}

void pxyAddRenderLevel(int levelBits)
{
    gPxyModelRenderStateStorage.renderLevel |= levelBits;
}

void pxySetShadowMode(int shadowMode, float alpha, float planeHeight, float scale)
{
    if (alpha <= 0.0f) {
        return;
    }
    gPxyModelRenderStateStorage.shadowScale = scale;
    gPxyModelRenderStateStorage.shadowAlpha = alpha;
    gPxyModelRenderStateStorage.shadowPlaneHeight = planeHeight;
    gPxyModelRenderStateStorage.shadowMode = shadowMode;
    if (shadowMode == 4 || shadowMode == 1 || shadowMode == 5) {
        gPxyModelRenderStateStorage.renderStatus |= 0x2;
    }
}

void pxySetPlaneParams(f32 paramX, f32 paramY)
{
    gPxyModelRenderStateStorage.planeParamX = paramX;
    gPxyModelRenderStateStorage.planeParamY = paramY;
}

void pxySetJointMatrices(Matrix4 *joints, Matrix4 *globalMatrices)
{
    gPxyModelRenderStateStorage.jointMatrices = joints;
    gPxyModelRenderStateStorage.globalMatrices = globalMatrices;
}
