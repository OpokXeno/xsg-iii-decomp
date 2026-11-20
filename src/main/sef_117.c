#include "common.h"
#include "shared.h"

void sefSetFogOverride(Vector4 *vectorA, Vector4 *vectorB);

#include "quadword.h"
#include "vu0_macro.h"

/* A word set when either override vector was supplied; sefGetFogOverrideBlock hands out its address. */
extern unsigned char sefOverrideFlag[16];

extern AlignedVector4 sefOverrideVectorA;
extern AlignedVector4 sefOverrideVectorB;

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8AF0);

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8BF8);

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8CB8);

/*
 * Copies the two override vectors; a null source resets that vector to the
 * origin (0, 0, 0, 1). The flag records whether any override was given.
 */
void sefSetFogOverride(Vector4 *vectorA, Vector4 *vectorB)
{
    if (vectorA != 0) {
        QuadwordCopyT0(&sefOverrideVectorA, vectorA);
    } else {
        Vu0SetOrigin(&sefOverrideVectorA);
    }
    if (vectorB != 0) {
        QuadwordCopyT0(&sefOverrideVectorB, vectorB);
    } else {
        Vu0SetOrigin(&sefOverrideVectorB);
    }
    *(int *) sefOverrideFlag = vectorA != 0 || vectorB != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8E40);

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8E70);

INCLUDE_ASM("asm/main/nonmatchings/sef_117", func_001E8F78);
