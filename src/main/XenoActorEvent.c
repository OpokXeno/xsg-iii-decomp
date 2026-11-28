#include "common.h"
#include "quadword.h"
#include "main/xenoActor.h"
#include "main/assert_fail.h"
#include "main/xgl_2.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))

typedef int s32;

typedef unsigned char u8;

typedef struct XenoCdRsrcHeader {
    u8 unmodeled_00[0xD0];
    s32 subPackOffset;
} XenoCdRsrcHeader;

typedef struct XenoSubPack {
    u8 unmodeled_00[0x60];
    s32 matrixCount;
    u8 unmodeled_64[0x4];
    s32 matrixOffset;
} XenoSubPack;

extern XenoMatrix *gXenoActorMatrixTable[];
void pxyUnscrambleHeader(void *header);
XenoMatrix *func_0028D350(s32 count);

typedef unsigned int u32;

/* When gXenoActorMatrixTable[index] is empty, allocates it and copies actor's sub-pack matrices into it. */
void xenoActorMatrixTableLoad(XenoActor *actor, s32 index) {
    XenoCdRsrcHeader *rsrcHeader;
    XenoSubPack *subPack;
    u32 matrixSource;
    s32 i;

    if (actor == 0) {
        return;
    }
    rsrcHeader = (XenoCdRsrcHeader *) actor->rsrc[0];
    if (rsrcHeader == 0) {
        return;
    }
    pxyUnscrambleHeader(rsrcHeader);
    if (gXenoActorMatrixTable[index] != 0) {
        return;
    }
    if (rsrcHeader->subPackOffset == 0) {
        return;
    }
    subPack = (XenoSubPack *) ((u8 *) rsrcHeader + rsrcHeader->subPackOffset);
    pxyUnscrambleHeader(subPack);
    matrixSource = (u32) subPack + subPack->matrixOffset;
    gXenoActorMatrixTable[index] = func_0028D350(subPack->matrixCount);
    for (i = 0; i < subPack->matrixCount; i++) {
        QuadwordCopyMatrix(gXenoActorMatrixTable[index] + i, (u8 *) (i * 64 + matrixSource));
    }
}

/* Stores the matrices of gXenoActorMatrixTable[index] back into actor's sub-pack (the reverse of xenoActorMatrixTableLoad). */
void xenoActorMatrixTableStore(XenoActor *actor, s32 index) {
    XenoCdRsrcHeader *rsrcHeader;
    XenoSubPack *subPack;
    u32 matrixDest;
    s32 i;

    if (actor == 0) {
        return;
    }
    rsrcHeader = (XenoCdRsrcHeader *) actor->rsrc[0];
    if (rsrcHeader == 0) {
        return;
    }
    if (gXenoActorMatrixTable[index] == 0) {
        return;
    }
    i = 0;
    subPack = (XenoSubPack *) ((u8 *) rsrcHeader + rsrcHeader->subPackOffset);
    matrixDest = (u32) subPack + subPack->matrixOffset;
    if (subPack->matrixCount <= 0) {
        return;
    }
    for (; i < subPack->matrixCount; i++) {
        QuadwordCopyMatrix((u8 *) (i * 64 + matrixDest), gXenoActorMatrixTable[index] + i);
    }
}

typedef struct XenoActorTable {
    u8 unk_00[0x50];
    u32 actorCount;
} XenoActorTable;

typedef struct XenoActorRefA {
    u8 unk_00[0x30];
    s32 id;
} XenoActorRefA;

typedef struct XenoActorRefB {
    u8 unk_00[0x24];
    s32 id;
    s32 kind;
} XenoActorRefB;

typedef struct XenoCdActorSlot {
    u8 unk_000[0x640];
    XenoActorRefA *refA;
    XenoActorRefB *refB;
} XenoCdActorSlot;

