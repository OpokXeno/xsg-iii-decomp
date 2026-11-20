#include "common.h"
#include "shared.h"
#include "main/xgl_cd.h"

unsigned int xglClockDayTime2UInt(XglClock *clock_time);

/* One request of the xglMc queue: the file number, its buffer and size, and
 * a callback the list and old-data scans call per entry (with the result,
 * and the entry bytes for the old-data check).  The callback is unprototyped
 * because the callers in yajima_test.c differ: DebugCallbackLog takes the result
 * only, McOldDataCheckCallback the result and the entry. */
typedef struct McRequestParam {
    int file;
    void *data;
    int size;
    int (*callback)();
} McRequestParam;

void xglMcReset(void);
char * xglMcSetFullPath(int card, int selector);
int xglMcMain(void);
int xglMcRequest(int port, int command, McRequestParam *param, int *result);

/* xglMcRequest commands (the callers in yajima_test.c use the same values). */
#define MC_REQ_FREE_SPACE 1
#define MC_REQ_EXIST 2
#define MC_REQ_LIST 3
#define MC_REQ_LOAD 4
#define MC_REQ_SAVE 5
#define MC_REQ_CREATE 6
#define MC_REQ_FORMAT 7
#define MC_REQ_CHECK_OLD 8
#define MC_REQ_UNFORMAT 9
/* The request queue holds 16 entries. */
#define MC_QUEUE_MASK 0xF

/* One queued request (0x20 bytes, xglMcQueue). */
typedef struct McQueueEntry {
    short port;               /* +0x00 */
    short command;            /* +0x02 */
    int *result;              /* +0x04 */
    int file;                 /* +0x08 */
    int data;                 /* +0x0C: the buffer address */
    int size;                 /* +0x10 */
    int (*callback)(int code, void *work);     /* +0x14 */
    unsigned char unmodeled_18[8];
} McQueueEntry;

extern unsigned char xglMcQueueTop;
extern unsigned char xglMcQueueEnd;
extern McQueueEntry xglMcQueue[16];

/* The icon.sys contents the create request keeps (sizeof(sceMcIconSys) bytes, byte aligned). */
typedef struct McIconSys {
    unsigned char bytes[0x3C4];
} McIconSys;

/* Memory-card state block at xglMcState. */
typedef struct XglMcState {
    unsigned char state;            /* +0x00: phase of the request state machine */
    unsigned char substate;         /* +0x01: step inside the current state, cleared per request */
    unsigned char unmodeled_02[1];
    unsigned char next_card;        /* +0x03: bit 0 is the card the next status poll asks about */
    union {
        int combined;               /* +0x04: the status word xglMcGetState returns */
        short slots[2];             /* +0x04: one MC_STATUS_* halfword per card */
    } card_status;
    int descriptor;                 /* +0x08: the open file, or the card of a status poll */
    int card_type;                  /* +0x0C: sceMcGetInfo type */
    unsigned char unmodeled_10[4];
    int formatted;                  /* +0x14: sceMcGetInfo format flag */
    int create_index;               /* +0x18: progress of the create request */
    unsigned char unmodeled_1c[1];
    unsigned char selector;         /* +0x1D: selector for the card search */
    unsigned char card;             /* +0x1E: card index chosen by the search */
    unsigned char status;           /* +0x1F */
    char full_path[0x60];           /* +0x20: full path of the current request (xglMcBuildPath) */
    McQueueEntry request;           /* +0x80: the request being worked on; the card search reuses its
                                     * file number as the search mode (0..2) and data as the work address */
    McIconSys icon;                 /* +0xA0: icon.sys contents kept by the create request */
} XglMcState;

extern XglMcState xglMcState;
/* xglMcState.card_status.combined (+0x04) under its own data label. xglMcGetState and the final
 * return of xglMcMain read it through this label; the early return of xglMcMain (no card operation
 * finished, the queue empty) reads the field. Both were checked: the label in the early return
 * changes four bytes, the field in the final return shortens the function. */
extern int xglMcStatusWord[];
extern char xglMcWildcardName[];
extern const char xglMcBasePath[];

/* Card numbers xglMcBuildPath does not print as digits. */
#define MC_CARD_NONE (-2)
#define MC_CARD_ANY (-1)
/* Selectors of xglMcBuildPath; other values point to a file name. */
#define MC_PATH_REPEAT 0
#define MC_PATH_END (-1)

