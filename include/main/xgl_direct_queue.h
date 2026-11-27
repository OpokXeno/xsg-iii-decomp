#ifndef INCLUDE_MAIN_XGL_DIRECT_QUEUE_H
#define INCLUDE_MAIN_XGL_DIRECT_QUEUE_H

/* One A+D entry of the direct GS queue: the register value and its address (the queue head reuses the words as
 * the GIF tag). */
typedef union XglDirectQueueEntry {
    struct {
        u64 registerValue;
        u64 registerAddress;
    };
    u32 tagAndHeaderWords[4];
} XglDirectQueueEntry;

#endif /* INCLUDE_MAIN_XGL_DIRECT_QUEUE_H */
