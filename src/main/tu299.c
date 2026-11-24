#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027DB68);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027DDE8);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027DEF0);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027E0F8);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027E4F0);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027E600);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027E6D8);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027E838);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027ED88);

typedef unsigned short u16;

typedef int s32;

typedef float f32;

typedef short s16;

/* The object a stream record is written for (arg0 of the functions below); offsets measured from the original code. */
typedef struct StreamLoadRequest {
    unsigned char unk_00[0xE];
    u16 readyFlag;                              /* +0x0E: 1 when the entry is ready */
} StreamLoadRequest;

typedef struct StreamLoader {
    unsigned char unk_00[0x24];
    StreamLoadRequest *activeLoad;                     /* +0x24 */
} StreamLoader;

/* Position and collision block of an actor (+0x20 and +0x6A0 are passed to the collision code). */
typedef struct StreamActorState {
    unsigned char unk_00[0x10];
    f32 value10;                            /* +0x10 */
    unsigned char unk_14[0x20 - 0x14];
    unsigned char positionBlock[0x6A0 - 0x20];  /* +0x20: vector used by isOutsideRadius3D */
    unsigned char collision[0x10];          /* +0x6A0: collision object */
} StreamActorState;

typedef struct StreamBounds {
    unsigned char unk_00[0x2C];
    f32 radius;                             /* +0x2C */
    unsigned char unk_30[0x8];
    f32 position[3];                        /* +0x38 */
    unsigned char unk_44[0xC];
    unsigned char subBlock[0x10];           /* +0x50: its address goes to func_0027B590 */
    f32 value60;                            /* +0x60 */
    s32 value64;                            /* +0x64 */
} StreamBounds;

typedef struct StreamSubject {
    unsigned char unk_00[0x50];
    f32 value50;                            /* +0x50 */
    unsigned char unk_54[0x8];
    f32 value5C;                            /* +0x5C */
    f32 value60;                            /* +0x60 */
    s32 recordFormat;                               /* +0x64: 1 and 0x3E8 select the record variants */
    unsigned char unk_68[0x4];
    s16 rangeEnd;                            /* +0x6C */
    unsigned char unk_6E[0x2];
    s16 rangeStart;                            /* +0x70 */
    unsigned char unk_72[0x80 - 0x72];
    StreamBounds *bounds;                     /* +0x80 */
    unsigned char unk_84[0x8];
    StreamLoadRequest *cachedLoadRequest;                     /* +0x8C: cached by streamCacheEntryReady */
    unsigned char unk_90[0xC];
    StreamActorState *targetActor;                    /* +0x9C */
    StreamActorState *actorState;                     /* +0xA0 */
    unsigned char unk_a4[0x4];
    s32 *streamFlags;                             /* +0xA8: flag word, bit 30 and bit 31 are set/cleared by the writers */
    StreamLoader *loader;                   /* +0xAC */
    s32 packetHandle;                             /* +0xB0: packet handle of the record builder */
} StreamSubject;

s32 streamCacheEntryReady(StreamSubject *subject) {
    StreamLoadRequest *activeLoad;

    activeLoad = subject->loader->activeLoad;
    subject->cachedLoadRequest = activeLoad;
    return activeLoad->readyFlag == 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027EE40);

s32 pendingCountGet();

s32 isOutsideRadius3D(void *, void *, f32);

s32 streamNeedsUpdate(StreamSubject *subject) {
    s32 needsUpdate;

    if ((isOutsideRadius3D(subject->targetActor->positionBlock, subject->actorState->positionBlock, subject->bounds->radius) != 0) || (needsUpdate = pendingCountGet(), (needsUpdate != 0))) {
        needsUpdate = 1;
    }
    return needsUpdate;
}

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027EF38);

void streamBeginRecord();

void func_0027B450();

void streamPutWord();

void streamPutFloat(f32);

void func_0027B590(void *);

void streamFlushNoop();

