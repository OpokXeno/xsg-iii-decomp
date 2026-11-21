#include "common.h"

typedef int s32;
typedef signed char s8;
typedef float f32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef struct { unsigned char bytes[8]; } Doubleword;

/* The animated object a slot follows. Only the fields these functions touch are named. */
typedef struct FollowedObject {
    u8 unk_00[0x34];
    f32 progress;            /* 0x34: below 1.0 the slot is only released */
    u8 unk_38[0x18];
    u32 startTime;           /* 0x50 */
    u8 unk_54[4];
    u32 endTime;             /* 0x58 */
    s32 handle;              /* 0x5C: passed to func_001D0660 */
    u8 unk_60[0x10];
    f32 position[4];         /* 0x70 */
    f32 scaleX;              /* 0x80 */
    f32 scaleY;              /* 0x84 */
} FollowedObject;

typedef union FollowOffset {
    Doubleword bytes;
    struct { f32 x; f32 y; } xy;
} FollowOffset;

/* One entry of the 8-slot table gFollowSlots (0x40 bytes each). */
typedef struct FollowSlot {
    u32 bindTime;                /* 0x00: nonzero while the slot is in use */
    FollowedObject *owner;        /* 0x04 */
    u8 unk_08[8];
    Doubleword source[2];    /* 0x10: 16 bytes copied from the caller */
    FollowOffset anchorOffset;       /* 0x20 */
    s8 active;               /* 0x28 */
    u8 unk_29[3];
    f32 blend;               /* 0x2C: 1.0 when the slot is set up */
    f32 screenPosition[4];           /* 0x30: written by the VU0 add */
} FollowSlot;

extern FollowSlot gFollowSlots[];
void *memset(void *dest, s32 value, u32 size);
FollowSlot *func_00269648(s32 key);
void blendWeightSetFull(f32 *value);

void followSlotsClear(void) {
    memset(gFollowSlots, 0, 0x200);
}

s32 func_001D0660();
s32 func_00268710(f32 *, f32, f32);
s32 func_00269A78();

void followSlotsUpdate(void) {
    f32 scaledOffset[4];
    FollowSlot *followSlot;
    s32 followSlotNumber;
    FollowedObject *followedObject;
    f32 *screenPosition;

    followSlot = gFollowSlots;
    for (followSlotNumber = 0; followSlotNumber < 8; followSlotNumber++) {
        if ((followSlot->bindTime != 0) && (followSlot->active != 0)) {
            followedObject = followSlot->owner;
            func_00269A78(&followSlot->blend);
            if (followedObject->progress < 1.0f) {
                func_001D0660(followedObject->handle, 0);
            } else {
                func_00268710(&scaledOffset[0], followedObject->scaleX * followSlot->anchorOffset.xy.x, -(followedObject->scaleY * followSlot->anchorOffset.xy.y));
                screenPosition = followSlot->screenPosition;
                __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(followedObject->position) : "memory");
                __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(scaledOffset) : "memory");
                __asm__ __volatile__("vadd.xyzw vf20, vf20, vf21" : : : "memory");
                __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(screenPosition) : "memory");
                func_001D0660(followedObject->handle, screenPosition);
            }
        }
        followSlot++;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu274", func_00269280);

void slotBindOwner(FollowedObject *owner, u32 bindTime) {
    FollowSlot *followSlot;

    followSlot = func_00269648(0);
    if (followSlot != 0) {
        memset(followSlot, 0, 0x40);
        followSlot->owner = owner;
        followSlot->bindTime = bindTime;
    }
}

void slotActivate(s32 key, Doubleword *anchorOffset, Doubleword *source) {
    FollowSlot *followSlot;

    followSlot = func_00269648(key);
    if (followSlot != 0) {
        followSlot->source[0] = source[0];
        followSlot->source[1] = source[1];
        followSlot->anchorOffset.bytes = *anchorOffset;
        followSlot->active = 1;
        blendWeightSetFull(&followSlot->blend);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu274", func_00269648);

INCLUDE_ASM("asm/main/nonmatchings/tu274", func_00269688);

INCLUDE_ASM("asm/main/nonmatchings/tu274", func_00269708);

void slotReleaseOwner(FollowedObject *owner) {
    s32 followSlotNumber;
    u32 startTime;
    u32 endTime;
    u32 bindTime;
    FollowSlot *followSlot;

    followSlot = gFollowSlots;
    for (followSlotNumber = 0; followSlotNumber < 8; followSlotNumber++) {
        if (followSlot->owner == owner) {
            bindTime = followSlot->bindTime;
            startTime = owner->startTime;
            if (bindTime >= startTime) {
                endTime = owner->endTime;
                if (endTime >= bindTime) {
                    if (followSlot->active != 0) {
                        func_001D0660(owner->handle, 0);
                    }
                    memset(followSlot, 0, 0x40);
                }
            }
        }
        followSlot++;
    }
}

s32 func_00269688();

s32 slotAnyActive(void) {
    return func_00269688() != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu274", func_00269820);

INCLUDE_ASM("asm/main/nonmatchings/tu274", func_00269880);
