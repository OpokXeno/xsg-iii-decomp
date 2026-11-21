/*
 * OV04 original TU 7: 0x00a974e0..0x00a98450 (8 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A974E0);

typedef struct MovingActorSlot {
    unsigned char unmodeled_00[4];
    struct XenoActor *actor;
    unsigned char unmodeled_08[664];
} MovingActorSlot;
typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
/* The part of XenoActor the channel functions write: a float at +0x574, the second word of shadowB2 (include/main/xenoActor.h). */
typedef struct ActorShadowView {
    unsigned char unmodeled_000[0x574];
    f32 shadowB2Value;                  /* +0x574 */
} ActorShadowView;
extern MovingActorSlot *gMovingActorSlots;
s32 XenoActorCheckMode();
s32 XenoActorEnableChannel();
s32 XenoActorSetChannelValue(struct XenoActor *, f32);

void actorTableStartChannelValue(s16 actorIndex, s16 actorSelector, f32 channelValue) {
    MovingActorSlot *actorSlot;
    f32 saved_value;
    s16 saved_selector;
    struct XenoActor *actor;

    saved_selector = actorSelector;
    actorSlot = &gMovingActorSlots[actorIndex];
    saved_value = channelValue;
    actor = actorSlot->actor;
    XenoActorSetChannelValue(actor, 5.0f);
    XenoActorEnableChannel(actor);
    XenoActorCheckMode(actor, saved_selector);
    ((ActorShadowView *) actor)->shadowB2Value = saved_value;
}

struct XenoActor;

/* 672-byte table entry; the XenoActor pointer sits at offset 4. */

void XenoActorSetChannelValueB(struct XenoActor *actor, float channelValue);
void XenoActorEnableChannelB(struct XenoActor *actor);
void XenoActorSetSelector(struct XenoActor *actor, int actorSelector);

void actorTableStartChannelSelector(short actorIndex, short actorSelector) {
    MovingActorSlot *actorSlot;
    struct XenoActor *actor;

    actorSlot = gMovingActorSlots + actorIndex;
    actor = actorSlot->actor;
    XenoActorSetChannelValueB(actor, 5.0f);
    XenoActorEnableChannelB(actor);
    XenoActorSetSelector(actor, actorSelector);
}

void actorTableResetChannel(s16 actorIndex) {
    MovingActorSlot *actorSlot;
    struct XenoActor *actor;

    actorSlot = &gMovingActorSlots[actorIndex];
    actor = actorSlot->actor;
    XenoActorCheckMode(actor, 0);
    XenoActorSetChannelValue(actor, 4.0f);
    ((ActorShadowView *) actor)->shadowB2Value = 1.0f;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97BE8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97CD0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97D90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97EB8);
