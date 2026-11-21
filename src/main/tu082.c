#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu082", func_001C6E10);

INCLUDE_ASM("asm/main/nonmatchings/tu082", func_001C7000);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* Child block of an object (pointer at 0x24 of struct ChildOwningObject). */
struct ChildStateBlock {
    u8 unk_00[0x1C];
    f32 scale;
    u8 unk_20[0x6A0 - 0x20];
    u8 collisionObjectHead[0x0A];
    s16 collisionBits;
    u8 unk_6AC[0x6B3 - 0x6AC];
    s8 collisionProbeMode;
    u8 unk_6B4[0x6BC - 0x6B4];
    s32 collisionBlock;
    u8 unk_6C0[0x6CC - 0x6C0];
    f32 savedScale;
};

struct ChildOwningObject {
    s32 ownerFlagBits;
    u8 unk_04[0x1C - 0x04];
    u8 *hooks;
    u8 unk_20[0x24 - 0x20];
    struct ChildStateBlock *child;
    u8 unk_28[0x2C - 0x28];
    s32 ownerId;
    u8 unk_30[0x154 - 0x30];
    s32 unk_154;
};

void sceneEmptyHook(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu082", func_001C7060);

INCLUDE_ASM("asm/main/nonmatchings/tu082", func_001C7100);

s32 func_002A6FA8(s32 resourceKind, s32 ownerId);
void func_002A70E8(u8 *block);

void objectSetChildId(struct ChildOwningObject *owner, s32 newId) {
    f32 scale;
    s32 collisionBlock;
    s32 storedOwnerId;
    struct ChildStateBlock *child;

    storedOwnerId = newId;
    owner->ownerId = storedOwnerId;
    child = owner->child;
    if (child != 0) {
        func_002A70E8(child->collisionObjectHead);
        child->collisionProbeMode = 3;
        child->collisionBits = 0;
        collisionBlock = func_002A6FA8(0x100, owner->ownerId);
        scale = child->scale;
        child->savedScale = scale;
        child->collisionBlock = collisionBlock;
    }
}

void func_001C1590(struct ChildOwningObject *owner, s32 mode);
extern unsigned char func_001C76B8[];

void objectSetDefaults(struct ChildOwningObject *owner)
{
  s32 updatedFlagBits;
  updatedFlagBits = (owner->ownerFlagBits & (~8)) | 0x11000;
  owner->hooks = func_001C76B8;
  owner->unk_154 = 0;
  owner->ownerFlagBits = updatedFlagBits;
  func_001C1590(owner, 1);
}
