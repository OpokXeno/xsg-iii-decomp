#include "common.h"

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;

typedef struct PagedTable {
    u16 cursor;           /* 0x00 */
    u16 filledEntryCount;         /* 0x02 */
    u16 pageSize;         /* 0x04 */
    u16 altPageSize;      /* 0x06 */
    s32 unk_08[4];        /* 0x08 */
    s32 headerSize;       /* 0x18 */
    s32 headerOffset;     /* 0x1C */
    s32 unk_20;           /* 0x20 */
    void **entryPointers;      /* 0x24 */
    s16 *entryKeys;            /* 0x28 */
    s8 *entrySlotBytes;        /* 0x2C */
    s8 *entryFlags;            /* 0x30 */
    u16 capacity;         /* 0x34 */
    u16 pending;          /* 0x36 */
    s32 unk_38;           /* 0x38 */
} PagedTable;

void func_00235E58(PagedTable *pagedTable, s32 count);
void func_00235EB8(PagedTable *pagedTable, s32 cursor);
void func_00236AC8(PagedTable *pagedTable);
s32 func_00236570(PagedTable *pagedTable, s32 index);

void pagedTableResetCounters(PagedTable *pagedTable) {
    pagedTable->cursor = 0;
    pagedTable->filledEntryCount = 0;
    pagedTable->pageSize = 0;
    pagedTable->altPageSize = 0;
}

void pagedTableSetLayout(PagedTable *pagedTable, s32 cursor, s32 filledEntryCount, s32 pageSize, s32 altPageSize) {
    pagedTable->pageSize = pageSize;
    pagedTable->altPageSize = altPageSize;
    if ((cursor == 0) && (filledEntryCount == 0)) {
        pagedTable->cursor = 0;
        pagedTable->filledEntryCount = 0;
        return;
    }
    func_00235E58(pagedTable, filledEntryCount);
    func_00235EB8(pagedTable, cursor);
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00235E58);

void pagedTableIncrementCount(PagedTable *pagedTable) {
    s32 filledCount;

    filledCount = pagedTable->filledEntryCount;
    if (filledCount <= 0xFFFEU) {
        pagedTable->filledEntryCount = filledCount + 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00235EB8);

u16 pagedTableGetPageLength(PagedTable *pagedTable) {
    s32 pageLength;

    pageLength = pagedTable->pageSize;
    if (pageLength == 0) {
        pageLength = pagedTable->filledEntryCount;
    }
    return pageLength;
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00235EF0);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00235F20);

u32 pagedTableGetPageOffset(PagedTable *pagedTable)
{
  if (pagedTable->pageSize != 0)
  {
    return pagedTable->cursor % pagedTable->pageSize;
  }
  return pagedTable->cursor;
}

s32 pagedTableGetPageCount(PagedTable *pagedTable) {
    if (pagedTable->pageSize != 0) {
        return ((pagedTable->filledEntryCount - 1) / pagedTable->pageSize) + 1;
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00235FD0);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_002363E0);

extern s32 gPagedTablePoolCounter;

void pagedTablePoolResetCounter(void) {
    gPagedTablePoolCounter = 0;
}

void pagedTablePoolResetCounter(void);
extern s32 gPagedTablePoolHandle;
s32 memPoolAlloc(s32 size);

void pagedTablePoolInit(void) {
    if (gPagedTablePoolHandle == 0) {
        gPagedTablePoolHandle = memPoolAlloc(0x4000);
    }
    pagedTablePoolResetCounter();
}

s32 memPoolFree(s32);

s32 pagedTablePoolShutdown(void) {
    s32 pagedPoolBlock;

    pagedTablePoolResetCounter();
    pagedPoolBlock = gPagedTablePoolHandle;
    if (pagedPoolBlock != 0) {
        pagedPoolBlock = memPoolFree(pagedPoolBlock);
    }
    gPagedTablePoolHandle = 0;
    return pagedPoolBlock;
}

void pagedTableResetCounters(PagedTable *pagedTable);
void *func_002363E0(s32 size);

void pagedTableInit(PagedTable *pagedTable, s32 capacity)
{
    extern void memset(void *dst, s32 value, s32 size);
    s32 pointerArrayBytes;
    s32 entryKeyBytes;
    s16 *entryKeyTable;
    s8 *slotByteArray;
    s8 *flagArray;
    void **pointerArray;
    s32 initialHeaderOffset;

    pointerArrayBytes = capacity * 4;
    do {
        entryKeyBytes = capacity * 2;
    } while (0);
    pagedTableResetCounters(pagedTable);
    pagedTable->capacity = capacity;
    if (capacity != 0)
    {
        pointerArray = func_002363E0(pointerArrayBytes);
        pagedTable->entryPointers = pointerArray;
        entryKeyTable = func_002363E0(entryKeyBytes);
        pagedTable->entryKeys = entryKeyTable;
        slotByteArray = func_002363E0(capacity);
        pagedTable->entrySlotBytes = slotByteArray;
        flagArray = func_002363E0(capacity);
        pagedTable->entryFlags = flagArray;
        memset(pagedTable->entryPointers, 0, pointerArrayBytes);
        memset(pagedTable->entryKeys, 0, entryKeyBytes);
        memset(pagedTable->entrySlotBytes, 0, capacity);
        memset(pagedTable->entryFlags, 0, capacity);
    }
    pagedTable->unk_38 = 0;
    pagedTable->unk_20 = 0;
    pagedTable->unk_08[0] = 0;
    pagedTable->unk_08[1] = 0;
    pagedTable->unk_08[2] = 0;
    initialHeaderOffset = 0x18;
    pagedTable->headerSize = 0x40;
    pagedTable->unk_08[3] = 0;
    pagedTable->pending = 0;
    pagedTable->headerOffset = initialHeaderOffset;
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00236570);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_002365B8);

