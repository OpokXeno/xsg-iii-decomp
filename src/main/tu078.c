#include "common.h"

#include "main/xenoActor.h"
typedef int s32;

typedef struct ActorTransform {
    unsigned char unmodeled_00[0x54];
    float heading; /* +0x54: MMathCalcDir result */
} ActorTransform;

typedef struct ActorSlotState {
    unsigned char unmodeled_00[0x8];
    s32 slotLimit;    /* +0x08 */
    short slotCursor; /* +0x0C */
    unsigned char unmodeled_0e[0x90 - 0x0E];
    short slotState; /* +0x90 */
    unsigned char unmodeled_92[0xD0 - 0x92];
    AlignedVector4 aimPoint; /* +0xD0: x, y, z from the caller; w = MMathCalcDir result */
} ActorSlotState;

typedef struct ActorSpawnRequest {
    AlignedVector4 position; /* +0x00: copy of the aim point */
    s32 kind;                /* +0x10 */
    s32 ownerModel;               /* +0x14 */
} ActorSpawnRequest;

typedef union ModelFlags {
    s32 bits;
} ModelFlags; /* kept: the union member access decides the emitted order */

typedef struct ActorObject {
    ModelFlags modelFlags; /* +0x00: bit 0x100 */
    unsigned char unmodeled_04[0x24 - 0x04];
    ActorTransform *transform; /* +0x24 */
    unsigned char unmodeled_28[0x2C - 0x28];
    s32 parentHandle; /* +0x2C: passed to flatSlotSpawn in xenoSpawnerStartAim */
    unsigned char unmodeled_30[0x40 - 0x30];
    float position[4]; /* +0x40 */
    unsigned char unmodeled_50[0x150 - 0x50];
    ActorSlotState spawnerState; /* +0x150 */
} ActorObject;

ActorObject *RgDispModelFindByKey(s32 kind);
float MMathCalcDir(float *from, float *to);
void flatSlotSpawn(ActorSpawnRequest *spawnRequest);
s32 Get_Rnd_001B8CD0(s32 min, s32 max);
void func_001B8D08(s32 unk0, s32 unk1, s32 unk2);

/* Starts the spawner state (limit 4, state 7) aimed at (x, y, z), queues a kind-10 spawn request and rolls a random index. */
s32 xenoSpawnerStartAim(float x, float y, float z) {
    ActorObject *ownerModel;
    ActorSlotState *spawnerState;
    ActorSpawnRequest spawnRequest;
    s32 randomIndex;

    ownerModel = RgDispModelFindByKey(0);
    spawnerState = &ownerModel->spawnerState;
    spawnerState->slotLimit = 4;
    spawnerState->slotState = 7;
    spawnerState->aimPoint.x = x;
    spawnerState->aimPoint.y = y;
    spawnerState->aimPoint.z = z;
    spawnerState->aimPoint.w = 1.0f;
    ownerModel->transform->heading = MMathCalcDir(&ownerModel->position[0], &spawnerState->aimPoint.x);
    __asm__ __volatile__("lq $8, 0(%1)\n"
                         "sq $8, 0(%0)\n"
                         :
                         : "r"(&spawnRequest.position), "r"(&spawnerState->aimPoint)
                         : "$8", "memory");
    spawnRequest.kind = 10;
    spawnRequest.ownerModel = ownerModel->parentHandle;
    flatSlotSpawn(&spawnRequest);
    randomIndex = Get_Rnd_001B8CD0(0, 0x38);
    func_001B8D08(0, 0x38, randomIndex - 1);
    return 1;
}

ActorObject *RgDispModelNextByKind(s32 kind, ActorObject *previous);
void func_001C1320(ActorObject *targetModel, s32 level);
void playerSoundEffectStart(ActorObject *ownerModel, ActorObject *targetModel);

/* Aims the owner model at the first kind-0x10000 model and sets its slot cursor (15 when flag 0x100 is set, else 10). */
s32 xenoSpawnerAimAtTarget(void) {
    ActorObject *ownerModel;
    ActorObject *targetModel;
    ActorSlotState *spawnerState;
    s32 level;

    ownerModel = RgDispModelFindByKey(0);
    targetModel = RgDispModelNextByKind(0x10000, 0);
    if (targetModel == 0) {
        return 0;
    }
    spawnerState = &ownerModel->spawnerState;
    spawnerState->slotState = 3;
    spawnerState->slotLimit = 5;
    ownerModel->transform->heading = MMathCalcDir(&ownerModel->position[0], &targetModel->position[0]);
    func_001C1320(targetModel, spawnerState->slotCursor = (ownerModel->modelFlags.bits & 0x100) ? 15 : 10);
    playerSoundEffectStart(ownerModel, targetModel);
    return 1;
}

XenoActor *func_0028D670(void);
s32 func_001B93C0(s32 kind);
s32 payloadSectionsLocate(XenoActor *actor, s32 effectHandle);
void XenoActorRelease(s32 actor);
extern float D_003E481C[];

/* Allocates an actor bound to effect kind 0x16 with zero position and rotation and a uniform scale from D_003E481C; releases it on failure. */
XenoActor *func_001C5070(void) {
    XenoActor *actor;
    s32 effectHandle;

    actor = func_0028D670();
    effectHandle = func_001B93C0(0x16);
    if (payloadSectionsLocate(actor, effectHandle) == 0) {
        XenoActorRelease((s32) actor);
        return 0;
    }
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&actor->position) : "memory");
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&actor->rotation) : "memory");
    __asm__ __volatile__("mfc1 $8, %0\n"
                         "qmtc2.ni $8, vf1\n"
                         "vaddx.xyz vf2, vf0, vf1x\n"
                         "vmove.w vf2, vf0\n"
                         "sqc2 vf2, 0(%1)\n"
                         :
                         : "f"(D_003E481C[0]), "r"(&actor->scale)
                         : "$8", "memory");
    actor->drawFlags |= 4;
    return actor;
}

typedef struct ActorSlotBlock {
    unsigned char unk_00[0x10];
    s32 sefResource;
    unsigned char unk_14[0x80];
    s32 firstActor;
    s32 secondActor;
    s32 thirdActor;
} ActorSlotBlock;

typedef struct ActorSlotHost {
    unsigned char unk_000[0x150];
    ActorSlotBlock actorSlots;
} ActorSlotHost;

void sefEffectHandleRelease(s32 sefResource);

/* Releases the three actors and the sef effect held by the spawner state block. */
void xenoActorReleaseSlots(ActorSlotHost *host) {
    ActorSlotBlock *actorSlotBlock;
    s32 firstActor;
    s32 secondActor;
    s32 thirdActor;
    s32 sefResource;

    actorSlotBlock = &host->actorSlots;
    firstActor = actorSlotBlock->firstActor;
    if (firstActor != 0) {
        XenoActorRelease(firstActor);
    }
    secondActor = actorSlotBlock->secondActor;
    if (secondActor != 0) {
        XenoActorRelease(secondActor);
    }
    thirdActor = actorSlotBlock->thirdActor;
    if (thirdActor != 0) {
        XenoActorRelease(thirdActor);
    }
    sefResource = actorSlotBlock->sefResource;
    if (sefResource != 0) {
        sefEffectHandleRelease(sefResource);
    }
}
