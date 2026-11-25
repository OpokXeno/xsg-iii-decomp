#include "common.h"

typedef int s32;

typedef signed char s8;

void sbNativeSlotsClear(void);
s32 sbClearVmTables();
s32 memset(s32 *, s32, s32);

/* [0] the number of the loaded file, [1] the owner of the block. */
static s32 gSbNativeOwnerFlags[2];

void sbNativeOwnersReset(void) {
    memset(gSbNativeOwnerFlags, 0, 8);
    sbNativeSlotsClear();
    sbClearVmTables();
}

extern s32 gSbCfmapNativeTable[];
extern s32 gSbDebugNativeTable[];
extern s32 gSbMathNativeTable[];
extern s32 gSbObjxNativeTable[];
extern s32 gSbRuntimeNativeTable[];
extern s32 gSbSoundNativeTable[];
s32 sbNativeOwnerAdd(s32 *, s32 *);

void sbNativeOwnersInit(void) {
    sbNativeSlotsClear();
    sbClearVmTables();
    sbNativeOwnerAdd("deb", gSbDebugNativeTable);
    sbNativeOwnerAdd("runtime", gSbRuntimeNativeTable);
    sbNativeOwnerAdd("objx", gSbObjxNativeTable);
    sbNativeOwnerAdd("cfmap", gSbCfmapNativeTable);
    sbNativeOwnerAdd("math", gSbMathNativeTable);
    sbNativeOwnerAdd("sound", gSbSoundNativeTable);
}

void sbNativeOwnersAddExtra(void) {
    sbNativeOwnerAdd("deb", gSbDebugNativeTable);
    sbNativeOwnerAdd("sound", gSbSoundNativeTable);
}

s32 sbNativeOwnerGetValue(s32 ownerIndex) {
    return gSbNativeOwnerFlags[ownerIndex];
}

extern unsigned char gSbNativeFileBuffer[];
extern unsigned char gSbNativeBlockBuffer[];

/* Returns the native buffer of the given index: 0 is gSbNativeFileBuffer, 1 is gSbNativeBlockBuffer, others 0. */
unsigned char *sbNativeBufferGet(s32 nativeBufferId) {
    unsigned char *nativeBuffer = gSbNativeFileBuffer;

    switch (nativeBufferId) {
    case 0:
        break;
    case 1:
        return gSbNativeBlockBuffer;
    default:
        nativeBuffer = 0;
        break;
    }
    return nativeBuffer;
}

#include "main/tu105.h"


struct CdFileRequest {
    s32 channel;
    char *path;
    void *fileBuffer;
    unsigned char unk_0C[24];
    char name[80];
};

struct CdFilePath {
    char path[100];
    char fileName[28];
};

void xglCdFileRequestClear(struct CdFileRequest *fileRequest);
s32 xglCdReadRecordPrepare(struct CdFileRequest *fileRequest);
s32 GameMpeg2Play(char *destination, const char *format, ...);
s32 func_001B6FF8();

/* Builds the file name from fileNumber, requests it on the CD channel into buffer 0
 * (gSbNativeFileBuffer) and records the number in gSbNativeOwnerFlags[0]; returns 1 on success. */
s32 sbNativeFileLoad(short fileNumber, s32 channel) {
    struct CdFileRequest nativeFileRequest;
    struct CdFilePath filePath;
    s32 succeeded;

    GameMpeg2Play(filePath.fileName, "%04d", fileNumber);
    func_001B6FF8(filePath.path, 0x6000000, filePath.fileName);
    xglCdFileRequestClear(&nativeFileRequest);
    nativeFileRequest.channel = channel;
    nativeFileRequest.path = filePath.path;
    nativeFileRequest.fileBuffer = gSbNativeFileBuffer;
    succeeded = xglCdReadRecordPrepare(&nativeFileRequest);
    if (succeeded == 0) {
        gSbNativeOwnerFlags[0] = 0;
        return 0;
    }
    gSbNativeOwnerFlags[0] = fileNumber;
    return 1;
}

typedef struct SbNativeLoadRequest {
    s32 sourceId;               /* +0x00 */
    s32 ownerIndex;             /* +0x04 */
    unsigned char *loadDestination;/* +0x08 */
    s32 unk_0C[3];
} SbNativeLoadRequest;

void xcBlockClear(void *clearedBlock);
s32 func_001B70F0();

/* Loads the block of sourceId into buffer 1 (gSbNativeBlockBuffer) and records ownerIndex in
 * gSbNativeOwnerFlags[1]; returns 1 on success. */
s32 sbNativeBlockLoad(s32 ownerIndex, s32 sourceId) {
    SbNativeLoadRequest nativeLoadRequest;
    s32 succeeded;

    xcBlockClear(&nativeLoadRequest);
    nativeLoadRequest.sourceId = sourceId;
    nativeLoadRequest.ownerIndex = ownerIndex;
    nativeLoadRequest.loadDestination = gSbNativeBlockBuffer;
    succeeded = func_001B70F0(&nativeLoadRequest);
    if (succeeded == 0) {
        gSbNativeOwnerFlags[1] = 0;
        return 0;
    }
    gSbNativeOwnerFlags[1] = ownerIndex;
    return 1;
}

void sbNativeSlotsClear(void) {
    extern s32 gSbNativeTableWords[];
    memset(gSbNativeTableWords, 0, 0x1C);
}

s32 sbNativeSlotSet(s32 nativeSlotIndex, s32 nativeSlotWord) {
    extern s32 gSbNativeTableWords[];

    gSbNativeTableWords[nativeSlotIndex] = nativeSlotWord;
    return 1;
}

extern s32 gSbNativeTableWords[];
int sbThreadEnqueue(void *key, int entryPc, SbMessage *message, SbThread *sender);

/* Posts a message of the given kind to the handler stored in gSbNativeTableWords[kind];
 * returns 0 when no handler is set. */
s32 sbNativeHandlerMessagePost(s32 messageKind, s32 messageFirstWord, s32 messageSecondWord) {
    SbMessage message;
    s32 entryPc;

    entryPc = gSbNativeTableWords[messageKind];
    if (entryPc == 0) {
        return 0;
    }
    message.kind = messageKind;
    message.args[0] = messageFirstWord;
    message.args[1] = messageSecondWord;
    sbThreadEnqueue(gSbNativeFileBuffer, entryPc, &message, 0);
    return 1;
}

unsigned char gSbNativeFileBuffer[0x14000];
unsigned char gSbNativeBlockBuffer[0x10000];
s32 gSbNativeTableWords[7];
