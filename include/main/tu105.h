/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_TU105_H
#define INCLUDE_MAIN_TU105_H

/* Types owned by main/tu105 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/* One 0x10-byte message of a thread's queue (sbThreadEnqueue): the caller's
 * words, with sender replaced by the sending thread's value (-1 for none). */
typedef struct SbMessage SbMessage;

/* One native method record (12 bytes, the sbNativeOwnerAdd copy moves it whole): name, the function, a third word. */
typedef struct SbNativeMethod SbNativeMethod;

/* A thread's message queue (sbQueueGet, 0x114 bytes): a ring of message indices from
 * first to last, the read index head, the write index tail and the count used;
 * sbThreadEnqueue writes messages[tail], sbThreadDequeue reads messages[head]. */
typedef struct SbQueue SbQueue;

/* One 0x10-byte slot of a thread's slot table: key 0 marks a free slot.
 * sbThreadSlotStore copies whole slots; the rest of the slot is not read here. */
typedef struct SbSlot SbSlot;

/*
 * One 0x48-byte record of sbThreadTable: a script thread. image (+0x18) is its key
 * (0 marks a free record; sbFindThread matches it with value), pc counts halfwords
 * from the image's code, the operand stack (+0x20, stackLimit words) holds sp words
 * and the thread's locals, slots are its jump slots and queue (+0x30) its messages.
 */
typedef struct SbThread SbThread;

/* The SB image header (also the header of an SBL library image: code_offset is then the offset of its entry index array and
 * global_offset the offset of its entry data): magic at +0, the native name count at +0xC, the code
 * offset at +0x14, the global table offset at +0x18 and the name table offset
 * at +0x20 (offsets count from the header). */
struct SbImageHeader {
    int magic;                          /* +0x00: "SB  " (sbIsSbImage) or "SBL " (sbIsSblImage) */
    int flags;                          /* +0x04 */
    int entry;                          /* +0x08: offset of the code table (word 1 is the entry position) */
    int global_table;                   /* +0x0C: offset of the global table */
    int import_table;                   /* +0x10 */
    int name_table;                     /* +0x14: offset of the name table (sbNameTableFind) */
    unsigned char unmodeled_18[0x4];    /* +0x18 */
    int keyed_table;                    /* +0x1C: offset of the keyed entry table */
    struct SbImageHeader *keys;         /* +0x20: the SBL library image linked in (sbLibraryLink stores it) */
    unsigned char unmodeled_24[0x8];    /* +0x24 */
};

/* One 0x10-byte message of a thread's queue (sbThreadEnqueue): the caller's
 * words, with sender replaced by the sending thread's value (-1 for none). */
struct SbMessage {
    int kind;                           /* +0x00 */
    int sender;                         /* +0x04 */
    int args[2];                        /* +0x08 */
};

/* One native method record (12 bytes, the sbNativeOwnerAdd copy moves it whole): name, the function, a third word. */
struct SbNativeMethod {
    const char *name;                   /* +0x00: a table ends with a null name */
    int (*func)();                      /* +0x04: native; unprototyped (thread, status word or nothing) */
    unsigned char unmodeled_08[0x4];    /* +0x08 */
};

/* A thread's message queue (sbQueueGet, 0x114 bytes): a ring of message indices from
 * first to last, the read index head, the write index tail and the count used;
 * sbThreadEnqueue writes messages[tail], sbThreadDequeue reads messages[head]. */
struct SbQueue {
    int ringFirstIndex;                          /* +0x00 */
    int ringLastIndex;                           /* +0x04 */
    int head;                           /* +0x08: read index */
    int tail;                           /* +0x0C: write index */
    int used;                           /* +0x10 */
    SbMessage messages[0x10];           /* +0x14: SB_QUEUE_LAST + 1 entries */
};

/* One 0x10-byte slot of a thread's slot table: key 0 marks a free slot.
 * sbThreadSlotStore copies whole slots; the rest of the slot is not read here. */
struct SbSlot {
    int key;                            /* +0x00: 0 marks a free slot */
    int target;                         /* +0x04 */
    int low;                            /* +0x08 */
    int high;                           /* +0x0C */
};

/*
 * One 0x48-byte record of sbThreadTable: a script thread. image (+0x18) is its key
 * (0 marks a free record; sbFindThread matches it with value), pc counts halfwords
 * from the image's code, the operand stack (+0x20, stackLimit words) holds sp words
 * and the thread's locals, slots are its jump slots and queue (+0x30) its messages.
 */
struct SbThread {
    int pc;                             /* +0x00 */
    int sp;                             /* +0x04: stack words in use */
    int frame;                          /* +0x08: stack index of the local frame */
    unsigned char unmodeled_0C[0x4];    /* +0x0C */
    int jumpKey;                        /* +0x10: pending slot jump */
    int jumpPosition;                   /* +0x14 */
    struct SbImageHeader *image;        /* +0x18: key (0 marks a free record) */
    int entryPc;                          /* +0x1C */
    int *stack;                         /* +0x20 */
    unsigned char *tags;                /* +0x24: one byte per stack word */
    int stackLimit;                     /* +0x28 */
    SbSlot *slots;                      /* +0x2C */
    SbQueue *queue;                     /* +0x30 */
    int status;                         /* +0x34: suspended bit 0x80000000 */
    int wait[4];                        /* +0x38 */
};

/* One native owner record (12 bytes): owner name, its method table and the method count. */
typedef struct SbNativeOwner {
    const char *name;                   /* +0x00 */
    SbNativeMethod *methods;            /* +0x04 */
    int count;                          /* +0x08 */
} SbNativeOwner;

extern SbNativeOwner sbNativeOwners[];
/* The methods words of the owner records, one 12-byte record per row (label 0x616CC4). */
extern SbNativeMethod *D_00616CC4[][3];
extern int sbNativeOwnerCount;

#endif
