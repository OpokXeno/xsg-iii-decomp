#include "common.h"

#include "main/sound_driver.h"
#include "shared.h"
typedef struct EffectProgress EffectProgress;

typedef struct EffectState EffectState;

typedef struct EffectActor EffectActor;

struct SefEffectObject;

/* Progress block of the effect actor at +0x150. */
struct EffectProgress {
    int startDelay; /* +0x00: frames before the first effect, counted down by func_001C7C48 */
    int hitCount;   /* +0x04: remaining pad hits, counted down by func_001C7CE8 */
    int timer;      /* +0x08: frames left of the final effect */
    struct SefEffectObject *effectObject; /* +0x0C: effect started by func_001BF848 */
};

/* State block of the effect actor (+0x24). */
struct EffectState {
    unsigned char unmodeled_00[0x10];
    union {
        float triggerRadius; /* +0x10 */
        int radiusWord;
    };
    unsigned char unmodeled_14[0xC];
    union {
        Vector4 probePoint; /* +0x20; y is written from the float result */
        int probeWords[4];
    };
    unsigned char unmodeled_30[0x60];
    union {
        int stateFlags; /* +0x90 */
        struct {
            u32 low2 : 2;
            u32 hidden : 1;
            u32 high : 29;
        } bits;
    };
    unsigned char unmodeled_94[0x6A0 - 0x94];
    unsigned char probeRecord[0x10]; /* +0x6A0: passed by address to the probe calls */
};

/* Slot object allocated by RgDispModelAlloc (stride 0x230, see rg_dispmodel.c). */
struct EffectActor {
    union {
        int flags; /* +0x00 */
        struct {
            u32 low21 : 21;
            u32 stageDone : 1;
            u32 high : 10;
        } bits;
    };
    unsigned char unmodeled_04[0x18];
    void (*update)(EffectActor *self); /* +0x1C */
    unsigned char unmodeled_20[4];
    EffectState *state; /* +0x24 */
    unsigned char unmodeled_28[0x18];
    Vector4 position; /* +0x40 */
    unsigned char unmodeled_50[0x100];
    EffectProgress progress; /* +0x150 */
};

struct SefEffectObject *func_001BF848(u32 id, Vector4 *position);
void func_001C7CE8(EffectActor *self);

/* First stage: after the start delay it plays voice 0x12 and the two opening effects, then hands over to func_001C7CE8. */
void func_001C7C48(EffectActor *self) {
    EffectProgress *progress = &self->progress;

    if (--progress->startDelay > 0) {
        return;
    }
    self->bits.stageDone = 1;
    self->state->bits.hidden = 0;
    self->update = func_001C7CE8;
    seVoiceStartDefault(0x12);
    func_001BF848(0x10064027, &self->position);
    progress->effectObject = func_001BF848(0x1006402C, &self->position);
}

void mapProbeFlagOn(void *source, void *id, void *value);
float func_002AA490(void *a0, void *a1, void *a2);
void func_001EAA08(void *a0, void *a1);
void sefEffectHandleRelease(struct SefEffectObject *effectObject);
void vibrationModeSet(int a0);

void func_001C7E08(EffectActor *self);
extern f32 D_003E481C[];

/* Second stage: probes the ground under the effect each frame and, on the pad press, counts down to the final effect. */
void func_001C7CE8(EffectActor *self) {
    Vector4 probe;
    int flags;
    int newFlags;
    int hitCount;
    EffectProgress *progress = &self->progress;
    EffectState *state = self->state;

    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&probe) : "memory");
    mapProbeFlagOn(&probe, &probe, state->probeRecord);
    state->probePoint.y = func_002AA490(&state->probePoint, &probe, state->probeRecord);
    func_001EAA08(progress->effectObject, &state->probePoint);
    flags = self->flags;
    if (!(flags & 0x20)) {
        return;
    }
    newFlags = flags & ~0x200000;
    hitCount = progress->hitCount - 1;
    self->flags = newFlags;
    progress->hitCount = hitCount;
    if (hitCount > 0) {
        return;
    }
    sefEffectHandleRelease(progress->effectObject);
    progress->effectObject = func_001BF848(0x10064028, &self->position);
    seVoiceStartDefault(0x13);
    vibrationModeSet(1);
    self->update = func_001C7E08;
    self->state->triggerRadius = D_003E481C[0] * 4.0f;
    self->state->stateFlags |= 4;
    progress->timer = 300;
}

typedef struct EffectListEntry {
    int flags; /* +0x00 */
    unsigned char unmodeled_04[0x24];
    int id28; /* +0x28 */
    unsigned char unmodeled_2C[0x40 - 0x2C];
    Vector4 position; /* +0x40 */
} EffectListEntry;

void *RgDispModelNextByKind(int kind, void *previous);
float MMathCalcDist(Vector4 *a, Vector4 *b);
void sefRunEffectFrames(struct SefEffectObject *effectObject, int timer, float remaining, float value);
void slotEffectStartOnce(void *entry, int mode);
void sbNativeHandlerMessagePost(int kind, int value, int extra);
void RgDispModelSetLocal(EffectActor *self, int timer);
extern unsigned char gGameLoopRuntimeStorage[];

/* Final stage: counts the timer down, damages the slots of kind 6 inside the radius, and releases itself at zero. */
void func_001C7E08(EffectActor *self) {
    EffectProgress *progress;
    EffectListEntry *entry;
    int timer;
    int remaining;
    int oldTimer;
    int remainingTimer;
    unsigned short *activeEffectCounter;
    int counterValue;

    progress = &self->progress;
    entry = 0;
    if (self->flags & 0x100000) {
        timer = progress->timer;
        remaining = timer - 30;
        self->flags = (self->flags & 0xFFEFFFFF) | 0x400000;
        if (remaining > 0) {
            progress->timer = timer - remaining;
            sefRunEffectFrames(progress->effectObject, timer, (float) remaining, 10.0f);
            oldTimer = progress->timer;
        } else {
            oldTimer = timer;
        }
    } else {
        oldTimer = progress->timer;
    }
    remainingTimer = oldTimer - 1;
    progress->timer = remainingTimer;
    if (remainingTimer <= 0) {
        activeEffectCounter = (unsigned short *) gGameLoopRuntimeStorage;
        counterValue = activeEffectCounter[0xB8];
        activeEffectCounter[0xB8] = counterValue - 1;
        RgDispModelSetLocal(self, oldTimer);
        return;
    }
    if (!(self->flags & 0x400000)) {
        while ((entry = RgDispModelNextByKind(6, entry)) != 0) {
            if (entry->flags & 0x2000) {
                continue;
            }
            if (!(entry->flags & 0x20)) {
                continue;
            }
            if (MMathCalcDist(&self->position, &entry->position) < self->state->triggerRadius) {
                slotEffectStartOnce(entry, 0);
                sbNativeHandlerMessagePost(5, entry->id28, 0);
            }
        }
    }
}
