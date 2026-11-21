/*
 * OV02 original TU 24: 0x00a8ac10..0x00a8b2d0 (11 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8AC10);

#include "main/xenoActor.h"
#include "ov02/tu060.h"
struct BattleStats {
    unsigned char unmodeled_000[0x38];
    int gate;                               /* +0x38: nonzero takes the unitActorKeyNext branch of unitRefreshPhase */
    unsigned char unmodeled_03C[0x3E - 0x3C];
    short key;                              /* +0x3E */
};
typedef signed char s8;
typedef int s32;

s32 unitActorKeyNext(struct BattleUnit *battleUnit) {
    return battleUnit->motionActor->currentMotionSlot + 1;
}

s32 XenoActorSetMode();
s32 func_0028D760();

void unitResetActor(struct XenoActor *actor) {
    extern s32 XenoActorSetChannelValue(struct XenoActor *, f32);
    func_0028D760(actor);
    XenoActorSetChannelValue(actor, 0.0f);
    XenoActorSetMode(actor, 0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8AD20);

#include "main/xenoActor.h"
#include "ov02/tu060.h"

/* Partial battle unit: the stats record at +0x2B0 and the actor at +0x2B4. */








void XenoActorSetChannelValueB(struct XenoActor *actor, float scale);
void XenoActorEnableChannelB(struct XenoActor *actor);
int XenoActorSetSelector(struct XenoActor *actor, int arg);

/* Sets the unit's actor up for the phase: unless its flag word has bit 0x200, it is set to scale 7.0 and
 * updated, then the phase call returns the XenoActorSetSelector result. */
int unitSetupPhaseActor(struct BattleUnit *battleUnit, int selector)
{
    struct XenoActor *actor;

    actor = battleUnit->motionActor;
    if ((actor->resetFlags & 0x200) == 0) {
        XenoActorSetChannelValueB(actor, 7.0f);
        XenoActorEnableChannelB(actor);
    }
    return XenoActorSetSelector(actor, selector);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8AF78);

void unitClearSlotCounters(struct BattleUnit *battleUnit) {
    s32 childIndex;
    struct XenoActor **childSlot;
    struct XenoActor *actor;
    struct XenoActor *slotActor;

    actor = battleUnit->motionActor;
    childSlot = battleUnit->children;
    actor->rsrc[5] = 0;             /* +0x518 */
    actor->rsrc[6] = 0;             /* +0x51C */
    for (childIndex = 0; childIndex < 4; childIndex++) {
        slotActor = *childSlot;
        childSlot += 1;
        if (slotActor != 0) {
            slotActor->rsrc[5] = 0; /* +0x518 */
        }
    }
}

s32 XenoActorEnableChannel();
s32 XenoActorSetModeInSlot();

void unitSetSlotMode(struct BattleUnit *battleUnit, s32 slotNumber) {
    extern s32 XenoActorSetChannelValue(void *, f32);
    s32 updatedResetFlags;
    struct XenoActor *actor;

    actor = battleUnit->motionActor;
    if (actor->rsrc[6] != 0) {
        updatedResetFlags = actor->resetFlags | 2;
        actor->resetFlags = updatedResetFlags;
        XenoActorSetChannelValue(actor, 0.0f);
        XenoActorEnableChannel(actor);
        XenoActorSetModeInSlot(actor, 2, slotNumber - 1);
    }
}

s32 XenoActorIsDone(struct XenoActor *actor);

void unitIsActorDone(struct BattleUnit *battleUnit) {
    XenoActorIsDone(battleUnit->motionActor);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", unitRefreshPhase);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8B210);
