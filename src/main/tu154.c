#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef short s16;
extern s32 gCdPrimaryCommandSent;
extern s32 gCdSecondaryCommandSent;
void xcBlockClear(void *block);
void func_001B7190();
void xglCdWaitControl();
extern unsigned char func_0020FD50[];

struct CdCommandRecord {
    /* 0x00 */ s32 commandStatus;
    /* 0x04 */ s32 command;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ u8 *commandData;
    /* 0x10 */ s32 commandMode;
};

void xglCdSetupControlRecords(void) {
    struct CdCommandRecord primaryRecord;
    struct CdCommandRecord secondaryRecord;

    if (gCdPrimaryCommandSent == 0) {
        xcBlockClear(&primaryRecord.commandStatus);
        primaryRecord.command = 0x04000001;
        primaryRecord.commandData = func_0020FD50;
        primaryRecord.commandStatus = 0;
        primaryRecord.unk08 = 0;
        primaryRecord.commandMode = 0;
        func_001B7190(&primaryRecord.commandStatus);
    }
    if (gCdSecondaryCommandSent == 0) {
        xcBlockClear(&secondaryRecord.commandStatus);
        secondaryRecord.command = 0x04000002;
        secondaryRecord.commandData = func_0020FD50;
        secondaryRecord.commandMode = 1;
        secondaryRecord.commandStatus = 0;
        secondaryRecord.unk08 = 0;
        func_001B7190(&secondaryRecord.commandStatus);
    }
    xglCdWaitControl();
}

extern s16 D_00475628[];
s32 eBattleWinInit();
s32 func_001E9300();

void eBattleWinReset(void) {
    if (D_00475628[0] != 0) {
        func_001E9300(-1);
    }
    eBattleWinInit(0);
    gCdPrimaryCommandSent = 0;
    gCdSecondaryCommandSent = 0;
}

void xglCdEmptyCallback(void) {

}
