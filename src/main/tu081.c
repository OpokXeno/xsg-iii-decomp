#include "common.h"

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;

typedef struct OwnerObject {
    unsigned char unk00[0x24];
    struct OwnerStateBlock *emitterState;
    unsigned char unk28[0x4];
    s32 effectKind;                   /* +0x2C: the kind the effect was started with */
} OwnerObject;

typedef struct OwnerStateBlock {
    unsigned char unk00[0x1C];
    f32 baseScale;
    unsigned char unk20[0x6A0 - 0x20];
    unsigned char collisionState[0xA];
    s16 emitterFlags;
    unsigned char unk6AC[0x7];
    s8 emitterMode;
    unsigned char unk6B4[0x8];
    s32 effectHandle;
    unsigned char unk6C0[0xC];
    f32 currentScale;
} OwnerStateBlock;

s32 func_002A6FA8();
void func_002A70E8();

void emitterRestartWithKind(OwnerObject *owner, s32 newEffectKind) {
    s32 newEffectHandle;
    OwnerStateBlock *emitterState;

    owner->effectKind = newEffectKind;
    emitterState = owner->emitterState;
    if (emitterState != 0) {
        func_002A70E8(&emitterState->collisionState);
        emitterState->emitterMode = 3;
        emitterState->emitterFlags = 0;
        newEffectHandle = func_002A6FA8(0x100, owner->effectKind);
        emitterState->currentScale = emitterState->baseScale;
        emitterState->effectHandle = newEffectHandle;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu081", func_001C6A70);

INCLUDE_ASM("asm/main/nonmatchings/tu081", func_001C6B30);

INCLUDE_ASM("asm/main/nonmatchings/tu081", func_001C6BD0);
