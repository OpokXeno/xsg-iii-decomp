#include "common.h"

typedef int s32;

typedef unsigned char u8;

typedef struct Record144Table {
    u8 unk_00[0x54];
    s32 firstRecordDelta; /* +0x54 */
} Record144Table;

typedef struct Record144 {
    u8 unk_00[0x50];
    s32 variableDataSize; /* +0x50 */
    u8 unk_54[0x3C];
} Record144;

typedef unsigned int u32;

/* Returns the record recordSteps records after the table's first record. Each step adds
   the 0x90-byte fixed part of a record plus its variableDataSize (at +0x50). */
u8 *xenoActorEntryAt(Record144Table *table, u32 recordSteps) {
    u32 stepsDone;
    u8 *cursor;
    s32 recordStride;

    stepsDone = 0;
    cursor = (u8 *) table + table->firstRecordDelta;
    if (recordSteps != 0) {
        recordStride = 0x90;
        do {
            stepsDone += 1;
            cursor = (cursor + recordStride) + ((Record144 *) cursor)->variableDataSize;
        } while (stepsDone < recordSteps);
    }
    return cursor;
}

typedef struct Record336Table {
    u8 unk_00[0x5C];
    s32 firstRecordDelta; /* +0x5C */
} Record336Table;

typedef struct Record336 {
    u8 unk_00[0x110];
    s32 variableDataSize; /* +0x110 */
    u8 unk_114[0x3C];
} Record336;

/* Same walk as xenoActorEntryAt for 0x150-byte records (variableDataSize at +0x110). */
u8 *func_0024E380(Record336Table *table, u32 recordSteps) {
    u32 stepsDone;
    u8 *cursor;
    s32 recordStride;

    stepsDone = 0;
    cursor = (u8 *) table + table->firstRecordDelta;
    if (recordSteps != 0) {
        recordStride = 0x150;
        do {
            stepsDone += 1;
            cursor = (cursor + recordStride) + ((Record336 *) cursor)->variableDataSize;
        } while (stepsDone < recordSteps);
    }
    return cursor;
}

/* A table whose first record sits at actorTable + firstRecordDelta (actorRecordsEnd). */
typedef struct ActorRecordTable {
    u8 unk_00[0x4C];
    s32 firstRecordDelta;          /* +0x4C */
} ActorRecordTable;

/* One 0xD4-byte record; variableDataSize is added to the end of the record to reach the next one. */
typedef struct ActorRecord {
    u8 unk_00[0x90];
    s32 variableDataSize;             /* +0x90 */
    u8 unk_94[0x40];
} ActorRecord;

/* A table whose first slot sits at base + firstSlotDelta (tiSlotTableEnd). */
typedef struct TiSlotTable {
    u8 unk_00[0x14];
    s32 firstSlotDelta;          /* +0x14 */
} TiSlotTable;

/* One 0x20-byte slot; variableDataSize is added to the end of the slot to reach the next one. */
typedef struct TiSlot {
    u8 unk_00[0x10];
    s32 variableDataSize;             /* +0x10 */
    u8 unk_14[0x0C];
} TiSlot;

/* Returns the end of the actor records: recordSteps records after the first one (0xD4-byte records). */
u8 *actorRecordsEnd(ActorRecordTable *actorTable, u32 recordSteps) {
    s32 recordStride;
    u32 stepsDone;
    u8 *cursor;

    stepsDone = 0;
    cursor = (u8 *) actorTable + actorTable->firstRecordDelta;
    if (recordSteps != 0) {
        recordStride = 0xD4;
        do {
            stepsDone += 1;
            cursor = (cursor + recordStride) + ((ActorRecord *) cursor)->variableDataSize;
        } while (stepsDone < recordSteps);
    }
    return cursor;
}

typedef struct Record96Table {
    u8 unk_00[0x44];
    s32 firstRecordDelta; /* +0x44 */
} Record96Table;

typedef struct Record96 {
    u8 unk_00[0x10];
    s32 variableDataSize; /* +0x10 */
    u8 unk_14[0x4C];
} Record96;

