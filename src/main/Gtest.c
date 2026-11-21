#include "common.h"

typedef signed char s8;
typedef int s32;
s32 XenoActorEnableChannel();
s32 XenoActorSetMode();

/* The actor record XenoActorEnableAndSetMode reads: the primary actor handle at +0x50, the secondary one at +0x54. */
typedef struct ActorPairRecord {
    unsigned char unmodeled_00[0x50];
    s32 primary;                        /* +0x50 */
    s32 secondary;                      /* +0x54 */
} ActorPairRecord;

s32 XenoActorEnableAndSetMode(ActorPairRecord *actorPair, s32 actorMode) {
    s32 secondary;

    XenoActorEnableChannel(actorPair->primary);
    XenoActorSetMode(actorPair->primary, actorMode);
    secondary = actorPair->secondary;
    if (secondary != 0) {
        secondary = XenoActorSetMode(secondary, actorMode);
    }
    return secondary;
}

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00245700);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_002459C0);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00245C70);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00245FE0);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246048);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_002461C0);

void XenoActorEnableAndSetModeGroup(s32 *actorGroup, s32 actorMode) {
    s32 *pSecondaryEntry;
    s32 secondaryActor;
    s32 i;

    pSecondaryEntry = actorGroup + 1;
    XenoActorEnableChannel(actorGroup[0]);
    XenoActorSetMode(actorGroup[0], actorMode);
    for (i = 0; i < 4; i++) {
        secondaryActor = *pSecondaryEntry;
        pSecondaryEntry += 1;
        if (secondaryActor != 0) {
            XenoActorSetMode(secondaryActor, actorMode);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_002463A8);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246480);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246558);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246650);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246710);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246A68);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246B00);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246C18);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00246D70);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00247130);

INCLUDE_ASM("asm/main/nonmatchings/Gtest", func_00247790);
