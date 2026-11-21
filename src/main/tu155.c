#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;

typedef struct StateBudget {
    u8 unk00[8];
    u32 remainingAmount; /* 0x08 */
} StateBudget;

typedef struct StateBlock {
    /* 0x00 */ StateBudget *budget;
    /* 0x04 */ u8 unk04[0x10B];
    /* 0x10F */ u8 flag;
    /* 0x110 */ u8 unk110[0x10];
    /* 0x120 */ u8 unk120[0x350];
} StateBlock;
s32 func_00211C38(u8);
void *memPoolAlloc(s32 size);
s32 bufferPointersRewind();
s32 func_00213C18();
s32 func_002169A0(void *);
s32 mcModuleReset();
s32 memset(void *dest, s32 value, s32 size);

/* Allocates and zeroes the 0x470-byte state block D_00550C38, sets its flag byte at 0x10F from bit 0 of
 * the argument and returns 1. Destroyed by destroyStateBlock. */
s32 createStateBlock(s32 blockFlagBits) {
    extern StateBlock *D_00550C38;
    extern s32 func_00224600(s32);
    void *stateBlockMemory;

    func_00224600(0);
    func_00213C18();
    bufferPointersRewind();
    stateBlockMemory = memPoolAlloc(0x470);
    D_00550C38 = stateBlockMemory;
    memset(stateBlockMemory, 0, 0x470);
    func_002169A0(D_00550C38->unk120);
    mcModuleReset();
    if (blockFlagBits & 1) {
        D_00550C38->flag = 1U;
    } else {
        D_00550C38->flag = 0U;
    }
    func_00211C38(D_00550C38->flag);
    return 1;
}

s32 memPoolFree(void *block);
s32 xcBlockFlagsReset(void *block);

/* Releases the state block created by createStateBlock. */
void destroyStateBlock(void) {
    extern StateBlock *D_00550C38;
    xcBlockFlagsReset(D_00550C38->unk120);
    memPoolFree(D_00550C38);
    D_00550C38 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00210A98);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_002110C0);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211998);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211B90);

/* Takes amount from the counter at +8 of the block that D_00550C38 points to (clamped at zero) and passes the
 * amount taken to func_00224600. */
void stateBlockConsume(u32 consumeAmount) {
    extern StateBlock *D_00550C38;
    extern s32 func_00224600();
    u32 remainingBefore;
    u32 remainingAfter;
    u32 takenAmount;
    StateBudget *stateBudget;

    takenAmount = consumeAmount;
    stateBudget = D_00550C38->budget;
    remainingBefore = stateBudget->remainingAmount;
    remainingAfter = remainingBefore - takenAmount;
    if (remainingBefore >= takenAmount) {
        stateBudget->remainingAmount = remainingAfter;
    } else {
        stateBudget->remainingAmount = 0U;
        takenAmount = remainingBefore;
    }
    func_00224600(takenAmount);
}

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211C38);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211D58);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211DC0);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211EF0);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211F48);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00211FE8);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00212100);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00212170);

s32 runAfterCompletionIdle(s32 value);
extern s32 D_004CABC4[];
extern s8 gVsyncsPerFrame;
s32 func_00213490();
s32 memPoolReset();

/* Runs runAfterCompletionIdle once (guarded by gVsyncsPerFrame and the mode word at D_004CABC4 != 2). */
s32 runWithStateBlockOnce(s32 runArgument) {
    extern s32 runAfterCompletionIdle();
    s32 runOutcome;
    s32 blockRunOutcome;
    s8 savedRunGuard;

    runOutcome = 0;
    savedRunGuard = gVsyncsPerFrame;
    if (D_004CABC4[0] != 2) {
        gVsyncsPerFrame = 1;
        func_00213490(0x980000, 0x10000);
        blockRunOutcome = runAfterCompletionIdle(runArgument);
        memPoolReset();
        gVsyncsPerFrame = savedRunGuard;
        runOutcome = blockRunOutcome;
    }
    return runOutcome;
}

s32 createStateBlock(s32 flags);
void destroyStateBlock(void);
s32 func_00212F38(s32);
s32 completionFlagsAny();
s32 func_00213C50();

/* Creates the state block, runs func_00212F38 on it once the queue of completionFlagsAny is empty, then releases the
 * block. Returns 0 when the block could not be created. */
s32 runAfterCompletionIdle(s32 runArgument) {
    extern s32 createStateBlock();
    s32 runOutcome;
    s32 blockOutcome;

    blockOutcome = createStateBlock();
    if (blockOutcome != 0) {
        while (completionFlagsAny() != 0) {
            func_00213C50();
        }
        runOutcome = func_00212F38(runArgument);
        destroyStateBlock();
        blockOutcome = runOutcome;
    }
    return blockOutcome;
}

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00212C50);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00212E20);

INCLUDE_ASM("asm/main/nonmatchings/tu155", func_00212F38);
