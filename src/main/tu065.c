#include "common.h"

#include "main/tu007.h"
typedef int s32;

typedef short s16;

typedef struct GameLoopRuntime {
    s32 loopFlags;                 /* +0x00 */
    s32 subFlags;                  /* +0x04: bit 4 is cleared and set by valueSlotAdvanceFromPlayerCounter */
    unsigned short valueSlotId;    /* +0x08 */
    unsigned short valueSlotIndex; /* +0x0A */
    s16 valuePrevId;               /* +0x0C */
    s16 valuePrevIndex;            /* +0x0E */
    s16 slotId;                    /* +0x10: passed to valueSlotChangeRequest by gameLoopStartMenuUpdate */
    s16 slotIndex;                 /* +0x12 */
    unsigned char unk_14[4];       /* +0x14: four bytes cleared by gameRuntimeFullReset (0x80 at +0x17) */
    f32 valueSlotPosition[4];      /* +0x18 */
    unsigned char unk_28[0x124 - 0x28];
    s32 effectHandle; /* +0x124: released by sefEffectHandleRelease */
    unsigned char unk_128[0x15C - 0x128];
    s16 effectMode; /* +0x15C */
    unsigned char unk_15E[0x160 - 0x15E];
    s32 param160; /* +0x160 */
    s32 param164; /* +0x164 */
    s32 param168; /* +0x168 */
    unsigned char unk_16C[0x1B0 - 0x16C];
    s32 unmodeled_1B0; /* +0x1B0: cleared by gameRuntimeFullReset */
    unsigned char unk_1B4[4];
    s32 frameCountdown; /* +0x1B8: gameLoopStartMenuUpdate counts it down while positive */
    s32 stepCount;      /* +0x1BC: gameLoopStartMenuUpdate steps it up while the flag is set */
    unsigned char unk_1C0[0x1F0 - 0x1C0];
    s32 unmodeled_1F0; /* +0x1F0: cleared by gameRuntimeFullReset */
} GameLoopRuntime;

/* Bits of GameLoopRuntime.loopFlags (only the ones this file proves). */
#define LOOP_FLAG_SCENE_BIT8_EFFECT 0x4
#define LOOP_FLAG_EFFECT_SILENT 0x80
#define LOOP_FLAG_SLOT_CHANGE_PENDING 0x200
#define LOOP_FLAG_START_MENU_OPEN 0x4000
#define LOOP_FLAG_EFFECT_MODE_ACTIVE 0x20000
#define LOOP_FLAG_BIT26 0x04000000

extern unsigned char gGameLoopRuntimeStorage[];

#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)

extern PadRecord PadData[];
s32 padStartAndSoftResetHeld(void);
void valueSlotChangeRequest(s16 slotId, s16 slotIndex);
void sefFlagSetBit26(void);
void func_002D3890(s32 flag);
void slotSetAllEnableBits(s32 mode);
void xglClockSampleDayTime(void);
void soundDriverSendOpcode2(void);
void soundDriverSetOutputMode(s32 mode);
void soundSlotSetLevelIfStill(s32 slot, s32 level);
void gameClearRuntimeFlag(void);
void func_00225B90(void);
void soundDriverSetMode(s32 mode);
void slotReleaseFixedSlots(void);
void func_001B7B98(s32 arg);
void slotReleaseSlot1(void);
void seVoiceStartDefault(unsigned int handle);

#define PAD_START 0x0800
#define PAD_SOFT_RESET_HELD 0x010C

typedef struct PadHalves {
    u8 pad[0x28];
    u16 held;
    u16 pressed;
} PadHalves;

