#include "common.h"
#include "main/tu176.h"

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
    float wavePhase;      /* 0x38 */
} PagedTable;

void pagedTableClampFilledCount(PagedTable *pagedTable, s32 count);
void pagedTableSetCursor(PagedTable *pagedTable, s32 cursor);
void pagedTableUpdatePending(PagedTable *pagedTable);
s32 pagedTableGetPageRelativeIndex(PagedTable *pagedTable, s32 entryIndex);

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
    pagedTableClampFilledCount(pagedTable, filledEntryCount);
    pagedTableSetCursor(pagedTable, cursor);
}

/* Clamps the filled entry count to 0..0xFFFF and pulls the cursor back inside it. */
void pagedTableClampFilledCount(PagedTable *pagedTable, s32 count) {
    if (count < 0) {
        count = 0;
    } else if (count > 0xFFFF) {
        count = 0xFFFF;
    }
    pagedTable->filledEntryCount = count;
    if (pagedTable->cursor >= (u16) count) {
        pagedTable->cursor = (u16) count - 1;
    }
}

void pagedTableIncrementCount(PagedTable *pagedTable) {
    s32 filledCount;

    filledCount = pagedTable->filledEntryCount;
    if (filledCount <= 0xFFFEU) {
        pagedTable->filledEntryCount = filledCount + 1;
    }
}

/* Stores a new cursor clamped to the filled entries (0 .. filled - 1). */
void pagedTableSetCursor(PagedTable *pagedTable, s32 cursor) {
    s32 filledCount;

    filledCount = pagedTable->filledEntryCount;
    if (cursor >= filledCount) {
        cursor = filledCount - 1;
    }
    if (cursor < 0) {
        cursor = 0;
    }
    pagedTable->cursor = cursor;
}

u16 pagedTableGetPageLength(PagedTable *pagedTable) {
    s32 pageLength;

    pageLength = pagedTable->pageSize;
    if (pageLength == 0) {
        pageLength = pagedTable->filledEntryCount;
    }
    return pageLength;
}

/* Returns the cursor's page index (cursor / pageSize), or 0 when there is no page size. */
u32 pagedTableGetPageIndex(PagedTable *pagedTable) {
    if (pagedTable->pageSize != 0) {
        return (u32) pagedTable->cursor / (u32) pagedTable->pageSize;
    }
    return 0;
}

/* Returns the first entry index of the cursor's page (page index * pageSize), or 0 without a page size. */
u32 pagedTableGetPageStart(PagedTable *pagedTable) {
    if (pagedTable->pageSize != 0) {
        return pagedTableGetPageIndex(pagedTable) * pagedTable->pageSize;
    }
    return 0;
}

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

u16 *getStatusHeader(void);
s32 statusHeaderIsReadyMode20(void);
s32 statusHeaderIsReadyMode40(void);
u16 pagedTableGetPageLength(PagedTable *pagedTable);

/*
 * Applies the navigation bits of the status header to the cursor and returns the status bits.
 * *statusOut is cursor + 1 for mode 20, ~cursor for mode 40, 0 otherwise.
 */
