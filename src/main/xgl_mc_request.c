#include "common.h"
#include "shared.h"
#include "main/xgl_cd.h"

unsigned int xglClockDayTime2UInt(XglClock *clock_time);

/* One request of the xglMc queue: the file number, its buffer and size, and
 * a callback the list and old-data scans call per entry (with the result,
 * and the entry bytes for the old-data check).  The callback is unprototyped
 * because the callers in yajima_test.c differ: DebugCallbackLog takes the result
 * only, McOldDataCheckCallback the result and the entry. */
typedef struct McFileRequest {
    int file;
    void *bufferAddress;
    int contentsSize;
    int (*callback)();
} McFileRequest;

void xglMcReset(void);
char * xglMcSetFullPath(int card, int selector);
int xglMcMain(void);
int xglMcRequest(int port, int command, McFileRequest *fileRequest, int *callerResult);

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
    int *callerResult;              /* +0x04 */
    int file;                 /* +0x08 */
    int bufferAddress;                 /* +0x0C: the buffer address */
    int contentsSize;                 /* +0x10 */
    int (*callback)(int code, void *work);     /* +0x14 */
    unsigned char unmodeled_18[8];
} McQueueEntry;

extern unsigned char xglMcQueueTop;
extern unsigned char xglMcQueueEnd;
extern McQueueEntry xglMcQueue[16];

/* The icon.sys contents the create request keeps (sizeof(sceMcIconSys) bytes, byte aligned). */
typedef struct McIconSys {
    unsigned char header[0x1C4];   /* +0x000: the sceMcIconSys fields up to the delete icon name */
    unsigned char reserved[0x200]; /* +0x1C4: the reserved tail, cleared before a save writes icon.sys */
} McIconSys;

/* Memory-card state block at xglMcCardState. */
typedef struct XglMcCardState {
    union {
        struct {
            unsigned char requestPhase; /* +0x00: phase of the request requestPhase machine */
            unsigned char substate;     /* +0x01: step inside the current state, cleared per request */
        };
        unsigned short phaseAndStep;    /* +0x00: requestPhase | substate << 8 (MC_PHASE_STEP) */
    };
    unsigned char unmodeled_02[1];
    unsigned char next_card;        /* +0x03: bit 0 is the card the next status poll asks about */
    union {
        int combined;               /* +0x04: the status word xglMcGetState returns */
        short perCardStatus[2];             /* +0x04: one MC_STATUS_* halfword per card */
    } card_status;
    int fileDescriptor;                 /* +0x08: the open file, or the card of a status poll */
    int card_type;                  /* +0x0C: sceMcGetInfo type */
    unsigned char unmodeled_10[4];
    int formatted;                  /* +0x14: sceMcGetInfo format flag */
    int create_index;               /* +0x18: progress of the create request */
    unsigned char unmodeled_1c[1];
    unsigned char selector;         /* +0x1D: selector for the card search */
    unsigned char card;             /* +0x1E: card index chosen by the search */
    unsigned char cardSearchStatus;   /* +0x1F */
    char full_path[0x60];           /* +0x20: full path of the current request (xglMcBuildPath) */
    McQueueEntry activeRequest;           /* +0x80: the request being worked on; the card search reuses its
                                     * file number as the search mode (0..2) and data as the work address */
    McIconSys icon;                 /* +0xA0: icon.sys contents kept by the create request */
} XglMcCardState;

extern XglMcCardState xglMcCardState;
/* xglMcCardState.card_status.combined (+0x04) under its own data label. xglMcGetState and the final
 * return of xglMcMain read it through this label; the early return of xglMcMain (no card operation
 * finished, the queue empty) reads the field. Both were checked: the label in the early return
 * changes four bytes, the field in the final return shortens the function. */
extern int xglMcCardStatusWord[];
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

/* xglMcCardState.requestPhase while a request runs (set by xglMcStartQueuedRequest). */
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
/* Entries a directory scan of one card's folder reads (xglMcStartDirectoryScan). */
#define MC_SCAN_ENTRIES 0x10
/* Entries of a list / check-old work buffer's slot table (McScanWork.cardScanState). */
#define MC_SLOT_COUNT 100
/* sceMcGetDir writes its entry table into the request's work buffer, behind the
 * MC_SLOT_COUNT slot bytes and rounded up to the 64-byte alignment the SDK needs. */
#define MC_DIRECTORY_TABLE(work) (((work) + MC_SLOT_COUNT + 0x3F) & ~0x3F)

int sceMcGetInfo(int port, int slot, int *type, int *free, int *format);
int sceMcMkdir(int port, int slot, const char *name);
int sceMcGetDir(int port, int slot, const char *name, int mode, int max_entries, int table);
int sceMcOpen(int port, int slot, const char *name, int mode);
/* Save folders of the check-old search, one per search mode (xglMcCardState.activeRequest.file):
 * /BASLUS-20469Xeno, /BISLPS-29005Xeno, /BISLPS-25353. This game's own folder is
 * xglMcBasePath, /BASLUS-20892Xeno2. */
extern char *xglMcBasePaths[];
int sceMcFormat(int port, int slot);
int sceMcUnformat(int port, int slot);
extern const char xglMcBasePath[24];
void xglMcStartQueuedRequest(McQueueEntry *queuedRequest);

/* The create request's buffer: a table of file offsets (file k spans offset[k]..offset[k + 1],
 * each start rounded up to 64 bytes, relative to the end of the rounded header size offset[0]),
 * then at +0x40 the icon.sys contents. */
typedef struct McCreateFileTable {
    int fileOffsets[16];
    McIconSys icon;
} McCreateFileTable;

/* Steps of the create request (xglMcCardState.substate). From MC_CREATE_FILES_START on, each file
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

/* xglMcCardState.icon (+0xA0, 0x959FD8) under its own data label: the copy goes through the label.
 * Through the field it reuses the xglMcCardState base register and does not match. */
extern McIconSys xglMcIconSys;
int sceMcDelete(int port, int slot, const char *name);
int sceMcClose(int fileDescriptor);
int sceMcWrite(int fileDescriptor, const void *contents, int contentsSize);
int xglMcAdvanceCreateRequest(int cardResult);

