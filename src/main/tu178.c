#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_00224600);

typedef signed char s8;

typedef int s32;

typedef unsigned char u8;

typedef struct AgedEntryRing {
    unsigned char pad_0000[0x1408];
    unsigned short agedEntries[50];
    unsigned char padBeforeTrailer[0x130AA - 0x146C];
    s8 field130AA;
    u8 ringCursor;
} AgedEntryRing;

extern AgedEntryRing gXglMcAgedEntries;

void *memset(void *destination, int value, unsigned int count);

void xglMcAgedEntriesReset(void) {
    gXglMcAgedEntries.field130AA = 0;
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
