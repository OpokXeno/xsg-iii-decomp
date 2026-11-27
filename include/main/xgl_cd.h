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

/* The 0x30-byte file-position record xglCdGetFilePos / xglCdCheckFile fill: the start sector, the stored
 * and the unpacked size, how the file is read and its descriptor. */
typedef struct CdFilePosition {
    unsigned int lbn;
    unsigned int stored_bytes;
    unsigned char unmodeled_08[0x1c];
    unsigned char readMedium;
    unsigned char unmodeled_25;
    unsigned char packed;
    unsigned char unmodeled_27;
    int fileDescriptor;
    int file_size;
} CdFilePosition;

/* A callback gets a third argument: the current read tag from xglCdGetFilePos (lw $6 at
 * 0x19BA04), the request's argument from the read service. */
typedef void CdCompletionCallback(int event, int eventValue, int tag);

/* xglCdReadFilePart's callback argument may instead select one: the current global callback,
 * the dummy or the default one (xglCdReadFile passes its notify argument through as it). */
#define CD_NOTIFY_CURRENT ((CdCompletionCallback *) 0)
#define CD_NOTIFY_DUMMY ((CdCompletionCallback *) 1)
#define CD_NOTIFY_DEFAULT ((CdCompletionCallback *) 2)
#define CD_READ_ASYNC 1
#define CD_READ_NO_EXTRACT 2
#define CD_READ_WHILE_BUSY 4

int xglCdReadFilePart(const char *name, void *transferBuffer, int readOptions,
                      CdCompletionCallback *notify, int offset, int length);
int xglCdReadFile(const char *name, void *transferBuffer, int readOptions, CdCompletionCallback *notify);

#endif
