/*
 * OV02 original TU 85: 0x00aad248..0x00ab06c8 (67 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned int u32;
typedef struct EntryOwner {
    u8 unmodeled_00[0x2B4];
    s32 battleUnitHandle;                     /* +0x2B4 */
} EntryOwner;

typedef struct EntryActor {
    u8 unmodeled_00[0xA0];
    EntryOwner *owner;              /* +0xA0 */
    u8 unmodeled_A4[0xE4 - 0xA4];
    s32 entryId;                    /* +0xE4 */
    u8 unmodeled_E8[0x238 - 0xE8];
    s32 mode4Requested;              /* +0x238 */
    s32 mode1Requested;              /* +0x23C */
    u8 unmodeled_240[0x600 - 0x240];
    s32 stateTag;                   /* +0x600 */
    s32 actionState;                 /* +0x604 */
} EntryActor;

typedef struct BattleSourceRecord {
    u8 unmodeled_00[0xAC];
    s32 sourceId;                   /* +0xAC */
} BattleSourceRecord;

typedef struct BattleSourceTable {
    u8 unmodeled_00[0x18];
    BattleSourceRecord *activeSourceRecords[11];       /* +0x18 */
    s32 activeSourceIds[11];             /* +0x44 */
    s32 activeRecordCount;                /* +0x70 */
    u8 unmodeled_74[0x78 - 0x74];
    s32 currentActionKind;                /* +0x78 */
    s32 currentActiveIndex;                /* +0x7C */
    u8 unmodeled_80[0x13C - 0x80];
} BattleSourceTable;

typedef struct EntrySourceIdList {
    s32 sourceIds[13];              /* +0x00 */
    s32 sourceIdCount;                      /* +0x34 */
} EntrySourceIdList;

typedef struct BattleStatRecord {
    u8 unmodeled_00[6];
    u8 statFlags;                       /* +0x06 */
} BattleStatRecord;

typedef struct BattleTextRecord {  /* 0x18-byte entries indexed by setBattleEntryRecord */
    s32 firstText;                      /* +0x00 */
    s32 secondText;                     /* +0x04 */
    s32 thirdValue;                      /* +0x08 */
    u8 unmodeled_0C[0x10 - 0x0C];
    s32 usesFlagValue;                       /* +0x10 */
    s32 selectedValue;                      /* +0x14 */
} BattleTextRecord;

typedef struct EntryTemplate {  /* 0xC-byte entries of gEntryTemplateTable */
    s32 firstText;                      /* +0x00 */
    s32 secondText;                     /* +0x04 */
    s32 flaggedValue;                      /* +0x08 */
} EntryTemplate;

s32 func_002E3850();
void *memset(void *, s32, s32);

void battleSourceTableInit(void) {
    extern BattleSourceTable gBattleSourceTable;
    extern s32 func_00AAFCC8(s32);
    memset(&gBattleSourceTable, 0, 0x13C);
    func_00AAFCC8(0);
    func_002E3850();
}

s32 mainCallbackHookClear();

void battleSourceTableInitWithCallback(void) {
    extern BattleSourceTable gBattleSourceTable;
    memset(&gBattleSourceTable, 0, 0x13C);
    mainCallbackHookClear();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAD2A0);

s32 padPressedAll();
s32 padHeldAll();
s32 seVoiceIsEnabled();
s32 func_002C3668();
s32 mainSetPlaybackFlag();
s32 func_00AA1928();

void seVoiceUpdateFlags(s32 padButtonsRecord) {
    extern unsigned char D_00ABB198[];
    extern s32 roboBattleUnitGet();
    s32 voiceUnitSlot;
    voiceUnitSlot = func_00AA1928();
    roboBattleUnitGet(voiceUnitSlot, D_00ABB198, 0x107);
    if (padButtonsRecord == 0) {
        return;
    }
    if (seVoiceIsEnabled() != 0) {
        if ((padPressedAll(padButtonsRecord, 0x800) != 0) || (padPressedAll(padButtonsRecord, 0x20) != 0) || (padPressedAll(padButtonsRecord, 0x40) != 0)) {
            func_002C3668();
        }
    }
    if ((padHeldAll(padButtonsRecord, 4) != 0) || (padHeldAll(padButtonsRecord, 1) != 0)) {
        mainSetPlaybackFlag(1);
        return;
    }
    mainSetPlaybackFlag(0);
}