typedef struct XglCdBlockStreamView {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    XenoCdActorSlot *actorEntries[0x98D];
    u32 actorEntryCount;
    u8 unk_3F10[0x10];
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStreamView;

extern unsigned char gXglCdBlockStreamStorage[];

#define gXglCdBlockStreamView (*(XglCdBlockStreamView *)&gXglCdBlockStreamStorage)

/* Returns the slot of the actor table whose refA id (or refB id, kind 1) is id, or 0 when none matches. */
XenoCdActorSlot *xenoCdActorSlotFind(s32 id) {
    u32 i;
    XenoCdActorSlot *slot;

    if (id < 0) {
        return 0;
    }
    i = 0;
    while (i < gXglCdBlockStreamView.actorEntryCount) {
        slot = gXglCdBlockStreamView.actorEntries[i];
        if (slot->refA != 0) {
            if (slot->refA->id == id) {
                return slot;
            }
        } else if (slot->refB != 0 && slot->refB->kind == 1 && slot->refB->id == id) {
            return slot;
        }
        i++;
    }
    return 0;
}

/* Global block gXglCdBlockStreamStorage (gXglCdBlockStream: CD block-stream loader; path buffers at +0x1810 are D_003E8B10/50/90). */
typedef struct XglCdBlockStream {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    u8 unk_18D8[0x2648];
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStream;

/* Current actor record pointed to by gXenoCurrentActorRecord. */
typedef struct XenoActorRecord {
    u8 unk_00[0xC];
    s32 actorDataAddress;
    u8 unk_10[0x8];
    s32 unk_18;
    u8 unk_1C[0x14];
    s32 unk_30;
    u8 unk_34[0x4];
    s32 unk_38;
} XenoActorRecord;

/* Range record pointed to by gXenoCurrentActorRange: length is rangeEnd - rangeBegin. */
typedef struct XenoActorRange {
    u8 unk_00[0xC];
    s32 rangeBegin;
    s32 rangeEnd;
} XenoActorRange;

s32 pxySetSyncMode(s32 mode);
s32 pxySetHeaderScanEnabled(s32 mode);
s32 xglCdDriveReadyStatusLowBitSet(void);

void xenoActorEventSetSyncMode(void) {
    if (xglCdDriveReadyStatusLowBitSet() != 0) {
        pxySetHeaderScanEnabled(0);
    } else {
        pxySetHeaderScanEnabled(1);
    }
    pxySetSyncMode(2);
}

void func_0029BEE0(void *record, u8 **cursor, void *data, s32 kind);
void tiVu0LoadIdentityMatrix(void);
void xglCurMulTranslate(float *translation);
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurMulRotateZ(float angle);
void xglCurMulScaleXYZ(const float scale[3]);
void xglCurMatrixGet(float matrix[4][4]);

typedef struct XenoPlacement {
    u8 unmodeled_00[0x14];
    s32 count;
    s32 dataOffset;
} XenoPlacement;

typedef struct XenoPlacementRecord {
    float translate[3];
    float rotate[3];
    float scale[3];
} XenoPlacementRecord;

typedef struct XenoMatrixSlot {
    float matrix[15];
    u32 flags;
} XenoMatrixSlot;

/* Sets the flagged matrices of actor's sub-pack from the placement records: each record's translate, rotate and scale is applied to the current matrix and stored. */
void xenoActorSubPackApplyPlacements(XenoActor *actor, XenoPlacement *placement, void *data) {
    XenoCdRsrcHeader *rsrcHeader;
    XenoSubPack *subPack;
    XenoMatrixSlot *matrices;
    XenoPlacementRecord record;
    u8 *cursor;
    u32 i;

    rsrcHeader = (XenoCdRsrcHeader *) actor->rsrc[0];
    if (placement->count == 0) {
        return;
    }
    pxyUnscrambleHeader(rsrcHeader);
    subPack = (XenoSubPack *) ((u8 *) rsrcHeader + rsrcHeader->subPackOffset);
    pxyUnscrambleHeader(subPack);
    if (placement->count != subPack->matrixCount) {
        return;
    }
    matrices = (XenoMatrixSlot *) ((u8 *) subPack + subPack->matrixOffset);
    cursor = (u8 *) placement + placement->dataOffset;
    for (i = 0; i < placement->count; i++) {
        func_0029BEE0(&record, &cursor, data, 9);
        if (record.translate[0] > -99999.0f) {
            tiVu0LoadIdentityMatrix();
            if (record.translate[0] != 0.0f || record.translate[1] != 0.0f || record.translate[2] != 0.0f) {
                xglCurMulTranslate(record.translate);
            }
            if (record.rotate[2] != 0.0f) {
                xglCurMulRotateZ(record.rotate[2]);
            }
            if (record.rotate[1] != 0.0f) {
                xglCurMulRotateY(record.rotate[1]);
            }
            if (record.rotate[0] != 0.0f) {
                xglCurMulRotateX(record.rotate[0]);
            }
            xglCurMulScaleXYZ(record.scale);
            xglCurMatrixGet((float (*)[4]) & matrices[i]);
            matrices[i].flags |= 1;
        }
    }
}

void xglMatrixStackUnit(void);
void xglMatrixStackFrustum(float left, float right, float bottom, float top, float nearPlane, float farPlane);
void xglMatrixStackSave(float matrix[4][4]);
void xglCurMatrixSet(float matrix[4][4]);
void xglCurApplyVector(float *destination, float *source);
struct XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

typedef struct XenoCameraView {
    u8 unmodeled_00[0x20];
    float nearPlane;
    float farPlane;
    u8 unmodeled_28[0x50];
    float depthOffset;
    u8 unmodeled_7C[0x0C];
    float depthScale;
    u8 unmodeled_8C[0x3AC];
    float left;
    u8 unmodeled_43C[0x0C];
    float right;
    u8 unmodeled_44C[0x0C];
    float bottom;
    u8 unmodeled_45C[0x0C];
    float top;
} XenoCameraView;

typedef struct XenoUnalignedVec4 {
    u8 unmodeled_00[16];
} XenoUnalignedVec4;

/* Projects the point source through the active camera's frustum and returns its depth, 0 when it is behind the camera. */
s32 xenoActorEventDepthOfPoint(XenoUnalignedVec4 *worldPoint) {
    XenoCameraView *camera;
    XenoUnalignedVec4 vector;
    float saved[4][4];
    float *components;
    s32 depth;

    camera = (XenoCameraView *) xglStudioSelectGetActiveCamera(0);
    xglMatrixStackUnit();
    xglMatrixStackFrustum(camera->left, camera->right, camera->bottom, camera->top, camera->nearPlane,
                          camera->farPlane);
    xglMatrixStackSave(saved);
    vector = *worldPoint;
    xglCurMatrixSet(saved);
    xglCurApplyVector((float *) &vector, (float *) &vector);
    components = (float *) &vector;
    depth = 0;
    if (components[2] < 0.0f) {
        depth = 0;
    } else {
        depth = (s32) (u32) (components[2] / components[3] * camera->depthScale + camera->depthOffset);
    }
    return depth;
}

s32 xenoActorEventAddOffset(s32 baseAddress, s32 displacement) {
    return baseAddress + displacement;
}

#define gXglCdBlockStream (*(XglCdBlockStream *) gXglCdBlockStreamStorage)

s32 xenoActorEntryAt(XenoActorTable *table, u32 index);
s32 func_002A1B30(s32 entry, s32 value);

void xenoActorEventApplyEntries(void)
{
  u32 actorCount;
  XglCdBlockStream *loopBlockStream;
  XglCdBlockStream *blockStream;
  u32 actorIndex;
  u32 visitedCount;
  s32 actorEntry;
  visitedCount = 0;
  blockStream = &gXglCdBlockStream;
  if (blockStream->currentActorRecord->actorCount != 0)
  {
    do
    {
      actorIndex = visitedCount;
      visitedCount += 1;
      loopBlockStream = &gXglCdBlockStream;
      actorEntry = xenoActorEntryAt(loopBlockStream->currentActorRecord, actorIndex);
      func_002A1B30(actorEntry, loopBlockStream->unk_3F20);
      actorCount = loopBlockStream->currentActorRecord->actorCount;
    }
    while (visitedCount < actorCount);
  }
}

typedef struct XenoActorRefAView {
    u8 unmodeled_00[0x30];
    s32 id;
} XenoActorRefAView;

typedef struct XenoActorRefBView {
    u8 unmodeled_00[0x24];
    s32 id;
    s32 kind;
} XenoActorRefBView;

typedef struct XenoCdActorSlotView {
    u8 unmodeled_000[0x640];
    XenoActorRefAView *refA;
    XenoActorRefBView *refB;
} XenoCdActorSlotView;

typedef struct XglCdBlockStreamAlt {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    XenoCdActorSlotView *actorEntries[0x98D];
    u32 actorEntryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStreamAlt;

#define gXglCdBlockStreamAlt (*(XglCdBlockStreamAlt *)&gXglCdBlockStreamStorage)

s32 func_0024E380(XenoActorTable *table, u32 index);
void func_00297240(s32 entry, s32 value);

typedef struct XenoActorTableCount58 {
    u8 unmodeled_00[0x58];
    u32 entryCount;
} XenoActorTableCount58;

/* Resets the entry list, then runs func_00297240 for every entry of the current actor record (assert on a missing entry). */
void xenoActorEventReloadEntries(void) {
    u32 index;
    s32 actorEntry;

    func_00297240(0, 0);
    for (index = 0;
         index < ((XenoActorTableCount58 *) gXglCdBlockStreamAlt.currentActorRecord)->entryCount;
         index++) {
        actorEntry = func_0024E380(gXglCdBlockStreamAlt.currentActorRecord, index);
        if (actorEntry == 0) {
            assertFailed("pColl", "XenoActorEvent.euc.c", 533);
        }
        func_00297240(actorEntry, gXglCdBlockStreamAlt.unk_3F20);
    }
}

static s32 D_005A6C58[2];
void *actorRecordsEnd(XenoActorTable *table, u32 index);
void slotReleaseAllPools(void);
void slotSetAllEnableBits(s32 enable);
void slotSetupMode0(s32 slot, s32 mode, void *payload);
void slotFixedRecordSetPayload(void *payload);
void scratchpadSendCmd0A(void *command);
void *memset(void *dest, s32 value, u32 size);

typedef struct XenoActorBlock {
    u8 unk_00[0xC];
    u32 entryCount;
    s32 entryKind[16];
    s32 entryOffset[16];
} XenoActorBlock;

typedef struct XenoActorTableCount48 {
    u8 unk_00[0x48];
    u32 entryCount;
} XenoActorTableCount48;

typedef struct XenoStreamRecord8 {
    f32 field0;
    f32 field4;
    f32 field8;
    f32 fieldC;
    f32 field10;
    f32 field14;
    f32 field18;
    f32 field1C;
} XenoStreamRecord8;

typedef struct XenoPackedRecord8 {
    s32 field0;
    s32 field4;
    s32 field8;
    s32 fieldC;
    s32 field10;
    s32 field14;
    s32 field18;
    s32 field1C;
} XenoPackedRecord8;

typedef struct XenoStreamRecord5 {
    f32 field0;
    f32 field4;
    f32 field8;
    f32 fieldC;
    f32 field10;
} XenoStreamRecord5;

typedef struct XenoPackedRecord5 {
    s32 field0;
    s32 field4;
    s32 field8;
    s32 fieldC;
    s32 field10;
} XenoPackedRecord5;

typedef struct XenoPackedRecord6 {
    s32 field0;
    s32 field4;
    s32 field8;
    s32 fieldC;
    s32 field10;
    s32 field14;
} XenoPackedRecord6;

typedef struct XenoStreamRecord3 {
    f32 field0;
    f32 field4;
    f32 field8;
} XenoStreamRecord3;

typedef struct XenoPackedRecord3 {
    s32 field0;
    s32 field4;
    s32 field8;
} XenoPackedRecord3;

typedef struct XenoPackedRecord4 {
    s32 field0;
    s32 field4;
    s32 field8;
    s32 fieldC;
} XenoPackedRecord4;

typedef struct XenoFloatPair2 {
    f32 field0;
    f32 field4;
} XenoFloatPair2;

typedef union XenoFloatBits {
    f32 f;
    s32 s;
} XenoFloatBits;

const char gAssertElemHeaderName[24] = "pElemHeader";

/* Sends one slot-setup packet per entry of every actor block (the packet kind is the entry kind, the slot is entryIndex + 0xA0); enableSlots == 1 enables all slots, else they are cleared. Assert when a block is missing. */
void xenoActorEventSendSlotPackets(s32 enableSlots) {
    XenoStreamRecord8 kind0Record;
    XenoPackedRecord8 kind0Payload;
    XenoStreamRecord5 kind1Record;
    XenoPackedRecord6 kind1Payload;
    XenoStreamRecord3 kind2Record;
    XenoPackedRecord3 kind2Payload;
    XenoStreamRecord3 kind3Record;
    XenoPackedRecord4 kind3Payload;
    XenoStreamRecord5 kind4Record;
    XenoPackedRecord5 kind4Payload;
    XenoStreamRecord5 kind6Record;
    XenoPackedRecord5 kind6Payload;
    f32 kind6Point[4];
    XenoStreamRecord3 kind7Record;
    XenoPackedRecord4 kind7Payload;
    XenoFloatPair2 kind5Pair;
    s32 kind0Cursor;
    s32 kind1Cursor;
    s32 kind2Cursor;
    s32 kind3Cursor;
    s32 kind4Cursor;
    s32 kind6Cursor;
    s32 kind7Cursor;
    s32 kind5Cursor;
    XenoActorBlock *actorBlock;
    s32 actorIndex;
    s32 entryIndex;
    s32 commandWord;
    XenoFloatBits floatBits;
    XglCdBlockStreamAlt *blockStream;

    slotReleaseAllPools();
    actorIndex = 0;
    while (actorIndex < ((XenoActorTableCount48 *) gXglCdBlockStreamAlt
                             .currentActorRecord)
                            ->entryCount) {
        actorBlock = actorRecordsEnd(gXglCdBlockStreamAlt.currentActorRecord,
                                (u32) actorIndex);
        if (actorBlock == 0) {
            assertFailed(gAssertElemHeaderName, "XenoActorEvent.euc.c", 0x229);
        }
        entryIndex = 0;
        while (entryIndex < actorBlock->entryCount) {
            switch (actorBlock->entryKind[entryIndex] - 0x29) {
            case 0:
                kind0Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind0Record, (u8 **) &kind0Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 8);
                kind0Payload.field0 = (s32) kind0Record.field0;
                kind0Payload.field4 = (s32) kind0Record.field4;
                kind0Payload.field8 = (s32) kind0Record.field8;
                kind0Payload.fieldC = (s32) kind0Record.fieldC;
                kind0Payload.field10 = (s32) kind0Record.field10;
                kind0Payload.field14 = (s32) kind0Record.field14;
                kind0Payload.field18 = (s32) kind0Record.field18;
                kind0Payload.field1C = (s32) kind0Record.field1C;
                slotSetupMode0(entryIndex + 0xA0, 4, &kind0Payload.field0);
                break;
            case 1:
                kind1Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind1Record, (u8 **) &kind1Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 5);
                kind1Payload.field14 = 0x80;
                kind1Payload.field8 = (s32) (kind1Record.field0 * 255.0f);
                kind1Payload.fieldC = (s32) (kind1Record.field4 * 255.0f);
                kind1Payload.field0 = (s32) (kind1Record.fieldC * 255.0f) - 0x80;
                kind1Payload.field4 = (s32) (kind1Record.field10 * 255.0f) - 0x80;
                kind1Payload.field10 = (s32) (kind1Record.field8 * 255.0f);
                slotFixedRecordSetPayload(&kind1Payload.field0);
                break;
            case 2:
                kind2Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind2Record, (u8 **) &kind2Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 3);
                kind2Payload.field0 = (s32) kind2Record.field0;
                kind2Payload.field4 = (s32) kind2Record.field4;
                kind2Payload.field8 = (s32) kind2Record.field8;
                slotSetupMode0(entryIndex + 0xA0, 0x1E, &kind2Payload.field0);
                break;
            case 3:
                kind3Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind3Record, (u8 **) &kind3Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 3);
                kind3Payload.fieldC = 0;
                kind3Payload.field0 = (s32) kind3Record.field0;
                kind4Record.field0 = 0.0f;
                kind3Payload.field4 = (s32) (kind3Record.field4 * 255.0f);
                kind3Payload.field8 = (s32) (kind3Record.field8 * 255.0f);
                slotSetupMode0(entryIndex + 0xA0, 9, &kind3Payload.field0);
                break;
            case 4:
                kind4Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind4Record.field0, (u8 **) &kind4Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 5);
                kind4Payload.field0 = (s32) kind4Record.field0;
                kind4Payload.fieldC = (s32) kind4Record.fieldC;
                kind4Payload.field4 = (s32) (kind4Record.field4 * 255.0f);
                kind4Payload.field8 = (s32) (kind4Record.field8 * 255.0f);
                kind4Payload.field10 = (s32) kind4Record.field10;
                slotSetupMode0(entryIndex + 0xA0, 9, &kind4Payload.field0);
                break;
            case 6:
                kind6Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                memset(kind6Point, 0, 0x10);
                blockStream = &gXglCdBlockStreamStorage;
                kind6Point[3] = 1.0f;
                func_0029BEE0(&kind6Record, (u8 **) &kind6Cursor, (void *) blockStream->unk_3F20, 5);
                kind6Point[2] = -kind6Record.fieldC;
                kind6Payload.field0 = xenoActorEventDepthOfPoint((XenoUnalignedVec4 *) kind6Point);
                kind6Payload.field4 = (u32) kind6Record.field0;
                kind6Payload.field8 = (u32) (kind6Record.field8 * 128.0f);
                kind6Payload.fieldC = (u32) kind6Record.field10;
                kind6Payload.field10 = (u32) kind6Record.field4;
                slotSetupMode0(entryIndex + 0xA0, 0x21, &kind6Payload.field0);
                break;
            case 7:
                kind7Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind7Record, (u8 **) &kind7Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 3);
                kind7Payload.field0 = 0xFFFFFF;
                kind7Payload.field4 = (u32) kind7Record.field0;
                kind7Payload.field8 = (u32) (kind7Record.field8 * 128.0f);
                kind7Payload.fieldC = 0;
                floatBits.s = (u32) kind7Record.field4;
                kind5Pair.field0 = floatBits.f;
                slotSetupMode0(entryIndex + 0xA0, 0x21, &kind7Payload.field0);
                break;
            case 5:
                kind5Cursor = xenoActorEventAddOffset((s32) actorBlock, actorBlock->entryOffset[entryIndex]);
                func_0029BEE0(&kind5Pair.field0, (u8 **) &kind5Cursor,
                              (void *) gXglCdBlockStreamAlt.unk_3F20, 2);
                if (0.9999f <= kind5Pair.field0) {
                    commandWord = (s32) kind5Pair.field4;
                    D_005A6C58[0] = commandWord;
                    D_005A6C58[1] = 1;
                    scratchpadSendCmd0A(D_005A6C58);
                }
                break;
            }
            entryIndex += 1;
        }
        actorIndex += 1;
    }
    if (enableSlots != 1) {
        slotSetAllEnableBits(0);
        return;
    }
    slotSetAllEnableBits(1);
}

typedef struct XenoActorTableView {
    u8 unk_00[0x38];
    s32 unk_38;
    u8 unk_3C[0x14];
    u32 actorCount;
} XenoActorTableView;

typedef struct XenoActorSlot {
    u8 unk_00[0x20];
    float unk_20;
    float unk_24;
    float unk_28;
    u8 unk_2C[0x24];
    float unk_50;
    float unk_54;
    float unk_58;
    u8 unk_5C[0x24];
    float unk_80;
    float unk_84;
    union {
        float f;
        s32 i;
    } unk_88;
    u8 unk_8C[0x4];
    s32 flags;
    float unk_94;
    u8 unk_98[0x5A8];
    u8 *entry640;
    u8 *entry644;
} XenoActorSlot;

typedef struct XglCdBlockStreamLocal {
    u8 unk_0000[0x1810];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x84];
    XenoActorTableView *currentActorRecord;
    XenoActorSlot *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 matchValue;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
    s32 unk_415C;
} XglCdBlockStreamLocal;

