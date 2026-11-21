/*
 * OV02 original TU 86: 0x00ab06c8..0x00ab0868 (5 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned short u16;
typedef unsigned char u8;

/* Slot record returned by ov02BattleSlotGet (0x28 bytes per slot in ov02/tu009). */
struct BattleStatEntry {
    u8 unk_00[4];
    u16 readyFlags;
    u8 battleStatFlags;
};

/* Sort key read by entryCompareKey: a float at +4 of the entry. */
struct SortKeyEntry {
    s32 unk_00;
    f32 key;
};

struct BattleStatEntry *ov02BattleSlotGet();
s32 func_00A86530(s32);

s32 entryCompareKey(const struct SortKeyEntry *left, const struct SortKeyEntry *right) {
    f32 leftValue;
    f32 rightValue;

    leftValue = left->key;
    rightValue = right->key;
    if (leftValue < rightValue) {
        return -1;
    }
    if (rightValue < leftValue) {
        return 1;
    }
    return 0;
}

s32 slotGetReadyValue(s32 id, s32 *readyValue) {
    s32 isReady;

    isReady = (ov02BattleSlotGet()->readyFlags & 0x12) == 0x12;
    if ((isReady != 0) && (readyValue != 0)) {
        *readyValue = func_00A86530(id);
    }
    return isReady;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu086", func_00AB0768);

INCLUDE_ASM("asm/nonmatchings/ov02/tu086", func_00AB07C0);

s32 battleActorStoreAndRefresh(s32, s32);
void unitSetCommandTarget();

void battleUnitSetStatValue(s32 battleUnit, s32 battleStatIndex, s32 battleStatValue) {
    unitSetCommandTarget();
    if (ov02BattleSlotGet(battleStatIndex)->battleStatFlags & 2) {
        battleActorStoreAndRefresh(battleUnit, battleStatValue);
    }
}
