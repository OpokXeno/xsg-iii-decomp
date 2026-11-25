#include "common.h"

#include "shared.h"
#include "quadword.h"
typedef signed char s8;

typedef short s16;

typedef int s32;

typedef float f32;

/* Parameter block of a spawner, at +0x150 of the emitter owner. */
typedef struct EmitterParams {
    unsigned short flags; /* +0x0: bit 0x1 set while the player is inside the emitter range */
    unsigned char unmodeled_02[0x4 - 0x2];
    unsigned int soundTableRow; /* +0x4: row of gEmitterSpawnTable (16 bytes each) and sound action selector */
    s32 voiceId;                /* +0x8: argument of seVoiceStartGuarded */
    unsigned char unmodeled_0C[0x10 - 0xC];
    s32 waitFrames;  /* +0x10: frames to wait before the voice step */
    f32 scale;       /* +0x14: flag-selected scale (1.0 or 5.0), broadcast into the spawn vector */
    s32 spawnCountdown; /* +0x18: counts down, reset to 30 */
    f32 rate;        /* +0x1C: scaled by gEmitterRateDecay each frame, clamped at gEmitterRateFloor */
} EmitterParams;

typedef struct EmitterState {
    unsigned char unmodeled_00[0x10];
    f32 scaleX; /* +0x10 */
    f32 scaleY; /* +0x14 */
    unsigned char unmodeled_18[0x4];
    f32 baseScale; /* +0x1C */
    unsigned char unmodeled_20[0x54 - 0x20];
    f32 accumulatedRate; /* +0x54: each frame gains params->rate */
    unsigned char unmodeled_58[0x80 - 0x58];
    AlignedVector4 scaleVector; /* +0x80: (s, s, s, 1.0) */
    unsigned char unmodeled_90[0x6A0 - 0x90];
    unsigned char collisionState[0xA];
    s16 emitterFlags;
    unsigned char unmodeled_6AC[0x7];
    s8 emitterMode;
    unsigned char unmodeled_6B4[0x8];
    s32 effectHandle;
    unsigned char unmodeled_6C0[0xC];
    f32 currentScale;
} EmitterState;

typedef struct EmitterOwner {
    s32 flags; /* +0x00: bit 0x100 follows the flag argument of emitterScaleSelect */
    unsigned char unmodeled_04[0x10 - 0x4];
    s32 typeKey; /* +0x10: 0x8000 selects the update in emitterScaleSelect */
    unsigned char unmodeled_14[0x24 - 0x14];
    EmitterState *emitterState; /* +0x24 */
    unsigned char unmodeled_28[0x4];
    s32 effectKind; /* +0x2C: the kind the effect was started with */
    unsigned char unmodeled_30[0x40 - 0x30];
    AlignedVector4 position; /* +0x40: copied into the spawn record */
    unsigned char unmodeled_50[0x60 - 0x50];
    AlignedVector4 rotation; /* +0x60: copied into the spawn record */
    unsigned char unmodeled_70[0x134 - 0x70];
    f32 halfScale; /* +0x134 */
    unsigned char unmodeled_138[0x150 - 0x138];
    EmitterParams params; /* +0x150 */
} EmitterOwner;

s32 func_002A6FA8();
void func_002A70E8();

