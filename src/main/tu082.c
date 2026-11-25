#include "common.h"

#include "quadword.h"
#include "shared.h"
typedef signed char s8;

typedef short s16;

typedef int s32;

typedef float f32;

typedef unsigned char u8;

typedef unsigned short u16;

typedef unsigned int u32;

/* Stored kind and flag words of a spawnSlot object; only the declared width matters. */
enum SlotKind {
    SLOT_KIND_NONE = 0,
    SLOT_KIND_WIDTH = 0x7fffffff
};

enum SlotFlags {
    SLOT_FLAGS_NONE = 0,
    SLOT_FLAGS_WIDTH = 0x7fffffff
};

/* The model (actor) a spawned spawnSlot object draws, allocated by func_0028D670. */
typedef struct SpawnedModel {
    s32 unmodeled_00;
    s32 active; /* +0x04: set to 1 on creation */
    u8 unmodeled_08[8];
    union {
        Vector4 vector;
        s32 words[4];
    } extents; /* +0x10: (size, size, size, half length in XZ) */
    Vector4 position; /* +0x20 */
    Vector4 unmodeled_30;
    Vector4 unmodeled_40;
    Vector4 rotation; /* +0x50 */
    Vector4 unmodeled_60;
    Vector4 unmodeled_70;
    Vector4 scale; /* +0x80 */
    s32 drawFlags; /* +0x90: bit 0x8 set on creation */
    u8 unmodeled_94[0x4ec - 0x94];
    s32 stateFlags; /* +0x4EC: low bits 3 set once loaded */
    u8 unmodeled_4F0[0x510 - 0x4f0];
    void *loadedResource; /* +0x510 */
    u8 unmodeled_514[0x6A0 - 0x514];
    u8 collisionObjectHead[0x0A]; /* +0x6A0 */
    s16 collisionBits;
    u8 unmodeled_6AC[0x6B3 - 0x6AC];
    s8 collisionProbeMode;
    u8 unmodeled_6B4[0x6BC - 0x6B4];
    s32 collisionBlock; /* +0x6BC */
    u8 unmodeled_6C0[0x6CC - 0x6C0];
    f32 savedScale; /* +0x6CC: copy of the scale at +0x1C */
} SpawnedModel;

/* State block of a spawnSlot of kind 0x20000: one row of the game's child table and its matrix. */
typedef struct ChildTableState {
    s32 rowIndex;
    u8 *tableRow; /* +0x4: row of the child table (112 bytes each) */
    Matrix4 *sourceMatrix;
    s32 unmodeled_0C;
    Matrix4 matrix;
} ChildTableState;

/* State block of a spawnSlot of kind 0x200. */
typedef struct DescriptorResourceState {
    u16 descriptorFlags; /* +0x0: copy of the descriptor flags, bit 2 selects the second resource loader */
    s32 callbackIndex;   /* +0x4: row of the callback table */
    s32 descriptorKey;   /* +0x8 */
    struct SpawnDescriptor *descriptor;
    s32 resourceKeyA; /* +0x10 */
    s32 resourceKeyB; /* +0x14 */
    f32 scale;        /* +0x18 */
} DescriptorResourceState;

typedef struct SlotAttachHandles {
    s32 unmodeled_00;
    s32 attachHandle; /* +0x4: handle used by slotAttachFind and cleared by objectSetDefaults */
} SlotAttachHandles;

/* Slot object allocated by RgDispModelAlloc (stride 0x230, see rg_dispmodel.c). */
typedef struct SpawnSlot {
    enum SlotFlags flags;
    u8 unmodeled_04[0xC];
    enum SlotKind kind; /* +0x10 */
    u8 unmodeled_14[8];
    void (*update)(); /* +0x1C */
    void (*release)(); /* +0x20 */
    SpawnedModel *model; /* +0x24 */
    s32 slotId;          /* +0x28 */
    s32 objectId;        /* +0x2C */
    s32 childHandle;     /* +0x30 */
    u8 unmodeled_34[0xc];
    Vector4 position; /* +0x40 */
    u8 unmodeled_50[0x10];
    Vector4 rotation; /* +0x60 */
    u8 unmodeled_70[0x134 - 0x70];
    f32 halfScale; /* +0x134 */
    u8 unmodeled_138[0x150 - 0x138];
    union {
        ChildTableState childTable;
        DescriptorResourceState descriptor;
        SlotAttachHandles attachHandles;
    } state; /* +0x150 */
} SpawnSlot;

