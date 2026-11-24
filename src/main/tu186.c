#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu186", func_00237188);

typedef int s32;
typedef unsigned char u8;
typedef signed char s8;
s32 func_001B9228();
s32 func_00237188();

s32 vmRunNativeWithPluginFlush(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    s32 nativeResult;

    nativeResult = func_00237188();
    if (VM_PLUGIN_CNT_GET(vmThread) != 0) {
        func_001B9228();
    }
    return nativeResult;
}

INCLUDE_ASM("asm/main/nonmatchings/tu186", func_00237248);

s32 func_00237248();

s32 vmRunSecondNativeWithPluginFlush(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    s32 secondNativeResult;

    secondNativeResult = func_00237248();
    if (VM_PLUGIN_CNT_GET(vmThread) != 0) {
        func_001B9228();
    }
    return secondNativeResult;
}

void func_00222048(s32);
void battleRosterPairReset();

s32 vmForEachArgCall(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET();
    extern s32 vmArgGet(s32, s32);
    s32 vmArgCount;
    s32 vmArgIndex;
    s32 vmArgValue;

    vmArgCount = VM_PLUGIN_CNT_GET();
    if (vmArgCount != 0) {
        vmArgIndex = 0;
        if (vmArgCount > 0) {
            do {
                vmArgIndex += 1;
                vmArgValue = vmArgGet(vmThread, vmArgIndex);
                func_00222048(vmArgValue);
            } while (vmArgIndex < vmArgCount);
        }
    } else {
        battleRosterPairReset();
    }
    return 1;
}

s32 vmForEachArgCall(s32 context);

s32 vmForEachArgCallWithFlush(void) {
    extern s32 battleCurrentRosterEntryCountGet();
    extern s32 vmForEachArgCall();
    s32 vmForEachResult;

    vmForEachResult = vmForEachArgCall();
    if (battleCurrentRosterEntryCountGet() > 0) {
        func_001B9228();
    }
    return vmForEachResult;
}

INCLUDE_ASM("asm/main/nonmatchings/tu186", func_00237428);

s32 func_00237428();

s32 vmRunNativeFlushOnResult(void) {
    s32 nativeResult;

    nativeResult = func_00237428();
    if (nativeResult != 0) {
        func_001B9228();
    }
    return nativeResult;
}

s32 func_00221FA0();

s32 vmSetFlagFromArgOrDefault(s32 vmThread) {
    extern u8 gBattleCurrentRosterIndex[];
    extern s32 VM_PLUGIN_CNT_GET();
    extern s32 vmArgGet(s32, s32);
    s32 vmArgValue;

    if (VM_PLUGIN_CNT_GET() > 0) {
        vmArgValue = vmArgGet(vmThread, 1);
        func_00221FA0(vmArgValue > 0);
    } else {
        func_00221FA0(gBattleCurrentRosterIndex[0] == 0, 1);
    }
    return 1;
}

s32 vmSetFlagFromArgOrDefault(s32 context);

s32 vmSetFlagFromArgWithFlush(void) {
    extern s32 vmSetFlagFromArgOrDefault();
    s32 vmSetFlagResult;

    vmSetFlagResult = vmSetFlagFromArgOrDefault();
    func_001B9228();
    return vmSetFlagResult;
}

s32 vmIsFlagSet(void) {
    extern u8 gBattleCurrentRosterIndex[];
    return gBattleCurrentRosterIndex[0] != 0;
}

s32 func_00222120(s32, s32, s32);
s32 func_00222238(s32, s32);
s32 func_00222360(s32);

s32 vmApplyIndexedKindArg(s32 vmThread)
{
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    s32 vmTargetIndex;
    s32 vmTargetKind;

    if (VM_PLUGIN_CNT_GET(vmThread) > 0) {
        vmTargetIndex = vmArgGet(vmThread, 1);
        vmTargetKind = func_00222360(vmTargetIndex);
        if (vmTargetKind >= 3) {
            func_00222120(0, vmTargetKind, vmTargetIndex >= 0xD);
            vmTargetKind = 0;
        }
        func_00222238(vmTargetKind, (vmTargetIndex < 0xD) ^ 1);
        return 1;
    }
    return 0;
}

s32 vmApplyIndexedKindArg(s32 context);

s32 vmApplyIndexedKindWithFlush(void) {
    extern s32 vmApplyIndexedKindArg();
    s32 vmApplyKindResult;

    vmApplyKindResult = vmApplyIndexedKindArg();
    if (vmApplyKindResult != 0) {
        func_001B9228();
    }
    return vmApplyKindResult;
}

s32 battleRosterCursorEntryIdGet(s32);
s32 battleCurrentRosterCursorEntryIdGet(s32, s32);

void vmSetModeFromArgOrDefault(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET();
    extern s32 vmArgGet(s32, s32);
    s32 vmArgValue;

    if (VM_PLUGIN_CNT_GET() > 0) {
        vmArgValue = vmArgGet(vmThread, 1);
        battleRosterCursorEntryIdGet(vmArgValue > 0);
        return;
    }
    battleCurrentRosterCursorEntryIdGet(vmThread, 1);
}

void vmSetTableFromArgOrDefault(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET();
    extern s32 battleRosterEntryIdCountGet(s32);
    extern s32 battleCurrentRosterEntryCountGet(s32, s32);
    extern s32 vmArgGet(s32, s32);
    s32 vmArgValue;

    if (VM_PLUGIN_CNT_GET() > 0) {
        vmArgValue = vmArgGet(vmThread, 1);
        battleRosterEntryIdCountGet(vmArgValue > 0);
        return;
    }
    battleCurrentRosterEntryCountGet(vmThread, 1);
}

s32 battleRosterEntryIdGet();

s32 vmGetRecordSubEntryFromArgs(s32 vmThread)
{
    extern unsigned char gBattleCurrentRosterIndex[];
    extern s32 VM_PLUGIN_CNT_GET();
    extern s32 battleRosterEntryIdCountGet();
    extern s32 vmArgGet();
    s32 vmRecordIndex;
    s32 vmSelectFlag;

    vmRecordIndex = vmArgGet(vmThread, 1);
    vmSelectFlag = gBattleCurrentRosterIndex[0];
    if (VM_PLUGIN_CNT_GET(vmThread) >= 2) {
        vmSelectFlag = vmArgGet(vmThread, 2) > 0;
    }
    if (vmRecordIndex < battleRosterEntryIdCountGet(vmSelectFlag)) {
        return battleRosterEntryIdGet(vmRecordIndex, vmSelectFlag);
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu186", func_00237808);

s32 battleRosterHeaderPairSet();

s32 vmInvokePairFromArgs(s32 vmThread) {
    extern s32 vmArgGet();
    s32 vmFirstHeaderValue;
    s32 vmSecondHeaderValue;

    vmFirstHeaderValue = vmArgGet(vmThread, 1);
    vmSecondHeaderValue = vmArgGet(vmThread, 2);
    battleRosterHeaderPairSet(vmFirstHeaderValue, vmSecondHeaderValue);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu186", func_002378C0);

INCLUDE_ASM("asm/main/nonmatchings/tu186", func_00237930);

s32 seVoiceStartGuarded(s32);
s32 func_0025F5C0(s32);

s32 vmStartSeVoiceFromArgGuarded(s32 vmThread) {
    extern s32 vmArgGet(s32, s32);
    s32 seVoiceResult;
    s32 seVoiceId;

    func_0025F5C0(1);
    seVoiceId = vmArgGet(vmThread, 1);
    seVoiceResult = seVoiceStartGuarded(seVoiceId);
    func_0025F5C0(0);
    return seVoiceResult;
}
