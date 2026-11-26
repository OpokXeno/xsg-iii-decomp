#include "common.h"
#include "main/tu007.h"

#include "main/xgl_render.h"
#include "quadword.h"
#include "main/battle_shared.h"
typedef signed char s8;

typedef int s32;

typedef unsigned char u8;

typedef short s16;

typedef unsigned short u16;

typedef long long s64;

/* Game loop runtime block at .data 0x3E46B0 (addressed absolutely; the object is larger than the small-data limit).
   Only the fields used by this TU are named; the rest is padding. */
typedef struct GameLoopRuntime {
    s32 loopFlags;                   /* +0x00 */
    s32 loopFlagsHigh;               /* +0x04: upper halfword only is touched by the state setups */
    s64 resetCombo;                  /* +0x08: held-button bits compared by padCheckResetCombo */
    s16 unk_10;                      /* +0x10: cleared by the scene setup */
    s16 unk_12;                      /* +0x12: set to -1 by the scene setup */
    u8 colorRed;                     /* +0x14: sent to the scratchpad color command, then cleared */
    u8 colorGreen;                   /* +0x15 */
    u8 colorBlue;                    /* +0x16 */
    u8 unk_17[0x100 - 0x17];
    s32 unk_100;                     /* +0x100: cleared by func_001BC918 */
    u8 unk_104[0x10C - 0x104];
    s16 unk_10C;                     /* +0x10C: set from the reset combo's second halfword */
    u8 unk_10E[0x110 - 0x10E];
    s16 unk_110;                     /* +0x110: copied into the thread block */
    u8 unk_112[0x114 - 0x112];
    u8 unk_114[0x124 - 0x114];       /* +0x114: 16 bytes copied into the thread block */
    s32 unk_124;                     /* +0x124 */
    s32 activeHandle;                /* +0x128 */
    s32 pendingHandle;               /* +0x12C */
    u8 unk_130[0x15C - 0x130];
    s16 unk_15C;                     /* +0x15C */
    s16 unk_15E;                     /* +0x15E */
    s32 unk_160;                     /* +0x160 */
    s32 unk_164;                     /* +0x164 */
    s32 unk_168;                     /* +0x168 */
    f32 unk_16C;                     /* +0x16C: 5.0 or 1.0 by the scene setup */
    s16 unk_170;                     /* +0x170 */
    u8 unk_172[0x174 - 0x172];
    u8 unk_174[20];                  /* +0x174: cleared by the scene setup */
    u8 unk_188[0x1B0 - 0x188];
    s32 comboState;                  /* +0x1B0: passed to func_001B7A60 */
    u8 unk_1B4[0x1B8 - 0x1B4];
    s32 unk_1B8;                     /* +0x1B8: set to 15 by func_001BC918 */
    u8 unk_1BC[0x1C0 - 0x1BC];
    s32 waitFrames;                  /* +0x1C0: set to 30 by gameStartWaitFrames */
    s32 unk_1C4;                     /* +0x1C4: passed to stateBlockSetHead */
    u8 unk_1C8[0x1F0 - 0x1C8];
    s32 unk_1F0;                     /* +0x1F0: cleared by func_001BC918 */
} GameLoopRuntime;

/* Record reached through stateBlockGet() + 0x310 in sceneRecordUpdate. */
typedef struct SceneStateRecord {
    s32 unk_00;
    s32 sceneState;                       /* +0x04: compared with 4 */
    s32 sceneHandle;                      /* +0x08: passed to func_001BE6A8, tested against 0 */
} SceneStateRecord;

/* The block stateBlockGet returns: its SceneStateRecord sits at +0x310. */
typedef struct RuntimeStateBlock {
    u8 unk_000[0x310];
    SceneStateRecord sceneRecord;
    u8 unk_31C[0x5E4 - 0x31C];
    u8 eventViewerState[1];               /* +0x5E4: passed to eventViewerResetStart */
    u8 unk_5E5[0x830 - 0x5E5];
    u8 *fogBlock;                         /* +0x830: fog parameters, 704 and 720 bytes in */
} RuntimeStateBlock;

typedef struct ActorStatusRecord {
    s32 key;     /* +0x00: passed to RgDispModelFindByActor */
    f32 float04; /* +0x04: compared with 0.0f */
    f32 float08; /* +0x08: compared with 0.0f */
} ActorStatusRecord;

/* Motion block of a slot object, +0x150 of ActorSlotRecord. */
typedef struct ActorMotionRecord {
    u8 unk_00[0x1C];
    f32 unk_1C;                       /* +0x1C: cleared by func_001BDDD8 */
    f32 unk_20;                       /* +0x20: cleared by func_001BDDD8 */
    u8 unk_24[0x90 - 0x24];
    s16 unk_90;                       /* +0x90: cleared by func_001BDDD8 */
    s16 unk_92;                       /* +0x92: cleared by func_001BDDD8 */
} ActorMotionRecord;

/* Slot object returned by RgDispModelFindByKey (see the shared slot layout). */
typedef struct ActorSlotRecord {
    s32 slotFlags;                    /* +0x00 */
    u8 unk_04[4];
    s32 flags;                        /* +0x08 */
    u8 unk_0C[0x24 - 0x0C];
    struct XenoActor *actor;          /* +0x24 */
    u8 unk_28[0x50 - 0x28];
    AlignedVector4 vector50;          /* +0x50 */
    u8 unk_60[0x150 - 0x60];
    ActorMotionRecord motion;         /* +0x150 */
    u8 unk_1E4[0x200 - 0x1E4];
    ActorStatusRecord threadStatus;   /* +0x200 */
} ActorSlotRecord;

extern unsigned char gGameLoopRuntimeStorage[];
extern u16 gPadDataPressed[];
extern unsigned char PadData[];

/* Typed views of the unsized objects: the extern stays unsized, so the symbols and their addressing are unchanged. */
#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)
#define gPadData (*(PadRecord *) PadData)

s32 Vibration_Stop();
s32 xglPacketSetSendMode();
s32 soundStateReset();
s32 sbNativeValueSlotPop();
s32 soundCueSnapshotSave();
s32 subsystemResetClearState();
s32 slotReleaseFixedSlots();
s32 sefResetBattleWinState();
s32 func_00265E68();
s32 movieShutdown();
s32 XenoActorPoolInitDefault();
s32 njPlaneSetHook();
s32 gameClearRuntimeFlag();
s32 sefResetResourceTable();
s32 xglRenderClearFrame();

/* Clears the loop flags 0x110 and shuts down the per-scene subsystems (battle win state, movie, sound, rumble, actor pool, render frame). */
void gameResetRuntimeState(void) {
    gGameLoopRuntime.loopFlags &= ~0x110;
    sefResetBattleWinState();
    slotReleaseFixedSlots();
    njPlaneSetHook(0);
    gameClearRuntimeFlag();
    func_00265E68();
    movieShutdown();
    soundCueSnapshotSave();
    soundStateReset();
    subsystemResetClearState();
    sefResetResourceTable();
    XenoActorPoolInitDefault();
    Vibration_Stop();
    xglPacketSetSendMode(0);
    xglRenderClearFrame();
    sbNativeValueSlotPop(0);
}

s32 func_001B4618();

/* Shuts the task queue down: clears the state twice, then runs func_001B4618. */
void gameStopTasks(void) {
    sbNativeValueSlotPop(0);
    sbNativeValueSlotPop(0);
    func_001B4618();
}

/* Button masks of the pad halfwords (PadRecord.held/pressed/repeated, SDK bit layout). */
enum PadButton {
    PAD_BUTTON_SELECT = 0x100,
    PAD_BUTTON_START = 0x800,
    PAD_BUTTON_UP = 0x1000,
    PAD_BUTTON_RIGHT = 0x2000,
    PAD_BUTTON_DOWN = 0x4000,
    PAD_BUTTON_LEFT = 0x8000,
    PAD_BUTTON_L1 = 0x4,
    PAD_BUTTON_R1 = 0x8,
    PAD_BUTTON_TRIANGLE = 0x10,
    PAD_BUTTON_CIRCLE = 0x20
};

