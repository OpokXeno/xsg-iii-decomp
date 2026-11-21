#include "common.h"
#include "shared.h"

void dataVPadModeSet(int mode);
void dataVPadSet(int virtualPad);

extern int padMode;
extern short padData;

#define VIRTUAL_PAD_BIT_COUNT 14

extern unsigned short padConvTbl[];

#define SAVE_STACK_LEVEL_CELLS 5  /* cells per SaveStack level */
#define SAVE_STACK_LEVELS 4       /* levels of a SaveStack (a push to this depth is refused) */

struct SaveStack;

#include "ov02/tu030.h"


/* battleStackSetSlot: writes value to slot of the current level; returns the cell's byte offset. */
int battleStackSetSlot(struct SaveStack *stack, int cellInLevel, int cellValue) {
    int cellIndex;
    int *cell;

    cellIndex = stack->count * SAVE_STACK_LEVEL_CELLS + cellInLevel;
    cell = &stack->cells[cellIndex];
    *cell = cellValue;
    return cellIndex * 4;
}

/* battleStackGetSlot: reads slot of the current level. */
int battleStackGetSlot(struct SaveStack *stack, int cellInLevel)
{
    int *cell = &stack->cells[stack->count * SAVE_STACK_LEVEL_CELLS + cellInLevel];
    return *cell;
}

struct BattleUnit;
extern struct BattleUnit *gBattleUnitTable[];
extern int gBattleUnitGrid[];

/* Zeroes the 32-word table at gBattleUnitTable (walked from its last word down), the two rows of
 * two words at gBattleUnitGrid (each row written from its second word down), and the words
 * gBattleUnitCount and gBattleCounter. */
void battleTablesReset(void)
{
    extern int gBattleUnitCount;
    extern int gBattleCounter;
    int rowStep;
    int column;
    int index;
    struct BattleUnit **cell;
    int *rowCell;
    int *row;

    cell = gBattleUnitTable;
    cell += 31;
    for (index = 0; index < 32; index++) {
        *cell = 0;
        cell -= 1;
    }

    index = 0;
    row = gBattleUnitGrid;
    do {
        do {
            rowStep = 1;
            column = rowStep;
            rowCell = row + 1;
        loop_column:
            column -= 1;
            *rowCell = 0;
            rowCell -= 1;
            if (column >= 0) {
                goto loop_column;
            }
            index += rowStep;
            row += 2;
        } while (0);
    } while (index < 2);
    gBattleUnitCount = 0;
    gBattleCounter = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BB60);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BBB0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BC80);

void battleEmptyHook(void);
void func_00A8BDF8(int);

void battleCountersReset(void) {
    extern int gBattleUnitCount;
    extern int gBattleCounter;
    extern void battleEmptyHook();
    gBattleUnitCount = 0;
    gBattleCounter = 0;
    func_00A8BDF8(2);
    battleEmptyHook();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BDF8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8BF90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C058);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C1D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C270);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C2C8);

int boostGaugeAddStockCurrent(void *, int);

/* Stock-count block of a battle unit (reached through BattleUnit.stockCounters at 0x2B0). */
struct BattleUnitStats {
    u8 unk_00[0x3E];
    s16 stockCurrent;
};

struct BattleUnit {
    u8 unk_000[0x2B0];
    struct BattleUnitStats *stockCounters;
};

/* Stores record into the 2x2 table at gBattleUnitGrid, in the column given by index and the row
 * chosen from the 16-bit count at record->0x2B0->0x3E; returns that cell's byte offset. */
int battleUnitTableStore(struct BattleUnit *battleUnit, int column)
{
    int cellIndex;
    int overLimit;

    if (column == 1) {
        boostGaugeAddStockCurrent(battleUnit, -1);
    }
    overLimit = battleUnit->stockCounters->stockCurrent <= (0x12 - 1);
    overLimit = overLimit == 0;
    cellIndex = overLimit * 2 + column;
    ((struct BattleUnit **) gBattleUnitGrid)[cellIndex] = battleUnit;
    return cellIndex * 4;
}

/* Battle unit pointer at index of the table at gBattleUnitTable (data_vpad.c reads the units with it). */
struct BattleUnit *battleUnitGet(int unitNumber)
{
    return gBattleUnitTable[unitNumber];
}

/* Word at row, column of the 2x2 table at gBattleUnitGrid. */
int battleTableCellGet(int row, int column)
{
    return gBattleUnitGrid[row * 2 + column];
}

int battleCounterGetRemaining(void)
{
    extern int gBattleCounter;
    return gBattleCounter - (gBattleCounter > 0);
}

/* Unit count: the word gBattleUnitCount (data_vpad.c loops over it). */
int battleUnitCount(void)
{
    extern int gBattleUnitCount;
    return gBattleUnitCount;
}

void battleEmptyHook(void) {

}

void battleVPadReset(void) {
    dataVPadModeSet(0);
}

void dataVPadModeSet(int newPadMode)
{
    padMode = newPadMode;
    padData = 0;
}

void dataVPadSet(int virtualPad)
{
    unsigned short convertedPad = 0;
    int shift = 0;
    const unsigned short *convEntry = padConvTbl;
    virtualPad &= 0xFFFF;
    /* Original entry at 0x00A1D2A0 enters the body before testing the
     * loop count; keep the table pointer advance and 16-bit mask. */
    do {
        if ((virtualPad >> shift) & 1) {
            convertedPad |= *convEntry;
        }
        shift++;
        convEntry++;
    } while (shift < VIRTUAL_PAD_BIT_COUNT);
    padData = convertedPad;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C4F8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu030", func_00A8C570);