#define gXglCdBlockStreamLocal (*(XglCdBlockStreamLocal *) gXglCdBlockStreamStorage)

char *strcpy();
void sprintf();
void func_0030E3D0();
s32 strlen(char *name);

/* Opens the event file path: strips its extension, plays evt\<name> through sprintf, then resets the block-stream state and cached path for the .xev file. */
void xenoActorEventOpenFile(char *path) {
    char *eventFileName;
    char *charCursor;
    s32 pathLength;

    eventFileName = (char *) gXglCdBlockStreamLocal.pathBuffer;
    charCursor = (char *) gXglCdBlockStreamLocal.cachedFilePath + 0x40;
    strcpy(charCursor, path);
    while (*charCursor != '.' && *charCursor != 0) {
        charCursor++;
    }
    if (*charCursor == '.') {
        *charCursor = 0;
    }
    sprintf(eventFileName, "evt\\%s", path);
    charCursor = eventFileName;
    while (*charCursor != '.' && *charCursor != 0) {
        charCursor++;
    }
    if (*charCursor == '.') {
        *charCursor = 0;
    }
    func_0030E3D0(eventFileName, ".xev");
    pathLength = strlen(eventFileName);
    gXglCdBlockStreamLocal.unk_3F1C = 1;
    gXglCdBlockStreamLocal.readOffset = 0;
    gXglCdBlockStreamLocal.unk_3F20 = 0;
    gXglCdBlockStreamLocal.matchValue = 0;
    gXglCdBlockStreamLocal.readIndex = 0;
    gXglCdBlockStreamLocal.entryIndex = 0;
    ((void *(*) ()) memset)(gXglCdBlockStreamLocal.cachedFilePath, 0, 0x40);
    strcpy((char *) gXglCdBlockStreamLocal.cachedFilePath, (char *) gXglCdBlockStreamLocal.pathBuffer);
    gXglCdBlockStreamLocal.cachedFilePath[pathLength - 1] = 'p';
    gXglCdBlockStreamLocal.cachedFileSize = -1;
    gXglCdBlockStreamLocal.unk_415C = -1;
}

extern unsigned char D_003E8B10[];

u8 *xenoActorEventGetBufferA(void) {
    return D_003E8B10;
}

extern unsigned char gXenoActorBufferB[];

u8 *xenoActorEventGetBufferB(void) {
    return gXenoActorBufferB;
}

extern unsigned char gXenoActorBufferC[];

u8 *xenoActorEventGetBufferC(void) {
    return gXenoActorBufferC;
}

typedef struct XenoActorSlotEntry {
    u8 unmodeled_00[0x50C];
    s32 activeFlag;
} XenoActorSlotEntry;

typedef struct XglCdBlockStreamOther {
    u8 unk_0000[0x1800];
    s32 unk_1800;
    u8 unk_1804[0xC];
    u8 unk_1810[0x40];
    u8 cachedFilePath[0x80];
    void *unk_18D0;
    XenoActorTable *currentActorRecord;
    XenoActorSlotEntry *entries[0x98D];
    s32 entryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    s32 unk_3F24;
    u8 *unk_3F28;
    u8 *unk_3F2C;
    s32 unk_3F30;
    s32 unk_3F34;
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 unk_3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStreamOther;

#define gXglCdBlockStreamOther (*(XglCdBlockStreamOther *) gXglCdBlockStreamStorage)

s32 xglCdServiceIfPending(void);
void xglCdReadFilePart();
void xglCdRingReadNoNotify();

/* When a pending prefetch is flagged, reads 0x1800 bytes into the stream buffer and points the two buffer cursors at it. */
void xenoActorEventPrefetchBlock(void) {
    if (gXglCdBlockStreamOther.unk_1800 != 0) {
        while (xglCdServiceIfPending() != 0) {
        }
        xglCdReadFilePart(gXglCdBlockStreamOther.unk_1810, &gXglCdBlockStreamOther, 0,
                          xglCdRingReadNoNotify, 0, 0x1800);
        while (xglCdServiceIfPending() != 0) {
        }
        gXglCdBlockStreamOther.unk_3F28 = (u8 *) &gXglCdBlockStreamOther + 0x1000;
        gXglCdBlockStreamOther.unk_3F2C = (u8 *) &gXglCdBlockStreamOther + 0x1004;
        gXglCdBlockStreamOther.unk_18D0 = &gXglCdBlockStreamOther;
    }
}

s32 xglCdGetFileSize();

s32 xglCdGetCachedFileSize(void)
{
    XglCdBlockStream *blockStream = &gXglCdBlockStream;
    s32 cachedSize;
    cachedSize = blockStream->cachedFileSize;
    if (cachedSize < 0)
    {
        cachedSize = xglCdGetFileSize(blockStream->cachedFilePath);
        blockStream->cachedFileSize = cachedSize;
    }
    return cachedSize;
}

s32 xglCdGetCachedFileSize();

s32 xglCdGetSectorAlignedFileSize(void) {
    return (xglCdGetCachedFileSize() + 0x7FF) & ~0x7FF;
}

extern s32 D_003EB45C[];

s32 xenoActorEventGetStatus45C(void) {
    return D_003EB45C[0];
}

s32 xglCdDriveStatusLowBitSet(void);
s32 xglCdDriveStatusFifthBitSet(void);
s32 itemSourceResolveIndex();
s32 xenoActorLoadEventItemList();
extern XenoActorRange *gXenoCurrentActorRange[];

/* Runs the drive-status reader for request: itemSourceResolveIndex when only the low status bit is set, xenoActorLoadEventItemList otherwise. */
s32 xenoActorEventReadDispatch(s32 request) {
    if (xglCdDriveStatusFifthBitSet() != 0) {
        request = xenoActorLoadEventItemList(request, (u8 *) gXenoCurrentActorRange[0] + 0x50);
    } else if (xglCdDriveStatusLowBitSet() != 0) {
        request = itemSourceResolveIndex(request, (u8 *) gXenoCurrentActorRange[0] + 0x50);
    } else {
        request = xenoActorLoadEventItemList(request, (u8 *) gXenoCurrentActorRange[0] + 0x50);
    }
    return request;
}

typedef struct XenoActorTableAlt {
    u8 unk_00[0xC];
    s32 dataSize;
    u8 unk_10[0x40];
    u32 actorCount;
} XenoActorTableAlt;

typedef struct XglCdBlockStreamSecond {
    u8 unk_0000[0x1800];
    XenoActorTableAlt *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x84];
    XenoActorTableAlt *currentActorRecord;
    u8 unk_18D8[0x2628];
    s32 readIndex;
    s32 entryIndex;
    u8 unk_3F08[0x10];
    s32 readOffset;
    u8 unk_3F1C[0x4];
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStreamSecond;

#define gXglCdBlockStreamSecond (*(XglCdBlockStreamSecond *) gXglCdBlockStreamStorage)

/* Reads the next 0x800-byte chunk of the stream, then the record of the current ring entry, and advances readOffset past it. */
void xenoActorEventReadNextChunk(void) {
    XenoActorTableAlt *currentEntry;
    XenoActorTableAlt *readBuffer;
    s32 dataSize;

    gXglCdBlockStreamSecond.readOffset += 0x1800;
    xglCdReadFilePart(gXglCdBlockStreamSecond.pathBuffer,
                      gXglCdBlockStreamSecond.ringEntry[gXglCdBlockStreamSecond.readIndex], 0,
                      xglCdRingReadNoNotify, gXglCdBlockStreamSecond.readOffset, 0x800);
    while (xglCdServiceIfPending() != 0) {
    }
    currentEntry = gXglCdBlockStreamSecond.ringEntry[gXglCdBlockStreamSecond.entryIndex];
    readBuffer = gXglCdBlockStreamSecond.ringEntry[gXglCdBlockStreamSecond.readIndex];
    dataSize = currentEntry->dataSize;
    gXglCdBlockStreamSecond.currentActorRecord = currentEntry;
    xglCdReadFilePart(gXglCdBlockStreamSecond.pathBuffer, readBuffer, 0, xglCdRingReadNoNotify,
                      gXglCdBlockStreamSecond.readOffset, dataSize + 0xB0);
    gXglCdBlockStreamSecond.readOffset += gXglCdBlockStreamSecond.currentActorRecord->dataSize + 0xB0;
    while (xglCdServiceIfPending() != 0) {
    }
}

typedef struct XenoActorTableLocal {
    u8 unk_00[0x4];
    u32 unk_04;
    u32 unk_08;
    s32 dataSize;
    s32 unk_10;
    s32 unk_14;
    s32 entryCount;
    u8 unk_1C[0x34];
    u32 actorCount;
} XenoActorTableLocal;

typedef struct XenoActorCount {
    s32 count;
} XenoActorCount;

typedef struct XenoActorEntryView {
    u8 unk_00[0x14];
    s32 actorPayloadSize;
    u8 unk_18[0x8];
    u32 slotIndex;
} XenoActorEntryView;

typedef struct XenoActorSlotView {
    u8 unk_000[0x90];
    s32 flags;
    u8 unk_094[0x5AC];
    XenoActorEntryView *entry640;
    u8 unk_644[0x4];
    s32 slotKey648;
} XenoActorSlotView;

typedef struct XglCdBlockStreamThird {
    u8 unk_0000[0x1800];
    XenoActorTableLocal *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x80];
    XenoActorTableLocal *pendingRecord;
    XenoActorTableLocal *currentActorRecord;
    XenoActorSlotView *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 matchValue;
    XenoActorCount *countList;
    s32 *valueList;
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 flags3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
    s32 unk_415C;
} XglCdBlockStreamThird;

#define gXglCdBlockStreamThird (*(XglCdBlockStreamThird *) gXglCdBlockStreamStorage)

u8 *tiSlotTableEnd(void *base, s32 unused_arg1, u32 slotSteps);
void xglEulerFromRotation(Vector4 *euler, Vector4 *rotation);

/* Sets the camera travel position, rotation and field of view from each record of the current actor table (fixed clip range). */
/* Sets the camera travel position, rotation and field of view from each record of the current actor table (fixed clip range). */
void xenoActorEventApplyCamera(XglCamera *camera) {
    Vector4 rotation;
    float values[6];
    u8 *recordCursor;
    u32 i;
    u8 *cameraRecord;

    for (i = 0; i < gXglCdBlockStreamThird.currentActorRecord->unk_10; i++) {
        cameraRecord =
            tiSlotTableEnd(gXglCdBlockStreamThird.currentActorRecord, gXglCdBlockStreamThird.unk_3F20, i);
        if (cameraRecord == 0) {
            assertFailed("pElemCamera", "XenoActorEvent.euc.c", 1048);
        }
        recordCursor = cameraRecord + 32;
        func_0029BEE0(values, &recordCursor, gXglCdBlockStreamThird.unk_3F20, 6);
        camera->travel.position.x = values[0];
        camera->travel.position.y = values[1];
        camera->travel.position.z = values[2];
        rotation.x = values[3];
        rotation.y = values[4];
        rotation.z = values[5];
        xglEulerFromRotation(&camera->travel.rotation, &rotation);
        func_0029BEE0(values, &recordCursor, gXglCdBlockStreamThird.unk_3F20, 1);
        camera->screen.clipRange.vector.x = 0.08f;
        camera->screen.clipRange.vector.y = 15000.0f;
        camera->travel.fov = values[0];
    }
}

typedef struct XenoActorTableOther {
    u8 unk_00[0x4];
    u32 unk_04;
    u32 unk_08;
    s32 dataSize;
    s32 unk_10;
    s32 unk_14;
    s32 entryCount;
    u8 unk_1C[0x24];
    s32 unk_40;
    u8 unk_44[0xC];
    u32 actorCount;
} XenoActorTableOther;

typedef struct XenoActorEntryAlt {
    u8 unk_00[0x14];
    s32 actorPayloadSize;
    u8 unk_18[0x8];
    u32 slotIndex;
} XenoActorEntryAlt;

