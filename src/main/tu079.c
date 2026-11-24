#include "common.h"
#include "main/tu007.h"
#include "main/tu118.h"

#include "main/xenoActor.h"
#include "quadword.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef short s16;

struct SefEffectObject;
struct SefEffectObject;

typedef struct Matrix4Rows {
    Vector4 m[4];
} Matrix4Rows;

typedef struct ActorModel ActorModel;

/*
 * Pad-driven movement state of the player actor, at +0x150 of the actor record: the stick or d-pad deflection
 * (inputMagnitude) drives the walk animation mode, the footstep sound and the movement vector.
 */
typedef struct ActorSoundState {
    s32 checkKey; /* +0x00: first argument of keyedWordTableFind */
    unsigned char unmodeled_04[0x08 - 0x04];
    s32 mode; /* +0x08: movement mode, the switch value of playerFrameUpdate */
    u16 countdown; /* +0x0C: frames until the action ends */
    s16 soundResetHalf; /* +0x0E: frames until the next footstep cue */
    struct SefEffectObject *effectObject; /* +0x10: footstep effect object to release */
    f32 fadeLevel; /* +0x14: faded toward inputMagnitude by func_001C5250 */
    f32 moveSpeed; /* +0x18: top speed of the movement vector */
    f32 inputMagnitude; /* +0x1C: stick deflection, 0 when idle */
    f32 previousMagnitude; /* +0x20: inputMagnitude of the previous frame */
    unsigned char unmodeled_24[0x30 - 0x24];
    AlignedVector4 moveVector; /* +0x30 */
    f32 cameraYaw; /* +0x40 */
    f32 previousCameraYaw; /* +0x44 */
    unsigned char unmodeled_48[0x50 - 0x48];
    Matrix4Rows cameraMatrix; /* +0x50: yaw rotation of the camera */
    s16 requestedMode; /* +0x90: 0xC when the wait ends, 0 when it is cleared */
    s16 appliedMode; /* +0x92 */
    XenoActor *linkedA; /* +0x94: first linked actor, also copied into the sefSpawnEffectFromRequest argument block */
    XenoActor *linkedB; /* +0x98: second linked actor */
    XenoActor *linkedNode; /* +0x9C: actor whose draw flags and position playerLinkedActorAimUpdate edits */
    union {
        Vector4 v;
        s32 quadBits[4];
    } targetPoint; /* +0xA0: point found by playerProbeForwardPoint, copied to the linked node by playerLinkedActorAimUpdate */
    u8 probeBlock[0xC0 - 0xB0]; /* +0xB0: block passed to func_0027E838 (actor +0x200) */
    s32 cue; /* +0xC0: selected entry of the cue list */
    s32 waitFrames; /* +0xC4: frames counted while the sound waits */
} ActorSoundState;

typedef struct SoundActor {
    s32 activeFlags;     /* +0x00: bit 0x400000 = live */
    s32 previousFlags;   /* +0x04 */
    s32 moveEdgeFlags; /* +0x08: 0x80000 = input just stopped, 0x40000 = input just started */
    unsigned char unmodeled_0C[0x24 - 0x0C];
    ActorModel *model; /* +0x24: the actor model, argument of XenoActorIsDone */
    unsigned char unmodeled_28[0x2C - 0x28];
    s32 slotId; /* +0x2C */
    unsigned char unmodeled_30[0x40 - 0x30];
    f32 position[4]; /* +0x40: vector tested by the list scan of playerFindNearestTarget */
    unsigned char unmodeled_50[0x150 - 0x50];
    ActorSoundState sound; /* +0x150 */
} SoundActor;

/* Argument block of the sound effect start calls (sefObjectSetStatePlay fills the defaults, sefSpawnEffectFromRequest starts it). */
typedef struct SefObjectPlayArgs {
    ActorModel *ownerModel;
    ActorModel *otherModel;
    XenoActor *linkedA;
    XenoActor *linkedB;
    s32 unmodeled_10[7];
    s32 effectId; /* +0x2C: effect id; for playerSoundEffectStart the result of keyedWordTableFind */
    s32 unmodeled_30[20];
} SefObjectPlayArgs;

typedef struct FootstepEffectEntry {
    s32 surfaceKey;
    s32 effectId;
} FootstepEffectEntry;

extern FootstepEffectEntry D_00531560[4];
void sefObjectSetStatePlay(SefObjectPlayArgs *args);
struct SefEffectObject *sefSpawnEffectFromRequest(SefObjectPlayArgs *args);
void linkedObjectSetFloat(struct SefEffectObject *object, f32 value);

/* Starts the footstep effect of the surface type in checkKey (table D_00531560), silent until faded in. */
void func_001C5180(SoundActor *actor) {
    s32 footstepEffectId;
    SefObjectPlayArgs effectArgs;
    ActorSoundState *sound;
    s32 i;

    footstepEffectId = 0;
    sound = &actor->sound;
    if ((actor->activeFlags & 0x100) == 0) {
        return;
    }
    sound->soundResetHalf = 0;
    for (i = 0; i < 4; i++) {
        if (sound->checkKey == D_00531560[i].surfaceKey) {
            footstepEffectId = D_00531560[i].effectId;
            break;
        }
    }
    if (footstepEffectId == 0) {
        return;
    }
    sefObjectSetStatePlay(&effectArgs);
    effectArgs.ownerModel = actor->model;
    effectArgs.effectId = footstepEffectId;
    sound->effectObject = sefSpawnEffectFromRequest(&effectArgs);
    linkedObjectSetFloat(sound->effectObject, 0.0f);
}

void sefEffectHandleRelease(struct SefEffectObject *effectObject);

