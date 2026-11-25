#include "common.h"

#include "shared.h"
#include "quadword.h"
#include "main/tu118.h"
#include "main/xenoActor.h"
typedef int s32;

typedef struct KeyedWordEntry {
    s32 key;
    s32 value;
} KeyedWordEntry;

extern KeyedWordEntry D_003E6280[];
extern KeyedWordEntry D_003E62E0[];

/* Looks searchKey up in one of the two key/value tables (tableKind 3 picks the first); returns its value or 0. */
s32 keyedWordTableFind(s32 searchKey, s32 tableKind) {
    KeyedWordEntry *keyedEntry;

    if (tableKind == 3) {
        keyedEntry = D_003E6280;
    } else {
        keyedEntry = D_003E62E0;
    }
    while (keyedEntry->key >= 0) {
        if (keyedEntry->key == searchKey) {
            return keyedEntry->value;
        }
        keyedEntry++;
    }
    return 0;
}

extern s32 gShapeAngleSecondTable[];

s32 wordTableGetEntry(s32 entryNumber) {
    return gShapeAngleSecondTable[entryNumber];
}

/* The block at +0x150 of a display-model record; storedValue is its slot +0x210 (+0xC0 here). */
typedef struct RgDispModelTail {
    unsigned char unmodeled_00[0xC0];
    s32 storedValue;                        /* +0xC0 (slot +0x210) */
    s32 countdownFrames;                    /* +0xC4 (slot +0x214): set to 300 by rgDispModelSetTurnToward */
} RgDispModelTail;

/* One display-model record as this TU reads it (the fields of its three views merged; include/main/tu075.h
   publishes another view of the same slot, kept apart: its field types differ). */
typedef struct RgDispModelRecord {
    s32 modelFlags;                         /* +0x00 */
    unsigned char unmodeled_04[0x24 - 0x04];
    s32 actorId;                            /* +0x24 */
    unsigned char unmodeled_28[0x40 - 0x28];
    AlignedVector4 position;                /* +0x40 */
    unsigned char unmodeled_50[0xB0 - 0x50];
    AlignedVector4 vectorB0;                /* +0xB0 */
    unsigned char unmodeled_C0[0x150 - 0xC0];
    RgDispModelTail tailBlock;              /* +0x150 */
} RgDispModelRecord;

RgDispModelRecord *RgDispModelFindByKey();

s32 rgDispModelGetActorId(void) {
    RgDispModelRecord *dispModel;
    s32 actorId;

    dispModel = RgDispModelFindByKey();
    actorId = 0;
    if (dispModel != 0) {
        actorId = dispModel->actorId;
    }
    return actorId;
}

static inline void CopyVectorMem(AlignedVector4 *dest, const AlignedVector4 *src) {
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(src), "m"(src->x), "m"(src->y), "m"(src->z), "m"(src->w)
                         : "$8");
}

/* Sets or clears the 0x04000000 flag of the record for key; a non-null src also copies its vector to vectorB0. */
s32 rgDispModelSetAuxVector(s32 key, AlignedVector4 *src) {
    RgDispModelRecord *model;

    model = RgDispModelFindByKey(key);
    if (model == 0) {
        return 0;
    }
    if (src == 0) {
        model->modelFlags &= ~0x04000000;
    } else {
        model->modelFlags |= 0x04000000;
        CopyVectorMem(&model->vectorB0, src);
    }
    return 1;
}

extern u16 gMovieAutoSelectFlags[];
s32 *eventSlotFind(s32 key, s32 mode);

/* Returns the low 24 bits of the entry named by the record's storedValue, or -1 when a gate blocks it. */
s32 rgDispModelGetMatchedEntryFlags(void) {
    u16 autoSelectFlags;
    RgDispModelRecord *model;
    s32 *matchedEntry;

    autoSelectFlags = gMovieAutoSelectFlags[0];
    model = RgDispModelFindByKey(0);
    if (model == 0) {
        return -1;
    }
    if ((gGameLoopRuntimeStorage.flags & 0x20) != 0 && (autoSelectFlags & 0x2) != 0) {
        return -1;
    }
    if ((model->tailBlock.storedValue & 0x20000000) == 0) {
        return -1;
    }
    matchedEntry = eventSlotFind(model->tailBlock.storedValue, 0);
    if (matchedEntry == 0) {
        return -1;
    }
    return *matchedEntry & 0x00FFFFFF;
}

extern f32 MMathCalcDir(const Vector4 *fromPoint, const Vector4 *toPoint);

/* Enables (enable != 0) or disables the 0x800 runtime flag; when enabling, turns the actor of the record for key's
   source toward the target record and starts its 300-frame countdown. */
void rgDispModelSetTurnToward(s32 enable, s32 targetKey) {
    RgDispModelRecord *sourceModel;
    RgDispModelRecord *targetModel;
    RgDispModelTail *tail;

    sourceModel = RgDispModelFindByKey(0);
    if (enable != 0) {
        gGameLoopRuntimeStorage.flags |= 0x800;
        if (sourceModel != 0) {
            tail = &sourceModel->tailBlock;
            targetModel = RgDispModelFindByKey(targetKey);
            if (targetModel != 0) {
                ((XenoActor *) sourceModel->actorId)->rotation.y =
                    MMathCalcDir(&sourceModel->position, &targetModel->position);
            }
            tail->countdownFrames = 300;
        }
    } else {
        gGameLoopRuntimeStorage.flags &= ~0x800;
    }
}

KeyedWordEntry D_003E6280[12] = {
    { 1, 0x10140001 },
    { 2, 0x10145001 },
    { 3, 0x10144001 },
    { 4, 0x10141001 },
    { 5, 0x10142001 },
    { 6, 0x10146001 },
    { 7, 0x10143001 },
    { 13, 0x1014D001 },
    { 14, 0x1014B001 },
    { 15, 0x1014A001 },
    { 16, 0x1014C001 },
    { -1, 0x00000000 },
};

KeyedWordEntry D_003E62E0[12] = {
    { 1, 0x10140002 },
    { 2, 0x10145002 },
    { 3, 0x10144002 },
    { 4, 0x10141002 },
    { 5, 0x10142002 },
    { 6, 0x10146002 },
    { 7, 0x10143002 },
    { 13, 0x1014D002 },
    { 14, 0x1014B002 },
    { 15, 0x1014A002 },
    { 16, 0x1014C002 },
    { -1, 0x00000000 },
};
