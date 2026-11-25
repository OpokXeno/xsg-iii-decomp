#include "common.h"

#include "quadword.h"
#include "shared.h"
typedef unsigned char u8;

typedef int s32;

typedef short s16;

typedef unsigned short u16;

typedef unsigned int u32;

typedef float f32;

/* One archive entry table row: offset from the archive start and size (size 0 = entry absent). */
typedef struct ArchiveEntryRef {
    s32 offset;
    s32 size;
} ArchiveEntryRef;

typedef struct ArchiveHeader {
    ArchiveEntryRef entries[4];
} ArchiveHeader;

/* Header of the scene data (entry 0 of the 0xCF080000 archive): record counts and offsets of two record tables. */
typedef struct SceneData {
    u8 unmodeled_00[0x34];
    s32 recordCount;
    u8 unmodeled_38[0x34];
    s32 recordTableOffset;
    u8 unmodeled_70[0x60];
    s32 subSceneOffset;
} SceneData;

/* The stage model allocated by func_0028D670 (an actor record, cf. XenoActor in include/main/xenoActor.h). Kept as a
 * local view: the published type changes the emitted order. */
typedef struct StageModel {
    u8 unmodeled_00[0x20];
    Vector4 position;
    u8 unmodeled_30[0x20];
    Vector4 rotation;
    u8 unmodeled_60[0x20];
    Vector4 scale;
    s32 drawFlags;
    u8 unmodeled_94[0x25c];
    u8 visibility[0x20];
    u8 subVisibility[0x20];
    u8 unmodeled_330[0x20];
    u8 partAlpha[0x100];
    u8 unmodeled_450[0xB4];
    s32 sceneData; /* kept as s32: a pointer-typed field changes the emitted order of stageSetup */
    s32 extraData;
} StageModel;

typedef struct VecEntry {
    f32 x;
    f32 y;
    f32 z;
    s16 value;
    u16 id;
} VecEntry;

typedef struct PointSlot {
    s32 key;
    f32 x;
    f32 y;
    f32 z;
} PointSlot;

typedef struct GameStateBlock {
    s32 headValue;
    s16 pendingValue;
    u8 unk_06[0x2];
    u8 pendingRecord[0x304];
    s16 committedValue;
    u8 unk_30E[0x2];
    u8 committedRecord[0x304];
    s32 slotIds[2];
    s32 slotFlags[2];
    s32 currentSlot;
    u8 unmodeled_628[0x208];
    StageModel *stageModel;
    f32 cameraFovParam;
    f32 cameraNear;
    f32 cameraFar;
    f32 cameraClipFar;
    u8 unmodeled_844[12];
    Vector4 vector850;
    u8 light[0x180];
    s32 activeEntryKey;
    PointSlot pointSlots[16];
    u8 stageName[0x190];
    f32 fadeScale;
    SceneData *sceneData;
    void *extraData;
    void *collisionData;
    u8 *markerTable;
    VecEntry *markerEntries;
    s32 sceneRecordCount;
    u8 *sceneRecords;
    s32 subSceneRecordCount;
    u8 *subSceneRecords;
    SceneData *subSceneData;
    u8 partFlags[0x100];
    f32 partWeights[256];
} GameStateBlock;

/* Header of the stage marker table: the entry count is its first u16, the VecEntry rows start after 0x10 bytes. */
typedef struct MarkerTableHeader {
    u16 count;
    u8 unmodeled_02[0xE];
} MarkerTableHeader;

/* One tracked point of gSlotIdTable: position (w = the weight markerPositionGet returns), id (-1 = free) and two stored values. */
typedef struct TrackedPoint {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
    s16 id;
    s16 slotRef;
    s16 userValue;
    u8 unmodeled_16[0xA];
} TrackedPoint;

#define SLOT_COUNT 16

extern GameStateBlock D_0055CC00;

#define gGameStateBlock D_0055CC00

