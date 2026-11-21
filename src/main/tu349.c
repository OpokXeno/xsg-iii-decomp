#include "common.h"

typedef unsigned char u8;
typedef int s32;

/* Per-index flag table: one flag byte per index, then two word arrays of eight entries. */
typedef struct MapFlagTable {
    u8 indexBits[0x30];                     /* +0x00, bit 0 and bit 1 are set or cleared by the functions below */
    s32 unk_30[8];                      /* +0x30, cleared by mapFlagTableReset */
    s32 unk_50[8];                      /* +0x50, set to -1 by mapFlagTableReset */
} MapFlagTable;

extern unsigned char gMapFlagTableStorage[];
#define gMapFlagTable (*(MapFlagTable *) gMapFlagTableStorage)

void mapFlagTableReset(void)
{
    s32 mapFlagIndex;
    mapFlagIndex = 0;
    do
    {
        gMapFlagTable.indexBits[mapFlagIndex] = 0;
        gMapFlagTable.unk_30[mapFlagIndex] = 0;
        gMapFlagTable.unk_50[mapFlagIndex] = -1;
        mapFlagIndex += 1;
    }
    while (mapFlagIndex < 8);
}

void mapFlagSetBit0(s32 mapFlagIndex, s32 bitIsSet) {
    if (bitIsSet == 0) {
        gMapFlagTable.indexBits[mapFlagIndex] &= 0xFE;
        return;
    }
    gMapFlagTable.indexBits[mapFlagIndex] |= 1;
}

void mapFlagSetBit1(s32 mapFlagIndex, s32 bitIsSet) {
    if (bitIsSet == 0) {
        gMapFlagTable.indexBits[mapFlagIndex] &= 0xFD;
        return;
    }
    gMapFlagTable.indexBits[mapFlagIndex] |= 2;
}

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5308);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5518);

s32 mapFlagTestBit0(s32 mapFlagIndex) {
    return gMapFlagTable.indexBits[mapFlagIndex] & 1;
}

s32 mapFlagTestBit1(s32 mapFlagIndex) {
    return gMapFlagTable.indexBits[mapFlagIndex] & 2;
}

s32 mapFlagTestBit2(s32 mapFlagIndex) {
    return gMapFlagTable.indexBits[mapFlagIndex] & 4;
}

s32 mapFlagTestBit3(s32 mapFlagIndex) {
    return gMapFlagTable.indexBits[mapFlagIndex] & 8;
}

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5618);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5668);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A56B8);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5880);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5A30);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5AF8);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5B40);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5C88);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5C90);

INCLUDE_ASM("asm/main/nonmatchings/tu349", func_002A5D40);
