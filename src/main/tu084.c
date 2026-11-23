#include "common.h"

#include "quadword.h"
#include "vu0_macro.h"
typedef unsigned char u8;

typedef signed char s8;

typedef short s16;

typedef unsigned short u16;

typedef int s32;

typedef float f32;

/* This TU does not include shared.h: local copy of its Vector4. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vector4;

/* Stored kind and flag words of a slot object; only the declared width matters. */
enum SlotKind {
    SLOT_KIND_NONE = 0,
    SLOT_KIND_WIDTH = 0x7fffffff
};

enum SlotFlags {
    SLOT_FLAGS_NONE = 0,
    SLOT_FLAGS_WIDTH = 0x7fffffff
};

/* The model (actor) a spawned slot object draws, allocated by func_0028D670. */
typedef struct SpawnedModel {
    s32 unmodeled_00;
    s32 active; /* +0x04 */
    u8 unmodeled_08[8];
    union {
        Vector4 vector;
        s32 words[4];
    } extents; /* +0x10: (half scale, 0.15, 0, half scale) */
    Vector4 position; /* +0x20 */
    Vector4 unmodeled_30;
    Vector4 unmodeled_40;
    Vector4 rotation; /* +0x50 */
    Vector4 unmodeled_60;
    Vector4 unmodeled_70;
    Vector4 scale; /* +0x80 */
    s32 drawFlags; /* +0x90: bits 0xC set on creation */
    u8 unmodeled_94[0x6a0 - 0x94];
    u8 collisionObjectHead[0x6aa - 0x6a0]; /* +0x6A0, passed to func_002A70E8 */
    s16 collisionBits;                      /* +0x6AA */
    u8 unmodeled_6AC[0x6b3 - 0x6ac];
    s8 collisionProbeMode; /* +0x6B3 */
    u8 unmodeled_6B4[0x6bc - 0x6b4];
    s32 collisionBlock; /* +0x6BC */
    u8 unmodeled_6C0[0x6cc - 0x6c0];
    f32 copiedScale; /* +0x6CC: copy of the value at +0x10 */
} SpawnedModel;

typedef struct SpawnSlotState {
    s32 spawnParam; /* +0x0: the request word at +0x10 */
    s32 unmodeled_04;
    s32 unmodeled_08;
    s32 clearedWord; /* +0xC: cleared on creation */
} SpawnSlotState;

/* Slot object allocated by RgDispModelAlloc (stride 0x230, see rg_dispmodel.c). */
typedef struct SpawnSlot {
    enum SlotFlags flags;
    u8 unmodeled_04[0xC];
    enum SlotKind kind; /* +0x10 */
    u8 unmodeled_14[8];
    void (*update)();  /* +0x1C */
    void (*release)(); /* +0x20 */
    SpawnedModel *model; /* +0x24 */
    s32 slotId;          /* +0x28: the id from RgDispModelNextId */
    s32 objectId;        /* +0x2C */
    u8 unmodeled_30[0x134 - 0x30];
    f32 halfScale; /* +0x134 */
    u8 unmodeled_138[0x150 - 0x138];
    SpawnSlotState state; /* +0x150 */
} SpawnSlot;

typedef struct SpawnRequest {
    Vector4 position;
    s32 spawnParam; /* +0x10 */
    s32 layerId;      /* +0x14 */
} SpawnRequest;

/* The part of the game loop runtime this TU reads. */
typedef struct GameLoopRuntimeView {
    s32 flags;
    u8 unmodeled_04[0x168];
    f32 spawnUnitScale; /* +0x16C */
    u16 spawnedCount;   /* +0x170: incremented for every slot spawned here */
    u8 unmodeled_172[0x100];
} GameLoopRuntimeView;

extern GameLoopRuntimeView gGameLoopRuntimeStorage;
s32 RgDispModelNextId(void);
SpawnSlot *RgDispModelAlloc(s32 id);
SpawnedModel *func_0028D670(void);
void RgDispModelSetLocal(SpawnSlot *slot);
s32 func_001B93C0(s32 index);
void payloadSectionsLocate(SpawnedModel *model, s32 value);
void func_001C7C48();