s32 pagedTableUpdateCursorFromStatus(PagedTable *pagedTable, s32 *statusOut) {
    s32 statusBits = 0;
    s32 maskLow;
    s32 maskHigh;
    s32 pageLength;
    s32 firstIndex;
    s32 lastIndex;
    s32 altEven;
    s32 altFlags;
    s32 pageSteps;
    s32 headerBitsA;
    s32 headerBitsB;

    altFlags = pagedTable->altPageSize;
    altEven = (altFlags ^ 1) & 1;
    pageLength = pagedTableGetPageLength(pagedTable);
    if (((pagedTable->altPageSize & 0x40) == 0) && (pagedTable->filledEntryCount != 0)) {
        if ((pagedTable->altPageSize & 8) != 0) {
            maskLow = 0x8000;
            maskHigh = 0x2000;
        } else {
            maskLow = 0x1000;
            maskHigh = 0x4000;
        }
        headerBitsA = 0;
        headerBitsB = 0;
        if ((pagedTable->altPageSize & 0x10) != 0) {
            if ((pagedTable->altPageSize & 8) != 0) {
                s32 headerWordA;
                s32 headerWordB;
                headerWordA = getStatusHeader()[2];
                headerBitsA = headerWordA & 0x1000;
                headerWordB = getStatusHeader()[2];
                headerBitsB = headerWordB & 0x4000;
            } else {
                s32 headerWordA;
                s32 headerWordB;
                headerWordA = getStatusHeader()[2];
                headerBitsA = headerWordA & 0x8000;
                headerWordB = getStatusHeader()[2];
                headerBitsB = headerWordB & 0x2000;
            }
        }
        if ((pagedTable->altPageSize & 0x20) != 0) {
            headerBitsA |= getStatusHeader()[5] & 0x90;
            headerBitsB |= getStatusHeader()[5] & 0x60;
        }
        if (pageLength >= pagedTable->filledEntryCount) {
            headerBitsA = 0;
            headerBitsB = 0;
        }
        if ((pagedTable->altPageSize & 4) != 0) {
            firstIndex = pagedTableGetPageStart(pagedTable);
            lastIndex = firstIndex + pagedTableGetPageLength(pagedTable) - 1;
            if (lastIndex >= pagedTable->filledEntryCount) {
                lastIndex = pagedTable->filledEntryCount - 1;
            }
            if ((getStatusHeader()[2] & maskLow) != 0 && firstIndex < pagedTable->cursor) {
                pagedTable->cursor = pagedTable->cursor + 0xFFFF;
                statusBits = 1;
            } else if ((getStatusHeader()[1] & maskLow) != 0) {
                pagedTable->cursor = lastIndex;
                statusBits = 1;
            }
            if ((getStatusHeader()[2] & maskHigh) != 0 && pagedTable->cursor < lastIndex) {
                statusBits |= 1;
                pagedTable->cursor = pagedTable->cursor + 1;
            } else if ((getStatusHeader()[1] & maskHigh) != 0) {
                pagedTable->cursor = firstIndex;
                statusBits |= 1;
            }
        } else {
            if ((getStatusHeader()[2] & maskLow) != 0 && pagedTable->cursor != 0) {
                pagedTable->cursor = pagedTable->cursor + 0xFFFF;
                statusBits = 1;
            } else if (altEven != 0 && (getStatusHeader()[1] & maskLow) != 0) {
                pagedTable->cursor = pagedTable->filledEntryCount + 0xFFFF;
                statusBits = 1;
            }
            if ((getStatusHeader()[2] & maskHigh) != 0 &&
                pagedTable->cursor < pagedTable->filledEntryCount - 1) {
                pagedTable->cursor = pagedTable->cursor + 1;
                statusBits |= 1;
            } else if (altEven != 0 && (getStatusHeader()[1] & maskHigh) != 0) {
                pagedTable->cursor = 0;
                statusBits |= 1;
            }
        }
        if (headerBitsA != 0) {
            if (pagedTable->cursor - pageLength >= 0) {
                pagedTable->cursor = pagedTable->cursor - pageLength;
            } else {
                pagedTable->cursor = 0;
            }
            statusBits |= 2;
        }
        if (headerBitsB != 0) {
            pageSteps = (pagedTable->cursor + pageLength) / pageLength;
            if (pagedTableGetPageCount(pagedTable) - 1 >= pageSteps) {
                s32 nextCursor = pagedTable->cursor + pageLength;
                if (pagedTable->filledEntryCount - 1 >= nextCursor) {
                    pagedTable->cursor = nextCursor;
                } else {
                    pagedTable->cursor = pagedTable->filledEntryCount + 0xFFFF;
                }
            } else {
                pagedTable->cursor = pagedTable->filledEntryCount + 0xFFFF;
            }
            statusBits |= 2;
        }
    }
    *statusOut = 0;
    if (statusHeaderIsReadyMode20() != 0 && (pagedTable->altPageSize & 0x80) == 0) {
        statusBits |= 4;
        *statusOut = pagedTable->cursor + 1;
    } else if (statusHeaderIsReadyMode40() != 0 && (pagedTable->altPageSize & 0x100) == 0) {
        statusBits |= 8;
        *statusOut = ~pagedTable->cursor;
    }
    return statusBits;
}

