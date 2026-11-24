#include "common.h"

#include "shared.h"
#include "vu0_macro.h"
#include "main/xenoActor.h"
extern float D_003E481C[];
extern const AlignedVector4 MMathUnitVector;
void sefObjectSetStatePlay(void *effectObject);
void sefSpawnEffectFromRequest(void *request);

typedef int s32;

typedef struct RgEffectRequest {
    unsigned char unmodeled_00[0x2c];
    s32 effectId; /* +0x2c */
    unsigned char unmodeled_30[0x40 - 0x30];
    AlignedVector4 position; /* +0x40 */
    AlignedVector4 zeroVector; /* +0x50 */
    AlignedVector4 scale; /* +0x60 */
    unsigned char unmodeled_70[0x80 - 0x70];
} RgEffectRequest;

static inline void RgCopyQuadword(AlignedVector4 *storedVector, const AlignedVector4 *loadedVector) {
    __asm__ __volatile__("lq $8, 0(%1)\n"
                         "sq $8, 0(%0)\n"
                         :
                         : "r"(storedVector), "r"(loadedVector)
                         : "$8");
}

static inline void RgVu0SplatScalarXyz(AlignedVector4 *splatTarget, float splatScalar) {
    __asm__ __volatile__("mfc1 $8, %1\n"
                         "qmtc2 $8, vf1\n"
                         "vaddx.xyz vf2, vf0, vf1x\n"
                         "vmove.w vf2, vf0\n"
                         "sqc2 vf2, 0(%0)\n"
                         :
                         : "r"(splatTarget), "f"(splatScalar)
                         : "$8");
}

/* Starts an effect at the position (unit vector when null) with the scale constant D_003E481C[0]; returns through sefSpawnEffectFromRequest. */
void func_001BF848(s32 effectId, AlignedVector4 *spawnPosition) {
    RgEffectRequest effectRequest;
    const AlignedVector4 *effectPosition;

    sefObjectSetStatePlay(&effectRequest);
    effectRequest.effectId = effectId;
    effectPosition = spawnPosition;
    if (effectPosition == 0) {
        effectPosition = &MMathUnitVector;
    }
    RgCopyQuadword(&effectRequest.position, effectPosition);
    RgVu0SplatScalarXyz(&effectRequest.scale, D_003E481C[0]);
    sefSpawnEffectFromRequest(&effectRequest);
}

typedef unsigned char u8;

#define RG_DISPMODEL_SLOT_COUNT 255

/* Counter block of the slot at +0xD0: each timed move below runs a frame counter. */
typedef struct RgDispModelState {
    u32 moveFlags;  /* +0x00: bits 0, 2, 4 and 6 each run a counter */
    u16 framesLeft[4]; /* +0x04: frames left of the position, rotation, scale and alpha moves */
    unsigned char unmodeled_0c[0x10 - 0xC];
    Vector4 offsetPosition; /* +0x10 */
    Vector4 offsetRotation; /* +0x20 */
    Vector4 offsetScale;    /* +0x30 */
    float alphaStep;        /* +0x40 */
    float alphaTarget;      /* +0x44 */
    unsigned char unmodeled_48[0x50 - 0x48];
} RgDispModelState;

/* Playback state block at +0x150 of the slot. */
typedef struct RgDispPlayback {
    s32 effectHandleId; /* +0x00: passed to stagePartAlphaSet for the effect kinds */
    unsigned char unk_04[4];
    void *playbackObject; /* +0x08: a matrix for kinds 0x10 and 0x20000, an effect for 0x4000 */
} RgDispPlayback;

