#include "common.h"

typedef unsigned int u32;

typedef unsigned char u8;

#define BYTE_OFFSET(base, off) ((u8 *) (base) + (off))
/* Rounds a disc-stream cursor up to the next 64-byte boundary. */
#define ALIGN_TO_64(p) ((u8 *) (((s32) (p) + 0x3F) & -0x40))
/* Size of the stream pool (12 MB) that func_002437A0 hands out and the item-source reader fills. */
#define ITEM_STREAM_POOL_SIZE 0x00C00000
/* Scratch buffer that the item-source reader fills from the disc. */
#define ITEM_SCRATCH_BUFFER ((u8 *) 0x00980000)

typedef int s32;

typedef struct XenoActorItemList {
    u8 unk_00[0x24];
    s32 firstItemOffset;
} XenoActorItemList;

typedef struct XenoActorItem {
    u8 unk_00[0x54];
    s32 payloadSize;
} XenoActorItem;

/*
 * Returns the item that follows itemsToSkip items of an item list: the first item sits at
 * firstItemOffset, every item is 0x118 bytes plus its own payloadSize.
 */
void *xenoActorItemAt(void *itemListHeader, u32 itemsToSkip) {
    int fixedItemSize;
    u32 walkedCount;
    void *itemCursor;
    XenoActorItem *skippedItem;
    XenoActorItemList *listHeader;
    listHeader = itemListHeader;
    walkedCount = 0;
    itemCursor = BYTE_OFFSET(listHeader, listHeader->firstItemOffset);
    if (itemsToSkip != 0) {
        fixedItemSize = 0x118;
        do {
            walkedCount += 1;
            skippedItem = itemCursor;
            itemCursor = BYTE_OFFSET(BYTE_OFFSET(itemCursor, fixedItemSize), skippedItem->payloadSize);
        } while (walkedCount < itemsToSkip);
    }
    return itemCursor;
}

extern s32 D_003EB490[];
extern const char *itemSourceExtensions[];
char *func_0030EF98(char *fileName, const char *extension);

/*
 * Index of the first extension in itemSourceExtensions that fileName matches; the table ends at the first
 * negative entry of D_003EB490. Returns -1 when none matches.
 */
s32 itemSourceExtensionIndex(char *fileName) {
    s32 index;
    for (index = 0; D_003EB490[index] >= 0; index++) {
        if (func_0030EF98(fileName, itemSourceExtensions[index]) != 0) {
            return index;
        }
    }
    return -1;
}

typedef short s16;

typedef struct CdRegisteredEntryView {
    u8 unk_00[0x9C];
    s16 unk_9C;
    u8 unk_9E[0x504 - 0x9E];
    void *resolvedFirstLink;
    void *resolvedSecondLink;
    void *resolvedThirdLink;
    u8 unk_510[0x514 - 0x510];
    void *unk_514;
    u8 unk_518[0x530 - 0x518];
    void *resolvedFourthLink;
} CdRegisteredEntryView;

/* Item entry returned by xglCdRegisterEntry: link pointers at 0x504, 0x508, 0x50C and 0x530; an s16 at 0x9C. */
typedef struct CdRegisteredEntry {
    u8 unk_00[0x9C];
    s16 unk_9C;
    u8 unk_9E[0x504 - 0x9E];
    void *resolvedFirstLink;
    void *resolvedSecondLink;
    void *resolvedThirdLink;
    u8 unk_510[0x514 - 0x510];
    void *resolvedFifthLink;
    u8 unk_518[0x530 - 0x518];
    void *resolvedFourthLink;
} CdRegisteredEntry;

CdRegisteredEntry *xglCdRegisterEntry();
s32 func_0028D760(void *);

typedef struct CdLoadCursor {
    s32 position;
    s32 entryCount;
} CdLoadCursor;