extern s32 gPagedTablePoolHandle;
static s32 gPagedTablePoolCounter;
void *pagedTablePoolAlloc(s32 size);

/* Bump allocator inside the table pool: returns the next block of `bytes` (rounded up to 4) or 0 for size 0. */
void *pagedTablePoolAlloc(s32 poolBytes) {
    s32 poolBlock;

    if (poolBytes == 0) {
        return 0;
    }
    poolBlock = gPagedTablePoolHandle + gPagedTablePoolCounter;
    gPagedTablePoolCounter = (gPagedTablePoolCounter + poolBytes + 3) & ~3;
    return (void *) poolBlock;
}

void pagedTablePoolResetCounter(void) {
    gPagedTablePoolCounter = 0;
}

void pagedTablePoolResetCounter(void);
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
        pointerArray = pagedTablePoolAlloc(pointerArrayBytes);
        pagedTable->entryPointers = pointerArray;
        entryKeyTable = pagedTablePoolAlloc(entryKeyBytes);
        pagedTable->entryKeys = entryKeyTable;
        slotByteArray = pagedTablePoolAlloc(capacity);
        pagedTable->entrySlotBytes = slotByteArray;
        flagArray = pagedTablePoolAlloc(capacity);
        pagedTable->entryFlags = flagArray;
        memset(pagedTable->entryPointers, 0, pointerArrayBytes);
        memset(pagedTable->entryKeys, 0, entryKeyBytes);
        memset(pagedTable->entrySlotBytes, 0, capacity);
        memset(pagedTable->entryFlags, 0, capacity);
    }
    pagedTable->wavePhase = 0;
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

/*
 * Maps an entry index to its slot in the current page (index - pending, clamped to the page).
 * With flag 2 set in unk_20 the index is returned unchanged.
 */
s32 pagedTableGetPageRelativeIndex(PagedTable *pagedTable, s32 entryIndex) {
    s32 pageOffset;

    pageOffset = entryIndex - pagedTable->pending;
    if ((pagedTable->unk_20 & 2) != 0) {
        return entryIndex;
    }
    if (pageOffset < 0) {
        pageOffset = 0;
    } else if (pageOffset >= pagedTable->pageSize) {
        pageOffset = pagedTable->pageSize - 1;
    }
    return pageOffset;
}

/* Sets the page sizes, then resets the header, the pending page, the counters and the slot bytes. */
void pagedTableSetPageSizes(PagedTable *pagedTable, s32 pageSize, s32 altPageSize) {
    extern void memset(void *dst, s32 value, s32 size);

    pagedTableSetLayout(pagedTable, 0, 0, (u16) pageSize, (u16) altPageSize);
    pagedTable->headerOffset = 0x18;
    pagedTable->headerSize = 0x40;
    pagedTable->unk_20 = 0;
    pagedTable->unk_08[0] = 0;
    pagedTable->unk_08[1] = 0;
    pagedTable->unk_08[2] = 0;
    pagedTable->unk_08[3] = 0;
    pagedTable->pending = 0;
    pagedTable->wavePhase = 0;
    if (pagedTable->capacity != 0) {
        memset(pagedTable->entrySlotBytes, 0, pagedTable->capacity);
    }
}

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

/* Stores an entry pointer and its key (masked to 16 bits) at one index; only when the table has capacity. */
void pagedTableSetEntry(PagedTable *pagedTable, s32 entryIndex, void *entryPointer, s32 entryKey) {
    s32 storedKey;

    storedKey = entryKey & 0xFFFF;
    if (pagedTable->capacity != 0) {
        pagedTable->entryPointers[entryIndex] = entryPointer;
        pagedTable->entryKeys[entryIndex] = storedKey;
    }
}

