#include "common.h"
#include "main/tu007.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef long long s64;

/* Game loop runtime block at .data 0x3E46B0 (addressed absolutely; the object is larger than the small-data limit).
   Only the fields used by this TU are named; the rest is padding. */
typedef struct GameLoopRuntime {
    s32 loopFlags;                       /* +0x00 */
    s32 loopFlagsHigh;                      /* +0x04: upper halfword only is touched by the state setups */
    s64 resetCombo;                  /* +0x08: held-button bits compared by padCheckResetCombo */
    u8 unk_10[0x128 - 0x10];
    s32 activeHandle;                /* +0x128 */
    s32 pendingHandle;               /* +0x12C */
    u8 unk_130[0x1B0 - 0x130];
    s32 comboState;                  /* +0x1B0: passed to func_001B7A60 */
    u8 unk_1B4[0x1C0 - 0x1B4];
    s32 waitFrames;                  /* +0x1C0: set to 30 by gameStartWaitFrames */
} GameLoopRuntime;

/* Record reached through stateBlockGet() + 0x310 in sceneRecordUpdate. */
typedef struct SceneStateRecord {
    s32 unk_00;
    s32 sceneState;                       /* +0x04: compared with 4 */
} SceneStateRecord;

/* The block stateBlockGet returns: its SceneStateRecord sits at +0x310. */
typedef struct RuntimeStateBlock {
    u8 unk_000[0x310];
    SceneStateRecord sceneRecord;
} RuntimeStateBlock;

/* Thread block at .data 0x4CABA8. */
typedef struct SbThreadEntry {
    u8 unk_00[8];
    s32 threadSlot;                   /* +0x08 */
} SbThreadEntry;

/* The SbThreadEntry table at +0x150 of a thread table (gSbThreadBlock.threadEntryTable). */
typedef struct SbThreadEntryTable {
    u8 unk_000[0x150];
    SbThreadEntry threadEntries[1];
} SbThreadEntryTable;

typedef struct SbThreadBlock {
    u8 unk_00[0x18];
    s32 threadFlags;                       /* +0x18 */
    u8 unk_1C[0x24 - 0x1C];
    s32 currentThreadSlot;                        /* +0x24 */
    SbThreadEntryTable *threadEntryTable;   /* +0x28: the SbThreadEntry table, 0x150 bytes in */
} SbThreadBlock;

extern unsigned char gGameLoopRuntimeStorage[];
extern unsigned char gSbThreadBlockStorage[];
extern u16 gPadDataPressed[];
extern unsigned char PadData[];
/* Typed views of the unsized objects: the extern stays unsized, so the symbols and their addressing are unchanged. */
#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)
#define gSbThreadBlock (*(SbThreadBlock *) gSbThreadBlockStorage)
#define gPadData (*(PadRecord *) PadData)

s32 Vibration_Stop();
s32 xglPacketSetSendMode();
s32 soundStateReset();
s32 func_001CE7B8();
s32 func_001CF7B0();
s32 subsystemResetClearState();
s32 func_001E8498();
s32 sefResetBattleWinState();
s32 func_00265E68();
s32 movieShutdown();
s32 XenoActorPoolInitDefault();
s32 njPlaneSetHook();
s32 gameClearRuntimeFlag();
s32 sefResetResourceTable();
s32 xglRenderClearFrame();

void gameResetRuntimeState(void) {
    gGameLoopRuntime.loopFlags &= ~0x110;
    sefResetBattleWinState();
    func_001E8498();
    njPlaneSetHook(0);
    gameClearRuntimeFlag();
    func_00265E68();
    movieShutdown();
    func_001CF7B0();
    soundStateReset();
    subsystemResetClearState();
    sefResetResourceTable();
    XenoActorPoolInitDefault();
    Vibration_Stop();
    xglPacketSetSendMode(0);
    xglRenderClearFrame();
    func_001CE7B8(0);
}

s32 func_001B4618();

/* Shuts the task queue down: clears the state twice, then runs func_001B4618. */
void gameStopTasks(void) {
    func_001CE7B8(0);
    func_001CE7B8(0);
    func_001B4618();
}

extern unsigned char func_001BC210[];
s32 sbNativeValueSlotSet();
s32 xglPadSetRepeat();

void gameEnterPadRepeatState(void) {
    xglPadSetRepeat(-1, 0xF00C, 5, 1);
    xglRenderClearFrame();
    sbNativeValueSlotSet(func_001BC210, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC210);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC5A8);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC6A8);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC720);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC808);

extern unsigned char gDebugValueFormat[];
s32 xglFontDebugPrintf();

void debugPrintValue(s16 *printedValue) {
    xglFontDebugPrintf(8, 0x20, gDebugValueFormat, *printedValue);
    if (gPadDataPressed[0] & 0x820) {
        func_001CE7B8(0);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC918);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BC9B0);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BCAD0);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BCCA0);