typedef struct XglCdBlockStream {
    u8 unk_0000[0x18D8];
    void *itemSourcePointers[(0x3F0C - 0x18D8) / 4];
    s32 itemSourceCount;   /* +0x3F0C */
    s32 loadedBytes;       /* +0x3F10 */
    s32 expectedIndexWord; /* +0x3F14 */
    u8 unk_3F18[0x3F3C - 0x3F18];
    s32 optionalStreamBytes; /* +0x3F3C */
    s32 primaryStreamBytes;  /* +0x3F40 */
    u8 unk_3F44[0x3F48 - 0x3F44];
    s32 itemStatus; /* +0x3F48 */
    u8 unk_3F4C[0x4154 - 0x3F4C];
    u8 *streamCursor; /* +0x4154 */
    u8 unk_4158[0x415C - 0x4158];
    s32 payloadBytes; /* +0x415C */
} XglCdBlockStream;

extern XglCdBlockStream gXglCdBlockStreamStorage[];

#define gXglCdBlockStream (gXglCdBlockStreamStorage[0])

void *testLoadFile(CdLoadCursor *cursor, u8 *path);
s32 strlen(u8 *path);
void *memset(void *dst, s32 value, unsigned int size);
void assertFailed(const char *message, const char *file, int line);

static inline void *LoadCdFile(CdLoadCursor *cursor, u8 *path) {
    return testLoadFile(cursor, path);
}

/*
 * Loads the resource files named after base (base + .pxy/.txy/.xhr/.epf) from the disc and registers
 * entryCount entries that link them. Returns the next 64-byte aligned disc position.
 */
s32 xenoActorLoadItemResources(s32 position, u8 *baseName, s32 linkEntryCount) {
    s32 resourceLinks[15];
    s32 extensionOffset;
    CdLoadCursor cursor;
    s32 startPosition;
    u8 *extension;
    u8 *extByte1;
    u8 *extByte2;
    u8 *extByte3;
    s32 entriesRemaining;
    CdRegisteredEntryView *resourceEntry;

    cursor.position = position;
    cursor.entryCount = linkEntryCount;
    extensionOffset = strlen(baseName);
    startPosition = cursor.position;
    memset(resourceLinks, 0, sizeof(resourceLinks));
    extension = baseName + extensionOffset;
    extByte1 = extension + 1;
    extByte2 = extension + 2;
    extByte3 = extension + 3;
    extension[0] = '.';
    *extByte1 = 'p';
    *extByte2 = 'x';
    *extByte3 = 'y';
    extension[4] = 0;
    resourceLinks[0] = LoadCdFile(&cursor, baseName);
    *extByte1 = 't';
    *extByte2 = 'x';
    *extByte3 = 'y';
    resourceLinks[1] = LoadCdFile(&cursor, baseName);
    *extByte1 = 'x';
    *extByte2 = 'h';
    *extByte3 = 'r';
    resourceLinks[2] = LoadCdFile(&cursor, baseName);
    *extByte1 = 'e';
    *extByte2 = 'p';
    *extByte3 = 'f';
    resourceLinks[11] = LoadCdFile(&cursor, baseName);
    if (resourceLinks[0] == 0) {
        assertFailed("rsrc[XAct_Rsrc_MDL] != NULL", "XenoActorEvent_ReadItem.euc.c", 259);
    }
    if (resourceLinks[1] == 0) {
        assertFailed("rsrc[XAct_Rsrc_TEX] != NULL", "XenoActorEvent_ReadItem.euc.c", 260);
    }
    if (!(resourceLinks[2] == 0 && resourceLinks[11] == 0)) {
        s32 streamBytes = cursor.position - startPosition;
        gXglCdBlockStream.optionalStreamBytes += streamBytes;
    } else {
        s32 streamBytes = cursor.position - startPosition;
        gXglCdBlockStream.primaryStreamBytes += streamBytes;
    }
    if (cursor.entryCount > 0) {
        entriesRemaining = cursor.entryCount;
        do {
            entriesRemaining -= 1;
            resourceEntry = (CdRegisteredEntryView *) xglCdRegisterEntry();
            resourceEntry->resolvedFirstLink = (void *) resourceLinks[0];
            resourceEntry->resolvedSecondLink = (void *) resourceLinks[1];
            resourceEntry->resolvedThirdLink = (void *) resourceLinks[2];
            resourceEntry->unk_514 = (void *) resourceLinks[4];
            resourceEntry->resolvedFourthLink = (void *) resourceLinks[11];
            func_0028D760(resourceEntry);
            resourceEntry->unk_9C = 0;
        } while (entriesRemaining != 0);
    }
    return (cursor.position + 63) & ~63;
}

