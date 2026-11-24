#include "common.h"
#define SLOT_SIZE 0x54

typedef unsigned char u8;

typedef unsigned int u32;

typedef int s32;

typedef struct SlotControlBlock {
    u8 hookIndex; /* +0x00: index of the release hook in gSlotReleaseHooks */
    u8 unk01;
    u8 allocIndex; /* +0x02 */
    u8 slotId;     /* +0x03: key compared by func_001E7890, 0 when the record is free */
    s32 issuedCount;
    u8 unmodeled_08[0x40 - 0x08];
    s32 doneCount;
    u8 unmodeled_44[0x50 - 0x44];
    u32 controlFlags; /* +0x50 */
} SlotControlBlock;

extern SlotControlBlock gFixedSlots[];
void *memset(void *destination, int value, unsigned int blockCount);
void slotSetFixedValue(s32 fixedValue);
void slotSendFrame(s32 packet);

typedef struct DrawAreaTail {
    u8 unmodeled_00[0x50];
    unsigned long long unk50;
    unsigned long long unk58;
    unsigned long long unk60;
    u8 unmodeled_68[0x08];
} DrawAreaTail;

extern DrawAreaTail gDrawArea;
extern s32 D_00550FFC;
extern SlotControlBlock D_0061BEE0[];
extern SlotControlBlock D_0061B018[];
extern SlotControlBlock *D_0061BF38[];
extern SlotControlBlock *D_0061BFB8[];
extern SlotControlBlock D_0061BA98[];

typedef struct DrawCallbackBlock {
    u8 unmodeled_00[0x08];
    unsigned short unk08;
    u8 unmodeled_0A[0x38 - 0x0A];
    void (*unk38)(void);
    u8 unmodeled_3C[0x64 - 0x3C];
    void (*unk64)(s32 fixedValue);
} DrawCallbackBlock;

extern DrawCallbackBlock D_004C8390;

/* Clears the slot tables and the draw-area record, then installs the two callbacks of D_004C8390. */
void func_001E7268(void) {
    D_00550FFC = 0;
    memset(D_0061B018, 0, SLOT_SIZE * 32);
    memset(D_0061BF38, 0, sizeof(SlotControlBlock *) * 32);
    memset(D_0061BA98, 0, SLOT_SIZE * 8);
    memset(D_0061BFB8, 0, sizeof(SlotControlBlock *) * 8);
    memset(gFixedSlots, 0, SLOT_SIZE * 5);
    memset(D_0061BEE0, 0, SLOT_SIZE);
    memset(&gDrawArea, 0, 0x70);
    gDrawArea.unk58 = 0x30000;
    gDrawArea.unk60 = 0;
    gDrawArea.unk50 = ((unsigned long long) 0x9880 << 17) | D_004C8390.unk08;
    D_004C8390.unk38 = slotSendFrame;
    D_004C8390.unk64 = slotSetFixedValue;
}

typedef void (*SlotHandler)(SlotControlBlock *slotControl, s32 fixedValue);

extern SlotHandler gSlotHandlers[];

typedef struct SbStudioBlockHead {
    unsigned long long flags;
    u8 unmodeled_08[0x10];
    s32 unk18;
} SbStudioBlockHead;

extern SbStudioBlockHead gSbStudioBlock;
void drawEnvSetCameraScissor(SlotControlBlock *slotControl, s32 fixedValue);
void func_001E7358(SlotControlBlock *slotControl, s32 fixedValue);

