#include "common.h"
#include "shared.h"

/* A 128-bit store or load of the +0x30 analog group (a C store of a zero is `por $4,$0,$0; sq $4,0x30(pad)`;
 * QuadwordZero would drop the por and change the register allocation of xglPadRead). */
typedef unsigned int PadQuadword __attribute__((mode(TI)));

#include "main/tu007.h"

extern PadRecord PadData[2];

struct RgHeap;
struct RgHeap *InstanceOfRgHeapById(unsigned int heapId);

void InitRgHeap(struct RgHeap *pHeap, void *pMemory, int size);

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
 * before the block it returns (RgHeapDumpBlock reads it at pPtr - 0x40).
 * pre/next link the block into whichever circular list currently owns it
 * (RgHeap's free chain from _SetHeapHead, or its allocated chain from
 * _Link/_InsertNext), evidenced by "pre %p next %p\n". size is the block's
 * byte size and mark is set/cleared by _MarkAlloc/_UnmarkAlloc/_IsAllocated,
 * evidenced by "size %p mark %d\n" (size itself is dumped through %p in that
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
    struct RgHeapBlock *pre;
    struct RgHeapBlock *next;
    u32 size;
    u16 mark;
    u16 line;
    char module[32];
    char magic[16];
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
    int size;
    struct RgHeapBlock *freeHead;
    struct RgHeapBlock *allocHead;
    int mapStride;
};

int printf(const char *format, ...);

char *strcpy(char *destination, const char *source);

void RgHeapDump_sub(struct RgHeap *pHeap, const char *pComment, const char *pSourceFile, int line);

extern const char D_00542178[];
extern const char D_005421A0[];
extern const char D_005421B8[];
extern const char D_005421D8[];
extern const char D_005421E8[];
extern const char D_005421F8[];

void _Error(const char *pExpression, const void *unused, struct RgHeap *pHeap,
                   const char *pSourceFile, int line);

extern unsigned char *heapMagicStringPtr;
extern const char D_00542220[];
extern const char D_00542238[];

struct RgHeapBlock *_SetHeapHead(struct RgHeapBlock *pBlock, u32 size);

/* InstanceOfRgHeapById: the heap heapId (0x20 bytes apart); its 0x800-byte arena is set up by the first call, tracked by a word flag per id. */
extern unsigned char D_008E26D0[HEAP_MAX][0x20];

extern unsigned char D_008E2790[HEAP_MAX][HEAP_ARENA_SIZE];
extern int D_004C8B20[HEAP_MAX];

struct RgHeap *InstanceOfRgHeapById(u32 heapId);

extern const char D_00542268[];
extern char D_00542280[];

void *memset(void *destination, int value, unsigned int count);

int RgHeapIsInSelf(struct RgHeap *pHeap, void *pPtr);

extern const char heapDumpSeparatorFormat[];
extern const char heapDumpHeadFormat[];
extern const char heapDumpFreeMapFormat[];
extern const char heapDumpFreeBlockFormat[];
extern const char heapDumpAllocMapFormat[];
extern const char heapDumpAllocBlockFormat[];
extern const char heapDumpBlockHeaderFormat[];
extern const char heapDumpBlockLinksFormat[];
extern const char heapDumpBlockSizeMarkFormat[];
extern const char heapDumpBlockModuleFormat[];
extern const char heapDumpBlockNotInHeapFormat[];

void *testWindowGetUserData(int window);

struct HeapWindowInfo {
    unsigned char unmodeled_00[0xC];
    int rowCount;
};

void testWindowGetInfo(int window, struct HeapWindowInfo *pInfo);
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

extern char heapWindowAllocTitle[];
extern const char D_00542178[40];
extern const char D_005421A0[24];
extern const char D_005421B8[32];
extern const char D_005421D8[16];
extern const char D_005421E8[16];
extern const char D_005421F8[40];
extern const char D_00542220[24];
extern const char D_00542238[40];
extern const char D_00542268[24];
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

extern char D_00542280[8];

