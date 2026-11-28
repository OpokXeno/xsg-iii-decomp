#include "common.h"
#include "shared.h"
#include "main/tu105.h"
#include "main/xenoActor.h"

/* Messages in a thread's queue (sbQueueReset: first 0, last 0xF). */
#define SB_QUEUE_LAST 0xF

int sbSlotJumpRequest();

struct SbThread;

int VM_PLUGIN_CNT_GET(struct SbThread *thread);

int vmArgGet(struct SbThread *thread, int vmArgPosition);

int Get_Rnd_001B8CD0(int min, int max);

int mathRandom(SbThread *thread);

extern const char D_005379F8[7][16];
extern const char D_00537D38[16];

extern u16 gMovieAutoSelectFlags[];

int scriptMovieAutoSelectFlagGet(SbThread *thread) {
    int key;
    int autoSelectFlags;

    autoSelectFlags = 0;
    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    key = vmArgGet(thread, 1);
    switch (key) {
    case 0:
        autoSelectFlags = gMovieAutoSelectFlags[0];
        break;
    case 1:
        autoSelectFlags = gMovieAutoSelectFlags[1];
        break;
    case 2:
        autoSelectFlags = gMovieAutoSelectFlags[2];
        break;
    }
    return autoSelectFlags;
}

extern signed char gMovieFlagBytes[];

int scriptMovieFlagByteGet(SbThread *thread) {
    int key;
    int movieFlagValue;

    movieFlagValue = 0;
    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    key = vmArgGet(thread, 1);
    switch (key) {
    case 0:
        movieFlagValue = gMovieFlagBytes[0];
        break;
    case 1:
        movieFlagValue = gMovieFlagBytes[1];
        break;
    case 2:
        movieFlagValue = gMovieFlagBytes[2];
        break;
    case 3:
        movieFlagValue = gMovieFlagBytes[3];
        break;
    }
    return movieFlagValue;
}

typedef signed char s8;

typedef int s32;

/* The part of the game loop runtime these natives use. */
typedef struct GameLoopRuntime {
    u8 unmodeled_000[0x10];
    s16 firstVmArg;        /* +0x010: the two values sbDeferredCallStore stores */
    s16 secondVmArg;       /* +0x012 */
    u8 unmodeled_014[0x1E8 - 0x14];
    float loopScaleFirst;  /* +0x1E8 */
    float loopScaleSecond; /* +0x1EC */
    s32 partHideFlags;     /* +0x1F0 */
} GameLoopRuntime;

extern GameLoopRuntime gGameLoopRuntimeStorage[];

s32 vmArgTagIsZero(SbThread *thread, s32 vmArgPosition);

s32 sbDeferredCallStore(SbThread *thread)
{
  s16 firstVmArg;
  s16 secondVmArg;
  s16 storedSecondVmArg;
    if ((VM_PLUGIN_CNT_GET(thread) <= 0) || (VM_PLUGIN_CNT_GET(thread) >= 3))
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  firstVmArg = vmArgGet(thread, 1);
  if (vmArgTagIsZero(thread, 2) == 0)
  {
    secondVmArg = vmArgGet(thread, 2);
    storedSecondVmArg = secondVmArg;
  }
  else
  {
    storedSecondVmArg = -1;
  }
  gGameLoopRuntimeStorage->firstVmArg = firstVmArg;
  gGameLoopRuntimeStorage->secondVmArg = storedSecondVmArg;
  return 1;
}

s32 valueSlotChangeRequest(s16 first, s16 second);

s32 sbDeferredCallRun(void)
{
  GameLoopRuntime *deferredCall;
  s16 secondVmArg;
  deferredCall = gGameLoopRuntimeStorage;
  secondVmArg = deferredCall->secondVmArg;
  if (((deferredCall->firstVmArg) > 0) == 0U)
  {
    return 0;
  }
  valueSlotChangeRequest(deferredCall->firstVmArg, secondVmArg);
  return 1;
}

/* Script native "math.random": two arguments, min and max, passed to Get_Rnd. */
int mathRandom(SbThread *thread)
{
    int randomMin;
    int randomMax;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    randomMin = vmArgGet(thread, 1);
    randomMax = vmArgGet(thread, 2);
    return Get_Rnd_001B8CD0(randomMin, randomMax);
}

