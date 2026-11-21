#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu084", func_001C7A18);

typedef unsigned char u8;
typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;

/* Child block reached from ActorOwner.child. */
typedef struct ActorChild {
    u8 unk_00[0x10];
    f32 baseScale;                /* 0x10 */
    u8 unk_14[0x6A0 - 0x14];
    u8 collisionObjectHead[0x6AA - 0x6A0];     /* 0x6A0, passed to func_002A70E8 */
    s16 collisionBits;                       /* 0x6AA */
    u8 unk_6AC[0x6B3 - 0x6AC];
    s8 collisionProbeMode;                        /* 0x6B3 */
    u8 unk_6B4[0x6BC - 0x6B4];
    s32 collisionBlock;                     /* 0x6BC */
    u8 unk_6C0[0x6CC - 0x6C0];
    f32 copiedScale;                /* 0x6CC */
} ActorChild;

/* Owner record: child pointer at +0x24, stored state value at +0x2C. */
typedef struct ActorOwner {
    u8 unk_00[0x24];
    ActorChild *child;
    u8 unk_28[0x2C - 0x28];
    s32 handleId;
} ActorOwner;

s32 func_002A6FA8();
void func_002A70E8();

void actorChildBindLayer(ActorOwner *owner, s32 handleId) {
    f32 baseScale;
    s32 collisionBlock;
    s32 storedHandleId;
    ActorChild *child;

    storedHandleId = handleId;
    owner->handleId = storedHandleId;
    child = owner->child;
    if (child != 0) {
        func_002A70E8(child->collisionObjectHead);
        child->collisionBits = 0x102;
        child->collisionProbeMode = 3;
        collisionBlock = func_002A6FA8(0x100, owner->handleId);
        baseScale = child->baseScale;
        child->copiedScale = baseScale;
        child->collisionBlock = collisionBlock;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu084", func_001C7BD0);