/* File names of selectors 1..5: /icon.sys, /system.ico, /copy.ico, /delete.ico, /dummy.bin. */
extern const char *xglMcFileNames[9];

/* xglMcState.state while a request runs (set by xglMcExecute). */
#define MC_STATE_OPEN_CLOSE 2
#define MC_STATE_WRITE 3
#define MC_STATE_READ 4
#define MC_STATE_CREATE 5
#define MC_STATE_EXIST_SCAN 6
#define MC_STATE_LIST 7
#define MC_STATE_CHECK_OLD 8
/* sceMcOpen modes and the directory read size. */
#define MC_OPEN_READ 1
#define MC_OPEN_WRITE_CREATE 0x202
#define MC_DIRECTORY_MAX_ENTRIES 100
/* Entries of a list / check-old work buffer's slot table (McListWork.slot). */
#define MC_SLOT_COUNT 100
/* sceMcGetDir writes its entry table into the request's work buffer, behind the
 * MC_SLOT_COUNT slot bytes and rounded up to the 64-byte alignment the SDK needs. */
#define MC_DIRECTORY_TABLE(work) (((work) + MC_SLOT_COUNT + 0x3F) & ~0x3F)

int sceMcGetInfo(int port, int slot, int *type, int *free, int *format);
int sceMcMkdir(int port, int slot, const char *name);
int sceMcGetDir(int port, int slot, const char *name, int mode, int max_entries, int table);
int sceMcOpen(int port, int slot, const char *name, int mode);
/* Save folders of the check-old search, one per search mode (xglMcState.request.file):
 * /BASLUS-20469Xeno, /BISLPS-29005Xeno, /BISLPS-25353. This game's own folder is
 * xglMcBasePath, /BASLUS-20892Xeno2. */
extern char *xglMcBasePaths[];
int sceMcFormat(int port, int slot);
int sceMcUnformat(int port, int slot);
extern const char xglMcBasePath[24];
void xglMcExecute(McQueueEntry *request);

/* The create request's buffer: a table of file offsets (file k spans offset[k]..offset[k + 1],
 * each start rounded up to 64 bytes, relative to the end of the rounded header size offset[0]),
 * then at +0x40 the icon.sys contents. */
typedef struct McCreateData {
    int offset[16];
    McIconSys icon;
} McCreateData;

/* Steps of the create request (xglMcState.substate). From MC_CREATE_FILES_START on, each file
 * takes three steps: open, write, close; step / 3 + 1 is its xglMcBuildPath selector
 * (2 system.ico, 3 copy.ico, 4 delete.ico, 5 dummy.bin). */
#define MC_CREATE_DELETE_SAVE 0
#define MC_CREATE_DELETE_DUMMY 1
#define MC_CREATE_COPY_ICON 2
#define MC_CREATE_FILES_START 3 /* no card operation; the delete steps before it accept MC_RESULT_NO_ENTRY */
#define MC_CREATE_OPEN_SYSTEM_ICO 4
#define MC_CREATE_WRITE_SYSTEM_ICO 5
#define MC_CREATE_CLOSE_SYSTEM_ICO 6
#define MC_CREATE_OPEN_COPY_ICO 7
#define MC_CREATE_WRITE_COPY_ICO 8
#define MC_CREATE_CLOSE_COPY_ICO 9
#define MC_CREATE_OPEN_DELETE_ICO 10
#define MC_CREATE_WRITE_DELETE_ICO 11
#define MC_CREATE_CLOSE_DELETE_ICO 12
#define MC_CREATE_OPEN_DUMMY 13
#define MC_CREATE_DUMMY_WRITE 14
#define MC_CREATE_LAST_CLOSE 15
#define MC_CREATE_STEPS_PER_FILE 3
/* The save folder is filled up to this many bytes by the dummy file. */
#define MC_CREATE_TOTAL_BYTES 0x42400
/* sceMc result the delete steps accept: the file did not exist. */
#define MC_RESULT_NO_ENTRY (-4)

/* xglMcState.icon (+0xA0, 0x959FD8) under its own data label: the copy goes through the label.
 * Through the field it reuses the xglMcState base register and does not match. */
