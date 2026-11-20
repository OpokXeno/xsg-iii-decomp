/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_OV02_TU012_H
#define INCLUDE_OV02_TU012_H

/* Types owned by ov02/tu012 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One 0x18-byte item of a lookup block. dataLookupAppend clears the word at +4 with an int store, which is why it is
 * a union there. */
struct DataLookupItem {
    int command;                            /* +0x00 */
    union {
        void (*nextStep)(int context);
        int word;
    } next;                                 /* +0x04 */
    int unmodeled_08[4];
};

/* Lookup block (reached through DataTarget.lookup in battle_init.c, and as the command queue at +0x10 of an BattleUnit in unit_cmd.c):
 * up to DATA_LOOKUP_CAPACITY 0x18-byte items, the read cursor and the index of the last item (-1 when empty). */
struct DataLookup {
    struct DataLookupItem items[12];         /* +0x00: DATA_LOOKUP_CAPACITY */
    int cursor;                             /* +0x120 */
    int lastIndex;                          /* +0x124 */
    int flag;                               /* +0x128 */
};

#endif
