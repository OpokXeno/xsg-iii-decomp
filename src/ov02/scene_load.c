#include "common.h"
#include "shared.h"

struct XenoActor;
void gifStateEnableEffect(void);
void nmlModelEnableSubWindow0(void);
int sefCheckLoad(void);

#define DATA_LOOKUP_CAPACITY 12 /* items in a DataLookup block */
#define SAVE_STACK_LEVEL_CELLS 5  /* cells per SaveStack level */
#define SAVE_STACK_LEVELS 4       /* levels of a SaveStack (a push to this depth is refused) */

struct SaveStack;

#include "main/res_get_path.h"
#include "main/tu105.h"
#include "ov02/tu012.h"
#include "ov02/tu030.h"
#include "ov02/tu060.h"

struct SaveStack *battleStackNew(void);
void battleStackPop(struct SaveStack *stack);
void battleStackPush(struct SaveStack *stack, SaveStackStep step);
void battleStackSetSuccessorStep(struct SaveStack *stack, SaveStackStep nextStep);

struct DataFileRecord {
    int loaded_index;
    char *fileBuffer;
    int fileSize;
};

/* Two adjacent 0x0c-byte file records: the .prj and .vsf of a sound bank (the battle bank bts_se,
 * and the voice of a slot). */
struct DataFileRecordPair {
    struct DataFileRecord prj;
    struct DataFileRecord vsf;
};

/* First two words of every file table (slot, stream entry, entry block): the cursor the next file is read to
 * (dataFileSlotLoad advances it) and the heap block the table owns (the same address, the heap block
 * of battleHeapBlockOf). */
struct DataFileHead {
    char *nextFileCursor;
    union {
        char *heapBuffer;
    } heapBlock;
};

/* The four files of a model: pxy, txy, xhr, epf (0x30 bytes). */
struct DataModelFiles {
    struct DataFileRecord pxy;
    struct DataFileRecord txy;
    struct DataFileRecord xhr;
    struct DataFileRecord epf;
};

/* A motion file record and the 0x0c bytes after it that nothing reads. */
struct DataMotionFile {
    struct DataFileRecord file;
    unsigned char unmodeled_0C[0xC];
};

/* The voice files of a slot at +0x130: the prj record (loaded_index is the voice) and the vsf record. */
struct BattleUnitFiles {
    struct DataFileHead head;
    int id;
    int handle;
    /* Up to four models, one DataModelFiles from +0x10 (index 0 is the unit itself) and one motion file from +0xD0. */
    struct DataModelFiles models[4];
    struct DataMotionFile motions[4];
    struct DataFileRecordPair voice_files;
};

/* One map of a stream: the pxy and txy files (0x18 bytes). */
struct DataMapFiles {
    struct DataFileRecord pxy;
    struct DataFileRecord txy;
};

/* A stream: the file table head, its key and its maps from +0x0C. */
struct DataStreamEntry {
    struct DataFileHead head;
    int key;
    struct DataMapFiles maps[1];
};

void dataBattleSeLoad(void);
void dataBattleSeRelease(void);
void dataFaceFileLoad(void);
void dataSlotUnload(struct BattleUnitFiles *slot);
void dataStatFileLoad(void);
struct DataStreamEntry *dataStreamOpen(int count, int small_buffer);
int unitEffectSlotAcquire(int key);
struct BattleUnitFiles *dataUnitFileLoad(int id);
void dataCdArenaOpen(void);
/* The path of the battle sound file of an action code, in dataPathBuf; null when the code has none. */
struct BattleUnit;
int dataCdSync(void);
int battleHeapProbeLargeAlloc(void);
struct BattleUnitFiles;
int unitBindActors(struct BattleUnit *unit, struct BattleUnitFiles *slot);

/* A unit has three child units (and three slot flags, one per child); the command table has 40 handlers. */
#define UNIT_CHILD_COUNT 3