/* Same walk as xenoActorEntryAt for 0x60-byte records (variableDataSize at +0x10). */
u8 *func_0024E3F0(Record96Table *table, u32 recordSteps) {
    u32 stepsDone;
    u8 *cursor;
    s32 recordStride;

    stepsDone = 0;
    cursor = (u8 *) table + table->firstRecordDelta;
    if (recordSteps != 0) {
        recordStride = 0x60;
        do {
            stepsDone += 1;
            cursor = (cursor + recordStride) + ((Record96 *) cursor)->variableDataSize;
        } while (stepsDone < recordSteps);
    }
    return cursor;
}

/* Returns the end of the TI slot table: slotSteps 0x20-byte slots after the first one. */
u8 *tiSlotTableEnd(TiSlotTable *base, s32 unused_arg1, u32 slotSteps) {
    u32 stepsDone;
    u8 *cursor;
    s32 slotStride;

    stepsDone = 0;
    cursor = (u8 *) base + base->firstSlotDelta;
    if (slotSteps != 0) {
        slotStride = 0x20;
        do {
            stepsDone += 1;
            cursor = (cursor + slotStride) + ((TiSlot *) cursor)->variableDataSize;
        } while (stepsDone < slotSteps);
    }
    return cursor;
}

typedef struct CountedRecord96Table {
    u8 unk_00[0x38];
    s32 recordCount;      /* +0x38 */
    s32 firstRecordDelta; /* +0x3C */
} CountedRecord96Table;

/* Same walk as func_0024E3F0, but returns 0 unless recordSteps is below the table's recordCount. */
u8 *func_0024E460(CountedRecord96Table *table, s32 unused_arg1, u32 recordSteps) {
    u32 stepsDone;
    u8 *cursor;
    s32 recordStride;

    cursor = (u8 *) table + table->firstRecordDelta;
    if (recordSteps < table->recordCount) {
        stepsDone = 0;
        if (recordSteps != 0) {
            recordStride = 0x60;
            do {
                stepsDone += 1;
                cursor = (cursor + recordStride) + ((Record96 *) cursor)->variableDataSize;
            } while (stepsDone < recordSteps);
        }
        return cursor;
    }
    return 0;
}

typedef struct CountedRecord128Table {
    u8 unk_00[0x18];
    s32 recordCount;      /* +0x18 */
    s32 firstRecordDelta; /* +0x1C */
} CountedRecord128Table;

typedef struct Record128 {
    u8 unk_00[0x1C];
    s32 variableDataSize; /* +0x1C */
    u8 unk_20[0x60];
} Record128;

/* Same walk as func_0024E460 for 0x80-byte records (variableDataSize at +0x1C). */
u8 *func_0024E4A8(CountedRecord128Table *table, s32 unused_arg1, u32 recordSteps) {
    u32 stepsDone;
    u8 *cursor;
    s32 recordStride;

    cursor = (u8 *) table + table->firstRecordDelta;
    if (recordSteps < table->recordCount) {
        stepsDone = 0;
        if (recordSteps != 0) {
            recordStride = 0x80;
            do {
                stepsDone += 1;
                cursor = (cursor + recordStride) + ((Record128 *) cursor)->variableDataSize;
            } while (stepsDone < recordSteps);
        }
        return cursor;
    }
    return 0;
}

s32 xenoActorEventResetEffects();

typedef struct XglCdBlockStream {
    u8 unk_0000[0x1800];
    s32 unk_1800[4];
    u8 unk_1810[0xC0];
    s32 unk_18D0;
    s32 unk_18D4;
    s32 registeredIds[0x98D];
    s32 registeredIdCount;
    s32 unk_3F10;
    s32 unk_3F14;
    s32 unk_3F18;
    s32 unk_3F1C;
    s32 unk_3F20;
    s32 unk_3F24;
    u8 unk_3F28[0x8];
    s32 unk_3F30;
    s32 unk_3F34;
    s32 unk_3F38;
    s32 unk_3F3C;
    s32 unk_3F40;
    s32 unk_3F44;
    s32 unk_3F48;
    s32 cdReadyBits;
    s32 unk_3F50;
    s32 unk_3F54[0x81];
} XglCdBlockStream;