s32 mainFrameUpdate();

void entryCancelCurrent(void) {
    mainFrameUpdate();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAD490);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAD5B8);

void publishEntryRecords(void);
s32 func_002E39D0();

void publishEntryRecordsAndSync(void) {
    publishEntryRecords();
    func_002E39D0();
}

void entryTeardown(void);
s32 effectPoolResetCounters();

void entryDrainThenTeardown(void) {
    effectPoolResetCounters();
    entryTeardown();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAD680);

s32 publishMaskSet();

void entryStopWait(void) {
    publishMaskSet();
}

void battleSlotsAndVoiceClear(void);
s32 sceneEntryStopCurrent();
s32 entryResetSeVoiceSlots();

void entryTeardown(void) {
    battleSlotsAndVoiceClear();
    entryResetSeVoiceSlots();
    sceneEntryStopCurrent();
}

s32 entryPrepareStart();
s32 roboCurrentIsPlayer();

void battleEntryStartWithMode(s32 battleUnitHandle, s32 tag, s32 actionFlags) {
    s32 sideMode;

    sideMode = 0;
    if (tag != 0) {
        sideMode = 2;
        if (roboCurrentIsPlayer(tag) == 0) {
            sideMode = 1;
        }
    }
    entryPrepareStart(battleUnitHandle, actionFlags, sideMode);
}

void battleEntryStartWithMode(s32 unit, s32 tag, s32 actionFlags);

void battleEntryStartDefault(s32 battleUnitHandle) {
    battleEntryStartWithMode(battleUnitHandle, 0, 0);
}

s32 entrySyncStep(s32);
s32 roboCurrentIsEnemy();

void notifyOwnerHandle(EntryActor *actor) {
    s32 ownerHandle;
    EntryOwner *owner;

    if (roboCurrentIsEnemy() == 0) {
        return;
    }
    owner = actor->owner;
    ownerHandle = 0;
    if (owner != 0) {
        ownerHandle = owner->battleUnitHandle;
    }
    entrySyncStep(ownerHandle);
}

s32 recordQueueSetup();
s32 recordSubmitQueued(s32, s32, s32);

