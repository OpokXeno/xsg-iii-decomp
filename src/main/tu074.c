#include "common.h"

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
typedef unsigned short u16;
s32 func_002A6FA8();
void func_002A70E8(u8 *collisionObject);

/* Sub-block at ActorObject.displayState (+0x24): the 0x6A0 state block, the 0x6AA flag, the 0x6B3 mode
 * byte, the stored result at 0x6BC and the scale copied from baseScale at 0x6CC. */
typedef struct DisplayModelState {
    unsigned char unmodeled_000[0x04];
    s32 stateWord04; /* 0x04 */
    unsigned char unmodeled_008[0x10 - 0x08];
    f32 halfScaleX; /* 0x10 */
    f32 halfScaleY; /* 0x14 */
    unsigned char unmodeled_018[0x1C - 0x18];
    f32 baseScale; /* 0x1C */
    unsigned char unmodeled_020[0x30];
    unsigned char unmodeled_050[0x30];
    unsigned char unmodeled_080[0x6A0 - 0x80];
    unsigned char collisionObject[0xA]; /* 0x6A0 */
    s16 stateFlag;                      /* 0x6AA */
    unsigned char unmodeled_6AC[0x6B3 - 0x6AC];
    s8 mode; /* 0x6B3 */
    unsigned char unmodeled_6B4[0x6BC - 0x6B4];
    s32 resultId; /* 0x6BC */
    unsigned char unmodeled_6C0[0x6CC - 0x6C0];
    f32 scale; /* 0x6CC */
} DisplayModelState;

/* One 8-byte row of the table actorScanEntryFind searches by its short key. */
typedef struct ActorScanEntry {
    s16 key;       /* 0x00: compared with the request's bound */
    u8 flags;      /* 0x02: bit 0 selects the 5.0 scale */
    s8 tableIndex; /* 0x03 */
    s16 step;      /* 0x04 */
    u16 word6;     /* 0x06 */
} ActorScanEntry;

/* The lookup block at ActorObject +0x150 that actorScanLookupStart fills from the entry it found. */
typedef struct ActorScanBlock {
    s16 flags; /* 0x00: the entry's flags byte */
    unsigned char unmodeled_02[0x154 - 0x152];
    s32 pending;           /* 0x04 (ActorObject +0x154) */
    s32 bound;             /* 0x08 */
    ActorScanEntry *entry; /* 0x0C */
    s32 typeIndex;         /* 0x10 */
    s32 step;              /* 0x14 */
    s32 word6;             /* 0x18 */
    u8 *table;             /* 0x1C */
    s32 count;             /* 0x20 */
    s32 value;             /* 0x24 */
    s32 flushed;           /* 0x28 */
    f32 scale;             /* 0x2C */
} ActorScanBlock;

/* The lookup request of actorScanLookupStart (tu083 fills it for XenoActorAdvanceLookup). */
typedef struct ActorScanRequest {
    s32 scanFromIndex; /* 0x00 */
    s32 scanMode;      /* 0x04 */
    s32 bound;         /* 0x08 */
    s32 found;         /* 0x0C */
} ActorScanRequest;

/* Object record: +0x00 actorFlags, +0x1C table, +0x24 sub-block pointer, +0x2C id, +0x150 scan block. */
typedef struct ActorObject {
    s32 actorFlags; /* 0x00 */
    unsigned char unmodeled_04[0x10 - 0x04];
    s32 stateWord; /* 0x10 */
    unsigned char unmodeled_14[0x1C - 0x14];
    u8 *table; /* 0x1C */
    unsigned char unmodeled_20[0x24 - 0x20];
    DisplayModelState *displayState; /* 0x24 */
    s32 lastScanIndex;               /* 0x28 */
    s32 id;                          /* 0x2C */
    unsigned char unmodeled_30[0x134 - 0x30];
    f32 halfScale; /* 0x134 */
    unsigned char unmodeled_138[0x150 - 0x138];
    ActorScanBlock scanBlock; /* 0x150 */
} ActorObject;

extern unsigned char func_001C33D8[];
extern unsigned char func_001C3490[];
extern unsigned char func_001C3598[];
extern unsigned char RgDispModelResetTable[];
ActorScanEntry *actorScanEntryFind(s32 bound);
s32 bitmapSecondTestBit(s32 bit);
ActorObject *RgDispModelAlloc(s32 index);
DisplayModelState *func_0028D670(void);
void RgDispModelSetLocal(ActorObject *actor);
void payloadSectionsLocate(DisplayModelState *state, s32 value);
u8 *indexedTableGet();
s32 indexedCountGet();
s32 indexedValueGet(s32 count);
s32 actorFlushPending(s32 count);
void actorSetId(ActorObject *actor, s32 id);

/* Looks up the scan entry for request->bound, allocates an actor slot and fills its scan block and display state;
 * returns 0 on failure, 1 on success (found = 0 when the bound is already marked in the bitmap). */
