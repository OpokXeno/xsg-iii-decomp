#include "common.h"
#include "shared.h"
#include "main/tu105.h"
#include "main/battle_shared.h"

/* One entry of an image's name table: the name (offset from the header) and its value. */
typedef struct SbNameEntry {
    int name_offset;
    int entryPc;
} SbNameEntry;

/* Thread records of sbThreadTable, and the per-thread areas sbInit clears. */
#define SB_THREAD_COUNT 32
/* Words of a thread's operand stack (0x200 bytes). */
#define SB_STACK_WORDS 0x80
/* Slots in a thread's slot table (sbThreadSlotFind and sbThreadSlotStore scan 8). */
#define SB_SLOT_COUNT 8
/* Messages in a thread's queue (sbQueueReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF
/* The suspended bit of a thread's status word (sbThreadSuspend sets it). */
#define SB_IMAGE_LINKED 0x80000000

/* Image headers start with these magic words: "SB  " and "SBL ". */
#define SB_IMAGE_MAGIC 0x20204253
#define SBL_IMAGE_MAGIC 0x204C4253
/* Number of opcodes; the handler table of sbRunOpcode has this many entries. */
#define SB_OPCODE_COUNT 99
/* Slot keys that sbOpBinary requests for a zero divisor and sbSlotJumpUnhandledKey skips. */
#define SB_SLOT_KEY_ZERO_DIVIDE 1
#define SB_SLOT_KEY_SKIP_OPERATION 2

void sbFatal(SbThread *thread);
int sbThreadEnqueue(struct SbImageHeader *key, int entryPc, SbMessage *message, SbThread *sender);
struct SbThread;
void sbFatalReport(struct SbThread *thread, int pc);
extern SbThread sbThreadTable[SB_THREAD_COUNT];
/* The per-thread areas: operand stack, slot table and message queue of thread index. */
extern int sbStacks[SB_THREAD_COUNT][SB_STACK_WORDS];
extern SbSlot sbThreadSlotTables[SB_THREAD_COUNT][SB_SLOT_COUNT];
extern SbQueue sbChains[SB_THREAD_COUNT];
int sbIsSbImage(struct SbImageHeader *header);
/* The thread's slot with this key, or 0. */
SbSlot *sbThreadSlotFind(SbThread *thread, int key);
int strcmp(const char *a, const char *b);
int *sbStackGet(int index);
SbSlot *sbThreadSlotsGet(int index);
SbQueue *sbQueueGet(int index);
SbThread *sbFindThread(struct SbImageHeader *key, int entryPc);
int sbQueuePush(SbThread *thread);
SbMessage *sbQueueEntryGet(SbThread *thread, int index);
/* Takes the next index from the thread's cursor ring; -1 when it is empty. */
int sbQueuePop(SbThread *thread);
int sbQueueCountGet(SbThread *thread);
int sbNameTableFind(struct SbImageHeader *image, const char *name);
SbMessage *sbThreadDequeue(struct SbImageHeader *key, int entryPc);
int sbImageLinkImports(struct SbImageHeader *image, struct SbImageHeader *library);
int *sbImageCodeTableGet(struct SbImageHeader *image);
/*
 * Creates a thread on the first free record of sbThreadTable for this image and entry: the record
 * takes the image, the entry and the stack, tag, slot and queue of its index; returns 1, or 0 when all
 * records are in use. The record is stored in the first zero word of sbThreadRing, from word 0 (the head
 * of the free-slot list); when no word is free it is not stored.
 */
int sbThreadStart(struct SbImageHeader *image, int entry);
int *sbImageKeyedEntryFind(struct SbImageHeader *image, int key);
void *sbLinkedLibraryGet(struct SbImageHeader *image);
int sbStackPop(SbThread *thread);
void sbGlobalVariableSet(SbThread *thread, int index, int globalVarContents);
int sbGlobalVariableGet(SbThread *thread, int index);
int sbOpLength(unsigned char opcode, int count);
extern void xglRenderClearFrame(void);
extern void xglCullingEnable(void);
extern void xglCullingDisable(void);
extern void soundSlotStop(int slot);
extern void soundSlotSetPause(int slot, int pause);
extern void soundSlotSetVolume(int slot, int volume, int unk);
extern void soundSlotSetDriverParam(int slot, int param);
extern void soundDriverSetEnable(int enable, int unk);
extern void func_A8E160(void);
extern void func_AB1A38(void);
extern void func_A9FF88(void);
extern void func_AA00F8(void);
extern void func_AA0910(void);
extern void func_AA0088(void);
extern void func_A85A88(void);
extern void func_A8E2F0(void);
extern void func_A85B38(void);
extern void func_A85B50(void);
extern void func_A96EA8(int mode);
extern void func_A85C68(void);
extern void runWithStateBlockOnce(int mode);
extern int D_00551304;
extern int D_004C83D0[];

/* Resets the render, sound and VM block state, then runs the state block once unless D_00551304 is set. */
void func_001D1108(void)
{
    int blockMode;
    int stateMode;
    int blockStatusWord;

    xglRenderClearFrame();
    xglCullingEnable();
    soundSlotStop(1);
    soundSlotSetPause(1, 0);
    soundSlotSetVolume(1, 127, 0);
    soundSlotSetDriverParam(1, 0);
    func_A8E160();
    func_AB1A38();
    D_00551304 = 0;
    if (gSbThreadBlock.threadFlags & 1) {
        func_A9FF88();
        func_AA00F8();
        func_AA0910();
        func_AA0088();
        blockMode = 1;
    } else {
        func_A85A88();
        func_A8E2F0();
        func_A85B38();
        func_A85B50();
        blockMode = 0;
    }
    func_A96EA8(blockMode);
    soundSlotStop(1);
    soundDriverSetEnable(0, 0);
    soundSlotSetDriverParam(1, 1);
    func_A85C68();
    D_004C83D0[0] = 0;
    blockStatusWord = gSbThreadBlock.threadFlags;
    stateMode = 0;
    if (blockStatusWord & 1) {
        stateMode = 1;
    }
    if (blockStatusWord & 0x10) {
        stateMode |= 4;
    }
    if (D_00551304 == 0) {
        runWithStateBlockOnce(stateMode);
    }
    xglCullingDisable();
}

typedef int s32;

extern unsigned char sbBaseBlockStorage[];

u8 *sbBaseBlockIdGet(void) {
    return sbBaseBlockStorage;
}

extern SbThread D_0060E140[32];
void sbNativeTablesReset(void);
extern SbThread *sbThreadRing[32];
static s32 sbVmStateWord;
extern unsigned char sbThreadTagBytes[];
void *memset(void *dest, int value, unsigned int count);

/* Clears the VM tables: the state word, every thread record, the ring, the operand stacks, tags, slots and queues. */
void sbClearVmTables(void)
{
    int threadIndex;

    sbVmStateWord = 0;
    for (threadIndex = SB_THREAD_COUNT - 1; threadIndex >= 0; threadIndex--) {
        D_0060E140[threadIndex].image = 0;
    }
    memset(sbThreadRing, 0, sizeof(sbThreadRing));
    memset(sbStacks, 0, sizeof(sbStacks));
    memset(sbThreadTagBytes, 0, 0x1000);
    memset(sbThreadSlotTables, 0, sizeof(sbThreadSlotTables));
    memset(sbChains, 0, sizeof(sbChains));
    sbNativeTablesReset();
}

static int sbNativeMethodCount;

static int sbNativeOwnerCount;

