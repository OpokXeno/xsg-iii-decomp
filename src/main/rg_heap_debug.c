#include "common.h"
#include "shared.h"

/* A 128-bit store or load of the +0x30 analog group (a C store of a zero is `por $4,$0,$0; sq $4,0x30(pad)`;
 * QuadwordZero would drop the por and change the register allocation of xglPadRead). */
typedef unsigned int PadQuadword __attribute__((mode(TI)));

#include "main/tu007.h"

extern PadRecord PadData[2];

struct RgHeap;
void InitRgHeap(struct RgHeap *pHeap, void *pMemory, int arenaSize);

/* Heap count of the heap windows (the assert text names it HEAP_MAX). */
#define HEAP_MAX 6

/* Size of one heap arena (the 2048 bytes InitRgHeap is given). */
#define HEAP_ARENA_SIZE 0x800

/* Smallest block a heap keeps: the 0x40-byte RgHeapBlock header plus one byte. */
#define HEAP_MIN_BLOCK_SIZE 65

/* Length of the block magic stamp (RgHeapBlock.magic holds it plus a terminator). */
#define HEAP_MAGIC_SIZE 15

/* Heap map: 256 pixels wide, painted over the arena in colour HEAP_MAP_ARENA_COLOR and the blocks in HEAP_MAP_BLOCK_COLOR. */
#define HEAP_MAP_WIDTH 256

/* mapStride that InitRgHeap stores; the map row length and the bytes per map cell. */
#define HEAP_MAP_STRIDE_DEFAULT 0x100

/* Allocation granularity: payload sizes are rounded up to 64 bytes, and payloads are 16-byte aligned. */
#define HEAP_BLOCK_GRANULE 64
#define HEAP_PAYLOAD_ALIGN 16

#define BYTE_OFFSET(base, offset) ((u8 *) (base) + (offset))
#define HEAP_MAP_ARENA_COLOR 0x80000080
#define HEAP_MAP_BLOCK_COLOR 0x80008080
#define HEAP_ALLOC_WINDOW_X 10
#define HEAP_ALLOC_WINDOW_Y 120
#define HEAP_ALLOC_WINDOW_WIDTH 480
#define HEAP_ALLOC_WINDOW_HEIGHT 200

/* Text scale options of heapWindowMapStrideAdjust and the clamp range of the scale (1.0 is 0x10000). */
#define TEXT_SCALE_OPTION_DOUBLE 0x10

#define TEXT_SCALE_OPTION_HALF 0x80
#define TEXT_SCALE_MIN 1
#define TEXT_SCALE_MAX 0x10000

/*
 * RgHeapBlock: the fixed 0x40-byte header RgHeapAlloc places immediately
 * before the block it returns (RgHeapDumpBlock reads it at payloadAddress - 0x40).
 * chainPrev/chainNext link the block into whichever circular list currently owns it
 * (RgHeap's free chain from _SetHeapHead, or its allocated chain from
 * _Link/_InsertNext), evidenced by "pre %p next %p\n". blockLength is the block's
 * byte size and mark is set/cleared by _MarkAlloc/_UnmarkAlloc/_IsAllocated,
 * evidenced by "size %p mark %d\n" (blockLength itself is dumped through %p in that
 * string). module and line record the RgHeapAlloc call site (module is
 * strncpy'd from its source_file argument and terminated at module[31]),
 * evidenced by "module '%s' line %d\n". The dumps print module + 1, and
 * RgHeapFree reads module[0] as the owning heap id before it looks the heap
 * up. magic holds the 15-character
 * s_szMagicString stamp _SetMagicString/_IsCollectMagicString read and write
 * at block+0x30. Every byte from 0x00 to 0x3F is accounted for by these six
 * members (4+4+4+2+2+32+16 == 0x40).
 */
struct RgHeapBlock {
    struct RgHeapBlock *chainPrev;
    struct RgHeapBlock *chainNext;
    u32 blockLength;
    u16 mark;
    u16 line;
    char module[32];
    u8 magic[16];
};

/*
 * The heap head: top and size are printed as "heap = %p top = %p size = %d",
 * +0x08 is the top of the free chain that the dump prints as "FREE MAP top",
 * +0x0C heads the allocated chain. +0x10 (mapStride) is stored as 0x100 by
 * InitRgHeap; heapWindowPaintMap uses it both as the map row length and as the
 * bytes per map cell.
 */
