/*
 * OV02 original TU 50: 0x00a93ea0..0x00a947e8 (14 functions)
 */
#include "common.h"

struct BattleUnit;

#include "ov02/tu060.h"

struct UnitBitTable {
    int value;                          /* +0x00 */
    unsigned char unmodeled_004[0x8C];
    signed char byBit[2][32];           /* +0x90 */
};


int unitTblGet(int side, BattleUnit ***table);
int func_00A93EA8(unsigned int mask);
int calcStatGet(BattleUnit *unit, int row, int flag);
void func_00A940E8(BattleUnit *unit, int row, int flag);
void func_00A81868(BattleUnit *unit, int delta);
void func_00A91D08(BattleUnit *unit, int delta, int kind);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93EA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93EA8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93EF0);

int calcStatGet(BattleUnit *unit, int row, int flag)
{
    int bitIndex;

    if (row >= 2) {
        return 0;
    }
    bitIndex = func_00A93EA8(flag);
    return unit->bitTable->byBit[row][bitIndex];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93F58);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A93FE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A940E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A94178);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A94190);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A94218);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A942F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A94380);

INCLUDE_ASM("asm/nonmatchings/ov02/tu050", func_00A943F8);

void func_00A946B0(void)
{
    BattleUnit **units;
    BattleUnit *unit;
    int count;
    int i;
    int delta;

    count = unitTblGet(2, &units);
    for (i = 0; i < count; i++) {
        delta = 0;
        unit = units[i];
        if (unit == 0) {
            continue;
        }
        if (unit->flags & 0x8000) {
            if (calcStatGet(unit, 1, 2) != 0) {
                delta = -unit->bitTable->value;
                func_00A940E8(unit, 1, 2);
            } else if (calcStatGet(unit, 1, 1) != 0) {
                delta = unit->bitTable->value / 2;
                delta = -delta;
                func_00A940E8(unit, 1, 1);
            }
        }
        if (delta != 0) {
            unit->flags = (unit->flags & 0xffff7fff) | 1;
            func_00A81868(unit, -delta);
            func_00A91D08(unit, delta, 3);
        }
    }
}
