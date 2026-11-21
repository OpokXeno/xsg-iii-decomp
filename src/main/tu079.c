#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef short s16;

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5180);

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5250);

/* Sound state of an actor, kept at +0x150 of the actor record. */
struct SefEffectObject;

typedef struct ActorSoundState {
    unsigned char unmodeled_00[0x10];
    struct SefEffectObject *effectObject;         /* +0x10: effect object to release */
    unsigned char unmodeled_14[0x1C - 0x14];
    f32 muteLevel;                          /* +0x1C: above zero while the sound is muted */
    f32 playLevel;                          /* +0x20: above zero while the sound plays */
    unsigned char unmodeled_24[0x90 - 0x24];
    s16 waitState;                              /* +0x90: 0xC when the wait ends, 0 when it is cleared */
    unsigned char unmodeled_92[0xC4 - 0x92];
    s32 waitFrames;                         /* +0xC4: frames counted while the sound waits */
} ActorSoundState;

/* The actor record as the sound code sees it: the flag words at +0x00, +0x04 and +0x08, the done key at +0x24. */
typedef struct SoundActor {
    s32 activeFlags;                        /* +0x00: bit 0x400000 = live */
    s32 previousFlags;                      /* +0x04 */
    s32 soundStatusBits;                              /* +0x08: bits 0x80000 and 0x40000 are the sound's status */
    unsigned char unmodeled_0C[0x24 - 0x0C];
    s32 doneKey;                            /* +0x24: argument of XenoActorIsDone */
    unsigned char unmodeled_28[0x150 - 0x28];
    ActorSoundState sound;                  /* +0x150 */
} SoundActor;

void sefEffectHandleRelease(struct SefEffectObject *effectObject);

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

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C53C0);

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5498);

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5700);

s32 MMathMakeRandomInt();
void seVoiceStartDefault(unsigned int effectObject);

void seVoicePlayCue(s32 cueKind)
{
  s32 voice;
  if (cueKind == 0)
  {
    voice = (MMathMakeRandomInt() % 3) + 0x5A;
  }
  else
  {
    voice = 0x5D;
  }
  seVoiceStartDefault(voice);
  seVoiceStartDefault((cueKind != 0) ? (0x5F) : (0x5E));
}

void func_001C1C30();

void actorSoundSyncFlags(SoundActor *actor)
{
  ActorSoundState *actorSound;
  s32 cleanedFlags;
  actorSound = &actor->sound;
  if (!(actor->activeFlags & 0x400000))
  {
    func_001C1C30();
    if ((actor->previousFlags & 0x400000) && (actorSound->playLevel > 0.0f))
    {
      actor->soundStatusBits = actor->soundStatusBits | 0x80000;
    }
  }
  else
  {
    cleanedFlags = actor->soundStatusBits;
    cleanedFlags = cleanedFlags & 0xFFF2FFFF;
    actor->soundStatusBits = cleanedFlags;
    if (actorSound->playLevel > 0.0f)
    {
      if (actorSound->muteLevel <= 0.0f)
      {
        actor->soundStatusBits = cleanedFlags | 0x80000;
      }
    }
    else if (actorSound->muteLevel > 0.0f)
    {
      actor->soundStatusBits = cleanedFlags | 0x40000;
    }
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C58C8);

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5A18);

s32 XenoActorIsDone();

void actorSoundTickWait(SoundActor *actor) {
    ActorSoundState *actorSound;
    s32 frames;
    s32 belowLimit;

    actorSound = &actor->sound;
    if (actor->activeFlags & 0x400000) {
        if (!(actorSound->muteLevel > 0.0f)) {
            frames = actorSound->waitFrames + 1;
            belowLimit = frames < 0x12D;
            actorSound->waitFrames = frames;
            if (frames == 0x12C) {
                actorSound->waitState = 0xC;
                return;
            }
            if ((belowLimit == 0) && (XenoActorIsDone(actor->doneKey) != 0)) {
                actorSound->waitFrames = 0;
                actorSound->waitState = 0;
            }
        } else {
            actorSound->waitFrames = 0;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5D50);

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C5F80);

INCLUDE_ASM("asm/main/nonmatchings/tu079", func_001C60C8);