/* Runs the handler of one slot record (hookIndex 1..48 in gSlotHandlers) and updates its control flags. */
void func_001E7358(SlotControlBlock *slotControl, s32 fixedValue) {
    u32 controlFlags;

    if (slotControl == 0) {
        return;
    }
    if (slotControl->hookIndex == 0) {
        return;
    }
    if (slotControl->hookIndex >= 49) {
        return;
    }
    if (gSlotHandlers[slotControl->hookIndex] == 0) {
        return;
    }
    drawEnvSetCameraScissor(slotControl, fixedValue);
    gSlotHandlers[slotControl->hookIndex](slotControl, fixedValue);
    if (slotControl->unk01 == 0) {
        slotControl->unk01++;
    }
    controlFlags = slotControl->controlFlags & ~1;
    slotControl->controlFlags = controlFlags;
    if ((gSbStudioBlock.flags & 1) != 0 && gSbStudioBlock.unk18 != 0 && slotControl->hookIndex != 27) {
        slotControl->controlFlags = controlFlags | 1;
    }
    slotControl->controlFlags &= ~8;
}

#define SCRATCHPAD_BASE 0x70000000
#define SCRATCHPAD_WORDS ((s32 *) SCRATCHPAD_BASE)
/* Word 5 of the scratchpad parameter block: the sendMode of the send. */
#define SCRATCHPAD_MODE SCRATCHPAD_WORDS[5]

/* Release hook called for a record before the record is cleared; gSlotReleaseHooks[hookIndex]. */
typedef void (*SlotHook)(void);

extern SlotHook gSlotReleaseHooks[];
extern SlotControlBlock gFixedSlotControl[];
extern u8 D_0061BE35[];
void slotSendCommand(s32 command, s32 sendParams);
void slotCallReleaseHookAndClear(SlotControlBlock *slotControl);
void slotRelease(s32 slotIndex);

void slotCallReleaseHookAndClear(SlotControlBlock *slotControl) {
    SlotHook releaseHook;

    if (slotControl != 0) {
        releaseHook = gSlotReleaseHooks[slotControl->hookIndex];
        if (releaseHook != 0) {
            releaseHook();
        }
        memset(slotControl, 0, SLOT_SIZE);
    }
}

void slotSetFixedValue(s32 fixedValue) {
    u32 controlFlags;

    func_001E7358(gFixedSlotControl, fixedValue);
    controlFlags = gFixedSlotControl->controlFlags;
    if ((controlFlags >> 1) & 1) {
        slotRelease(4);
    }
}

void slotReleaseChecked(s32 slotIndex, s32 forceRelease);
void slotReleaseKind8Checked(s32 slotIndex);

typedef struct StudioCamera {
    u8 unmodeled_00[0x60];
    float unk60;
    float unk64;
    float unk68;
    float unk6C;
} StudioCamera;

extern unsigned short movieFramebufferBasePage[];
StudioCamera *xglStudioSelectGetActiveCamera(s32 index);
s32 xglRenderMovieIsActive(void);
s32 stateIsReady(void);
void xglPacketSendFixedHeader(void);
void drawEnvSetHalfDraw(void);
void sceVif1PkCnt(s32 packet, s32 blockCount);
void sceVif1PkAddDataN(s32 packet, u8 *data, s32 blockCount);
extern u8 D_00407FC0[];
extern u8 D_00408020[];

