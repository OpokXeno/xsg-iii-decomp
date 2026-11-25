#include "common.h"
#include "shared.h"
#include "main/xgl_cd.h"

unsigned int xglClockDayTime2UInt(XglClock *clock_time);
void xglClockUInt2DayTime(XglClock *clock_time, unsigned int elapsed);

/* Where a file is read from (CdFilePosition.readMedium, CdStreamParam.readMedium). */
#define CD_SOURCE_DISC 0
#define CD_SOURCE_HOST 1
#define CD_SOURCE_HDD 2

typedef struct StreamXssBuffer StreamXssBuffer;

typedef struct StreamRing StreamRing;

typedef struct CdStreamParam CdStreamParam;

struct StreamRing {
    unsigned char *ringBuffer;
    int capacity;
    int write_position;
    int read_position;
};

struct StreamXssBuffer {
    unsigned char *xssInputStart;
    unsigned char *position;
    /* The current byte of the interleave map and how many of its bits are left. */
    unsigned char map_byte;
    unsigned char bits_left;
    short sectors;
};

/* A stream record; xgl_1.c reads movies through one. */
struct CdStreamParam {
    /* The XSS interleave state; a stream that has one reads through
     * StreamReadRingCoreXss (xglCdStreamReadRingCore). */
    StreamXssBuffer *xss;       /* +0x00 */
    /* Set once the stream has no bytes left (xglCdStreamRead and the ring
     * readers); cleared by xglCdStreamSeek and xglCdStreamParamInit. */
    unsigned char exhausted;    /* +0x04 */
    /* CD_SOURCE_*: xglCdStreamSeek, xglCdStreamClose, xglCdStreamStat and
     * StreamReadRingCoreSub switch on this byte. */
    unsigned char readMedium;       /* +0x05 */
    short block_count;
    int sector_position;        /* +0x08: advanced by StreamReadRingCoreSub */
    int read_alignment;             /* +0x0c: xglCdStreamRead pads reads to it */
    int descriptor;             /* +0x10 */
    int file_byte_count;        /* +0x14 */
    int remaining; /* +0x18: bytes not yet transferred into the stream rings. */
    int ring_size;
    StreamRing ring;            /* +0x20 */
    StreamRing alternate_ring;  /* +0x30 */
};

/* A CD sector is 2048 bytes (1 << CD_SECTOR_SHIFT). */
#define CD_SECTOR_SHIFT 11

int xglCdCheckFile(const char *path, void *positionRecord);

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
void xglCdOpenDevice(void);
int xglCdServiceIfPending(void);
signed char xglCdGetServicePending(void);
void xglCdSetReadTag(int readTag);
int xglCdGetFileSize(const char *path);
void xglCdReadCancel(int immediate);
void xglHddErrorScreen(void);
int xglHddCheck(void);
int xglArxExtract(unsigned int *dest, unsigned int *src);
extern XglTaskPrefix *xglTaskEntryPrev(XglTaskScheduler *scheduler,
                                int (*callback)(XglTaskPrefix *task),
                                XglTaskPrefix *entry);
extern XglTaskPrefix *xglTaskEntryNext(XglTaskScheduler *scheduler,
                                int (*callback)(XglTaskPrefix *task),
                                XglTaskPrefix *entry);
extern u8 *buffer;
int sceLseek(int descriptor, int offset, int whence);

/* sceLseek origins (SDK sceio.h). */
#define SCE_SEEK_SET 0
#define SCE_SEEK_CUR 1
#define SCE_SEEK_END 2

typedef struct CdReadRetryPolicy {
    unsigned char try_count;
    unsigned char spindle_control;
    unsigned char data_pattern;
} CdReadRetryPolicy;

/* Only the fields xglCdArcInitSub1 writes are evidenced here. */
typedef struct CdArchiveEntry {
    unsigned char readMedium;   /* CD_SOURCE_*, or CD_SOURCE_UNUSED */
    unsigned char unmodeled_01;
    unsigned char packed;
    unsigned char unmodeled_03;
    unsigned char *archiveHeader;
    int lbn;
    int file_lbn_offset;
    int stored_bytes;
    int file_size;
} CdArchiveEntry;

/* Events passed to a completion callback (int event, int eventValue). */
#define CD_EVENT_LOOKUP 0 /* value: the path being looked up */
#define CD_EVENT_SIZE 1
#define CD_EVENT_PROGRESS 2
#define CD_EVENT_DONE 4
#define CD_EVENT_READ_ERROR (-2)
/* xglCdSetCallback arguments that are not a callback address. */
#define CD_CALLBACK_DEFAULT 0
#define CD_CALLBACK_DUMMY 1
#define CD_CALLBACK_KEEP (-1)
/* xglCdReadFilePart result while another read is active. */
#define CD_READ_BUSY (-3)
/* sceCdGetError: the read was aborted (SDK libcdvd.h); higher codes are real errors. */
#define SCECdErABRT 1
/* A CD sector is fixed at 2048 bytes on this drive/read path (see also
 * xglCdStreamReadRing's own `& -2048` sector rounding later in this TU). */
#define CD_SECTOR_BYTES 2048
#define ARC_RECORD_EXTENDED        0x80
#define ARC_RECORD_TYPE_EXTENDED   0x40
#define ARC_RECORD_LENGTH_MASK     0x3f
#define ARC_RECORD_HEADER_BYTES    7
#define ARC_RECORD_TYPE_EXTRA_BYTES 3
#define ARC_TABLE_ALIGN_BYTES      64
#define CD_PACKED_MAGIC 0x585241
/* CdArchiveEntry.readMedium of an archive slot that holds nothing. */
#define CD_SOURCE_UNUSED 0xff
/* CdDriverState.threadActivity: what the control thread is doing. */
#define CD_STATE_IDLE 0
#define CD_STATE_SYNC_READ 1
#define CD_STATE_ASYNC_READ 2
#define CD_STATE_STREAM 3
/* LW has room for 32 queued requests (+0x40..+0x103F), but queue_next, xglCdServiceRead and
 * xglCdReadFilePart wrap the indices with 0x7F, as if there were 128. */
#define CD_QUEUE_SIZE 32
#define CD_QUEUE_INDEX_MASK 0x7f
/* Host and HDD files are read in pieces of at most 64 KiB. */
#define CD_FILE_CHUNK_BYTES 0x10000
#define CD_SECTOR_MASK (CD_SECTOR_BYTES - 1)
#define CD_PATH_MAX 256
/* Calendar arithmetic of xglClockDayTime2UInt()/xglClockUInt2DayTime(). */
#define CLOCK_BASE_YEAR 2000
/* The year whose leap days ((year - 1997) >> 2) counts from; see xglClockDayTime2UInt(). */
#define CLOCK_LEAP_REFERENCE_YEAR 1997
#define CLOCK_SECONDS_PER_MINUTE 60
#define CLOCK_SECONDS_PER_HOUR 3600
#define CLOCK_SECONDS_PER_DAY 86400
#define CLOCK_SECONDS_PER_COMMON_YEAR 31536000

/*
 * The queue entries occupy 0x80 bytes; only their used prefix is named.
 * xglCdReadFilePart enqueues by
 * storing its own second argument (buffer) at entry+0x00 and its own first
 * argument (name) at entry+0x14, with the offset and length at +0x04 and +0x08
 * and the completion callback at +0x0C. The queue-next helper queue_next
 * passes entry+0x10 to xglCdSetReadTag (xglCdSetReadTag) and entry+0x14 back as
 * the name, in xglCdReadFile's (name, buffer, ...) order from main/xgl_cd.h.
 */