s32 actorScanLookupStart(ActorScanRequest *lookupRequest) {
    u8 *handlers[4] = {func_001C33D8, func_001C3490, func_001C3598, RgDispModelResetTable};
    ActorScanEntry *scanEntry;
    ActorObject *actor;
    ActorScanBlock *actorScanBlock;
    DisplayModelState *displayState;
    f32 *halfScaleX;
    u8 *scanTable;
    f32 scale;
    s32 entryFlags;
    s32 scanFromIndex;

    if ((unsigned int) lookupRequest->scanMode >= 4) {
        return 0;
    }
    scanEntry = actorScanEntryFind(lookupRequest->bound);
    if (scanEntry == 0) {
        return 0;
    }
    if (bitmapSecondTestBit(lookupRequest->bound) != 0) {
        lookupRequest->found = 0;
        return 1;
    }
    entryFlags = scanEntry->flags;
    scale = (entryFlags & 1) ? 5.0f : 1.0f;
    actor = RgDispModelAlloc(lookupRequest->scanFromIndex);
    if (actor == 0) {
        return 0;
    }
    scanFromIndex = lookupRequest->scanFromIndex;
    scanTable = handlers[lookupRequest->scanMode];
    actor->stateWord = 0x400;
    actor->table = scanTable;
    actor->lastScanIndex = scanFromIndex;
    actor->halfScale = 0.5f * scale;
    if ((s8) scanEntry->flags & 1) {
        actor->actorFlags |= 0x100;
    }
    actorScanBlock = &actor->scanBlock;
    actorScanBlock->flags = (s8) scanEntry->flags;
    actorScanBlock->pending = lookupRequest->scanMode;
    actorScanBlock->bound = lookupRequest->bound;
    actorScanBlock->entry = scanEntry;
    actorScanBlock->typeIndex = scanEntry->tableIndex;
    actorScanBlock->step = scanEntry->step;
    actorScanBlock->word6 = scanEntry->word6;
    actorScanBlock->table = indexedTableGet(actorScanBlock->typeIndex, actorScanBlock->step);
    actorScanBlock->count = indexedCountGet(actorScanBlock->typeIndex, actorScanBlock->step);
    actorScanBlock->value = indexedValueGet(actorScanBlock->count);
    actorScanBlock->flushed = actorFlushPending(actorScanBlock->count);
    actorScanBlock->scale = scale;
    switch (actorScanBlock->pending) {
    case 0:
        break;
    case 1:
    case 2:
    case 3:
        actor->actorFlags |= 8;
        break;
    }
    displayState = func_0028D670();
    actor->displayState = displayState;
    if (displayState == 0) {
        RgDispModelSetLocal(actor);
        return 0;
    }
    halfScaleX = &displayState->halfScaleX;
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(halfScaleX));
    displayState->stateWord04 = 0;
    *halfScaleX = displayState->halfScaleY = displayState->baseScale = 0.5f * scale;
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(displayState->unmodeled_020));
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(displayState->unmodeled_050));
    __asm__ __volatile__("mfc1 $8, %1\n"
                         "qmtc2 $8, $vf1\n"
                         "vaddx.xyz $vf2xyz, $vf0xyz, $vf1x\n"
                         "vmove.w $vf2w, $vf0w\n"
                         "sqc2 $vf2, 0(%0)\n"
                         :
                         : "r"(displayState->unmodeled_080), "f"(scale)
                         : "$8");
    payloadSectionsLocate(displayState, actorScanBlock->flushed);
    actorSetId(actor, -1);
    lookupRequest->found = 1;
    return 1;
}

void actorSetId(ActorObject *actor, s32 id) {
    f32 baseScale;
    s32 resultId;
    DisplayModelState *displayState;

    actor->id = id;
    displayState = actor->displayState;
    if (displayState != 0) {
        func_002A70E8(displayState->collisionObject);
        displayState->mode = 3;
        displayState->stateFlag = 0;
        resultId = func_002A6FA8(0x100, actor->id);
        baseScale = displayState->baseScale;
        displayState->scale = baseScale;
        displayState->resultId = resultId;
    }
}

void actorInit(ActorObject *actor) {
    s32 newActorFlags;

    newActorFlags = (actor->actorFlags & ~8) | 0x11000;
    actor->actorFlags = newActorFlags;
    actor->scanBlock.pending = 0;
    actor->table = func_001C33D8;
}

extern unsigned char gIndexedTableDefault[];
u8 *gIndexedTableFallback = (u8 *) "**unknown**";
u8 *func_00A85208(s32 index);

u8 *indexedTableGet(s32 tableIndex) {
    u8 *selectedTable;

    if (tableIndex != 0xA) {
        selectedTable = func_00A85208(tableIndex + 1);
    } else {
        selectedTable = gIndexedTableDefault;
    }
    return (selectedTable != 0) ? selectedTable : gIndexedTableFallback;
}

s32 func_00A848C8(s32);

s32 indexedCountGet(s32 tableIndex) {
    if (tableIndex != 0xA) {
        return func_00A848C8(tableIndex + 1) - 1;
    }
    return 0x11;
}

s32 gIndexedValueFallback = (s32) "**unknown**";
s32 func_00A85028(s32);

s32 indexedValueGet(s32 valueIndex) {
    s32 lookedUpValue;

    lookedUpValue = func_00A85028(valueIndex + 0x4C);
    return (lookedUpValue == 0) ? gIndexedValueFallback : lookedUpValue;
}

s32 func_001B93C0(s32 count);

s32 actorFlushPending(s32 tableRowCount) {
    return func_001B93C0(tableRowCount);
}