struct RgHeap {
    void *top;
    u32 arenaSize;
    struct RgHeapBlock *freeHead;
    struct RgHeapBlock *allocHead;
    int mapStride;
};

int printf(const char *format, ...);

char *strcpy(char *destination, const char *source);

void RgHeapDump_sub(struct RgHeap *pHeap, const char *pComment, const char *pSourceFile, int line);

void _Error(const char *pExpression, const void *unused, struct RgHeap *pHeap,
                   const char *pSourceFile, int line);

extern unsigned char *heapMagicStringPtr;
extern const char heapAssertHeadSizeBelowBlockSize[];
extern const char heapMakeBlockComment[];

struct RgHeapBlock *_SetHeapHead(struct RgHeapBlock *heapBlock, u32 blockLength);

/* One 0x20-byte heap slot per heap id: the RgHeap head, then 0xC bytes not modeled. */
struct RgHeapSlot {
    struct RgHeap heap;
    unsigned char unmodeled_14[0xC];
};

/* InstanceOfRgHeapById: the heap heapId (0x20 bytes apart); its 0x800-byte arena is set up by the first call, tracked by a word flag per id. */
extern struct RgHeapSlot heapInstances[HEAP_MAX];

extern unsigned char heapArenas[HEAP_MAX][HEAP_ARENA_SIZE];
extern int heapArenaInitialized[HEAP_MAX];

struct RgHeap *InstanceOfRgHeapById(u32 heapId);

extern const char heapAssertBufSizeAligned[];
extern char heapInitStageComment[];

void *memset(void *destination, int value, unsigned int count);

int RgHeapIsInSelf(struct RgHeap *pHeap, void *payloadAddress);

void *testWindowGetUserData(int window);

struct HeapMapLayout {
    unsigned char unmodeled_00[0xC];
    int rowCount;
};

void testWindowGetInfo(int window, struct HeapMapLayout *pInfo);
void testWindowDrawLine(int window, int x0, int y0, int x1, int y1, u32 color);

struct HeapWindowRow {
    void *callbacks[2];
    char *pModuleName;
    u16 *pLine;
    unsigned char unmodeled_10[0x24];
    union {
        struct RgHeap *pHeap;
        u32 heapId;
    } heap;
    unsigned char unmodeled_38[0x18];
};

/* A 0xa0-byte template: the HeapWindowRow the code copies, then 0x50 more bytes. */
struct HeapWindowRowTemplate {
    struct HeapWindowRow row;
    unsigned char unmodeled_50[0x50];
};

extern struct HeapWindowRowTemplate heapWindowAllocRowTemplate;

void testWindowAddRow(int window, struct HeapWindowRow *pRow);
void testWindowClearRows(void);

struct WindowCreateParam {
    int x;
    int y;
    int width;
    int height;
    char name[32];
};

int testWindowFindByName(int parentWindow, const char *pName);

void testWindowShow(int window);

int testWindowCreate(struct WindowCreateParam *pParam, int parentWindow);

extern const char heapAssertHeaderFormat[40];
extern const char heapAssertExpressionFormat[24];
extern const char heapAssertHeapInfoFormat[32];
extern const char heapAssertDumpComment[16];
extern const char heapSourcePath[16];
extern const char heapAssertHeapNilFormat[40];
extern const char heapAssertHeadSizeBelowBlockSize[24];
extern const char heapMakeBlockComment[40];
extern const char heapAssertBufSizeAligned[24];
extern const char heapDumpSeparatorFormat[40];
extern const char heapDumpHeadFormat[32];
extern const char heapDumpFreeMapFormat[40];
extern const char heapDumpFreeBlockFormat[32];
extern const char heapDumpAllocMapFormat[40];
extern const char heapDumpAllocBlockFormat[48];
extern const char heapDumpBlockHeaderFormat[24];
extern const char heapDumpBlockLinksFormat[16];
extern const char heapDumpBlockSizeMarkFormat[24];
extern const char heapDumpBlockModuleFormat[24];
extern const char heapDumpBlockNotInHeapFormat[32];

/* The title of the allocation position window (the original string is Japanese). */
extern char heapWindowAllocTitle[16];

extern char heapInitStageComment[8];

