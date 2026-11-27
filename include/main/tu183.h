/* header_harvest: hand-maintained (owner header of main/tu183; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU183_H
#define INCLUDE_MAIN_TU183_H

/* Types owned by main/tu183 and shared with other TUs. */

#include "shared.h"

/* Header returned by getStatusHeader(); flags at +2. */
typedef struct StatusHeader {
    u8 unk00[2];
    u16 headerFlags;
} StatusHeader;

#endif /* INCLUDE_MAIN_TU183_H */