/* Empties the native method table and the owner table (sbNativeOwnerAdd appends to both). */
void sbNativeTablesReset(void)
{
    sbNativeMethodCount = 0;
    sbNativeOwnerCount = 0;
}

/* Address of the image's code table (the header's entry offset), or 0 when the image has none. */
int *sbImageCodeTableGet(struct SbImageHeader *image)
{
    int imageOffset;

    imageOffset = image->entry;
    if (imageOffset == 0) {
        return 0;
    }
    return (int *) ((u8 *) image + imageOffset);
}

/* A table offset in an image header is a byte offset from the header itself (read at runtime). */
#define BYTE_OFFSET(headerBase, tableOffset) ((u8 *) (headerBase) + (tableOffset))

unsigned char *sbGlobalSection(struct SbImageHeader *image);

unsigned char *sbGlobalSection(struct SbImageHeader *image)
{
    int imageOffset;

    imageOffset = image->global_table;
    if (imageOffset == 0) {
        return 0;
    }
    return BYTE_OFFSET(image, imageOffset);
}

void *sbKeyTable(struct SbImageHeader *image)
{
    int imageOffset;

    imageOffset = image->import_table;
    if (imageOffset == 0) {
        return 0;
    }
    return BYTE_OFFSET(image, imageOffset);
}

int *sbNameTable(struct SbImageHeader *image);

int *sbNameTable(struct SbImageHeader *image)
{
    int imageOffset;

    imageOffset = image->name_table;
    if (imageOffset == 0) {
        return 0;
    }
    return (int *) BYTE_OFFSET(image, imageOffset);
}

/* Address of the library image whose offset sits in the header's unmodeled_18 word, or 0 when there is none. */
void *sbLinkedLibraryGet(struct SbImageHeader *image)
{
    int libraryOffset;

    libraryOffset = *(int *) image->unmodeled_18;
    if (libraryOffset == 0) {
        return 0;
    }
    return (char *) image + libraryOffset;
}

/* Finds the keyed entry with this key in the image's keyed table. Each entry is a key word, a byte size word and
 * size bytes of data; a zero key ends the table. Returns 0 when there is none. */
int *sbImageKeyedEntryFind(struct SbImageHeader *image, int key)
{
    int *keyedEntry;

    if (image->keyed_table == 0) {
        return 0;
    }
    keyedEntry = (int *) ((u8 *) image + image->keyed_table);
    while (keyedEntry[0] != 0) {
        if (keyedEntry[0] == key) {
            return keyedEntry;
        }
        keyedEntry = (int *) ((u8 *) keyedEntry + keyedEntry[1] + 8);
    }
    return 0;
}

void sbThreadRingRebuild();
void sbRunThread();

void sbRunThreads(void) {
    int threadIndex;
    SbThread *thread;

    sbThreadRingRebuild();
    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        thread = sbThreadRing[threadIndex];
        if ((thread != 0) && (thread->image != 0) && !(thread->status & SB_IMAGE_LINKED)) {
            sbRunThread(thread);
        }
    }
}

/* Rebuilds sbThreadRing from the thread records that have an image, in table order. */
void sbThreadRingRebuild(void)
{
    int ringCount;
    int threadIndex;

    ringCount = 0;
    memset(sbThreadRing, 0, sizeof(sbThreadRing));
    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        if (D_0060E140[threadIndex].image != 0) {
            sbThreadRing[ringCount++] = &D_0060E140[threadIndex];
        }
    }
}

void sbRunThread(SbThread *thread) {
    s32 codeLimit;

    codeLimit = sbImageCodeTableGet(thread->image)[2];
    do {
        if (codeLimit < thread->pc) {
            sbFatal(thread);
        }
        sbSlotJump(thread);
    } while (sbRunOpcode(thread) == 0);
}

/* sbStartImage: starts the image at its code entry; an image that is not an SB image spins forever. */
void sbStartImage(struct SbImageHeader *image)
{
    if (!sbIsSbImage(image)) {
        for (;;) { }
    }
    sbThreadStart(image, sbImageCodeTableGet(image)[1]);
    do {
        sbImageKeyedEntryFind(image, 1);
    } while (0);
}

SbSlot *sbThreadSlotsGet(int threadIndex) {
    return sbThreadSlotTables[threadIndex];
}

/* The thread's slot with this key, or 0. */
SbSlot *sbThreadSlotFind(SbThread *thread, int key)
{
    SbSlot *candidate = thread->slots;
    int i;

    for (i = 0; i < SB_SLOT_COUNT; i++, candidate++) {
        if (candidate->key == key) {
            return candidate;
        }
    }
    return 0;
}

/* Copies entry into the slot with its key, else into the first free slot;
 * returns 0 when the table is full. */
int sbThreadSlotStore(SbThread *thread, SbSlot *incomingSlot)
{
    SbSlot *matchingSlot = sbThreadSlotFind(thread, incomingSlot->key);
    SbSlot *threadSlots;
    int i;

    if (matchingSlot == 0) {
        threadSlots = thread->slots;
        for (i = 0; i < SB_SLOT_COUNT; i++) {
            if (threadSlots[i].key == 0) {
                matchingSlot = &threadSlots[i];
                break;
            }
        }
    }
    if (matchingSlot != 0) {
        *matchingSlot = *incomingSlot;
        return 1;
    }
    return 0;
}

/* Frees the slot with this key; returns 0 when there is none. */
int sbThreadSlotRemove(SbThread *thread, int key)
{
    SbSlot *matchedSlot = sbThreadSlotFind(thread, key);

    if (matchedSlot == 0) {
        return 0;
    }
    matchedSlot->key = 0;
    return 1;
}

void sbSlotJumpRequest(SbThread *thread, int key)
{
    thread->jumpKey = key;
    thread->jumpPosition = thread->pc;
}

void sbSlotJumpUnhandledKey(SbThread *thread);

/* Runs the thread's pending slot jump (jumpKey at jumpPosition): an unknown key is
 * fatal; a position inside the slot's range jumps to its target. */
int sbSlotJump(SbThread *thread)
{
    int key;
    SbSlot *jumpSlot;
    int position;

    key = thread->jumpKey;
    if (key == 0) {
        return 0;
    }
    jumpSlot = sbThreadSlotFind(thread, key);
    if (jumpSlot == 0) {
        thread->pc = thread->jumpPosition;
        if (key < 3) {
            sbSlotJumpUnhandledKey(thread);
        }
        sbFatal(thread);
        return 1;
    }
    position = thread->jumpPosition;
    thread->jumpKey = 0;
    thread->jumpPosition = 0;
    if (position >= jumpSlot->low) {
        if (jumpSlot->high >= position) {
            thread->pc = jumpSlot->target;
            return 1;
        }
    }
    return 0;
}

/* Runs the pending slot jump of a thread whose jumpKey is 2: skips the operation at jumpPosition, then the operand bytes. */
void sbSlotJumpUnhandledKey(SbThread *thread)
{
    unsigned char opcode;
    int length;

    if (thread->jumpKey == SB_SLOT_KEY_ZERO_DIVIDE) {
        return;
    }
    if (thread->jumpKey == SB_SLOT_KEY_SKIP_OPERATION) {
        opcode = sbCodeReadUnsigned(thread, thread->jumpPosition, 1);
        length = sbOpLength(opcode, 0);
        sbCodeReadUnsigned(thread, thread->jumpPosition + 1, length - 1);
        sbLinkedLibraryGet(thread->image);
        sbLinkedLibraryGet(thread->image);
    }
}

