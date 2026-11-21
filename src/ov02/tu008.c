/*
 * OV02 original TU 8: 0x00a83d10..0x00a84170 (8 functions)
 */
#include "common.h"

typedef int s32;
extern s32 gRecordQueueTable;
void *memset(void *, int, unsigned int);

void ov02RecordQueueClear(void) {
    memset(&gRecordQueueTable, 0, 0x38);
    memset(&gRecordQueueTable, 0, 0x38);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu008", func_00A83D50);

INCLUDE_ASM("asm/nonmatchings/ov02/tu008", func_00A83D78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu008", func_00A83F20);

INCLUDE_ASM("asm/nonmatchings/ov02/tu008", func_00A84040);

typedef struct RecordQueue {
    s32 queuedValues[9];
    s32 queuedCount;
    s32 valueCursor;
} RecordQueue;

RecordQueue *func_00A83D50();

s32 ov02RecordQueueCursorValue(s32 recordId) {
    RecordQueue *queue;

    queue = func_00A83D50(recordId);
    if (!(queue->queuedCount > 0)) {
        return 0;
    }
    return queue->queuedValues[queue->valueCursor];
}

s32 ov02RecordPeekNext(void) {
    RecordQueue *queue;

    queue = func_00A83D50();
    if (!(queue->queuedCount > 0)) {
        return 0;
    }
    return queue->queuedValues[queue->valueCursor + 1];
}

void ov02RecordNoOp(void) {

}
