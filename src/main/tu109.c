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
    StateEntryName *name;            /* +0x04 */
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

#define SLOT_FLAG_LARGE 0x100           /* large variant (x5 scale) */
#define SLOT_FLAG_EFFECT_STARTED 0x2000 /* the effect was started once */
#define SLOT_FLAG_FOLLOW_HEIGHT 0x10000 /* the model follows the target height */

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

s32 slotFollowTargetHeight(SpawnerSlot *slot, f32 targetY);

s32 slotChildStateSelect(SpawnerSlot *slot, s32 useAlternate);

void slotStateFlagEdgeUpdate(SpawnerSlot *slot);

/* Kept as its own helper: merging it with QuadwordCopyT0 renumbers the compiler's labels. */
static inline void quadwordCopyVector(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(destination),"r"(source) : "$8","memory");
}

/* Kept as its own helper: merging it with QuadwordCopyMatrixT0 renumbers the compiler's labels. */
static inline void quadwordCopyMatrix(SlotMatrix *destination, SlotMatrix *source) {
    __asm__ __volatile__("lq $8,0(%1)\n lq $9,0x10(%1)\n lq $10,0x20(%1)\n lq $11,0x30(%1)\n sq $8,0(%0)\n sq $9,0x10(%0)\n sq $10,0x20(%0)\n sq $11,0x30(%0)" : : "r"(destination),"r"(source) : "$8","$9","$10","$11","memory");
}

static inline void vectorCopy(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source), "m"(source->x), "m"(source->y), "m"(source->z), "m"(source->w)
                         : "$8");
}

