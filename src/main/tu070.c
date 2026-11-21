#include "common.h"

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
    u8 collisionBlock[0x6AA - 0x6A0];     /* 0x6A0, passed to func_002A70E8 */
    s16 collisionFlags;                       /* 0x6AA */
    u8 unk_6AC[0x6B3 - 0x6AC];
    s8 collisionMode;                        /* 0x6B3 */
    u8 unk_6B4[0x6BC - 0x6B4];
    s32 collisionHandle;                     /* 0x6BC */
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

void actorChildBindMode(ActorOwner *self, s32 handleId) {
    f32 baseScale;
    s32 collisionHandle;
    s32 storedHandleId;
    ActorChild *child;

    storedHandleId = handleId;
    self->handleId = storedHandleId;
    child = self->child;
    if (child != 0) {
        func_002A70E8(child->collisionBlock);
        child->collisionFlags = 0x103;
        child->collisionMode = 3;
        collisionHandle = func_002A6FA8(0x100, self->handleId);
        baseScale = child->baseScale;
        child->copiedScale = baseScale;
        child->collisionHandle = collisionHandle;
    }
}

void actorChildNoop(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu070", func_001C08E0);

INCLUDE_ASM("asm/main/nonmatchings/tu070", func_001C09A0);

INCLUDE_ASM("asm/main/nonmatchings/tu070", func_001C0A48);
