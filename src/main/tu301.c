#include "common.h"
#include "main/xenoActor.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef short s16;

/* Owner record passed to the xenoActor channel helpers: the actor pointer at +4, a 16-bit slot at +0xC. */
struct XenoChannelOwner {
    u8 pad_00[4];
    XenoActor *actor;
    u8 pad_08[4];
    s16 channelSlot;
};

void XenoActorSetChannelValue(XenoActor *actor, f32 value);
void XenoActorEnableChannel(XenoActor *actor);
void XenoActorSetModeInSlot(XenoActor *actor, u32 index, s32 slot);

void xenoActorActivateChannel(struct XenoChannelOwner *owner) {
    XenoActorSetChannelValue(owner->actor, 8.0f);
    XenoActorEnableChannel(owner->actor);
    XenoActorSetModeInSlot(owner->actor, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu301", func_00282A68);

void func_00279FB8(s16 slot);

void xenoActorClearResetFlags(struct XenoChannelOwner *owner) {
    s32 remainingResetFlags;
    XenoActor *actor;

    actor = owner->actor;
    remainingResetFlags = actor->resetFlags & ~3;
    actor->resetFlags = remainingResetFlags;
    XenoActorSetChannelValue(actor, 10.0f);
    func_00279FB8(owner->channelSlot);
}

INCLUDE_ASM("asm/main/nonmatchings/tu301", func_00283118);

s16 *slotInstanceFetch(s32 index, s32 kind, s32 slot);

s32 xenoActorWriteSlotWord(s32 entryNumber, s32 unmaskedValue) {
    s32 storedHalfword;

    storedHalfword = unmaskedValue & 0xFFFF;
    *slotInstanceFetch(entryNumber, 0, 8) = storedHalfword;
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu301", func_00283710);

INCLUDE_ASM("asm/main/nonmatchings/tu301", func_00283940);

void xenoActorSetResetFlags(struct XenoChannelOwner *owner) {
    s32 combinedResetFlags;
    XenoActor *actor;

    actor = owner->actor;
    combinedResetFlags = actor->resetFlags | 3;
    actor->resetFlags = combinedResetFlags;
    XenoActorSetChannelValue(actor, 0.0f);
}

INCLUDE_ASM("asm/main/nonmatchings/tu301", func_00283C30);
