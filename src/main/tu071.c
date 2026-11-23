#include "common.h"

#include "shared.h"
#include "quadword.h"

/* Two words at +0x6A0 of the actor that the update callback clears for the simple kinds. */
typedef struct ActorCollisionPair {
    u32 firstWord;
    u32 secondWord;
} ActorCollisionPair;

/* Actor reached through SpawnSlot.actor. */
typedef struct SlotActor {
    u8 unmodeled_00[0x20];
    AlignedVector4 position;
    u8 unmodeled_30[0x20];
    AlignedVector4 vector50;
    u8 unmodeled_60[0x20];
    AlignedVector4 scale;
    u8 unmodeled_90[0x610];
    ActorCollisionPair collisionPair;
} SlotActor;

/* Effect object whose +0x30 vector holds the point of the attached effect. */
typedef struct EffectHandle {
    u8 unmodeled_00[0x30];
    AlignedVector4 point;
} EffectHandle;

/* Spawner state block at +0x150 of the slot; its second word is the attached effect. */
typedef struct SpawnerState {
    u8 unmodeled_00[8];
    EffectHandle *effect;
} SpawnerState;

/* Slot object of RgDispModelAlloc (stride 0x230); only the fields these setters touch. */
typedef struct SpawnSlot {
    u8 unmodeled_00[0x10];
    u32 spawnKind;
    u8 unmodeled_14[0x10];
    SlotActor *actor;
    u8 unmodeled_28[0x18];
    AlignedVector4 position;
    AlignedVector4 velocity;
    AlignedVector4 rotation;
    u8 unmodeled_70[0x10];
    AlignedVector4 scale;
    u8 unmodeled_90[0xC0];
    SpawnerState spawnerState;
    u8 unmodeled_15c[0x34];
    AlignedVector4 effectOffset;
} SpawnSlot;

void func_001EAA08(void *effect);
void func_00284E30(void *spawnerState, AlignedVector4 *source, AlignedVector4 *sum);
void func_001EAA70(void *effect, AlignedVector4 *source);
void func_001EAA98(void *effect, AlignedVector4 *source);

/* Sets the slot position and forwards it to the actor or effect that the slot spawnKind drives. */
void SpawnSlotSetPosition(SpawnSlot *slot, AlignedVector4 *newPosition) {
    SlotActor *actor;
    AlignedVector4 *actorVector;
    AlignedVector4 shiftedPosition;
    u32 spawnKind;

    QuadwordCopyT0(&slot->position, newPosition);
    QuadwordCopyT0(&slot->velocity, newPosition);
    spawnKind = slot->spawnKind;
    switch (spawnKind) {
    case 1:
    case 2:
    case 4:
        actor = slot->actor;
        if (actor != 0) {
            ActorCollisionPair *collisionWords = &actor->collisionPair;
            collisionWords->firstWord = 0;
            collisionWords->secondWord = 0;
        }
        actorVector = &actor->position;
        break;
    case 64:
    case 128:
    case 512:
    case 1024:
    case 0x8000:
    case 0x10000:
        actor = slot->actor;
        actorVector = &actor->position;
        break;
    case 256:
    case 8192: {
        SpawnerState *spawnerState = &slot->spawnerState;
        QuadwordCopyT0(&slot->actor->position, newPosition);
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                             "vmove.w vf1, vf0\n"
                             "sqc2 vf1, 48(%1)\n"
                             :
                             : "r"(newPosition), "r"(spawnerState->effect)
                             : "memory");
    }
        return;
    case 4096: {
        SpawnerState *spawnerState = &slot->spawnerState;
        __asm__ __volatile__("lqc2 vf1, 0(%0)\n"
                             "lqc2 vf2, 0(%1)\n"
                             "vadd.xyz vf1, vf1, vf2\n"
                             "sqc2 vf1, 0(%2)\n"
                             :
                             : "r"(newPosition), "r"(&slot->effectOffset), "r"(&shiftedPosition)
                             : "memory");
        func_00284E30(spawnerState, newPosition, &shiftedPosition);
    }
        return;
    case 16384:
        func_001EAA08(slot->spawnerState.effect);
        return;
    case 16:
    case 0x20000:
        actorVector = &slot->actor->position;
        break;
    default:
        return;
    }
    QuadwordCopyT0(actorVector, newPosition);
}

/* Copies a vector with one lq/sq collisionWords; kept as an inline helper, the memory operands decide the emitted order. */
static inline void CopyVectorMem(AlignedVector4 *dest, const AlignedVector4 *src) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(src), "m"(src->x), "m"(src->y), "m"(src->z), "m"(src->w)
                         : "$8");
}

/* Sets the slot rotation and forwards it to the actor or effect that the slot spawnKind drives. */
void SpawnSlotSetRotation(SpawnSlot *slot, AlignedVector4 *newRotation) {
    CopyVectorMem(&slot->rotation, newRotation);
    switch (slot->spawnKind) {
    case 16384:
        func_001EAA70(slot->spawnerState.effect, newRotation);
        break;
    case 1:
    case 2:
    case 4:
    case 16:
    case 64:
    case 128:
    case 512:
    case 1024:
    case 32768:
    case 65536:
    case 131072:
        CopyVectorMem(&slot->actor->vector50, newRotation);
        break;
    }
}

/* Sets the slot scale and forwards it to the actor or effect that the slot spawnKind drives. */
void SpawnSlotSetScale(SpawnSlot *slot, AlignedVector4 *newScale) {
    CopyVectorMem(&slot->scale, newScale);
    switch (slot->spawnKind) {
    case 16384:
        func_001EAA98(slot->spawnerState.effect, newScale);
        break;
    case 1:
    case 2:
    case 4:
    case 16:
    case 64:
    case 128:
    case 512:
    case 1024:
    case 32768:
    case 65536:
    case 131072:
        CopyVectorMem(&slot->actor->scale, newScale);
        break;
    }
}
