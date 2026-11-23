#include "common.h"

#include "shared.h"
#include "quadword.h"
#include "vu0_macro.h"
typedef int s32;

typedef float f32;

typedef struct SlotActorHead {
    unsigned char unmodeled_00[4];
    s32 modeActiveFlag;
    unsigned char unmodeled_08[0x8];
    Vector4 vector10; /* 0x10: written by sbSetObjectFlagVector; w at 0x1C is half the XZ length */
} SlotActorHead;

typedef struct SbVmObjectState {
    unsigned char unk_000[0x18];
    f32 scaledArg; /* 0x168: the vm argument scaled by scriptSlotObjectScaledArgSet */
    unsigned char unmodeled_01C[0x50 - 0x1C];
    s32 vmObjectFlagBits; /* 0x1A0: bits set or cleared by sbSetObjectFlagBits */
    u32 behaviorMode;        /* 0x1A4: written by the mode setter */
    unsigned char unmodeled_1A8[0x8];
    Vector4 flagVector; /* object +0x1B0: stored by sbSetObjectFlagVector, copied to the flag slotActor */
    f32 scaleExtent;
    f32 angleRadians; /* 0x1C4: written by sbSetObjectAngle */
    s32 angleFlag;    /* 0x1C8: written by sbSetObjectAngle when nonzero */
    s32 valuePairPrimary;
    s32 valuePairSecondary;
} SbVmObjectState;

typedef struct SbVmObject {
    unsigned char unk_000[0x18];
    struct SbVmObject *boundObject; /* 0x18: object bound to this one; scriptSlotObjectFlagBitsSet flags it too */
    unsigned char unk_01C[0x24 - 0x1C];
    SlotActorHead *actor; /* 0x24: flag slotActor written by the mode setter */
    unsigned char unk_028[0x40 - 0x28];
    f32 defaultScaleX; /* 0x40, 0x44, 0x48: defaults of the first vector (scriptSlotObjectScaleOffsetSet) */
    f32 defaultScaleY;
    f32 defaultScaleZ;
    unsigned char unk_04C[0x60 - 0x4C];
    f32 defaultOffsetX; /* 0x60, 0x64, 0x68: defaults of the second vector (scriptSlotObjectScaleOffsetSet) */
    f32 defaultOffsetAngle;
    f32 defaultOffsetZ;
    unsigned char unk_06C[0x134 - 0x6C];
    f32 objectScale;
    unsigned char unk_138[0x18];
    SbVmObjectState vmState; /* +0x150 */
    unsigned char unk_1D4[0x850 - 0x1D4];
    f32 colorRed;   /* 0x850: set by vmSetColor */
    f32 colorGreen; /* 0x854 */
    f32 colorBlue;  /* 0x858 */
} SbVmObject;

s32 VM_PLUGIN_CNT_GET();
s32 sbSlotJumpRequest();
s32 vmArgGet();
s32 vmArgTagIsZero();
void SpawnSlotSetPosition(SbVmObject *object, Vector4 *vector);
void SpawnSlotSetRotation(SbVmObject *object, Vector4 *vector);
void func_00290548(SlotActorHead *actor, s32 timeout);
SbVmObject *RgDispModelFindByKey(s32 index);