void actorChildBindLayer(SpawnSlot *slot, s32 layerId);

static inline void ZeroVectorMemory(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(destination) : "memory");
}

static inline void ZeroVector(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void CopyVectorT0(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%1)\n sq $8,0(%0)" : : "r"(destination), "r"(source) : "$8", "memory");
}

static inline void BroadcastScalarXYZ(Vector4 *destination, f32 value) {
    __asm__ __volatile__(
        "mfc1 $8, %5\n qmtc2 $8, $vf1\n vaddx.xyz $vf2, $vf0, $vf1x\n vmove.w $vf2, $vf0\n sqc2 $vf2, 0(%4)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
        : "r"(destination), "f"(value)
        : "$8");
}

/* Spawns a slot of kind 0x10000 with a small flat model at the request position, scaled by the runtime's unit scale. */
s32 flatSlotSpawn(SpawnRequest *request) {
    SpawnSlot *slot;
    SpawnedModel *model;
    s32 slotId;

    slotId = RgDispModelNextId();
    slot = RgDispModelAlloc(slotId);
    if (slot == 0) {
        return 0;
    }
    {
        SpawnSlotState *state = &slot->state;
        slot->kind = 0x10000;
        slot->slotId = slotId;
        slot->halfScale = gGameLoopRuntimeStorage.spawnUnitScale * 3.2f;
        slot->update = func_001C7C48;
        state->spawnParam = request->spawnParam;
        state->clearedWord = 0;
        model = func_0028D670();
        slot->model = model;
        if (model == 0) {
            RgDispModelSetLocal(slot);
            return 0;
        }
        model->drawFlags |= 0xc;
        ZeroVectorMemory(&model->extents.vector);
        model->active = 0;
        model->extents.vector.x = model->extents.vector.w = gGameLoopRuntimeStorage.spawnUnitScale * 0.5f;
        model->extents.vector.y = 0.15f;
        CopyVectorT0(&model->position, &request->position);
        ZeroVector(&model->rotation);
        BroadcastScalarXYZ(&model->scale, gGameLoopRuntimeStorage.spawnUnitScale);
        payloadSectionsLocate(model, func_001B93C0(0x16));
        actorChildBindLayer(slot, request->layerId);
        gGameLoopRuntimeStorage.spawnedCount++;
        return 1;
    }
}

s32 func_002A6FA8();
void func_002A70E8();

/* Stores the layer id in the slot and restarts the model's collision block with it. */
void actorChildBindLayer(SpawnSlot *slot, s32 layerId) {
    f32 baseScale;
    s32 collisionBlock;
    s32 storedLayerId;
    SpawnedModel *model;

    storedLayerId = layerId;
    slot->objectId = storedLayerId;
    model = slot->model;
    if (model != 0) {
        func_002A70E8(model->collisionObjectHead);
        model->collisionBits = 0x102;
        model->collisionProbeMode = 3;
        collisionBlock = func_002A6FA8(0x100, slot->objectId);
        baseScale = model->extents.vector.x;
        model->copiedScale = baseScale;
        model->collisionBlock = collisionBlock;
    }
}

typedef struct RgDispModel {
    s32 modelFlags; /* +0x00, bit 0x10000 is set here */
    unsigned char unmodeled_04[0x10 - 0x04];
    s32 kindBits; /* +0x10, ANDed with the iterator kind */
    unsigned char unmodeled_14[0x1C - 0x14];
    void (*handlerFn)(void); /* +0x1C */
    unsigned char unmodeled_20[0x230 - 0x20];
} RgDispModel;

RgDispModel *RgDispModelNextByKind(s32 kind, RgDispModel *previous);
void func_001C7E08(void);

/* Counts the slots of kind 0x10000 whose update is func_001C7E08, flagging each with 0x100000. */
s32 flatSlotMarkAll(void) {
    RgDispModel *model;
    s32 count;

    count = 0;
    model = 0;
    while ((model = RgDispModelNextByKind(0x10000, model)) != 0) {
        if (model->handlerFn == func_001C7E08) {
            model->modelFlags |= 0x100000;
            count++;
        }
    }
    return count;
}