typedef signed char s8;

/* Source item record: four offsets relative to the relativeBase address passed to the reader (0 = none). */
struct RelativeOffsetRecord {
    s8 unk00[4];
    s32 firstRel;
    s32 secondRel;
    s32 thirdRel;
    s32 fourthRel;
};

s32 itemSourceResolveEntries(u8 *relativeBase, struct RelativeOffsetRecord *relativeRecords, s32 registerCount) {
    struct RelativeOffsetRecord *relativeRecord;
    CdRegisteredEntry *registeredEntry;
    s32 entriesRemaining;
    s32 firstRelative;
    s32 secondRelative;
    s32 thirdRelative;
    s32 fourthRelative;

    if (registerCount > 0) {
        relativeRecord = relativeRecords;
        entriesRemaining = registerCount;
        do {
            registeredEntry = xglCdRegisterEntry();
            firstRelative = relativeRecord->firstRel;
            if (firstRelative != 0) {
                registeredEntry->resolvedFirstLink = relativeBase + firstRelative;
            } else {
                registeredEntry->resolvedFirstLink = 0;
            }
            secondRelative = relativeRecord->secondRel;
            if (secondRelative != 0) {
                registeredEntry->resolvedSecondLink = relativeBase + secondRelative;
            } else {
                registeredEntry->resolvedSecondLink = 0;
            }
            thirdRelative = relativeRecord->thirdRel;
            if (thirdRelative != 0) {
                registeredEntry->resolvedThirdLink = relativeBase + thirdRelative;
            } else {
                registeredEntry->resolvedThirdLink = 0;
            }
            fourthRelative = relativeRecord->fourthRel;
            if (fourthRelative != 0) {
                registeredEntry->resolvedFourthLink = relativeBase + fourthRelative;
            } else {
                registeredEntry->resolvedFourthLink = 0;
            }
            entriesRemaining -= 1;
            func_0028D760(registeredEntry);
            registeredEntry->unk_9C = 0;
        } while (entriesRemaining != 0);
    }
    return 1;
}

typedef struct ItemSourceUnpack {
    u8 *output;
    s32 remaining;
    s32 backOffset;
    s32 backLength;
    u8 flags;
    u8 bitsLeft;
    u8 state;
} ItemSourceUnpack;

/*
 * Resumable LZSS-style unpacker for item sources: writes unpack->remaining more bytes into
 * unpack->output from input. Each flag byte drives 8 steps: a set bit copies one literal byte, a clear
 * bit reads a length byte and a 16-bit back offset, then copies length+1 bytes from earlier output.
 * A zero length byte returns early.
 */
void itemSourceLzssUnpack(ItemSourceUnpack *unpack, u8 *inputBytes, s32 inputLeft) {
    u8 *copyFrom;
    s32 copyLength;

    while (unpack->remaining > 0) {
        if (inputLeft == 0) {
            return;
        }
        if (unpack->bitsLeft != 0) {
            if (unpack->flags & 0x80) {
                u8 *outputCursor = unpack->output;
                *outputCursor = *inputBytes;
                outputCursor++;
                unpack->output = outputCursor;
                inputBytes++;
                inputLeft--;
                unpack->remaining--;
                unpack->flags <<= 1;
                unpack->bitsLeft--;
                unpack->state = 0;
            } else if (unpack->state == 0) {
                unpack->backLength = *inputBytes;
                inputBytes++;
                inputLeft--;
                unpack->state = 1;
                if (unpack->backLength == 0) {
                    return;
                }
            } else if (unpack->state == 1) {
                unpack->backOffset = *inputBytes;
                inputBytes++;
                inputLeft--;
                unpack->state = 2;
            } else if (unpack->state == 2) {
                unpack->backOffset = (unpack->backOffset | (*inputBytes << 8)) + 1;
                inputBytes++;
                inputLeft--;
                unpack->state = 3;
            } else {
                u8 *outputCursor;
                copyLength = unpack->backLength + 1;
                unpack->remaining -= copyLength;
                unpack->state = 0;
                outputCursor = unpack->output;
                copyFrom = outputCursor - unpack->backOffset;
                if (unpack->backLength != -1) {
                /* kept: the goto loop is the shape the match needs */
                copy:
                    *outputCursor = *copyFrom;
                    copyFrom++;
                    unpack->output = outputCursor + 1;
                    outputCursor++;
                    unpack->backLength--;
                    if (unpack->backLength != -1) {
                        goto copy;
                    }
                }
                unpack->flags <<= 1;
                unpack->bitsLeft--;
            }
            if (inputLeft == 0) {
                return;
            }
        } else {
            unpack->flags = *inputBytes;
            inputBytes++;
            unpack->bitsLeft = 8;
            unpack->state = 0;
            inputLeft--;
        }
    }
}

