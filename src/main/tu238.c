#include "common.h"
#include "shared.h"

void textFormatHanTag(char *dst, const char *src);
int xglVectorMulMat(char *destination, const char *format, ...);
static char textHanTagFormat[64];
void xglMatrixStackUnit(void);
void xglMatrixStackRotX(f32 angle);
void xglMatrixStackRotY(f32 angle);
void xglMatrixStackTrans(DoublewordAlignedVector4 *translation);
void xglMatrixStackSave(f32 *matrix);

/* Camera view for the battle text: identity matrix, rotate by -angles[0] (X) and -angles[1] (Y), translate by -offset,
   then save the 16-float result into textHanTagFormat (64-byte static, same storage as the format string). */
void func_0025C408(Vector4 *cameraOffset, f32 *angles) {
    DoublewordAlignedVector4 translation = {-cameraOffset->x, -cameraOffset->y, -cameraOffset->z, 1.0f};

    xglMatrixStackUnit();
    xglMatrixStackRotX(-angles[0]);
    xglMatrixStackRotY(-angles[1]);
    xglMatrixStackTrans(&translation);
    xglMatrixStackSave((f32 *) textHanTagFormat);
}

/* Formats the "$han1;" markup tag followed by the name into first. */
void textFormatHanTag(char *outText, const char *name)
{
    xglVectorMulMat(outText, textHanTagFormat, name);
}

extern unsigned char gBattleChatterControlStorage[];

typedef int s32;

s32 memset(void *, s32, s32);

/* Clears the 0x50-byte battle chatter control block. */
void battleResultReset(void) {
    memset(gBattleChatterControlStorage, 0, 0x50);
}

/* Battle chatter control block at gBattleChatterControlStorage (gBattleChatterControl): random line selection state; tu239 views the same block. */
/* One entry of a text candidate list: the state block, then the entry's id string. */
typedef struct TextStateEntry {
    f32 *textStateId;
    const char *stateEntryIds;
} TextStateEntry;

typedef struct BattleChatterControl {
    u8 chatterFlags; /* 0x00: bit 0x2 suspends submissions; bit 0x1 is tu239's */
    unsigned char unk_01[0x1];
    s16 waitFrames; /* 0x02: cleared by textStateSetup, counted by the wait command */
    unsigned char unk_04[0x4];
    s32 lastLineId;            /* 0x08: line picked by the last submission (tu239) */
    TextStateEntry **queue;    /* 0x0C: entries of the last submission */
    f32 *currentTextState;     /* 0x10 */
    f32 *lastTextState;        /* 0x14: copy of currentTextState taken by textRefreshActorModes */
    const char *stateEntryIds; /* 0x18: default stateEntryIds, set by textStateReset */
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
    textRunCommands();
    gBattleChatterControl.chatterFlags &= 0xFD;
}

void textStateSuspend(void) {
    gBattleChatterControl.chatterFlags |= 2;
}

void textStateReset(void) {
    gBattleChatterControl.currentTextState = 0;
    gBattleChatterControl.lastTextState = 0;
    gBattleChatterControl.stateEntryIds = "\245\253\245\341\245\351\314\265\244\267";
}

TextStateEntry *textPickRandomEntry(TextStateEntry **candidateIds);
void textApplyPickedEntry(TextStateEntry *pickedEntry, s32 actorSlotA, s32 actorSlotB);

void textSubmitRandomEntry(TextStateEntry **candidateIds, s32 actorSlotA, s32 actorSlotB) {
    extern s32 battleSetActorSlots();
    TextStateEntry *picked;

    battleSetActorSlots(actorSlotA, actorSlotB);
    if (!(gBattleChatterControl.chatterFlags & 2)) {
        gBattleChatterControl.queue = candidateIds;
        picked = textPickRandomEntry(candidateIds);
        textApplyPickedEntry(picked, actorSlotA, actorSlotB);
    }
}

extern s32 battleSetActorSlots(s32, s32);
void textStateSetup(f32 *textStateId, const char *stateEntryIds);
void xglScaleSetXY(f32, f32);
void cameraTablesClear(void);

/* Applies one picked entry: its actor slots, then (unless submissions are suspended) its text state, the 0.7/0.55 scale
   and a camera-table clear. */
void textApplyPickedEntry(TextStateEntry *pickedEntry, s32 actorSlotA, s32 actorSlotB) {
    battleSetActorSlots(actorSlotA, actorSlotB);
    if (gBattleChatterControl.chatterFlags & 2) {
        return;
    }
    textStateSetup(pickedEntry->textStateId, pickedEntry->stateEntryIds);
    xglScaleSetXY(0.7f, 0.55f);
    cameraTablesClear();
}

void textStateSetup(f32 *textStateId, const char *stateEntryIds) {
    extern void battleSetPhase();
    battleSetPhase(2);
    gBattleChatterControl.stateEntryIds = stateEntryIds;
    gBattleChatterControl.currentTextState = textStateId;
    gBattleChatterControl.waitFrames = 0;
    gBattleChatterControl.lastTextState = textStateId;
}

s32 xglSRand();