/* Command-table indices (unitCommandTable in X2 src/ov01/unit_cmd.c) that the unit move and mode handlers store as step status. */
enum {
    UNIT_CMD_MODE_DISPATCH = 0x0A,
    UNIT_CMD_JUMP_EFFECT = 0x0B,    /* unitCmdEva: issues UNIT_CMD_MOVE_CHA_JUMP and effect 0xB */
    UNIT_CMD_HOVER_EFFECT = 0x0C,   /* unitCmdDef: issues UNIT_CMD_MOVE_CHA_HOVER and effect 0xC */
    UNIT_CMD_CODE_WAIT = 0x0E,      /* unitCmdMot */
    UNIT_CMD_MOVE_CHA = 0x10,       /* unitCmdMoveCha */
    UNIT_CMD_MOVE_POS = 0x11,       /* unitCmdMovePos */
    UNIT_CMD_PHASE_0E_WAIT = 0x12,  /* unitCmdSoundWaitRefresh */
    UNIT_CMD_POSE_ARRIVE = 0x14,    /* unitCmdPoseArrive */
    UNIT_CMD_POSE_ARRIVE_WAIT = 0x15, /* unitCmdPoseArriveWait */
    UNIT_CMD_MOVE_CHA_RUN = 0x16,
    UNIT_CMD_MOVE_POS_JUMP = 0x17,
    UNIT_CMD_MOVE_CHA_JUMP = 0x18,
    UNIT_CMD_MOVE_CHA_HOVER = 0x19,
    UNIT_CMD_MOVE_POS_HOVER = 0x1A,
    UNIT_CMD_MODE_RAMP = 0x1B,
    UNIT_CMD_MOVE_RATIO = 0x1C,
    UNIT_CMD_MOVE_RAMP = 0x1D,      /* unitCmdMoveRamp */
    UNIT_CMD_FINISH = 0x1E,         /* unitCmdFinish */
    UNIT_CMD_PHASE_2C_WAIT = 0x1F,  /* unitCmdBted */
    UNIT_CMD_CLEAR_ACTIVE = 0x20,
    UNIT_CMD_PHASE_2B_WAIT = 0x21,
    UNIT_CMD_DEACTIVATE = 0x22,     /* unitCmdStat: clears UNIT_FLAG_ACTIVE */
    UNIT_CMD_TURN_AIM = 0x23,       /* unitCmdTurn */
    UNIT_CMD_STREAM_PLAY = 0x24,    /* unitCmdStreamPlay */
    UNIT_CMD_FINISH_REFRESH = 0x26, /* unitCmdFinishRefresh */
    UNIT_CMD_MODE_START = 0x27
};

struct FadeTask;
struct FadeTask *unitFadeIn(BattleUnit *object);
BattleUnit *unitPlCreate(int id);
BattleUnit *unitEnCreate(int id);
void unitLoadSnapshot(BattleUnit *object, int id);
void unitSetAngleDegrees(BattleUnit *object, int degrees);
void unitCmdSet(BattleUnit *object, BattleUnitCommand *command);
struct MonsActorRecord;
struct MonsActorRecord *monsterSpawn(void);
struct DataStreamEntry;
int monsSlotCopyParams(struct MonsActorRecord *slot, struct DataStreamEntry *stream);
void objEnCurCreateAtOrigin(void);

/* One 0x14-byte record of battlePartyTable (one per party slot; tgtSelectInit sets up three). The record doubles as the
 * party spawn record: scene_load spawns the unit charaId at position and turns it to direction, and the debug config and
 * the plPosInit/plDirInit natives edit the same words. */
typedef struct TgtSlot TgtSlot;

struct TgtSlot {
    int charaId;                            /* +0x00: party member id of the slot (the PLAYERn row of the debug config) */
    int position;                           /* +0x04: spawn position index; tgtSelectInit numbers the slots 0 to 2 */
    int direction;                          /* +0x08: facing in degrees 0..360; starts at 180 */
    int spawnArgC;                          /* +0x0C */
    int spawnArgD;                          /* +0x10 */
};

int cfEventGet(void);
TgtSlot *battlePartyListGet(void);
void battleStartBonusApply(void);
struct BattleUnit *selectPL(struct BattleUnit *keep);
void battleStepChainStart(void);
int battleLoadIsDone(void);
void battleLoadRequest(struct BattleUnitFiles *owner);
void battleStepRunCombat(struct SaveStack *saveStack);
struct SbImageHeader;
struct SbImageHeader *battleScriptKeyGet(void);
struct SbImageHeader *battleScriptImageGet(void);
struct EffectPosRecord;
void effectPosTableSet(struct EffectPosRecord *table);
int sbLibraryLink(struct SbImageHeader *image, struct SbImageHeader *library);
void sbStartImage(struct SbImageHeader *image);
void *monsTblGet(void);
struct EffectPosRecord *posDataGet(void);
void runtimeInit(void);
int bgmIndexGet(void);
void menuStatPreObjCreate(void);
void menuStockObjCreate(void);
void menuBoostObjCreate(void);
void menuKeyGuideObjCreate(void);
void menuBoostGaugeCreate(void);
void menuStatRecordsCreate(void);
void actStepIconCreate(void);
void menuWepSpPanelNew(void);
void menuTargetListNew(void);
void menuTgtStatusNew(void);
void menuUnitPanelNew(void);
void menuIconListNew(void);
void menuComboPanelNew(void);
void menuPanelCreate(void);
int handleStateGet(unsigned int handle);
void seVoiceStartTagged(int tag, int id);
void bgmPlayTrack(int track);
void streamPlayByKey(struct BattleUnit *unit, int mode);

