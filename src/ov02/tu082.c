/*
 * OV02 original TU 82: 0x00aaca68..0x00aace98 (12 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACA68);

typedef int s32;
typedef unsigned char u8;

/* The command block of a unit: the fields at +0x2CC..+0x2D8 this TU reads and writes. */
typedef struct UnitCommandBlock {
    u8 unk_00[0x2CC];
    s32 linkedIndex;                    /* +0x2CC */
    s32 commandIndex;                   /* +0x2D0 */
    s32 commandArgA;                    /* +0x2D4 */
    s32 commandArgB;                    /* +0x2D8 */
} UnitCommandBlock;

s32 roboCurrentIsPlayer(void);
s32 func_00AA6F08(s32 battleUnitHandle);

s32 unitCommandSlotActive(s32 battleUnitHandle) {
    s32 isActive;

    isActive = 0;
    if (roboCurrentIsPlayer() != 0) {
        isActive = func_00AA6F08(battleUnitHandle) != 0;
    }
    return isActive;
}

/* Four s32 slots: the three command slots (0, 2, 3) are reset to -1; slot 1 is the current index. */
extern s32 gUnitCommandSlots[4];
void *memset(void *dest, s32 value, s32 size);

void unitCommandSlotsReset(void) {
    memset(&gUnitCommandSlots, 0, 0x10);
    gUnitCommandSlots[0] = -1;
    gUnitCommandSlots[3] = -1;
    gUnitCommandSlots[2] = -1;
}

void unitSetLinkedIndex(UnitCommandBlock *commandBlock, s32 linkedIndex) {
    commandBlock->linkedIndex = linkedIndex;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACB10);

s32 unitCommandSlotActive(s32 battleUnitHandle);

void unitSetCommandIndex(UnitCommandBlock *commandBlock, s32 commandIndex)
{
    extern s32 unitCommandSlotActive();
    s32 *commandSlots;

    commandSlots = gUnitCommandSlots;
    commandBlock->commandIndex = commandIndex;
    if (unitCommandSlotActive())
    {
        commandSlots[1] = commandIndex;
    }
}

u8 *ov02BattleSlotGet(s32 commandIndex);

void unitSetCommandTarget(UnitCommandBlock *commandBlock, s32 commandIndex, s32 commandArg)
{
    extern s32 unitCommandSlotActive();
    s32 *commandSlots;
    u8 *battleStatEntry;

    commandSlots = gUnitCommandSlots;
    battleStatEntry = ov02BattleSlotGet(commandIndex);
    commandBlock->commandIndex = commandIndex;
    if (battleStatEntry[6] & 2)
    {
        commandBlock->commandArgA = commandArg;
        if (unitCommandSlotActive(commandBlock) != 0)
        {
            commandSlots[2] = commandArg;
            return;
        }
        return;
    }
    commandBlock->commandArgB = commandArg;
    if (unitCommandSlotActive(commandBlock) != 0)
    {
        commandSlots[3] = commandArg;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACC20);

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACC68);

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACCA8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACCE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu082", func_00AACDA8);