void gameLoopStartMenuUpdate(void) {
    u16 held;
    u16 pressed;

    held = ((PadHalves *) PadData)[0].held;
    gGameLoopRuntime.loopFlags &= ~0x8000;
    pressed = ((PadHalves *) PadData)[0].pressed;
    if (gGameLoopRuntime.frameCountdown > 0) {
        gGameLoopRuntime.frameCountdown--;
        return;
    }
    if (!(gGameLoopRuntime.loopFlags & 0x80000000) && !(held & 0x100) && (pressed & PAD_START)) {
        if (!(gGameLoopRuntime.loopFlags & LOOP_FLAG_START_MENU_OPEN)) {
            gGameLoopRuntime.loopFlags |= LOOP_FLAG_START_MENU_OPEN;
            gGameLoopRuntime.stepCount = 0;
            func_002D3890(gGameLoopRuntime.slotId > 0);
            slotSetAllEnableBits(1);
            xglClockSampleDayTime();
            soundDriverSendOpcode2();
            soundDriverSetOutputMode(1);
            soundSlotSetLevelIfStill(1, 1);
            seVoiceStartDefault(0x1A);
            return;
        }
        if (gGameLoopRuntime.stepCount < 16) {
            return;
        }
        gGameLoopRuntime.loopFlags &= ~LOOP_FLAG_START_MENU_OPEN;
        gameClearRuntimeFlag();
        slotSetAllEnableBits(0);
        func_00225B90();
        soundDriverSetMode(16);
        soundDriverSetOutputMode(0);
        soundSlotSetLevelIfStill(1, 0);
        seVoiceStartDefault(0x1A);
        return;
    }
    if (!(gGameLoopRuntime.loopFlags & LOOP_FLAG_START_MENU_OPEN)) {
        return;
    }
    gGameLoopRuntime.stepCount++;
    if (padStartAndSoftResetHeld() != 0) {
        gGameLoopRuntime.loopFlags &= ~LOOP_FLAG_START_MENU_OPEN;
        gameClearRuntimeFlag();
        slotSetAllEnableBits(0);
        slotReleaseFixedSlots();
        func_00225B90();
        func_001B7B98(0);
        soundDriverSetOutputMode(0);
        sefFlagSetBit26();
        return;
    }
    if (!(pressed & 0x10)) {
        return;
    }
    if (gGameLoopRuntime.slotId <= 0) {
        return;
    }
    gGameLoopRuntime.loopFlags &= ~(LOOP_FLAG_START_MENU_OPEN | 0x10);
    gameClearRuntimeFlag();
    slotSetAllEnableBits(0);
    slotReleaseSlot1();
    func_00225B90();
    soundDriverSetMode(32);
    soundDriverSetOutputMode(0);
    seVoiceStartDefault(2);
    valueSlotChangeRequest(gGameLoopRuntime.slotId, gGameLoopRuntime.slotIndex);
}

void func_001B6AB0(void);
void func_001BA1D8(void);
void func_00A83D00(void);
void cycleStepIdle(void);

void gameLoopDebugPadUpdate(void) {
    s32 held;
    s32 pressed;

    pressed = PadData[0].pressed;
    held = PadData[0].held;
    if (held & 0x100) {
        if (pressed & 1) {
            func_001B6AB0();
        }
    }
    if (held & 4) {
        if (pressed & 0x400) {
            gGameLoopRuntime.subFlags ^= 2;
        }
    }
    if (held & 1) {
        if (pressed & 0x200) {
            func_001BA1D8();
        }
        if (pressed & 0x400) {
            func_00A83D00();
        }
    }
    if (gGameLoopRuntime.subFlags & 2) {
        cycleStepIdle();
    }
}

void soundWorkSource(s32, s32, s32);
s32 func_0018F7E8(s32);
s32 func_001B6520(s32);
s32 func_001B6918(s32, s32, s32, s32);
void poolBlockTagWordSet(s32, s32, s32);
s32 soundWorkSourcesInit(void);

s32 soundWorkSourcesInit(void) {
    s32 workSourceIndex;
    s32 workBufferAddress;
    s32 firstSourceSize;

    workSourceIndex = 0;
    do {
        soundWorkSource(workSourceIndex, 0, 0);
        workSourceIndex += 1;
    } while (workSourceIndex < 4);
    workBufferAddress = func_001B6918(0, 1, 0xCF800000, -1);
    if (workBufferAddress == 0) {
        workBufferAddress = func_001B6520(0x100000);
        if (workBufferAddress == 0) {
            return 0;
        }
        poolBlockTagWordSet(workBufferAddress, 1, 0xCF800000);
    }
    firstSourceSize = func_0018F7E8(7);
    soundWorkSource(0, workBufferAddress, firstSourceSize);
    workBufferAddress += firstSourceSize;
    soundWorkSource(1, workBufferAddress, func_0018F7E8(8));
    return 1;
}

typedef signed char s8;

s32 soundSlotsReset();
s32 soundSlotStatus(s32);
void xglSleep(void);