/* Builds the active studio camera window packet and sends the slot records of the frame through VIF1. */
void slotSendFrame(s32 packet) {
    u8 *packetWords;
    const StudioCamera *camera;
    unsigned long long packed;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 i;
    s32 j;
    s32 busy;
    s32 notReady;
    SlotControlBlock *slotRecord;

    packetWords = D_00407FC0;
    camera = xglStudioSelectGetActiveCamera(0);
    x0 = camera->unk60;
    y0 = camera->unk64;
    x1 = camera->unk68;
    y1 = camera->unk6C;
    packed =
        ((unsigned long long) (long long) (x0) | ((unsigned long long) (long long) (x1 - 1) << 16) |
         ((unsigned long long) (long long) (y0) << 32) | ((unsigned long long) (long long) (y1 - 1) << 48));
    {
        unsigned long long *pkt = (unsigned long long *) D_00407FC0;
        pkt[10] = packed;
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, pkt, 24);
    }

    for (i = 0; i < 32; i++) {
        if (D_0061BF38[i] == 0) {
            break;
        }
        if (D_0061BF38[i]->hookIndex == 27 || (D_0061BF38[i]->controlFlags & 1)) {
            func_001E7358(D_0061BF38[i], packet);
        }
    }
    for (i = 0; i < 8; i++) {
        if (D_0061BFB8[i] == 0) {
            break;
        }
        func_001E7358(D_0061BFB8[i], packet);
    }
    for (i = 0; i < 5; i++) {
        if (i != 4) {
            func_001E7358(&gFixedSlots[i], packet);
        }
    }
    func_001E7358(&D_0061BEE0[0], packet);

    for (i = 0; i < 32; i++) {
        if ((D_0061B018[i].controlFlags >> 1) & 1) {
            slotReleaseChecked(D_0061B018[i].slotId, 0);
        }
    }
    for (i = 0; i < 8; i++) {
        if ((D_0061BA98[i].controlFlags >> 1) & 1) {
            slotReleaseKind8Checked(D_0061BA98[i].slotId);
        }
    }
    for (i = 0; i < 5; i++) {
        if (((gFixedSlots[i].controlFlags >> 1) & 1) && i != 4) {
            slotRelease(i);
        }
    }
    if ((D_0061BEE0[0].controlFlags >> 1) & 1) {
        slotReleaseChecked(D_0061BEE0[0].slotId, 0);
    }

    if (xglRenderMovieIsActive() != 0) {
        busy = 0;
        for (j = 0; j < 5; j++) {
            if (gFixedSlots[j].hookIndex != 0) {
                busy = 1;
                break;
            }
        }
        busy |= stateIsReady() == 0;
        if (!busy) {
            xglPacketSendFixedHeader();
            drawEnvSetHalfDraw();
            ((void (*)()) drawEnvSetCameraScissor)();
        }
    }

    {
        unsigned long long *pkt = (unsigned long long *) D_00408020;
        pkt[8] = ((unsigned long long) 0xFF << 56) | ((unsigned long long) 0x8 << 16) |
                 movieFramebufferBasePage[0];
        pkt[10] = ((unsigned long long) 0x1BF << 48) | ((unsigned long long) 0x1FF << 16);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, pkt, 24);
    }
}

SlotControlBlock *func_001E7890(s32 slotIndex, s32 entryCount, u8 *slotTable);

/* Returns the record in a table of 0x54-byte blocks whose slotId equals slotIndex, or 0. */
SlotControlBlock *func_001E7890(s32 slotIndex, s32 entryCount, u8 *slotTable) {
    SlotControlBlock *tableEntry;
    s32 i;

    if (slotIndex == 0) {
        return 0;
    }
    i = 0;
    if (entryCount > 0) {
        tableEntry = (SlotControlBlock *) slotTable;
        slotTable += 3;
        do {
            if (*slotTable == slotIndex) {
                return tableEntry;
            }
            tableEntry++;
            slotTable += SLOT_SIZE;
            i++;
        } while (i < entryCount);
    }
    return 0;
}

/* Claims a free block of blocks for kind, registers it in table, and returns it (0 when none is free). */
SlotControlBlock *slotClaimRecord(s32 slotKind, s32 blockCount, SlotControlBlock *blocks, SlotControlBlock **slotTable) {
    s32 blockIndex;
    s32 tableIndex;

    if (slotKind == 0) {
        return 0;
    }
    blockIndex = 0;
    if (blockCount > 0) {
        for (; blockIndex < blockCount; blockIndex++) {
            if (blocks[blockIndex].slotId == 0) {
                for (tableIndex = 0; tableIndex < blockCount; tableIndex++) {
                    if (slotTable[tableIndex] == 0) {
                        slotTable[tableIndex] = &blocks[blockIndex];
                        blocks[blockIndex].slotId = slotKind;
                        blocks[blockIndex].allocIndex = blockIndex;
                        return &blocks[blockIndex];
                    }
                }
            }
        }
    }
    return 0;
}