/* Fatal VM error (stack overflow in sbRun): reports the thread, then stops for good. */
void sbFatal(SbThread *thread)
{
    sbFatalReport(thread, thread->pc);
    for (;;) {
    }
}

extern SbNativeMethod sbNativeMethodTable[];

/*
 * Registers a native owner: appends its methods (a table ending with a null name) to sbNativeMethodTable and
 * the owner to sbNativeOwners. Returns 0 when there are 128 owners or 1024 methods already.
 */
int sbNativeOwnerAdd(const char *name, SbNativeMethod *methods)
{
    int methodCount;

    if (sbNativeOwnerCount >= 128) {
        return 0;
    }
    sbNativeOwners[sbNativeOwnerCount].name = name;
    sbNativeOwners[sbNativeOwnerCount].methods = &sbNativeMethodTable[sbNativeMethodCount];
    for (methodCount = 0; methods[methodCount].name != 0; methodCount++) {
        if (sbNativeMethodCount >= 1024) {
            return 0;
        }
        sbNativeMethodTable[sbNativeMethodCount] = methods[methodCount];
        sbNativeMethodCount++;
    }
    sbNativeOwners[sbNativeOwnerCount].count = methodCount;
    sbNativeOwnerCount = sbNativeOwnerCount + 1;
    return 1;
}

int sbIsSbImage(struct SbImageHeader *image) {
    if (image == 0) {
        return 0;
    }
    return image->magic == SB_IMAGE_MAGIC;
}

int sbIsSblImage(struct SbImageHeader *image) {
    if (image == 0) {
        return 0;
    }
    return image->magic == SBL_IMAGE_MAGIC;
}

/*
 * Links the library image into image (sbLinkImage) and makes it the image's key table. Returns 0 when linking
 * fails; hangs on a library that is not an SBL image.
 */
int sbLibraryLink(struct SbImageHeader *image, struct SbImageHeader *library)
{
    if (sbImageLinkImports(image, library) == 0) {
        return 0;
    }
    if (!sbIsSblImage(library)) {
        for (;;) {
        }
    }
    image->keys = library;
    return 1;
}

int sbIsSbImage(struct SbImageHeader *image);
int sbFindNative(const char *ownerName, const char *methodName);
s32 sbStateWordGet(void);

struct SbImportTable {
    int imports_offset;
    int import_count;
};

static inline int sbImportCountGet(struct SbImageHeader *image) {
    return ((struct SbImportTable *) sbLinkedLibraryGet(image))->import_count;
}

static inline int *sbImportsGet(struct SbImageHeader *image) {
    return (int *) ((char *) sbLinkedLibraryGet(image) +
                    ((struct SbImportTable *) sbLinkedLibraryGet(image))->imports_offset);
}

/*
 * Resolves each import of the image to a native id, stored in place of the import word: the high half of the
 * word is the owner name offset, the low half the method name offset. An unknown native is kept as -1 when
 * the state word has bit 0; otherwise the link fails. Hangs on an image that is not an SB image.
 */
int sbImageLinkImports(struct SbImageHeader *image, struct SbImageHeader *library)
{
    int importCount;
    int *imports;
    char *names;
    int importIndex;
    int importWord;
    int nativeId;

    if (sbIsSbImage(image) == 0) {
        for (;;) {
        }
    }
    if (image->flags < 0) {
        return 1;
    }
    image->flags = image->flags | SB_IMAGE_LINKED;
    importCount = sbImportCountGet(image);
    imports = sbImportsGet(image);
    names = (char *) (imports + importCount);
    for (importIndex = 0; importIndex < importCount; importIndex++) {
        importWord = imports[importIndex];
        nativeId = sbFindNative(names + (importWord >> 16), names + (importWord & 0xFFFF));
        if (nativeId < 0) {
            if (sbStateWordGet() & 1) {
                nativeId = -1;
            } else {
                return 0;
            }
        }
        imports[importIndex] = nativeId;
    }
    return 1;
}

int strcmp(const char *a, const char *b);

/* Native id of a method: owner index in the high half, method index in the low half; -1 when the owner or the method is not registered. */
int sbFindNative(const char *ownerName, const char *methodName)
{
    int owner;
    int method;

    for (owner = 0; owner < sbNativeOwnerCount; owner++) {
        if (strcmp(ownerName, sbNativeOwners[owner].name) == 0) {
            for (method = 0; method < sbNativeOwners[owner].count; method++) {
                if (strcmp(methodName, D_00616CC4[owner][0][method].name) == 0) {
                    return (owner << 16) + method;
                }
            }
        }
    }
    return -1;
}

s32 sbStateWordGet(void) {
    return sbVmStateWord;
}

/* Sets the bits of the VM state word (sbVmStateWord). */
void sbVmStateBitsSet(s32 stateBits)
{
    s32 *vmStateWord;

    vmStateWord = &sbVmStateWord;
    *vmStateWord |= stateBits;
}

/* Clears the bits of the VM state word (sbVmStateWord) that are set in clearMask. */
void sbVmStateBitsClear(s32 clearMask)
{
    s32 *vmStateWord;

    vmStateWord = &sbVmStateWord;
    *vmStateWord &= ~clearMask;
}

SbThread *sbThreadTableGet(void) {
    return D_0060E140;
}

extern SbThread D_0060E140[SB_THREAD_COUNT];
extern SbThread *sbThreadRing[SB_THREAD_COUNT];
unsigned char *sbTagsGet(int index);
void sbQueueReset(SbThread *thread);

/* Creates a thread on the first free record of sbThreadTable for this image and entry: the record
 * takes the image, the entry and the stack, tag, slot and queue of its index; returns 1, or 0 when all
 * records are in use. The record is stored in the first zero word of sbThreadRing, from word 0 (the head
 * of the free-slot list); when no word is free it is not stored. */
int sbThreadStart(struct SbImageHeader *image, int entryPc)
{
    SbThread *thread;
    int threadIndex;
    int threadRingIndex;

    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        if (D_0060E140[threadIndex].image == 0) {
            thread = &D_0060E140[threadIndex];
            for (threadRingIndex = 0; threadRingIndex < SB_THREAD_COUNT; threadRingIndex++) {
                if (sbThreadRing[threadRingIndex] == 0) {
                    sbThreadRing[threadRingIndex] = thread;
                    break;
                }
            }
            memset(thread->wait, 0, 16);
            thread->pc = entryPc;
            thread->frame = 3;
            thread->image = image;
            thread->entryPc = entryPc;
            thread->sp = 0;
            thread->jumpKey = 0;
            thread->jumpPosition = 0;
            thread->stack = sbStackGet(threadIndex);
            thread->tags = sbTagsGet(threadIndex);
            thread->stackLimit = SB_STACK_WORDS;
            thread->slots = sbThreadSlotsGet(threadIndex);
            thread->queue = sbQueueGet(threadIndex);
            memset(thread->slots, 0, SB_SLOT_COUNT * sizeof(SbSlot));
            sbQueueReset(thread);
            thread->status = 0;
            return 1;
        }
    }
    return 0;
}

SbThread *sbFindThread(struct SbImageHeader *key, int entryPc)
{
    int threadIndex;

    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        if (D_0060E140[threadIndex].image == key && D_0060E140[threadIndex].entryPc == entryPc) {
            return &D_0060E140[threadIndex];
        }
    }
    return 0;
}

int sbThreadRelease(struct SbImageHeader *key, int entryPc)
{
    SbThread *thread = sbFindThread(key, entryPc);

    if (thread == 0) {
        return 0;
    }
    thread->image = 0;
    return 1;
}

