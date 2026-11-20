#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264CB0);

#include "shared.h"

float CheckDist2DXY(const Vector4 *first, const Vector4 *second)
{
    return __builtin_sqrtf((second->x - first->x) * (second->x - first->x)
                           + (second->y - first->y) * (second->y - first->y));
}

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264D30);

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264D58);

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264D80);

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264DD8);

INCLUDE_ASM("asm/main/nonmatchings/distance", func_00264E30);
