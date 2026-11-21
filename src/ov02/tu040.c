/*
 * OV02 original TU 40: 0x00a8e120..0x00a8e830 (10 functions)
 */
#include "common.h"

typedef int s32;
extern s32 bgmNativeTable;
extern s32 bgmModuleName;
void bgmResetState();
s32 sbNativeOwnerAdd(s32 *, s32 *);

void bgmInit(void) {
    sbNativeOwnerAdd(&bgmModuleName, &bgmNativeTable);
    bgmResetState();
}

void bgmInitEntry(void) {
    bgmInit();
}

extern s32 unitTblCachedFloat;
extern s32 bgmIndex;

void bgmResetState(void) {
    unitTblCachedFloat = 0;
    bgmIndex = 0;
}

extern s32 partyRecordTable[];

s32 *partyRecordTableGet(void) {
    return partyRecordTable;
}

extern s32 enemyRecordTable[];

s32 *enemyRecordTableGet(void) {
    return enemyRecordTable;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu040", func_00A8E198);

INCLUDE_ASM("asm/nonmatchings/ov02/tu040", func_00A8E2F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu040", func_00A8E570);

INCLUDE_ASM("asm/nonmatchings/ov02/tu040", func_00A8E628);

INCLUDE_ASM("asm/nonmatchings/ov02/tu040", func_00A8E7D0);