const unsigned int D_00531388[100] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0x101F4029, 0x101F4029, 0x101F4029, 0x101F402C, 0x101F402C, 0x101F402C, 0x101F402F,
    0x101F402F, 0x101F402F, 0x101F4032, 0x101F4032, 0x101F4032, 0x101F4035, 0x101F4035, 0x101F4035,
    0x101F4038, 0x101F4038, 0x101F4038, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0x101F4047,
    0x101F4047, 0x101F4047, 0x101F404A, 0x101F404A, 0x101F404A, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};

void *memset();

void stateBlockClear(void) {
    memset(&gGameStateBlock, 0, sizeof(GameStateBlock));
}

void stateBlockSetHead(s32 newHeadValue) {
    gGameStateBlock.headValue = newHeadValue;
}

GameStateBlock *stateBlockGet(void) {
    return &gGameStateBlock;
}

typedef struct CdReadRequest {
    s32 mode;
    char *path;
    void *dest;
    void (*onDone)();
    s32 signedId;
    u8 reserved[0x6c];
} CdReadRequest;

extern struct { s16 loadedId; u8 unmodeled_02[0x40]; } D_0055CC04;
extern u8 D_0055DDC0[];
void func_001BE5B0();
void xglCdFileRequestClear();
void effectListsClear();
s32 GameMpeg2Play();
s32 func_001B6FF8();
s32 xglCdReadRecordPrepare();

/* Reads the CD record for `id` (tries variant 0x1020000, then 0x1030000) and stores it as the loaded id of the state block. */
int func_001BE470(s16 id, s32 isPositive) {
    struct {
        char path[0x64];
        char name[0x1c];
    } names;
    CdReadRequest request;
    if (id == 0) {
        D_0055CC04.loadedId = 0;
        return 1;
    }
    if (id == gGameStateBlock.pendingValue) {
        return 1;
    }
    xglCdFileRequestClear(&request);
    request.mode = 2;
    request.path = names.path;
    request.dest = D_0055DDC0;
    request.onDone = func_001BE5B0;
    request.signedId = isPositive ? id : -id;
    GameMpeg2Play(names.name, "%04d", id);
    func_001B6FF8(names.path, 0x1020000, names.name);
    if (xglCdReadRecordPrepare(&request) != 0) {
        gGameStateBlock.pendingValue = id;
        return 1;
    }
    GameMpeg2Play(names.name, "%04d", id);
    func_001B6FF8(names.path, 0x1030000, names.name);
    if (xglCdReadRecordPrepare(&request) != 0) {
        gGameStateBlock.pendingValue = id;
        return 1;
    }
    gGameStateBlock.pendingValue = 0;
    effectListsClear(gGameStateBlock.pendingRecord);
    return 0;
}

/* The state block that holds a pending record. */
#define STATE_OF_PENDING_RECORD(record) ((GameStateBlock *) ((record) - 8))
extern u8 D_0055CC08[];
extern u8 D_0055E5BC[];
/* The file buffer func_001BE470 loads into (D_0055DDC0), reached from D_0055E5BC: that symbol is the relocation base. */
#define gLoadedRecordFile (D_0055E5BC - 0x7FC)
s32 func_001B7CA8(u8 *list, s16 value, u8 *out);
s32 func_001BE6A8(s32 key, s32 slotIndex, s32 aux);

typedef struct CdDoneRecord {
    u8 unmodeled_00[0x10];
    union {
        s32 signedId;
        u16 idMagnitude;
    } id;
} CdDoneRecord;

/* Completion callback of func_001BE470: the record's signed word at +0x10 carries the id and, by its sign, whether slotIndex 0 is loaded afterwards. */
void func_001BE5B0(CdDoneRecord *doneRecord) {
    s32 isPositive;
    s16 recordId;
    u8 *effectLists;

    isPositive = doneRecord->id.signedId >= 0;
    recordId = isPositive ? doneRecord->id.idMagnitude : -doneRecord->id.idMagnitude;
    effectLists = D_0055CC08;
    if (recordId != STATE_OF_PENDING_RECORD(effectLists)->pendingValue) {
        effectListsClear(effectLists);
        return;
    }
    *(s32 *) D_0055E5BC = 0x240000;
    effectListsClear(effectLists);
    func_001B7CA8(effectLists, recordId, gLoadedRecordFile);
    if (isPositive) {
        func_001BE6A8(((s32 *) effectLists)[2], 0, 0);
    }
}

