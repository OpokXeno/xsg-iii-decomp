/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU022_H
#define INCLUDE_MAIN_TU022_H

/* Types owned by main/tu022 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One record of the INTC or DMAC table: the handler id AddIntcHandler or
 * AddDmacHandler returned, the event count the handler bumps, the INTC
 * cause or DMAC channel (a negative id ends the table) and the handler. */
typedef struct XglIntrRecord {
    int handle;
    int eventCount;
    int id;
    int (*handler)(int cause);
} XglIntrRecord;

#endif
