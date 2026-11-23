#include "common.h"

#include "shared.h"
#include "vu0_macro.h"
typedef int s32;

typedef float f32;

typedef struct GameStateScaleParts {
    unsigned char pad_0000[0x830];
    s32 partsSelf;
    f32 partsScaleA;
    f32 partsScaleB;
} GameStateScaleParts;

s32 VM_PLUGIN_CNT_GET();
void sbSlotJumpRequest(s32 thread, s32 key);
s32 vmArgTagIsZero(s32 thread, s32 index);
s32 vmArgGet(s32 thread, s32 index);

typedef struct GameStateParts {
    unsigned char pad_0000[0x830];
    s32 partsSelf;
} GameStateParts;

void *stateBlockGet();

/* Script native: stores two part scales in the game state block; arg 1 and arg 2 are hundredths, a missing tag uses the default. */
s32 func_001C7F68(s32 thread) {
    GameStateScaleParts *gameState;
    f32 scaleA;
    f32 scaleB;

    if (VM_PLUGIN_CNT_GET(thread) > 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgTagIsZero(thread, 1) == 0) {
        scaleA = vmArgGet(thread, 1) * -0.01f / 465.0f;
    } else {
        scaleA = -0.0210752692f;
    }
    if (vmArgTagIsZero(thread, 2) == 0) {
        scaleB = vmArgGet(thread, 2) * 0.01f;
    } else {
        scaleB = 0.5f;
    }
    gameState = stateBlockGet();
    gameState->partsScaleB = scaleB;
    gameState->partsScaleA = scaleA;
    return 1;
}

void mapProbeInitTimeout(int timeout);

struct GameStateTimeout {
    unsigned char unmodeled_0000[0x9E0];
    int mapProbeTimeout;
};

/* Script native: stores the map-probe timeout (arg 1, default -1) in the game state and passes it to mapProbeInitTimeout. */
int func_001C8068(int thread) {
    struct GameStateTimeout *gameState;
    int timeout;

    gameState = stateBlockGet();
    if (VM_PLUGIN_CNT_GET(thread) >= 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgTagIsZero(thread, 1) == 0) {
        timeout = vmArgGet(thread, 1);
    } else {
        timeout = -1;
    }
    gameState->mapProbeTimeout = timeout;
    mapProbeInitTimeout(timeout);
    return 1;
}

/* Script native: returns 0. */
s32 sbNativeReturnZero(void) {
    return 0;
}

typedef struct SetupParams {
    s32 mode;
    f32 fractionB;
    f32 fractionC;
    f32 fractionA;
} SetupParams;

void pointSlotStore(void *params);

/* Script native: arg 2 is the mode; args 3-5 are percentages (fractions after *0.01) with defaults; calls pointSlotStore. */
s32 func_001C8100(s32 thread) {
    s32 fractionATagZero;
    s32 mode;
    f32 fractionA;
    f32 halfOfRest;
    f32 fractionB;
    f32 fractionC;
    SetupParams params;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    vmArgGet(thread, 1);
    mode = vmArgGet(thread, 2);
    fractionATagZero = vmArgTagIsZero(thread, 3);
    if (fractionATagZero == 0) {
        fractionA = vmArgGet(thread, 3) * 0.01f;
    } else {
        fractionA = 0.4f;
    }
    halfOfRest = (1.0f - fractionA) * 0.5f;
    if (vmArgTagIsZero(thread, 4) != 0) {
        fractionB = halfOfRest;
    } else {
        fractionB = vmArgGet(thread, 4) * 0.01f;
    }
    if (vmArgTagIsZero(thread, 5) != 0) {
        fractionC = halfOfRest;
    } else {
        fractionC = vmArgGet(thread, 5) * 0.01f;
    }
    params.mode = mode;
    params.fractionB = fractionB;
    params.fractionC = fractionC;
    params.fractionA = fractionA;
    pointSlotStore(&params);
    return 1;
}

s32 MMathNormalizeVector2(Vector4 *outVector, Vector4 *inputVector);
void *RgDispModelNextByKind(s32 kind, void *previous);

typedef struct GameStateLightParts {
    unsigned char unmodeled_0000[0x830];
    s32 partsSelf;
    f32 partsScaleA;
    f32 partsScaleB;
    f32 partsScaleC;
    f32 partsScaleD;
    unsigned char unmodeled_0844[0x45C];
    unsigned char partsFlags[0x100];
    f32 partsScales[0x100];
} GameStateLightParts;

enum SlotKind {
    SLOT_KIND_NONE,
    SLOT_KIND_WIDTH = 0x7fffffff /* kept: the enum width decides the alias set of the kind field */
};

enum SlotFlags {
    SLOT_FLAGS_NONE,
    SLOT_FLAGS_WIDTH = 0x7fffffff /* kept: the enum width decides the alias set of the flags field */
};

/* The actor block a slot object points to (+0x24): size/extents, angles, rotation and scale vectors. */
typedef struct SlotActor {
    s32 unmodeled_00;
    s32 mode;
    u8 unmodeled_08[8];
    union {
        Vector4 vector;
        s32 words[4];
    } extents;
    Vector4 position;
    Vector4 vector30;
    Vector4 angle;
    Vector4 rot;
    Vector4 vector60;
    Vector4 vector70;
    Vector4 scale;
    s32 flags90;
    u8 unmodeled_98[8];
    s16 halfword9C;
    s16 halfword9E;
    f32 fogDensity;
    u8 unmodeled_A4[0xf8 - 0xa4];
    s32 field0F8;
    u8 unmodeled_FC[0x12c - 0xfc];
    f32 field12C;
    u8 unmodeled_130[0x4ec - 0x130];
    s32 field4EC;
    u8 unmodeled_4F0[0x510 - 0x4f0];
    void *field510;
    u8 unmodeled_514[0x528 - 0x514];
    void *field528;
    u8 unmodeled_52C[0x6cc - 0x52c];
    f32 field6CC;
} SlotActor;

typedef struct SlotState {
    s32 unmodeled_00;
    s32 unmodeled_04;
    union {
        void *pointer;
        s32 handle;
        f32 fraction;
    } value;
    u8 unmodeled_0C[0x20 - 0x0C];
    f32 timer;
    u8 unmodeled_24[0x90 - 0x24];
    s16 slotA;
    s16 slotB;
} SlotState;