void func_0030C130();

void stateBlockCommit(void)
{
    gGameStateBlock.committedValue = gGameStateBlock.pendingValue;
    func_0030C130(gGameStateBlock.pendingRecord, gGameStateBlock.committedRecord, 0x304);
}

extern s32 D_0055D214[];
void xcBlockClear(void *clearedBlock);
s32 func_001B70F0(s32 *loadRequest);
void func_001BE7C0(s32 *loadRequest);
s32 func_001B6918(s32 a, s32 b, s32 c, s32 d);

/* Requests the load of record `key` into `slot` (key 0 clears the slot); 1 when the slot already holds it or the load is queued. */
s32 func_001BE6A8(s32 key, s32 slot, s32 aux) {
    s32 loadRequest[6];
    s32 poolBlock;

    if (key == 0) {
        D_0055D214[slot] = 0;
        return 1;
    }
    if (gGameStateBlock.slotIds[slot] == key) {
        return 1;
    }
    poolBlock = func_001B6918(0, 1, 0xCF010000 + slot, -1);
    if (poolBlock == 0) {
        return 0;
    }
    xcBlockClear(loadRequest);
    loadRequest[0] = aux;
    loadRequest[1] = key;
    loadRequest[2] = poolBlock;
    loadRequest[3] = (s32) func_001BE7C0;
    loadRequest[4] = slot;
    loadRequest[5] = 1;
    if (func_001B70F0(loadRequest) == 0) {
        gGameStateBlock.slotIds[slot] = 0;
        return 0;
    }
    gGameStateBlock.slotIds[slot] = key;
    gGameStateBlock.slotFlags[slot] = 0;
    return 1;
}

void poolBlockTagWordSet(s32 block, s32 zero, s32 value);

/* Load-completion callback: when the slot still holds the requested id it tags the pool block and marks the slot loaded. */
void func_001BE7C0(s32 *loadRequest) {
    s32 slotIndex = loadRequest[4];
    s32 slotId = gGameStateBlock.slotIds[slotIndex];

    if (slotId == loadRequest[6]) {
        poolBlockTagWordSet(loadRequest[7], 0, slotId);
        gGameStateBlock.slotFlags[slotIndex] = 1;
    }
}

/* Id loaded in `slot`: -1 when the slot is empty, its id when its loaded flag is set, 0 while it still loads. */
s32 func_001BE828(s32 slotIndex) {
    s32 slotId = gGameStateBlock.slotIds[slotIndex];
    if (slotId == 0) {
        return -1;
    }
    if (gGameStateBlock.slotFlags[slotIndex] != 0) {
        return slotId;
    }
    return 0;
}

/* Empties `slot`: clears its id and its loaded flag. */
void func_001BE860(s32 slotIndex) {
    gGameStateBlock.slotIds[slotIndex] = gGameStateBlock.slotFlags[slotIndex] = 0;
}

GameStateBlock *stateBlockGet(void);
void slotTableClearIds();
void func_001BEF78();
void mapCollisionInit(void *collisionData);

/* Clears the stage pointers and record counts of the state block, the tracked points and the point slots, then
 * re-initialises collision from the cleared collision data. */
void stateBlockReset(void) {
    GameStateBlock *gameStateBlock;

    gameStateBlock = stateBlockGet();
    gameStateBlock->stageModel = 0;
    gameStateBlock->sceneData = 0;
    gameStateBlock->extraData = 0;
    gameStateBlock->collisionData = 0;
    gameStateBlock->markerTable = 0;
    gameStateBlock->markerEntries = 0;
    gameStateBlock->sceneRecordCount = 0;
    gameStateBlock->subSceneRecordCount = 0;
    slotTableClearIds();
    func_001BEF78();
    mapCollisionInit(gameStateBlock->collisionData);
}

