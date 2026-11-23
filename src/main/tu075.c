#include "common.h"

typedef float f32;

typedef int s32;

typedef unsigned char u8;

/* Per-model state block at +0x150 of a display-model slot (rg_dispmodel.c, 0x230 bytes per slot). */
typedef struct RgDispModelState {
    unsigned char unk_00[0x08];
    s32 bitmapId; /* +0x08 */
    unsigned char unk_0C[0x10 - 0x0C];
    s32 playMode;      /* +0x10, compared with 10 */
    s32 rndLimit;  /* +0x14 */
    s32 rndOffset; /* +0x18 */
    s32 movieFlagByte;  /* +0x1C */
    unsigned char unk_20[0x24 - 0x20];
    s32 movieHandle; /* +0x24 */
    unsigned char unk_28[0x2C - 0x28];
    f32 threshold;         /* +0x2C */
    s32 proximityCooldown; /* +0x30 */
    f32 scaledThreshold;   /* +0x34, threshold * 0.08f */
} RgDispModelState;

typedef struct RgDispActor {
    unsigned char unk_00[0x24];
    union { /* +0x24: kept as a union: alias set 0 decides the emitted reloads */
        f32 offsetY;
        s32 offsetYBits;
    };
    unsigned char unk_28[0x54 - 0x28];
    f32 sceneValue; /* +0x54, copied from the scene block */
} RgDispActor;

typedef struct RgDispModel RgDispModel;

/* One display-model slot as this TU reads it. */
struct RgDispModel {
    s32 modelBits; /* +0x00, bit 0x40 requests a table reset */
    unsigned char unk_04[0x18];
    s32 updateFnAddr; /* +0x1C, address of the per-frame update function, kept as an integer word */
    unsigned char unk_20[0x4];
    union { /* +0x24: an actor id, or the actor pointer (same word) */
        s32 actorId;
        RgDispActor *actor;
    };
    s32 argAt28; /* +0x28, passed to sbNativeHandlerMessagePost */
    unsigned char unk_2C[0x40 - 0x2C];
    f32 position[3]; /* +0x40 */
    unsigned char unk_4C[0x12C - 0x4C];
    f32 effectScale; /* +0x12C */
    f32 effectAlpha; /* +0x130 */
    f32 range; /* +0x134 */
    unsigned char unk_138[0x150 - 0x138];
    RgDispModelState block; /* +0x150 */
};

/* The global scene block root at gRgSceneRoot: its pointer leads to a block with a float at +0xA4. */
typedef struct RgDispScene {
    unsigned char unk_00[0xA4];
    f32 sceneValue; /* +0xA4 */
} RgDispScene;

extern RgDispScene *gRgSceneRoot[];

s32 Get_Rnd_001B8CD0(s32, s32);
void func_001B8D08(s32, s32, s32);
void bitmapSecondSetBit(s32, s32);
void movieDecoderReset(s32, s32);
void movieSetFlagByte(s32, s32, s32);
void sbNativeHandlerMessagePost(s32, s32, s32);
void seVoiceStartDefault(unsigned int handle);
f32 func_0027DEF0(s32);
f32 MMathCalcDist(void *, void *);
f32 MMathCalcDistXZ(void *, void *);
s32 RgDispModelSetLocal(void *);
s32 SpawnSlotSetPosition(void *, s32);
s32 slotFollowTargetHeight(void *, f32);
RgDispModel *RgDispModelFindByKey(s32);
void stepRatioStart(RgDispModel *model, f32 value, s32 index);
void func_001C3320(RgDispModel *model);
void RgDispModelCopyActorValue(RgDispModel *model);
void func_001C34E0(RgDispModel *model);
void RgDispModelUpdateProximity(RgDispModel *model);
void func_001C3780(RgDispModel *model);

void func_001C3320(RgDispModel *model) {
    RgDispModelState *modelBlock;
    s32 rnd;

    modelBlock = &model->block;
    rnd = Get_Rnd_001B8CD0(modelBlock->playMode, modelBlock->rndLimit);
    func_001B8D08(modelBlock->playMode, modelBlock->rndLimit, rnd + modelBlock->rndOffset);
    bitmapSecondSetBit(modelBlock->bitmapId, 1);
    if (modelBlock->playMode == 10) {
        movieDecoderReset(modelBlock->movieHandle, modelBlock->rndOffset);
    } else {
        movieSetFlagByte(modelBlock->movieHandle, modelBlock->movieFlagByte, modelBlock->rndOffset);
    }
    sbNativeHandlerMessagePost(2, model->argAt28, modelBlock->bitmapId);
    seVoiceStartDefault(12);
}

void RgDispModelCopyActorValue(RgDispModel *model) {
    f32 sceneValue;

    sceneValue = gRgSceneRoot[0]->sceneValue;
    model->actor->sceneValue = sceneValue;
}

