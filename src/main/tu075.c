#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu075", func_001C3320);

typedef float f32;
typedef int s32;
typedef unsigned char u8;
/* The global scene block at gRgSceneRoot: its root pointer leads to a block with a float at +0xA4. */
typedef struct RgDispScene {
    unsigned char unk_00[0xA4];
    f32 unk_A4;
} RgDispScene;

/* The global scene block root at gRgSceneRoot: its pointer leads to a block with a float at +0xA4. */
extern RgDispScene *gRgSceneRoot[];

/* The per-model state block at +0x150 of the display-model slot. */
typedef struct RgDispModelState {
    unsigned char unk_00[0x2C];
    f32 threshold;                  /* +0x2C */
    s32 proximityCooldown;                    /* +0x30 */
} RgDispModelState;

/* The actor a display model drives: only the value at +0x54 is claimed. */
typedef struct RgDispActor {
    unsigned char unk_00[0x54];
    f32 unk_54;
} RgDispActor;

/* One display-model slot as this TU reads it (the table in rg_dispmodel.c, 0x230 bytes per slot). */
typedef struct RgDispModel {
    s32 modelBits;                      /* +0x00, bit 0x40 requests a table reset */
    unsigned char unk_04[0x18];
    u8 *table;                      /* +0x1C */
    unsigned char unk_20[0x4];
    union {                         /* +0x24: an actor id, or the actor pointer (same word) */
        s32 actorId;
        RgDispActor *actor;
    };
    unsigned char unk_28[0x18];
    f32 position[3];                /* +0x40 */
    unsigned char unk_4C[0x134 - 0x4C];
    f32 range;                      /* +0x134 */
    unsigned char unk_138[0x150 - 0x138];
    RgDispModelState state;         /* +0x150 */
} RgDispModel;

void RgDispModelCopyActorValue(RgDispModel *model) {
    f32 value;

    value = gRgSceneRoot[0]->unk_A4;
    model->actor->unk_54 = value;
}

INCLUDE_ASM("asm/main/nonmatchings/tu075", func_001C33D8);

INCLUDE_ASM("asm/main/nonmatchings/tu075", func_001C3490);

INCLUDE_ASM("asm/main/nonmatchings/tu075", func_001C34E0);

INCLUDE_ASM("asm/main/nonmatchings/tu075", func_001C3598);

f32 MMathCalcDistXZ(void *, void *);
s32 RgDispModelSetLocal(void *);
RgDispModel *func_001C0570(s32);
s32 func_001C0B38(void *, s32);
s32 func_001C1388(void *, f32);
s32 func_001C3320(void *);
void RgDispModelCopyActorValue(RgDispModel *model);
f32 func_0027DEF0(s32);

void RgDispModelUpdateProximity(RgDispModel *model)
{
  f32 dy;
  f32 threshold;
  s32 actorId;
  s32 cooldownFrames;
  RgDispModelState *proximityBlock;
  RgDispModel *reference;
  proximityBlock = &model->state;
  actorId = model->actorId;
  reference = func_001C0570(0);
  RgDispModelCopyActorValue(model);
  if (func_001C1388(model, func_0027DEF0(actorId)) != 0)
  {
    func_001C0B38(model, actorId + 0x20);
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

extern unsigned char func_001C3780[];

void RgDispModelResetTable(RgDispModel *model) {
    s32 modelBits;
    s32 remainingBits;

    modelBits = model->modelBits;
    if (modelBits & 0x40) {
        remainingBits = modelBits & ~0x48;
        model->modelBits = remainingBits;
        model->table = func_001C3780;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu075", func_001C3780);
