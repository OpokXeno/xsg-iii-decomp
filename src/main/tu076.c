#include "common.h"

#include "shared.h"
#include "quadword.h"
#include "vu0_macro.h"
typedef int s32;

typedef float f32;

typedef struct SpawnRequest {
    u32 kind;  /* +0x00 */
    s32 state; /* +0x04 */
    unsigned char unmodeled_08[0x10 - 0x08];
    AlignedVector4 position; /* +0x10 */
    AlignedVector4 offset;   /* +0x20 */
    f32 extent;              /* +0x30 */
    f32 sinCos;              /* +0x34 */
    f32 angle;               /* +0x38 */
} SpawnRequest;

void *RgDispModelAlloc(s32);

typedef struct MotionRecord {
    unsigned char unmodeled_00[0x40];
} MotionRecord;

typedef struct SpawnVector4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} SpawnVector4;

typedef struct {
    SpawnVector4 m[4];
} SpawnMatrix4;

enum SpawnModelKind {
    SPAWN_MODEL_KIND_NONE,
    SPAWN_MODEL_KIND_WIDTH = 0x7fffffff
};

enum SpawnActorFlagsKind {
    SPAWN_ACTOR_FLAGS_NONE,
    SPAWN_ACTOR_FLAGS_WIDTH = 0x7fffffff
};

typedef struct SpawnModel {
    s32 unmodeled_00;
    s32 modelKind; /* +0x04: set from the decoded record word 0 */
    u8 unmodeled_08[8];
    union {
        SpawnVector4 vector;
        s32 words[4];
    } extents; /* +0x10: kept as a union, the word view decides the emitted order */
    SpawnVector4 position; /* +0x20 */
    SpawnVector4 vector30;
    SpawnVector4 angle; /* +0x40 */
    SpawnVector4 rotation; /* +0x50 */
    SpawnVector4 vector60;
    SpawnVector4 vector70;
    SpawnVector4 scale;
    s32 drawFlags;
    u8 unmodeled_94[8];
    s16 unk_9C;
    s16 unk_9E;
    f32 unk_A0;
    u8 unmodeled_A4[0xf8 - 0xa4];
    s32 unk_F8;
    u8 unmodeled_FC[0x12c - 0xfc];
    f32 unk_12C;
    u8 unmodeled_130[0x4ec - 0x130];
    s32 unk_4EC;
    u8 unmodeled_4F0[0x510 - 0x4f0];
    void *unk_510;
    u8 unmodeled_514[0x528 - 0x514];
    void *unk_528;
} SpawnModel;

typedef struct SpawnActorState {
    s32 displayIndex;
    u8 *record;
    SpawnMatrix4 *sourceMatrix;
    s32 pad;
    SpawnMatrix4 matrix;
    enum SpawnStateWordKind {
        SPAWN_STATE_WORD_NONE,
        SPAWN_STATE_WORD_WIDTH = 0x7fffffff
    } unk_50, unk_54, unk_58;
} SpawnActorState;

typedef struct SpawnMotionState {
    MotionRecord motion;
    AlignedVector4 delta;
} SpawnMotionState;

typedef struct SpawnEffectSlot {
    unsigned char unmodeled_00[0x54];
    s32 sefEffectHandle; /* +0x54 */
    s32 voiceHandle;     /* +0x58 */
    s32 voiceCountdown;  /* +0x5C */
} SpawnEffectSlot;

