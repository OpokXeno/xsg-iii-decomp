#include "common.h"
#include "main/xgl_cd.h"

INCLUDE_ASM("asm/main/nonmatchings/tu178", func_00224600);

typedef signed char s8;

typedef int s32;

typedef unsigned char u8;

/* The 0x130B0-byte memory-card save image at gXglMcAgedEntries (same definition as in tu176). */
typedef struct AgedEntryRing {
    unsigned char unmodeled_0000[0x10];
    s32 newGameCounters[2];                     /* +0x10: zeroed by the new-game reset */
    XglClock newGameClock;                      /* +0x18: clock read when a new game starts */
    unsigned char unmodeled_0020[0x4A - 0x20];
    unsigned char flag4A;                       /* +0x4A */
    unsigned char flag4B;                       /* +0x4B */
    unsigned char unmodeled_004C[0x1330 - 0x4C];
    unsigned char bytes1330[0x1408 - 0x1330];   /* +0x1330 */
    unsigned short agedEntries[50];             /* +0x1408 */
    unsigned char unmodeled_146C[0x146D - 0x146C];
    unsigned char bytes146D[0x147C - 0x146D];   /* +0x146D */
    unsigned char bytes147C[0x157C - 0x147C];   /* +0x147C */
    unsigned char bytes157C[0x177C - 0x157C];   /* +0x157C */
    unsigned char unmodeled_177C[0x3A0C - 0x177C];
    unsigned char bytes3A0C[0x1302C - 0x3A0C];  /* +0x3A0C */
    unsigned char unmodeled_1302C[0x130AA - 0x1302C];
    s8 ringWriteIndex;                          /* +0x130AA: slot of agedEntries the next entry goes to */
    u8 ringCursor;                              /* +0x130AB: slot of the newest entry */
    unsigned char unmodeled_130AC[0x130B0 - 0x130AC];
} AgedEntryRing;

extern AgedEntryRing gXglMcAgedEntries;

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