void func_001E7990(s32 slotKind, SlotControlBlock *slotControl, SlotControlBlock **slotTable);

void func_001E7990(s32 slotKind, SlotControlBlock *slotControl, SlotControlBlock **slotTable) {
    SlotControlBlock **tableEntry;
    s32 readIndex;
    s32 skipIndex;
    s32 writeIndex;

    if (slotControl == 0) {
        return;
    }
    tableEntry = slotTable;
    writeIndex = 0;
    if (slotKind > 0 && slotTable[0] != 0) {
        readIndex = (slotTable[0] == slotControl);
        slotTable[0] = slotTable[readIndex];
    loop:
        readIndex++;
        skipIndex = readIndex + 1;
        tableEntry++;
        writeIndex++;
        if (readIndex >= slotKind || slotTable[readIndex] == 0) {
            goto done;
        }
        if (*tableEntry == slotControl) {
            readIndex++;
        }
        *tableEntry = slotTable[readIndex];
        goto loop;
    }
done:
    slotTable[writeIndex] = 0;
}

s32 slotFindFreeId(void) {
    SlotControlBlock *slotTable;
    s32 slotId;
    s32 entryIndex;

    slotTable = D_0061B018;
    for (slotId = 1; slotId < 32; slotId++) {
        for (entryIndex = 0; entryIndex < 32; entryIndex++) {
            if (slotTable[entryIndex].slotId == slotId) {
                goto found;
            }
        }
        return slotId;
    found:;
    }
    return 0;
}

typedef struct SlotSendBlock {
    s32 words[15];
} SlotSendBlock;

/* The 0x54-byte slot block viewed as a send record: sendBlock at +0x04 is kept apart from SlotControlBlock.issuedCount
   because the field types decide the emitted order. */
typedef struct SlotSendRecord {
    u8 hookIndex; /* +0x00: index of the release hook in gSlotReleaseHooks */
    u8 unk01;
    u8 allocIndex;           /* +0x02 */
    u8 slotId;               /* +0x03: 0 when the record is free */
    SlotSendBlock sendBlock; /* +0x04: 60-byte send block */
    s32 doneCount;
    u8 unmodeled_44[0x50 - 0x44];
    u32 controlFlags; /* +0x50 */
} SlotSendRecord;

typedef struct SlotGlobalFlags {
    u32 enabled : 1;
    u32 rest : 31;
    s32 unmodeled_04[7];
} SlotGlobalFlags;

extern SlotGlobalFlags gSlotDelayEnabled;

/* Bitfield view of a slot record (controlFlags at +0x50); the same layout as SlotSendRecord up to +0x40. */
typedef struct SlotControlBits {
    u8 hookIndex;
    u8 unk01;
    u8 allocIndex;
    u8 slotId;
    SlotSendBlock sendBlock;
    s32 unmodeled_40[4];
    u32 active : 1;
    u32 sendMode : 1;
    u32 bit2 : 1;
    u32 delayed : 1;
    u32 bit4 : 1;
    u32 bit5 : 1;
    u32 rest : 26;
} SlotControlBits;

s32 slotSetupRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode);

/* Stores a send for subId in the D_0061B018 record of id (claiming one when absent); returns the slot id. */
s32 slotSetupRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode) {
    SlotControlBits *slotControl = (SlotControlBits *) func_001E7890(id, 32, (u8 *) D_0061B018);

    if (subId == 0) {
        return 0;
    }
    if (slotControl == 0) {
        if (id == 0) {
            id = slotFindFreeId();
        }
        slotControl = (SlotControlBits *) slotClaimRecord(id, 32, D_0061B018, D_0061BF38);
        if (slotControl == 0) {
            return 0;
        }
    }
    if (sendParams == 0) {
        slotReleaseChecked(id, 0);
    }
    slotControl->hookIndex = subId;
    slotControl->sendMode = sendMode;
    slotControl->active = 1;
    slotControl->delayed = gSlotDelayEnabled.enabled;
    if (subId == 41) {
        slotControl->bit5 = 1;
    }
    slotControl->sendBlock = *sendParams;
    return id;
}