s32 xglFontGetFormattedWidth(void *entry);

/*
 * Lays out the text box of the table: the extent is the widest font entry when width is 0, the span is
 * the page length times pageRows; unk_08 is moved by the alignment bits (low nibble horizontal, high vertical).
 */
void pagedTableSetTextBox(PagedTable *pagedTable, u8 alignment, s32 width, s32 pageRows) {
    s32 extent = 0;
    s32 span;
    s32 i;
    s32 entryWidth;
    s32 horizontal;
    s32 vertical;

    pagedTable->unk_08[3] = 0;
    pagedTable->unk_08[2] = 0;
    pagedTable->unk_08[1] = 0;
    pagedTable->unk_08[0] = 0;
    if ((pagedTable->altPageSize & 8) != 0) {
        span = pageRows;
        extent = pagedTableGetPageLength(pagedTable) * width;
    } else {
        span = pagedTableGetPageLength(pagedTable) * pageRows;
        if (pagedTable->capacity != 0 && width == 0) {
            i = 0;
            if (pagedTable->filledEntryCount > width) {
                do {
                    entryWidth = 0;
                    if ((pagedTable->entryKeys[i] & 7) == 1) {
                        entryWidth = xglFontGetFormattedWidth(pagedTable->entryPointers[i]);
                    }
                    if (extent < entryWidth) {
                        extent = entryWidth;
                    }
                    i++;
                } while (i < pagedTable->filledEntryCount);
            }
            width = extent;
        } else {
            extent = width;
        }
    }

    horizontal = alignment & 3;
    switch (horizontal) {
    case 0:
        break;
    case 1:
        pagedTable->unk_08[0] = pagedTable->unk_08[0] - (extent >> 1);
        break;
    case 2:
        pagedTable->unk_08[0] = pagedTable->unk_08[0] - extent;
        break;
    }
    pagedTable->unk_08[2] = extent;

    vertical = alignment >> 4;
    switch (vertical) {
    case 0:
        break;
    case 1:
        pagedTable->unk_08[1] = pagedTable->unk_08[1] - (span >> 1);
        break;
    case 2:
        pagedTable->unk_08[1] = pagedTable->unk_08[1] - span;
        break;
    }
    pagedTable->unk_08[3] = span;
    pagedTable->headerOffset = pageRows;
    pagedTable->headerSize = width;
}

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
    pagedTableUpdatePending(pagedTable);
}

