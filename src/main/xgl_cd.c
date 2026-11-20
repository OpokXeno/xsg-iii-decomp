#include "common.h"
#include "shared.h"
#include "main/xgl_cd.h"

unsigned int xglClockDayTime2UInt(XglClock *clock_time);
void xglClockUInt2DayTime(XglClock *clock_time, unsigned int elapsed);

/* xglCdGetFilePos and xglCdGetFileData fill a 0x30-byte file-position record;
 * only its leading lbn word is evidenced by the callers in xgl_cd.c. */
typedef struct CdFilePosition {
    unsigned int lbn;
    int stored_bytes;
    unsigned char unmodeled_08[0x1c];
    unsigned char source;
    unsigned char unmodeled_25;
    unsigned char packed;
    unsigned char unmodeled_27;
    int descriptor;
    int size_bytes;
} CdFilePosition;

/* Where a file is read from (CdFilePosition.source, CdStreamParam.source). */
#define CD_SOURCE_DISC 0
#define CD_SOURCE_HOST 1
#define CD_SOURCE_HDD 2

typedef struct StreamXssBuffer StreamXssBuffer;

typedef struct StreamRing StreamRing;

typedef struct CdStreamParam CdStreamParam;

struct StreamRing {
    unsigned char *buffer;
    int capacity;
    int write_position;
    int read_position;
};

struct StreamXssBuffer {
    unsigned char *source;
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
    unsigned char source;       /* +0x05 */
    short block_count;
    int sector_position;        /* +0x08: advanced by StreamReadRingCoreSub */
    int block_size;             /* +0x0c: xglCdStreamRead pads reads to it */
    int descriptor;             /* +0x10 */
    int file_byte_count;        /* +0x14 */
    int remaining; /* +0x18: bytes not yet transferred into the stream rings. */
    int ring_size;
    StreamRing ring;            /* +0x20 */
    StreamRing alternate_ring;  /* +0x30 */
};

/* A CD sector is 2048 bytes (1 << CD_SECTOR_SHIFT). */
#define CD_SECTOR_SHIFT 11

int xglCdCheckFile(const char *path, void *record);

/* A callback gets a third argument: the current read tag from xglCdGetFilePos (lw $6 at
 * 0x19BA04), the request's argument from the read service. */
typedef void CdCompletionCallback(int event, int value, int tag);

/* xglCdReadFilePart's callback argument may instead select one: the current global callback,
 * the dummy or the default one (xglCdReadFile passes its notify argument through as it). */
#define CD_NOTIFY_CURRENT ((CdCompletionCallback *) 0)
#define CD_NOTIFY_DUMMY ((CdCompletionCallback *) 1)
#define CD_NOTIFY_DEFAULT ((CdCompletionCallback *) 2)
#define CD_READ_ASYNC 1
#define CD_READ_NO_EXTRACT 2
#define CD_READ_WHILE_BUSY 4

int xglCdReadFilePart(const char *name, void *buffer, int mode,
                      CdCompletionCallback *notify, int offset, int length);
int xglCdReadFile(const char *name, void *buffer, int mode, CdCompletionCallback *notify);
void xglCdOpenDevice(void);
int xglCdServiceIfPending(void);
signed char xglCdGetServicePending(void);
void xglCdSetReadTag(int value);
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

typedef struct CdReadMode {
    unsigned char try_count;
    unsigned char spindle_control;
    unsigned char data_pattern;
} CdReadMode;

/* Only the fields xglCdArcInitSub1 writes are evidenced here. */
typedef struct CdArchiveEntry {
    unsigned char source;   /* CD_SOURCE_*, or CD_SOURCE_UNUSED */
    unsigned char unmodeled_01;
    unsigned char packed;
    unsigned char unmodeled_03;
    unsigned char *destination;
    int lbn;
    int file_lbn_offset;
    int stored_bytes;
    int size_bytes;
} CdArchiveEntry;