void battleUnitSubmitQueuedValue(s32 unused, s32 queuedAmount, s32 queuedValue) {
    recordQueueSetup();
    recordSubmitQueued(queuedAmount, queuedValue, 0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAD7F8);

s32 ownerModeRequest(s32, s32);

s32 requestOwnerMode1(EntryActor *actor) {
    s32 requestDone;
    s32 ownerHandle;
    EntryOwner *owner;

    requestDone = actor->mode1Requested;
    if (requestDone == 0) {
        owner = actor->owner;
        ownerHandle = 0;
        if (owner != 0) {
            ownerHandle = owner->battleUnitHandle;
        }
        ownerModeRequest(ownerHandle, 1);
        requestDone = 1;
        actor->mode1Requested = 1;
    }
    return requestDone;
}

s32 requestOwnerMode4(EntryActor *actor) {
    s32 requestDone;
    s32 ownerHandle;
    EntryOwner *owner;

    requestDone = actor->mode4Requested;
    if (requestDone == 0) {
        owner = actor->owner;
        ownerHandle = 0;
        if (owner != 0) {
            ownerHandle = owner->battleUnitHandle;
        }
        ownerModeRequest(ownerHandle, 4);
        requestDone = 1;
        actor->mode4Requested = 1;
    }
    return requestDone;
}

s32 recordSubmitRelease(s32);

void actorOwnerHandleRelease(EntryActor *actor) {
    s32 ownerHandle;
    EntryOwner *owner;

    owner = actor->owner;
    ownerHandle = 0;
    if (owner != 0) {
        ownerHandle = owner->battleUnitHandle;
    }
    recordSubmitRelease(ownerHandle);
}

s32 func_002E69B8();

void robotEntryUpdateStep(void) {
    func_002E69B8();
}

s32 func_002E4B00(s32);
s32 seVoiceStateReset();

void battleSlotsAndVoiceClear(void) {
    func_002E4B00(0);
    func_002E4B00(1);
    seVoiceStateReset();
}

s32 battleActorHasUnitHandle();

void publishEntryRecord(s32 battleUnitSlot) {
    extern unsigned char D_00ABB198[];
    extern s32 func_002E4290();
    extern s32 func_002E4858();
    extern s32 roboBattleUnitGet();
    extern s32 func_00AB0180();
    s32 unitSnapshot[0x10];
    s32 battleUnitHandle;

    if (battleUnitSlot != -1) {
        battleUnitHandle = roboBattleUnitGet(battleUnitSlot, D_00ABB198, 0x219);
        if (battleActorHasUnitHandle(battleUnitHandle) != 0) {
            func_00AB0180(&unitSnapshot[0], battleUnitHandle);
            func_002E4290(5, &unitSnapshot[0]);
            func_002E4858(5, 1);
        }
    }
}

void func_002E49D0();
void publishedCountSet();
s32 roboRemainingEnemiesGet();
s32 func_00AA1410();

void publishEntryRecords(void) {
    extern unsigned char D_00ABB198[];
    extern void func_002E4290();
    extern void func_002E4858();
    extern s32 roboBattleUnitGet();
    extern void func_00AB0180();
    s32 unitSnapshot[0x10];
    s32 unitSlotCursor;
    s32 battleUnitSlot;
    s32 battleUnitHandle;
    s32 unitActorSlot;
    s32 recordId;
    s32 publishedCount;

    unitSlotCursor = 0;
    do {
        battleUnitSlot = unitSlotCursor;
        unitSlotCursor += 1;
        battleUnitHandle = roboBattleUnitGet(battleUnitSlot, D_00ABB198, 0x22C);
        if (battleActorHasUnitHandle(battleUnitHandle) != 0) {
            unitActorSlot = func_00AA1410(battleUnitHandle);
            func_00AB0180(&unitSnapshot[0], battleUnitHandle);
            recordId = unitActorSlot + 1;
            func_002E4290(recordId, &unitSnapshot[0]);
            func_002E4858(recordId, 1);
        }
    } while (unitSlotCursor < 4);
    func_002E49D0(1);
    publishedCount = roboRemainingEnemiesGet();
    publishedCountSet(publishedCount);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AADBA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AADC40);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AADDA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AADE80);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AADFC8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAE040);

s32 slotEntryClear();

