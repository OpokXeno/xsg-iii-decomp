/*
 * OV02 original TU 2: 0x00a81920..0x00a81b48 (8 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu002", func_00A81920);

INCLUDE_ASM("asm/nonmatchings/ov02/tu002", func_00A819D8);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
s32 func_00A92900();

/* The entry record of this TU group (the offsets are read from the original code): costKind at +0x07, baseCost at +0x12. */
typedef struct EntryCostRecord {
    u8 unk_00[0x07];
    u8 costKind;                /* +0x07 */
    u8 unk_08[0x12 - 0x08];
    u8 baseCost;                /* +0x12 */
} EntryCostRecord;

/* The object of battleEntryClearFlag2000; its battleFlags word is at +0x0C. */
typedef struct BattleEntryFlags {
    u8 unk_00[0x0C];
    s32 battleFlags;            /* +0x0C */
} BattleEntryFlags;

s8 entryCostGet(s32 id, EntryCostRecord *costEntry) {
    s32 halvedCost;
    s8 entryCost;
    u32 valueSignBits;
    u32 costKindIndex;
    u8 baseCost;

    baseCost = costEntry->baseCost;
    costKindIndex = costEntry->costKind - 1;
    valueSignBits = baseCost << 0x18;
    entryCost = baseCost;
    if (costKindIndex < 2U) {
        if (func_00A92900(id, 0xA, 1, 0) != 0) {
            halvedCost = entryCost + (valueSignBits >> 0x1F);
            entryCost = halvedCost >> 1;
        }
        if (func_00A92900(id, 0xA, 2, 0) != 0) {
            entryCost = 1;
        }
    }
    return entryCost;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu002", func_00A81AB0);

void battleEntryClearFlag2000(BattleEntryFlags *battleEntry) {
    battleEntry->battleFlags &= ~0x2000;
}

s32 func_00A94218();

void battleStateReset(void) {
    func_00A94218();
}

extern s32 gBattleSessionState;
s32 memset(s32 *, s32, s32);

void battleRecordBlockClear(void) {
    memset(&gBattleSessionState, 0, 0x430);
}

s32 *battleRecordBlockGet(void) {
    return &gBattleSessionState;
}
