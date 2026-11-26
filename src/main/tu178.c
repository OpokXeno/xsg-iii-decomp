#include "common.h"
#include "main/tu178.h"

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_00224600);

typedef int s32;

void *memset(void *destination, int value, unsigned int count);

void xglMcAgedEntriesReset(void) {
    gXglMcAgedEntries.ringWriteIndex = 0;
    gXglMcAgedEntries.ringCursor = 0;
    memset(gXglMcAgedEntries.agedEntries, 0, 0x64);
}

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_002246A0);

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_002246E8);

void *xglMcTableGetAgedEntry(s32 age) {
    s32 ringIndex;

    ringIndex = (gXglMcAgedEntries.ringCursor - age) + 0x32;
    return &gXglMcAgedEntries.agedEntries[ringIndex % 50];
}

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_00224958);

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_00224A80);

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_00225108);