s32 soundWaitAndInitSlots(void) {
    s32 soundSlotIndex;
    s32 workSourceCount;
    s32 statusOrSource; /* kept shared: slot status in the first loop, work source in the second */

    soundSlotIndex = 0;
    soundSlotsReset();
    do {
        while ((statusOrSource = soundSlotStatus(soundSlotIndex)) != 0) {
            xglSleep();
        }
        soundSlotIndex += 1;
    } while (soundSlotIndex < 2);
    workSourceCount = 0;
    statusOrSource = 0;
    do {
        workSourceCount += 1;
        soundWorkSource(statusOrSource, 0, 0);
        statusOrSource = workSourceCount;
    } while (workSourceCount < 4);
    return 1;
}

s32 func_001BE470();

/* The entry trackedPointNearest returns: the id at +0x12 is what func_001BE470 takes. */
typedef struct SefSlotEffectRecord {
    unsigned char unmodeled_00[0x12];
    s16 id;                             /* +0x12 */
} SefSlotEffectRecord;

struct SefSlotEffectRecord *trackedPointNearest();
typedef struct SlotActorCounters {
    unsigned char unmodeled_00[0x6C4];
    s32 valueCount; /* +0x6C4: low byte counts up to 0x100 */
} SlotActorCounters;

/* The slot object RgDispModelFindByKey(kind) finds (stride 0x230, owner rg_dispmodel.c). */
typedef struct SlotObject {
    s32 flags;                 /* +0x00: bit 9 (0x200) makes slotModelStateIsFour return 0 */
    unsigned char unmodeled_04[0x24 - 0x04];
    SlotActorCounters *actor;  /* +0x24 */
    unsigned char unmodeled_28[0x40 - 0x28];
    f32 position[4];           /* +0x40 */
    unsigned char unmodeled_50[0x200 - 0x50];
    s32 handle;                /* +0x200 */
} SlotObject;

SlotObject *RgDispModelFindByKey();

void sefStartCurrentEffect(void) {
    SlotObject *slotObject;
    SefSlotEffectRecord *effectRecord;

    if (!(gGameLoopRuntime.loopFlags & 0x400000)) {
        slotObject = RgDispModelFindByKey(0);
        if (slotObject != 0) {
            effectRecord = trackedPointNearest(slotObject->position);
            if (effectRecord != 0) {
                func_001BE470(effectRecord->id, 1);
            }
        }
    }
}

typedef struct ValueSlotRecord {
    unsigned char unmodeled_00[0x12];
    s16 slotId;    /* +0x12 */
    s16 slotIndex; /* +0x14 */
} ValueSlotRecord;

ValueSlotRecord *func_001BEE18(s32 valueIndex);

void valueSlotAdvanceFromPlayerCounter(void) {
    SlotObject *playerSlot;
    ValueSlotRecord *nextSlotRecord;
    s32 currentSubFlags;

    playerSlot = RgDispModelFindByKey(0);
    if (playerSlot == 0) {
        return;
    }
    if ((playerSlot->actor->valueCount & ~0xFF) != 0x100) {
        gGameLoopRuntime.subFlags &= ~0x10;
        return;
    }
    nextSlotRecord = func_001BEE18((playerSlot->actor->valueCount & 0xFF) + 1);
    if (nextSlotRecord == 0) {
        currentSubFlags = gGameLoopRuntime.subFlags;
        if ((currentSubFlags & 0x10) == 0) {
            gGameLoopRuntime.subFlags = currentSubFlags | 0x10;
        }
    } else {
        gGameLoopRuntime.subFlags &= ~0x10;
        valueSlotChangeRequest(nextSlotRecord->slotId, nextSlotRecord->slotIndex);
        return;
    }
}

typedef struct DispModelState {
    unsigned char unmodeled_00[0x10];
    s32 state; /* +0x10 */
} DispModelState;

DispModelState *RgDispModelFindByActor(s32 handle);

s32 slotModelStateIsFour(void) {
    SlotObject *slot;
    DispModelState *modelState;

    slot = RgDispModelFindByKey(0);
    if (slot == 0) {
        return 0;
    }
    if (slot->flags & 0x200) {
        return 0;
    }
    modelState = RgDispModelFindByActor(slot->handle);
    if (modelState == 0) {
        return 0;
    }
    return modelState->state == 4;
}

s32 padStartAndSoftResetHeld(void) {
    unsigned short pressed;
    unsigned short held;

    pressed = PadData[0].pressed;
    held = PadData[0].held;
    if (pressed & PAD_START) {
        if ((held & PAD_SOFT_RESET_HELD) == PAD_SOFT_RESET_HELD) {
            return 1;
        }
    }
    return 0;
}

