#include "common.h"
#include "shared.h"
#include "vu0_macro.h"
#include "quadword.h"

typedef int s32;

typedef signed char s8;

typedef short s16;

typedef float f32;

/* One entry name of the state block table (0x70 bytes each). */
typedef struct {
    u8 unmodeled_00[0x70];
} StateEntryName;

/* Header of the entry matrices: the matrices of the table start entryOffset bytes after it. */
typedef struct {
    u8 unmodeled_00[0x68];
    s32 entryOffset;                 /* +0x68 */
} StateTableHeader;

typedef struct {
    s32 unk_00;
    s32 mode;                        /* +0x04, 3 = no extra reset flag on the models */
} StateModeBlock;

/* The game state block returned by stateBlockGet. */
typedef struct StateBlock {
    unsigned char unk_000[0x310];
    StateModeBlock modeBlock;        /* +0x310 */
    unsigned char unk_318[0x834 - 0x318];
    f32 scale834;                    /* +0x834 */
    unsigned char unk_838[0x860 - 0x838];
    u8 lightSet[0x180];              /* +0x860 */
    unsigned char unk_9E0[0xc74 - 0x9e0];
    f32 modelAlpha;                  /* +0xC74 */
    unsigned char unk_C78[0xc94 - 0xc78];
    s32 entryCount;                  /* +0xC94 */
    StateEntryName *entryNames;      /* +0xC98 */
    StateTableHeader *table;         /* +0xC9C */
} StateBlock;

StateBlock *stateBlockGet(void *, s32);

/* A request to create a slot from an entry of the state block table. */
typedef struct {
    s32 resourceId;                  /* +0x00 */
    s32 index;                       /* +0x04 */
    s32 variant;                     /* +0x08 */
} TableSlotRequest;

typedef union {
    Vector4 v;
    s32 words[4];
} Vector4Words;

enum SpawnerKind {
    SPAWNER_KIND_NONE,
    SPAWNER_KIND_WIDTH = 0x7fffffff /* filler: only widens the enum */
};

enum SpawnerFlags {
    SPAWNER_FLAGS_NONE,
    SPAWNER_FLAGS_WIDTH = 0x7fffffff /* filler: only widens the enum */
};

/* Model object of a spawner slot (layout of the XenoActor made by func_0028D670). */
typedef struct {
    s32 unk_00;
    s32 shape;                       /* +0x04: 0 = cylinder (x radius, y height), 1 = box */
    u8 unk_08[8];
    union {
        Vector4 size;
        s32 words[4];
    } extents;                       /* +0x10 */
    Vector4Words position;           /* +0x20 */
    Vector4 unk_30;
    Vector4 angle;                   /* +0x40 */
    Vector4 rotation;                /* +0x50 */
    Vector4 unk_60;
    Vector4 unk_70;
    Vector4 scale;                   /* +0x80 */
    s32 drawFlags;                   /* +0x90 */
    f32 alpha;                       /* +0x94 */
    f32 unk_98;
    s16 unk_9C;
    s16 unk_9E;
    f32 unk_A0;
    u8 unk_A4[0xf8 - 0xa4];
    f32 targetHeight;                /* +0xF8 */
    u8 unk_FC[0x12c - 0xfc];
    f32 unk_12C;
    u8 unk_130[0x140 - 0x130];
    u8 lightSet[0x180];              /* +0x140 */
    u8 unk_2C0[0x4ec - 0x2c0];
    s32 unk_4EC;
    u8 unk_4F0[0x510 - 0x4f0];
    void *unk_510;
    u8 unk_514[0x528 - 0x514];
    void *unk_528;
    u8 unk_52C[0x6a0 - 0x52c];
    u8 restartBlock[0x6aa - 0x6a0];  /* +0x6A0 */
    s16 playbackFlags;               /* +0x6AA */
    u8 unk_6AC[0x6b3 - 0x6ac];
    s8 playbackMode;                 /* +0x6B3 */
    u8 unk_6B4[0x6bc - 0x6b4];
    s32 playerHandle;                /* +0x6BC */
    u8 unk_6C0[0x6cc - 0x6c0];
    f32 playRate;                    /* +0x6CC */
} SpawnedModel;