s32 func_001B6520(s32 a);
s32 func_001B6890(s32 a);
s32 func_001B7330(s32 *a);

/* Allocates the pool blocks for the slot-0 load (0xCF010000 and 0xCF080000) and passes them to func_001B7330. */
s32 func_001BE8E0(void) {
    s32 poolBlocks[8];
    s32 slotZeroEntry;
    s32 archiveBlock;
    s32 isLoaded;

    slotZeroEntry = D_0055D214[0];
    poolBlocks[0] = func_001B6918(0, 1, 0xCF010000, -1);
    if (poolBlocks[0] == 0) {
        return 0;
    }
    if (slotZeroEntry == 0) {
        return 0;
    }
    poolBlocks[2] = func_001B6918(0, 1, 0xCF080000, -1);
    if (poolBlocks[2] == 0) {
        archiveBlock = func_001B6520(((s32 *) poolBlocks[0])[1]);
        poolBlocks[2] = archiveBlock;
        if (archiveBlock == 0) {
            return 0;
        }
        poolBlockTagWordSet(archiveBlock, 1, 0xCF080000);
    }
    poolBlocks[3] = func_001B6890(poolBlocks[2]);
    isLoaded = func_001B7330(poolBlocks);
    return isLoaded != 0;
}

void slotTableClearIds(void);
void func_001BEF78(void);

static inline void SetOne(Vector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(destination) : "memory");
}

static inline void ZeroOut4(Vector4 *destination) {
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

void func_00293A08();
void xglLightPrepare();
void pxyModelRenderStateLoadLight();
void func_0019C538();
void func_0028F2C8(f32);
u8 *func_002A6FA8();
StageModel *func_0028D670();
void func_0028D760();
void func_0028F8B8();
void func_0028F8C0();

static inline void *archiveEntryGet(ArchiveHeader *archive, s32 entryIndex) {
    return archive->entries[entryIndex].size ? (u8 *) archive + archive->entries[entryIndex].offset : 0;
}

/* Stage setup: loads the 0xCF080000 archive, sets the camera/light state of the state block, resolves the scene,
 * marker and collision data and creates the stage model. Returns 0 when the archive or the model is missing. */
int stageSetup() {
    s32 i;
    ArchiveHeader *archive;
    u8 *markerBlock;
    StageModel *model;
    GameStateBlock *state = &gGameStateBlock;
    archive = (ArchiveHeader *) func_001B6918(0, 1, 0xCF080000, -1);
    if (!archive) {
        return 0;
    }
    state->currentSlot = state->slotIds[0];
    slotTableClearIds();
    state->cameraFovParam = -0.0210752692f;
    state->cameraNear = 0.5f;
    state->cameraFar = 3.0f;
    state->cameraClipFar = 10000.0f;
    SetOne(&state->vector850);
    func_00293A08();
    xglLightPrepare(state->light);
    pxyModelRenderStateLoadLight(state->light);
    func_001BEF78();
    func_0030C130(D_00531388, state->stageName, 0x190);
    state->sceneData = archiveEntryGet(archive, 0);
    state->extraData = archiveEntryGet(archive, 1);
    state->collisionData = archiveEntryGet(archive, 2);
    state->fadeScale = 0.2f;
    func_0019C538(state->sceneData);
    func_0028F2C8(1.0f);
    mapCollisionInit(state->collisionData);
    markerBlock = func_002A6FA8(0x300, -1);
    state->markerTable = markerBlock;
    state->markerEntries = 0;
    if (markerBlock) {
        state->markerEntries = (VecEntry *) (markerBlock + sizeof(MarkerTableHeader));
    }
    state->sceneRecordCount = 0;
    state->sceneRecords = 0;
    if (state->sceneData) {
        state->sceneRecordCount = state->sceneData->recordCount;
        state->sceneRecords = (u8 *) state->sceneData + state->sceneData->recordTableOffset;
    }
    state->subSceneRecordCount = 0;
    state->subSceneRecords = 0;
    if (state->sceneData) {
        state->subSceneData = (SceneData *) ((u8 *) state->sceneData + state->sceneData->subSceneOffset);
        state->subSceneRecordCount = state->subSceneData->recordCount;
        state->subSceneRecords = (u8 *) state->subSceneData + state->subSceneData->recordTableOffset;
    }
    model = func_0028D670();
    state->stageModel = model;
    if (!model) {
        return 0;
    }
    model->drawFlags |= 0x40000000;
    ZeroOut4(&model->position);
    ZeroOut4(&model->rotation);
    SetOne(&model->scale);
    memset(model->visibility, 0, 0x20);
    memset(state->stageModel->subVisibility, 0, 0x20);
    state->stageModel->extraData = state->extraData;
    state->stageModel->sceneData = state->sceneData;
    func_0028D760(state->stageModel, state->extraData);
    memset(state->partFlags, 0, 0x100);
    func_0028F8B8(state->stageModel, state->partFlags);
    for (i = 0; i < 256; i++) {
        state->partWeights[i] = 1.0f;
    }
    func_0028F8C0(state->stageModel, state->partWeights);
    return 1;
}

/* Looks up marker `id` in the stage's marker table: writes its position to *dst (zero vector when absent) and returns its
 * value; an id that is not in the table falls back to the first entry. */
f32 markerPositionGet(DoublewordVector4 *outPosition, s16 id) {
    GameStateBlock *state;
    VecEntry *marker;
    s32 markerIndex;
    f32 value;

    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(outPosition) : "memory");
    if (id == -1) {
        return 0.0f;
    }
    state = &gGameStateBlock;
    if (state->markerTable == 0) {
        return 0.0f;
    }
    marker = state->markerEntries;
    for (markerIndex = 0; markerIndex < *(u16 *) state->markerTable; markerIndex++, marker++) {
        if (marker->id == id) {
            outPosition->vector.x = marker->x;
            outPosition->vector.y = marker->y;
            outPosition->vector.z = marker->z;
            value = marker->value;
            return value;
        }
    }
    marker = gGameStateBlock.markerEntries;
    outPosition->vector.x = marker->x;
    outPosition->vector.y = marker->y;
    outPosition->vector.z = marker->z;
    value = marker->value;
    return value;
}