typedef struct CdRequestPartial {
    void *transferBuffer;
    int file_offset;
    int length;
    CdCompletionCallback *completion_callback;
    int tag;        /* +0x10: handed to xglCdSetReadTag before the read starts */
    char name[0x6c]; /* +0x14 */
} CdRequestPartial;

/*
 * Partial view of LW, limited to fields evidenced by this TU.
 * unmodeled_XX spans are opaque bytes between evidenced fields; naming them
 * `unmodeled_<offset>[<size>]` follows the convention used across the
 * project (e.g. src/main/game.c, src/main/ssd_init.h) instead of inventing
 * members to complete the struct's size.
 */
typedef struct CdDriverState {
    int active_file_descriptor;                 /* +0x00 */
    u8 *write_cursor;                            /* +0x04 */
    int remaining_bytes;                         /* +0x08 */
    int read_poll_mode;                          /* +0x0c */
    /* queue_next and xglCdServiceRead call it with callback_argument as
     * a third argument. */
    CdCompletionCallback *completion_callback;  /* +0x10 */
    int callback_argument;                       /* +0x14: the read tag (xglCdReadTag) */
    u8 unmodeled_018[4];                         /* +0x18..0x1b */
    u8 disk_state;                               /* +0x1c */
    u8 disk_flags;                               /* +0x1d */
    u8 unmodeled_01e[2];                         /* +0x1e..0x1f */
    void *extract_source;                         /* +0x20 */
    void *extract_destination;                    /* +0x24 */
    int extract_byte_count;                      /* +0x28 */
    u8 unmodeled_02c[0x4];                       /* +0x2c..0x2f */
    signed char threadActivity;                  /* +0x30 */
    u8 spindle_control;                          /* +0x31 */
    u8 search_layer;                             /* +0x32 */
    u8 dual_layer_disc;                          /* +0x33 */
    u8 power_off_pending;                        /* +0x34 */
    u8 archive_init_active;                      /* +0x35: 1 while xglCdArcInit rebuilds the archive slots */
    signed char queuedReadDelay;                 /* +0x36: frames until the queued read starts (xglCdControlThread) */
    u8 unmodeled_037[0x7];                       /* +0x37..0x3d */
    u8 queue_cursor;                             /* +0x3e */
    u8 queue_tail;                               /* +0x3f */
    CdRequestPartial readQueue[CD_QUEUE_SIZE];    /* +0x40..0x103f */
    u8 unmodeled_1040[0x3000];                   /* +0x1040..0x403f: the rest of the work area, up to xglCdStreamService */
} CdDriverState;

#define CD_STREAM_LIST_SIZE 2

typedef struct CdStreamService {
    unsigned short turn_countdown;  /* +0x00 */
    unsigned short turn_period;     /* +0x02 */
    unsigned char current_stream_index;          /* +0x04 */
    unsigned char unmodeled_05[0xb];
    CdStreamParam *streams[CD_STREAM_LIST_SIZE]; /* +0x10 */
} CdStreamService;

/* Four 0x20-byte archive slots (slot n at +0x20 * n); the table that follows them at
 * D_0089B660 is cleared by func_00187AB8. */
typedef union CdArchiveStorage {
    CdArchiveEntry archiveEntry;
    unsigned char slotImage[0x20];
} CdArchiveStorage;

extern int sceOpen(const char *path, int flags, ...);
extern int sceCdSync(int mode);
extern int sceCdDiskReady(int mode);
extern int sceCdRead(int lbn, int sectors, void *transferBuffer, CdReadRetryPolicy *readPolicy);
int BCD2INT(unsigned char bcd);
void extract(u32 *extractDestination, u32 *extractSource, int extractByteCount);
extern int sceRead(int descriptor, void *buffer, int bytes);
extern u32 sceCdGetReadPos(void);
extern int sceCdGetError(void);
extern int sceCdLayerSearchFile(CdFilePosition *file, const char *name, int layer);
/* Twelve month lengths followed by twelve cumulative month offsets. */
extern unsigned short monthday[24];
extern char xglCdHddPathHead[8];
extern char xenosaga00FileName[16];
extern char xenosaga10FileName[16];
extern char xenosaga20FileName[16];
extern char xglCdHostPathHead[32];
extern int xglCdDeviceHandle;
extern CdCompletionCallback *xglCdNotifyCallback;
extern int xglCdReadTag;
extern CdDriverState LW;
/* The stream service state at 0x959CC0 (its list at +0x10 is the symbol xglCdStreamList). */
extern CdStreamService xglCdStreamService;
extern CdArchiveStorage xglCdArchiveSlots[4];
void xglCdDefaultCallback(int event, int eventValue, int tag);
void xglCdReset(void);
int ADXF_StopNw(int handle);
int ADXF_Stop(int handle);
int sceCdBreak(void);
int sceCdPause(void);
void xglCdDummyCallback(int event, int eventValue, int tag);
int xglCdGetFilePosSub(CdArchiveEntry *entry, const char *path);
/* Locates a file: in the archive slots first (the path then becomes the archive's own file name),
 * else by its name on the disc or the host. Fills the position record and returns it, or 0 when the
 * file is missing. The callback gets CD_EVENT_LOOKUP with the path and the current read tag. */
CdFilePosition *xglCdGetFilePos(CdFilePosition *position, const char *name, CdCompletionCallback *notify);
void hdd_error(void);
int queue_next(void);

/* completion_callback event of xglCdServiceRead: value is the read position (bytes read for a
 * device-handle read, the drive's read position, or the file offset). */
#define CD_EVENT_PROGRESS_ASYNC 3
#define CD_DEVICE_READ_DONE 3
#define CD_DEVICE_READ_ERROR 4

extern unsigned char xglCdServiceActive;
int ADXF_GetNumReadSct(int handle);
int ADXF_GetStat(int handle);
void ADXM_ExecMain(void);
void xglCdServiceRead(void);
int ADXF_Open(const unsigned char *table, int mode);

/* The stream service state at 0x959CC0 is defined with the other statics above. xglCdControlThread
 * gives each open stream of the list a turn: turn_countdown runs down from turn_period, then `current`
 * moves on to the next open stream. The list at +0x10 is the symbol xglCdStreamList. */
#define CD_STREAM_TURN_PERIOD 0x40

/* Where a name stands in an archive's directory tree: its depth and the length of its directory
 * part. Both halves are compared as one word. */
typedef union ArcNameKey {
    struct {
        short depth;
        short length;
    } halves;
    int both;
} ArcNameKey;

#define ARC_DIRECTORY_DEPTH_MAX 16
#define ARC_RECORD_POSITION_BYTES 7
#define ARC_RECORD_SIZE_BYTES 3

/* Debug output for a path that starts with "data\": the prefix is dropped. */
extern const char xglCdReplacePathFormat[];
extern const char xglCdReplacedPathFormat[];
int printf(const char *format, ...);

#define CD_DATA_PREFIX_LENGTH 5
#define CD_OPEN_READ 1
/* sceLseek moves at most this far in one call. */
#define CD_SEEK_STEP 0x40000000

int ADXF_Seek(int handle, unsigned int lbn, int mode);
int ADXF_ReadNw(int handle, int sectors, void *buffer);
/* Unsized: the original addresses it absolutely, not through $gp. */
extern signed char xglCdStreamsActive[];
/* .rodata strings of the CD module (typed definitions of the extern declarations above). */
extern const unsigned char xglCdDeviceName[16];
extern const char xglCdReplacePathFormat[32];
extern const char xglCdReplacedPathFormat[8];

void extract(u32 *extractDestination, u32 *extractSource, int extractByteCount)
{
    if (extractDestination != extractSource) {
        while (extractByteCount > 0) {
            *extractDestination++ = *extractSource++;
            extractByteCount -= 4;
        }
    }
}