typedef struct SpawnActor {
    enum SpawnActorFlagsKind flags;
    unsigned char unmodeled_04[0x08 - 0x04];
    s32 stateFlags; /* +0x08: bit 0 gates the update */
    unsigned char unmodeled_0C[0x10 - 0x0C];
    enum SpawnModelKind kind; /* +0x10 */
    s32 ownerModel;           /* +0x14: non-zero stops the update */
    unsigned char unmodeled_18[0x1C - 0x18];
    void (*update)(); /* +0x1C */
    void (*release)(); /* +0x20 */
    SpawnModel *actor; /* +0x24 */
    s32 key;           /* +0x28 */
    unsigned char unmodeled_2C[0x30 - 0x2C];
    s32 actorHandle; /* +0x30: result of func_0027A7E8 on the actor */
    s32 drawHandle; /* +0x34 */
    unsigned char unmodeled_38[0x40 - 0x38];
    SpawnVector4 position; /* +0x40 */
    SpawnVector4 vector50; /* +0x50 */
    SpawnVector4 rotation; /* +0x60 */
    SpawnVector4 vector70; /* +0x70 */
    SpawnVector4 scale;    /* +0x80 */
    unsigned char unmodeled_90[0x130 - 0x90];
    void *effectHandle; /* +0x130 */
    f32 halfScale;      /* +0x134 */
    unsigned char unmodeled_138[0x150 - 0x138];
    union {
        SpawnActorState bound;
        SpawnMotionState spawned;
        SpawnEffectSlot effects;
    } state; /* +0x150: kept as a union, the two spawners lay it out differently */
} SpawnActor;
void spawnMotionActorUpdate(SpawnActor *object);
void func_00284E30();
void motionRecordSetState(void *, s32);
void motionRecordSetFlags(void *, s32);
void motionRecordSetExtent(void *, f32);
void setSinCosFields(void *, f32);
void setRotationAngle(void *, f32);
void *memset();

static inline void spawnVectorSubtract(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 $vf1,0(%5)\n"
                         "lqc2 $vf2,0(%6)\n"
                         "vsub.xyz $vf1,$vf1,$vf2\n"
                         "sqc2 $vf1,0(%4)\n"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(left), "r"(right));
}

/* Creates a slot object from a spawn request: copies the position and builds the motion record between offset and position. */
s32 spawnMotionActorCreate(SpawnRequest *spawnRequest) {
    SpawnActor *spawnActor;
    SpawnMotionState *motionState;
    s32 modelKind;

    spawnActor = RgDispModelAlloc(spawnRequest->kind);
    if (spawnActor == 0) {
        return 0;
    }
    modelKind = spawnRequest->kind;
    spawnActor->kind = 0x1000;
    spawnActor->update = spawnMotionActorUpdate;
    spawnActor->key = modelKind;
    QuadwordCopyT0(&spawnActor->position, &spawnRequest->position);
    motionState = &spawnActor->state.spawned;
    spawnVectorSubtract(&spawnActor->state.spawned.delta, &spawnRequest->offset, &spawnRequest->position);
    memset(motionState, 0, 0x40);
    func_00284E30(motionState, &spawnRequest->position, &spawnRequest->offset);
    motionRecordSetState(motionState, spawnRequest->state);
    motionRecordSetFlags(motionState, 0x20);
    setRotationAngle(motionState, spawnRequest->angle);
    setSinCosFields(motionState, spawnRequest->sinCos);
    motionRecordSetExtent(motionState, spawnRequest->extent);
    return 1;
}

typedef signed char s8;

typedef short s16;

typedef unsigned short u16;

typedef struct GameLoopState {
    s32 loopFlags; /* +0x00: bit 0x800 stops the scan */
} GameLoopState;

extern unsigned char gGameLoopRuntimeStorage[];
void storeValueWithSceneBit(MotionRecord *motion);