/* Events passed to a completion callback (int event, int value). */
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
/* Keep the direct state expression used by both queue loops. */
#define CD_READ_CONTROL (&LW)
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
/* CdArchiveEntry.source of an archive slot that holds nothing. */
#define CD_SOURCE_UNUSED 0xff
/* CdReadControl.dispatch_state: what the control thread is doing. */
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
    void *buffer;
    int offset;
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
typedef struct CdReadControl {
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
    signed char dispatch_state;                  /* +0x30 */
    u8 spindle_control;                          /* +0x31 */
    u8 search_layer;                             /* +0x32 */
    u8 dual_layer_disc;                          /* +0x33 */
    u8 power_off_pending;                        /* +0x34 */
    u8 archive_init_active;                      /* +0x35: 1 while xglCdArcInit rebuilds the archive slots */
    u8 unmodeled_036[0x8];                       /* +0x36..0x3d */
    u8 queue_cursor;                             /* +0x3e */
    u8 queue_tail;                               /* +0x3f */
    CdRequestPartial requests[CD_QUEUE_SIZE];    /* +0x40..0x103f */
    u8 unmodeled_1040[0x3000];                   /* +0x1040..0x403f: the rest of the work area, up to xglCdStreamService */
} CdReadControl;

#define CD_STREAM_LIST_SIZE 2

typedef struct CdStreamService {
    unsigned short turn_countdown;  /* +0x00 */
    unsigned short turn_period;     /* +0x02 */
    unsigned char current;          /* +0x04 */
    unsigned char unmodeled_05[0xb];
    CdStreamParam *list[CD_STREAM_LIST_SIZE]; /* +0x10 */
} CdStreamService;

/* Four 0x20-byte archive slots (slot n at +0x20 * n); the table that follows them at
 * D_0089B660 is cleared by func_00187AB8. */
typedef union CdArchiveStorage {
    CdArchiveEntry entry;
    unsigned char bytes[0x20];
} CdArchiveStorage;

extern int sceOpen(const char *path, int flags, ...);
extern int sceCdSync(int mode);
extern int sceCdDiskReady(int mode);
extern int sceCdRead(int lbn, int sectors, void *destination, CdReadMode *read_mode);
int BCD2INT(unsigned char bcd);
void extract(u32 *destination, u32 *source, int byte_count);
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
extern CdCompletionCallback *callback;
extern int xglCdReadTag;
extern CdReadControl LW;
/* The stream service state at 0x959CC0 (its list at +0x10 is the symbol xglCdStreamList). */
extern CdStreamService xglCdStreamService;
extern CdArchiveStorage D_0089B5E0[4];
void xglCdDefaultCallback(int event, int value, int tag);
void xglCdReset(void);
int ADXF_StopNw(int handle);
int ADXF_Stop(int handle);
int sceCdBreak(void);
int sceCdPause(void);
void xglCdDummyCallback(int event, int value, int tag);
int xglCdGetFilePosSub(CdArchiveEntry *entry, const char *path);
/* Locates a file: in the archive slots first (the path then becomes the archive's own file name), * else by its name on the disc or the host. Fills the position record and returns it, or 0 when the * file is missing. The callback gets CD_EVENT_LOOKUP with the path and the current read tag. */ CdFilePosition *xglCdGetFilePos(CdFilePosition *position, const char *name, CdCompletionCallback *notify);
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
/* One step of the active asynchronous read. A disc read reports its position and, once the drive * (or the device handle, when one is open) is done, unpacks the data and starts the next queued * request. A host or HDD read transfers up to CD_FILE_CHUNK_BYTES; after the last chunk it pads * the sector, closes the file, unpacks and starts the next request. The progress callback is * skipped while xglCdServiceActive is set; the function sets it on return. */ void xglCdServiceRead(void);
int ADXF_Open(const unsigned char *table, int mode);
extern const unsigned char xglCdDeviceName[];

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
    } part;
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