int sbThreadEnqueue(struct SbImageHeader *key, int entryPc, SbMessage *message, SbThread *sender)
{
    SbThread *thread;
    SbMessage *queuedMessage;
    int messageIndex;

    thread = sbFindThread(key, entryPc);
    if (thread == 0) {
        return 0;
    }
    messageIndex = sbQueuePush(thread);
    if (messageIndex == -1) {
        return 0;
    }
    queuedMessage = sbQueueEntryGet(thread, messageIndex);
    *queuedMessage = *message;
    if (sender != 0) {
        queuedMessage->sender = sender->entryPc;
    } else {
        queuedMessage->sender = -1;
    }
    return 1;
}

/* Dequeue side of sbThreadEnqueue: pops the next cursor index of the thread and takes its chain entry. */
SbMessage *sbThreadDequeue(struct SbImageHeader *key, int entryPc)
{
    SbThread *thread;
    int messageIndex;

    thread = sbFindThread(key, entryPc);
    if (thread == 0) {
        return 0;
    }
    if (sbQueueCountGet(thread) == 0) {
        return 0;
    }
    messageIndex = sbQueuePop(thread);
    return sbQueueEntryGet(thread, messageIndex);
}

int sbNameTableFind(struct SbImageHeader *image, const char *name)
{
    SbNameEntry *nameEntries;
    char *nameBase;
    int nameCount;
    int nameIndex;

    nameIndex = 0;
    nameCount = sbNameTable(image)[1];
    nameBase = (char *)sbNameTable(image);
    nameBase = nameBase + sbNameTable(image)[0];
    nameEntries = (SbNameEntry *)nameBase;
    for (; nameIndex < nameCount; nameIndex++) {
        if (strcmp(name, nameBase + nameEntries[nameIndex].name_offset) == 0) {
            return nameEntries[nameIndex].entryPc;
        }
    }
    return -1;
}

extern int D_00616CC8[];

char *sbNameTableNameOf(struct SbImageHeader *image, int entryPc)
{
    SbNameEntry *nameEntries;
    char *nameBase;
    int nameCount;
    int nameIndex;

    nameCount = sbNameTable(image)[1];
    nameBase = (char *)sbNameTable(image);
    nameBase = nameBase + sbNameTable(image)[0];
    nameEntries = (SbNameEntry *)nameBase;
    for (nameIndex = 0; nameIndex < nameCount; nameIndex++) {
        if (entryPc == nameEntries[nameIndex].entryPc) {
            return nameBase + nameEntries[nameIndex].name_offset;
        }
    }
    return 0;
}

/* Enqueues a thread at the entry the image's name table gives for name; 0 when
 * the name is not there. */
int sbThreadEnqueueByName(struct SbImageHeader *image, const char *name, SbMessage *message)
{
    int entryPc = sbNameTableFind(image, name);

    if (entryPc == -1) {
        return 0;
    }
    return sbThreadEnqueue(image, entryPc, message, 0);
}

void sbThreadsRelease(struct SbImageHeader *key)
{
    int threadIndex;

    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        if (D_0060E140[threadIndex].image == key) {
            sbThreadRelease(key, D_0060E140[threadIndex].entryPc);
        }
    }
}

int sbThreadSuspend(struct SbImageHeader *key, int entryPc)
{
    SbThread *thread = sbFindThread(key, entryPc);

    if (thread == 0) {
        return 0;
    }
    thread->status |= SB_IMAGE_LINKED;
    return 1;
}

void sbThreadsSuspend(struct SbImageHeader *key)
{
    int threadIndex;

    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        if (D_0060E140[threadIndex].image == key) {
            sbThreadSuspend(key, D_0060E140[threadIndex].entryPc);
        }
    }
}

/* Clears the suspended bit of the thread with this key and value; 0 when there is none. */
int sbThreadResume(struct SbImageHeader *key, int entryPc)
{
    SbThread *thread = sbFindThread(key, entryPc);

    if (thread == 0) {
        return 0;
    }
    thread->status &= ~SB_IMAGE_LINKED;
    return 1;
}

void sbThreadsResume(struct SbImageHeader *key)
{
    int threadIndex;

    for (threadIndex = 0; threadIndex < SB_THREAD_COUNT; threadIndex++) {
        if (D_0060E140[threadIndex].image == key) {
            sbThreadResume(key, D_0060E140[threadIndex].entryPc);
        }
    }
}

SbQueue *sbQueueGet(int threadIndex) {
    return &sbChains[threadIndex];
}

void sbQueueReset(SbThread *thread)
{
    SbQueue *queue;

    queue = thread->queue;
    queue->ringLastIndex = SB_QUEUE_LAST;
    queue->ringFirstIndex = 0;
    queue->tail = 0;
    queue->head = 0;
    queue->used = 0;
}

/* Pushes one index onto the thread's cursor ring; -1 when the ring is full. */
int sbQueuePush(SbThread *thread)
{
    SbQueue *queue;
    int nextTail;

    queue = thread->queue;
    if (!(queue->used < (queue->ringLastIndex - queue->ringFirstIndex) + 1)) {
        return -1;
    }
    queue->used = queue->used + 1;
    nextTail = queue->tail + 1;
    queue->tail = nextTail;
    if (queue->ringLastIndex < nextTail) {
        queue->tail = queue->ringFirstIndex;
    }
    return queue->tail;
}

/* Takes the next index from the thread's cursor ring; -1 when it is empty. */
int sbQueuePop(SbThread *thread)
{
    SbQueue *queue;
    int nextHead;

    queue = thread->queue;
    if (queue->used <= 0) {
        return -1;
    }
    nextHead = queue->head + 1;
    queue->used = queue->used - 1;
    queue->head = nextHead;
    if (queue->ringLastIndex < nextHead) {
        queue->head = queue->ringFirstIndex;
    }
    return queue->head;
}

int sbQueueCountGet(SbThread *thread)
{
    return thread->queue->used;
}

SbMessage *sbQueueEntryGet(SbThread *thread, int messageIndex)
{
    return &thread->queue->messages[messageIndex];
}

int *sbStackGet(int threadIndex) {
    return sbStacks[threadIndex];
}

unsigned char * sbTagsGet(int threadIndex) {
    return sbThreadTagBytes + (threadIndex << 7);
}

void sbStackPush(SbThread *thread, int stackValue, int tag);
int sbCodeReadUnsigned(SbThread *thread, int index, int count);

unsigned char sbImageByteAt(SbThread *thread, int codePosition)
{
    unsigned char *codeStream;

    codeStream = (unsigned char *) sbImageCodeTableGet(thread->image);
    codeStream += *sbImageCodeTableGet(thread->image);
    return codeStream[codePosition];
}

/*
 * Reads count code bytes, big-endian, from byte offset of the code stream: the code base is the code table
 * (sbImageCodeTableGet) plus the code offset stored in its first word.
 */
int sbCodeReadUnsigned(SbThread *thread, int codePosition, int readWidth)
{
    unsigned char *codeStream;
    int bigEndianValue;
    int i;

    codeStream = (unsigned char *) sbImageCodeTableGet(thread->image) + *sbImageCodeTableGet(thread->image);
    bigEndianValue = 0;
    if (readWidth > 0) {
        for (i = readWidth; i != 0; i--) {
            bigEndianValue <<= 8;
            bigEndianValue |= codeStream[codePosition++];
        }
    }
    return bigEndianValue;
}

