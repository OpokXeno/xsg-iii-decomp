#include "common.h"
#include "shared.h"

void textFormatHanTag(char *dst, const char *src);

int xglVectorMulMat(char *destination, const char *format, ...);

extern const char textHanTagFormat[];
extern const char textHanTagFormat[16];

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025C408);

/* Formats the "$han1;" markup tag followed by the name into first. */
void textFormatHanTag(char *outText, const char *name)
{
    xglVectorMulMat(outText, textHanTagFormat, name);
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", battleResultReset);

typedef signed char s8;
typedef int s32;

extern unsigned char gBattleChatterControlStorage[];

/* Battle chatter control block at gBattleChatterControlStorage (gBattleChatterControl): random line selection state; tu239 views the same block. */
typedef struct BattleChatterControl {
    u8 chatterFlags;                   /* 0x00: bit 0x2 suspends submissions; bit 0x1 is tu239's */
    unsigned char unk_01[0x1];
    s16 unk_02;                 /* 0x02: cleared by textStateSetup */
    unsigned char unk_04[0x4];
    s32 lastLineId;             /* 0x08: line picked by the last submission (tu239) */
    s32 *queue;                 /* 0x0C: entries of the last submission */
    s32 currentTextState;           /* 0x10 */
    s32 lastTextState;              /* 0x14: copy of currentTextState taken by textRefreshActorModes */
    void *stateEntryIds;                /* 0x18: default stateEntryIds, set by textStateReset */
} BattleChatterControl;

/* Table at gBattleModeActors: bytes 1 and 2 select the two actors that textRefreshActorModes refreshes. */
typedef struct TextActorTable {
    u8 unk_00;
    u8 actorIndexA;                 /* +0x01 */
    u8 actorIndexB;                 /* +0x02 */
} TextActorTable;

/* Actor view: the mode word at +0x4F4 that XenoActorSetMode takes (xenoActor.h names it unmodeled_4f4). */
typedef struct TextActor {
    u8 unk_00[0x4F4];
    s16 actorMode;                       /* +0x4F4 */
} TextActor;

extern unsigned char gBattleModeActors[];

#define gBattleChatterControl (*(BattleChatterControl *) gBattleChatterControlStorage)
#define gTextActorTable (*(TextActorTable *) gBattleModeActors)

void textStateResume(void) {
    func_0025C938();
    gBattleChatterControl.chatterFlags &= 0xFD;
}

void textStateSuspend(void) {
    gBattleChatterControl.chatterFlags |= 2;
}

extern unsigned char gTextDefaultEntryIds[];

void textStateReset(void)
{
  gBattleChatterControl.currentTextState = 0;
  gBattleChatterControl.lastTextState = 0;
  gBattleChatterControl.stateEntryIds = gTextDefaultEntryIds;
}

s32 textPickRandomEntry(s32 *candidateIds);
s32 func_0025C5D0();

void textSubmitRandomEntry(s32 *candidateIds, s32 actorSlotA, s32 actorSlotB) {
    extern s32 battleSetActorSlots();
    extern s32 textPickRandomEntry();
    s32 picked;

    battleSetActorSlots(actorSlotA, actorSlotB);
    if (!(gBattleChatterControl.chatterFlags & 2)) {
        gBattleChatterControl.queue = candidateIds;
        picked = textPickRandomEntry(candidateIds);
        func_0025C5D0(picked, actorSlotA, actorSlotB);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025C5D0);

void textStateSetup(s32 textStateId, void *stateEntryIds)
{
    extern void battleSetPhase();
  battleSetPhase(2);
  gBattleChatterControl.stateEntryIds = stateEntryIds;
  gBattleChatterControl.currentTextState = textStateId;
  gBattleChatterControl.unk_02 = 0;
  gBattleChatterControl.lastTextState = textStateId;
}

s32 xglSRand();

s32 textPickRandomEntry(s32 *candidateIds) {
    s32 *cursor;
    s32 randomRoll;
    s32 candidateCount;

    candidateCount = 0;
    if (candidateIds[0] != 0) {
        cursor = candidateIds;
        do {
            cursor += 1;
            candidateCount += 1;
        } while (cursor[0] != 0);
    }
    randomRoll = xglSRand();
    return candidateIds[randomRoll % candidateCount];
}

s32 XenoActorSetMode();
void *actorSlotGetActor();

void textRefreshActorModes(void)
{
  TextActorTable *actorTable = &gTextActorTable;
  BattleChatterControl *chatterControl = &gBattleChatterControl;
  TextActor *actorA;
  TextActor *actorB;

  chatterControl->lastTextState = chatterControl->currentTextState;
  actorA = actorSlotGetActor(actorTable->actorIndexA);
  if (actorA != 0)
  {
    XenoActorSetMode(actorA, actorA->actorMode);
  }
  actorB = actorSlotGetActor(actorTable->actorIndexB);
  if (actorB != 0)
  {
    XenoActorSetMode(actorB, actorB->actorMode);
  }
}

s32 func_0025ED28();

void *textCmdSetFloat(f32 *cmdArgs) {
    func_0025ED28((s32) cmdArgs[0]);
    return cmdArgs + 1;
}

s32 func_0025EF60(f32, f32, f32, f32);

void *textCmdSetVec4(f32 *cmdArgs) {
    func_0025EF60(cmdArgs[0], cmdArgs[1], cmdArgs[2], cmdArgs[3]);
    return cmdArgs + 4;
}

s32 func_0025EFF0(f32, f32, f32, f32);

void *textCmdSetRect(f32 *cmdArgs) {
    func_0025EFF0(cmdArgs[0], cmdArgs[1], cmdArgs[2], cmdArgs[3]);
    return cmdArgs + 4;
}

void textCommandStateSetKinds(s32, s32);

void *textCmdSetKinds(f32 *cmdArgs) {
    textCommandStateSetKinds((s32) cmdArgs[0], (s32) cmdArgs[1]);
    return cmdArgs + 2;
}

void textCommandStateSetScaledValue(f32);

void *textCmdSetScaledValue(f32 *cmdArgs) {
    textCommandStateSetScaledValue(cmdArgs[0]);
    return cmdArgs + 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025C8A0);

void textSetupSlotBlock(s32 actorSlotA, s32 actorSlotB);
s32 func_0025D518(s32, s32);
void textSubmitRandomClearA(s32 actorSlotA, s32 actorSlotB);
s32 func_0025D688();
void stateSetModeAndMark(s32 mode);

void textResetSubsystems(void) {
    textSetupSlotBlock(0, 0);
    textSubmitRandomClearA(0, 0);
    func_0025D518(0, 0);
    stateSetModeAndMark(0);
    func_0025D688();
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025C938);

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025CA10);

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025CA68);

