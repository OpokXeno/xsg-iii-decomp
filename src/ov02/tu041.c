/*
 * OV02 original TU 41: 0x00a8e830..0x00a8ed30 (6 functions)
 */
#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
s32 func_00A8E8C0(void *actor, s32 variant);
s32 func_00A8EA30(void *actor);
s32 func_00A8EAF0(void *actor);
s32 func_00A8EBA8(void *actor);

/* Per-actor block reached through the pointer at +0x18 of the actor. */
typedef struct ActorBlock {
    u8 unk_00[0x06];
    u8 actorUpdateFlags;   /* +0x06: bit 0 blocks the update, bit 5 selects func_00A8EAF0 */
    u8 unk_07[0x1E - 0x07];
    s8 handlerSelector;    /* +0x1E: high nibble selects func_00A8E8C0 */
} ActorBlock;

/* Actor whose only field used here is the block pointer at +0x18. */
typedef struct ActorWithBlock {
    u8 unk_00[0x18];
    ActorBlock *actorBlock;
} ActorWithBlock;

void actorBlockUpdate(ActorWithBlock *actor) {
    ActorBlock *actorBlock = actor->actorBlock;
    s32 handlerSelect = actorBlock->handlerSelector & 0xF0;
    s32 handled;

    if (!(actorBlock->actorUpdateFlags & 1 & 0xFF)) {
        handled = 0;
        if (handlerSelect != 0) {
            func_00A8E8C0(actor, handlerSelect);
            handled = 1;
        }
        if (handled == 0) {
            if (actorBlock->actorUpdateFlags & 0x20) {
                func_00A8EAF0(actor);
            } else {
                func_00A8EA30(actor);
            }
        }
    }
    func_00A8EBA8(actor);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu041", func_00A8E8C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu041", func_00A8EA30);

INCLUDE_ASM("asm/nonmatchings/ov02/tu041", func_00A8EAF0);

void textEndScene(void);

void ov02PendingCommit(void) {
    textEndScene();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu041", func_00A8EBA8);