/* Script native: sets the scale and offset vectors of a slot object from 4-5 arguments, defaulting to the stored ones. */
s32 scriptSlotObjectScaleOffsetSet(s32 thread) {
    SbVmObject *modelSlot;
    AlignedVector4 scale;
    AlignedVector4 offsetVector;

    if (VM_PLUGIN_CNT_GET(thread) < 4 || VM_PLUGIN_CNT_GET(thread) >= 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    modelSlot = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (modelSlot == 0) {
        return 0;
    }
    Vu0SetOrigin(&scale);
    Vu0SetOrigin(&offsetVector);
    scale.x = vmArgTagIsZero(thread, 2) ? modelSlot->defaultScaleX : vmArgGet(thread, 2) * 0.01f;
    scale.y = vmArgTagIsZero(thread, 3) ? modelSlot->defaultScaleY : vmArgGet(thread, 3) * 0.01f;
    scale.z = vmArgTagIsZero(thread, 4) ? modelSlot->defaultScaleZ : vmArgGet(thread, 4) * 0.01f;
    offsetVector.x = modelSlot->defaultOffsetX;
    offsetVector.y =
        vmArgTagIsZero(thread, 5) ? modelSlot->defaultOffsetAngle : vmArgGet(thread, 5) * 0.01f * 0.0174532924f;
    offsetVector.z = modelSlot->defaultOffsetZ;
    SpawnSlotSetPosition(modelSlot, &scale);
    SpawnSlotSetRotation(modelSlot, &offsetVector);
    if (modelSlot->actor == 0) {
        return 1;
    }
    func_00290548(modelSlot->actor, 30);
    return 1;
}

typedef struct ObjectSetupRequest {
    s32 setupFirstWord;  /* 0x00 */
    s32 setupSecondWord; /* 0x04 */
    unsigned char unk_008[0x10 - 0x08];
    AlignedVector4 primaryVector; /* 0x10 */
    AlignedVector4 secondaryVector; /* 0x20 */
    f32 scaleFactor;          /* 0x30 */
    f32 angleRadiansA;          /* 0x34 */
    f32 angleRadiansB;          /* 0x38 */
} ObjectSetupRequest;

s32 spawnMotionActorCreate(ObjectSetupRequest *setupRequest);

/* Script native: builds an object setup request from 8 to 11 arguments (scaled by 0.01) and submits it to spawnMotionActorCreate. */
s32 scriptObjectSetupRequestSubmit(s32 thread) {
    ObjectSetupRequest setupRequest;

    if (VM_PLUGIN_CNT_GET(thread) < 8 || VM_PLUGIN_CNT_GET(thread) >= 12) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    Vu0SetOrigin(&setupRequest.primaryVector);
    Vu0SetOrigin(&setupRequest.secondaryVector);
    setupRequest.setupFirstWord = vmArgGet(thread, 1);
    setupRequest.setupSecondWord = vmArgGet(thread, 2);
    setupRequest.primaryVector.x = vmArgGet(thread, 3) * 0.01f;
    setupRequest.primaryVector.y = vmArgGet(thread, 4) * 0.01f;
    setupRequest.primaryVector.z = vmArgGet(thread, 5) * 0.01f;
    setupRequest.secondaryVector.x = vmArgGet(thread, 6) * 0.01f;
    setupRequest.secondaryVector.y = vmArgGet(thread, 7) * 0.01f;
    setupRequest.secondaryVector.z = vmArgGet(thread, 8) * 0.01f;
    setupRequest.scaleFactor = vmArgTagIsZero(thread, 9) ? 1.0f : vmArgGet(thread, 9) * 0.01f;
    setupRequest.angleRadiansA = vmArgTagIsZero(thread, 10) ? 0.0f : vmArgGet(thread, 10) * 0.01f * 0.017453292f;
    setupRequest.angleRadiansB = vmArgTagIsZero(thread, 11) ? 0.0f : vmArgGet(thread, 11) * 0.01f * 0.017453292f;
    return spawnMotionActorCreate(&setupRequest);
}

/* Script native: stores its argument scaled by 0.01 and by 1/30 in the scaled argument of slot object kind 0. */
s32 scriptSlotObjectScaledArgSet(s32 thread) {
    SbVmObject *modelSlot;
    s32 scaleArg;

    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    modelSlot = RgDispModelFindByKey(0);
    if (modelSlot == 0) {
        return 0;
    }
    scaleArg = vmArgGet(thread, 1);
    modelSlot->vmState.scaledArg = scaleArg * 0.01f * 0.033333f;
    return 1;
}

typedef struct SlotActorFogView {
    unsigned char unmodeled_000[0x90];
    s32 overrideFlags; /* 0x90: bit 0 is set on every call */
    unsigned char unmodeled_094[0x2C0 - 0x94];
    Vector4 fogParamsVector; /* 0x2C0: copied from the fogParams vector argument */
    Vector4 fogColorVector; /* 0x2D0: copied from the fogColor vector argument */
} SlotActorFogView;

typedef struct StateBlockFogView {
    unsigned char unmodeled_0000[0x830];
    SlotActorFogView *fogOverrideActor; /* 0x830: the fog slotActor scriptFogOverrideSet hands to slotActorFogOverrideStore */
} StateBlockFogView;

void slotActorFogOverrideStore(SlotActorFogView *slotActor, const Vector4 *fogParams, const Vector4 *fogColor);
SbVmObject *RgDispModelNextByKind(s32 mask, SbVmObject *previous);
void sefSetFogOverride(Vector4 *fogParams, Vector4 *fogColor);
void *stateBlockGet();

/* Script native: sets the fog override vectors of the state block (-1), of every slot object (-2 or -1), or of one slot object. */
s32 scriptFogOverrideSet(s32 thread) {
    Vector4 fogParams;
    Vector4 fogColor;
    SbVmObject *modelSlot;
    s32 fogTarget;

    if (VM_PLUGIN_CNT_GET(thread) != 8) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    fogTarget = vmArgGet(thread, 1);
    fogParams.x = vmArgGet(thread, 2) * 0.01f;
    fogParams.y = vmArgGet(thread, 3) * 0.01f;
    fogParams.z = vmArgGet(thread, 4) * 0.01f;
    fogParams.w = vmArgGet(thread, 5) * 0.01f;
    fogColor.x = vmArgGet(thread, 6);
    fogColor.y = vmArgGet(thread, 7);
    fogColor.z = vmArgGet(thread, 8);
    fogColor.w = 128.0f;

    switch (fogTarget) {
    case -1:
        slotActorFogOverrideStore(((StateBlockFogView *) stateBlockGet())->fogOverrideActor, &fogParams, &fogColor);
        sefSetFogOverride(&fogParams, &fogColor);
        /* fall through */
    case -2:
        modelSlot = 0;
        while ((modelSlot = RgDispModelNextByKind(0xfffff, modelSlot)) != 0) {
            slotActorFogOverrideStore((SlotActorFogView *) modelSlot->actor, &fogParams, &fogColor);
        }
        return 1;
    default:
        break;
    }
    modelSlot = RgDispModelFindByKey(fogTarget);
    if (modelSlot == 0) {
        return 0;
    }
    slotActorFogOverrideStore((SlotActorFogView *) modelSlot->actor, &fogParams, &fogColor);
    return 1;
}

/* Marks the actor head as overridden and copies the two fog vectors into it. */
void slotActorFogOverrideStore(SlotActorFogView *slotActor, const Vector4 *fogParams, const Vector4 *fogColor) {
    if (slotActor != 0) {
        slotActor->overrideFlags |= 1;
        QuadwordCopy(&slotActor->fogParamsVector, fogParams);
        QuadwordCopy(&slotActor->fogColorVector, fogColor);
    }
}

typedef struct GameLoopRuntime {
    union {
        s32 loopFlags; /* 0x00: bit 0x2000 is set or cleared by vmSetFlag2000 */
        struct {
            unsigned int lowFlagBits : 6;
            unsigned int scriptParamsActive : 1; /* 0x40: set or cleared by scriptRuntimeParamsSet */
            unsigned int highFlagBits : 25;
        } bits;
    };
    unsigned char unk_04[0x10];
    unsigned char colorBytes[3]; /* 0x14: written by vmSetGlobalColor */
    unsigned char unk_17[0x111];
    s32 activeHandle;  /* 0x128: written by vmSetGlobalTag */
    s32 pendingHandle; /* 0x12C: activeHandle with the 0x06010000 class bits */
    unsigned char unk_130[0x174 - 0x130];
    s32 scriptParamSlots[5]; /* 0x174: vm arguments 2..6 written by scriptRuntimeParamsSet */
} GameLoopRuntime;

extern unsigned char gGameLoopRuntimeStorage[];

#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)

/* Script native: with a nonzero first argument stores five parameters in the runtime block and activates them; otherwise deactivates. */
s32 scriptRuntimeParamsSet(s32 thread) {
    s32 scriptParamA;
    s32 scriptParamB;
    s32 scriptParamC;
    s32 scriptParamD;
    s32 scriptParamE;

    if (VM_PLUGIN_CNT_GET(thread) != 1 && VM_PLUGIN_CNT_GET(thread) != 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgGet(thread, 1) != 0) {
        if (VM_PLUGIN_CNT_GET(thread) != 6) {
            sbSlotJumpRequest(thread, 2);
            return 0;
        }
        scriptParamA = vmArgGet(thread, 2);
        scriptParamB = vmArgGet(thread, 3);
        scriptParamC = vmArgGet(thread, 4);
        scriptParamD = vmArgGet(thread, 5);
        scriptParamE = vmArgGet(thread, 6);
        gGameLoopRuntime.bits.scriptParamsActive = 1;
        gGameLoopRuntime.scriptParamSlots[0] = scriptParamA;
        gGameLoopRuntime.scriptParamSlots[1] = scriptParamB;
        gGameLoopRuntime.scriptParamSlots[2] = scriptParamC;
        gGameLoopRuntime.scriptParamSlots[3] = scriptParamD;
        gGameLoopRuntime.scriptParamSlots[4] = scriptParamE;
    } else {
        gGameLoopRuntime.bits.scriptParamsActive = 0;
    }
    return 1;
}

typedef signed char s8;


/* Sets the flag 0x2000 of the global state when argument 1 is nonzero, clears it otherwise. */
s32 vmSetFlag2000(s32 thread) {
    s32 flagsWithBit;
    s32 flagsWithoutBit;

    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgGet(thread, 1) != 0) {
        flagsWithBit = gGameLoopRuntime.loopFlags | 0x2000;
        gGameLoopRuntime.loopFlags = flagsWithBit;
    } else {
        flagsWithoutBit = gGameLoopRuntime.loopFlags & ~0x2000;
        gGameLoopRuntime.loopFlags = flagsWithoutBit;
    }
    return 1;
}