/* The 16-byte record unitScriptEventSend takes: the unit and three words it passes on, with the selector, as a script
 * message (sbThreadEnqueue). */
typedef struct UnitScriptEvent {
    struct BattleUnit *messageUnit;
    int messageArgs[3];
} UnitScriptEvent;

void unitScriptEventSend(int kind, UnitScriptEvent *event);
void monsSlotShow(struct MonsActorRecord *slot);
int monsTblMapNoGet(void);

/* ptStatusBlock (0x24 bytes) as ptResetStatusBlock resets it and ptRefresh fills
 * it: the ids of the three members on screen and of the leader; memberMask, a
 * copy of ptProgressRecord.memberMask (one bit per member who joined the party);
 * presentMask, one bit per member of the group on screen (front slots 0-6,
 * or back slots 7-9 in split mode; X2 ov01's debug_entry.c edits it as the MEMBER row);
 * and the ptGetClothId cloth id of each of the seven members (X2 ov01's debug_entry.c
 * edits them as cloth ids). */
typedef struct PartyStatusBlock {
    unsigned char memberIds[3];
    unsigned char leaderId;
    unsigned short memberMask;
    unsigned short presentMask;
    int clothIds[7];
} PartyStatusBlock;

PartyStatusBlock *ptGetStatusBlock(void);
/* Handles of the battle setup: the stream buffer and the sound/monster slot handle (battleStepOpenHandles). */
extern struct DataStreamEntry *battleStreamBuffer[1];
extern struct MonsActorRecord *monsSlotHandle;
/* The unit being spawned and its data slot (dataUnitFileLoad), kept between the spawn, load and await steps. */
extern struct BattleUnitFiles *battleCurSlot;
extern BattleUnit *battleCurUnit;
/* Party and enemy spawn cursors and their record lists (battlePartyListGet, monsTblGet; both lists hold TgtSlot spawn records). */
extern int battlePartyIndex;
extern int battleEnemyIndex;
extern TgtSlot *battleEnemyList;
extern TgtSlot *battlePartyList;
/* Load thread stack (battleLoadBlockStart, battleLoadRequest), load-done flag and owner of the pending load. */
extern unsigned char loadThreadStack[0x1000];
extern int battleLoadDone;
extern struct BattleUnitFiles *battleLoadOwner;
void ExitDeleteThread(void);
void battleLoadBlockStart(void);
void battleStepLoadPartyMember(struct SaveStack *saveStack);
void battleStepNextEnemy(struct SaveStack *saveStack);
void battleStepStartBlockLoad(struct SaveStack *saveStack);
void battleStepAwaitBlockLoad(struct SaveStack *saveStack);
void battleStepOpenHandles(struct SaveStack *saveStack);
void battleStepBindAndShow(struct SaveStack *saveStack);
void battleStepLoadPartyIds(struct SaveStack *saveStack);
void battleStepSpawnPartyMember(struct SaveStack *saveStack);
void battleStepNextPartyMember(struct SaveStack *saveStack);
void battleStepStartEnemies(struct SaveStack *saveStack);
void battleStepSpawnEnemy(struct SaveStack *saveStack);
void battleStepLoadEnemy(struct SaveStack *saveStack);
void battleStepAwaitEnemyLoad(struct SaveStack *saveStack);
void battleStepAwaitLoads(struct SaveStack *saveStack);

/*
 * The EE kernel's ThreadParam: battleLoadBlockStart and battleLoadRequest fill one on the stack for CreateThread
 * (syscall 0x20, main 0x00120320) and start the thread with StartThread (syscall 0x22). The entry ends with
 * ExitDeleteThread (syscall 0x24).
 */