extern TrackedPoint gSlotIdTable[];

void slotTableClearIds(void) {
    s32 i;
    TrackedPoint *slotCursor;

    slotCursor = gSlotIdTable;
    memset(gSlotIdTable, 0, SLOT_COUNT * sizeof(TrackedPoint));
    for (i = 0; i < SLOT_COUNT; i++) {
        slotCursor->id = -1;
        slotCursor++;
    }
}

/* Adds or updates tracked point `id`: a new point takes the first free entry, copying initialPosition or the marker
 * position from markerPositionGet; 0 when the table is full. */
s32 func_001BED60(s16 id, s16 slotRef, s16 userValue, void *initialPosition) {
    TrackedPoint *point;
    s32 pointIndex;

    point = gSlotIdTable;
    for (pointIndex = 0; pointIndex < SLOT_COUNT; pointIndex++, point++) {
        if (point->id < 0) {
            point->id = id;
            if (initialPosition != 0) {
                QuadwordCopyT0(point, initialPosition);
            } else {
                point->w = markerPositionGet((DoublewordVector4 *) point, id);
            }
            point->slotRef = slotRef;
            point->userValue = userValue;
            return 1;
        }
        if (point->id == id) {
            point->slotRef = slotRef;
            point->userValue = userValue;
            return 1;
        }
    }
    return 0;
}

/* Finds the tracked point with `id`: the scan stops at the first free point or after SLOT_COUNT entries; NULL when absent. */
TrackedPoint *func_001BEE18(s16 id) {
    TrackedPoint *point;
    s32 pointIndex;

    point = gSlotIdTable;
    pointIndex = 0;
    if (point->id >= 0) {
        while (1) {
            if (point->id == id) {
                return point;
            }
            pointIndex++;
            if (pointIndex >= SLOT_COUNT) {
                break;
            }
            point++;
            if ((u16) point->id >= 0x8000) {
                break;
            }
        }
    }
    return 0;
}