void _Error(const char *pExpression, const void *unused, struct RgHeap *pHeap,
                   const char *pSourceFile, int line)
{
    printf(heapAssertHeaderFormat);
    printf(heapAssertExpressionFormat, pExpression, pSourceFile, line);
    if (pHeap != 0) {
        printf(heapAssertHeapInfoFormat, pHeap, pHeap->top, pHeap->arenaSize);
        RgHeapDump_sub(pHeap, heapAssertDumpComment, heapSourcePath, 41);
    } else {
        printf(heapAssertHeapNilFormat);
    }
    for (;;) {
    }
}

void _Link(struct RgHeapBlock *heapBlock, struct RgHeapBlock *pPre,
                  struct RgHeapBlock *followingBlock)
{
    heapBlock->chainPrev = pPre;
    heapBlock->chainNext = followingBlock;
}

void _InsertPre(struct RgHeapBlock *heapBlock, struct RgHeapBlock *insertedBlock)
{
    _Link(insertedBlock, heapBlock->chainPrev, heapBlock);
    heapBlock->chainPrev->chainNext = insertedBlock;
    heapBlock->chainPrev = insertedBlock;
}

void _InsertNext(struct RgHeapBlock *heapBlock, struct RgHeapBlock *insertedBlock)
{
    _Link(insertedBlock, heapBlock, heapBlock->chainNext);
    heapBlock->chainNext->chainPrev = insertedBlock;
    heapBlock->chainNext = insertedBlock;
}

void _Unlink(struct RgHeapBlock *heapBlock)
{
    heapBlock->chainNext->chainPrev = heapBlock->chainPrev;
    heapBlock->chainPrev->chainNext = heapBlock->chainNext;
}

void _MarkAlloc(struct RgHeapBlock *heapBlock)
{
    heapBlock->mark = 1;
}

void _UnmarkAlloc(struct RgHeapBlock *heapBlock)
{
    heapBlock->mark = 0;
}

int _IsAllocated(struct RgHeapBlock *heapBlock)
{
    return heapBlock->mark != 0;
}

/* Stamps the 15-byte magic (read through the heapMagicStringPtr pointer) into the block header. */
void _SetMagicString(struct RgHeapBlock *heapBlock)
{
    u8 *pMagic = heapBlock->magic;
    int magicIndex = 0;

    do {
        *pMagic = heapMagicStringPtr[magicIndex];
        magicIndex += 1;
        pMagic += 1;
    } while (magicIndex < HEAP_MAGIC_SIZE);
}

/* Returns 1 when the 15 magic bytes at block+0x30 equal the stamp heapMagicStringPtr points to, else 0. */
int _IsCollectMagicString(struct RgHeapBlock *heapBlock)
{
    int magicIndex = 0;

    do {
        if (heapBlock->magic[magicIndex] != (signed char)heapMagicStringPtr[magicIndex]) {
            return 0;
        }
        magicIndex += 1;
    } while (magicIndex < HEAP_MAGIC_SIZE);
    return 1;
}

struct RgHeapBlock *_SetHeapHead(struct RgHeapBlock *heapBlock, u32 blockLength)
{
    if (blockLength < HEAP_MIN_BLOCK_SIZE) {
        _Error(heapAssertHeadSizeBelowBlockSize, heapMakeBlockComment, 0, heapSourcePath, 149);
    }
    _Link(heapBlock, heapBlock, heapBlock);
    heapBlock->blockLength = blockLength;
    return heapBlock;
}

/*
 * Walks the circular free list once and folds every block that ends exactly
 * where its successor starts into that block.
 */
void _MergeBlocks(struct RgHeapBlock *pHead)
{
    struct RgHeapBlock *heapBlock = pHead;
    struct RgHeapBlock *mergeNextBlock;

    if (pHead == 0) {
        return;
    }
    do {
        mergeNextBlock = heapBlock->chainNext;
        while ((u8 *)heapBlock + heapBlock->blockLength == (u8 *)mergeNextBlock) {
            heapBlock->chainNext = mergeNextBlock->chainNext;
            heapBlock->blockLength += mergeNextBlock->blockLength;
            mergeNextBlock->chainNext->chainPrev = heapBlock;
            mergeNextBlock = heapBlock->chainNext;
        }
        heapBlock = mergeNextBlock;
    } while (heapBlock != pHead);
}

struct RgHeap *InstanceOfRgHeapById(u32 heapId)
{
    if (heapArenaInitialized[heapId] == 0) {
        heapArenaInitialized[heapId] = 1;
        InitRgHeap(&heapInstances[heapId].heap, heapArenas[heapId], HEAP_ARENA_SIZE);
    }
    return &heapInstances[heapId].heap;
}