typedef int s32;
typedef signed char s8;

s32 xglCdStartQueuedRead(void) {
    CdRequestPartial *queuedRead;

    if (LW.queue_tail != LW.queue_cursor) {
        queuedRead = &LW.readQueue[LW.queue_cursor];
        xglCdSetReadTag(queuedRead->tag);
        xglCdReadFilePart(queuedRead->name, queuedRead->transferBuffer, 1, queuedRead->completion_callback, queuedRead->file_offset, queuedRead->length);
        LW.queue_cursor = (LW.queue_cursor + 1) & CD_QUEUE_INDEX_MASK;
        return 0;
    }
    return 1;
}

int queue_next(void)
{
    LW.completion_callback(CD_EVENT_DONE,
                           (LW.queue_tail - LW.queue_cursor) & CD_QUEUE_INDEX_MASK,
                           LW.callback_argument);
    LW.threadActivity = CD_STATE_IDLE;
    return xglCdStartQueuedRead();
}

/* Splat labels of the callees that the X2 names do not cover in X3. */
int func_0015D3A0(int handle);
int func_0015D408(int handle);
void func_00160200(void);
u32 func_002F7E38(void);
int func_002E9B60(int descriptor, void *buffer, int bytes);
int func_002E9920(int descriptor, int offset, int whence);

/* One step of the active asynchronous read. A disc read reports its position and, once the drive
 * (or the device handle, when one is open) is done, unpacks the data and starts the next queued
 * request. A host or HDD read transfers up to CD_FILE_CHUNK_BYTES; after the last chunk it pads
 * the sector, closes the file, unpacks and starts the next request. The progress callback is
 * skipped while xglCdServiceActive is set; the function sets it on return. */
void xglCdServiceRead(void)
{
    int requested_bytes;
    int transferred;

    switch (LW.read_poll_mode) {
    case CD_SOURCE_DISC:
        if (xglCdDeviceHandle != 0) {
            if (xglCdServiceActive == 0) {
                LW.completion_callback(CD_EVENT_PROGRESS_ASYNC,
                                       func_0015D3A0(xglCdDeviceHandle) << CD_SECTOR_SHIFT,
                                       LW.callback_argument);
            }
            if (func_0015D408(xglCdDeviceHandle) == CD_DEVICE_READ_DONE) {
                extract(LW.extract_destination, LW.extract_source, LW.extract_byte_count);
                queue_next();
            }
            func_00160200();
        } else {
            if (xglCdServiceActive == 0) {
                LW.completion_callback(CD_EVENT_PROGRESS_ASYNC, func_002F7E38(),
                                       LW.callback_argument);
            }
            if (sceCdSync(1) != 0) {
                break;
            }
            extract(LW.extract_destination, LW.extract_source, LW.extract_byte_count);
            if (queue_next() != 0) {
                sceCdPause();
            }
        }
        break;
    case CD_SOURCE_HOST:
    case CD_SOURCE_HDD:
        requested_bytes = LW.remaining_bytes;
        if (requested_bytes > CD_FILE_CHUNK_BYTES) {
            requested_bytes = CD_FILE_CHUNK_BYTES;
        }
        transferred = func_002E9B60(LW.active_file_descriptor, LW.write_cursor, requested_bytes);
        if (transferred < 0) {
            LW.queue_cursor = (LW.queue_cursor + CD_QUEUE_INDEX_MASK) & CD_QUEUE_INDEX_MASK;
            break;
        }
        LW.write_cursor += transferred;
        LW.remaining_bytes -= transferred;
        if (xglCdServiceActive == 0) {
            LW.completion_callback(CD_EVENT_PROGRESS_ASYNC,
                                   func_002E9920(LW.active_file_descriptor, 0, SCE_SEEK_CUR),
                                   LW.callback_argument);
        }
        if (LW.remaining_bytes > 0) {
            break;
        }
        {
            int partial_sector = transferred & CD_SECTOR_MASK;

            if (partial_sector != 0) {
                int remaining_padding = (-partial_sector) & CD_SECTOR_MASK;

                while (remaining_padding > 0) {
                    *LW.write_cursor++ = remaining_padding;
                    remaining_padding--;
                }
            }
        }
        sceClose(LW.active_file_descriptor);
        LW.active_file_descriptor = -1;
        extract(LW.extract_destination, LW.extract_source, LW.extract_byte_count);
        queue_next();
        break;
    }
    xglCdServiceActive = 1;
}

/* Clears the busy byte xglCdServiceActive and stores the device handle from the table
 * at xglCdDeviceName in xglCdDeviceHandle. */
void xglCdOpenDevice(void)
{
    xglCdServiceActive = 0;
    xglCdDeviceHandle = ADXF_Open(xglCdDeviceName, 0);
}

/* Small-data flag: a positive value means the clock was read already; the first
 * read stores 10 when the console reports 1. xglCdControlThread counts it down once per tick. */
extern signed char xglClockRefreshCountdown;

/* One tick of the CD service: counts the clock refresh down, starts the next queued read when its
 * delay runs out, serves an asynchronous read, and resets an unknown state to idle. */
void xglCdControlThread(void)
{
    if (xglClockRefreshCountdown > 0) {
        xglClockRefreshCountdown--;
    }
    if (LW.queuedReadDelay > 0) {
        LW.queuedReadDelay--;
        if (LW.queuedReadDelay <= 0) {
            xglCdStartQueuedRead();
        }
    }
    switch (LW.threadActivity) {
    case CD_STATE_IDLE:
    case CD_STATE_SYNC_READ:
        break;
    case CD_STATE_ASYNC_READ:
        xglCdServiceRead();
        break;
    default:
        LW.threadActivity = CD_STATE_IDLE;
        break;
    }
    xglCdServiceActive = 0;
}

/* One BCD byte: high nibble tens, low nibble units. */
int BCD2INT(unsigned char bcd)
{
    return (bcd >> 4) * 10 + (bcd & 15);
}

/* The console's clock record: xglCdClockRecord holds the BCD fields read below. */
extern unsigned char xglCdClockRecord[];
int func_002F8588(unsigned char *clock_record);

int xglClockReadBcd(XglClock *clock_time)
{
    int clockReadResult = 0;
    unsigned char secondOfMinute;

    if (xglClockRefreshCountdown <= 0) {
        clockReadResult = func_002F8588(xglCdClockRecord) - 1;
        if (clockReadResult == 0) {
            xglClockRefreshCountdown = 10;
        }
    }
    clock_time->year = BCD2INT(xglCdClockRecord[7]) + CLOCK_BASE_YEAR;
    clock_time->month = BCD2INT(xglCdClockRecord[6]);
    clock_time->day = BCD2INT(xglCdClockRecord[5]);
    clock_time->hour = BCD2INT(xglCdClockRecord[3]);
    clock_time->minute = BCD2INT(xglCdClockRecord[2]);
    secondOfMinute = BCD2INT(xglCdClockRecord[1]);
    clock_time->status = 0;
    clock_time->second = secondOfMinute;
    return clockReadResult;
}

#define CLOCK_READ_ATTEMPTS 20
void sceGsSyncV(int);

/* Reads the clock, retrying up to CLOCK_READ_ATTEMPTS times while xglClockReadBcd fails. */
void xglClockRead(XglClock *clock_time)
{
    int attempt;

    for (attempt = 0; attempt < CLOCK_READ_ATTEMPTS; attempt++) {
        if (xglClockReadBcd(clock_time) == 0) {
            break;
        }
        sceGsSyncV(0);
    }
}

