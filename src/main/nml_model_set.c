#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

#include "main/xgl_2.h"
#include "main/nml_model_set.h"


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

void func_0019EF08(Vector4 *color)
{
    gPxyModelRenderStateStorage.fogColor.vector = *color;
    gPxyModelRenderStateStorage.renderStatus |= 1;
}