void ClearRgHeap(struct RgHeap *pHeap)
{
    pHeap->freeHead = _SetHeapHead(pHeap->top, pHeap->arenaSize);
}

void InitRgHeap(struct RgHeap *pHeap, void *pMemory, int arenaSize)
{
    struct RgHeapBlock *initialFreeBlock;

    if (arenaSize & (HEAP_PAYLOAD_ALIGN - 1)) {
        _Error(heapAssertBufSizeAligned, heapInitStageComment, 0, heapSourcePath, 201);
    }
    memset(pMemory, 0, arenaSize);
    pHeap->arenaSize = arenaSize;
    pHeap->top = pMemory;
    initialFreeBlock = _SetHeapHead(pMemory, arenaSize);
    pHeap->allocHead = 0;
    pHeap->freeHead = initialFreeBlock;
    pHeap->mapStride = HEAP_MAP_STRIDE_DEFAULT;
}

int GameMpeg2Play(char *destination, const char *format, ...);

extern const char heapAllocZeroSizeFormat[];
extern const char heapAssertHeapNotNilParenthesized[];
extern const char heapAllocEntryHeapPtrComment[];
extern const char heapAllocNoHeapFormat[];
extern const char heapNoHeapIdFormat[];
extern const char heapAllocTooBigDataHeapFormat[];
extern const char heapAssertMatchBlockSizeEnough[];
extern const char heapBlockAlignComment[];
extern const char heapAssertAllocAligned[];
extern const char heapNotAlignedBlockComment[];

void func_0030ED88(char *pDest, const char *pSrc, u32 count);

void *RgHeapAllocById(u32 heapId, u32 requestedBytes, const char *pSourceFile, int line)
{
    char dumpComment[32];
    struct RgHeap *pHeap;
    struct RgHeapBlock *pFreeHead;
    struct RgHeapBlock *heapBlock;
    struct RgHeapBlock *pFit;
    struct RgHeapBlock *pSplit;
    u32 bestSize;
    u32 requiredBlockSize;
    u32 remainder;
    void *payloadAddress;

    pHeap = InstanceOfRgHeapById(heapId);
    if (requestedBytes == 0) {
        printf(heapAllocZeroSizeFormat, pSourceFile, line);
    }
    if (pHeap == 0) {
        _Error(heapAssertHeapNotNilParenthesized, heapAllocEntryHeapPtrComment, 0, heapSourcePath, 227);
    }
    pFreeHead = pHeap->freeHead;
    if (pFreeHead == 0) {
        printf(heapAllocNoHeapFormat, pSourceFile, line);
        return 0;
    }
    pFit = 0;
    requiredBlockSize = ((requestedBytes + HEAP_BLOCK_GRANULE - 1) & -HEAP_BLOCK_GRANULE) + sizeof(struct RgHeapBlock);
    bestSize = 0;
    heapBlock = pFreeHead;
    do {
        if (heapBlock->blockLength >= requiredBlockSize) {
            if (heapBlock->blockLength < bestSize || bestSize == 0) {
                bestSize = heapBlock->blockLength;
                pFit = heapBlock;
            }
        }
        heapBlock = heapBlock->chainNext;
    } while (heapBlock != pFreeHead);
    if (pFit == 0) {
        GameMpeg2Play(dumpComment, heapNoHeapIdFormat, heapId);
        RgHeapDump_sub(pHeap, dumpComment, heapSourcePath, 255);
        printf(heapAllocTooBigDataHeapFormat, requestedBytes, pSourceFile, line);
        return 0;
    }
    if (bestSize < requiredBlockSize) {
        _Error(heapAssertMatchBlockSizeEnough, heapBlockAlignComment, pHeap, heapSourcePath, 265);
    }
    remainder = bestSize - requiredBlockSize;
    if (remainder >= HEAP_MIN_BLOCK_SIZE) {
        pSplit = (struct RgHeapBlock *) BYTE_OFFSET(pFit, requiredBlockSize);
        _SetHeapHead(pSplit, remainder);
    } else {
        requiredBlockSize = bestSize;
        pSplit = 0;
    }
    if (pSplit != 0) {
        _InsertNext(pFit, pSplit);
    }
    if (pFit == pFreeHead) {
        pHeap->freeHead = pFit->chainNext;
    }
    _Unlink(pFit);
    _SetHeapHead(pFit, requiredBlockSize);
    _MarkAlloc(pFit);
    if (pSourceFile != 0) {
        func_0030ED88(pFit->module + 1, pSourceFile, 30);
        pFit->module[31] = 0;
        pFit->line = line;
    } else {
        pFit->module[1] = 0;
        pFit->line = 0;
    }
    _SetMagicString(pFit);
    pFit->module[0] = heapId;
    if (pHeap->allocHead == 0) {
        pHeap->allocHead = pFit;
        _Link(pFit, pFit, pFit);
    } else {
        _InsertNext(pHeap->allocHead, pFit);
    }
    payloadAddress = pFit + 1;
    if (((u32)payloadAddress & (HEAP_PAYLOAD_ALIGN - 1)) != 0) {
        _Error(heapAssertAllocAligned, heapNotAlignedBlockComment, pHeap, heapSourcePath, 309);
    }
    return payloadAddress;
}