/*
 * Converts a calendar record to the number of seconds elapsed since
 * 2000-01-01 00:00:00.  monthday[12..23] holds the cumulative day offset of
 * each month; the leap day of the current year counts from March onwards,
 * and ((year - CLOCK_LEAP_REFERENCE_YEAR) >> 2) counts the leap days of the whole years before it.
 */
unsigned int xglClockDayTime2UInt(XglClock *clock_time)
{
    unsigned int seconds;
    unsigned int year;
    unsigned char month;

    month = clock_time->month;
    seconds = (monthday[month + 11] + clock_time->day - 1) * CLOCK_SECONDS_PER_DAY
        + clock_time->hour * CLOCK_SECONDS_PER_HOUR + clock_time->minute * CLOCK_SECONDS_PER_MINUTE
        + clock_time->second;
    year = clock_time->year;
    if (((year - CLOCK_BASE_YEAR) & 3) == 0 && month >= 3) {
        seconds += CLOCK_SECONDS_PER_DAY;
    }
    return seconds
        + ((year - CLOCK_BASE_YEAR) * CLOCK_SECONDS_PER_COMMON_YEAR
           + ((year - CLOCK_LEAP_REFERENCE_YEAR) >> 2) * CLOCK_SECONDS_PER_DAY);
}

void xglClockUInt2DayTime(XglClock *clock_time, unsigned int elapsed)
{
    XglClock *clock;
    unsigned int year_length = 366;
    unsigned int year = 0;
    unsigned int common_year_length = 365;
    unsigned int leap_year_length = 366;
    unsigned int days;
    unsigned int remainder;
    unsigned int month;
    unsigned int month_length;

    clock = clock_time;
    days = elapsed / CLOCK_SECONDS_PER_DAY;
    remainder = elapsed % CLOCK_SECONDS_PER_DAY;
    clock->hour = remainder / CLOCK_SECONDS_PER_HOUR;
    remainder -= clock->hour * CLOCK_SECONDS_PER_HOUR;
    clock->minute = remainder / CLOCK_SECONDS_PER_MINUTE;
    remainder -= clock->minute * CLOCK_SECONDS_PER_MINUTE;
    clock->second = remainder;

    while (days >= year_length) {
        year += 1;
        days -= year_length;
        year_length = (year & 3) == 0 ? leap_year_length : common_year_length;
    }
    clock->year = year + CLOCK_BASE_YEAR;

    for (month = 0; ; month += 1) {
        month_length = monthday[month];
        if (month == 1 && (clock->year & 3) == 0)
            month_length += 1;
        if (days < month_length)
            break;
        days -= month_length;
    }
    clock->month = month + 1;
    clock->day = days + 1;
}

extern CdArchiveStorage D_0089B640;

/* Where xglCdDiskCheck reads system.cnf; the buffer lies in the EE memory map. */
#define CD_SYSTEM_CNF_BUFFER ((unsigned char *) 0x00A7F800)
#define CD_DISK_STATE_UNKNOWN 0
#define CD_DISK_STATE_IDENTIFIED 1
#define CD_DISK_STATE_TRAY_CHANGED 2
#define CD_DISC_FLAG_ARC10 1
#define CD_DISC_FLAG_ARC20 2
#define CD_ARC_HEADER_RESERVE 0x10000
#define CD_STATUS_SHELL_OPEN 1
#define CD_TRAY_CHECK 2
#define CD_DISK_READY_COMPLETE 2
#define CD_DISK_READY_NOT_READY 6
#define CD_DISC_NONE 0
#define CD_DISC_PS2_DVD 20
#define CD_DISC_DETECT_END 5
#define CD_CHECK_UNSUPPORTED (-1)
#define CD_CHECK_SHELL_OPEN (-2)
#define CD_CHECK_NOT_READY (-3)

extern char xglCdArchiveFileName10[];
extern char xglCdArchiveFileName20[];
extern char xglCdSystemCnfName[];
int func_002F8320(void);
int func_002F8260(void);
int func_002F8648(int mode, int *tray_changed);

/* Disc identification: returns the disc's archive flags, 0 with no disc, or a CD_CHECK_*
 * value. A disc not identified yet must be ready and a PS2 DVD; its flags come from the sizes
 * of the two archive headers, and system.cnf must name an executable whose third letter is 'U'.
 * An identified disc only has its tray and shell checked. */
int xglCdDiskCheck(void)
{
    int tray_changed;
    unsigned char *systemCnfText;
    unsigned char diskState;
    int diskCode;
    int trayStatus;
    int archiveSize;

    diskState = LW.disk_state;
    if (diskState != CD_DISK_STATE_IDENTIFIED) {
        trayStatus = func_002F8320();
        if (trayStatus == CD_STATUS_SHELL_OPEN) {
            LW.disk_state = CD_DISK_STATE_UNKNOWN;
            return CD_CHECK_SHELL_OPEN;
        }
        diskCode = sceCdDiskReady(1);
        if (diskCode != CD_DISK_READY_COMPLETE) {
            if (LW.disk_state == CD_DISK_STATE_TRAY_CHANGED
                || (trayStatus == 0 && diskCode == CD_DISK_READY_NOT_READY)) {
                return CD_CHECK_SHELL_OPEN;
            }
            LW.disk_state = CD_DISK_STATE_UNKNOWN;
            return CD_CHECK_NOT_READY;
        }
        LW.disk_state = CD_DISK_STATE_UNKNOWN;
        diskCode = func_002F8260();
        if (diskCode == CD_DISC_NONE) {
            return 0;
        }
        if (diskCode != CD_DISC_PS2_DVD) {
            if (diskCode >= CD_DISC_DETECT_END) {
                return CD_CHECK_UNSUPPORTED;
            }
            return CD_CHECK_NOT_READY;
        }
        LW.archive_init_active = 1;
        LW.disk_flags = 0;
        archiveSize = xglCdGetFileSize(xglCdArchiveFileName10);
        if (archiveSize > 0 && archiveSize <= CD_ARC_HEADER_RESERVE) {
            LW.disk_flags |= CD_DISC_FLAG_ARC10;
        }
        archiveSize = xglCdGetFileSize(xglCdArchiveFileName20);
        if (archiveSize > 0 && archiveSize <= CD_ARC_HEADER_RESERVE) {
            LW.disk_flags |= CD_DISC_FLAG_ARC20;
        }
        LW.archive_init_active = 0;
        if (LW.disk_flags == 0) {
            return CD_CHECK_UNSUPPORTED;
        }
        systemCnfText = CD_SYSTEM_CNF_BUFFER;
        while (xglCdServiceIfPending() != 0) {
        }
        LW.archive_init_active = 1;
        xglCdReadFilePart(xglCdSystemCnfName, systemCnfText, 0, CD_NOTIFY_DUMMY, 0, CD_SECTOR_BYTES);
        LW.archive_init_active = 0;
        while (*systemCnfText++ != '\\') {
        }
        if (systemCnfText[2] != 'U') {
            return CD_CHECK_UNSUPPORTED;
        }
        LW.disk_state = CD_DISK_STATE_IDENTIFIED;
    } else {
        if (func_002F8648(CD_TRAY_CHECK, &tray_changed) == 0 || tray_changed != 0) {
            LW.disk_state = CD_DISK_STATE_TRAY_CHANGED;
            return CD_CHECK_SHELL_OPEN;
        }
        if (func_002F8320() == CD_STATUS_SHELL_OPEN) {
            LW.disk_state = CD_DISK_STATE_UNKNOWN;
            return CD_CHECK_SHELL_OPEN;
        }
    }
    return LW.disk_flags;
}