extern const char D_00539560[];
extern const char D_00539578[];
extern const char D_00539590[];
extern const char D_005395A8[];
extern const char D_005395C0[];
extern const char D_005395D8[];
extern const char D_005395F0[];
extern const char D_00539608[];
extern const char D_00539620[];
extern const char D_00539638[];
extern const char D_00539650[];
extern const char D_00539668[];
extern const char D_00539680[];
extern const char D_00539698[];
extern const char D_005396B0[];
extern const char D_005396C8[];
extern const char D_005396E0[];
extern const char D_005396F8[];
extern const char D_00539710[];
extern const char D_00539728[];
extern const char D_00539740[];
extern const char D_00539758[];
extern const char D_00539770[];
extern const char D_00539788[];
extern const char D_005397A0[];
extern const char D_005397B8[];
extern const char D_005397D0[];
extern const char D_005397E8[];
extern const char D_00539800[];
extern const char D_00539818[];
extern const char D_00539830[];
extern const char D_00539848[];
extern const char D_00539860[];
extern const char D_00539878[];
extern const char D_00539890[];
extern const char D_005398A8[];
extern const char D_005398C0[];
extern const char D_005398D8[];
extern const char D_005398F0[];
extern const char D_00539908[];
extern const char D_00539920[];
extern const char D_00539938[];
extern const char D_00539950[];
extern const char D_00539968[];
extern const char D_00539980[];
extern const char D_00539998[];
extern u8 gXenoActorBufferB[];
s32 xglCdGetCachedFileSize(void);
void func_001B6AB0(void);
u8 *func_002437A0(s32 size);
void func_001B6798(u8 *streamBase);
s32 xglCdServiceIfPending(void);
void xglSleep(void);
s32 xglCdReadFilePart(u8 *fileName, u8 *buffer, s32 mode, s32 flags, u32 offset, u32 size);

typedef struct LzDecodeState {
    u8 *dst;
    s32 outRemaining;
    s32 distance;
    s32 copyCount;
    u8 flags;
    u8 bitsLeft;
    u8 state;
} LzDecodeState;

/*
 * Reads the item-source file from the disc in chunks (first 4 MB, then 1 MB at a time) and
 * unpacks it into the stream area with itemSourceLzssUnpack. Returns the stream base.
 */
u8 *xenoActorLoadItemStream(void) {
    LzDecodeState lzState;
    u32 fileSize;
    u32 chunkSize;
    u32 roundedSize;
    u32 readOffset;
    u8 *streamBase;
    u8 *firstChunk;
    s32 payloadSize;
    s32 regionEnd;

    chunkSize = 0x400000;
    fileSize = xglCdGetCachedFileSize();
    func_001B6AB0();
    streamBase = func_002437A0(ITEM_STREAM_POOL_SIZE);
    if (streamBase == 0) {
        assertFailed("pTop", "XenoActorEvent_ReadItem.euc.c", 445);
    }
    if (fileSize < chunkSize) {
        chunkSize = fileSize;
    }
    roundedSize = (chunkSize + 2047) & 0xFFFFF800;
    firstChunk = (u8 *) ITEM_STREAM_POOL_SIZE + ((u32) streamBase - roundedSize);
    xglCdReadFilePart(gXenoActorBufferB, firstChunk, 1, 0, 0, chunkSize);
    while (xglCdServiceIfPending()) {
        xglSleep();
    }
    payloadSize = ((s32 *) firstChunk)[1];
    regionEnd = payloadSize + 0x80000;
    readOffset = chunkSize;
    gXglCdBlockStream.payloadBytes = payloadSize;
    if (ITEM_STREAM_POOL_SIZE < regionEnd) {
        func_001B6798(streamBase);
        streamBase = func_002437A0(regionEnd);
    }
    gXglCdBlockStream.streamCursor = streamBase + 0x80000;
    memset(&lzState, 0, sizeof(LzDecodeState));
    lzState.dst = gXglCdBlockStream.streamCursor;
    lzState.outRemaining = payloadSize;
    itemSourceLzssUnpack(&lzState, firstChunk + 16, readOffset - 16);
    while (readOffset < fileSize) {
        chunkSize = 0x100000;
        if (fileSize < readOffset + chunkSize) {
            chunkSize = fileSize - readOffset;
        }
        xglCdReadFilePart(gXenoActorBufferB, ITEM_SCRATCH_BUFFER, 1, 0, readOffset, chunkSize);
        while (xglCdServiceIfPending()) {
        }
        readOffset += 0x100000;
        itemSourceLzssUnpack(&lzState, ITEM_SCRATCH_BUFFER, chunkSize);
    }
    func_001B6AB0();
    return streamBase;
}

