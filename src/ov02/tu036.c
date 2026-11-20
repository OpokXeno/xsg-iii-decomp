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
    int count;
    int i;

    count = unitTblGet(2, &units);
    for (i = 0; i < count; i++) {
        if (units[i] != 0) {
            units[i]->lightActor->light = *light;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu036", func_00A8D7B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu036", func_00A8D848);

INCLUDE_ASM("asm/nonmatchings/ov02/tu036", func_00A8D858);
