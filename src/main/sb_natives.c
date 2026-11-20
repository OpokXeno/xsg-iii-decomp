#include "common.h"
#include "shared.h"

#include "main/tu105.h"



/* Messages in a thread's queue (sbChainReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF



struct SbThread;
void sbFatal(struct SbThread *thread);
void sbFatalReport(struct SbThread *thread, int pc);

/* The argument count of the current call: the word below the stack top. */
int VM_PLUGIN_CNT_GET(SbThread *thread)
{
    return thread->stack[thread->sp - 1];
}

/* Returns the word of argument argIndex (1-based) of the current call. */
int vmArgGet(SbThread *thread, int argIndex)
{
    int *frameTop;
    int argCount;

    frameTop = thread->stack + thread->sp;
    argCount = frameTop[-1];
    if (argCount < argIndex || argIndex == 0) {
        sbFatal(thread);
    }
    return frameTop[-2 - (argCount - argIndex)];
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4058);

/* Returns the tag byte of argument argIndex (1-based) of the current call. */
int vmArgTagGet(SbThread *thread, int argIndex)
{
    int *frameTop;
    signed char *tagTop;
    int argCount;

    frameTop = thread->stack + thread->sp;
    tagTop = thread->tags + thread->sp;
    argCount = frameTop[-1];
    if (argCount < argIndex || argIndex == 0) {
        sbFatal(thread);
    }
    return tagTop[-2 - (argCount - argIndex)];
}

/*
 * Reports whether argument argIndex has a zero tag. An index past the frame's argument count returns 1.
 */
int vmArgTagIsZero(SbThread *thread, int argIndex)
{
    int *frameTop;
    signed char *tagTop;
    int argCount;

    frameTop = thread->stack + thread->sp;
    tagTop = thread->tags + thread->sp;
    if (argIndex == 0) {
        sbFatal(thread);
    }
    argCount = frameTop[-1];
    if (!(argCount >= argIndex)) {
        return 1;
    }
    return tagTop[-2 - (argCount - argIndex)] == 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4188);

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D41E0);

/* A keyed table: the words of its index array start index_offset bytes from the table, count entries long; the
 * entry data follows the count index words. */
struct SbKeyTable {
    int index_offset;                   /* +0x00 */
    int count;                          /* +0x04 */
};

/* Table of the image's keys (a leaf in main/tu105). */
struct SbKeyTable *func_001D1340(struct SbImageHeader *image);

/* Address of the keyed entry `key`, or 0 when key is past the table. */
char *vmKeyedEntryGet(SbThread *thread, int key)
{
    struct SbKeyTable *table;
    char *indexBase;

    table = func_001D1340(thread->image);
    if (!(key < table->count)) {
        return 0;
    }
    indexBase = (char *)table + table->index_offset;
    return indexBase + table->count * 4 + ((int *)indexBase)[key];
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4298);

/* Base of the image's global table (a leaf in main/tu105). */
int *func_001D1328(struct SbImageHeader *image);

/* Address of global word `index` of the thread's image, as vmEntryGet computes it. */
int func_001D42D8(SbThread *thread, int index)
{
    int base;

    base = (int)func_001D1328(thread->image);
    return base + func_001D1328(thread->image)[0] + index * 4;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4330);

/* Empty in this build: sbFatal passes the thread and its pc, as for a debug report. */
void sbFatalReport(SbThread *thread, int pc)
{
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D43B0);

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4400);

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4448);
