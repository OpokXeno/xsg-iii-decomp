#include "common.h"

extern void *memset(void *destination, int value, unsigned int count);
extern struct CdFileRequest gXglCdReadQueue[3][64];
extern int gXglCdReadQueueTail[3];
extern int gXglCdReadQueueHead[3];
extern int gXglCdReadQueuedCount[3];

void xglCdReadQueueClear(void) {
    memset(gXglCdReadQueue, 0, 22272);
    memset(gXglCdReadQueueTail, 0, 12);
    memset(gXglCdReadQueueHead, 0, 12);
    memset(gXglCdReadQueuedCount, 0, 12);
}

typedef int s32;
struct CdFileRequest {
    s32 channel;
    const char *path;
    void *fileBuffer;
    unsigned char unk_0c[24];
    char name[80];
};

void xglCdFileRequestClear(struct CdFileRequest *fileRequest) {
    memset(fileRequest, 0, 0x74U);
}

s32 xglCdGetAlignedFileSize(void);
s32 xglCdReadQueueAdd(struct CdFileRequest *fileRequest);
s32 func_0030ED88(char *destination, const char *source, s32 count);

s32 xglCdReadRecordPrepare(struct CdFileRequest *fileRequest) {
    extern s32 xglCdGetAlignedFileSize();
    extern s32 xglCdReadQueueAdd();

    func_0030ED88(fileRequest->name, fileRequest->path, 0x50);
    fileRequest->name[79] = 0;
    if (xglCdGetAlignedFileSize(fileRequest->name) < 0) {
        return 0;
    }
    return xglCdReadQueueAdd(fileRequest);
}

s32 xglCdReadRecordPrepare(struct CdFileRequest *fileRequest);
void *func_001B6520(s32);
s32 func_001B6798(void *);

void *xglCdReadRecordAllocate(struct CdFileRequest *fileRequest) {
    extern s32 xglCdReadRecordPrepare();
    extern s32 xglCdGetAlignedFileSize();
    s32 fileSize;
    void *fileBuffer;

    fileSize = xglCdGetAlignedFileSize(fileRequest->path);
    if (fileSize < 0) {
        return 0;
    }
    fileBuffer = func_001B6520(fileSize);
    if (fileBuffer == 0) {
        return 0;
    }
    fileRequest->fileBuffer = fileBuffer;
    if (xglCdReadRecordPrepare(fileRequest) != 0) {
        return fileBuffer;
    }
    func_001B6798(fileBuffer);
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B5F28);

s32 xglCdGetFileSizeForward(void);

s32 xglCdGetAlignedFileSize(void) {
    s32 fileSize;

    fileSize = xglCdGetFileSizeForward();
    if (fileSize >= 0) {
        fileSize = (fileSize + 0x7FF) & ~0x7FF;
    }
    return fileSize;
}

s32 xglCdGetFileSize();

s32 xglCdGetFileSizeForward(void) {
    return xglCdGetFileSize();
}

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6018);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B60B8);

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6118);

s32 xglCdReadQueueService(void);
extern int func_001B60B8(int channel);
s32 func_0030C130();

s32 xglCdReadQueueAdd(struct CdFileRequest *fileRequest) {
    extern s32 xglCdReadQueueService();
    s32 *queuedCountPtr;
    s32 *head_ptr;
    s32 next_head;
    s32 channel;
    s32 nextQueuedCount;
    s32 head;

    if (func_001B60B8(fileRequest->channel) == 0x40) {
        return 0;
    }
    channel = fileRequest->channel;
    head = gXglCdReadQueueTail[channel];
    func_0030C130(fileRequest, &gXglCdReadQueue[channel][head], 0x74);
    head_ptr = &gXglCdReadQueueTail[fileRequest->channel];
    queuedCountPtr = &gXglCdReadQueuedCount[fileRequest->channel];
    next_head = (*head_ptr + 1) & 0x3F;
    nextQueuedCount = *queuedCountPtr + 1;
    *head_ptr = next_head;
    *queuedCountPtr = nextQueuedCount;
    if (func_001B60B8(3) < 2) {
        xglCdReadQueueService();
    }
    return 1;
}

s32 xglCdReadQueueService(void) {
    s32 started;

    started = xglCdReadChannelNext(2);
    if (started == 0) {
        started = xglCdReadChannelNext(0);
        if (started == 0) {
            started = xglCdReadChannelNext(1);
        }
    }
    return started;
}

extern int func_001B60B8(int channel);
extern void func_001B6418(int event, int value, int tag);
extern void xglCdSetReadTag(int value);
extern int xglCdReadFile(const char *name, void *fileBuffer, int mode, void (*notify)(int, int, int));

int xglCdReadChannelNext(int channel)
{
    int *head;
    struct CdFileRequest *fileRequest;

    if (func_001B60B8(channel) == 0) {
        return 0;
    }
    head = &gXglCdReadQueueHead[channel];
    fileRequest = &gXglCdReadQueue[channel][*head];
    xglCdSetReadTag((channel << 16) | *head);
    xglCdReadFile(fileRequest->name, fileRequest->fileBuffer, 1, func_001B6418);
    *head = (*head + 1) & 63;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu054", func_001B6418);