static inline void vectorSetOne(Vector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void matrixTranslationCopy(Vector4 *destination, SlotMatrix *source) {
    __asm__ __volatile__("lqc2 $vf1, 0x30(%5)\n vmove.w $vf1,$vf0\n sqc2 $vf1, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source));
}

SpawnerSlot *RgDispModelAlloc();
SpawnedModel *func_0028D670();
f32 MMathCalcLengthXZ(Vector4*);
s32 RgDispModelSetLocal(void *);
void MMathCalcAngleMatrixZ();
void MMathRotateMatrixY(SlotMatrix*,SlotMatrix*,f32);
void MMathRotateMatrixX(SlotMatrix*,SlotMatrix*,f32);
void slotSetValueSelector();
void func_0027AB00();

extern char gEntryNameLargePrefix[];
s32 func_0030EBD0();




void func_001C1FE0(SpawnerSlot *slot);


void playbackSetupWithoutFlags(SpawnerSlot *slot, s32 newResourceId);
s32 func_0027A7E8();

static inline void vectorScaleAll(Vector4 *destination, Vector4 *source, f32 factor) {
    __asm__ __volatile__(
        "mfc1 $8, %6\n qmtc2 $8, $vf2\n lqc2 $vf1,0(%5)\n vmulx.xyzw $vf1,$vf1,$vf2x\n sqc2 $vf1, 0(%4)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
        : "r"(destination), "r"(source), "f"(factor)
        : "$8");
}

/* Creates a kind 0x100 slot for one entry of the state block table: copies its matrix, sets trigger parameters and voices, then builds the model. */
int slotEntryDoorCreate(TableSlotRequest *request) {
    StateBlock *stateBlock;
    SpawnerSlot *slot;
    SpawnerEntryState *entry;
    SpawnedModel *model;
    stateBlock = ((StateBlock * (*) ()) stateBlockGet)();
    if (request->index < 0 || request->index >= stateBlock->entryCount) {
        return 0;
    }
    slot = RgDispModelAlloc(request->resourceId);
    if (slot == 0) {
        return 0;
    }
    entry = &slot->state.entry;
    slot->requestId = request->resourceId;
    slot->kind = 0x100;
    slot->update = func_001C1FE0;
    entry->name = &stateBlock->entryNames[request->index];
    entry->tableIndex = request->index;
    slot->halfScale = 1.5f;
    entry->matrix = (SlotMatrix *) ((u8 *) stateBlock->table + stateBlock->table->entryOffset) + request->index;
    quadwordCopyMatrix(&entry->localMatrix, entry->matrix);
    entry->extents.x = 1.5f;
    entry->extents.y = 2.25f;
    entry->extents.z = 0.25f;
    entry->extents.w = 0.75f;
    entry->triggerRadius = slot->halfScale * 3.0f;
    entry->angle = 1.5f;
    entry->rate = 20;
    entry->flags = 0;
    entry->mode = 0;
    entry->rateCopy = 0;
    if (func_0030EBD0(entry->name, gEntryNameLargePrefix, 6) == 0) {
        entry->voiceA = 0x60;
        entry->voiceB = 0x61;
        entry->flags |= ENTRY_FLAG_NORMAL;
    } else {
        slot->halfScale *= 5.0f;
        slot->flags |= SLOT_FLAG_LARGE;
        entry->flags |= ENTRY_FLAG_LARGE;
        vectorScaleAll(&entry->extents, &entry->extents, 5.0f);
        entry->triggerRadius *= 5.0f;
        entry->angle *= 5.0f;
        entry->voiceA = 0x62;
        entry->voiceB = 0x63;
    }
    model = func_0028D670();
    slot->model = model;
    if (model == 0) {
        RgDispModelSetLocal(slot);
        return 0;
    }
    model->shape = 1;
    quadwordCopyVector(&model->extents.size, &entry->extents);
    model->extents.size.w = MMathCalcLengthXZ(&model->extents.size) * 0.5f;
    matrixTranslationCopy(&model->position.v, entry->matrix);
    MMathCalcAngleMatrixZ(&model->rotation, entry->matrix);
    vectorSetOne(&model->scale);
    vectorCopy(&slot->position, &model->position.v);
    quadwordCopyVector(&slot->rotation, &model->rotation);
    MMathRotateMatrixY(&entry->localMatrix, &entry->localMatrix, -model->rotation.y);
    MMathRotateMatrixX(&entry->localMatrix, &entry->localMatrix, -model->rotation.x);
    playbackSetupWithoutFlags(slot, -1);
    slot->handle = func_0027A7E8(model);
    slotSetValueSelector(slot->handle, 0);
    func_0027AB00(slot->handle);
    return 1;
}

s32 func_002A6FA8();
void func_002A70E8(unsigned char *);

/* Sets the model of a slot to the "without flags" playback: mode 3, rate from the radius slot, handle from func_002A6FA8. */
void playbackSetupWithoutFlags(SpawnerSlot *slot, s32 newResourceId) {
    f32 playRate;
    s32 playerHandle;
    s32 resourceId;
    SpawnedModel *model;

    resourceId = newResourceId;
    slot->resourceId = resourceId;
    model = slot->model;
    if (model != 0) {
        func_002A70E8(model->restartBlock);
        model->playbackMode = 3;
        model->playbackFlags = 0;
        playerHandle = func_002A6FA8(0x100, slot->resourceId);
        playRate = model->extents.size.w;
        model->playRate = playRate;
        model->playerHandle = playerHandle;
    }
}

SpawnerSlot *RgDispModelFindByKey(s32 kind);

void slotEntryMotionStep(SpawnerSlot *slot);



void slotEntryPadTriggerCheck(SpawnerSlot *slot);



void slotEntryOpenOnPad(SpawnerSlot *slot);

/* Update of a kind 0x100 slot: runs its motion while moving, otherwise tests the leader slot (kind 0) and the trigger flags. */
void func_001C1FE0(SpawnerSlot *slot) {
    SpawnerEntryState *entry;
    SpawnerSlot *leader;
    s32 mask;

    entry = &slot->state.entry;
    if (entry->flags & ENTRY_FLAG_MOVING) {
        slotEntryMotionStep(slot);
        return;
    }
    leader = RgDispModelFindByKey(0);
    if (leader == 0) {
        return;
    }
    mask = (leader->flags & SLOT_FLAG_LARGE) ? ENTRY_FLAG_LARGE : ENTRY_FLAG_NORMAL;
    if ((entry->flags & mask) == 0) {
        return;
    }
    slotEntryPadTriggerCheck(slot);
    if (entry->flags & 0x2) {
        return;
    }
    if (entry->flags & ENTRY_FLAG_OPEN) {
        slotEntryCloseWhenFar(slot);
    } else {
        slotEntryOpenOnPad(slot);
    }
}

static inline void vectorZeroQuad(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

void MMathApplyMatrix();
void SpawnSlotSetPosition();

/* Per-frame update of a motion slot: counts the move down, then slides or turns its matrix by the per-frame amount. */
void slotEntryMotionStep(SpawnerSlot *slot) {
    Vector4 offset;
    SpawnerEntryState *entry = &slot->state.entry;
    if (--entry->rateCopy <= 0) {
        entry->flags &= ~ENTRY_FLAG_MOVING;
    }
    switch (entry->mode) {
    case 0:
        vectorZeroQuad(&offset);
        offset.x = entry->ratio;
        MMathApplyMatrix(&offset, entry->matrix, &offset);
        SpawnSlotSetPosition(slot, &offset);
        break;
    case 1:
        MMathRotateMatrixY(entry->matrix, entry->matrix, entry->ratio);
        if (entry->flags & ENTRY_FLAG_MOVING) {
            break;
        }
        if (entry->flags & ENTRY_FLAG_OPEN) {
            slotChildStateSelect(slot, 0);
        }
        break;
    case 2:
        vectorZeroQuad(&offset);
        offset.y = entry->ratio;
        MMathApplyMatrix(&offset, entry->matrix, &offset);
        SpawnSlotSetPosition(slot, &offset);
        break;
    }
}

f32 MMathCalcDist(void *, void *);
f32 actorFacingError(void *state, void *vector);
s32 sbNativeHandlerMessagePost(s32 mode, s32 count, s32 flag);
extern f32 gPadTriggerMaxAngle;
extern u16 gPadDataPressed[];

/* Starts the shorter slot limit of the pair when the pad button is pressed within reach of the leader slot. */
void slotEntryPadTriggerCheck(SpawnerSlot *slot) {
    SpawnerSlot *leader;
    SpawnerEntryState *entry;
    SpawnedModel *leaderModel;
    Vector4 *position;
    f32 distance;
    s32 slotLimit;
    u16 padFlags;
    s32 partnerLimit;

    entry = &slot->state.entry;
    leader = RgDispModelFindByKey(0);
    padFlags = gPadDataPressed[0];
    if ((entry->flags & (ENTRY_FLAG_OPEN | ENTRY_FLAG_MOVING)) || (!(entry->flags & 0x4) && !(padFlags & 0x20))) {
        return;
    }
    position = &slot->position;
    distance = MMathCalcDist(position, &leader->position);
    leaderModel = leader->model;
    if (slot->halfScale < distance - leaderModel->extents.size.x) {
        return;
    }
    if (gPadTriggerMaxAngle < actorFacingError(leaderModel, position)) {
        return;
    }
    if (slot->flags & 0x800) {
        return;
    }
    slotLimit = slot->requestId;
    if (slot->partner != 0) {
        partnerLimit = slot->partner->requestId;
        if (partnerLimit < slotLimit) {
            slotLimit = partnerLimit;
        }
    }
    do {
        sbNativeHandlerMessagePost(6, slotLimit, 0);
    } while (0);
}

/* Opens the slot and its partner when the pad button is pressed within reach of the leader slot. */
void slotEntryOpenOnPad(SpawnerSlot *slot) {
    SpawnerEntryState *entry;
    SpawnerSlot *leader;
    SpawnedModel *leaderModel;
    unsigned short padPressed;
    f32 distance;

    entry = &slot->state.entry;
    leader = RgDispModelFindByKey(0);
    padPressed = gPadDataPressed[0];
    if ((entry->flags & 0x4) == 0 && (padPressed & 0x20) == 0) {
        return;
    }
    distance = MMathCalcDist(&slot->position, &leader->position);
    leaderModel = leader->model;
    if (slot->halfScale < distance - leaderModel->extents.size.x) {
        return;
    }
    if (1.04719758f < actorFacingError(leaderModel, &slot->position)) {
        return;
    }
    slotEntryMotionStart(slot, 0, 1);
    slotEntryMotionStart(slot->partner, 0, 0);
}

/* Closes the slot and its partner once both are farther than the trigger radius from the leader slot. */
void slotEntryCloseWhenFar(SpawnerSlot *slot) {
    SpawnerEntryState *entry;
    SpawnerSlot *leader;

    entry = &slot->state.entry;
    leader = RgDispModelFindByKey(0);
    if (MMathCalcDist(&slot->position, &leader->position) < entry->triggerRadius) {
        return;
    }
    if (slot->partner != 0) {
        if (MMathCalcDist(&slot->partner->position, &leader->position) < entry->triggerRadius) {
            return;
        }
    }
    slotEntryMotionStart(slot, 1, 1);
    slotEntryMotionStart(slot->partner, 1, 0);
}

void seVoiceStartDefault(unsigned int handle);

/* Starts the opening (mode 0) or closing (mode 1) motion of a slot; returns 0 for a null slot. */
s32 slotEntryMotionStart(SpawnerSlot *slot, unsigned int mode, s32 startVoice) {
    SpawnerEntryState *entry;

    if (slot == 0) {
        return 0;
    }
    entry = &slot->state.entry;
    switch (mode) {
    case 0: {
        s32 flags;
        s32 rate;
        f32 ratio;

        flags = entry->flags;
        if ((flags & ENTRY_FLAG_MOVING) == 0) {
            if ((flags & ENTRY_FLAG_OPEN) == 0) {
                rate = entry->rate;
                entry->flags = flags | (ENTRY_FLAG_OPEN | ENTRY_FLAG_MOVING);
                entry->rateCopy = rate;
                ratio = entry->angle / (f32) rate;
                entry->ratio = ratio;
                if (startVoice != 0) {
                    seVoiceStartDefault(entry->voiceA);
                }
            }
        }
        break;
    }
    case 1: {
        s32 flags;
        s32 rate;
        f32 ratio;

        flags = entry->flags;
        if ((flags & ENTRY_FLAG_MOVING) == 0) {
            if ((flags & ENTRY_FLAG_OPEN) != 0) {
                rate = entry->rate;
                ratio = -entry->angle / (f32) rate;
                entry->flags = (flags & ~ENTRY_FLAG_OPEN) | ENTRY_FLAG_MOVING;
                entry->rateCopy = rate;
                entry->ratio = ratio;
                if (entry->mode == mode) {
                    slotChildStateSelect(slot, mode);
                }
                if (startVoice != 0) {
                    seVoiceStartDefault(entry->voiceB);
                }
            }
        }
        break;
    }
    }
    return 1;
}

/* Finishes the motion of a slot at once: applies the full move or turn and marks it done. */
s32 slotEntryMotionFinish(SpawnerSlot *slot) {
    AlignedVector4 point;
    SpawnerEntryState *entry;
    s32 result;

    result = 0;
    if (slot != 0) {
        entry = &slot->state.entry;
        entry->flags = (entry->flags & ~ENTRY_FLAG_MOVING) | ENTRY_FLAG_OPEN;
        switch (entry->mode) {
        case 0:
            Vu0SetOrigin(&point);
            point.x = entry->angle;
            MMathApplyMatrix(&point, entry->matrix, &point);
            SpawnSlotSetPosition(slot, &point);
            break;
        case 1:
            MMathRotateMatrixY(entry->matrix, entry->matrix, entry->angle);
            slotChildStateSelect(slot, 0);
            break;
        case 2:
            Vu0SetOrigin(&point);
            point.y = entry->angle;
            MMathApplyMatrix(&point, entry->matrix, &point);
            SpawnSlotSetPosition(slot, &point);
            break;
        }
        result = 1;
    }
    return result;
}

/* Effect query filled by sefObjectSetStatePlay and started by sefSpawnEffectFromRequest. */
typedef struct {
    s32 resourceId;                  /* +0x00 */
    u8 unk_04[0x28];
    s32 effectCode;                  /* +0x2C */
    u8 unk_30[0x10];
    Vector4 position;                /* +0x40 */
    Vector4 rotation;                /* +0x50 */
    u8 unk_60[0x20];
} EffectQuery;

typedef struct {
    s32 id;
    s32 sourceId;
    s32 variantId;
} EffectSlotRequest;

typedef struct {
    u8 unmodeled_00[0x24];
    s32 resourceId;                  /* +0x24 */
} EffectSource;

void sefObjectSetStatePlay();
s32 sefSpawnEffectFromRequest();
void playbackSetLocalIfIdle(SpawnerSlot *slot);
void playbackReleaseHandle(SpawnerSlot *slot);

/* Creates a kind 0x4000 slot that plays an effect looked up from the request (source id, variant). */
int effectSlotCreate(EffectSlotRequest *request) {
    EffectQuery query;
    SpawnerSlot *slot = RgDispModelAlloc(request->id);
    EffectSource *source;
    s32 lookup;
    s32 resource;

    if (slot == 0) {
        return 0;
    }
    {
        SpawnerChannelState *channel = &slot->state.channel;
        slot->kind = 0x4000;
        slot->requestId = request->id;
        slot->update = playbackSetLocalIfIdle;
        channel->sourceId = request->sourceId;
        channel->variantId = request->variantId;
        slot->release = playbackReleaseHandle;
        sefObjectSetStatePlay(&query);
        query.effectCode = channel->variantId;
        lookup = channel->sourceId;
        if (lookup >= 0) {
            source = ((EffectSource * (*) ()) RgDispModelFindByKey)(lookup);
            if (source) {
                resource = source->resourceId;
                if (resource != 0) {
                    query.resourceId = resource;
                }
            }
        }
        lookup = sefSpawnEffectFromRequest(&query);
        channel->playingHandle = lookup;
        if (lookup == 0) {
            RgDispModelSetLocal(slot);
            return 0;
        }
        return 1;
    }
}

void sefEffectHandleRelease(s32);

void playbackReleaseHandle(SpawnerSlot *slot) {
    s32 playerHandle;

    playerHandle = slot->state.channel.playingHandle;
    if (playerHandle != 0) {
        sefEffectHandleRelease(playerHandle);
    }
}

s32 func_001EA050(s32);

void playbackSetLocalIfIdle(SpawnerSlot *slot) {
    if (func_001EA050(slot->state.channel.playingHandle) == 0) {
        RgDispModelSetLocal(slot);
    }
}

void slotUpdateNoOp(void);
void func_002A5A30(s32 resourceId, s32 variant, SpawnerSlot *slot);

/* Kept as its own helper: merging it with vectorCopy renumbers the compiler's labels. */
static inline void vectorCopyToSlot(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
        : "=m" (destination->x), "=m" (destination->y), "=m" (destination->z), "=m" (destination->w)
        : "r" (destination), "r" (source), "m" (source->x), "m" (source->y), "m" (source->z), "m" (source->w) : "$8");
}

/* Creates a kind 0x2000 slot for one entry of the state block table: copies its matrix into the slot and builds the model from it. */
s32 func_001C27C0(TableSlotRequest *request) {
    StateBlock *stateBlock;
    SpawnerSlot *slot;
    SpawnedModel *model;
    SpawnerTableState *table;
    StateTableHeader *header;
    SlotMatrix *entry;
    SlotMatrix *source;
    SlotMatrix *matrix;
    stateBlock = ((StateBlock * (*) (void) ) stateBlockGet)();
    if (request->index < 0 || request->index >= stateBlock->entryCount) {
        return 0;
    }
    slot = RgDispModelAlloc(request->resourceId);
    if (slot == 0) {
        return 0;
    }
    slot->update = slotUpdateNoOp;
    slot->requestId = request->resourceId;
    slot->kind = 0x2000;
    table = &slot->state.table;
    table->tableIndex = request->index;
    table->variant = request->variant;
    header = stateBlock->table;
    entry = (SlotMatrix *) ((u8 *) header + header->entryOffset) + request->index;
    table->matrix = entry;
    matrix = &slot->state.table.localMatrix;
    QuadwordCopyMatrixT0(matrix, entry);
    model = func_0028D670();
    slot->model = model;
    if (model == 0) {
        RgDispModelSetLocal(slot);
        return 0;
    }
    model->shape = 1;
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%0)" : : "r"(&model->extents.size) : "memory");
    model->extents.size.w = MMathCalcLengthXZ(&model->extents.size) * 0.5f;
    source = table->matrix;
    __asm__ __volatile__("lqc2 vf1, 48(%1)\n\tvmove.w vf1w, vf0w\n\tsqc2 vf1, 0(%0)" : : "r"(&model->position.v), "r"(source) : "memory");
    MMathCalcAngleMatrixZ(&model->rotation, source);
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%0)" : : "r"(&model->scale) : "memory");
    vectorCopyToSlot(&slot->position, &model->position.v);
    vectorCopyToSlot(&slot->rotation, &model->rotation);
    MMathRotateMatrixY(matrix, matrix, -model->rotation.y);
    MMathRotateMatrixX(matrix, matrix, -model->rotation.x);
    func_002A5A30(request->resourceId, request->variant, slot);
    return 1;
}