TextStateEntry *textPickRandomEntry(TextStateEntry **candidateIds) {
    TextStateEntry **cursor;
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

s32 battleTextCommandInit();

void *textCmdSetFloat(f32 *cmdArgs) {
    battleTextCommandInit((s32) cmdArgs[0]);
    return cmdArgs + 1;
}

s32 battleTextCommandSetFirstPoint(f32, f32, f32, f32);

void *textCmdSetVec4(f32 *cmdArgs) {
    battleTextCommandSetFirstPoint(cmdArgs[0], cmdArgs[1], cmdArgs[2], cmdArgs[3]);
    return cmdArgs + 4;
}

s32 battleTextCommandSetSecondPoint(f32, f32, f32, f32);

void *textCmdSetRect(f32 *cmdArgs) {
    battleTextCommandSetSecondPoint(cmdArgs[0], cmdArgs[1], cmdArgs[2], cmdArgs[3]);
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

/* Text wait command: the first call loads the frame count from the argument, later calls count it down. While the count
   is positive the command repeats (returns cmdArgs - 1); at zero the stream moves on. */
void *textCmdWait(f32 *cmdArgs) {
    if (gBattleChatterControl.waitFrames <= 0) {
        gBattleChatterControl.waitFrames = (s16) cmdArgs[0];
    } else {
        gBattleChatterControl.waitFrames = gBattleChatterControl.waitFrames - 1;
    }
    if (gBattleChatterControl.waitFrames > 0) {
        return cmdArgs - 1;
    }
    return cmdArgs + 1;
}

void textSetupSlotBlock(s32 actorSlotA, s32 actorSlotB);
s32 func_0025D518(s32, s32);
void textSubmitRandomClearA(s32 actorSlotA, s32 actorSlotB);
s32 textSubmitBattleModeLine();
void stateSetModeAndMark(s32 mode);

void textResetSubsystems(void) {
    textSetupSlotBlock(0, 0);
    textSubmitRandomClearA(0, 0);
    func_0025D518(0, 0);
    stateSetModeAndMark(0);
    textSubmitBattleModeLine();
}

extern void *(*gTextCmdHandlers[8])(f32 *cmdArgs);

/* Runs the text command stream from lastTextState: each code word dispatches through gTextCmdHandlers and returns the next
   position, stopping at the end or at a bad code. */
void textRunCommands(void) {
    f32 *cmdArgs;
    f32 *nextCmd;

    if (gBattleChatterControl.lastTextState == 0) {
        return;
    }
    if ((unsigned int) ((s32) *gBattleChatterControl.lastTextState - 1) >= 6) {
        gBattleChatterControl.lastTextState = 0;
        return;
    }
    /* kept: the goto into the loop body decides the emitted loop shape */
    goto runCommand;
    do {
        nextCmd = 0;
    runCommand:
        cmdArgs = gBattleChatterControl.lastTextState;
        nextCmd = (*gTextCmdHandlers[(s32) *cmdArgs])(cmdArgs + 1);
        gBattleChatterControl.lastTextState = nextCmd;
        if (nextCmd == 0 || nextCmd == cmdArgs) {
            return;
        }
        if ((unsigned int) ((s32) *nextCmd - 1) >= 6) {
            gBattleChatterControl.lastTextState = 0;
            return;
        }
    } while (1);
}

void *battleGetRecord();
static s32 gTextAvailable;
static s32 gTextLastTargetId;
/* Scale pending flag: textResetRecord sets it, textSetScaleRecord clears it, textApplyPendingScale checks it. The name stays gTextScalePending
   because it is a local symbol of the object. */
static s32 gTextScalePending;

/* The 0x60-byte battle record that battleGetRecord returns (global gBattleRecord); textSetupSlotBlock, textResetRecord and
   textSetScaleRecord all use this one type. */
typedef struct TextSlotBlock {
    f32 unk_00; /* +0x00: set by textSetScaleRecord */
    f32 unk_04; /* +0x04: set by textSetScaleRecord */
    f32 unk_08; /* +0x08: set by textSetScaleRecord */
    u8 unk_0C[0x10 - 0x0C];
    f32 unk_10; /* +0x10: set by textSetScaleRecord */
    f32 unk_14; /* +0x14: set by textSetScaleRecord */
    u8 unk_18[0x20 - 0x18];
    f32 unk_20;     /* set from 0.6981312f (textSetupSlotBlock, textResetRecord) */
    s32 actorSlotA; /* +0x24 */
    s32 actorSlotB; /* +0x28 */
    f32 unk_2C;     /* set to 13.0f */
    u8 unk_30[0x50 - 0x30];
    s16 unk_50; /* +0x50: set to 45 by textResetRecord */
    u8 unk_52[0x56 - 0x52];
    union {
        s16 unk_56; /* +0x56: set by textSetScaleRecord */
        u16 unk_56Unsigned;
    } halfword56;
    u8 unk_58[0x60 - 0x58];
} TextSlotBlock;

/* Clears the battle record and the text availability state; the record gets the default angle and 45 at +0x50, and the
   scale pending flag is set. */
void textResetRecord(void) {
    TextSlotBlock *battleRecord;

    battleRecord = battleGetRecord();
    memset(battleRecord, 0, sizeof(TextSlotBlock));
    battleRecord->unk_20 = 0.6981312037f;
    battleRecord->unk_50 = 45;
    gTextScalePending = 1;
    gTextAvailable = 0;
    gTextLastTargetId = 0;
}


/* Stores one scale record into the battle record: the mode halfword at +0x56 and the five scale floats, then clears
   gTextScalePending. The stores take the floats in the callee's order (see textApplyPendingScale for the caller's order). */
void textSetScaleRecord(s32 scaleMode, f32 scaleA, f32 scaleB, f32 scaleC, f32 scaleD, f32 scaleE) {
    TextSlotBlock *battleRecord;

    battleRecord = battleGetRecord();
    battleRecord->unk_14 = scaleB;
    battleRecord->unk_00 = scaleC;
    battleRecord->unk_04 = scaleD;
    battleRecord->unk_08 = scaleE;
    battleRecord->halfword56.unk_56 = scaleMode;
    battleRecord->unk_10 = scaleA;
    gTextScalePending = 0;
}

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
void textApplyPendingScale(void);

void textFinishScene(void) {
    textApplyPendingScale();
    textEndScene();
}

void textEndScene(void) {
    extern s32 battleSetPhase(s32);
    extern s32 battleSetActorSlots(s32, s32);
    battleSetActorSlots(0, 0);
    battleSetPhase(4);
}

void textSubmitRandomEntry(TextStateEntry **candidateIds, s32 actorSlotA, s32 actorSlotB);
extern TextStateEntry *gTextTargetSecondaryEntries[];

void textSubmitTargetSecondary(s32 targetId) {
    textSubmitRandomEntry(gTextTargetSecondaryEntries, targetId, targetId);
}

extern TextStateEntry *gTextTargetPrimaryEntries[];

void textSubmitEntry0408(s32 targetId) {
    textSubmitRandomEntry(gTextTargetPrimaryEntries, targetId, targetId);
}

extern TextStateEntry *D_003E0610[];
extern TextStateEntry *D_003E0620[];
extern TextStateEntry *D_003E07C0[];
extern TextStateEntry *D_003E07D0[];
extern TextStateEntry *D_003E0970[];
extern TextStateEntry *D_003E0980[];
s32 textMaxSlotValueWith(s32 unused_arg0, s32 actorSlotB);

/* Picks a candidate list by the highest slot value (none, below 257, or higher) and by the actor slot (below 3 is the
   low-slot list). */
void textSubmitSlotPairByLevel(s32 actorSlotA, s32 actorSlotB) {
    s32 maxSlotLevel;

    maxSlotLevel = textMaxSlotValueWith(actorSlotA, actorSlotB);
    if (maxSlotLevel <= 0) {
        if (actorSlotA < 3) {
            textSubmitRandomEntry(D_003E0610, actorSlotA, actorSlotB);
        } else {
            textSubmitRandomEntry(D_003E0620, actorSlotA, actorSlotB);
        }
    } else {
        if (maxSlotLevel < 257) {
            if (actorSlotA < 3) {
                textSubmitRandomEntry(D_003E07C0, actorSlotA, actorSlotB);
            } else {
                textSubmitRandomEntry(D_003E07D0, actorSlotA, actorSlotB);
            }
        } else {
            if (actorSlotA < 3) {
                textSubmitRandomEntry(D_003E0970, actorSlotA, actorSlotB);
            } else {
                textSubmitRandomEntry(D_003E0980, actorSlotA, actorSlotB);
            }
        }
    }
}

extern TextStateEntry *gTextLowSlotEntries[];
extern TextStateEntry *gTextHighSlotEntries[];

void textSubmitIndexEntry(s32 actorSlotA, s32 actorSlotB) {
    if (actorSlotA < 3) {
        textSubmitRandomEntry(gTextLowSlotEntries, actorSlotA, actorSlotB);
        return;
    }
    textSubmitRandomEntry(gTextHighSlotEntries, actorSlotA, actorSlotB);
}

extern TextStateEntry *D_003E0D40[];
extern TextStateEntry *D_003E0D50[];

/* Submits an index entry when textCanPlayIndex(actorSlotB) allows it, otherwise a fixed list picked by the actor slot. */
void func_0025CD08(s32 actorSlotA, s32 actorSlotB) {
    if (textCanPlayIndex(actorSlotB)) {
        textSubmitIndexEntry(actorSlotA, actorSlotB);
        return;
    }
    if (actorSlotA < 3) {
        textSubmitRandomEntry(D_003E0D40, actorSlotA, actorSlotB);
    } else {
        textSubmitRandomEntry(D_003E0D50, actorSlotA, actorSlotB);
    }
}

extern TextStateEntry *gTextScoredNoLevelEntries[];
extern TextStateEntry *gTextScoredLowLevelEntries[];
extern TextStateEntry *gTextScoredHighLevelEntries[];

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

extern TextStateEntry *gTextTargetTertiaryEntries[];

void textSubmitTargetTertiary(s32 targetId) {
    textSubmitRandomEntry(gTextTargetTertiaryEntries, targetId, targetId);
}

extern DoublewordAlignedVector4 D_003E0FE0;
extern DoublewordAlignedVector4 D_003E0FF0;
extern Vector4 *D_003E3354[2];
float sinf(float);
void func_00265398(void *table, s32 index, s32 count);
void func_002652B0(void *table, s32 index, s32 count);

/* Builds two camera-table vectors from D_003E0FE0 and D_003E0FF0 (x and y flipped when sinf of the first actor's y is
   negative), sets the camera tables and enters phase 1. */
void func_0025CE58(void) {
    DoublewordAlignedVector4 firstVector;
    DoublewordAlignedVector4 secondVector;
    extern void battleSetPhase();

    firstVector = D_003E0FE0;
    secondVector = D_003E0FF0;
    if (sinf(D_003E3354[0]->y) < 0.0f) {
        secondVector.x = -secondVector.x;
        firstVector.y = -firstVector.y;
    }
    cameraTablesClear();
    func_00265398(&secondVector, 0, 0);
    func_00265398(&secondVector, 0x14, 0);
    func_002652B0(&firstVector, 0, 0);
    func_002652B0(&firstVector, 0x14, 0);
    battleSetPhase(1);
}

extern TextStateEntry *gTextTargetLatchedEntries[];

void textSubmitEntry1100(s32 targetId) {
    s32 lastTargetId;

    lastTargetId = targetId;
    textSubmitRandomEntry(gTextTargetLatchedEntries, lastTargetId, lastTargetId);
    gTextLastTargetId = lastTargetId;
}

extern TextStateEntry *gTextTargetLatchedSecondEntries[];

void textSubmitTargetLatchedSecond(s32 targetId) {
    s32 lastTargetId;

    lastTargetId = targetId;
    textSubmitRandomEntry(gTextTargetLatchedSecondEntries, lastTargetId, lastTargetId);
    gTextLastTargetId = lastTargetId;
}

extern TextStateEntry *gTextLevelTierAZeroEntries[];
extern TextStateEntry *gTextLevelTierALowEntries[];
extern TextStateEntry *gTextLevelTierAHighEntries[];

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

extern TextStateEntry *gTextLevelTierBZeroEntries[];
extern TextStateEntry *gTextLevelTierBLowEntries[];
extern TextStateEntry *gTextLevelTierBHighEntries[];

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

extern TextStateEntry *D_003E1A60[];
extern TextStateEntry *D_003E1A70[];

/* Band entry for one slot: textSubmitBandEntry1500 when textCanPlayIndex allows it, else a list keyed by the last target
   and the slot side. */
void func_0025D0B8(s32 actorSlot) {
    if (textCanPlayIndex(actorSlot)) {
        textSubmitBandEntry1500(actorSlot);
        return;
    }
    if (actorSlot < 3) {
        textSubmitRandomEntry(D_003E1A70, gTextLastTargetId, actorSlot);
    } else {
        textSubmitRandomEntry(D_003E1A60, gTextLastTargetId, actorSlot);
    }
}

extern TextStateEntry *D_003E1C40[];
extern TextStateEntry *D_003E1C50[];

/* Band entry for one slot: textSubmitBandEntry1830 when textCanPlayIndex allows it, else a list keyed by the last target
   and the slot side. */
void func_0025D130(s32 actorSlot) {
    if (textCanPlayIndex(actorSlot)) {
        textSubmitBandEntry1830(actorSlot);
        return;
    }
    if (gTextLastTargetId < 3 ? actorSlot < 3 : actorSlot >= 3) {
        textSubmitRandomEntry(D_003E1C40, gTextLastTargetId, actorSlot);
    } else {
        textSubmitRandomEntry(D_003E1C50, gTextLastTargetId, actorSlot);
    }
}

extern TextStateEntry *gTextTargetIfSetEntries[];

void textSubmitTargetIfSet(s32 targetId) {
    textSubmitRandomEntry(gTextTargetIfSetEntries, targetId, targetId);
}

extern TextStateEntry *gTextNotAllFlaggedEntries[];

void textSubmitNotAllFlagged(void) {
    textSubmitRandomEntry(gTextNotAllFlaggedEntries, 0, 0);
}

extern TextStateEntry *gTextAllFlaggedEntries[];

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

extern Vector4 D_003E1F50[];
extern Vector4 D_003E1F30[];
void func_00265528(f32 scale, s32 mode, s32 index);

/* Scene camera and phase setup: clears the camera tables, applies the two vector sets for slots 0 and 30 (5 items each)
   and the 1.134 scale, then enters phase 1. */
void func_0025D2D8(void) {
    extern void battleSetPhase();

    cameraTablesClear();
    func_00265398(&D_003E1F50[0], 0, 5);
    func_00265398(&D_003E1F50[1], 30, 5);
    func_002652B0(&D_003E1F30[0], 0, 5);
    func_002652B0(&D_003E1F30[1], 30, 5);
    func_00265528(1.134463191f, 0, 5);
    battleSetPhase(1);
}

typedef struct TextScaleEntry {
    f32 scaleA; /* +0x00 */
    f32 scaleB; /* +0x04 */
    f32 unk_08[2];
    f32 scaleC; /* +0x10 */
    f32 scaleD; /* +0x14 */
    f32 scaleE; /* +0x18 */
    f32 unk_1C;
    s32 scaleMode; /* +0x20 */
    s32 unk_24[3];
} TextScaleEntry;

extern TextScaleEntry D_003E01F0;
extern TextScaleEntry D_003E0220;

/* Applies the scale record when gTextScalePending is set: a random sign flips scaleB and scaleC, and the record chosen by
   textMaxActiveSlotValue goes to textSetScaleRecord. */
void textApplyPendingScale(void) {
    u16 randomBit;
    f32 sign;
    s32 activeSlotValue;

    if (gTextScalePending == 0) {
        return;
    }
    randomBit = xglSRand() & 1;
    sign = randomBit ? 1.0f : -1.0f;
    activeSlotValue = textMaxActiveSlotValue();
    /* kept: the call goes through this prototype so the caller's float argument order (a..e, then mode) is emitted */
    if (activeSlotValue <= 0) {
        ((void (*)(f32 a, f32 b, f32 c, f32 d, f32 e, s32 scaleMode)) textSetScaleRecord)(
            D_003E01F0.scaleA, D_003E01F0.scaleB * sign, D_003E01F0.scaleC * sign, D_003E01F0.scaleD,
            D_003E01F0.scaleE, D_003E01F0.scaleMode);
    } else {
        ((void (*)(f32 a, f32 b, f32 c, f32 d, f32 e, s32 scaleMode)) textSetScaleRecord)(
            D_003E0220.scaleA, D_003E0220.scaleB * sign, D_003E0220.scaleC * sign, D_003E0220.scaleD,
            D_003E0220.scaleE, D_003E0220.scaleMode);
    }
}

extern TextStateEntry *gTextScaledEntries[];

void textSubmitEntryWithScale(void) {
    textSubmitRandomEntry(gTextScaledEntries, 0, 0);
    xglScaleSetXY(0.95f, 0.95f);
}


void textSetupSlotBlock(s32 actorSlotA, s32 actorSlotB) {
    extern s32 battleSetPhase(s32);
    extern s32 battleSetActorSlots(s32, s32);
    TextSlotBlock *battleRecord;
    battleRecord = battleGetRecord();
    memset(battleRecord, 0, sizeof(TextSlotBlock));
    battleRecord->unk_20 = 0.6981312f;
    battleRecord->unk_2C = 13.0f;
    battleRecord->actorSlotB = actorSlotB;
    battleRecord->actorSlotA = actorSlotA;
    battleSetActorSlots(actorSlotA, actorSlotB);
    battleSetPhase(3);
}

extern TextStateEntry *D_003E2028[];

void textSubmitEntry2028(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2028, actorSlot, actorSlot);
}

extern TextStateEntry *D_003E2198[];

void textSubmitEntry2198(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2198, actorSlot, actorSlot);
}

