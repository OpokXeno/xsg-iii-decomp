/* header_harvest: hand-maintained (owner header of main/tu178; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU178_H
#define INCLUDE_MAIN_TU178_H

/* Types owned by main/tu178 and shared with other TUs. */

#include "main/xgl_cd.h"

/* The 0x130B0-byte memory-card save image at gXglMcAgedEntries. The newGameCleared* members are the bytes and regions
   newGameStateReset (tu176) zeroes; nothing else about them is known yet. */
typedef struct AgedEntryRing {
    unsigned char unmodeled_0000[0x10];
    int newGameCounters[2];                                           /* +0x10: zeroed by the new-game reset */
    XglClock newGameClock;                                            /* +0x18: clock read when a new game starts */
    unsigned char unmodeled_0020[0x4A - 0x20];
    unsigned char newGameClearedFlag4A;                               /* +0x4A */
    unsigned char newGameClearedFlag4B;                               /* +0x4B */
    unsigned char unmodeled_004C[0x1330 - 0x4C];
    unsigned char newGameClearedRegion1330[0x1408 - 0x1330];          /* +0x1330 */
    unsigned short agedEntries[50];                                   /* +0x1408 */
    unsigned char unmodeled_146C[0x146D - 0x146C];
    unsigned char newGameClearedRegion146D[0x147C - 0x146D];          /* +0x146D */
    unsigned char newGameClearedRegion147C[0x157C - 0x147C];          /* +0x147C */
    unsigned char newGameClearedRegion157C[0x177C - 0x157C];          /* +0x157C */
    unsigned char unmodeled_177C[0x3A0C - 0x177C];
    unsigned char newGameClearedRegion3A0C[0x1302C - 0x3A0C];         /* +0x3A0C */
    unsigned char unmodeled_1302C[0x130AA - 0x1302C];
    signed char ringWriteIndex;                                       /* +0x130AA: slot the next entry goes to */
    unsigned char ringCursor;                                         /* +0x130AB: slot of the newest entry */
    unsigned char unmodeled_130AC[0x130B0 - 0x130AC];
} AgedEntryRing;

extern AgedEntryRing gXglMcAgedEntries;

#endif /* INCLUDE_MAIN_TU178_H */