s32 GameSynopsisCrossfadeWait();
s32 xglRenderLockClear();
s32 func_001B5F28();
s32 func_001B8508();
s32 func_001B9850();
s32 vmStreamPoll();
s32 func_001B9B88();
extern unsigned char func_001BCF08[];
extern unsigned char func_001BE320[];
RuntimeStateBlock *stateBlockGet();
s32 func_001BE828();
s32 stateBlockReset();
s32 func_001BE8E0();
s32 func_001BE990();
s32 func_001CFE68();
void func_00225B90();

void sceneRecordUpdate(void) {
    SceneStateRecord *sceneRecord;
    RuntimeStateBlock *sceneBlock;

    sceneRecord = &stateBlockGet()->sceneRecord;
    if (func_001BE828(0) == 0) {
        return;
    }
    stateBlockReset();
    if (!(gGameLoopRuntime.loopFlags & 2)) {
        func_001BE8E0();
        func_001BE990();
    }
    func_001B9850();
    if (sceneRecord->sceneState != 4) {
        sceneBlock = stateBlockGet();
        func_001B8508(&sceneBlock->sceneRecord);
    }
    func_001B5F28(0, 1, 0);
    func_001CFE68();
    vmStreamPoll(1);
    func_001B9B88(0);
    GameSynopsisCrossfadeWait();
    if (gGameLoopRuntime.loopFlags & 0x04000000) {
        sbNativeValueSlotSet(func_001BE320, 0);
        return;
    }
    func_00225B90();
    if (!(gGameLoopRuntime.loopFlags & 0x10000000)) {
        xglRenderClearFrame();
    }
    xglRenderLockClear();
    sbNativeValueSlotSet(func_001BCF08, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BCF08);

extern unsigned char func_001BD1F8[];
s32 sbNativeOwnerGetValue();
s32 func_001CE990();
s32 func_001D1A50();
s32 slotRecord1CountDiff();
s32 sbStartImage();

void gameStartSbImage(void) {
    s32 imageId;

    if ((gGameLoopRuntime.loopFlags & 0x10000000) || (slotRecord1CountDiff() == 0)) {
        func_001E8498();
        if (sbNativeOwnerGetValue(0) != 0) {
            imageId = func_001CE990(0);
            gGameLoopRuntime.loopFlags |= 0x10;
            func_001D1A50(imageId);
            sbStartImage(imageId);
        }
        sbNativeValueSlotSet(func_001BD1F8, 0);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BD1F8);

s32 func_001B6AB0();
s32 func_001B7A60();
extern unsigned char func_001BD450[];
s32 func_001BE6A8();
s32 func_001CEA70();

void padCheckResetCombo(void) {
    func_001B6AB0();
    if (gGameLoopRuntime.loopFlags & 0x20000000) {
        func_001B7A60(gGameLoopRuntime.comboState, -1, 8, 0x20);
    } else {
        func_001B7A60(gGameLoopRuntime.comboState, -1, 0, 0x20);
    }
    if (gGameLoopRuntime.pendingHandle != 0) {
        func_001CEA70(gGameLoopRuntime.pendingHandle, 1);
    }
    if (gGameLoopRuntime.activeHandle != 0) {
        func_001BE6A8(gGameLoopRuntime.activeHandle, 1, 1);
    }
    if ((gGameLoopRuntime.resetCombo & ((0xFFFFLL << 0x20) | 0xFFFF)) == ((0xDB6LL << 0x20) | 0xDAC)) {
        gPadData.leftY = 0;
        gPadData.leftX = 0;
        gPadData.position[3] = 0;
        gPadData.position[2] = 0;
        gPadData.held = 0;
    }
    sbNativeValueSlotSet(func_001BD450, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BD378);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BD450);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BD730);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BD7B8);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BD830);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BDA58);

s32 func_001B7B98();
void gameActorSetupStep(void);
s32 sceneCallbackReset();
s32 sbThreadsSuspend();
void seVoiceStartDefault(unsigned int handle);

void gameSuspendSoundThreads(void) {
    SbThreadEntry *threadEntry;
    s32 threadFlags;
    s32 threadSlot;
    s32 suspendHandle;

    threadEntry = &gSbThreadBlock.threadEntryTable->threadEntries[0];
    if ((gSbThreadBlock.threadEntryTable != 0) && (threadEntry->threadSlot >= 0)) {
        threadFlags = gSbThreadBlock.threadFlags | 0x20;
        gSbThreadBlock.threadFlags = threadFlags;
        threadSlot = threadEntry->threadSlot;
        gSbThreadBlock.currentThreadSlot = threadSlot;
    }
    suspendHandle = func_001CE990(0);
    sbThreadsSuspend(suspendHandle);
    func_001B7B98(0);
    func_001CF7B0();
    seVoiceStartDefault(0x1F);
    sceneCallbackReset();
    sbNativeValueSlotSet(gameActorSetupStep, 0);
}

