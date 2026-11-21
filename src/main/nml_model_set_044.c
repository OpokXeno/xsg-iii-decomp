#include "common.h"
#include "shared.h"

/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */

#include "main/tu141.h"

/* The .bss object at 0x008C28B0 (splat name gPxyModelRenderStateStorage). Only the fields this TU touches are named;
 * the flag word at +0x300 is or-ed with the nmlModelSet* bits below. */
typedef struct PxyModelRenderState {
    unsigned char unmodeled_000[0x300];
    unsigned int renderStatus; /* +0x300 */
    unsigned char unmodeled_304[0x350 - 0x304];
    f32 transparency[2]; /* +0x350 and +0x354, both set to the clamped transparency */
    unsigned char unmodeled_358[0x380 - 0x358];
    long long stencil_alpha; /* +0x380 */
    long long stencil_mode; /* +0x388, set to 0x42 */
    unsigned char unmodeled_390[0x4B0 - 0x390];
    unsigned char fog_near[0x10]; /* +0x4B0: the fog parameters of nmlModelSetFogDist (gNmlModelFogNearParams) */
    Vector4 position; /* +0x4C0 */
    unsigned char unmodeled_4D0[0x698 - 0x4D0];
    unsigned int drawFlags; /* +0x698, or-ed with 0x20 in nmlModelSetFogFarDist */
    unsigned char unmodeled_69C[0x760 - 0x69C];
    unsigned char fog_far[0x10]; /* +0x760: the fog parameters of nmlModelSetFogFarDist (gNmlModelFogFarParams) */
} PxyModelRenderState;

/* The block that contains the member `member` stored at `field` (the fog parameters are reached by their own symbols). */
#define MEMBER_OFFSET(type, member) ((int) &((type *) 0)->member)
#define BLOCK_OF_MEMBER(field, member) ((PxyModelRenderState *) ((field) - MEMBER_OFFSET(PxyModelRenderState, member)))

void pxySetLightPtr(XglLightSet *light);
extern XglLightSet *pxyCurrentLight;
extern void *gNmlModelSetCurrent;
extern PxyModelRenderState gPxyModelRenderStateStorage;

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", nmlModelFogPara);

extern unsigned char gNmlModelFogNearParams[];
void nmlModelFogPara(u8 *, f32, f32, f32, f32);

void nmlModelSetFogDist(f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    PxyModelRenderState *modelSet;

    modelSet = BLOCK_OF_MEMBER(gNmlModelFogNearParams, fog_near);
    nmlModelFogPara(gNmlModelFogNearParams, fogNear, fogFar, fogMin, fogMax);
    modelSet->renderStatus |= 1;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F048);

extern unsigned char gNmlModelFogFarParams[];

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F210);

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

extern int D_008C2BC0;

void nmlModelSetStoredMode(int storedMode) {
    D_008C2BC0 = storedMode;
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

extern int D_008C2C24;

void nmlModelSetStoredFlag(int storedFlag) {
    D_008C2C24 = storedFlag;
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

extern int D_008C2BB4;

void nmlModelSetStoredValue(int storedValue) {
    D_008C2BB4 = storedValue;
}
