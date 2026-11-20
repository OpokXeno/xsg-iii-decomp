#include "common.h"
#include "shared.h"
#include "main/tu105.h"

/* One entry of an image's name table: the name (offset from the header) and its value. */
typedef struct SbNameEntry {
    int name_offset;
    int value;
} SbNameEntry;

/* Thread records of sbThreadTable, and the per-thread areas sbInit clears. */
#define SB_THREAD_COUNT 32
/* Words of a thread's operand stack (0x200 bytes). */
#define SB_STACK_WORDS 0x80
/* Slots in a thread's slot table (sbSlotFind and sbSlotStore scan 8). */
#define SB_SLOT_COUNT 8
/* Messages in a thread's queue (sbChainReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF
/* The suspended bit of a thread's status word (sbThreadSuspend sets it). */
#define SB_THREAD_SUSPENDED 0x80000000

void sbFatal(SbThread *thread);
int sbThreadEnqueue(struct SbImageHeader *key, int value, SbMessage *message, SbThread *sender);
struct SbThread;
void sbFatalReport(struct SbThread *thread, int pc);
extern SbThread sbThreadTable[SB_THREAD_COUNT];
/* The per-thread areas: operand stack, slot table and message queue of thread index. */
extern int sbStacks[SB_THREAD_COUNT][SB_STACK_WORDS];
extern SbSlot sbSlotTables[SB_THREAD_COUNT][SB_SLOT_COUNT];
extern SbQueue sbChains[SB_THREAD_COUNT];
int sbIsSbImage(struct SbImageHeader *header);
/* The thread's slot with this key, or 0. */
SbSlot *sbSlotFind(SbThread *thread, int key);
int strcmp(const char *a, const char *b);
int *sbStackGet(int index);
SbSlot *sbSlotTableGet(int index);
SbQueue *sbChainGet(int index);
SbThread *sbFindThread(struct SbImageHeader *key, int value);
int sbCursorPush(SbThread *thread);
SbMessage *sbChainEntryGet(SbThread *thread, int index);
/* Takes the next index from the thread's cursor ring; -1 when it is empty. */
int sbCursorPop(SbThread *thread);
int sbChainCountGet(SbThread *thread);
int sbNameTableFind(struct SbImageHeader *image, const char *name);
SbMessage *sbThreadDequeue(struct SbImageHeader *key, int value);
int func_001D1A50(struct SbImageHeader *image, struct SbImageHeader *library);
void *func_001D1310(struct SbImageHeader *image);
/* Creates a thread on the first free record of sbThreadTable for this image and entry: the record * takes the image, the entry and the stack, tag, slot and queue of its index; returns 1, or 0 when all * records are in use. The record is stored in the first zero word of D_0060E0C0, from word 0 (the head * of the free-slot list); when no word is free it is not stored. */ int sbThreadStart(struct SbImageHeader *image, int entry);
int func_001D1388(struct SbImageHeader *image, int index);
void *func_001D1370(struct SbImageHeader *image);
int sbStackPop(SbThread *thread);
void sbGlobalSet(SbThread *thread, int index, int value);
int func_001D2528(SbThread *thread, int index);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1108);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1240);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1250);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1310);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1328);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1340);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1358);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1370);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1388);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D13E0);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1460);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D14D0);

/* sbStartImage: starts the image at its code entry; an image that is not an SB image spins forever. */
void sbStartImage(struct SbImageHeader *image)
{
    if (!sbIsSbImage(image)) {
        for (;;) { }
    }
    sbThreadStart(image, ((int *) func_001D1310(image))[1]);
    do {
        func_001D1388(image, 1);
    } while (0);
}

SbSlot *sbSlotTableGet(int index) {
    return sbSlotTables[index];
}

/* The thread's slot with this key, or 0. */
SbSlot *sbSlotFind(SbThread *thread, int key)
{
    SbSlot *slot = thread->slots;
    int i;

    for (i = 0; i < SB_SLOT_COUNT; i++, slot++) {
        if (slot->key == key) {
            return slot;
        }
    }
    return 0;
}