int sbGlobalVariableGet(SbThread *thread, int globalVarId)
{
    int *globals;

    globals = (int *) (sbGlobalSection(thread->image) + *(int *) sbGlobalSection(thread->image));
    return globals[globalVarId];
}

void sbGlobalVariableSet(SbThread *thread, int globalVarId, int globalVarContents)
{
    int *globals;

    globals = (int *) (sbGlobalSection(thread->image) + *(int *) sbGlobalSection(thread->image));
    globals[globalVarId] = globalVarContents;
}

int sbLocalVariableGet(SbThread *thread, int localVarId)
{
    int *locals;

    locals = thread->stack + thread->frame;
    return locals[localVarId];
}

void sbLocalVariableSet(SbThread *thread, int localVarId, int localVarContents)
{
    int *locals;

    locals = thread->stack + thread->frame;
    locals[localVarId] = localVarContents;
}

/* Pushes value on the operand stack with its tag byte: a full stack is fatal. */
void sbStackPush(SbThread *thread, int stackValue, int tag)
{
    if (thread->sp >= thread->stackLimit) {
        sbFatal(thread);
    }
    thread->tags[thread->sp] = tag;
    thread->stack[thread->sp++] = stackValue;
}

int sbStackPop(SbThread *thread)
{
    int top;

    if (thread->sp <= 0) {
        sbFatal(thread);
    }
    top = thread->sp - 1;
    thread->sp = top;
    return thread->stack[top];
}

extern signed char sbOpLengthTable[];

/* Length of the instruction with this opcode: opcode 98 takes 8 bytes per operand word
 * plus 6; the others use the signed table entry at sbOpLengthTable. */
int sbOpLength(unsigned char opcode, int operandCount)
{
    if (opcode == 98) {
        return operandCount * 8 + 6;
    }
    return sbOpLengthTable[opcode];
}

/* Opcode table: 99 handlers, then a null word. */
extern int (*sbOpHandlers[])(SbThread *, int);
unsigned char sbImageByteAt(SbThread *thread, int index);

/* One step of the VM: fetch the opcode at pc and run its handler. */
int sbRunOpcode(SbThread *thread)
{
    unsigned int opcode;
    int startPc;
    int opcodeOutcome;

    opcode = sbImageByteAt(thread, thread->pc);
    if (opcode < SB_OPCODE_COUNT) {
        startPc = thread->pc;
        opcodeOutcome = sbOpHandlers[opcode](thread, opcode);
        if (opcodeOutcome == 1) {
            thread->status = opcodeOutcome;
            thread->pc = startPc;
        } else {
            thread->status = 0;
        }
        if (opcodeOutcome == 2) {
            thread->pc = thread->entryPc;
        }
    } else {
        opcodeOutcome = 2;
        sbFatal(thread);
    }
    return opcodeOutcome;
}