void streamWriteBlockRecord(StreamSubject *subject) {
    s32 *streamFlagWord;
    s32 updatedFlags;

    streamBeginRecord(subject->packetHandle, 0, 0, 2);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 1, 0x1C);
    func_0027B590(0);
    func_0027B590(subject->bounds->subBlock);
    streamPutFloat(subject->value5C);
    streamPutFloat(subject->bounds->value60);
    streamPutWord(subject->bounds->value64);
    streamFlushNoop();
    streamFlagWord = subject->streamFlags;
    updatedFlags = (*streamFlagWord & 0x7FFFFFFF) | 0x40000000;
    *streamFlagWord = updatedFlags;
    if (updatedFlags & 0x02000000) {
        func_0027B450(subject->packetHandle, 0, 5, 0x12);
        streamPutFloat(subject->bounds->position[0]);
        streamPutFloat(subject->bounds->position[1]);
        streamPutFloat(subject->bounds->position[2]);
        streamFlushNoop();
    } else {
        func_0027B450(subject->packetHandle, 0, 5, 0);
        streamFlushNoop();
    }
    func_0027B450(subject->packetHandle, 0, 3, 0xE);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 4, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 2, 0x13);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 6, 0x1D);
    streamFlushNoop();
}

void streamPutHalfword(s16);

void streamWriteStateRecord(StreamSubject *subject) {
    s32 *streamFlagWord;
    s32 updatedFlags;
    s32 packetHandle;

    packetHandle = subject->packetHandle;
    if (subject->recordFormat == 0x3E8) {
        streamBeginRecord(packetHandle, 0, 0, 6);
        func_0027B590(0);
        streamPutFloat(subject->actorState->value10);
        streamFlushNoop();
    } else {
        streamBeginRecord(packetHandle, 0, 0, 5);
        func_0027B590(0);
        streamPutFloat(subject->actorState->value10);
        streamPutHalfword(subject->recordFormat);
        streamFlushNoop();
    }
    func_0027B450(subject->packetHandle, 0, 1, 0x18);
    func_0027B590(0);
    func_0027B590(0);
    streamPutFloat(subject->value5C);
    streamFlushNoop();
    streamFlagWord = subject->streamFlags;
    updatedFlags = (*streamFlagWord & 0x7FFFFFFF) | 0x40000000;
    *streamFlagWord = updatedFlags;
    func_0027B450(subject->packetHandle, 0, 3, 0xE);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 4, 0xC);
    streamPutWord(subject->recordFormat);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 2, 0x13);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 5, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 6, 0x1D);
    streamFlushNoop();
}

void func_002AA490(void *, s32, void *);

void streamWriteRangeRecord(StreamSubject *subject) {
    s32 *streamFlagWord;
    s32 updatedFlags;
    StreamActorState *actorState;

    streamBeginRecord(subject->packetHandle, 0, 0, 1);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 1, 0x16);
    streamFlushNoop();
    streamFlagWord = subject->streamFlags;
    updatedFlags = *streamFlagWord & 0x3FFFFFFF;
    *streamFlagWord = updatedFlags;
    if (updatedFlags & 0x02000000) {
        func_0027B450(subject->packetHandle, 0, 5, 0x12);
        streamPutFloat(subject->bounds->position[0]);
        streamPutFloat(subject->bounds->position[1]);
        streamPutFloat(subject->bounds->position[2]);
        streamFlushNoop();
    } else {
        func_0027B450(subject->packetHandle, 0, 5, 0);
        streamFlushNoop();
    }
    func_0027B450(subject->packetHandle, 0, 3, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 4, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 2, 0x14);
    streamPutHalfword(subject->rangeStart);
    streamPutHalfword(subject->rangeEnd);
    streamPutHalfword(0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 6, 0x1D);
    streamFlushNoop();
    actorState = subject->actorState;
    func_002AA490(actorState->positionBlock, 0, actorState->collision);
}

