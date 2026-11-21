#include "common.h"

typedef int s32;

void func_001B6798(int handle);

typedef struct NarrEvSlot {
    int primaryHandle;                       /* +0 */
    s32 decodeRequest[8];                    /* 4..35: decode buffer, words 0 and 1 written after the memset */
    int secondaryHandle;                     /* +36 */
    unsigned char unmodeled_28[32];   /* 40..71 */
    int flag;                         /* +72 */
    unsigned char unmodeled_4C[68];   /* 76..143 */
} NarrEvSlot;

#define NARR_EV_SLOT_COUNT 4

typedef struct NarrEvState {
    unsigned char unmodeled_00[16];   /* 0..15 */
    NarrEvSlot decodeSlots[NARR_EV_SLOT_COUNT];  /* 16..591 */
} NarrEvState;

void narrEvReleaseSlots(NarrEvState *narrState) {
    int i;

    for (i = 0; i < NARR_EV_SLOT_COUNT; i++) {
        NarrEvSlot *decodeSlot = &narrState->decodeSlots[i];
        if (decodeSlot->primaryHandle != 0) {
            func_001B6798(decodeSlot->primaryHandle);
            decodeSlot->primaryHandle = 0;
            func_001B6798(decodeSlot->secondaryHandle);
            decodeSlot->secondaryHandle = 0;
            decodeSlot->flag = 0;
        }
    }
}

typedef unsigned char u8;
extern const char gNarrEvAssertMessage[];
extern const char gNarrEvAssertFile[];
void assertFailed(const char *message, const char *file, int line);
s32 func_001B6520(s32);
s32 xglCdGetFileSize(s32);
s32 xglCdReadFile(s32, s32, s32, s32);

void narrEvLoadFile(s32 *outLoadedAddr, s32 fileHandle) {
    s32 loadedAddr;
    s32 fileSize;

    fileSize = xglCdGetFileSize(fileHandle);
    loadedAddr = func_001B6520((fileSize + 0x800) & ~0x7FF);
    *outLoadedAddr = loadedAddr;
    if (loadedAddr == 0) {
        assertFailed(gNarrEvAssertMessage, gNarrEvAssertFile, 0x260);
    }
    xglCdReadFile(fileHandle, *outLoadedAddr, 0, 0);
}

void *memset(void *, int, unsigned int);
s32 xglJpegDecode(s32 *request);

void narrEvDecodeSlot(NarrEvSlot *decodeSlot) {
    s32 secondaryHandle;
    s32 primaryHandle;

    if (decodeSlot->primaryHandle != 0) {
        memset(decodeSlot->decodeRequest, 0, 0x20);
        primaryHandle = decodeSlot->primaryHandle;
        secondaryHandle = decodeSlot->secondaryHandle;
        decodeSlot->decodeRequest[0] = primaryHandle;
        decodeSlot->decodeRequest[1] = secondaryHandle;
        xglJpegDecode(decodeSlot->decodeRequest);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A868);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A8F0);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024A9A8);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AA30);

extern s32 gNarrEvActionCode[];
extern unsigned char gNarrEvMatchName[];
extern s32 gNarrEvMatchFound;
s32 func_0030E68C();

/* Table read by narrEvStartOnNameMatch and narrEvIncrementCounter: entries of 0x290 bytes start at entryBase. */
typedef struct NarrEvEntryTable {
    s32 elapsedTicks;                    /* 0x00, advanced by narrEvIncrementCounter */
    unsigned char unk_04[4];
    s32 narrEntryCount;                 /* 0x08 */
    unsigned char *narrEntryBase;       /* 0x0C */
    unsigned char unk_10[0x5C];
    s32 elapsedPaused;                       /* 0x6C */
} NarrEvEntryTable;

void narrEvStartOnNameMatch(NarrEvEntryTable *narrTable) {
    if (func_0030E68C(narrTable->narrEntryBase + (narrTable->narrEntryCount * 0x290) + 0x250, gNarrEvMatchName) != 0) {
        gNarrEvActionCode[0] = 0x3C;
        gNarrEvMatchFound = 1;
    }
}

void narrEvIncrementCounter(NarrEvEntryTable *narrTable) {
    if (narrTable->elapsedPaused == 0) {
        narrTable->elapsedTicks += 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AB18);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024ABC8);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AC68);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AD18);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AD60);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024AF50);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024B080);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024B540);

INCLUDE_ASM("asm/main/nonmatchings/NarrEv", func_0024BA78);

s32 xglCdGetServicePending();
s32 xglSleep();

void xglCdWaitForService(void) {
    while (xglCdGetServicePending() != 0) {
        xglSleep();
    }
}