void func_001C33D8(RgDispModel *model) {
    RgDispModel *reference;
    s32 actorId;

    actorId = model->actorId;
    reference = RgDispModelFindByKey(0);
    RgDispModelCopyActorValue(model);
    if (slotFollowTargetHeight(model, func_0027DEF0(actorId)) != 0) {
        SpawnSlotSetPosition(model, actorId + 0x20);
    }
    if (!(model->range < MMathCalcDist(reference->position, model->position))) {
        func_001C3320(model);
        RgDispModelSetLocal(model);
    }
}

void func_001C3490(RgDispModel *model) {
    s32 modelBits;
    RgDispModelState *modelBlock;

    modelBits = model->modelBits;
    if (modelBits & 0x40) {
        modelBlock = &model->block;
        modelBlock->scaledThreshold = modelBlock->threshold * 0.08f;
        model->modelBits = modelBits & ~0x48;
        model->updateFnAddr = (s32) func_001C34E0;
        modelBlock->proximityCooldown = 75;
    }
}

void func_001C34E0(RgDispModel *model) {
    RgDispModelState *modelBlock;
    RgDispActor *actor;

    modelBlock = &model->block;
    actor = model->actor;
    modelBlock->proximityCooldown--;
    RgDispModelCopyActorValue(model);
    if (modelBlock->proximityCooldown < 56) {
        actor->offsetY += modelBlock->scaledThreshold;
        modelBlock->scaledThreshold = modelBlock->scaledThreshold * 0.9f;
        if (modelBlock->proximityCooldown == 48) {
            func_001C3320(model);
        }
        if (modelBlock->proximityCooldown == 10) {
            stepRatioStart(model, 0.0f, 9);
        }
        if (modelBlock->proximityCooldown < 0) {
            /* kept: the do/while(0) block decides the emitted jump layout */
            do {
                RgDispModelSetLocal(model);
            } while (0);
        }
    }
}

void func_001C3598(RgDispModel *model) {
    RgDispModelState *modelBlock;
    s32 modelBits;
    f32 liftStep;
    f32 scaled;

    modelBits = model->modelBits;
    if (modelBits & 0x40) {
        modelBlock = &model->block;
        liftStep = modelBlock->threshold * 0.2f;
        model->modelBits = (modelBits & ~0x48) | 0x11000;
        model->updateFnAddr = (s32) RgDispModelUpdateProximity;
        model->position[1] += liftStep;
        model->actor->offsetY += liftStep;
        modelBlock->proximityCooldown = 5;
        scaled = modelBlock->threshold * 0.27f;
        model->effectAlpha = 0.5f;
        model->effectScale = scaled;
    }
}

void RgDispModelUpdateProximity(RgDispModel *model)
{
  f32 dy;
  f32 threshold;
  s32 actorId;
  s32 cooldownFrames;
  RgDispModelState *proximityBlock;
  RgDispModel *reference;
  proximityBlock = &model->block;
  actorId = model->actorId;
  reference = RgDispModelFindByKey(0);
  RgDispModelCopyActorValue(model);
  if (slotFollowTargetHeight(model, func_0027DEF0(actorId)) != 0)
  {
    SpawnSlotSetPosition(model, actorId + 0x20);
  }
  cooldownFrames = proximityBlock->proximityCooldown;
  if (cooldownFrames != 0)
  {
    proximityBlock->proximityCooldown = cooldownFrames - 1;
  }
  else if (!(model->range < MMathCalcDistXZ(reference->position, model->position)))
  {
    threshold = proximityBlock->threshold;
    dy = model->position[1] - reference->position[1];
    if ((!(dy < (threshold * (-0.5f)))) && (!((2.0f * threshold) < dy)))
    {
      func_001C3320(model);
      RgDispModelSetLocal(model);
    }
  }
}

void RgDispModelResetTable(RgDispModel *model) {
    s32 modelBits;
    s32 remainingBits;

    modelBits = model->modelBits;
    if (modelBits & 0x40) {
        remainingBits = modelBits & ~0x48;
        model->modelBits = remainingBits;
        model->updateFnAddr = (s32) func_001C3780;
    }
}

void func_001C3780(RgDispModel *model) {
    RgDispModel *reference;
    s32 actorId;

    actorId = model->actorId;
    reference = RgDispModelFindByKey(0);
    RgDispModelCopyActorValue(model);
    if (slotFollowTargetHeight(model, func_0027DEF0(actorId)) != 0) {
        SpawnSlotSetPosition(model, actorId + 0x20);
    }
    if (!(model->range < MMathCalcDist(reference->position, model->position))) {
        func_001C3320(model);
        RgDispModelSetLocal(model);
    }
}
