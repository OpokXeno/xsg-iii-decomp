/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_OV02_TU060_H
#define INCLUDE_OV02_TU060_H

/* Types owned by ov02/tu060 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/tu141.h"
#include "main/xenoActor.h"
#include "ov02/tu062.h"

/* Partial battle unit: only the actor pointer at +0x2B4 is claimed. */
typedef struct BattleUnit BattleUnit;

/* The part of a pool entry that unitTblSet reads: the lookup key halfword at +0x3E. */
typedef struct UnitPoolEntry UnitPoolEntry;

/* Partial actor: the light block sits at +0x140, the bytes before it are not claimed here. */
typedef struct LightActor {
    unsigned char unmodeled_000[0x140];
    XglLightSet light;                  /* +0x140 */
} LightActor;

/* Partial battle unit: only the actor pointer at +0x2B4 is claimed. */
struct BattleUnit {
    unsigned char unmodeled_00[0xC];    /* +0x00 */
    unsigned int flags;                 /* +0x0C */
    unsigned char commandQueue[0x12C];  /* +0x10: the command queue (a DataLookup block) */
    unsigned char unmodeled_13C[0x2B0 - 0x13C];
    union {                             /* +0x2B0: one pointer, seen through three partial records */
        struct UnitPoolEntry *poolEntry; /* lookupKey at +0x3E */
        struct UnitBitTable *bitTable;   /* byBit[2][32] at +0x90 */
        struct BattleStats *stats;
    };
    union {                             /* +0x2B4: the unit's motion actor */
        struct XenoActor *unit;
        struct LightActor *lightActor;  /* lights at +0x140 of the same actor */
    };
    struct XenoActor *children[4];      /* +0x2B8 */
    unsigned char unmodeled_2C8[0x300 - 0x2C8];
    struct FadeTask *fade;              /* +0x300: the running fade task, 0 when none runs */
};

/* The part of a pool entry that unitTblSet reads: the lookup key halfword at +0x3E. */
struct UnitPoolEntry {
    unsigned char unmodeled_00[0x3E];
    short lookupKey;                    /* +0x3E */
};

#endif
