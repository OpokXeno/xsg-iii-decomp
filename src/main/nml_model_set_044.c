#include "common.h"
#include "shared.h"

/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */

#include "main/tu141.h"

/* The .bss object at 0x008C28B0 (splat name D_008C28B0). Only the two fields this TU touches are named;
 * the flag word at +0x300 is or-ed with 0x00200000, the vector at +0x4C0 is copied from the caller. */
typedef struct NmlModelSetBlock {
    unsigned char unmodeled_000[0x300];
    unsigned int flags; /* +0x300 */
    unsigned char unmodeled_304[0x4C0 - 0x304];
    Vector4 position; /* +0x4C0 */
} NmlModelSetBlock;

void pxySetLightPtr(XglLightSet *light);
extern XglLightSet *pxyCurrentLight;
extern void *D_008C2BBC;
extern NmlModelSetBlock D_008C28B0;

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", nmlModelFogPara);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F008);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F048);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F138);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F178);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F1B0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F1C0);

void func_0019F1F0(void *value)
{
    D_008C2BBC = value;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F200);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F210);

void pxySetModelPosition(Vector4 *value)
{
    D_008C28B0.position = *value;
}

void pxySetLightPtr(XglLightSet *light)
{
  pxyCurrentLight = light;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F308);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F338);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F3A0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F3C0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F408);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F418);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F458);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F478);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F488);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F4F0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F508);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F530);

void func_0019F550(void)
{
    D_008C28B0.flags |= 0x200000;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_044", func_0019F570);
