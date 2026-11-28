#include "common.h"

typedef signed char s8;

typedef short s16;

typedef int s32;

typedef float f32;

typedef unsigned char u8;

typedef struct SpawnVector4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} SpawnVector4;

enum SpawnKind {
    SPAWN_KIND_NONE,
    SPAWN_KIND_WIDTH = 0x7fffffff
};

enum SpawnFlagsKind {
    SPAWN_FLAGS_NONE,
    SPAWN_FLAGS_WIDTH = 0x7fffffff
};

typedef struct SpawnModel {
    s32 unmodeled_00;
    s32 modelKind; /* +0x04 */
    u8 unmodeled_08[8];
    union {
        SpawnVector4 vector;
        struct {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        s32 words[4];
    } extents; /* +0x10: kept as a union, the views decide the emitted order */
    SpawnVector4 position; /* +0x20 */
    SpawnVector4 vector30;
    SpawnVector4 angle; /* +0x40 */
    SpawnVector4 rotation; /* +0x50 */
    SpawnVector4 vector60;
    SpawnVector4 vector70;
    SpawnVector4 scale; /* +0x80 */
    s32 drawFlags;
    u8 unmodeled_94[8];
    s16 unk_9C;
    s16 unk_9E;
    f32 unk_A0;
    u8 unmodeled_A4[0xf8 - 0xa4];
    s32 unk_F8;
    u8 unmodeled_FC[0x12c - 0xfc];
    f32 unk_12C;
    u8 unmodeled_130[0x2f0 - 0x130];
    s32 flags2F0;
    u8 unmodeled_2F4[0x4ec - 0x2f4];
    s32 flags4EC;
    u8 unmodeled_4F0[0x504 - 0x4f0];
    u8 *payload504;
    u8 *payload508;
    u8 *payload50C;
    void *unk_510;
    s32 child514;
    u8 unmodeled_518[0x528 - 0x518];
    void *unk_528;
    u8 unmodeled_52C[0x530 - 0x52c];
    s32 payload530;
    u8 unmodeled_534[0x6A0 - 0x534];
    u8 setupBlock[0xA];     /* +0x6A0: the block passed to func_002A70E8 */
    s16 emitterStartFlags;  /* +0x6AA */
    u8 unmodeled_6AC[0x7];
    s8 startModeId;         /* +0x6B3 */
    u8 unmodeled_6B4[0x8];
    s32 emitterEffectHandle; /* +0x6BC */
    u8 unmodeled_6C0[0xC];
    f32 emitterScale;       /* +0x6CC */
} SpawnModel;
typedef struct SpawnStateBasic {
    s32 value;
    s32 valueAux;
} SpawnStateBasic;

typedef struct SpawnStateEnumWords {
    s32 value;
    s32 valueAux;
    enum SpawnStateWordKind {
        SPAWN_STATE_WORD_NONE,
        SPAWN_STATE_WORD_WIDTH = 0x7fffffff
    } word08, word0C, word10;
} SpawnStateEnumWords;

typedef struct SpawnStateFull {
    s32 value;
    s32 valueAux;
    s32 word08;
    s32 word0C;
    s32 word10;
    s32 word14;
    f32 unk_18;
    f32 unk_1C;
    f32 unk_20;
    u8 unmodeled_24[0x90 - 0x24];
    s16 unk_90;
    s16 unk_92;
    s32 firstActor; /* +0x94: released by func_001C4A80 */
    s32 secondActor; /* +0x98 */
    SpawnModel *thirdActor; /* +0x9C: made by func_001C5070 */
} SpawnStateFull;

typedef struct EmitterSlotState {
    s32 emitterId; /* +0x00 */
    s32 unk_04;
    s32 sefEffectCountdown; /* +0x08 */
    s32 sefEffectHandle;    /* +0x0C */
    s32 voiceHandle;        /* +0x10 */
    s32 voiceCountdown;     /* +0x14 */
    f32 unk_18;
    f32 unk_1C;
    f32 unk_20;
    unsigned char unk_24[0x90 - 0x24];
    s16 unk90;
    s16 unk92;
    s32 firstActor; /* +0x94 */
    s32 secondActor; /* +0x98 */
    s32 unk_9C;
} EmitterSlotState;

typedef struct SpawnActor {
    enum SpawnFlagsKind flags;
    unsigned char unmodeled_04[0x08 - 0x04];
    s32 stateFlags; /* +0x08: bit 0 gates the slot routine */
    unsigned char unmodeled_0C[0x10 - 0x0C];
    enum SpawnKind kind; /* +0x10 */
    s32 ownerModel;      /* +0x14: non-zero stops the update */
    unsigned char unmodeled_18[0x1C - 0x18];
    void (*update)(); /* +0x1C */
    void (*release)(); /* +0x20 */
    SpawnModel *actor; /* +0x24 */
    s32 key;           /* +0x28 */
    s32 emitterKind; /* +0x2C */
    s32 actorHandle;   /* +0x30 */
    unsigned char unmodeled_34[0x40 - 0x34];
    SpawnVector4 position; /* +0x40 */
    SpawnVector4 vector50; /* +0x50 */
    SpawnVector4 rotation; /* +0x60 */
    SpawnVector4 vector70; /* +0x70 */
    unsigned char unmodeled_80[0x130 - 0x80];
    void *effectHandle; /* +0x130 */
    unsigned char unmodeled_134[0x150 - 0x134];
    union {
        SpawnStateBasic basic;
        SpawnStateEnumWords enumWords;
        SpawnStateFull full;
        EmitterSlotState emitterSlot;
    } state; /* +0x150: kept as a union, the spawners lay it out differently */
} SpawnActor;

extern unsigned char gEmitterModelTable[];

typedef struct SpawnParams {
    s32 key;
    s32 value;
} SpawnParams;

extern unsigned char gGameLoopRuntimeStorage[];
s32 func_001B9590(s32 value);
s32 requestNameMappedPathGet(s32 value);

void emitterSlotRelease(SpawnActor *owner);

void func_001C4368(SpawnActor *owner);
void emitterStartMode2(SpawnActor *owner, s32 emitterKind);
void streamBeginRecord(s32 handle, s32 a, s32 b, s32 c);
void streamFlushNoop(void);
void func_0027B450(s32 stream, s32 a, s32 b, s32 c);
void func_0027AB00(s32 stream);
SpawnModel *func_0028D670(void);
SpawnActor *RgDispModelAlloc(s32 key);
typedef struct StateBlockHeader {
    s32 unmodeled_00;
    s32 mode; /* +0x04 */
} StateBlockHeader;

typedef struct StateBlock {
    unsigned char unmodeled_000[0x310];
    StateBlockHeader header; /* +0x310 */
    unsigned char unmodeled_318[0xC74 - 0x318];
    f32 unk_C74; /* +0xC74: copied into the model field +0xA0 */
} StateBlock;

StateBlock *stateBlockGet(void);
void payloadSectionsLoad(SpawnModel *node, s32 value);
s32 func_001C0A48(SpawnModel *node, s32 value, s32 flag);
void RgDispModelSetLocal(SpawnActor *owner);
void func_0028D760(SpawnModel *node);
void XenoActorSetChannelValue(SpawnModel *node, f32 value);
void XenoActorSetMode(SpawnModel *node, s32 mode);
void XenoActorSetChannelValueB(SpawnModel *node, f32 value);
void XenoActorEnableChannelB(SpawnModel *node);
void XenoActorSetSelector(SpawnModel *node, s32 value);
s32 func_0027A728(SpawnModel *node);
void slotSetValueSelector(s32 value, s32 which);

static inline void spawnVectorSetOne(SpawnVector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void spawnVectorZeroOut(SpawnVector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

typedef struct GameLoopRuntimeView {
    union {
        unsigned long long flags64; /* +0x00: the 64-bit flag view */
        s32 flags;                  /* +0x00 */
    };
    u8 unmodeled_08[0x10C - 0x08];
    s16 unk_10C;
    s16 unk_10E;
    u8 unmodeled_110[0x16C - 0x110];
    f32 scaleFactor; /* +0x16C */
    u8 unmodeled_170[0x1F0 - 0x170];
    s32 unk_1F0; /* +0x1F0: bit 0x10 is tested */
    u8 unmodeled_1F4[0x270 - 0x1F4];
} GameLoopRuntimeView;

#define gGameLoopView ((GameLoopRuntimeView *) gGameLoopRuntimeStorage)

/* Creates the kind-2 emitter slot object from a spawn request and starts its actor model in start mode 2. */
s32 func_001C3FD0(SpawnParams *spawnParams) {
    StateBlock *stateBlock;
    StateBlockHeader *stateHeader;
    SpawnActor *spawnActor;
    SpawnModel *actorModel;
    SpawnStateBasic *spawnState;
    s32 valueKind;
    stateBlock = stateBlockGet();
    if (gGameLoopView->flags64 & 0x0400000001000000LL) {
        spawnParams->value = func_001B9590(spawnParams->value);
    }
    spawnActor = RgDispModelAlloc(spawnParams->key);
    if (spawnActor == 0) {
        return 0;
    }
    spawnActor->flags |= 0x10000;
    valueKind = spawnParams->value & 0x7FFF0000;
    if (valueKind == 0x02060000 || valueKind == 0x02070000) {
        spawnActor->flags |= 0x100;
    } else {
        spawnActor->flags |= 0x80;
    }
    spawnState = &spawnActor->state.basic;
    spawnActor->kind = 2;
    spawnActor->key = spawnParams->key;
    spawnState->value = spawnParams->value;
    spawnActor->update = func_001C4368;
    spawnActor->release = emitterSlotRelease;
    spawnActor->effectHandle = 0;
    spawnState->valueAux = requestNameMappedPathGet(spawnParams->value);
    actorModel = func_0028D670();
    spawnActor->actor = actorModel;
    if (actorModel == 0) {
        RgDispModelSetLocal(spawnActor);
        return 0;
    }
    actorModel->drawFlags |= 0x20008;
    if (!(gGameLoopView->unk_1F0 & 0x10)) {
        actorModel->drawFlags |= 0x4000;
    }
    stateHeader = &stateBlock->header;
    if (stateHeader != 0 && stateHeader->mode != 3) {
        actorModel->flags4EC |= 0x8000;
    }
    actorModel->unk_9C = 3;
    actorModel->unk_A0 = stateBlock->unk_C74;
    actorModel->unk_12C = 1.0f;
    actorModel->unk_F8 = 0;
    spawnVectorZeroOut(&actorModel->extents.vector);
    actorModel->modelKind = 0;
    actorModel->extents.x = actorModel->extents.w = (spawnActor->flags & 0x100) ? 4.5f : 0.25f;
    actorModel->extents.y = (spawnActor->flags & 0x100) ? 8.0f : 1.5f;
    spawnVectorZeroOut(&actorModel->position);
    spawnVectorZeroOut(&actorModel->rotation);
    spawnVectorSetOne(&actorModel->scale);
    payloadSectionsLoad(actorModel, spawnState->value);
    func_001C0A48(actorModel, spawnState->valueAux, 0);
    actorModel->unk_528 = gEmitterModelTable;
    func_0028D760(actorModel);
    XenoActorSetChannelValue(actorModel, 12.0f);
    XenoActorSetMode(actorModel, 0);
    XenoActorSetChannelValueB(actorModel, 12.0f);
    XenoActorEnableChannelB(actorModel);
    XenoActorSetSelector(actorModel, 0);
    emitterStartMode2(spawnActor, -1);
    spawnActor->actorHandle = func_0027A728(actorModel);
    streamBeginRecord(spawnActor->actorHandle, 0, 0, 1);
    streamFlushNoop();
    func_0027B450(spawnActor->actorHandle, 0, 7, 40);
    streamFlushNoop();
    slotSetValueSelector(spawnActor->actorHandle, 0);
    func_0027AB00(spawnActor->actorHandle);
    return 1;
}

s32 func_002A6FA8();
void func_002A70E8(unsigned char *block);

/* Starts the actor model effect in mode 2 (flags 0x2112) and records the owner emitter kind. */
void emitterStartMode2(SpawnActor *spawnActor, s32 emitterKind) {
    f32 startScale;
    s32 effectHandle;
    SpawnModel *emitterModel;

    spawnActor->emitterKind = emitterKind;
    emitterModel = spawnActor->actor;
    if (emitterModel != 0) {
        func_002A70E8(emitterModel->setupBlock);
        emitterModel->emitterStartFlags = 0x2112;
        emitterModel->startModeId = 2;
        effectHandle = func_002A6FA8(0x100, spawnActor->emitterKind);
        startScale = emitterModel->extents.x;
        emitterModel->emitterScale = startScale;
        emitterModel->emitterEffectHandle = effectHandle;
    }
}

s32 sefEffectHandleRelease(s32);

/* Release callback: frees the sef effect handle held by the slot. */
void emitterSlotRelease(SpawnActor *spawnActor) {
    s32 sefHandle;
    EmitterSlotState *sefEffectSlot;

    sefEffectSlot = &spawnActor->state.emitterSlot;
    sefHandle = sefEffectSlot->sefEffectHandle;
    if (sefHandle != 0) {
        sefEffectHandleRelease(sefHandle);
        sefEffectSlot->sefEffectHandle = 0;
    }
}

f32 slotGetScale(s32);
s32 slotFollowTargetHeight(SpawnActor *owner, f32 scale);
void sefLinkedObjectSetPosition(s32 handle, SpawnVector4 *work);
void func_0027B020(s32);
void slotStateFlagEdgeUpdate(SpawnActor *owner);
void scEffectObjectUpdate(SpawnActor *owner);

/* Update callback of mode 2 slots: counts down the sef effect timer and releases it when it expires. */
void func_001C4368(SpawnActor *spawnActor) {
    EmitterSlotState *effectSlot;
    s32 sefHandle;
    f32 slotScale;

    effectSlot = &spawnActor->state.emitterSlot;
    slotScale = slotGetScale(spawnActor->actorHandle);
    slotFollowTargetHeight(spawnActor, slotScale);
    if (spawnActor->flags & 0x2000) {
        sefLinkedObjectSetPosition(effectSlot->sefEffectHandle, &spawnActor->position);
        effectSlot->sefEffectCountdown -= 1;
        if (effectSlot->sefEffectCountdown <= 0) {
            sefHandle = effectSlot->sefEffectHandle;
            spawnActor->flags &= ~0x2000;
            if (sefHandle != 0) {
                sefEffectHandleRelease(sefHandle);
                effectSlot->sefEffectHandle = 0;
            }
            func_0027B020(spawnActor->actorHandle);
        }
    }
    slotStateFlagEdgeUpdate(spawnActor);
    scEffectObjectUpdate(spawnActor);
}

typedef struct SpawnDecodeOut {
    s32 modelKind;
    s32 unmodeled_04[3];
    SpawnVector4 v10;
    s32 word20;
    s32 word24;
    s32 word28;
    s32 word2C;
} SpawnDecodeOut;

s32 actorExtentShapeFromName();
void emitterStartMode3(SpawnActor *owner, s32 emitterKind);

void func_001C4620(SpawnActor *owner);

/* Kept as its own helper: merging it with spawnVectorSetOne renumbers the compiler's labels. */
static inline void spawnRecordSetOne(SpawnVector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void spawnVectorZero(SpawnVector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

static inline void spawnVectorCopy(SpawnVector4 *destination, SpawnVector4 *source) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(destination), "r"(source) : "$8", "memory");
}

/* Creates the kind-0x40 emitter slot object from a spawn request and starts its actor model in start mode 3. */
int func_001C4410(SpawnParams *spawnParams) {
    SpawnDecodeOut decodedRecord;
    SpawnActor *spawnActor = RgDispModelAlloc(spawnParams->key);
    StateBlockHeader *stateHeader;
    SpawnModel *actorModel;
    SpawnStateEnumWords *spawnState;
    if (spawnActor == 0) {
        return 0;
    }
    spawnState = &spawnActor->state.enumWords;
    spawnActor->kind = 0x40;
    spawnActor->key = spawnParams->key;
    spawnActor->update = func_001C4620;
    spawnState->value = spawnParams->value;
    spawnState->valueAux = 0;
    actorExtentShapeFromName(&decodedRecord, 0);
    spawnState->word08 = decodedRecord.word20;
    spawnState->word0C = decodedRecord.word24;
    spawnState->word10 = decodedRecord.word28;
    actorModel = func_0028D670();
    spawnActor->actor = actorModel;
    if (actorModel == 0) {
        RgDispModelSetLocal(spawnActor);
        return 0;
    }
    actorModel->drawFlags |= 8;
    stateHeader = &stateBlockGet()->header;
    if (stateHeader != 0 && stateHeader->mode != 3) {
        actorModel->flags4EC |= 0x8000;
    }
    spawnVectorZero(&actorModel->extents.vector);
    actorModel->modelKind = decodedRecord.modelKind;
    spawnVectorCopy(&actorModel->extents.vector, &decodedRecord.v10);
    spawnVectorZero(&actorModel->position);
    spawnVectorZero(&actorModel->rotation);
    spawnRecordSetOne(&actorModel->scale);
    payloadSectionsLoad(actorModel, spawnState->value);
    func_001C0A48(actorModel, spawnState->valueAux, 0);
    func_0028D760(actorModel);
    XenoActorSetChannelValue(actorModel, 12.0f);
    XenoActorSetMode(actorModel, 0);
    emitterStartMode3(spawnActor, -1);
    spawnActor->actorHandle = func_0027A728(actorModel);
    slotSetValueSelector(spawnActor->actorHandle, 0);
    return 1;
}

extern unsigned char gEmitterStartDelta[];
void func_002AA490();

/* Starts the actor model effect in mode 3 and probes its setup block. */
void emitterStartMode3(SpawnActor *spawnActor, s32 emitterKind) {
    s32 probePosition[4];
    f32 startScale;
    unsigned char *setupBlock;
    s32 effectHandle;
    SpawnModel *emitterModel;

    spawnActor->emitterKind = emitterKind;
    emitterModel = spawnActor->actor;
    setupBlock = emitterModel->setupBlock;
    if (emitterModel != 0) {
        func_002A70E8(setupBlock);
        emitterModel->startModeId = 3;
        emitterModel->emitterStartFlags = 0;
        effectHandle = func_002A6FA8(0x100, spawnActor->emitterKind);
        startScale = emitterModel->extents.w;
        emitterModel->emitterEffectHandle = effectHandle;
        emitterModel->emitterScale = startScale;
        func_002AA490(&probePosition[0], gEmitterStartDelta, setupBlock);
    }
}

f32 func_0027DEF0(SpawnModel *emitter);
void SpawnSlotSetPosition(SpawnActor *owner, SpawnVector4 *position);
void sbNativeHandlerMessagePost(s32 a, s32 b, s32 c);
s32 func_001C1890(SpawnActor *owner);
void vibrationModeSet(s32 kind);
void slotSceneTickSubmit(SpawnActor *owner, s32 handle);
void seVoiceStartDefault(unsigned int handle);

/* Update callback of mode 3 slots: counts down the voice timer, then releases the effect and voice handles. */
void func_001C4620(SpawnActor *spawnActor) {
    EmitterSlotState *effectSlot;
    SpawnModel *emitterModel;
    f32 slotScale;
    s32 actorFlags;
    s32 sefHandle;
    s32 stateKind;
    s32 slotState;

    effectSlot = &spawnActor->state.emitterSlot;
    emitterModel = spawnActor->actor;
    if (spawnActor->ownerModel != 0) {
        return;
    }
    slotScale = func_0027DEF0(emitterModel);
    if (slotFollowTargetHeight(spawnActor, slotScale) != 0) {
        SpawnSlotSetPosition(spawnActor, &emitterModel->position);
    }
    actorFlags = spawnActor->flags;
    if ((actorFlags & 0x20) == 0) {
        return;
    }
    effectSlot->voiceCountdown -= 1;
    if (effectSlot->voiceCountdown > 0) {
        return;
    }
    if ((actorFlags & 0x800) == 0) {
        sbNativeHandlerMessagePost(0, spawnActor->key, 0);
        actorFlags = spawnActor->flags;
    }
    if (actorFlags & 0x800000) {
        spawnActor->flags = actorFlags & ~0x20;
        if ((spawnActor->stateFlags & 1) == 0) {
            slotState = func_001C1890(spawnActor);
            if (slotState == 3) {
                stateKind = 1;
            } else {
                stateKind = 2;
            }
            vibrationModeSet(stateKind);
        }
    } else {
        sefHandle = effectSlot->sefEffectHandle;
        spawnActor->flags = actorFlags & ~0x200000;
        if (sefHandle != 0) {
            slotSceneTickSubmit(spawnActor, sefHandle);
        }
        if (effectSlot->voiceHandle != 0) {
            seVoiceStartDefault(effectSlot->voiceHandle);
        }
        if ((spawnActor->stateFlags & 1) == 0) {
            slotState = func_001C1890(spawnActor);
            if (slotState == 3) {
                stateKind = 1;
            } else {
                stateKind = 2;
            }
            vibrationModeSet(stateKind);
        }
        RgDispModelSetLocal(spawnActor);
    }
}

s32 *func_001B5C68(s32 kind);

void emitterSlotAttachActors(SpawnActor *owner);
void emitterStartMode1(SpawnActor *owner, s32 emitterKind);
void playerFrameUpdate(SpawnActor *owner);
void xenoActorReleaseSlots(void);
SpawnModel *func_001C5070(void);

s32 func_001B5D70(s32 kind);

/* Kept as its own helper: merging it with spawnVectorSetOne renumbers the compiler's labels. */
static inline void spawnActorSetOne(SpawnVector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

/* Kept as its own helper: merging it with spawnVectorZeroOut renumbers the compiler's labels. */
static inline void spawnActorZeroVector(SpawnVector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

/* Creates the kind-1 emitter slot object and starts its actor model in start mode 1. */
SpawnActor *func_001C4778(void) {
    StateBlock *stateBlock = stateBlockGet();
    SpawnActor *spawnActor = RgDispModelAlloc(0);
    SpawnModel *actorModel;
    s32 *tableEntry;
    SpawnStateFull *spawnState;
    if (spawnActor == 0) {
        return 0;
    }
    spawnActor->flags |= 0x410000;
    if (func_001B5D70(gGameLoopView->unk_10C)) {
        spawnActor->flags |= 0x100;
    }
    if (gGameLoopView->flags & 0x20) {
        spawnActor->flags |= 0x100000;
    }
    spawnActor->kind = 1;
    spawnActor->update = playerFrameUpdate;
    spawnActor->release = xenoActorReleaseSlots;
    spawnActor->effectHandle = 0;
    actorModel = func_0028D670();
    spawnActor->actor = actorModel;
    if (actorModel == 0) {
        RgDispModelSetLocal(spawnActor);
        return 0;
    }
    actorModel->drawFlags |= 8;
    actorModel->flags4EC |= 0x8000;
    actorModel->unk_9C = 1;
    actorModel->unk_A0 = stateBlock->unk_C74;
    actorModel->unk_12C = 1.0f;
    actorModel->unk_F8 = 0;
    spawnActorZeroVector(&actorModel->extents.vector);
    actorModel->modelKind = 0;
    actorModel->extents.vector.x = actorModel->extents.vector.w = (spawnActor->flags & 0x100) ? 4.5f : 0.64f;
    actorModel->extents.vector.y = (spawnActor->flags & 0x100) ? 8.0f : 1.5f;
    spawnActorZeroVector(&actorModel->position);
    spawnActorZeroVector(&actorModel->rotation);
    spawnActorSetOne(&actorModel->scale);
    if (gGameLoopView->flags & 1) {
        spawnActor->flags = (spawnActor->flags | 8) & 0xFFBEFFFF;
        return spawnActor;
    }
    payloadSectionsLoad(actorModel, gGameLoopView->unk_10E + 0xCF010100);
    func_001C0A48(actorModel, 0xCF010300, 1);
    spawnState = &spawnActor->state.full;
    actorModel->unk_528 = gEmitterModelTable;
    func_0028D760(actorModel);
    XenoActorSetChannelValue(actorModel, 12.0f);
    XenoActorSetMode(actorModel, 0);
    XenoActorSetChannelValueB(actorModel, 12.0f);
    XenoActorEnableChannelB(actorModel);
    XenoActorSetSelector(actorModel, 0);
    emitterStartMode1(spawnActor, -1);
    spawnState->value = gGameLoopView->unk_10C;
    tableEntry = func_001B5C68(gGameLoopView->unk_10C);
    spawnState->word08 = 0;
    spawnState->unk_18 = gGameLoopView->scaleFactor * 0.16666667f;
    spawnState->valueAux = *tableEntry;
    emitterSlotAttachActors(spawnActor);
    spawnState->thirdActor = func_001C5070();
    return spawnActor;
}

/* Starts the actor model effect in mode 1 (flags 0x2132). */
void emitterStartMode1(SpawnActor *spawnActor, s32 emitterKind) {
    f32 startScale;
    s32 effectHandle;
    SpawnModel *emitterModel;

    spawnActor->emitterKind = emitterKind;
    emitterModel = spawnActor->actor;
    if (emitterModel != 0) {
        func_002A70E8(emitterModel->setupBlock);
        emitterModel->emitterStartFlags = 0x2132;
        emitterModel->startModeId = 1;
        effectHandle = func_002A6FA8(0x100, spawnActor->emitterKind);
        startScale = emitterModel->extents.x;
        emitterModel->emitterScale = startScale;
        emitterModel->emitterEffectHandle = effectHandle;
    }
}

void func_001BBC68(SpawnActor *owner);
void pendingCountSet(s32 count);
void XenoActorRelease(s32 handle);
void XenoActorSetFirstWord(SpawnModel *node, s32 value);

static inline void spawnVectorSetUniform(SpawnVector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %5\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%4)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
        : "r"(destination), "f"(value)
        : "$8");
}

/* Resets a kind-1 slot object for the current game loop type: releases its actors and effect, then restarts mode 1. */
s32 func_001C4A80(SpawnActor *spawnActor) {
    SpawnStateFull *spawnState = &spawnActor->state.full;
    SpawnModel *actorModel = spawnActor->actor;
    s32 typeId = gGameLoopView->unk_10C;
    s32 variant = gGameLoopView->unk_10E;
    s32 oldFlags = spawnActor->flags;
    s32 *tableEntry;
    f32 zero;
    func_001BBC68(spawnActor);
    pendingCountSet(3);
    if (spawnState->firstActor != 0) {
        XenoActorRelease(spawnState->firstActor);
        spawnState->firstActor = 0;
    }
    if (spawnState->secondActor != 0) {
        XenoActorRelease(spawnState->secondActor);
        spawnState->secondActor = 0;
    }
    if (spawnState->word10 != 0) {
        sefEffectHandleRelease(spawnState->word10);
        spawnState->word10 = 0;
    }
    if (func_001B5D70(typeId)) {
        spawnActor->flags |= 0x100;
    } else {
        spawnActor->flags &= ~0x100;
    }
    spawnState->value = typeId;
    tableEntry = func_001B5C68(typeId);
    spawnState->word08 = 0;
    spawnState->unk_92 = 0;
    spawnState->unk_90 = 0;
    spawnState->valueAux = *tableEntry;
    zero = 0.0f;
    spawnState->unk_20 = zero;
    spawnState->unk_1C = zero;
    if (((spawnActor->flags ^ oldFlags) & 0x100)) {
        if (func_001B5D70(typeId)) {
            gGameLoopView->scaleFactor = 5.0f;
            actorModel->extents.vector.x = 4.5f;
            actorModel->extents.vector.y = 8.0f;
        } else {
            gGameLoopView->scaleFactor = 1.0f;
            actorModel->extents.vector.x = 0.64f;
            actorModel->extents.vector.y = 1.5f;
        }
        spawnState->unk_18 = gGameLoopView->scaleFactor * 0.16666667f;
    }
    payloadSectionsLoad(actorModel, variant + 0xCF010100);
    func_001C0A48(actorModel, 0xCF010300, 1);
    actorModel->unk_528 = gEmitterModelTable;
    func_0028D760(actorModel);
    XenoActorSetChannelValue(actorModel, 12.0f);
    XenoActorSetMode(actorModel, 0);
    XenoActorSetChannelValueB(actorModel, 1.0f);
    XenoActorEnableChannelB(actorModel);
    XenoActorSetSelector(actorModel, 0);
    XenoActorSetFirstWord(actorModel, 0x1e);
    actorModel->flags2F0 &= ~4;
    emitterStartMode1(spawnActor, spawnActor->emitterKind);
    emitterSlotAttachActors(spawnActor);
    if (spawnState->thirdActor != 0) {
        spawnVectorSetUniform(&spawnState->thirdActor->scale, gGameLoopView->scaleFactor);
    }
    return 0;
}

typedef struct EmitterCurve {
    unsigned char unk_00[0x40];
    s16 firstResourceKey;  /* +0x40 */
    s16 secondResourceKey; /* +0x42 */
} EmitterCurve;

EmitterCurve *func_A982F0(s32 key);

s32 emitterActorAttachChild(SpawnModel *owner, s32 key, s32 jointId, s32 mode);

/* Attaches the emitter model child actors listed by the emitter curve of the slot. */
void emitterSlotAttachActors(SpawnActor *spawnActor) {
    EmitterSlotState *emitterSlot;
    SpawnModel *emitterModel;
    EmitterCurve *slotCurve;

    emitterSlot = &spawnActor->state.emitterSlot;
    emitterModel = spawnActor->actor;
    slotCurve = func_A982F0(emitterSlot->emitterId);
    if (emitterSlot->emitterId < 13) {
        if (emitterSlot->emitterId == 5) {
            emitterSlot->secondActor = emitterActorAttachChild(emitterModel, slotCurve->secondResourceKey, 3, 5);
        } else {
            emitterSlot->firstActor = emitterActorAttachChild(emitterModel, slotCurve->firstResourceKey, 0, emitterSlot->emitterId);
            emitterSlot->secondActor = emitterActorAttachChild(emitterModel, slotCurve->secondResourceKey, 1, emitterSlot->emitterId);
        }
    }
}

/* One slot of a payload table: present flag and byte offset from the payload base. */
typedef struct PayloadEntry {
    s32 offset;  /* from the payload base */
    s32 present;
} PayloadEntry;

typedef struct Payload {
    PayloadEntry entries[4];
} Payload;

s32 gameOverSlotAt();
s32 func_001B6918();
void xcLocatePayload(Payload **payload, s32 base);
s32 XenoActorAttachChildToJoint(SpawnModel *actor, SpawnModel *child, s32 jointId);

/* Creates a child actor from a resource key, locates its payload table and attaches it to a joint of the parent. */
s32 emitterActorAttachChild(SpawnModel *parentModel, s32 resourceKey, s32 jointId, s32 mode) {
    Payload *payloadRef;
    Payload *payloadHeader;
    SpawnModel *childModel;
    s32 resourceBase;

    if (resourceKey == 0) {
        return 0;
    }
    resourceBase = func_001B6918(0, 0, *(s32 *) gameOverSlotAt(resourceKey), -1);
    if (resourceBase == 0) {
        return 0;
    }
    childModel = func_0028D670();
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&childModel->position) : "memory");
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&childModel->rotation) : "memory");
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n\tsqc2 vf1, 0(%0)" : : "r"(&childModel->scale) : "memory");
    xcLocatePayload(&payloadRef, resourceBase);
    payloadHeader = payloadRef;
    childModel->payload504 =
        payloadHeader->entries[0].present ? (unsigned char *) payloadHeader + payloadHeader->entries[0].offset : 0;
    childModel->payload508 =
        payloadHeader->entries[1].present ? (unsigned char *) payloadHeader + payloadHeader->entries[1].offset : 0;
    childModel->payload50C =
        payloadHeader->entries[2].present ? (unsigned char *) payloadHeader + payloadHeader->entries[2].offset : 0;
    childModel->payload530 =
        payloadHeader->entries[3].present ? (s32) payloadHeader + payloadHeader->entries[3].offset : 0;
    childModel->child514 = parentModel->child514;
    XenoActorAttachChildToJoint(parentModel, childModel, jointId);
    if (mode != 4 && mode != 6) {
        func_0028D760(childModel);
    }
    childModel->drawFlags |= 4;
    return (s32) childModel;
}

unsigned char gEmitterModelTable[32768] = { 0 };
unsigned char gFontBuffer[266240] = { 0 };
char D_003635C0[196608] = { 0 };
char D_003935C0[196608] = { 0 };
unsigned char gBufferArena[16384] = { 0 };
