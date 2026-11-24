#include "common.h"

#include "shared.h"
#include "quadword.h"
typedef int s32;

extern s32 gDispModelEventCount;

typedef struct DispModelBlock {
    u8 unmodeled_00[0xC0];
    s32 lookupKey;
} DispModelBlock;

typedef struct DistancePose {
    u8 unmodeled_00[0x14];
    f32 scaleAt14;
    u8 unmodeled_18[0x1C - 0x18];
    f32 scaleAt1C;
    u8 unmodeled_20[0x6A0 - 0x20];
} DistancePose;

typedef struct DistanceModel {
    u32 modelFlags;
    u8 unmodeled_04[0x10 - 0x04];
    s32 displayKind;
    u8 unmodeled_14[0x24 - 0x14];
    DistancePose *pose;
    s32 eventKind;
    u8 unmodeled_2C[0x40 - 0x2C];
    Vector4 position;
    u8 unmodeled_50[0xB0 - 0x50];
    Vector4 distanceTarget;
    u8 unmodeled_C0[0x134 - 0xC0];
    f32 eventRange;
    f32 heightBand;
    u32 sectorMaskBits;
    u8 unmodeled_140[0x150 - 0x140];
    DispModelBlock eventKeyBlock;
} DistanceModel;

extern s32 gDispModelPreviousEventCount;
extern DistanceModel *gDispModelEventSorted[];
DistanceModel *RgDispModelFindByKey(s32 key);
DistanceModel *RgDispModelNextByKind(s32 displayKind, DistanceModel *prev);
s32 distanceModelTestEvent(DistanceModel *owner, DistanceModel *model);

s32 distanceModelPushEvent(DistanceModel *owner, DistanceModel *model);
void eventSlotSortPending(void);

/* The event array: each slot holds slotEventKind (flag bits 0x40000000 and 0x20000000 mark state) and distanceMilli. */
DistanceModel *eventSlotFind(s32 key, u32 selectionMode);
void seVoiceStartDefault(unsigned int handle);

/* Per-frame pass over the display models: resets the pending event slots, collects the candidates (kind 0x10052 and 2),
   sorts them with eventSlotSortPending and starts the default voice (13) when the first event lacks flag 0x20000000. */
void rgDispModelEventFrame(void) {
    DistanceModel *owner;
    DistanceModel *candidate;

    owner = RgDispModelFindByKey(0);
    gDispModelPreviousEventCount = gDispModelEventCount;
    gDispModelEventCount = 0;
    memset(gDispModelEventSorted, 0, 0x50);
    if (!(owner->modelFlags & 0x400000)) {
        return;
    }
    candidate = 0;
    while ((candidate = RgDispModelNextByKind(0x10052, candidate)) != 0) {
        if (distanceModelTestEvent(owner, candidate)) {
            candidate->modelFlags |= 0x01000000;
        }
    }
    candidate = 0;
    while ((candidate = RgDispModelNextByKind(2, candidate)) != 0) {
        distanceModelPushEvent(owner, candidate);
    }
    eventSlotSortPending();
    if (gDispModelPreviousEventCount != 0) {
        return;
    }
    if (gDispModelEventCount == 0) {
        return;
    }
    if (!(eventSlotFind(0, 3)->modelFlags & 0x20000000)) {
        seVoiceStartDefault(13);
    }
}

f32 MMathCalcDist(const Vector4 *fromPoint, const Vector4 *toPoint);
s32 eventSlotPush(s32 eventKind, f32 scale);

typedef struct DistanceCamera {
    u8 unmodeled_00[0x94];
    f32 fov;
    u8 unmodeled_98[0xD0 - 0x98];
    u8 eye[0x10];
} DistanceCamera;

typedef struct DistanceLoopRuntime {
    u8 unmodeled_00[0x28];
    DistanceCamera *camera;
    u8 unmodeled_2C[0x30 - 0x2C];
    Vector4 direction;
} DistanceLoopRuntime;

extern DistanceLoopRuntime gGameLoopRuntimeStorage;
float MMathVectorDotProduct(const Vector4 *a, const Vector4 *b);
void MMathCalcDirVector(Vector4 *dst, void *eye, Vector4 *pos);
s32 boxPointSectorClassify(DistanceModel *box, DistanceModel *point);

s32 distanceModelOcclusionClear(DistanceModel *owner, DistanceModel *model);

/* Tests one model against owner: range, height band, sector mask, view direction and occlusion; a hit pushes it
   as an event (eventSlotPush). The 0x400 flag skips the occlusion test. */