const char D_00530840[16] = "\014\300@@ c03";
const char D_00530850[16] = "\014@\300@ c05";
const char D_00530860[16] = "\014@\300@ c04";
const char D_00530870[16] = "\014@\300@ c06";
const char D_00530880[16] = "\014\300@@ c02";
const char D_00530890[16] = "\014\300@@ c23";
const char D_005308A0[16] = "\014@\300@ c25";
const char D_005308B0[16] = "\014@\300@ c26";
const char D_005308C0[16] = "\014\300@@ c27";
const char D_005308D0[16] = "\014\300@@ c33";
const char D_005308E0[16] = "\014@\300@ c36";
const char D_005308F0[16] = "\014@\300@ c35";
const char D_00530900[16] = "\014\300@@ c37";
const char D_00530910[16] = "\014@\300@ c09";
const char D_00530920[16] = "\014\300@@ c072";
const char D_00530930[16] = "\014\300@@ c082";
const char D_00530940[16] = "\014@\300@ c10";
const char D_00530950[16] = "\014@\300@ c11";
const char D_00530960[16] = "\014\300@@ c12";
const char D_00530970[16] = "\014\300@@ c13";
const char D_00530980[8] = "c502";
const char D_00530988[8] = "c512";
const char D_00530990[8] = "c503";
const char D_00530998[8] = "c513";
const char D_005309A0[16] = "\014@\300@ c85";
const char D_005309B0[16] = "\014@\300@ c852";
const char D_005309C0[16] = "\014\300@@ c86";
const char D_005309D0[16] = "\014\300@@ c862";
const char D_005309E0[8] = "c872";
const char D_005309E8[8] = "c882";
const char D_005309F0[8] = "r2000";
char D_005505E0[8] = "c15";
char D_005505E8[8] = "c16";
char D_005505F0[8] = "c17";
char D_005505F8[8] = "c20";
char D_00550600[8] = "c07";
char D_00550608[8] = "c08";
char D_00550610[8] = "c60";
char D_00550618[8] = "c61";
char D_00550620[8] = "c62";
char D_00550628[8] = "c70";
char D_00550630[8] = "c71";
char D_00550638[8] = "c40";
char D_00550640[8] = "c41";
char D_00550648[8] = "c45";
char D_00550650[8] = "c46";
char D_00550658[8] = "c50";
char D_00550660[8] = "c51";
char D_00550668[8] = "c52";
char D_00550670[8] = "c53";
char D_00550678[8] = "c55";
char D_00550680[8] = "c56";
char D_00550688[8] = "c58";
char D_00550690[8] = "c59";
char D_00550698[8] = "c87";
char D_005506A0[8] = "c88";
char D_005506A8[8] = "c90";
char D_005506B0[8] = "c91";
char D_005506B8[8] = "c93";
char D_005506C0[8] = "c95";
char D_005506C8[8] = "c96";
char D_005506D0[8] = "r70";
char D_005506D8[8] = "r80";
char D_005506E0[8] = "r81";
char D_005506E8[8] = "r82";
char D_005506F0[8] = "r02";
char D_005506F8[8] = "r03";
char D_00550700[8] = "r04";