int sbOpSkipInstruction(SbThread *thread, int opcode)
{
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpNegate(SbThread *thread, unsigned char opcode)
{
    int poppedValue;

    poppedValue = sbStackPop(thread);
    sbStackPush(thread, -poppedValue, 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Binary operator opcode (operators 2 to 18): pops the right and the left operand, applies the
 * operator of the opcode and pushes the result. A division or remainder by zero requests the
 * zero-divide jump and pushes 0; an unknown operator is fatal. */
int sbOpBinary(SbThread *thread, int opcode)
{
    int binaryOp;
    int right;
    int left;
    int binaryResult;

    right = sbStackPop(thread);
    left = sbStackPop(thread);
    binaryOp = opcode & 0xff;
    switch (binaryOp) {
    case 2:
        binaryResult = right + left;
        break;
    case 3:
        binaryResult = left - right;
        break;
    case 4:
        binaryResult = left * right;
        break;
    case 5:
        if (right == 0) {
            binaryResult = 0;
            sbSlotJumpRequest(thread, SB_SLOT_KEY_ZERO_DIVIDE);
            break;
        }
        binaryResult = left / right;
        break;
    case 6:
        if (right == 0) {
            binaryResult = 0;
            sbSlotJumpRequest(thread, SB_SLOT_KEY_ZERO_DIVIDE);
            break;
        }
        binaryResult = left % right;
        break;
    case 7:
        binaryResult = left | right;
        break;
    case 8:
        binaryResult = left & right;
        break;
    case 9:
        binaryResult = left >> right;
        break;
    case 10:
        binaryResult = left << right;
        break;
    case 11:
        binaryResult = left == right;
        break;
    case 12:
        binaryResult = right < left;
        break;
    case 13:
        binaryResult = left < right;
        break;
    case 14:
        binaryResult = left >= right;
        break;
    case 15:
        binaryResult = left <= right;
        break;
    case 16:
        binaryResult = left != right;
        break;
    case 17:
        binaryResult = left != 0 || right != 0;
        break;
    case 18:
        binaryResult = left != 0 && right != 0;
        break;
    default:
        sbFatal(thread);
        binaryResult = 0;
        break;
    }
    sbStackPush(thread, binaryResult, 1);
    thread->pc += sbOpLength(binaryOp, 0);
    return 0;
}

int sbOpLength(unsigned char opcode, int operandCount);
int sbCodeReadUnsigned(SbThread *thread, int codePosition, int readWidth);

/*
 * Pushes the immediate of opcodes 19 to 31: a 4-byte word (19), a sign-extended byte (20), a sign-extended
 * half word (21), -1 (22) or the small constant opcode - 23 (23 to 31). Then advances pc. Other opcodes are fatal.
 */
int sbOpPushSmallConstant(SbThread *thread, int opcode)
{
    unsigned char opcodeByte;
    int operandValue;

    opcodeByte = opcode & 0xff;
    switch (opcodeByte) {
    case 19:
        operandValue = sbCodeReadUnsigned(thread, thread->pc + 1, 4);
        break;
    case 20:
        operandValue = sbCodeReadUnsigned(thread, thread->pc + 1, 1);
        if (operandValue & 0x80) {
            operandValue |= 0xffffff00;
        }
        break;
    case 21:
        operandValue = sbCodeReadUnsigned(thread, thread->pc + 1, 2);
        if (operandValue & 0x8000) {
            operandValue |= 0xffff0000;
        }
        break;
    case 22:
        operandValue = -1;
        break;
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        operandValue = opcodeByte - 23;
        break;
    default:
        operandValue = 0;
        sbFatal(thread);
        break;
    }
    sbStackPush(thread, operandValue, 1);
    thread->pc += sbOpLength(opcodeByte, 0);
    return 0;
}

/* Pushes a word operand: a count of one byte, or two for opcode 0x20; then advances pc past the operation. */
int sbOpPushImmediate(SbThread *thread, unsigned char opcode)
{
    int operandValue;

    operandValue = sbCodeReadUnsigned(thread, thread->pc + 1, (opcode == 0x20) ? 2 : 1);
    sbStackPush(thread, operandValue, 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpPushNameEntryPc(SbThread *thread, unsigned char opcode)
{
    int nameIndex;
    char *nameBase;
    int *nameTable;
    SbNameEntry *nameEntries;

    nameIndex = sbCodeReadUnsigned(thread, thread->pc + 1, opcode == 0x22 ? 2 : 1);
    nameBase = (char *) sbNameTable(thread->image);
    nameTable = sbNameTable(thread->image);
    nameBase = nameBase + *nameTable;
    nameEntries = (SbNameEntry *) nameBase;
    sbStackPush(thread, nameEntries[nameIndex].entryPc, 4);
    thread->pc = thread->pc + sbOpLength(opcode, 0);
    return 0;
}

int sbOpPushNull(SbThread *thread, unsigned char opcode)
{
    sbStackPush(thread, 0, 0);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pushes a word operand of the 0x25 family: a count of one byte, or two for opcode 0x25; then advances pc. */
int sbOpPushTaggedWord(SbThread *thread, unsigned char opcode)
{
    int operandValue;

    operandValue = sbCodeReadUnsigned(thread, thread->pc + 1, (opcode == 0x25) ? 2 : 1);
    sbStackPush(thread, operandValue, 2);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pushes a word operand of the 0x27 family: a count of one byte, or two for opcode 0x27; then advances pc. */
int sbOpPushGlobalId(SbThread *thread, unsigned char opcode)
{
    int operandValue;

    operandValue = sbCodeReadUnsigned(thread, thread->pc + 1, (opcode == 0x27) ? 2 : 1);
    sbStackPush(thread, operandValue, 3);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbGlobalVariableGet(SbThread *thread, int globalVarId);

/*
 * Pushes a global variable: opcodes 41 to 45 read a 2-byte id, 46 to 50 a 1-byte id; the tag is 1 to 5 by
 * opcode. Then advances pc. Other opcodes are fatal.
 */
int sbOpPushGlobalVariable(SbThread *thread, unsigned char opcode)
{
    unsigned char opcodeByte;
    int operandSize;
    int tag;
    int globalVarId;
    int globalValue;

    opcodeByte = opcode & 0xff;
    switch (opcodeByte) {
    case 41:
        operandSize = 2;
        tag = 1;
        break;
    case 42:
        operandSize = 2;
        tag = 2;
        break;
    case 43:
        operandSize = 2;
        tag = 3;
        break;
    case 44:
        operandSize = 2;
        tag = 4;
        break;
    case 45:
        operandSize = 2;
        tag = 5;
        break;
    case 46:
        operandSize = 1;
        tag = 1;
        break;
    case 47:
        operandSize = 1;
        tag = 2;
        break;
    case 48:
        operandSize = 1;
        tag = 3;
        break;
    case 49:
        operandSize = 1;
        tag = 4;
        break;
    case 50:
        operandSize = 1;
        tag = 5;
        break;
    default:
        operandSize = 0;
        tag = 0;
        sbFatal(thread);
        break;
    }
    globalVarId = sbCodeReadUnsigned(thread, thread->pc + 1, operandSize);
    globalValue = sbGlobalVariableGet(thread, globalVarId);
    sbStackPush(thread, globalValue, tag);
    thread->pc += sbOpLength(opcodeByte, 0);
    return 0;
}

/* Load opcode (51 to 60): reads an operand of 2 bytes (opcodes 51 to 55) or 1 byte (56 to 60),
 * resolves it through sbLocalVariableGet and pushes it with the tag 1 to 5 of the opcode; an unknown
 * opcode is fatal. Then advances pc past the operands. */
int sbOpPushLocalVariable(SbThread *thread, int opcode)
{
    unsigned char opcodeByte;
    int operandSize;
    int tag;
    int localVarId;

    opcodeByte = opcode & 0xff;
    switch (opcodeByte) {
    case 51:
        operandSize = 2;
        tag = 1;
        break;
    case 52:
        operandSize = 2;
        tag = 2;
        break;
    case 53:
        operandSize = 2;
        tag = 3;
        break;
    case 54:
        operandSize = 2;
        tag = 4;
        break;
    case 55:
        operandSize = 2;
        tag = 5;
        break;
    case 56:
        operandSize = 1;
        tag = 1;
        break;
    case 57:
        operandSize = 1;
        tag = 2;
        break;
    case 58:
        operandSize = 1;
        tag = 3;
        break;
    case 59:
        operandSize = 1;
        tag = 4;
        break;
    case 60:
        operandSize = 1;
        tag = 5;
        break;
    default:
        operandSize = 0;
        tag = 0;
        sbFatal(thread);
        break;
    }
    localVarId = sbCodeReadUnsigned(thread, thread->pc + 1, operandSize);
    localVarId = sbLocalVariableGet(thread, localVarId);
    sbStackPush(thread, localVarId, tag);
    thread->pc += sbOpLength(opcodeByte, 0);
    return 0;
}

/* Pops the index and then the value and stores the value through the global index; then advances pc past the operation. */
int sbOpAssignGlobal(SbThread *thread, unsigned char opcode)
{
    int globalVarId;
    int storedValue;

    globalVarId = sbStackPop(thread);
    storedValue = sbStackPop(thread);
    sbGlobalVariableSet(thread, globalVarId, storedValue);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pops the index and then the value and stores the value into the local at index; then advances pc past the operation. */
int sbOpAssignLocal(SbThread *thread, unsigned char opcode)
{
    int localVarId;
    int storedValue;

    localVarId = sbStackPop(thread);
    storedValue = sbStackPop(thread);
    sbLocalVariableSet(thread, localVarId, storedValue);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pops the index and then the value, stores the value through the global index and keeps the stack depth one word higher; then advances pc. */
int sbOpStoreGlobalKeep(SbThread *thread, unsigned char opcode)
{
    int globalVarId;
    int storedValue;

    globalVarId = sbStackPop(thread);
    storedValue = sbStackPop(thread);
    sbGlobalVariableSet(thread, globalVarId, storedValue);
    thread->sp++;
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pops the index and then the value, stores the value into the local at index and keeps the stack depth one word higher; then advances pc. */
int sbOpStoreLocalKeep(SbThread *thread, unsigned char opcode)
{
    int localVarId;
    int storedValue;

    localVarId = sbStackPop(thread);
    storedValue = sbStackPop(thread);
    sbLocalVariableSet(thread, localVarId, storedValue);
    thread->sp++;
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpJump(SbThread *thread)
{
    int branchOffset;

    branchOffset = sbCodeReadUnsigned(thread, thread->pc + 1, 2);
    if (branchOffset & 0x8000) {
        branchOffset |= 0xffff0000;
    }
    thread->pc += branchOffset;
    return 0;
}

/* Branch on a popped word: a nonzero word falls through past the operation; zero jumps by the signed 16-bit displacement after it. */
int sbOpJumpIfNotZero(SbThread *thread, unsigned char opcode)
{
    int branchOffset;

    if (sbStackPop(thread) == 0) {
        branchOffset = sbCodeReadUnsigned(thread, thread->pc + 1, 2);
        if (branchOffset & 0x8000) {
            branchOffset |= 0xFFFF0000;
        }
        thread->pc += branchOffset;
    } else {
        thread->pc += sbOpLength(opcode, 0);
    }
    return 0;
}

/* Branch on a popped word: a nonzero word jumps by the signed 16-bit displacement after the operation, zero falls through past it. */
int sbOpJumpIfZero(SbThread *thread, unsigned char opcode)
{
    int branchOffset;

    if (sbStackPop(thread) != 0) {
        branchOffset = sbCodeReadUnsigned(thread, thread->pc + 1, 2);
        if (branchOffset & 0x8000) {
            branchOffset |= 0xFFFF0000;
        }
        thread->pc += branchOffset;
    } else {
        thread->pc += sbOpLength(opcode, 0);
    }
    return 0;
}

/* Opens a call frame: pushes 0, the frame index and 0 with tag 1, saves the stack depth in the local -1; then advances pc. */
int sbOpEnterFrame(SbThread *thread, unsigned char opcode)
{
    sbStackPush(thread, 0, 1);
    sbStackPush(thread, thread->frame, 1);
    sbStackPush(thread, 0, 1);
    sbLocalVariableSet(thread, -1, thread->sp);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/*
 * Calls a named entry: opcode 0x45 reads a 2-byte name index, the others 1 byte. Takes the frame saved at
 * local -1 (sbOpEnterFrame), stores the return pc in local -3, then jumps to the entry's pc.
 */
int sbOpCallNamed(SbThread *thread, int opcode)
{
    int width;
    int savedFrame;
    int returnPc;
    int nameEntryIndex;
    int nameBase;
    SbNameEntry *nameEntry;

    width = ((opcode & 0xff) == 0x45) ? 2 : 1;
    savedFrame = sbLocalVariableGet(thread, -1);
    returnPc = thread->pc;
    thread->frame = savedFrame;
    sbLocalVariableSet(thread, -3, returnPc + width + 1);
    nameEntryIndex = sbCodeReadUnsigned(thread, thread->pc + 1, width);
    nameBase = (int) sbNameTable(thread->image);
    nameBase = nameBase + sbNameTable(thread->image)[0];
    nameEntry = (SbNameEntry *) (nameEntryIndex * 8 + nameBase);
    nameEntryIndex = nameEntry->entryPc;
    thread->pc = nameEntryIndex;
    return 0;
}

int sbOpReturn(SbThread *thread)
{
    if (thread->sp == 0) {
        sbFatal(thread);
    }
    thread->sp = thread->frame;
    sbStackPop(thread);
    thread->frame = sbStackPop(thread);
    thread->pc = sbStackPop(thread);
    return 0;
}

int sbOpReturnValue(SbThread *thread)
{
    int operand;
    int tag;

    tag = (signed char) thread->tags[thread->sp];
    operand = sbStackPop(thread);
    if (thread->sp == 0) {
        sbFatal(thread);
    }
    thread->sp = thread->frame;
    sbStackPop(thread);
    thread->frame = sbStackPop(thread);
    thread->pc = sbStackPop(thread);
    sbStackPush(thread, operand, tag);
    return 0;
}

/*
 * Calls a native: the operand (1 byte, or 2 for 0x49) indexes the native id table of the linked library image
 * (sbLinkedLibraryGet). The id's owner and method select the function, which runs with the status word; its result
 * goes to the thread's result word and the argument words are dropped. A status of 1 returns 1 to the caller.
 */
int sbOpCallNative(SbThread *thread, unsigned char opcode)
{
    int operandSize;
    int nativeIndex;
    unsigned char *linkedImageBase;
    int nativeIdTableOffset;
    int *stackTop;
    int nativeStatus;
    int nativeResult;
    int droppedWordCount;

    operandSize = opcode != 0x49 ? 1 : 2;
    nativeIndex = sbCodeReadUnsigned(thread, thread->pc + 1, operandSize);
    linkedImageBase = sbLinkedLibraryGet(thread->image);
    nativeIdTableOffset = *(int *) sbLinkedLibraryGet(thread->image);
    nativeIndex = ((int *) (linkedImageBase + nativeIdTableOffset))[nativeIndex];
    stackTop = thread->stack + thread->sp;
    if (stackTop[-1] < *(int *) sbNativeOwners[nativeIndex >> 16].methods[nativeIndex & 0xFFFF].unmodeled_08) {
        sbSlotJumpRequest(thread, 2);
    }
    nativeStatus = 0;
    nativeResult = sbNativeOwners[nativeIndex >> 16].methods[nativeIndex & 0xFFFF].func(thread, &nativeStatus);
    if (nativeStatus == 1) {
        return 1;
    }
    droppedWordCount = stackTop[-1] + 1;
    *(int *) thread->unmodeled_0C = nativeResult;
    thread->sp -= droppedWordCount;
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

s32 sbOpPushIndirect(SbThread *thread, s32 opcode) {
    u8 opcodeByte;

    opcodeByte = opcode & 0xFF;
    sbStackPush(thread, *(s32 *) thread->unmodeled_0C, 1);
    thread->pc = thread->pc + sbOpLength(opcodeByte, 0);
    return 0;
}

int sbOpSpawn(SbThread *thread, unsigned char opcode)
{
    int entryPc;

    entryPc = sbStackPop(thread);
    thread->pc += sbOpLength(opcode, 0);
    return (sbThreadStart(thread->image, entryPc) != 0) ? 0 : 2;
}

/* Compares the popped key with the thread's entry, advances pc past the operation, and returns 2 unless the image has the key. */
int sbOpKillThread(SbThread *thread, unsigned char opcode)
{
    int key;
    int matched;

    key = sbStackPop(thread);
    matched = (thread->entryPc == key) ? 2 : 0;
    thread->pc += sbOpLength(opcode, 0);
    if (sbThreadRelease(thread->image, key)) {
        return matched;
    }
    return 2;
}

int sbOpDrop(SbThread *thread, int opcode)
{
    thread->sp = thread->sp - 1;
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/*
 * Pops a global variable id and pushes that global with tag 1 to 5 (opcodes 79 to 83); other opcodes are fatal.
 * Then advances pc.
 */
int sbOpPushGlobalFromStack(SbThread *thread, int opcode)
{
    unsigned char opcodeByte;
    int globalVarId;
    int tag;
    int globalValue;

    globalVarId = sbStackPop(thread);
    opcodeByte = opcode & 0xff;
    switch (opcodeByte) {
    case 79:
        tag = 1;
        break;
    case 80:
        tag = 2;
        break;
    case 81:
        tag = 3;
        break;
    case 82:
        tag = 4;
        break;
    case 83:
        tag = 5;
        break;
    default:
        tag = 0;
        sbFatal(thread);
        break;
    }
    globalValue = sbGlobalVariableGet(thread, globalVarId);
    sbStackPush(thread, globalValue, tag);
    thread->pc += sbOpLength(opcodeByte, 0);
    return 0;
}

/* Loads the local at the popped index and pushes it; the operation is one of the five load opcodes 84 to 88, anything else is fatal. */
int sbOpLoadLocalFromStack(SbThread *thread, unsigned char opcode)
{
    int localVarId;
    int loadedValue;

    localVarId = sbStackPop(thread);
    if ((unsigned int) (opcode - 84) >= 5) {
        sbFatal(thread);
    }
    loadedValue = sbLocalVariableGet(thread, localVarId);
    sbStackPush(thread, loadedValue, 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpRewindToEntry(SbThread *thread, int opcode)
{
    thread->pc += sbOpLength(opcode, 0);
    return 2;
}

int sbOpYield(SbThread *thread, int opcode)
{
    thread->pc += sbOpLength(opcode, 0);
    return 3;
}

/* Waits a number of frames: the first run takes the count from the stack, each later run counts it down, and the operation completes at zero. */
int sbOpWaitFrames(SbThread *thread, unsigned char opcode)
{
    int waitResult;

    if (thread->status == 0) {
        thread->wait[0] = sbStackPop(thread);
        waitResult = 1;
    } else {
        thread->wait[0] = thread->wait[0] - 1;
        if (thread->wait[0] <= 0) {
            thread->pc += sbOpLength(opcode, 0);
            waitResult = 0;
        } else {
            waitResult = 1;
        }
    }
    return waitResult;
}

SbMessage *sbGlobalEntryGet(SbThread *thread, int index);

/* Send opcode: pops a thread value and an entry index and queues that entry as a
 * message from this thread to the thread with the same key and the popped value. */
int sbOpSend(SbThread *thread, int opcode)
{
    unsigned char opcodeByte;
    int targetEntryPc;
    int globalVarId;
    SbMessage message;

    opcodeByte = opcode;
    targetEntryPc = sbStackPop(thread);
    globalVarId = sbStackPop(thread);
    message = *sbGlobalEntryGet(thread, globalVarId);
    sbThreadEnqueue(thread->image, targetEntryPc, &message, thread);
    thread->pc += sbOpLength(opcodeByte, 0);
    return 0;
}

/* Receive opcode: pops a thread value and an entry index, takes the next message
 * of the thread with the same key and that value into the entry and pushes 1, or
 * pushes 0 when there is none. */
int sbOpReceive(SbThread *thread, int opcode)
{
    unsigned char opcodeByte;
    int sourceEntryPc;
    int globalVarId;
    SbMessage *globalMessage;
    SbMessage *message;
    int received;

    opcodeByte = opcode;
    sourceEntryPc = sbStackPop(thread);
    globalVarId = sbStackPop(thread);
    globalMessage = sbGlobalEntryGet(thread, globalVarId);
    message = sbThreadDequeue(thread->image, sourceEntryPc);
    if (message != 0) {
        *globalMessage = *message;
        received = 1;
    } else {
        received = 0;
    }
    sbStackPush(thread, received, 1);
    thread->pc += sbOpLength(opcodeByte, 0);
    return 0;
}

/* Adds one to the global at the popped index; then advances pc past the operation. */
int sbOpIncGlobal(SbThread *thread, unsigned char opcode)
{
    int globalVarId;

    globalVarId = sbStackPop(thread);
    sbGlobalVariableSet(thread, globalVarId, sbGlobalVariableGet(thread, globalVarId) + 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Adds one to the local at the popped index; then advances pc past the operation. */
int sbOpIncLocal(SbThread *thread, unsigned char opcode)
{
    int localVarId;

    localVarId = sbStackPop(thread);
    sbLocalVariableSet(thread, localVarId, sbLocalVariableGet(thread, localVarId) + 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Subtracts one from the global at the popped index; then advances pc past the operation. */
int sbOpDecGlobal(SbThread *thread, unsigned char opcode)
{
    int globalVarId;

    globalVarId = sbStackPop(thread);
    sbGlobalVariableSet(thread, globalVarId, sbGlobalVariableGet(thread, globalVarId) - 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Subtracts one from the local at the popped index; then advances pc past the operation. */
int sbOpDecLocal(SbThread *thread, unsigned char opcode)
{
    int localVarId;

    localVarId = sbStackPop(thread);
    sbLocalVariableSet(thread, localVarId, sbLocalVariableGet(thread, localVarId) - 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Switch opcode: pops a key and looks it up by binary search in the sorted (key, target)
 * word pairs after the opcode (pair count byte at +1, default target at +2, pair i at
 * +6 + 8 * i); the pc moves by the matching or the default target. */
int sbOpSwitch(SbThread *thread)
{
    int key;
    int bisectLow;
    int bisectHigh;
    int mid;
    int entryKey;

    key = sbStackPop(thread);
    bisectLow = 0;
    bisectHigh = sbCodeReadUnsigned(thread, thread->pc + 1, 1);
    while (bisectLow < bisectHigh) {
        mid = (bisectLow + bisectHigh) / 2;
        entryKey = sbCodeReadUnsigned(thread, thread->pc + mid * 8 + 6, 4);
        if (entryKey < key) {
            bisectLow = mid + 1;
        } else if (key < entryKey) {
            bisectHigh = mid;
        } else {
            thread->pc += sbCodeReadUnsigned(thread, thread->pc + mid * 8 + 10, 4);
            return 0;
        }
    }
    thread->pc += sbCodeReadUnsigned(thread, thread->pc + 2, 4);
    return 0;
}

int D_00551304 = 0;

signed char sbOpLengthTable[104] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 5, 2, 3, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 3, 2, 3, 2, 1, 3, 2,
    3, 2, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 3,
    3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1,
    3, 3, 3, 1, 3, 2, 1, 1, 3, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0
};

int (*sbOpHandlers[100])(SbThread *, int) = {
    sbOpSkipInstruction, sbOpNegate, sbOpBinary, sbOpBinary,
    sbOpBinary, sbOpBinary, sbOpBinary, sbOpBinary,
    sbOpBinary, sbOpBinary, sbOpBinary, sbOpBinary,
    sbOpBinary, sbOpBinary, sbOpBinary, sbOpBinary,
    sbOpBinary, sbOpBinary, sbOpBinary, sbOpPushSmallConstant,
    sbOpPushSmallConstant, sbOpPushSmallConstant, sbOpPushSmallConstant, sbOpPushSmallConstant,
    sbOpPushSmallConstant, sbOpPushSmallConstant, sbOpPushSmallConstant, sbOpPushSmallConstant,
    sbOpPushSmallConstant, sbOpPushSmallConstant, sbOpPushSmallConstant, sbOpPushSmallConstant,
    sbOpPushImmediate, sbOpPushImmediate, sbOpPushNameEntryPc, sbOpPushNameEntryPc,
    sbOpPushNull, sbOpPushTaggedWord, sbOpPushTaggedWord, sbOpPushGlobalId,
    sbOpPushGlobalId, sbOpPushGlobalVariable, sbOpPushGlobalVariable, sbOpPushGlobalVariable,
    sbOpPushGlobalVariable, sbOpPushGlobalVariable, sbOpPushGlobalVariable, sbOpPushGlobalVariable,
    sbOpPushGlobalVariable, sbOpPushGlobalVariable, sbOpPushGlobalVariable, sbOpPushLocalVariable,
    sbOpPushLocalVariable, sbOpPushLocalVariable, sbOpPushLocalVariable, sbOpPushLocalVariable,
    sbOpPushLocalVariable, sbOpPushLocalVariable, sbOpPushLocalVariable, sbOpPushLocalVariable,
    sbOpPushLocalVariable, sbOpAssignGlobal, sbOpAssignLocal, sbOpStoreGlobalKeep,
    sbOpStoreLocalKeep, sbOpJump, sbOpJumpIfNotZero, sbOpJumpIfZero,
    sbOpEnterFrame, sbOpCallNamed, sbOpCallNamed, sbOpReturn,
    sbOpReturnValue, sbOpCallNative, sbOpCallNative, sbOpPushIndirect,
    sbOpSpawn, sbOpKillThread, sbOpDrop, sbOpPushGlobalFromStack,
    sbOpPushGlobalFromStack, sbOpPushGlobalFromStack, sbOpPushGlobalFromStack, sbOpPushGlobalFromStack,
    sbOpLoadLocalFromStack, sbOpLoadLocalFromStack, sbOpLoadLocalFromStack, sbOpLoadLocalFromStack,
    sbOpLoadLocalFromStack, sbOpRewindToEntry, sbOpYield, sbOpWaitFrames,
    sbOpSend, sbOpReceive, sbOpIncGlobal, sbOpIncLocal,
    sbOpDecGlobal, sbOpDecLocal, sbOpSwitch, 0
};

SbThread *sbThreadRing[32];
SbThread D_0060E140[32];
int sbStacks[32][128];
unsigned char sbThreadTagBytes[4096];
SbSlot sbThreadSlotTables[32][8];
SbQueue sbChains[32];
SbNativeOwner sbNativeOwners[128];
SbNativeMethod sbNativeMethodTable[1024];