/* Rows of the debug stage menu (debugStageMenuStep). */
enum DebugMenuRow {
    DEBUG_MENU_ROW_MODE,
    DEBUG_MENU_ROW_NUMBER,
    DEBUG_MENU_ROW_ENTRY,
    DEBUG_MENU_ROW_RESTORE,
    DEBUG_MENU_ROW_LAST = DEBUG_MENU_ROW_RESTORE
};

#define DEBUG_MENU_MODE_LAST 2
#define DEBUG_MENU_NUMBER_MAX 9999
#define DEBUG_MENU_ENTRY_LAST 14

/* Map selection handed from the debug menu to gameStartSelectedMap (same layout as ResetComboBits.half). */
typedef struct DebugSelection {
    s16 mapId;
    s16 rosterId;
} DebugSelection;

void debugStageMenuStep(DebugSelection *selection);
s32 sbNativeValueSlotSet();
s32 xglPadSetRepeat();

/* Sets the pad repeat for every port, clears the frame and runs the debug stage menu (debugStageMenuStep) as the next step. */
void gameEnterPadRepeatState(void) {
    xglPadSetRepeat(-1, 0xF00C, 5, 1);
    xglRenderClearFrame();
    sbNativeValueSlotSet(debugStageMenuStep, 1);
}

/* One row of the debug stage menu table gDebugMenuEntries. */
typedef struct DebugMenuEntry {
    s16 id;
    s16 unk_02;
    char *name;
} DebugMenuEntry;

extern unsigned char D_003E4874[];

/* Header of the record stateRestore reads (gSavedPlacement), seen as a word. */
typedef struct StateRestoreHeader {
    s32 id;
    s32 unk_04[4];
} StateRestoreHeader;

extern s32 gDebugMenuRow;
extern s32 gDebugMenuMode;
extern s32 gDebugMenuNumber;
extern s32 gDebugMenuEntryIndex;

/* Saved placement record gSavedPlacement: an id and a position. */
typedef struct StatePlacement {
    u16 id;
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    } position;
} StatePlacement;

extern StatePlacement gSavedPlacement;
void stateRestore();
extern char *D_003E5290[];
void gameResetRuntimeState(void);
void gameEnterPadRepeatState(void);

typedef union ResetComboBits {
    s64 all;
    struct {
        u16 held;
        s16 pressed;
    } half;
} ResetComboBits;

/* Kept apart: the union view of resetCombo (+0x08) decides the emitted halfword stores. */
typedef struct GameLoopResetComboBitsView {
    u8 unk_00[8];
    ResetComboBits combo;
} GameLoopResetComboBitsView;

#define gGameLoopResetComboBits (*(GameLoopResetComboBitsView *) gGameLoopRuntimeStorage)

void gameStartSelectedMap(ResetComboBits *combo);
void sbNativeValueSlotPush();
s32 saveScreenRun();
s32 GameMpeg2Play();
s32 xglFontDebugPrintf();

/* Debug stage menu: four rows (mode, map number, table entry, restore). Circle on the number row hands the map to gameStartSelectedMap. */
void debugStageMenuStep(DebugSelection *selection) {
    extern DebugMenuEntry gDebugMenuEntries[];
    char title[64];
    s32 held = gPadData.held | gPadData.directionHeld;
    s32 pressed = gPadData.pressed | gPadData.directionPressed;
    s32 repeated = gPadData.repeated | gPadData.directionRepeated;
    if ((held & PAD_BUTTON_SELECT) && (pressed & PAD_BUTTON_START)) {
        sbNativeValueSlotSet(gameResetRuntimeState, 0);
        return;
    }
    if (pressed & PAD_BUTTON_UP) {
        gDebugMenuRow = gDebugMenuRow > 0 ? --gDebugMenuRow : DEBUG_MENU_ROW_LAST;
    }
    if (pressed & PAD_BUTTON_DOWN) {
        gDebugMenuRow = gDebugMenuRow < DEBUG_MENU_ROW_LAST ? ++gDebugMenuRow : 0;
    }
    switch (gDebugMenuRow) {
    case DEBUG_MENU_ROW_MODE:
        if (repeated & PAD_BUTTON_LEFT) {
            gDebugMenuMode = gDebugMenuMode > 0 ? --gDebugMenuMode : DEBUG_MENU_MODE_LAST;
        }
        if (repeated & PAD_BUTTON_RIGHT) {
            gDebugMenuMode = gDebugMenuMode < DEBUG_MENU_MODE_LAST ? ++gDebugMenuMode : 0;
        }
        *(s32 *) D_003E4874 = gDebugMenuMode;
        break;
    case DEBUG_MENU_ROW_NUMBER:
        if (repeated & PAD_BUTTON_LEFT) {
            gDebugMenuNumber = gDebugMenuNumber > 0 ? --gDebugMenuNumber : DEBUG_MENU_NUMBER_MAX;
        }
        if (repeated & PAD_BUTTON_RIGHT) {
            gDebugMenuNumber = gDebugMenuNumber < DEBUG_MENU_NUMBER_MAX ? ++gDebugMenuNumber : 0;
        }
        if (repeated & PAD_BUTTON_L1) {
            gDebugMenuNumber = gDebugMenuNumber >= 100 ? gDebugMenuNumber - 100 : gDebugMenuNumber + 9900;
        }
        if (repeated & PAD_BUTTON_R1) {
            gDebugMenuNumber = gDebugMenuNumber < 9899 ? gDebugMenuNumber + 100 : gDebugMenuNumber - 9900;
        }
        if (pressed & PAD_BUTTON_CIRCLE) {
            if (gDebugMenuNumber) {
                selection->mapId = gDebugMenuNumber;
                selection->rosterId = gDebugMenuEntries[gDebugMenuEntryIndex].id;
                sbNativeValueSlotSet(gameEnterPadRepeatState, 0);
                sbNativeValueSlotPush(gameStartSelectedMap, 0);
            }
        }
        break;
    case DEBUG_MENU_ROW_ENTRY:
        if (repeated & PAD_BUTTON_LEFT) {
            gDebugMenuEntryIndex = gDebugMenuEntryIndex > 0 ? --gDebugMenuEntryIndex : DEBUG_MENU_ENTRY_LAST;
        }
        if (repeated & PAD_BUTTON_RIGHT) {
            gDebugMenuEntryIndex = gDebugMenuEntryIndex < DEBUG_MENU_ENTRY_LAST ? ++gDebugMenuEntryIndex : 0;
        }
        break;
    case DEBUG_MENU_ROW_RESTORE:
        if (pressed & PAD_BUTTON_CIRCLE) {
            if (saveScreenRun(1)) {
                ((s32 (*)()) stateRestore)(&gSavedPlacement);
                gDebugMenuNumber = ((StateRestoreHeader *) &gSavedPlacement)->id;
            }
        }
        break;
    }
    GameMpeg2Play(title, "\022\010\010Compiled on %s %s", "Jul 12 2006", "13:22:53");
    xglFontDebugPrintf(16, 16, title);
    xglFontDebugPrintf(16, 32, "Map :%s", D_003E5290[gDebugMenuMode]);
    xglFontDebugPrintf(16, 42, "No. :%4d", gDebugMenuNumber);
    xglFontDebugPrintf(16, 52, "MDL :%s", gDebugMenuEntries[gDebugMenuEntryIndex].name);
    ((void (*)()) xglFontDebugPrintf)(16, 62, "MCLOAD");
    xglFontDebugPrintf(8, gDebugMenuRow * 10 + 32, ">");
}

s32 battleRosterPairReset();
s32 func_00221FB8();
void battleCurrentRosterIndexSet(s32 rosterIndex);
s32 battleRosterCursorSet(s32 index, s32 rosterIndex);
s32 xglPadSetDefaults();
void func_001BC918(s16 *out);