void battleSourceRecordRelease(BattleSourceRecord *releasedRecord) {
    extern BattleSourceTable gBattleSourceTable;
    BattleSourceTable *activeSourceTable = &gBattleSourceTable;
    s32 activeIndex;
    BattleSourceRecord **activeSourceSlot;

    activeIndex = 0;
    if (activeSourceTable->activeRecordCount > 0) {
        activeSourceSlot = activeSourceTable->activeSourceRecords;
        do {
            if (*activeSourceSlot == releasedRecord) {
                slotEntryClear(activeIndex, 4);
                *activeSourceSlot = 0;
            }
            activeIndex += 1;
            activeSourceSlot += 1;
        } while (activeIndex < activeSourceTable->activeRecordCount);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAE158);

void battleSourceIdListClear(void) {
    extern EntrySourceIdList gBattleSourceIdList;
    memset(&gBattleSourceIdList, 0, 0x38);
}

void appendEntryRecord(BattleSourceRecord *battleSourceRecord)
{
    extern BattleSourceTable gBattleSourceTable;
    extern EntrySourceIdList gBattleSourceIdList;
    BattleSourceTable *activeSourceTable = &gBattleSourceTable;
    EntrySourceIdList *sourceIdList = &gBattleSourceIdList;
    BattleSourceRecord *appendedRecord = battleSourceRecord;
    s32 *sourceIdCountPtr;
    s32 sourceId;
    s32 sourceKey;
    s32 newSourceIdCount;
    s32 updatedActiveCount;

    sourceIdCountPtr = &sourceIdList->sourceIdCount;
    sourceId = appendedRecord->sourceId;
    newSourceIdCount = *sourceIdCountPtr + 1;
    sourceIdList->sourceIds[*sourceIdCountPtr] = sourceId;
    *sourceIdCountPtr = newSourceIdCount;
    activeSourceTable->activeSourceRecords[activeSourceTable->activeRecordCount] = appendedRecord;
    sourceKey = appendedRecord->sourceId;
    activeSourceTable->activeSourceIds[activeSourceTable->activeRecordCount] = sourceKey;
    updatedActiveCount = activeSourceTable->activeRecordCount + 1;
    activeSourceTable->activeRecordCount = updatedActiveCount;
}

s32 recordListFlush(s32 *, s32);

void flushRecordList(void) {
    extern EntrySourceIdList gBattleSourceIdList;

    if (gBattleSourceIdList.sourceIdCount > 0) {
        recordListFlush(gBattleSourceIdList.sourceIds, gBattleSourceIdList.sourceIdCount);
    }
}

s32 slotRecord2CountDiff();

s32 entryQueueHasItems(void) {
    return slotRecord2CountDiff() > 0;
}

extern s32 gEntryQueueDefaultBlock;
s32 scratchpadSendCmd23Mode1(s32 *);

void entryQueueFlushDefault(void) {
    scratchpadSendCmd23Mode1(&gEntryQueueDefaultBlock);
}

extern s32 gEntryQueuePrimaryBlock;
extern s32 gEntryQueueAlternateBlock;
s32 scratchpadSendCmd23Mode0(s32 *);

void entryQueueFlushSelected(s32 alternate) {
    if (alternate != 0) {
        scratchpadSendCmd23Mode0(&gEntryQueueAlternateBlock);
        return;
    }
    scratchpadSendCmd23Mode0(&gEntryQueuePrimaryBlock);
}

void entryHookNoop(void) {

}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAE318);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAE5A0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAE6C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAE9B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAEA78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAEC80);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAEDA8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAEEE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAF0B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAF200);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAF368);

void publishEntryRecord(s32 unitIndex);
s32 func_002E4A08(s32);
s32 func_00AAE040(s32);
s32 battleUnitSetStatValue(s32, s32, s32);

void entryPublishCurrent(s32 battleUnitHandle) {
    extern s32 D_00ABB198;
    extern BattleSourceTable gBattleSourceTable;
    extern s32 func_002E4858(s32, s32);
    extern BattleStatRecord *ov02BattleSlotGet(s32);
    extern s32 func_00AAB820(s32, s32, s32, s32 *, s32);
    BattleSourceTable *activeSourceTable = &gBattleSourceTable;
    s32 activeIndex;
    BattleStatRecord *actionStat;

    actionStat = ov02BattleSlotGet(activeSourceTable->currentActionKind);
    activeIndex = activeSourceTable->currentActiveIndex;
    if (activeSourceTable->currentActionKind == 0x1A) {
        func_00AAB820(0x12, battleUnitHandle, 0, &D_00ABB198, 0x5BC);
    } else {
        func_00AAB820(0x11, battleUnitHandle, 0, &D_00ABB198, 0x5BE);
    }
    if (actionStat->statFlags & 8) {
        func_002E4858(5, 0);
    }
    publishEntryRecords();
    publishEntryRecord(activeIndex);
    battleUnitSetStatValue(battleUnitHandle, activeSourceTable->currentActionKind, activeSourceTable->currentActiveIndex);
    func_00AAE040(activeIndex);
    func_002E4A08(1);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAF690);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAF918);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAFB60);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAFCC8);

s32 mainClearPackStateB();
s32 seStartDefaultVoice(s32);

void battleResetToSlot(s32 actionSlot) {
    extern s32 func_00AAFCC8(s32);
    seStartDefaultVoice(4);
    mainClearPackStateB();
    func_00AAFCC8(actionSlot);
}