/* Matrix of 4 rows used by the door/animation slots. */
typedef struct {
    Vector4 m[4];
} SlotMatrix;

/* Kind 2 and 4 slots: effect lifetime and handle pairs (same block as tu073). */
typedef struct {
    s32 unk_00;
    s32 unk_04;
    s32 effectLifetime;              /* +0x08, kind 2 */
    s32 effectHandle;                /* +0x0C, kind 2 */
    s32 unk_10;
    s32 unk_14;
    s32 auxEffectLifetime;           /* +0x18, kind 4 */
    s32 auxEffectHandle;             /* +0x1C, kind 4 */
} SpawnerState;

/* Kind 0x100 slots: one entry of the state block table (name, matrix) with its trigger and motion parameters. */
typedef struct {
    s32 tableIndex;                  /* +0x00 */
    char *name;                      /* +0x04 */
    SlotMatrix *matrix;              /* +0x08 */
    s32 unk_0C;
    SlotMatrix localMatrix;          /* +0x10 */
    s32 flags;                       /* +0x50 */
    enum SpawnerKind mode;           /* +0x54 */
    u8 unk_58[8];
    Vector4 extents;                 /* +0x60 */
    f32 triggerRadius;               /* +0x70 */
    f32 angle;                       /* +0x74 */
    s32 rate;                        /* +0x78 */
    s32 voiceA;                      /* +0x7C */
    s32 voiceB;                      /* +0x80 */
    s32 rateCopy;                    /* +0x84 */
    f32 ratio;                       /* +0x88 */
} SpawnerEntryState;

enum SpawnerResource {
    SPAWNER_RESOURCE_NONE,
    SPAWNER_RESOURCE_WIDTH = 0x7fffffff /* filler: only widens the enum */
};

/* Kind 4 slots: source id, variant and the three resource words found for it. */
typedef struct {
    s32 sourceId;                    /* +0x00 */
    s32 variantId;                   /* +0x04 */
    s32 playingHandle;               /* +0x08 */
    enum SpawnerResource resourceA;  /* +0x0C */
    enum SpawnerResource resourceB;  /* +0x10 */
    enum SpawnerResource resourceC;  /* +0x14 */
    s32 auxEffectLifetime;           /* +0x18 */
    s32 auxEffectHandle;             /* +0x1C */
} SpawnerChannelState;

/* Kind 0x2000 slots: one entry of the state block table, copied into the slot. */
typedef struct {
    s32 tableIndex;                  /* +0x00 */
    s32 variant;                     /* +0x04 */
    SlotMatrix *matrix;              /* +0x08 */
    s32 unk_0C;
    SlotMatrix localMatrix;          /* +0x10 */
} SpawnerTableState;

/* Slots of kind 16, 64 and 0x10000 keep their values here (arrays of four words per kind). */
typedef struct {
    s32 unk_00;
    s32 valueKind10000;              /* +0x04 */
    s32 valuesKind64[4];             /* +0x08 */
    u8 unk_18[0x50 - 0x18];
    s32 valuesKind16[4];             /* +0x50 */
} SpawnerValueState;

typedef union {
    SpawnerState effect;
    SpawnerEntryState entry;
    SpawnerChannelState channel;
    SpawnerTableState table;
    SpawnerValueState values;
} SpawnerSlotState;

#define SLOT_FLAG_LARGE 0x100           /* large variant (x5 scale, also x5 gravity step in slotFollowTargetHeight) */
#define SLOT_FLAG_MODE_ACTIVE 0x200     /* a mode value is set (func_001C1290) */
#define SLOT_FLAG_BOUNCING 0x1000       /* height follower is bouncing */
#define SLOT_FLAG_EFFECT_STARTED 0x2000 /* the effect was started once */
#define SLOT_FLAG_FOLLOW_HEIGHT 0x10000 /* the model follows the target height */
#define SLOT_FLAG_SNAP_HEIGHT 0x20000   /* the model jumps straight to the target height */