s32 func_001B8D08(s32 groupId, s32 entryId, s32 scoreBias);

s32 indexedTableGet(s32 groupId, s32 entryId);

s32 indexedCountGet(s32 groupId, s32 entryId);

s32 indexedValueGet(s32 entryIndex);

void movieSetFlagByte(s32 value, s32 movieDataValue, s32 scoreBias);

void movieDecoderReset(s32 value, s32 scoreBias);

void seVoiceStartDefault(unsigned int handle);

s32 func_001CD248(SbThread *thread) {
    s32 groupId;
    s32 entryId;
    s32 scoreBias;
    s32 groupStatus;
    s32 playsVoice;
    s32 random;
    s32 movieDataValue;
    s32 entryTally;
    s32 movieEntryValue;

    if ((VM_PLUGIN_CNT_GET(thread) < 3) || (VM_PLUGIN_CNT_GET(thread) >= 5)) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    groupId = vmArgGet(thread, 1);
    entryId = vmArgGet(thread, 2);
    scoreBias = vmArgGet(thread, 3);
    if (vmArgTagIsZero(thread, 4) == 0) {
        playsVoice = vmArgGet(thread, 4) != 0;
    } else {
        playsVoice = 1;
    }
    random = Get_Rnd_001B8CD0(groupId, entryId);
    groupStatus = func_001B8D08(groupId, entryId, random + scoreBias);
    movieDataValue = indexedTableGet(groupId, entryId);
    entryTally = indexedCountGet(groupId, entryId);
    movieEntryValue = indexedValueGet(entryTally);
    if (playsVoice != 0) {
        if (groupId == 10) {
            movieDecoderReset(movieEntryValue, scoreBias);
        } else {
            movieSetFlagByte(movieEntryValue, movieDataValue, scoreBias);
        }
        seVoiceStartDefault(12);
    }
    return groupStatus;
}

s32 func_001B5C38(s32, s32);

s32 mathArgsTest(SbThread *thread) {
    s32 firstVmArg;
    int secondVmArg;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    firstVmArg = vmArgGet(thread, 1);
    secondVmArg = vmArgGet(thread, 2);
    return func_001B5C38(firstVmArg, secondVmArg) != 0;
}

/* The dispModelRecord RgDispModelFindByKey returns (rg_dispmodel.c); this native stores its argument at +0x210, partial. */
typedef struct RgDispModelRecord {
    unsigned char unmodeled_00[0x210];
    s32 storedValue; /* +0x210 */
} RgDispModelRecord;

RgDispModelRecord *RgDispModelFindByKey();

s32 scriptDispModelValueStore(SbThread *thread) {
    RgDispModelRecord *dispModelRecord;
    s32 recordedValue;

    dispModelRecord = RgDispModelFindByKey(0);
    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    recordedValue = vmArgGet(thread, 1);
    if (RgDispModelFindByKey(recordedValue) == 0) {
        return 0;
    }
    dispModelRecord->storedValue = recordedValue;
    return 1;
}

void xenoSpawnerStartAim(f32 x, f32 y, f32 z);

static inline f32 argScaled(SbThread *thread, int vmArgPosition) {
    return (f32) vmArgGet(thread, vmArgPosition) * 0.01f;
}

