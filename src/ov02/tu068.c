/*
 * OV02 original TU 68: 0x00a9ff28..0x00aa00f8 (3 functions)
 */
#include "common.h"

typedef int s32;
s32 func_001B60B8();
void xcBlockClear(void *block);
void func_001B70F0();
s32 sbBaseBlockIdGet();
void xglSleep();
/* Four-word command record filled in place and handed to xcBlockClear/func_001B70F0. */
typedef struct Ov02CommandRecord {
    s32 commandHandle;    /* 0x00: written by xcBlockClear, cleared before the send */
    s32 command;          /* 0x04: command word, 0x06020001 */
    s32 soundTableBase;   /* 0x08: result of sbBaseBlockIdGet */
    s32 reserved;         /* 0x0C: always zero */
} Ov02CommandRecord;

void ov02SendCommandAndWait(void) {
    Ov02CommandRecord commandRecord;
    /* The volatile pad keeps the frame layout of the original; it is not data. */
    volatile unsigned int stackPad;

    xcBlockClear(&commandRecord.commandHandle);
    commandRecord.commandHandle = 0;
    commandRecord.command = 0x06020001;
    commandRecord.reserved = 0;
    commandRecord.soundTableBase = sbBaseBlockIdGet();
    func_001B70F0(&commandRecord.commandHandle);
    while (func_001B60B8(0) != 0) {
        xglSleep();
    }
}

extern unsigned char gRoboBattleTable[];
void battleResultReset();
void gifStateResetTimings();
void advanceInit();
void func_00A81558();
void ov02RecordQueueClear();
void ov02RecordNoOp();
void sceneStateReset();
void monsReadyCountReset();
void func_00A88B50();
void targetingSlotTablesReset();
void battleTablesReset();
void battleVPadReset();
void sbScriptImageInit();
void monsCommandQueuesInit();
void func_00A92CB0();
void soundToggleReset();
void battleEntryClearPools();
void func_00A9D8E0();
void roboUnitTableSerialBump();
void roboOrderReset();
void roboAttackSeqClear();
void func_00AAB6C8();
void unitCommandSlotsReset();
void battleSourceTableInit();
void seInitFlagClearThunk();
void roboBattleFlowInit();
void *memset(void *dest, s32 value, unsigned int count);
void battleResetCallbacksAndTables();
void unitInit();

void ov02BattleStateReset(void) {
    memset(gRoboBattleTable, 0, 0x2860);
    func_00A9D8E0();
    battleResetCallbacksAndTables();
    monsReadyCountReset();
    battleVPadReset();
    func_00A92CB0();
    soundToggleReset();
    ov02RecordNoOp();
    func_00A81558();
    battleEntryClearPools();
    func_00A88B50();
    unitInit();
    battleTablesReset();
    sbScriptImageInit();
    advanceInit();
    monsCommandQueuesInit();
    ov02RecordQueueClear();
    targetingSlotTablesReset();
    sceneStateReset();
    seInitFlagClearThunk();
    roboBattleFlowInit();
    battleResultReset();
    roboAttackSeqClear();
    func_00AAB6C8();
    roboOrderReset();
    unitCommandSlotsReset();
    battleSourceTableInit();
    gifStateResetTimings();
    roboUnitTableSerialBump();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu068", func_00AA0088);
