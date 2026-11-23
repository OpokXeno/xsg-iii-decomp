#include "common.h"

typedef unsigned char u8;

typedef signed char s8;

typedef short s16;

typedef int s32;

typedef float f32;

/* Child sectionTable reached from ActorOwner.child. */
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

/* Stores the handle id and, when the slot has a child actor, rebuilds its collision handle with the child base scale. */
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

typedef struct SectionOwner {
    u8 unk_00[0x504];
    u8 *sectionA; /* 0x504 */
    u8 *sectionB; /* 0x508 */
    u8 *sectionC; /* 0x50C */
    u8 unk_510[0x514 - 0x510];
    u8 *sectionE; /* 0x514 */
    u8 unk_518[0x530 - 0x518];
    u8 *sectionD; /* 0x530 */
} SectionOwner;

void *func_001B6918(void *fromBlock, s32 fieldNumber, s32 wantedValue, s32 fieldMask);
s32 poolBlockTagWordGet(void *payloadAddr, s32 fieldNumber);
/* Fills the four section pointers from the payload block offsets that are marked present. */
s32 payloadSectionsLocate(SectionOwner *self, s32 handle);

/* Locates the pool block of a value (tried as tag 1, then 0) and loads its four payload sections. */
s32 payloadSectionsLoad(SectionOwner *self, s32 blockValue) {
    void *handle;

    self->sectionA = 0;
    self->sectionB = 0;
    self->sectionC = 0;
    self->sectionD = 0;
    if (blockValue == 0) {
        return 1;
    }
    handle = func_001B6918(0, 1, blockValue, -1);
    if (handle == 0) {
        handle = func_001B6918(0, 0, blockValue, -1);
        if (handle == 0) {
            return 0;
        }
    }
    if (poolBlockTagWordGet(handle, 0) == 0) {
        return 0;
    }
    return payloadSectionsLocate(self, handle);
}

typedef struct PayloadSection {
    s32 sectionOffset;  /* from the payload sectionTable */
    s32 present; /* nonzero when the section is stored */
} PayloadSection;

typedef struct PayloadBlock {
    PayloadSection section[4];
} PayloadBlock;

s32 xcLocatePayload(PayloadBlock **out, s32 handle);

s32 payloadSectionsLocate(SectionOwner *self, s32 handle) {
    PayloadBlock *sectionTable;

    self->sectionA = 0;
    self->sectionB = 0;
    self->sectionC = 0;
    self->sectionD = 0;
    if (xcLocatePayload(&sectionTable, handle) == 0) {
        return 0;
    }
    if (sectionTable->section[0].present != 0) {
        self->sectionA = (u8 *) sectionTable + sectionTable->section[0].sectionOffset;
    }
    if (sectionTable->section[1].present != 0) {
        self->sectionB = (u8 *) sectionTable + sectionTable->section[1].sectionOffset;
    }
    if (sectionTable->section[2].present != 0) {
        self->sectionC = (u8 *) sectionTable + sectionTable->section[2].sectionOffset;
    }
    if (sectionTable->section[3].present != 0) {
        self->sectionD = (u8 *) sectionTable + sectionTable->section[3].sectionOffset;
    }
    return 1;
}

void func_00210480(void *payload, s32 zero);

/* Loads the first payload section as sectionE and, when flag is set, passes the second section to func_00210480. */
s32 func_001C0A48(SectionOwner *self, s32 blockValue, s32 flag) {
    PayloadBlock *sectionTable;
    void *handle;

    self->sectionE = 0;
    if (blockValue == 0) {
        return 1;
    }
    handle = func_001B6918(0, 1, blockValue, -1);
    if (handle == 0) {
        handle = func_001B6918(0, 0, blockValue, -1);
        if (handle == 0) {
            return 0;
        }
    }
    if (poolBlockTagWordGet(handle, 0) == 0) {
        return 0;
    }
    if (xcLocatePayload(&sectionTable, handle) == 0) {
        return 0;
    }
    if (sectionTable->section[0].present != 0) {
        self->sectionE = (u8 *) sectionTable + sectionTable->section[0].sectionOffset;
    }
    if (flag == 0) {
        return 1;
    }
    if (sectionTable->section[1].present != 0) {
        func_00210480((u8 *) sectionTable + sectionTable->section[1].sectionOffset, 0);
    }
    return 1;
}