/* Footstep sound update: replays the step cue every 30 frames while moving and fades the effect with the input magnitude. */
void func_001C5250(SoundActor *actor) {
    SefObjectPlayArgs effectArgs;
    ActorSoundState *sound;

    sound = &actor->sound;
    if ((actor->activeFlags & 0x100) == 0) {
        return;
    }
    if (sound->inputMagnitude <= 0.0f) {
        sound->soundResetHalf = 0;
    } else {
        if (sound->effectObject == 0) {
            func_001C5180(actor);
        }
        sound->soundResetHalf = sound->soundResetHalf - 1;
        if (sound->soundResetHalf <= 0) {
            sound->soundResetHalf = 30;
            sefObjectSetStatePlay(&effectArgs);
            effectArgs.effectId = 0x10064033;
            effectArgs.ownerModel = actor->model;
            sefSpawnEffectFromRequest(&effectArgs);
        }
    }
    if (sound->effectObject == 0) {
        return;
    }
    if (sound->inputMagnitude < sound->fadeLevel) {
        sound->fadeLevel = sound->fadeLevel - 0.05f;
        if (sound->fadeLevel < sound->inputMagnitude) {
            sound->fadeLevel = sound->inputMagnitude;
        }
    } else {
        sound->fadeLevel = sound->fadeLevel + 0.05f;
        if (sound->inputMagnitude < sound->fadeLevel) {
            sound->fadeLevel = sound->inputMagnitude;
        }
    }
    if (sound->fadeLevel <= 0.0f) {
        sefEffectHandleRelease(sound->effectObject);
        sound->effectObject = 0;
    } else {
        linkedObjectSetFloat(sound->effectObject, sound->fadeLevel);
    }
}

void actorSoundStop(SoundActor *actor) {
    ActorSoundState *actorSound;
    struct SefEffectObject *effectObject;

    actorSound = &actor->sound;
    effectObject = actorSound->effectObject;
    if (effectObject != 0) {
        sefEffectHandleRelease(effectObject);
        actorSound->effectObject = 0;
    }
}

f32 MMathCalcDist(f32 *a, f32 *b);
f32 actorFacingError(ActorModel *model, f32 *position);
SoundActor *RgDispModelNextByKind(s32 kind, SoundActor *prev);

/* Returns the nearest kind-6 actor within 5.0 units and 80 degrees of the actor facing, or 0. */
SoundActor *playerFindNearestTarget(SoundActor *actor) {
    SoundActor *nearest;
    SoundActor *candidate;
    f32 nearestDistance;
    f32 distance;

    nearestDistance = 5.0f;
    nearest = 0;
    for (candidate = 0; (candidate = RgDispModelNextByKind(6, candidate)) != 0;) {
        if ((candidate->activeFlags & 0x80) != 0) {
            distance = MMathCalcDist(actor->position, candidate->position);
            if (distance > nearestDistance) {
                continue;
            }
            if (actorFacingError(actor->model, candidate->position) > 1.3962634f) {
                continue;
            }
            nearestDistance = distance;
            nearest = candidate;
        }
    }
    return nearest;
}

/* Collision block at +0x6A0 of the actor model (XenoActor.collision). */
typedef struct ModelCollision {
    s32 unk_00;
    s32 unk_04;
    u8 unmodeled_08[0x1c];
    s32 attribute; /* +0x24: attribute word, shown as ATTR; bits 8 and up hold the surface kind */
} ModelCollision;

/* The model object made by func_0028D670: the head follows the XenoActor layout of include/main/xenoActor.h. */
typedef struct ActorModel {
    s32 unk_00;
    s32 unk_04;
    u8 unmodeled_08[8];
    union {
        Vector4 v;
        s32 quadBits[4];
    } extents; /* +0x10 */
    Vector4 position;  /* +0x20 */
    Vector4 vector30;  /* +0x30 */
    Vector4 angle;     /* +0x40 */
    union {
        Vector4 v;
        s32 quadBits[4];
    } rotation;        /* +0x50: y is the facing angle */
    Vector4 vector60;  /* +0x60 */
    Vector4 vector70;  /* +0x70 */
    Vector4 scale;     /* +0x80 */
    s32 drawFlags;     /* +0x90 */
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
    u8 unmodeled_52C[0x544 - 0x52c];
    Matrix4Rows *globalMatrices; /* +0x544: one 0x40-byte matrix per hierarchy transform */
    u8 unmodeled_548[0x6a0 - 0x548];
    ModelCollision ct;
} ActorModel;


static inline void CopyVectorMem(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source), "m"(source->x), "m"(source->y), "m"(source->z), "m"(source->w)
                         : "$8");
}

/* kept apart from PadRecord: its union halfwords are alias set 0, which changes the emitted order of the pad reads. */
typedef struct PadButtonStickView {
    u8 unmodeled_00[0x28];
    u16 held;    /* +0x28 */
    u16 pressed; /* +0x2A */
    u8 unmodeled_2C[0x3a - 0x2c];
    s8 stickX; /* +0x3A: analog stick X, -128 to 127 */
    s8 stickY; /* +0x3B */
    u8 unmodeled_3C[0x66 - 0x3c];
    s8 leftX; /* +0x66 */
    s8 leftY; /* +0x67 */
} PadButtonStickView;

/* Camera record behind the game loop runtime block. */
typedef struct GameCamera {
    u8 unmodeled_00[0xa4];
    f32 yaw; /* +0xA4: camera yaw in radians */
} GameCamera;

/* The game loop runtime block (gGameLoopRuntimeStorage) as this file reads it. */
typedef struct GameLoopRuntimeView {
    s32 loopModeFlags; /* +0x00: bit 0x800 = pad input frozen */
    u8 unmodeled_04[0x24];
    GameCamera *camera; /* +0x28 */
    u8 unmodeled_2C[0x16c - 0x2c];
    f32 gameScale; /* +0x16C: size of one game unit */
    u8 unmodeled_170[0x100];
} GameLoopRuntimeView;

#define gGameLoopRuntimeView (*(GameLoopRuntimeView *) &gGameLoopRuntimeStorage)

