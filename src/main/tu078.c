#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu078", func_001C4EF0);

INCLUDE_ASM("asm/main/nonmatchings/tu078", func_001C4FC8);

INCLUDE_ASM("asm/main/nonmatchings/tu078", func_001C5070);

typedef int s32;

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

void XenoActorRelease(s32 actor);
void sefEffectHandleRelease(s32 sefResource);

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