s32 slotSetupMode0(s32 slotIndex, s32 setupValue, s32 valueCount) {
    return slotSetupRecord(slotIndex, setupValue, valueCount, 0);
}

void slotSetupMode1(s32 slotIndex, s32 setupValue, s32 valueCount) {
    slotSetupRecord(slotIndex, setupValue, valueCount, 1);
}

s32 xglPacketGetCurrent(void);
s32 func_001E7C50(s32 id, s32 hook, SlotSendBlock *sendParams, s32 sendMode);

/* Finds the record of slotRecord id (or claims a free one when id is 0 or new), stores hook, sendParams and sendMode,
   then runs its update. Returns the slotRecord id, or 0 when no record is free. */
s32 func_001E7C50(s32 id, s32 hook, SlotSendBlock *sendParams, s32 sendMode) {
    SlotControlBits *record;
    SlotControlBits *freeRecord;
    s32 recordIndex;
    s32 i;
    s32 packet;

    packet = xglPacketGetCurrent();
    if (hook) {
        record = 0;
        freeRecord = 0;
        recordIndex = 0;
        if (id == 0) {
            id = slotFindFreeId();
        }
        for (i = 0; i < 32; i++) {
            record = (SlotControlBits *) &D_0061B018[i];
            if (record->slotId == id) {
                recordIndex = i;
                break;
            }
            if (record->slotId == 0) {
                freeRecord = record;
                recordIndex = i;
            }
        }
        if (i == 32) {
            record = freeRecord;
        }
        if (record == 0) {
            return 0;
        }
        record->hookIndex = hook;
        record->allocIndex = recordIndex;
        record->slotId = id;
        record->bit2 = sendMode;
        record->active = 1;
        if (sendParams) {
            record->sendBlock = *sendParams;
        } else {
            memset(&record->sendBlock, 0, sizeof(SlotSendBlock));
        }
        func_001E7358((SlotControlBlock *) record, packet);
    }
    return id;
}

void slotUpdateMode0(s32 slotIndex, s32 updateValue, s32 valueCount) {
    func_001E7C50(slotIndex, updateValue, valueCount, 0);
}

void slotUpdateMode1(s32 slotIndex, s32 updateValue, s32 valueCount) {
    func_001E7C50(slotIndex, updateValue, valueCount, 1);
}

void slotReleaseChecked(s32 slotIndex, s32 forceRelease)
{
    SlotControlBlock **slotControlPtr;
    SlotControlBlock *slotControl;

    slotControlPtr = &slotControl;
    slotControl = func_001E7890(slotIndex, 0x20, D_0061B018);
    if (*slotControlPtr != 0 && (forceRelease != 0 || ((*slotControlPtr)->controlFlags & 0x22) != 0x20)) {
        func_001E7990(0x20, *slotControlPtr, D_0061BF38);
        slotCallReleaseHookAndClear(slotControl);
    }
}

void func_001E8550(void);

/* Releases the D_0061BF38 records unless one is pinned (flag 5 set, flag 1 clear), clears both pools, then runs func_001E8550. */
void slotReleaseAllPools(void) {
    s32 i;
    SlotControlBlock *slotControl;
    u32 controlFlags;

    for (i = 0; i < 32; i++) {
        slotControl = D_0061BF38[i];
        if (slotControl != 0) {
            controlFlags = slotControl->controlFlags;
            if (((controlFlags >> 1) & 1) == 0 && ((controlFlags >> 5) & 1) != 0) {
                return;
            }
            slotCallReleaseHookAndClear(slotControl);
        }
    }
    memset(D_0061B018, 0, SLOT_SIZE * 32);
    memset(D_0061BF38, 0, sizeof(SlotControlBlock *) * 32);
    func_001E8550();
}

void slotSendPoolRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode);