/* One display-model slot, 0x230 bytes, in the table at gRgDispModelTable (255 slots), allocated by RgDispModelAlloc. */
typedef struct RgDispModel {
    s32 modelFlags;     /* +0x00: bit 0 live, 2 updated this frame, 4 released */
    s32 previousModelFlags;      /* +0x04: modelFlags of the previous frame */
    s32 stateBits;      /* +0x08 */
    s32 previousStateBits;  /* +0x0C */
    u32 spawnKind;           /* +0x10: one bit per spawner kind, searched by RgDispModelNextByKind */
    struct RgDispModel *ownerModel; /* +0x14: the model that links to this slot */
    struct RgDispModel *partnerModel; /* +0x18 */
    void (*update)(struct RgDispModel *); /* +0x1C */
    void (*release)(void); /* +0x20 */
    XenoActor *actor;   /* +0x24 */
    s32 lookupKey;      /* +0x28: lookup key of RgDispModelAlloc / RgDispModelFindByKey */
    s32 id;             /* +0x2C: set to -1 by RgDispModelReset */
    s32 modelId;        /* +0x30: -1 when unset */
    unsigned char unk_34[0xC];
    AlignedVector4 position;     /* +0x40 */
    AlignedVector4 prevPosition; /* +0x50 */
    AlignedVector4 rotation;     /* +0x60 */
    AlignedVector4 prevRotation; /* +0x70 */
    AlignedVector4 scale;        /* +0x80 */
    AlignedVector4 prevScale;    /* +0x90 */
    AlignedVector4 linkedOffset; /* +0xA0 */
    AlignedVector4 vectorB0;     /* +0xB0 */
    AlignedVector4 effectOffset; /* +0xC0 */
    RgDispModelState moveState;      /* +0xD0 */
    s32 effectHandle;    /* +0x120 */
    s32 auxEffectHandle; /* +0x124 */
    float alpha;         /* +0x128 */
    unsigned char unk_12C[4];
    float unk_130;       /* +0x130 */
    float halfScale0;    /* +0x134 */
    float halfScale1;    /* +0x138 */
    s32 unk_13C;         /* +0x13C */
    s16 effectFramesLeft;          /* +0x140, signed in the update */
    s16 blinkFramesLeft;          /* +0x142 */
    unsigned char unk_144[2];
    s16 unk_146;         /* +0x146 */
    unsigned char unk_148[8];
    RgDispPlayback playback; /* +0x150 */
    unsigned char unk_15C[4];
    AlignedVector4 modelAngles; /* +0x160 */
    unsigned char unk_170[0x230 - 0x170];
} RgDispModel;

extern RgDispModel gRgDispModelTable[RG_DISPMODEL_SLOT_COUNT];

void *memset(void *clearedVector, int value, unsigned int size);
/* Scaffold data of rg_dispmodel, named by its splat label (no config/symbols name exists). */
extern int sefActiveCount;

/* Clears the active count and the whole resource slot table. */
void sefResetResourceTable(void)
{
    sefActiveCount = 0;
    memset(gRgDispModelTable, 0, sizeof(gRgDispModelTable));
}

static inline void rgCopyT0(AlignedVector4 *storedVector, AlignedVector4 *loadedVector) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(storedVector), "r"(loadedVector) : "$8", "memory");
}

static inline void rgCopyWithMemInputs(AlignedVector4 *storedVector, AlignedVector4 *loadedVector) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)"
                         :
                         : "r"(storedVector), "r"(loadedVector), "m"(loadedVector->x), "m"(loadedVector->y), "m"(loadedVector->z), "m"(loadedVector->w)
                         : "$8", "memory");
}

/* Latches the previous-frame copies of every live slot (position, rotation, scale) and clears the update bits. */
void func_001BF8F0() {
    RgDispModel *model;
    s32 modelIndex;
    model = gRgDispModelTable;
    for (modelIndex = 0; modelIndex < RG_DISPMODEL_SLOT_COUNT; modelIndex++, model++) {
        s32 modelFlags = model->modelFlags;
        if (modelFlags & 1) {
            model->previousModelFlags = modelFlags;
            model->previousStateBits = model->stateBits;
            rgCopyT0(&model->prevPosition, &model->position);
            rgCopyT0(&model->prevRotation, &model->rotation);
            rgCopyWithMemInputs(&model->prevScale, &model->scale);
            model->modelFlags = modelFlags & 0xf4fffffd;
        }
    }
}