/* The default and the dummy completion callbacks; both ignore every event. */
void xglCdDefaultCallback(int event, int eventValue, int tag)
{
}

void xglCdDummyCallback(int event, int eventValue, int tag)
{
}

/* Resets the request queue, the stream service and the callback, then pauses the drive and waits
 * for it. */
void xglCdReset(void)
{
    int i;

    LW.queue_cursor = 0;
    LW.queue_tail = 0;
    xglCdStreamService.turn_period = CD_STREAM_TURN_PERIOD;
    xglCdStreamService.turn_countdown = CD_STREAM_TURN_PERIOD;
    xglCdStreamService.current_stream_index = 0;
    for (i = 0; i < CD_STREAM_LIST_SIZE; i++) {
        xglCdStreamService.streams[i] = 0;
    }
    LW.threadActivity = CD_STATE_IDLE;
    xglCdNotifyCallback = xglCdDefaultCallback;
    sceCdPause();
    sceCdSync(0);
}

CdCompletionCallback *xglCdSetCallback(int callback_address)
{
    CdCompletionCallback *old_callback = xglCdNotifyCallback;

    switch (callback_address) {
    case CD_CALLBACK_DEFAULT:
        xglCdNotifyCallback = xglCdDefaultCallback;
        break;
    case CD_CALLBACK_DUMMY:
        xglCdNotifyCallback = xglCdDummyCallback;
        break;
    case CD_CALLBACK_KEEP:
        break;
    default:
        xglCdNotifyCallback = (CdCompletionCallback *)callback_address;
        break;
    }
    return old_callback;
}

/* Sets the current read tag xglCdReadTag. */
void xglCdSetReadTag(int readTag)
{
    xglCdReadTag = readTag;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00187AB8);

/* Expands one name record of an archive table into `name` and returns the name's end. A record starts with
 * a count byte. Zero: the bytes after it, up to a terminator, replace the name. Otherwise the low
 * nibble is the number of bytes copied over the name, starting (high nibble + 1) characters
 * before its end; the rest of the name stays. */
char *xglCdArcNameExpand(char *name, const unsigned char *nameRecord)
{
    unsigned char nameCountByte;
    int backChars;

    nameCountByte = *nameRecord++;
    if (nameCountByte == 0) {
        while ((*name++ = *nameRecord++) != 0) {
        }
    } else {
        backChars = (nameCountByte >> 4) + 1;
        nameCountByte &= 0xF;
        while (*name != 0) {
            name++;
        }
        name -= backChars;
        while (nameCountByte != 0) {
            *name++ = *nameRecord++;
            nameCountByte--;
        }
        name--;
        while (*name++ != 0) {
        }
    }
    return name - 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00187BD8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00187E28);

/* Splat labels of the callees and data that xglCdGetFilePos uses, at their X3 addresses. */
int func_00187E28(CdArchiveEntry *entry, const char *path);
int func_002E9518(char *path, int mode);
int func_002F71F0(CdFilePosition *position, char *buffer);
extern char D_00551188[4];
extern CdArchiveStorage D_0089B600;
extern const char D_005401D0[32];
/* Unsized: the original addresses these strings absolutely, not through $gp. */
extern const char D_005401F0[];
extern const char xglCdSlotZeroArchivePath[];
extern const char xglCdSlotOneArchivePath[];
extern const char xglCdSlotThreeArchivePath[];

/* Locates a file: in the archive slots first (the path then becomes the archive's own file name),
 * else by its name on the disc or the host. Fills the position record and returns it, or 0 when the
 * file is missing. The callback gets CD_EVENT_LOOKUP with the path and the current read tag. */
CdFilePosition *xglCdGetFilePos(CdFilePosition *position, const char *name, CdCompletionCallback *notify)
{
    char resolvedPath[CD_PATH_MAX];
    const char *lookupPath = name;
    CdArchiveEntry *archiveEntry;
    CdArchiveStorage *archiveSlot;
    const char *cursor;
    const char *absolute_source;
    char *absolute_destination;
    char *pathWrite;
    char *scan;
    int fileDescriptor;
    int fileSize;

    if (lookupPath[0] == 'd' && lookupPath[1] == 'a' && lookupPath[2] == 't' && lookupPath[3] == 'a' && lookupPath[4] == '\\') {
        printf(D_005401D0, name);
        name = lookupPath + CD_DATA_PREFIX_LENGTH;
        printf(D_005401F0, name);
    }
    position->readMedium = 0;
    if (name[0] != '\\') {
        for (scan = (char *) name; *scan != 0; scan++) {
            if (*scan == '/') {
                *scan = '\\';
            }
        }
    }
    do {
        archiveEntry = &xglCdArchiveSlots[0].archiveEntry;
        if (archiveEntry->readMedium != CD_SOURCE_UNUSED && func_00187E28(archiveEntry, name) != 0) {
            name = xglCdSlotZeroArchivePath;
            position->readMedium = archiveEntry->readMedium;
            break;
        }
        archiveSlot = &D_0089B600;
        archiveEntry = &archiveSlot->archiveEntry;
        if (archiveEntry->readMedium != CD_SOURCE_UNUSED && func_00187E28(archiveEntry, name) != 0) {
            name = xglCdSlotOneArchivePath;
            position->readMedium = archiveEntry->readMedium;
        } else {
            archiveSlot = archiveSlot + 2;
            archiveEntry = &archiveSlot->archiveEntry;
            if (archiveEntry->readMedium != CD_SOURCE_UNUSED && func_00187E28(archiveEntry, name) != 0) {
                name = xglCdSlotThreeArchivePath;
                position->readMedium = archiveEntry->readMedium;
            } else {
                archiveEntry = 0;
            }
        }
    } while (0);
    if (name[0] == '\\') {
        absolute_source = name + 1;
        absolute_destination = resolvedPath;
        while ((*absolute_destination = *absolute_source) != 0) {
            absolute_source++;
            absolute_destination++;
        }
        position->readMedium = CD_SOURCE_HOST;
    } else {
        pathWrite = resolvedPath;
        switch (position->readMedium) {
        case CD_SOURCE_DISC:
            resolvedPath[0] = '\\';
            pathWrite = resolvedPath + 1;
            for (cursor = name; *cursor != 0; cursor++, pathWrite++) {
                if (*cursor < 'a' || *cursor > 'z') {
                    *pathWrite = *cursor;
                } else {
                    *pathWrite = *cursor - ('a' - 'A');
                }
            }
            pathWrite[0] = ';';
            pathWrite[1] = '1';
            pathWrite[2] = 0;
            break;
        case CD_SOURCE_HOST:
        case CD_SOURCE_HDD:
            cursor = D_00551188;
            while ((*pathWrite = *cursor) != 0) {
                cursor++;
                pathWrite++;
            }
            cursor = name;
            for (;;) {
                if (*cursor == '\\') {
                    *pathWrite = '/';
                } else {
                    *pathWrite = *cursor;
                }
                if (*cursor == 0) {
                    break;
                }
                cursor++;
                pathWrite++;
            }
            break;
        }
    }
    notify(CD_EVENT_LOOKUP, (int) lookupPath, xglCdReadTag);
    switch (position->readMedium) {
    case CD_SOURCE_DISC:
        if (archiveEntry != 0) {
            position->lbn = archiveEntry->lbn + archiveEntry->file_lbn_offset;
            position->stored_bytes = archiveEntry->stored_bytes;
            position->packed = archiveEntry->packed;
            position->file_size = archiveEntry->file_size;
            break;
        }
        if (LW.archive_init_active == 0) {
            return 0;
        }
        sceCdSync(0);
        sceCdDiskReady(0);
        if (func_002F71F0(position, resolvedPath) == 0) {
            return 0;
        }
        position->packed = 1;
        position->file_size = position->stored_bytes;
        break;
    case CD_SOURCE_HOST:
        fileDescriptor = func_002E9518(resolvedPath, 1);
        if (fileDescriptor < 0) {
            return 0;
        }
        if (archiveEntry != 0) {
            position->lbn = archiveEntry->lbn + archiveEntry->file_lbn_offset;
            position->stored_bytes = archiveEntry->stored_bytes;
            position->packed = archiveEntry->packed;
            position->file_size = archiveEntry->file_size;
        } else {
            position->lbn = 0;
            position->packed = 0;
            fileSize = func_002E9920(fileDescriptor, 0, SCE_SEEK_END);
            position->stored_bytes = fileSize;
            if (position->packed == 0) {
                position->file_size = fileSize;
            }
        }
        position->fileDescriptor = fileDescriptor;
        break;
    case CD_SOURCE_HDD:
        return 0;
    }
    return position;
}