/* Applies a map selection: stores its map id, resets the party for the chosen roster id (1..10 and 13..16 are the others) and continues in func_001BC918. */
void gameStartSelectedMap(ResetComboBits *combo) {
    s32 memberIndex;
    s32 pressed;
    u16 held;

    held = combo->half.held;
    gGameLoopResetComboBits.combo.half.held = held;
    gGameLoopResetComboBits.combo.half.pressed = -1;
    pressed = combo->half.pressed;
    if (pressed >= 0) {
        gGameLoopRuntime.unk_10C = pressed;
        memberIndex = 0;
        battleRosterPairReset();
        func_00221FB8(pressed);
        do {
            if (memberIndex + 1 != pressed) {
                func_00221FB8(memberIndex + 1);
            }
            memberIndex++;
        } while (memberIndex < 10);
        memberIndex = 0;
        do {
            if (memberIndex + 13 != pressed) {
                func_00221FB8(memberIndex + 13);
            }
            memberIndex++;
        } while (memberIndex < 4);
        if (pressed < 13) {
            battleCurrentRosterIndexSet(0);
            battleRosterCursorSet(0, 0);
        } else {
            battleCurrentRosterIndexSet(1);
            battleRosterCursorSet(0, 1);
        }
    }
    xglPadSetDefaults();
    sbNativeValueSlotSet(func_001BC918, 0);
}

typedef struct GameLoopResetComboHalves {
    u8 unk_00[8];
    s16 resetComboHalves[4];
} GameLoopResetComboHalves;

#define gGameLoopResetComboHalves (*(GameLoopResetComboHalves *) gGameLoopRuntimeStorage)

s32 seScriptTablesClear();

/* Starts map 7000 with the default roster, clears the script tables and continues in func_001BC918. */
void func_001BC6A8(void) {
    gGameLoopRuntime.unk_1C4 = 0;
    gGameLoopResetComboHalves.resetComboHalves[0] = 7000;
    gGameLoopResetComboHalves.resetComboHalves[1] = -1;
    ((void (*)()) battleRosterPairReset)();
    battleCurrentRosterIndexSet(0);
    ((void (*)()) func_00221FB8)(1);
    ((void (*)()) func_00221FB8)(8);
    ((void (*)()) func_00221FB8)(7);
    battleRosterCursorSet(0, 0);
    seScriptTablesClear();
    sbNativeValueSlotSet(func_001BC918, 0);
}

/* Kept apart: the bitfield halves at +0x08 and the float position at +0x18 of the game loop runtime. */
typedef struct GameLoopPlacementView {
    s32 loopFlags;                    /* +0x00 */
    u8 unk_04[4];
    s32 mapId : 16;                   /* +0x08 */
    s32 subMapId : 16;                /* +0x0A */
    u8 unk_0C[8];
    u8 colorRed;                      /* +0x14 */
    u8 colorGreen;
    u8 colorBlue;
    u8 colorAlpha;
    f32 positionX;                    /* +0x18 */
    f32 positionY;
    f32 positionZ;
    f32 positionW;
    u8 unk_28[0x1c4 - 0x28];
    s32 unk_1C4;                      /* +0x1C4 */
    u8 unk_1C8[0x100];
} GameLoopPlacementView;

#define gGameLoopPlacement (*(GameLoopPlacementView *) gGameLoopRuntimeStorage)

void func_00225108();
void saveStateIndexSet();
s32 bitmapTestBit();

/* Starts the saved placement: copies gSavedPlacement's id and position into the runtime block and restores its state. */
void gameStartSavedPlacement() {
    StatePlacement *placement = &gSavedPlacement;
    gGameLoopPlacement.mapId = placement->id;
    gGameLoopPlacement.subMapId = -1;
    gGameLoopPlacement.unk_1C4 = 0;
    gGameLoopPlacement.positionX = placement->position.x;
    gGameLoopPlacement.positionY = placement->position.y;
    gGameLoopPlacement.positionZ = placement->position.z;
    gGameLoopPlacement.positionW = placement->position.w;
    gGameLoopPlacement.loopFlags |= 0x800000;
    stateRestore(placement);
    func_00225108();
    if (bitmapTestBit(0xc7)) {
        gGameLoopPlacement.loopFlags |= 0x8000000;
    }
    if (gGameLoopPlacement.mapId == 0x1db0) {
        gGameLoopPlacement.mapId = 0x5a0;
        gGameLoopPlacement.positionX = 8.2f;
        gGameLoopPlacement.positionY = 1.2f;
        gGameLoopPlacement.positionZ = 4.31f;
        gGameLoopPlacement.positionW = 0;
        saveStateIndexSet(0x16e);
    }
    sbNativeValueSlotSet(func_001BC918, 0);
}

extern u16 D_003E46A8[];

/* Starts the map requested through D_003E46A8 (map id and two flag bits), restores the saved state and continues in func_001BC918. */
void func_001BC808(void) {
    u16 *mapRequest = D_003E46A8;
    s32 requestBits;

    gGameLoopResetComboHalves.resetComboHalves[1] = -1;
    gGameLoopResetComboHalves.resetComboHalves[0] = mapRequest[1];
    gGameLoopRuntime.unk_1C4 = 0;
    stateRestore(gSavedPlacement);
    func_00225108();
    battleRosterPairReset();
    battleCurrentRosterIndexSet(0);
    func_00221FB8(1);
    battleRosterCursorSet(0, 0);
    requestBits = mapRequest[0];

    if (requestBits & 1) {
        gGameLoopRuntime.loopFlags |= 0x01000000;
    }
    if (requestBits & 2) {
        gGameLoopRuntime.loopFlags |= 0x02000000;
    }
    sbNativeValueSlotSet(func_001BC918, 0);
}


const char gDebugValueFormat[24] = "NO MAP [%4d]";

/* Prints the value at the debug position; start or circle returns to the loop. */
void debugPrintValue(s16 *printedValue) {
    xglFontDebugPrintf(8, 0x20, gDebugValueFormat, *printedValue);
    if (gPadDataPressed[0] & (PAD_BUTTON_START | PAD_BUTTON_CIRCLE)) {
        sbNativeValueSlotPop(0);
    }
}

void xglClockSampleDayTime();
void func_001B4810();
s32 func_001BE470();
void func_001BC9B0(void);

/* Prepares the next scene load: sets the wait counters to 15, stops rumble, loads the requested map and falls back to the value printer when the load fails. */
void func_001BC918(s16 *mapIdOut) {
    sbNativeValueSlotSet(func_001BC9B0, 0);
    xglClockSampleDayTime();
    func_001B4810();
    gGameLoopRuntime.unk_1B8 = 15;
    gGameLoopRuntime.waitFrames = 15;
    gGameLoopRuntime.unk_100 = 0;
    Vibration_Stop();
    sefResetResourceTable();
    if (func_001BE470(gGameLoopResetComboHalves.resetComboHalves[0], 0) == 0) {
        *mapIdOut = gGameLoopResetComboHalves.resetComboHalves[0];
        sbNativeValueSlotSet(debugPrintValue, 0);
    }
    gGameLoopRuntime.unk_1F0 = 0;
}

/* Kept apart: the unsigned sceneState field type decides the emitted compare (sltiu). */
typedef struct UnsignedSceneStateRecord {
    s32 unk_00;
    u32 sceneState;
} UnsignedSceneStateRecord;

typedef struct UnsignedRuntimeStateBlock {
    u8 unk_000[8];
    UnsignedSceneStateRecord runtimeRecord;   /* +0x08: its sceneState (+0x0C) is the runtime scene state */
    u8 unk_010[0x310 - 0x10];
    UnsignedSceneStateRecord sceneRecord;
} UnsignedRuntimeStateBlock;

void func_001BCAD0(void);
s32 func_001B60B8();
s32 func_001B7B98();
RuntimeStateBlock *stateBlockGet();
void soundStateClearCounters();
void stateBlockCommit();

/* Waits for the load, sets loop flag 0x20000000 from the scene transition and commits the state block. */
void func_001BC9B0(void) {
    UnsignedRuntimeStateBlock *sceneStateBlock;
    UnsignedSceneStateRecord *sceneRecord;
    UnsignedSceneStateRecord *runtimeSceneRecord;
    u32 sceneState;

    sceneStateBlock = ((UnsignedRuntimeStateBlock * (*) ()) stateBlockGet)();
    runtimeSceneRecord = &sceneStateBlock->runtimeRecord;
    sceneRecord = &sceneStateBlock->sceneRecord;
    if (func_001B60B8(2) != 0) {
        return;
    }
    sceneState = sceneRecord->sceneState;
    gGameLoopRuntime.loopFlags &= ~0x20000000;
    switch (sceneState) {
    case 0:
    case 1:
    case 3:
        switch (runtimeSceneRecord->sceneState) {
        case 4:
        case 5:
        case 6:
            gGameLoopRuntime.loopFlags |= 0x20000000;
            func_001B7B98(8);
            break;
        }
        break;
    case 4:
    case 5:
        soundStateClearCounters();
        gGameLoopRuntime.comboState = 0;
        break;
    }
    if (runtimeSceneRecord->sceneState == 2) {
        gGameLoopRuntime.comboState = 0;
    }
    stateBlockCommit();
    sbNativeValueSlotSet(func_001BCAD0, 1);
}