/* A slot object allocated by RgDispModelAlloc (stride 0x230). */
typedef struct SlotObject {
    enum SlotFlags flags;
    s32 unmodeled_04;
    s32 statusFlags;
    s32 flags0C;
    enum SlotKind kind;
    s32 unmodeled_14;
    u8 unmodeled_18[4];
    void (*update)();
    void (*release)();
    SlotActor *model;
    s32 slotIndex;
    u8 unmodeled_2C[4];
    s32 childId;
    s32 linkedSlot;
    u8 unmodeled_38[8];
    Vector4 pos;
    Vector4 vector50;
    Vector4 ang;
    Vector4 vector70;
    Vector4 scale;
    u8 unmodeled_90[0xC0 - 0x90];
    f32 vectorC0[3];
    u8 unmodeled_CC[0x128 - 0xCC];
    f32 vectorW;
    s32 field12C;
    void *field130;
    f32 halfScale;
    f32 field138;
    s32 field13C;
    u8 unmodeled_140[4];
    s16 shortA;
    s16 shortB;
    u8 unmodeled_148[0x150 - 0x148];
    SlotState state;
} SlotObject;

static inline void Vector4ZeroSqc2(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}



typedef struct {
    Vector4 dir[7];
    Vector4 col[6];
    Vector4 amb;
    Vector4 x;
    Vector4 fog;
} LightTable;

typedef struct {
    u8 unmodeled_0000[0x860];
    LightTable lights;
    u8 unmodeled_0960[0xc74 - 0x960];
    f32 fogDensity;
} LightStateBlock;

/* kept apart: float-array indexing of the light table decides the emitted addressing */
typedef struct {
    f32 dirComponents[7][4];
    f32 colComponents[6][4];
} LightComponentView;

/* Script native: set directional light 0/1, ambient (2) or fog (3) colour and direction from the arguments. */
int sbNativeSetLight(s32 thread) {
    LightStateBlock *stateBlock;
    Vector4 direction;
    s32 lightId;
    SlotObject *slotObject;
    f32 red;
    f32 green;
    f32 blue;
    LightTable *lights;
    LightComponentView *components;
    stateBlock = stateBlockGet();
    lights = &stateBlock->lights;
    components = (LightComponentView *) lights;
    if (VM_PLUGIN_CNT_GET(thread) < 1 || VM_PLUGIN_CNT_GET(thread) > 7) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    lightId = vmArgGet(thread, 1);
    if (!vmArgTagIsZero(thread, 2)) {
        red = vmArgGet(thread, 2) / 255.0f;
    } else {
        red = 0.0f;
    }
    if (!vmArgTagIsZero(thread, 3)) {
        green = vmArgGet(thread, 3) / 255.0f;
    } else {
        green = red;
    }
    if (!vmArgTagIsZero(thread, 4)) {
        blue = vmArgGet(thread, 4) / 255.0f;
    } else {
        blue = red;
    }
    Vector4ZeroSqc2(&direction);
    direction.x = vmArgTagIsZero(thread, 5) ? lights->dir[0].x : vmArgGet(thread, 5) * 0.01f;
    direction.y = vmArgTagIsZero(thread, 6) ? lights->dir[0].y : vmArgGet(thread, 6) * 0.01f;
    direction.z = vmArgTagIsZero(thread, 7) ? lights->dir[0].z : vmArgGet(thread, 7) * 0.01f;
    MMathNormalizeVector2(&direction, &direction);
    switch (lightId) {
    case 0:
    case 1:
        components->dirComponents[lightId][0] = direction.x;
        components->dirComponents[lightId][1] = direction.y;
        components->dirComponents[lightId][2] = direction.z;
        components->colComponents[lightId][0] = red;
        components->colComponents[lightId][1] = green;
        components->colComponents[lightId][2] = blue;
        break;
    case 2:
        lights->amb.x = red;
        lights->amb.y = green;
        lights->amb.z = blue;
        break;
    case 3:
        lights->fog.x = direction.x;
        lights->fog.y = direction.y;
        lights->fog.z = direction.z;
        lights->fog.w = 1.0f;
        slotObject = 0;
        if (!vmArgTagIsZero(thread, 2)) {
            red = vmArgGet(thread, 2) / 100.0f;
            stateBlock->fogDensity = red;
        } else {
            red = 0.2f;
            stateBlock->fogDensity = red;
        }
        while ((slotObject = RgDispModelNextByKind(7, slotObject)) != 0) {
            if (slotObject->model != 0) {
                slotObject->model->fogDensity = red;
            }
        }
        break;
    default:
        return 0;
    }
    return 1;
}

typedef struct DispParamBlock {
    f32 pos[4];
    s32 flag;
    s32 reserved[3];
    f32 scale[4];
} DispParamBlock;

void func_00293AB8(s32 object, DispParamBlock *block, s32 *flag);

/* Script native: 8 arguments: object id (arg 1), colour (args 2-4, 0..255), flag 1, four scales (args 5-8, hundredths) passed to func_00293AB8. */
s32 func_001C85C8(s32 thread) {
    DispParamBlock block;
    s32 object;
    f32 scaleUnit;

    if (VM_PLUGIN_CNT_GET(thread) != 8) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    object = vmArgGet(thread, 1);
    block.pos[0] = vmArgGet(thread, 2) / 255.0f;
    block.pos[1] = vmArgGet(thread, 3) / 255.0f;
    block.pos[2] = vmArgGet(thread, 4) / 255.0f;
    block.flag = 1;
    scaleUnit = 0.01f;
    block.scale[0] = vmArgGet(thread, 5) * scaleUnit;
    block.scale[1] = vmArgGet(thread, 6) * scaleUnit;
    block.scale[2] = vmArgGet(thread, 7) * scaleUnit;
    block.scale[3] = vmArgGet(thread, 8) * scaleUnit;
    func_00293AB8(object, &block, &block.flag);
    return 1;
}

void xenoActorLightSlotClear(s32 index);

/* Script native: clears the light slot given by arg 1. */
s32 sbNativeLightClear(s32 thread) {
    s32 lightSlot;
    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    lightSlot = vmArgGet(thread, 1);
    xenoActorLightSlotClear(lightSlot);
    return 1;
}

struct GameStateView {
    unsigned char unmodeled_0000[0x9E0];
    int mapProbeTimeout;
    unsigned char unmodeled_09E4[0xAE4 - 0x9E4];
    int slotValue[100];
};

void *vmKeyedEntryGet(int thread, int key);
int func_0020F790(void *dest);