typedef struct ThreadParam {
    int status;
    void (*entry)(void);      /* +0x04 */
    void *stack;              /* +0x08 */
    int stackSize;            /* +0x0C */
    void *gpReg;              /* +0x10 */
    int initPriority;         /* +0x14 */
    int currentPriority;      /* +0x18 */
    unsigned int attr;        /* +0x1C */
    unsigned int option;      /* +0x20 */
    int waitType;             /* +0x24 */
    int waitId;               /* +0x28 */
    int wakeupCount;          /* +0x2C */
} ThreadParam;

/* Stack size and priority of the load threads; their stack is loadThreadStack, their gp the main gp value mainGpBase. */
#define BATTLE_LOAD_THREAD_STACK_SIZE 0x1000
#define BATTLE_LOAD_THREAD_PRIORITY 8

extern unsigned char mainGpBase[];
int CreateThread(ThreadParam *param);
int StartThread(int thread, void *arg);
void battleLoadComplete(void);
void battleLoadCompleteForOwner(void);
extern int battleSceneDone;
void battleStepSceneScript(struct SaveStack *saveStack);
void battleStepLoadUnitFiles(struct SaveStack *saveStack);
void battleStepAwaitPartyLoad(struct SaveStack *saveStack);

/* Enemies in an enemy list at most; the list ends early at a record with id 0. */
#define BATTLE_ENEMY_MAX 5
/* streamPlayByKey key of the line a party member says when the battle starts. */
#define BATTLE_START_STREAM 6

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A859A0);

typedef int s32;

/* One 0x10-byte spawn record of the party and enemy lists (battleStepPlaceUnits walks them 0x10 bytes apart). */
typedef struct SpawnRecord {
    int charaId;
    int position;
    int direction;
    f32 scale;
} SpawnRecord;

/* The scene state block (sceneStateGet, 0x6C bytes): sceneStateSpinWait counts spinCount down at +0x5C. */
typedef struct SceneStateBlock {
    unsigned char pad_00[0x5C];
    s32 spinCount;
    unsigned char pad_60[0xC];
} SceneStateBlock;

/* The script block at gSceneScriptHandlerBlock: battleStepSceneScript stores the script handler at +0x190. */
typedef struct SceneScriptBlock {
    unsigned char pad_00[0x190];
    void (*scriptHandler)(void);
} SceneScriptBlock;
s32 sceneScriptNoOp();
s32 soundSlotTableClear();

void sceneSetupPair(void) {
    sceneScriptNoOp();
    soundSlotTableClear();
}

/* Step-chain start: a new save stack (battleStackNew), the first step battleStepSceneScript,
 * and the scene done flag battleSceneDone cleared. */
void battleStepChainStart(void)
{
    battleStackSetSuccessorStep(battleStackNew(), battleStepSceneScript);
    battleSceneDone = 0;
}

s32 battleSceneIsDone(void) {
    return battleSceneDone;
}

s32 sefSetSceneMode(s32);
s32 gifStateResetTimings();
s32 battleEnterModeZero();
s32 sceneSystemsInit();
s32 advanceInit();
s32 func_00A81558();
s32 ov02RecordQueueClear();
s32 ov02RecordNoOp();
s32 sceneStateReset();
s32 monsReadyCountReset();
s32 func_00A88B50();
s32 targetingSlotTablesReset();
s32 battleTablesReset();
s32 battleVPadReset();
s32 sbScriptImageInit();
s32 monsCommandQueuesInit();
s32 battleEntryClearPools();
s32 battleResetCallbacksAndTables();
s32 unitInit();

void battleSceneInit(void) {
    battleEnterModeZero();
    sceneSystemsInit();
    sefSetSceneMode(0);
    gifStateResetTimings();
    battleResetCallbacksAndTables();
    monsReadyCountReset();
    battleVPadReset();
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
    battleStepChainStart();
}

s32 func_00A88A18();

void sceneStartSubsystem(void) {
    func_00A88A18();
}

s32 calcPara2Org();
s32 srsReleaseEffectRecordsByKey(s32, s32);
s32 sceneHookClear();
s32 func_00A815B0();
void unitReleaseResources(BattleUnit *unit);
s32 monsActiveSlotsGet(struct MonsActorRecord ***table);
s32 monsterRelease(struct MonsActorRecord *slot);
s32 battleClearTaskAndSlotTables();
s32 sbThreadsRelease(void *);
void sefSetFogOverride(Vector4 *fogParams, Vector4 *fogColor);
s32 unitTblGet(s32, BattleUnit ***table);
s32 xglSleep();