void func_001BCCA0(void);
void poolReleaseNonMatching();
void memoryBanksReset();
void memoryBanksTest();
void stateBlockSetHead();
void func_001BE860();
void sbNativeFileLoad();
s32 func_001BE6A8();

/* Sets up the memory banks for the scene kind in the state block, then starts its scene handle (func_001BE6A8). */
void func_001BCAD0(void) {
    SceneStateRecord *scene;

    scene = &stateBlockGet()->sceneRecord;
    gGameLoopRuntime.loopFlags = (gGameLoopRuntime.loopFlags & 0x3FF00000) | 3;
    poolReleaseNonMatching(1, 0xCF000000, 0xFF000000);
    memoryBanksReset(240);
    memoryBanksTest(1);
    switch (scene->sceneState) {
    default:
    case 0:
        gGameLoopRuntime.loopFlags &= ~1;
        memoryBanksReset(12);
        memoryBanksTest(2);
        break;
    case 1:
        gGameLoopRuntime.loopFlags = (gGameLoopRuntime.loopFlags & ~1) | 0x20;
        memoryBanksTest(14);
        break;
    case 2:
        memoryBanksReset(8);
        memoryBanksTest(6);
        soundStateClearCounters();
        break;
    case 3:
        memoryBanksReset(12);
        memoryBanksTest(2);
        break;
    case 4:
        memoryBanksReset(12);
        memoryBanksTest(2);
        func_001BE860(1);
        soundCueSnapshotSave();
        break;
    case 5:
        memoryBanksReset(8);
        memoryBanksTest(6);
        func_001BE860(1);
        soundCueSnapshotSave();
        break;
    case 6:
        memoryBanksReset(12);
        memoryBanksTest(2);
        func_001BE860(1);
        soundCueSnapshotSave();
        break;
    }
    if (scene->sceneHandle != 0) {
        gGameLoopRuntime.loopFlags &= ~2;
        stateBlockSetHead(gGameLoopRuntime.unk_1C4);
        func_001BE6A8(scene->sceneHandle, 0, 0);
    }
    sbNativeFileLoad((s16) gGameLoopRuntime.resetCombo, 0);
    sbNativeValueSlotSet(func_001BCCA0, 1);
}

void sceneRecordUpdate(void);
void studioCameraInit();
void func_001BF688();
void xglCullingMapClear();
void func_00296528();
void sceneInitSubsystems();
void mapFlagTableReset();
void func_0027A4D0();
void func_001B8DB8();
void func_001B8EB0();
s32 func_001B5D70();
void hwStatusReady();
void func_001B9438();
void func_001B8FA0();
void memset();
s32 sefSetSceneMode();

/* Initializes the scene subsystems (camera, culling map, actor pool, map flags) and clears the runtime handles. */
void func_001BCCA0(void) {
    SceneStateRecord *sceneRecord;
    RuntimeStateBlock *block;

    block = stateBlockGet();
    sceneRecord = &block->sceneRecord;
    studioCameraInit();
    func_001BF688();
    xglCullingMapClear();
    Vibration_Stop();
    XenoActorPoolInitDefault();
    func_00296528();
    sceneInitSubsystems();
    sefSetSceneMode(1);
    mapFlagTableReset();
    func_0027A4D0(20);
    func_001B8DB8();
    func_001B8EB0();
    gGameLoopRuntime.unk_10 = 0;
    gGameLoopRuntime.unk_12 = -1;
    gGameLoopRuntime.unk_124 = 0;
    gGameLoopRuntime.activeHandle = 0;
    gGameLoopRuntime.pendingHandle = 0;
    gGameLoopRuntime.unk_16C =
        func_001B5D70(gGameLoopRuntime.unk_10C) ? 5.0f : 1.0f;
    gGameLoopRuntime.unk_170 = 0;
    memset(gGameLoopRuntime.unk_174, 0, 20);
    hwStatusReady();
    func_001B9438(0);
    if (sceneRecord->sceneState != 4) {
        func_001B8FA0();
    }
    sbNativeValueSlotSet(sceneRecordUpdate, 0);
}

s32 GameSynopsisCrossfadeWait();
s32 xglRenderLockClear();
s32 func_001B5F28();
s32 func_001B8508();
s32 func_001B9850();
s32 vmStreamPoll();
s32 func_001B9B88();
void func_001BCF08(void);
void func_001BE320(void);
s32 func_001BE828();
s32 stateBlockReset();
s32 func_001BE8E0();
s32 stageSetup();
s32 soundCueReset();
void func_00225B90();

/* Per-frame scene update: resets the state block, runs the streams and picks the next step by the loop flags. */
void sceneRecordUpdate(void) {
    SceneStateRecord *sceneRecord;
    RuntimeStateBlock *sceneBlock;

    sceneRecord = &stateBlockGet()->sceneRecord;
    if (func_001BE828(0) == 0) {
        return;
    }
    stateBlockReset();
    if (!(gGameLoopRuntime.loopFlags & 2)) {
        func_001BE8E0();
        stageSetup();
    }
    func_001B9850();
    if (sceneRecord->sceneState != 4) {
        sceneBlock = stateBlockGet();
        func_001B8508(&sceneBlock->sceneRecord);
    }
    func_001B5F28(0, 1, 0);
    soundCueReset();
    vmStreamPoll(1);
    func_001B9B88(0);
    GameSynopsisCrossfadeWait();
    if (gGameLoopRuntime.loopFlags & 0x04000000) {
        sbNativeValueSlotSet(func_001BE320, 0);
        return;
    }
    func_00225B90();
    if (!(gGameLoopRuntime.loopFlags & 0x10000000)) {
        xglRenderClearFrame();
    }
    xglRenderLockClear();
    sbNativeValueSlotSet(func_001BCF08, 0);
}

typedef struct BcfState {
    s32 flags;
    u8 unk_04[0x6];
    s16 unk_0A;
    u8 unk_0C[0xC];
    f32 posX;
    f32 posY;
    f32 posZ;
    f32 unk_24;
    u8 unk_28[0x100 - 0x28];
    void *camera;
    u8 unk_104[0x1E8 - 0x104];
    f32 zoomX;
    f32 zoomY;
} BcfState;

#define gBcfState (*(BcfState *) gGameLoopRuntimeStorage)

s32 soundStateGetCounter();
s32 soundSlotSetVolume();
void xglCdSetDriveStatus(s32 driveStatus);
void eventViewerResetStart();
s32 eventViewerCdPoll();
void sbNativeOwnersInit();
void cameraInitModule();
void cameraInit();
void movieInit();
void sbRegisterIdNatives();
void enepcRegisterNatives();
void addDefaultNativeOwner();
void scriptRegisterNatives();
void sbNativeOwnersRegister();
void func_A8E148();
void func_AB3218();
ActorSlotRecord *func_001C4778();
f32 SpawnSlotSetPosition();
void SpawnSlotSetRotation();
f32 markerPositionGet();
void cameraSetCurrent();
void streamSetValueFlag();
void func_001BD378(void);
void gameStartSbImage(void);

#define ZERO_VECTOR(v) __asm__("sqc2 $vf0, 0(%0)" : : "r"(v) : "memory")