extern McIconSys xglMcIconSys;
int sceMcDelete(int port, int slot, const char *name);
int sceMcClose(int descriptor);
int sceMcWrite(int descriptor, const void *buffer, int size);
int xglMcCreateStep(int result);

#define MC_SLOT_WANTED 1 /* the value the search looks for */
/* McListWork.latest: the card number with the newest time stamp, behind the slot table (X1). */
#define MC_LATEST_SLOT_INDEX MC_SLOT_COUNT

int xglMcStartDirectoryScan(void);
int xglMcOpenNearestSave(void);

/* Result of xglMcMain / the card status word while a request is running. */
#define MC_STATUS_BUSY_BOTH 0xFF00FF
/* Per-card status halfwords (xglMcState.card_status.slots). */
#define MC_STATUS_NONE 0
#define MC_STATUS_READY 1
#define MC_STATUS_UNFORMATTED 2
#define MC_STATUS_ERROR 0x80
#define MC_STATUS_PS1_CARD 0x81
#define MC_STATUS_POCKETSTATION 0x82
/* sceMcGetInfo card types. */
#define MC_TYPE_PS1 1
#define MC_TYPE_PS2 2
#define MC_TYPE_POCKETSTATION 3
#define MC_STATE_IDLE 0
#define MC_STATE_POLL_INFO 1
/* Steps of a list / check-old scan (xglMcState.substate). */
#define MC_SCAN_DIRECTORY 0
#define MC_SCAN_NEXT_CARD 1
#define MC_SCAN_READ 2
#define MC_SCAN_REPORT 3
#define MC_SCAN_ICON_CHECK 4
/* McListWork.status values. */
#define MC_LIST_FOUND 1
#define MC_LIST_FAILED 2
/* Codes the scan callback gets instead of a card number. */
#define MC_CALLBACK_DIRECTORY (-1)
#define MC_CALLBACK_NO_DATA (-2)
#define MC_CALLBACK_ERROR (-3)
/* sceMc results below this one are hard errors for a scan. */
#define MC_RESULT_SOFT_LIMIT (-9)
/* Bytes read from a save of the other save folders (check-old). */
#define MC_OLD_SAVE_BYTES 0x163B0
/* Bytes of an old save the check-old scan reports, relative to the read data. */
#define MC_OLD_CLEAR_FLAG 0x10250
#define MC_OLD_CLEAR_VALUE 10
#define MC_OLD_INFO_1 0x10418
#define MC_OLD_INFO_2 0x10598
#define MC_OLD_INFO_3 0x10718
#define MC_OLD_INFO_5 0x10A18
#define MC_OLD_INFO_6 0x10B98
#define MC_OLD_INFO_7 0x10D18
#define MC_OLD2_FLAG 0xA4
#define MC_OLD2_INFO_1 0x50

/* The work buffer of a list / check-old request (request.data). */
typedef struct McListWork {
    unsigned char slot[100];   /* +0x00: per card number: 1 present, 2 read */
    unsigned char latest;      /* +0x64: card number with the newest time stamp */
    unsigned char status;      /* +0x65 */
    unsigned char has_icon;    /* +0x66: the card's folder holds a full icon.sys */
    unsigned char unmodeled_67[0x2000 - 0x67];
    /* +0x2000: the file contents (up to MC_OLD_SAVE_BYTES for check-old), then the bytes reported
     * from them at the start. */
    unsigned char data[MC_OLD_SAVE_BYTES];
} McListWork;

/* One sceMcGetDir entry (0x40 bytes). */
typedef struct McDirectoryEntry {
    unsigned char unmodeled_00[9];
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    int size;                  /* +0x10 */
    unsigned char unmodeled_14[0xC];
    char name[32];             /* +0x20 */
} McDirectoryEntry;

/* xglMcState.request (+0x80, 0x959FB8) under its own data label: xglMcMain copies the queued
 * request through it. Through the field the copy reuses the xglMcState base register and is two
 * instructions shorter. */
extern McQueueEntry savedMcRequest;
int sceMcSync(int mode, int *command, int *result);
int sceMcRead(int descriptor, void *buffer, int size);
int strcmp(const char *left, const char *right);
void *memset(void *destination, int value, unsigned int size);
extern char *xglMcBasePaths[4];
extern char xglMcWildcardName[8];
int xglMcEasyLoad(const char *path, void *buffer, int size);
int xglMcEasySave(const char *path, const void *buffer, int size);
extern u8 *buffer;
int sceMcClose(int fd);
int sceMcRead(int fd, void *buffer, int size);
int sceMcWrite(int fd, const void *buffer, int size);

