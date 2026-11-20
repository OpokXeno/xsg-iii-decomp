#include "common.h"
#include "shared.h"

void testWindowSetColumnTable(int, void *);

extern unsigned char heapOverviewMenuColumnTable[];

void testOverlayMenuOpen(int, int, int, int, int);

extern unsigned char heapOverviewMenuColumnTable[160];

void heapOverviewMenuAttach(int value)
{
    testWindowSetColumnTable(value, heapOverviewMenuColumnTable);
}

void testOverlayEntryCall(int value)
{
    testOverlayMenuOpen(value, 0, 0, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu051", func_001B1BE0);

INCLUDE_ASM("asm/main/nonmatchings/tu051", func_001B1CF8);

INCLUDE_ASM("asm/main/nonmatchings/tu051", func_001B1D28);

INCLUDE_ASM("asm/main/nonmatchings/tu051", func_001B2200);