void pagedTableIncrementCount(PagedTable *pagedTable);

void pagedTableAppend(PagedTable *pagedTable, s8 *appendedPointer, s32 entryKey, s32 reserved) {
    s32 storedKey;
    u16 appendIndex;

    storedKey = entryKey & 0xFFFF;
    if (pagedTable->capacity != 0) {
        appendIndex = pagedTable->filledEntryCount;
        pagedTable->entryKeys[appendIndex] = storedKey;
        pagedTable->entryPointers[appendIndex] = appendedPointer;
        pagedTable->entryFlags[pagedTable->filledEntryCount] = 0;
    }
    pagedTableIncrementCount(pagedTable);
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_002366A0);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_002366D8);

void pagedTableSetCount(PagedTable *pagedTable, s32 filledEntryCount) {
    s32 currentCursor;

    pagedTable->filledEntryCount = filledEntryCount;
    if (filledEntryCount <= 0) {
        pagedTable->cursor = 0;
    } else {
        currentCursor = pagedTable->cursor;
        if (currentCursor >= filledEntryCount) {
            pagedTable->cursor = filledEntryCount - 1;
        }
    }
    func_00236AC8(pagedTable);
}

u16 pagedTableSeek(PagedTable *pagedTable, s32 cursor) {
    extern u16 memset(void *, s32, u16);
    u16 previous;
    u16 cleared;
    u16 seekResult;

    previous = pagedTable->cursor;
    func_00235EB8(pagedTable, cursor & 0xFFFF);
    func_00236AC8(pagedTable);
    seekResult = pagedTable->cursor;
    if (seekResult != previous) {
        seekResult = pagedTable->capacity;
        if (seekResult != 0) {
            cleared = memset(pagedTable->entrySlotBytes, 0, seekResult);
            seekResult = cleared;
        }
    }
    return seekResult;
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00236940);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00236A40);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00236AC8);

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00236B90);

s32 func_00236570();

void pagedTableSetSlotByte(PagedTable *pagedTable, s32 entryIndex, s32 slotByte) {
    if (pagedTable->capacity != 0) {
        pagedTable->entrySlotBytes[func_00236570(pagedTable, entryIndex)] = slotByte;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu185", func_00236DE8);

void *pagedTableGetEntryAt(PagedTable *pagedTable, s32 entryIndex) {
    if (pagedTable->capacity != 0) {
        if (entryIndex < pagedTable->filledEntryCount) {
            return pagedTable->entryPointers[entryIndex];
        }
        return 0;
    }
    return 0;
}

void *pagedTableGetCurrentEntry(PagedTable *pagedTable) {
    if (pagedTable->capacity != 0) {
        return pagedTable->entryPointers[pagedTable->cursor];
    }
    return 0;
}

u16 pagedTableGetKeyAt(PagedTable *pagedTable, s32 entryIndex) {
    if (pagedTable->capacity != 0) {
        return pagedTable->entryKeys[entryIndex];
    }
    return 0U;
}

u8 pagedTableGetSlotByte(PagedTable *pagedTable, s32 entryIndex) {
    if (pagedTable->capacity != 0) {
        return pagedTable->entrySlotBytes[func_00236570(pagedTable, entryIndex)];
    }
    return ((pagedTable->cursor ^ entryIndex) != 0) ? 0U : 0x80U;
}

u8 pagedTableGetFlagByte(PagedTable *pagedTable, s32 entryIndex) {
    if (pagedTable->capacity != 0) {
        return pagedTable->entryFlags[entryIndex];
    }
    return 0x80U;
}

u16 pagedTableGetPageLength(PagedTable *pagedTable);

s32 pagedTableGetStatus(PagedTable *pagedTable) {
    s32 statusBits;
    s32 pending;

    pending = pagedTable->pending;
    statusBits = pending != 0;
    return ((pagedTable->pending + pagedTableGetPageLength(pagedTable)) < pagedTable->filledEntryCount) ? (statusBits | 2) : statusBits;
}

void func_00221C50();

s32 pagedTableFlush(void) {
    func_00221C50();
    return 1;
}
