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

struct SaveStack *ov01StackNew(void);
void ov01StackPop(struct SaveStack *stack);
void ov01StackPush(struct SaveStack *stack, SaveStackStep step);
void ov01StepSetNext(struct SaveStack *stack, SaveStackStep nextStep);

struct DataFileRecord {
    int loaded_index;
    char *buffer;
    int size;
};

/* Two adjacent 0x0c-byte file records: the .prj and .vsf of a sound bank (the battle bank bts_se,
 * and the voice of a slot). */
struct DataFileRecordPair {
    struct DataFileRecord prj;
    struct DataFileRecord vsf;
};

/* First two words of every file table (slot, stream entry, entry block): the cursor the next file is read to
 * (dataFileSlotLoad advances it) and the heap block the table owns (the same address, the heap block
 * of ov01HeapBlockOf). */
struct DataFileHead {
    char *next_buffer;
    union {
        char *buffer;
    } base;
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
struct DataSlot {
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
void dataSlotUnload(struct DataSlot *slot);
void dataStatFileLoad(void);
struct DataStreamEntry *dataStreamOpen(int count, int small_buffer);
int unitEffectSlotAcquire(int key);
struct DataSlot *dataUnitFileLoad(int id);
void dataCdArenaOpen(void);
/* The path of the battle sound file of an action code, in dataPathBuf; null when the code has none. */
struct BattleUnit;
int dataCdSync(void);
int ov01HeapProbeLargeAlloc(void);
struct DataSlot;
int unitBindActors(struct BattleUnit *unit, struct DataSlot *slot);

/* A unit has three child units (and three slot flags, one per child); the command table has 40 handlers. */
#define UNIT_CHILD_COUNT 3

/* Command-table indices (unitCommandTable in src/ov01/unit_cmd.c) that the unit move and mode handlers store as step status. */
enum {
    UNIT_CMD_MODE_DISPATCH = 0x0A,
    UNIT_CMD_JUMP_EFFECT = 0x0B,    /* unitCmdEva: issues UNIT_CMD_MOVE_CHA_JUMP and effect 0xB */
    UNIT_CMD_HOVER_EFFECT = 0x0C,   /* unitCmdDef: issues UNIT_CMD_MOVE_CHA_HOVER and effect 0xC */
    UNIT_CMD_CODE_WAIT = 0x0E,      /* unitCmdMot */
    UNIT_CMD_MOVE_CHA = 0x10,       /* unitCmdMoveCha */
    UNIT_CMD_MOVE_POS = 0x11,       /* unitCmdMovePos */
    UNIT_CMD_PHASE_0E_WAIT = 0x12,  /* unitCmdPhase0EWait */
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
void unitCmdSet(BattleUnit *object, Ov01Command *command);
struct MonsSlot;
struct MonsSlot *monsSlotCreate(void);
struct DataStreamEntry;
int monsSlotCopyParams(struct MonsSlot *slot, struct DataStreamEntry *stream);
void objEnCurCreateAtOrigin(void);

/* One 0x14-byte record of ov01PartyTable (one per party slot; tgtSelectInit sets up three). The record doubles as the
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
TgtSlot *ov01PartyListGet(void);
void battleStartBonusApply(void);
struct BattleUnit *selectPL(struct BattleUnit *keep);
void ov01StepChainStart(void);
int ov01LoadIsDone(void);
void ov01LoadRequest(struct DataSlot *owner);
void ov01StepBattle(struct SaveStack *context);
struct SbImageHeader;
struct SbImageHeader *ov01ScriptKeyGet(void);
struct SbImageHeader *ov01ScriptImageGet(void);
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
    struct BattleUnit *unit;
    int words[3];
} UnitScriptEvent;

void unitScriptEventSend(int kind, UnitScriptEvent *event);
void monsSlotShow(struct MonsSlot *slot);
int monsTblMapNoGet(void);

/* ptStatusBlock (0x24 bytes) as ptResetStatusBlock resets it and ptRefresh fills
 * it: the ids of the three members on screen and of the leader; memberMask, a
 * copy of ptProgressRecord.memberMask (one bit per member who joined the party);
 * presentMask, one bit per member of the group on screen (front slots 0-6,
 * or back slots 7-9 in split mode; ov01's debug_entry.c edits it as the MEMBER row);
 * and the ptGetClothId cloth id of each of the seven members (ov01's debug_entry.c
 * edits them as cloth ids). */
typedef struct PartyStatusBlock {
    unsigned char memberIds[3];
    unsigned char leaderId;
    unsigned short memberMask;
    unsigned short presentMask;
    int clothIds[7];
} PartyStatusBlock;

PartyStatusBlock *ptGetStatusBlock(void);
/* Handles of the battle setup: the stream buffer and the sound/monster slot handle (ov01StepOpenHandles). */
extern struct DataStreamEntry *ov01StreamBuffer[1];
extern struct MonsSlot *monsSlotHandle;
/* The unit being spawned and its data slot (dataUnitFileLoad), kept between the spawn, load and await steps. */
extern struct DataSlot *ov01CurSlot;
extern BattleUnit *ov01CurUnit;
/* Party and enemy spawn cursors and their record lists (ov01PartyListGet, monsTblGet; both lists hold TgtSlot spawn records). */
extern int ov01PartyIndex;
extern int ov01EnemyIndex;
extern TgtSlot *ov01EnemyList;
extern TgtSlot *ov01PartyList;
/* Load thread stack (ov01LoadBlockStart, ov01LoadRequest), load-done flag and owner of the pending load. */
extern unsigned char loadThreadStack[0x1000];
extern int ov01LoadDone;
extern struct DataSlot *ov01LoadOwner;
void ExitDeleteThread(void);
void ov01LoadBlockStart(void);
void ov01StepLoadPartyMember(struct SaveStack *context);
void ov01StepNextEnemy(struct SaveStack *context);
void ov01StepStartBlockLoad(struct SaveStack *context);
void ov01StepAwaitBlockLoad(struct SaveStack *context);
void ov01StepOpenHandles(struct SaveStack *context);
void ov01StepBindAndShow(struct SaveStack *context);
void ov01StepLoadPartyIds(struct SaveStack *context);
void ov01StepSpawnPartyMember(struct SaveStack *context);
void ov01StepNextPartyMember(struct SaveStack *context);
void ov01StepStartEnemies(struct SaveStack *context);
void ov01StepSpawnEnemy(struct SaveStack *context);
void ov01StepLoadEnemy(struct SaveStack *context);
void ov01StepAwaitEnemyLoad(struct SaveStack *context);
void ov01StepAwaitLoads(struct SaveStack *context);

/*
 * The EE kernel's ThreadParam: ov01LoadBlockStart and ov01LoadRequest fill one on the stack for CreateThread
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
#define OV01_LOAD_THREAD_STACK_SIZE 0x1000
#define OV01_LOAD_THREAD_PRIORITY 8

extern unsigned char mainGpBase[];
int CreateThread(ThreadParam *param);
int StartThread(int thread, void *arg);
void ov01LoadComplete(void);
void ov01LoadCompleteForOwner(void);
extern int ov01SceneDone;
void ov01StepSceneScript(struct SaveStack *context);
void ov01StepLoadBattleFiles(struct SaveStack *context);
void ov01StepAwaitPartyLoad(struct SaveStack *context);

/* Enemies in an enemy list at most; the list ends early at a record with id 0. */
#define OV01_ENEMY_MAX 5
/* streamPlayByKey key of the line a party member says when the battle starts. */
#define BATTLE_START_STREAM 6

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A859A0);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85A20);

/* Step-chain start: a new save stack (ov01StackNew), the first step ov01StepSceneScript,
 * and the scene done flag ov01SceneDone cleared. */
void ov01StepChainStart(void)
{
    ov01StepSetNext(ov01StackNew(), ov01StepSceneScript);
    ov01SceneDone = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85A78);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85A88);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85B38);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85B50);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85C68);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85CB0);