typedef struct DispModelState {
    unsigned char unmodeled_000[4];
    s32 field04; /* +0x04: the first word of actorExtentShapeFromName's output */
    unsigned char unmodeled_008[0x1C - 0x08];
    f32 scale; /* +0x1C */
    unsigned char unmodeled_020[0x50 - 0x20];
    f32 angle50; /* +0x50 */
    f32 angle54; /* +0x54 */
    unsigned char unmodeled_058[0x6A0 - 0x58];
    unsigned char collision[0x6AA - 0x6A0];
    s16 counter6AA; /* +0x6AA: cleared to 0 by dispModelBindValue */
    unsigned char unmodeled_6AC[0x6B3 - 0x6AC];
    s8 mode6B3; /* +0x6B3: set to 3 by dispModelBindValue */
    unsigned char unmodeled_6B4[0x6BC - 0x6B4];
    s32 allocatedHandle; /* +0x6BC: the allocatedHandle allocated by func_002A6FA8 */
    unsigned char unmodeled_6C0[0x6CC - 0x6C0];
    f32 savedScale; /* +0x6CC: a copy of scale */
} DispModelState;

typedef struct DispModelTail {
    unsigned char unmodeled_00[0x08];
    s32 unk_08; /* +0x158 */
} DispModelTail;

typedef struct DispModel {
    s32 flags; /* +0x00 */
    unsigned char unmodeled_04[0x08 - 0x04];
    s32 unk_08; /* +0x08: bit 2 is set on the target entry */
    unsigned char unmodeled_0C[0x24 - 0x0C];
    DispModelState *modelState; /* +0x24 */
    s32 unk_28;                 /* +0x28 */
    s32 bindValue;              /* +0x2C */
    s32 drawHandle;             /* +0x30: released with func_0027AC90, then set to -1 */
    s32 displayEntryKey;        /* +0x34: looked up with RgDispModelFindByKey */
    unsigned char unmodeled_38[0x50 - 0x38];
    unsigned char unk_50[0x150 - 0x50]; /* +0x50: an address passed to func_00284F50 */
    DispModelTail tail;                 /* +0x150 */
} DispModel;

typedef struct DispModelEntry {
    s32 ownerFlags; /* +0x00: bit 0x40 is set by spawnDisplayRecordUpdate */
    unsigned char unmodeled_04[0x10 - 0x04];
    s32 kind;       /* +0x10: 0x200 for the entries dispModelDetachEntry releases */
    unsigned char unmodeled_14[0x150 - 0x14];
    u16 flags;      /* +0x150: bit 0x10 is tested */
} DispModelEntry;

DispModel *RgDispModelNextByKind(s32 type, DispModel *previous);
s32 func_00284F50(DispModelState *modelState, void *area, float scale);
void sbNativeHandlerMessagePost(s32 a, s32 b, s32 key);
DispModelEntry *RgDispModelFindByKey(s32);

/* Update callback of spawnMotionActorCreate objects: scans kind-7 slot objects and starts func_00284F50 probes, scale 5.0 when flag 0x100 is set. */
void spawnMotionActorUpdate(SpawnActor *spawnActor) {
    DispModelTail *modelTail;
    DispModel *displayModel;
    DispModel *ownerModel;
    MotionRecord *motionState;
    float scale;

    displayModel = 0;
    motionState = &spawnActor->state.spawned.motion;
    ownerModel = RgDispModelFindByKey(0);
    storeValueWithSceneBit(motionState);
    if ((*(GameLoopState *) gGameLoopRuntimeStorage).loopFlags & 0x800) {
        return;
    }
    if (spawnActor->flags & 0x800) {
        return;
    }
    for (; (displayModel = RgDispModelNextByKind(7, displayModel)) != 0;) {
        if (displayModel == ownerModel) {
            modelTail = &displayModel->tail;
            if (!(displayModel->flags & 0x400000)) {
                continue;
            }
            if (modelTail->unk_08 != 0) {
                continue;
            }
        }
        if (displayModel->flags & 0x100) {
            scale = 5.0f;
        } else {
            scale = 1.0f;
        }
        if (func_00284F50(displayModel->modelState, displayModel->unk_50, scale) != 0) {
            if (displayModel == ownerModel) {
                displayModel->unk_08 |= 2;
            }
            sbNativeHandlerMessagePost(1, displayModel->unk_28, spawnActor->key);
        }
    }
}

