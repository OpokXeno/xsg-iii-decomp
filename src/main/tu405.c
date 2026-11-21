#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu405", func_002E6580);

typedef int s32;
typedef signed char s8;
s32 func_002BEB80();
s32 func_002BEF50();
s32 drawSetFlagA(s32);
s32 drawSetFlagB(s32);

void entryPrepareStart(s32 entryUnitId, s32 entryDrawFlagB, s32 entryDrawFlagA) {
    func_002BEB80();
    drawSetFlagA(entryDrawFlagA);
    drawSetFlagB(entryDrawFlagB);
    func_002BEF50(entryUnitId);
}

void sceneEntryStopCurrent(void) {
    func_002BEB80();
    func_002BEF50(0);
}

s32 func_002C45E8();

void entrySyncStep(void) {
    func_002C45E8();
}

s32 seVoiceSlotsReset();

void entryResetSeVoiceSlots(void) {
    seVoiceSlotsReset();
}

s32 sceneLinkStateSetEnabled();
s32 sceneLinkSetFlag44(s8);

void entrySetCodeAndFlush(s8 sceneLinkCode) {
    sceneLinkSetFlag44(sceneLinkCode);
    sceneLinkStateSetEnabled();
}

s32 sceneLinkStateClearCounters();

void entryCommitStep(void) {
    sceneLinkStateClearCounters();
}

INCLUDE_ASM("asm/main/nonmatchings/tu405", func_002E6880);