extern const char heapAssertHeapNotNil[];
extern const char heapFreeEntryComment[];
extern const char heapFreeNilAddressFormat[];
extern const char heapErrorComment[];
extern const char heapAlreadyFreeFormat[];
extern const char heapNotAllocatedFreeFormat[];
extern const char heapBlockDestroyedFormat[];
extern const char heapPtrNotInAreaFormat[];
extern const char heapAllocListNodeFreeBlockMismatchFormat[];

void RgHeapFree(void *payloadAddress, const char *pSourceFile, int line)
{
    struct RgHeapBlock *heapBlock;
    struct RgHeap *pHeap;
    struct RgHeapBlock *pAllocHead;
    struct RgHeapBlock *blockAfterAllocHead;
    struct RgHeapBlock *freeChainCursor;

    heapBlock = (struct RgHeapBlock *)payloadAddress - 1;
    pHeap = InstanceOfRgHeapById(heapBlock->module[0]);
    if (pHeap == 0) {
        _Error(heapAssertHeapNotNil, heapFreeEntryComment, 0, heapSourcePath, 323);
    }
    if (payloadAddress == 0) {
        printf(heapFreeNilAddressFormat, pSourceFile, line);
    }
    if (_IsAllocated(heapBlock) == 0) {
        if (_IsCollectMagicString(heapBlock) != 0) {
            RgHeapDump_sub(pHeap, heapErrorComment, heapSourcePath, 336);
            printf(heapAlreadyFreeFormat, payloadAddress, pSourceFile, line, heapBlock->module + 1, heapBlock->line);
        } else {
            printf(heapNotAllocatedFreeFormat, payloadAddress, pSourceFile, line);
        }
    }
    if (_IsCollectMagicString(heapBlock) == 0) {
        printf(heapBlockDestroyedFormat, payloadAddress, pSourceFile, line);
    }
    if (RgHeapIsInSelf(pHeap, heapBlock) == 0) {
        printf(heapPtrNotInAreaFormat, pSourceFile, pHeap, pSourceFile, line);
    }
    pAllocHead = pHeap->allocHead;
    blockAfterAllocHead = pAllocHead->chainNext;
    if (blockAfterAllocHead == pAllocHead) {
        if (heapBlock != blockAfterAllocHead) {
            printf(heapAllocListNodeFreeBlockMismatchFormat);
        }
        pHeap->allocHead = 0;
    } else {
        if (pAllocHead == heapBlock) {
            _Unlink(heapBlock);
            pHeap->allocHead = blockAfterAllocHead;
        } else {
            _Unlink(heapBlock);
        }
    }
    _UnmarkAlloc(heapBlock);
    if (pHeap->freeHead == 0) {
        pHeap->freeHead = _SetHeapHead(heapBlock, heapBlock->blockLength);
        return;
    }
    freeChainCursor = pHeap->freeHead;
    _Link(heapBlock, 0, 0);
    do {
        if (heapBlock < freeChainCursor) {
            _InsertPre(freeChainCursor, heapBlock);
            if (heapBlock < pHeap->freeHead) {
                pHeap->freeHead = heapBlock;
            }
            break;
        }
        freeChainCursor = freeChainCursor->chainNext;
    } while (freeChainCursor != pHeap->freeHead);
    if (heapBlock->chainNext == 0) {
        _InsertPre(pHeap->freeHead, heapBlock);
    }
    _MergeBlocks(pHeap->freeHead);
}