extern f32 gGameLoopRuntimeStorage[];
float MMathCalcDist();

/* Nearest tracked point to `pos` within 14 units of the runtime scale; points bound to the current slot get a 3-unit bonus. */
TrackedPoint *trackedPointNearest(void *pos) {
    TrackedPoint *point;
    TrackedPoint *nearestPoint;
    GameStateBlock *stateBlock;
    f32 bestDistance;
    f32 distance;
    s32 checkedCount;

    bestDistance = gGameLoopRuntimeStorage[364 / 4] * 14.0f;
    nearestPoint = 0;
    checkedCount = 0;
    point = gSlotIdTable;
    if (point->id >= 0) {
        stateBlock = (GameStateBlock *) ((u8 *) point - 1584);
        do {
            distance = MMathCalcDist(pos, point);
            if (point->slotRef == stateBlock->slotIds[0]) {
                distance = distance - gGameLoopRuntimeStorage[364 / 4] * 3.0f;
            }
            if (distance <= bestDistance) {
                bestDistance = distance;
                nearestPoint = point;
            }
            checkedCount++;
            point++;
        } while (checkedCount < 16 && point->id >= 0);
    }
    return nearestPoint;
}

void mapProbeInitTimeout(s32 value);

/* Resets the active entry key and frees all 16 point slots (key -1). */
void func_001BEF78(void) {
    GameStateBlock *state;
    s32 *slotKey;
    s32 none;
    s32 i;

    state = &gGameStateBlock;
    state->activeEntryKey = -1;
    mapProbeInitTimeout(-1);
    slotKey = &state->pointSlots[0].key;
    none = -1;
    i = 15;
    do {
        *slotKey = none;
        slotKey += 4;
        i--;
    } while (i >= 0);
}

/* Stores a point in the first slot with the same key or the first free one (key -1); 0 when all 16 are taken. */
int pointSlotStore(PointSlot *point) {
    s32 i;
    PointSlot *slot;
    GameStateBlock *state;
    state = &gGameStateBlock;
    for (i = 0; i < 16; i++) {
        slot = &state->pointSlots[i];
        if (slot->key == point->key) {
            slot->x = point->x;
            slot->y = point->y;
            slot->z = point->z;
            return 1;
        }
        if (slot->key == -1) {
            slot->key = point->key;
            slot->x = point->x;
            slot->y = point->y;
            slot->z = point->z;
            return 1;
        }
    }
    return 0;
}

/* Finds the point slot with `key`; the scan stops at the first free slot (key -1). */
PointSlot *pointSlotFind(s32 key) {
    GameStateBlock *state;
    PointSlot *slot;
    s32 index;

    state = &gGameStateBlock;
    slot = state->pointSlots;
    index = 0;
    if (slot->key != -1) {
        while (1) {
            if (slot->key == key) {
                return slot;
            }
            index++;
            if (index >= SLOT_COUNT) {
                break;
            }
            slot++;
            if (slot->key == -1) {
                break;
            }
        }
    }
    return 0;
}

typedef signed char s8;

/* Clears bit 0 of part flag `partIndex` and stores `weight` (0..1, 0.99 and above = opaque) as its alpha byte. */
int stagePartAlphaSet(s32 partIndex, f32 weight) {
    GameStateBlock *state;
    u8 *partFlag;
    s8 *partAlpha;
    s32 flagOffset;
    s32 alphaOffset;

    state = stateBlockGet();
    if ((u32) partIndex > 0x100) {
        return 0;
    }
    flagOffset = partIndex + 0xca0;
    partFlag = (u8 *) state + flagOffset;
    *partFlag &= 0xfe;
    alphaOffset = partIndex + 0x350;
    partAlpha = (s8 *) ((u8 *) state->stageModel + alphaOffset);
    *partAlpha = (weight > 0.99f) ? -1 : (s32) (weight * 255.0f);
    return 1;
}