void MMathApplyMatrix();
f32 MMathVectorDotProduct(Vector4*,Vector4*);
f32 MMathCalcDistXZ(Vector4*,Vector4*);
f32 func_002AA490();
void MMathNormalizeVector2();

static inline void VectorSetUnitZ(Vector4 *destination) {
    __asm__ __volatile__("vmove.xyw $vf1, $vf0\n vaddw.z $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination));
}

static inline void VectorSetZ(Vector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %1\n qmtc2 $8, $vf1\n vmove.xyw $vf2, $vf0\n vaddx.z $vf2, $vf0, $vf1x\n sqc2 $vf2, 0(%0)"
        :
        : "r"(destination), "f"(value)
        : "$8", "memory");
}

static inline void VectorSubXyz(Vector4 *destination, Vector4 *left, Vector4 *right) {
    __asm__ __volatile__("lqc2 $vf1,0(%1)\n lqc2 $vf2,0(%2)\n vsub.xyz $vf1,$vf1,$vf2\n sqc2 $vf1,0(%0)"
                         :
                         : "r"(destination), "r"(left), "r"(right)
                         : "memory");
}

extern f32 D_0054F440;
void MMathRotateMatrixY(Matrix4Rows*,Matrix4Rows*,f32);

/*
 * Looks ahead of the actor: casts a ray along its facing direction, at most three game units long and cut short by
 * the actors of kind 0x21D0 in front of it, and stores the end point (snapped by func_002AA490) in targetPoint.
 */
void playerProbeForwardPoint(SoundActor *actor) {
    Matrix4Rows facingMatrix;
    Vector4 direction;
    Vector4 directionToOther;
    ActorModel *model = actor->model;
    ModelCollision *collision = &model->ct;
    s32 savedFirstWord;
    s32 savedSecondWord;
    s32 savedAttribute;
    ActorSoundState *sound;
    SoundActor *otherActor;
    ActorModel *otherModel;
    f32 reach;
    f32 alignment;
    f32 clearance;

    reach = gGameLoopRuntimeView.gameScale * 3.0f;
    savedFirstWord = collision->unk_00;
    savedSecondWord = collision->unk_04;
    savedAttribute = collision->attribute;
    sound = &actor->sound;
    MMathRotateMatrixY(&facingMatrix, 0, model->rotation.v.y);
    VectorSetUnitZ(&direction);
    MMathApplyMatrix(&direction, &facingMatrix, &direction);
    actor->activeFlags &= 0x7fffffff;
    otherActor = 0;
    while ((otherActor = RgDispModelNextByKind(0x21d0, otherActor)) != 0) {
        if (otherActor->activeFlags & 0xc) {
            continue;
        }
        otherModel = otherActor->model;
        if (otherModel == 0) {
            continue;
        }
        if (__builtin_fabsf(model->position.y - otherModel->position.y) >
            gGameLoopRuntimeView.gameScale * D_0054F440) {
            continue;
        }
        VectorSubXyz(&directionToOther, &otherModel->position, &model->position);
        MMathNormalizeVector2(&directionToOther, &directionToOther);
        alignment = MMathVectorDotProduct(&direction, &directionToOther);
        if (alignment > 0.0f) {
            alignment *= 1.5f;
            clearance = MMathCalcDistXZ(&model->position, &otherModel->position) - otherModel->extents.v.w * alignment;
            if (clearance < reach) {
                reach = clearance;
            }
        }
    }
    CopyVectorMem(&sound->targetPoint.v, &model->position);
    VectorSetZ(&direction, reach);
    MMathApplyMatrix(&direction, &facingMatrix, &direction);
    sound->targetPoint.v.y = func_002AA490(&sound->targetPoint.v, &direction, collision);
    if ((collision->attribute & ~0xff) == 0x200) {
        actor->activeFlags |= 0x80000000;
    }
    collision->unk_00 = savedFirstWord;
    collision->unk_04 = savedSecondWord;
    collision->attribute = savedAttribute;
}

s32 keyedWordTableFind(s32 checkKey, s16 waitState);

/* Starts the effect that keyedWordTableFind selects from the sound key, between the actor and the other actor. */
void playerSoundEffectStart(SoundActor *actor, SoundActor *partner) {
    ActorSoundState *sound;
    SefObjectPlayArgs effectArgs;

    sound = &actor->sound;
    sefObjectSetStatePlay(&effectArgs);
    effectArgs.effectId = keyedWordTableFind(sound->checkKey, sound->requestedMode);
    if (effectArgs.effectId != 0) {
        effectArgs.ownerModel = actor->model;
        effectArgs.otherModel = partner->model;
        effectArgs.linkedA = sound->linkedA;
        effectArgs.linkedB = sound->linkedB;
        sefSpawnEffectFromRequest(&effectArgs);
    }
}

s32 MMathMakeRandomInt();
void seVoiceStartDefault(unsigned int handle);

/* Plays the voice pair of a cue: kind 0 picks one of three random voices (0x5A to 0x5C), any other kind plays 0x5D. */
void seVoicePlayCue(s32 cueKind) {
    s32 voice;

    if (cueKind == 0) {
        voice = (MMathMakeRandomInt() % 3) + 0x5A;
    } else {
        voice = 0x5D;
    }
    seVoiceStartDefault(voice);
    seVoiceStartDefault((cueKind != 0) ? 0x5F : 0x5E);
}

void slotStateFlagEdgeUpdate();