s32 func_001CD490(SbThread *thread) {
    f32 x;
    f32 y;
    f32 z;

    if (VM_PLUGIN_CNT_GET(thread) != 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    x = argScaled(thread, 1);
    y = argScaled(thread, 2);
    z = argScaled(thread, 3);
    xenoSpawnerStartAim(x, y, z);
    return 1;
}

s32 xenoSpawnerAimAtTarget();

s32 vmShootTrap(void) {
    xenoSpawnerAimAtTarget();
    return 1;
}

s32 flatSlotMarkAll(void);

s32 vmClearTrap(void) {
    flatSlotMarkAll();
    return 1;
}

s32 sceneOverlayReset();

s32 vmMinigame(void) {
    sceneOverlayReset();
    return 1;
}

s32 vmArgGetOrDefault();

s32 func_001B4810();

extern short D_003E4822[];

s32 func_001CD5B0(s32 thread) {
    s32 storedHalfword;
    s32 applyImmediately;

    storedHalfword = vmArgGetOrDefault(thread, 1, 0);
    applyImmediately = vmArgGetOrDefault(thread, 2, 0);
    D_003E4822[0] = storedHalfword;
    if (applyImmediately != 0) {
        func_001B4810();
    }
    return 1;
}

s32 vmArgGet();

s32 vmArgTagIsZero();

s32 padSetLatchedValue();

void Vibration_Set_Strong(unsigned char startValue, unsigned char endValue, int framesTotal);

void Vibration_Stop(void);

s32 sbNativeVibrationStart(s32 thread) {
    s32 latchedPadValue;
    s32 durationFrames;
    s32 startStrength;
    s32 endStrength;

    if (vmArgTagIsZero(thread, 1) == 0) {
        latchedPadValue = vmArgGet(thread, 1);
    } else {
        latchedPadValue = 0;
    }
    if (vmArgTagIsZero(thread, 2) == 0) {
        durationFrames = vmArgGet(thread, 2);
    } else {
        durationFrames = 0;
    }
    if (vmArgTagIsZero(thread, 3) == 0) {
        startStrength = vmArgGet(thread, 3);
    } else {
        startStrength = 255;
    }
    if (vmArgTagIsZero(thread, 4) == 0) {
        endStrength = vmArgGet(thread, 4);
    } else {
        endStrength = 0;
    }
    if ((latchedPadValue | durationFrames) != 0) {
        padSetLatchedValue(latchedPadValue);
        Vibration_Set_Strong(startStrength, endStrength, durationFrames);
    } else {
        Vibration_Stop();
    }
    return 1;
}

s32 sefFlagSetBit26();

s32 sbNativeSynopsisSkip(void) {
    sefFlagSetBit26();
    return 1;
}

s32 sbNativeArgFlagIsSet(s32 thread) {
    s32 stateSourceId;
    stateSourceId = vmArgGet(thread, 1);
    return RgDispModelFindByKey(stateSourceId) != 0;
}

s32 VM_PLUGIN_CNT_GET();

s32 sbSlotJumpRequest();

s32 sbNativeLoopScaleSet(s32 thread) {
    s32 loopScaleFirstPercent;
    s32 loopScaleSecondPercent;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    loopScaleFirstPercent = vmArgGet(thread, 1);
    loopScaleSecondPercent = vmArgGet(thread, 2);
    gGameLoopRuntimeStorage->loopScaleSecond = (float) loopScaleSecondPercent / 100.0f;
    gGameLoopRuntimeStorage->loopScaleFirst = (float) loopScaleFirstPercent / 100.0f;
    return 1;
}

typedef struct ActorOwnerBlock {
    unsigned char unmodeled_000[0x830];
    XenoActor *actor;
} ActorOwnerBlock;

typedef struct HidePartNames {
    const char *names[8];
} HidePartNames;

const HidePartNames gActorHidePartNames[] = {
    { D_005379F8[0], D_005379F8[1], D_005379F8[2], D_005379F8[3], D_005379F8[4], D_005379F8[5], D_005379F8[6], 0 },
};

ActorOwnerBlock *stateBlockGet(void);

s32 XenoActorHidePart(XenoActor *actor, const char *partName);

void func_0028F2C8(float scale);

s32 sbNativeActorPartsHide(s32 thread) {
    s32 hideFirst;
    s32 hideSecond;
    s32 hideThird;
    s32 hideFourth;
    s32 hideFifth;
    s32 modelScalePercent;
    s32 hideNamedParts;
    HidePartNames partNames;
    ActorOwnerBlock *stateBlock;
    s32 i;

    if (vmArgTagIsZero(thread, 1) == 0) {
        hideFirst = vmArgGet(thread, 1);
    } else {
        hideFirst = 0;
    }
    if (vmArgTagIsZero(thread, 2) == 0) {
        hideSecond = vmArgGet(thread, 2);
    } else {
        hideSecond = 0;
    }
    if (vmArgTagIsZero(thread, 3) == 0) {
        hideThird = vmArgGet(thread, 3);
    } else {
        hideThird = 0;
    }
    if (vmArgTagIsZero(thread, 4) == 0) {
        hideFourth = vmArgGet(thread, 4);
    } else {
        hideFourth = 0;
    }
    if (vmArgTagIsZero(thread, 5) == 0) {
        hideFifth = vmArgGet(thread, 5);
    } else {
        hideFifth = 0;
    }
    if (vmArgTagIsZero(thread, 6) == 0) {
        modelScalePercent = vmArgGet(thread, 6);
    } else {
        modelScalePercent = 0;
    }
    if (vmArgTagIsZero(thread, 7) == 0) {
        hideNamedParts = vmArgGet(thread, 7);
    } else {
        hideNamedParts = 0;
    }

    if (hideFirst != 0) {
        gGameLoopRuntimeStorage->partHideFlags |= 0x1;
    }
    if (hideSecond != 0) {
        gGameLoopRuntimeStorage->partHideFlags |= 0x2;
    }
    if (hideThird != 0) {
        gGameLoopRuntimeStorage->partHideFlags |= 0x4;
    }
    if (hideFourth != 0) {
        gGameLoopRuntimeStorage->partHideFlags |= 0x8;
    }
    if (hideFifth != 0) {
        gGameLoopRuntimeStorage->partHideFlags |= 0x10;
    }
    if (modelScalePercent != 0) {
        func_0028F2C8((float) modelScalePercent * 0.01f);
    }
    if (hideNamedParts != 0) {
        partNames = gActorHidePartNames[0];
        stateBlock = stateBlockGet();
        for (i = 0; partNames.names[i] != 0; i++) {
            XenoActorHidePart(stateBlock->actor, partNames.names[i]);
        }
    }
    return 1;
}

typedef struct RgDispModel {
    unsigned char unk_00[0x24];
    s32 actorId;
} RgDispModel;

RgDispModel *RgDispModelNextByKind();

s32 XenoActorSetFirstWord();

s32 sbNativeModelsTimeoutSet(s32 thread) {
    RgDispModel *model;
    s32 timeoutFrames;

    model = 0;
    if (vmArgTagIsZero(thread, 1) == 0) {
        timeoutFrames = vmArgGet(thread, 1);
    } else {
        timeoutFrames = 30;
    }
    for (;;) {
        model = RgDispModelNextByKind(7, model);
        if (model == 0) {
            break;
        }
        if (model->actorId != 0) {
            XenoActorSetFirstWord(model->actorId, timeoutFrames);
        }
    }
    return 1;
}

s32 func_001BBC68();

s32 pendingCountSet();

s32 sbNativeWaitArg(s32 thread) {
    s32 pendingCount;

    if (vmArgTagIsZero(thread, 1) == 0) {
        pendingCount = vmArgGet(thread, 1);
    } else {
        pendingCount = 0;
    }
    func_001BBC68();
    if (pendingCount > 0) {
        pendingCountSet(pendingCount);
    }
    return 1;
}

typedef struct RuntimeNativeMethod {
    const char *name;
    int (*func)();
    s32 argCount;
} RuntimeNativeMethod;

s32 func_001CAC00();
s32 func_001CC678();
s32 sbNativeAlwaysSucceed();
s32 sbNativeCallKeyedEntry();
s32 sbNativeCdUpdateMaxByte();
s32 sbNativeGetStoredValue();
s32 sbNativeJumpToKeyedEntry();
s32 sbNativeNoop();
s32 sbNativeQueueObjectRequest();
s32 sbNativeSetBit();
s32 sbNativeSetStoredValue();
s32 sbNativeSetTableEntry();
s32 sbNativeTestBit();
s32 scriptCameraTargetFlagSet();
s32 scriptDeferredCallRequest();
s32 scriptDispModelSlotPairHide();
s32 scriptDispModelSlotPairVisibleSet();
s32 scriptEffectParamsSetChained();
s32 scriptEffectParamsSetDefault();
s32 scriptFogOverrideSet();
s32 scriptGameStateQuery();
s32 scriptObjectSetupRequestSubmit();
s32 scriptRuntimeFlagToggle();
s32 scriptRuntimeParamsSet();
s32 scriptSlotObjectFlagBitsSet();
s32 scriptSlotObjectPropertySet();
s32 scriptSlotObjectScaleOffsetSet();
s32 scriptSlotObjectScaledArgSet();
s32 vmSetColor();
s32 vmSetFlag2000();
s32 vmSetGlobalColorBytes();
s32 vmSetGlobalTag();

RuntimeNativeMethod gSbRuntimeNativeTable[] = {
    { D_00537D38, func_001CAC00, 2 },
    { "setPos", scriptSlotObjectScaleOffsetSet, 1 },
    { "setLocator", scriptObjectSetupRequestSubmit, 8 },
    { "setPcSpeed", scriptSlotObjectScaledArgSet, 1 },
    { "setFog", scriptFogOverrideSet, 8 },
    { "setColorFilter", scriptRuntimeParamsSet, 1 },
    { "setMapAlphaInvert", vmSetFlag2000, 1 },
    { "setMapColor", vmSetColor, 3 },
    { "setFadeColor", vmSetGlobalColorBytes, 3 },
    { "setBattleMap", vmSetGlobalTag, 1 },
    { "setDoorAttr", scriptSlotObjectFlagBitsSet, 3 },
    { "setDoorParam", scriptSlotObjectPropertySet, 2 },
    { "openDoor", scriptDispModelSlotPairVisibleSet, 1 },
    { "closeDoor", scriptDispModelSlotPairHide, 1 },
    { "loadEnemy", sbNativeNoop, 1 },
    { "encount", scriptEffectParamsSetDefault, 1 },
    { "encountBoss", scriptEffectParamsSetChained, 1 },
    { "message", sbNativeSetTableEntry, 2 },
    { "jumpPos", func_001CC678, 3 },
    { "jumpLocator", sbNativeQueueObjectRequest, 1 },
    { "mapPreload", scriptCameraTargetFlagSet, 1 },
    { "mapJump", scriptDeferredCallRequest, 1 },
    { "getSceneFlag", sbNativeGetStoredValue, 0 },
    { "setSceneFlag", sbNativeSetStoredValue, 1 },
    { "getFlag", sbNativeTestBit, 1 },
    { "setFlag", sbNativeSetBit, 2 },
    { "getSysParam", scriptGameStateQuery, 1 },
    { "setSysParam", scriptRuntimeFlagToggle, 2 },
    { "event", sbNativeJumpToKeyedEntry, 1 },
    { "openingNarr", sbNativeAlwaysSucceed, 0 },
    { "mpeg2", sbNativeCallKeyedEntry, 1 },
    { "padRead", scriptMovieAutoSelectFlagGet, 1 },
    { "analogRead", scriptMovieFlagByteGet, 1 },
    { "setNextMap", sbDeferredCallStore, 1 },
    { "jumpToNext", sbDeferredCallRun, 0 },
    { "getItem", mathRandom, 2 },
    { "setItem", func_001CD248, 3 },
    { "kisekae", mathArgsTest, 2 },
    { "chgTarget", scriptDispModelValueStore, 1 },
    { "setTrap", func_001CD490, 3 },
    { "shootTrap", vmShootTrap, 0 },
    { "clearTrap", vmClearTrap, 0 },
    { "minigame", vmMinigame, 0 },
    { "setIntermis", func_001CD5B0, 1 },
    { "vibration", sbNativeVibrationStart, 0 },
    { "abort", sbNativeSynopsisSkip, 0 },
    { "exist", sbNativeArgFlagIsSet, 0 },
    { "shadowRange", sbNativeLoopScaleSet, 2 },
    { "setKonishi", sbNativeActorPartsHide, 0 },
    { "dynamaki", sbNativeModelsTimeoutSet, 0 },
    { "warningOff", sbNativeWaitArg, 0 },
    { "waitRead", sbNativeCdUpdateMaxByte, 0 },
    { 0, 0, 0 },
};

const char D_00537D38[16] = "setNumber";