/* Copies entry into the slot with its key, else into the first free slot;
 * returns 0 when the table is full. */
int sbSlotStore(SbThread *thread, SbSlot *entry)
{
    SbSlot *slot = sbSlotFind(thread, entry->key);
    SbSlot *slots;
    int i;

    if (slot == 0) {
        slots = thread->slots;
        for (i = 0; i < SB_SLOT_COUNT; i++) {
            if (slots[i].key == 0) {
                slot = &slots[i];
                break;
            }
        }
    }
    if (slot != 0) {
        *slot = *entry;
        return 1;
    }
    return 0;
}

/* Frees the slot with this key; returns 0 when there is none. */
int sbSlotRemove(SbThread *thread, int key)
{
    SbSlot *slot = sbSlotFind(thread, key);

    if (slot == 0) {
        return 0;
    }
    slot->key = 0;
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
    SbSlot *slot;
    int position;

    key = thread->jumpKey;
    if (key == 0) {
        return 0;
    }
    slot = sbSlotFind(thread, key);
    if (slot == 0) {
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
    if (position >= slot->low) {
        if (slot->high >= position) {
            thread->pc = slot->target;
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

    if (thread->jumpKey == 1) {
        return;
    }
    if (thread->jumpKey == 2) {
        opcode = func_001D2498(thread, thread->jumpPosition, 1);
        length = sbOpLength(opcode, 0);
        func_001D2498(thread, thread->jumpPosition + 1, length - 1);
        func_001D1370(thread->image);
        do {
            func_001D1370(thread->image);
        } while (0);
    }
}

/* Fatal VM error (stack overflow in sbRun): reports the thread, then stops for good. */
void sbFatal(SbThread *thread)
{
    sbFatalReport(thread, thread->pc);
    for (;;) {
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", sbNativeOwnerAdd);

int sbIsSbImage(struct SbImageHeader *header) {
    if (header == 0) {
        return 0;
    }
    return header->magic == 0x20204253;
}

int sbIsSblImage(struct SbImageHeader *header) {
    if (header == 0) {
        return 0;
    }
    return header->magic == 0x204C4253;
}

/*
 * Links the library image into image (sbLinkImage) and makes it the image's key table. Returns 0 when linking
 * fails; hangs on a library that is not an SBL image.
 */
int sbLibraryLink(struct SbImageHeader *image, struct SbImageHeader *library)
{
    if (func_001D1A50(image, library) == 0) {
        return 0;
    }
    if (!sbIsSblImage(library)) {
        for (;;) {
        }
    }
    image->keys = library;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1A50);

int func_0030E68C(const char *a, const char *b);

/* Native id of a method: owner index in the high half, method index in the low half; -1 when the owner or the method is not registered. */
int sbFindNative(const char *ownerName, const char *methodName)
{
    int owner;
    int method;

    for (owner = 0; owner < D_00551504; owner++) {
        if (func_0030E68C(ownerName, D_00616CC0[owner].name) == 0) {
            for (method = 0; method < D_00616CC0[owner].count; method++) {
                if (func_0030E68C(methodName, D_00616CC4[owner][0][method].name) == 0) {
                    return (owner << 16) + method;
                }
            }
        }
    }
    return -1;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1C80);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1C88);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1CA0);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D1CB8);

extern SbThread D_0060E140[SB_THREAD_COUNT];
extern int D_0060E0C0[SB_THREAD_COUNT];
void *memset(void *dest, int value, unsigned int count);
unsigned char *func_001D2428(int index);
void sbChainReset(SbThread *thread);

/* Creates a thread on the first free record of sbThreadTable for this image and entry: the record
 * takes the image, the entry and the stack, tag, slot and queue of its index; returns 1, or 0 when all
 * records are in use. The record is stored in the first zero word of D_0060E0C0, from word 0 (the head
 * of the free-slot list); when no word is free it is not stored. */
int sbThreadStart(struct SbImageHeader *image, int entry)
{
    SbThread *thread;
    int index;
    int slot;

    index = 0;
    while (index < SB_THREAD_COUNT) {
        if (D_0060E140[index].image == 0) {
            thread = &D_0060E140[index];
            for (slot = 0; slot < SB_THREAD_COUNT; slot++) {
                if (D_0060E0C0[slot] == 0) {
                    D_0060E0C0[slot] = (int) thread;
                    break;
                }
            }
            memset(thread->wait, 0, 16);
            thread->pc = entry;
            thread->frame = 3;
            thread->image = image;
            thread->value = entry;
            thread->sp = 0;
            thread->jumpKey = 0;
            thread->jumpPosition = 0;
            thread->stack = sbStackGet(index);
            thread->tags = func_001D2428(index);
            thread->stackLimit = 128;
            thread->slots = sbSlotTableGet(index);
            thread->queue = sbChainGet(index);
            memset(thread->slots, 0, 128);
            sbChainReset(thread);
            thread->status = 0;
            return 1;
        }
        index++;
    }
    return 0;
}

SbThread *sbFindThread(struct SbImageHeader *key, int value)
{
    int index;

    for (index = 0; index < SB_THREAD_COUNT; index++) {
        if (D_0060E140[index].image == key && D_0060E140[index].value == value) {
            return &D_0060E140[index];
        }
    }
    return 0;
}

int sbThreadRelease(struct SbImageHeader *key, int value)
{
    SbThread *thread = sbFindThread(key, value);

    if (thread == 0) {
        return 0;
    }
    thread->image = 0;
    return 1;
}

/* Pushes one index onto the thread's cursor ring; -1 when the ring is full. */
int sbCursorPush(SbThread *thread);

int sbThreadEnqueue(struct SbImageHeader *key, int value, SbMessage *message, SbThread *sender)
{
    SbThread *thread;
    SbMessage *entry;
    int index;

    thread = sbFindThread(key, value);
    if (thread == 0) {
        return 0;
    }
    index = sbCursorPush(thread);
    if (index == -1) {
        return 0;
    }
    entry = sbChainEntryGet(thread, index);
    *entry = *message;
    if (sender != 0) {
        entry->sender = sender->value;
    } else {
        entry->sender = -1;
    }
    return 1;
}

/* Dequeue side of sbThreadEnqueue: pops the next cursor index of the thread and takes its chain entry. */
SbMessage *sbThreadDequeue(struct SbImageHeader *key, int value)
{
    SbThread *thread;
    int index;

    thread = sbFindThread(key, value);
    if (thread == 0) {
        return 0;
    }
    if (sbChainCountGet(thread) == 0) {
        return 0;
    }
    index = sbCursorPop(thread);
    return sbChainEntryGet(thread, index);
}

extern SbNativeMethod D_006172C0[];
extern int D_00551508;
int *func_001D1358(struct SbImageHeader *image);

int sbNameTableFind(struct SbImageHeader *image, const char *name)
{
    SbNameEntry *entries;
    char *base;
    int count;
    int index;

    index = 0;
    count = func_001D1358(image)[1];
    base = (char *)func_001D1358(image);
    base = base + func_001D1358(image)[0];
    entries = (SbNameEntry *)base;
    for (; index < count; index++) {
        if (func_0030E68C(name, base + entries[index].name_offset) == 0) {
            return entries[index].value;
        }
    }
    return -1;
}

extern int D_00616CC8[];

char *sbNameTableNameOf(struct SbImageHeader *image, int value)
{
    SbNameEntry *entries;
    char *base;
    int count;
    int index;

    count = func_001D1358(image)[1];
    base = (char *)func_001D1358(image);
    base = base + func_001D1358(image)[0];
    entries = (SbNameEntry *)base;
    for (index = 0; index < count; index++) {
        if (value == entries[index].value) {
            return base + entries[index].name_offset;
        }
    }
    return 0;
}

/* Enqueues a thread at the entry the image's name table gives for name; 0 when
 * the name is not there. */
int sbThreadEnqueueByName(struct SbImageHeader *image, const char *name, SbMessage *message)
{
    int entry = sbNameTableFind(image, name);

    if (entry == -1) {
        return 0;
    }
    return sbThreadEnqueue(image, entry, message, 0);
}

void sbThreadsRelease(struct SbImageHeader *key)
{
    int index;

    for (index = 0; index < SB_THREAD_COUNT; index++) {
        if (D_0060E140[index].image == key) {
            sbThreadRelease(key, D_0060E140[index].value);
        }
    }
}

int sbThreadSuspend(struct SbImageHeader *key, int value)
{
    SbThread *thread = sbFindThread(key, value);

    if (thread == 0) {
        return 0;
    }
    thread->status |= SB_THREAD_SUSPENDED;
    return 1;
}

void sbThreadsSuspend(struct SbImageHeader *key)
{
    int index;

    for (index = 0; index < SB_THREAD_COUNT; index++) {
        if (D_0060E140[index].image == key) {
            sbThreadSuspend(key, D_0060E140[index].value);
        }
    }
}

/* Clears the suspended bit of the thread with this key and value; 0 when there is none. */
int sbThreadResume(struct SbImageHeader *key, int value)
{
    SbThread *thread = sbFindThread(key, value);

    if (thread == 0) {
        return 0;
    }
    thread->status &= ~SB_THREAD_SUSPENDED;
    return 1;
}

void sbThreadsResume(struct SbImageHeader *key)
{
    int index;

    for (index = 0; index < SB_THREAD_COUNT; index++) {
        if (D_0060E140[index].image == key) {
            sbThreadResume(key, D_0060E140[index].value);
        }
    }
}

SbQueue *sbChainGet(int index) {
    return &sbChains[index];
}

void sbChainReset(SbThread *thread)
{
    SbQueue *queue;

    queue = thread->queue;
    queue->last = SB_QUEUE_LAST;
    queue->first = 0;
    queue->tail = 0;
    queue->head = 0;
    queue->used = 0;
}

/* Pushes one index onto the thread's cursor ring; -1 when the ring is full. */
int sbCursorPush(SbThread *thread)
{
    SbQueue *cursor;
    int next;

    cursor = thread->queue;
    if (!(cursor->used < (cursor->last - cursor->first) + 1)) {
        return -1;
    }
    cursor->used = cursor->used + 1;
    next = cursor->tail + 1;
    cursor->tail = next;
    if (cursor->last < next) {
        cursor->tail = cursor->first;
    }
    return cursor->tail;
}

/* Takes the next index from the thread's cursor ring; -1 when it is empty. */
int sbCursorPop(SbThread *thread)
{
    SbQueue *cursor;
    int next;

    cursor = thread->queue;
    if (cursor->used <= 0) {
        return -1;
    }
    next = cursor->head + 1;
    cursor->used = cursor->used - 1;
    cursor->head = next;
    if (cursor->last < next) {
        cursor->head = cursor->first;
    }
    return cursor->head;
}

int sbChainCountGet(SbThread *thread)
{
    return thread->queue->used;
}

SbMessage *sbChainEntryGet(SbThread *thread, int index)
{
    return &thread->queue->messages[index];
}

int *sbStackGet(int index) {
    return sbStacks[index];
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D2428);

void sbStackPush(SbThread *thread, int value, int tag);
int func_001D2498(SbThread *thread, int index, int count);
unsigned char *func_001D1328(struct SbImageHeader *image);

unsigned char func_001D2440(SbThread *thread, int index)
{
    unsigned char *code;

    code = func_001D1310(thread->image);
    code += *(int *) func_001D1310(thread->image);
    return code[index];
}

/*
 * Reads count code bytes, big-endian, from byte offset of the code stream: the code base is the code table
 * (func_001D1310) plus the code offset stored in its first word.
 */
int func_001D2498(SbThread *thread, int offset, int count)
{
    unsigned char *code;
    int value;
    int i;

    code = (unsigned char *) func_001D1310(thread->image) + *(int *) func_001D1310(thread->image);
    value = 0;
    if (count > 0) {
        for (i = count; i != 0; i--) {
            value <<= 8;
            value |= code[offset++];
        }
    }
    return value;
}

int func_001D2528(SbThread *thread, int index)
{
    int *globals;

    globals = (int *) (func_001D1328(thread->image) + *(int *) func_001D1328(thread->image));
    return globals[index];
}

void sbGlobalSet(SbThread *thread, int index, int value)
{
    int *globals;

    globals = (int *) (func_001D1328(thread->image) + *(int *) func_001D1328(thread->image));
    globals[index] = value;
}

int sbLocalGet(SbThread *thread, int index)
{
    int *locals;

    locals = thread->stack + thread->frame;
    return locals[index];
}

void sbLocalSet(SbThread *thread, int index, int value)
{
    int *locals;

    locals = thread->stack + thread->frame;
    locals[index] = value;
}

/* Pushes value on the operand stack with its tag byte: a full stack is fatal. */
void sbStackPush(SbThread *thread, int value, int tag)
{
    if (thread->sp >= thread->stackLimit) {
        sbFatal(thread);
    }
    thread->tags[thread->sp] = tag;
    thread->stack[thread->sp++] = value;
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

extern signed char D_00407B08[];

/* Length of the instruction with this opcode: opcode 98 takes 8 bytes per operand word
 * plus 6; the others use the signed table entry at D_00407B08. */
int sbOpLength(unsigned char opcode, int count)
{
    if (opcode == 98) {
        return count * 8 + 6;
    }
    return D_00407B08[opcode];
}

/* Opcode table: 99 handlers, then a null word. */
extern int (*D_00407B70[])(SbThread *, int);
unsigned char func_001D2440(SbThread *thread, int index);

/* One step of the VM: fetch the opcode at pc and run its handler. */
int sbStep(SbThread *thread)
{
    unsigned int opcode;
    int original_total;
    int result;

    opcode = func_001D2440(thread, thread->pc);
    if (opcode < 99) {
        original_total = thread->pc;
        result = D_00407B70[opcode](thread, opcode);
        if (result == 1) {
            thread->status = result;
            thread->pc = original_total;
        } else {
            thread->status = 0;
        }
        if (result == 2) {
            thread->pc = thread->value;
        }
    } else {
        result = 2;
        sbFatal(thread);
    }
    return result;
}

int sbOpNop(SbThread *thread, int opcode)
{
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpLoadGlobal(SbThread *thread, unsigned char opcode)
{
    int slot;

    slot = sbStackPop(thread);
    sbStackPush(thread, -slot, 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Binary operator opcode (operators 2 to 18): pops the right and the left operand, applies the
 * operator of the opcode and pushes the result. A division or remainder by zero requests the
 * zero-divide jump and pushes 0; an unknown operator is fatal. */
int sbOpBinary(SbThread *thread, int opcode)
{
    int op;
    int right;
    int left;
    int value;

    right = sbStackPop(thread);
    left = sbStackPop(thread);
    op = opcode & 0xff;
    switch (op) {
    case 2:
        value = right + left;
        break;
    case 3:
        value = left - right;
        break;
    case 4:
        value = left * right;
        break;
    case 5:
        if (right == 0) {
            value = 0;
            sbSlotJumpRequest(thread, 1);
            break;
        }
        value = left / right;
        break;
    case 6:
        if (right == 0) {
            value = 0;
            sbSlotJumpRequest(thread, 1);
            break;
        }
        value = left % right;
        break;
    case 7:
        value = left | right;
        break;
    case 8:
        value = left & right;
        break;
    case 9:
        value = left >> right;
        break;
    case 10:
        value = left << right;
        break;
    case 11:
        value = left == right;
        break;
    case 12:
        value = right < left;
        break;
    case 13:
        value = left < right;
        break;
    case 14:
        value = left >= right;
        break;
    case 15:
        value = left <= right;
        break;
    case 16:
        value = left != right;
        break;
    case 17:
        value = left != 0 || right != 0;
        break;
    case 18:
        value = left != 0 && right != 0;
        break;
    default:
        sbFatal(thread);
        value = 0;
        break;
    }
    sbStackPush(thread, value, 1);
    thread->pc += sbOpLength(op, 0);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D2A38);

/* Pushes a word operand: a count of one byte, or two for opcode 0x20; then advances pc past the operation. */
int sbOpPushWord(SbThread *thread, unsigned char opcode)
{
    int value;

    value = func_001D2498(thread, thread->pc + 1, (opcode == 0x20) ? 2 : 1);
    sbStackPush(thread, value, 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpPushWord04(SbThread *thread, unsigned char opcode)
{
    int index;
    int base;
    int *table;
    int *entry;

    index = func_001D2498(thread, thread->pc + 1, opcode == 0x22 ? 2 : 1);
    base = (int)func_001D1358(thread->image);
    table = (int *)func_001D1358(thread->image);
    base = base + *table;
    entry = (int *)((index << 3) + base);
    sbStackPush(thread, entry[1], 4);
    thread->pc = thread->pc + sbOpLength(opcode, 0);
    return 0;
}

int sbOpPushWord06(SbThread *thread, unsigned char opcode)
{
    sbStackPush(thread, 0, 0);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pushes a word operand of the 0x25 family: a count of one byte, or two for opcode 0x25; then advances pc. */
int sbOpPushWord07(SbThread *thread, unsigned char opcode)
{
    int value;

    value = func_001D2498(thread, thread->pc + 1, (opcode == 0x25) ? 2 : 1);
    sbStackPush(thread, value, 2);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pushes a word operand of the 0x27 family: a count of one byte, or two for opcode 0x27; then advances pc. */
int sbOpPushGlobal(SbThread *thread, unsigned char opcode)
{
    int value;

    value = func_001D2498(thread, thread->pc + 1, (opcode == 0x27) ? 2 : 1);
    sbStackPush(thread, value, 3);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D2DC8);

/* Load opcode (51 to 60): reads an operand of 2 bytes (opcodes 51 to 55) or 1 byte (56 to 60),
 * resolves it through sbLocalGet and pushes it with the tag 1 to 5 of the opcode; an unknown
 * opcode is fatal. Then advances pc past the operands. */
int sbOpPushLocal(SbThread *thread, int opcode)
{
    unsigned char op;
    int size;
    int tag;
    int value;

    op = opcode & 0xff;
    switch (op) {
    case 51:
        size = 2;
        tag = 1;
        break;
    case 52:
        size = 2;
        tag = 2;
        break;
    case 53:
        size = 2;
        tag = 3;
        break;
    case 54:
        size = 2;
        tag = 4;
        break;
    case 55:
        size = 2;
        tag = 5;
        break;
    case 56:
        size = 1;
        tag = 1;
        break;
    case 57:
        size = 1;
        tag = 2;
        break;
    case 58:
        size = 1;
        tag = 3;
        break;
    case 59:
        size = 1;
        tag = 4;
        break;
    case 60:
        size = 1;
        tag = 5;
        break;
    default:
        size = 0;
        tag = 0;
        sbFatal(thread);
        break;
    }
    value = func_001D2498(thread, thread->pc + 1, size);
    value = sbLocalGet(thread, value);
    sbStackPush(thread, value, tag);
    thread->pc += sbOpLength(op, 0);
    return 0;
}

/* Pops the index and then the value and stores the value through the global index; then advances pc past the operation. */
int sbOpStoreGlobal(SbThread *thread, unsigned char opcode)
{
    int first;
    int second;

    first = sbStackPop(thread);
    second = sbStackPop(thread);
    sbGlobalSet(thread, first, second);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pops the index and then the value and stores the value into the local at index; then advances pc past the operation. */
int sbOpStoreLocal(SbThread *thread, unsigned char opcode)
{
    int first;
    int second;

    first = sbStackPop(thread);
    second = sbStackPop(thread);
    sbLocalSet(thread, first, second);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pops the index and then the value, stores the value through the global index and keeps the stack depth one word higher; then advances pc. */
int sbOpStoreGlobalKeep(SbThread *thread, unsigned char opcode)
{
    int first;
    int second;

    first = sbStackPop(thread);
    second = sbStackPop(thread);
    sbGlobalSet(thread, first, second);
    thread->sp++;
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Pops the index and then the value, stores the value into the local at index and keeps the stack depth one word higher; then advances pc. */
int sbOpStoreLocalKeep(SbThread *thread, unsigned char opcode)
{
    int first;
    int second;

    first = sbStackPop(thread);
    second = sbStackPop(thread);
    sbLocalSet(thread, first, second);
    thread->sp++;
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int func_001D3268(SbThread *thread)
{
    int offset;

    offset = func_001D2498(thread, thread->pc + 1, 2);
    if (offset & 0x8000) {
        offset |= 0xffff0000;
    }
    thread->pc += offset;
    return 0;
}

/* Branch on a popped word: a nonzero word falls through past the operation; zero jumps by the signed 16-bit displacement after it. */
int sbOpJumpIfNotZero(SbThread *thread, unsigned char opcode)
{
    int offset;

    if (sbStackPop(thread) == 0) {
        offset = func_001D2498(thread, thread->pc + 1, 2);
        if (offset & 0x8000) {
            offset |= 0xFFFF0000;
        }
        thread->pc += offset;
    } else {
        thread->pc += sbOpLength(opcode, 0);
    }
    return 0;
}

/* Branch on a popped word: a nonzero word jumps by the signed 16-bit displacement after the operation, zero falls through past it. */
int sbOpJumpIfZero(SbThread *thread, unsigned char opcode)
{
    int offset;

    if (sbStackPop(thread) != 0) {
        offset = func_001D2498(thread, thread->pc + 1, 2);
        if (offset & 0x8000) {
            offset |= 0xFFFF0000;
        }
        thread->pc += offset;
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
    sbLocalSet(thread, -1, thread->sp);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D3468);

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
    int opcode;

    opcode = *(signed char *)&thread->tags[thread->sp];
    operand = sbStackPop(thread);
    if (thread->sp == 0) {
        sbFatal(thread);
    }
    thread->sp = thread->frame;
    sbStackPop(thread);
    thread->frame = sbStackPop(thread);
    thread->pc = sbStackPop(thread);
    sbStackPush(thread, operand, opcode);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D3610);

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D3770);

int sbOpSpawn(SbThread *thread, unsigned char opcode)
{
    int slot;

    slot = sbStackPop(thread);
    thread->pc += sbOpLength(opcode, 0);
    return (sbThreadStart(thread->image, slot) != 0) ? 0 : 2;
}

/* Compares the popped key with the thread's entry, advances pc past the operation, and returns 2 unless the image has the key. */
int sbOpKillThread(SbThread *thread, unsigned char opcode)
{
    int key;
    int matched;

    key = sbStackPop(thread);
    matched = (thread->value == key) ? 2 : 0;
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

INCLUDE_ASM("asm/main/nonmatchings/sb_vm", func_001D3908);

/* Loads the local at the popped index and pushes it; the operation is one of the five load opcodes 84 to 88, anything else is fatal. */
int sbOpLoadLocal(SbThread *thread, unsigned char opcode)
{
    int index;
    int value;

    index = sbStackPop(thread);
    if ((unsigned int) (opcode - 84) >= 5) {
        sbFatal(thread);
    }
    value = sbLocalGet(thread, index);
    sbStackPush(thread, value, 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

int sbOpEnd(SbThread *thread, int opcode)
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
    int result;

    if (thread->status == 0) {
        thread->wait[0] = sbStackPop(thread);
        result = 1;
    } else {
        thread->wait[0] = thread->wait[0] - 1;
        if (thread->wait[0] <= 0) {
            thread->pc += sbOpLength(opcode, 0);
            result = 0;
        } else {
            result = 1;
        }
    }
    return result;
}

int func_001D42D8(SbThread *thread, int index);

/* Send opcode: pops a thread value and an entry index and queues that entry as a
 * message from this thread to the thread with the same key and the popped value. */
int sbOpSend(SbThread *thread, int opcode)
{
    unsigned char opcode16;
    int value;
    int index;
    SbMessage message;

    opcode16 = opcode;
    value = sbStackPop(thread);
    index = sbStackPop(thread);
    message = *(SbMessage *)func_001D42D8(thread, index);
    sbThreadEnqueue(thread->image, value, &message, thread);
    thread->pc += sbOpLength(opcode16, 0);
    return 0;
}

/* Receive opcode: pops a thread value and an entry index, takes the next message
 * of the thread with the same key and that value into the entry and pushes 1, or
 * pushes 0 when there is none. */
int sbOpReceive(SbThread *thread, int opcode)
{
    unsigned char opcode16;
    int value;
    int index;
    SbMessage *entry;
    SbMessage *message;
    int received;

    opcode16 = opcode;
    value = sbStackPop(thread);
    index = sbStackPop(thread);
    entry = (SbMessage *)func_001D42D8(thread, index);
    message = sbThreadDequeue(thread->image, value);
    if (message != 0) {
        *entry = *message;
        received = 1;
    } else {
        received = 0;
    }
    sbStackPush(thread, received, 1);
    thread->pc += sbOpLength(opcode16, 0);
    return 0;
}

/* Adds one to the global at the popped index; then advances pc past the operation. */
int sbOpIncGlobal(SbThread *thread, unsigned char opcode)
{
    int index;

    index = sbStackPop(thread);
    sbGlobalSet(thread, index, func_001D2528(thread, index) + 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Adds one to the local at the popped index; then advances pc past the operation. */
int sbOpIncLocal(SbThread *thread, unsigned char opcode)
{
    int index;

    index = sbStackPop(thread);
    sbLocalSet(thread, index, sbLocalGet(thread, index) + 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Subtracts one from the global at the popped index; then advances pc past the operation. */
int sbOpDecGlobal(SbThread *thread, unsigned char opcode)
{
    int index;

    index = sbStackPop(thread);
    sbGlobalSet(thread, index, func_001D2528(thread, index) - 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Subtracts one from the local at the popped index; then advances pc past the operation. */
int sbOpDecLocal(SbThread *thread, unsigned char opcode)
{
    int index;

    index = sbStackPop(thread);
    sbLocalSet(thread, index, sbLocalGet(thread, index) - 1);
    thread->pc += sbOpLength(opcode, 0);
    return 0;
}

/* Switch opcode: pops a key and looks it up by binary search in the sorted (key, target)
 * word pairs after the opcode (pair count byte at +1, default target at +2, pair i at
 * +6 + 8 * i); the pc moves by the matching or the default target. */
int sbOpSwitch(SbThread *thread)
{
    int key;
    int low;
    int high;
    int mid;
    int entryKey;

    key = sbStackPop(thread);
    low = 0;
    high = func_001D2498(thread, thread->pc + 1, 1);
    while (low < high) {
        mid = (low + high) / 2;
        entryKey = func_001D2498(thread, thread->pc + mid * 8 + 6, 4);
        if (entryKey < key) {
            low = mid + 1;
        } else if (key < entryKey) {
            high = mid;
        } else {
            thread->pc += func_001D2498(thread, thread->pc + mid * 8 + 10, 4);
            return 0;
        }
    }
    thread->pc += func_001D2498(thread, thread->pc + 2, 4);
    return 0;
}