int RgHeapIsInSelf(struct RgHeap *pHeap, void *payloadAddress)
{
    if (payloadAddress < pHeap->top || (u8 *)payloadAddress >= (u8 *)pHeap->top + pHeap->arenaSize) {
        return 0;
    }
    return 1;
}

int RgHeapIsInvalidMemory(struct RgHeap *pHeap, void *payloadAddress)
{
    struct RgHeapBlock *heapBlock;

    if (payloadAddress == 0) {
        return 1;
    }
    heapBlock = (struct RgHeapBlock *)payloadAddress - 1;
    if (_IsAllocated(heapBlock) == 0) {
        return 1;
    }
    if (_IsCollectMagicString(heapBlock) == 0) {
        return 1;
    }
    return RgHeapIsInSelf(pHeap, heapBlock) == 0;
}

void RgHeapDump_sub(struct RgHeap *pHeap, const char *pComment, const char *pSourceFile, int line)
{
    struct RgHeapBlock *heapBlock;

    heapBlock = pHeap->freeHead;
    printf(heapDumpSeparatorFormat);
    printf(heapDumpHeadFormat, pHeap, pSourceFile, line);
    printf(heapDumpFreeMapFormat, heapBlock, pComment);
    do {
        printf(heapDumpFreeBlockFormat, heapBlock, heapBlock->blockLength, heapBlock->blockLength, heapBlock->chainNext);
        heapBlock = heapBlock->chainNext;
    } while (heapBlock != pHeap->freeHead);
    heapBlock = pHeap->allocHead;
    printf(heapDumpAllocMapFormat, heapBlock, pComment);
    if (heapBlock != 0) {
        do {
            printf(heapDumpAllocBlockFormat, heapBlock, heapBlock->blockLength - sizeof(struct RgHeapBlock), heapBlock->blockLength, heapBlock->chainNext,
                   heapBlock->module + 1, heapBlock->line);
            heapBlock = heapBlock->chainNext;
        } while (heapBlock != pHeap->allocHead);
    }
}

void RgHeapDumpBlock(struct RgHeap *pHeap, void *payloadAddress)
{
    struct RgHeapBlock *heapBlock;

    printf(heapDumpBlockHeaderFormat);
    heapBlock = (struct RgHeapBlock *) payloadAddress - 1;
    if (RgHeapIsInSelf(pHeap, payloadAddress) != 0) {
        printf(heapDumpBlockLinksFormat, heapBlock->chainPrev, heapBlock->chainNext);
        printf(heapDumpBlockSizeMarkFormat, heapBlock->blockLength, heapBlock->mark);
        printf(heapDumpBlockModuleFormat, heapBlock->module + 1, heapBlock->line);
        return;
    }
    printf(heapDumpBlockNotInHeapFormat, pHeap);
}

/* Formats the heap id (heapDumpHeapIdFormat) and dumps the heap under that comment. */
extern const char heapDumpHeapIdFormat[16];

void RgHeapDumpById(u32 heapId)
{
    char comment[0x80];
    struct RgHeap *pHeap;

    if (heapId < HEAP_MAX) {
        pHeap = InstanceOfRgHeapById(heapId);
        GameMpeg2Play(comment, heapDumpHeapIdFormat, heapId);
        RgHeapDump_sub(pHeap, comment, heapSourcePath, 491);
    }
}

void textScaleApplyOptions(int window)
{
    struct RgHeap *pHeap;
    u16 pressedButtons;

    pHeap = testWindowGetUserData(window);
    pressedButtons = PadData[0].pressed;
    if ((pressedButtons & TEXT_SCALE_OPTION_DOUBLE) != 0) {
        pHeap->mapStride = pHeap->mapStride * 2;
    }
    if ((pressedButtons & TEXT_SCALE_OPTION_HALF) != 0) {
        pHeap->mapStride = pHeap->mapStride / 2;
    }
    if (pHeap->mapStride <= 0) {
        pHeap->mapStride = TEXT_SCALE_MIN;
    }
    if (pHeap->mapStride > TEXT_SCALE_MAX) {
        pHeap->mapStride = TEXT_SCALE_MAX;
    }
}