/* Sets the "input just started" or "input just stopped" bit of moveEdgeFlags from the two last input magnitudes. */
void actorSoundSyncFlags(SoundActor *actor) {
    ActorSoundState *actorSound;
    s32 cleanedFlags;

    actorSound = &actor->sound;
    if (!(actor->activeFlags & 0x400000)) {
        slotStateFlagEdgeUpdate();
        if ((actor->previousFlags & 0x400000) && (actorSound->previousMagnitude > 0.0f)) {
            actor->moveEdgeFlags = actor->moveEdgeFlags | 0x80000;
        }
    } else {
        cleanedFlags = actor->moveEdgeFlags;
        cleanedFlags = cleanedFlags & 0xFFF2FFFF;
        actor->moveEdgeFlags = cleanedFlags;
        if (actorSound->previousMagnitude > 0.0f) {
            if (actorSound->inputMagnitude <= 0.0f) {
                actor->moveEdgeFlags = cleanedFlags | 0x80000;
            }
        } else if (actorSound->inputMagnitude > 0.0f) {
            actor->moveEdgeFlags = cleanedFlags | 0x40000;
        }
    }
}

void actorSoundTickWait(SoundActor *actor);

void playerPadMoveVectorUpdate(SoundActor *actor);


void playerCueSelectUpdate(SoundActor *actor);

void playerWalkModeUpdate(SoundActor *actor);

void playerLinkedActorAimUpdate(SoundActor *actor);

/* kept apart from SoundActor: the flag word as a bit-field and pointer union decides the emitted order in playerButtonLatchUpdate. */
typedef struct SoundMagnitudeView {
    u8 unmodeled_00[0x1c];
    f32 inputMagnitude; /* +0x1C of the sound state */
} SoundMagnitudeView;

typedef struct SoundActorFlagsView {
    union {
        s32 activeFlags;
        struct {
            u32 unusedLow : 28;
            u32 confirmLatch : 1; /* bit 0x10000000 */
            u32 unusedHigh : 3;
        } bits;
        s32 *wordPointer;
    };
    u8 unmodeled_04[0x14c];
    SoundMagnitudeView sound; /* +0x150 */
} SoundActorFlagsView;

void playerButtonLatchUpdate(SoundActorFlagsView *actor);

void playerFocusTargetUpdate(SoundActor *actor);

void playerModeOneUpdate(SoundActor *actor);

void playerModeTwoUpdate(SoundActor *actor);

void XenoActorClearModeWhenDone(SoundActor *actor);
void func_001ECA60(SoundActor *actor);
void XenoActorApplyMode(SoundActor *actor);
void XenoActorWrapRotation(ActorModel *model);

/* Per-frame update of the player actor: runs the handler of its movement mode, then syncs flags, footsteps and modes. */
void playerFrameUpdate(SoundActor *actor) {
    ActorSoundState *sound;
    f32 zeroLevel;

    sound = &actor->sound;
    actor->moveEdgeFlags &= ~0x2;
    playerPadMoveVectorUpdate(actor);
    switch (sound->mode) {
    case 0:
        if ((gGameLoopRuntimeStorage.flags & 0x800) != 0) {
            sound->requestedMode = 0;
            break;
        }
        actorSoundTickWait(actor);
        playerCueSelectUpdate(actor);
        playerWalkModeUpdate(actor);
        playerLinkedActorAimUpdate(actor);
        playerButtonLatchUpdate(actor);
        playerFocusTargetUpdate(actor);
        break;
    case 1:
        playerModeOneUpdate(actor);
        break;
    case 2:
        playerModeTwoUpdate(actor);
        break;
    case 3:
    case 4:
    case 5:
        sound->inputMagnitude = 0.0f;
        XenoActorClearModeWhenDone(actor);
        break;
    case 6:
        zeroLevel = 0.0f;
        sound->mode = 0;
        sound->previousMagnitude = zeroLevel;
        sound->inputMagnitude = zeroLevel;
        break;
    }
    actorSoundSyncFlags(actor);
    func_001ECA60(actor);
    func_001C5250(actor);
    XenoActorApplyMode(actor);
    if ((actor->model->ct.attribute & ~0xFF) != 512) {
        return;
    }
    XenoActorWrapRotation(actor->model);
}

f32 MMathCalcLengthXZ();
extern PadRecord PadData[2];