void *sbScriptImageGet(void);

void battleSceneTeardown(void) {
    BattleUnit **activeUnits;
    struct MonsActorRecord **monster_slot_list;
    BattleUnit *activeUnit;
    struct MonsActorRecord *monster_slot;
    s32 activeCount;
    s32 activeIndex;
    void *thread_table;

    activeIndex = 0;
    srsReleaseEffectRecordsByKey(0x1003200A, 1);
    sefSetFogOverride(0, 0);
    thread_table = sbScriptImageGet();
    sbThreadsRelease(thread_table);
    activeCount = unitTblGet(2, &activeUnits);
    if (activeCount > 0) {
        do {
            activeUnit = activeUnits[activeIndex];
            activeIndex += 1;
            if (activeUnit != 0) {
                unitReleaseResources(activeUnit);
            }
        } while (activeIndex < activeCount);
    }
    activeIndex = 0;
    activeCount = monsActiveSlotsGet(&monster_slot_list);
    if (activeCount > 0) {
        do {
            monster_slot = monster_slot_list[activeIndex];
            activeIndex += 1;
            if (monster_slot != 0) {
                monsterRelease(monster_slot);
            }
        } while (activeIndex < activeCount);
    }
    battleClearTaskAndSlotTables();
    sceneHookClear();
    calcPara2Org();
    func_00A815B0();
    xglSleep();
    xglSleep();
}

SceneStateBlock *sceneStateGet(void);

void sceneStateSpinWait(void) {
    s32 spin_count;

    spin_count = sceneStateGet()->spinCount;
    if (spin_count > 0) {
        do {
            spin_count -= 1;
        } while (spin_count != 0);
    }
}

s32 func_001B60B8(s32);

void battleStepAwaitSoundIdle(struct SaveStack *saveStack) {
    if (func_001B60B8(3) == 0) {
        battleStackPop(saveStack);
    }
}

int soundSlotCurrentIdle(struct SaveStack *saveStack);

void battleStepAwaitLoads(struct SaveStack *saveStack)
{
    if (soundSlotCurrentIdle(saveStack)) {
        battleStackPop(saveStack);
    }
}

void battleStepWaitFrames(struct SaveStack *saveStack);
s32 battleStackGetSlot();
int battleStackSetSlot(struct SaveStack *stack, int slot, int value);

void battleStepWaitFrames(struct SaveStack *saveStack) {
    s32 frames_left;

    frames_left = battleStackGetSlot(saveStack, 0);
    if (frames_left <= 0) {
        battleStackPop(saveStack);
        return;
    }
    battleStackSetSlot(saveStack, 0, frames_left - 1);
}

void func_00A92CB0(void);
void func_00A9D8E0(void);
void func_00A9DD70(void);
int sbImageLinkImports(void *key);
void func_00A85E10(struct SaveStack *saveStack);
extern SceneScriptBlock gSceneScriptHandlerBlock[];

void battleStepSceneScript(struct SaveStack *saveStack)
{
    void *script_key;

    func_00A92CB0();
    func_00A9D8E0();
    gSceneScriptHandlerBlock->scriptHandler = func_00A9DD70;
    script_key = sbScriptImageGet();
    if (sbImageLinkImports(script_key) == 0) {
        for (;;) {
        }
    }
    sbStartImage(script_key);
    battleStackSetSuccessorStep(saveStack, func_00A85E10);
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85E10);

s32 func_00A81618(s32);
void func_00A85FE0(struct SaveStack *saveStack);
void *partyRecordTableGet();
void *enemyRecordTableGet();
s32 func_00A8F490(s32, s32);
s32 sbThreadPostValue(s32, s32);
s32 unitPlayerCreate(s32);
s32 unitEnemyCreate(s32);
s32 unitSetPositionPercent(s32, s32, s32);
s32 unitSetTargetPercent(s32, s32, s32);
s32 unitSetFacing(s32, f32);
s32 unitSetDrawFlagTree(s32);
s32 unitRefreshPhase(s32);