void *(*gTextCmdHandlers[8])(f32 *cmdArgs) = {
    0,
    textCmdWait,
    textCmdSetFloat,
    textCmdSetVec4,
    textCmdSetRect,
    textCmdSetKinds,
    textCmdSetScaledValue,
    0
};
unsigned char gBattleChatterControlStorage[0x50] = { 0 };
TextScaleEntry D_003E01F0 = { -0.241727918f, -0.410152078f, { 0.0f, 1.0f }, -2.0999999f, 2.42000008f, 6.88000011f, 1.0f, 0, { 0, 0, 0 } };
TextScaleEntry D_003E0220 = { -0.0317649692f, -0.412246466f, { 0.0f, 1.0f }, -2.75999999f, 2.13000011f, 7.80000019f, 1.0f, 0, { 0, 0, 0 } };
DoublewordAlignedVector4 D_003E0FE0 = { -0.248709232f, 0.426732689f, 0.0f, 1.0f };
DoublewordAlignedVector4 D_003E0FF0 = { 2.99000001f, 2.29999995f, 8.56000042f, 1.0f };
Vector4 D_003E1F30[2] = { { 0.270525843f, -3.14159036f, 0.0f, 1.0f }, { 0.88051796f, -3.14159036f, 0.0f, 1.0f } };
Vector4 D_003E1F50[2] = { { 0.0f, 1.0f, 1.70000005f, 1.0f }, { 0.0f, 1.0f, 1.70000005f, 1.0f } };

