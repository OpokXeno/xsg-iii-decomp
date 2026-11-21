#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1290);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1320);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1388);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1548);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1590);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1638);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C17D0);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1890);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C18C8);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C19A0);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1A58);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1A88);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1AB8);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1AE8);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1B60);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1C30);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1C88);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
s32 func_002A6FA8();
void func_002A70E8(unsigned char *);

/* The block at +0x150 of the owner: a handle at +0x08 (owner +0x158) and one at +0x1C (owner +0x16C). */
typedef struct PlaybackHandles {
    unsigned char unk_00[0x8];
    s32 playingHandle;                   /* +0x08 */
    unsigned char unk_0C[0x1C - 0x0C];
    s32 pendingHandle;                   /* +0x1C */
} PlaybackHandles;

/* Owner of the playback state: the state pointer at +0x24, the value at +0x2C, the block at +0x150. */
typedef struct PlaybackOwner {
    unsigned char unk_00[0x24];
    struct PlaybackState *playbackState;       /* 0x24 */
    unsigned char unk_28[0x4];
    s32 resourceId;                      /* 0x2C */
    unsigned char unk_30[0x150 - 0x30];
    PlaybackHandles effectHandles;               /* 0x150 */
} PlaybackOwner;

/* Playback state (the object of +0x24): float pairs at +0x10/+0x1C, flags and handle at +0x6AA..+0x6BC. */
typedef struct PlaybackState {
    unsigned char unk_00[0x10];
    f32 modeTwoRate;
    unsigned char unk_14[0x8];
    f32 modeThreeRate;
    unsigned char unk_20[0x6A0 - 0x20];
    unsigned char playbackRestartBlock[0x6AA - 0x6A0];
    s16 playbackFlags;
    unsigned char unk_6AC[0x6B3 - 0x6AC];
    s8 playbackMode;
    unsigned char unk_6B4[0x6BC - 0x6B4];
    s32 playerHandle;
    unsigned char unk_6C0[0x6CC - 0x6C0];
    f32 playRate;
} PlaybackState;

void playbackSetupWithoutFlags(PlaybackOwner *owner, s32 newResourceId) {
    f32 playRate;
    s32 playerHandle;
    s32 resourceId;
    PlaybackState *playbackState;

    resourceId = newResourceId;
    owner->resourceId = resourceId;
    playbackState = owner->playbackState;
    if (playbackState != 0) {
        func_002A70E8(playbackState->playbackRestartBlock);
        playbackState->playbackMode = 3;
        playbackState->playbackFlags = 0;
        playerHandle = func_002A6FA8(0x100, owner->resourceId);
        playRate = playbackState->modeThreeRate;
        playbackState->playRate = playRate;
        playbackState->playerHandle = playerHandle;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C1FE0);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C20A0);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C21B0);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C22A0);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2398);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2458);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2580);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2670);

void sefEffectHandleRelease(s32);

void playbackReleaseHandle(PlaybackOwner *owner) {
    s32 playerHandle;

    playerHandle = owner->effectHandles.playingHandle;
    if (playerHandle != 0) {
        sefEffectHandleRelease(playerHandle);
    }
}

s32 RgDispModelSetLocal(void *);
s32 func_001EA050(s32);

void playbackSetLocalIfIdle(PlaybackOwner *owner) {
    if (func_001EA050(owner->effectHandles.playingHandle) == 0) {
        RgDispModelSetLocal(owner);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C27C0);

void playbackNoOp(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2990);

void playbackSetupWithFlags(PlaybackOwner *owner, s32 newResourceId) {
    f32 playRate;
    s32 playerHandle;
    s32 resourceId;
    PlaybackState *playbackState;

    resourceId = newResourceId;
    owner->resourceId = resourceId;
    playbackState = owner->playbackState;
    if (playbackState != 0) {
        func_002A70E8(playbackState->playbackRestartBlock);
        playbackState->playbackFlags = 0x2112;
        playbackState->playbackMode = 2;
        playerHandle = func_002A6FA8(0x100, owner->resourceId);
        playRate = playbackState->modeTwoRate;
        playbackState->playRate = playRate;
        playbackState->playerHandle = playerHandle;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2D90);

void playbackReleaseBlock(PlaybackOwner *owner) {
    PlaybackHandles *effectHandles;
    s32 playerHandle;

    effectHandles = &owner->effectHandles;
    playerHandle = effectHandles->pendingHandle;
    if (playerHandle != 0) {
        sefEffectHandleRelease(playerHandle);
        effectHandles->pendingHandle = 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2E38);

INCLUDE_ASM("asm/main/nonmatchings/tu073", func_001C2F08);
