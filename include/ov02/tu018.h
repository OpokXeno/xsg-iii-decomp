/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_OV02_TU018_H
#define INCLUDE_OV02_TU018_H

/* Types owned by ov02/tu018 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/xenoActor.h"

/*
 * One MonsActorRecord record. The flags word uses BATTLE_SLOT_* (battle_slot.h). monsterClaimInit stores its argument in the word at
 * +0x10, clears the words at +0x4, +0x8 and +0xC, and calls func_00A88C58, which stores the new actor at +0x14. The
 * update and draw callbacks sit at +0x4 and +0x8, so those words have two views (a union where both are used).
 */
typedef struct MonsActorRecord MonsActorRecord;

/*
 * One MonsActorRecord record. The flags word uses BATTLE_SLOT_* (battle_slot.h). monsterClaimInit stores its argument in the word at
 * +0x10, clears the words at +0x4, +0x8 and +0xC, and calls func_00A88C58, which stores the new actor at +0x14. The
 * update and draw callbacks sit at +0x4 and +0x8, so those words have two views (a union where both are used).
 */
struct MonsActorRecord {
    int flags;
    union {
        int (*update)(struct MonsActorRecord *slot); /* +0x4: update sweep */
        void (*callback)(void);               /* +0x4: monsterClaimInit stores the empty callback */
    };
    union {
        int (*draw)(struct MonsActorRecord *slot);   /* +0x8: draw sweep */
        unsigned int initWord08;              /* +0x8: monsterClaimInit clears it */
    };
    unsigned int initWord0C; /* +0xC: monsterClaimInit clears it */
    void *initArgument; /* +0x10: monsterClaimInit stores its argument */
    struct XenoActor *actor; /* +0x14: the actor the record drives (func_00A88C58 stores it) */
};

#endif
