/*
 * OV02 original TU 19: 0x00a88e28..0x00a89128 (6 functions)
 */
#include "common.h"

typedef int s32;
typedef signed char s8;
typedef short s16;
typedef unsigned char u8;

typedef struct TargetingInfo {
    u8 unk_00[0x3E];
    s16 lookupKey;
    u8 unk_40[0x32];
    s16 targetId;
} TargetingInfo;

typedef struct TargetingUnit {
    u8 unk_000[0x2B0];
    TargetingInfo *targetingInfo;
} TargetingUnit;

extern unsigned char gTargetingSlotTable[];
extern s32 D_00ABFA68;
s32 func_00A89768();
s32 memset();

void targetingSlotTablesReset(void) {
    func_00A89768();
    memset(&gTargetingSlotTable, 0, 0x78);
    memset(&D_00ABFA68, 0, 8);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu019", func_00A88E68);

INCLUDE_ASM("asm/nonmatchings/ov02/tu019", func_00A88E78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu019", func_00A88F60);

s32 func_00A88E68();

void slotEntryClearIfFlag(TargetingUnit *targetingUnit) {
    if ((targetingUnit->targetingInfo->lookupKey < 0x12) && (func_00A88E68() != 0)) {
        memset(gTargetingSlotTable + (targetingUnit->targetingInfo->targetId * 0x28), 0, 0x28);
    }
}

void slotEntryClearUnlessFlag(TargetingUnit *targetingUnit) {
    if ((targetingUnit->targetingInfo->lookupKey < 0x12) && (func_00A88E68() == 0)) {
        memset(gTargetingSlotTable + (targetingUnit->targetingInfo->targetId * 0x28), 0, 0x28);
    }
}
