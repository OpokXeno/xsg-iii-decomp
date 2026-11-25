#include "common.h"

typedef unsigned char u8;

typedef int s32;

typedef float f32;

/* Owner record returned by RgDispModelAlloc; the layout of tu070's ActorOwner. */
typedef struct ActorOwner {
    u8 unmodeled_00[0x10];
    s32 childAllocSize; /* 0x10: 128 */
    u8 unmodeled_14[0x1C - 0x14];
    void (*callback)(void); /* 0x1C: actorChildNoop */
    u8 unmodeled_20[0x24 - 0x20];
    void *child;      /* 0x24 */
    s32 unmodeled_28; /* 0x28: the first word of the argument */
    s32 handleId;     /* 0x2C */
    s32 selector;     /* 0x30 */
} ActorOwner;

/* Block returned by func_0028D670 and initialised here. */
typedef struct ActorBlock {
    u8 unmodeled_00[4];
    f32 zeroedFloat04; /* 0x04 */
    u8 unmodeled_08[0x10 - 0x08];
    f32 extentX; /* 0x10 */
    f32 extentY; /* 0x14 */
    u8 unmodeled_18[0x1C - 0x18];
    f32 extentW; /* 0x1C */
    u8 unmodeled_20[0x90 - 0x20];
} ActorBlock;

void actorChildNoop(void);
void actorChildBindMode(ActorOwner *self, s32 handleId);
void RgDispModelSetLocal(ActorOwner *owner);
ActorOwner *RgDispModelAlloc(s32 value);
ActorBlock *func_0028D670(void);
s32 func_0027A7E8(ActorBlock *actorBlock);
void slotSetValueSelector(s32 value, s32 mode);
void func_0027AB00(s32 value);

/* Creates the kind-requested slot object with a 128-byte actor block, binds its handle mode and registers the selector; returns 0 when no slot or actor is free. */
s32 actorChildSlotCreate(s32 *slotRequest) {
    ActorOwner *owner;
    ActorBlock *actorBlock;
    s32 actorBlockSize;
    s32 spawnKind;
    f32 actorExtent;
    u8 *scaleVector;
    s32 handleId;

    owner = RgDispModelAlloc(*slotRequest);
    if (owner == 0) {
        return 0;
    }

    spawnKind = *slotRequest;
    actorBlockSize = 128;
    owner->childAllocSize = actorBlockSize;
    owner->callback = actorChildNoop;
    owner->unmodeled_28 = spawnKind;
    actorBlock = func_0028D670();
    owner->child = actorBlock;
    if (actorBlock == 0) {
        RgDispModelSetLocal(owner);
        return 0;
    }
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(actorBlock->unmodeled_00 + 0x10) : "memory");
    actorBlock->zeroedFloat04 = 0.0f;
    actorExtent = 0.64f;
    actorBlock->extentX = actorExtent;
    actorBlock->extentY = 1.5f;
    actorBlock->extentW = actorExtent;
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(actorBlock->unmodeled_00 + 0x20) : "memory");
    __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(actorBlock->unmodeled_00 + 0x50) : "memory");
    scaleVector = actorBlock->unmodeled_00 + 0x80;
    __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(scaleVector) : "memory");
    do {
        handleId = -1;
        actorChildBindMode(owner, handleId);
    } while (0);
    owner->selector = func_0027A7E8(actorBlock);
    slotSetValueSelector(owner->selector, 0);
    func_0027AB00(owner->selector);
    return 1;
}
