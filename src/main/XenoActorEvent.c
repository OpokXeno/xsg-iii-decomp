#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024E678);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024E758);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024E7F0);

typedef int s32;
typedef signed char s8;
typedef unsigned int u32;
typedef unsigned char u8;

/* Table reached through XglCdBlockStream.currentActorRecord: entry count at 0x50. */
typedef struct XenoActorTable {
    u8 unk_00[0x50];
    u32 actorCount;
} XenoActorTable;

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

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024E8C8);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024EA68);

s32 xenoActorEventAddOffset(s32 baseAddress, s32 displacement) {
    return baseAddress + displacement;
}

extern unsigned char gXglCdBlockStreamStorage[];
#define gXglCdBlockStream (*(XglCdBlockStream *) gXglCdBlockStreamStorage)
s32 func_0024E348(XenoActorTable *table, u32 index);
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
      actorEntry = func_0024E348(loopBlockStream->currentActorRecord, actorIndex);
      func_002A1B30(actorEntry, loopBlockStream->unk_3F20);
      actorCount = loopBlockStream->currentActorRecord->actorCount;
    }
    while (visitedCount < actorCount);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024EBF0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024ECB0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024F3C0);

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

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024F590);

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

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024F690);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024F718);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024F7F0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024F948);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024FAB0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024FC20);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024FCB0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024FE78);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_0024FF78);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250060);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_002500E0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250210);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250330);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250440);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250488);

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

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250540);

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

extern XenoActorRange *gXenoCurrentActorRange[];

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

extern unsigned char D_00539440[];
extern unsigned char D_00539450[];
s32 strcmp();

s32 xenoActorEventClassifyId(s32 id) {
    if (strcmp(id, D_00539440) == 0) {
        return 1;
    }
    return (strcmp(id, D_00539450) != 0) ? 0 : 2;
}

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250768);

s32 func_00250768(void);

void xenoActorEventRunStep(void) {
    func_00250768();
}

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_002507E8);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250828);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250960);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250A30);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250BB0);

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250D58);

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

INCLUDE_ASM("asm/main/nonmatchings/XenoActorEvent", func_00250E48);