static inline void VectorSetOrigin(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

f32 MMathCalcRotNear(f32,f32);
void MMathDivVectorS(Vector4*,Vector4*,f32);

static inline void VectorScaleXyz(Vector4 *destination, Vector4 *source, f32 value) {
    __asm__ __volatile__(
        "lqc2 $vf1,0(%1)\n mfc1 $8, %2\n qmtc2 $8, $vf2\n vmulx.xyz $vf1,$vf1,$vf2x\n sqc2 $vf1, 0(%0)"
        :
        : "r"(destination), "r"(source), "f"(value)
        : "$8", "memory");
}

/* Reads the pad: builds the movement vector (stick or d-pad, rotated by the camera yaw) and its magnitude. */
void playerPadMoveVectorUpdate(SoundActor *actor) {
    ActorSoundState *sound = &actor->sound;
    Vector4 *moveVector = &sound->moveVector;
    PadButtonStickView *pad = (PadButtonStickView *) PadData;
    s32 padHeld = pad->held;
    s32 stickX = pad->stickX;
    s32 stickY = pad->stickY;
    s32 leftX = pad->leftX;
    s32 leftY = pad->leftY;
    sound->previousMagnitude = sound->inputMagnitude;
    sound->inputMagnitude = 0;
    VectorSetOrigin(moveVector);
    if (!(actor->activeFlags & 0x400000)) {
        return;
    }
    if (gGameLoopRuntimeView.loopModeFlags & 0x800) {
        return;
    }
    sound->previousCameraYaw = sound->cameraYaw;
    sound->cameraYaw = gGameLoopRuntimeView.camera->yaw;
    if (sound->previousMagnitude > 0.0f) {
        sound->cameraYaw = sound->previousCameraYaw + MMathCalcRotNear(sound->previousCameraYaw, sound->cameraYaw) * 0.1f;
    }
    /* kept: the cast gives the call an int result, which decides the register of the next test */
    ((s32 (*)(Matrix4Rows *, Matrix4Rows *, f32)) MMathRotateMatrixY)(&sound->cameraMatrix, 0, sound->cameraYaw);
    if (leftX | leftY) {
        moveVector->x = stickX * 0.0078125f;
        moveVector->z = stickY * 0.0078125f;
        MMathApplyMatrix(moveVector, &sound->cameraMatrix, moveVector);
        sound->inputMagnitude = MMathCalcLengthXZ(moveVector);
        if (sound->inputMagnitude > 1.0f) {
            MMathDivVectorS(moveVector, moveVector, sound->inputMagnitude);
            sound->inputMagnitude = 1.0f;
        }
        if (padHeld & 1) {
            if (sound->inputMagnitude > 0.2842f) {
                sound->inputMagnitude = 0.2842f;
            }
        }
        MMathNormalizeVector2(moveVector, moveVector);
        VectorScaleXyz(moveVector, moveVector, sound->moveSpeed * sound->inputMagnitude);
    }
    if (sound->inputMagnitude <= 0.0f) {
        VectorSetOrigin(moveVector);
        if (padHeld & 0x8000) {
            moveVector->x = -0.2842f;
        }
        if (padHeld & 0x2000) {
            moveVector->x = 0.2842f;
        }
        if (padHeld & 0x1000) {
            moveVector->z = -0.2842f;
        }
        if (padHeld & 0x4000) {
            moveVector->z = 0.2842f;
        }
        MMathApplyMatrix(moveVector, &sound->cameraMatrix, moveVector);
        sound->inputMagnitude = MMathCalcLengthXZ(moveVector);
        if (sound->inputMagnitude > 0.2842f) {
            MMathDivVectorS(moveVector, moveVector, sound->inputMagnitude);
            sound->inputMagnitude = 0.2842f;
        }
        MMathNormalizeVector2(moveVector, moveVector);
        VectorScaleXyz(moveVector, moveVector, sound->moveSpeed * sound->inputMagnitude);
    }
}

s32 XenoActorIsDone();

/* Counts idle frames while live; after 300 frames sets the wait request 0xC, then clears it once the model is done. */
void actorSoundTickWait(SoundActor *actor) {
    ActorSoundState *actorSound;
    s32 frames;
    s32 belowLimit;

    actorSound = &actor->sound;
    if (actor->activeFlags & 0x400000) {
        if (!(actorSound->inputMagnitude > 0.0f)) {
            frames = actorSound->waitFrames + 1;
            belowLimit = frames < 0x12D;
            actorSound->waitFrames = frames;
            if (frames == 0x12C) {
                actorSound->requestedMode = 0xC;
                return;
            }
            if ((belowLimit == 0) && (XenoActorIsDone(actor->model) != 0)) {
                actorSound->waitFrames = 0;
                actorSound->requestedMode = 0;
            }
        } else {
            actorSound->waitFrames = 0;
        }
    }
}

s32 *eventSlotFind();
SoundActor *RgDispModelFindByKey();
s32 func_001C1890();
f32 MMathCalcDir();
void seVoiceStartDefault(unsigned int handle);
void seVoicePlayCue();
void func_001C1320();
void playerSoundEffectStart();
void XenoActorSetMode();

/* Cue selection on the pad: buttons 4 and 8 step through the cue list, button 0x80 starts the cue on its target actor. */
void playerCueSelectUpdate(SoundActor *actor) {
    u16 padHeld = PadData[0].held;
    u16 padPressed = PadData[0].pressed;
    ActorSoundState *sound = &actor->sound;
    s32 *cueEntry;
    s32 cueId;
    s32 talkKind;
    SoundActor *talkTarget;
    XenoActor *linkedA;
    XenoActor *linkedB;

    if ((actor->activeFlags & 0x400000) == 0) {
        return;
    }
    if (padHeld & 2) {
        return;
    }
    if (padPressed & 0x20) {
        return;
    }
    if (padPressed & 0xC) {
        cueEntry = eventSlotFind(sound->cue, (padPressed & 4) ? 1 : 2);
        cueId = cueEntry ? *cueEntry : 0;
        sound->cue = cueId;
        if (cueId) {
            seVoiceStartDefault(14);
            return;
        }
    }
    if (!(padPressed & 0x80) || sound->cue == 0 || (sound->cue & 0x20000000)) {
        return;
    }
    cueEntry = eventSlotFind(sound->cue, 0);
    if (cueEntry == 0) {
        return;
    }
    talkTarget = RgDispModelFindByKey(*cueEntry & 0xFFFFFF);
    talkKind = 3;
    sound->requestedMode = func_001C1890(talkTarget);
    sound->mode = (sound->requestedMode == talkKind) ? 1 : 2;
    actor->model->rotation.v.y = MMathCalcDir(actor->position, talkTarget->position);
    if (sound->requestedMode == talkKind) {
        sound->countdown = (actor->activeFlags & 0x100) ? 15 : 10;
        seVoicePlayCue(0);
    } else {
        sound->countdown = 50;
        seVoicePlayCue(1);
    }
    func_001C1320(talkTarget, (s16) sound->countdown);
    playerSoundEffectStart(actor, talkTarget);
    if (sound->requestedMode == 4) {
        linkedA = sound->linkedA;
        if (linkedA != 0) {
            linkedA->drawFlags &= ~4;
            if (sound->checkKey != 4 && sound->checkKey != 6) {
                XenoActorSetMode(linkedA, 6);
            }
        }
        linkedB = sound->linkedB;
        if (linkedB != 0) {
            linkedB->drawFlags &= ~4;
            if (sound->checkKey != 4 && sound->checkKey != 6) {
                XenoActorSetMode(linkedB, 5);
            }
        }
    }
}

extern s16 D_003E4820[];
s32 Get_Rnd_001B8CD0(s32 low, s32 high);
void flatSlotSpawn(s32 *block);
void func_001B8D08(s32 a, s32 b, s32 c);

/* Kept as its own helper: merging it with CopyVectorMem renumbers the compiler's labels. */
static inline void copyVectorQuadword(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source), "m"(source->x), "m"(source->y), "m"(source->z), "m"(source->w)
                         : "$8");
}