struct RelativeOffsetRecordView {
    s32 kind;
    s32 firstRel;
    s32 secondRel;
    s32 thirdRel;
    s32 fourthRel;
};

typedef struct ItemSourceEntry {
    u8 unk00[32];
    s32 registerCount;
} ItemSourceEntry;

typedef struct ItemSourceIndex {
    s32 count;
    struct RelativeOffsetRecordView records[0];
} ItemSourceIndex;

typedef struct ItemSourceTable {
    s32 headerWord;
    u8 unk04[12];
    ItemSourceEntry entries[0];
} ItemSourceTable;

typedef struct ReadItemLoopFlags {
    s32 flags; /* +0x00 */
    unsigned char unmodeled_04[0x0C];
} ReadItemLoopFlags;

extern ReadItemLoopFlags gGameLoopRuntimeStorage;
extern s32 D_003EB508[];
extern const char *D_003EB5F8[];
void xenoActorMatrixTableLoad(void *entry, s32 index);
s32 xenoActorEventRegisterEffect(u8 *cursor, struct RelativeOffsetRecordView *record, ItemSourceEntry *entry);
s32 xenoActorEventGetStatus45C(void);
s32 rndRollKind(s32 kind);
s32 func_0028B368(void *entry, u8 *src);
s32 xglCdGetFileSize(u8 *path);
void *memcpy(void *dst, const void *src, unsigned int size);
char *strcpy(char *dst, const char *src);
void xglCdReadFile(u8 *path, void *dst, s32 unused0, s32 unused1);

/*
 * Resolves the item-source index at the stream cursor, record by record: kind 1 (and kind 2 without a
 * replacement roll) registers the record's links; kind 2 with a roll reads a .chr replacement from the disc;
 * kind 3 goes to xenoActorEventRegisterEffect; kind 4 does nothing. Then xenoActorMatrixTableLoad runs over every item source.
 */
