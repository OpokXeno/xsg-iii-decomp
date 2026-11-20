#ifndef INCLUDE_MAIN_TIDYNAMICSCOLL_H
#define INCLUDE_MAIN_TIDYNAMICSCOLL_H

/* Types owned by main/tu334 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One collision entry of a dynamics node (0x10 bytes): the index of a record of the hierarchy's sphere, matrix or
 * capsule table. */
typedef struct TiCollEntry {
    int recordIndex;
    unsigned char unmodeled_04[0xC];
} TiCollEntry;

/* Three floats in x, y, z order, copied as one struct. */
typedef struct TiVector3 {
    float x;
    float y;
    float z;
} TiVector3;

#endif