s32 distanceModelTestEvent(DistanceModel *owner, DistanceModel *model) {
    AlignedVector4 targetPos;
    AlignedVector4 viewDir;
    f32 distance;
    f32 ownerHeight;
    f32 heightDelta;
    s32 sectorIndex;
    s32 sectorMask;
    f32 viewDot;

    if (!(model->modelFlags & 0x200000)) {
        return 0;
    }
    QuadwordCopyT0NoClobber(&targetPos, &model->position);
    if (model->pose != 0) {
        targetPos.y += model->pose->scaleAt14 * 0.5f;
    }
    distance = MMathCalcDist(&owner->position, &targetPos);
    if (distance > model->eventRange) {
        return 0;
    }
    ownerHeight = owner->position.y;
    heightDelta = ownerHeight - model->position.y;
    if (heightDelta < -model->heightBand || heightDelta > model->heightBand) {
        return 0;
    }
    sectorIndex = boxPointSectorClassify(model, owner);
    sectorMask = 1 << (sectorIndex * 4);
    if (!(model->sectorMaskBits & sectorMask)) {
        return 0;
    }
    MMathCalcDirVector(&viewDir, gGameLoopRuntimeStorage.camera->eye, &targetPos);
    viewDot = MMathVectorDotProduct(&gGameLoopRuntimeStorage.direction, &viewDir);
    heightDelta = viewDot + gGameLoopRuntimeStorage.camera->fov * 0.15915495f;
    if (heightDelta < 1.0f) {
        return 0;
    }
    if (!(model->modelFlags & 0x400)) {
        if (!distanceModelOcclusionClear(owner, model)) {
            return 0;
        }
    }
    return eventSlotPush(model->eventKind, distance);
}

s32 func_002861C8(Vector4 *a, Vector4 *b, void *c, void *d);
s32 func_00285720(Vector4 *a, Vector4 *b, Vector4 *c, f32 scale);

static inline f32 distanceTop(DistanceModel *distanceModel) {
    return distanceModel->position.y + distanceModel->pose->scaleAt14 * 0.95f;
}

/* Occlusion test: func_002861C8 must pass for the pair, then each other kind-0xD0 model must clear it in height
   or pass func_00285720. Returns 1 when clear. */
s32 distanceModelOcclusionClear(DistanceModel *owner, DistanceModel *model) {
    s32 ok;
    DistanceModel *otherModel;
    f32 bottom;
    f32 top;
    Vector4 unused; /* kept: its stack slot keeps the frame layout that matches */

    ok = func_002861C8(&model->position, &owner->position, model->pose + 1,
                       owner->pose + 1);
    if (ok) {
        bottom = model->position.y;
        top = distanceTop(model);
        otherModel = 0;
        ok = 1;
        while ((otherModel = RgDispModelNextByKind(0xD0, otherModel)) != 0) {
            if (otherModel != model && !(otherModel->modelFlags & 0x0C) && otherModel->pose != 0 &&
                (otherModel->displayKind != 16 || (otherModel->modelFlags & 0x200000))) {
                ok = func_00285720(&owner->position, &model->position, &otherModel->position,
                                   otherModel->pose->scaleAt1C) == 0;
                if (!ok) {
                    if (distanceTop(otherModel) <= bottom) {
                        ok = 1;
                    }
                    if (top <= otherModel->position.y) {
                        ok = 1;
                    }
                    if (!ok) {
                        break;
                    }
                }
            }
        }
    }
    return ok;
}

/* Pushes model as an event when its 0x04000000 flag is set, at the owner-to-target distance. */
s32 distanceModelPushEvent(DistanceModel *owner, DistanceModel *model) {
    f32 distance;

    if (!(model->modelFlags & 0x04000000)) {
        return 0;
    }
    distance = MMathCalcDist(&owner->position, &model->distanceTarget);
    return eventSlotPush(model->eventKind | 0x20000000, distance);
}

typedef struct EventSlot {
    s32 slotEventKind;
    s32 distanceMilli;
} EventSlot;

extern EventSlot gDispModelEventSlots[];

/* Appends one event (kind, scale * 1000) to the 20-slot array; returns 0 when it is full. */
s32 eventSlotPush(s32 eventKind, f32 scale) {
    s32 eventIndex;
    s32 *pendingCount;
    EventSlot *eventSlot;

    if (gDispModelEventCount >= 20) {
        return 0;
    }
    pendingCount = &gDispModelEventCount;
    eventIndex = *pendingCount;
    *pendingCount = eventIndex + 1;
    eventSlot = &gDispModelEventSlots[eventIndex];
    eventSlot->slotEventKind = eventKind;
    eventSlot->distanceMilli = (s32) (scale * 1000.0f);
    return 1;
}

static inline void eventSlotMark(s32 *slotEventKind) {
    *slotEventKind |= 0x40000000;
}