/* Stores the three colour arguments (0..255) as floats in the colour fields of the object returned by stateBlockGet. */
s32 vmSetColor(s32 thread)
{
  f32 red;
  f32 green;
  f32 blue;
  SbVmObject *scriptObject;
  if (VM_PLUGIN_CNT_GET() != 3)
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  red = vmArgGet(thread, 1);
  green = vmArgGet(thread, 2);
  blue = vmArgGet(thread, 3);
  scriptObject = stateBlockGet();
  scriptObject->colorBlue = blue / 255.0f;
  scriptObject->colorGreen = green / 255.0f;
  scriptObject->colorRed = red / 255.0f;
  return 1;
}

s32 vmSetGlobalColorBytes(s32 thread)
{
  s32 red;
  s32 green;
  s32 blue;
  if (VM_PLUGIN_CNT_GET(thread) != 3)
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  red = vmArgGet(thread, 1) & 0xFF;
  green = vmArgGet(thread, 2) & 0xFF;
  blue = vmArgGet(thread, 3);
  gGameLoopRuntime.colorBytes[1] = green;
  gGameLoopRuntime.colorBytes[2] = blue;
  gGameLoopRuntime.colorBytes[0] = red;
  return 1;
}

s32 vmSetGlobalTag(s32 thread)
{
  s32 tagArg;
  s32 tag;
  if (VM_PLUGIN_CNT_GET(thread) != 1)
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  tagArg = vmArgGet(thread, 1);
  tag = (tagArg & 0xFFFF) | 0x06010000;
  gGameLoopRuntime.activeHandle = tagArg;
  gGameLoopRuntime.pendingHandle = tag;
  return 1;
}