void sefEffectHandleRelease(s32 handle);

/* Kept apart from XenoActor: the enum-typed draw flags decide the emitted order of the blink update. */
enum BlinkDrawFlags {
    BLINK_DRAW_FLAGS_NONE = 0,
    BLINK_DRAW_FLAGS_WIDTH = 0x7fffffff /* only forces an int-wide enum */
};

/* The actor fields the blink countdown of func_001BF970 touches. */
typedef struct BlinkActorView {
    unsigned char unmodeled_00[0x90];
    enum BlinkDrawFlags drawFlags; /* +0x90 */
    f32 alpha;                     /* +0x94 */
} BlinkActorView;

typedef struct RgDispLoopBlock {
    unsigned char unmodeled_000[0x16C];
    f32 scale16C; /* +0x16C */
} RgDispLoopBlock;

extern RgDispLoopBlock gGameLoopRuntimeStorage;
s32 slotModelTopPointGet(AlignedVector4 *out, RgDispModel *model);
void func_001EAA08(s32 effectHandle, AlignedVector4 *offset);

void RgDispModelRelease(RgDispModel *model);

static inline void RgDispCopyVector(AlignedVector4 *storedVector, AlignedVector4 *loadedVector) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(storedVector->x), "=m"(storedVector->y), "=m"(storedVector->z), "=m"(storedVector->w)
                         : "r"(storedVector), "r"(loadedVector)
                         : "$8");
}

static inline void RgDispZeroVector(AlignedVector4 *clearedVector) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)"
                         : "=m"(clearedVector->x), "=m"(clearedVector->y), "=m"(clearedVector->z), "=m"(clearedVector->w)
                         : "r"(clearedVector));
}

static inline void RgDispScaleVector(AlignedVector4 *scaleTarget, f32 scaleFactor) {
    __asm__ __volatile__(
        "mfc1 $8, %5\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%4)"
        : "=m"(scaleTarget->x), "=m"(scaleTarget->y), "=m"(scaleTarget->z), "=m"(scaleTarget->w)
        : "r"(scaleTarget), "f"(scaleFactor)
        : "$8");
}

static inline void RgDispAddVector(AlignedVector4 *sumVector, AlignedVector4 *firstTerm, AlignedVector4 *secondTerm) {
    __asm__ __volatile__("lqc2 $vf1,0(%1)\n lqc2 $vf2,0(%2)\n vadd.xyz $vf1,$vf1,$vf2\n sqc2 $vf1,0(%0)"
                         :
                         : "r"(sumVector), "r"(firstTerm), "r"(secondTerm)
                         : "memory");
}

void func_001BF970(void) {
    s32 modelIndex;
    RgDispModel *model;
    AlignedVector4 effectPosition;
    RgEffectRequest spawn;
    s32 effectMode;
    BlinkActorView *actor;
    model = gRgDispModelTable;
    for (modelIndex = 0; modelIndex < RG_DISPMODEL_SLOT_COUNT; modelIndex++, model++) {
        if (!(model->modelFlags & 1)) {
            continue;
        }
        if (model->modelFlags & 4) {
            RgDispModelRelease(model);
            sefActiveCount--;
            continue;
        }
        effectMode = model->modelFlags & 0x03000000;
        if (effectMode != (model->previousModelFlags & 0x03000000)) {
            if (model->effectHandle != 0) {
                sefEffectHandleRelease(model->effectHandle);
                model->effectHandle = 0;
            }
            if (effectMode != 0) {
                sefObjectSetStatePlay(&spawn);
                spawn.effectId = (effectMode & 0x02000000) ? 0x1006401E : 0x1006401F;
                RgDispCopyVector(&spawn.position, &model->position);
                RgDispZeroVector(&spawn.zeroVector);
                RgDispScaleVector(&spawn.scale, gGameLoopRuntimeStorage.scale16C);
                model->effectHandle = ((s32 (*)(RgEffectRequest *spawn)) sefSpawnEffectFromRequest)(&spawn);
            }
        }
        if (model->effectHandle != 0) {
            slotModelTopPointGet(&effectPosition, model);
            RgDispAddVector(&effectPosition, &effectPosition, &model->effectOffset);
            func_001EAA08(model->effectHandle, &effectPosition);
        }
        if ((model->modelFlags & 0x200) && (actor = (BlinkActorView *) model->actor) != 0) {
            if (--model->effectFramesLeft <= 0) {
                model->modelFlags &= ~0x200;
                actor->drawFlags &= ~0x10000;
                actor->alpha = 1.0f;
            } else if (--model->blinkFramesLeft <= 0) {
                actor->alpha = 0.5f;
                model->blinkFramesLeft = (s16) (model->effectFramesLeft < 46 ? (45 - model->effectFramesLeft) / 4 + 2 : 2);
            } else {
                actor->alpha = 1.0f;
            }
        }
    }
}

