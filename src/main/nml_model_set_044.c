#include "common.h"
#include "shared.h"
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
#include "main/tu141.h"

/* The .bss object at 0x008C28B0 (splat name gPxyModelRenderStateStorage). Only the fields this TU touches are named;
 * the flag word at +0x300 is or-ed with the nmlModelSet* bits below. */
typedef struct PxyModelRenderState {
    unsigned char pad_000[0x300];
    unsigned int renderStatus; /* +0x300 */
    unsigned char pad_304[0x350 - 0x304];
    f32 transparency[2]; /* +0x350 and +0x354, both set to the clamped transparency */
    unsigned char pad_358[0x380 - 0x358];
    long long stencil_alpha; /* +0x380 */
    long long stencil_mode; /* +0x388, set to 0x42 */
    unsigned char pad_390[0x4B0 - 0x390];
    unsigned char fog_near[0x10]; /* +0x4B0: the fog parameters of nmlModelSetFogDist (gNmlModelFogNearParams) */
    Vector4 position; /* +0x4C0 */
    unsigned char pad_4D0[0x698 - 0x4D0];
    unsigned int drawFlags; /* +0x698, or-ed with 0x20 in nmlModelSetFogFarDist */
    unsigned char pad_69C[0x760 - 0x69C];
    unsigned char fog_far[0x10]; /* +0x760: the fog parameters of nmlModelSetFogFarDist (gNmlModelFogFarParams) */
} PxyModelRenderState;

/* The block that contains the member `member` stored at `field` (the fog parameters are reached by their own symbols). */
#define MEMBER_OFFSET(type, member) ((int) &((type *) 0)->member)
#define BLOCK_OF_MEMBER(field, member) ((PxyModelRenderState *) ((u8 *) (field) - MEMBER_OFFSET(PxyModelRenderState, member)))

void pxySetLightPtr(XglLightSet *light);
extern XglLightSet *pxyCurrentLight;
extern void *gNmlModelSetCurrent;
extern PxyModelRenderState gPxyModelRenderStateStorage;
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
#include "main/xgl_2.h"

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F178);

extern f32 gNmlModelSortOffset;

void nmlModelSetSortOffset(f32 sortOffset) {
    gNmlModelSortOffset = sortOffset;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F1C0);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F308);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F338);

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

extern int nmlModelStoredFlag;

void nmlModelSetStoredFlag(int storedFlag) {
    nmlModelStoredFlag = storedFlag;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F488);

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
