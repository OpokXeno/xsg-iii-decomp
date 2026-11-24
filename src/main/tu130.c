#include "common.h"

typedef signed char s8;

typedef unsigned int u32;

/* One packet head (0x90 bytes). Kept as one byte array: a field split changes the object. */
typedef struct SdvPacketHead {
    unsigned char unmodeled_00[0x90];
} SdvPacketHead;

/* One packet record (0x18 bytes); sdvPacketClaim clears it when the record is claimed. */
typedef struct SdvPacketRecord {
    unsigned char unmodeled_00[0x18];
} SdvPacketRecord;

/* The packet table (0xF50 bytes): 16 heads, 64 records, then their in-use flags. */
typedef struct SdvPacketTable {
    SdvPacketHead heads[16];
    SdvPacketRecord packetRecords[64];
    s8 headUsed[16];
    s8 recordUsed[64];
} SdvPacketTable;

extern SdvPacketTable gSdvPacketTable;

/* Returns the head at headIndex when its in-use flag is set, otherwise 0. */
void *sdvPacketHeadGet(u32 headIndex) {
    if (headIndex < 16 && gSdvPacketTable.headUsed[headIndex] == 1) {
        return &gSdvPacketTable.heads[headIndex];
    }
    return 0;
}

/* Returns the record at recordIndex when its in-use flag is set, otherwise 0. */
void *sdvPacketRecordGet(u32 recordIndex) {
    if (recordIndex < 64 && gSdvPacketTable.recordUsed[recordIndex] == 1) {
        return &gSdvPacketTable.packetRecords[recordIndex];
    }
    return 0;
}

typedef int s32;

/* Stub: returns 0. No caller in the tree reads it. */
s32 sefFlagGetOff(void) {
    return 0;
}

/* Stub: returns 0. No caller in the tree reads it. */
s32 sefFlagGetOffB(void) {
    return 0;
}

/* Stub: returns 0. No caller in the tree reads it. */
s32 sefFlagGetOffC(void) {
    return 0;
}
