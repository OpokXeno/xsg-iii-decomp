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
    int *slotCell;

    cellIndex = stack->count * SAVE_STACK_LEVEL_CELLS + cellInLevel;
    slotCell = &stack->cells[cellIndex];
    *slotCell = cellValue;
    return cellIndex * 4;
}

/* battleStackGetSlot: reads slot of the current level. */
int battleStackGetSlot(struct SaveStack *stack, int cellInLevel)
{
    int *slotCell = &stack->cells[stack->count * SAVE_STACK_LEVEL_CELLS + cellInLevel];
    return *slotCell;
}

struct BattleUnit;
extern struct BattleUnit *gBattleUnitTable[];
extern struct BattleUnit *gBattleUnitGrid[];

/* Zeroes the 32-word table at gBattleUnitTable (walked from its last word down), the two rows of
 * two words at gBattleUnitGrid (each row written from its second word down), and the words
 * gBattleUnitCount and gBattleCounter. */
void battleTablesReset(void)
{
    extern int gBattleUnitCount;
    extern int gBattleCounter;
    int rowsPerPass;
    int columnsLeft;
    int clearedCount;
    struct BattleUnit **tableCell;
    struct BattleUnit **rowCellCursor;
    struct BattleUnit **rowCursor;

    tableCell = gBattleUnitTable;
    tableCell += 31;
    for (clearedCount = 0; clearedCount < 32; clearedCount++) {
        *tableCell = 0;
        tableCell -= 1;
    }

    clearedCount = 0;
    rowCursor = gBattleUnitGrid;
    do {
        rowsPerPass = 1;
        columnsLeft = rowsPerPass;
        rowCellCursor = rowCursor + 1;
    loop_column:
        columnsLeft -= 1;
        *rowCellCursor = 0;
        rowCellCursor -= 1;
        if (columnsLeft >= 0) {
            goto loop_column;
        }
        clearedCount += rowsPerPass;
        rowCursor += 2;
    } while (clearedCount < 2);
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
    gBattleUnitGrid[cellIndex] = battleUnit;
    return cellIndex * 4;
}

/* Battle unit pointer at index of the table at gBattleUnitTable (data_vpad.c reads the units with it). */
struct BattleUnit *battleUnitGet(int unitNumber)
{
    return gBattleUnitTable[unitNumber];
}

/* Battle unit pointer at row, column of the 2x2 table at gBattleUnitGrid. */
struct BattleUnit *battleTableCellGet(int row, int column)
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

#include "main/tu007.h"

struct PadRecordView {
    unsigned char unmodeled_00[0x30];
    unsigned short directionHeld;
    unsigned short directionPressed;
    unsigned short directionRepeated;
    unsigned char unmodeled_36[0x32];
    unsigned short tailHalfword[3];
};

extern struct PadRecordView gPadPortRecords[2];

/* Reads the pad word of the current mode: while padMode is zero, the directionHeld, directionPressed or
 * directionRepeated halfword of port 0 (ports 0 to 2); otherwise the padData word. */
unsigned short dataVPadReadButtons(int port)
{
    unsigned short padButtons;

    padButtons = 0;
    if (padMode == 0) {
        switch (port) {
        case 0:
            padButtons = gPadPortRecords[0].directionHeld;
            break;
        case 1:
            padButtons = gPadPortRecords[0].directionPressed;
            break;
        case 2:
            padButtons = gPadPortRecords[0].directionRepeated;
            break;
        }
    } else {
        padButtons = padData;
    }
    return padButtons;
}

/* Port record of the pad table as this TU reads it: the direction halfwords at +0x30 and the three
 * halfwords at +0x68 that PadRecord leaves unmodeled. */

/* Reads the same pad word as dataVPadReadButtons for the three halfwords at +0x68 of the port 0 record. */
unsigned short dataVPadReadTailWord(int port)
{
    unsigned short padButtons;

    padButtons = 0;
    if (padMode == 0) {
        switch (port) {
        case 0:
            padButtons = gPadPortRecords[0].tailHalfword[0];
            break;
        case 1:
            padButtons = gPadPortRecords[0].tailHalfword[1];
            break;
        case 2:
            padButtons = gPadPortRecords[0].tailHalfword[2];
            break;
        }
    } else {
        padButtons = padData;
    }
    return padButtons;
}
