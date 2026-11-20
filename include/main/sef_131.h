#ifndef INCLUDE_MAIN_SEF_131_H
#define INCLUDE_MAIN_SEF_131_H

/* Types owned by main/sef_131 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One 8-byte record of a keyed track: the key is compared with the integer
 * time, 0x1000 ends the track, link is the index of the next key (-1 when the
 * track stops there), value is the sample the key holds. */
typedef struct SefTrackKey {
    short key;
    unsigned short link;
    float value;
} SefTrackKey;

#endif