void _Error(const char *pExpression, const void *unused, struct RgHeap *pHeap,
                   const char *pSourceFile, int line)
{
    printf(D_00542178);
    printf(D_005421A0, pExpression, pSourceFile, line);
    if (pHeap != 0) {
        printf(D_005421B8, pHeap, pHeap->top, pHeap->size);
        RgHeapDump_sub(pHeap, D_005421D8, D_005421E8, 41);
    } else {
        printf(D_005421F8);
    }
    for (;;) {
    }
}

void _Link(struct RgHeapBlock *pBlock, struct RgHeapBlock *pPre,
                  struct RgHeapBlock *pNext)
{
    pBlock->pre = pPre;
    pBlock->next = pNext;
}

void _InsertPre(struct RgHeapBlock *pBlock, struct RgHeapBlock *pNew)
{
    _Link(pNew, pBlock->pre, pBlock);
    pBlock->pre->next = pNew;
    pBlock->pre = pNew;
}

void _InsertNext(struct RgHeapBlock *pBlock, struct RgHeapBlock *pNew)
{
    _Link(pNew, pBlock, pBlock->next);
    pBlock->next->pre = pNew;
    pBlock->next = pNew;
}

void _Unlink(struct RgHeapBlock *pBlock)
{
    pBlock->next->pre = pBlock->pre;
    pBlock->pre->next = pBlock->next;
}

void _MarkAlloc(struct RgHeapBlock *pBlock)
{
    pBlock->mark = 1;
}

void _UnmarkAlloc(struct RgHeapBlock *pBlock)
{
    pBlock->mark = 0;
}

int _IsAllocated(struct RgHeapBlock *pBlock)
{
    return pBlock->mark != 0;
}

/* Stamps the 15-byte magic (read through the heapMagicStringPtr pointer) into the block header. */
void _SetMagicString(struct RgHeapBlock *pBlock)
{
    char *pMagic = pBlock->magic;
    int index = 0;

    do {
        *pMagic = heapMagicStringPtr[index];
        index += 1;
        pMagic += 1;
    } while (index < HEAP_MAGIC_SIZE);
}

/* Returns 1 when the 15 magic bytes at block+0x30 equal the stamp heapMagicStringPtr points to, else 0. */
int _IsCollectMagicString(struct RgHeapBlock *pBlock)
{
    int index = 0;

    do {
        if ((unsigned char)pBlock->magic[index] != (signed char)heapMagicStringPtr[index]) {
            return 0;
        }
        index += 1;
    } while (index < HEAP_MAGIC_SIZE);
    return 1;
}

struct RgHeapBlock *_SetHeapHead(struct RgHeapBlock *pBlock, u32 size)
{
    if (size < HEAP_MIN_BLOCK_SIZE) {
        _Error(D_00542220, D_00542238, 0, D_005421E8, 149);
    }
    _Link(pBlock, pBlock, pBlock);
    pBlock->size = size;
    return pBlock;
}

/*
 * Walks the circular free list once and folds every block that ends exactly
 * where its successor starts into that block.
 */
void _MergeBlocks(struct RgHeapBlock *pHead)
{
    struct RgHeapBlock *pBlock = pHead;
    struct RgHeapBlock *pNext;

    if (pHead == 0) {
        return;
    }
    do {
        pNext = pBlock->next;
        while ((u8 *)pBlock + pBlock->size == (u8 *)pNext) {
            pBlock->next = pNext->next;
            pBlock->size += pNext->size;
            pNext->next->pre = pBlock;
            pNext = pBlock->next;
        }
        pBlock = pNext;
    } while (pBlock != pHead);
}

struct RgHeap *InstanceOfRgHeapById(u32 heapId)
{
    if (D_004C8B20[heapId] == 0) {
        D_004C8B20[heapId] = 1;
        InitRgHeap((struct RgHeap *)D_008E26D0[heapId], D_008E2790[heapId], HEAP_ARENA_SIZE);
    }
    return (struct RgHeap *)D_008E26D0[heapId];
}

void ClearRgHeap(struct RgHeap *pHeap)
{
    pHeap->freeHead = _SetHeapHead(pHeap->top, pHeap->size);
}