/* Orders the pending events by value: marks each one with 0x40000000, then fills gDispModelEventSorted from the smallest
   marked value upward, clearing each mark as it is taken. */
void eventSlotSortPending(void) {
    s32 i;
    s32 j;
    s32 best;
    s32 pendingCount;
    EventSlot *eventSlot;

    pendingCount = gDispModelEventCount;
    eventSlot = gDispModelEventSlots;
    for (j = 0; j < pendingCount; j++, eventSlot++) {
        eventSlotMark(&eventSlot->slotEventKind);
    }
    for (i = 0; i < gDispModelEventCount; i++) {
        best = 0x70000000;
        eventSlot = gDispModelEventSlots;
        for (j = 0; j < gDispModelEventCount; j++, eventSlot++) {
            if ((eventSlot->slotEventKind & 0x40000000) && eventSlot->distanceMilli < best) {
                gDispModelEventSorted[i] = (DistanceModel *) eventSlot;
                best = eventSlot->distanceMilli;
            }
        }
        eventSlot = (EventSlot *) gDispModelEventSorted[i];
        eventSlot->slotEventKind &= ~0x40000000;
    }
}

/* Returns the number of pending events. */
s32 eventCountGet(void) {
    return gDispModelEventCount;
}

/* Finds an entry of the event array by key (its flags): mode 0 exact, 1 previous, 2 next, 3 the first entry. */
DistanceModel *eventSlotFind(s32 key, u32 selectionMode) {
    DistanceModel *candidate;
    s32 pendingCount;
    s32 i;

    pendingCount = gDispModelEventCount;
    if (pendingCount == 0) {
        return 0;
    }
    if (key == 0) {
        return gDispModelEventSorted[0];
    }
    switch (selectionMode) {
    case 0:
        for (i = 0; i < gDispModelEventCount; i++) {
            candidate = gDispModelEventSorted[i];
            if (candidate->modelFlags == key) {
                return candidate;
            }
        }
        break;
    case 1:
        for (i = 0; i < gDispModelEventCount; i++) {
            candidate = gDispModelEventSorted[i];
            if (candidate->modelFlags == key) {
                break;
            }
        }
        if (i == pendingCount) {
            return gDispModelEventSorted[0];
        }
        i = i - 1;
        if (i < 0) {
            i = pendingCount - 1;
        }
        return gDispModelEventSorted[i];
    case 2:
        for (i = 0; i < gDispModelEventCount; i++) {
            candidate = gDispModelEventSorted[i];
            if (candidate->modelFlags == key) {
                break;
            }
        }
        if (i == pendingCount) {
            return gDispModelEventSorted[0];
        }
        i = i + 1;
        if (i >= pendingCount) {
            i = 0;
        }
        return gDispModelEventSorted[i];
    case 3:
        return gDispModelEventSorted[0];
    default:
        break;
    }
    return 0;
}

/* Sets the 0x08000000 or 0x02000000 flag of the display record whose key is key & 0xFFFFFF, by the flags of the event entry. */
s32 eventKeyApplyToRecord(s32 key) {
    DistanceModel *eventEntry;
    DistanceModel *displayRecord;
    s32 recordKey;

    eventEntry = eventSlotFind(key, 0);
    recordKey = key & 0xFFFFFF;
    if (eventEntry == 0) {
        return 0;
    }
    displayRecord = RgDispModelFindByKey(recordKey);
    if (displayRecord == 0) {
        return 0;
    }
    if (eventEntry->modelFlags & 0x20000000) {
        displayRecord->modelFlags |= 0x08000000;
    } else {
        displayRecord->modelFlags |= 0x02000000;
    }
    return 1;
}

/* Frame driver: keeps block->lookupKey pointing at the first event while the event entry still exists. */
void distanceModelFrameDriver(void) {
    DistanceModel *owner;
    DispModelBlock *modelBlock;
    DistanceModel *firstEvent;

    owner = RgDispModelFindByKey(0);
    if (owner == 0) {
        return;
    }
    modelBlock = &owner->eventKeyBlock;
    rgDispModelEventFrame();
    if (modelBlock->lookupKey == 0) {
        if (eventCountGet() == 0) {
            return;
        }
        firstEvent = eventSlotFind(0, 3);
        modelBlock->lookupKey = firstEvent->modelFlags;
        eventKeyApplyToRecord(modelBlock->lookupKey);
        return;
    }
    if (eventSlotFind(modelBlock->lookupKey, 0) == 0) {
        modelBlock->lookupKey = 0;
        return;
    }
    eventKeyApplyToRecord(modelBlock->lookupKey);
}

s32 func_001BAFB8();

/* Forwards to func_001BAFB8 (the caller of this refresh is not yet identified). */
void globalCounterRefresh(void) {
    func_001BAFB8();
}