#define ENTRY_FLAG_OPEN 0x1             /* the slot is open */
#define ENTRY_FLAG_MOVING 0x8           /* the open or close motion is running */
#define ENTRY_FLAG_NORMAL 0x100         /* trigger mask of a normal entry */
#define ENTRY_FLAG_LARGE 0x200          /* trigger mask of a large entry */

/* Slot object of RgDispModelAlloc (stride 0x230); kind selects the view of the state at +0x150. */
typedef struct SpawnerSlot {
    enum SpawnerFlags flags;         /* +0x00 */
    s32 unk_04;
    unsigned int stateFlags;         /* +0x08 */
    unsigned int previousStateFlags; /* +0x0C */
    enum SpawnerKind kind;           /* +0x10 */
    s32 ownerModel;                  /* +0x14 */
    struct SpawnerSlot *partner;     /* +0x18 */
    void (*update)();                /* +0x1C */
    void (*release)();               /* +0x20 */
    SpawnedModel *model;             /* +0x24 */
    s32 requestId;                   /* +0x28 */
    s32 resourceId;                  /* +0x2C */
    s32 handle;                      /* +0x30 */
    u8 unk_34[0xc];
    Vector4 position;                /* +0x40 */
    Vector4 vector50;                /* +0x50 */
    Vector4 rotation;                /* +0x60 */
    Vector4 vector70;                /* +0x70 */
    u8 unk_80[0x12c - 0x80];
    f32 bounceVelocity;              /* +0x12C */
    f32 bounceDamping;               /* +0x130 */
    f32 halfScale;                   /* +0x134 */
    u8 unk_138[0x140 - 0x138];
    s16 modeValue;                   /* +0x140 */
    s16 modeEnabled;                 /* +0x142 */
    u8 unk_144[0x150 - 0x144];
    SpawnerSlotState state;          /* +0x150 */
} SpawnerSlot;

#define MODEL_DRAW_FLAG_MODE 0x10000     /* draw flag set while a mode value is active */

/* Sets (mode > 0) or clears the mode value of a slot and the matching draw flag of its model. */
void func_001C1290(SpawnerSlot *slot, s32 mode) {
    SpawnedModel *model = slot->model;
    if (model == 0) {
        return;
    }
    if (mode <= 0) {
        slot->flags &= ~SLOT_FLAG_MODE_ACTIVE;
        slot->modeValue = 0;
        slot->modeEnabled = 0;
        model->drawFlags &= ~MODEL_DRAW_FLAG_MODE;
        model->alpha = 1.0f;
    } else {
        slot->flags |= SLOT_FLAG_MODE_ACTIVE;
        slot->modeValue = mode;
        slot->modeEnabled = 1;
        model->drawFlags |= MODEL_DRAW_FLAG_MODE;
        model->alpha = 1.0f;
    }
}


/* Stores a value for the kind of the slot (16, 64 or 0x10000) and flags the slot as changed (0x20). */
void func_001C1320(SpawnerSlot *slot, s32 value) {
    switch (slot->kind) {
    case 16:
        slot->state.values.valuesKind16[3] = value;
        slot->flags |= 0x20;
        break;
    case 64:
        slot->state.values.valuesKind64[3] = value;
        slot->flags |= 0x20;
        break;
    case 0x10000:
        slot->state.values.valueKind10000 = value;
        slot->flags |= 0x20;
        break;
    default:
        return;
    }
}

extern s32 gDebugFlagWords[];