void XenoActorRelease(XenoActor *actor);
void func_0027AC90(s32 modelId);

/* Runs the release callback and frees the effects, model id and actor owned by the slot. */
void RgDispModelRelease(RgDispModel *model) {
    void (*callback)(void);
    s32 effectHandle;
    s32 auxEffectHandle;
    s32 modelId;
    XenoActor *actor;

    callback = model->release;
    if (callback != 0) {
        callback();
    }
    effectHandle = model->effectHandle;
    model->modelFlags &= ~1;
    if (effectHandle != 0) {
        sefEffectHandleRelease(effectHandle);
    }
    auxEffectHandle = model->auxEffectHandle;
    if (auxEffectHandle != 0) {
        sefEffectHandleRelease(auxEffectHandle);
    }
    modelId = model->modelId;
    if (modelId != -1) {
        func_0027AC90(modelId);
    }
    actor = model->actor;
    if (actor != 0) {
        XenoActorRelease(actor);
    }
}

/* Camera of the game loop; only the fields the model label code reads. */
typedef struct RgCamera {
    unsigned char unk_00[0x94];
    float depthBias; /* +0x94 */
    unsigned char unk_98[0xD0 - 0x98];
    unsigned char cameraViewBlock[0x10]; /* +0xD0: passed to MMathCalcDirVector */
} RgCamera;

typedef struct GameLoopRuntimeStorage {
    unsigned char unk_00[0x4];
    s32 labelFlags; /* +0x04, bits 0x10000 and 0x20000 tested by func_001BFC50 */
    unsigned char unk_08[0x28 - 0x08];
    RgCamera *camera; /* +0x28 */
    unsigned char unk_2C[0x30 - 0x2C];
    AlignedVector4 viewDirection; /* +0x30, read by the dot product */
    AlignedVector4 clipPoint;     /* +0x40, the Vector4 passed to the clip */
    unsigned char unk_50[0x16C - 0x50];
    float unk_16C; /* +0x16C, the float pool entry 91 used by RgDispModelReset */
} GameLoopRuntimeStorage;

void MMathCalcDirVector(AlignedVector4 *clearedVector, void *source, AlignedVector4 *extra);
float MMathVectorDotProduct(AlignedVector4 *left, AlignedVector4 *right);
s32 MMathRotTransPersClip(s32 *screen, RgCamera *camera, void *left, void *right);
void GameMpeg2Play(void *clearedVector, void *source, s32 actorId);
extern char D_00531520[];
extern char D_00531528[];
void xglFontDebugPrintf(s32 x, s32 y, void *text, ...);
extern float D_0054F3F8;