/* Registers the natives and the camera of the new scene and picks the next step by the scene state (6 starts func_001BD378). */
void func_001BCF08(void) {
    RuntimeStateBlock *sceneStateBlock;
    SceneStateRecord *sceneRecord;
    ActorSlotRecord *sceneSlot;
    f32 cameraOffset[4];
    f32 zoom;

    sceneStateBlock = stateBlockGet();
    sceneRecord = &sceneStateBlock->sceneRecord;
    if (gBcfState.flags & 0x20000000) {
        if (soundStateGetCounter() != 0) {
            return;
        }
        if (soundSlotSetVolume(0, 0, -1) != 0) {
            return;
        }
    }
    if (sceneRecord->sceneState == 4) {
        xglCdSetDriveStatus(1);
        eventViewerResetStart(sceneStateBlock->eventViewerState);
        eventViewerCdPoll();
    }
    sbNativeOwnersInit();
    cameraInitModule();
    cameraInit();
    movieInit();
    sbRegisterIdNatives();
    enepcRegisterNatives();
    addDefaultNativeOwner();
    scriptRegisterNatives();
    sbNativeOwnersRegister();
    func_A8E148();
    func_AB3218();
    sceneSlot = func_001C4778();
    if (sceneSlot != 0) {
        gBcfState.camera = sceneSlot;
        if (gBcfState.flags & 0x800000) {
            ZERO_VECTOR(cameraOffset);
            cameraOffset[0] = gBcfState.posX;
            cameraOffset[1] = gBcfState.posY;
            cameraOffset[2] = gBcfState.posZ;
            SpawnSlotSetPosition(sceneSlot, cameraOffset);
            ZERO_VECTOR(cameraOffset);
            cameraOffset[1] = gBcfState.unk_24;
            SpawnSlotSetRotation(sceneSlot, cameraOffset);
        } else {
            ZERO_VECTOR(cameraOffset);
            zoom = markerPositionGet(cameraOffset, gBcfState.unk_0A) * 0.0015339807f;
            SpawnSlotSetPosition(sceneSlot, cameraOffset);
            ZERO_VECTOR(cameraOffset);
            cameraOffset[1] = zoom;
            SpawnSlotSetRotation(sceneSlot, cameraOffset);
        }
        cameraSetCurrent(sceneSlot->actor);
        streamSetValueFlag((s32) sceneSlot->actor);
        gBcfState.zoomX = 2.0f;
        gBcfState.zoomY = -2.0f;
        if (sceneSlot->slotFlags & 0x100) {
            gBcfState.zoomX = 10.0f;
            gBcfState.zoomY = -10.0f;
        }
    }
    if (sceneRecord->sceneState == 6) {
        sbNativeValueSlotSet(func_001BD378, 0);
    } else {
        sbNativeValueSlotSet(gameStartSbImage, 0);
    }
}

void func_001BD1F8(void);
s32 sbNativeOwnerGetValue();
s32 sbNativeBufferGet();
s32 sbImageLinkImports();
s32 slotRecord1CountDiff();
s32 sbStartImage();

/* Starts the script image of the current native owner, then clears the color bytes in func_001BD1F8. */
void gameStartSbImage(void) {
    s32 imageId;

    if ((gGameLoopRuntime.loopFlags & 0x10000000) || (slotRecord1CountDiff() == 0)) {
        slotReleaseFixedSlots();
        if (sbNativeOwnerGetValue(0) != 0) {
            imageId = sbNativeBufferGet(0);
            gGameLoopRuntime.loopFlags |= 0x10;
            sbImageLinkImports(imageId);
            sbStartImage(imageId);
        }
        sbNativeValueSlotSet(func_001BD1F8, 0);
    }
}

extern s32 D_003E52A0[];
void padCheckResetCombo(void);
void scratchpadSendCmd27Mode2(s32 *sendParams);

/* Sends the stored color to the scratchpad, clears it with the environment color and enters padCheckResetCombo. */
void func_001BD1F8(void) {
    s32 *commandBlock;

    if (!(gGameLoopRuntime.loopFlags & 0x10000000)) {
        commandBlock = (s32 *) D_003E52A0;
        commandBlock[1] = gGameLoopRuntime.colorRed;
        commandBlock[2] = gGameLoopRuntime.colorGreen;
        commandBlock[3] = gGameLoopRuntime.colorBlue;
        scratchpadSendCmd27Mode2(commandBlock);
    }
    gGameLoopRuntime.colorBlue = 0;
    gGameLoopRuntime.colorGreen = 0;
    gGameLoopRuntime.colorRed = 0;
    gEnvFlags.color.red = gEnvFlags.color.green = gEnvFlags.color.blue = 0;
    gGameLoopRuntime.loopFlags = (gGameLoopRuntime.loopFlags & 0xFF3FFFFF) | 0x100;
    sbNativeValueSlotSet(padCheckResetCombo, 0);
}

s32 func_001B6AB0();
s32 func_001B7A60();
void func_001BD450();
s32 sbNativeBlockLoad();

/* Re-applies the combo state and handles; the map-id combo 0xDAC/0xDB6 clears the pad directions and buttons. */
void padCheckResetCombo(void) {
    func_001B6AB0();
    if (gGameLoopRuntime.loopFlags & 0x20000000) {
        func_001B7A60(gGameLoopRuntime.comboState, -1, 8, 0x20);
    } else {
        func_001B7A60(gGameLoopRuntime.comboState, -1, 0, 0x20);
    }
    if (gGameLoopRuntime.pendingHandle != 0) {
        sbNativeBlockLoad(gGameLoopRuntime.pendingHandle, 1);
    }
    if (gGameLoopRuntime.activeHandle != 0) {
        func_001BE6A8(gGameLoopRuntime.activeHandle, 1, 1);
    }
    if ((gGameLoopRuntime.resetCombo & ((0xFFFFLL << 0x20) | 0xFFFF)) == ((0xDB6LL << 0x20) | 0xDAC)) {
        gPadData.leftY = 0;
        gPadData.leftX = 0;
        gPadData.position[3] = 0;
        gPadData.position[2] = 0;
        gPadData.held = 0;
    }
    sbNativeValueSlotSet(func_001BD450, 0);
}

/* Variant of gameStartSbImage plus the color clear, entered from func_001BCF08 when the scene state is 6. */
void func_001BD378(void) {
    s32 imageId;

    if (func_001B60B8(0) == 0 && slotRecord1CountDiff() == 0) {
        ((void (*)()) slotReleaseFixedSlots)();
        gGameLoopRuntime.colorBlue = 0;
        gGameLoopRuntime.colorGreen = 0;
        gGameLoopRuntime.colorRed = 0;
        gEnvFlags.color.blue = 0;
        gEnvFlags.color.green = 0;
        gEnvFlags.color.red = 0;
        gGameLoopRuntime.loopFlags = (gGameLoopRuntime.loopFlags & 0xFF3FFFFF) | 0x100;
        func_001B6AB0();
        sbNativeValueSlotSet(func_001BD450, 0);
        if (sbNativeOwnerGetValue(0) != 0) {
            imageId = sbNativeBufferGet(0);
            sbImageLinkImports(imageId);
            ((void (*)()) sbStartImage)(imageId);
            gGameLoopRuntime.loopFlags |= 0x10;
        }
    }
}

/* Filler enum: only its alias set matters, the flag bits are tested as plain masks. */
enum SlotEnumWord {
    SLOT_ENUM_NONE,
    SLOT_ENUM_WIDTH = 0x7fffffff
};

/* Kept apart from ActorSlotRecord: the enum flag words decide the emitted order. */
typedef struct SlotEnumView {
    enum SlotEnumWord flags;         /* +0x00 */
    s32 unk_04;
    s32 unk_08;
    enum SlotEnumWord flagsC;        /* +0x0C */
    u8 unk_10[0x154 - 0x10];
    s32 unk_154;                      /* +0x154: copied into SbThreadBlock.unk_00 */
    s32 unk_158;                      /* +0x158 */
} SlotEnumView;

extern const s32 D_00531340[];
s32 padStartAndSoftResetHeld();
s32 Get_Rnd_001B8CD0();
s32 battleCurrentRosterCursorAdvance();
s32 slotModelStateIsFour();
s32 MMathRandomNext();
void sefFlagSetBit26();
void njPlaneSetRange();
void func_001BBBB8();
void func_0025F5C0();
void soundDriverSetOutputMode();
void soundSlotSetLevelIfStill();
void func_00230218();
void scratchpadSendCmd0A();
void func_001BD730(AlignedVector4 *savedVector);
void sefStartCurrentEffect();
void valueSlotAdvanceFromPlayerCounter();
void func_001BD830(void);
void func_001BDA58();