/* Moves the model of a spawner slot toward a target height (bouncing when it falls); returns 1 while it is still moving. */
s32 slotFollowTargetHeight(SpawnerSlot *slot, f32 targetY) {
    SpawnedModel *model;
    Vector4Words *position;
    f32 gravityScale;
    f32 gravityStep;

    model = slot->model;
    if (model == 0) {
        return 0;
    }
    position = &model->position;
    gravityScale = (slot->flags & SLOT_FLAG_LARGE) ? 5.0f : 1.0f;
    if (targetY == -1000.0f) {
        if (gDebugFlagWords[0] & 8) {
            model->targetHeight = model->position.v.y;
        }
        return 0;
    }
    model->targetHeight = targetY;
    if (!(slot->flags & SLOT_FLAG_FOLLOW_HEIGHT)) {
        return 0;
    }
    if (slot->flags & SLOT_FLAG_SNAP_HEIGHT) {
        position->v.y = targetY;
        return 1;
    }
    if (slot->flags & SLOT_FLAG_BOUNCING) {
        gravityStep = stateBlockGet(model, slot->flags)->scale834 * gravityScale;
        slot->bounceVelocity += gravityStep;
        position->v.y += slot->bounceVelocity;
        if (position->v.y < targetY) {
            position->v.y = targetY;
            slot->bounceVelocity = slot->bounceVelocity * -slot->bounceDamping;
            if (__builtin_fabsf(slot->bounceVelocity) < __builtin_fabsf(gravityStep * 0.5f)) {
                position->v.y = targetY;
                slot->flags &= ~SLOT_FLAG_BOUNCING;
            }
        }
        return 1;
    }
    switch (slot->kind) {
    case 1:
    case 2:
    case 4:
        if (gravityScale * -0.2f < targetY - position->v.y) {
            position->v.y = targetY;
            return 0;
        }
        break;
    default:
        if (position->v.y == targetY) {
            return 0;
        }
        break;
    }
    slot->bounceVelocity = 0;
    slot->flags |= SLOT_FLAG_BOUNCING;
    return 1;
}

/* Sets origin from the model position of the slot raised by half its Y extent; returns 0 without a slot or model. */
int slotModelTopPointGet(Vector4 *origin, SpawnerSlot *slot) {
    SpawnedModel *model;

    Vu0SetOrigin(origin);
    if (slot == 0) {
        return 0;
    }
    model = slot->model;
    if (model == 0) {
        return 0;
    }
    QuadwordCopyT0(origin, &model->position.v);
    origin->y += model->extents.size.y * 0.5f;
    return 1;
}

void func_0027B128(s32);
void func_0027B1C8(s32);

/* For the slot kinds with a child handle, selects one of two child states; returns 1 for those kinds. */
s32 slotChildStateSelect(SpawnerSlot *slot, s32 useAlternate) {
    unsigned int kind;

    if (slot == 0) {
        return 0;
    }
    kind = slot->kind;
    switch (kind) {
    case 2:
    case 4:
    case 16:
    case 64:
    case 256:
    case 512:
        if (useAlternate != 0) {
            func_0027B128(slot->handle);
        } else {
            func_0027B1C8(slot->handle);
        }
        return 1;
    default:
        return 0;
    }
}

void func_0030C130();
void *func_002A64D8(void *out, void *src);

static inline void vectorZeroQuad(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

typedef struct {
    Vector4 directionRow0;
    Vector4 directionRow1;
    Vector4 directionRow2;
    Vector4 unmodeled_30[4];
    Vector4 lightColor0;
    Vector4 lightColor1;
    Vector4 lightColor2;
    Vector4 unmodeled_A0[3];
    Vector4 ambientColor;
    u8 unmodeled_E0[0xa0];
} LightSet;

extern LightSet D_00581390;

typedef struct {
    s32 unk_00;
    f32 weights[3];
} LightBlend;

typedef struct {
    u8 unmodeled_00[0x20];
    Vector4 lightDirection2;         /* +0x20 */
    u8 unmodeled_30[0x60];
    Vector4 lightColor2;             /* +0x90 */
} ActorLight;

LightBlend *pointSlotFind(void *in);
ActorLight *func_00293DA0(SpawnedModel *model);

/* Kept as its own helper: merging it with QuadwordCopyT0 renumbers the compiler's labels. */
static inline void quadwordCopyVector(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(destination),"r"(source) : "$8","memory");
}