typedef struct SpawnRequest {
    s32 slotId;
    s32 rowIndex;
    s32 key; /* used by descriptorSlotSpawn only */
} SpawnRequest;

typedef struct SpawnDescriptor {
    u8 unmodeled_00[2];
    s8 flags; /* bit 0 = large (scale 5.0), bit 1 = second loader */
    u8 unmodeled_03;
} SpawnDescriptor;

typedef struct SpawnCallbackTable {
    void (*update[3])();
} SpawnCallbackTable;

struct GameMatrixTable {
    u8 unmodeled_00[0x68];
    s32 firstMatrixOffset;
};

struct ActorVisibility {
    u8 unmodeled_00[0x310];
    unsigned int bits[1];
};

struct GameWorldTables {
    u8 unmodeled_000[0x830];
    struct ActorVisibility *visibility; /* +0x830 */
    u8 unmodeled_834[0xC94 - 0x834];
    s32 childCount;
    u8 *childTable;
    struct GameMatrixTable *matrixTable;
};

struct GameWorldTables *stateBlockGet(void);
SpawnSlot *RgDispModelAlloc(s32 flags);
SpawnedModel *func_0028D670(void);
void RgDispModelSetLocal(SpawnSlot *spawnSlot);
float MMathCalcLengthXZ(const Vector4 *vector);
Vector4 *MMathCalcAngleMatrixZ(Vector4 *outAngles, Matrix4 *matrix);
Matrix4 *MMathRotateMatrixX(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixY(Matrix4 *outMatrix, Matrix4 *matrix, float angle);

s32 slotAttachFind(SpawnSlot *spawnSlot);
void childTableSlotRelease(SpawnSlot *spawnSlot);
void sceneEmptyHook(void);

static inline void SetVectorOne(Vector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void CopyVector(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8, 0(%5)\n sq $8, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source), "m"(source->x), "m"(source->y), "m"(source->z), "m"(source->w)
                         : "$8");
}