extern s32 gActorEntryStarted;
s32 mainActorPrepare();

void startActorEntry(EntryActor *actor) {
    extern s32 D_00ABB198;
    extern s32 roboBattleUnitGet(s32, s32 *, s32);
    extern s32 func_00AAB820(s32, void *, s32, s32 *, s32);
    extern s32 func_00AAFCC8(s32);
    s32 actorUnitSlot;
    gActorEntryStarted = 1;
    mainActorPrepare();
    actorUnitSlot = roboBattleUnitGet(actor->entryId, &D_00ABB198, 0x6B8);
    func_00AAB820(1, actor, actorUnitSlot, &D_00ABB198, 0x6B8);
    func_00AAFCC8(1);
}

s32 entryRecordIsMarked(void) {
    extern BattleStatRecord *ov02BattleSlotGet();
    return (ov02BattleSlotGet()->statFlags >> 1) & 1;
}

void setActorState(EntryActor *actor, s32 actionState) {
    actor->stateTag = 0;
    actor->actionState = actionState;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAFDF0);

extern char gSetEntryRecordAssertMessage[];
void assertFailed(const char *message, const char *file, int line);

void setBattleEntryRecord(BattleTextRecord *textRecords, s32 textRecordIndex, s32 firstText, s32 secondText, s32 thirdValue, s32 flaggedValue, s32 fallbackValue, s32 usesFlagValue) {
    extern char D_00ABB198[];
    BattleTextRecord *textRecord;

    if (textRecordIndex >= 0x40) {
        assertFailed(gSetEntryRecordAssertMessage, D_00ABB198, 0x6DE);
    }
    textRecord = &textRecords[textRecordIndex];
    textRecord->firstText = firstText;
    textRecord->secondText = secondText;
    if (usesFlagValue != 0) {
        textRecord->selectedValue = flaggedValue;
        textRecord->thirdValue = thirdValue;
        textRecord->usesFlagValue = 1;
        return;
    }
    textRecord->thirdValue = thirdValue;
    textRecord->usesFlagValue = 0;
    textRecord->selectedValue = fallbackValue;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAFF88);

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AAFFD0);

void setBattleEntryRecord(BattleTextRecord *entries, s32 index, s32 firstText, s32 secondText, s32 thirdValue, s32 flaggedValue, s32 defaultValue, s32 usesFlagValue);
extern EntryTemplate gEntryTemplateTable[];
extern unsigned char gEntryTemplateAssertMessage[];
s32 stringTableEntryGet(s32);

void setEntryRecordFromTemplate(BattleTextRecord *textRecords, s32 textRecordIndex, u32 templateIndex, s32 usesFlagValue) {
    extern unsigned char D_00ABB198[];
    s32 firstText;
    EntryTemplate *templateEntry;
    s32 secondText;

    if (templateIndex >= 0x18U) {
        assertFailed(gEntryTemplateAssertMessage, D_00ABB198, 0x711);
    }
    templateEntry = &gEntryTemplateTable[templateIndex];
    firstText = stringTableEntryGet(templateEntry->firstText);
    secondText = stringTableEntryGet(templateEntry->secondText);
    setBattleEntryRecord(textRecords, textRecordIndex, firstText, secondText, 0, templateEntry->flaggedValue, 0, usesFlagValue);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AB0180);

s32 mainSetPlaybackMode();
s32 stateApplyEnableLevel();
s32 roboParamSumMatching();

void actorSetOwnerMode(EntryActor *actor, s32 ownerMode) {
    s32 ownerHandle;
    EntryOwner *owner;

    owner = actor->owner;
    ownerHandle = 0;
    if (owner != 0) {
        ownerHandle = owner->battleUnitHandle;
    }
    stateApplyEnableLevel(ownerHandle, ownerMode);
    if (ownerMode == 0) {
        if (roboParamSumMatching(0xB, 2, 0) != 0) {
            mainSetPlaybackMode(3);
            return;
        }
        mainSetPlaybackMode(2);
        return;
    }
    mainSetPlaybackMode(0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu085", func_00AB0350);