/* While live and pad bit 2 is held, runs playerProbeForwardPoint and copies its target point to the linked actor; button 0x80 then starts the request flatSlotSpawn. */
void playerLinkedActorAimUpdate(SoundActor *actor) {
    ActorSoundState *sound;
    XenoActor *linked;
    XenoActor *aimActor;
    s32 randomRoll;
    s32 activeSnapshot;
    s32 isLive;
    s32 spawnRequest[6];
    u16 padPressed;
    u16 padHeld;

    sound = &actor->sound;
    linked = sound->linkedNode;
    activeSnapshot = actor->activeFlags;
    isLive = activeSnapshot & 0x400000;
    padHeld = PadData[0].held;
    padPressed = PadData[0].pressed;
    linked->drawFlags |= 4;
    if (isLive == 0) {
        return;
    }
    if ((activeSnapshot & 0x100000) == 0) {
        return;
    }
    if (D_003E4820[0] != 0) {
        return;
    }
    randomRoll = Get_Rnd_001B8CD0(0, 56);
    if (randomRoll <= 0) {
        return;
    }
    if ((padHeld & 0x2) == 0) {
        return;
    }
    playerProbeForwardPoint(actor);
    aimActor = sound->linkedNode;
    aimActor->drawFlags &= ~4;
    copyVectorQuadword(&aimActor->position, &sound->targetPoint.v);
    if ((padPressed & 0x80) == 0) {
        return;
    }
    if (actor->activeFlags < 0) {
        seVoiceStartDefault(3);
        return;
    }
    sound->mode = 3;
    sound->requestedMode = 7;
    copyVectorQuadword((Vector4 *) spawnRequest, &sound->targetPoint.v);
    spawnRequest[4] = 10;
    spawnRequest[5] = actor->slotId;
    flatSlotSpawn(spawnRequest);
    func_001B8D08(0, 56, randomRoll - 1);
}


/* Clears flag 0x10000000 every frame and sets it again when pad button 0x100 is pressed while the actor is idle. */
void playerButtonLatchUpdate(SoundActorFlagsView *actor) {
    SoundMagnitudeView *sound = &actor->sound;
    u16 pressed;
    u16 held;

    held = ((PadButtonStickView *) PadData)->held;
    pressed = ((PadButtonStickView *) PadData)->pressed;
    actor->bits.confirmLatch = 0;
    if (actor->activeFlags & 0x400000) {
        if (sound->inputMagnitude > 0.0f) {
            return;
        }
        if (gGameLoopRuntimeStorage.flags & 0x4008) {
            return;
        }
        if (held & 0xC) {
            return;
        }
        if (pressed & 0x20) {
            return;
        }
        if (pressed & 0x100) {
            actor->activeFlags |= 0x10000000;
        }
    }
}

extern f32 gPlayerTurnStepMax;
extern f32 gPlayerTurnStepMin;
f32 srsAtan2(f32, f32);
void mapProbeFlagOn(void *, void *, void *);
void func_0027E838(void *, void *, void *, void *);
void slotFollowTargetHeight(SoundActor *, f32);
void soundCueSeCommand(s32, s32, s32, s32);

/* Movement mode 0: sets requestedMode from inputMagnitude (1 below 0.5, 2 above) and turns the model toward moveVector. */
void playerWalkModeUpdate(SoundActor *actor) {
    ActorSoundState *sound = &actor->sound;
    XenoActor *model = (XenoActor *) actor->model;
    AlignedVector4 probe;
    f32 angle;
    f32 turn;

    if ((u32) (sound->mode - 1) < 2U) {
        sound->inputMagnitude = 0.0f;
        __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(&sound->moveVector) : "memory");
        return;
    }

    QuadwordCopyT0(&probe, &sound->moveVector);
    if (!(actor->activeFlags & 0x80000)) {
        probe.w = sound->moveSpeed;
        mapProbeFlagOn(&probe, &probe, model->collision);
        func_0027E838(&probe, model, &probe, actor->sound.probeBlock);
        angle = func_002AA490(&model->position, &probe, model->collision);
        slotFollowTargetHeight(actor, angle);
    }

    actor->activeFlags &= 0xFFFF7FFF;
    if (sound->inputMagnitude <= 0.0f) {
        if (sound->requestedMode != 0xC) {
            sound->requestedMode = 0;
        }
    } else {
        if (sound->inputMagnitude < 0.5f) {
            sound->requestedMode = 1;
        } else {
            sound->requestedMode = 2;
            actor->activeFlags |= 0x8000;
        }
        if (!(actor->activeFlags & 0x40000)) {
            angle = srsAtan2(sound->moveVector.x, sound->moveVector.z);
            turn = MMathCalcRotNear(model->rotation.y, angle);
            if (gPlayerTurnStepMax < turn) {
                turn = gPlayerTurnStepMax;
            }
            if (turn < gPlayerTurnStepMin) {
                turn = gPlayerTurnStepMin;
            }
            model->rotation.y += turn;
        }
        soundCueSeCommand(0, -1, 0, 0);
    }
    if (actor->activeFlags & 0x40000) {
        sound->requestedMode = 2;
    }
}

extern u16 gPadDataPressed[];