typedef struct StateBitTable {
    unsigned char unmodeled_000[0x310];
    unsigned int bits[1]; /* indexed by bit index >> 5; longer in the original */
} StateBitTable;

typedef struct SpawnMatrixTable {
    unsigned char unmodeled_00[0x68];
    s32 matricesOffset; /* +0x68: byte offset of the matrix array from the table */
} SpawnMatrixTable;

typedef struct StateBlock {
    unsigned char unmodeled_000[0x830];
    StateBitTable *bitTable; /* +0x830 */
    unsigned char unmodeled_834[0xC94 - 0x834];
    s32 displayCount;                /* +0xC94 */
    unsigned char *records;          /* +0xC98: 112-byte records */
    SpawnMatrixTable *matrixTable;   /* +0xC9C: 64-byte matrices */
} StateBlock;

typedef struct SpawnDecodeOut {
    s32 modelKind;
    s32 unmodeled_04[3];
    SpawnVector4 extents;
    enum SpawnDecodeWordKind {
        SPAWN_DECODE_WORD_NONE,
        SPAWN_DECODE_WORD_WIDTH = 0x7fffffff
    } unk_20, unk_24;
    s32 unk_28;
    s32 unk_2C;
} SpawnDecodeOut;

void spawnDisplayRecordUpdate(SpawnActor *object);

typedef struct BitIndexedObject {
    unsigned char unmodeled_000[0x150];
    s32 bitIndex; /* +0x150 */
} BitIndexedObject;

/* Release callback of spawnDisplayRecordBind objects: sets the display bit of the object in the state block bit table. */
void spawnDisplayRecordRelease(BitIndexedObject *obj);
s32 actorExtentShapeFromName(SpawnDecodeOut *, unsigned char *);
SpawnModel *func_0028D670();
void RgDispModelSetLocal(void *);
void MMathCalcAngleMatrixZ(void *, void *);
void MMathRotateMatrixY(void *, void *, f32);
void MMathRotateMatrixX(void *, void *, f32);
s32 func_0027A7E8(DispModelState *);
void slotSetValueSelector(s32, s32);
void func_0027AB00(s32);
void func_0027B1C8(s32);

void dispModelBindValue(DispModel *dispModel, s32 bindValue);
StateBlock *stateBlockGet();

static inline void spawnVectorSetOne(SpawnVector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void spawnVectorZero(SpawnVector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

static inline void spawnVectorCopy(SpawnVector4 *destination, SpawnVector4 *source) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(destination), "r"(source) : "$8", "memory");
}