/* Probes a path: -1 when missing, closes the descriptor of a host or HDD file
 * and returns 0 otherwise. */
int xglCdCheckFile(const char *path, void *positionRecord)
{
    CdFilePosition *position = positionRecord;

    if (xglCdGetFilePos(position, path, xglCdDummyCallback) == 0) {
        return -1;
    }
    switch (position->readMedium) {
    case CD_SOURCE_DISC:
        break;
    case CD_SOURCE_HOST:
    case CD_SOURCE_HDD:
        sceClose(position->fileDescriptor);
        break;
    }
    return 0;
}

/* Size of a path in bytes, or -1 when it is missing. */
int xglCdGetFileSize(const char *path)
{
    CdFilePosition position;

    if (xglCdCheckFile(path, &position) != 0) {
        return -1;
    }
    return position.file_size;
}

/*
 * Reads `length` bytes (the whole file when 0) of `name` from byte `offset` on into `transferBuffer`.
 * An asynchronous request while a read runs or one is about to start goes to the request queue.
 * A disc read restarts after a read error, a failed host/HDD read restarts the whole request.
 * Returns the file size in whole sectors, the file size when the read was only started, 0 when
 * queued, CD_READ_BUSY or -1 when the file is missing.
 */
int xglCdReadFilePart(const char *name, void *transferBuffer, int readOptions,
                      CdCompletionCallback *notify, int offset, int length)
{
    CdFilePosition position;
    CdReadRetryPolicy readPolicy;
    CdRequestPartial *request;
    const char *source;
    char *destination;
    int asynchronous;
    int result; /* sceRead byte count, or sceCdGetError status */
    int padding;
    int partial;
    long long seekBytes;
    int medium;

    switch ((int) notify) {
    case (int) CD_NOTIFY_CURRENT:
        notify = xglCdNotifyCallback;
        break;
    case (int) CD_NOTIFY_DUMMY:
        notify = xglCdDummyCallback;
        break;
    case (int) CD_NOTIFY_DEFAULT:
        notify = xglCdDefaultCallback;
        break;
    }
    asynchronous = readOptions & CD_READ_ASYNC;
    if (asynchronous != 0) {
        if (LW.threadActivity == CD_STATE_SYNC_READ || LW.threadActivity == CD_STATE_ASYNC_READ ||
            LW.queuedReadDelay > 0) {
            request = &LW.readQueue[LW.queue_tail];
            source = name;
            destination = request->name;
            while ((*destination++ = *source++) != 0) {
            }
            request->transferBuffer = transferBuffer;
            request->file_offset = offset;
            request->length = length;
            request->completion_callback = notify;
            request->tag = xglCdReadTag;
            LW.queue_tail = (LW.queue_tail + 1) & CD_QUEUE_INDEX_MASK;
            return 0;
        }
    }
    if (LW.threadActivity != CD_STATE_IDLE && readOptions != CD_READ_WHILE_BUSY) {
        return CD_READ_BUSY;
    }
    if (xglCdGetFilePos(&position, name, notify) == 0) {
        return -1;
    }
    position.lbn += offset >> CD_SECTOR_SHIFT;
    if (length > 0) {
        position.stored_bytes = length;
    }
    notify(CD_EVENT_SIZE, (position.stored_bytes + CD_SECTOR_BYTES - 1) & -CD_SECTOR_BYTES, xglCdReadTag);
    medium = position.readMedium;
    LW.completion_callback = notify;
    LW.callback_argument = xglCdReadTag;
    LW.extract_byte_count = (position.stored_bytes + CD_SECTOR_BYTES - 1) & -CD_SECTOR_BYTES;
    LW.extract_destination = transferBuffer;
    LW.extract_source = transferBuffer;
    LW.read_poll_mode = medium;
    switch (medium) {
    case CD_SOURCE_DISC:
    retry_cd:
        if (xglCdDeviceHandle != 0) {
            func_0015D000(xglCdDeviceHandle, position.lbn, 0);
            func_0015CBF0(xglCdDeviceHandle, LW.extract_byte_count / CD_SECTOR_BYTES, LW.extract_source);
        } else {
            sceCdSync(0);
            sceCdDiskReady(0);
            readPolicy.try_count = 0;
            readPolicy.spindle_control = LW.spindle_control;
            readPolicy.data_pattern = 0;
            sceCdRead(position.lbn, LW.extract_byte_count / CD_SECTOR_BYTES, LW.extract_source, &readPolicy);
        }
        if (asynchronous != 0) {
            LW.threadActivity = CD_STATE_ASYNC_READ;
            return position.file_size;
        }
        LW.threadActivity = CD_STATE_SYNC_READ;
        if (xglCdDeviceHandle != 0) {
            while (func_0015D408(xglCdDeviceHandle) != CD_DEVICE_READ_DONE) {
                notify(CD_EVENT_PROGRESS, func_0015D3A0(xglCdDeviceHandle) << CD_SECTOR_SHIFT, xglCdReadTag);
                func_00160200();
            }
        } else {
            while (sceCdSync(1) > 0) {
                notify(CD_EVENT_PROGRESS, func_002F7E38(), xglCdReadTag);
            }
        }
        if (xglCdDeviceHandle != 0) {
            if (func_0015D408(xglCdDeviceHandle) == CD_DEVICE_READ_ERROR) {
                notify(CD_EVENT_READ_ERROR, CD_DEVICE_READ_ERROR, xglCdReadTag);
                goto retry_cd;
            }
        } else {
            result = func_002F8288();
            if ((unsigned int) result > SCECdErABRT) {
                notify(CD_EVENT_READ_ERROR, result, xglCdReadTag);
                goto retry_cd;
            }
        }
        break;
    case CD_SOURCE_HOST:
    case CD_SOURCE_HDD:
        LW.active_file_descriptor = position.fileDescriptor;
        LW.write_cursor = LW.extract_source;
        LW.remaining_bytes = position.stored_bytes;
        /* sceLseek takes an int offset: seek from the start in steps of at most CD_SEEK_STEP. */
        seekBytes = (long long) position.lbn << CD_SECTOR_SHIFT;
        func_002E9920(LW.active_file_descriptor, 0, SCE_SEEK_SET);
        while (seekBytes > CD_SEEK_STEP) {
            func_002E9920(LW.active_file_descriptor, CD_SEEK_STEP, SCE_SEEK_CUR);
            seekBytes -= CD_SEEK_STEP;
        }
        func_002E9920(LW.active_file_descriptor, seekBytes, SCE_SEEK_CUR);
        if (asynchronous != 0) {
            LW.threadActivity = CD_STATE_ASYNC_READ;
            return position.file_size;
        }
        LW.threadActivity = CD_STATE_SYNC_READ;
        /* Host/HDD files are read synchronously in chunks of at most 64 KiB;
         * a failed read restarts the request. */
        do {
            if (LW.remaining_bytes > CD_FILE_CHUNK_BYTES - 1) {
                result = func_002E9B60(LW.active_file_descriptor, LW.write_cursor, CD_FILE_CHUNK_BYTES);
            } else {
                result = func_002E9B60(LW.active_file_descriptor, LW.write_cursor, LW.remaining_bytes);
            }
            if (result < 0) {
                LW.threadActivity = CD_STATE_IDLE;
                return xglCdReadFilePart(name, transferBuffer, readOptions, notify, offset, length);
            }
            LW.write_cursor += result;
            LW.remaining_bytes -= result;
            notify(CD_EVENT_PROGRESS, position.stored_bytes - LW.remaining_bytes, xglCdReadTag);
        } while (LW.remaining_bytes > 0);
        partial = result & CD_SECTOR_MASK;
        if (partial != 0) {
            padding = -partial & CD_SECTOR_MASK;
            while (padding > 0) {
                *LW.write_cursor++ = padding;
                padding--;
            }
        }
        sceClose(LW.active_file_descriptor);
        LW.active_file_descriptor = -1;
        break;
    }
    if ((readOptions & CD_READ_NO_EXTRACT) == 0) {
        extract(LW.extract_destination, LW.extract_source, LW.extract_byte_count);
    }
    notify(CD_EVENT_DONE, 0, xglCdReadTag);
    LW.threadActivity = CD_STATE_IDLE;
    return (position.file_size + CD_SECTOR_BYTES - 1) & -CD_SECTOR_BYTES;
}

