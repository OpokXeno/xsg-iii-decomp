/*
 * OV02 original TU 24: 0x00a8ac10..0x00a8b2d0 (11 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8AC10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8ACD8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8ACE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8AD20);

#include "main/xenoActor.h"
#include "ov02/tu060.h"

/* Partial battle unit: the stats record at +0x2B0 and the actor at +0x2B4. */
struct BattleStats {
    unsigned char unmodeled_000[0x38];
    int gate;                               /* +0x38: nonzero takes the func_00A8ACD8 branch of unitRefreshPhase */
    unsigned char unmodeled_03C[0x3E - 0x3C];
    short key;                              /* +0x3E */
};


void XenoActorSetChannelValueB(struct XenoActor *actor, float scale);
void XenoActorEnableChannelB(struct XenoActor *actor);
int XenoActorSetSelector(struct XenoActor *actor, int arg);

/* Sets the unit's actor up for the phase: unless its flag word has bit 0x200, it is set to scale 7.0 and
 * updated, then the phase call returns the XenoActorSetSelector result. */
int func_00A8AF18(struct BattleUnit *unit, int arg)
{
    struct XenoActor *actor;

    actor = unit->unit;
    if ((actor->resetFlags & 0x200) == 0) {
        XenoActorSetChannelValueB(actor, 7.0f);
        XenoActorEnableChannelB(actor);
    }
    return XenoActorSetSelector(actor, arg);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8AF78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8B050);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8B098);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8B110);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", unitRefreshPhase);

INCLUDE_ASM("asm/nonmatchings/ov02/tu024", func_00A8B210);