extern s32 gTextAvailable;

void textSetAvailable(void) {
    gTextAvailable = 1;
}

s32 textCanPlayIndex(s32 actorSlot)
{
  if (gTextAvailable == 0)
  {
    return 0;
  }
  if (actorSlot < 3)
  {
    return 0;
  }
  return 1;
}

void textEndScene(void);
s32 func_0025D360();

void textFinishScene(void) {
    func_0025D360();
    textEndScene();
}

void textEndScene(void) {
    extern s32 battleSetPhase(s32);
    extern s32 battleSetActorSlots(s32, s32);
    battleSetActorSlots(0, 0);
    battleSetPhase(4);
}

void textSubmitRandomEntry(s32 *candidateIds, s32 actorSlotA, s32 actorSlotB);
extern s32 gTextTargetSecondaryEntries[];

void textSubmitTargetSecondary(s32 targetId) {
    textSubmitRandomEntry(gTextTargetSecondaryEntries, targetId, targetId);
}

extern s32 gTextTargetPrimaryEntries[];

void textSubmitEntry0408(s32 targetId) {
    textSubmitRandomEntry(gTextTargetPrimaryEntries, targetId, targetId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025CBB0);

extern s32 gTextLowSlotEntries[];
extern s32 gTextHighSlotEntries[];

void textSubmitIndexEntry(s32 actorSlotA, s32 actorSlotB) {
    if (actorSlotA < 3) {
        textSubmitRandomEntry(gTextLowSlotEntries, actorSlotA, actorSlotB);
        return;
    }
    textSubmitRandomEntry(gTextHighSlotEntries, actorSlotA, actorSlotB);
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025CD08);

s32 textMaxSlotValueWith(s32 unused_arg0, s32 actorSlotB);
extern s32 gTextScoredNoLevelEntries[];
extern s32 gTextScoredLowLevelEntries[];
extern s32 gTextScoredHighLevelEntries[];

void textSubmitScoredEntry(s32 actorSlotA, s32 actorSlotB) {
    s32 maxLevel;

    maxLevel = textMaxSlotValueWith(actorSlotA, actorSlotB);
    if (maxLevel <= 0) {
        textSubmitRandomEntry(gTextScoredNoLevelEntries, actorSlotA, actorSlotB);
        return;
    }
    if (maxLevel < 0x101) {
        textSubmitRandomEntry(gTextScoredLowLevelEntries, actorSlotA, actorSlotB);
        return;
    }
    textSubmitRandomEntry(gTextScoredHighLevelEntries, actorSlotA, actorSlotB);
}

extern s32 gTextTargetTertiaryEntries[];

void textSubmitTargetTertiary(s32 targetId) {
    textSubmitRandomEntry(gTextTargetTertiaryEntries, targetId, targetId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025CE58);

extern s32 gTextTargetLatchedEntries[];
extern s32 gTextLastTargetId;

void textSubmitEntry1100(s32 targetId) {
    s32 lastTargetId;

    lastTargetId = targetId;
    textSubmitRandomEntry(gTextTargetLatchedEntries, lastTargetId, lastTargetId);
    gTextLastTargetId = lastTargetId;
}

extern s32 gTextTargetLatchedSecondEntries[];

void textSubmitTargetLatchedSecond(s32 targetId) {
    s32 lastTargetId;

    lastTargetId = targetId;
    textSubmitRandomEntry(gTextTargetLatchedSecondEntries, lastTargetId, lastTargetId);
    gTextLastTargetId = lastTargetId;
}

extern s32 gTextLevelTierAZeroEntries[];
extern s32 gTextLevelTierALowEntries[];
extern s32 gTextLevelTierAHighEntries[];

void textSubmitBandEntry1500(s32 actorSlot) {
    extern s32 actorSlotGetLevel();
    s32 actorLevel;

    actorLevel = actorSlotGetLevel(actorSlot);
    if (actorLevel <= 0) {
        textSubmitRandomEntry(gTextLevelTierAZeroEntries, actorSlot, actorSlot);
        return;
    }
    if (actorLevel < 0x101) {
        textSubmitRandomEntry(gTextLevelTierALowEntries, actorSlot, actorSlot);
        return;
    }
    textSubmitRandomEntry(gTextLevelTierAHighEntries, actorSlot, actorSlot);
}

extern s32 gTextLevelTierBZeroEntries[];
extern s32 gTextLevelTierBLowEntries[];
extern s32 gTextLevelTierBHighEntries[];

void textSubmitBandEntry1830(s32 actorSlot) {
    extern s32 actorSlotGetLevel();
    s32 actorLevel;

    actorLevel = actorSlotGetLevel(actorSlot);
    if (actorLevel <= 0) {
        textSubmitRandomEntry(gTextLevelTierBZeroEntries, actorSlot, actorSlot);
        return;
    }
    if (actorLevel < 0x101) {
        textSubmitRandomEntry(gTextLevelTierBLowEntries, actorSlot, actorSlot);
        return;
    }
    textSubmitRandomEntry(gTextLevelTierBHighEntries, actorSlot, actorSlot);
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025D0B8);

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025D130);

extern s32 gTextTargetIfSetEntries[];

void textSubmitTargetIfSet(s32 targetId) {
    textSubmitRandomEntry(gTextTargetIfSetEntries, targetId, targetId);
}

extern s32 gTextNotAllFlaggedEntries[];

void textSubmitNotAllFlagged(void) {
    textSubmitRandomEntry(gTextNotAllFlaggedEntries, 0, 0);
}

extern s32 gTextAllFlaggedEntries[];

void textSubmitAllFlagged(void) {
    textSubmitRandomEntry(gTextAllFlaggedEntries, 3, 3);
}

s32 textMaxActiveSlotValue(void) {
    extern s32 actorSlotGetLevel(s32);
    s32 slotLevel;
    s32 checkedActorSlot;
    s32 nextActorSlot;
    s32 maxSlotLevel;

    nextActorSlot = 3;
    maxSlotLevel = 0;
    do {
        checkedActorSlot = nextActorSlot;
        nextActorSlot += 1;
        slotLevel = actorSlotGetLevel(checkedActorSlot);
        if (maxSlotLevel < slotLevel) {
            maxSlotLevel = slotLevel;
        }
    } while (nextActorSlot < 8);
    return maxSlotLevel;
}

s32 textMaxSlotValueWith(s32 unused_arg0, s32 actorSlotB) {
    extern s32 actorSlotGetLevel();
    s32 levelA;
    s32 levelB;

    levelA = actorSlotGetLevel();
    levelB = actorSlotGetLevel(actorSlotB);
    return (levelA >= levelB) ? levelA : levelB;
}

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025D2D8);