s32 itemSourceResolveIndex(s32 selectorMode, ItemSourceTable *sourceTable) {
    char replacementName[64];
    u8 *cursor;
    ItemSourceIndex *indexBlock;
    struct RelativeOffsetRecordView *record;
    ItemSourceEntry *tableEntry;
    CdRegisteredEntryView *registeredEntry;
    unsigned int recordCount;
    unsigned int i;
    s32 registerIndex;
    s32 kind;
    s32 *registerCountField;
    s32 replacementIndex;
    s32 useReplacement;
    void *itemSource;
    s32 baseNameLength;
    s32 replacementSize;
    u8 *replacementDest;

    tableEntry = sourceTable->entries;
    gXglCdBlockStream.expectedIndexWord = sourceTable->headerWord;
    if (gXglCdBlockStream.streamCursor == 0) {
        if ("not read" != 0) {
            assertFailed("!\"not read\"", "XenoActorEvent_ReadItem.euc.c", 537);
        }
    }
    cursor = gXglCdBlockStream.streamCursor;
    recordCount = ((ItemSourceIndex *) cursor)->count;
    if (recordCount != gXglCdBlockStream.expectedIndexWord) {
        if ("\245\242\245\244\245\306\245\340\277\364\244\254\260\354\303\327\244\267\244\336\244\273\244\363" !=
            0) {
            assertFailed(
                "!\"\245\242\245\244\245\306\245\340\277\364\244\254\260\354\303\327\244\267\244\336\244\273\244\363\"",
                "XenoActorEvent_ReadItem.euc.c", 543);
        }
    }
    gXglCdBlockStream.itemStatus = xenoActorEventGetStatus45C();
    for (i = 0; i < recordCount; i++) {
        indexBlock = (ItemSourceIndex *) cursor;
        record = &indexBlock->records[i];
        registerCountField = &tableEntry->registerCount;
        kind = record->kind;
        if (kind == 1) {
            itemSourceResolveEntries(cursor, record, *registerCountField);
        } else if (kind == 2) {
            useReplacement = 0;
            replacementIndex = 0;
            if ((gGameLoopRuntimeStorage.flags & 0x1000000) != 0) {
                replacementIndex = itemSourceExtensionIndex(tableEntry);
                if (replacementIndex >= 0) {
                    useReplacement = rndRollKind(D_003EB508[replacementIndex]) != 0;
                }
            }
            if (useReplacement) {
                baseNameLength = strlen((u8 *) D_003EB5F8[D_003EB490[replacementIndex]]);
                strcpy(replacementName, D_003EB5F8[D_003EB490[replacementIndex]]);
                replacementName[baseNameLength] = '.';
                replacementName[baseNameLength + 1] = 'c';
                replacementName[baseNameLength + 2] = 'h';
                replacementName[baseNameLength + 3] = 'r';
                replacementName[baseNameLength + 4] = 0;
                replacementSize = xglCdGetFileSize((u8 *) replacementName);
                if (replacementSize > 0) {
                    replacementDest = cursor + record->firstRel;
                    xglCdReadFile((u8 *) replacementName, ITEM_SCRATCH_BUFFER, 0, 0);
                    memcpy(replacementDest, ITEM_SCRATCH_BUFFER, replacementSize);
                    for (registerIndex = 0; registerIndex < *registerCountField; registerIndex++) {
                        registeredEntry = ((CdRegisteredEntryView * (*) ()) xglCdRegisterEntry)();
                        func_0028B368(registeredEntry, replacementDest);
                        func_0028D760(registeredEntry);
                        registeredEntry->unk_9C = 0;
                    }
                }
            } else {
                itemSourceResolveEntries(cursor, record, *registerCountField);
            }
        } else if (kind == 3) {
            xenoActorEventRegisterEffect(cursor, record, tableEntry);
        } else if (kind != 4) {
            if (kind == 0) {
                if ("\311\324\300\265\244\312\245\263\245\363\245\320\241\274\245\310\244\254\271\324\244\357\244\354\244\336\244\267\244\277" !=
                    0) {
                    assertFailed(
                        "!\"\311\324\300\265\244\312\245\263\245\363\245\320\241\274\245\310\244\254\271\324\244\357\244\354\244\336\244\267\244\277\"",
                        "XenoActorEvent_ReadItem.euc.c", 635);
                }
            } else {
                if ("\245\321\245\303\245\255\245\363\245\260\245\307\241\274\245\277\244\254\262\365\244\354\244\306\244\244\244\336\244\271" !=
                    0) {
                    assertFailed(
                        "!\"\245\321\245\303\245\255\245\363\245\260\245\307\241\274\245\277\244\254\262\365\244\354\244\306\244\244\244\336\244\271\"",
                        "XenoActorEvent_ReadItem.euc.c", 640);
                }
            }
        }
        tableEntry++;
    }
    i = 0;
    while (i < gXglCdBlockStream.itemSourceCount) {
        itemSource = gXglCdBlockStream.itemSourcePointers[i];
        xenoActorMatrixTableLoad(itemSource, i);
        i++;
    }
    return selectorMode;
}

const char itemWeaponPathPrefix[8] = "mdl/wpn";

struct CdLoadBlocks {
    u8 *pxyBlock;
    u8 *txyBlock;
    u8 *unk_08;
    u8 unk_0C[0x10 - 0x0C];
    u8 *unk_10;
    u8 unk_14[0x28 - 0x14];
    u8 *chrBlock;
    u8 *unk_2C;
    u8 unk_30[0x3C - 0x30];
};

extern char D_00550CE0[];

