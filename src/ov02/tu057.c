/*
 * OV02 original TU 57: 0x00a96e70..0x00a975d8 (9 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned int u32;
void *memset(void *, s32, s32);

/* The battle counter block at gOv02BattleCounters (0x56C bytes): two event counts at +0x564 and +0x568. */
/* Counter records inside the block at gOv02BattleCounters: ids 1..10 are 0x18 bytes from +0xC, ids 13..16 are 0x24 bytes from +0xF8. */
typedef struct Ov02PlayerCounter { s32 battleEntryCount; s32 pad[5]; } Ov02PlayerCounter;
typedef struct Ov02WideCounter { s32 battleEntryCount; s32 pad[8]; } Ov02WideCounter;
typedef struct Ov02PlayerCounterTable { unsigned char unk_000[0xC]; Ov02PlayerCounter records[10]; } Ov02PlayerCounterTable;
typedef struct Ov02WideCounterTable { unsigned char unk_000[0xF8]; Ov02WideCounter records[4]; } Ov02WideCounterTable;
typedef struct Ov02BattleCounterBlock {
    unsigned char unk_000[0x564];
    s32 eventCountA;            /* +0x564 */
    s32 eventCountB;            /* +0x568 */
} Ov02BattleCounterBlock;

extern Ov02BattleCounterBlock gOv02BattleCounters;

void ov02BattleCountersClear(void) {
    memset(&gOv02BattleCounters, 0, 0x56C);
}

Ov02BattleCounterBlock *ov02BattleCountersGet(void) {
    return &gOv02BattleCounters;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu057", func_00A96EA8);

Ov02BattleCounterBlock *ov02BattleCountersGet();

void counterBlockIncrementById(s32 counterId) {
    Ov02BattleCounterBlock *battleCounters;

    battleCounters = ov02BattleCountersGet();
    if (counterId - 1 < 0xAU) {
        ((Ov02PlayerCounterTable *) battleCounters)->records[counterId - 1].battleEntryCount += 1;
        return;
    }
    if (counterId - 0xD < 4U) {
        ((Ov02WideCounterTable *) battleCounters)->records[counterId - 0xD].battleEntryCount += 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu057", func_00A97168);

s32 func_00A87608(s32, s32);
s32 func_00A878E0(s32, s32);
s32 func_00A87A88(s32, s32);

s32 counterCountOwnerSlots(s32 owner) {
    s32 ownerSlot;
    s32 ownerSecondSlot;
    s32 ownerThirdSlot;
    s32 ownerSlotIndex;
    s32 ownerSecondSlotIndex;
    s32 ownerThirdSlotIndex;
    s32 matchingSlotCount;

    ownerSlotIndex = 0;
    matchingSlotCount = 0;
    do {
        ownerSlot = ownerSlotIndex;
        ownerSlotIndex += 1;
        if (func_00A878E0(owner, ownerSlot) != 0) {
            matchingSlotCount = matchingSlotCount + 1;
        }
    } while (ownerSlotIndex < 0x58);
    ownerSecondSlotIndex = 0;
    do {
        ownerSecondSlot = ownerSecondSlotIndex;
        ownerSecondSlotIndex += 1;
        if (func_00A87608(owner, ownerSecondSlot) != 0) {
            matchingSlotCount = matchingSlotCount + 1;
        }
    } while (ownerSecondSlotIndex < 0x58);
    ownerThirdSlotIndex = 0;
    do {
        ownerThirdSlot = ownerThirdSlotIndex;
        ownerThirdSlotIndex += 1;
        if (func_00A87A88(owner, ownerThirdSlot) != 0) {
            matchingSlotCount = matchingSlotCount + 1;
        }
    } while (ownerThirdSlotIndex < 0x36);
    return matchingSlotCount;
}

void counterBlockIncrementEventA(void) {
    Ov02BattleCounterBlock *battleCounters;

    battleCounters = ov02BattleCountersGet();
    battleCounters->eventCountA += 1;
}

void counterBlockIncrementEventB(void) {
    Ov02BattleCounterBlock *battleCounters;

    battleCounters = ov02BattleCountersGet();
    battleCounters->eventCountB += 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu057", func_00A97330);