static inline void vectorScaleXyz(Vector4 *destination, Vector4 *source, f32 factor) {
    __asm__ __volatile__(
        "lqc2 $vf1,0(%1)\n mfc1 $8, %2\n qmtc2 $8, $vf2\n vmulx.xyz $vf1,$vf1,$vf2x\n sqc2 $vf1, 0(%0)"
        :
        : "r"(destination), "r"(source), "f"(factor)
        : "$8", "memory");
}

static inline void vectorAddXyz(Vector4 *destination, Vector4 *base, Vector4 *addend) {
    __asm__ __volatile__("lqc2 $vf1,0(%1)\n lqc2 $vf2,0(%2)\n vadd.xyz $vf1,$vf1,$vf2\n sqc2 $vf1,0(%0)"
                         :
                         : "r"(destination), "r"(base), "r"(addend)
                         : "memory");
}

static inline void vectorClampUnit(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__(
        "lqc2 $vf1,0(%1)\n vadd.w $vf2,$vf0,$vf0\n vmaxx.xyz $vf1,$vf1,$vf0x\n vminiw.xyz $vf1,$vf1,$vf2w\n sqc2 $vf1,0(%0)"
        :
        : "r"(destination), "r"(source)
        : "memory");
}

/* Copies the light set of the state block to D_00581390, blends in the lighting of the model position and the colors of its actor, then copies it back to the model. */
s32 slotLightSetBuild(SpawnerSlot *slot) {
    Vector4 modelPoint;
    Vector4 contribution;
    SpawnedModel *model;
    LightBlend *blend;
    ActorLight *light;
    StateBlock *stateBlock;
    if (slot == 0) {
        return 0;
    }
    if (slot->model == 0) {
        return 0;
    }
    stateBlock = ((StateBlock * (*) ()) stateBlockGet)();
    model = slot->model;
    func_0030C130(stateBlock->lightSet, &D_00581390, 0x180);
    blend = pointSlotFind(func_002A64D8(&modelPoint, &model->position));
    if (blend != 0) {
        vectorScaleXyz(&contribution, &modelPoint, blend->weights[0]);
        vectorAddXyz(&D_00581390.lightColor0, &D_00581390.lightColor0, &contribution);
        vectorClampUnit(&D_00581390.lightColor0, &D_00581390.lightColor0);
        vectorScaleXyz(&contribution, &modelPoint, blend->weights[1]);
        vectorAddXyz(&D_00581390.lightColor1, &D_00581390.lightColor1, &contribution);
        vectorClampUnit(&D_00581390.lightColor1, &D_00581390.lightColor1);
        vectorScaleXyz(&contribution, &modelPoint, blend->weights[2]);
        vectorAddXyz(&D_00581390.ambientColor, &D_00581390.ambientColor, &contribution);
        vectorClampUnit(&D_00581390.ambientColor, &D_00581390.ambientColor);
    }
    light = func_00293DA0(model);
    if (light == 0) {
        vectorZeroQuad(&D_00581390.lightColor2);
    } else {
        quadwordCopyVector(&D_00581390.directionRow2, &light->lightDirection2);
        quadwordCopyVector(&D_00581390.lightColor2, &light->lightColor2);
    }
    func_0030C130(&D_00581390, model->lightSet, 0x180);
    return 1;
}

