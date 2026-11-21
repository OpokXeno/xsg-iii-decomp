#include "common.h"

typedef int s32;
extern s32 gSbNativeValueSlotIndex;
extern s32 D_00551408;
extern s32 gSbNativeValueSlots[];
void *memset(void *s, int c, unsigned int n);

void sbNativeValueSlotReset(void) {
    gSbNativeValueSlotIndex = 0;
    D_00551408 = 0;
    memset(gSbNativeValueSlots, 0, 0x20);
}

void sbNativeValueSlotSet(s32 gameStateFn, s32 extra) {
    D_00551408 = extra;
    gSbNativeValueSlots[gSbNativeValueSlotIndex] = gameStateFn;
}

INCLUDE_ASM("asm/main/nonmatchings/tu098", func_001CE758);

INCLUDE_ASM("asm/main/nonmatchings/tu098", func_001CE7B8);

INCLUDE_ASM("asm/main/nonmatchings/tu098", func_001CE7F8);