/* Script native: stores func_0020F790 of the keyed entry (arg 2) at gameState->slotValue[arg 1] (index below 100). */
int func_001C87B0(int thread) {
    struct GameStateView *gameState;
    unsigned int slotIndex;
    int slotValue;

    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotIndex = vmArgGet(thread, 1);
    slotValue = func_0020F790(vmKeyedEntryGet(thread, vmArgGet(thread, 2)));
    if (slotIndex >= 100) {
        return 0;
    }
    gameState = stateBlockGet();
    gameState->slotValue[slotIndex] = slotValue;
    return 1;
}

void xrgObjectSetPartsFlags(s32 object, s32 partIndex, s32 flags);
void xrgObjectClearPartsFlags(s32 object, s32 partIndex, s32 flags);
void xrgObjectResetPartsFlags(s32 object, s32 partIndex);

/* Script native: set (mode 0), clear (mode 1) or reset (mode 2) the parts flags of partsSelf in the game state block. */
int sbNativeSetPartsFlags(s32 thread) {
    GameStateParts *gameState = stateBlockGet();
    s32 partIndex;
    s32 mode;
    if (gameState == 0) {
        return 0;
    }
    if (VM_PLUGIN_CNT_GET(thread) < 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    partIndex = vmArgGet(thread, 1);
    mode = vmArgGet(thread, 2);
    if (partIndex != 2 && VM_PLUGIN_CNT_GET(thread) < 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    switch (mode) {
    case 0:
        xrgObjectSetPartsFlags(gameState->partsSelf, partIndex, vmArgGet(thread, 3));
        break;
    case 1:
        xrgObjectClearPartsFlags(gameState->partsSelf, partIndex, vmArgGet(thread, 3));
        break;
    case 2:
        xrgObjectResetPartsFlags(gameState->partsSelf, partIndex);
        break;
    default:
        return 0;
    }
    return 1;
}

s32 xrgObjectSetPartsTime(s32 self, s32 partNumber, f32 fraction);

/* Script native: sets the time fraction (arg 2, percent) of part arg 1 of partsSelf. */
s32 sbNativeSetPartsFirstFraction(s32 thread) {
    GameStateParts *gameState;
    s32 partNumber;
    s32 percent;

    gameState = stateBlockGet();
    if (gameState != 0) {
        if (VM_PLUGIN_CNT_GET(thread) != 2) {
            sbSlotJumpRequest(thread, 2);
            return 0;
        }
        partNumber = vmArgGet(thread, 1);
        percent = vmArgGet(thread, 2);
        xrgObjectSetPartsTime(gameState->partsSelf, partNumber, percent / 100.0f);
        return 1;
    }
    return 0;
}

s32 xrgObjectSetPartsWeight(s32 self, s32 partNumber, f32 fraction);

/* Script native: sets the weight fraction (arg 2, percent) of part arg 1 of partsSelf. */
s32 sbNativeSetPartsSecondFraction(s32 thread) {
    GameStateParts *gameState;
    s32 partNumber;
    s32 percent;

    gameState = stateBlockGet();
    if (gameState != 0) {
        if (VM_PLUGIN_CNT_GET(thread) != 2) {
            sbSlotJumpRequest(thread, 2);
            return 0;
        }
        partNumber = vmArgGet(thread, 1);
        percent = vmArgGet(thread, 2);
        xrgObjectSetPartsWeight(gameState->partsSelf, partNumber, percent / 100.0f);
        return 1;
    }
    return 0;
}

static inline void setFlagWord(GameStateParts *block, s32 offset, s32 mask) {
    *(s32 *) ((char *) block->partsSelf + offset) |= mask;
}

static inline void clearFlagWord(GameStateParts *block, s32 offset, s32 mask) {
    *(s32 *) ((char *) block->partsSelf + offset) &= ~mask;
}

/* Script native: arg 1 is a bit index into the 32-bit flag words at partsSelf + 0x2F0; arg 2 nonzero clears the bit, zero sets it. */
s32 func_001C8AF8(s32 thread) {
    GameStateParts *gameState;
    s32 bitIndex;
    s32 clearBit;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    bitIndex = vmArgGet(thread, 1);
    clearBit = vmArgGet(thread, 2);
    gameState = stateBlockGet();
    if (clearBit != 0) {
        clearFlagWord(gameState, (bitIndex >> 5) * 4 + 0x2F0, 1 << (bitIndex & 31));
    } else {
        setFlagWord(gameState, (bitIndex >> 5) * 4 + 0x2F0, 1 << (bitIndex & 31));
    }
    return 1;
}

typedef struct GameStatePartsTable {
    unsigned char pad_0000[0x830];
    s32 partsSelf;
    f32 partsScaleA;
    f32 partsScaleB;
    f32 partsScaleC;
    f32 partsScaleD;
    unsigned char unmodeled_0844[0x45C];
    unsigned char partsFlags[0x100];
    f32 partsScales[0x100];
} GameStatePartsTable;

/* Script native: index (arg 1, below 0x101) sets partsScales[index] from arg 3 (hundredths, unless its tag is zero) and bit 0 of partsFlags[index] from arg 2. */
s32 func_001C8BC0(s32 thread) {
    GameStatePartsTable *gameState;
    s32 index;
    s32 bit0Value;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 4) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    index = vmArgGet(thread, 1);
    bit0Value = vmArgGet(thread, 2);
    gameState = stateBlockGet();
    if ((unsigned int) index >= 0x101) {
        return 0;
    }
    if (vmArgTagIsZero(thread, 3) == 0) {
        gameState->partsScales[index] = vmArgGet(thread, 3) * 0.01f;
    }
    if (bit0Value != 0) {
        gameState->partsFlags[index] |= 1;
    } else {
        gameState->partsFlags[index] &= 0xFE;
    }
    return 1;
}

/* Script native: sets partsScaleC (arg 1, hundredths, default 3.0) and partsScaleD (arg 2, hundredths, default 10000.0). */
s32 func_001C8CD0(s32 thread) {
    GameStatePartsTable *gameState;
    f32 scaleC;
    f32 scaleD;
    s32 scaleDTagZero;

    gameState = stateBlockGet();
    if (VM_PLUGIN_CNT_GET(thread) > 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgTagIsZero(thread, 1) == 0) {
        scaleC = vmArgGet(thread, 1) * 0.01f;
    } else {
        scaleC = 3.0f;
    }
    scaleDTagZero = vmArgTagIsZero(thread, 2);
    if (scaleDTagZero == 0) {
        scaleD = vmArgGet(thread, 2) * 0.01f;
    } else {
        scaleD = 10000.0f;
    }
    gameState->partsScaleC = scaleC;
    gameState->partsScaleD = scaleD;
    return 1;
}

/* Script native: returns 1. */
s32 sbNativeReturnOne(void) {
    return 1;
}

float srsAtan2(float x, float y);

/* Script native: returns srsAtan2(arg 1, arg 2) (both hundredths) scaled by 10000, truncated to s32. */
s32 func_001C8DC0(s32 thread) {
    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    return (s32) (srsAtan2(vmArgGet(thread, 1) * 0.01f, vmArgGet(thread, 2) * 0.01f) * 10000.0f);
}

static inline f32 VuCosine(f32 angle) {
    f32 result;

    __asm__ __volatile__("mfc1 $8, %1\n\t"
                         "qmtc2.ni $8, $vf4\n\t"
                         "vcallms 0xE8\n\t"
                         "qmfc2.i $8, $vf1\n\t"
                         "mtc1 $8, %0"
                         : "=f"(result)
                         : "f"(angle)
                         : "$8");
    return result;
}

/* Script native: cosine of an angle given in hundredths of a degree, scaled by 10000 (VU0 microprogram 0xE8). */
s32 sbNativeCosine(s32 thread) {
    s32 angleDegrees;
    f32 cosine;
    f32 cosineSlot[4]; /* kept: the array store decides the emitted order */

    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    angleDegrees = vmArgGet(thread, 1);
    cosine = VuCosine(angleDegrees * 0.01f * 0.017453292f);
    cosineSlot[0] = cosine;
    return (s32) (cosineSlot[0] * 10000.0f);
}

static inline f32 VuSine(f32 angle) {
    f32 result;

    __asm__ __volatile__("mfc1 $8, %1\n\t"
                         "qmtc2.ni $8, $vf4\n\t"
                         "vcallms 0x20\n\t"
                         "qmfc2.i $8, $vf1\n\t"
                         "mtc1 $8, %0"
                         : "=f"(result)
                         : "f"(angle)
                         : "$8");
    return result;
}

/* Script native: sine of an angle given in hundredths of a degree, scaled by 10000 (VU0 microprogram 0x20). */
s32 sbNativeSine(s32 thread) {
    s32 angleDegrees;
    f32 sine;
    f32 sineSlot[4]; /* kept: the array store decides the emitted order */

    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    angleDegrees = vmArgGet(thread, 1);
    sine = VuSine(angleDegrees * 0.01f * 0.017453292f);
    sineSlot[0] = sine;
    return (s32) (sineSlot[0] * 10000.0f);
}

s32 MMathMakeRandomInt();

/* Script native: random integer from arg 1 up to arg 2 (inclusive). */
s32 sbNativeRandomRange(s32 thread) {
    s32 spanLength;
    s32 randomLow;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    randomLow = vmArgGet(thread, 1);
    spanLength = vmArgGet(thread, 2) - randomLow;
    return randomLow + (MMathMakeRandomInt() % (spanLength + 1));
}

float MMathCalcLengthXZ(const Vector4 *vector);

void *RgDispModelFindByKey(s32 dispModelIndex);

/* Script native: set the size extents of a slot's model; mode 0 uses X/Y, mode 1 derives the radius from the XZ length. */
int sbNativeSetModelSize(s32 thread) {
    SlotObject *slotObject;
    SlotActor *model;
    s32 mode;
    f32 extentX;
    f32 extentY;
    f32 extentZ;

    if (VM_PLUGIN_CNT_GET(thread) < 1 || VM_PLUGIN_CNT_GET(thread) > 5) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    model = slotObject->model;
    mode = vmArgTagIsZero(thread, 2) ? model->mode : vmArgGet(thread, 2);
    extentX = vmArgTagIsZero(thread, 3) ? model->extents.vector.x : vmArgGet(thread, 3) * 0.01f;
    extentY = vmArgTagIsZero(thread, 4) ? model->extents.vector.y : vmArgGet(thread, 4) * 0.01f;
    extentZ = vmArgTagIsZero(thread, 5) ? model->extents.vector.z : vmArgGet(thread, 5) * 0.01f;
    switch (mode) {
    case 0:
        model->mode = mode;
        model->extents.vector.x = extentX;
        model->extents.vector.y = extentY;
        model->extents.vector.w = extentX;
        model->field6CC = extentX;
        break;
    case 1:
        model->mode = mode;
        model->extents.vector.x = extentX;
        model->extents.vector.y = extentY;
        model->extents.vector.z = extentZ;
        model->field6CC = model->extents.vector.w = MMathCalcLengthXZ((Vector4 *) &model->extents) * 0.5f;
        break;
    default:
        return 0;
    }
    return 1;
}

/* Script native: move a slot's actor by the given offset (hundredths, default the stored vector60) and store the negated offset in vector60. */
s32 func_001C91F0(s32 thread) {
    SlotObject *slotObject;
    SlotActor *actor;
    f32 valueX;
    f32 valueY;
    f32 valueZ;

    if (VM_PLUGIN_CNT_GET(thread) < 1 || VM_PLUGIN_CNT_GET(thread) >= 5) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    actor = slotObject->model;
    if (actor == 0) {
        return 0;
    }
    if (vmArgTagIsZero(thread, 2)) {
        valueX = actor->vector60.x;
    } else {
        valueX = vmArgGet(thread, 2) * 0.01f;
    }
    if (vmArgTagIsZero(thread, 3)) {
        valueY = actor->vector60.y;
    } else {
        valueY = vmArgGet(thread, 3) * 0.01f;
    }
    if (vmArgTagIsZero(thread, 4)) {
        valueZ = actor->vector60.z;
    } else {
        valueZ = vmArgGet(thread, 4) * 0.01f;
    }
    actor->position.x = actor->position.x + valueX;
    actor->position.y = actor->position.y + valueY;
    actor->position.z = actor->position.z + valueZ;
    actor->vector60.x = -valueX;
    actor->vector60.y = -valueY;
    actor->vector60.z = -valueZ;
    return 1;
}

void XenoActorEnableChannel(void *actor);
void XenoActorCheckMode(void *actor, s32 mode);

/* Script native: set or clear the flag mask on a slot; mask bit 0x400000 also resets the effect state and the actor channel. */
s32 func_001C9370(s32 thread) {
    SlotObject *object;
    SlotState *state;
    s32 partIndex;
    u32 mask;
    s32 enable;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 4) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    partIndex = vmArgGet(thread, 1);
    mask = vmArgGet(thread, 2);
    enable = vmArgTagIsZero(thread, 3) ? 1 : vmArgGet(thread, 3);
    object = RgDispModelFindByKey(partIndex);
    if (object == 0) {
        return 0;
    }
    object->flags = enable ? (object->flags | mask) : (object->flags & ~mask);
    if (mask & 0x400000) {
        if (object->kind == 1) {
            state = &object->state;
            if (enable == 0) {
                if (state->timer <= 0.0f) {
                    object->flags0C &= ~0x10000;
                } else {
                    object->flags0C |= 0x10000;
                }
            }
            state->slotA = 0;
            state->slotB = 0;
        }
        if (object->model != 0) {
            XenoActorEnableChannel(object->model);
            XenoActorCheckMode(object->model, 0);
        }
    }
    return 1;
}