typedef struct XenoActorSlotAlt {
    u8 unk_000[0x90];
    s32 flags;
    u8 unk_094[0x5AC];
    XenoActorEntryAlt *entry640;
    u8 unk_644[0x4];
    s32 slotKey648;
} XenoActorSlotAlt;

typedef struct XglCdBlockStreamFourth {
    u8 unk_0000[0x1800];
    XenoActorTableOther *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x80];
    XenoActorTableOther *pendingRecord;
    XenoActorTableOther *currentActorRecord;
    XenoActorSlotAlt *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 matchValue;
    XenoActorCount *countList;
    s32 *valueList;
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 flags3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
    s32 unk_415C;
} XglCdBlockStreamFourth;

#define gXglCdBlockStreamFourth (*(XglCdBlockStreamFourth *) gXglCdBlockStreamStorage)

u8 *func_0024E3F0(XenoActorTableOther *table, u32 index);
void gifStateSetTimings(s32 a, s32 b, s32 c);

extern const char D_00550CB8[];

/* Applies the 36-float camera records of the current actor table: clip range, two camera flags and the gifState timings. */
void xenoActorEventApplyCameraTimings(XglCamera *camera) {
    float values[36];
    u8 *recordCursor;
    u8 *cameraRecord;
    u32 i;

    for (i = 0; i < gXglCdBlockStreamFourth.currentActorRecord->unk_40; i++) {
        cameraRecord = func_0024E3F0(gXglCdBlockStreamFourth.currentActorRecord, i);
        if (cameraRecord == 0) {
            assertFailed(D_00550CB8, "XenoActorEvent.euc.c", 1088);
        }
        recordCursor = cameraRecord + 96;
        func_0029BEE0(values, &recordCursor, gXglCdBlockStreamFourth.unk_3F20, 36);
        camera->screen.clipRange.vector.x = values[0];
        camera->screen.clipRange.vector.y = values[1];
        if (0.9999899864f <= values[2]) {
            gXglCdBlockStreamFourth.flags3F50 |= 8;
        }
        if (values[3] < 9.999999747e-06f) {
            gXglCdBlockStreamFourth.flags3F50 |= 0x10;
        }
        gifStateSetTimings((s32) values[4], (s32) (values[5] * 128.0f), (s32) values[6]);
    }
}

typedef struct XenoActorTableSecond {
    u8 unk_00[0x4];
    u32 unk_04;
    u32 unk_08;
    s32 dataSize;
    s32 unk_10;
    s32 unk_14;
    s32 entryCount;
    u8 unk_1C[0x4];
    s32 unk_20;
    u8 unk_24[0x1C];
    s32 unk_40;
    u8 unk_44[0xC];
    u32 actorCount;
} XenoActorTableSecond;

typedef struct XenoActorEntryLocal {
    u8 unk_00[0x14];
    s32 actorPayloadSize;
    u8 unk_18[0x8];
    u32 slotIndex;
} XenoActorEntryLocal;

typedef struct XenoActorSlotLocal {
    u8 unk_000[0x90];
    s32 flags;
    u8 unk_094[0x5AC];
    XenoActorEntryLocal *entry640;
    u8 unk_644[0x4];
    s32 slotKey648;
} XenoActorSlotLocal;

typedef struct XglCdBlockStreamFifth {
    u8 unk_0000[0x1800];
    XenoActorTableSecond *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x80];
    XenoActorTableSecond *pendingRecord;
    XenoActorTableSecond *currentActorRecord;
    XenoActorSlotLocal *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 matchValue;
    XenoActorCount *countList;
    s32 *valueList;
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 flags3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
    s32 unk_415C;
} XglCdBlockStreamFifth;

#define gXglCdBlockStreamFifth (*(XglCdBlockStreamFifth *) gXglCdBlockStreamStorage)

u8 *xenoActorItemAt(XenoActorTableSecond *table, u32 index);
void func_00292A70(u8 *entry, s32 unk_3F20);
void func_002923C8(u32 index, u8 *entry, u8 *field, s32 unk_3F20);
void xglLightPrepare(u8 *light);
s32 pxyModelRenderStateLoadLight(u8 *light);
void xenoActorLightFlagsClear(void);
s32 xenoActorLightFlagIsSet(void);
u8 *xenoActorLightTableGet(void);

/* Clears the light flags, prepares the light table and loads every light of the current actor record (up to 0x21 per record); returns the table's render-state load result. */
s32 xenoActorEventSetupLights(void) {
    u8 *lightRecord;
    u32 i;
    s32 lightIndex;

    xenoActorLightFlagsClear();
    gXglCdBlockStreamFifth.flags3F50 &= ~1;
    xglLightPrepare(xenoActorLightTableGet());
    for (i = 0; i < gXglCdBlockStreamFifth.currentActorRecord->unk_20; i++) {
        lightRecord = xenoActorItemAt(gXglCdBlockStreamFifth.currentActorRecord, i);
        if (lightRecord == 0) {
            assertFailed("pLight", "XenoActorEvent.euc.c", 1124);
        }
        func_00292A70(lightRecord, gXglCdBlockStreamFifth.unk_3F20);
        for (lightIndex = 0; lightIndex < *(u32 *) (lightRecord + 0x90) && lightIndex < 0x21; lightIndex++) {
            func_002923C8(lightIndex, lightRecord, lightRecord + ((u32 *) (lightRecord + 0x94))[lightIndex], gXglCdBlockStreamFifth.unk_3F20);
        }
    }
    if (xenoActorLightFlagIsSet() != 0) {
        gXglCdBlockStreamFifth.flags3F50 |= 1;
    }
    return pxyModelRenderStateLoadLight(xenoActorLightTableGet());
}

typedef struct XenoActorSlotEntryView {
    u8 unmodeled_00[0x90];
    s32 flags;
    u8 unmodeled_94[0x68];
    s32 unk_FC;
    u8 unmodeled_100[0x40C];
    s32 activeFlag;
    u8 unmodeled_510[0x130];
    s32 unk_640;
    s32 unk_644;
} XenoActorSlotEntryView;

typedef struct XglCdBlockStreamSixth {
    u8 unk_0000[0x1800];
    s32 unk_1800;
    u8 unk_1804[0xC];
    u8 unk_1810[0x40];
    u8 cachedFilePath[0x80];
    void *unk_18D0;
    XenoActorTable *currentActorRecord;
    XenoActorSlotEntryView *entries[0x98D];
    s32 entryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    s32 unk_3F24;
    u8 *unk_3F28;
    u8 *unk_3F2C;
    s32 unk_3F30;
    s32 unk_3F34;
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 unk_3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStreamSixth;

#define gXglCdBlockStreamSixth (*(XglCdBlockStreamSixth *) gXglCdBlockStreamStorage)

/* Clears the slot flags and fields (unk_FC, unk_640, unk_644, flags bits) of every slot entry of the block. */
void xenoActorSlotEntriesReset(void) {
    u32 i;
    XenoActorSlotEntryView *slotEntry;
    s32 flags;

    i = 0;
    if (gXglCdBlockStreamSixth.entryCount != 0) {
        do {
            slotEntry = gXglCdBlockStreamSixth.entries[i];
            i++;
            flags = slotEntry->flags;
            slotEntry->unk_FC = 0;
            flags = flags | 4;
            slotEntry->unk_640 = 0;
            flags = flags & ~8;
            slotEntry->unk_644 = 0;
            flags = flags & ~0x10;
            flags = flags & ~0x80000;
            flags = flags & ~0x100000;
            flags = flags & ~0x10000;
            flags = flags & ~1;
            slotEntry->flags = flags;
        } while (i < gXglCdBlockStreamSixth.entryCount);
    }
}

typedef struct XglCdBlockStream_24FCB0 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTableView *currentActorRecord;
    XenoActorSlot *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStream_24FCB0;

#define gXglCdBlockStream_24FCB0 (*(XglCdBlockStream_24FCB0 *) gXglCdBlockStreamStorage)

u8 *func_0024E460(XenoActorTableView *table, s32 unk_3F20, u32 index);

/* For each actor record whose slot index is valid: loads the 9 slot values, rebuilds the slot's matrices, sets its flags and, when present, loads the 15-float extra block into unk_94. */
/* For each actor record whose slot index is valid: loads the 9 slot values, rebuilds the slot's matrices, sets its flags and, when present, loads the 15-float extra block into unk_94. */
void xenoActorEventUpdateSlots(void) {
    float slotValues[9];
    float slotExtraValues[15];
    float slotUnk94;
    u8 *recordCursor;
    XenoActorSlot *slot;
    u8 *actorSlotRecord;
    u32 i;
    u32 flags;

    for (i = 0; i < gXglCdBlockStream_24FCB0.currentActorRecord->unk_38; i++) {
        actorSlotRecord =
            func_0024E460(gXglCdBlockStream_24FCB0.currentActorRecord, gXglCdBlockStream_24FCB0.unk_3F20, i);
        if (actorSlotRecord != 0) {
            if (*(u32 *) (actorSlotRecord + 0xC) < gXglCdBlockStream_24FCB0.slotCount) {
                slot = gXglCdBlockStream_24FCB0.slots[*(u32 *) (actorSlotRecord + 0xC)];
                recordCursor = actorSlotRecord + 0x60;
                func_0029BEE0(slotValues, &recordCursor, gXglCdBlockStream_24FCB0.unk_3F20,
                                                           9);
                flags = slot->flags;
                slot->unk_20 = slotValues[0];
                flags &= ~4;
                slot->flags = flags;
                slot->unk_24 = slotValues[1];
                slot->unk_28 = slotValues[2];
                slot->unk_50 = slotValues[3];
                slot->unk_54 = slotValues[4];
                slot->unk_58 = slotValues[5];
                slot->unk_80 = slotValues[6];
                slot->unk_84 = slotValues[7];
                slot->unk_88.f = slotValues[8];
                xenoActorMatrixTableStore(slot, *(u32 *) (actorSlotRecord + 0xC));
                xenoActorSubPackApplyPlacements(
                    slot, actorSlotRecord, gXglCdBlockStream_24FCB0.unk_3F20);
                slot->flags |= 0x18;
                slot->flags &= ~0x1000;
                slot->flags &= ~0x10000;
                slot->entry644 = actorSlotRecord;
                if (*(s32 *) (actorSlotRecord + 0x1C) != 0) {
                    recordCursor = actorSlotRecord + *(s32 *) (actorSlotRecord + 0x20);
                    func_0029BEE0(slotExtraValues, &recordCursor,
                                                               gXglCdBlockStream_24FCB0.unk_3F20, 15);
                    slotUnk94 = slotExtraValues[0];
                    slot->unk_94 = slotUnk94;
                    if (slotUnk94 < 0.999f) {
                        slot->flags |= 0x10000;
                    }
                    if (0.99999f < slotExtraValues[1]) {
                        slot->flags |= 0x1000;
                    }
                }
            }
        }
    }
}

typedef struct XenoActorTableThird {
    u8 unk_00[0x4];
    u32 unk_04;
    u32 unk_08;
    s32 dataSize;
    s32 unk_10;
    s32 unk_14;
    s32 entryCount;
    u8 unk_1C[0x34];
    u32 actorCount;
} XenoActorTableThird;

typedef struct XenoActorEntryOther {
    u8 unk_00[0x14];
    s32 actorPayloadSize;
    u8 unk_18[0x8];
    u32 slotIndex;
} XenoActorEntryOther;

typedef struct XenoActorSlotOther {
    u8 unk_000[0x90];
    s32 flags;
    u8 unk_094[0x5AC];
    XenoActorEntryOther *entry640;
    u8 unk_644[0x4];
    s32 slotKey648;
} XenoActorSlotOther;

typedef struct XglCdBlockStream_24FE78 {
    u8 unk_0000[0x1800];
    XenoActorTableThird *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x80];
    XenoActorTableThird *pendingRecord;
    XenoActorTableThird *currentActorRecord;
    XenoActorSlotOther *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 matchValue;
    XenoActorCount *countList;
    s32 *valueList;
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 flags3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
    s32 unk_415C;
} XglCdBlockStream_24FE78;