void slotUpdateNoOp(void) {
}



void playbackSetupWithFlags(SpawnerSlot *slot, s32 newResourceId);
void playbackReleaseBlock(SpawnerSlot *slot);
s32 func_001B8B58(s32 index, s32 flag);
typedef struct {
    u8 unmodeled_00[4];
    s16 flags;                       /* +0x04 */
} SourceRecord;

SourceRecord *nativeMethodEntryFind(s32 index);
s32 func_001B5D70(s32 variant);
s32 *func_001B5C68(s32 variant);
s32 func_001B6918(s32 a0, s32 a1, s32 a2, s32 a3);
s32 poolBlockTagWordGet(s32 block, s32 flags);
s32 requestNameMappedPathGet(s32 value);
s32 func_001D0258(s32 value);
s32 payloadSectionsLoad(SpawnedModel *model, s32 value);
s32 func_001C0A48(SpawnedModel *model, s32 value, s32 flags);
s32 func_0028D760(SpawnedModel *model);
s32 XenoActorSetChannelValue(SpawnedModel *model, f32 value);
s32 XenoActorSetMode(SpawnedModel *model, s32 mode);
s32 func_0027A728(SpawnedModel *model);
void streamBeginRecord(s32 handle, s32 a1, s32 a2, s32 a3);
void streamFlushNoop(void);
s32 func_0027B450(s32 handle, s32 a1, s32 a2, s32 a3);