/* Kept: the inline helper decides how the flag test is scheduled. */
static inline s32 gameLoopPlaneFlagSet() {
    return gGameLoopRuntime.loopFlags & 0x20;
}

void *RgDispModelFindByKey();

/* Main play step: updates the plane hook, handles the triangle button and the pending requests that switch to other steps. */
const s32 D_00531340[2] = { 0x14, 0x01 };

void func_001BD450() {
    SlotEnumView *slot = (SlotEnumView *) RgDispModelFindByKey(0);
    u16 pressedButtons = gPadDataPressed[0];
    s32 planeHookActive;
    if (padStartAndSoftResetHeld()) {
        sefFlagSetBit26();
    }
    MMathRandomNext();
    planeHookActive = gameLoopPlaneFlagSet() != 0;
    if (slot && !(slot->flags & 0x400000)) {
        planeHookActive = 0;
    }
    if (planeHookActive) {
        njPlaneSetRange(Get_Rnd_001B8CD0(0, 0x38), gGameLoopRuntime.unk_170 == 0);
    }
    njPlaneSetHook(planeHookActive);
    func_001BBBB8();
    if (gGameLoopRuntime.loopFlags & 0x4000000) {
        sbNativeValueSlotSet(func_001BE320, 0);
        return;
    }
    if (gGameLoopRuntime.loopFlags & 0x10000) {
        gGameLoopRuntime.loopFlags |= 0x80000000;
        sbNativeValueSlotPush(func_001BDA58, 0);
        return;
    }
    if (gGameLoopRuntime.loopFlags & 1) {
        return;
    }
    if (!slot) {
        return;
    }
    if (!(gGameLoopRuntime.loopFlags & 0x4800) && (slot->flags & 0x400000) && !slot->unk_158) {
        if (pressedButtons & PAD_BUTTON_TRIANGLE) {
            func_0025F5C0(1);
            soundDriverSetOutputMode(1);
            soundSlotSetLevelIfStill(1, 1);
            func_00230218(((s32) slot->flags >> 8) & 1);
            func_0025F5C0(0);
            soundDriverSetOutputMode(0);
            soundSlotSetLevelIfStill(1, 0);
            gGameLoopRuntime.waitFrames = 5;
            slot->flagsC &= 0xfff2ffff;
        }
        if (gGameLoopRuntime.waitFrames > 0) {
            gGameLoopRuntime.waitFrames--;
        } else if (slot->flags & 0x10000000) {
            if (battleCurrentRosterCursorAdvance()) {
                scratchpadSendCmd0A(D_00531340);
                gGameLoopRuntime.loopFlags |= 8;
                slot->flags &= 0xefbfffff;
                sbNativeValueSlotPush(func_001BD730, 0);
                return;
            }
        }
    }
    sefStartCurrentEffect();
    valueSlotAdvanceFromPlayerCounter();
    if (slotModelStateIsFour()) {
        gGameLoopRuntime.loopFlags |= 0x80000000;
        sbNativeValueSlotPush(func_001BD830, 0);
        return;
    }
}

s32 func_001B9228();
void func_001BD7B8(AlignedVector4 *restoredVector);
extern const s32 D_00531348[];

/* Copies the current slot's vector at +0x50 into dst, runs func_001B9228 under loop flag 0x40000000 and waits in func_001BD7B8. */
const s32 D_00531348[6] = { 5, 0, 0, 0, 0x80, 0 };

void func_001BD730(AlignedVector4 *savedVector) {
    ActorSlotRecord *slot;
    AlignedVector4 *slotVector;

    slot = (ActorSlotRecord *) RgDispModelFindByKey(0);
    slotVector = &slot->vector50;
    __asm__ __volatile__("lq $8, 0(%2)\n"
                         "sq $8, 0(%1)\n"
                         : "=m"(*savedVector)
                         : "r"(savedVector), "r"(slotVector)
                         : "$8", "memory");
    gGameLoopRuntime.loopFlags |= 0x40000000;
    func_001B9228();
    gGameLoopRuntime.loopFlags &= 0xBFFFFFFF;
    scratchpadSendCmd27Mode2((s32 *) D_00531348);
    sbNativeValueSlotSet(func_001BD7B8, 0);
}

/* Writes source back into the current slot's vector at +0x50 when the wait ends and sets its flag 0x400000. */
void func_001BD7B8(AlignedVector4 *restoredVector) {
    ActorSlotRecord *slot;
    AlignedVector4 *slotVector;

    if (slotRecord1CountDiff(restoredVector) == 0) {
        gGameLoopRuntime.loopFlags &= ~8;
        slot = (ActorSlotRecord *) RgDispModelFindByKey(0);
        slot->slotFlags |= 0x400000;
        slotVector = &slot->vector50;
        __asm__ __volatile__("lq $8, 0(%2)\n"
                             "sq $8, 0(%1)\n"
                             : "=m"(*slotVector)
                             : "r"(slotVector), "r"(restoredVector)
                             : "$8", "memory");
        sbNativeValueSlotPop(0);
    }
}



void func_0030C130();
void gameSuspendSoundThreads(void);
void *RgDispModelFindByActor();

/* Fills the thread block from the thread table of the current slot (it must be in state 4) and goes to gameSuspendSoundThreads. */
void func_001BD830(void) {
    ActorSlotRecord *slot;
    ActorStatusRecord *threadStatus;
    SbThreadEntryTable *threadTable;
    SbThreadEntry *headEntry;
    SbThreadBlock *threadBlock;
    s32 threadMode;
    s32 threadTableFlags;

    slot = (ActorSlotRecord *) RgDispModelFindByKey(0);
    threadStatus = &slot->threadStatus;
    threadTable = (SbThreadEntryTable *) RgDispModelFindByActor(threadStatus->key);
    headEntry = &threadTable->threadEntries[0];
    threadBlock = &gSbThreadBlock;
    if (threadTable == 0) {
        gGameLoopRuntime.loopFlags &= 0x7FFFFFFF;
        sbNativeValueSlotPop(0);
        return;
    }
    if (threadTable->tableTypeId != 4) {
        gGameLoopRuntime.loopFlags &= 0x7FFFFFFF;
        sbNativeValueSlotPop(0);
        return;
    }
    threadBlock->unk_00 = headEntry->entryWord4;
    threadBlock->unk_04 = gGameLoopRuntime.unk_110;
    func_0030C130(gGameLoopRuntime.unk_114, threadBlock->unk_08, 16);
    threadBlock->threadFlags = 0;
    if (threadStatus->float04 < 0.0f) {
        threadMode = (threadStatus->float08 < 0.0f) ? 2 : 3;
    } else {
        threadMode = (threadStatus->float08 < 0.0f) ? 2 : 0;
    }
    threadTableFlags = threadTable->tableFlags;
    threadBlock->threadMode = threadMode;
    threadBlock->currentThreadSlot = -1;
    threadBlock->threadEntryTable = threadTable;
    if (threadTableFlags & 0x40000000) {
        threadBlock->threadMode = 0;
    }
    if (slot->slotFlags & 0x100) {
        threadBlock->threadFlags |= 1;
    }
    if (threadTableFlags & 0x20000000) {
        threadBlock->threadFlags |= 2;
    }
    if (gGameLoopRuntime.loopFlags & 0x40000) {
        threadBlock->threadFlags |= 4;
    }
    if (gGameLoopRuntime.loopFlags & 0x80000) {
        threadBlock->threadFlags |= 0x10;
    }
    if (threadTableFlags & 0x2000) {
        threadBlock->threadFlags |= 8;
    }
    sbNativeValueSlotSet(gameSuspendSoundThreads, 1);
}