int func_00A92F90(struct SaveStack *context);

void ov01StepAwaitLoads(struct SaveStack *context)
{
    if (func_00A92F90(context)) {
        ov01StackPop(context);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85D30);

void func_00A92CB0(void);
void func_00A9D8E0(void);
void func_00A9DD70(void);
void *func_00A8FA90(void);
int func_001D1A50(void *key);
void func_00A85E10(struct SaveStack *context);
extern unsigned char D_4C8240[];

void ov01StepSceneScript(struct SaveStack *context)
{
    void *key;

    func_00A92CB0();
    func_00A9D8E0();
    *(void (**)(void))(D_4C8240 + 0x190) = func_00A9DD70;
    key = func_00A8FA90();
    if (func_001D1A50(key) == 0) {
        for (;;) {
        }
    }
    sbStartImage(key);
    ov01StepSetNext(context, func_00A85E10);
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85E10);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85E70);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A85FE0);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86108);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86190);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86200);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86268);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A862B0);

void func_00A85D30(struct SaveStack *context);
void func_00A86388(struct SaveStack *context);
int ov01StackSetSlot(struct SaveStack *stack, int slot, int value);

void func_00A86340(struct SaveStack *context)
{
    ov01StepSetNext(context, func_00A86388);
    ov01StackPush(context, func_00A85D30);
    ov01StackSetSlot(context, 0, 60);
}

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86388);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A863C0);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86408);

INCLUDE_ASM("asm/nonmatchings/ov02/scene_load", func_00A86490);