void InitRgHeap(struct RgHeap *pHeap, void *pMemory, int size)
{
    struct RgHeapBlock *pFreeBlock;

    if (size & 0xF) {
        _Error(D_00542268, D_00542280, 0, D_005421E8, 201);
    }
    memset(pMemory, 0, size);
    pHeap->size = size;
    pHeap->top = pMemory;
    pFreeBlock = _SetHeapHead(pMemory, size);
    pHeap->allocHead = 0;
    pHeap->freeHead = pFreeBlock;
    pHeap->mapStride = 0x100;
}

int GameMpeg2Play(char *destination, const char *format, ...);

extern const char D_00542288[];
extern const char D_005422B0[];
extern const char D_005422C0[];
extern const char D_005422D8[];
extern const char D_00542308[];
extern const char D_00542318[];
extern const char D_00542350[];
extern const char D_00542378[];
extern const char D_00542388[];
extern const char D_005423B8[];

void func_0030ED88(char *pDest, const char *pSrc, u32 count);

void *RgHeapAllocById(u32 heapId, u32 size, const char *pSourceFile, int line)
{
    char buf[32];
    struct RgHeap *pHeap;
    struct RgHeapBlock *pFreeHead;
    struct RgHeapBlock *pBlock;
    struct RgHeapBlock *pFit;
    struct RgHeapBlock *pSplit;
    u32 bestSize;
    u32 blockSize;
    u32 remainder;
    void *pPayload;

    pHeap = InstanceOfRgHeapById(heapId);
    if (size == 0) {
        printf(D_00542288, pSourceFile, line);
    }
    if (pHeap == 0) {
        _Error(D_005422B0, D_005422C0, 0, D_005421E8, 227);
    }
    pFreeHead = pHeap->freeHead;
    if (pFreeHead == 0) {
        printf(D_005422D8, pSourceFile, line);
        return 0;
    }
    pFit = 0;
    blockSize = ((size + 63) & -64) + sizeof(struct RgHeapBlock);
    bestSize = 0;
    pBlock = pFreeHead;
    do {
        if (pBlock->size >= blockSize) {
            if (pBlock->size < bestSize || bestSize == 0) {
                bestSize = pBlock->size;
                pFit = pBlock;
            }
        }
        pBlock = pBlock->next;
    } while (pBlock != pFreeHead);
    if (pFit == 0) {
        GameMpeg2Play(buf, D_00542308, heapId);
        RgHeapDump_sub(pHeap, buf, D_005421E8, 255);
        printf(D_00542318, size, pSourceFile, line);
        return 0;
    }
    if (bestSize < blockSize) {
        _Error(D_00542350, D_00542378, pHeap, D_005421E8, 265);
    }
    remainder = bestSize - blockSize;
    if (remainder >= HEAP_MIN_BLOCK_SIZE) {
        pSplit = (struct RgHeapBlock *)((u8 *)pFit + blockSize);
        _SetHeapHead(pSplit, remainder);
    } else {
        blockSize = bestSize;
        pSplit = 0;
    }
    if (pSplit != 0) {
        _InsertNext(pFit, pSplit);
    }
    if (pFit == pFreeHead) {
        pHeap->freeHead = pFit->next;
    }
    _Unlink(pFit);
    _SetHeapHead(pFit, blockSize);
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
    pPayload = pFit + 1;
    if (((u32)pPayload & 15) != 0) {
        _Error(D_00542388, D_005423B8, pHeap, D_005421E8, 309);
    }
    return pPayload;
}

extern const char D_005423D0[];
extern const char D_005423E0[];
extern const char D_005423F0[];
extern const char D_00542410[];
extern const char D_00542420[];
extern const char D_00542460[];
extern const char D_00542480[];
extern const char D_005424B8[];
extern const char D_005424E8[];