/* Script native: set or clear the bit mask in a slot's statusFlags. */
s32 func_001C94F0(s32 thread) {
    s32 modelIndex;
    s32 bitMask;
    s32 setBits;
    SlotObject *slotObject;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 4) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    modelIndex = vmArgGet(thread, 1);
    bitMask = vmArgGet(thread, 2);
    if (!vmArgTagIsZero(thread, 3)) {
        setBits = vmArgGet(thread, 3);
    } else {
        setBits = 1;
    }
    slotObject = RgDispModelFindByKey(modelIndex);
    if (0 == slotObject) {
        return 0;
    }
    slotObject->statusFlags = setBits ? (slotObject->statusFlags | bitMask) : (slotObject->statusFlags & ~bitMask);
    return 1;
}

void bindRequestDispatch(SlotObject *slotObject, s32 mode);

/* Script native: apply bindRequestDispatch with a mode to one slot, or to every slot of kind 0x10017 when the index is negative. */
s32 func_001C95D0(s32 thread) {
    s32 partIndex;
    s32 mode;
    SlotObject *slotObject;

    if (VM_PLUGIN_CNT_GET(thread) < 1 || VM_PLUGIN_CNT_GET(thread) >= 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    partIndex = vmArgGet(thread, 1);
    if (vmArgTagIsZero(thread, 2) == 0) {
        mode = vmArgGet(thread, 2);
    } else {
        mode = -1;
    }
    if (partIndex < 0) {
        slotObject = 0;
        for (;;) {
            slotObject = RgDispModelNextByKind(0x10017, slotObject);
            if (slotObject == 0) {
                break;
            }
            bindRequestDispatch(slotObject, mode);
        }
        return 1;
    }
    slotObject = RgDispModelFindByKey(partIndex);
    if (slotObject == 0) {
        return 0;
    }
    bindRequestDispatch(slotObject, mode);
    return 1;
}

/* Script native: store a value in shortA of one slot, or of every kind-7 slot when the index is negative. */
s32 func_001C96C8(s32 thread) {
    s32 dispModelIndex;
    SlotObject *slotObject;
    s32 value;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    value = vmArgGet(thread, 2);
    if (dispModelIndex < 0) {
        slotObject = 0;
        while ((slotObject = RgDispModelNextByKind(7, slotObject)) != 0) {
            slotObject->shortA = value;
        }
        return 1;
    }
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject == 0) {
        return 0;
    }
    slotObject->shortA = value;
    return 1;
}

