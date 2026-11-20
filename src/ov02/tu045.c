/*
 * OV02 original TU 45: 0x00a914d0..0x00a92030 (13 functions)
 */
#include "common.h"

struct BattleUnit;

typedef struct BattleUnit BattleUnit;

int unitTblGet(int side, BattleUnit ***table);
int calcStatGet(BattleUnit *unit, int row, int flag);
int func_00A99990(BattleUnit *unit);
void func_00A946B0(void);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A914D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A914E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91520);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91630);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91928);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91A10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91BA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91BC0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91BE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91C18);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91D08);

INCLUDE_ASM("asm/nonmatchings/ov02/tu045", func_00A91E78);

int battleStatusQueueWait(void)
{
    BattleUnit **units;
    BattleUnit *unit;
    int count;
    int i;

    count = unitTblGet(2, &units);
    for (i = 0; i < count; i++) {
        unit = units[i];
        if (unit != 0) {
            if (calcStatGet(unit, 1, 1) != 0 || calcStatGet(unit, 1, 2) != 0) {
                if (func_00A99990(unit) == 0) {
                    return 0;
                }
            }
        }
    }
    func_00A946B0();
    return 1;
}
