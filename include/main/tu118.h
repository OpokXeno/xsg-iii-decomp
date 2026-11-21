/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU118_H
#define INCLUDE_MAIN_TU118_H

/* Types owned by main/tu118 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* Sef entry table in .data at 0x4541A0: 24 entries of 0x15A0 bytes, then the entry count at +0x20710. */
typedef struct SefEntry {
    unsigned char unmodeled_000[0xCC0];
    Vector4 slotVectors[64];        /* +0xCC0 */
    unsigned char unmodeled_10C0[0x1550 - 0x10C0];
    int id;                         /* +0x1550 */
    int effectEntryFlags;                      /* +0x1554 */
    unsigned char unmodeled_1558[0x155C - 0x1558];
    signed char slotFlags[0x20];    /* +0x155C: bit 0 = slot set */
    unsigned char unmodeled_157C[0x1592 - 0x157C];
    short unmodeled_1592;           /* +0x1592 */
    unsigned char unmodeled_1594[0x15A0 - 0x1594];
} SefEntry;

/* The entry table of the effect system in .data at 0x4541A0: 24 entries of 0x15A0 bytes, then the entry count at +0x20710. */
typedef struct SefEntryTable {
    SefEntry effectEntries[24];
    unsigned char unmodeled_20700[0x10];
    int effectEntryCount;             /* +0x20710 */
    unsigned char unmodeled_20714[0x0C];
} SefEntryTable;

/* .data word at 0x3E46B0; the object is larger than the -G8 small-data limit, so it is addressed absolutely. */
typedef struct SefFlagBlock {
    int flags;                  /* +0x00 */
    unsigned char unmodeled_04[0x0C];
} SefFlagBlock;

extern SefFlagBlock gGameLoopRuntimeStorage;

/* Clears the status words of D_004C8390; takes no argument. */
void sefResetDrawState(void);

#endif