/*
 * Loads one item file named by path from the disc into cursor, then registers linkEntryCount entries
 * for it. A name that matches D_00550CE0, or whose base has "_B0" at offset 3, loads a .pxy and a .txy
 * block; a name that matches itemWeaponPathPrefix ("mdl/wpn") loads a .wpn file; other names load a .chr file.
 * Returns the next 64-byte aligned position.
 */
u8 *xenoActorLoadItemFile(u8 *cursor, char *path, s32 linkEntryCount) {
    struct CdLoadBlocks loadedBlocks;
    u8 *startCursor;
    char *scan;
    char *baseName;
    s32 pathLen;
    s32 fileSize;
    s32 i;
    CdRegisteredEntry *resourceEntry;
    char *extByte1;
    char *extByte2;
    char *extByte3;

    startCursor = cursor;
    pathLen = strlen((u8 *) path);
    scan = path;
    while (*scan != '\0') {
        scan++;
    }
    while (*scan != '/' && scan != path) {
        scan--;
    }
    baseName = (*scan == '/') ? scan + 1 : path;
    memset(&loadedBlocks, 0, sizeof(loadedBlocks));
    if (func_0030EF98(path, D_00550CE0) != 0 || (baseName[3] == '_' && baseName[4] == 'B' && baseName[5] == '0')) {
        path[pathLen] = '.';
        extByte1 = path + pathLen + 1;
        extByte2 = path + pathLen + 2;
        extByte3 = path + pathLen + 3;
        *extByte1 = 'p';
        *extByte2 = 'x';
        *extByte3 = 'y';
        path[pathLen + 4] = '\0';
        fileSize = xglCdGetFileSize((u8 *) path);
        if (fileSize > 0) {
            xglCdReadFile((u8 *) path, cursor, 0, 0);
            loadedBlocks.pxyBlock = cursor;
            cursor += fileSize;
            cursor = ALIGN_TO_64(cursor);
        }
        *extByte1 = 't';
        *extByte2 = 'x';
        *extByte3 = 'y';
        fileSize = xglCdGetFileSize((u8 *) path);
        if (fileSize > 0) {
            xglCdReadFile((u8 *) path, cursor, 0, 0);
            loadedBlocks.txyBlock = cursor;
            cursor += fileSize;
            cursor = ALIGN_TO_64(cursor);
        }
    } else {
        if (func_0030EF98(path, itemWeaponPathPrefix) != 0) {
            path[pathLen] = '.';
            path[pathLen + 1] = 'w';
            path[pathLen + 2] = 'p';
            path[pathLen + 3] = 'n';
            path[pathLen + 4] = '\0';
        } else {
            path[pathLen] = '.';
            path[pathLen + 1] = 'c';
            path[pathLen + 2] = 'h';
            path[pathLen + 3] = 'r';
            path[pathLen + 4] = '\0';
        }
        fileSize = xglCdGetFileSize((u8 *) path);
        if (fileSize > 0) {
            xglCdReadFile((u8 *) path, cursor, 0, 0);
            loadedBlocks.chrBlock = cursor;
            cursor += fileSize;
            cursor = ALIGN_TO_64(cursor);
        }
    }

    if (loadedBlocks.chrBlock != 0) {
        gXglCdBlockStream.optionalStreamBytes += cursor - startCursor;
    } else {
        gXglCdBlockStream.primaryStreamBytes += cursor - startCursor;
    }

    if (loadedBlocks.chrBlock != 0) {
        for (i = 0; i < linkEntryCount; i++) {
            resourceEntry = xglCdRegisterEntry();
            func_0028B368(resourceEntry, loadedBlocks.chrBlock);
            func_0028D760(resourceEntry);
            resourceEntry->unk_9C = 0;
        }
    } else if (linkEntryCount > 0) {
        i = linkEntryCount;
        do {
            i--;
            resourceEntry = xglCdRegisterEntry();
            resourceEntry->resolvedFirstLink = loadedBlocks.pxyBlock;
            resourceEntry->resolvedSecondLink = loadedBlocks.txyBlock;
            resourceEntry->resolvedThirdLink = loadedBlocks.unk_08;
            resourceEntry->resolvedFifthLink = loadedBlocks.unk_10;
            resourceEntry->resolvedFourthLink = loadedBlocks.unk_2C;
            func_0028D760(resourceEntry);
            resourceEntry->unk_9C = 0;
        } while (i != 0);
    }
    return cursor;
}

