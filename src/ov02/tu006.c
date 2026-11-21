/*
 * OV02 original TU 6: 0x00a83b80..0x00a83ce0 (6 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;

/* Command queue header: the cursor at +4 walks 0x14-byte entries from entryStorage up to the limit at +8. */
typedef struct CommandQueue {
    s32 entryStorage;                   /* +0x00 */
    s32 cursor;                     /* +0x04 */
    s32 limit;                      /* +0x08 */
    s32 entryCapacity;                       /* +0x0C */
    s32 commandSteps;                     /* +0x10 */
    s32 commandWaitFrames;                     /* +0x14 */
} CommandQueue;

void commandQueueReset(CommandQueue *queue);

void commandQueueInit(CommandQueue *queue, s32 entryStorage, s32 entryCapacity) {
    commandQueueReset(queue);
    queue->entryStorage = entryStorage;
    queue->entryCapacity = entryCapacity;
}

void commandQueueReset(CommandQueue *queue)
{
  queue->commandWaitFrames = 0;
  queue->cursor = 0;
  queue->commandSteps = 0;
  queue->limit = -1;
}

/* True once the cursor at +4 has passed the limit at +8. */
s32 commandQueueDone(CommandQueue *queue)
{
  return queue->cursor > queue->limit;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu006", func_00A83BF0);

s32 commandQueueDone(CommandQueue *queue);

s32 commandQueueNext(CommandQueue *queue) {
    if (commandQueueDone(queue) != 0) {
        commandQueueReset(queue);
        return 0;
    }
    return queue->entryStorage + (queue->cursor * 0x14);
}

s32 commandQueueNext(CommandQueue *queue);

s32 commandQueueAdvance(CommandQueue *queue) {
    s32 entryAddress;
    s32 cursor;

    entryAddress = commandQueueNext(queue);
    if (entryAddress != 0) {
        cursor = queue->cursor + 1;
        queue->cursor = cursor;
    }
    return entryAddress;
}