/* Stores a send for subId in the D_0061BA98 pool record of id (claiming one when absent). */
void slotSendPoolRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode) {
    SlotSendRecord *slotControl;
    u32 controlFlags;

    slotControl = (SlotSendRecord *) func_001E7890(id, 8, (u8 *) D_0061BA98);
    if (subId == 0) {
        return;
    }
    if (slotControl == 0) {
        slotControl = (SlotSendRecord *) slotClaimRecord(id, 32, D_0061BA98, D_0061BFB8);
        if (slotControl == 0) {
            return;
        }
    }
    controlFlags = slotControl->controlFlags;
    slotControl->hookIndex = subId;
    controlFlags &= ~2;
    controlFlags |= (sendMode & 1) << 1;
    controlFlags |= 1;
    slotControl->controlFlags = controlFlags;
    if (sendParams != 0) {
        slotControl->sendBlock = *sendParams;
    } else {
        memset(&slotControl->sendBlock, 0, 60);
    }
}

void scratchpadSendMode0(s32 id, s32 subId, s32 sendParams) {
    slotSendPoolRecord(id, subId, sendParams, 0);
}

void scratchpadSendMode1(s32 id, s32 subId, s32 sendParams) {
    slotSendPoolRecord(id, subId, sendParams, 1);
}

s32 slotRecord1CountDiff(void);

/* Sets or clears bit 4 of the controlFlags of gFixedSlots[1] when that slot runs hook 10 with pending counts. */
void func_001E8108(s32 enableFlag) {
    u32 bit;

    if (gFixedSlots[1].hookIndex == 10 && slotRecord1CountDiff()) {
        bit = (u32) enableFlag > 0;
        gFixedSlots[1].controlFlags = (gFixedSlots[1].controlFlags & ~0x10) | (bit << 4);
    }
}

void scratchpadSendCmd0A(s32 sendParams) {
    slotSendCommand(0xA, sendParams);
}

void scratchpadSendCmd1F(s32 sendParams) {
    slotSendCommand(0x1F, sendParams);
}

void scratchpadLoadParams(s32 *sendParams) {
    SCRATCHPAD_WORDS[0] = sendParams[0];
    SCRATCHPAD_WORDS[1] = sendParams[1];
    SCRATCHPAD_WORDS[2] = sendParams[2];
    SCRATCHPAD_WORDS[3] = sendParams[3];
    SCRATCHPAD_WORDS[4] = sendParams[4];
}

void scratchpadSendCmd23Mode0(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 0;
    slotSendCommand(0x23, SCRATCHPAD_BASE);
}