#define gXglCdBlockStream_24FE78 (*(XglCdBlockStream_24FE78 *) gXglCdBlockStreamStorage)

XenoActorEntryOther *func_0024E4A8(XenoActorTableThird *table, s32 value, u32 index);

/* Links every actor-table entry with a valid slot index to its slot: slot->entry640 and slotKey648 are set and the slot flags get bits 0x18. */
void xenoActorEventLinkSlots(void) {
    u32 i;
    XenoActorEntryOther *actorTableEntry;
    XenoActorSlotOther *slot;
    s32 flags;
    s32 slotKey;

    for (i = 0; i < gXglCdBlockStream_24FE78.currentActorRecord->entryCount; i++) {
        if (gXglCdBlockStream_24FE78.currentActorRecord == 0) {
            assertFailed("sXevDesc.pCluster", "XenoActorEvent.euc.c", 1266);
        }
        actorTableEntry =
            func_0024E4A8(gXglCdBlockStream_24FE78.currentActorRecord, gXglCdBlockStream_24FE78.unk_3F20, i);
        if (actorTableEntry != 0) {
            if (actorTableEntry->slotIndex < gXglCdBlockStream_24FE78.slotCount) {
                slot = gXglCdBlockStream_24FE78.slots[actorTableEntry->slotIndex];
                flags = slot->flags;
                slot->entry640 = actorTableEntry;
                flags &= ~4;
                slot->flags = flags;
                flags |= 0x18;
                slotKey = gXglCdBlockStream_24FE78.unk_3F20;
                slot->flags = flags;
                slot->slotKey648 = slotKey;
            }
        }
    }
}

typedef struct XenoActorSlotSecond {
    u8 unk_000[0x640];
    s32 slotField640;
} XenoActorSlotSecond;

typedef struct XglCdBlockStream_24FF78 {
    u8 unk_0000[0x1800];
    XenoActorTableAlt *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x84];
    XenoActorTableAlt *currentActorRecord;
    XenoActorSlotSecond *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    u8 unk_3F08[0x4];
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    u8 unk_3F1C[0x4];
    s32 unk_3F20;
    s32 matchValue;
    XenoActorCount *countList;
    s32 *valueList;
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStream_24FF78;

#define gXglCdBlockStream_24FF78 (*(XglCdBlockStream_24FF78 *) gXglCdBlockStreamStorage)

void xenoActorEventResetEffects(void);
void XenoActorSetFirstWord(XenoActorSlotSecond *slot, s32 mode);

/* For each value-list entry equal to matchValue: runs xenoActorEventResetEffects, then sends mode 30 to every slot with slotField640 set. */
void xenoActorEventNotifySlots(void) {
    s32 i;
    s32 j;

    for (i = 0; i < gXglCdBlockStream_24FF78.countList->count; i++) {
        if (gXglCdBlockStream_24FF78.valueList[i] == gXglCdBlockStream_24FF78.matchValue) {
            xenoActorEventResetEffects();
            for (j = 0; j < gXglCdBlockStream_24FF78.slotCount; j++) {
                if (gXglCdBlockStream_24FF78.slots[j]->slotField640 != 0) {
                    XenoActorSetFirstWord(gXglCdBlockStream_24FF78.slots[j], 30);
                }
            }
        }
    }
}

typedef struct XglCdBlockStream_250060 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    XenoActorSlotEntry *entries[0x98D];
    s32 entryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    s32 unk_3F24;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    s32 unk_3F34;
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 unk_3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_250060;

#define gXglCdBlockStream_250060 (*(XglCdBlockStream_250060 *) gXglCdBlockStreamStorage)

/* Advances the two frame counters (unk_3F20, unk_3F24; both jump by 6 when flag 4 is set) unless mode is 1; every 30 ticks latches unk_3F30 into unk_3F34. */
void xenoActorEventAdvanceFrame(s32 mode) {
    s32 savedCounter;
    s32 savedTick;

    if (mode != 1) {
        savedCounter = gXglCdBlockStream_250060.unk_3F20;
        savedTick = gXglCdBlockStream_250060.unk_3F24;
        gXglCdBlockStream_250060.unk_3F24 = savedTick + 1;
        gXglCdBlockStream_250060.unk_3F20 = savedCounter + 1;
        if ((gXglCdBlockStream_250060.unk_3F50 & 4) != 0) {
            gXglCdBlockStream_250060.unk_3F20 = savedCounter + 6;
            gXglCdBlockStream_250060.unk_3F24 = savedTick + 6;
        }
    }
    if ((u32) gXglCdBlockStream_250060.unk_3F24 % 30 == 0) {
        gXglCdBlockStream_250060.unk_3F34 = gXglCdBlockStream_250060.unk_3F30;
        gXglCdBlockStream_250060.unk_3F30 = 0;
    }
}

typedef struct XenoActorTableFourth {
    u8 unk_00[0x4];
    u32 unk_04;
    u32 unk_08;
    s32 dataSize;
    s32 unk_10;
    s32 unk_14;
    s32 entryCount;
    u8 unk_1C[0x34];
    u32 actorCount;
} XenoActorTableFourth;

typedef struct XenoActorEntrySecond {
    u8 unk_00[0x14];
    s32 actorPayloadSize;
    u8 unk_18[0x8];
    u32 slotIndex;
} XenoActorEntrySecond;

typedef struct XenoActorSlotThird {
    u8 unk_000[0x90];
    s32 flags;
    u8 unk_094[0x5AC];
    XenoActorEntrySecond *entry640;
    u8 unk_644[0x4];
    s32 slotKey648;
} XenoActorSlotThird;

typedef struct XglCdBlockStream_2500E0 {
    u8 unk_0000[0x1800];
    XenoActorTableFourth *ringEntry[4];
    u8 pathBuffer[0x40];
    u8 cachedFilePath[0x80];
    XenoActorTableFourth *pendingRecord;
    XenoActorTableFourth *currentActorRecord;
    XenoActorSlotThird *slots[0x98A];
    s32 readIndex;
    s32 entryIndex;
    s32 unk_3F08;
    u32 slotCount;
    u8 unk_3F10[0x8];
    s32 readOffset;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 matchValue;
    XenoActorCount *countList;
    s32 *valueList;
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 flags3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_2500E0;

#define gXglCdBlockStream_2500E0 (*(XglCdBlockStream_2500E0 *) gXglCdBlockStreamStorage)

s32 xglCdGetServicePending(void);
void xglCdServiceSlotRead(void);

/* When the current record's span is used up, moves the ring to its next entry, waits for the reads in flight and updates matchValue; sets flag 2 once matchValue passes the pending record. */
void xenoActorEventAdvanceRing(void) {
    XenoActorTableFourth *currentRecord;
    XenoActorTableFourth *pendingRecord;
    XenoActorTableFourth *nextRecord;
    u32 recordSpan;
    s32 consumedInRecord;
    u32 nextEntryIndex;
    s32 nextMatchValue;

    currentRecord = gXglCdBlockStream_2500E0.currentActorRecord;
    consumedInRecord = gXglCdBlockStream_2500E0.unk_3F20;
    recordSpan = currentRecord->unk_08 - currentRecord->unk_04;
    if (recordSpan < (u32) consumedInRecord) {
        gXglCdBlockStream_2500E0.unk_3F1C++;
        nextMatchValue = gXglCdBlockStream_2500E0.matchValue + (currentRecord->unk_08 - currentRecord->unk_04 - consumedInRecord) + 1;
        nextEntryIndex = gXglCdBlockStream_2500E0.entryIndex + 1;
        gXglCdBlockStream_2500E0.unk_3F20 = 0;
        gXglCdBlockStream_2500E0.entryIndex = nextEntryIndex;
        gXglCdBlockStream_2500E0.matchValue = nextMatchValue;
        if (nextEntryIndex >= 4) {
            gXglCdBlockStream_2500E0.entryIndex = 0;
        }
        if (xglCdGetServicePending() != 0) {
            if (gXglCdBlockStream_2500E0.entryIndex == gXglCdBlockStream_2500E0.readIndex) {
                do {
                } while (xglCdServiceIfPending() != 0);
                if (gXglCdBlockStream_2500E0.unk_3F08 != 0) {
                    xglCdServiceSlotRead();
                    do {
                    } while (xglCdServiceIfPending() != 0);
                }
            }
        }
        pendingRecord = gXglCdBlockStream_2500E0.pendingRecord;
        nextRecord = gXglCdBlockStream_2500E0.ringEntry[gXglCdBlockStream_2500E0.entryIndex];
        gXglCdBlockStream_2500E0.currentActorRecord = nextRecord;
        if (!((u32) gXglCdBlockStream_2500E0.matchValue < (u32) (pendingRecord->unk_10 - pendingRecord->dataSize - 1))) {
            gXglCdBlockStream_2500E0.flags3F50 |= 2;
        }
    }
}

typedef struct XglCdBlockStream_250210 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    u8 unk_18D8[0x2648];
    s32 unk_3F20;
    s32 unk_3F24;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x10];
    s32 cdDriveStatus;
    s32 cdDriveReadyStatus;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_250210;

#define gXglCdBlockStream_250210 (*(XglCdBlockStream_250210 *) gXglCdBlockStreamStorage)

s32 func_0018F6E0(s32 soundSlotNumber);
s32 soundSlotStatus(s32 soundSlotNumber);

/* Polls the CD drive ready bits against sound slot 1: a drift of more than 2 from unk_3F24 holds the read back; otherwise advances the event (xenoActorEventAdvanceFrame, xenoActorEventAdvanceRing) and sets ready bit 0x100. */
void xenoActorEventPollCdDrive(s32 eventContext) {
    s32 mayProceed;
    s32 soundSlotValue;
    s32 cdReadyBits;
    s32 slotDrift;

    mayProceed = 1;
    soundSlotValue = func_0018F6E0(1);
    cdReadyBits = gXglCdBlockStream_250210.cdDriveReadyStatus;
    if ((cdReadyBits & 4) != 0) {
        gXglCdBlockStream_250210.cdDriveStatus &= ~4;
    }
    gXglCdBlockStream_250210.cdDriveReadyStatus = cdReadyBits & ~0xC0;
    if ((gXglCdBlockStream_250210.cdDriveStatus & 4) != 0) {
        if (soundSlotStatus(1) != 5) {
            slotDrift = soundSlotValue - gXglCdBlockStream_250210.unk_3F24;
            if (slotDrift != 0) {
                if (slotDrift > 0) {
                    if (slotDrift < 5) {
                        gXglCdBlockStream_250210.cdDriveReadyStatus |= 0x40;
                    } else {
                        gXglCdBlockStream_250210.cdDriveReadyStatus |= 0x40;
                    }
                } else if (slotDrift < 0) {
                    if (slotDrift < -2) {
                        gXglCdBlockStream_250210.cdDriveReadyStatus |= 0x80;
                        mayProceed = 0;
                    }
                }
            }
        }
    }
    if (mayProceed != 0) {
        xenoActorEventAdvanceFrame(eventContext);
        ((void (*)(s32 eventContext)) xenoActorEventAdvanceRing)(eventContext);
        gXglCdBlockStream_250210.cdDriveReadyStatus |= 0x100;
    }
}

typedef struct XglCdBlockStream_250330 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    u8 unk_18D8[0x2634];
    s32 unk_3F0C;
    u8 unk_3F10[0x10];
    s32 unk_3F20;
    s32 status224;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x10];
    s32 cdDriveStatus;
    s32 cdDriveReadyStatus;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_250330;

#define gXglCdBlockStream_250330 (*(XglCdBlockStream_250330 *) gXglCdBlockStreamStorage)

void xglStudioMove(void);
void pxySetTexMapDropAll(void);
void pxyEnableFlag8(void);
void xglCdPollRead(void);
void xenoActorEventSpawnEffects(s32 mode);
void xenoActorEventEmptyHookB(void);