/* Classifies where the point of a slot lies around the box of another slot: 0 inside (in front), 1..7 the surrounding sectors. */
s32 boxPointSectorClassify(SpawnerSlot *box, SpawnerSlot *point) {
    f32 halfSize;
    f32 minX;
    f32 maxX;
    f32 minZ;
    f32 maxZ;

    halfSize = 0.5f;
    if (box->model != 0) {
        halfSize = box->model->extents.size.w;
    }
    minX = box->position.x - halfSize;
    maxX = box->position.x + halfSize;
    minZ = box->position.z - halfSize;
    maxZ = box->position.z + halfSize;
    if (point->position.x < minX) {
        if (point->position.z < minZ) {
            return 5;
        }
        if (maxZ < point->position.z) {
            return 7;
        }
        return 6;
    }
    if (maxX < point->position.x) {
        if (point->position.z < minZ) {
            return 3;
        }
        if (maxZ < point->position.z) {
            return 1;
        }
        return 2;
    }
    if (point->position.z < box->position.z) {
        return 4;
    }
    return 0;
}

/* Value 0 of a slot of kind 16 or 64; 3 for other kinds. */
int func_001C1890(SpawnerSlot *slot) {
    int value;

    value = 3;
    switch (slot->kind) {
    case 16:
        value = slot->state.values.valuesKind16[0];
        break;
    case 64:
        value = slot->state.values.valuesKind64[0];
        break;
    }
    return value;
}

extern f32 D_003E481C[];

/* Builds the 1.1x extents vector of the slot model into out and returns how many particles to spawn (4..12, 1 without a model). */
s32 slotParticleCountCompute(AlignedVector4 *out, SpawnerSlot *slot) {
    SpawnedModel *model;
    f32 volume;
    f32 divisor;
    f32 scale;
    s32 count;

    count = 1;
    __asm__ __volatile__("sqc2 $vf0,0(%0)\n" : : "r"(out) : "memory");
    if (slot == 0) {
        return count;
    }
    model = slot->model;
    if (model == 0) {
        return count;
    }
    switch (model->shape) {
    case 0:
        out->x = model->extents.size.x;
        out->y = model->extents.size.y;
        out->z = model->extents.size.x;
        break;
    case 1:
        out->x = model->extents.size.x;
        out->y = model->extents.size.y;
        out->z = model->extents.size.z;
        break;
    default:
        return count;
    }
    scale = 1.1f;
    volume = out->x * out->y * out->z;
    divisor = D_003E481C[0];
    out->x = out->x * scale;
    out->y = out->y * scale;
    out->z = out->z * scale;
    count = (s32) (volume / divisor) + 3;
    count = (count < 4) ? 4 : count;
    return (count < 13) ? count : 12;
}

typedef struct SceneTickRequest {
    s32 tickParam;                  /* +0x00 */
    s32 spawnCount;                  /* +0x04 */
    s32 spawnTotal;                  /* +0x08 */
    f32 angleY;                      /* +0x0C */
    AlignedVector4 entryPosition; /* +0x10 */
    unsigned char unk_20[0x30 - 0x20];
    AlignedVector4 shapeExtents; /* +0x30 */
} SceneTickRequest;

void vuLaneMaskQuadsWrite(void *);
s32 MMathMakeRandomInt(void);
void sceneTickSubsystem();

/* Submits a scene tick request for the extents of the slot model at the slot position. */
void slotSceneTickSubmit(SpawnerSlot *slot, s32 tickParam) {
    s32 spawnCount;
    AlignedVector4 shapeExtents;
    SceneTickRequest tickRequest;
    s32 spawnTotal;

    spawnCount = slotParticleCountCompute(&shapeExtents, slot);
    if (spawnCount != 0) {
        vuLaneMaskQuadsWrite(&tickRequest);
        tickRequest.tickParam = tickParam;
        tickRequest.spawnCount = spawnCount;
        spawnTotal = spawnCount + MMathMakeRandomInt() % spawnCount / 2;
        tickRequest.angleY = slot->rotation.y;
        tickRequest.spawnTotal = spawnTotal;
        QuadwordCopyT0(&tickRequest.entryPosition, &slot->position);
        QuadwordCopyT0(&tickRequest.shapeExtents, &shapeExtents);
        sceneTickSubsystem(&tickRequest, spawnTotal);
    }
}

