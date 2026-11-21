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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019EF08);