static inline void CopyMatrixTranslation(Vector4 *destination, Matrix4 *source) {
    __asm__ __volatile__("lqc2 $vf1, 0x30(%5)\n vmove.w $vf1, $vf0\n sqc2 $vf1, 0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source));
}

/* Spawns a spawnSlot of kind 0x20000 for one row of the game's child table: its model sits at the row's matrix translation. */
s32 childTableSlotSpawn(SpawnRequest *spawnRequest) {
    struct GameWorldTables *worldTables;
    SpawnSlot *spawnSlot;
    ChildTableState *childState;
    SpawnedModel *model;

    worldTables = stateBlockGet();
    if (spawnRequest->rowIndex < 0 || spawnRequest->rowIndex >= worldTables->childCount) {
        return 0;
    }
    spawnSlot = RgDispModelAlloc(spawnRequest->slotId);
    if (spawnSlot == 0) {
        return 0;
    }
    childState = &spawnSlot->state.childTable;
    spawnSlot->slotId = spawnRequest->slotId;
    spawnSlot->kind = 0x20000;
    spawnSlot->update = sceneEmptyHook;
    spawnSlot->release = childTableSlotRelease;
    childState->rowIndex = spawnRequest->rowIndex;
    childState->tableRow = worldTables->childTable + childState->rowIndex * 112;
    childState->sourceMatrix = (Matrix4 *) ((u8 *) worldTables->matrixTable + worldTables->matrixTable->firstMatrixOffset) + spawnRequest->rowIndex;
    QuadwordCopyMatrixT0(childState->matrix, childState->sourceMatrix);
    model = func_0028D670();
    spawnSlot->model = model;
    if (model == 0) {
        RgDispModelSetLocal(spawnSlot);
        return 0;
    }
    model->active = 1;
    model->drawFlags |= 8;
    SetVectorOne(&model->extents.vector);
    model->extents.vector.w = MMathCalcLengthXZ(&model->extents.vector) * 0.5f;
    CopyMatrixTranslation(&model->position, childState->sourceMatrix);
    MMathCalcAngleMatrixZ(&model->rotation, childState->sourceMatrix);
    SetVectorOne(&model->scale);
    CopyVector(&spawnSlot->position, &model->position);
    CopyVector(&spawnSlot->rotation, &model->rotation);
    MMathRotateMatrixY(&childState->matrix, &childState->matrix, -model->rotation.y);
    MMathRotateMatrixX(&childState->matrix, &childState->matrix, -model->rotation.x);
    if (slotAttachFind(spawnSlot) == 0) {
        RgDispModelSetLocal(spawnSlot);
        return 0;
    }
    return 1;
}

/* Release callback of a child-table slot: marks its child index in the actor visibility bitmap. */
void childTableSlotRelease(SpawnSlot *spawnSlot) {
    struct GameWorldTables *worldTables;
    s32 *bitIndex;
    s32 childBit;

    bitIndex = &spawnSlot->state.childTable.rowIndex;
    worldTables = stateBlockGet();
    childBit = *bitIndex;
    worldTables->visibility->bits[childBit >> 5] |= 1 << (childBit & 0x1F);
}

void sceneEmptyHook(void) {

}

s32 func_0030E8E4(s32 handle);
s32 strncmp(s32 ownerHandle, s32 childHandle, s32 value);
SpawnSlot *RgDispModelNextByKind(s32 kind, SpawnSlot *from);
void func_001C06F0(SpawnSlot *owner, SpawnSlot *child);

/* Looks for another spawnSlot of kind 16 that the owner can attach to (strncmp returns 0) and links it. */
s32 slotAttachFind(SpawnSlot *owner) {
    SpawnSlot *cursor;
    s32 ownerHandle;
    s32 childHandle;

    ownerHandle = owner->state.attachHandles.attachHandle;
    cursor = 0;
    for (;;) {
        cursor = RgDispModelNextByKind(16, cursor);
        if (cursor == 0) {
            break;
        }
        if (cursor != owner) {
            childHandle = cursor->state.attachHandles.attachHandle;
            if (strncmp(ownerHandle, childHandle, func_0030E8E4(childHandle)) == 0) {
                func_001C06F0(owner, cursor);
                return 1;
            }
        }
    }
    return 0;
}

static inline void ZeroVector(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void BroadcastScalarXYZ(Vector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %5\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%4)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
        : "r"(destination), "f"(value)
        : "$8");
}

void modeSlotWaitHandler();
void modeSlotActivate();
void XenoActorSwitchHandler();
const SpawnCallbackTable gDescriptorSlotCallbacks = {
    { modeSlotWaitHandler, modeSlotActivate, XenoActorSwitchHandler },
};
extern u32 gLoadedResourceTable[];
void slotSetValueSelector();
void func_0027AB00();
s32 func_0028D760();
s32 payloadSectionsLoad();
s32 payloadSectionsLocate();
SpawnDescriptor *actorScanEntryFind();
s32 func_001B93C0();
s32 func_0027A7E8();
s32 XenoActorAdvanceLookup();
void objectSetChildId(SpawnSlot *spawnSlot, s32 newId);
void slotChildStateSelect(SpawnSlot *spawnSlot, s32 mode);