void func_001C06F0(SlotObject *slotObject, SlotObject *targetObject);

/* Script native: link a slot to another slot (func_001C06F0), or unlink it when the value is negative. */
s32 func_001C9778(s32 thread) {
    s32 dispModelIndex;
    s32 value;
    SlotObject *slotObject;
    SlotObject *targetObject;

    if (VM_PLUGIN_CNT_GET(thread) <= 0 || VM_PLUGIN_CNT_GET(thread) >= 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    value = vmArgGet(thread, 2);
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject == 0) {
        return 0;
    }
    if (value < 0) {
        func_001C06F0(slotObject, 0);
    } else {
        targetObject = RgDispModelFindByKey(value);
        if (targetObject == 0) {
            return 0;
        }
        func_001C06F0(slotObject, targetObject);
    }
    return 1;
}

void slotChildStateSelect(SlotObject *slotObject, s32 mode);

/* Script native: call slotChildStateSelect on a slot with a mode. */
s32 func_001C9850(s32 thread) {
    SlotObject *slotObject;
    s32 dispModelIndex;
    s32 mode;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    mode = vmArgGet(thread, 2);
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject != 0) {
        slotChildStateSelect(slotObject, mode);
        return 1;
    }
    return 0;
}

s32 func_001BF7C8(f32 value, f32 scale);
s32 angleWrapTurns(f32 angle, f32 scale);