/* Debug labels: draws the slot lookup key and state bits at its projected screen position when the camera sees it. */
void func_001BFC50(RgDispModel *model) {
    AlignedVector4 direction;
    s32 projected[4];
    char labelText[16];
    float dot;
    s32 cellX;
    s32 cellY;
    s32 labelX;
    s32 labelY;
    GameLoopRuntimeStorage *loopRuntime = &gGameLoopRuntimeStorage;

    MMathCalcDirVector(&direction, loopRuntime->camera->cameraViewBlock, &model->position);
    dot = MMathVectorDotProduct(&loopRuntime->viewDirection, &direction);
    if (dot + loopRuntime->camera->depthBias * D_0054F3F8 < 1.0f) {
        return;
    }
    if (MMathRotTransPersClip(projected, loopRuntime->camera, &loopRuntime->clipPoint,
                              &model->position) != 0) {
        return;
    }
    cellX = projected[0] >> 5;
    cellY = projected[1] >> 5;
    labelX = cellX - 902;
    labelY = cellY - 916;
    if ((loopRuntime->labelFlags & 0x10000) != 0) {
        GameMpeg2Play(labelText, D_00531520, model->lookupKey);
        xglFontDebugPrintf(labelX, labelY, labelText);
    }
    if ((loopRuntime->labelFlags & 0x20000) != 0) {
        GameMpeg2Play(labelText, D_00531528, model->stateBits);
        xglFontDebugPrintf(cellX - 912, cellY - 911, labelText);
    }
}

void RgDispModelUpdate(RgDispModel *model);

#define RG_DISPMODEL_SLOT_SIZE 0x230

/* Updates every slot of the table once. */
void RgDispModelReleaseAll(void)
{
  s32 modelSlotIndex;
  RgDispModel *modelSlot;
  modelSlot = gRgDispModelTable;
  for (modelSlotIndex = 0; modelSlotIndex < RG_DISPMODEL_SLOT_COUNT; modelSlotIndex++)
  {
    RgDispModelUpdate(modelSlot);
    modelSlot++;
  }

}

void *MMathRotateMatrixYXZ(void *matrix, void *angles, void *rotation);
void MMathApplyMatrix(void *dst, void *matrix, void *src);
void MMathScaleMatrix(void *dst, void *src, void *scale);
void MMathTranslateMatrix(void *dst, void *src, void *offset);
void XenoActorWrapRotation(XenoActor *actor);

void func_001C0040(RgDispModel *self);
void SpawnSlotSetPosition(RgDispModel *model, void *transformed);
void slotLightSetBuild(RgDispModel *model);

/* Per-frame update of one live slot: copies the actor pose, runs the update callback and rebuilds the matrices. */
void RgDispModelUpdate(RgDispModel *model) {
    Matrix4 rotation;
    AlignedVector4 transformed;
    XenoActor *actor;
    RgDispModel *owner;
    void (*update)(RgDispModel *);
    s32 modelFlags;
    s32 flagsAfterMark;
    void *matrix;
    RgDispPlayback *playback;

    actor = model->actor;
    modelFlags = model->modelFlags;
    if ((modelFlags & 1) == 0) {
        return;
    }
    if ((modelFlags & 2) != 0) {
        return;
    }
    model->modelFlags = modelFlags | 2;
    modelFlags = model->modelFlags;
    if ((modelFlags & 4) != 0) {
        if (actor != 0) {
            actor->attr |= 8;
            actor->resetFlags |= 0x2000;
            actor->drawFlags |= 4;
        }
        return;
    }
    if (actor != 0) {
        actor->attr &= ~8;
    }
    flagsAfterMark = model->modelFlags;
    if ((flagsAfterMark & 8) != 0) {
        if ((flagsAfterMark & 0x40) == 0) {
            if (actor != 0) {
                actor->attr |= 8;
            }
            return;
        }
    }
    if (actor != 0) {
        if ((model->stateBits & 0x100000) != 0) {
            XenoActorWrapRotation(model->actor);
        }
    }
    owner = model->ownerModel;
    if (owner != 0) {
        RgDispModelUpdate(owner);
    }
    update = model->update;
    if (update != 0) {
        update(model);
    }
    func_001C0040(model);
    if (actor != 0) {
        RgCopyQuadword(&model->position, (AlignedVector4 *) &actor->position);
        RgCopyQuadword(&model->rotation, (AlignedVector4 *) &actor->rotation);
        RgCopyQuadword(&model->scale, (AlignedVector4 *) &actor->scale);
        slotLightSetBuild(model);
    }
    if (model->ownerModel != 0) {
        MMathRotateMatrixYXZ(rotation, 0, &model->ownerModel->rotation);
        Vu0MMathSetTranslation(rotation, &model->ownerModel->position);
        MMathApplyMatrix(&transformed, rotation, &model->linkedOffset);
        SpawnSlotSetPosition(model, &transformed);
    }
    switch (model->spawnKind) {
    case 0x10:
        playback = &model->playback;
        MMathRotateMatrixYXZ(playback->playbackObject, &model->modelAngles, &model->rotation);
        MMathScaleMatrix(playback->playbackObject, playback->playbackObject, &model->scale);
        matrix = playback->playbackObject;
        Vu0MMathSetTranslation(matrix, &model->position);
        MMathTranslateMatrix(matrix, matrix, &actor->vector60);
        break;
    case 0x20000:
        playback = &model->playback;
        MMathRotateMatrixYXZ(playback->playbackObject, &model->modelAngles, &model->rotation);
        MMathScaleMatrix(playback->playbackObject, playback->playbackObject, &model->scale);
        Vu0MMathSetTranslation(playback->playbackObject, &model->position);
        break;
    }
}