void RgHeapFree(void *pPtr, const char *pSourceFile, int line)
{
    struct RgHeapBlock *pBlock;
    struct RgHeap *pHeap;
    struct RgHeapBlock *pAllocHead;
    struct RgHeapBlock *pNext;
    struct RgHeapBlock *pCur;

    pBlock = (struct RgHeapBlock *)pPtr - 1;
    pHeap = InstanceOfRgHeapById(pBlock->module[0]);
    if (pHeap == 0) {
        _Error(D_005423D0, D_005423E0, 0, D_005421E8, 323);
    }
    if (pPtr == 0) {
        printf(D_005423F0, pSourceFile, line);
    }
    if (_IsAllocated(pBlock) == 0) {
        if (_IsCollectMagicString(pBlock) != 0) {
            RgHeapDump_sub(pHeap, D_00542410, D_005421E8, 336);
            printf(D_00542420, pPtr, pSourceFile, line, pBlock->module + 1, pBlock->line);
        } else {
            printf(D_00542460, pPtr, pSourceFile, line);
        }
    }
    if (_IsCollectMagicString(pBlock) == 0) {
        printf(D_00542480, pPtr, pSourceFile, line);
    }
    if (RgHeapIsInSelf(pHeap, pBlock) == 0) {
        printf(D_005424B8, pSourceFile, pHeap, pSourceFile, line);
    }
    pAllocHead = pHeap->allocHead;
    pNext = pAllocHead->next;
    if (pNext == pAllocHead) {
        if (pBlock != pNext) {
            printf(D_005424E8);
        }
        pHeap->allocHead = 0;
    } else {
        if (pAllocHead == pBlock) {
            _Unlink(pBlock);
            pHeap->allocHead = pNext;
        } else {
            _Unlink(pBlock);
        }
    }
    _UnmarkAlloc(pBlock);
    if (pHeap->freeHead == 0) {
        pHeap->freeHead = _SetHeapHead(pBlock, pBlock->size);
        return;
    }
    pCur = pHeap->freeHead;
    _Link(pBlock, 0, 0);
    do {
        if (pBlock < pCur) {
            _InsertPre(pCur, pBlock);
            if (pBlock < pHeap->freeHead) {
                pHeap->freeHead = pBlock;
            }
            break;
        }
        pCur = pCur->next;
    } while (pCur != pHeap->freeHead);
    if (pBlock->next == 0) {
        _InsertPre(pHeap->freeHead, pBlock);
    }
    _MergeBlocks(pHeap->freeHead);
}

int RgHeapIsInSelf(struct RgHeap *pHeap, void *pPtr)
{
    if (pPtr < pHeap->top || (char *)pPtr >= (char *)pHeap->top + pHeap->size) {
        return 0;
    }
    return 1;
}

int RgHeapIsInvalidMemory(struct RgHeap *pHeap, void *pPtr)
{
    struct RgHeapBlock *pBlock;

    if (pPtr == 0) {
        return 1;
    }
    pBlock = (struct RgHeapBlock *)pPtr - 1;
    if (_IsAllocated(pBlock) == 0) {
        return 1;
    }
    if (_IsCollectMagicString(pBlock) == 0) {
        return 1;
    }
    return RgHeapIsInSelf(pHeap, pBlock) == 0;
}

void RgHeapDump_sub(struct RgHeap *pHeap, const char *pComment, const char *pSourceFile, int line)
{
    struct RgHeapBlock *pBlock;

    pBlock = pHeap->freeHead;
    printf(heapDumpSeparatorFormat);
    printf(heapDumpHeadFormat, pHeap, pSourceFile, line);
    printf(heapDumpFreeMapFormat, pBlock, pComment);
    do {
        printf(heapDumpFreeBlockFormat, pBlock, pBlock->size, pBlock->size, pBlock->next);
        pBlock = pBlock->next;
    } while (pBlock != pHeap->freeHead);
    pBlock = pHeap->allocHead;
    printf(heapDumpAllocMapFormat, pBlock, pComment);
    if (pBlock != 0) {
        do {
            printf(heapDumpAllocBlockFormat, pBlock, pBlock->size - sizeof(struct RgHeapBlock), pBlock->size, pBlock->next,
                   pBlock->module + 1, pBlock->line);
            pBlock = pBlock->next;
        } while (pBlock != pHeap->allocHead);
    }
}

void RgHeapDumpBlock(struct RgHeap *pHeap, void *pPtr)
{
    struct RgHeapBlock *pBlock;

    printf(heapDumpBlockHeaderFormat);
    pBlock = (struct RgHeapBlock *) pPtr - 1;
    if (RgHeapIsInSelf(pHeap, pPtr) != 0) {
        printf(heapDumpBlockLinksFormat, pBlock->pre, pBlock->next);
        printf(heapDumpBlockSizeMarkFormat, pBlock->size, pBlock->mark);
        printf(heapDumpBlockModuleFormat, pBlock->module + 1, pBlock->line);
        return;
    }
    printf(heapDumpBlockNotInHeapFormat, pHeap);
}