s32 func_001BBC68();
s32 gameEnterPlayState(void);
s32 func_001C0570();
s32 actorSoundStop();
s32 func_001E8CB8();
s32 func_001EA6B0();
s32 xenoActorSetAttrBit8();

void gameActorSetupStep(void) {
    s32 actorSoundHandle;

    actorSoundHandle = func_001C0570(0);
    actorSoundStop(actorSoundHandle);
    func_001BBC68();
    xenoActorSetAttrBit8(1);
    func_001EA6B0();
    func_001E8CB8(0);
    Vibration_Stop();
    gGameLoopRuntime.loopFlags &= ~0x100;
    sbNativeValueSlotSet(gameEnterPlayState, 1);
}

s32 soundDriverCheckStatus();
s32 func_0018FAC8();
s32 func_001B60B8();
s32 func_001B9228();
s32 func_001B9BF0();
extern unsigned char func_001BDDD8[];
s32 func_001CF8C8();
s32 func_001D1108();
s32 cameraStepPending();

s32 gameEnterPlayState(void) {
    s32 playStateResult;

    playStateResult = func_001B60B8(3);
    if (playStateResult == 0) {
        soundDriverCheckStatus(0x13);
        cameraStepPending(1);
        func_001D1108();
        cameraStepPending(0);
        if (gGameLoopRuntime.loopFlags & 0x04000000) {
            gGameLoopRuntime.loopFlags = gGameLoopRuntime.loopFlags & (~0x100);
            func_001CE7B8(0);
            return sbNativeValueSlotSet(func_001BE320, 0);
        }
        func_0018FAC8(0);
        func_001B9228();
        func_001B9B88(1);
        func_001B9BF0();
        func_001CF8C8();
        func_001B7A60(gGameLoopRuntime.comboState, -1, 0, 0x20);
        return sbNativeValueSlotSet(func_001BDDD8, 0);
    }
    return playStateResult;
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BDDD8);

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BDEC8);

void gameStartWaitFrames(void) {
    s32 nextRuntimeFlags;

    nextRuntimeFlags = (gGameLoopRuntime.loopFlags | 0x100) & 0xFFFCFFFF;
    gGameLoopRuntime.waitFrames = 0x1E;
    gGameLoopRuntime.loopFlags = nextRuntimeFlags;
    func_001CE7B8(0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BE060);

extern unsigned char func_001BC918[];
s32 func_001E8F78();

void gameResetFlagsAndAdvance(void) {
    s32 nextRuntimeFlags;
    s32 nextRuntimeFlagsHigh;

    nextRuntimeFlags = gGameLoopRuntime.loopFlags & (~0xB10);
    nextRuntimeFlagsHigh = gGameLoopRuntime.loopFlagsHigh & 0xFFFF0000;
    gGameLoopRuntime.loopFlags = nextRuntimeFlags;
    gGameLoopRuntime.loopFlagsHigh = nextRuntimeFlagsHigh;
    func_001E8F78();
    njPlaneSetHook(0);
    func_001CFE68();
    sbNativeValueSlotSet(func_001BC918, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BE1E0);

void gameWaitSceneDone(void);
s32 func_001E8E70();

void gameResetFlagsAndWait(void) {
    s32 nextRuntimeFlags;
    s32 nextRuntimeFlagsHigh;

    nextRuntimeFlags = gGameLoopRuntime.loopFlags & 0xFCFFF4EF;
    nextRuntimeFlagsHigh = gGameLoopRuntime.loopFlagsHigh & 0xFFFF0000;
    gGameLoopRuntime.loopFlags = nextRuntimeFlags;
    gGameLoopRuntime.loopFlagsHigh = nextRuntimeFlagsHigh;
    func_001E8E70();
    njPlaneSetHook(0);
    sbNativeValueSlotSet(gameWaitSceneDone, 1);
}

void gameResetRuntimeState(void);

void gameWaitSceneDone(void) {
    func_001B5F28(3, 1, 0);
    if (slotRecord1CountDiff() == 0) {
        sbNativeValueSlotSet(gameResetRuntimeState, 0);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu066", func_001BE320);

void gameFinishSceneReset(void);

void gameResetFlagsAndFinish(void) {
    s32 nextRuntimeFlags;
    s32 nextRuntimeFlagsHigh;

    nextRuntimeFlags = gGameLoopRuntime.loopFlags & 0xF8FFF4EF;
    nextRuntimeFlagsHigh = gGameLoopRuntime.loopFlagsHigh & 0xFFFF0000;
    gGameLoopRuntime.loopFlags = nextRuntimeFlags;
    gGameLoopRuntime.loopFlagsHigh = nextRuntimeFlagsHigh;
    func_001E8E70();
    njPlaneSetHook(0);
    sbNativeValueSlotSet(gameFinishSceneReset, 1);
}

s32 mapCollisionInit();

void gameFinishSceneReset(void) {
    func_001B5F28(3, 1, 0);
    mapCollisionInit(0);
    sbNativeValueSlotSet(gameResetRuntimeState, 0);
}