void sbSetObjectFlagBits(SbVmObject *scriptObject, s32 mask, s32 setBits);

/* Script native: sets or clears flag bits of a slot object and of its bound object. */
s32 scriptSlotObjectFlagBitsSet(s32 thread) {
    SbVmObject *modelSlot;
    s32 slotKindSelector;
    s32 flagMask;
    s32 setBitsArg;

    if (VM_PLUGIN_CNT_GET(thread) != 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotKindSelector = vmArgGet(thread, 1);
    flagMask = vmArgGet(thread, 2);
    setBitsArg = vmArgGet(thread, 3);
    modelSlot = RgDispModelFindByKey(slotKindSelector);
    if (modelSlot == 0) {
        return 0;
    }
    sbSetObjectFlagBits(modelSlot, flagMask, setBitsArg);
    if (modelSlot->boundObject != 0) {
        sbSetObjectFlagBits(modelSlot->boundObject, flagMask, setBitsArg);
    }
    return 1;
}

/* Sets (setBits != 0) or clears the bits of mask in the object flags at vmState +0x50 (0x1A0). */
void sbSetObjectFlagBits(SbVmObject *scriptObject, s32 mask, s32 setBits) {
    SbVmObjectState *vmState;

    vmState = &scriptObject->vmState;
    if (setBits != 0) {
        vmState->vmObjectFlagBits |= mask;
        return;
    }
    vmState->vmObjectFlagBits &= ~mask;
}

void sbSetObjectBehaviorMode(SbVmObject *scriptObject, s32 behaviorMode);

void sbSetObjectFlagVector(SbVmObject *scriptObject, f32 x, f32 y, f32 z);

void sbSetObjectAngle(SbVmObject *scriptObject, s32 angleFlagArg, f32 angleInput, s32 negate);
void sbSetObjectScale(SbVmObject *scriptObject, f32 scale);
void sbSetObjectFloatAlt(SbVmObject *scriptObject, f32 extentOverride);
void sbSetObjectWordPair(SbVmObject *scriptObject, s32 valuePairPrimary, s32 valuePairSecondary);

/* Script native: sets one property (mode, vector, angle, scale, extent, value pair) of a slot object and of its bound object. */
s32 scriptSlotObjectPropertySet(s32 thread) {
    SbVmObject *modelSlot;
    s32 slotKindSelector;
    s32 propertyKind;
    s32 attrPairSecond;
    f32 x;
    f32 y;
    f32 z;

    if (VM_PLUGIN_CNT_GET(thread) < 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotKindSelector = vmArgGet(thread, 1);
    propertyKind = vmArgGet(thread, 2);
    modelSlot = RgDispModelFindByKey(slotKindSelector);
    if (modelSlot == 0) {
        return 0;
    }
    switch (propertyKind) {
    case 0:
        propertyKind = vmArgGet(thread, 3);
        sbSetObjectBehaviorMode(modelSlot, propertyKind);
        if (modelSlot->boundObject != 0) {
            sbSetObjectBehaviorMode(modelSlot->boundObject, propertyKind);
        }
        break;
    case 1:
        x = vmArgGet(thread, 3) * 0.01f;
        y = vmArgGet(thread, 4) * 0.01f;
        z = vmArgTagIsZero(thread, 5) ? 0.0f : vmArgGet(thread, 5) * 0.01f;
        sbSetObjectFlagVector(modelSlot, x, y, z);
        if (modelSlot->boundObject != 0) {
            sbSetObjectFlagVector(modelSlot->boundObject, x, y, z);
        }
        break;
    case 2:
        propertyKind = vmArgTagIsZero(thread, 3) ? 0 : vmArgGet(thread, 3);
        y = vmArgTagIsZero(thread, 4) ? 0.0f : vmArgGet(thread, 4) * 0.01f;
        sbSetObjectAngle(modelSlot, propertyKind, y, 0);
        if (modelSlot->boundObject != 0) {
            sbSetObjectAngle(modelSlot->boundObject, propertyKind, y, 1);
        }
        break;
    case 3:
        x = vmArgGet(thread, 3) * 0.01f;
        sbSetObjectScale(modelSlot, x);
        if (modelSlot->boundObject != 0) {
            sbSetObjectScale(modelSlot->boundObject, x);
        }
        break;
    case 4:
        x = vmArgGet(thread, 3) * 0.01f;
        sbSetObjectFloatAlt(modelSlot, x);
        if (modelSlot->boundObject != 0) {
            sbSetObjectFloatAlt(modelSlot->boundObject, x);
        }
        break;
    case 5:
        propertyKind = vmArgTagIsZero(thread, 3) ? 96 : vmArgGet(thread, 3);
        attrPairSecond = vmArgTagIsZero(thread, 4) ? 97 : vmArgGet(thread, 4);
        sbSetObjectWordPair(modelSlot, propertyKind, attrPairSecond);
        if (modelSlot->boundObject != 0) {
            sbSetObjectWordPair(modelSlot->boundObject, propertyKind, attrPairSecond);
        }
        break;
    default:
        break;
    }
    return 1;
}

/* Sets the behavior mode and the mode active flag of the actor head (1 for modes 0 and 2, 0 for mode 1). */
void sbSetObjectBehaviorMode(SbVmObject *scriptObject, s32 behaviorMode) {
    SlotActorHead *actor;

    scriptObject->vmState.behaviorMode = behaviorMode;
    actor = scriptObject->actor;
    switch (behaviorMode) {
    case 0:
    case 2:
        actor->modeActiveFlag = 1;
        break;
    case 1:
        actor->modeActiveFlag = 0;
        break;
    }
}

f32 MMathCalcLengthXZ(Vector4 *vector);

/* Stores the vector at the object state and the actor head according to the behavior mode (0/2: XYZ and half the XZ length, 1: XY). */
void sbSetObjectFlagVector(SbVmObject *scriptObject, f32 x, f32 y, f32 z) {
    SbVmObjectState *vmState;
    SlotActorHead *actor;
    f32 length;
    f32 angle;

    vmState = &scriptObject->vmState;
    actor = scriptObject->actor;
    switch (vmState->behaviorMode) {
    case 0:
    case 2:
        vmState->flagVector.z = z;
        vmState->flagVector.x = x;
        vmState->flagVector.y = y;
        QuadwordCopyT0(&actor->vector10, &vmState->flagVector);
        length = MMathCalcLengthXZ(&actor->vector10);
        actor->vector10.w = length * 0.5f;
        /* kept: the do/while(0) scope decides the emitted order of the angle store */
        do {
            angle = x;
            if (vmState->behaviorMode != 0) {
                angle = y;
            }
            vmState->angleRadians = angle;
        } while (0);
        break;
    case 1:
        vmState->flagVector.y = y;
        vmState->flagVector.w = x;
        actor->vector10.x = x;
        actor->vector10.y = y;
        break;
    }
}

/* Stores the angle flag, and the angle (degrees to radians in mode 1; negated when negate is set) when nonzero. */
void sbSetObjectAngle(SbVmObject *scriptObject, s32 angleFlagArg, f32 angleInput, s32 negate) {
    SbVmObjectState *vmState;

    vmState = &scriptObject->vmState;
    if (angleFlagArg != 0) {
        vmState->angleFlag = angleFlagArg;
    }
    if (vmState->behaviorMode != 1) {
        if (vmState->behaviorMode != 0 && vmState->behaviorMode != 2) {
            return;
        }
        if (angleInput != 0.0f) {
            vmState->angleRadians = angleInput;
        }
        return;
    }
    if (angleInput != 0.0f) {
        if (negate != 0) {
            vmState->angleRadians = -angleInput * 0.017453292f;
        } else {
            vmState->angleRadians = angleInput * 0.017453292f;
        }
    }
}

void sbSetObjectScale(SbVmObject *scriptObject, f32 scale) {
    scriptObject->objectScale = scale;
    scriptObject->vmState.scaleExtent = scale * 3.0f;
}

void sbSetObjectFloatAlt(SbVmObject *scriptObject, f32 extentOverride) {
    scriptObject->vmState.scaleExtent = extentOverride;
}

void sbSetObjectWordPair(SbVmObject *scriptObject, s32 valuePairPrimary, s32 valuePairSecondary) {
    SbVmObjectState *vmState;

    vmState = &scriptObject->vmState;
    vmState->valuePairPrimary = valuePairPrimary;
    vmState->valuePairSecondary = valuePairSecondary;
}