/* Sets value 0 of a slot of kind 16 or 64; returns 0 for other kinds. */
int func_001C1A58(SpawnerSlot *slot, int value) {
    switch (slot->kind) {
    case 16:
        slot->state.values.valuesKind16[0] = value;
        break;
    case 64:
        slot->state.values.valuesKind64[0] = value;
        break;
    default:
        return 0;
    }
    return 1;
}

/* Sets value 1 of a slot of kind 16 or 64; returns 0 for other kinds. */
int func_001C1A88(SpawnerSlot *slot, int value) {
    switch (slot->kind) {
    case 16:
        slot->state.values.valuesKind16[1] = value;
        break;
    case 64:
        slot->state.values.valuesKind64[1] = value;
        break;
    default:
        return 0;
    }
    return 1;
}

/* Sets value 2 of a slot of kind 16 or 64; returns 0 for other kinds. */
int func_001C1AB8(SpawnerSlot *slot, int value) {
    switch (slot->kind) {
    case 16:
        slot->state.values.valuesKind16[2] = value;
        break;
    case 64:
        slot->state.values.valuesKind64[2] = value;
        break;
    default:
        return 0;
    }
    return 1;
}

void padSetLatchedValue(s32 value);
void Vibration_Stop(void);
void Vibration_Set_Strong(unsigned char startValue, unsigned char endValue, int framesTotal);

void vibrationModeSet(unsigned int mode) {
    switch (mode) {
    case 0:
        Vibration_Stop();
        return;
    case 1:
        padSetLatchedValue(12);
        return;
    case 2:
        padSetLatchedValue(6);
        Vibration_Set_Strong(255, 0, 30);
        break;
    }
}

s32 func_001BF848();
s32 func_0027AFA8();

static inline void slotStartPrimaryEffect(SpawnerSlot *slot, s32 endless) {
    SpawnerState *state = &slot->state.effect;
    state->effectLifetime = endless ? 0x7FFFFFFF : 300;
    state->effectHandle = func_001BF848(0x10064029, &slot->position);
}

static inline void slotStartAuxEffect(SpawnerSlot *slot, s32 endless) {
    SpawnerState *state = &slot->state.effect;
    state->auxEffectLifetime = endless ? 0x7FFFFFFF : 300;
    state->auxEffectHandle = func_001BF848(0x10064029, &slot->position);
}

/* Starts the effect of a kind 2 or kind 4 slot once; returns 1 if it is started (or was) and 0 for other kinds. */
int slotEffectStartOnce(SpawnerSlot *slot, s32 endless) {
    if (slot->flags & SLOT_FLAG_EFFECT_STARTED) {
        return 1;
    }
    switch (slot->kind) {
    case 2:
        slotStartPrimaryEffect(slot, endless);
        break;
    case 4:
        slotStartAuxEffect(slot, endless);
        break;
    default:
        return 0;
    }
    slot->flags |= SLOT_FLAG_EFFECT_STARTED;
    func_0027AFA8(slot->handle);
    return 1;
}

/* Turns the edge of state flag 0x10000 into flag 0x40000 (rising) or 0x80000 (falling) and clears both old bits. */
void slotStateFlagEdgeUpdate(SpawnerSlot *slot) {
    unsigned int flags;
    unsigned int previous;
    unsigned int masked;
    unsigned int curBit;
    unsigned int prevBit;
    unsigned int value;

    flags = slot->stateFlags;
    previous = slot->previousStateFlags;
    masked = flags & 0xfff3ffff;
    curBit = flags & 0x10000;
    prevBit = previous & 0x10000;
    slot->stateFlags = masked;
    if (prevBit != 0) {
        if (curBit != 0) {
            return;
        }
        value = 0x80000;
        value = masked | value;
    } else {
        if (curBit == 0) {
            return;
        }
        value = 0x40000;
        value = masked | value;
    }
    slot->stateFlags = value;
}