void SpawnSlotSetRotation(RgDispModel *model, Vector4 *vector);
void SpawnSlotSetScale(RgDispModel *model, Vector4 *vector);
void stagePartAlphaSet(s32 handle);
void linkedObjectSetFloat(void *linkedObject, float value);

/* Advances the timed position, rotation, scale and alpha moves of a slot by one frame each. */
void func_001C0040(RgDispModel *self) {
    RgDispModelState *moveState;
    s32 framesLeft;
    XenoActor *actor;
    Vector4 movedPosition;
    float alphaNow;
    s32 drawFlags;
    float alpha;

    moveState = &self->moveState;
    actor = self->actor;
    if (moveState->moveFlags & 0x1) {
        Vu0SefAddVectorXYZ(&movedPosition, actor ? &actor->position : &self->position, &moveState->offsetPosition);
        SpawnSlotSetPosition(self, &movedPosition);
        framesLeft = moveState->framesLeft[0] - 1;
        moveState->framesLeft[0] = framesLeft;
        if ((s16) framesLeft <= 0) {
            moveState->moveFlags &= ~0x1;
        }
    }
    if (moveState->moveFlags & 0x4) {
        Vu0SefAddVectorXYZ(&movedPosition, actor ? &actor->rotation : &self->rotation, &moveState->offsetRotation);
        SpawnSlotSetRotation(self, &movedPosition);
        framesLeft = moveState->framesLeft[1] - 1;
        moveState->framesLeft[1] = framesLeft;
        if ((s16) framesLeft <= 0) {
            moveState->moveFlags &= ~0x4;
        }
    }
    if (moveState->moveFlags & 0x10) {
        Vu0SefAddVectorXYZ(&movedPosition, actor ? &actor->scale : &self->scale, &moveState->offsetScale);
        SpawnSlotSetScale(self, &movedPosition);
        framesLeft = moveState->framesLeft[2] - 1;
        moveState->framesLeft[2] = framesLeft;
        if ((s16) framesLeft <= 0) {
            moveState->moveFlags &= ~0x10;
        }
    }
    if (moveState->moveFlags & 0x40) {
        framesLeft = moveState->framesLeft[3] - 1;
        moveState->framesLeft[3] = framesLeft;
        if ((s16) framesLeft <= 0) {
            moveState->moveFlags &= ~0x40;
        }
        if ((s16) moveState->framesLeft[3] <= 0) {
            alphaNow = moveState->alphaTarget;
        } else {
            alphaNow = self->alpha + moveState->alphaStep;
        }
        self->alpha = alphaNow;
        switch (self->spawnKind) {
        case 0x4000:
            if (self->playback.playbackObject != 0) {
                linkedObjectSetFloat(self->playback.playbackObject, alphaNow);
            }
            break;
        case 0x10:
        case 0x100:
        case 0x2000:
        case 0x20000:
            stagePartAlphaSet(self->playback.effectHandleId);
            break;
        default:
            if (actor != 0) {
                alpha = self->alpha;
                actor->alpha = alpha;
                if (0.999f <= alpha) {
                    drawFlags = actor->drawFlags;
                    drawFlags &= ~0x10000;
                } else {
                    drawFlags = actor->drawFlags;
                    drawFlags |= 0x10000;
                }
                actor->drawFlags = drawFlags;
            }
            break;
        }
    }
}