typedef struct EventItemFile {
    char name[0x20];
    u32 size;
} EventItemFile;

/* An event item list: the file count, then the files from +0x10. */
typedef struct EventItemList {
    u32 count;
    u8 unmodeled_04[0x0C];
    EventItemFile files[1];
} EventItemList;

s32 xglCdDriveStatusFifthBitSet(void);
void xenoActorEventLoadEffectCd(EventItemFile *file, s32 *state);
void xenoActorEventLoadEffectFile(EventItemFile *file, s32 *state);
char *func_0030E3D0(char *destination, const char *source);

/*
 * Loads the files of an event item list: CM_* and PS2* files are skipped; the e/c/p actor files
 * ("?e???_*") go through xenoActorEventLoadEffectCd or xenoActorEventLoadEffectFile; the others come from the disc or from the
 * host directory. Then xenoActorMatrixTableLoad runs on every loaded item source. Returns the next disc position.
 */
s32 xenoActorLoadEventItemList(s32 diskPosition, EventItemList *eventList) {
    EventItemFile *file;
    u32 i;

    gXglCdBlockStream.expectedIndexWord = eventList->count;
    file = eventList->files;
    for (i = 0; i < eventList->count; i++) {
        char hostPath[128] = "host0:/home/xeno/work/EV_EP3/VCD/";

        char *fileName = file->name;
        u32 *fileSize = &file->size;

        if (fileName[0] == 'C' && fileName[1] == 'M' && fileName[2] == '_') {
        } else if (fileName[0] == 'P' && fileName[1] == 'S' && fileName[2] == '2') {
        } else if ((fileName[0] == 'e' || fileName[0] == 'c' || fileName[0] == 'p') && fileName[1] == 'e' && fileName[5] == '_') {
            if (xglCdDriveStatusFifthBitSet()) {
                xenoActorEventLoadEffectCd(file, &diskPosition);
            } else {
                xenoActorEventLoadEffectFile(file, &diskPosition);
            }
            file++;
            gXglCdBlockStream.loadedBytes += *fileSize;
            continue;
        } else if (xglCdDriveStatusFifthBitSet()) {
            diskPosition = xenoActorLoadItemFile(diskPosition, file->name, *fileSize);
        } else {
            func_0030E3D0(hostPath, file->name);
            diskPosition = xenoActorLoadItemResources(diskPosition, hostPath, *fileSize);
        }
        file++;
    }
    for (i = 0; i < eventList->count; i++) {
        xenoActorMatrixTableLoad(gXglCdBlockStream.itemSourcePointers[i], i);
    }
    return diskPosition;
}

s32 D_003EB490[30] = {
    0, 1, 2, 3, 4, 5, 6, 6, 7, 7,
    7, 7, 8, 9, 10, 10, 10, 11, 11, 11,
    12, 13, 14, 14, 14, 15, 15, 16, 16, -1,
};

s32 D_003EB508[30] = {
    6, 6, 2, 2, 7, 7, 5, 5, 5, 5,
    5, 5, 3, 3, 1, 1, 1, 1, 1, 1,
    4, 4, 10, 10, 10, 10, 10, 5, 5, -1,
};

const char *itemSourceExtensions[30] = {
    D_00539800, D_005397E8, D_005397D0, D_005397B8, D_005397A0, D_00539788, D_00539770, D_00539758, D_00539740, D_00539728,
    D_00539710, D_005396F8, D_005396E0, D_005396C8, D_005396B0, D_00539698, D_00539680, D_00539668, D_00539650, D_00539638,
    D_00539620, D_00539608, D_005395F0, D_005395D8, D_005395C0, D_005395A8, D_00539590, D_00539578, D_00539560, 0,
};

const char *D_003EB5F8[18] = {
    D_00539998, D_00539980, D_00539968, D_00539950, D_00539938, D_00539920, D_00539908, D_005398F0, D_005398D8,
    D_005398C0, D_005398A8, D_00539890, D_00539878, D_00539860, D_00539848, D_00539830, D_00539818, 0,
};

char D_00550CE0[4] = "E3_";