u16 pagedTableSeek(PagedTable *pagedTable, s32 cursor) {
    extern u16 memset(void *, s32, u16);
    u16 previous;
    u16 cleared;
    u16 seekResult;

    previous = pagedTable->cursor;
    pagedTableSetCursor(pagedTable, cursor & 0xFFFF);
    pagedTableUpdatePending(pagedTable);
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

/* Moves the cursor by delta over the filled entries; wraps unless altPageSize bit 0 clamps it. */
void pagedTableMoveCursor(PagedTable *pagedTable, s32 delta) {
    s32 newCursor;

    if (pagedTable->filledEntryCount == 0) {
        pagedTableSeek(pagedTable, 0);
        return;
    }
    newCursor = pagedTable->cursor + delta;
    if (newCursor < 0) {
        if ((pagedTable->altPageSize & 1) != 0) {
            newCursor = 0;
        } else {
            do {
                newCursor += pagedTable->filledEntryCount;
            } while (newCursor < 0);
            newCursor = newCursor % pagedTable->filledEntryCount;
        }
    }
    if (newCursor >= pagedTable->filledEntryCount) {
        if ((pagedTable->altPageSize & 1) != 0) {
            newCursor = pagedTable->filledEntryCount - 1;
        } else {
            do {
                newCursor -= pagedTable->filledEntryCount;
            } while (newCursor >= pagedTable->filledEntryCount);
        }
    }
    pagedTableSetCursor(pagedTable, newCursor);
    pagedTableUpdatePending(pagedTable);
}

u16 pagedTableGetKeyAt(PagedTable *pagedTable, s32 entryIndex);

/* Steps the cursor by delta up to `count` times, stopping at the first entry whose key has no bit of mask. */
void pagedTableStepCursorUnmasked(PagedTable *pagedTable, s32 delta, s32 mask) {
    s32 i;
    s32 count = pagedTable->filledEntryCount;

    for (i = 0; i < count; i++) {
        pagedTableMoveCursor(pagedTable, delta);
        if (!(pagedTableGetKeyAt(pagedTable, pagedTable->cursor) & mask)) {
            break;
        }
    }
}

/* Recomputes the pending index (first visible entry) from the cursor and the page length. */
void pagedTableUpdatePending(PagedTable *pagedTable) {
    if ((pagedTable->unk_20 & 4) != 0) {
        s32 limit = pagedTable->filledEntryCount - pagedTableGetPageLength(pagedTable);
        if (pagedTable->pending >= limit && pagedTable->cursor >= limit) {
            if (limit < 0) {
                pagedTable->pending = 0;
            } else {
                pagedTable->pending = limit;
            }
        } else {
            if (pagedTable->cursor < pagedTable->pending ||
                pagedTable->cursor >= pagedTable->pending + pagedTableGetPageLength(pagedTable)) {
                pagedTable->pending = pagedTable->cursor;
            }
        }
    } else {
        pagedTable->pending = pagedTableGetPageStart(pagedTable);
    }
}

float xglCos(float angle);

/*
 * Animates the slot bytes and flag bytes toward a target set by a cosine wave; the phase advances
 * by 0.0785 radians per call unless unk_20 bit 4 resets it.
 */
void pagedTableAnimateSlots(PagedTable *pagedTable) {
    s32 filledCount;
    s32 target;
    s32 count;
    s32 i;

    if (pagedTable->capacity == 0) {
        return;
    }
    if ((pagedTable->unk_20 & 0x10) != 0) {
        pagedTable->wavePhase = 0;
    } else {
        pagedTable->wavePhase += 0.07853981853f;
        if (pagedTable->wavePhase > 6.283185482f) {
            pagedTable->wavePhase -= 6.283185482f;
        }
    }
    target = (s32) (xglCos(pagedTable->wavePhase) * 32.0f) + 96;

    if ((pagedTable->unk_20 & 2) != 0) {
        count = pagedTable->filledEntryCount;
    } else {
        count = pagedTable->pageSize;
    }
    if (pagedTable->filledEntryCount < count) {
        count = pagedTable->filledEntryCount;
    }
    for (i = 0; i < count; i++) {
        if (((pagedTable->unk_20 & 1) == 0) &&
            (((pagedTable->unk_20 & 0x20) != 0) || (pagedTableGetPageRelativeIndex(pagedTable, pagedTable->cursor) == i))) {
            u8 value = pagedTable->entrySlotBytes[i];
            if (value + 16 < target) {
                pagedTable->entrySlotBytes[i] = value + 16;
            } else if (value - 16 > target) {
                pagedTable->entrySlotBytes[i] = value - 16;
            } else {
                pagedTable->entrySlotBytes[i] = target;
            }
        } else {
            u8 value = pagedTable->entrySlotBytes[i];
            if (value - 16 > 0) {
                pagedTable->entrySlotBytes[i] = value - 16;
            } else {
                pagedTable->entrySlotBytes[i] = 0;
            }
        }
    }
    filledCount = pagedTable->filledEntryCount;
    for (i = 0; i < filledCount; i++) {
        if ((pagedTable->entryKeys[i] & 0x100) != 0) {
            u8 value = pagedTable->entryFlags[i];
            if (value + 16 < 48) {
                pagedTable->entryFlags[i] = value + 16;
            } else if (value - 16 > 48) {
                pagedTable->entryFlags[i] = value - 16;
            } else {
                pagedTable->entryFlags[i] = 48;
            }
        } else {
            u8 value = pagedTable->entryFlags[i];
            if (value + 16 < 128) {
                pagedTable->entryFlags[i] = value + 16;
            } else {
                pagedTable->entryFlags[i] = 128;
            }
        }
    }
}

s32 pagedTableGetPageRelativeIndex();

void pagedTableSetSlotByte(PagedTable *pagedTable, s32 entryIndex, s32 slotByte) {
    if (pagedTable->capacity != 0) {
        pagedTable->entrySlotBytes[pagedTableGetPageRelativeIndex(pagedTable, entryIndex)] = slotByte;
    }
}

/*
 * Runs pagedTableUpdateCursorFromStatus and, when it reports a status, updates the pending index and plays the seVoice cues
 * (1 cursor moved, 2 positive status, 8 negative status, 3 pending entry).
 */
s32 pagedTableUpdate(PagedTable *pagedTable, s32 *statusOut) {
    s32 statusBits;
    s32 cursorBefore;
    s32 step;
    s32 pageLength;
    s32 limit;
    s32 pendingFlag = 0;

    cursorBefore = pagedTable->cursor;
    statusBits = pagedTableUpdateCursorFromStatus(pagedTable, statusOut);
    if (statusBits != 0) {
        if ((pagedTable->unk_20 & 4) != 0) {
            if ((statusBits & 2) != 0) {
                step = pagedTableGetPageLength(pagedTable);
            } else {
                step = 1;
            }
            while (pagedTable->cursor < pagedTable->pending) {
                if (pagedTable->pending - step > 0) {
                    pagedTable->pending = pagedTable->pending - step;
                } else {
                    pagedTable->pending = 0;
                }
            }
            pageLength = pagedTableGetPageLength(pagedTable);
            while (pagedTable->cursor > pagedTable->pending + pageLength - 1) {
                limit = pagedTable->filledEntryCount - pageLength;
                if (pagedTable->pending + step < limit) {
                    pagedTable->pending = pagedTable->pending + step;
                } else if (limit > 0) {
                    pagedTable->pending = limit;
                } else {
                    pagedTable->pending = 0;
                }
            }
        } else {
            pagedTable->pending = pagedTableGetPageStart(pagedTable);
        }

        if (pagedTable->capacity != 0 && *statusOut > 0) {
            if ((pagedTable->entryKeys[*statusOut - 1] & 0x100) != 0) {
                *statusOut = 0;
                pendingFlag = 1;
            }
        }
        if ((pagedTable->unk_20 & 8) != 0) {
            if (cursorBefore != pagedTable->cursor) {
                seVoiceStartDefault(1);
            }
            if (*statusOut > 0 && (pagedTable->unk_20 & 0x40) == 0) {
                seVoiceStartDefault(2);
            }
            if (*statusOut < 0) {
                seVoiceStartDefault(8);
            }
            if (pendingFlag != 0) {
                seVoiceStartDefault(3);
            }
        }
        if (cursorBefore != pagedTable->cursor) {
            pagedTable->wavePhase = 0;
        } else if (*statusOut > 0) {
            pagedTable->wavePhase = 0;
        } else if (*statusOut < 0) {
            pagedTable->wavePhase = 0;
        }
    }
    return statusBits;
}

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
        return pagedTable->entrySlotBytes[pagedTableGetPageRelativeIndex(pagedTable, entryIndex)];
    }
    return ((pagedTable->cursor ^ entryIndex) != 0) ? 0U : 0x80U;
}

u8 pagedTableGetFlagByte(PagedTable *pagedTable, s32 entryIndex) {
    if (pagedTable->capacity != 0) {
        return pagedTable->entryFlags[entryIndex];
    }
    return 0x80U;
}

s32 pagedTableGetStatus(PagedTable *pagedTable) {
    s32 statusBits;
    s32 pending;

    pending = pagedTable->pending;
    statusBits = pending != 0;
    return ((pagedTable->pending + pagedTableGetPageLength(pagedTable)) < pagedTable->filledEntryCount) ? (statusBits | 2) : statusBits;
}

s32 pagedTableFlush(void) {
    newGameStateReset();
    return 1;
}

s32 gPagedTablePoolHandle = 0;