/* Script native: return position, angle or scale component (selected by field 0-8) of a slot object, scaled by 100. */
int sbNativeGetObjectTransform(s32 thread) {
    SlotObject *slotObject;
    s32 slotKind;
    s32 result;
    s32 field;
    if (VM_PLUGIN_CNT_GET(thread) != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotKind = vmArgGet(thread, 1);
    field = vmArgGet(thread, 2);
    slotObject = RgDispModelFindByKey(slotKind);
    if (slotObject == 0) {
        return 0;
    }
    switch (field) {
    case 0:
        result = func_001BF7C8(slotObject->pos.x, 100.0f);
        break;
    case 1:
        result = func_001BF7C8(slotObject->pos.y, 100.0f);
        break;
    case 2:
        result = func_001BF7C8(slotObject->pos.z, 100.0f);
        break;
    case 3:
        result = angleWrapTurns(slotObject->ang.x, 100.0f);
        break;
    case 4:
        result = angleWrapTurns(slotObject->ang.y, 100.0f);
        break;
    case 5:
        result = angleWrapTurns(slotObject->ang.z, 100.0f);
        break;
    case 6:
        result = func_001BF7C8(slotObject->scale.x, 100.0f);
        break;
    case 7:
        result = func_001BF7C8(slotObject->scale.y, 100.0f);
        break;
    case 8:
        result = func_001BF7C8(slotObject->scale.z, 100.0f);
        break;
    default:
        return 0;
    }
    return result;
}

void func_001C1A58(SlotObject *slotObject, s32 value);
void func_001C1A88(SlotObject *slotObject, s32 value);
void func_001C1AB8(SlotObject *slotObject, s32 value);
void emitterScaleSelect(SlotObject *slotObject, s32 value);

/* Script native: set the slot property chosen by argument 2 (halfScale, flags, effect scale, handlers, state bits...) from argument 3. */
s32 func_001C99F8(s32 thread) {
    SlotObject *slotObject;
    SlotActor *sub;
    s32 selector;
    s32 index;
    f32 value;

    if (VM_PLUGIN_CNT_GET(thread) != 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    index = vmArgGet(thread, 1);
    selector = vmArgGet(thread, 2);
    slotObject = RgDispModelFindByKey(index);
    if (slotObject == 0) {
        return 0;
    }
    sub = slotObject->model;
    switch (selector) {
    case 0:
        value = vmArgGet(thread, 3) * 0.01f;
        slotObject->halfScale = value;
        break;
    case 1: {
        s32 bits = vmArgGet(thread, 3);
        if ((bits & 0xEEEEEEEE) != 0) {
            return 0;
        }
        slotObject->field13C = bits;
    } break;
    case 2:
        if (sub != 0) {
            value = vmArgGet(thread, 3) * 0.01f;
            if (value < 0.0f) {
                value = 0.0f;
            }
            if (value > 1.0f) {
                value = 1.0f;
            }
            slotObject->model->fogDensity = value;
        }
        break;
    case 3:
        func_001C1A58(slotObject, vmArgGet(thread, 3));
        break;
    case 4:
        func_001C1A88(slotObject, func_0020F790(vmKeyedEntryGet(thread, vmArgGet(thread, 3))));
        break;
    case 5:
        func_001C1AB8(slotObject, vmArgGet(thread, 3));
        break;
    case 6:
        if (sub != 0) {
            if (vmArgGet(thread, 3) != 0) {
                sub->flags90 &= 0x7FFFFFFF;
            } else {
                sub->flags90 |= 0x80000000;
            }
        }
        break;
    case 7:
        if (sub != 0) {
            if (vmArgGet(thread, 3) != 0) {
                sub->flags90 |= 0x20000;
            } else {
                sub->flags90 &= 0xFFFDFFFF;
            }
        }
        break;
    case 8:
        value = vmArgGet(thread, 3) * 0.01f;
        slotObject->field138 = value;
        break;
    case 9:
        emitterScaleSelect(slotObject, vmArgGet(thread, 3));
        break;
    case 10:
        if (slotObject->kind != 4) {
            return 0;
        }
        slotObject->state.value.handle = vmArgGet(thread, 3);
        break;
    case 11: {
        s32 step = vmArgGet(thread, 3);
        if ((unsigned int) (step + 1) >= 5) {
            return 0;
        }
        slotObject->shortB = step;
    } break;
    case 12:
        if (vmArgGet(thread, 3) != 0) {
            slotObject->statusFlags &= 0xFFEFFFFF;
        } else {
            slotObject->statusFlags |= 0x100000;
        }
        break;
    default:
        return 0;
    }
    return 1;
}

void func_001EAB00(void *target, AlignedVector4 *vector);

typedef Vector4 EffectVector;

/* Kept as its own helper: merging it with Vector4ZeroSqc2 renumbers the compiler's labels. */
static inline void EffectVectorZeroSqc2(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

/* Script native: set the vector of a slot object's effect (kind 0x4000) from three arguments in hundredths. */
int sbNativeSetEffectVector(s32 thread) {
    EffectVector vector;
    SlotObject *slotObject;
    s32 xArg;
    s32 yArg;
    s32 zArg;
    SlotState *state;
    if (VM_PLUGIN_CNT_GET(thread) != 4) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    xArg = vmArgGet(thread, 2);
    yArg = vmArgGet(thread, 3);
    zArg = vmArgGet(thread, 4);
    state = &slotObject->state;
    if (slotObject->kind != 0x4000) {
        return 0;
    }
    if (state->value.pointer == 0) {
        return 0;
    }
    EffectVectorZeroSqc2(&vector);
    vector.x = xArg * 0.01f;
    vector.y = yArg * 0.01f;
    vector.z = zArg * 0.01f;
    vector.w = slotObject->vectorW;
    func_001EAB00(state->value.pointer, &vector);
    return 1;
}

void RgDispModelSetLocal(SlotObject *slotObject);
void objectSetDefaults(SlotObject *object);
void actorInit(SlotObject *actor);

/* Script native: free a slot through RgDispModelSetLocal, first resetting its linked object by kind when mode is 1. */
s32 func_001C9E50(s32 thread) {
    SlotObject *slotObject;
    SlotObject *linkedObject;
    s32 mode;

    if (VM_PLUGIN_CNT_GET(thread) <= 0 || VM_PLUGIN_CNT_GET(thread) >= 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    if (vmArgTagIsZero(thread, 2) != 0) {
        mode = 0;
    } else {
        mode = vmArgGet(thread, 2);
    }
    if (mode == 1 && slotObject->kind == 16 && slotObject->linkedSlot != 0) {
        linkedObject = RgDispModelFindByKey(slotObject->linkedSlot);
        if (linkedObject != 0) {
            switch (linkedObject->kind) {
            case 512:
                objectSetDefaults(linkedObject);
                break;
            case 1024:
                actorInit(linkedObject);
                break;
            }
        }
    }
    RgDispModelSetLocal(slotObject);
    return 1;
}

void stepMoveStart(SlotObject *slotObject, s32 index, Vector4 *offset);

/* Script native: build an offset vector from arguments (defaults: the slot position) and pass it to stepMoveStart. */
s32 func_001C9F68(s32 thread) {
    AlignedVector4 offset;
    SlotObject *slotObject;
    s32 index;
    f32 value;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    Vu0SetOrigin(&offset);
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    index = vmArgGet(thread, 2);
    if (vmArgTagIsZero(thread, 3) != 0) {
        value = slotObject->pos.x;
    } else {
        value = vmArgGet(thread, 3) * 0.01f;
    }
    offset.x = value;
    if (vmArgTagIsZero(thread, 4) != 0) {
        value = slotObject->pos.y;
    } else {
        value = vmArgGet(thread, 4) * 0.01f;
    }
    offset.y = value;
    if (vmArgTagIsZero(thread, 5) != 0) {
        value = slotObject->pos.z;
    } else {
        value = vmArgGet(thread, 5) * 0.01f;
    }
    offset.z = value;
    stepMoveStart(slotObject, index, &offset);
    return 1;
}

/* Script native: add the argument offsets to the slot position and pass the result to stepMoveStart. */
s32 func_001CA0D8(s32 thread) {
    AlignedVector4 offset;
    SlotObject *slotObject;
    s32 index;
    f32 value;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    Vu0SetOrigin(&offset);
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    index = vmArgGet(thread, 2);
    if (vmArgTagIsZero(thread, 3) == 0) {
        value = vmArgGet(thread, 3) * 0.01f;
    } else {
        value = 0.0f;
    }
    offset.x = value;
    if (vmArgTagIsZero(thread, 4) == 0) {
        value = vmArgGet(thread, 4) * 0.01f;
    } else {
        value = 0.0f;
    }
    offset.y = value;
    if (vmArgTagIsZero(thread, 5) == 0) {
        value = vmArgGet(thread, 5) * 0.01f;
    } else {
        value = 0.0f;
    }
    offset.z = value;
    Vu0SefAddVectorXYZ(&offset, &slotObject->pos, &offset);
    stepMoveStart(slotObject, index, &offset);
    return 1;
}

void stepRotationStart(SlotObject *slotObject, s32 index, Vector4 *offset);

/* Script native: build an angle vector (degrees to radians, defaults: the slot ang) and pass it to stepRotationStart. */
s32 func_001CA260(s32 thread) {
    AlignedVector4 offset;
    SlotObject *slotObject;
    s32 index;
    f32 value;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    Vu0SetOrigin(&offset);
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    index = vmArgGet(thread, 2);
    if (vmArgTagIsZero(thread, 3) != 0) {
        value = slotObject->ang.x;
    } else {
        value = vmArgGet(thread, 3) * 0.01f * 0.0174532925f;
    }
    offset.x = value;
    if (vmArgTagIsZero(thread, 4) != 0) {
        value = slotObject->ang.y;
    } else {
        value = vmArgGet(thread, 4) * 0.01f * 0.0174532925f;
    }
    offset.y = value;
    if (vmArgTagIsZero(thread, 5) != 0) {
        value = slotObject->ang.z;
    } else {
        value = vmArgGet(thread, 5) * 0.01f * 0.0174532925f;
    }
    offset.z = value;
    stepRotationStart(slotObject, index, &offset);
    return 1;
}

void func_001C1058(SlotObject *slotObject, s32 index, Vector4 *offset);

/* Script native: build a scale vector (w = 1, defaults: the slot scale) and pass it to func_001C1058. */
s32 func_001CA3E8(s32 thread) {
    AlignedVector4 offset;
    SlotObject *slotObject;
    s32 index;
    f32 value;
    f32 zValue;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 6) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    Vu0SetOrigin(&offset);
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    index = vmArgGet(thread, 2);
    if (vmArgTagIsZero(thread, 3) != 0) {
        value = slotObject->scale.x;
    } else {
        value = vmArgGet(thread, 3) * 0.01f;
    }
    offset.x = value;
    if (vmArgTagIsZero(thread, 4) != 0) {
        value = slotObject->scale.y;
    } else {
        value = vmArgGet(thread, 4) * 0.01f;
    }
    offset.y = value;
    if (vmArgTagIsZero(thread, 5) != 0) {
        zValue = slotObject->scale.z;
    } else {
        zValue = vmArgGet(thread, 5) * 0.01f;
    }
    offset.z = zValue;
    offset.w = 1.0f;
    func_001C1058(slotObject, index, &offset);
    return 1;
}

extern float D_0054F57C;
void stepRatioStart(SlotObject *slotObject, s32 value, float scale);

/* Script native: call stepRatioStart with a slot, a value and argument 3 scaled by D_0054F57C. */
s32 func_001CA560(s32 thread) {
    SlotObject *slotObject;
    s32 value;

    if (VM_PLUGIN_CNT_GET() != 3) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    value = vmArgGet(thread, 2);
    stepRatioStart(slotObject, value, vmArgGet(thread, 3) * D_0054F57C);
    return 1;
}

void func_001E9500(s32 value, f32 x, f32 y);

/* Script native: call func_001E9500 with x and y on the effect handle of one kind-0x4000 slot, or of all of them when the index is negative. */
s32 func_001CA618(s32 thread) {
    s32 index;
    f32 x;
    f32 y;
    SlotObject *slotObject;

    if (VM_PLUGIN_CNT_GET(thread) >= 4) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgTagIsZero(thread, 1) == 0) {
        index = vmArgGet(thread, 1);
    } else {
        index = -1;
    }
    x = vmArgGet(thread, 2);
    if (vmArgTagIsZero(thread, 3) == 0) {
        y = vmArgGet(thread, 3);
    } else {
        y = 5.0f;
    }
    if (index < 0) {
        slotObject = 0;
        while ((slotObject = RgDispModelNextByKind(16384, slotObject)) != 0) {
            func_001E9500(slotObject->state.value.handle, x, y);
        }
        return 1;
    }
    slotObject = RgDispModelFindByKey(index);
    if (slotObject == 0) {
        return 0;
    }
    if (slotObject->kind != 16384) {
        return 0;
    }
    func_001E9500(slotObject->state.value.handle, x, y);
    return 1;
}

void objectSetLinkedFlag(s32 object, s32 flag);

/* Script native: call objectSetLinkedFlag on the effect handle of a kind-0x4000 slot. */
s32 func_001CA758(s32 thread) {
    SlotObject *slotObject;
    s32 dispModelIndex;
    s32 flag;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    flag = vmArgGet(thread, 2) != 0;
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject == 0) {
        return 0;
    }
    if (slotObject->kind != 16384) {
        return 0;
    }
    objectSetLinkedFlag(slotObject->state.value.handle, flag);
    return 1;
}

/* Script native: return 1 when the slot is of kind 2 and its state word, masked with 0x7FFF0000, equals 0x02000000. */
s32 func_001CA7F0(s32 thread) {
    SlotObject *slotObject;
    s32 dispModelIndex;

    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject == 0) {
        return 0;
    }
    if (slotObject->kind == 2 && (slotObject->state.unmodeled_00 & 0x7FFF0000) == 0x02000000) {
        return 1;
    }
    return 0;
}