/* Spawns a spawnSlot of kind 0x200 from descriptor spawnRequest->key, with update callback spawnRequest->rowIndex of the callback table. */
int descriptorSlotSpawn(SpawnRequest *spawnRequest) {
    SpawnCallbackTable callbacks = gDescriptorSlotCallbacks;
    SpawnDescriptor *descriptor;
    SpawnSlot *spawnSlot;
    SpawnedModel *model;
    f32 scale;
    s32 resourceLoaded;
    DescriptorResourceState *descriptorState;

    if ((u32) spawnRequest->rowIndex >= 3) {
        return 0;
    }
    descriptor = actorScanEntryFind(spawnRequest->key);
    if (descriptor == 0) {
        return 0;
    }
    scale = (descriptor->flags & 1) ? 5.0f : 1.0f;
    spawnSlot = RgDispModelAlloc(spawnRequest->slotId);
    if (spawnSlot == 0) {
        return 0;
    }
    spawnSlot->update = callbacks.update[spawnRequest->rowIndex];
    spawnSlot->kind = 0x200;
    spawnSlot->slotId = spawnRequest->slotId;
    spawnSlot->halfScale = scale * 1.5f;
    if (descriptor->flags & 1) {
        spawnSlot->flags |= 0x100;
    }
    {
        descriptorState = &spawnSlot->state.descriptor;
        descriptorState->descriptorFlags = (s8) descriptor->flags;
        descriptorState->callbackIndex = spawnRequest->rowIndex;
        descriptorState->descriptorKey = spawnRequest->key;
        descriptorState->descriptor = descriptor;
        descriptorState->resourceKeyA = (descriptorState->descriptorFlags & 2) ? 0x2040188 : 0x2040187;
        descriptorState->resourceKeyB = func_001B93C0(0x17);
        descriptorState->scale = scale;
        model = func_0028D670();
        spawnSlot->model = model;
        if (model == 0) {
            RgDispModelSetLocal(spawnSlot);
            return 0;
        }
        ZeroVector(&model->extents.vector);
        model->active = 1;
        model->extents.vector.x = model->extents.vector.y = model->extents.vector.z = scale * 0.7f;
        model->extents.vector.w = MMathCalcLengthXZ(&model->extents.vector) * 0.5f;
        ZeroVector(&model->position);
        ZeroVector(&model->rotation);
        BroadcastScalarXYZ(&model->scale, scale);
        if (descriptorState->descriptorFlags & 2) {
            resourceLoaded = payloadSectionsLoad(model, descriptorState->resourceKeyA);
        } else {
            resourceLoaded = payloadSectionsLocate(model, descriptorState->resourceKeyB);
        }
        if (resourceLoaded) {
            model->loadedResource = gLoadedResourceTable;
        } else {
            model->loadedResource = 0;
        }
        func_0028D760(model);
        model->stateFlags |= 3;
        objectSetChildId(spawnSlot, -1);
        spawnSlot->childHandle = func_0027A7E8(model);
        slotSetValueSelector(spawnSlot->childHandle, 0);
        func_0027AB00(spawnSlot->childHandle);
        switch (descriptorState->callbackIndex) {
        case 0:
            break;
        case 1:
        case 2:
            spawnSlot->flags |= 8;
            slotChildStateSelect(spawnSlot, 0);
            break;
        }
        XenoActorAdvanceLookup(spawnSlot);
        return 1;
    }
}

s32 func_002A6FA8(s32 resourceKind, s32 ownerId);
void func_002A70E8(u8 *block);

void objectSetChildId(SpawnSlot *spawnSlot, s32 newId) {
    f32 scale;
    s32 collisionBlock;
    s32 storedOwnerId;
    SpawnedModel *model;

    storedOwnerId = newId;
    spawnSlot->objectId = storedOwnerId;
    model = spawnSlot->model;
    if (model != 0) {
        func_002A70E8(model->collisionObjectHead);
        model->collisionProbeMode = 3;
        model->collisionBits = 0;
        collisionBlock = func_002A6FA8(0x100, spawnSlot->objectId);
        scale = model->extents.vector.w;
        model->savedScale = scale;
        model->collisionBlock = collisionBlock;
    }
}

void modeSlotWaitHandler();

void objectSetDefaults(SpawnSlot *spawnSlot)
{
    s32 updatedFlagBits;

    updatedFlagBits = (spawnSlot->flags & (~8)) | 0x11000;
    spawnSlot->update = modeSlotWaitHandler;
    spawnSlot->state.attachHandles.attachHandle = 0;
    spawnSlot->flags = updatedFlagBits;
    slotChildStateSelect(spawnSlot, 1);
}

unsigned int gLoadedResourceTable[48] = {
    0x20434158u, 0x3FD9999Au, 0, 0x42160000u, 4, 0, 0, 0,
    0, 0, 0, 0, 80, 192, 192, 192,
    192, 192, 192, 192, 511, 16384, 0, 16384,
    0, 16384, 0, 16384, 0, 16384, 0, 16384,
    0, 16384, 0x3F800000u, 16384, 0x3F800000u, 16384, 0x3F800000u, 0x10000u,
    0x10000u, 0x10020u, 2, 0x3E999864u, 0, 0x1E4002u, 0x3EB9A50Bu, 0x3FE429F9u
};
