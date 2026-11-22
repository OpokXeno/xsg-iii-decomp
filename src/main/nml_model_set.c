#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

#include "main/xgl_2.h"


extern Matrix *pxyMotionGlobalMatrices;

void pxySetMotionGlobalMatrices(Matrix *motionMatrices)
{
  pxyMotionGlobalMatrices = motionMatrices;
}

typedef int s32;
extern s32 D_008C2C14;

void nmlModelSetStoreValue(s32 storedValue) {
    D_008C2C14 = storedValue;
}

/* The two fields of the model render state (gPxyModelRenderStateStorage, nml_model_set_044.c) this TU writes. */
typedef struct PxyModelFogColorView {
    unsigned char pad_000[0x300];
    unsigned int renderStatus; /* +0x300 */
    unsigned char pad_304[0x4A0 - 0x304];
    Vector4Elements fogColor; /* +0x4A0 */
} PxyModelFogColorView;

extern PxyModelFogColorView gPxyModelRenderStateStorage;

void func_0019EF08(Vector4 *color)
{
    gPxyModelRenderStateStorage.fogColor.vector = *color;
    gPxyModelRenderStateStorage.renderStatus |= 1;
}