/*
 * Paints the heap map of a window. The arena is first filled in map rows
 * numbered from 1 with colour 0x80000080; a map row is mapStride cells wide
 * (InitRgHeap stores 0x100 there) and a cell is mapStride bytes, so the
 * partial last row is scaled to its share of the 256-pixel width. Every block
 * of the allocated chain (allocHead) is then painted over it with colour
 * 0x80008080, one run per map row. testWindowGetInfo supplies the row count in
 * mapLayout.rowCount.
 */
void heapWindowPaintMap(int window)
{
    struct HeapMapLayout mapLayout;
    struct RgHeap *pHeap;
    struct RgHeapBlock *heapBlock;
    int arenaCells;
    int blockRow;
    u32 blockCells;
    int mapRow = 1;
    u32 blockCol;
    u32 runCells;
    u32 arenaOffset;
    u32 cellIndex;

    pHeap = testWindowGetUserData(window);
    testWindowGetInfo(window, &mapLayout);

    arenaCells = pHeap->arenaSize / pHeap->mapStride;
    if (arenaCells != 0) {
        do {
            if (arenaCells >= pHeap->mapStride) {
                testWindowDrawLine(window, 0, mapRow, HEAP_MAP_WIDTH, mapRow, HEAP_MAP_ARENA_COLOR);
                arenaCells -= pHeap->mapStride;
            } else {
                testWindowDrawLine(window, 0, mapRow, (arenaCells * HEAP_MAP_WIDTH) / pHeap->mapStride, mapRow, HEAP_MAP_ARENA_COLOR);
                arenaCells = 0;
            }
            mapRow += 1;
        } while (mapRow < mapLayout.rowCount && arenaCells != 0);
    }

    heapBlock = pHeap->allocHead;
    if (heapBlock != 0) {
        do {
            arenaOffset = (u8 *)heapBlock - (u8 *)pHeap->top;
            cellIndex = arenaOffset / pHeap->mapStride;
            blockCells = heapBlock->blockLength / pHeap->mapStride;
            blockRow = cellIndex / pHeap->mapStride;
            blockCol = cellIndex % pHeap->mapStride;

            runCells = pHeap->mapStride - blockCol;
            if (blockCells < runCells) {
                runCells = blockCells;
            }
            testWindowDrawLine(window, blockCol, blockRow, ((blockCol + runCells) * HEAP_MAP_WIDTH) / pHeap->mapStride,
                        blockRow, HEAP_MAP_BLOCK_COLOR);
            blockCells -= runCells;
            blockRow += 1;

            do {
                runCells = pHeap->mapStride;
                if (blockCells < runCells) {
                    runCells = blockCells;
                }
                testWindowDrawLine(window, 0, blockRow, (runCells * HEAP_MAP_WIDTH) / pHeap->mapStride, blockRow, HEAP_MAP_BLOCK_COLOR);
                blockCells -= runCells;
                blockRow += 1;
            } while (blockRow <= mapLayout.rowCount && blockCells != 0);
            heapBlock = heapBlock->chainNext;
        } while (heapBlock != pHeap->allocHead);
    }
}

/*
 * One window row per allocated block of heap 0. Every row starts as a copy of
 * the heapWindowAllocRowTemplate template; pModuleName points at module[1] (module[0] holds the
 * owning heap id, see RgHeapFree) and pLine at the block's line field.
 */
void heapWindowAddAllocRows(int window)
{
    struct HeapWindowRow row = heapWindowAllocRowTemplate.row;
    struct RgHeap *pHeap = InstanceOfRgHeapById(0);
    struct RgHeapBlock *heapBlock = pHeap->allocHead;

    if (heapBlock != 0) {
        do {
            row = heapWindowAllocRowTemplate.row;
            row.pModuleName = &heapBlock->module[1];
            row.pLine = &heapBlock->line;
            testWindowAddRow(window, &row);
            heapBlock = heapBlock->chainNext;
        } while (heapBlock != pHeap->allocHead);
    }
}

/* Window callback registered by heapWindowOpen: calls testWindowClearRows on the window, then rebuilds its heap
 * contents with heapWindowAddAllocRows, the same call heapWindowOpen makes once when it opens the window. */
void heapWindowRefresh(int window)
{
    testWindowClearRows();
    heapWindowAddAllocRows(window);
}

extern char *heapWindowTitles[HEAP_MAX];
extern struct HeapWindowRowTemplate heapWindowRowTemplate;
extern char heapWindowEmptyModuleName[4];
extern const char heapWindowIdAssertText[];