/* Per-frame update: when unk_3F0C is set, runs the camera, light, slot, entry, sync and CD steps in order, then polls the drive; finally sets the texture and enable flags from the ready bits. */
void xenoActorEventFrameUpdate(s32 eventContext) {
    struct XglCamera *camera;

    camera = xglStudioSelectGetActiveCamera(0);
    if (gXglCdBlockStream_250330.unk_3F0C == 0) {
        return;
    }
    gXglCdBlockStream_250330.cdDriveReadyStatus = 0;
    ((void (*)(struct XglCamera *camera, s32 eventContext)) xenoActorEventApplyCamera)(camera, eventContext);
    ((void (*)(struct XglCamera *camera, s32 eventContext)) xenoActorEventApplyCameraTimings)(camera, eventContext);
    xglStudioMove();
    ((void (*)(s32 eventContext)) xenoActorEventSetupLights)(eventContext);
    xenoActorSlotEntriesReset();
    ((void (*)(s32 eventContext)) xenoActorEventUpdateSlots)(eventContext);
    ((void (*)(s32 eventContext)) xenoActorEventLinkSlots)(eventContext);
    xenoActorEventApplyEntries();
    xenoActorEventReloadEntries();
    ((void (*)(s32 eventContext)) xenoActorEventNotifySlots)(eventContext);
    xenoActorEventSpawnEffects(eventContext);
    xenoActorEventSetSyncMode();
    xenoActorEventSendSlotPackets(eventContext);
    xglCdServiceSlotRead();
    xglCdPollRead();
    xenoActorEventEmptyHookB();
    xenoActorEventPollCdDrive(eventContext);
    if ((gXglCdBlockStream_250330.cdDriveReadyStatus & 8) != 0) {
        pxySetTexMapDropAll();
    }
    if ((gXglCdBlockStream_250330.cdDriveReadyStatus & 0x10) == 0) {
        pxyEnableFlag8();
    }
}

typedef struct XglCdBlockStream_250440 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    XenoActorSlotEntry *entries[0x98D];
    s32 entryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStream_250440;

#define gXglCdBlockStream_250440 (*(XglCdBlockStream_250440 *) gXglCdBlockStreamStorage)

/* Counts the slot entries whose activeFlag is set. */
s32 xenoActorEventCountActive(void) {
    XglCdBlockStream_250440 *blockStream = &gXglCdBlockStream_250440;
    XenoActorSlotEntry **entryList;
    XenoActorSlotEntry *slotEntry;
    u32 i;
    s32 activeCount;

    i = 0;
    activeCount = 0;
    if (blockStream->entryCount != 0) {
        entryList = blockStream->entries;
        do {
            slotEntry = entryList[i];
            i++;
            if (slotEntry->activeFlag != 0) {
                activeCount++;
            }
        } while (i < blockStream->entryCount);
    }
    return activeCount;
}

typedef struct XglCdBlockStream_250488 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    XenoActorSlotEntry *entries[0x98D];
    s32 entryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    u8 unk_3F24[0xC];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x21C];
    s32 cachedFileSize;
} XglCdBlockStream_250488;

#define gXglCdBlockStream_250488 (*(XglCdBlockStream_250488 *) gXglCdBlockStreamStorage)

/* Counts the slot entries whose activeFlag is clear. */
s32 xenoActorEventCountInactive(void) {
    XglCdBlockStream_250488 *blockStream = &gXglCdBlockStream_250488;
    XenoActorSlotEntry **entryList;
    XenoActorSlotEntry *slotEntry;
    u32 i;
    s32 activeCount;

    i = 0;
    activeCount = 0;
    if (blockStream->entryCount != 0) {
        entryList = blockStream->entries;
        do {
            slotEntry = entryList[i];
            i++;
            if (slotEntry->activeFlag == 0) {
                activeCount++;
            }
        } while (i < blockStream->entryCount);
    }
    return activeCount;
}

extern s32 D_003EB210[];

s32 xenoActorEventGetStatus210(void) {
    return D_003EB210[0];
}

extern XenoActorRecord *gXenoCurrentActorRecord[];

s32 xenoActorEventGetCurrentField38(void) {
    if (!(gXenoCurrentActorRecord[0] != 0)) {
        return 0;
    }
    return gXenoCurrentActorRecord[0]->unk_38;
}

s32 xenoActorEventGetCurrentField18(void) {
    if (!(gXenoCurrentActorRecord[0] != 0)) {
        return 0;
    }
    return gXenoCurrentActorRecord[0]->unk_18;
}

s32 xenoActorEventGetCurrentField30(void) {
    if (!(gXenoCurrentActorRecord[0] != 0)) {
        return 0;
    }
    return gXenoCurrentActorRecord[0]->unk_30;
}

typedef struct XenoActorSlotEntryAlt {
    u8 unmodeled_00[0x90];
    s32 flags;
    u8 unmodeled_94[0x68];
    s32 unk_FC;
    u8 unmodeled_100[0x40C];
    s32 activeFlag;
    u8 unmodeled_510[0x130];
    s32 unk_640;
    s32 unk_644;
} XenoActorSlotEntryAlt;

typedef struct XglCdBlockStream_250540 {
    u8 unk_0000[0x1800];
    s32 unk_1800;
    u8 unk_1804[0xC];
    u8 unk_1810[0x40];
    u8 cachedFilePath[0x80];
    void *unk_18D0;
    XenoActorTable *currentActorRecord;
    XenoActorSlotEntryAlt *entries[0x98D];
    s32 entryCount;
    u8 unmodeled_3F10[0x10];
    s32 unk_3F20;
    s32 unk_3F24;
    u8 *unk_3F28;
    u8 *unk_3F2C;
    s32 unk_3F30;
    s32 unk_3F34;
    s32 unk_3F38;
    u8 unk_3F3C[0x14];
    s32 unk_3F50;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_250540;

#define gXglCdBlockStream_250540 (*(XglCdBlockStream_250540 *) gXglCdBlockStreamStorage)

/* Returns 1 + the index of the boundary interval (unk_3F2C table) that holds unk_3F24; -1 when the table is empty. */
s32 xenoActorEventFindTableInterval(void) {
    s32 i;
    u32 *boundaryTable;
    u32 position;

    if (gXglCdBlockStream_250540.entryCount == 0 || gXglCdBlockStream_250540.unk_3F28 == 0) {
        return -1;
    }
    for (i = 0; i < *(s32 *) gXglCdBlockStream_250540.unk_3F28 - 1; i++) {
        boundaryTable = (u32 *) gXglCdBlockStream_250540.unk_3F2C;
        position = gXglCdBlockStream_250540.unk_3F24;
        if (position >= boundaryTable[i] && position < boundaryTable[i + 1]) {
            return i + 1;
        }
    }
    return i + 1;
}

extern s32 gXglCdDriveReadyStatus[];
extern u8 D_004C8414[];

s32 xenoActorEventIsReadyFlagSet(void) {
    if (!(!(gXglCdDriveReadyStatus[0] & 0x80))) {
        return 0;
    }
    return D_004C8414[0] != 0;
}

extern s32 D_003EB224[];

s32 xenoActorEventGetStatus224(void) {
    return D_003EB224[0];
}

s32 xenoActorEventGetCurrentRangeLength(void) {
    if (!(gXenoCurrentActorRange[0] != 0)) {
        return 0;
    }
    return gXenoCurrentActorRange[0]->rangeEnd - gXenoCurrentActorRange[0]->rangeBegin;
}

void xenoActorEventAdvanceCounters(void)
{
    XglCdBlockStream *blockStream = &gXglCdBlockStream;
    s32 nextA;
    s32 nextB;
    nextA = blockStream->unk_3F30 + 1;
    nextB = blockStream->unk_3F38 + 1;
    blockStream->unk_3F38 = nextB;
    blockStream->unk_3F30 = nextA;
}

extern s32 D_003EB234[];

s32 xenoActorEventGetStatus234(void) {
    return D_003EB234[0];
}

extern s32 D_003EB238[];

s32 xenoActorEventGetStatus238(void) {
    return D_003EB238[0];
}

extern s32 D_003EB23C[];

s32 xenoActorEventGetStatus23C(void) {
    return D_003EB23C[0];
}

extern s32 D_003EB240[];

s32 xenoActorEventGetStatus240(void) {
    return D_003EB240[0];
}

extern s32 D_003EB244[];

s32 xenoActorEventGetStatus244(void) {
    return D_003EB244[0];
}

extern s32 D_003EB248[];

s32 xenoActorEventGetStatus248(void) {
    return D_003EB248[0];
}

extern s32 gXglCdDriveStatus[];

s32 xglCdDriveStatusLowBitSet(void) {
    return gXglCdDriveStatus[0] & 1;
}

s32 xglCdDriveStatusFifthBitSet(void) {
    return gXglCdDriveStatus[0] & 0x20;
}

void xenoActorEventEmptyHookA(void) {

}

s32 xglCdDriveReadyStatusLowBitSet(void) {
    return gXglCdDriveReadyStatus[0] & 1;
}

s32 xenoActorEventGetCurrentDataBase(void) {
    if (!(gXenoCurrentActorRecord[0] != 0)) {
        return 0;
    }
    return gXenoCurrentActorRecord[0]->actorDataAddress + 0xB0;
}

void xenoActorEventEmptyHookB(void) {

}

s32 strcmp();

s32 xenoActorEventClassifyId(s32 id) {
    if (strcmp(id, "ee200_000") == 0) {
        return 1;
    }
    return (strcmp(id, "ee200_001") != 0) ? 0 : 2;
}

void sefReleaseOwnerEffects(s32 ownerKey);
extern u8 gXenoActorEffectRecords[];

/* Releases the owner effects and the pools, then clears 64 records of 64 bytes, 144 bytes apart, at gXenoActorEffectRecords + 0x48. */
void xenoActorEventResetEffects(void) {
    s32 i;
    u8 *clearCursor;
    u8 *effectRecordBase;

    sefReleaseOwnerEffects(-1);
    slotReleaseAllPools();
    effectRecordBase = gXenoActorEffectRecords;
    clearCursor = effectRecordBase + 0x48;
    for (i = 0; i < 64; i++) {
        memset(clearCursor, 0, 64);
        clearCursor += 144;
    }
}

void xenoActorEventRunStep(void) {
    xenoActorEventResetEffects();
}

typedef struct XenoActorSlotBlock {
    u8 unk_00[0x48];
    s32 slots[16];
} XenoActorSlotBlock;

/* Stores value in the first empty slot of block (16 slots); does nothing when all are full. kept: the goto spelling decides the emitted branch order (loops do not match). */
void xenoActorSlotBlockAdd(XenoActorSlotBlock *block, s32 handleValue) {
    s32 i;
    s32 *slotCursor;

    slotCursor = block->slots;
    i = 0;
    goto check;
next:
    i++;
    if (i >= 16) {
        return;
    }
    slotCursor = (s32 *) (i * 4 + (s32) block + 72);
check:
    if (*slotCursor != 0) {
        goto next;
    }
    *slotCursor = handleValue;
}

typedef struct XenoActorEffectGroup {
    u8 unk_00[0x10];
    u8 unk_10[0x10];
    u8 unk_20[0x10];
    float unk_30;
    float unk_34;
    float unk_38;
    u8 unk_3C[0x4];
    float unk_40;
    u8 unk_44[0x4];
    s32 effectHandles[16];
} XenoActorEffectGroup;

s32 func_001EA050(s32 effectHandle);
void sefEffectHandleRelease(s32 effectHandle);
void sefLinkedObjectSetPosition(s32 effectHandle, void *group);
void sefLinkedObjectSetRotation(s32 effectHandle, void *source);
void func_001EAA98(s32 effectHandle, void *source);
void func_001EAB00(s32 effectHandle, void *vector);
void objectSetLinkedValue(s32 effectHandle, s32 value);
void sefLinkedObjectSetLinkedFloat(s32 effectHandle, float value);

/* Updates each live effect handle of the group: releases handles that no longer exist, then pushes the group's position, rotation, vectors and linked values to the rest. */
void xenoActorEffectGroupUpdate(XenoActorEffectGroup *group) {
    s32 handleIndex;
    s32 *handleSlot;
    s32 effectHandle;
    float valueSum;

    for (handleIndex = 15, handleSlot = group->effectHandles; handleIndex >= 0; handleIndex--, handleSlot++) {
        effectHandle = *handleSlot;
        if (effectHandle == 0) {
            continue;
        }
        if (func_001EA050(effectHandle) == 0) {
            sefEffectHandleRelease(effectHandle);
            *handleSlot = 0;
            continue;
        }
        sefLinkedObjectSetPosition(effectHandle, group);
        sefLinkedObjectSetRotation(effectHandle, group->unk_10);
        func_001EAA98(effectHandle, group->unk_20);
        func_001EAB00(effectHandle, &group->unk_30);
        valueSum = group->unk_30 + group->unk_34 + group->unk_38;
        if (valueSum <= 0.0001f) {
            objectSetLinkedValue(effectHandle, 0);
        } else {
            objectSetLinkedValue(effectHandle, 1);
        }
        if (xenoActorEventIsReadyFlagSet() != 0) {
            sefLinkedObjectSetLinkedFloat(effectHandle, group->unk_40 + group->unk_40);
        } else {
            sefLinkedObjectSetLinkedFloat(effectHandle, group->unk_40);
        }
    }
}

const unsigned char gEffectNameZero[8] = { 0 };
extern char *gEsdFilePrefixes[];

/* Builds the ESD path into destination: a group directory (number / 100 * 100 under the prefix of index) and the file esd/<directory><prefix><number>_<part>.esd. */
void xenoActorEventEsdPath(char *destination, s32 index, s32 number, s32 partNumber) {
    char groupDirectory[16];

    groupDirectory[0] = gEffectNameZero[0];
    memset(groupDirectory + 1, 0, 15);
    sprintf(
        groupDirectory, "%s%03d/", gEsdFilePrefixes[index], (number / 100) * 100);
    sprintf(
        destination, "esd/%s%s%03d_%03d.esd", groupDirectory, gEsdFilePrefixes[index], number, partNumber);
}

typedef struct XglCdBlockStream_250A30 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    u8 unk_18D8[0x2634];
    s32 unk_3F0C;
    u8 unk_3F10[0x10];
    s32 unk_3F20;
    s32 status224;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x8];
    s32 unk_3F44;
    u8 unk_3F48[0x4];
    s32 cdDriveStatus;
    s32 cdDriveReadyStatus;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_250A30;