/* Starts a thread block for the current slot: from the runtime block when flag 0x20000 is set, otherwise from the slot found by id. */
void func_001BDA58() {
    SbThreadBlock *threadBlock;
    SlotEnumView *slot;
    s32 runtimeFlags;
    slot = (SlotEnumView *) RgDispModelFindByKey(0);
    threadBlock = &gSbThreadBlock;
    threadBlock->threadFlags = 0;
    threadBlock->currentThreadSlot = -1;
    threadBlock->threadMode = 0;
    threadBlock->threadEntryTable = 0;
    if (slot->flags & 0x100) {
        threadBlock->threadFlags = 1;
    }
    runtimeFlags = gGameLoopRuntime.loopFlags;
    if (runtimeFlags & 0x40000) {
        threadBlock->threadFlags |= 4;
    }
    if (runtimeFlags & 0x80000) {
        threadBlock->threadFlags |= 0x10;
    }
    if (runtimeFlags & 0x20000) {
        threadBlock->unk_00 = gGameLoopRuntime.unk_160;
        threadBlock->unk_04 = gGameLoopRuntime.unk_15C;
        threadBlock->unk_08[0] = gGameLoopRuntime.unk_160;
        threadBlock->unk_08[1] = gGameLoopRuntime.unk_164;
        threadBlock->unk_08[2] = gGameLoopRuntime.unk_168;
        threadBlock->threadFlags |= 2;
    } else {
        slot = (SlotEnumView *) RgDispModelFindByKey(gGameLoopRuntime.unk_160);
        if (!slot) {
            gGameLoopRuntime.loopFlags &= 0x7ffeffff;
            sbNativeValueSlotPop(0);
            return;
        }
        threadBlock->unk_00 = slot->unk_154;
        threadBlock->unk_04 = gGameLoopRuntime.unk_110;
        func_0030C130(gGameLoopRuntime.unk_114, threadBlock->unk_08, 16);
        threadBlock->threadEntryTable = (SbThreadEntryTable *) slot;
        if (slot->flags & 0x20000000) {
            threadBlock->threadFlags |= 2;
        }
        if (slot->flags & 0x2000) {
            threadBlock->threadFlags |= 8;
        }
    }
    sbNativeValueSlotSet(gameSuspendSoundThreads, 1);
}

void gameActorSetupStep(void);
s32 sceneCallbackReset();
s32 sbThreadsSuspend();
void seVoiceStartDefault(unsigned int handle);

/* Marks the current thread slot, suspends the sound threads and starts the actor setup step. */
void gameSuspendSoundThreads(void) {
    SbThreadEntry *threadEntry;
    s32 threadFlags;
    s32 threadSlot;
    s32 suspendHandle;

    threadEntry = &gSbThreadBlock.threadEntryTable->threadEntries[0];
    if ((gSbThreadBlock.threadEntryTable != 0) && (threadEntry->threadSlot >= 0)) {
        threadFlags = gSbThreadBlock.threadFlags | 0x20;
        gSbThreadBlock.threadFlags = threadFlags;
        threadSlot = threadEntry->threadSlot;
        gSbThreadBlock.currentThreadSlot = threadSlot;
    }
    suspendHandle = sbNativeBufferGet(0);
    sbThreadsSuspend(suspendHandle);
    func_001B7B98(0);
    soundCueSnapshotSave();
    seVoiceStartDefault(0x1F);
    sceneCallbackReset();
    sbNativeValueSlotSet(gameActorSetupStep, 0);
}

s32 func_001BBC68();
s32 gameEnterPlayState(void);
s32 actorSoundStop();
s32 sefBufferOwnedEffects();
s32 xenoActorSetAttrBit8();

/* Stops the actor sound, resets the actor attributes and enters the play state. */
void gameActorSetupStep(void) {
    s32 actorSoundHandle;

    actorSoundHandle = (s32) RgDispModelFindByKey(0);
    actorSoundStop(actorSoundHandle);
    func_001BBC68();
    xenoActorSetAttrBit8(1);
    sefBufferOwnedEffects();
    sefSetSceneMode(0);
    Vibration_Stop();
    gGameLoopRuntime.loopFlags &= ~0x100;
    sbNativeValueSlotSet(gameEnterPlayState, 1);
}

s32 soundDriverCheckStatus();
s32 func_0018FAC8();
s32 func_001B9BF0();
void func_001BDDD8(void);
s32 soundCueSnapshotRestore();
s32 func_001D1108();
s32 cameraStepPending();

/* Waits for the transition, sets up the camera and sound, then continues in func_001BDDD8 (or leaves through func_001BE320 on flag 0x4000000). */
s32 gameEnterPlayState(void) {
    s32 playStateResult;

    playStateResult = func_001B60B8(3);
    if (playStateResult == 0) {
        soundDriverCheckStatus(0x13);
        cameraStepPending(1);
        func_001D1108();
        cameraStepPending(0);
        if (gGameLoopRuntime.loopFlags & 0x04000000) {
            gGameLoopRuntime.loopFlags = gGameLoopRuntime.loopFlags & (~0x100);
            sbNativeValueSlotPop(0);
            return sbNativeValueSlotSet(func_001BE320, 0);
        }
        func_0018FAC8(0);
        func_001B9228();
        func_001B9B88(1);
        func_001B9BF0();
        soundCueSnapshotRestore();
        func_001B7A60(gGameLoopRuntime.comboState, -1, 0, 0x20);
        return sbNativeValueSlotSet(func_001BDDD8, 0);
    }
    return playStateResult;
}


void func_001BDEC8(void);
s32 sbThreadsResume();
void XenoActorApplySourceValue();
void XenoActorSetMode();
s32 sbNativeHandlerMessagePost();

/* Clears the current slot's motion and mode, resumes the threads and sets up the next step (func_001BDEC8). */
void func_001BDDD8(void) {
    ActorSlotRecord *slot;
    ActorMotionRecord *motion;
    SbThreadBlock *threadBlock;
    struct XenoActor *model;

    slot = (ActorSlotRecord *) RgDispModelFindByKey(0);
    motion = &slot->motion;
    threadBlock = &gSbThreadBlock;
    if (slot != 0) {
        model = slot->actor;
        slot->flags &= 0xFFF2FFFF;
        motion->unk_1C = motion->unk_20 = 0.0f;
        motion->unk_92 = 0;
        motion->unk_90 = 0;
        XenoActorApplySourceValue(model);
        XenoActorSetMode(model, 0);
    }
    sbThreadsResume(sbNativeBufferGet(0));
    gGameLoopRuntime.loopFlags &= 0x7FFFFFFF;
    if (gGameLoopRuntime.loopFlags & 0x10000) {
        sbNativeHandlerMessagePost(3, threadBlock->resumeAction, gGameLoopRuntime.unk_160);
    } else {
        sbNativeHandlerMessagePost(3, threadBlock->resumeAction, threadBlock->threadEntryTable->unk_28);
    }
    sbNativeValueSlotSet(func_001BDEC8, 0);
}


void gameStartWaitFrames(void);

/* Resumes the thread entry by its mode, applies the fog override and starts the wait-frames step. */
void func_001BDEC8(void) {
    RuntimeStateBlock *sceneStateBlock;
    ActorSlotRecord *slot;
    SbThreadEntryTable *threadTable;
    SbThreadBlock *threadBlock;
    s32 headEntryWord;
    s32 loopFlag21;

    sceneStateBlock = stateBlockGet();
    slot = (ActorSlotRecord *) RgDispModelFindByKey(0);
    threadBlock = &gSbThreadBlock;
    switch (threadBlock->resumeAction) {
    case 0:
    case 3:
    case 4:
        break;
    case 1:
        threadTable = threadBlock->threadEntryTable;
        if (threadTable != 0) {
            loopFlag21 = (gGameLoopRuntime.loopFlags >> 21) & 1;
            headEntryWord = threadTable->threadEntries[0].entryWord0;
            if (headEntryWord != 0) {
                func_001B8BA0(headEntryWord, loopFlag21, 1);
            }
            RgDispModelSetLocal(threadTable);
        }
        break;
    case 2:
        if ((gGameLoopRuntime.loopFlags & 0x80) == 0) {
            gGameLoopRuntime.loopFlags &= 0xFFFFFEFF;
            xglRenderClearFrame();
            sbNativeValueSlotSet(gameStopTasks, 0);
            sbNativeValueSlotPush(gameResetRuntimeState, 0);
            return;
        }
        break;
    }
    sefSetSceneMode(1);
    sefReplayBufferedEffects();
    if (sceneStateBlock->fogBlock != 0) {
        sefSetFogOverride(sceneStateBlock->fogBlock + 704, sceneStateBlock->fogBlock + 720);
    }
    xenoActorSetAttrBit8(0);
    flatSlotMarkAll();
    kindFourEffectsArm();
    if (slot != 0) {
        func_001C1290(slot, 90);
    }
    sbNativeValueSlotSet(gameStartWaitFrames, 0);
}