void streamWriteModeRecord(StreamSubject *subject) {
    s32 *streamFlagWord;
    s32 updatedFlags;
    s32 packetHandle;
    StreamActorState *actorState;

    streamBeginRecord(subject->packetHandle, 0, 0, 1);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 1, 0x16);
    streamFlushNoop();
    streamFlagWord = subject->streamFlags;
    updatedFlags = *streamFlagWord & 0x3FFFFFFF;
    *streamFlagWord = updatedFlags;
    if (updatedFlags & 0x02000000) {
        func_0027B450(subject->packetHandle, 0, 5, 0x12);
        streamPutFloat(subject->bounds->position[0]);
        streamPutFloat(subject->bounds->position[1]);
        streamPutFloat(subject->bounds->position[2]);
        streamFlushNoop();
    } else {
        func_0027B450(subject->packetHandle, 0, 5, 0);
        streamFlushNoop();
    }
    packetHandle = subject->packetHandle;
    if (subject->recordFormat == 1) {
        func_0027B450(packetHandle, 0, 3, 0xF);
        streamPutFloat(subject->value60);
        streamPutWord(0x3E8);
        streamFlushNoop();
    } else {
        func_0027B450(packetHandle, 0, 3, 0x11);
        streamPutFloat(subject->value50);
        streamPutFloat(subject->value60);
        streamFlushNoop();
    }
    func_0027B450(subject->packetHandle, 0, 4, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 2, 0x14);
    streamPutHalfword(subject->rangeStart);
    streamPutHalfword(subject->rangeEnd);
    streamPutHalfword(0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 6, 0x1D);
    streamFlushNoop();
    actorState = subject->actorState;
    func_002AA490(actorState->positionBlock, 0, actorState->collision);
}

void streamWriteSimpleRecord(StreamSubject *subject) {
    s32 *streamFlagWord;
    s32 updatedFlags;

    streamBeginRecord(subject->packetHandle, 0, 0, 1);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 1, 0x17);
    func_0027B590(0);
    func_0027B590(0);
    streamPutFloat(subject->value5C);
    streamFlushNoop();
    streamFlagWord = subject->streamFlags;
    updatedFlags = (*streamFlagWord & 0x7FFFFFFF) | 0x40000000;
    *streamFlagWord = updatedFlags;
    if (updatedFlags & 0x02000000) {
        func_0027B450(subject->packetHandle, 0, 5, 0x12);
        streamPutFloat(subject->bounds->position[0]);
        streamPutFloat(subject->bounds->position[1]);
        streamPutFloat(subject->bounds->position[2]);
        streamFlushNoop();
    } else {
        func_0027B450(subject->packetHandle, 0, 5, 0);
        streamFlushNoop();
    }
    func_0027B450(subject->packetHandle, 0, 3, 0xE);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 4, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 2, 0x15);
    streamPutHalfword(subject->rangeStart);
    streamPutHalfword(subject->rangeEnd);
    streamPutHalfword(0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 6, 0x1D);
    streamFlushNoop();
}

void streamWriteMinimalRecord(StreamSubject *subject) {
    s32 *streamFlagWord;

    streamBeginRecord(subject->packetHandle, 0, 0, 2);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 1, 0x1B);
    func_0027B590(0);
    func_0027B590(subject);
    streamPutFloat(subject->value5C);
    streamFlushNoop();
    streamFlagWord = subject->streamFlags;
    *streamFlagWord &= 0x3FFFFFFF;
    func_0027B450(subject->packetHandle, 0, 5, 0x12);
    streamPutFloat(subject->bounds->position[0]);
    streamPutFloat(subject->bounds->position[1]);
    streamPutFloat(subject->bounds->position[2]);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 3, 0xE);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 4, 0);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 2, 0x13);
    streamFlushNoop();
    func_0027B450(subject->packetHandle, 0, 6, 0x1D);
    streamFlushNoop();
}

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027FAA0);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027FC58);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_0027FE20);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00280060);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00280348);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00280580);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00280790);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00280B28);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_002810F0);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_002813E8);

void streamNoOpA(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_002817E8);

void streamNoOpB(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00281980);

INCLUDE_ASM("asm/main/nonmatchings/tu299", func_00281C20);
