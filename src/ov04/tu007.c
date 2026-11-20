/*
 * OV04 original TU 7: 0x00a974e0..0x00a98450 (8 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A974E0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97A70);

struct XenoActor;

/* 672-byte table entry; the XenoActor pointer sits at offset 4. */
typedef struct ActorTableEntry {
    unsigned char unmodeled_00[4];
    struct XenoActor *actor;
    unsigned char unmodeled_08[664];
} ActorTableEntry;

extern ActorTableEntry *D_00AA9074;
void XenoActorSetChannelValueB(struct XenoActor *actor, float value);
void XenoActorEnableChannelB(struct XenoActor *actor);
void XenoActorSetSelector(struct XenoActor *actor, int selector);

void func_00A97B00(short index, short selector) {
    ActorTableEntry *entry;
    struct XenoActor *actor;

    entry = D_00AA9074 + index;
    actor = entry->actor;
    XenoActorSetChannelValueB(actor, 5.0f);
    XenoActorEnableChannelB(actor);
    XenoActorSetSelector(actor, selector);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97B78);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97BE8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97CD0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97D90);

INCLUDE_ASM("asm/nonmatchings/ov04/tu007", func_00A97EB8);