int xglCdReadFile(const char *name, void *transferBuffer, int readOptions, CdCompletionCallback *notify)
{
    return xglCdReadFilePart(name, transferBuffer, readOptions, notify, 0, -1);
}

/* Cancels the active read: clears pending requests, breaks or pauses a sync
 * read, closes an open descriptor and goes idle. */
void xglCdReadCancel(int immediate)
{
    LW.queue_tail = LW.queue_cursor;
    if (LW.threadActivity == CD_STATE_SYNC_READ) {
        switch (LW.read_poll_mode) {
        case CD_SOURCE_DISC:
            if (xglCdDeviceHandle != 0) {
                if (immediate) {
                    ADXF_StopNw(xglCdDeviceHandle);
                } else {
                    ADXF_Stop(xglCdDeviceHandle);
                }
            } else {
                sceCdBreak();
                sceCdPause();
            }
            break;
        case CD_SOURCE_HOST:
        case CD_SOURCE_HDD:
            LW.remaining_bytes = 0;
            break;
        }
    } else if (LW.threadActivity == CD_STATE_ASYNC_READ) {
        switch (LW.read_poll_mode) {
        case CD_SOURCE_DISC:
            if (xglCdDeviceHandle != 0) {
                if (immediate) {
                    ADXF_StopNw(xglCdDeviceHandle);
                } else {
                    ADXF_Stop(xglCdDeviceHandle);
                }
            } else {
                sceCdBreak();
                sceCdPause();
            }
            break;
        case CD_SOURCE_HOST:
        case CD_SOURCE_HDD:
            if (LW.active_file_descriptor >= 0) {
                sceClose(LW.active_file_descriptor);
                LW.active_file_descriptor = -1;
            }
            break;
        }
    }
    LW.threadActivity = CD_STATE_IDLE;
}

/* Services the stream list when xglCdStreamsActive is set. */
int xglCdServiceIfPending(void)
{
    if (xglCdStreamsActive[0] == 0) {
        return 0;
    }
    xglCdServiceRead();
    return 1;
}

/* Returns the stream-service flag xglCdStreamsActive. */
signed char xglCdGetServicePending(void)
{
    return xglCdStreamsActive[0];
}

void xglCdRaiseMaxByte(s32 candidate) {
    if (LW.queuedReadDelay < candidate) {
        LW.queuedReadDelay = candidate;
    }
}

s8 *xglCdGetHddPathHead(void) {
    return D_00551188;
}

extern s32 xglCdHddPathTail;

s32 *xglCdGetHddPathTail(void) {
    return &xglCdHddPathTail;
}

void xglCdInitHookNoop(void) {

}

/* The head and the tail of an IOP module path: cdrom0:\IOP\ and .IRX;1 (.data strings). */
extern char xglCdIopModuleHead[];
extern char xglCdIopModuleTail[];
int func_002ED390(char *name, int args_length, const char *moduleArgs);

int xglCdSifLoadModule(const char *path, const char *moduleArgs)
{
    char name[CD_PATH_MAX];
    const char *stringCursor;
    char *nameCursor;
    int args_length;

    stringCursor = xglCdIopModuleHead;
    nameCursor = name;
    while ((*nameCursor = *stringCursor) != 0) {
        stringCursor++;
        nameCursor++;
    }
    for (stringCursor = path; *stringCursor != 0; stringCursor++, nameCursor++) {
        if (*stringCursor < 'a')
            *nameCursor = *stringCursor;
        else
            *nameCursor = *stringCursor - ('a' - 'A');
    }
    stringCursor = xglCdIopModuleTail;
    while ((*nameCursor = *stringCursor) != 0) {
        stringCursor++;
        nameCursor++;
    }
    args_length = 0;
    if (moduleArgs != 0) {
        const char *scan = moduleArgs;

        args_length = 1;
        if (scan[0] != 0 || scan[1] != 0) {
            for (;;) {
                scan++;
                args_length++;
                if (*scan == 0 && scan[1] == 0)
                    break;
            }
        }
    }
    return func_002ED390(name, args_length, moduleArgs);
}

/* The overlay number last loaded (gp-relative byte). */
extern unsigned char xglCdLoadedOverlay;
/* .data strings: "cdrom0:\\" and "OV00.OVL;1". */
extern char xglCdOverlayPathHead[];
extern char xglCdOverlayPathTail[];
void FlushCache(int operation);
void *memset(void *destination, int fillPattern, unsigned int size);
int func_002ED4F8(char *name, unsigned char *exec_data);

/* The overlay load area in the EE memory map and its size (xglCdLoadOverlay clears it). */
#define CD_OVERLAY_ADDRESS ((void *) 0xa80000)
#define CD_OVERLAY_BYTES 0x80000

void xglCdLoadOverlay(int overlay)
{
    char name[CD_PATH_MAX];
    unsigned char overlayExecInfo[16];
    const char *stringCursor;
    char *nameCursor;

    if (overlay == xglCdLoadedOverlay) {
        return;
    }
    xglCdLoadedOverlay = overlay;
    stringCursor = xglCdOverlayPathHead;
    nameCursor = name;
    while ((*nameCursor = *stringCursor) != 0) {
        stringCursor++;
        nameCursor++;
    }
    stringCursor = xglCdOverlayPathTail;
    while ((*nameCursor = *stringCursor) != 0) {
        stringCursor++;
        nameCursor++;
    }
    while (*nameCursor-- != '0') {
    }
    nameCursor[0] = overlay / 10 + '0';
    nameCursor[1] = overlay % 10 + '0';
    memset(CD_OVERLAY_ADDRESS, 0, CD_OVERLAY_BYTES);
    FlushCache(0);
    FlushCache(2);
    while (func_002ED4F8(name, overlayExecInfo) != 0) {
    }
    FlushCache(0);
    FlushCache(2);
}

