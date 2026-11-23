#include "common.h"

#include "main/xenoActor.h"
typedef int s32;

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

s32 RgDispModelFindByKey();

s32 sbNativeArgFlagIsSet(s32 thread) {
    s32 stateSourceId;
    stateSourceId = vmArgGet(thread, 1);
    return RgDispModelFindByKey(stateSourceId) != 0;
}

extern float gGameLoopRuntimeStorage[];
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
    gGameLoopRuntimeStorage[123] = (float) loopScaleSecondPercent / 100.0f;
    gGameLoopRuntimeStorage[122] = (float) loopScaleFirstPercent / 100.0f;
    return 1;
}

typedef struct GameLoopRuntime {
    unsigned char unmodeled_000[0x1E8];
    float loopScaleFirst;
    float loopScaleSecond;
    s32 partHideFlags;
} GameLoopRuntime;

#define gGameLoopRuntimeView (*(GameLoopRuntime *) gGameLoopRuntimeStorage)

typedef struct ActorOwnerBlock {
    unsigned char unmodeled_000[0x830];
    XenoActor *actor;
} ActorOwnerBlock;

typedef struct HidePartNames {
    const char *names[8];
} HidePartNames;

extern HidePartNames gActorHidePartNames[];
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
        gGameLoopRuntimeView.partHideFlags |= 0x1;
    }
    if (hideSecond != 0) {
        gGameLoopRuntimeView.partHideFlags |= 0x2;
    }
    if (hideThird != 0) {
        gGameLoopRuntimeView.partHideFlags |= 0x4;
    }
    if (hideFourth != 0) {
        gGameLoopRuntimeView.partHideFlags |= 0x8;
    }
    if (hideFifth != 0) {
        gGameLoopRuntimeView.partHideFlags |= 0x10;
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
s32 func_00290548();

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
            func_00290548(model->actorId, timeoutFrames);
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
