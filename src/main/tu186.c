#include "common.h"
#include "main/tu176.h"

typedef int s32;

/* VM native: calls func_00221FB8 with each script argument, from the first to the last. Returns 1. */
s32 vmCallEachArgument(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    extern void func_00221FB8(s32);
    s32 vmArgCount;
    s32 vmArgIndex;

    vmArgIndex = 0;
    vmArgCount = VM_PLUGIN_CNT_GET(vmThread);
    if (vmArgCount > 0) {
        do {
            vmArgIndex += 1;
            func_00221FB8(vmArgGet(vmThread, vmArgIndex));
        } while (vmArgIndex < vmArgCount);
    }
    return 1;
}

typedef unsigned char u8;

s32 func_001B9228();

s32 vmRunNativeWithPluginFlush(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    s32 nativeResult;

    nativeResult = vmCallEachArgument(vmThread);
    if (VM_PLUGIN_CNT_GET(vmThread) != 0) {
        func_001B9228();
    }
    return nativeResult;
}

/* VM native with two arguments (entry id, index): adds the entry id to its roster (func_00221FB8), then swaps it
   one slot forward until it sits at the given index. */
s32 vmRaiseEntryKindTo(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    extern void func_00221FB8(s32);
    s32 vmArgCount;
    s32 vmEntryId;
    s32 vmTargetIndex;
    s32 vmEntryIndex;
    s32 vmRosterIndex;

    vmArgCount = VM_PLUGIN_CNT_GET(vmThread);
    if (vmArgCount != 2) {
        return 0;
    }
    vmEntryId = vmArgGet(vmThread, 1);
    vmTargetIndex = vmArgGet(vmThread, 2);
    vmRosterIndex = vmEntryId >= 13;
    func_00221FB8(vmEntryId);
    vmEntryIndex = battleRosterEntryIdIndexGet(vmEntryId);
    if (vmEntryIndex != -1 && vmTargetIndex >= 0 && vmTargetIndex < vmEntryIndex) {
        do {
            battleRosterEntrySwap(vmEntryIndex, vmEntryIndex - 1, vmRosterIndex);
            vmEntryIndex = battleRosterEntryIdIndexGet(vmEntryId);
        } while (vmTargetIndex < vmEntryIndex);
    }
    return 1;
}

s32 vmRunSecondNativeWithPluginFlush(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    s32 secondNativeResult;

    secondNativeResult = vmRaiseEntryKindTo(vmThread);
    if (VM_PLUGIN_CNT_GET(vmThread) != 0) {
        func_001B9228();
    }
    return secondNativeResult;
}

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
                battleRosterEntryIdRemove(vmArgValue);
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

void battleUpdateEntryKinds(s32, s32, s32);

/* VM native with three arguments: all nonzero arguments must share one kind class (value >= 13), then battleUpdateEntryKinds runs. */
s32 vmUpdateEntryKinds(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    s32 vmArgCount;
    s32 vmArgIndex;
    s32 vmArgValue;
    s32 vmHaveKindClass;
    s32 vmKindClass;
    s32 vmFirstArg;
    s32 vmSecondArg;
    s32 vmThirdArg;

    vmArgCount = VM_PLUGIN_CNT_GET(vmThread);
    if (vmArgCount != 3) {
        return 0;
    }
    vmHaveKindClass = 0;
    vmKindClass = -1;
    vmArgIndex = 0;
    do {
        vmArgIndex += 1;
        vmArgValue = vmArgGet(vmThread, vmArgIndex);
        if (vmHaveKindClass != 0) {
            if (vmArgValue == 0) {
                continue;
            }
            if (vmKindClass != (vmArgValue >= 13)) {
                return 0;
            }
            continue;
        }
        if (vmArgValue == 0) {
            continue;
        }
        vmHaveKindClass = 1;
        vmKindClass = vmArgValue >= 13;
    } while (vmArgIndex < 3);
    vmFirstArg = vmArgGet(vmThread, 1);
    vmSecondArg = vmArgGet(vmThread, 2);
    vmThirdArg = vmArgGet(vmThread, 3);
    battleUpdateEntryKinds(vmFirstArg, vmSecondArg, vmThirdArg);
    return 1;
}

s32 vmRunNativeFlushOnResult(s32 vmThread) {
    s32 nativeResult;

    nativeResult = vmUpdateEntryKinds(vmThread);
    if (nativeResult != 0) {
        func_001B9228();
    }
    return nativeResult;
}