/* Sets the wait counter to 30 and the loop flag 0x100, then ends this step. */
void gameStartWaitFrames(void) {
    s32 newLoopFlags;

    newLoopFlags = (gGameLoopRuntime.loopFlags | 0x100) & 0xFFFCFFFF;
    gGameLoopRuntime.waitFrames = 0x1E;
    gGameLoopRuntime.loopFlags = newLoopFlags;
    sbNativeValueSlotPop(0);
}

extern const s32 D_00531378[];

void xglRenderClearDepth();
void soundSlotStop();
void scQueueDefaultRequest();
void gameResetFlagsAndAdvance(void);

/* Applies the map's color to the environment (or clears depth for the 6000 maps) and shuts the movie down. */
const s32 D_00531378[2] = { 0x1E, 0x01 };

void func_001BE060() {
    SceneStateRecord *sceneRecord = &stateBlockGet()->sceneRecord;
    gGameLoopPlacement.loopFlags |= 0x80000000;
    func_001BE470(gGameLoopPlacement.mapId, 1);
    if (gGameLoopPlacement.mapId / 1000 == 6) {
        gGameLoopPlacement.loopFlags |= 0x10000000;
        xglRenderClearDepth();
    } else {
        scratchpadSendCmd0A(D_00531378);
        ((void (*)()) xglRenderClearFrame)();
        gEnvFlags.color.red = gGameLoopPlacement.colorRed;
        gEnvFlags.color.green = gGameLoopPlacement.colorGreen;
        gEnvFlags.color.blue = gGameLoopPlacement.colorBlue;
    }
    if (sceneRecord->sceneState != 4) {
        soundSlotStop(1);
    }
    scQueueDefaultRequest();
    if (!(gGameLoopPlacement.loopFlags & 2)) {
        func_00265E68();
        movieShutdown();
    }
    sbNativeValueSlotSet(gameResetFlagsAndAdvance, 0);
}

s32 sefResetSceneEffects();

/* Clears the scene loop flags and goes back to func_001BC918. */
void gameResetFlagsAndAdvance(void) {
    s32 newLoopFlags;
    s32 newLoopFlagsHigh;

    newLoopFlags = gGameLoopRuntime.loopFlags & (~0xB10);
    newLoopFlagsHigh = gGameLoopRuntime.loopFlagsHigh & 0xFFFF0000;
    gGameLoopRuntime.loopFlags = newLoopFlags;
    gGameLoopRuntime.loopFlagsHigh = newLoopFlagsHigh;
    sefResetSceneEffects();
    njPlaneSetHook(0);
    soundCueReset();
    sbNativeValueSlotSet(func_001BC918, 1);
}

void gameResetFlagsAndWait(void);
s32 func_001B6018();
extern const s32 D_00531380[];

/* Scene-end step: clears the environment color and the movie, then goes to gameResetFlagsAndWait. */
const s32 D_00531380[2] = { 0x1E, 0x01 };

void func_001BE1E0(void) {
    s32 skipShutdown;

    gGameLoopRuntime.loopFlags |= 0x80000000;
    scratchpadSendCmd0A((s32) D_00531380);
    gEnvFlags.color.red = 0;
    gEnvFlags.color.green = 0;
    gEnvFlags.color.blue = 0;
    skipShutdown = gGameLoopRuntime.loopFlags & 2;
    if (!skipShutdown) {
        func_00265E68();
        movieShutdown();
        gGameLoopRuntime.loopFlags &= ~0x10;
    }
    func_001B6018(3);
    sbNativeValueSlotSet(gameResetFlagsAndWait, 0);
}

void gameWaitSceneDone(void);
s32 sefResetEffectWorld();

/* Clears the scene loop flags and waits for the scene to finish. */
void gameResetFlagsAndWait(void) {
    s32 newLoopFlags;
    s32 newLoopFlagsHigh;

    newLoopFlags = gGameLoopRuntime.loopFlags & 0xFCFFF4EF;
    newLoopFlagsHigh = gGameLoopRuntime.loopFlagsHigh & 0xFFFF0000;
    gGameLoopRuntime.loopFlags = newLoopFlags;
    gGameLoopRuntime.loopFlagsHigh = newLoopFlagsHigh;
    sefResetEffectWorld();
    njPlaneSetHook(0);
    sbNativeValueSlotSet(gameWaitSceneDone, 1);
}

/* Returns to gameResetRuntimeState when the slot record count is back to zero. */
void gameWaitSceneDone(void) {
    func_001B5F28(3, 1, 0);
    if (slotRecord1CountDiff() == 0) {
        sbNativeValueSlotSet(gameResetRuntimeState, 0);
    }
}

void gameResetFlagsAndFinish(void);

/* Same as func_001BE1E0 without the loop-flag set; used on the abort path. */
void func_001BE320(void) {
    s32 loopFlags;
    s32 skipShutdown;

    loopFlags = gGameLoopRuntime.loopFlags;
    gEnvFlags.color.red = 0;
    gEnvFlags.color.green = 0;
    gEnvFlags.color.blue = 0;
    skipShutdown = loopFlags & 2;
    if (!skipShutdown) {
        func_00265E68();
        movieShutdown();
        gGameLoopRuntime.loopFlags &= ~0x10;
    }
    func_001B6018(3);
    sbNativeValueSlotSet(gameResetFlagsAndFinish, 0);
}

void gameFinishSceneReset(void);

/* Clears the scene loop flags and finishes in gameFinishSceneReset. */
void gameResetFlagsAndFinish(void) {
    s32 newLoopFlags;
    s32 newLoopFlagsHigh;

    newLoopFlags = gGameLoopRuntime.loopFlags & 0xF8FFF4EF;
    newLoopFlagsHigh = gGameLoopRuntime.loopFlagsHigh & 0xFFFF0000;
    gGameLoopRuntime.loopFlags = newLoopFlags;
    gGameLoopRuntime.loopFlagsHigh = newLoopFlagsHigh;
    sefResetEffectWorld();
    njPlaneSetHook(0);
    sbNativeValueSlotSet(gameFinishSceneReset, 1);
}

s32 mapCollisionInit();

/* Initializes the map collision and returns to gameResetRuntimeState. */
void gameFinishSceneReset(void) {
    func_001B5F28(3, 1, 0);
    mapCollisionInit(0);
    sbNativeValueSlotSet(gameResetRuntimeState, 0);
}

unsigned char D_003E4874[44] = { 0 };
s32 gDebugFlagWords[4] = { 0 };
s32 gSaveBitmapA[32] = { 0 };
unsigned char gSaveFlagBytes[32] = { 0 };
s32 gSaveWordTable[128] = { 0 };
s32 gSaveBitmapB[32] = { 0 };
signed char gSaveRndTable[1728] = { 0 };
char *D_003E5290[4] = { "NORMAL", "TEST", "CHECK", 0 };
s32 D_003E52A0[8] = { 0x1E, 0, 0, 0, 0x80, 0, 0, 0 };

const DebugMenuEntry gDebugMenuEntries[15] = {
    { 1, 0, "SHION" },
    { 2, 0, "JIN" },
    { 3, 0, "MOMO" },
    { 4, 0, "ZIGGY" },
    { 5, 0, "KOSMOS" },
    { 6, 0, "CHAOS" },
    { 7, 0, "JR" },
    { 8, 0, "MIYUKI" },
    { 9, 0, "CANNAN" },
    { 10, 0, "ALLEN" },
    { 13, 0, "ASHER" },
    { 14, 0, "ZEBULUN" },
    { 15, 0, "DINAN" },
    { 16, 0, "REUBEN" },
    { -1, -1, "NO CHANGE" },
};

s32 gDebugMenuRow = 1;
s32 gDebugMenuMode = 0;
s32 gDebugMenuNumber = 0;
s32 gDebugMenuEntryIndex = 0;