/* Script native: set the vectorC0 components of a slot from the given arguments (hundredths). */
s32 func_001CA880(s32 thread) {
    SlotObject *slotObject;

    if (VM_PLUGIN_CNT_GET(thread) <= 0 || VM_PLUGIN_CNT_GET(thread) >= 5) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    if (vmArgTagIsZero(thread, 2) == 0) {
        slotObject->vectorC0[0] = vmArgGet(thread, 2) * 0.01f;
    }
    if (vmArgTagIsZero(thread, 3) == 0) {
        slotObject->vectorC0[1] = vmArgGet(thread, 3) * 0.01f;
    }
    if (vmArgTagIsZero(thread, 4) == 0) {
        slotObject->vectorC0[2] = vmArgGet(thread, 4) * 0.01f;
    }
    return 1;
}

s32 func_001C1320(SlotObject *slotObject, s32 mode);

/* Script native: for a slot, clear flag 0x800000, set 0x800 and call func_001C1320 when argument 2 is zero. */
s32 sbNativeEntrySetFlag(s32 thread) {
    SlotObject *slotObject;
    s32 dispModelIndex;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject != 0) {
        if (vmArgGet(thread, 2) != 0) {
            return 0;
        }
        slotObject->flags = (slotObject->flags & 0xFF7FFFFF) | 0x800;
        func_001C1320(slotObject, 1);
        return 1;
    }
    return 0;
}