/* Movement mode 1: ends when the model is done or, after input or button 0x80, when the countdown runs out. */
void playerModeOneUpdate(SoundActor *actor) {
    ActorSoundState *sound = &actor->sound;
    XenoActor *model;
    AlignedVector4 probe;
    u16 padPressed;
    s32 countdown;
    s32 done;
    f32 angle;

    padPressed = gPadDataPressed[0];
    model = (XenoActor *) actor->model;
    if (!(actor->activeFlags & 0x80000)) {
        __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(&probe) : "memory");
        probe.w = sound->moveSpeed;
        mapProbeFlagOn(&probe, &probe, model->collision);
        func_0027E838(&probe, model, &probe, actor->sound.probeBlock);
        angle = func_002AA490(&model->position, &probe, model->collision);
        slotFollowTargetHeight(actor, angle);
        model = (XenoActor *) actor->model;
    }

    countdown = sound->countdown;
    countdown -= 1;
    sound->countdown = countdown;
    done = 0;
    if (XenoActorIsDone(model)) {
        done = 1;
    }
    if (sound->inputMagnitude > 0.0f || (padPressed & 0x80) != 0) {
        countdown = sound->countdown;
        if ((s16) countdown < 0) {
            done = 1;
        }
    }
    sound->inputMagnitude = 0.0f;
    if (done) {
        sound->mode = 0;
        sound->requestedMode = 0;
    }
}

/* Movement mode 2: like mode 1, and on completion sets draw flag 4 of the two linked actors. */
void playerModeTwoUpdate(SoundActor *actor) {
    ActorSoundState *sound = &actor->sound;
    u16 padPressed;
    s32 countdown;
    s32 done;

    padPressed = gPadDataPressed[0];
    countdown = sound->countdown;
    countdown -= 1;
    sound->countdown = countdown;
    done = XenoActorIsDone(actor->model) != 0;
    if (sound->inputMagnitude > 0.0f || (padPressed & 0x80) != 0) {
        if ((s16) sound->countdown < 0) {
            done = 1;
        }
    }
    sound->inputMagnitude = 0.0f;
    if (done) {
        sound->mode = 0;
        sound->requestedMode = 0;
        if (sound->linkedA != 0) {
            sound->linkedA->drawFlags |= 4;
        }
        if (sound->linkedB != 0) {
            sound->linkedB->drawFlags |= 4;
        }
    }
}

void XenoActorClearModeWhenDone(SoundActor *actor) {
    ActorSoundState *sound = &actor->sound;

    if (XenoActorIsDone(actor->model) != 0) {
        sound->mode = 0;
        sound->requestedMode = 0;
    }
}

void XenoActorCheckMode();
void XenoActorEnableChannel();
void XenoActorSetChannelValue(ActorModel *, f32);

void XenoActorApplyMode(SoundActor *actor) {
    ActorSoundState *sound = &actor->sound;
    ActorModel *model;
    u16 requested;

    model = actor->model;
    requested = sound->requestedMode;
    if (sound->appliedMode != sound->requestedMode) {
        sound->appliedMode = requested;
        XenoActorSetChannelValue(model, 12.0f);
        XenoActorEnableChannel(model);
        XenoActorCheckMode(model, sound->requestedMode);
    }
}

static inline void MatrixTranslationPoint(Vector4 *destination, Matrix4Rows *source) {
    __asm__ __volatile__("lqc2 $vf1, 0x30(%1)\n vmove.w $vf1,$vf0\n sqc2 $vf1, 0(%0)"
                         :
                         : "r"(destination), "r"(source)
                         : "memory");
}

SoundActor *playerFindNearestTarget(SoundActor *actor);
s32 xrgObjectIsPartsKind1E();
void XenoActorSetFocusTarget();

static inline void VectorAddXyz(Vector4 *destination, Vector4 *left, Vector4 *right) {
    __asm__ __volatile__("lqc2 $vf1,0(%1)\n lqc2 $vf2,0(%2)\n vadd.xyz $vf1,$vf1,$vf2\n sqc2 $vf1,0(%0)"
                         :
                         : "r"(destination), "r"(left), "r"(right)
                         : "memory");
}

typedef union {
    Vector4 v;
    s32 q[4];
} UV;

/* Sets the focus target of the actor model to a point on the target found by playerFindNearestTarget. */
/* kept: the union member access (alias set 0) decides the emitted order of the focus point stores. */
typedef union FocusPoquadBits {
    Vector4 v;
    s32 quadBits[4];
} FocusPoquadBits;

void playerFocusTargetUpdate(SoundActor *actor) {
    FocusPoquadBits focusPoint;
    SoundActor *focusCandidate;
    Matrix4Rows *matrices;
    ActorModel *targetModel;

    if (!(actor->activeFlags & 0x400000)) {
        return;
    }
    if (actor->activeFlags & 0x100) {
        return;
    }
    focusCandidate = playerFindNearestTarget(actor);
    if (focusCandidate == 0) {
        return;
    }
    if (focusCandidate->model == 0) {
        return;
    }
    if (xrgObjectIsPartsKind1E(focusCandidate->model)) {
        targetModel = focusCandidate->model;
        matrices = &targetModel->globalMatrices[29];
        MatrixTranslationPoint(&focusPoint.v, matrices);
        VectorAddXyz(&focusPoint.v, &focusPoint.v, &targetModel->position);
        XenoActorSetFocusTarget(actor->model, &focusPoint.v);
    } else {
        matrices = &focusCandidate->model->globalMatrices[1];
        MatrixTranslationPoint(&focusPoint.v, matrices);
        focusPoint.v.y *= 1.7f;
        VectorAddXyz(&focusPoint.v, &focusPoint.v, &focusCandidate->model->position);
        XenoActorSetFocusTarget(actor->model, &focusPoint.v);
    }
}

/* Filler enums: only their alias sets matter, the slot words they type hold flags and a block index. */
enum FocusFlags {
    FOCUS_FLAGS_NONE,
    FOCUS_FLAGS_WIDTH = 0x7fffffff
};

enum FocusKind {
    FOCUS_KIND_NONE,
    FOCUS_KIND_WIDTH = 0x7fffffff
};