#define MC_SLOT_WANTED 1 /* the value the search looks for */
/* McScanWork.latest: the card number with the newest time stamp, behind the slot table (X1). */
#define MC_LATEST_SLOT_INDEX MC_SLOT_COUNT

int xglMcStartDirectoryScan(void);
int xglMcOpenNearestSave(void);

/* Result of xglMcMain / the card status word while a request is running. */
#define MC_STATUS_BUSY_BOTH 0xFF00FF
/* Per-card status halfwords (xglMcCardState.card_status.perCardStatus). */
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
/* Steps of a list / check-old scan (xglMcCardState.substate). */
#define MC_SCAN_DIRECTORY 0
#define MC_SCAN_NEXT_CARD 1
#define MC_SCAN_READ 2
#define MC_SCAN_REPORT 3
#define MC_SCAN_ICON_CHECK 4
/* McScanWork.scanStatus values. */
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

/* The work buffer of a list / check-old request (activeRequest.bufferAddress). */
typedef struct McScanWork {
    unsigned char cardScanState[100];   /* +0x00: per card number: 1 present, 2 read */
    unsigned char latest;      /* +0x64: card number with the newest time stamp */
    unsigned char scanStatus;      /* +0x65 */
    unsigned char saveCount;   /* +0x66: cards with a full save, counted by a counting list request */
    unsigned char iconFlags;   /* +0x67: MC_ICON_SYS_FOUND / MC_SAVE_FILE_FOUND of the card's folder */
    unsigned char unmodeled_68[0x2000 - 0x68];
    /* +0x2000: the file contents (up to MC_OLD_SAVE_BYTES for check-old), then the bytes reported
     * from them at the start. */
    unsigned char saveContents[MC_OLD_SAVE_BYTES];
} McScanWork;

/* One sceMcGetDir entry (0x40 bytes). */
typedef struct McDirectoryEntry {
    unsigned char unmodeled_00[9];
    unsigned char seconds;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    int fileSize;                  /* +0x10 */
    unsigned char unmodeled_14[0xC];
    char name[32];             /* +0x20 */
} McDirectoryEntry;

/* xglMcCardState.activeRequest (+0x80, 0x959FB8) under its own data label: xglMcMain copies the queued
 * request through it. Through the field the copy reuses the xglMcCardState base register and is two
 * instructions shorter. */
extern McQueueEntry savedMcRequest;
int sceMcSync(int mode, int *command, int *syncResult);
int sceMcRead(int fileDescriptor, void *contents, int contentsSize);
int strcmp(const char *left, const char *right);
void *memset(void *destination, int value, unsigned int size);
extern char *xglMcBasePaths[4];
extern char xglMcWildcardName[8];
int xglMcEasyLoad(const char *path, void *loadedContents, int loadedSize);
int xglMcEasySave(const char *path, const void *contentsToSave, int saveSize);
extern u8 *buffer;

/* Queues a memory-card request (the parameters are copied for the commands that use them),
 * clears *result and runs one step of xglMcMain. */
int xglMcRequest(int port, int command, McFileRequest *fileRequest, int *callerResult)
{
    int queueIndex;
    McQueueEntry *queuedEntry;

    queueIndex = xglMcQueueEnd;
    *callerResult = 0;
    queuedEntry = &xglMcQueue[queueIndex];
    queuedEntry->port = port;
    queuedEntry->callerResult = callerResult;
    queuedEntry->command = command;
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
        queuedEntry->file = fileRequest->file;
        queuedEntry->bufferAddress = (int)fileRequest->bufferAddress;
        queuedEntry->callback = fileRequest->callback;
        queuedEntry->contentsSize = fileRequest->contentsSize;
        break;
    }
    xglMcQueueEnd = (queueIndex + 1) & MC_QUEUE_MASK;
    xglMcMain();
    return 0;
}

/* Returns the status word of the card state (+0x04 of xglMcCardState). */
int xglMcGetState(void)
{
    return xglMcCardStatusWord[0];
}

/* Clears both queue indices and the request phase of the memory-card state. */
void xglMcReset(void)
{
    xglMcQueueTop = 0;
    xglMcCardState.requestPhase = 0;
    xglMcQueueEnd = 0;
}

typedef int s32;

