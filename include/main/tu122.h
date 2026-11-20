#ifndef INCLUDE_MAIN_TU122_H
#define INCLUDE_MAIN_TU122_H

/* Types owned by main/tu122 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* A key sequence: the frame counter (fractional, in thousandths), the key rows and the row it has reached. */
typedef struct SdvSequence {
    float elapsed;          /* +0x00 frames since the sequence started */
    short *keys;            /* +0x04 */
    signed char position;   /* +0x08 current row */
    signed char stride;     /* +0x09 halfwords per row */
} SdvSequence;

#endif