s32 slotEffectStartOnce(SlotObject *slotObject, s32 mode);

/* Script native: return slotEffectStartOnce(slot, 1) != 0 for a slot. */
s32 func_001CAA48(s32 thread) {
    SlotObject *slotObject;
    s32 dispModelIndex;

    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    slotObject = RgDispModelFindByKey(dispModelIndex);
    if (slotObject != 0) {
        return slotEffectStartOnce(slotObject, 1) != 0;
    }
    return 0;
}

void SpawnSlotSetPosition(SlotObject *linkedObject, Vector4 *dest);
void dispModelDetachEntry(SlotObject *slotObject);

/* Script native: link a slot to another slot id (linkedSlot) and copy its vector with SpawnSlotSetPosition. */
s32 func_001CAAC0(s32 thread) {
    SlotObject *slotObject;
    SlotObject *linkedObject;
    s32 value;
    s32 mode;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 4) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    slotObject = RgDispModelFindByKey(vmArgGet(thread, 1));
    if (slotObject == 0) {
        return 0;
    }
    value = vmArgGet(thread, 2);
    if (value == 0) {
        slotObject->linkedSlot = 0;
        return 1;
    }
    linkedObject = RgDispModelFindByKey(value);
    if (linkedObject == 0) {
        return 0;
    }
    slotObject->linkedSlot = value;
    SpawnSlotSetPosition(linkedObject, &slotObject->pos);
    if (linkedObject->kind == 4) {
        linkedObject->flags |= 8;
        slotChildStateSelect(linkedObject, 0);
    }
    if (vmArgTagIsZero(thread, 3) != 0) {
        mode = 0;
    } else {
        mode = vmArgGet(thread, 3);
    }
    if (mode != 0) {
        if (mode != 1) {
            return 0;
        }
        dispModelDetachEntry(slotObject);
    }
    return 1;
}

s32 func_001CAFD8(s32 thread, s32 value);
s32 getClassFromSignature(s32 thread);
s32 nativeInvokePairVia1C88(s32 thread, s32 value);
s32 nativeInvokePairVia3A40(s32 thread, s32 value);
s32 nativeInvokePairVia3FD0(s32 thread, s32 value);
s32 nativeInvokePairVia4410(s32 thread, s32 value);
s32 nativeInvokePairVia6E10(s32 thread, s32 value);
s32 nativeInvokeTripleLookupVia2670(s32 thread, void *slotObject, s32 value);
s32 nativeInvokeTripleSwapVia2F78(s32 thread, s32 value, s32 second);
s32 nativeInvokeTripleSwapVia7100(s32 thread, s32 value, s32 second);
s32 nativeInvokeTripleVia27C0(s32 thread, s32 value, s32 second);
s32 nativeInvokeTripleVia2990(s32 thread, s32 value);
s32 nativeInvokeTripleVia6830(s32 thread, s32 value, s32 second);
s32 rgDispModelLinkPairAt(s32 index);
s32 vmArgTagGet(s32 thread, s32 index);

/* Script native: dispatch on the selector mask of argument 2 to the nativeInvoke* helpers with the optional arguments 3 and 4. */
s32 func_001CAC00(s32 thread) {
    s32 index;
    s32 selector;
    s32 arg3Value;
    void *arg3Entry;
    s32 arg4Value;
    s32 result;

    if (VM_PLUGIN_CNT_GET(thread) < 2 || VM_PLUGIN_CNT_GET(thread) >= 5) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    index = vmArgGet(thread, 1);
    selector = vmArgGet(thread, 2);
    arg3Value = 0;
    arg3Entry = 0;
    arg4Value = -1;
    if (vmArgTagIsZero(thread, 3) == 0) {
        if ((vmArgTagGet(thread, 3) & 0xFF) == 2) {
            arg3Entry = vmKeyedEntryGet(thread, vmArgGet(thread, 3));
        } else {
            arg3Value = vmArgGet(thread, 3);
        }
    }
    if (vmArgTagIsZero(thread, 4) == 0) {
        if ((vmArgTagGet(thread, 4) & 0xFF) == 2) {
            vmKeyedEntryGet(thread, vmArgGet(thread, 4));
        } else {
            arg4Value = vmArgGet(thread, 4);
        }
    }
    switch (selector) {
    case 0x2:
        result = nativeInvokePairVia3FD0(index, arg3Value);
        break;
    case 0x4:
        result = nativeInvokeTripleVia2990(index, arg3Value);
        break;
    case 0x8:
        result = func_001CAFD8(index, arg3Value);
        break;
    case 0x10:
        result = nativeInvokePairVia3A40(index, arg3Value);
        break;
    case 0x40:
        result = nativeInvokePairVia4410(index, arg3Value);
        break;
    case 0x80:
        result = getClassFromSignature(index);
        break;
    case 0x100:
        result = nativeInvokePairVia1C88(index, arg3Value);
        if (arg4Value >= 0) {
            nativeInvokePairVia1C88(index + 1, arg4Value);
            result = rgDispModelLinkPairAt(index);
        }
        break;
    case 0x200:
        result = nativeInvokeTripleSwapVia7100(index, arg3Value, arg4Value);
        break;
    case 0x400:
        result = nativeInvokeTripleSwapVia2F78(index, arg3Value, arg4Value);
        break;
    case 0x2000:
        result = nativeInvokeTripleVia27C0(index, arg3Value, arg4Value);
        break;
    case 0x4000:
        result = nativeInvokeTripleLookupVia2670(index, arg3Entry, arg4Value);
        break;
    case 0x8000:
        result = nativeInvokeTripleVia6830(index, arg3Value, arg4Value);
        break;
    case 0x20000:
        result = nativeInvokePairVia6E10(index, arg3Value);
        break;
    default:
        return 0;
    }
    return result;
}