void xglMcEUC2SJIS(u8 *lead, u8 *trail)
{
  s32 trail_adjusted;
  s32 trail_value;
  u32 lead_value;
  unsigned char lead_byte;
  u8 sjis_trail;
  u8 sjis_lead;
  lead_value = ((*lead) + 0x80) & 0xFF;
  trail_value = ((*trail) + 0x80) & 0xFF;
  lead_byte = lead_value;
  if (lead_byte & 1)
  {
    trail_adjusted = trail_value + 0x1F;
    sjis_lead = (lead_byte >> 1) + 0x71;
  }
  else
  {
    trail_adjusted = trail_value + 0x7D;
    sjis_lead = (lead_byte >> 1) + 0x70;
  }
  sjis_trail = trail_adjusted & 0xFF;
  if (sjis_lead >= 0xA0U)
  {
    sjis_lead = (sjis_lead + 0x40) & 0xFF;
  }
  if (sjis_trail >= 0x7FU)
  {
    sjis_trail = (sjis_trail + 1) & 0xFF;
  }
  if ((sjis_lead == 0x87) && (sjis_trail == 0x54))
  {
    sjis_lead = 0x82;
    sjis_trail = 0x50;
  }
  *lead = sjis_lead;
  *trail = sjis_trail;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_mc_request", func_001894A0);

/* Builds the full path in xglMcCardState.full_path: base, then the card number as two digits ("*" for
 * MC_CARD_ANY, nothing for MC_CARD_NONE), then by selector the path so far once more, one of the
 * file names 1..5, nothing, or "/" and the name selector points to. Returns the path. */
char *xglMcBuildPath(const char *basePath, int card, int selector)
{
    char *pathCursor;
    const char *sourceCursor;
    const char *prefix_end;

    pathCursor = xglMcCardState.full_path;
    sourceCursor = basePath;
    while ((*pathCursor = *sourceCursor) != 0) {
        pathCursor++;
        sourceCursor++;
    }
    switch (card) {
    case MC_CARD_NONE:
        break;
    case MC_CARD_ANY:
        *pathCursor = '*';
        pathCursor++;
        break;
    default:
        pathCursor[1] = '0' + card % 10;
        pathCursor[0] = '0' + (card / 10) % 10;
        pathCursor += 2;
        break;
    }
    switch (selector) {
    case MC_PATH_REPEAT:
        prefix_end = pathCursor;
        for (sourceCursor = xglMcCardState.full_path; sourceCursor != prefix_end; sourceCursor++) {
            *pathCursor = *sourceCursor;
            pathCursor++;
        }
        *pathCursor = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        sourceCursor = xglMcFileNames[selector];
        while ((*pathCursor = *sourceCursor) != 0) {
            pathCursor++;
            sourceCursor++;
        }
        break;
    case MC_PATH_END:
        *pathCursor = 0;
        break;
    default:
        *pathCursor = '/';
        pathCursor++;
        sourceCursor = (const char *)selector;
        while ((*pathCursor = *sourceCursor) != 0) {
            pathCursor++;
            sourceCursor++;
        }
        break;
    }
    return xglMcCardState.full_path;
}

/* Full path for card `card` (or -1 for the wildcard) with the suffix path, built on the base path xglMcBasePath. */
char *xglMcSetFullPath(int card, int selector)
{
    return xglMcBuildPath(xglMcBasePath, card, selector);
}

/* The memory-card state block starts at the splat label xglMcCardBlock, 0x3C4 bytes before xglMcCardState.
 * The request functions reach the state through this base; its first part is not modeled. */
/* Entries of the directory read that checks for icon.sys after a read or write request. */
#define MC_ICON_CHECK_ENTRIES 6

typedef struct XglMcCardBlock {
    union {
        McIconSys icon;
        /* +0x200: the directory table of the icon.sys check (64-byte aligned for sceMcGetDir). */
        struct {
            unsigned char unmodeled_000[0x200];
            McDirectoryEntry directory[MC_ICON_CHECK_ENTRIES];
        } iconCheck;
    };
    XglMcCardState cardState;
} XglMcCardBlock;

extern XglMcCardBlock xglMcCardBlock;
extern char xglMcAnyFolderName[];
int func_002F9218(int port, int slot, const char *name);
/* sceMcGetDir under its scaffold label (config/symbols/main.txt func_002F9A40). */
int func_002F9A40(int port, int slot, const char *name, int mode, int max_entries, int table);
/* Save folders of the check-old search, one per search mode (xglMcCardState.activeRequest.file):
 * /BASLUS-20469Xeno, /BISLPS-29005Xeno, /BISLPS-25353. This game's own folder is
 * xglMcBasePath, /BASLUS-20892Xeno2. */
extern char *D_004B3698[4];
/* Entries 1..3 of that table under their own splat labels (each pointer is a separate label). */
extern char *xglMcBasePathSecond[];
extern char *xglMcBasePathThird[];
extern char *xglMcBasePathFourth[];
int func_002F9D60(int port, int slot);
int func_002FA338(int port, int slot);
/* Starts the queued request: the SDK call for the command and the state xglMcMain then follows. */
void xglMcStartQueuedRequest(McQueueEntry *queuedRequest);
/* Picks a card for the request, then starts a directory scan of 16 entries on it. Returns 1 when no card fits. */
int xglMcStartDirectoryScan(void);
/* Opens the save file on the card nearest to the selector. When no card fits, the search mode * (activeRequest.file) moves on: mode 0 starts a directory read of up to 100 entries on the second base * path (returns -1), mode 1 opens the third base path, mode 2 starts a directory read on the * fourth (returns -1), mode 3 gives up (returns 1). Returns 0 when a file open was started. */
int xglMcOpenNearestSave(void);

/* Starts the queued request: the SDK call for the command and the state xglMcMain then follows. */
void xglMcStartQueuedRequest(McQueueEntry *queuedRequest)
{

    xglMcCardBlock.cardState.substate = 0;
    switch (queuedRequest->command) {
    case MC_REQ_FREE_SPACE:
        sceMcGetInfo(queuedRequest->port, 0, 0, queuedRequest->callerResult, 0);
        return;
    case MC_REQ_EXIST:
        if (queuedRequest->file == -1) {
            xglMcSetFullPath(-1, -1);
            func_002F9A40(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                          MC_DIRECTORY_TABLE(queuedRequest->bufferAddress));
            xglMcCardBlock.cardState.requestPhase = MC_STATE_EXIST_SCAN;
        } else {
            xglMcSetFullPath(queuedRequest->file, 0);
            sceMcOpen(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_READ);
            xglMcCardBlock.cardState.requestPhase = MC_STATE_OPEN_CLOSE;
        }
        return;
    case MC_REQ_LIST:
        xglMcSetFullPath(-1, -1);
        xglMcCardBlock.cardState.requestPhase = MC_STATE_LIST;
        func_002F9A40(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                      MC_DIRECTORY_TABLE(queuedRequest->bufferAddress));
        return;
    case MC_REQ_CHECK_OLD:
        xglMcCardBlock.cardState.activeRequest.file = 3;
        xglMcBuildPath(xglMcBasePathFourth[0], MC_CARD_ANY, MC_PATH_END);
        func_002F9A40(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                      MC_DIRECTORY_TABLE(queuedRequest->bufferAddress));
        xglMcCardBlock.cardState.requestPhase = MC_STATE_CHECK_OLD;
        return;
    case MC_REQ_CREATE:
        xglMcSetFullPath(queuedRequest->file, -1);
        func_002F9218(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path);
        xglMcCardBlock.cardState.create_index = 0;
        xglMcCardBlock.cardState.requestPhase = MC_STATE_CREATE;
        return;
    case MC_REQ_SAVE:
        xglMcSetFullPath(queuedRequest->file, 0);
        sceMcOpen(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_WRITE_CREATE);
        xglMcCardBlock.cardState.requestPhase = MC_STATE_WRITE;
        return;
    case MC_REQ_LOAD:
        xglMcSetFullPath(queuedRequest->file, 0);
        sceMcOpen(queuedRequest->port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_READ);
        xglMcCardBlock.cardState.requestPhase = MC_STATE_READ;
        return;
    case MC_REQ_FORMAT:
        func_002F9D60(queuedRequest->port, 0);
        return;
    case MC_REQ_UNFORMAT:
        func_002FA338(queuedRequest->port, 0);
        break;
    }
}

/* sceMcDelete under its scaffold label (config/symbols/main.txt func_002F9E30). */
int func_002F9E30(int port, int slot, const char *name);
/* Advances the create request by one step (xglMcCardState.substate): deletes the old files, copies * icon.sys, then opens, writes and closes each file. Returns 1 when the request is finished, -1 * when the card result is an error, and 0 when the step ran. */
int xglMcAdvanceCreateRequest(int cardResult);

/* Advances the create request by one step (xglMcCardState.substate): deletes the old files, copies
 * icon.sys, then opens, writes and closes each file. Returns 1 when the request is finished, -1
 * when the card result is an error, and 0 when the step ran. */
int xglMcAdvanceCreateRequest(int cardResult)
{
    int file;
    McCreateFileTable *fileTable;
    char *fileData;
    int *fileOffsetEntry;
    int fileOffset;
    int fileSize;

    file = xglMcCardBlock.cardState.activeRequest.file;
    if (cardResult >= 0 || (xglMcCardBlock.cardState.substate < MC_CREATE_FILES_START && cardResult == MC_RESULT_NO_ENTRY)) {
        fileTable = (McCreateFileTable *)xglMcCardBlock.cardState.activeRequest.bufferAddress;
        switch (xglMcCardBlock.cardState.substate) {
        case MC_CREATE_DELETE_SAVE:
            xglMcSetFullPath(file, 0);
            func_002F9E30(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path);
            break;
        case MC_CREATE_DELETE_DUMMY:
            xglMcSetFullPath(file, 5);
            func_002F9E30(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path);
            break;
        case MC_CREATE_COPY_ICON:
            xglMcCardBlock.icon = fileTable->icon;
            break;
        case MC_CREATE_FILES_START:
            break;
        case MC_CREATE_CLOSE_SYSTEM_ICO:
        case MC_CREATE_CLOSE_COPY_ICO:
        case MC_CREATE_CLOSE_DELETE_ICO:
        case MC_CREATE_LAST_CLOSE:
            sceMcClose(xglMcCardBlock.cardState.fileDescriptor);
            /* An fileOffset table that ends after system.ico (fileOffset[3] == 0) has no copy.ico,
               delete.ico or dummy file. */
            if (fileTable->fileOffsets[3] == 0 && xglMcCardBlock.cardState.substate == MC_CREATE_CLOSE_SYSTEM_ICO) {
                return 1;
            }
            if (xglMcCardBlock.cardState.substate == MC_CREATE_LAST_CLOSE) {
                return 1;
            }
            break;
        case MC_CREATE_OPEN_SYSTEM_ICO:
        case MC_CREATE_OPEN_COPY_ICO:
        case MC_CREATE_OPEN_DELETE_ICO:
        case MC_CREATE_OPEN_DUMMY:
            xglMcSetFullPath(file, xglMcCardBlock.cardState.substate / MC_CREATE_STEPS_PER_FILE + 1);
            sceMcOpen(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_WRITE_CREATE);
            break;
        case MC_CREATE_WRITE_SYSTEM_ICO:
        case MC_CREATE_WRITE_COPY_ICO:
        case MC_CREATE_WRITE_DELETE_ICO:
            xglMcCardBlock.cardState.fileDescriptor = cardResult;
            fileData = (char *)fileTable + ((fileTable->fileOffsets[0] + 0x3F) & ~0x3F);
            /* The fileOffsetEntry address is computed as an integer, index first (addu index*4, base). */
            fileOffsetEntry = (int *)(xglMcCardBlock.cardState.substate / MC_CREATE_STEPS_PER_FILE * 4 + (int)fileTable);
            fileOffset = (fileOffsetEntry[0] + 0x3F) & ~0x3F;
            fileSize = fileOffsetEntry[1] - fileOffset;
            xglMcCardBlock.cardState.create_index += (fileSize + 0x3FF) & ~0x3FF;
            sceMcWrite(cardResult, fileData + fileOffset, fileSize);
            break;
        case MC_CREATE_DUMMY_WRITE:
            xglMcCardBlock.cardState.fileDescriptor = cardResult;
            sceMcWrite(cardResult, fileTable, MC_CREATE_TOTAL_BYTES - xglMcCardBlock.cardState.create_index);
            break;
        }
        xglMcCardBlock.cardState.substate++;
        *xglMcCardBlock.cardState.activeRequest.callerResult = xglMcCardBlock.cardState.substate;
        return 0;
    }
    *xglMcCardBlock.cardState.activeRequest.callerResult = cardResult;
    return -1;
}

/* Returns the index of the entry with value 1 in the 100-entry table nearest to index, searching
 * index, index+1, index-1, index+2, ...; -1 once both directions have run off the table. */
int xglMcFindNearestSlot(unsigned char *cardScanState, int searchCard)
{
    int spiralStep;
    int misses;

    spiralStep = 1;
    misses = 0;
    while (cardScanState[searchCard] != MC_SLOT_WANTED) {
        for (;;) {
            searchCard += spiralStep;
            if (spiralStep > 0) {
                spiralStep = -spiralStep - 1;
            } else {
                spiralStep = 1 - spiralStep;
            }
            if ((unsigned int)searchCard < MC_SLOT_COUNT) {
                break;
            }
            misses++;
            if (misses >= 2) {
                return -1;
            }
        }
        misses = 0;
    }
    return searchCard;
}

/* Picks a card for the request, then starts a directory scan of 16 entries on it. Returns 1 when no card fits. */
int xglMcStartDirectoryScan(void)
{
    int card;

    card = xglMcFindNearestSlot((unsigned char *)xglMcCardBlock.cardState.activeRequest.bufferAddress, xglMcCardBlock.cardState.selector);
    if (card < 0)
    {
        return 1;
    }
    xglMcCardBlock.cardState.card = card;
    xglMcCardBlock.cardState.cardSearchStatus = 0;
    /* selector values outside the small codes are name pointers (xglMcBuildPath); this one is "*", every save folder */
    xglMcSetFullPath(card, (int)xglMcAnyFolderName);
    func_002F9A40(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, 0, MC_SCAN_ENTRIES,
                  MC_DIRECTORY_TABLE(xglMcCardBlock.cardState.activeRequest.bufferAddress));
    return 0;
}

/* Opens the save file on the card nearest to the selector. When no card fits, the search mode
 * (activeRequest.file) moves on: mode 0 starts a directory read of up to 100 entries on the second base
 * path (returns -1), mode 1 opens the third base path, mode 2 starts a directory read on the
 * fourth (returns -1), mode 3 gives up (returns 1). Returns 0 when a file open was started. */
int xglMcOpenNearestSave(void)
{
    int card;

    card = xglMcFindNearestSlot((unsigned char *)xglMcCardBlock.cardState.activeRequest.bufferAddress, xglMcCardBlock.cardState.selector);
    if (card < 0) {
        switch ((unsigned int)xglMcCardBlock.cardState.activeRequest.file) {
        case 0:
            xglMcCardBlock.cardState.activeRequest.file = 1;
            xglMcBuildPath(xglMcBasePathSecond[0], MC_CARD_ANY, MC_PATH_END);
            func_002F9A40(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                          MC_DIRECTORY_TABLE(xglMcCardBlock.cardState.activeRequest.bufferAddress));
            return -1;
        case 1:
            xglMcCardBlock.cardState.activeRequest.file = 2;
            xglMcBuildPath(xglMcBasePathThird[0], MC_CARD_NONE, MC_PATH_REPEAT);
            sceMcOpen(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_READ);
            return 0;
        case 2:
            xglMcCardBlock.cardState.activeRequest.file = 3;
            xglMcBuildPath(xglMcBasePathFourth[0], MC_CARD_ANY, MC_PATH_END);
            func_002F9A40(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, 0, MC_DIRECTORY_MAX_ENTRIES,
                          MC_DIRECTORY_TABLE(xglMcCardBlock.cardState.activeRequest.bufferAddress));
            return -1;
        case 3:
            return 1;
        }
    }
    xglMcCardBlock.cardState.card = card;
    xglMcCardBlock.cardState.cardSearchStatus = 0;
    xglMcBuildPath(D_004B3698[xglMcCardBlock.cardState.activeRequest.file], card, MC_PATH_REPEAT);
    sceMcOpen(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_READ);
    return 0;
}

/* xglMcCardState.requestPhase and .substate read together as one halfword (MC_PHASE_STEP). */
#define MC_PHASE_STEP(phase, step) (((step) << 8) | (phase))
/* The last check-old search mode (the fourth base path, xglMcOpenNearestSave). */
#define MC_OLD_SEARCH_LAST 3
/* Check-old mode 3: the flag byte of the save that marks a clear. */
#define MC_OLD3_FLAG 0x50E6
/* McScanWork.cardScanState: the card's save was read. */
#define MC_SLOT_READ 2
/* Scan step of a counting list request (activeRequest.file < 0): counts the cards with a full save. */
#define MC_SCAN_COUNT 5
/* McScanWork.iconFlags: the folder holds icon.sys (MC_ICON_SYS_FOUND) and the save file
 * (MC_SAVE_FILE_FOUND), each with its full size. */
#define MC_ICON_SYS_FOUND 1
#define MC_SAVE_FILE_FOUND 2
#define MC_SAVE_COMPLETE (MC_ICON_SYS_FOUND | MC_SAVE_FILE_FOUND)
/* The save file of this game: its name prefix (the folder name) and its size. */
#define MC_SAVE_NAME_LENGTH 17 /* "BASLUS-21389Xeno3" */
#define MC_SAVE_FILE_BYTES 0x130B0
/* Steps of a read or write request (xglMcCardState.substate). */
#define MC_RW_OPENED 0
#define MC_RW_TRANSFERRED 1
#define MC_RW_OPEN_ICON 2
#define MC_RW_WRITE_ICON 3
#define MC_RW_ICON_WRITTEN 4
#define MC_RW_LIST_FOLDER 5
#define MC_RW_CHECK_ICON 6

int sceMcClose(int fileDescriptor);
int sceMcWrite(int fileDescriptor, const void *contents, int contentsSize);
int strncmp(const char *left, const char *right, unsigned int length);

/* XS2 xglMcMain: one step of the memory-card state machine. Without a finished card operation it
 * returns the card status (busy while requests wait); otherwise it handles the result of the
 * operation for the current request phase, starts the next queued request or the next status
 * poll, and returns the status word. XS3 adds the save count of a counting list request, the
 * save-file check of the list scan, and the icon.sys check after a read or write. */
int xglMcMain(void)
{
    XglClock clock;
    int command;
    int result;
    int sync;
    short *status;
    int stored;
    McScanWork *list;
    int (*callback)(int code, void *work);
    McDirectoryEntry *directory;
    McDirectoryEntry *scanEntry;
    McDirectoryEntry *folderEntry;
    int folderIndex;
    unsigned int latest;
    unsigned int timestamp;
    int latestSlot;
    int index;
    int entry;
    int slot;
    int created;
    int cardType;
    int scan;
    unsigned char card;
    unsigned char phase;
    void *transfer;
    int size;
    unsigned char *data;
    int chosen;
    int selected;
    char *end;
    McQueueEntry *request;
    unsigned char *clearSlot;
    int found;

    sync = sceMcSync(1, &command, &result);
    if (sync == 0) {
        if (xglMcCardBlock.cardState.requestPhase == MC_STATE_POLL_INFO && xglMcQueueEnd == xglMcQueueTop) {
            return xglMcCardBlock.cardState.card_status.combined;
        }
        return MC_STATUS_BUSY_BOTH;
    }
    if (sync == -1) {
        result = 0;
        command = 0;
    }
    switch (xglMcCardBlock.cardState.requestPhase) {
    case MC_STATE_POLL_INFO:
        if (xglMcQueueEnd == xglMcQueueTop) {
            status = &xglMcCardBlock.cardState.card_status.perCardStatus[xglMcCardBlock.cardState.fileDescriptor];
            switch (xglMcCardBlock.cardState.card_type) {
            case MC_TYPE_PS1:
                *status = MC_STATUS_PS1_CARD;
                break;
            case MC_TYPE_PS2:
                *status = MC_STATUS_READY;
                break;
            case MC_TYPE_POCKETSTATION:
                *status = MC_STATUS_POCKETSTATION;
                break;
            default:
                *status = MC_STATUS_NONE;
                break;
            }
            if (result == -1 || result == -2) {
                *status = MC_STATUS_ERROR;
            } else {
                cardType = xglMcCardBlock.cardState.card_type;
                if (cardType == MC_TYPE_PS2 && result == 0 && xglMcCardBlock.cardState.formatted == 0) {
                    *status = MC_STATUS_UNFORMATTED;
                }
            }
        }
        xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
    case MC_STATE_IDLE:
        if (xglMcQueueEnd != xglMcQueueTop) {
            request = &xglMcQueue[xglMcQueueTop];
            savedMcRequest = *request;
            xglMcStartQueuedRequest(request);
            xglMcQueueTop = (xglMcQueueTop + 1) & MC_QUEUE_MASK;
            return MC_STATUS_BUSY_BOTH;
        }
        card = xglMcCardBlock.cardState.next_card & 1;
        xglMcCardBlock.cardState.fileDescriptor = card;
        sceMcGetInfo(card, 0, &xglMcCardBlock.cardState.card_type, 0, &xglMcCardBlock.cardState.formatted);
        xglMcCardBlock.cardState.requestPhase = MC_STATE_POLL_INFO;
        xglMcCardBlock.cardState.next_card ^= 1;
        break;
    case MC_STATE_OPEN_CLOSE:
        *xglMcCardBlock.cardState.activeRequest.callerResult = result;
        xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
        if (result < 0) {
            break;
        }
        sceMcClose(result);
        return MC_STATUS_BUSY_BOTH;
    case MC_STATE_EXIST_SCAN:
    case MC_STATE_LIST:
    case MC_STATE_CHECK_OLD:
        list = (McScanWork *)xglMcCardBlock.cardState.activeRequest.bufferAddress;
        callback = xglMcCardBlock.cardState.activeRequest.callback;
        while ((stored = result) < 0) {
            if (xglMcCardBlock.cardState.phaseAndStep == MC_PHASE_STEP(MC_STATE_CHECK_OLD, MC_SCAN_READ)
                && xglMcCardBlock.cardState.activeRequest.file == MC_OLD_SEARCH_LAST) {
                list->scanStatus = MC_LIST_FAILED;
                if (result >= MC_RESULT_SOFT_LIMIT) {
                    callback(MC_CALLBACK_NO_DATA, list);
                } else {
                    callback(MC_CALLBACK_ERROR, list);
                }
            } else {
                if (stored >= MC_RESULT_SOFT_LIMIT
                    && (xglMcCardBlock.cardState.substate == MC_SCAN_REPORT
                        || xglMcCardBlock.cardState.substate == MC_SCAN_ICON_CHECK)) {
                    break;
                }
                if (xglMcCardBlock.cardState.requestPhase == MC_STATE_EXIST_SCAN) {
                    break;
                }
                if (xglMcCardBlock.cardState.substate == MC_SCAN_READ) {
                    break;
                }
                if (xglMcCardBlock.cardState.substate == MC_SCAN_COUNT) {
                    break;
                }
                list->scanStatus = MC_LIST_FAILED;
                callback(MC_CALLBACK_ERROR, list);
            }
            xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
            return MC_STATUS_BUSY_BOTH;
        }
        switch (xglMcCardBlock.cardState.substate) {
        case MC_SCAN_DIRECTORY:
            *xglMcCardBlock.cardState.activeRequest.callerResult = stored;
            /* Clears the slot table, latest, status and the save count. */
            index = MC_LATEST_SLOT_INDEX + 2;
            clearSlot = &list->saveCount;
            do {
                index--;
                *clearSlot = 0;
                clearSlot--;
            } while (index >= 0);
            if (result > 0) {
                directory = (McDirectoryEntry *)MC_DIRECTORY_TABLE((unsigned int)list);
                latest = 0;
                index = 0;
                latestSlot = 0;
                while (index < result) {
                    slot = 0;
                    for (end = directory->name; *end != 0; end++) {
                    }
                    if ((end[-2] & 0xF0) == '0' && (end[-1] & 0xF0) == '0') {
                        slot = end[-2] * 10 + end[-1] - ('0' * 10 + '0');
                    }
                    list->cardScanState[slot] = MC_SLOT_WANTED;
                    clock.year = directory->year;
                    clock.month = directory->month;
                    clock.day = directory->day;
                    clock.hour = directory->hour;
                    clock.minute = directory->minute;
                    clock.second = directory->seconds;
                    clock.status = 0;
                    timestamp = xglClockDayTime2UInt(&clock);
                    if (latest < timestamp) {
                        latest = timestamp;
                        latestSlot = slot;
                    }
                    index++;
                    directory++;
                }
                list->latest = latestSlot;
                list->scanStatus = MC_LIST_FOUND;
            }
            if (xglMcCardBlock.cardState.requestPhase == MC_STATE_EXIST_SCAN) {
                xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
            } else if (xglMcCardBlock.cardState.requestPhase == MC_STATE_LIST) {
                chosen = callback(MC_CALLBACK_DIRECTORY, list);
                if (chosen < 0) {
                    chosen = list->latest;
                }
                xglMcCardBlock.cardState.selector = chosen;
            } else {
                xglMcCardBlock.cardState.selector = list->latest;
            }
            xglMcCardBlock.cardState.substate = MC_SCAN_NEXT_CARD;
            return MC_STATUS_BUSY_BOTH;
        case MC_SCAN_REPORT:
            selected = -1;
            if (xglMcCardBlock.cardState.fileDescriptor >= 0) {
                sceMcClose(xglMcCardBlock.cardState.fileDescriptor);
            }
            list->cardScanState[xglMcCardBlock.cardState.card] = MC_SLOT_READ;
            if (xglMcCardBlock.cardState.requestPhase == MC_STATE_LIST) {
                if (list->iconFlags == 0) {
                    memset(list->saveContents, 0, xglMcCardBlock.cardState.activeRequest.contentsSize);
                }
                selected = callback(xglMcCardBlock.cardState.card, list->saveContents);
            } else {
                data = list->saveContents;
                switch ((unsigned int)xglMcCardBlock.cardState.activeRequest.file) {
                case 0:
                case 1:
                    data[0] = 0;
                    if (data[MC_OLD_CLEAR_FLAG] == MC_OLD_CLEAR_VALUE) {
                        data[0] = 1;
                    }
                    list->saveContents[1] = data[MC_OLD_INFO_1];
                    list->saveContents[2] = data[MC_OLD_INFO_2];
                    list->saveContents[3] = data[MC_OLD_INFO_3];
                    list->saveContents[4] = 0;
                    list->saveContents[5] = data[MC_OLD_INFO_5];
                    list->saveContents[6] = data[MC_OLD_INFO_6];
                    list->saveContents[7] = data[MC_OLD_INFO_7];
                    break;
                case 2:
                    data[0] = data[MC_OLD2_FLAG];
                    list->saveContents[1] = data[MC_OLD2_INFO_1];
                    break;
                case MC_OLD_SEARCH_LAST:
                    data[0] = 0;
                    if (data[MC_OLD3_FLAG] != 0) {
                        data[0] = 1;
                    }
                    break;
                }
                callback(xglMcCardBlock.cardState.activeRequest.file, data);
            }
            if (selected >= 0) {
                xglMcCardBlock.cardState.selector = selected;
            }
            xglMcCardBlock.cardState.substate = MC_SCAN_NEXT_CARD;
            return MC_STATUS_BUSY_BOTH;
        case MC_SCAN_NEXT_CARD:
            if (xglMcCardBlock.cardState.requestPhase == MC_STATE_LIST) {
                scan = xglMcStartDirectoryScan();
                xglMcCardBlock.cardState.substate = MC_SCAN_ICON_CHECK;
            } else {
                scan = xglMcOpenNearestSave();
                xglMcCardBlock.cardState.substate = MC_SCAN_READ;
            }
            switch (scan) {
            case 0:
                return MC_STATUS_BUSY_BOTH;
            case -1:
                xglMcCardBlock.cardState.substate = MC_SCAN_DIRECTORY;
                return MC_STATUS_BUSY_BOTH;
            case 1:
                list->scanStatus = MC_LIST_FAILED;
                callback(MC_CALLBACK_NO_DATA, list);
                if (xglMcCardBlock.cardState.activeRequest.file < 0) {
                    *xglMcCardBlock.cardState.activeRequest.callerResult = list->saveCount;
                }
                xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
                break;
            }
            break;
        case MC_SCAN_READ:
            xglMcCardBlock.cardState.fileDescriptor = result;
            if (result >= 0) {
                if (xglMcCardBlock.cardState.requestPhase == MC_STATE_LIST) {
                    sceMcRead(stored, list->saveContents, xglMcCardBlock.cardState.activeRequest.contentsSize);
                } else {
                    sceMcRead(stored, list->saveContents, MC_OLD_SAVE_BYTES);
                }
            } else {
                list->iconFlags = 0;
            }
            xglMcCardBlock.cardState.substate = MC_SCAN_REPORT;
            return MC_STATUS_BUSY_BOTH;
        case MC_SCAN_ICON_CHECK:
            list->iconFlags = 0;
            if (result >= 0) {
                scanEntry = (McDirectoryEntry *)MC_DIRECTORY_TABLE((unsigned int)list);
                for (entry = 0; entry < result; entry++, scanEntry++) {
                    if (strcmp(scanEntry->name, "icon.sys") == 0 && scanEntry->fileSize == sizeof(McIconSys)) {
                        list->iconFlags |= MC_ICON_SYS_FOUND;
                    }
                    if (strncmp(scanEntry->name, "BASLUS-21389Xeno3", MC_SAVE_NAME_LENGTH) == 0
                        && scanEntry->fileSize == MC_SAVE_FILE_BYTES) {
                        list->iconFlags |= MC_SAVE_FILE_FOUND;
                    }
                }
            }
            xglMcSetFullPath(xglMcCardBlock.cardState.card, 0);
            sceMcOpen(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, MC_OPEN_READ);
            xglMcCardBlock.cardState.substate = MC_SCAN_READ;
            if (xglMcCardBlock.cardState.activeRequest.file < 0) {
                xglMcCardBlock.cardState.substate = MC_SCAN_COUNT;
            }
            return MC_STATUS_BUSY_BOTH;
        case MC_SCAN_COUNT:
            list->cardScanState[xglMcCardBlock.cardState.card] = 0;
            if (result >= 0) {
                sceMcClose(result);
                if (list->iconFlags == MC_SAVE_COMPLETE) {
                    list->cardScanState[xglMcCardBlock.cardState.card] = MC_SLOT_READ;
                    list->saveCount++;
                    chosen = callback(xglMcCardBlock.cardState.card, 0);
                    if (chosen >= 0) {
                        xglMcCardBlock.cardState.selector = chosen;
                    }
                }
            }
            xglMcCardBlock.cardState.substate = MC_SCAN_NEXT_CARD;
            return MC_STATUS_BUSY_BOTH;
        }
        break;
    case MC_STATE_CREATE:
        created = xglMcAdvanceCreateRequest(result);
        if (created == 0) {
            return MC_STATUS_BUSY_BOTH;
        }
        xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
        if (created < 0) {
            xglMcCardBlock.cardState.card_status.combined = MC_STATUS_BUSY_BOTH;
        }
        break;
    case MC_STATE_WRITE:
    case MC_STATE_READ:
        if (result >= 0) {
            switch (xglMcCardBlock.cardState.substate) {
            case MC_RW_OPENED:
                xglMcCardBlock.cardState.fileDescriptor = result;
                phase = xglMcCardBlock.cardState.requestPhase;
                size = xglMcCardBlock.cardState.activeRequest.contentsSize;
                transfer = (void *)xglMcCardBlock.cardState.activeRequest.bufferAddress;
                switch (phase) {
                case MC_STATE_WRITE:
                    sceMcWrite(result, transfer, size);
                    break;
                case MC_STATE_READ:
                    sceMcRead(result, transfer, size);
                    break;
                }
                xglMcCardBlock.cardState.substate = MC_RW_TRANSFERRED;
                break;
            case MC_RW_TRANSFERRED:
            case MC_RW_ICON_WRITTEN:
                sceMcClose(xglMcCardBlock.cardState.fileDescriptor);
                if (xglMcCardBlock.cardState.phaseAndStep == MC_PHASE_STEP(MC_STATE_WRITE, MC_RW_TRANSFERRED)) {
                    xglMcCardBlock.cardState.substate = MC_RW_OPEN_ICON;
                } else {
                    xglMcCardBlock.cardState.substate = MC_RW_LIST_FOLDER;
                }
                break;
            case MC_RW_OPEN_ICON:
                xglMcSetFullPath(xglMcCardBlock.cardState.activeRequest.file, 1);
                sceMcOpen(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path,
                          MC_OPEN_WRITE_CREATE);
                xglMcCardBlock.cardState.substate = MC_RW_WRITE_ICON;
                break;
            case MC_RW_WRITE_ICON:
                xglMcCardBlock.cardState.fileDescriptor = result;
                memset(xglMcCardBlock.icon.reserved, 0, sizeof(xglMcCardBlock.icon.reserved));
                sceMcWrite(xglMcCardBlock.cardState.fileDescriptor, &xglMcCardBlock.icon, sizeof(McIconSys));
                xglMcCardBlock.cardState.substate = MC_RW_ICON_WRITTEN;
                break;
            case MC_RW_LIST_FOLDER:
                xglMcSetFullPath(xglMcCardBlock.cardState.activeRequest.file, (int)xglMcAnyFolderName);
                func_002F9A40(xglMcCardBlock.cardState.activeRequest.port, 0, xglMcCardBlock.cardState.full_path, 0,
                              MC_ICON_CHECK_ENTRIES, (int)xglMcCardBlock.iconCheck.directory);
                xglMcCardBlock.cardState.substate = MC_RW_CHECK_ICON;
                break;
            case MC_RW_CHECK_ICON:
                found = 0;
                if (result > 0) {
                    folderEntry = xglMcCardBlock.iconCheck.directory;
                    for (folderIndex = 0; folderIndex < result; folderEntry++, folderIndex++) {
                        if (strcmp(folderEntry->name, "icon.sys") == 0 && folderEntry->fileSize == sizeof(McIconSys)) {
                            found = 1;
                            break;
                        }
                    }
                }
                if (!found) {
                    memset((void *)xglMcCardBlock.cardState.activeRequest.bufferAddress, 0,
                           xglMcCardBlock.cardState.activeRequest.contentsSize);
                }
                xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
                break;
            }
            return MC_STATUS_BUSY_BOTH;
        }
        *xglMcCardBlock.cardState.activeRequest.callerResult = result;
        xglMcCardBlock.cardState.card_status.combined = MC_STATUS_BUSY_BOTH;
        xglMcCardBlock.cardState.requestPhase = MC_STATE_IDLE;
        break;
    }
    return xglMcCardStatusWord[0];
}

int xglMcEasyLoad(const char *path, void *loadedContents, int loadedSize)
{
    int card_type;
    int command;
    int syncResult;
    int fileDescriptor;
    int loadedBytes;

    sceMcGetInfo(0, 0, 0, 0, &card_type);
    sceMcSync(0, &command, &syncResult);
    if (card_type == 0)
    {
        return -1;
    }
    if (syncResult < -1)
    {
        return syncResult;
    }

    sceMcOpen(0, 0, path, MC_OPEN_READ);
    sceMcSync(0, &command, &syncResult);
    fileDescriptor = syncResult;
    if (fileDescriptor < 0)
    {
        return fileDescriptor;
    }

    sceMcRead(fileDescriptor, loadedContents, loadedSize);
    sceMcSync(0, &command, &syncResult);
    loadedBytes = syncResult;
    if (loadedBytes < 0)
    {
        return loadedBytes;
    }

    sceMcClose(fileDescriptor);
    sceMcSync(0, &command, &syncResult);
    if (syncResult < 0)
    {
        return syncResult;
    }
    return loadedBytes;
}

int xglMcEasySave(const char *path, const void *contentsToSave, int saveSize)
{
    int card_type;
    int command;
    int syncResult;
    int fileDescriptor;

    sceMcGetInfo(0, 0, 0, 0, &card_type);
    sceMcSync(0, &command, &syncResult);
    if (syncResult < -1 || card_type == 0)
    {
        return -1;
    }

    sceMcOpen(0, 0, path, MC_OPEN_WRITE_CREATE);
    sceMcSync(0, &command, &syncResult);
    fileDescriptor = syncResult;
    if (fileDescriptor < 0)
    {
        return -1;
    }

    sceMcWrite(fileDescriptor, contentsToSave, saveSize);
    sceMcSync(0, &command, &syncResult);
    if (syncResult < 0)
    {
        return -1;
    }

    sceMcClose(fileDescriptor);
    sceMcSync(0, &command, &syncResult);
    return syncResult >= 0 ? 0 : -1;
}