static void xglCdArcInitSub0(const int *archive_entry, void *sectorBuffer, int sectors)
{
    int lbn;
    CdReadRetryPolicy readPolicy;

    sceCdSync(0);
    sceCdDiskReady(0);
    if (sectors == 0) {
        lbn = archive_entry[0];
        sectors = 1;
    } else {
        lbn = archive_entry[0] + 1;
    }
    readPolicy.spindle_control = 1;
    readPolicy.try_count = 0;
    readPolicy.data_pattern = 0;
    sceCdRead(lbn, sectors, sectorBuffer, &readPolicy);
    while (sceCdSync(1) > 0)
        ;
}

int xglCdArcInitSub1(CdArchiveEntry *archive_entry,
                            const char *path, unsigned char *archiveHeader)
{
    CdFilePosition file_position;
    int headerSectors;

    if (xglCdGetFilePos(&file_position, path, xglCdDummyCallback) == 0)
        return 0;

    archive_entry->archiveHeader = archiveHeader;
    archive_entry->lbn = file_position.lbn;
    archive_entry->readMedium = CD_SOURCE_DISC;

    /* xglCdArcInitSub0 only reads its `archive_entry` argument's leading
     * word as an LBN, so the file position's own lbn field (the one
     * evidenced member of CdFilePosition) is exactly what it needs. */
    xglCdArcInitSub0(&file_position.lbn, archiveHeader, 0);

    /* Sector 0 of the file is the archive header; its first byte is the
     * header's own sector count. */
    headerSectors = archiveHeader[0];
    if (headerSectors >= 2) {
        /* The header spans more than one sector: read the remaining
         * headerSectors - 1 sectors right after the first. */
        xglCdArcInitSub0(&file_position.lbn, archiveHeader + CD_SECTOR_BYTES,
                         headerSectors - 1);
    }
    return headerSectors + 1;
}

/*
 * Archive table-of-contents walk: archive_data is a header buffer filled by
 * xglCdArcInitSub1/xglCdArcInit, and this finds the byte right after the
 * table's last record so the caller can place fresh data there.
 *
 * Each record begins with a one-byte length/flags field:
 *   - ARC_RECORD_EXTENDED set: no fixed header past this byte.
 *   - ARC_RECORD_EXTENDED clear: a fixed ARC_RECORD_HEADER_BYTES-byte header
 *     follows, extended by ARC_RECORD_TYPE_EXTRA_BYTES more bytes when
 *     ARC_RECORD_TYPE_EXTENDED is also set.
 *   - ARC_RECORD_LENGTH_MASK (the low bits) is the record's payload length,
 *     walked past after the fixed header, if any.
 * A zero length/flags byte ends the table.
 */
unsigned char *xglCdArcInitSub2(unsigned char *archive_data)
{
    unsigned char *cursor = archive_data + 1;
    unsigned char recordLengthFlags = *cursor;
    u32 aligned;

    while (recordLengthFlags != 0) {
        int extendedFlag = recordLengthFlags & ARC_RECORD_EXTENDED;
        int typeExtended = recordLengthFlags & ARC_RECORD_TYPE_EXTENDED;
        if (extendedFlag == 0) {
            cursor += ARC_RECORD_HEADER_BYTES;
            if (typeExtended != 0)
                cursor += ARC_RECORD_TYPE_EXTRA_BYTES;
        }
        recordLengthFlags &= ARC_RECORD_LENGTH_MASK;
        cursor += recordLengthFlags;
        recordLengthFlags = *cursor;
    }

    /* Round up to the next ARC_TABLE_ALIGN_BYTES-byte boundary and write a
     * two-byte zero terminator marking the end of the table. */
    aligned = ((u32)cursor + ARC_TABLE_ALIGN_BYTES) & (u32)-ARC_TABLE_ALIGN_BYTES;
    cursor = (unsigned char *) aligned;
    cursor[1] = 0;
    cursor[0] = 0;
    return cursor;
}

void func_00187AB8(void);
extern unsigned char xglCdArchiveTocBuffer[];
extern char xglCdArchiveFileName00[];
extern char xglCdSlotThreeArchiveFileName[];
void xglCdArcInit(void);

void xglCdArcInit(void)
{
    CdArchiveStorage *archiveSlot;
    unsigned char *tocCursor;

    tocCursor = xglCdArchiveTocBuffer;
    LW.archive_init_active = 1;
    func_00187AB8();
    archiveSlot = xglCdArchiveSlots;
    if (xglCdArcInitSub1(&archiveSlot->archiveEntry, xglCdArchiveFileName00, tocCursor) != 0) {
        tocCursor = xglCdArcInitSub2(tocCursor);
        do {
            archiveSlot = archiveSlot + 1;
        } while (0);
        if (xglCdArcInitSub1(&archiveSlot->archiveEntry, xglCdArchiveFileName10, tocCursor) <= 0
            && xglCdArcInitSub1(&archiveSlot->archiveEntry, xglCdArchiveFileName20, tocCursor) <= 0) {
            archiveSlot->archiveEntry.readMedium = CD_SOURCE_UNUSED;
            tocCursor[0] = 0;
            tocCursor[1] = 0;
        }
        tocCursor = xglCdArcInitSub2(tocCursor);
        do {
            if (xglCdArcInitSub1(&D_0089B640.archiveEntry, xglCdSlotThreeArchiveFileName, tocCursor) <= 0) {
                D_0089B640.archiveEntry.readMedium = CD_SOURCE_UNUSED;
                tocCursor[0] = 0;
                tocCursor[1] = 0;
            }
        } while (0);
        xglCdArcInitSub2(tocCursor);
        LW.archive_init_active = 0;
    }
}

void xglCdArcReset(void)
{
    CdArchiveStorage *archiveSlot;

    archiveSlot = xglCdArchiveSlots;
    archiveSlot[0].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    archiveSlot[1].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    archiveSlot[2].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    archiveSlot[3].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    xglCdArcInit();
}

int sceClose(int descriptor);
int func_002F8288(void);
int func_0015D000(int handle, unsigned int lbn, int mode);
int func_0015CBF0(int handle, int sectors, void *buffer);
int func_002EC4C8(int size);
void func_002F7718(unsigned int mode);
void func_002F8490(int media);
void func_002F8738(int count, int size, int aligned_buffer);
extern unsigned char xglCdTocCopy[];

void xglCdInitial(void)
{
    int remaining;
    unsigned int iop_buffer;
    unsigned char *tocBufferCursor;
    unsigned char *tocCopyCursor;

    xglCdDeviceHandle = 0;
    LW.archive_init_active = 1;
    LW.spindle_control = 0;
    LW.search_layer = 0;
    LW.dual_layer_disc = 0;
    LW.power_off_pending = 0;
    LW.queuedReadDelay = 0;
    LW.disk_state = 0;
    LW.disk_flags = 0;
    func_002F7718(0);
    func_002F8490(2);
    iop_buffer = func_002EC4C8(0x20080);
    func_002F8738(0x40, 0x10, (iop_buffer + 127) & -128);
    xglCdReset();
    tocBufferCursor = xglCdArchiveTocBuffer;
    xglCdLoadedOverlay = 1;
    tocCopyCursor = xglCdTocCopy;
    xglCdArchiveSlots[0].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    xglCdArchiveSlots[1].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    xglCdArchiveSlots[2].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    xglCdArchiveSlots[3].archiveEntry.readMedium = CD_SOURCE_UNUSED;
    xglCdReadFile(xglCdSystemCnfName, tocBufferCursor, 0, CD_NOTIFY_DUMMY);
    for (remaining = 79; remaining >= 0; remaining--) {
        *tocCopyCursor++ = *tocBufferCursor++;
    }
    xglCdArcInit();
}