INCLUDE_ASM("asm/main/nonmatchings/tu238", func_0025D360);

extern s32 gTextScaledEntries[];
extern f32 gTextUniformScale;
void xglScaleSetXY(f32, f32);

void textSubmitEntryWithScale(void) {
    textSubmitRandomEntry(gTextScaledEntries, 0, 0);
    xglScaleSetXY(gTextUniformScale, gTextUniformScale);
}

extern f32 D_0054F250;
void *battleGetRecord();
s32 memset(void *, s32, s32);

/* The 0x60-byte block battleGetRecord returns (global gBattleRecord); textSetupSlotBlock fills it. */
typedef struct TextSlotBlock {
    u8 unk_00[0x20];
    f32 unk_20;                     /* set from D_0054F250 */
    s32 actorSlotA;                      /* +0x24 */
    s32 actorSlotB;                     /* +0x28 */
    f32 unk_2C;                     /* set to 13.0f */
    u8 unk_30[0x60 - 0x30];
} TextSlotBlock;

void textSetupSlotBlock(s32 actorSlotA, s32 actorSlotB)
{
    extern s32 battleSetPhase(s32);
    extern s32 battleSetActorSlots(s32, s32);
  TextSlotBlock *battleRecord;
  battleRecord = battleGetRecord();
  memset(battleRecord, 0, sizeof(TextSlotBlock));
  battleRecord->unk_20 = D_0054F250;
  battleRecord->unk_2C = 13.0f;
  battleRecord->actorSlotB = actorSlotB;
  battleRecord->actorSlotA = actorSlotA;
  battleSetActorSlots(actorSlotA, actorSlotB);
  battleSetPhase(3);
}

extern s32 D_003E2028[];

void textSubmitEntry2028(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2028, actorSlot, actorSlot);
}

extern s32 D_003E2198[];

void textSubmitEntry2198(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2198, actorSlot, actorSlot);
}