extern s32 D_0055D6E4[];

#define WORD_TABLE_COUNT 0x64

s32 wordTableGet(u32 wordIndex) {
    if (!(wordIndex < WORD_TABLE_COUNT)) {
        return 0;
    }
    return D_0055D6E4[wordIndex];
}

s32 wordTableSet(u32 wordIndex, s32 wordValue) {
    if (!(wordIndex < WORD_TABLE_COUNT)) {
        return 0;
    }
    D_0055D6E4[wordIndex] = wordValue;
    return 1;
}

void func_001B64D8(s32 begin, s32 end);

void memoryRangeInitDefault(void) {
    func_001B64D8(0xE00000, 0x01200000);
}

s32 func_001BF1B8(s32 obj, s32 value);

/* Allocates a pool block at bank `value` (func_001B6918), or else a block of `obj` (func_001B6520), tagged with `value`. */
s32 func_001BF1B8(s32 obj, s32 poolTag) {
    s32 block;
    s32 status;

    status = func_001B6918(0, 1, poolTag, -1);
    if (status != 0) {
        return status;
    }
    block = func_001B6520(obj);
    if (block == 0) {
        return 0;
    }
    poolBlockTagWordSet(block, 1, poolTag);
    return block;
}

/* Runs the allocation test of each memory bank selected in the mask; returns 1 when every allocation succeeds. */
s32 memoryBanksTest(s32 memoryBankMask) {
    s32 errors;

    errors = 0;
    if (memoryBankMask & 1) {
        errors = func_001BF1B8(0x100000, 0xCF800000) == 0;
        if (func_001BF1B8(0x39800, 0xCF801000) == 0) {
            errors++;
        }
    }
    if (memoryBankMask & 2) {
        if (func_001BF1B8(0x200000, 0xCF010000) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x120080, 0xCF010001) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010100) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010101) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010102) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x70880, 0xCF010100) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010200) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010201) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010210) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010211) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010220) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x10080, 0xCF010221) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x4E080, 0xCF010300) == 0) {
            errors++;
        }
    }
    if (memoryBankMask & 4) {
        if (func_001BF1B8(0xA3880, 0xCF020000) == 0) {
            errors++;
        }
        if (func_001BF1B8(0xA3880, 0xCF020001) == 0) {
            errors++;
        }
        if (func_001BF1B8(0xA3880, 0xCF020002) == 0) {
            errors++;
        }
    }
    if (memoryBankMask & 8) {
        if (func_001BF1B8(0x64080, 0xCF040000) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x64080, 0xCF040001) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x64080, 0xCF040002) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x64080, 0xCF040003) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040100) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040101) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040102) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x85080, 0xCF040103) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040200) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040201) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040202) == 0) {
            errors++;
        }
        if (func_001BF1B8(0x1C080, 0xCF040203) == 0) {
            errors++;
        }
    }
    return errors == 0;
}

s32 poolReleaseMatching(s32 mode, s32 expected, s32 mask);

/* Releases the pool blocks of each memory bank selected in the mask. */
void memoryBanksReset(s32 memoryBankMask) {
    if (memoryBankMask & 1) {
        poolReleaseMatching(1, 0xCF800000, 0xFFFF0000);
    }
    if (memoryBankMask & 2) {
        poolReleaseMatching(1, 0xCF010000, 0xFFFF0000);
    }
    if (memoryBankMask & 4) {
        poolReleaseMatching(1, 0xCF020000, 0xFFFF0000);
    }
    if (memoryBankMask & 8) {
        poolReleaseMatching(1, 0xCF040000, 0xFFFF0000);
    }
    if (memoryBankMask & 0x10) {
        poolReleaseMatching(1, 0xCF080000, 0xFFFF0000);
    }
    if (memoryBankMask & 0x20) {
        poolReleaseMatching(1, 0xCF100000, 0xFFFF0000);
    }
}

void xglStudioReset(void);
void xglStudioMainCameraInit(void);