s32 vmSetFlagFromArgOrDefault(s32 vmThread) {
    extern u8 gBattleCurrentRosterIndex[];
    extern s32 VM_PLUGIN_CNT_GET();
    extern s32 vmArgGet(s32, s32);
    s32 vmArgValue;

    if (VM_PLUGIN_CNT_GET() > 0) {
        vmArgValue = vmArgGet(vmThread, 1);
        battleCurrentRosterIndexSet(vmArgValue > 0);
    } else {
        battleCurrentRosterIndexSet(gBattleCurrentRosterIndex[0] == 0);
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

s32 vmApplyIndexedKindArg(s32 vmThread)
{
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    s32 vmEntryId;
    s32 vmEntryIndex;

    if (VM_PLUGIN_CNT_GET(vmThread) > 0) {
        vmEntryId = vmArgGet(vmThread, 1);
        vmEntryIndex = battleRosterEntryIdIndexGet(vmEntryId);
        if (vmEntryIndex >= 3) {
            battleRosterEntrySwap(0, vmEntryIndex, vmEntryId >= 0xD);
            vmEntryIndex = 0;
        }
        battleRosterCursorSet(vmEntryIndex, (vmEntryId < 0xD) ^ 1);
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

/* Copies the party (both rosters) of gBattleRosterTable into the save header and sets bit 1 of its snapshotFlags. */
s32 battleRosterSnapshotTake(void) {
    D_003E6E30.snapshotFlags |= 2;
    D_003E6E30.savedParty = gBattleRosterTable.party;
    return 1;
}

s32 vmInvokePairFromArgs(s32 vmThread) {
    extern s32 vmArgGet();
    s32 vmFirstGuestEntryId;
    s32 vmSecondGuestEntryId;

    vmFirstGuestEntryId = vmArgGet(vmThread, 1);
    vmSecondGuestEntryId = vmArgGet(vmThread, 2);
    battleRosterGuestPairSet(vmFirstGuestEntryId, vmSecondGuestEntryId);
    return 1;
}

/* VM native: sets bit 0 of roster table byte 0x27 when its argument is nonzero (default 1), clears it otherwise. */
s32 vmSetRosterTableFlag(s32 vmThread) {
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    s32 vmArgCount;
    s32 vmArgValue;

    vmArgCount = VM_PLUGIN_CNT_GET(vmThread);
    vmArgValue = 1;
    if (vmArgCount > 0) {
        vmArgValue = vmArgGet(vmThread, 1);
    }
    if (vmArgValue != 0) {
        gBattleRosterTable.unlockFlags |= 1;
    } else {
        gBattleRosterTable.unlockFlags &= 0xFE;
    }
    return 1;
}

s32 func_00230218(s32);
s32 func_0025F5C0(s32);

/* VM native: builds the roster flags from the current roster index and an optional shifted bit, then calls func_00230218 between func_0025F5C0(1) and func_0025F5C0(0). */
s32 vmSetRosterFlagsFromArgs(s32 vmThread) {
    extern u8 gBattleCurrentRosterIndex[];
    extern s32 VM_PLUGIN_CNT_GET(s32);
    extern s32 vmArgGet(s32, s32);
    s32 rosterBit;
    s32 vmArgCount;
    s32 vmArgValue;
    s32 rosterFlags;

    rosterBit = 1;
    rosterFlags = gBattleCurrentRosterIndex[0] == rosterBit;
    vmArgCount = VM_PLUGIN_CNT_GET(vmThread);
    if (vmArgCount > 0) {
        vmArgValue = vmArgGet(vmThread, 1);
        if (vmArgValue != 0) {
            rosterFlags |= rosterBit << vmArgValue;
        }
    }
    func_0025F5C0(1);
    func_00230218(rosterFlags);
    func_0025F5C0(0);
    return 1;
}

s32 seVoiceStartGuarded(s32);

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

extern s32 pagedTableFlush();

typedef struct NativeMethod {
    const char *name;
    s32 (*handler)();
    s32 argCount;
} NativeMethod;

NativeMethod gDefaultNativeMethods[22] = {
    { "ptInit", pagedTableFlush, 0 },
    { "ptIn", vmCallEachArgument, 1 },
    { "ptInTo", vmRaiseEntryKindTo, 2 },
    { "ptOut", vmForEachArgCall, 0 },
    { "ptSetAtk", vmUpdateEntryKinds, 3 },
    { "ptSetLeader", vmApplyIndexedKindArg, 1 },
    { "ptGetLeader", vmSetModeFromArgOrDefault, 0 },
    { "ptChangeMode", vmSetFlagFromArgOrDefault, 0 },
    { "ptGetNum", vmSetTableFromArgOrDefault, 0 },
    { "ptGetMember", vmGetRecordSubEntryFromArgs, 1 },
    { "ptGetMode", vmIsFlagSet, 0 },
    { "ptIn_Load", vmRunNativeWithPluginFlush, 1 },
    { "ptInTo_Load", vmRunSecondNativeWithPluginFlush, 2 },
    { "ptOut_Load", vmForEachArgCallWithFlush, 0 },
    { "ptSetAtk_Load", vmRunNativeFlushOnResult, 3 },
    { "ptSetLeader_Load", vmApplyIndexedKindWithFlush, 1 },
    { "ptChangeMode_Load", vmSetFlagFromArgWithFlush, 0 },
    { "ptSaveUMNParty", battleRosterSnapshotTake, 0 },
    { "ptSetGuest", vmInvokePairFromArgs, 2 },
    { "EnSeg", vmSetRosterTableFlag, 0 },
    { "OpenMenu", vmSetRosterFlagsFromArgs, 0 },
    { "OpenShop", vmStartSeVoiceFromArgGuarded, 1 },
};

s32 sbNativeOwnerAdd(const char *name, NativeMethod *methods);

/* Registers the default native module "mm". */
void addDefaultNativeOwner(void) {
    sbNativeOwnerAdd("mm", gDefaultNativeMethods);
}
