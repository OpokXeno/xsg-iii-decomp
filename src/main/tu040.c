#include "common.h"

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;

extern unsigned char gPxyModelRenderStateStorage[];

/* The 16-byte tail of a batch table. As in X2's PxyBatchTableTail the record is doubleword aligned: with a
 * word-aligned record GCC folds the field offset into the loop pointer, the original keeps it as the store offset. */
typedef union PxyBatchTableTail {
    struct {
        s32 unk2000;
        s32 unk2004;
        s32 count;                  /* +0x2008 */
        s32 unk200C;
    };
    unsigned long long doublewords[2];
} PxyBatchTableTail;

/* One 0x2010-byte batch table (nml_model_set_045.c's PxyBatchTable, batches left opaque here). */
typedef struct PxyBatchTable {
    u8 batches[0x2000];
    PxyBatchTableTail tail;         /* +0x2000 */
} PxyBatchTable;

/* Module globals at gPxyModelRenderStateStorage (pxy); offsets as used by the functions below, gaps are unknown. */
struct PxyModelRenderState {
    u8 unk000[0x180];
    float lightSlotTable[24][4];    /* +0x180: X2 pxyLightSlotTable; x of the first seven rows set to 1 */
    u8 unk300[0x670 - 0x300];
    s32 overlayBatchCount;          /* +0x670 (nml_model_set_045.c's name) */
    s32 unk674;
    s32 unk678;
    s32 unk67C;
    s32 packetSlotWriteCursor;
    void *firstBatch;               /* +0x684 (nml_model_set_045.c's name) */
    u8 unk688[0x4];
    s32 unk68C;
    s32 unk690;
    s32 unk694;                     /* +0x694: cleared with the entry table */
    s32 drawFlags;                  /* +0x698 */
    s32 drawCallbackCount;          /* +0x69C: counted up by cfePlugIn.c's func_0019ED68 */
    u8 unk6A0[0x4];
    void (*drawCallback)(int);      /* +0x6A4 */
    u8 unk6A8[0x4];
    s32 shadowCasterMode;
    u8 unk6B0[0xC];
    s32 entryGroupMask;             /* +0x6BC */
    s32 entryCount;                 /* +0x6C0 */
    u8 unk6C4[0x14];
    s32 unk6D8;
    s32 translucentEntryCursor;
    float opaqueEntryUsagePercent;  /* +0x6E0 */
    float translucentEntryUsagePercent; /* +0x6E4 */
    u8 unk6E8[0x8];
    s32 unk6F0;
    s32 unk6F4;
    u8 unk6F8[0x8];
    void (*drawCallbacks[16])(int); /* +0x700 */
    u8 unk740[0xA70 - 0x740];
    PxyBatchTable batchTables[2];   /* +0xA70: one per draw pass */
};

/* Typed view of gPxyModelRenderStateStorage as the module globals. */
#define gPxyModelRenderState (*(struct PxyModelRenderState *) gPxyModelRenderStateStorage)

void *memset(void *destination, int value, unsigned int size);

/* Empties both batch tables and the batch list words (X2 pxyResetBatchTables). */
void func_0019E148(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        gPxyModelRenderState.batchTables[i].tail.count = 0;
        gPxyModelRenderState.batchTables[i].tail.unk2000 = 0;
        gPxyModelRenderState.batchTables[i].tail.unk2004 = 0;
        memset(gPxyModelRenderState.batchTables[i].batches, 0, sizeof(gPxyModelRenderState.batchTables[i].batches));
    }
    gPxyModelRenderState.unk678 = 0;
    gPxyModelRenderState.overlayBatchCount = 0;
    gPxyModelRenderState.unk674 = 0;
    gPxyModelRenderState.firstBatch = 0;
    gPxyModelRenderState.unk6F0 = 0;
    gPxyModelRenderState.unk67C = 0;
}

void pxyFinishInit(void) {
    func_0019E148();
}

void pxyClearPacketSlots(void) {
    s32 packetSlotNumber;
    s32 *slotPacket;

    gPxyModelRenderState.packetSlotWriteCursor = 0;
    slotPacket = &((s32 *) gPxyModelRenderStateStorage)[0x12B0];
    for (packetSlotNumber = 0; packetSlotNumber < 4; packetSlotNumber++) {
        *slotPacket = 0;
        slotPacket -= 4;
    }
}

void pxyClearPacketSlots();

void pxyInitPacketSlots(void) {
    pxyClearPacketSlots();
}

INCLUDE_ASM("asm/main/nonmatchings/tu040", pxyResetGlobals);

void pxyResetGlobals();

void pxyInitGlobals(void) {
    struct PxyModelRenderState *renderBlock;

    pxyResetGlobals();
    renderBlock = &gPxyModelRenderState;
    renderBlock->shadowCasterMode = 1;
    renderBlock->translucentEntryCursor = 0x1000;
    renderBlock->unk68C = 0;
    renderBlock->unk690 = 0;
    renderBlock->unk6D8 = 0;
    renderBlock->unk6F4 = 0;
}

void pxyInitGlobals();

void pxyInitModelCounters(void) {
    pxyInitGlobals();
}

/* The global colour scale nml_model_set_044.c's func_0019F1C0 sets (X2 pxyGlobalColor). */
extern Vector4 D_004C8B10;

/* X2 pxyClearEntryTable. */
void func_0019E410(void)
{
    int i;

    D_004C8B10.x = 1.0f;
    D_004C8B10.y = 1.0f;
    D_004C8B10.z = 1.0f;
    D_004C8B10.w = 1.0f;
    gPxyModelRenderState.entryCount = 0;
    gPxyModelRenderState.entryGroupMask = 0;
    gPxyModelRenderState.unk694 = 0;
    for (i = 0; i < 16; i++) {
        gPxyModelRenderState.drawCallbacks[i] = 0;
    }
    gPxyModelRenderState.drawCallbackCount = 0;
    gPxyModelRenderState.drawFlags = 0;
    gPxyModelRenderState.drawCallback = 0;
}

extern int pxyCameraEnabled[4];
void func_001975C8(void *light);

/* X2 pxyInitTables: camera 0 on, the default light at the start of the block, the light slot table cleared with
 * unit x in its first seven rows, the usage percentages cleared, then the entry table. */
void func_0019E480(void)
{
    pxyCameraEnabled[0] = 1;
    pxyCameraEnabled[1] = 0;
    pxyCameraEnabled[2] = 0;
    pxyCameraEnabled[3] = 0;
    func_001975C8(&gPxyModelRenderState);
    memset(gPxyModelRenderState.lightSlotTable, 0, sizeof(gPxyModelRenderState.lightSlotTable));
    gPxyModelRenderState.lightSlotTable[0][0] = 1.0f;
    gPxyModelRenderState.lightSlotTable[1][0] = 1.0f;
    gPxyModelRenderState.lightSlotTable[2][0] = 1.0f;
    gPxyModelRenderState.lightSlotTable[3][0] = 1.0f;
    gPxyModelRenderState.lightSlotTable[4][0] = 1.0f;
    gPxyModelRenderState.lightSlotTable[5][0] = 1.0f;
    gPxyModelRenderState.lightSlotTable[6][0] = 1.0f;
    gPxyModelRenderState.opaqueEntryUsagePercent = 0;
    gPxyModelRenderState.translucentEntryUsagePercent = 0;
    func_0019E410();
}