s32 sefSpawnEffectFromRequest(void *setup);
void sefEffectHandleRelease(s32);
s32 sceneBit8Get(void);
void sefObjectSetStatePlay(void *setup);

void func_001BBBB8(void) {
    s32 effectSetup[32];

    if (sceneBit8Get() != 0) {
        gGameLoopRuntime.loopFlags |= LOOP_FLAG_SCENE_BIT8_EFFECT;
    } else {
        gGameLoopRuntime.loopFlags &= ~LOOP_FLAG_SCENE_BIT8_EFFECT;
    }
    if (gGameLoopRuntime.loopFlags & LOOP_FLAG_SCENE_BIT8_EFFECT) {
        if (gGameLoopRuntime.effectHandle == 0) {
            sefObjectSetStatePlay(effectSetup);
            effectSetup[11] = 0x10064024;
            gGameLoopRuntime.effectHandle = sefSpawnEffectFromRequest(effectSetup);
        }
    } else if (gGameLoopRuntime.effectHandle != 0) {
        sefEffectHandleRelease(gGameLoopRuntime.effectHandle);
        gGameLoopRuntime.effectHandle = 0;
    }
}

void func_001BBC68(void) {
    s32 effectHandle;

    effectHandle = gGameLoopRuntime.effectHandle;
    gGameLoopRuntime.loopFlags &= ~LOOP_FLAG_SCENE_BIT8_EFFECT;
    if (effectHandle != 0) {
        sefEffectHandleRelease(effectHandle);
        gGameLoopRuntime.effectHandle = 0;
    }
}

s32 sbNativeValueSlotSet();
s32 func_001BE060();
s32 func_001BE1E0();
s32 xglRenderClearFrame();
extern s16 gGameBootMode;
extern s16 gPreviousValueSlotIds[];

void valueSlotChangeRequest(s16 slotId, s16 slotIndex) {
    GameLoopRuntime *loopRuntime;
    s32 loopFlags;
    s32 newFlags;

    loopRuntime = &gGameLoopRuntime;
    loopFlags = loopRuntime->loopFlags;
    if (loopFlags & LOOP_FLAG_SLOT_CHANGE_PENDING) {
        return;
    }
    newFlags = (loopFlags | LOOP_FLAG_SLOT_CHANGE_PENDING) & 0xDFFFFFFF;
    loopRuntime->valuePrevId = loopRuntime->valueSlotId;
    loopRuntime->valuePrevIndex = loopRuntime->valueSlotIndex;
    loopRuntime->valueSlotId = slotId;
    loopRuntime->valueSlotIndex = slotIndex;
    loopRuntime->loopFlags = newFlags;
    if (newFlags & 0x10000000) {
        loopRuntime->loopFlags = newFlags & 0xEFFFFFFF;
        xglRenderClearFrame();
    }
    if (gGameBootMode == 2 && loopRuntime->valuePrevId == gPreviousValueSlotIds[0]) {
        sbNativeValueSlotSet(func_001BE1E0, 0);
        return;
    }
    sbNativeValueSlotSet(func_001BE060, 1);
}

void valueSlotChangeToPosition(s16 slotId, DoublewordVector4 *position) {
    GameLoopRuntime *loopRuntime;
    s32 currentFlags;
    unsigned short previousSlotId;
    unsigned short previousSlotIndex;

    loopRuntime = &gGameLoopRuntime;
    currentFlags = loopRuntime->loopFlags;
    if (currentFlags & LOOP_FLAG_SLOT_CHANGE_PENDING) {
        return;
    }
    previousSlotId = loopRuntime->valueSlotId;
    previousSlotIndex = loopRuntime->valueSlotIndex;
    loopRuntime->valuePrevId = previousSlotId;
    loopRuntime->loopFlags = (currentFlags | 0x800200) & 0xDFFFFFFF;
    loopRuntime->valuePrevIndex = previousSlotIndex;
    loopRuntime->valueSlotId = slotId;
    loopRuntime->valueSlotIndex = 0xFFFF;
    loopRuntime->valueSlotPosition[0] = position->vector.x;
    loopRuntime->valueSlotPosition[1] = position->vector.y;
    loopRuntime->valueSlotPosition[2] = position->vector.z;
    loopRuntime->valueSlotPosition[3] = position->vector.w;
    sbNativeValueSlotSet(func_001BE060, 1);
}