static inline void spawnMatrixCopyTranslation(SpawnVector4 *destination, SpawnMatrix4 *source) {
    __asm__ __volatile__("lqc2 $vf1, 0x30(%5)\n vmove.w $vf1,$vf0\n sqc2 $vf1, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source));
}

static inline void spawnMatrixCopy(SpawnMatrix4 *destination, SpawnMatrix4 *source) {
    __asm__ __volatile__(
        "lq $8,0(%1)\n lq $9,0x10(%1)\n lq $10,0x20(%1)\n lq $11,0x30(%1)\n sq $8,0(%0)\n sq $9,0x10(%0)\n sq $10,0x20(%0)\n sq $11,0x30(%0)"
        :
        : "r"(destination), "r"(source)
        : "$8", "$9", "$10", "$11", "memory");
}

typedef struct SpawnSlotRequest {
    s32 kind;         /* kind of the slot object to allocate */
    s32 displayIndex; /* index into the display records */
} SpawnSlotRequest;

/* Binds display record bindRequest->displayIndex to a new kind-0x10 slot object: copies its matrix, creates the actor model and clears the record bit. */
int spawnDisplayRecordBind(SpawnSlotRequest *bindRequest) {
    SpawnDecodeOut decoded;
    StateBlock *stateBlock = stateBlockGet();
    SpawnActor *spawnActor;
    SpawnModel *model;
    if (bindRequest->displayIndex < 0 || bindRequest->displayIndex >= stateBlock->displayCount) {
        return 0;
    }
    spawnActor = RgDispModelAlloc(bindRequest->kind);
    if (spawnActor == 0) {
        return 0;
    }
    {
        SpawnActorState *boundState = &spawnActor->state.bound;
        spawnActor->key = bindRequest->kind;
        spawnActor->flags |= 0x210000;
        spawnActor->kind = 0x10;
        spawnActor->update = spawnDisplayRecordUpdate;
        spawnActor->release = spawnDisplayRecordRelease;
        boundState->displayIndex = bindRequest->displayIndex;
        boundState->record = stateBlock->records + boundState->displayIndex * 112;
        boundState->sourceMatrix = (SpawnMatrix4 *) ((u8 *) stateBlock->matrixTable + stateBlock->matrixTable->matricesOffset) + bindRequest->displayIndex;
        spawnMatrixCopy(&boundState->matrix, boundState->sourceMatrix);
        actorExtentShapeFromName(&decoded, boundState->record);
        if (decoded.unk_2C == 0) {
            spawnActor->flags &= ~0x210000;
        }
        boundState->unk_50 = decoded.unk_20;
        boundState->unk_54 = decoded.unk_24;
        boundState->unk_58 = decoded.unk_28;
        model = func_0028D670();
        spawnActor->actor = model;
        if (model == 0) {
            RgDispModelSetLocal(spawnActor);
            return 0;
        }
        spawnVectorZero(&model->extents.vector);
        model->modelKind = decoded.modelKind;
        spawnVectorCopy(&model->extents.vector, &decoded.extents);
        spawnMatrixCopyTranslation(&model->position, boundState->sourceMatrix);
        MMathCalcAngleMatrixZ(&model->rotation, boundState->sourceMatrix);
        spawnVectorSetOne(&model->scale);
        spawnVectorCopy(&spawnActor->position, &model->position);
        spawnVectorCopy(&spawnActor->rotation, &model->rotation);
        spawnVectorCopy(&spawnActor->scale, &model->scale);
        MMathRotateMatrixY(&boundState->matrix, &boundState->matrix, -model->rotation.y);
        MMathRotateMatrixX(&boundState->matrix, &boundState->matrix, -model->rotation.x);
        stateBlock->bitTable->bits[boundState->displayIndex >> 5] &= ~(1 << (boundState->displayIndex & 0x1f));
        dispModelBindValue(spawnActor, -1);
        spawnActor->actorHandle = func_0027A7E8(model);
        slotSetValueSelector(spawnActor->actorHandle, 0);
        func_0027AB00(spawnActor->actorHandle);
        if (decoded.unk_2C == 0) {
            func_0027B1C8(spawnActor->actorHandle);
        }
        return 1;
    }
}

extern unsigned char gDispModelProbeDelta[];
s32 func_002A6FA8();
void func_002A70E8();
void func_002AA490();

void dispModelBindValue(DispModel *dispModel, s32 bindValue) {
    s32 probePosition[4];
    f32 scale;
    void *collision;
    s32 allocatedHandle;
    DispModelState *modelState;

    dispModel->bindValue = bindValue;
    modelState = dispModel->modelState;
    collision = modelState->collision;
    if (modelState != 0) {
        func_002A70E8(collision);
        modelState->mode6B3 = 3;
        modelState->counter6AA = 0;
        allocatedHandle = func_002A6FA8(0x100, dispModel->bindValue);
        scale = modelState->scale;
        modelState->allocatedHandle = allocatedHandle;
        modelState->savedScale = scale;
        func_002AA490(&probePosition[0], gDispModelProbeDelta, collision);
    }
}

void objectSetDefaults(void *);
void func_0027AC90(s32);

void dispModelDetachEntry(DispModel *dispModel) {
    s32 displayEntryKey;
    DispModelEntry *displayEntry;

    displayEntryKey = dispModel->displayEntryKey;
    if (displayEntryKey != 0) {
        displayEntry = RgDispModelFindByKey(displayEntryKey);
        if ((displayEntry != 0) && (displayEntry->kind == 0x200) && (displayEntry->flags & 0x10)) {
            func_0027AC90(dispModel->drawHandle);
            dispModel->drawHandle = -1;
            RgDispModelSetLocal(dispModel);
            objectSetDefaults(displayEntry);
        }
    }
}

/* Release callback of spawnDisplayRecordBind objects: sets the display bit of the object in the state block bit table. */
void spawnDisplayRecordRelease(BitIndexedObject *bitIndexedActor) {
    StateBlock *stateBlock;
    s32 *indexPtr;
    s32 index;

    indexPtr = &bitIndexedActor->bitIndex;
    stateBlock = stateBlockGet();
    index = *indexPtr;
    stateBlock->bitTable->bits[index >> 5] |= 1 << (index & 0x1f);
}

f32 func_0027DEF0(SpawnModel *actor);
s32 slotFollowTargetHeight(SpawnActor *object, f32 scale);
void SpawnSlotSetPosition(void *owner, void *block);
s32 func_001C1890(SpawnActor *object);
void vibrationModeSet(s32 kind);
void slotSceneTickSubmit(SpawnActor *object, s32 handle);
void seVoiceStartDefault(unsigned int handle);

/* Update callback of spawnDisplayRecordBind objects: counts down the voice timer and releases the effect, voice and draw handles. */
void spawnDisplayRecordUpdate(SpawnActor *spawnActor) {
    SpawnEffectSlot *effectSlot;
    SpawnModel *actor;
    DispModelEntry *displayEntry;
    f32 modelScale;
    s32 actorFlags;
    s32 sefEffectHandle;

    actor = spawnActor->actor;
    effectSlot = &spawnActor->state.effects;
    if (spawnActor->ownerModel != 0) {
        return;
    }
    modelScale = func_0027DEF0(actor);
    if (slotFollowTargetHeight(spawnActor, modelScale) != 0) {
        SpawnSlotSetPosition(spawnActor, &actor->position);
    }
    actorFlags = spawnActor->flags;
    if (actorFlags & 0x20) {
        effectSlot->voiceCountdown -= 1;
        if (effectSlot->voiceCountdown <= 0) {
            if ((actorFlags & 0x800) == 0) {
                sbNativeHandlerMessagePost(0, spawnActor->key, 0);
                actorFlags = spawnActor->flags;
            }
            if (actorFlags & 0x800000) {
                spawnActor->flags = actorFlags & ~0x20;
                if ((spawnActor->stateFlags & 1) == 0) {
                    vibrationModeSet(func_001C1890(spawnActor) == 3 ? 1 : 2);
                }
            } else {
                sefEffectHandle = effectSlot->sefEffectHandle;
                spawnActor->flags = actorFlags & ~0x200000;
                if (sefEffectHandle != 0) {
                    slotSceneTickSubmit(spawnActor, sefEffectHandle);
                }
                if (effectSlot->voiceHandle != 0) {
                    seVoiceStartDefault(effectSlot->voiceHandle);
                }
                if ((spawnActor->stateFlags & 1) == 0) {
                    vibrationModeSet(func_001C1890(spawnActor) == 3 ? 1 : 2);
                }
                if (spawnActor->drawHandle != 0) {
                    displayEntry = RgDispModelFindByKey(spawnActor->drawHandle);
                    if (displayEntry != 0) {
                        displayEntry->ownerFlags |= 0x40;
                        SpawnSlotSetPosition(displayEntry, &spawnActor->position);
                    }
                    spawnActor->drawHandle = 0;
                }
                RgDispModelSetLocal(spawnActor);
            }
        }
    }
}
