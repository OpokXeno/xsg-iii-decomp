#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu182", func_00225BD8);

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;

typedef struct SteppedValue {
    s16 steppedLevel;
    s16 steppedGoal;
} SteppedValue;

typedef struct SceneRecord {
    u8 pad_00[0x20];
    s32 recordStatusBits;
    u8 pad_24[0x14];
    s32 clearedOnActivate;
} SceneRecord;

#define SCENE_RECORD_ACTIVE 0x8

typedef struct SaveLoadMenu {
    SceneRecord *activeRecord;
    u8 pad_04[0xA04];
    u8 requestedMode;
    u8 swapTargets;
    SteppedValue primaryStep;
    SteppedValue secondaryStep;
    u8 pad_0A12[0x10D6 - 0xA12];
    s8 pendingMode;
    u8 activeMode;
} SaveLoadMenu;

/* gSaveLoadMenuStorage is gSaveLoadMenu (pointer to the 0x10E0-byte menu block, allocated by seEngineAllocate). */
extern SaveLoadMenu *gSaveLoadMenuStorage;
void setSteppedTarget(SteppedValue *stepped, s16 target);

void sceneSetStepTargets(s32 activeStepTarget) {

    if (gSaveLoadMenuStorage->swapTargets != 0) {
        setSteppedTarget(&gSaveLoadMenuStorage->primaryStep, 0);
        setSteppedTarget(&gSaveLoadMenuStorage->secondaryStep, activeStepTarget);
        return;
    }
    setSteppedTarget(&gSaveLoadMenuStorage->primaryStep, activeStepTarget);
    setSteppedTarget(&gSaveLoadMenuStorage->secondaryStep, 0);
}

void sceneSetActiveRecord(SceneRecord *recordToActivate) {
    SceneRecord *previousRecord;
    s32 previousStatusBits;
    s32 activeStatusBits;

    previousRecord = gSaveLoadMenuStorage->activeRecord;
    if (previousRecord != 0) {
        previousStatusBits = previousRecord->recordStatusBits & ~SCENE_RECORD_ACTIVE;
        previousRecord->recordStatusBits = previousStatusBits;
    }
    gSaveLoadMenuStorage->activeRecord = recordToActivate;
    if (recordToActivate != 0) {
        recordToActivate->clearedOnActivate = 0;
        activeStatusBits = recordToActivate->recordStatusBits | SCENE_RECORD_ACTIVE;
        recordToActivate->recordStatusBits = activeStatusBits;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu182", func_00225CC0);

void sceneCommitMode(void) {
    gSaveLoadMenuStorage->pendingMode = 0;
    gSaveLoadMenuStorage->activeMode = gSaveLoadMenuStorage->requestedMode;

}

INCLUDE_ASM("asm/main/nonmatchings/tu182", func_00225D38);

INCLUDE_ASM("asm/main/nonmatchings/tu182", func_00225DB8);

INCLUDE_ASM("asm/main/nonmatchings/tu182", func_00225E50);

void bufferCursorClear(void);
void xcClearPayloadSections(void);
s32 func_00225780(void);

void sceneReset(void) {
    gSaveLoadMenuStorage = 0;
    bufferCursorClear();
    xcClearPayloadSections();
    func_00225780();
}

s32 fontQueueColorState(s32 value);
s32 fontQueueSprite(s32 left, s32 top, s32 right, s32 bottom);
s32 xglFontSetAlphaScale(s32 alpha);
s32 func_0021A7E0(s32 x, s32 y, s32 secondTextId, s32 firstTextId);
void xglFontResetState(void);

void sceneShowMessagePair(s32 firstTextId, s32 secondTextId) {
    xglFontSetAlphaScale(0x80);
    xglFontResetState();
    fontQueueColorState(0x40000000);
    fontQueueSprite(0x6FF8, 0x71F8, 0x8FF8, 0x8DF8);
    func_0021A7E0(0x100, 0xA0, secondTextId, firstTextId);
}

s32 messageTextGet(s32 messageId);

void sceneShowMsgPair139(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0x139);
    secondTextId = messageTextGet(0x5A);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPair1D1(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0x1D1);
    secondTextId = messageTextGet(0x5A);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPairD7(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0xD7);
    secondTextId = messageTextGet(0x5A);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPairE5(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0xE5);
    secondTextId = messageTextGet(0xE4);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPairE7(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0xE7);
    secondTextId = messageTextGet(0xE4);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPairE8(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0xE8);
    secondTextId = messageTextGet(0xE9);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPair166(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0x166);
    secondTextId = messageTextGet(0xE4);
    sceneShowMessagePair(firstTextId, secondTextId);
}

void sceneShowMsgPair167(void) {
    s32 firstTextId;
    s32 secondTextId;

    firstTextId = messageTextGet(0x167);
    secondTextId = messageTextGet(0xE4);
    sceneShowMessagePair(firstTextId, secondTextId);
}
