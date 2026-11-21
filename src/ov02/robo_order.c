/*
 * OV02 original TU 70: 0x00aa1908..0x00aa2268 (17 functions)
 */
#include "common.h"

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;

extern s32 gRoboOrderCurrentUnit;

/* One entry of the robo order table: a key and an active flag. */
typedef struct RoboOrderEntry {
    s32 key;                        /* 0x00 */
    s32 active;                     /* 0x04: cleared by roboOrderClearKey */
} RoboOrderEntry;

/* The robo order table at gRoboOrderTable, 0x78 bytes. */
typedef struct RoboOrderTable {
    RoboOrderEntry orderEntries[13];     /* 0x00..0x67 */
    s32 orderEntryCount;                 /* 0x68 */
    u8 unk_6C[4];
    s32 orderCursor;                     /* 0x70: -1 after reset */
    u8 unk_74[4];
} RoboOrderTable;

extern RoboOrderTable gRoboOrderTable;

s32 roboOrderGetCurrentUnit(void) {
    return gRoboOrderCurrentUnit;
}

void roboOrderSetCurrentUnit(s32 unitId) {
    gRoboOrderCurrentUnit = unitId;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1928);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1988);

void *memset(void *, s32, s32);

void roboOrderReset(void) {
    memset(&gRoboOrderTable, 0, 0x78);
    gRoboOrderTable.orderCursor = -1;
}

s32 roboOrderCursorOutOfRange(void)
{
    s32 cursorOutOfRange;

    cursorOutOfRange = 0;
    if ((gRoboOrderTable.orderCursor < 0) || (gRoboOrderTable.orderCursor >= gRoboOrderTable.orderEntryCount))
    {
        cursorOutOfRange = 1;
    }
    return cursorOutOfRange;
}

extern s32 D_00AC7E60;

s32 roboOrderGetCounter(void) {
    return D_00AC7E60;
}

s32 roboOrderClearKey(s32 key) {
    RoboOrderEntry *orderEntry;
    s32 lastKey;
    s32 remaining;

    lastKey = (s32) &gRoboOrderTable;
    remaining = gRoboOrderTable.orderEntryCount;
    if (remaining > 0) {
        orderEntry = gRoboOrderTable.orderEntries;
        do {
            lastKey = orderEntry->key;
            if (lastKey == key) {
                orderEntry->active = 0;
            }
            remaining -= 1;
            orderEntry += 1;
        } while (remaining != 0);
    }
    return lastKey;
}

s32 func_00AA1DE0(s32);

void roboOrderAdvanceDefault(void) {
    func_00AA1DE0(0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1B58);

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1C00);

s32 roboOrderEntryKey(s32 orderEntryIndex) {
    return gRoboOrderTable.orderEntries[orderEntryIndex].key;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1D70);

extern s32 D_00AC7E68;

s32 roboOrderGetFlag(void) {
    return D_00AC7E68;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1DE0);

extern s32 gRoboOrderCallerFile;
#include "ov02/robo_battle.h"
s32 func_00AA1300(s32);
s32 roboParamRollScore(s32);

s32 roboOrderCompare(s32 *firstUnitId, s32 *secondUnitId) {
    s32 firstKey;
    s32 secondKey;
    s32 firstRank;
    s32 secondRank;

    firstKey = roboBattleUnitGet(*firstUnitId, &gRoboOrderCallerFile, 0xFB);
    secondKey = roboBattleUnitGet(*secondUnitId, &gRoboOrderCallerFile, 0xFC);
    firstRank = roboParamRollScore(firstKey);
    secondRank = roboParamRollScore(secondKey);
    if (firstRank == secondRank) {
        firstRank = func_00AA1300(firstKey);
        secondRank = func_00AA1300(secondKey);
    }
    return firstRank - secondRank;
}

INCLUDE_ASM("asm/nonmatchings/ov02/robo_order", func_00AA1FA0);
