#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BF848);

void *memset(void *destination, int value, unsigned int size);
/* Scaffold data of rg_dispmodel, named by its splat label (no config/symbols name exists). */
extern int sefActiveCount;
extern unsigned char D_0055E5C0[142800];

/* Clears the active count and the whole resource slot table. */
void sefResetResourceTable(void)
{
    sefActiveCount = 0;
    memset(D_0055E5C0, 0, sizeof(D_0055E5C0));
}

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BF8F0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BF970);

typedef int s32;
typedef unsigned char u8;
void XenoActorRelease(s32 actorId);
void sefEffectHandleRelease(s32 handle);
void func_0027AC90(s32 modelId);

/* One display-model slot, 0x230 bytes, in the table at D_0055E5C0 (255 slots). */
typedef struct RgDispModel {
    s32 modelFlags;                      /* +0x00, bit 0 is cleared on release */
    unsigned char unk_04[0x14];
    struct RgDispModel *partnerModel;       /* +0x18, partner slot */
    unsigned char unk_1C[0x4];
    void (*callback)(void);         /* +0x20 */
    s32 actorId;                    /* +0x24 */
    unsigned char unk_28[0x8];
    s32 modelId;                    /* +0x30, -1 when unset */
    unsigned char unk_34[0x120 - 0x34];
    s32 effectHandle;                 /* +0x120 */
    s32 auxEffectHandle;                  /* +0x124 */
    unsigned char unk_128[0x230 - 0x128];
} RgDispModel;

void RgDispModelRelease(RgDispModel *model) {
    void (*callback)(void);
    s32 effectHandle;
    s32 auxEffectHandle;
    s32 modelId;
    s32 actorId;

    callback = model->callback;
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
    actorId = model->actorId;
    if (actorId != 0) {
        XenoActorRelease(actorId);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BFC50);

void func_001BFDE8(RgDispModel *model);

#define RG_DISPMODEL_SLOT_SIZE 0x230
#define RG_DISPMODEL_SLOT_COUNT 255

void RgDispModelReleaseAll(void)
{
  s32 modelSlotIndex;
  RgDispModel *modelSlot;
  modelSlot = (RgDispModel *) D_0055E5C0;
  for (modelSlotIndex = 0; modelSlotIndex < RG_DISPMODEL_SLOT_COUNT; modelSlotIndex++)
  {
    func_001BFDE8(modelSlot);
    modelSlot++;
  }

}

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BFDE8);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0040);

extern s32 rgDispModelIdCounter;

s32 RgDispModelNextId(void) {
    s32 nextModelId;

    nextModelId = (rgDispModelIdCounter + 1) & 0xFFFF;
    rgDispModelIdCounter = nextModelId;
    return nextModelId | 0x10000;
}

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C02E0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C03B8);

void func_001C04D0();

void RgDispModelSetLocal(s32 localModeEnabled) {
    if (localModeEnabled != 0) {
        func_001C04D0();
    }
}

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C04D0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0570);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0628);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0688);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C06F0);

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