extern s32 rgDispModelIdCounter;

/* Next 16-bit model id counter value, tagged with 0x10000. */
s32 RgDispModelNextId(void) {
    s32 nextModelId;

    nextModelId = (rgDispModelIdCounter + 1) & 0xFFFF;
    rgDispModelIdCounter = nextModelId;
    return nextModelId | 0x10000;
}

void RgDispModelReset(RgDispModel *model);

/* Claims the first free slot probing from key % 255 in steps of 2; returns 0 when the table is full. */
RgDispModel *RgDispModelAlloc(s32 key) {
    RgDispModel *freeModel;
    s32 modelIndex;
    s32 slotsLeft;

    if (key > 0xFFFFFF) {
        key &= 0xFFFFFF;
    }
    slotsLeft = 255;
    modelIndex = key % 255;
    while (slotsLeft--) {
        freeModel = &(gRgDispModelTable)[modelIndex];
        if ((freeModel->modelFlags & 1) == 0) {
            sefActiveCount++;
            RgDispModelReset(freeModel);
            freeModel->lookupKey = key;
            return freeModel;
        }
        modelIndex = (modelIndex + 2) % 255;
    }
    return 0;
}

float *stateBlockGet(RgDispModel *model);

static inline void RgVu0ClearVector(AlignedVector4 *clearedVector) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)\n" : : "r"(clearedVector));
}

static inline void RgVu0SetOnesVector(AlignedVector4 *clearedVector) {
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w\n"
                         "sqc2 $vf1, 0(%0)\n"
                         :
                         : "r"(clearedVector));
}

/* Resets a slot: zeroes it, sets the ids to -1, unit scale vectors and the spawner scale constants. */
void RgDispModelReset(RgDispModel *model) {
    float *gameStateFloats;
    s32 unsetId;

    gameStateFloats = stateBlockGet(model);
    memset(model, 0, sizeof(RgDispModel));
    model->modelFlags = 1;
    unsetId = -1;
    model->id = unsetId;
    model->modelId = unsetId;
    RgVu0ClearVector(&model->position);
    RgVu0ClearVector(&model->prevPosition);
    RgVu0ClearVector(&model->rotation);
    RgVu0ClearVector(&model->prevRotation);
    RgVu0SetOnesVector(&model->scale);
    RgVu0SetOnesVector(&model->prevScale);
    RgVu0ClearVector(&model->linkedOffset);
    RgVu0ClearVector(&model->vectorB0);
    RgVu0ClearVector(&model->effectOffset);
    model->alpha = 1.0f;
    model->unk_146 = (s16) unsetId;
    model->unk_13C = 0x11111111;
    model->unk_130 = gameStateFloats[526];
    model->halfScale0 = gameStateFloats[527] * gGameLoopRuntimeStorage.scale16C;
    model->halfScale1 = gameStateFloats[528] * gGameLoopRuntimeStorage.scale16C;
}

void func_001C04D0(RgDispModel *model);

/* Releases a slot and everything linked to it; a null model is ignored. */
void RgDispModelSetLocal(RgDispModel *model) {
    if (model != 0) {
        func_001C04D0(model);
    }
}

void RgDispModelLink(RgDispModel *model, RgDispModel *partner);