#define gXglCdBlockStream_250A30 (*(XglCdBlockStream_250A30 *) gXglCdBlockStreamStorage)

s32 func_0030C108(s32);
s32 testLoadFile(s32 *loadedSize, void *buffer);
void func_001936F0(const char *file, s32 line, const char *format, ...);
s32 srsSetEffectHandleValueResult();

typedef struct XenoActorTemplateBlock {
    u8 bytes[33];
} XenoActorTemplateBlock;

const char D_00539488[40] = "host0:/home/xeno/work/_particle/";

/* Loads the effect file named by path into the scratchpad, links it to an effect handle keyed by the group and part numbers, and adds its size to unk_3F44; logs when the file is missing. */
void xenoActorEventLoadEffectFile(const char *path, s32 *loadedSize) {
    s32 sizeBefore;
    s32 groupNumber;
    s32 partNumber;
    s32 effectKey;
    s32 loadResult;
    XenoActorTemplateBlock *templateBlock;

    sizeBefore = *loadedSize;
    memset(&SEF_SCRATCHPAD(void, 0x0), 0, 64);
    strcpy(&SEF_SCRATCHPAD(char, 0x0), path);
    templateBlock = &SEF_SCRATCHPAD(XenoActorTemplateBlock, 0x0);
    ((char *) templateBlock)[5] = 0;
    groupNumber = func_0030C108(0x70000002);
    partNumber = func_0030C108(0x70000006);
    *templateBlock = *(XenoActorTemplateBlock *) D_00539488;
    xenoActorEventEsdPath(&SEF_SCRATCHPAD(char, 0x20), 2, groupNumber, partNumber);
    loadResult = testLoadFile(loadedSize, &SEF_SCRATCHPAD(void, 0x0));
    if (loadResult != 0) {
        effectKey = (groupNumber << 12) | partNumber;
        srsSetEffectHandleValueResult(loadResult, effectKey | 0x20000000, 0, (s32) templateBlock);
        gXglCdBlockStream_250A30.unk_3F44 += *loadedSize - sizeBefore;
    } else {
        func_001936F0("XenoActorEvent_Effect.euc.c", 181, "effect no file %s\n");
    }
}

typedef struct XglCdBlockStream_250BB0 {
    u8 unk_0000[0x1850];
    u8 cachedFilePath[0x84];
    XenoActorTable *currentActorRecord;
    u8 unk_18D8[0x2634];
    s32 unk_3F0C;
    u8 unk_3F10[0x10];
    s32 unk_3F20;
    s32 status224;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    u8 unk_3F34[0x4];
    s32 unk_3F38;
    u8 unk_3F3C[0x8];
    s32 unk_3F44;
    u8 unk_3F48[0x4];
    s32 cdDriveStatus;
    s32 cdDriveReadyStatus;
    u8 unk_3F54[0x204];
    s32 cachedFileSize;
} XglCdBlockStream_250BB0;

#define gXglCdBlockStream_250BB0 (*(XglCdBlockStream_250BB0 *) gXglCdBlockStreamStorage)

s32 xglCdReadFile();

typedef struct XenoActorTemplateWord {
    u8 bytes[4];
} XenoActorTemplateWord;

extern const char D_00550CD8[];

/* Reads the CD file named by path at the current end of loadedSize, registers it as an effect handle keyed by the group and part numbers and grows loadedSize by its padded size; logs when the file is missing. */
void xenoActorEventLoadEffectCd(const char *path, s32 *loadedSize) {
    s32 sizeBefore;
    s32 groupNumber;
    s32 partNumber;
    s32 fileOffset;
    s32 fileSize;
    s32 registerResult;
    XenoActorTemplateWord *templateWords;

    templateWords = &SEF_SCRATCHPAD(XenoActorTemplateWord, 0x0);
    sizeBefore = *loadedSize;
    memset(templateWords, 0, 64);
    strcpy((char *) templateWords, path);
    ((char *) templateWords)[5] = 0;
    groupNumber = func_0030C108(0x70000002);
    partNumber = func_0030C108(0x70000006);
    *templateWords = *(XenoActorTemplateWord *) D_00550CD8;
    xenoActorEventEsdPath(&SEF_SCRATCHPAD(char, 0x3), 2, groupNumber, partNumber);
    fileOffset = *loadedSize;
    fileSize = xglCdGetFileSize(templateWords);
    if (fileSize > 0) {
        *loadedSize = (*loadedSize + fileSize + 63) & ~63;
        xglCdReadFile(templateWords, fileOffset, 0, 0);
        while (xglCdServiceIfPending() != 0) {
        }
        registerResult = srsSetEffectHandleValueResult(fileOffset, (groupNumber << 12) | partNumber | 0x20000000, 0);
        if (registerResult == 0) {
            func_001936F0("XenoActorEvent_Effect.euc.c", 241, "failed regist effect\n");
        }
        gXglCdBlockStream_250BB0.unk_3F44 += *loadedSize - sizeBefore;
    } else {
        func_001936F0("XenoActorEvent_Effect.euc.c", 247, "effect no file %s\n", templateWords);
    }
}

typedef struct XenoActorEffectRef {
    u8 unmodeled_00[0x4];
    s32 offset;
} XenoActorEffectRef;

/* Registers the effect named by name at effectBase + ref->offset (nothing when the offset is 0); logs when the registration fails. */
void xenoActorEventRegisterEffect(s32 effectBase, XenoActorEffectRef *effectRef, const char *name) {
    s32 effectOffset;
    s32 effectAddress;
    s32 groupNumber;
    s32 partNumber;
    s32 registerResult;

    effectOffset = effectRef->offset;
    effectAddress = effectBase + effectOffset;
    if (effectOffset == 0) {
        return;
    }
    memset(&SEF_SCRATCHPAD(void, 0x0), 0, 64);
    strcpy(&SEF_SCRATCHPAD(char, 0x0), name);
    (&SEF_SCRATCHPAD(u8, 0x0))[5] = 0;
    groupNumber = func_0030C108(0x70000002);
    partNumber = func_0030C108(0x70000006);
    registerResult = srsSetEffectHandleValueResult(effectAddress, (groupNumber << 12) | partNumber | 0x20000000, 0);
    if (registerResult == 0) {
        func_001936F0("XenoActorEvent_Effect.euc.c", 288, "failed regist effect\n");
    }
}

/* Byte-addressed entry list: firstActorEntryOffset (0x34) is a byte offset from the list head. */
typedef struct XenoActorEntryList {
    u8 unk_00[0x34];
    s32 firstActorEntryOffset;
} XenoActorEntryList;

/* Entry: 0x60 bytes of fixed fields, then actorPayloadSize bytes of payload; the next entry follows both. */
typedef struct XenoActorEntry {
    u8 unk_00[0x14];
    s32 actorPayloadSize;
} XenoActorEntry;

#define BYTE_OFFSET(base, off) ((u8 *) (base) + (off))

void *xenoActorEventTableGetEntry(void *actorListHead, u32 entriesToSkip)
{
  int fixedEntrySize;
  u32 walkedCount;
  void *firstActorEntry;
  void *currentActorEntry;
  XenoActorEntry *steppedEntry;
  XenoActorEntryList *actorList;
  actorList = actorListHead;
  walkedCount = 0;
  firstActorEntry = BYTE_OFFSET(actorList, actorList->firstActorEntryOffset);
  currentActorEntry = firstActorEntry;
  if (entriesToSkip != 0)
  {
    fixedEntrySize = 0x60;
    do
    {
      walkedCount += 1;
      steppedEntry = currentActorEntry;
      currentActorEntry = BYTE_OFFSET(BYTE_OFFSET(currentActorEntry, fixedEntrySize), steppedEntry->actorPayloadSize);
    }
    while (walkedCount < entriesToSkip);
  }
  return currentActorEntry;
}

typedef struct XenoActorEffectTable {
    u8 unk_00[0x30];
    u32 entryCount;
} XenoActorEffectTable;

typedef struct XenoActorEffectEntry {
    u8 unk_00[0x18];
    s32 unk_18;
    u8 unk_1C[0x4];
    s32 unk_20;
    s32 unk_24;
} XenoActorEffectEntry;

typedef struct XenoActorEffectSlot {
    float unk_00[16];
    float unk_40;
    u8 flag44;
    u8 unk_45;
    u16 unk_46;
    u8 unk_48[0x48];
} XenoActorEffectSlot;

typedef struct XenoActorSpawnRequest {
    s32 effectResult;
    s32 unk_04;
    s32 unk_08;
    u8 unk_0C[0x20];
    s32 indexWord;
    u8 unk_30[0x0A];
    s16 unk_3A;
    u8 unk_3C[0x04];
    u8 slotCopy[0x40];
} XenoActorSpawnRequest;

union XenoActorEffectFrame {
    float out[16];
    struct {
        u8 pad[32];
        XenoActorSpawnRequest request;
    } req;
};

typedef struct XenoActorEffectObject {
    u8 unk_00[0x90];
    u32 flags : 32;
    u8 unk_94[0x460 - 0x94];
    u64 unk_460;
    u64 unk_468;
    u64 unk_470;
    u64 unk_478;
    u64 unk_480;
    u64 unk_488;
    u64 unk_490;
    u64 unk_498;
    u64 unk_4A0;
    u64 unk_4A8;
} XenoActorEffectObject;

XenoCdActorSlot *xenoCdActorSlotFind(s32 id);
void sefEffectSetStop();
s32 sefSpawnEffectFromRequest();
void sefRunEffectFrames(s32 effectHandle, float frames, float scale);
s32 xenoActorEventIsReadyFlagSet(void);
void xenoActorSlotBlockAdd(XenoActorSlotBlock *block, s32 value);
void xenoActorEffectGroupUpdate(XenoActorEffectGroup *group);