extern XglCdBlockStream gXglCdBlockStreamStorage[];

#define gXglCdBlockStream (gXglCdBlockStreamStorage[0])

void xglCdInitReadRing(void) {
    gXglCdBlockStream.unk_3F1C = 1;
    gXglCdBlockStream.unk_3F24 = 0;
    gXglCdBlockStream.unk_3F30 = 0;
    gXglCdBlockStream.unk_3F34 = 0;
    gXglCdBlockStream.unk_3F38 = 0;
    gXglCdBlockStream.unk_3F50 = 0;
    xenoActorEventResetEffects();
}

extern s32 gXglCdDriveStatus[];

void xglCdSetDriveStatus(s32 driveStatus) {
    gXglCdDriveStatus[0] = driveStatus;
}

void xglCdAddReadyBits(s32 bitsToSet) {
    s32 combined;

    combined = gXglCdBlockStream.cdReadyBits | bitsToSet;
    gXglCdBlockStream.cdReadyBits = combined;
}

s32 xglDebFileNameSet();
s32 xglCdResetBlockStream();

/* Sets bit 0 of cdReadyBits, sets the file name to host0:/home/xeno/work/_particle/ and resets the stream. */
void xglCdSetReadyFlag(void) {
    s32 combined;
    combined = gXglCdBlockStream.cdReadyBits | 1;
    gXglCdBlockStream.cdReadyBits = combined;
    xglDebFileNameSet("host0:/home/xeno/work/_particle/");
    xglCdResetBlockStream();
}

extern s32 gXglCdDriveReadyStatus[];

s32 xglCdGetReadyStatus(void) {
    return gXglCdDriveReadyStatus[0];
}

s32 xglCdGetDriveStatus(void) {
    return gXglCdDriveStatus[0];
}

void xenoActorEventRunStep(void);
s32 xenoActorLightStateInit(void);

/* Resets the CD block stream: read ring, registered-id list, counters and flags, then runs
   xenoActorEventRunStep and returns xenoActorLightStateInit(). */
s32 xglCdResetBlockStream(void) {
    s32 i;

    xglCdInitReadRing();
    gXglCdBlockStream.unk_3F20 = 0;
    gXglCdBlockStream.unk_18D0 = 0;
    gXglCdBlockStream.unk_18D4 = 0;
    gXglCdBlockStream.registeredIdCount = 0;
    gXglCdBlockStream.unk_3F10 = 0;
    gXglCdBlockStream.unk_3F14 = 0;
    gXglCdBlockStream.unk_1800[0] = 0;
    gXglCdBlockStream.unk_1800[1] = 0;
    gXglCdBlockStream.registeredIds[0x98C] = 0;
    gXglCdBlockStream.unk_3F3C = 0;
    gXglCdBlockStream.unk_3F40 = 0;
    gXglCdBlockStream.unk_3F44 = 0;
    gXglCdBlockStream.unk_3F48 = 0;
    gXglCdBlockStream.unk_3F54[0x80] = 0;
    gXglCdBlockStream.unk_3F50 = 0;
    for (i = 0; i < 128; i++) {
        gXglCdBlockStream.registeredIds[i] = 0;
        gXglCdBlockStream.unk_3F54[i] = 0;
    }
    xenoActorEventRunStep();
    return xenoActorLightStateInit();
}

s32 func_0028D670();

/* Gets a new entry id from func_0028D670 and appends it to registeredIds. */
s32 xglCdRegisterEntry(void) {
    s32 entryId;
    s32 newRegisteredIdCount;
    s32 *idSlot;
    XglCdBlockStream *blockStream;

    entryId = func_0028D670();
    blockStream = &gXglCdBlockStream;
    newRegisteredIdCount = blockStream->registeredIdCount + 1;
    idSlot = &blockStream->registeredIds[blockStream->registeredIdCount];
    blockStream->registeredIdCount = newRegisteredIdCount;
    *idSlot = entryId;
    return entryId;
}
