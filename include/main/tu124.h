#ifndef INCLUDE_MAIN_TU124_H
#define INCLUDE_MAIN_TU124_H

/* Types owned by main/tu124 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One key track of a camera schedule. */
typedef struct SdvScheduleChannel {
    short keyIndex;                 /* +0x00 */
    unsigned char unmodeled_02[2];
    short *keys;                    /* +0x04 */
} SdvScheduleChannel;

/* A camera schedule (memset to 0x2C by its owner): the selected camera and four key tracks. */
typedef struct SdvCameraSchedule {
    unsigned char unmodeled_00[4];
    int camera;                     /* +0x04: the selected camera's script address, 0 when none */
    unsigned char unmodeled_08;
    unsigned char animation;        /* +0x09: cleared on selection */
    unsigned char unmodeled_0a[2];
    SdvScheduleChannel channels[4]; /* +0x0c */
} SdvCameraSchedule;

#endif
