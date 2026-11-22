#include "common.h"
#include "shared.h"
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
#include "main/tu141.h"
#include "main/xgl_2.h"
#include "main/nml_model_set.h"


/* The block that contains the member `member` stored at `field` (the fog parameters are reached by their own symbols). */
#define MEMBER_OFFSET(type, member) ((int) &((type *) 0)->member)
#define BLOCK_OF_MEMBER(field, member) ((PxyModelRenderState *) ((u8 *) (field) - MEMBER_OFFSET(PxyModelRenderState, member)))

void pxySetLightPtr(XglLightSet *light);
extern XglLightSet *pxyCurrentLight;
extern void *gNmlModelSetCurrent;
extern Vector4 gNmlModelFogNearParams[];

void nmlModelFogPara(Vector4 *out, f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    f32 shadeMin;
    f32 shadeMax;

    shadeMin = (1.0f - fogMin) * 255.0f;
    shadeMax = (1.0f - fogMax) * 255.0f;
    if (shadeMax < 0.0f) {
        shadeMax = 0.0f;
    }
    if (255.0f < shadeMax) {
        shadeMax = 255.0f;
    }
    if (shadeMin < 0.0f) {
        shadeMin = 0.0f;
    }
    if (255.0f < shadeMin) {
        shadeMin = 255.0f;
    }
    out->x = shadeMax;
    out->y = shadeMin;
    out->z = (((shadeMax - shadeMin) * (fogFar + fogNear)) / (fogFar - fogNear) + (shadeMax + shadeMin)) * 0.5f;
    out->w = (fogFar * fogNear) * (shadeMin - shadeMax) / (fogFar - fogNear);
}

void nmlModelSetFogDist(f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    PxyModelRenderState *modelSet;

    modelSet = BLOCK_OF_MEMBER(gNmlModelFogNearParams, fog_near);
    nmlModelFogPara(gNmlModelFogNearParams, fogNear, fogFar, fogMin, fogMax);
    modelSet->renderStatus |= 1;
}

#include "main/tu045.h"

XglCamera *xglStudioSelectGetActiveCamera(int studio_index);
void _CurMatrixSet(Matrix4 *matrix);
void _CurRotTransPersFog(Vector4 *projected, const Vector4 *position, const Vector4 *fog);

int nmlModelGetFogParaStudio(const Vector4 *position, int studio)
{
    int fogShade;
    Vector4 projected;
    XglCamera *camera;
    Vector4 *fogPara;

    fogShade = 255;
    fogPara = gNmlModelFogNearParams;
    camera = xglStudioSelectGetActiveCamera(studio);
    if (camera != 0) {
        _CurMatrixSet(&camera->volume[0].viewMatrix);
        _CurRotTransPersFog(&projected, position, fogPara);
        fogShade = (u32)projected.w;
        if (projected.z < 0.0f) {
            fogShade = (u32)fogPara->y;
        }
    }
    return fogShade;
}

extern Vector4 gNmlModelFogFarParams[];

void nmlModelSetFogFarDist(f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    PxyModelRenderState *modelSet;

    modelSet = BLOCK_OF_MEMBER(gNmlModelFogFarParams, fog_far);
    nmlModelFogPara(gNmlModelFogFarParams, fogNear, fogFar, fogMin, fogMax);
    modelSet->drawFlags |= 0x20;
}

void nmlModelSetGlobalFogCol(Vector4 *color)
{
    gPxyModelRenderStateStorage.globalFogColor.vector = *color;
    gPxyModelRenderStateStorage.drawFlags |= 0x20;
}

extern f32 gNmlModelSortOffset;

void nmlModelSetSortOffset(f32 sortOffset) {
    gNmlModelSortOffset = sortOffset;
}

/* The global colour scale (X2 pxySetColorScale writes pxyGlobalColor). */
extern Vector4 pxyGlobalColor;

void pxySetColorScale(Vector4 *color)
{
    pxyGlobalColor = *color;
}

void nmlModelSetCurrentPtr(void *modelSetCurrent)
{
    gNmlModelSetCurrent = modelSetCurrent;
}

extern int gNmlModelSetIndex;

void nmlModelSetIndexValue(int modelSetIndex) {
    gNmlModelSetIndex = modelSetIndex;
}

/* 0x008C2BF0: the texture block the model draws with (X2 pxyTxyBlock); scaffold .bss label of this TU. */
extern TxyBlock *nmlCurrentTxyBlock;

