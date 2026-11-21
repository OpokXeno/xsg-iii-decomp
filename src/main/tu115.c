#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7268);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7358);

typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;

#define SLOT_SIZE 0x54
#define SCRATCHPAD_BASE 0x70000000
#define SCRATCHPAD_WORDS ((s32 *) SCRATCHPAD_BASE)
/* Word 5 of the scratchpad parameter block: the mode of the send. */
#define SCRATCHPAD_MODE SCRATCHPAD_WORDS[5]

/* One slot record, SLOT_SIZE bytes. Only the bytes the file reads are named. */
typedef struct SlotControlBlock {
    u8 hookIndex;               /* +0x00: index of the release hook in gSlotReleaseHooks */
    u8 unmodeled_01[0x03];
    s32 issuedCount;
    u8 unmodeled_08[0x40 - 0x08];
    s32 doneCount;
    u8 unmodeled_44[0x50 - 0x44];
    u32 controlFlags;                  /* +0x50 */
} SlotControlBlock;

/* Release hook called for a record before the record is cleared; gSlotReleaseHooks[hookIndex]. */
typedef void (*SlotHook)(void);

extern SlotHook gSlotReleaseHooks[];
extern SlotControlBlock D_0061BD38[];
extern SlotControlBlock gFixedSlotControl[];
extern u8 D_0061BE35[];

void *memset(void *destination, int value, unsigned int count);
void func_001E8318(s32 command, s32 sendParams);
void slotCallReleaseHookAndClear(SlotControlBlock *slotControl);
void slotRelease(s32 slotIndex);
void func_001E7358(SlotControlBlock *slotControl, s32 fixedValue);

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

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7518);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7890);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E78E8);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7990);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7A28);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7A88);

s32 func_001E7A88(s32 slotIndex, s32 setupValue, s32 valueCount, s32 mode);

s32 slotSetupMode0(s32 slotIndex, s32 setupValue, s32 valueCount) {
    return func_001E7A88(slotIndex, setupValue, valueCount, 0);
}

void slotSetupMode1(s32 slotIndex, s32 setupValue, s32 valueCount) {
    func_001E7A88(slotIndex, setupValue, valueCount, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7C50);

void func_001E7C50(s32 slotIndex, s32 updateValue, s32 valueCount, s32 mode);

void slotUpdateMode0(s32 slotIndex, s32 updateValue, s32 valueCount) {
    func_001E7C50(slotIndex, updateValue, valueCount, 0);
}

void slotUpdateMode1(s32 slotIndex, s32 updateValue, s32 valueCount) {
    func_001E7C50(slotIndex, updateValue, valueCount, 1);
}

extern u8 D_0061B018[];
extern u8 D_0061BF38[];
SlotControlBlock *func_001E7890(s32 slotIndex, s32 kind, void *table);
void func_001E7990(s32 kind, SlotControlBlock *slotControl, void *table);

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

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7EC8);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E7F88);

void func_001E7F88(s32 id, s32 subId, s32 sendParams, s32 mode);

void scratchpadSendMode0(s32 id, s32 subId, s32 sendParams) {
    func_001E7F88(id, subId, sendParams, 0);
}

void scratchpadSendMode1(s32 id, s32 subId, s32 sendParams) {
    func_001E7F88(id, subId, sendParams, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E8108);

void scratchpadSendCmd0A(s32 sendParams) {
    func_001E8318(0xA, sendParams);
}

void scratchpadSendCmd1F(s32 sendParams) {
    func_001E8318(0x1F, sendParams);
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
    func_001E8318(0x23, SCRATCHPAD_BASE);
}

void scratchpadSendCmd23Mode1(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 1;
    func_001E8318(0x23, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode2(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 2;
    func_001E8318(0x27, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode3(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 3;
    func_001E8318(0x27, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode4(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 4;
    func_001E8318(0x27, SCRATCHPAD_BASE);
}

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E82E8);

void slotReleaseSlot1(void) {
    slotRelease(1);
}

extern SlotControlBlock D_0061BA98[];
extern u8 D_0061BFB8[];

void slotRelease(s32 slotIndex) {
    SlotControlBlock *slotControl;

    slotControl = &D_0061BD38[slotIndex];
    if (slotControl != 0) {
        slotCallReleaseHookAndClear(slotControl);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E8498);

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

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E8550);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E8610);

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E8688);

s32 slotRecord1CountDiff(void)
{
    return D_0061BD38[1].issuedCount - D_0061BD38[1].doneCount;
}

s32 slotRecord2CountDiff(void)
{
    return D_0061BD38[2].issuedCount - D_0061BD38[2].doneCount;
}

void scratchpadClearSendCmd29(void) {
    memset(SCRATCHPAD_WORDS, 0, 0x38);

    func_001E8318(0x29, SCRATCHPAD_BASE);
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

INCLUDE_ASM("asm/main/nonmatchings/tu115", func_001E87D0);

void slotHookNoop(void) {

}