/* Formats the heap id (D_00542678) and dumps the heap under that comment. */
int GameMpeg2Play(char *destination, const char *format, ...);

extern const char D_00542678[16];

void RgHeapDumpById(u32 heapId)
{
    char comment[0x80];
    struct RgHeap *pHeap;

    if (heapId < HEAP_MAX) {
        pHeap = InstanceOfRgHeapById(heapId);
        GameMpeg2Play(comment, D_00542678, heapId);
        RgHeapDump_sub(pHeap, comment, D_005421E8, 491);
    }
}

void textScaleApplyOptions(int window)
{
    struct RgHeap *pHeap;
    u16 flags;

    pHeap = testWindowGetUserData(window);
    flags = PadData[0].pressed;
    if ((flags & TEXT_SCALE_OPTION_DOUBLE) != 0) {
        pHeap->mapStride = pHeap->mapStride * 2;
    }
    if ((flags & TEXT_SCALE_OPTION_HALF) != 0) {
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
 * info.rowCount.
 */
void heapWindowPaintMap(int window)
{
    struct HeapWindowInfo info;
    struct RgHeap *pHeap;
    struct RgHeapBlock *pBlock;
    int arenaCells;
    int blockRow;
    u32 blockCells;
    int mapRow = 1;
    u32 blockCol;
    u32 runCells;
    u32 byteOffset;
    u32 cellIndex;

    pHeap = testWindowGetUserData(window);
    testWindowGetInfo(window, &info);

    arenaCells = (u32)pHeap->size / pHeap->mapStride;
    if (arenaCells != 0) {
        do {
            if (arenaCells >= pHeap->mapStride) {
                testWindowDrawLine(window, 0, mapRow, HEAP_MAP_WIDTH, mapRow, HEAP_MAP_ARENA_COLOR);
                arenaCells -= pHeap->mapStride;
            } else {
                testWindowDrawLine(window, 0, mapRow, (arenaCells << 8) / pHeap->mapStride, mapRow, HEAP_MAP_ARENA_COLOR);
                arenaCells = 0;
            }
            mapRow += 1;
        } while (mapRow < info.rowCount && arenaCells != 0);
    }

    pBlock = pHeap->allocHead;
    if (pBlock != 0) {
        do {
            byteOffset = (u32)pBlock - (u32)pHeap->top;
            cellIndex = byteOffset / pHeap->mapStride;
            blockCells = pBlock->size / pHeap->mapStride;
            blockRow = cellIndex / pHeap->mapStride;
            blockCol = cellIndex % pHeap->mapStride;

            runCells = pHeap->mapStride - blockCol;
            if (blockCells < runCells) {
                runCells = blockCells;
            }
            testWindowDrawLine(window, blockCol, blockRow, ((blockCol + runCells) << 8) / pHeap->mapStride,
                        blockRow, HEAP_MAP_BLOCK_COLOR);
            blockCells -= runCells;
            blockRow += 1;

            do {
                runCells = pHeap->mapStride;
                if (blockCells < runCells) {
                    runCells = blockCells;
                }
                testWindowDrawLine(window, 0, blockRow, (runCells << 8) / pHeap->mapStride, blockRow, HEAP_MAP_BLOCK_COLOR);
                blockCells -= runCells;
                blockRow += 1;
            } while (blockRow <= info.rowCount && blockCells != 0);
            pBlock = pBlock->next;
        } while (pBlock != pHeap->allocHead);
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
    struct RgHeapBlock *pBlock = pHeap->allocHead;

    if (pBlock != 0) {
        do {
            row = heapWindowAllocRowTemplate.row;
            row.pModuleName = &pBlock->module[1];
            row.pLine = &pBlock->line;
            testWindowAddRow(window, &row);
            pBlock = pBlock->next;
        } while (pBlock != pHeap->allocHead);
    }
}

/* Window callback registered by heapWindowOpen: calls testWindowClearRows on the window, then rebuilds its heap
 * contents with heapWindowAddAllocRows, the same call heapWindowOpen makes once when it opens the window. */
void heapWindowRefresh(int window)
{
    testWindowClearRows();
    heapWindowAddAllocRows(window);
}

extern char *D_004C8BE0[HEAP_MAX];
extern struct HeapWindowRowTemplate D_004C8E80;
extern char D_005512B0[8];
extern const char D_00542688[];

void assertFailed(const char *pText, const char *pSourceFile, int line);

void heapWindowCreate(int parentWindow)
{
    struct WindowCreateParam param;
    struct HeapWindowRow row;
    u32 nId;
    int window;

    nId = (u32) testWindowGetUserData(parentWindow);
    if (nId >= HEAP_MAX) {
        assertFailed(D_00542688, D_005421E8, 680);
    }
    window = testWindowFindByName(parentWindow, D_004C8BE0[nId]);
    if (window >= 0) {
        testWindowShow(window);
        return;
    }
    param.x = 100;
    param.y = 80;
    param.width = 256;
    param.height = 300;
    strcpy(param.name, D_004C8BE0[nId]);
    window = testWindowCreate(&param, parentWindow);
    row = D_004C8E80.row;
    row.pModuleName = D_005512B0;
    row.heap.pHeap = InstanceOfRgHeapById(nId);
    testWindowAddRow(window, &row);
}

void heapWindowOpen(int parentWindow)
{
    const char *pName = heapWindowAllocTitle;
    struct WindowCreateParam param;
    int window;

    window = testWindowFindByName(parentWindow, pName);
    if (window >= 0) {
        testWindowShow(window);
    } else {
        param.x = HEAP_ALLOC_WINDOW_X;
        param.y = HEAP_ALLOC_WINDOW_Y;
        param.width = HEAP_ALLOC_WINDOW_WIDTH;
        param.height = HEAP_ALLOC_WINDOW_HEIGHT;
        strcpy(param.name, pName);
        window = testWindowCreate(&param, parentWindow);
        testWindowSetRefreshCallback(window, heapWindowRefresh);
        heapWindowAddAllocRows(window);
    }
}

extern char D_004C8B38[24];
extern char D_004C8B50[24];
extern char D_004C8B68[24];
extern char D_004C8B80[24];
extern char D_004C8B98[24];
extern char D_004C8BB0[24];
extern char D_004C8BC8[24];
extern unsigned char D_004C8D40[0xa0];
extern struct HeapWindowRowTemplate D_004C8DE0;
extern const char D_005426A8[32];

void testWindowSetColumnTable(int window, const unsigned char *pTable);
void func_AB46E8(const char *pText, int window);

void heapOverviewWindowOpen(int parentWindow)
{
    struct WindowCreateParam param;
    struct HeapWindowRow row;
    int window;

    window = testWindowFindByName(parentWindow, D_004C8B38);
    if (window >= 0) {
        testWindowShow(window);
        return;
    }
    param.x = 100;
    param.y = 100;
    param.width = 200;
    param.height = 140;
    strcpy(param.name, D_004C8B38);
    window = testWindowCreate(&param, parentWindow);
    testWindowSetColumnTable(window, D_004C8D40);
    row = D_004C8DE0.row;
    row.pModuleName = D_004C8B50;
    row.heap.heapId = 0;
    testWindowAddRow(window, &row);
    row = D_004C8DE0.row;
    row.pModuleName = D_004C8B68;
    row.heap.heapId = 1;
    testWindowAddRow(window, &row);
    row = D_004C8DE0.row;
    row.pModuleName = D_004C8B80;
    row.heap.heapId = 2;
    testWindowAddRow(window, &row);
    row = D_004C8DE0.row;
    row.pModuleName = D_004C8B98;
    row.heap.heapId = 3;
    testWindowAddRow(window, &row);
    row = D_004C8DE0.row;
    row.pModuleName = D_004C8BB0;
    row.heap.heapId = 4;
    testWindowAddRow(window, &row);
    row = D_004C8DE0.row;
    row.pModuleName = D_004C8BC8;
    row.heap.heapId = 5;
    testWindowAddRow(window, &row);
    func_AB46E8(D_005426A8, window);
}