void nmlModelSetTexture(TxyBlock *block)
{
    u8 *bytes;
    u32 byteIndex;

    if ((u32)block & 0xF) {
        return;
    }
    if (block != 0 && block->magic[0] == 't' && block->magic[1] == 'x' && block->magic[2] == 'y') {
        if (block->flags & 0x80000000) {
            bytes = (u8 *)block;
            byteIndex = 16;
            while (byteIndex < block->size) {
                if (bytes[byteIndex] == 1) {
                    bytes[byteIndex] = -1;
                } else if (bytes[byteIndex] != 0) {
                    bytes[byteIndex] = bytes[byteIndex] + 0xFF;
                }
                byteIndex++;
            }
            block->flags &= 0x7FFFFFFF;
        }
        nmlCurrentTxyBlock = block;
    }
}

void pxySetModelPosition(Vector4 *modelPosition)
{
    gPxyModelRenderStateStorage.position = *modelPosition;
}

void pxySetLightPtr(XglLightSet *light)
{
    pxyCurrentLight = light;
}

#include "quadword.h"

/* The matrix nmlModelSetDrawMatrix stores (nml_model_set .bss). */
extern Matrix4 nmlModelDrawMatrix;

void nmlModelSetDrawMatrix(Matrix4 matrix)
{
    QuadwordCopyMatrix(nmlModelDrawMatrix, matrix);
}

/* X2 nmlModelSetScale: keeps the scale and its largest component. */
void nmlModelSetScale(const Vector4 *scale)
{
    gPxyModelRenderStateStorage.modelScale = *scale;
    gPxyModelRenderStateStorage.modelScaleMax = scale->x;
    if (gPxyModelRenderStateStorage.modelScaleMax < scale->y) {
        gPxyModelRenderStateStorage.modelScaleMax = scale->y;
    }
    if (gPxyModelRenderStateStorage.modelScaleMax < scale->z) {
        gPxyModelRenderStateStorage.modelScaleMax = scale->z;
    }
}

void nmlModelSetZwrite(int zwrite) {
    if (zwrite != 0) {
        gPxyModelRenderStateStorage.renderStatus |= 8;
    }
}

void nmlModelSetTransparency(f32 transparency)
{
    f32 clamped;

    clamped = transparency;
    if (clamped < 0.0f)
    {
        clamped = 0.0f;
    }
    if (clamped > 1.0f)
    {
        clamped = 1.0f;
    }
    gPxyModelRenderStateStorage.transparency[0] = clamped;
    gPxyModelRenderStateStorage.transparency[1] = clamped;
}

extern int nmlModelStoredMode;

void nmlModelSetStoredMode(int storedMode) {
    nmlModelStoredMode = storedMode;
}

extern f32 gNmlModelGlobalAlpha;

void nmlModelSetGlobalAlpha(f32 alpha) {
    f32 clamped;

    clamped = alpha;
    if (clamped < 0.0f) {
        clamped = 0.0f;
    }
    if (clamped > 1.0f) {
        clamped = 1.0f;
    }
    gNmlModelGlobalAlpha = clamped;
}

void nmlModelSetStencil(int stencil) {
    if (stencil != 0) {
        gPxyModelRenderStateStorage.renderStatus |= 4;
    }
}

/* The light set copy pxySetLightCopy keeps (X2 pxySetLightCopy); the old setter still passes an int. */
extern XglLightSet *pxyLightCopy;

void nmlModelSetStoredFlag(int storedFlag) {
    pxyLightCopy = (XglLightSet *) storedFlag;
}

void *xglPacketReserve(int count);

/* Copies the light set into the packet and keeps the copy's address (X2 pxySetLightCopy). */
void pxySetLightCopy(XglLightSet *light)
{
    XglLightSet *copy;

    copy = xglPacketReserve(0x19);
    pxyLightCopy = copy;
    *copy = *light;
}

void nmlModelSetFlagBit40(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x40;
}

void nmlModelSetStencilAlpha(long long alpha)
{
    unsigned int renderFlags;

    renderFlags = gPxyModelRenderStateStorage.renderStatus | 0x80;
    gPxyModelRenderStateStorage.stencil_alpha = alpha;
    gPxyModelRenderStateStorage.stencil_mode = 0x42;
    gPxyModelRenderStateStorage.renderStatus = renderFlags;
}

void nmlModelSetFlagBit80000(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x80000;
}

void nmlModelSetFlagBit200000(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x200000;
}

extern int nmlModelStoredValue;

void nmlModelSetStoredValue(int storedValue) {
    nmlModelStoredValue = storedValue;
}