void sefSetEffectParams(s32 effectMode, s32 first, s32 second, s32 third, s32 audibleFlag) {
    gGameLoopRuntime.loopFlags = (gGameLoopRuntime.loopFlags | 0x10000) & 0xFFFDFF7F;
    if (effectMode > 0) {
        gGameLoopRuntime.loopFlags |= LOOP_FLAG_EFFECT_MODE_ACTIVE;
    }
    if (audibleFlag == 0) {
        gGameLoopRuntime.loopFlags |= LOOP_FLAG_EFFECT_SILENT;
    }
    gGameLoopRuntime.effectMode = effectMode;
    gGameLoopRuntime.param160 = first;
    gGameLoopRuntime.param164 = second;
    gGameLoopRuntime.param168 = third;
}

void sefFlagSetBit26(void) {
    gGameLoopRuntime.loopFlags |= LOOP_FLAG_BIT26;
}

s32 func_0018FAC8(s32);
s32 vmStreamPoll(s32);
s32 func_001B9B88(s32);
s32 func_A85A20();
s32 func_AA0C30();
s32 xglCdLoadOverlay(s32);

void sceneOverlayReset(void) {
    xglCdLoadOverlay(4);
    func_AA0C30();
    xglCdLoadOverlay(2);
    xglRenderClearFrame();
    func_0018FAC8(0);
    soundWorkSourcesInit();
    vmStreamPoll(1);
    func_001B9B88(1);
    func_A85A20();
}

void func_001B64D8(s32 a, s32 b);
void xglCdReadQueueClear(void);
void func_001B6BD8(void);
void memoryBanksTest(s32 flag);
void hwStatusReady(void);
void *memset(void *destination, int value, unsigned int count);
void clearSaveTables(void);
void sefResetResourceTable(void);
void stateBlockClear(void);
void sbNativeOwnersReset(void);
void func_001B97F0(void);
void soundStateReset(void);
void subsystemResetClearState(void);
void xglPadSetDefaults(void);
void xglRenderClearDepth(void);
void xglRenderLockClear(void);
void xglPacketSetSendMode(s32 mode);
void Vibration_Stop(void);
void sefResetEffectSystem(void);
void gameOverCopyRecord(void);
void func_A859A0(void);
void func_A9FF28(void);
void func_001B9438(s32 flag);
void bufferPointersRewind(void);
extern unsigned char gameEnterPadRepeatState[];
s32 gameStartSavedPlacement();
s32 func_001BC808();
s32 func_001BC6A8();

static inline s32 clearRenderThenSlotSet(void) {
    xglRenderClearFrame();
    xglRenderLockClear();
    return sbNativeValueSlotSet(gameEnterPadRepeatState, 0);
}

s32 gameRuntimeFullReset(void) {
    GameLoopRuntime *loopRuntime;

    func_001B64D8(0xE00000, 0x1200000);
    xglCdReadQueueClear();
    func_001B6BD8();
    memoryBanksTest(1);
    soundWorkSourcesInit();
    hwStatusReady();
    loopRuntime = &gGameLoopRuntime;
    memset(loopRuntime, 0, 0x200);
    loopRuntime->unk_14[2] = 0;
    loopRuntime->unk_14[1] = 0;
    loopRuntime->unk_14[3] = 0x80;
    loopRuntime->unk_14[0] = 0;
    loopRuntime->unmodeled_1B0 = 0;
    clearSaveTables();
    sefResetResourceTable();
    stateBlockClear();
    sbNativeOwnersReset();
    func_001B97F0();
    soundStateReset();
    subsystemResetClearState();
    xglPadSetDefaults();
    func_0018FAC8(0);
    xglRenderClearDepth();
    xglRenderLockClear();
    xglPacketSetSendMode(0);
    Vibration_Stop();
    sefResetEffectSystem();
    gameOverCopyRecord();
    func_A859A0();
    func_A9FF28();
    func_001B9438(1);
    bufferPointersRewind();
    loopRuntime->unmodeled_1F0 = 0;
    loopRuntime->loopFlags |= 0x100000;
    loopRuntime->subFlags |= 0x10010000;
    func_001B6AB0();
    switch (gGameBootMode) {
    case 0:
    default:
        return sbNativeValueSlotSet(func_001BC6A8, 0);
    case 1:
        return sbNativeValueSlotSet(gameStartSavedPlacement, 0);
    case 2:
        return sbNativeValueSlotSet(func_001BC808, 0);
    case -2:
    case -1:
        return clearRenderThenSlotSet();
    }
}