void battleStepPlaceUnits(struct SaveStack *saveStack) {
    s32 party_chara_id;
    s32 enemy_chara_id;
    s32 party_unit;
    s32 enemy_unit;
    s32 party_index;
    s32 enemy_index;
    SpawnRecord *party_record;
    SpawnRecord *enemy_record;

    party_record = partyRecordTableGet();
    for (party_index = 0; party_index < 3; party_index++) {
        party_chara_id = party_record->charaId;
        if (party_chara_id != 0) {
            party_unit = unitPlayerCreate(party_chara_id);
            unitSetTargetPercent(party_unit, party_record->position, party_record->direction);
            unitSetPositionPercent(party_unit, party_record->position, party_record->direction);
            unitSetFacing(party_unit, party_record->scale);
            if (func_00A8F490(party_unit, party_chara_id) != 0) {
                unitRefreshPhase(party_unit);
                unitSetDrawFlagTree(party_unit);
            }
        }
        party_record += 1;
    }
    enemy_record = enemyRecordTableGet();
    for (enemy_index = 0; enemy_index < 5; enemy_index++) {
        enemy_chara_id = enemy_record->charaId;
        if (enemy_chara_id != 0) {
            enemy_unit = unitEnemyCreate(enemy_chara_id);
            unitSetTargetPercent(enemy_unit, enemy_record->position, enemy_record->direction);
            unitSetPositionPercent(enemy_unit, enemy_record->position, enemy_record->direction);
            unitSetFacing(enemy_unit, enemy_record->scale);
            if (func_00A8F490(enemy_unit, enemy_chara_id) != 0) {
                unitRefreshPhase(enemy_unit);
                unitSetDrawFlagTree(enemy_unit);
            }
        }
        enemy_record += 1;
    }
    sbThreadPostValue(3, 0);
    func_00A81618(0);
    func_00A81618(1);
    battleStackSetSuccessorStep(saveStack, func_00A85FE0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85FE0);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86108);

extern s32 gBattleReadyCountBase;
s32 stateIsReady();
void func_00A86200(struct SaveStack *saveStack);
s32 monsReadyCountGet();

void battleStepAwaitReadyThreshold(struct SaveStack *saveStack) {
    s32 readyCountBase;

    if (stateIsReady() != 0) {
        readyCountBase = gBattleReadyCountBase;
        if (monsReadyCountGet() >= (readyCountBase + 0x2D)) {
            battleStackSetSuccessorStep(saveStack, func_00A86200);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86200);

s32 advanceUpdate();
void func_00A862B0(struct SaveStack *saveStack);

void battleStepAwaitContextUpdate(struct SaveStack *saveStack) {
    if (advanceUpdate() != 0) {
        battleStackSetSuccessorStep(saveStack, func_00A862B0);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A862B0);

void battleStepWaitFrames(struct SaveStack *saveStack);
void battleStepLoadScratchpadParams(struct SaveStack *saveStack);
int battleStackSetSlot(struct SaveStack *stack, int slot, int value);

void battleStepStartFrameWait(struct SaveStack *saveStack)
{
    battleStackSetSuccessorStep(saveStack, battleStepLoadScratchpadParams);
    battleStackPush(saveStack, battleStepWaitFrames);
    battleStackSetSlot(saveStack, 0, 60);
}

extern s32 gSceneScratchpadParams;
s32 scratchpadSendCmd23Mode1(s32 *);
void battleStepAwaitPendingCount();

void battleStepLoadScratchpadParams(struct SaveStack *saveStack) {
    scratchpadSendCmd23Mode1(&gSceneScratchpadParams);
    battleStackSetSuccessorStep(saveStack, battleStepAwaitPendingCount);
}

s32 slotRecord2CountDiff();
void func_00A86490(struct SaveStack *saveStack);

void battleStepAwaitPendingCount(struct SaveStack *saveStack) {
    if (slotRecord2CountDiff() < 3) {
        battleStackSetSuccessorStep(saveStack, func_00A86490);
    }
}

s32 advanceStatusGet();
s32 dataVPadReadButtons(s32);
s32 unitCountFlagged(s32);

void battleStepCheckUnitsReady(struct SaveStack *saveStack) {
    if ((unitCountFlagged(2) == 0) || ((advanceStatusGet() == 1) && ((dataVPadReadButtons(1) == 0x20) || (dataVPadReadButtons(1) == 0x40)))) {
        battleStackSetSuccessorStep(saveStack, func_00A86490);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86490);
