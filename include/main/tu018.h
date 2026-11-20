/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU018_H
#define INCLUDE_MAIN_TU018_H

/* Types owned by main/tu018 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* A volume ramp: the current level moves toward the target by step on each
 * call to seVolumeRampStep. */
typedef struct VolumeRamp {
    signed char step;             /* +0x00: added to current on each call; cleared when the ramp reaches target */
    unsigned char set_level;      /* +0x01: a sound slot's level as soundSlotSetPause last set it */
    unsigned char target;         /* +0x02: level the ramp moves toward */
    unsigned char current;        /* +0x03: level sent to the sound driver */
} VolumeRamp;

#endif