f32 D_003E0250[28] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -0.400000006f, 0.0f, 4.0f,
    0.0f, 0.800000012f, 0.400000006f, -1.04999995f,
    1.0f, 12.0f, 5.0f, 4.0f,
    10.0f, 3.0f, 0.0f, 0.150000006f,
    -0.5f, -0.150000006f, 4.0f, 0.0f,
    2.04999995f, 1.25f, -2.5f, 0.0f
};
f32 D_003E02C8[28] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -0.300000012f, 0.0f, 4.0f,
    0.0f, 0.75f, -0.550000012f, -1.5f,
    1.0f, 12.0f, 5.0f, 4.0f,
    10.0f, 3.0f, 0.0f, 0.0f,
    -0.400000006f, 0.0f, 4.0f, 0.0f,
    1.5f, -1.20000005f, -3.0f, 0.0f
};
f32 D_003E0340[28] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.150000006f, -0.349999994f, 0.0f, 4.0f,
    0.0f, 1.10000002f, -0.850000024f, 1.10000002f,
    1.0f, 12.0f, 5.0f, 4.0f,
    10.0f, 3.0f, 0.0f, 0.25f,
    -0.400000006f, 0.0f, 4.0f, 0.0f,
    2.5999999f, -1.29999995f, 2.6500001f, 0.0f
};
f32 D_003E03C8[14] = {
    2.0f, 2.0f, 3.0f, 0.0f,
    -0.200000003f, -0.300000012f, 0.0f, 4.0f,
    0.0f, 0.600000024f, -0.400000006f, -1.10000002f,
    0.0f, 0.0f
};
f32 D_003E0410[26] = {
    2.0f, 133120.0f, 3.0f, 0.600000024f,
    0.0f, -0.699999988f, 0.5f, 4.0f,
    0.800000012f, 0.600000024f, 0.400000006f, -2.0f,
    5.0f, 3.0f, 15.0f, 3.0f,
    1.0f, 0.300000012f, -0.449999988f, 0.5f,
    4.0f, 0.800000012f, 1.5f, 0.400000006f,
    -5.0f, 0.0f
};
f32 D_003E0480[26] = {
    2.0f, 135296.0f, 3.0f, 0.100000001f,
    0.0f, -0.600000024f, 0.0f, 4.0f,
    0.0f, 1.0f, -1.35000002f, 1.39999998f,
    5.0f, 3.0f, 20.0f, 3.0f,
    0.899999976f, 0.0f, -0.300000012f, 0.0f,
    4.0f, 0.0f, 1.0f, -1.35000002f,
    1.0f, 0.0f
};
f32 D_003E04F0[26] = {
    2.0f, 135296.0f, 3.0f, 0.400000006f,
    0.0f, -0.25f, 0.0f, 4.0f,
    0.0f, 2.79999995f, -0.600000024f, 6.0f,
    5.0f, 1.0f, 10.0f, 3.0f,
    0.899999976f, 0.0f, -0.550000012f, 0.0f,
    4.0f, 0.0f, 1.20000005f, -0.600000024f,
    1.10000002f, 0.0f
};
f32 D_003E0560[26] = {
    2.0f, 135296.0f, 3.0f, 0.5f,
    0.0f, 0.0f, 0.0f, 4.0f,
    0.0f, 2.5f, 3.0f, 5.5f,
    5.0f, 3.0f, 10.0f, 3.0f,
    0.899999976f, 0.0f, -0.699999988f, 0.0f,
    4.0f, 0.0f, 1.29999995f, 0.949999988f,
    1.54999995f, 0.0f
};
f32 D_003E05D0[14] = {
    2.0f, 133120.0f, 3.0f, 1.0f,
    0.0f, -0.400000006f, 1.0f, 4.0f,
    1.0f, 1.5f, -0.400000006f, -3.29999995f,
    0.0f, 0.0f
};
f32 D_003E0630[26] = {
    2.0f, 133120.0f, 3.0f, 0.0f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    1.0f, 0.600000024f, 0.400000006f, -2.0f,
    5.0f, 4.0f, 20.0f, 3.0f,
    0.5f, 0.0f, -0.699999988f, 0.0f,
    4.0f, 1.0f, 2.5f, -0.5f,
    -6.0f, 0.0f
};
f32 D_003E06A0[26] = {
    2.0f, 131200.0f, 3.0f, 0.150000006f,
    0.0f, -0.300000012f, 0.0f, 4.0f,
    0.0f, 1.60000002f, -1.35000002f, 1.39999998f,
    5.0f, 3.0f, 15.0f, 3.0f,
    0.699999988f, 0.0f, 0.200000003f, 0.0f,
    4.0f, 0.0f, 1.60000002f, -1.35000002f,
    1.0f, 0.0f
};
f32 D_003E0710[26] = {
    2.0f, 131200.0f, 3.0f, 0.449999988f,
    0.0f, 0.5f, 0.0f, 4.0f,
    0.0f, 2.20000005f, -0.899999976f, 6.0f,
    5.0f, 3.0f, 15.0f, 3.0f,
    0.899999976f, 0.0f, 0.600000024f, 0.0f,
    4.0f, 0.0f, 1.0f, -0.899999976f,
    2.0f, 0.0f
};
f32 D_003E0780[14] = {
    2.0f, 133120.0f, 3.0f, 0.600000024f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    1.0f, 1.39999998f, -1.20000005f, -4.30000019f,
    0.0f, 0.0f
};
f32 D_003E07E0[26] = {
    2.0f, 133120.0f, 3.0f, 0.5f,
    0.0f, -1.20000005f, -0.699999988f, 4.0f,
    1.0f, 2.0f, 0.0f, -3.5f,
    5.0f, 3.0f, 20.0f, 3.0f,
    0.5f, 0.0f, -1.20000005f, 0.0f,
    4.0f, 1.0f, 3.0f, -0.5f,
    -6.0f, 0.0f
};
f32 D_003E0850[26] = {
    2.0f, 131200.0f, 3.0f, 0.200000003f,
    0.0f, -0.100000001f, 0.0f, 4.0f,
    0.0f, 1.29999995f, -1.35000002f, 1.39999998f,
    5.0f, 3.0f, 20.0f, 3.0f,
    0.800000012f, 0.0f, 1.20000005f, 0.0f,
    4.0f, 0.0f, 1.5f, -1.35000002f,
    1.20000005f, 0.0f
};
f32 D_003E08C0[26] = {
    2.0f, 131200.0f, 3.0f, 0.5f,
    0.0f, 1.20000005f, 0.0f, 4.0f,
    0.0f, 1.5f, -1.20000005f, 5.5f,
    5.0f, 3.0f, 10.0f, 3.0f,
    0.899999976f, 0.0f, 1.0f, 0.0f,
    4.0f, 0.0f, 0.800000012f, -1.20000005f,
    1.79999995f, 0.0f
};
f32 D_003E0930[14] = {
    2.0f, 133120.0f, 3.0f, 0.5f,
    0.0f, -1.60000002f, 0.0f, 4.0f,
    1.0f, 1.0f, -0.5f, -4.69999981f,
    0.0f, 0.0f
};
f32 D_003E0990[26] = {
    2.0f, 168450.0f, 3.0f, 0.349999994f,
    0.0f, -0.400000006f, 0.0f, 4.0f,
    0.0f, -0.600000024f, -0.349999994f, 2.79999995f,
    5.0f, 1.0f, 60.0f, 3.0f,
    0.349999994f, 0.0f, -0.400000006f, 0.0f,
    4.0f, 0.0f, 1.10000002f, -0.349999994f,
    2.79999995f, 0.0f
};
f32 D_003E0A00[26] = {
    2.0f, 168450.0f, 3.0f, 0.300000012f,
    0.0f, -0.400000006f, 0.0f, 4.0f,
    0.0f, 2.0f, 1.79999995f, 5.69999981f,
    5.0f, 4.0f, 60.0f, 3.0f,
    0.300000012f, 0.0f, -0.400000006f, 0.0f,
    4.0f, 0.0f, 1.0f, -0.400000006f,
    2.5f, 0.0f
};
f32 D_003E0A70[14] = {
    2.0f, 167936.0f, 3.0f, 0.0500000007f,
    0.200000003f, -0.300000012f, 0.0500000007f, 4.0f,
    0.0f, 0.699999988f, -0.400000006f, 1.89999998f,
    0.0f, 0.0f
};
f32 D_003E0AC0[26] = {
    2.0f, 168450.0f, 3.0f, 0.349999994f,
    0.0f, -0.5f, 0.0f, 4.0f,
    0.0f, -0.400000006f, -0.300000012f, 3.79999995f,
    5.0f, 1.0f, 60.0f, 3.0f,
    0.349999994f, 0.0f, -0.5f, 0.0f,
    4.0f, 0.0f, 1.5f, -0.300000012f,
    3.79999995f, 0.0f
};
f32 D_003E0B30[26] = {
    2.0f, 168450.0f, 3.0f, 0.300000012f,
    0.0f, -0.400000006f, 0.0f, 4.0f,
    0.0f, 2.0f, 1.60000002f, 5.69999981f,
    5.0f, 4.0f, 60.0f, 3.0f,
    0.300000012f, 0.0f, -0.5f, 0.0f,
    4.0f, 0.0f, 1.29999995f, -0.200000003f,
    3.5f, 0.0f
};
f32 D_003E0BB0[14] = {
    2.0f, 17408.0f, 3.0f, 0.600000024f,
    0.0f, -0.75f, 0.0f, 4.0f,
    0.0f, 1.20000005f, -0.5f, 4.80000019f,
    0.0f, 0.0f
};
f32 D_003E0BF0[26] = {
    2.0f, 1536.0f, 3.0f, 0.600000024f,
    0.0f, -1.0f, 0.0f, 4.0f,
    0.0f, -2.0f, -0.600000024f, 3.9000001f,
    5.0f, 1.0f, 90.0f, 3.0f,
    0.800000012f, 0.0f, -1.29999995f, 0.0f,
    4.0f, 0.0f, 2.20000005f, 0.899999976f,
    6.0f, 0.0f
};
f32 D_003E0C60[26] = {
    2.0f, 1024.0f, 3.0f, 0.0f,
    0.0f, -0.400000006f, -0.5f, 4.0f,
    0.0f, 3.0f, 1.0f, -5.5f,
    5.0f, 4.0f, 35.0f, 3.0f,
    0.699999988f, 0.0f, -0.5f, 0.0f,
    4.0f, 1.0f, 4.80000019f, 1.79999995f,
    -6.0f, 0.0f
};
f32 D_003E0CD0[26] = {
    2.0f, 17920.0f, 3.0f, 0.600000024f,
    0.0f, -1.20000005f, 0.0f, 4.0f,
    1.0f, -1.20000005f, 0.200000003f, -7.0f,
    5.0f, 1.0f, 70.0f, 3.0f,
    0.600000024f, 0.0f, -0.899999976f, 0.0f,
    4.0f, 1.0f, 1.79999995f, 1.0f,
    -6.0f, 0.0f
};
f32 D_003E0D60[26] = {
    2.0f, 135552.0f, 3.0f, 0.100000001f,
    0.0f, -0.649999976f, 0.0f, 4.0f,
    0.0f, 1.0f, -1.0f, 1.60000002f,
    5.0f, 3.0f, 15.0f, 3.0f,
    0.899999976f, 0.0f, -0.400000006f, 0.0f,
    4.0f, 0.0f, 1.0f, -1.0f,
    1.20000005f, 0.0f
};
f32 D_003E0DD0[26] = {
    2.0f, 135552.0f, 3.0f, 0.5f,
    0.0f, 0.349999994f, 0.0f, 4.0f,
    0.0f, 2.70000005f, -0.800000012f, 4.80000019f,
    5.0f, 3.0f, 10.0f, 3.0f,
    0.899999976f, 0.0f, 0.300000012f, 0.0f,
    4.0f, 0.0f, 1.54999995f, -0.800000012f,
    1.0f, 0.0f
};
f32 D_003E0E40[26] = {
    2.0f, 135552.0f, 3.0f, 0.200000003f,
    0.0f, -0.100000001f, 0.0f, 4.0f,
    0.0f, 1.29999995f, -1.35000002f, 1.39999998f,
    5.0f, 3.0f, 15.0f, 3.0f,
    0.800000012f, 0.0f, 1.20000005f, 0.0f,
    4.0f, 0.0f, 1.5f, -1.35000002f,
    1.20000005f, 0.0f
};
f32 D_003E0EC8[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.5f, 0.0f, 4.0f,
    0.0f, 0.150000006f, 0.0f, -2.0f,
    6.0f, 0.00872663967f, 5.0f, 4.0f,
    60.0f, 3.0f, 0.0f, 0.0f,
    -0.5f, 0.0f, 4.0f, 0.0f,
    0.25f, -0.5f, -2.29999995f, 6.0f,
    0.0349065587f, 0.0f
};
f32 D_003E0F48[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    -0.0500000007f, -0.349999994f, 0.0f, 4.0f,
    0.0f, 0.699999988f, -0.5f, -2.79999995f,
    6.0f, 0.0f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, -0.0500000007f,
    -0.300000012f, 0.0f, 4.0f, 0.0f,
    0.400000006f, -0.400000006f, -1.70000005f, 6.0f,
    0.0f, 0.0f
};
f32 D_003E1000[30] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -0.300000012f, 0.0f, 4.0f,
    0.0f, 0.5f, -0.300000012f, -2.25f,
    6.0f, 0.0872664005f, 5.0f, 8.0f,
    40.0f, 3.0f, 0.0f, 0.0f,
    -0.300000012f, 0.0f, 4.0f, 0.0f,
    0.150000006f, -0.600000024f, -1.54999995f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E1080[30] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -0.300000012f, 0.0f, 4.0f,
    0.0f, 1.0f, -0.300000012f, -2.25f,
    6.0f, 0.0872664005f, 5.0f, 8.0f,
    40.0f, 3.0f, 0.0f, 0.0f,
    -0.300000012f, -0.200000003f, 4.0f, 0.0f,
    0.850000024f, -0.300000012f, -1.35000002f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E1110[26] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    -0.100000001f, -0.300000012f, -0.200000003f, 4.0f,
    0.0f, 0.800000012f, -0.5f, -1.5f,
    5.0f, 4.0f, 30.0f, 3.0f,
    0.0f, -0.100000001f, -0.400000006f, -0.200000003f,
    4.0f, 0.0f, 0.800000012f, -0.200000003f,
    -1.5f, 0.0f
};
f32 D_003E1180[26] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    -0.100000001f, -0.5f, 0.0f, 4.0f,
    0.0f, 0.400000006f, -0.5f, -1.79999995f,
    5.0f, 4.0f, 30.0f, 3.0f,
    0.0f, 0.0f, -0.300000012f, 0.0f,
    4.0f, 0.0f, 0.100000001f, -0.699999988f,
    -1.60000002f, 0.0f
};
f32 D_003E1200[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.699999988f, 0.0f, 4.0f,
    0.0f, -1.70000005f, -1.0f, -3.79999995f,
    6.0f, 0.0f, 5.0f, 4.0f,
    80.0f, 3.0f, 0.0f, 0.0f,
    -0.699999988f, 0.0f, 4.0f, 0.0f,
    1.10000002f, -0.5f, -5.0f, 6.0f,
    0.139626235f, 0.0f
};
f32 D_003E1280[30] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    0.0f, -1.0f, 0.5f, -4.5f,
    6.0f, 0.0f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -0.800000012f, 0.0f, 4.0f, 0.0f,
    2.0f, -1.0f, -5.0f, 6.0f,
    -0.139626235f, 0.0f
};
f32 D_003E1300[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    0.0f, 1.0f, -1.20000005f, -3.5f,
    6.0f, 0.0f, 5.0f, 4.0f,
    60.0f, 3.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 4.0f, 0.0f,
    2.5f, -4.0f, -10.1999998f, 6.0f,
    0.139626235f, 0.0f
};
f32 D_003E1380[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -1.20000005f, 0.0f, 4.0f,
    0.0f, -6.0f, 2.0f, -5.0f,
    6.0f, 0.0174532793f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -1.20000005f, 0.0f, 4.0f, 0.0f,
    3.5f, -1.79999995f, -7.0f, 6.0f,
    -0.174532801f, 0.0f
};
f32 D_003E1400[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    0.0f, 1.0f, -1.20000005f, -3.5f,
    6.0f, 0.0f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, -0.5f,
    -1.5f, 0.0f, 4.0f, 0.0f,
    1.5f, -3.0f, -10.1999998f, 6.0f,
    0.122172952f, 0.0f
};
f32 D_003E1480[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -1.70000005f, 0.0f, 4.0f,
    0.0f, -4.0f, -0.699999988f, -8.5f,
    6.0f, 0.0174532793f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, -0.5f,
    -1.89999998f, 0.0f, 4.0f, 0.0f,
    2.0f, -3.20000005f, -9.5f, 6.0f,
    -0.122172952f, 0.0f
};
f32 D_003E1530[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.600000024f, 0.0f, 4.0f,
    0.0f, 0.150000006f, 0.5f, -3.20000005f,
    6.0f, 0.052359838f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -0.600000024f, 0.0f, 4.0f, 0.0f,
    0.25f, -1.20000005f, -4.5f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E15B0[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.600000024f, 0.0f, 4.0f,
    0.0f, -1.5f, 0.5f, -4.0f,
    6.0f, 0.0f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -0.600000024f, 0.0f, 4.0f, 0.0f,
    2.0f, -1.20000005f, -4.30000019f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E1630[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -1.20000005f, 0.0f, 4.0f,
    0.0f, 0.300000012f, 1.20000005f, -4.5f,
    6.0f, 0.0872664005f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -1.20000005f, 0.0f, 4.0f, 0.0f,
    0.600000024f, -2.5999999f, -8.10000038f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E16B0[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -1.0f, 0.0f, 4.0f,
    0.0f, -6.5f, -0.5f, -5.5f,
    6.0f, 0.0f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -1.5f, 0.0f, 4.0f, 0.0f,
    3.79999995f, -3.0f, -9.0f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E1730[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -1.60000002f, 0.0f, 4.0f,
    0.0f, 0.300000012f, 1.20000005f, -4.5f,
    6.0f, 0.0872664005f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -1.79999995f, 0.0f, 4.0f, 0.0f,
    0.600000024f, -3.20000005f, -8.5f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E17B0[30] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -1.0f, 0.0f, 4.0f,
    0.0f, -6.5f, -0.5f, -5.5f,
    6.0f, 0.0f, 5.0f, 4.0f,
    50.0f, 3.0f, 0.0f, 0.0f,
    -1.5f, 0.0f, 4.0f, 0.0f,
    3.79999995f, -3.0f, -9.0f, 6.0f,
    0.0872664005f, 0.0f
};
f32 D_003E1860[30] = {
    2.0f, 17920.0f, 3.0f, 1.0f,
    0.0f, -1.0f, 0.0f, 4.0f,
    0.0f, -3.0f, 0.200000003f, 7.0f,
    6.0f, 0.0f, 5.0f, 4.0f,
    90.0f, 3.0f, 1.0f, 0.0f,
    -0.699999988f, 0.0f, 4.0f, 0.0f,
    2.5f, 1.0f, 10.0f, 6.0f,
    -0.139626235f, 0.0f
};
f32 D_003E18E0[30] = {
    2.0f, 17920.0f, 3.0f, 1.0f,
    0.0f, -1.5f, 0.0f, 4.0f,
    1.0f, 1.5f, 0.0f, 10.0f,
    6.0f, 0.0f, 5.0f, 4.0f,
    90.0f, 3.0f, 1.0f, 0.0f,
    -0.800000012f, 0.0f, 4.0f, 0.0f,
    2.70000005f, 1.0f, 10.0f, 6.0f,
    -0.139626235f, 0.0f
};
f32 D_003E1960[30] = {
    2.0f, 1536.0f, 3.0f, 1.0f,
    0.0f, -0.200000003f, 0.0f, 4.0f,
    1.0f, -6.0f, 1.20000005f, -6.5f,
    6.0f, 0.0f, 5.0f, 4.0f,
    80.0f, 3.0f, 1.0f, 0.0f,
    -0.200000003f, 0.0f, 4.0f, 1.0f,
    1.5f, 0.0f, -10.0f, 6.0f,
    0.139626235f, 0.0f
};
f32 D_003E19E0[30] = {
    2.0f, 1536.0f, 3.0f, 1.0f,
    0.0f, -0.200000003f, 0.0f, 4.0f,
    1.0f, 1.0f, 0.800000012f, -5.0f,
    6.0f, 0.0f, 5.0f, 4.0f,
    80.0f, 3.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 4.0f, 1.0f,
    1.0f, 0.300000012f, -10.0f, 6.0f,
    0.139626235f, 0.0f
};
f32 D_003E1A80[26] = {
    2.0f, 17920.0f, 3.0f, 0.0f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    0.0f, 1.5f, -0.800000012f, -3.0f,
    5.0f, 4.0f, 110.0f, 3.0f,
    0.0f, 0.0f, -0.699999988f, 0.0f,
    4.0f, 0.0f, 1.5f, 0.300000012f,
    -3.0f, 0.0f
};
f32 D_003E1AF0[26] = {
    2.0f, 17920.0f, 3.0f, 0.0f,
    0.0f, -0.699999988f, 0.0f, 4.0f,
    0.0f, -5.5f, 0.0f, -5.5f,
    5.0f, 4.0f, 110.0f, 3.0f,
    0.0f, 0.0f, -0.800000012f, 0.0f,
    4.0f, 0.0f, 4.0f, 2.0f,
    -6.0f, 0.0f
};
f32 D_003E1B60[26] = {
    2.0f, 17920.0f, 3.0f, 1.0f,
    0.0f, -0.800000012f, 0.0f, 4.0f,
    1.0f, 1.5f, -0.800000012f, 3.0f,
    5.0f, 4.0f, 110.0f, 3.0f,
    1.0f, 0.0f, -0.699999988f, 0.0f,
    4.0f, 1.0f, 1.5f, 0.300000012f,
    3.0f, 0.0f
};
f32 D_003E1BD0[26] = {
    2.0f, 17920.0f, 3.0f, 1.0f,
    0.0f, -0.699999988f, 0.0f, 4.0f,
    1.0f, -5.5f, 0.0f, 5.5f,
    5.0f, 4.0f, 110.0f, 3.0f,
    1.0f, 0.0f, -0.800000012f, 0.0f,
    4.0f, 1.0f, 4.0f, 2.0f,
    6.0f, 0.0f
};
f32 D_003E1C60[26] = {
    2.0f, 512.0f, 3.0f, 0.0f,
    0.0f, -0.5f, -0.300000012f, 4.0f,
    0.0f, 1.5f, 3.0f, -5.5f,
    5.0f, 4.0f, 50.0f, 3.0f,
    0.0f, 0.0f, -0.5f, -0.300000012f,
    4.0f, 0.0f, 0.899999976f, -0.899999976f,
    -1.5f, 0.0f
};
f32 D_003E1CD0[26] = {
    2.0f, 1024.0f, 3.0f, 0.0f,
    0.0f, -0.5f, -0.0500000007f, 4.0f,
    0.0f, 2.5f, 0.5f, -6.19999981f,
    5.0f, 4.0f, 45.0f, 3.0f,
    0.0f, 0.0f, -0.5f, -0.0500000007f,
    4.0f, 0.0f, -4.69999981f, -1.10000002f,
    -1.85000002f, 0.0f
};
f32 D_003E1D40[54] = {
    2.0f, 768.0f, 3.0f, 0.0f,
    0.0f, -0.25f, 0.0f, 4.0f,
    0.0f, 3.5f, -1.5f, -2.70000005f,
    5.0f, 4.0f, 35.0f, 3.0f,
    0.0f, 0.150000006f, -0.25f, 0.0f,
    4.0f, 0.0f, -0.800000012f, -0.5f,
    -2.70000005f, 1.0f, 25.0f, 2.0f,
    768.0f, 3.0f, 0.0f, 0.0500000007f,
    -0.300000012f, 0.0f, 4.0f, 0.0f,
    -1.0f, -0.300000012f, -2.8499999f, 5.0f,
    7.0f, 30.0f, 3.0f, 0.0f,
    0.0500000007f, -0.300000012f, 0.0f, 4.0f,
    0.0f, -0.100000001f, -0.550000012f, -1.20000005f,
    0.0f, 0.0f
};
f32 D_003E1E30[28] = {
    1.0f, 40.0f, 2.0f, 1792.0f,
    3.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 4.0f, 0.0f, -0.200000003f,
    3.0f, 0.200000003f, 5.0f, 8.0f,
    170.0f, 3.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 4.0f, 0.0f,
    0.300000012f, 12.0f, -0.200000003f, 0.0f
};
f32 D_003E1EB0[28] = {
    1.0f, 30.0f, 2.0f, 1536.0f,
    3.0f, 0.0f, 0.0f, -1.0f,
    0.0f, 4.0f, 0.0f, 4.0f,
    10.0f, -16.0f, 5.0f, 1.0f,
    170.0f, 3.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 4.0f, 0.0f,
    -4.0f, 10.0f, -16.0f, 0.0f
};
f32 D_003E1F70[26] = {
    2.0f, 1552.0f, 3.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 4.0f,
    0.0f, 0.0f, 150.0f, 110.0f,
    5.0f, 4.0f, 55.0f, 3.0f,
    0.0f, 0.0f, -1.5f, 0.0f,
    4.0f, 0.0f, 0.0f, -1.0f,
    55.0f, 0.0f
};
f32 D_003E1FE8[14] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -2.29999995f, 0.0f, 4.0f,
    0.0f, 3.0f, -4.5f, -7.0f,
    0.0f, 0.0f
};
f32 D_003E2030[28] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 7.5f, 3.20000005f, -5.5f,
    1.0f, 14.0f, 5.0f, 4.0f,
    11.0f, 3.0f, 0.0f, 0.0f,
    -3.5f, 0.0f, 4.0f, 0.0f,
    19.0f, 12.5f, -14.0f, 0.0f
};
f32 D_003E20A8[28] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 6.0f, -4.0f, -11.0f,
    1.0f, 14.0f, 5.0f, 4.0f,
    11.0f, 3.0f, 0.0f, 0.0f,
    -3.0f, 0.0f, 4.0f, 0.0f,
    12.0f, -8.0f, -22.0f, 0.0f
};
f32 D_003E2120[28] = {
    2.0f, 0.0f, 3.0f, 0.0f,
    0.0f, -2.0f, 0.0f, 4.0f,
    0.0f, 8.30000019f, -8.0f, -19.0f,
    1.0f, 14.0f, 5.0f, 4.0f,
    11.0f, 3.0f, 0.0f, 0.0f,
    -2.5f, 0.0f, 4.0f, 0.0f,
    4.0f, -7.0f, -9.0f, 0.0f
};
f32 D_003E21A8[26] = {
    2.0f, 98313.0f, 3.0f, 0.5f,
    0.0f, -1.29999995f, 0.0f, 4.0f,
    0.5f, 3.0f, -0.5f, 10.0f,
    5.0f, 1.0f, 10.0f, 3.0f,
    0.5f, 0.0f, -1.29999995f, 0.0f,
    4.0f, 0.5f, 3.0f, -0.5f,
    10.0f, 0.0f
};
f32 D_003E2218[26] = {
    2.0f, 102409.0f, 3.0f, 0.400000006f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.400000006f, 7.0f, 3.0f, 7.5f,
    5.0f, 1.0f, 10.0f, 3.0f,
    0.400000006f, 0.0f, -3.0f, 0.0f,
    4.0f, 0.400000006f, 7.0f, 3.0f,
    7.5f, 0.0f
};
f32 D_003E2288[26] = {
    2.0f, 102409.0f, 3.0f, 0.5f,
    0.0f, 2.0f, 0.0f, 4.0f,
    0.5f, 2.5f, 1.5f, 2.0f,
    5.0f, 1.0f, 10.0f, 3.0f,
    0.5f, 0.0f, 2.0f, 0.0f,
    4.0f, 0.5f, 2.5f, 1.5f,
    2.0f, 0.0f
};
f32 D_003E22F8[14] = {
    2.0f, 37123.0f, 3.0f, 0.349999994f,
    0.0f, -3.0f, 0.0f, 4.0f,
    0.0f, 3.5f, 2.0f, 20.0f,
    0.0f, 0.0f
};

TextStateEntry D_003E02C0 = { D_003E0250, D_005505E0 };
TextStateEntry D_003E0338 = { D_003E02C8, D_005505E8 };
TextStateEntry D_003E03B0 = { D_003E0340, D_005505F0 };
TextStateEntry D_003E0400 = { D_003E03C8, D_005505F8 };
TextStateEntry D_003E0478 = { D_003E0410, D_00530840 };
TextStateEntry D_003E04E8 = { D_003E0480, D_00530850 };
TextStateEntry D_003E0558 = { D_003E04F0, D_00530860 };
TextStateEntry D_003E05C8 = { D_003E0560, D_00530870 };
TextStateEntry D_003E0608 = { D_003E05D0, D_00530880 };
TextStateEntry D_003E0698 = { D_003E0630, D_00530890 };
TextStateEntry D_003E0708 = { D_003E06A0, D_005308A0 };
TextStateEntry D_003E0778 = { D_003E0710, D_005308B0 };
TextStateEntry D_003E07B8 = { D_003E0780, D_005308C0 };
TextStateEntry D_003E0848 = { D_003E07E0, D_005308D0 };
TextStateEntry D_003E08B8 = { D_003E0850, D_005308E0 };
TextStateEntry D_003E0928 = { D_003E08C0, D_005308F0 };
TextStateEntry D_003E0968 = { D_003E0930, D_00530900 };
TextStateEntry D_003E09F8 = { D_003E0990, D_00550600 };
TextStateEntry D_003E0A68 = { D_003E0A00, D_00550608 };
TextStateEntry D_003E0AA8 = { D_003E0A70, D_00530910 };
TextStateEntry D_003E0B28 = { D_003E0AC0, D_00530920 };
TextStateEntry D_003E0B98 = { D_003E0B30, D_00530930 };
TextStateEntry D_003E0BE8 = { D_003E0BB0, D_00530940 };
TextStateEntry D_003E0C58 = { D_003E0BF0, D_00530950 };
TextStateEntry D_003E0CC8 = { D_003E0C60, D_00530960 };
TextStateEntry D_003E0D38 = { D_003E0CD0, D_00530970 };
TextStateEntry D_003E0DC8 = { D_003E0D60, D_00550610 };
TextStateEntry D_003E0E38 = { D_003E0DD0, D_00550618 };
TextStateEntry D_003E0EA8 = { D_003E0E40, D_00550620 };
TextStateEntry D_003E0F40 = { D_003E0EC8, D_00550628 };
TextStateEntry D_003E0FC0 = { D_003E0F48, D_00550630 };
TextStateEntry D_003E1078 = { D_003E1000, D_00550638 };
TextStateEntry D_003E10F8 = { D_003E1080, D_00550640 };
TextStateEntry D_003E1178 = { D_003E1110, D_00550648 };
TextStateEntry D_003E11E8 = { D_003E1180, D_00550650 };
TextStateEntry D_003E1278 = { D_003E1200, D_00550658 };
TextStateEntry D_003E12F8 = { D_003E1280, D_00550660 };
TextStateEntry D_003E1378 = { D_003E1300, D_00530980 };
TextStateEntry D_003E13F8 = { D_003E1380, D_00530988 };
TextStateEntry D_003E1478 = { D_003E1400, D_00530990 };
TextStateEntry D_003E14F8 = { D_003E1480, D_00530998 };
TextStateEntry D_003E15A8 = { D_003E1530, D_00550668 };
TextStateEntry D_003E1628 = { D_003E15B0, D_00550670 };
TextStateEntry D_003E16A8 = { D_003E1630, D_00550678 };
TextStateEntry D_003E1728 = { D_003E16B0, D_00550680 };
TextStateEntry D_003E17A8 = { D_003E1730, D_00550688 };
TextStateEntry D_003E1828 = { D_003E17B0, D_00550690 };
TextStateEntry D_003E18D8 = { D_003E1860, D_005309A0 };
TextStateEntry D_003E1958 = { D_003E18E0, D_005309B0 };
TextStateEntry D_003E19D8 = { D_003E1960, D_005309C0 };
TextStateEntry D_003E1A58 = { D_003E19E0, D_005309D0 };
TextStateEntry D_003E1AE8 = { D_003E1A80, D_00550698 };
TextStateEntry D_003E1B58 = { D_003E1AF0, D_005506A0 };
TextStateEntry D_003E1BC8 = { D_003E1B60, D_005309E0 };
TextStateEntry D_003E1C38 = { D_003E1BD0, D_005309E8 };
TextStateEntry D_003E1CC8 = { D_003E1C60, D_005506A8 };
TextStateEntry D_003E1D38 = { D_003E1CD0, D_005506B0 };
TextStateEntry D_003E1E18 = { D_003E1D40, D_005506B8 };
TextStateEntry D_003E1EA0 = { D_003E1E30, D_005506C0 };
TextStateEntry D_003E1F20 = { D_003E1EB0, D_005506C8 };
TextStateEntry D_003E1FD8 = { D_003E1F70, D_005309F0 };
TextStateEntry D_003E2020 = { D_003E1FE8, D_005506D0 };
TextStateEntry D_003E20A0 = { D_003E2030, D_005506D8 };
TextStateEntry D_003E2118 = { D_003E20A8, D_005506E0 };
TextStateEntry D_003E2190 = { D_003E2120, D_005506E8 };
TextStateEntry D_003E2210 = { D_003E21A8, D_005506F0 };
TextStateEntry D_003E2280 = { D_003E2218, D_005506F8 };
TextStateEntry D_003E22F0 = { D_003E2288, D_00550700 };

TextStateEntry *gTextTargetSecondaryEntries[4] = { &D_003E02C0, &D_003E0338, &D_003E03B0, 0 };
TextStateEntry *gTextTargetPrimaryEntries[2] = { &D_003E0400, 0 };
TextStateEntry *D_003E0610[4] = { &D_003E0558, &D_003E05C8, &D_003E04E8, 0 };
TextStateEntry *D_003E0620[4] = { &D_003E0478, &D_003E0608, 0, 0 };
TextStateEntry *D_003E07C0[4] = { &D_003E0778, &D_003E0708, 0, 0 };
TextStateEntry *D_003E07D0[4] = { &D_003E0698, &D_003E07B8, 0, 0 };
TextStateEntry *D_003E0970[4] = { &D_003E0928, &D_003E08B8, 0, 0 };
TextStateEntry *D_003E0980[4] = { &D_003E0848, &D_003E0968, 0, 0 };
TextStateEntry *gTextLowSlotEntries[4] = { &D_003E09F8, &D_003E0A68, &D_003E0AA8, 0 };
TextStateEntry *gTextHighSlotEntries[4] = { &D_003E0B28, &D_003E0B98, 0, 0 };
TextStateEntry *D_003E0D40[4] = { &D_003E0BE8, &D_003E0C58, 0, 0 };
TextStateEntry *D_003E0D50[4] = { &D_003E0CC8, &D_003E0D38, 0, 0 };
TextStateEntry *gTextScoredNoLevelEntries[2] = { &D_003E0DC8, 0 };
TextStateEntry *gTextScoredLowLevelEntries[2] = { &D_003E0E38, 0 };
TextStateEntry *gTextScoredHighLevelEntries[2] = { &D_003E0EA8, 0 };
TextStateEntry *gTextTargetTertiaryEntries[6] = { &D_003E0F40, &D_003E0FC0, 0, 0, 0, 0 };
TextStateEntry *gTextTargetLatchedEntries[4] = { &D_003E1078, &D_003E10F8, 0, 0 };
TextStateEntry *gTextTargetLatchedSecondEntries[4] = { &D_003E1178, &D_003E11E8, 0, 0 };
TextStateEntry *gTextLevelTierAZeroEntries[4] = { &D_003E1278, &D_003E12F8, 0, 0 };
TextStateEntry *gTextLevelTierALowEntries[4] = { &D_003E1378, &D_003E13F8, 0, 0 };
TextStateEntry *gTextLevelTierAHighEntries[4] = { &D_003E1478, &D_003E14F8, 0, 0 };
TextStateEntry *gTextLevelTierBZeroEntries[4] = { &D_003E15A8, &D_003E1628, 0, 0 };
TextStateEntry *gTextLevelTierBLowEntries[4] = { &D_003E16A8, &D_003E1728, 0, 0 };
TextStateEntry *gTextLevelTierBHighEntries[4] = { &D_003E17A8, &D_003E1828, 0, 0 };
TextStateEntry *D_003E1A60[4] = { &D_003E18D8, &D_003E1958, 0, 0 };
TextStateEntry *D_003E1A70[4] = { &D_003E19D8, &D_003E1A58, 0, 0 };
TextStateEntry *D_003E1C40[4] = { &D_003E1AE8, &D_003E1B58, 0, 0 };
TextStateEntry *D_003E1C50[4] = { &D_003E1BC8, &D_003E1C38, 0, 0 };
TextStateEntry *gTextTargetIfSetEntries[4] = { &D_003E1CC8, &D_003E1D38, &D_003E1E18, 0 };
TextStateEntry *gTextNotAllFlaggedEntries[2] = { &D_003E1EA0, 0 };
TextStateEntry *gTextAllFlaggedEntries[2] = { &D_003E1F20, 0 };
TextStateEntry *gTextScaledEntries[2] = { &D_003E1FD8, 0 };
TextStateEntry *D_003E2028[2] = { &D_003E2020, 0 };
TextStateEntry *D_003E2198[4] = { &D_003E20A0, &D_003E2118, &D_003E2190, 0 };

unsigned char D_0055AF80[16]; /* D_0055AF88 and D_0055AF8C (tu265) are its words at +8 and +12 */
unsigned char D_0055AF90[16];