void scratchpadSendCmd23Mode1(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 1;
    slotSendCommand(0x23, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode2(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 2;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode3(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 3;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode4(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 4;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

/* Sends the parameter block as command 0x27 in mode 5. */
void func_001E82E8(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 5;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

typedef struct SlotSend2 {
    s32 w[2];
} SlotSend2;

typedef struct SlotSend3 {
    s32 w[3];
} SlotSend3;

typedef struct SlotSend6 {
    s32 w[6];
} SlotSend6;

extern u8 D_0061BE34[];

typedef struct SlotSendView {
    u8 hookIndex;
    u8 busy;
    u8 unmodeled_02[2];
    union {
        SlotSend6 six;
        struct {
            SlotSend3 three;
            s32 wasBusy;
        } withThree;
        struct {
            SlotSend2 two;
            s32 wasBusy;
        } withTwo;
        s32 words[15];
    } send;
    s32 doneCount;
    s32 unmodeled_44[4];
} SlotSendView;

extern SlotSendView D_0061BD8C;
extern SlotSendView D_0061BDE0;

/* Copies a send payload into the slot view that matches command: 0x27/0x28 the fixed slots, 0x1F/0x0A D_0061BD8C, 0x23 D_0061BDE0, 0x29 marks D_0061BE34 busy. */
void slotSendCommand(s32 command, s32 sendParams) {
    SlotSendView *slotRecord;

    if (sendParams == 0) {
        return;
    }
    switch (command) {
    case 0x27:
        slotRecord = (SlotSendView *) gFixedSlots;
        goto tail;
    case 0x28:
        slotRecord = (SlotSendView *) gFixedSlotControl;
        goto tail;
    case 0x1F:
        slotRecord = &D_0061BD8C;
        slotRecord->hookIndex = command;
        slotRecord->send.withThree.three = *(SlotSend3 *) sendParams;
        slotRecord->send.withThree.wasBusy = slotRecord->busy != 0;
        break;
    case 0x0A:
        slotRecord = &D_0061BD8C;
        slotRecord->hookIndex = command;
        slotRecord->send.withTwo.two = *(SlotSend2 *) sendParams;
        slotRecord->send.withTwo.wasBusy = slotRecord->busy != 0;
        break;
    case 0x23:
        slotRecord = &D_0061BDE0;
    tail:
        slotRecord->hookIndex = command;
        slotRecord->doneCount = 0;
        slotRecord->send.six = *(SlotSend6 *) sendParams;
        break;
    case 0x29:
        slotRecord = (SlotSendView *) D_0061BE34;
        slotRecord->hookIndex = command;
        slotRecord->busy = 1;
        break;
    default:
        break;
    }
}

void slotReleaseSlot1(void) {
    slotRelease(1);
}

void slotRelease(s32 slotIndex) {
    SlotControlBlock *slotControl;

    slotControl = &gFixedSlots[slotIndex];
    if (slotControl != 0) {
        slotCallReleaseHookAndClear(slotControl);
    }
}

/* Releases the five fixed slots gFixedSlots[0..4]. */
void slotReleaseFixedSlots(void) {
    s32 slotIndex;

    for (slotIndex = 0; slotIndex < 5; slotIndex++) {
        slotRelease(slotIndex);
    }
}

void slotReleaseKind8Checked(s32 slotIndex)
{
    u32 controlFlags;
    SlotControlBlock **slotControlPtr;
    SlotControlBlock *slotControl;

    slotControlPtr = &slotControl;
    slotControl = func_001E7890(slotIndex, 8, D_0061BA98);
    if (*slotControlPtr != 0) {
        controlFlags = (*slotControlPtr)->controlFlags;
        if ((((controlFlags >> 5) & 1) == 0) || ((controlFlags >> 1) & 1)) {
            func_001E7990(8, *slotControlPtr, D_0061BFB8);
            slotCallReleaseHookAndClear(slotControl);
        }
    }
}

/* Releases the D_0061BFB8 records unless one is pinned (flag 5 set, flag 1 clear), then clears the D_0061BA98 pool. */
void func_001E8550(void) {
    s32 i;
    SlotControlBlock *slotControl;
    u32 controlFlags;

    for (i = 0; i < 8; i++) {
        slotControl = D_0061BFB8[i];
        if (slotControl != 0) {
            controlFlags = slotControl->controlFlags;
            if (((controlFlags >> 5) & 1) != 0 && ((controlFlags >> 1) & 1) == 0) {
                return;
            }
            slotCallReleaseHookAndClear(slotControl);
        }
    }
    memset(D_0061BA98, 0, SLOT_SIZE * 8);
    memset(D_0061BFB8, 0, sizeof(SlotControlBlock *) * 8);
}

typedef union RecordWord {
    s32 word;
    u8 byte;
} RecordWord;

typedef struct RecordWords5 {
    RecordWord words[5];
} RecordWords5;

typedef struct RecordWords6 {
    RecordWords5 head;
    s32 extra;
} RecordWords6;

typedef struct FixedRecord {
    u8 slotKind;            /* +0x00 */
    RecordWords6 block; /* +0x04 */
    s32 tail;           /* +0x1C */
    u8 unmodeled_20[0x50 - 0x20];
    s32 flags; /* +0x50 */
} FixedRecord;

/* Typed view of the fixed slotRecord record D_0061BEE0 (the slotRecord-control block of the fixed record). */
#define gFixedRecord (*(FixedRecord *) D_0061BEE0)

void slotFixedRecordSetHead(const RecordWords5 *src) {
    u8 byte;

    gFixedRecord.slotKind = 29;
    gFixedRecord.flags |= 3;
    gFixedRecord.block.head = *src;
    gFixedRecord.tail = 0;
    byte = gFixedRecord.block.head.words[2].byte;
    gFixedRecord.block.extra = 0;
    gFixedRecord.block.head.words[2].word = (s32) (byte * 1.412f);
}

typedef struct SlotPayload {
    s32 words[6];
} SlotPayload;

/* Second view of the fixed record, kept apart from FixedRecord: the field types decide the emitted order
   (six plain words here, a union of bytes and words in FixedRecord.block). */
typedef struct FixedRecordPayload {
    u8 slotKind;             /* +0x00 */
    SlotPayload payload; /* +0x04 */
    s32 tail;            /* +0x1C */
    u8 unmodeled_20[0x50 - 0x20];
    s32 flags; /* +0x50 */
} FixedRecordPayload;

#define gFixedRecordPayload (*(FixedRecordPayload *) D_0061BEE0)

/* Stores a 6-word payload into the fixed record and marks it with slotKind 29 and flags 3. */
void slotFixedRecordSetPayload(const SlotPayload *src) {
    gFixedRecordPayload.slotKind = 29;
    gFixedRecordPayload.flags |= 3;
    gFixedRecordPayload.payload = *src;
    gFixedRecordPayload.tail = 0;
}

s32 slotRecord1CountDiff(void)
{
    return gFixedSlots[1].issuedCount - gFixedSlots[1].doneCount;
}

s32 slotRecord2CountDiff(void)
{
    return gFixedSlots[2].issuedCount - gFixedSlots[2].doneCount;
}

void scratchpadClearSendCmd29(void) {
    memset(SCRATCHPAD_WORDS, 0, 0x38);

    slotSendCommand(0x29, SCRATCHPAD_BASE);
}

s32 slotStateActive(void) {
    u32 stateMinusOne;

    stateMinusOne = (D_0061BE35[0] + 0xFF) & 0xFF;
    return stateMinusOne < 2U;
}

s32 slotStateInactive(void) {
    u32 stateMinusOne;

    stateMinusOne = (D_0061BE35[0] + 0xFF) & 0xFF;
    return stateMinusOne >= 2U;
}

void slotSetEnableBit(s32 slotIndex, s32 isEnabled) {
    s32 controlFlags;
    s32 flagBit;
    SlotControlBlock *slotControl;

    flagBit = isEnabled;
    slotControl = func_001E7890(slotIndex, 0x20, D_0061B018);
    flagBit = flagBit != 0;
    flagBit *= 0x10;
    if (slotControl != 0) {
        controlFlags = (slotControl->controlFlags & ~0x10) | flagBit;
        slotControl->controlFlags = controlFlags;
    }
}

/* Sets or clears bit 4 of the controlFlags of every pool record and fixed slot from isEnabled. */
void slotSetAllEnableBits(s32 isEnabled) {
    SlotControlBlock *slotRecord;
    s32 busy;
    s32 flagBits;
    s32 clearMask;
    s32 keepMask;
    s32 i;

    busy = isEnabled != 0;
    flagBits = busy << 4;
    clearMask = ~0x10;
    slotRecord = D_0061B018;
    for (i = 31; i >= 0; i--, slotRecord++) {
        if (slotRecord->slotId != 0) {
            slotRecord->controlFlags = (slotRecord->controlFlags & clearMask) | flagBits;
        }
    }
    slotRecord = gFixedSlots;
    keepMask = ~0x10;
    for (i = 4; i >= 0; i--, slotRecord++) {
        slotRecord->controlFlags = (slotRecord->controlFlags & keepMask) | ((u8) busy << 4);
    }
}

void slotHookNoop(void) {

}