void assertFailed(const char *pText, const char *pSourceFile, int line);

void heapWindowCreate(int parentWindow)
{
    struct WindowCreateParam windowSpec;
    struct HeapWindowRow row;
    u32 nId;
    int window;

    nId = (u32) testWindowGetUserData(parentWindow);
    if (nId >= HEAP_MAX) {
        assertFailed(heapWindowIdAssertText, heapSourcePath, 680);
    }
    window = testWindowFindByName(parentWindow, heapWindowTitles[nId]);
    if (window >= 0) {
        testWindowShow(window);
        return;
    }
    windowSpec.x = 100;
    windowSpec.y = 80;
    windowSpec.width = 256;
    windowSpec.height = 300;
    strcpy(windowSpec.name, heapWindowTitles[nId]);
    window = testWindowCreate(&windowSpec, parentWindow);
    row = heapWindowRowTemplate.row;
    row.pModuleName = heapWindowEmptyModuleName;
    row.heap.pHeap = InstanceOfRgHeapById(nId);
    testWindowAddRow(window, &row);
}

void heapWindowOpen(int parentWindow)
{
    const char *pName = heapWindowAllocTitle;
    struct WindowCreateParam windowSpec;
    int window;

    window = testWindowFindByName(parentWindow, pName);
    if (window >= 0) {
        testWindowShow(window);
    } else {
        windowSpec.x = HEAP_ALLOC_WINDOW_X;
        windowSpec.y = HEAP_ALLOC_WINDOW_Y;
        windowSpec.width = HEAP_ALLOC_WINDOW_WIDTH;
        windowSpec.height = HEAP_ALLOC_WINDOW_HEIGHT;
        strcpy(windowSpec.name, pName);
        window = testWindowCreate(&windowSpec, parentWindow);
        testWindowSetRefreshCallback(window, heapWindowRefresh);
        heapWindowAddAllocRows(window);
    }
}

extern char heapOverviewTitle[24];
extern char heapWindowTitleHeapZero[24];
extern char heapWindowTitleHeapOne[24];
extern char heapWindowTitleHeapTwo[24];
extern char heapWindowTitleHeapThree[24];
extern char heapWindowTitleSystem[24];
extern char heapWindowTitleMatrix[24];
extern unsigned char heapOverviewWindowColumnTable[0xa0];
extern struct HeapWindowRowTemplate heapOverviewRowTemplate;
extern const char heapOverviewWindowCloseFormat[32];

void testWindowSetColumnTable(int window, const unsigned char *pTable);
void func_AB46E8(const char *pText, int window);

void heapOverviewWindowOpen(int parentWindow)
{
    struct WindowCreateParam windowSpec;
    struct HeapWindowRow row;
    int window;

    window = testWindowFindByName(parentWindow, heapOverviewTitle);
    if (window >= 0) {
        testWindowShow(window);
        return;
    }
    windowSpec.x = 100;
    windowSpec.y = 100;
    windowSpec.width = 200;
    windowSpec.height = 140;
    strcpy(windowSpec.name, heapOverviewTitle);
    window = testWindowCreate(&windowSpec, parentWindow);
    testWindowSetColumnTable(window, heapOverviewWindowColumnTable);
    row = heapOverviewRowTemplate.row;
    row.pModuleName = heapWindowTitleHeapZero;
    row.heap.heapId = 0;
    testWindowAddRow(window, &row);
    row = heapOverviewRowTemplate.row;
    row.pModuleName = heapWindowTitleHeapOne;
    row.heap.heapId = 1;
    testWindowAddRow(window, &row);
    row = heapOverviewRowTemplate.row;
    row.pModuleName = heapWindowTitleHeapTwo;
    row.heap.heapId = 2;
    testWindowAddRow(window, &row);
    row = heapOverviewRowTemplate.row;
    row.pModuleName = heapWindowTitleHeapThree;
    row.heap.heapId = 3;
    testWindowAddRow(window, &row);
    row = heapOverviewRowTemplate.row;
    row.pModuleName = heapWindowTitleSystem;
    row.heap.heapId = 4;
    testWindowAddRow(window, &row);
    row = heapOverviewRowTemplate.row;
    row.pModuleName = heapWindowTitleMatrix;
    row.heap.heapId = 5;
    testWindowAddRow(window, &row);
    func_AB46E8(heapOverviewWindowCloseFormat, window);
}
