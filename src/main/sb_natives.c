#include "common.h"
#include "shared.h"

#include "main/tu105.h"



/* Messages in a thread's queue (sbQueueReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF



struct SbThread;
void sbFatal(struct SbThread *thread);
void sbFatalReport(struct SbThread *thread, int pc);

/* The argument count of the current call: the word below the stack top. */
int VM_PLUGIN_CNT_GET(SbThread *thread)
{
    return thread->stack[thread->sp - 1];
}

/* Returns the word of argument argumentNumber (1-based) of the current call. */
int vmArgGet(SbThread *thread, int argumentNumber)
{
    int *frameTop;
    int argumentCount;

    frameTop = thread->stack + thread->sp;
    argumentCount = frameTop[-1];
    if (argumentCount < argumentNumber || argumentNumber == 0) {
        sbFatal(thread);
    }
    return frameTop[-2 - (argumentCount - argumentNumber)];
}

extern f32 gSbArgFloatScale;

/* Returns argument argumentNumber (1-based) of the current call as a float, scaled by gSbArgFloatScale. */
f32 vmArgFloatGet(SbThread *thread, int argumentNumber)
{
    return vmArgGet(thread, argumentNumber) * gSbArgFloatScale;
}

/* Returns the tag byte of argument argumentNumber (1-based) of the current call. */
int vmArgTagGet(SbThread *thread, int argumentNumber)
{
    int *frameTop;
    signed char *tagTop;
    int argumentCount;

    frameTop = thread->stack + thread->sp;
    tagTop = thread->tags + thread->sp;
    argumentCount = frameTop[-1];
    if (argumentCount < argumentNumber || argumentNumber == 0) {
        sbFatal(thread);
    }
    return tagTop[-2 - (argumentCount - argumentNumber)];
}

/*
 * Reports whether argument argumentNumber has a zero tag. An index past the frame's argument count returns 1.
 */
int vmArgTagIsZero(SbThread *thread, int argumentNumber)
{
    int *frameTop;
    signed char *tagTop;
    int argumentCount;

    frameTop = thread->stack + thread->sp;
    tagTop = thread->tags + thread->sp;
    if (argumentNumber == 0) {
        sbFatal(thread);
    }
    argumentCount = frameTop[-1];
    if (!(argumentCount >= argumentNumber)) {
        return 1;
    }
    return tagTop[-2 - (argumentCount - argumentNumber)] == 0;
}

/* Returns argument argumentNumber of the current call when its tag is nonzero, else fallbackValue. */
int vmArgGetOrDefault(SbThread *thread, int argumentNumber, int fallbackValue)
{
    int argumentValue;

    argumentValue = fallbackValue;
    if (vmArgTagIsZero(thread, argumentNumber) == 0) {
        argumentValue = vmArgGet(thread, argumentNumber);
    }
    return argumentValue;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D41E0);

/* A keyed table: the words of its index array start keyIndexOffset bytes from the table, count entries long; the
 * entry data follows the count index words. */
struct SbKeyTable {
    int keyIndexOffset;                   /* +0x00 */
    int keyCount;                          /* +0x04 */
};

/* Table of the image's keys (a leaf in main/tu105). */
struct SbKeyTable *sbKeyTable(struct SbImageHeader *image);

/* Address of the keyed entry `key`, or 0 when key is past the table. */
char *vmKeyedEntryGet(SbThread *thread, int key)
{
    struct SbKeyTable *keyTable;
    char *keyIndexArray;

    keyTable = sbKeyTable(thread->image);
    if (!(key < keyTable->keyCount)) {
        return 0;
    }
    keyIndexArray = (char *)keyTable + keyTable->keyIndexOffset;
    return keyIndexArray + keyTable->keyCount * 4 + ((int *)keyIndexArray)[key];
}

/* Address of entry `index` of the library image linked into the thread's image (image->keys): the entry
 * count is at the header's +8, the name index array at its name_table offset, the entry data at the
 * offset stored at +0x18 (a field the SbImageHeader struct does not name yet). */
void *sbLibraryEntryGet(SbThread *thread, int libraryEntryIndex)
{
    struct SbImageHeader *library;
    char *nameIndexArray;

    library = thread->image->keys;
    if (libraryEntryIndex >= library->entry) {
        return 0;
    }
    nameIndexArray = (char *) library + library->name_table;
    return (char *) library + ((int *) library)[0x18 / 4] + ((int *) nameIndexArray)[libraryEntryIndex];
}

/* Base of the image's global table (a leaf in main/tu105). */
int *sbGlobalSection(struct SbImageHeader *image);

/* Address of global word `index` of the thread's image, as vmEntryGet computes it. */
int sbGlobalEntryGet(SbThread *thread, int globalWordNumber)
{
    int globalTableAddress;

    globalTableAddress = (int)sbGlobalSection(thread->image);
    return globalTableAddress + sbGlobalSection(thread->image)[0] + globalWordNumber * 4;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4330);

/* Empty in this build: sbFatal passes the thread and its pc, as for a debug report. */
void sbFatalReport(SbThread *thread, int pc)
{
}

extern unsigned char gSbStudioBlock[];
void cameraTablesReset();
void cameraPendingClear();
void cameraSlotsReset();
struct XglCamera *xglStudioGetCamera2(int camera_index);

/* The xgl camera fields this file touches (the full struct is in main/xgl_2.h, not included here). */
struct XglCamera {
    unsigned int active;
    unsigned int cameraState;
};

/* The block at gSbStudioBlock; its 64-bit word at +0x10 has bit 0 cleared by sbStudioCameraReset. */
typedef struct SbStudioBlock {
    unsigned char unmodeled_00[0x10];
    long long studioFlags; /* +0x10 */
} SbStudioBlock;

/* The status block at gSbStudioBlock as an lvalue. */
#define SB_STUDIO_BLOCK (*(SbStudioBlock *) gSbStudioBlock)

void sbStudioCameraReset(void)
{
    long long studioFlags;

    xglStudioGetCamera2(0)->cameraState = 4;
    cameraTablesReset();
    cameraSlotsReset();
    cameraPendingClear();
    studioFlags = SB_STUDIO_BLOCK.studioFlags & (~1);
    SB_STUDIO_BLOCK.studioFlags = studioFlags;
}

extern signed char D_003E0173[];

void sbStudioCameraResetIfFlagged(void) {
    if (D_003E0173[0] == 1) {
        cameraTablesReset();
        cameraSlotsReset();
        cameraPendingClear();
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sb_natives", func_001D4448);