/* Restarts the emitter's effect with a new kind: resets its collision state, mode and scale. */
void emitterRestartWithKind(EmitterOwner *owner, s32 newEffectKind) {
    s32 newEffectHandle;
    EmitterState *emitterState;

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

/* Broadcasts the scale into (s, s, s, 1.0) with VU0. */
static inline void SetVectorUniform(Vector4 *destination, f32 value) {
    __asm__ __volatile__("mfc1 $8, %5\n"
                         "qmtc2 $8, vf1\n"
                         "vaddx.xyz vf2, vf0, vf1x\n"
                         "vmove.w vf2, vf0\n"
                         "sqc2 vf2, 0(%4)\n"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "f"(value)
                         : "$8");
}

/* Selects the small (1.0) or large (5.0) emitter scale; only acts on owners whose typeKey is 0x8000. */
int emitterScaleSelect(EmitterOwner *owner, s32 useLargeScale) {
    EmitterParams *params = &owner->params;
    EmitterState *emitterState = owner->emitterState;

    if (owner->typeKey != 0x8000) {
        return 0;
    }
    if (useLargeScale == 0) {
        params->scale = 1.0f;
    } else {
        params->scale = 5.0f;
    }
    owner->halfScale = params->scale * 0.5f;
    emitterState->scaleX = emitterState->scaleY = emitterState->baseScale = params->scale * 0.5f;
    SetVectorUniform(&emitterState->scaleVector, params->scale);
    if (useLargeScale == 0) {
        owner->flags &= ~0x100;
    } else {
        owner->flags |= 0x100;
    }
    return 1;
}

typedef struct EffectSpawn {
    unsigned char unmodeled_00[0x2C];
    u32 unknown2C; /* +0x2C: constant 0x10064025 */
    unsigned char unmodeled_30[0x40 - 0x30];
    AlignedVector4 position;
    AlignedVector4 rotation;
    AlignedVector4 scaleVector; /* (s, s, s, 1.0) */
    AlignedVector4 tableRow;
} EffectSpawn;

/* Sound table rows: each row is one (x, y, z, w) entry of the effect spawn table. */
const AlignedVector4 D_00531600[4] = {
    { 1.0f, 1.0f, 0.300000012f, 1.0f },
    { 0.200000003f, 1.0f, 0.300000012f, 1.0f },
    { 0.699999988f, 0.200000003f, 1.79999995f, 1.0f },
    { 0.300000012f, 0.600000024f, 1.79999995f, 1.0f },
};
void sefObjectSetStatePlay(EffectSpawn *effectObject);
void sefSpawnEffectFromRequest(EffectSpawn *effectObject);

/* Spawns one effect record from the owner's position, rotation and scale. */
void emitterSpawnEffectRecord(EmitterOwner *owner) {
    EffectSpawn spawn;
    EmitterParams *params;

    sefObjectSetStatePlay(&spawn);
    spawn.unknown2C = 0x10064025;
    params = &owner->params;
    __asm__ __volatile__("lq $8, 0(%1)\n"
                         "sq $8, 0(%0)\n"
                         :
                         : "r"(&spawn.position), "r"(&owner->position)
                         : "$8", "memory");
    QuadwordCopyT0(&spawn.rotation, &owner->rotation);
    __asm__ __volatile__("mfc1 $8, %1\n"
                         "qmtc2 $8, vf1\n"
                         "vaddx.xyz vf2, vf0, vf1x\n"
                         "vmove.w vf2, vf0\n"
                         "sqc2 vf2, 0(%0)\n"
                         :
                         : "r"(&spawn.scaleVector), "f"(params->scale)
                         : "$8", "memory");
    QuadwordCopyT0(&spawn.tableRow, &D_00531600[params->soundTableRow]);
    sefSpawnEffectFromRequest(&spawn);
}

typedef struct PlayerObject {
    s32 flags; /* +0x0: bit 0x400000 checked by emitterSoundUpdate */
    unsigned char unmodeled_04[0x40 - 0x4];
    AlignedVector4 position; /* +0x40: measured against the owner's position */
} PlayerObject;

extern s32 gGameLoopRuntimeStorage[];
extern unsigned short gPadDataPressed[];
PlayerObject *RgDispModelFindByKey(s32 mode);
f32 MMathCalcDist(const Vector4 *fromPoint, const Vector4 *toPoint);
void seVoiceStartDefault(unsigned int handle);
void func_00A83760(void);
s32 func_0025F5C0(s32 flag);
s32 soundDriverSetOutputMode(s32 mode);
s32 soundSlotSetLevelIfStill(s32 slot, s32 level);
s32 saveScreenRun(s32 mode);
void runStreamPlayback(void);
s32 seVoiceStartGuarded(s32 voice);

/* Per-frame update of a sound emitter: starts spawning while the player is in range, and plays the row's sound on a pad press. */
void emitterSoundUpdate(EmitterOwner *owner) {
    EmitterParams *params;
    PlayerObject *player;
    EmitterState *emitterState;

    params = &owner->params;
    player = RgDispModelFindByKey(0);
    if (owner->halfScale < MMathCalcDist(&player->position, &owner->position)) {
        params->flags &= 0xFFFE;
    } else {
        if ((params->flags & 1) == 0) {
            params->spawnCountdown = 0;
            params->rate = 0.6283185f;
            seVoiceStartDefault(0x19);
            if (params->soundTableRow == 0) {
                func_00A83760();
            }
        }
        params->flags |= 1;
    }
    if (params->flags & 1) {
        params->spawnCountdown -= 1;
        if (params->spawnCountdown <= 0) {
            emitterSpawnEffectRecord(owner);
            params->spawnCountdown = 0x1E;
        }
    }
    emitterState = owner->emitterState;
    emitterState->accumulatedRate += params->rate;
    params->rate = params->rate * 0.95f;
    if (params->rate < 0.10471975f) {
        params->rate = 0.10471975f;
    }
    if (params->waitFrames != 0) {
        params->waitFrames -= 1;
        return;
    }
    if ((gGameLoopRuntimeStorage[0] & 0x04000800) == 0 && (player->flags & 0x400000) != 0 &&
        (params->flags & 1) != 0 && (gPadDataPressed[0] & 0x20) != 0) {
        if (params->soundTableRow == 0 || params->soundTableRow == 3) {
            func_0025F5C0(1);
            soundDriverSetOutputMode(1);
            soundSlotSetLevelIfStill(1, 1);
        }
        switch (params->soundTableRow) {
        case 0:
            saveScreenRun(0);
            break;
        case 1:
            runStreamPlayback();
            break;
        case 3:
            seVoiceStartGuarded(params->voiceId);
            break;
        }
        if (params->soundTableRow == 0 || params->soundTableRow == 3) {
            func_0025F5C0(0);
            soundDriverSetOutputMode(0);
            soundSlotSetLevelIfStill(1, 0);
        }
    }
}