/* Marks the slot released, unlinks its partner and releases every slot owned by it. */
void func_001C04D0(RgDispModel *model) {
    RgDispModel *childModel;
    s32 modelIndex;

    if (model->modelFlags & 1) {
        model->modelFlags |= 4;
    }
    if (model->partnerModel != 0) {
        childModel = model->partnerModel;
        RgDispModelLink(model, 0);
        RgDispModelSetLocal(childModel);
    }
    childModel = gRgDispModelTable;
    for (modelIndex = 0; modelIndex < RG_DISPMODEL_SLOT_COUNT; modelIndex++, childModel++) {
        if ((childModel->modelFlags & 1) && childModel->ownerModel == model) {
            func_001C04D0(childModel);
        }
    }
}

extern RgDispModel *D_003E47B0[];

/* Finds the live, unreleased slot with this lookup key (key 0 returns the table default D_003E47B0[0]). */
RgDispModel *RgDispModelFindByKey(s32 key) {
    RgDispModel *foundModel;
    s32 modelIndex;
    s32 slotsLeft;

    slotsLeft = 255;
    modelIndex = key % 255;
    if (key == 0) {
        return D_003E47B0[0];
    }
    while (slotsLeft--) {
        foundModel = &(gRgDispModelTable)[modelIndex];
        if ((foundModel->modelFlags & 1) != 0 && foundModel->lookupKey == key) {
            return ((foundModel->modelFlags & 4) == 0) ? foundModel : 0;
        }
        modelIndex = (modelIndex + 2) % 255;
    }
    return 0;
}

/* Finds the live, unreleased slot that owns this actor. */
RgDispModel *RgDispModelFindByActor(XenoActor *actor) {
    RgDispModel *modelSlot;
    s32 modelIndex;
    s32 liveFlags;

    if (actor == 0) {
        return 0;
    }
    modelSlot = gRgDispModelTable;
    for (modelIndex = 0; modelIndex < RG_DISPMODEL_SLOT_COUNT; modelIndex++) {
        liveFlags = modelSlot->modelFlags;
        if ((liveFlags & 1) != 0 && modelSlot->actor == actor) {
            return ((liveFlags & 4) == 0) ? modelSlot : 0;
        }
        modelSlot++;
    }
    return 0;
}

extern RgDispModel D_00581390[]; /* one past the end of gRgDispModelTable */

/* Next live, unreleased slot after previousModel (or from the table start) whose kind has a bit of kindMask. */
RgDispModel *RgDispModelNextByKind(s32 kindMask, RgDispModel *previousModel) {
    RgDispModel *modelSlot;
    RgDispModel *modelsEnd;
    s32 liveFlags;

    modelSlot = previousModel ? previousModel + 1 : gRgDispModelTable;
    modelsEnd = D_00581390;
    while (modelSlot != modelsEnd) {
        liveFlags = modelSlot->modelFlags;
        if ((liveFlags & 1) != 0 && (modelSlot->spawnKind & kindMask) != 0) {
            if ((liveFlags & 4) == 0) {
                return modelSlot;
            }
        }
        modelSlot++;
    }
    return 0;
}

/* Kept as a wrapper: the macro argument order decides the emitted address order. */
static inline void RgLinkedVectorSubtractTo(AlignedVector4 *difference, AlignedVector4 *minuend,
                                            AlignedVector4 *subtrahend) {
    Vu0SubVectorXYZ12(difference, minuend, subtrahend);
}

/* Links this slot to its owner model and stores the offset of its position from the owner position. */
void func_001C06F0(RgDispModel *model, RgDispModel *owner) {
    model->ownerModel = owner;
    if (owner != 0) {
        RgLinkedVectorSubtractTo(&model->linkedOffset, &model->position, &owner->position);
    }
}

/* Pairs two slots as partners (or unpairs when partner is null). */
void RgDispModelLink(RgDispModel *model, RgDispModel *partner) {
    RgDispModel *oldPartner;

    if (partner == 0) {
        oldPartner = model->partnerModel;
        if (oldPartner != 0) {
            model->partnerModel = 0;
            oldPartner->partnerModel = 0;
        }
    } else {
        if (model->partnerModel != 0) {
            RgDispModelLink(model, 0);
        }
        model->partnerModel = partner;
        partner->partnerModel = model;
    }
}