void extract(u32 *destination, u32 *source, int byte_count)
{
    if (destination != source) {
        while (byte_count > 0) {
            *destination++ = *source++;
            byte_count -= 4;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_001870E8);

int func_001870E8(void);

int queue_next(void)
{
    CD_READ_CONTROL->completion_callback(CD_EVENT_DONE,
                                         (CD_READ_CONTROL->queue_tail
                                          - CD_READ_CONTROL->queue_cursor) & CD_QUEUE_INDEX_MASK,
                                         CD_READ_CONTROL->callback_argument);
    CD_READ_CONTROL->dispatch_state = CD_STATE_IDLE;
    return func_001870E8();
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
                    *LW.write_cursor++ = (unsigned char) remaining_padding;
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

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", xglCdControlThread);

/* One BCD byte: high nibble tens, low nibble units. */
int BCD2INT(unsigned char bcd)
{
    return (bcd >> 4) * 10 + (bcd & 15);
}

/* The console's clock record: D_00897580 holds the BCD fields read below. */
extern unsigned char D_00897580[];
/* Small-data flag: a positive value means the clock was read already; the first
 * read stores 10 when the console reports 1. */
extern signed char D_0055157C;
int func_002F8588(unsigned char *clock_record);

int xglClockReadBcd(XglClock *clock_time)
{
    int result = 0;
    unsigned char second;

    if (D_0055157C <= 0) {
        result = func_002F8588(D_00897580) - 1;
        if (result == 0) {
            D_0055157C = 10;
        }
    }
    clock_time->year = BCD2INT(D_00897580[7]) + CLOCK_BASE_YEAR;
    clock_time->month = BCD2INT(D_00897580[6]);
    clock_time->day = BCD2INT(D_00897580[5]);
    clock_time->hour = BCD2INT(D_00897580[3]);
    clock_time->minute = BCD2INT(D_00897580[2]);
    second = BCD2INT(D_00897580[1]);
    clock_time->status = 0;
    clock_time->second = second;
    return result;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", xglClockRead);

/*
 * Converts a calendar record to the number of seconds elapsed since
 * 2000-01-01 00:00:00.  monthday[12..23] holds the cumulative day offset of
 * each month; the leap day of the current year counts from March onwards,
 * and ((year - 1997) >> 2) counts the leap days of the whole years before it.
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
           + ((year - 1997) >> 2) * CLOCK_SECONDS_PER_DAY);
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

extern char D_005401B0[];
extern char D_005401B8[];
extern char D_005401C0[];
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
    unsigned char *text;
    unsigned char state;
    int result;
    int status;
    int size;

    state = LW.disk_state;
    if (state != CD_DISK_STATE_IDENTIFIED) {
        status = func_002F8320();
        if (status == CD_STATUS_SHELL_OPEN) {
            LW.disk_state = CD_DISK_STATE_UNKNOWN;
            return CD_CHECK_SHELL_OPEN;
        }
        result = sceCdDiskReady(1);
        if (result != CD_DISK_READY_COMPLETE) {
            if (LW.disk_state == CD_DISK_STATE_TRAY_CHANGED
                || (status == 0 && result == CD_DISK_READY_NOT_READY)) {
                return CD_CHECK_SHELL_OPEN;
            }
            LW.disk_state = CD_DISK_STATE_UNKNOWN;
            return CD_CHECK_NOT_READY;
        }
        LW.disk_state = CD_DISK_STATE_UNKNOWN;
        result = func_002F8260();
        if (result == CD_DISC_NONE) {
            return 0;
        }
        if (result != CD_DISC_PS2_DVD) {
            if (result >= CD_DISC_DETECT_END) {
                return CD_CHECK_UNSUPPORTED;
            }
            return CD_CHECK_NOT_READY;
        }
        LW.archive_init_active = 1;
        LW.disk_flags = 0;
        size = xglCdGetFileSize(D_005401B0);
        if (size > 0 && size <= CD_ARC_HEADER_RESERVE) {
            LW.disk_flags |= CD_DISC_FLAG_ARC10;
        }
        size = xglCdGetFileSize(D_005401B8);
        if (size > 0 && size <= CD_ARC_HEADER_RESERVE) {
            LW.disk_flags |= CD_DISC_FLAG_ARC20;
        }
        LW.archive_init_active = 0;
        if (LW.disk_flags == 0) {
            return CD_CHECK_UNSUPPORTED;
        }
        text = (unsigned char *) 0x00A7F800;
        while (xglCdServiceIfPending() != 0) {
        }
        LW.archive_init_active = 1;
        xglCdReadFilePart(D_005401C0, text, 0, CD_NOTIFY_DUMMY, 0, 2048);
        LW.archive_init_active = 0;
        while (*text++ != '\\') {
        }
        if (text[2] != 'U') {
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
void xglCdDefaultCallback(int event, int value, int tag)
{
}

void xglCdDummyCallback(int event, int value, int tag)
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
    xglCdStreamService.current = 0;
    for (i = 0; i < CD_STREAM_LIST_SIZE; i++) {
        xglCdStreamService.list[i] = 0;
    }
    LW.dispatch_state = CD_STATE_IDLE;
    callback = xglCdDefaultCallback;
    sceCdPause();
    sceCdSync(0);
}

CdCompletionCallback *xglCdSetCallback(int callback_address)
{
    CdCompletionCallback *old_callback = callback;

    switch (callback_address) {
    case CD_CALLBACK_DEFAULT:
        callback = xglCdDefaultCallback;
        break;
    case CD_CALLBACK_DUMMY:
        callback = xglCdDummyCallback;
        break;
    case CD_CALLBACK_KEEP:
        break;
    default:
        callback = (CdCompletionCallback *)callback_address;
        break;
    }
    return old_callback;
}

/* Sets the current read tag xglCdReadTag. */
void xglCdSetReadTag(int value)
{
    xglCdReadTag = value;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00187AB8);

/* Expands one name record of an archive table into `name` and returns the name's end. A record starts with
 * a count byte. Zero: the bytes after it, up to a terminator, replace the name. Otherwise the low
 * nibble is the number of bytes copied over the name, starting (high nibble + 1) characters
 * before its end; the rest of the name stays. */
char *xglCdArcNameExpand(char *name, const unsigned char *record)
{
    unsigned char count;
    int back;

    count = *record++;
    if (count == 0) {
        while ((*name++ = *record++) != 0) {
        }
    } else {
        back = (count >> 4) + 1;
        count &= 0xF;
        while (*name != 0) {
            name++;
        }
        name -= back;
        while (count != 0) {
            *name++ = *record++;
            count--;
        }
        name--;
        while (*(unsigned char *) name++ != 0) {
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
extern char D_00551188[8];
extern CdArchiveStorage D_0089B600;
extern const char D_005401D0[32];
/* Unsized: the original addresses these strings absolutely, not through $gp. */
extern const char D_005401F0[];
extern const char D_004B35A0[];
extern const char D_004B35A8[];
extern const char D_004B35B0[];

/* Locates a file: in the archive slots first (the path then becomes the archive's own file name),
 * else by its name on the disc or the host. Fills the position record and returns it, or 0 when the
 * file is missing. The callback gets CD_EVENT_LOOKUP with the path and the current read tag. */
CdFilePosition *xglCdGetFilePos(CdFilePosition *position, const char *name, CdCompletionCallback *notify)
{
    char buffer[CD_PATH_MAX];
    const char *path = name;
    CdArchiveEntry *entry;
    CdArchiveStorage *slot;
    const char *cursor;
    const char *absolute_source;
    char *absolute_destination;
    char *destination;
    char *scan;
    int descriptor;
    int size;

    if (path[0] == 'd' && path[1] == 'a' && path[2] == 't' && path[3] == 'a' && path[4] == '\\') {
        printf(D_005401D0, name);
        name = path + CD_DATA_PREFIX_LENGTH;
        printf(D_005401F0, name);
    }
    position->source = 0;
    if (name[0] != '\\') {
        for (scan = (char *) name; *scan != 0; scan++) {
            if (*scan == '/') {
                *scan = '\\';
            }
        }
    }
    do {
        entry = &D_0089B5E0[0].entry;
        if (entry->source != CD_SOURCE_UNUSED && func_00187E28(entry, name) != 0) {
            name = D_004B35A0;
            position->source = entry->source;
            break;
        }
        slot = &D_0089B600;
        entry = &slot->entry;
        if (entry->source != CD_SOURCE_UNUSED && func_00187E28(entry, name) != 0) {
            name = D_004B35A8;
            position->source = entry->source;
        } else {
            slot = slot + 2;
            entry = &slot->entry;
            if (entry->source != CD_SOURCE_UNUSED && func_00187E28(entry, name) != 0) {
                name = D_004B35B0;
                position->source = entry->source;
            } else {
                entry = 0;
            }
        }
    } while (0);
    if (name[0] == '\\') {
        absolute_source = name + 1;
        absolute_destination = buffer;
        while ((*absolute_destination = *absolute_source) != 0) {
            absolute_source++;
            absolute_destination++;
        }
        position->source = CD_SOURCE_HOST;
    } else {
        destination = buffer;
        switch (position->source) {
        case CD_SOURCE_DISC:
            buffer[0] = '\\';
            destination = buffer + 1;
            for (cursor = name; *cursor != 0; cursor++, destination++) {
                if (*cursor < 'a' || *cursor > 'z') {
                    *destination = *cursor;
                } else {
                    *destination = *cursor - ('a' - 'A');
                }
            }
            destination[0] = ';';
            destination[1] = '1';
            destination[2] = 0;
            break;
        case CD_SOURCE_HOST:
        case CD_SOURCE_HDD:
            cursor = D_00551188;
            while ((*destination = *cursor) != 0) {
                cursor++;
                destination++;
            }
            cursor = name;
            for (;;) {
                if (*cursor == '\\') {
                    *destination = '/';
                } else {
                    *destination = *cursor;
                }
                if (*cursor == 0) {
                    break;
                }
                cursor++;
                destination++;
            }
            break;
        }
    }
    notify(CD_EVENT_LOOKUP, (int) path, xglCdReadTag);
    switch (position->source) {
    case CD_SOURCE_DISC:
        if (entry != 0) {
            position->lbn = entry->lbn + entry->file_lbn_offset;
            position->stored_bytes = entry->stored_bytes;
            position->packed = entry->packed;
            position->size_bytes = entry->size_bytes;
            break;
        }
        if (LW.archive_init_active == 0) {
            return 0;
        }
        sceCdSync(0);
        sceCdDiskReady(0);
        if (func_002F71F0(position, buffer) == 0) {
            return 0;
        }
        position->packed = 1;
        position->size_bytes = position->stored_bytes;
        break;
    case CD_SOURCE_HOST:
        descriptor = func_002E9518(buffer, 1);
        if (descriptor < 0) {
            return 0;
        }
        if (entry != 0) {
            position->lbn = entry->lbn + entry->file_lbn_offset;
            position->stored_bytes = entry->stored_bytes;
            position->packed = entry->packed;
            position->size_bytes = entry->size_bytes;
        } else {
            position->lbn = 0;
            position->packed = 0;
            size = func_002E9920(descriptor, 0, SCE_SEEK_END);
            position->stored_bytes = size;
            if (position->packed == 0) {
                position->size_bytes = size;
            }
        }
        position->descriptor = descriptor;
        break;
    case CD_SOURCE_HDD:
        return 0;
    }
    return position;
}

/* Probes a path: -1 when missing, closes the descriptor of a host or HDD file
 * and returns 0 otherwise. */
int xglCdCheckFile(const char *path, void *record)
{
    CdFilePosition *position = record;

    if (xglCdGetFilePos(position, path, xglCdDummyCallback) == 0) {
        return -1;
    }
    switch (position->source) {
    case CD_SOURCE_DISC:
        break;
    case CD_SOURCE_HOST:
    case CD_SOURCE_HDD:
        sceClose(position->descriptor);
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
    return position.size_bytes;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", xglCdReadFilePart);

int xglCdReadFile(const char *name, void *buffer, int mode, CdCompletionCallback *notify)
{
    return xglCdReadFilePart(name, buffer, mode, notify, 0, -1);
}

/* Cancels the active read: clears pending requests, breaks or pauses a sync
 * read, closes an open descriptor and goes idle. */
void xglCdReadCancel(int immediate)
{
    LW.queue_tail = LW.queue_cursor;
    if (LW.dispatch_state == CD_STATE_SYNC_READ) {
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
    } else if (LW.dispatch_state == CD_STATE_ASYNC_READ) {
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
    LW.dispatch_state = CD_STATE_IDLE;
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

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00188CF8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00188D18);

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00188D28);

INCLUDE_ASM("asm/main/nonmatchings/xgl_cd", func_00188D38);

/* The head and the tail of an IOP module path: cdrom0:\IOP\ and .IRX;1 (.data strings). */
extern char D_004B35B8[];
extern char D_004B35C8[];
int func_002ED390(char *name, int args_length, const char *args);

int xglCdSifLoadModule(const char *path, const char *args)
{
    char name[CD_PATH_MAX];
    const char *src;
    char *dst;
    int args_length;

    src = D_004B35B8;
    dst = name;
    while ((*dst = *src) != 0) {
        src++;
        dst++;
    }
    for (src = path; *src != 0; src++, dst++) {
        if (*src < 'a')
            *dst = *src;
        else
            *dst = *src - ('a' - 'A');
    }
    src = D_004B35C8;
    while ((*dst = *src) != 0) {
        src++;
        dst++;
    }
    args_length = 0;
    if (args != 0) {
        const char *scan = args;

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
    return func_002ED390(name, args_length, args);
}

/* The overlay number last loaded (gp-relative byte). */
extern unsigned char D_0055157E;
/* .data strings: "cdrom0:\\" and "OV00.OVL;1". */
extern char D_004B35D0[];
extern char D_004B35E0[];
void FlushCache(int operation);
void *memset(void *destination, int value, unsigned int size);
int func_002ED4F8(char *name, unsigned char *exec_data);

void xglCdLoadOverlay(int overlay)
{
    char name[CD_PATH_MAX];
    unsigned char exec_data[16];
    const char *src;
    char *dst;

    if (overlay == D_0055157E) {
        return;
    }
    D_0055157E = overlay;
    src = D_004B35D0;
    dst = name;
    while ((*dst = *src) != 0) {
        src++;
        dst++;
    }
    src = D_004B35E0;
    while ((*dst = *src) != 0) {
        src++;
        dst++;
    }
    while (*dst-- != '0') {
    }
    dst[0] = overlay / 10 + '0';
    dst[1] = overlay % 10 + '0';
    memset((void *) 0xa80000, 0, 0x80000);
    FlushCache(0);
    FlushCache(2);
    while (func_002ED4F8(name, exec_data) != 0) {
    }
    FlushCache(0);
    FlushCache(2);
}

static void xglCdArcInitSub0(const int *archive_entry, void *destination, int sectors)
{
    int lbn;
    CdReadMode read_mode;

    sceCdSync(0);
    sceCdDiskReady(0);
    if (sectors == 0) {
        lbn = archive_entry[0];
        sectors = 1;
    } else {
        lbn = archive_entry[0] + 1;
    }
    read_mode.spindle_control = 1;
    read_mode.try_count = 0;
    read_mode.data_pattern = 0;
    sceCdRead(lbn, sectors, destination, &read_mode);
    while (sceCdSync(1) > 0)
        ;
}

int xglCdArcInitSub1(CdArchiveEntry *archive_entry,
                            const char *path, unsigned char *destination)
{
    CdFilePosition file_position;
    int header_count;

    if (xglCdGetFilePos(&file_position, path, xglCdDummyCallback) == 0)
        return 0;

    archive_entry->destination = destination;
    archive_entry->lbn = file_position.lbn;
    archive_entry->source = CD_SOURCE_DISC;

    /* xglCdArcInitSub0 only reads its `archive_entry` argument's leading
     * word as an LBN, so the file position's own lbn field (the one
     * evidenced member of CdFilePosition) is exactly what it needs. */
    xglCdArcInitSub0(&file_position.lbn, destination, 0);

    /* Sector 0 of the file is the archive header; its first byte is the
     * header's own sector count. */
    header_count = destination[0];
    if (header_count >= 2) {
        /* The header spans more than one sector: read the remaining
         * header_count - 1 sectors right after the first. */
        xglCdArcInitSub0(&file_position.lbn, destination + CD_SECTOR_BYTES,
                         header_count - 1);
    }
    return header_count + 1;
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
    unsigned char value = *cursor;

    while (value != 0) {
        int high_bit = value & ARC_RECORD_EXTENDED;
        int type_flag = value & ARC_RECORD_TYPE_EXTENDED;
        if (high_bit == 0) {
            cursor += ARC_RECORD_HEADER_BYTES;
            if (type_flag != 0)
                cursor += ARC_RECORD_TYPE_EXTRA_BYTES;
        }
        value &= ARC_RECORD_LENGTH_MASK;
        cursor += value;
        value = *cursor;
    }

    /* Round up to the next ARC_TABLE_ALIGN_BYTES-byte boundary and write a
     * two-byte zero terminator marking the end of the table. */
    cursor = (unsigned char *)(((u32)cursor + ARC_TABLE_ALIGN_BYTES)
        & (u32)-ARC_TABLE_ALIGN_BYTES);
    cursor[1] = 0;
    cursor[0] = 0;
    return cursor;
}

void func_00187AB8(void);
extern unsigned char D_00867580[];
extern char D_005401F8[];
extern char D_00540200[];
void xglCdArcInit(void);

void xglCdArcInit(void)
{
    CdArchiveStorage *slot;
    unsigned char *work;

    work = D_00867580;
    LW.archive_init_active = 1;
    func_00187AB8();
    slot = D_0089B5E0;
    if (xglCdArcInitSub1(&slot->entry, D_005401F8, work) != 0) {
        work = xglCdArcInitSub2(work);
        do {
            slot = slot + 1;
        } while (0);
        if (xglCdArcInitSub1(&slot->entry, D_005401B0, work) <= 0
            && xglCdArcInitSub1(&slot->entry, D_005401B8, work) <= 0) {
            slot->entry.source = CD_SOURCE_UNUSED;
            work[0] = 0;
            work[1] = 0;
        }
        work = xglCdArcInitSub2(work);
        do {
            if (xglCdArcInitSub1(&D_0089B640.entry, D_00540200, work) <= 0) {
                D_0089B640.entry.source = CD_SOURCE_UNUSED;
                work[0] = 0;
                work[1] = 0;
            }
        } while (0);
        xglCdArcInitSub2(work);
        LW.archive_init_active = 0;
    }
}

void xglCdArcReset(void)
{
    CdArchiveStorage *slot;

    slot = D_0089B5E0;
    slot[0].entry.source = CD_SOURCE_UNUSED;
    slot[1].entry.source = CD_SOURCE_UNUSED;
    slot[2].entry.source = CD_SOURCE_UNUSED;
    slot[3].entry.source = CD_SOURCE_UNUSED;
    xglCdArcInit();
}

int sceClose(int descriptor);
int func_002F8288(void);
int func_0015D000(int handle, unsigned int lbn, int mode);
int func_0015CBF0(int handle, int sectors, void *buffer);
int func_002EC4C8(int size);
void func_002F7718(int arg);
void func_002F8490(int arg);
void func_002F8738(int arg0, int arg1, int aligned_buffer);
extern unsigned char D_004B35F0[];

void xglCdInitial(void)
{
    int remaining;
    unsigned int iop_buffer;
    unsigned char *src;
    unsigned char *dst;

    xglCdDeviceHandle = 0;
    LW.archive_init_active = 1;
    LW.spindle_control = 0;
    LW.search_layer = 0;
    LW.dual_layer_disc = 0;
    LW.power_off_pending = 0;
    LW.unmodeled_036[0] = 0;
    LW.disk_state = 0;
    LW.disk_flags = 0;
    func_002F7718(0);
    func_002F8490(2);
    iop_buffer = func_002EC4C8(0x20080);
    func_002F8738(0x40, 0x10, (iop_buffer + 127) & -128);
    xglCdReset();
    src = D_00867580;
    D_0055157E = 1;
    dst = D_004B35F0;
    D_0089B5E0[0].entry.source = CD_SOURCE_UNUSED;
    D_0089B5E0[1].entry.source = CD_SOURCE_UNUSED;
    D_0089B5E0[2].entry.source = CD_SOURCE_UNUSED;
    D_0089B5E0[3].entry.source = CD_SOURCE_UNUSED;
    xglCdReadFile(D_005401C0, src, 0, (CdCompletionCallback *)1);
    for (remaining = 79; remaining >= 0; remaining--) {
        *dst++ = *src++;
    }
    xglCdArcInit();
}
