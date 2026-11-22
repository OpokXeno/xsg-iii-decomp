/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_XGL_CD_H
#define INCLUDE_MAIN_XGL_CD_H

/* Types owned by main/tu012 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

typedef struct XglTaskPrefix XglTaskPrefix;

typedef struct XglTaskScheduler XglTaskScheduler;

/* The clock reading xglClockRead fills and xglClockDayTime2UInt / xglClockUInt2DayTime convert. */
typedef struct XglClock {
    unsigned char status;
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
} XglClock;

/* Only the link/callback prefix of a task record; real tasks are 0x80 bytes and callers add payload after it. */
struct XglTaskPrefix {
    struct XglTaskScheduler *scheduler;
    XglTaskPrefix *next;
    XglTaskPrefix *previous;
    int (*callback)(XglTaskPrefix *task);
};

struct XglTaskScheduler {
    XglTaskPrefix *free_tasks;
    XglTaskPrefix *active_head;
    XglTaskPrefix *active_tail;
    XglTaskPrefix *next_to_visit;
};

/* xglCdGetFilePos and xglCdGetFileData fill a 0x30-byte file-position record;
 * only its leading lbn word is evidenced by the callers in xgl_cd.c. */
typedef struct CdFilePosition {
    unsigned int lbn;
    int stored_bytes;
    unsigned char unmodeled_08[0x1c];
    unsigned char readMedium;
    unsigned char unmodeled_25;
    unsigned char packed;
    unsigned char unmodeled_27;
    int fileDescriptor;
    int file_size;
} CdFilePosition;

#endif
