#include "common.h"
#include "shared.h"

/* Row-major 4x4 float matrix. */

#include "main/xgl_2.h"


union Matrix;

void pxySetMotionGlobalMatrices(union Matrix *global);

extern Matrix *pxyMotionGlobalMatrices;

void pxySetMotionGlobalMatrices(Matrix *global)
{
  pxyMotionGlobalMatrices = global;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019EEF8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019EF08);