void kindFourSlotUpdate(SpawnerSlot *slot);

/* Kept as its own helper: merging it with vectorSetOne renumbers the compiler's labels. */
static inline void vectorSetOneScale(Vector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void vectorClear(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

typedef struct {
    s32 sourceId;                    /* +0x00 */
    s32 slotResourceId;              /* +0x04 */
    s32 variantId;                   /* +0x08 */
    s32 result;                      /* +0x0C */
} KindFourSlotRequest;

typedef struct {
    s32 flags;                       /* +0x00 */
    u8 unk_04[0x1ec];
    s32 modelOptionBits;             /* +0x1F0 */
    u8 unk_1F4[0x200];
} RuntimeStorageView;

extern RuntimeStorageView gGameLoopRuntimeStorage[];

static inline s32 pickFlagBit(s16 flags, s32 current, s32 reference) {
    if (current != reference) {
        return flags & 2;
    }
    return flags & 1;
}

/* Creates a kind 4 slot with its model (draw flags and size depend on the 0x100 flag) from a request; returns 0 when the pool or the model is exhausted. */
int kindFourSlotCreate(KindFourSlotRequest *request) {
    StateBlock *stateBlock = ((StateBlock * (*) (void) ) stateBlockGet)();
    SpawnerSlot *slot;
    SpawnedModel *model;
    s32 flagBit;
    s32 *resourceWords;
    s32 created;
    StateModeBlock *modeBlock;
    if (request->sourceId != 0) {
        flagBit = (gGameLoopRuntimeStorage[0].flags >> 21) & 1;
        if (func_001B8B58(request->sourceId, flagBit)) {
            if (!pickFlagBit(nativeMethodEntryFind(request->sourceId)->flags, flagBit, 0)) {
                request->result = 0;
                created = 1;
                return created;
            }
        }
    }
    slot = RgDispModelAlloc(request->slotResourceId);
    if (slot == 0) {
        return 0;
    }
    slot->flags |= (SLOT_FLAG_FOLLOW_HEIGHT | 0x20);
    if (func_001B5D70(request->variantId)) {
        slot->flags |= SLOT_FLAG_LARGE;
    } else {
        slot->flags |= 0x80;
    }
    {
        SpawnerChannelState *channel = &slot->state.channel;
        slot->bounceDamping = 0;
        slot->requestId = request->slotResourceId;
        slot->kind = 4;
        channel->sourceId = request->sourceId;
        slot->update = kindFourSlotUpdate;
        slot->release = playbackReleaseBlock;
        channel->variantId = request->variantId;
        channel->playingHandle = -1;
        resourceWords = func_001B5C68(channel->variantId);
        if (resourceWords == 0) {
            channel->resourceA = poolBlockTagWordGet(func_001B6918(0, 1, 0xCF010100, -1), 0);
            channel->resourceB = poolBlockTagWordGet(func_001B6918(0, 1, 0xCF010300, -1), 0);
            channel->resourceC = poolBlockTagWordGet(func_001B6918(0, 1, 0xCF020000, -1), 0);
        } else {
            channel->resourceA = *resourceWords;
            channel->resourceB = requestNameMappedPathGet(*resourceWords);
            channel->resourceC = func_001D0258(channel->resourceA);
        }
        model = func_0028D670();
        slot->model = model;
        if (model == 0) {
            RgDispModelSetLocal(slot);
            return 0;
        }
        model->drawFlags |= 0x24008;
        model->drawFlags |= (gGameLoopRuntimeStorage[0].modelOptionBits & 2) ? 0x8000 : 0;
        model->drawFlags |= (gGameLoopRuntimeStorage[0].modelOptionBits & 4) ? 0x4000000 : 0;
        modeBlock = &stateBlock->modeBlock;
        if (modeBlock != 0 && modeBlock->mode != 3) {
            model->unk_4EC |= 0x8000;
        }
        model->unk_9C = 1;
        model->unk_A0 = stateBlock->modelAlpha;
        model->unk_12C = 1.5f;
        model->targetHeight = 0;
        vectorClear(&model->extents.size);
        model->shape = 0;
        model->extents.size.x = model->extents.size.w = (slot->flags & SLOT_FLAG_LARGE) ? 4.5f : 0.64f;
        model->extents.size.y = (slot->flags & SLOT_FLAG_LARGE) ? 8.0f : 1.5f;
        vectorClear(&model->position.v);
        vectorClear(&model->rotation);
        vectorSetOneScale(&model->scale);
        payloadSectionsLoad(model, channel->resourceA);
        func_001C0A48(model, channel->resourceB, 0);
        func_0028D760(model);
        XenoActorSetChannelValue(model, 12.0f);
        XenoActorSetMode(model, 0);
        playbackSetupWithFlags(slot, -1);
        slot->handle = func_0027A728(model);
        streamBeginRecord(slot->handle, 0, 0, 1);
        streamFlushNoop();
        func_0027B450(slot->handle, 0, 7, 0x28);
        streamFlushNoop();
        slotSetValueSelector(slot->handle, 0);
        func_0027AB00(slot->handle);
        created = 1;
        request->result = created;
        return created;
    }
}

/* Sets the model of a slot to the "with flags" playback: mode 2, flags 0x2112, rate from the extents. */
void playbackSetupWithFlags(SpawnerSlot *slot, s32 newResourceId) {
    f32 playRate;
    s32 playerHandle;
    s32 resourceId;
    SpawnedModel *model;

    resourceId = newResourceId;
    slot->resourceId = resourceId;
    model = slot->model;
    if (model != 0) {
        func_002A70E8(model->restartBlock);
        model->playbackFlags = 0x2112;
        model->playbackMode = 2;
        playerHandle = func_002A6FA8(0x100, slot->resourceId);
        playRate = model->extents.size.x;
        model->playRate = playRate;
        model->playerHandle = playerHandle;
    }
}

SpawnerSlot *RgDispModelNextByKind(s32 kind, SpawnerSlot *previous);


/* Arms the release countdown (2 frames) of every started kind 4 slot whose countdown is still running; returns how many. */
s32 kindFourEffectsArm(void) {
    SpawnerSlot *slot;
    SpawnerChannelState *channel;
    s32 armedCount;

    armedCount = 0;
    slot = 0;
    while ((slot = RgDispModelNextByKind(4, slot)) != 0) {
        channel = &slot->state.channel;
        if ((slot->flags & SLOT_FLAG_EFFECT_STARTED) != 0) {
            if (channel->auxEffectLifetime < 301) {
                channel->auxEffectLifetime = 2;
                armedCount++;
            }
        }
    }
    return armedCount;
}

void playbackReleaseBlock(SpawnerSlot *slot) {
    SpawnerChannelState *channel;
    s32 playerHandle;

    channel = &slot->state.channel;
    playerHandle = channel->auxEffectHandle;
    if (playerHandle != 0) {
        sefEffectHandleRelease(playerHandle);
        channel->auxEffectHandle = 0;
    }
}

int slotEffectQueryStart(SpawnerSlot *slot);
void func_001EAA08(s32 handle, void *vector);
void func_0027B020(s32 handle);
void func_001ECA60(SpawnerSlot *slot);
f32 slotGetScale(s32 handle);

/* Update of a kind 4 slot: handles the 0x40 request, follows the height of the slot handle, and releases the aux effect when its countdown runs out. */
void kindFourSlotUpdate(SpawnerSlot *slot) {
    SpawnerChannelState *channel;
    s32 auxEffectHandle;

    channel = &slot->state.channel;
    if ((slot->flags & 0x40) != 0) {
        slot->flags &= ~0x48;
        slotChildStateSelect(slot, 1);
        slotEffectQueryStart(slot);
    }
    slotFollowTargetHeight(slot, slotGetScale(slot->handle));
    if ((slot->flags & SLOT_FLAG_EFFECT_STARTED) != 0) {
        func_001EAA08(channel->auxEffectHandle, &slot->position);
        channel->auxEffectLifetime -= 1;
        if (channel->auxEffectLifetime <= 0) {
            auxEffectHandle = channel->auxEffectHandle;
            slot->flags &= ~SLOT_FLAG_EFFECT_STARTED;
            if (auxEffectHandle != 0) {
                sefEffectHandleRelease(auxEffectHandle);
                channel->auxEffectHandle = 0;
            }
            func_0027B020(slot->handle);
        }
    }
    slotStateFlagEdgeUpdate(slot);
    func_001ECA60(slot);
}

/* Kept as its own helper: merging it with vectorCopy renumbers the compiler's labels. */
static inline void vectorCopyToQuery(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination), "r"(source), "m"(source->x), "m"(source->y), "m"(source->z), "m"(source->w) : "$8");
}

int slotEffectQueryStart(SpawnerSlot *slot) {
    EffectQuery query;

    sefObjectSetStatePlay(&query);
    query.effectCode = (slot->flags & SLOT_FLAG_LARGE) ? 0x1006401D : 0x1006401C;
    vectorCopyToQuery(&query.position, &slot->position);
    QuadwordCopyT0(&query.rotation, &slot->rotation);
    sefSpawnEffectFromRequest(&query);
}