/* Queues a memory-card request (the parameters are copied for the commands that use them),
 * clears *result and runs one step of xglMcMain. */
int xglMcRequest(int port, int command, McRequestParam *param, int *result)
{
    int index;
    McQueueEntry *entry;

    index = xglMcQueueEnd;
    *result = 0;
    entry = &xglMcQueue[index];
    entry->port = port;
    entry->result = result;
    entry->command = command;
    switch (command) {
    case MC_REQ_FREE_SPACE:
    case MC_REQ_FORMAT:
    case MC_REQ_UNFORMAT:
        break;
    case MC_REQ_EXIST:
    case MC_REQ_LIST:
    case MC_REQ_LOAD:
    case MC_REQ_SAVE:
    case MC_REQ_CREATE:
    case MC_REQ_CHECK_OLD:
        entry->file = param->file;
        entry->data = (int)param->data;
        entry->callback = param->callback;
        entry->size = param->size;
        break;
    }
    xglMcQueueEnd = (index + 1) & MC_QUEUE_MASK;
    xglMcMain();
    return 0;
}

/* Returns the status word of the card state (+0x04 of xglMcState). */
int xglMcGetState(void)
{
    return xglMcStatusWord[0];
}

/* Clears both queue indices and the request phase of the memory-card state. */
void xglMcReset(void)
{
    xglMcQueueTop = 0;
    xglMcState.state = 0;
    xglMcQueueEnd = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_mc_request", xglMcEUC2SJIS);

INCLUDE_ASM("asm/main/nonmatchings/xgl_mc_request", func_001894A0);

/* Builds the full path in xglMcState.full_path: base, then the card number as two digits ("*" for
 * MC_CARD_ANY, nothing for MC_CARD_NONE), then by selector the path so far once more, one of the
 * file names 1..5, nothing, or "/" and the name selector points to. Returns the path. */
char *xglMcBuildPath(const char *base, int card, int selector)
{
    char *dest;
    const char *source;
    const char *prefix_end;

    dest = xglMcState.full_path;
    source = base;
    while ((*dest = *source) != 0) {
        dest++;
        source++;
    }
    switch (card) {
    case MC_CARD_NONE:
        break;
    case MC_CARD_ANY:
        *dest = '*';
        dest++;
        break;
    default:
        dest[1] = '0' + card % 10;
        dest[0] = '0' + (card / 10) % 10;
        dest += 2;
        break;
    }
    switch (selector) {
    case MC_PATH_REPEAT:
        prefix_end = dest;
        for (source = xglMcState.full_path; source != prefix_end; source++) {
            *dest = *source;
            dest++;
        }
        *dest = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        source = xglMcFileNames[selector];
        while ((*dest = *source) != 0) {
            dest++;
            source++;
        }
        break;
    case MC_PATH_END:
        *dest = 0;
        break;
    default:
        *dest = '/';
        dest++;
        source = (const char *)selector;
        while ((*dest = *source) != 0) {
            dest++;
            source++;
        }
        break;
    }
    return xglMcState.full_path;
}

/* Full path for card `card` (or -1 for the wildcard) with the suffix path, built on the base path xglMcBasePath. */
char *xglMcSetFullPath(int card, int selector)
{
    return xglMcBuildPath(xglMcBasePath, card, selector);
}

/* The memory-card state block starts at the splat label D_0089BA80, 0x3C4 bytes before xglMcState.
 * The request functions reach the state through this base; its first part is not modeled. */
typedef struct XglMcBlock {
    McIconSys icon;
    XglMcState state;
} XglMcBlock;

extern XglMcBlock D_0089BA80;
extern char D_00551198[];
int func_002F9218(int port, int slot, const char *name);
/* sceMcGetDir under its scaffold label (config/symbols/main.txt func_002F9A40). */
int func_002F9A40(int port, int slot, const char *name, int mode, int max_entries, int table);
/* Save folders of the check-old search, one per search mode (xglMcState.request.file):
 * /BASLUS-20469Xeno, /BISLPS-29005Xeno, /BISLPS-25353. This game's own folder is
 * xglMcBasePath, /BASLUS-20892Xeno2. */
extern char *D_004B3698[4];
/* Entries 1..3 of that table under their own splat labels (each pointer is a separate label). */
extern char *D_004B369C[];
extern char *D_004B36A0[];
extern char *D_004B36A4[];
int func_002F9D60(int port, int slot);
int func_002FA338(int port, int slot);
/* Starts the queued request: the SDK call for the command and the state xglMcMain then follows. */
void xglMcExecute(McQueueEntry *request);
/* Picks a card for the request, then starts a directory scan of 16 entries on it. Returns 1 when no card fits. */
int xglMcStartDirectoryScan(void);
/* Opens the save file on the card nearest to the selector. When no card fits, the search mode * (request.file) moves on: mode 0 starts a directory read of up to 100 entries on the second base * path (returns -1), mode 1 opens the third base path, mode 2 starts a directory read on the * fourth (returns -1), mode 3 gives up (returns 1). Returns 0 when a file open was started. */
int xglMcOpenNearestSave(void);

/* Starts the queued request: the SDK call for the command and the state xglMcMain then follows. */
void xglMcExecute(McQueueEntry *request)
{

    D_0089BA80.state.substate = 0;
    switch (request->command) {
    case MC_REQ_FREE_SPACE:
        sceMcGetInfo(request->port, 0, 0, request->result, 0);
        return;
    case MC_REQ_EXIST:
        if (request->file == -1) {
            xglMcSetFullPath(-1, -1);
            func_002F9A40(request->port, 0, D_0089BA80.state.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                          MC_DIRECTORY_TABLE(request->data));
            D_0089BA80.state.state = MC_STATE_EXIST_SCAN;
        } else {
            xglMcSetFullPath(request->file, 0);
            sceMcOpen(request->port, 0, D_0089BA80.state.full_path, MC_OPEN_READ);
            D_0089BA80.state.state = MC_STATE_OPEN_CLOSE;
        }
        return;
    case MC_REQ_LIST:
        xglMcSetFullPath(-1, -1);
        D_0089BA80.state.state = MC_STATE_LIST;
        func_002F9A40(request->port, 0, D_0089BA80.state.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                      MC_DIRECTORY_TABLE(request->data));
        return;
    case MC_REQ_CHECK_OLD:
        D_0089BA80.state.request.file = 3;
        xglMcBuildPath(D_004B36A4[0], MC_CARD_ANY, MC_PATH_END);
        func_002F9A40(request->port, 0, D_0089BA80.state.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                      MC_DIRECTORY_TABLE(request->data));
        D_0089BA80.state.state = MC_STATE_CHECK_OLD;
        return;
    case MC_REQ_CREATE:
        xglMcSetFullPath(request->file, -1);
        func_002F9218(request->port, 0, D_0089BA80.state.full_path);
        D_0089BA80.state.create_index = 0;
        D_0089BA80.state.state = MC_STATE_CREATE;
        return;
    case MC_REQ_SAVE:
        xglMcSetFullPath(request->file, 0);
        sceMcOpen(request->port, 0, D_0089BA80.state.full_path, MC_OPEN_WRITE_CREATE);
        D_0089BA80.state.state = MC_STATE_WRITE;
        return;
    case MC_REQ_LOAD:
        xglMcSetFullPath(request->file, 0);
        sceMcOpen(request->port, 0, D_0089BA80.state.full_path, MC_OPEN_READ);
        D_0089BA80.state.state = MC_STATE_READ;
        return;
    case MC_REQ_FORMAT:
        func_002F9D60(request->port, 0);
        return;
    case MC_REQ_UNFORMAT:
        func_002FA338(request->port, 0);
        break;
    }
}

/* sceMcDelete under its scaffold label (config/symbols/main.txt func_002F9E30). */
int func_002F9E30(int port, int slot, const char *name);
/* Advances the create request by one step (xglMcState.substate): deletes the old files, copies * icon.sys, then opens, writes and closes each file. Returns 1 when the request is finished, -1 * when the card result is an error, and 0 when the step ran. */
int xglMcCreateStep(int result);

/* Advances the create request by one step (xglMcState.substate): deletes the old files, copies
 * icon.sys, then opens, writes and closes each file. Returns 1 when the request is finished, -1
 * when the card result is an error, and 0 when the step ran. */
int xglMcCreateStep(int result)
{
    int file;
    McCreateData *save;
    char *data;
    int *entry;
    int offset;
    int size;

    file = D_0089BA80.state.request.file;
    if (result >= 0 || (D_0089BA80.state.substate < MC_CREATE_FILES_START && result == MC_RESULT_NO_ENTRY)) {
        save = (McCreateData *)D_0089BA80.state.request.data;
        switch (D_0089BA80.state.substate) {
        case MC_CREATE_DELETE_SAVE:
            xglMcSetFullPath(file, 0);
            func_002F9E30(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path);
            break;
        case MC_CREATE_DELETE_DUMMY:
            xglMcSetFullPath(file, 5);
            func_002F9E30(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path);
            break;
        case MC_CREATE_COPY_ICON:
            D_0089BA80.icon = save->icon;
            break;
        case MC_CREATE_FILES_START:
            break;
        case MC_CREATE_CLOSE_SYSTEM_ICO:
        case MC_CREATE_CLOSE_COPY_ICO:
        case MC_CREATE_CLOSE_DELETE_ICO:
        case MC_CREATE_LAST_CLOSE:
            sceMcClose(D_0089BA80.state.descriptor);
            /* An offset table that ends after system.ico (offset[3] == 0) has no copy.ico,
               delete.ico or dummy file. */
            if (save->offset[3] == 0 && D_0089BA80.state.substate == MC_CREATE_CLOSE_SYSTEM_ICO) {
                return 1;
            }
            if (D_0089BA80.state.substate == MC_CREATE_LAST_CLOSE) {
                return 1;
            }
            break;
        case MC_CREATE_OPEN_SYSTEM_ICO:
        case MC_CREATE_OPEN_COPY_ICO:
        case MC_CREATE_OPEN_DELETE_ICO:
        case MC_CREATE_OPEN_DUMMY:
            xglMcSetFullPath(file, D_0089BA80.state.substate / MC_CREATE_STEPS_PER_FILE + 1);
            sceMcOpen(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path, MC_OPEN_WRITE_CREATE);
            break;
        case MC_CREATE_WRITE_SYSTEM_ICO:
        case MC_CREATE_WRITE_COPY_ICO:
        case MC_CREATE_WRITE_DELETE_ICO:
            D_0089BA80.state.descriptor = result;
            data = (char *)save + ((save->offset[0] + 0x3F) & ~0x3F);
            /* The entry address is computed as an integer, index first (addu index*4, base). */
            entry = (int *)(D_0089BA80.state.substate / MC_CREATE_STEPS_PER_FILE * 4 + (int)save);
            offset = (entry[0] + 0x3F) & ~0x3F;
            size = entry[1] - offset;
            D_0089BA80.state.create_index += (size + 0x3FF) & ~0x3FF;
            sceMcWrite(result, data + offset, size);
            break;
        case MC_CREATE_DUMMY_WRITE:
            D_0089BA80.state.descriptor = result;
            sceMcWrite(result, save, MC_CREATE_TOTAL_BYTES - D_0089BA80.state.create_index);
            break;
        }
        D_0089BA80.state.substate++;
        *D_0089BA80.state.request.result = D_0089BA80.state.substate;
        return 0;
    }
    *D_0089BA80.state.request.result = result;
    return -1;
}

/* Returns the index of the entry with value 1 in the 100-entry table nearest to index, searching
 * index, index+1, index-1, index+2, ...; -1 once both directions have run off the table. */
int xglMcFindNearestSlot(unsigned char *table, int index)
{
    int step;
    int misses;

    step = 1;
    misses = 0;
    while (table[index] != MC_SLOT_WANTED) {
        for (;;) {
            index += step;
            if (step > 0) {
                step = -step - 1;
            } else {
                step = 1 - step;
            }
            if ((unsigned int)index < MC_SLOT_COUNT) {
                break;
            }
            misses++;
            if (misses >= 2) {
                return -1;
            }
        }
        misses = 0;
    }
    return index;
}

/* Picks a card for the request, then starts a directory scan of 16 entries on it. Returns 1 when no card fits. */
int xglMcStartDirectoryScan(void)
{
    int card;

    card = xglMcFindNearestSlot((unsigned char *)D_0089BA80.state.request.data, D_0089BA80.state.selector);
    if (card < 0)
    {
        return 1;
    }
    D_0089BA80.state.card = (unsigned char)card;
    D_0089BA80.state.status = 0;
    /* selector values outside the small codes are name pointers (xglMcBuildPath); this one is "*", every save folder */
    xglMcSetFullPath(card, (int)D_00551198);
    func_002F9A40(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path, 0, 0x10,
                  MC_DIRECTORY_TABLE(D_0089BA80.state.request.data));
    return 0;
}

/* Opens the save file on the card nearest to the selector. When no card fits, the search mode
 * (request.file) moves on: mode 0 starts a directory read of up to 100 entries on the second base
 * path (returns -1), mode 1 opens the third base path, mode 2 starts a directory read on the
 * fourth (returns -1), mode 3 gives up (returns 1). Returns 0 when a file open was started. */
int xglMcOpenNearestSave(void)
{
    int card;

    card = xglMcFindNearestSlot((unsigned char *)D_0089BA80.state.request.data, D_0089BA80.state.selector);
    if (card < 0) {
        switch ((unsigned int)D_0089BA80.state.request.file) {
        case 0:
            D_0089BA80.state.request.file = 1;
            xglMcBuildPath(D_004B369C[0], MC_CARD_ANY, MC_PATH_END);
            func_002F9A40(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path, 0, 100,
                          MC_DIRECTORY_TABLE(D_0089BA80.state.request.data));
            return -1;
        case 1:
            D_0089BA80.state.request.file = 2;
            xglMcBuildPath(D_004B36A0[0], MC_CARD_NONE, MC_PATH_REPEAT);
            sceMcOpen(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path, MC_OPEN_READ);
            return 0;
        case 2:
            D_0089BA80.state.request.file = 3;
            xglMcBuildPath(D_004B36A4[0], MC_CARD_ANY, MC_PATH_END);
            func_002F9A40(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path, 0, 100,
                          MC_DIRECTORY_TABLE(D_0089BA80.state.request.data));
            return -1;
        case 3:
            return 1;
        }
    }
    D_0089BA80.state.card = (unsigned char)card;
    D_0089BA80.state.status = 0;
    xglMcBuildPath(D_004B3698[D_0089BA80.state.request.file], card, MC_PATH_REPEAT);
    sceMcOpen(D_0089BA80.state.request.port, 0, D_0089BA80.state.full_path, MC_OPEN_READ);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_mc_request", xglMcMain);

int xglMcEasyLoad(const char *path, void *buffer, int size)
{
    int card_type;
    int command;
    int result;
    int descriptor;
    int read_result;

    sceMcGetInfo(0, 0, 0, 0, &card_type);
    sceMcSync(0, &command, &result);
    if (card_type == 0)
    {
        return -1;
    }
    if (result < -1)
    {
        return result;
    }

    sceMcOpen(0, 0, path, MC_OPEN_READ);
    sceMcSync(0, &command, &result);
    descriptor = result;
    if (descriptor < 0)
    {
        return descriptor;
    }

    sceMcRead(descriptor, buffer, size);
    sceMcSync(0, &command, &result);
    read_result = result;
    if (read_result < 0)
    {
        return read_result;
    }

    sceMcClose(descriptor);
    sceMcSync(0, &command, &result);
    if (result < 0)
    {
        return result;
    }
    return read_result;
}

int xglMcEasySave(const char *path, const void *buffer, int size)
{
    int card_type;
    int command;
    int result;
    int descriptor;

    sceMcGetInfo(0, 0, 0, 0, &card_type);
    sceMcSync(0, &command, &result);
    if (result < -1 || card_type == 0)
    {
        return -1;
    }

    sceMcOpen(0, 0, path, MC_OPEN_WRITE_CREATE);
    sceMcSync(0, &command, &result);
    descriptor = result;
    if (descriptor < 0)
    {
        return -1;
    }

    sceMcWrite(descriptor, buffer, size);
    sceMcSync(0, &command, &result);
    if (result < 0)
    {
        return -1;
    }

    sceMcClose(descriptor);
    sceMcSync(0, &command, &result);
    return result >= 0 ? 0 : -1;
}
