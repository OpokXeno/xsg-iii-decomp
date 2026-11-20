/*
 * OV02 original TU 1: 0x00a81558..0x00a81920 (12 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A81558);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A815B0);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A815D8);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A81600);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A81618);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A81740);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A81770);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A817C8);

/* Boost gauge of one battle unit: maximum stock, stock and progress toward the next stock. */
struct BoostGauge {
    signed char maxStock;                   /* +0x00 */
    signed char stock;                      /* +0x01 */
    signed char progress;                   /* +0x02 */
};

int func_00A81770(struct BoostGauge *gauge, int amount);

void boostGaugeAddProgress(struct BoostGauge *gauge, int amount)
{
    int nextProgress;

    nextProgress = gauge->progress + amount;
    if (nextProgress < 0) {
        nextProgress = 0;
    } else if (nextProgress >= 100) {
        func_00A81770(gauge, 1);
        nextProgress -= 100;
    }
    if (gauge->stock >= gauge->maxStock) {
        nextProgress = 0;
    }
    gauge->progress = nextProgress;
}

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A81868);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A818A0);

INCLUDE_ASM("asm/nonmatchings/ov02/boost_gauge", func_00A818D0);