void studioCameraInit(void) {
    xglStudioReset();
    xglStudioMainCameraInit();
}

/* The head of an XglCamera (include/main/xgl_2.h) as func_001BF688 writes it; the header's field types are not used here. */
typedef struct StudioCameraSetup {
    u32 active;
    u8 unmodeled_04[0x9C];
    f32 unk_A0;
    u8 unmodeled_A4[0x2C];
    s32 unk_D0;
    f32 unk_D4;
    s32 unk_D8;
} StudioCameraSetup;

void setCurrentStudio(int studio);
StudioCameraSetup *xglStudioGetCamera2(int camera_index);
void xglCameraInit(void *camera);
void xglCameraMove(void *camera);
void xglCameraSetPadPort(void *camera, int port);

/* Initialises studio 0's main camera on pad port 0 and activates it. */
void func_001BF688(void) {
    StudioCameraSetup *camera;
    f32 cameraScale;
    f32 cameraOffsetA0;
    s32 isActive;

    setCurrentStudio(0);
    camera = xglStudioGetCamera2(0);
    xglCameraInit(camera);
    xglCameraMove(camera);
    xglCameraSetPadPort(camera, 0);
    isActive = 1;
    /* kept: the do/while block decides where the constant is materialised */
    do {
        cameraScale = 2.5f;
    } while (0);
    cameraOffsetA0 = -0.104719758f;
    camera->active = isActive;
    camera->unk_A0 = cameraOffsetA0;
    camera->unk_D4 = cameraScale;
    camera->unk_D8 = 0;
    camera->unk_D0 = 0;
}

f32 func_002AA490();

typedef struct CollisionContext {
    s32 first;
    s32 second;
    u8 unmodeled_08[0x1c];
    s32 mode;
} CollisionContext;

/* Sets p->y from func_002AA490 and leaves the first, second and mode words of the context unchanged. */
void func_001BF6F8(Vector4 *position, Vector4 *probe, CollisionContext *collisionContext) {
    s32 savedFirst;
    s32 savedSecond;
    s32 savedMode;
    savedFirst = collisionContext->first;
    savedSecond = collisionContext->second;
    savedMode = collisionContext->mode;
    position->y = func_002AA490(position, probe, collisionContext);
    collisionContext->first = savedFirst;
    collisionContext->second = savedSecond;
    collisionContext->mode = savedMode;
}

s32 func_001BF7C8(f32 firstFactor, f32 secondFactor);

/* Wraps the rounded angle (in angle units scaled by gAngleUnitsToDegrees) into 0..period, period = turns * 360. */
s32 angleWrapTurns(f32 angle, f32 turns) {
    s32 scaled;
    s32 period;
    s32 remainder;

    scaled = func_001BF7C8(angle * 57.2957802f, turns);
    period = turns * 360.0f;
    remainder = scaled % period;
    return ((remainder < 0) * period) + remainder;
}

/* Rounds firstFactor * secondFactor to the nearest integer, halves away from zero. */
s32 func_001BF7C8(f32 firstFactor, f32 secondFactor) {
    f32 product;

    if (firstFactor < 0.0f) {
        product = firstFactor * secondFactor + -0.5f;
    } else {
        product = firstFactor * secondFactor + 0.5f;
    }
    return (s32) product;
}

f32 MMathCalcDir(void *v);
f32 MMathCalcRotNear(f32 a, f32 b);

typedef struct FacingActor {
    u8 unmodeled_00[0x20];
    Vector4 position;
    u8 unmodeled_30[0x20];
    Vector4 rotation;
} FacingActor;

/* Absolute angle between the actor's facing (rotation.y) and the direction of its position vector. */
f32 actorFacingError(FacingActor *actor) {
    f32 direction;
    f32 angleDelta;

    direction = MMathCalcDir(&actor->position);
    angleDelta = MMathCalcRotNear(actor->rotation.y, direction);
    return __builtin_fabsf(angleDelta);
}

GameStateBlock D_0055CC00;
unsigned char D_0055DDA0[32];
unsigned char D_0055DDC0[2048];
