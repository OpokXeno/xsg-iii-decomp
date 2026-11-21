/*
 * OV02 original TU 36: 0x00a8d710..0x00a8d900 (4 functions)
 */
#include "common.h"
#include "shared.h"


#include "main/tu141.h"
#include "ov02/tu060.h"



int unitTblGet(int side, BattleUnit ***table);

/* Copies the light block into the actor of every unit of unitTblGet(2). */
void unitTblBodySetLight(XglLightSet *light)
{
    BattleUnit **units;
    int sideTwoUnitCount;
    int sideTwoUnitIndex;

    sideTwoUnitCount = unitTblGet(2, &units);
    for (sideTwoUnitIndex = 0; sideTwoUnitIndex < sideTwoUnitCount; sideTwoUnitIndex++) {
        if (units[sideTwoUnitIndex] != 0) {
            units[sideTwoUnitIndex]->lightActor->light = *light;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu036", func_00A8D7B8);

typedef int s32;
extern f32 unitTblCachedFloat;

f32 unitTblGetCachedFloat(void) {
    return unitTblCachedFloat;
}

void monsColorStore(void *src);
s32 vmArgGet(s32 vm, s32 index);

/* Byte-to-unit colour scale: 1 / 255. */
#define COLOR_BYTE_SCALE 0.003921569f

/* A colour as four unit floats, in the order the colour setter reads them. */
typedef struct Color4f {
    f32 red;
    f32 green;
    f32 blue;
    f32 alpha;
} Color4f;

/* Takes three colour bytes from script arguments 1..3, alpha is 1. */
s32 sbNativeSetColor(s32 vm) {
    Color4f color;
    f32 blueByte;

    color.red = vmArgGet(vm, 1) * COLOR_BYTE_SCALE;
    color.green = vmArgGet(vm, 2) * COLOR_BYTE_SCALE;
    blueByte = vmArgGet(vm, 3);
    color.alpha = 1.0f;
    color.blue = blueByte * COLOR_BYTE_SCALE;
    monsColorStore(&color);
    return 1;
}