/* State block of the focus debug display object, at +0x150 of the slot. */
typedef struct FocusBlock {
    s16 unk_00; /* +0x00, stored as 0 */
    s16 unmodeled_02;
    enum FocusFlags index; /* +0x04: request index, 0 to 3 */
    s32 requestArgument; /* +0x08 */
    s32 id;        /* +0x0C */
    s32 frameCount; /* +0x10, stored as 75 */
    f32 gameScale; /* +0x14: the game unit scale */
    s32 unk_18;    /* +0x18, stored as 0 */
    f32 moveValue; /* +0x1C: the MV value shown on screen, 6 degrees in radians */
} FocusBlock;

/* Slot object (RgDispModelAlloc) of the focus debug display; its model is the focused actor. */
typedef struct FocusObject {
    enum FocusFlags slotFlags; /* +0x00 */
    u8 unmodeled_04[0x0C];
    enum FocusKind slotKind; /* +0x10 */
    u8 unmodeled_14[0x08];
    void (*update)(); /* +0x1C */
    u8 unmodeled_20[0x04];
    ActorModel *model; /* +0x24 */
    s32 id;              /* +0x28 */
    u8 unmodeled_2C[0x40 - 0x2C];
    Vector4 position; /* +0x40 */
    u8 unmodeled_50[0x134 - 0x50];
    f32 halfScale; /* +0x134 */
    u8 unmodeled_138[0x150 - 0x138];
    FocusBlock focusBlock; /* +0x150 */
} FocusObject;

void GameMpeg2Play(char *dst, const char *fmt, ...);
void xglFontDebugPrintf(s32, s32, char *);
s32 mapStaticObjectProbe(void *, void *);

/* Debug overlay: prints the focused actor's position, rotation angle, attribute word and the MV and SEID values. */
void focusDebugOverlayDraw(FocusObject *focusObject) {
    char text[0x40];
    AlignedVector4 probeOut;
    FocusBlock *focusState;
    Vector4 *position;
    f32 angle;
    s32 displayedValue; /* the angle in degrees, then the SE id */

    focusState = &focusObject->focusBlock;
    position = &focusObject->model->position;
    GameMpeg2Play(text, "\022\010\010X:%.2f Y:%.2f Z:%.2f", position->x, position->y, position->z);
    xglFontDebugPrintf(8, 0x80, text);
    angle = focusObject->model->rotation.v.y;
    displayedValue = (s32) (angle * 57.29578f) % 360;
    if (displayedValue < 0) {
        displayedValue += 360;
    }
    GameMpeg2Play(text, "\022\010\010R:%.2f(%d)", angle, displayedValue);
    xglFontDebugPrintf(8, 0x88, text);
    GameMpeg2Play(text, "\022\010\010ATTR:%08X", focusObject->model->ct.attribute);
    xglFontDebugPrintf(8, 0x90, text);
    displayedValue = mapStaticObjectProbe(&focusObject->position, &probeOut);
    GameMpeg2Play(text, "\022\010\010MV:%.2f SEID:%2d", focusState->moveValue, displayedValue);
    xglFontDebugPrintf(8, 0x98, text);
}

/* Kept as its own helper: merging it with VectorSetOrigin renumbers the compiler's labels. */
static inline void VectorClearModel(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

static inline void VectorSetUniformXyz(Vector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %1\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%0)"
        :
        : "r"(destination), "f"(value)
        : "$8", "memory");
}

FocusObject *RgDispModelAlloc();
ActorModel *func_0028D670();
void RgDispModelSetLocal();
s32 payloadSectionsLoad();
void emitterSoundUpdate();
void actorSetId();

typedef struct FocusRequest {
    s32 id;
    enum FocusFlags index; /* 0 to 3, selects the entry of D_005315F0 */
    s32 argument;
} FocusRequest;

typedef struct FocusIdTable {
    s32 ids[4];
} FocusIdTable;

extern const FocusIdTable D_005315F0;

/* Creates the focus debug slot object for a request and gives it a model sized by the game scale; 0 when it cannot. */
int func_001C6830(FocusRequest *focusRequest) {
    FocusIdTable idTable = D_005315F0;
    FocusObject *focusObject;
    SoundActor *playerActor;
    FocusBlock *focusState;
    ActorModel *model;
    f32 halfExtent;

    if ((u32) focusRequest->index >= 4) {
        return 0;
    }
    focusObject = RgDispModelAlloc(focusRequest->id);
    focusState = &focusObject->focusBlock;
    if (focusObject == 0) {
        return 0;
    }
    playerActor = RgDispModelFindByKey(0);
    if (playerActor != 0 && (playerActor->activeFlags & 0x100)) {
        focusObject->slotFlags |= 0x100;
    }
    focusObject->id = focusRequest->id;
    focusState->index = focusRequest->index;
    focusState->requestArgument = focusRequest->argument;
    focusObject->halfScale = gGameLoopRuntimeView.gameScale * 0.5f;
    focusObject->slotKind = 0x8000;
    focusObject->update = emitterSoundUpdate;
    focusState->id = idTable.ids[focusRequest->index];
    focusState->frameCount = 75;
    focusState->gameScale = gGameLoopRuntimeView.gameScale;
    focusState->moveValue = 0.10471975f;
    focusState->unk_00 = 0;
    focusState->unk_18 = 0;
    model = func_0028D670();
    focusObject->model = model;
    if (model == 0) {
        RgDispModelSetLocal(focusObject);
        return 0;
    }
    model->drawFlags |= 0x24000;
    VectorClearModel(&model->extents.v);
    model->unk_04 = 0;
    halfExtent = focusState->gameScale * 0.5f;
    model->extents.v.w = halfExtent;
    model->extents.v.y = halfExtent;
    model->extents.v.x = halfExtent;
    VectorClearModel(&model->position);
    VectorClearModel(&model->rotation.v);
    VectorSetUniformXyz(&model->scale, focusState->gameScale);
    payloadSectionsLoad(model, focusState->id);
    actorSetId(focusObject, -1);
    return 1;
}
