#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C3FD0);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* The owner of an emitter: the emitter state at +0x24, the kind at +0x2C. */
typedef struct EmitterOwner {
    unsigned char unk00[0x24];
    struct EmitterState *emitter;       /* +0x24 */
    unsigned char unk28[0x2C - 0x28];
    s32 emitterKind;                           /* +0x2C */
} EmitterOwner;

/* The emitter state that the owner points at. */
typedef struct EmitterState {
    unsigned char unk00[0x10];
    f32 startScale;                        /* +0x10: copied to scale6CC by the mode 2 starts */
    unsigned char unk14[0x1C - 0x14];
    f32 startScaleModeThree;                        /* +0x1C: copied to scale6CC by the mode 3 start */
    unsigned char unk20[0x6A0 - 0x20];
    unsigned char setupBlock[0xA];          /* +0x6A0: the block passed to func_002A70E8 */
    s16 emitterStartFlags;                       /* +0x6AA */
    unsigned char unk6AC[0x7];
    s8 startModeId;                         /* +0x6B3 */
    unsigned char unk6B4[0x8];
    s32 effectHandle;                      /* +0x6BC */
    unsigned char unk6C0[0xC];
    f32 emitterScale;                       /* +0x6CC */
} EmitterState;

/* The slot inside an owner (at +0x150 of the owner, given as an integer): its handle at +0x0C. */
typedef struct EmitterSlot {
    unsigned char unk00[0xC];
    s32 sefEffectHandle;                         /* +0x0C */
} EmitterSlot;

/* An owner whose EmitterSlot sits at +0x150 (the integer owner argument of emitterSlotRelease). */
typedef struct EmitterSlotOwner {
    unsigned char unk000[0x150];
    EmitterSlot sefEffectSlot;
} EmitterSlotOwner;

s32 func_002A6FA8();
void func_002A70E8(unsigned char *block);

void emitterStartMode2(EmitterOwner *owner, s32 emitterKind) {
    f32 scale;
    s32 effectHandle;
    EmitterState *emitter;

    owner->emitterKind = emitterKind;
    emitter = owner->emitter;
    if (emitter != 0) {
        func_002A70E8(emitter->setupBlock);
        emitter->emitterStartFlags = 0x2112;
        emitter->startModeId = 2;
        effectHandle = func_002A6FA8(0x100, owner->emitterKind);
        scale = emitter->startScale;
        emitter->emitterScale = scale;
        emitter->effectHandle = effectHandle;
    }
}

s32 sefEffectHandleRelease(s32);

void emitterSlotRelease(EmitterSlotOwner *owner) {
    s32 sefEffectHandle;
    EmitterSlot *sefEffectSlot;

    sefEffectSlot = &owner->sefEffectSlot;
    sefEffectHandle = sefEffectSlot->sefEffectHandle;
    if (sefEffectHandle != 0) {
        sefEffectHandleRelease(sefEffectHandle);
        sefEffectSlot->sefEffectHandle = 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4368);

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4410);

extern unsigned char gEmitterStartDelta[];
void func_002AA490();

void emitterStartMode3(EmitterOwner *owner, s32 emitterKind) {
    s32 position[4];
    f32 scale;
    unsigned char *setupBlockPtr;
    s32 effectHandle;
    EmitterState *emitter;

    owner->emitterKind = emitterKind;
    emitter = owner->emitter;
    setupBlockPtr = emitter->setupBlock;
    if (emitter != 0) {
        func_002A70E8(setupBlockPtr);
        emitter->startModeId = 3;
        emitter->emitterStartFlags = 0;
        effectHandle = func_002A6FA8(0x100, owner->emitterKind);
        scale = emitter->startScaleModeThree;
        emitter->effectHandle = effectHandle;
        emitter->emitterScale = scale;
        func_002AA490(&position[0], gEmitterStartDelta, setupBlockPtr);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4620);

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4778);

void emitterStartMode1(EmitterOwner *owner, s32 emitterKind) {
    f32 scale;
    s32 effectHandle;
    EmitterState *emitter;

    owner->emitterKind = emitterKind;
    emitter = owner->emitter;
    if (emitter != 0) {
        func_002A70E8(emitter->setupBlock);
        emitter->emitterStartFlags = 0x2132;
        emitter->startModeId = 1;
        effectHandle = func_002A6FA8(0x100, owner->emitterKind);
        scale = emitter->startScale;
        emitter->emitterScale = scale;
        emitter->effectHandle = effectHandle;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4A80);

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4CE0);

INCLUDE_ASM("asm/main/nonmatchings/tu077", func_001C4D80);