/* Spawns or updates every effect of the current table (up to 64): copies its 16-float block into the slot, builds the spawn request, classifies the effect by its ee%03d_%03d name and updates an existing object or spawns one (mode 1 skips spawning). */
void xenoActorEventSpawnEffects(s32 mode) {
    XenoActorEffectEntry *effectEntry;
    XenoActorEffectSlot *effectSlot;
    union XenoActorEffectFrame spawnFrame;
    char effectName[16];
    void *payloadCursor;
    u64 *slotQuadwords;
    s32 entryIndex;
    s32 effectHandle;
    s32 effectKind;
    s32 objectFlagWord;
    s32 flag44Value;

    entryIndex = 0;
    if (((XenoActorEffectTable *) gXglCdBlockStream.currentActorRecord)->entryCount == 0) {
        return;
    }
    do {
        /* kept: the local 1 compared with mode decides the emitted compare (the literal form does not match). */
        s32 one = 1;
        if (entryIndex >= 64) {
            func_001936F0("XenoActorEvent_Effect.euc.c", 331, "not enough effect buffer !!\n");
        } else {
            effectEntry =
                (XenoActorEffectEntry *) xenoActorEventTableGetEntry(gXglCdBlockStream.currentActorRecord, entryIndex);
            if (effectEntry == 0) {
                assertFailed("pElemEff", "XenoActorEvent_Effect.euc.c", 337);
            }
            effectSlot = (XenoActorEffectSlot *) ((u8 *) &gXglCdBlockStream + 0x1B00 + entryIndex * 144);
            payloadCursor = (u8 *) effectEntry + 0x60;
            func_0029BEE0(spawnFrame.out, &payloadCursor, gXglCdBlockStream.unk_3F20, 15);
            flag44Value = 0.0f < spawnFrame.out[9];
            effectSlot->unk_46 = (s32) spawnFrame.out[10];
            effectSlot->flag44 = flag44Value;
            effectSlot->unk_00[0] = spawnFrame.out[0];
            effectSlot->unk_00[1] = spawnFrame.out[1];
            effectSlot->unk_00[2] = spawnFrame.out[2];
            effectSlot->unk_00[3] = 1.0f;
            effectSlot->unk_00[4] = spawnFrame.out[3];
            effectSlot->unk_00[5] = spawnFrame.out[4];
            effectSlot->unk_00[6] = spawnFrame.out[5];
            effectSlot->unk_00[7] = 1.0f;
            effectSlot->unk_00[8] = spawnFrame.out[6];
            effectSlot->unk_00[9] = spawnFrame.out[7];
            effectSlot->unk_00[10] = spawnFrame.out[8];
            effectSlot->unk_00[11] = 1.0f;
            effectSlot->unk_40 = spawnFrame.out[11];
            effectSlot->unk_00[12] = spawnFrame.out[12];
            effectSlot->unk_00[13] = spawnFrame.out[13];
            effectSlot->unk_00[14] = spawnFrame.out[14];
            effectSlot->unk_00[15] = 1.0f;
            sefEffectSetStop(&spawnFrame.req.request);
            spawnFrame.req.request.indexWord = (effectEntry->unk_20 << 12) | effectEntry->unk_24 | 0x20000000;
            spawnFrame.req.request.unk_04 = 0;
            spawnFrame.req.request.unk_08 = 0;
            spawnFrame.req.request.unk_3A = 0;
            QuadwordCopy(spawnFrame.req.request.slotCopy, effectSlot);
            QuadwordCopy(spawnFrame.req.request.slotCopy + 16, (u8 *) effectSlot + 16);
            QuadwordCopy(spawnFrame.req.request.slotCopy + 32, (u8 *) effectSlot + 32);
            QuadwordCopy(spawnFrame.req.request.slotCopy + 48, (u8 *) effectSlot + 48);
            if (effectEntry->unk_18 >= 0) {
                spawnFrame.req.request.effectResult = xenoCdActorSlotFind(effectEntry->unk_18);
            } else {
                spawnFrame.req.request.effectResult = 0;
            }
            ((int (*)(char *destination, const char *format, ...)) sprintf)(
                effectName, "ee%03d_%03d", effectEntry->unk_20, effectEntry->unk_24);
            effectKind = xenoActorEventClassifyId((s32) effectName);
            slotQuadwords = (u64 *) effectSlot;
            switch (effectKind) {
            case 1: {
                XenoActorEffectObject *effect = (XenoActorEffectObject *) spawnFrame.req.request.effectResult;
                if (effect != 0) {
                    objectFlagWord = effect->flags & 0xFFF7FFFF;
                    effect->flags = objectFlagWord;
                    effect->unk_460 = slotQuadwords[0];
                    effect->unk_468 = slotQuadwords[1];
                    effect->unk_480 = slotQuadwords[2];
                    effect->unk_488 = slotQuadwords[3];
                    effect->unk_4A0 = slotQuadwords[4];
                    effect->unk_4A8 = slotQuadwords[5];
                    if (effectSlot->flag44 != 0) {
                        effect->flags = objectFlagWord | 0x80000;
                    }
                }
            } break;
            case 2: {
                XenoActorEffectObject *effect = (XenoActorEffectObject *) spawnFrame.req.request.effectResult;
                if (effect != 0 && effectSlot->flag44 != 0) {
                    if (effect->flags & 0x100000) {
                        effect->unk_470 = slotQuadwords[0];
                        effect->unk_478 = slotQuadwords[1];
                        effect->unk_490 = slotQuadwords[2];
                        effect->unk_498 = slotQuadwords[3];
                    } else {
                        effect->unk_460 = slotQuadwords[0];
                        effect->unk_468 = slotQuadwords[1];
                        effect->unk_480 = slotQuadwords[2];
                        effect->unk_488 = slotQuadwords[3];
                        effect->flags = effect->flags | 0x100000;
                    }
                }
            } break;
            case 0:
            default: {
                if (effectSlot->flag44 != 0) {
                    one = 1;
                    if (mode != one) {
                        effectHandle = sefSpawnEffectFromRequest(&spawnFrame.req.request);
                        if (effectHandle != 0) {
                            xenoActorSlotBlockAdd(effectSlot, effectHandle);
                            if (effectSlot->unk_46 != 0) {
                                sefRunEffectFrames(effectHandle, (float) effectSlot->unk_46, 4.0f);
                            }
                            sefLinkedObjectSetLinkedFloat(effectHandle, effectSlot->unk_40);
                            if (xenoActorEventIsReadyFlagSet() != 0) {
                                sefLinkedObjectSetLinkedFloat(effectHandle, effectSlot->unk_40 + effectSlot->unk_40);
                            }
                        } else {
                            func_001936F0("XenoActorEvent_Effect.euc.c", 469,
                                          "*** failed craete effect %d %d\n", effectEntry->unk_20, effectEntry->unk_24);
                        }
                    }
                }
                xenoActorEffectGroupUpdate(effectSlot);
            }
            }
        }
        entryIndex++;
    } while (entryIndex < ((XenoActorEffectTable *) gXglCdBlockStream.currentActorRecord)->entryCount);
}

const char D_00539560[24] = "mdl/chr/pc/C3kosmos05";
const char D_00539578[24] = "mdl/chr/pc/C3kosmos01";
const char D_00539590[24] = "mdl/chr/pc/C3allenH02";
const char D_005395A8[24] = "mdl/chr/pc/C3allenH00";
const char D_005395C0[24] = "mdl/chr/pc/C3allen02";
const char D_005395D8[24] = "mdl/chr/pc/C3allen01";
const char D_005395F0[24] = "mdl/chr/pc/C3allen00";
const char D_00539608[24] = "mdl/chr/pc/C3ziggyH00";
const char D_00539620[24] = "mdl/chr/pc/C3ziggy00";
const char D_00539638[24] = "mdl/chr/pc/C3shionH05";
const char D_00539650[24] = "mdl/chr/pc/C3shionH03";
const char D_00539668[24] = "mdl/chr/pc/C3shionH00";
const char D_00539680[24] = "mdl/chr/pc/C3shion05";
const char D_00539698[24] = "mdl/chr/pc/C3shion03";
const char D_005396B0[24] = "mdl/chr/pc/C3shion00";
const char D_005396C8[24] = "mdl/chr/pc/C3momoH00";
const char D_005396E0[24] = "mdl/chr/pc/C3momo00";
const char D_005396F8[24] = "mdl/chr/pc/C3kosmosH05";
const char D_00539710[24] = "mdl/chr/pc/C3kosmosH04";
const char D_00539728[24] = "mdl/chr/pc/C3kosmosH01";
const char D_00539740[24] = "mdl/chr/pc/C3kosmosH00";
const char D_00539758[24] = "mdl/chr/pc/C3kosmos04";
const char D_00539770[24] = "mdl/chr/pc/C3kosmos00";
const char D_00539788[24] = "mdl/chr/pc/C3jrH00";
const char D_005397A0[24] = "mdl/chr/pc/C3jr00";
const char D_005397B8[24] = "mdl/chr/pc/C3jinH00";
const char D_005397D0[24] = "mdl/chr/pc/C3jin00";
const char D_005397E8[24] = "mdl/chr/pc/C3chaosH00";
const char D_00539800[24] = "mdl/chr/pc/C3chaos00";
const char D_00539818[24] = "mdl/chr/pc/C3kosmos18";
const char D_00539830[24] = "mdl/chr/pc/C3allenH03";
const char D_00539848[24] = "mdl/chr/pc/C3allen03";
const char D_00539860[24] = "mdl/chr/pc/C3ziggyH01";
const char D_00539878[24] = "mdl/chr/pc/C3ziggy01";
const char D_00539890[24] = "mdl/chr/pc/C3shionH06";
const char D_005398A8[24] = "mdl/chr/pc/C3shion06";
const char D_005398C0[24] = "mdl/chr/pc/C3momoH01";
const char D_005398D8[24] = "mdl/chr/pc/C3momo01";
const char D_005398F0[24] = "mdl/chr/pc/C3kosmosH07";
const char D_00539908[24] = "mdl/chr/pc/C3kosmos07";
const char D_00539920[24] = "mdl/chr/pc/C3jrH02";
const char D_00539938[24] = "mdl/chr/pc/C3jr02";
const char D_00539950[24] = "mdl/chr/pc/C3jinH02";
const char D_00539968[24] = "mdl/chr/pc/C3jin02";
const char D_00539980[24] = "mdl/chr/pc/C3chaosH01";
const char D_00539998[24] = "mdl/chr/pc/C3chaos01";

const char D_00550CB8[8] = "pRg";
const char D_00550CC0[8] = "ee";
const char D_00550CC8[8] = "ce";
const char D_00550CD0[8] = "pe";
const char D_00550CD8[8] = "ef\\";

unsigned char gXglCdBlockStreamStorage[6160] = { 0 };
unsigned char D_003E8B10[64] = { 0 };
unsigned char gXenoActorBufferB[64] = { 0 };
unsigned char gXenoActorBufferC[64] = { 0 };
XenoActorRange *gXenoCurrentActorRange[] = { 0 };
XenoActorRecord *gXenoCurrentActorRecord[139] = { 0 };
unsigned char gXenoActorEffectRecords[9224] = { 0 };
s32 xglCdPendingReadStage[2] = { 0 };
s32 D_003EB210[5] = { 0 };
s32 D_003EB224[4] = { 0 };
s32 D_003EB234[] = { 0 };
s32 D_003EB238[] = { 0 };
s32 D_003EB23C[] = { 0 };
s32 D_003EB240[] = { 0 };
s32 D_003EB244[] = { 0 };
s32 D_003EB248[] = { 0 };
s32 gXglCdDriveStatus[] = { 0 };
s32 gXglCdDriveReadyStatus[] = { 0 };
XenoMatrix *gXenoActorMatrixTable[130] = { 0 };
s32 D_003EB45C[9] = { 0 };
char *gEsdFilePrefixes[] = { D_00550CD0, D_00550CC8, D_00550CC0, 0 };
