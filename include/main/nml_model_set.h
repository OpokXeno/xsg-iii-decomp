/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_NML_MODEL_SET_H
#define INCLUDE_MAIN_NML_MODEL_SET_H

#include "shared.h"
#include "main/xgl_2.h"

/* The .bss object at 0x008C28B0 (splat name gPxyModelRenderStateStorage). Only the fields the model-set TUs touch are named;
 * the flag word at +0x300 is or-ed with the nmlModelSet* bits below. */
typedef struct PxyModelRenderState {
    unsigned char pad_000[0x300];
    unsigned int renderStatus; /* +0x300 */
    unsigned char pad_304[0x350 - 0x304];
    f32 transparency[2]; /* +0x350 and +0x354, both set to the clamped transparency */
    unsigned char pad_358[0x380 - 0x358];
    long long stencil_alpha; /* +0x380 */
    long long stencil_mode; /* +0x388, set to 0x42 */
    unsigned char pad_390[0x398 - 0x390];
    f32 modelScaleMax; /* +0x398: the largest of modelScale x, y, z */
    unsigned char pad_39C[0x3E4 - 0x39C];
    int texFlags; /* +0x3E4: PXY_TEX_PROREAL is set by func_0019E7F0 (Episode I nmlModelSetTexProreal) */
    unsigned char pad_3E8[0x470 - 0x3E8];
    Vector4 modelScale; /* +0x470 */
    unsigned char pad_480[0x4A0 - 0x480];
    Vector4Elements fogColor; /* +0x4A0: set by nmlModelSetFogCol (X2 nmlModelSetFogCol) */
    unsigned char fog_near[0x10]; /* +0x4B0: the fog parameters of nmlModelSetFogDist (gNmlModelFogNearParams) */
    Vector4 position; /* +0x4C0 */
    unsigned char pad_4D0[0x698 - 0x4D0];
    unsigned int drawFlags; /* +0x698, or-ed with 0x20 in nmlModelSetFogFarDist */
    unsigned char pad_69C[0x750 - 0x69C];
    Vector4Elements globalFogColor; /* +0x750: set by nmlModelSetGlobalFogCol (X2 nmlModelSetGlobalFogCol) */
    unsigned char fog_far[0x10]; /* +0x760: the fog parameters of nmlModelSetFogFarDist (gNmlModelFogFarParams) */
} PxyModelRenderState;

extern PxyModelRenderState gPxyModelRenderStateStorage;

#endif
