#include "common.h"
#include "shared.h"

struct XenoActor;

void sefCreateEffect(void *object);
int sefGetEntryFlags(struct XenoActor *actor);
int sefIsHitActor(struct XenoActor *actor);

#include "main/res_get_path.h"
#include "main/tu352.h"
#include "main/xenoActor.h"
#include "main/xgl_render.h"
#include "ov02/tu012.h"
#include "ov02/tu060.h"
#include "ov02/tu062.h"

/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyModelHeader *), the texture (.txy), the
 * hierarchy (.xhr, XenoActorAllocMatrices builds the matrices from it), the motion tables (.xap: slot n is
 * XAct_Rsrc_XAP + n, slots 0..5 = indices 4..9) and a few entries xenoActor.c reads as pointers. The names of the
 * first seven are those of the original's asserts; no assert names the rest. */
enum XActRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,                            /* entry passed to tiMRPackageSetAnimData */
    XAct_Rsrc_XAP,                          /* OffsetTable * of slot 0 */
    XAct_Rsrc_5,
    XAct_Rsrc_XAP_Ext2,
    XAct_Rsrc_7,
    XAct_Rsrc_SlotEntry,                    /* slot 4, read as an int entry (the motion set's second entry) */
    XAct_Rsrc_SlotTableB,                   /* slot 5, read as another table */
    XAct_Rsrc_PAM,                          /* non-zero when a PAM pose is active */
    XAct_Rsrc_EntryTableC,                  /* tiMRPackage range entry C */
    XAct_Rsrc_Count
};

#define DATA_LOOKUP_CAPACITY 12 /* items in a DataLookup block */

void *objCmdPtrGet(struct DataLookup *lookup);
void *objCmdNext(struct DataLookup *lookup);
/* The path of the battle sound file of an action code, in dataPathBuf; null when the code has none. */
struct BattleUnit;

typedef struct TecRecord TecRecord;

/* The 0x20-byte technique record tecRecordGet returns for an action index. */
struct TecRecord {
    /* +0x00: target flags (1, 2, 3 side, 4 self, 8 all, 0x10 flagged units); bit 0x20 is tested with
     * calcStatGet(unit, 4, 0x2000) */
    unsigned char flags;
    signed char code;                       /* +0x01: MMathActorsInRangeByKind's code; must be 0 for calcStatusHitRoll */
    short scale;                            /* +0x02: percent scale battleEffectApplies passes to MMathActorsInRangeByKind */
    signed char rollThreshold;              /* +0x04: a first roll below this goes to the second roll */
    signed char secondRollThreshold;        /* +0x05: a second roll below this returns 0 */
    signed char type;                       /* +0x06: compared with 2 by calcHitDmg */
    union {
        signed char hitKind;                   /* kind byte, mapped by calcHit and calcHitItem */
        unsigned char effectFlags;          /* the same byte as flags 0x1 to 0x40, read by calcUseItem */
        signed char commandSlot;                   /* the slot cmdTechniqueAttackQueue (cmd_exec.c) accepts below 8 */
    } kindBits;                             /* +0x07 */
    unsigned short hitFlags;                /* +0x08: bits 0x4, 0x8, 0x10 and 0x20 are tested; 0x100 sets object
                                             * flag 0x2000 (unitCmdSetPhase) */
    signed char epCost;                     /* +0x0A: the EP cost calcTakeEpGet scales */
    signed char levelDelta;                 /* +0x0B: unitRecordStateGet's result */
    /* +0x0C: tested against 0x100, 0x200, 0x500 and 0x600 by calcHitStateStep; the low five bits are
     * calcComboUpdate's mask */
    unsigned short actionBits;
    unsigned char power;                    /* +0x0E */
    signed char modeGuard[3];               /* +0x0F..+0x11: bit 0 of modeGuard[mode] blocks the hit */
    /* +0x12..+0x15: one gate byte per row slot (calcHitRowGate), the unit-state codes battleEffectApplies
     * tests; bit 0x10 of the first clears object flag 0x2000 (unitCmdSetPhase) */
    signed char rowGate[4];
    signed char inflictRate;                /* +0x16: chance the value of calcStatResist is taken from */
    signed char row;                        /* +0x17 */
    unsigned short rowStatusBits;                    /* +0x18: one bit per status of the row */
    short extent;                           /* +0x1A: the extent attackEffectsFinished (cmd_exec.c) passes to
                                             * unitEffectKeyFree */
    /* +0x1C: zero lets calcHitRowGate walk its row gates; unitSlotsClear tests 5, unitAttachRequest 1 */
    signed char gateFlag;
    signed char phase;                      /* +0x1D: the phase unitCmdSetPhase (unit_cmd.c) sets */
    short timer;                            /* +0x1E: nonzero selects unitSetTimedPhase */
};

TecRecord *tecRecordGet(int index);

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

/* Set by attackHitsShow per attack: the unit's reaction to the hit is queued, and its hit panels are shown;
 * unitHitFlagsClear clears both. */
#define UNIT_FLAG_HIT_REACTED 0x100
#define UNIT_FLAG_HIT_PANELS 0x40000

int unitEffectKeyFree(BattleUnit *unit, int keyHigh, int keyLow);
void unitCmdSet(BattleUnit *object, BattleUnitCommand *command);

/*
 * Hit record of one target: calcHitRate rolls for it and calcHitDmg fills it; the status
 * steps (calcCrit, calcHitStateStep, calcHitRowGate, calcFollowKind) read it.
 */
struct HitRecord {
    struct BattleUnit *targetUnit;                      /* +0x00 */
    int reaction;                           /* +0x04: the UNIT_CMD_* the target answers with (unitJobTypeAtk) */
    int damage;                             /* +0x08: calcDmg result (calcDmgItem for type 2) */
    int followResult;                       /* +0x0c: calcFollowKind result, stored by calcDmg */
    /* +0x10: calcBackAttack's facing test for kind 1; 1 skips calcHitRate and adds 50 to calcCrit's chance */
    int backAttack;
    int stepState;                          /* +0x14: 1 to 3, written by calcHitRowGate and calcHitStateStep */
    union {
        /* calcHit result (calcHitItem for type 2): bit 8 set once taken, bit 9 blocks it */
        int hitResult;
        unsigned char hitKind;                 /* its low byte: the hit kind calcStatAtk tests */
    } hitMode;                              /* +0x18 */
    int inflictRow;                         /* +0x1c: row calcStatAtk inflicts */
    int inflictMask;                        /* +0x20: bits calcStatAtk inflicts */
    int cureRow;                            /* +0x24: row calcStatClr clears */
    int cureMask;                           /* +0x28: bits calcStatClr clears */
};

/*
 * Command record of one attack (16 bytes); tgtFindFirst reads its attacker and action. The cmd_exec.c command
 * handlers (cmdAttackQueueAndSpend builds it, cmdTechniqueAttackQueue, cmdItemAttackQueue, attackEffectsFinished) take one too.
 */
struct AttackCommand {
    struct BattleUnit *attacker;                    /* +0x00 */
    int inflictKind;                               /* +0x04: 4 selects the base chance 200 in calcStatInflictRate */
    int actionIdx;                          /* +0x08: index tecRecordGet takes */
    int id;                                 /* +0x0C: the lookup key (the third argument of calcUseItem) */
};

/*
 * The step counter actStepCounter: step runs 0..3; at step 3 `hold` is a random value of 4 or
 * more. actStepCounterSet draws the multiplier and clears the words from +0xC.
 */
struct ActStepCounter {
    int step;                               /* +0x00 */
    int hold;                               /* +0x04 */
    int multiplier;                         /* +0x08: 2, 4 or 10; monsGainSet's rate at phase 2 */
    union {
        int sharedWord;                      /* +0x0C: cleared by actStepCounterSet */
        int levelGranted;                   /* same word: set once calcDmg granted the attacker a level */
        int statPanelState;                 /* same word: menuUnitPanelOpen creates the stat panel while it is 1, then sets 2 */
    };
    int bonusFlag;                          /* +0x10: set by calcCrit's bonus roll */
    int stepState;                          /* +0x14: 1 to 3, set by calcHitStateStep and calcHitRowGate */
};

struct ActStepCounter *actStepCounterGet(void);

/* A unit command: six words, copied whole into the command list (cmdListSet) and into kind-1 events
 * (eventQueueAddUnitCommand). command is its index; the pair command of cmdPairRun names its pair record in pairIndex. */
typedef struct CmdListBody {
    int command;                        /* +0x00 */
    unsigned char unmodeled_04[0x4];
    int code;                           /* +0x08: the key-row entry, technique or argument thinkDispatch passes */
    /* +0x0C: the target unit of the think commands as a word, like CmdListEntry.motionActor (think command 14 stores 1
     * here). One int member, not a union of a pointer and an int: a store through a union conflicts with every
     * load, and cmdExecute's original loads unit->stats before its arg.target store reaches memory. */
    struct {
        int thinkTargetUnit;
    } commandTarget;
    int pairIndex;                      /* +0x10: the pair record unitPairPartnerFind reads; the battle lookup of
                                         * think command 3 */
    int attackKind;                     /* +0x14: copied into AttackCommand.kind by cmdFindFirstTarget */
} CmdListBody;

/* Command kinds (CmdListBody.command): each is the unitCommandTable handler (unit_cmd.c) the unit runs once cmdExecute
 * has queued the command. */
enum {
    CMD_KIND_NONE = 0,                  /* unitCmdNoop */
    CMD_KIND_TECHNIQUE = 1,             /* unitCmdSetPhase: technique `code` on the target */
    CMD_KIND_WAIT = 2,                  /* unitCmdStatEffect: the recovery of a waiting unit */
    CMD_KIND_LOOKUP = 3,                /* unitCmdAction: the technique of battle lookup record pairIndex */
    CMD_KIND_ITEM = 4,                  /* unitCmdPlayVoice: item (hit action record) pairIndex */
    CMD_KIND_SWAP = 5,                  /* unitCmdRelease: the unit leaves and cmdExecute spawns unit `code` in its
                                         * place (unitSpawnFromRequest) */
    CMD_KIND_SNAPSHOT = 6,              /* unitCmdLoadSnapshot: the unit fades over to snapshot `code` */
    CMD_KIND_LEAVE = 7,                 /* unitCmdSetBattleTransition: every unit of the side fades out of the battle */
    CMD_KIND_REFRESH = 8,               /* unitCmdRefreshPhase */
    CMD_KIND_THINK_END = 9,             /* unitCmdUnbind: the end of a unit's think step (thinkSendEnd) */
    CMD_KIND_CODE_WAIT = 14,            /* unitCmdMot: sets phase `code`, then waits for unitIsDone when the
                                         * wait flag (commandTarget) is set */
    CMD_KIND_REMOVE = 37                /* unitCmdRemove: unitRemove */
};

/* The four-word payload of eventQueueAdd. */
typedef struct EventQueueArgs {
    union {
        int actions;                    /* kind 3: the kind-4 records of the attack */
        BattleUnit *commandingUnit;               /* the commanding unit (cmdExecute); kind 19 compares it with its own unit */
        int countdown;                  /* kind 20: the steps eventWaitCountdown has left */
    };                                  /* +0x00 */
    union {
        int tecIndex;                   /* the tec whose EP cost cmdStockRun charges */
        int hits;                       /* kind 3: the kind-5 records (targets) of the attack */
    };                                  /* +0x04 */
    int pairIndex;                      /* +0x08: the command's pair record (CmdListBody.pairIndex, pairLinkGet) */
    BattleUnit *partnerUnit;                 /* +0x0C: the partner / stocked unit of events 0x10 and 0x12 */
} EventQueueArgs;

/* One 0x30-byte record of eventQueueTable (eventQueueTable); kind is its first word,
 * the payload layout depends on the appender. */
typedef struct EventQueueEntry {
    int eventKind;
    union {
        EventQueueArgs addArgs;          /* eventQueueAdd */
        struct AttackCommand attack;  /* eventQueueAddAttackAction: the attack (unitJobTypeAtk) */
        struct {
            BattleUnit *commandUnit;
            CmdListBody commandRecord;
        } unitCommandEvent;                      /* eventQueueAddUnitCommand */
        struct HitRecord attackHitEvent;       /* eventQueueAddAttackHit: the hit record of one target */
    } eventPayload;
} EventQueueEntry;

void unitEffectStart(struct BattleUnit *object, int keyHigh, int keyLow);

/* Effect request block sent to the main-side effect queue (sefCreateEffect /
 * sefStartEffectWithValue); the senders clear it first. */
struct EffectRequest {
    struct XenoActor *ownerActor;            /* +0x00: the actor of the owner unit (BattleUnit.motionActor) */
    struct XenoActor *partner;         /* +0x04: the other unit's actor of a pair technique (attackEffectsStart) */
    struct XenoActor *targetActor;          /* +0x08: the same actor again (cmd_exec.c senders); the hit target's actor */
    struct XenoActor *childActors[4];        /* +0x0C: the child actors of the unit (two each for a pair technique) */
    int key;                           /* +0x1C: (high << 12) | low, see createEffect */
    unsigned char unmodeled_20[0x24 - 0x20];
    short hitMode;                     /* +0x24: the hit record's hitMode (attackEffectsStart) */
    unsigned short hitCode;            /* +0x26: the actionBits byte and the target's row bits (attackEffectsStart) */
    unsigned char unmodeled_28[0x2C - 0x28];
    short effectMode;                        /* +0x2C: 2 or 3 (unitEffectStartMode); the attacker's state
                                        * (attackEffectsStart) */
    short targetUnitState;                 /* +0x2E: the target's state (attackEffectsStart) */
    unsigned char unmodeled_30[0x38 - 0x30];
};

void objDmgNumCreate(struct BattleUnit *object, int amount, int kind, int sign);
struct CmdListBody;
struct BattleUnit *tgtUnitGet(void);
void menuUnitPanelOpen(struct BattleUnit *unit, int mode);
void menuIconListAppend(struct BattleUnit *unit);
void menuComboPanelStart(struct BattleUnit *unit);
void menuStatPanelNew(int kind);

/* menuStatPanelNew kinds attackHitsShow opens for a hit (rows of menuStatRects). */
enum {
    MENU_STAT_PANEL_NO_HIT = 1,         /* hit kind 0 */
    MENU_STAT_PANEL_BLOCKED = 2,        /* hitMode bit 9 */
    MENU_STAT_PANEL_TAKEN = 3,          /* hitMode bit 8 */
    MENU_STAT_PANEL_TAKEN_BONUS = 4,    /* hitMode bit 8 after calcCrit's bonus roll */
    MENU_STAT_PANEL_BACK_ATTACK = 6     /* HitRecord.backAttack */
};

void menuStatPanelOpen(int stepState);
struct FadeOwner;

/* Records of eventQueueTable (eventQueueReserve stops at this count). */
#define EVENT_QUEUE_COUNT 0x40

extern EventQueueEntry eventQueueTable[EVENT_QUEUE_COUNT];
extern int eventQueueCursor;
void *memset(void *destination, int value, unsigned int count);

/* HitRecord.hitMode: the hit kind in the low byte, the flags byte above it. */
#define HIT_MODE_KIND_MASK 0xFF
#define HIT_MODE_FLAGS_SHIFT 8
#define HIT_MODE_FLAGS_MASK 0xFF
/* The hit kinds, as calcHpDmg applies them: no hit (no damage, nothing inflicted), HP and EP loss and gain, two
 * kinds that change neither (calcStatAtk skips HIT_KIND_NO_INFLICT), HP and EP taken over by the attacker
 * (calcDmg: a share of the target's maximum up to its current value, or its maximum but 1), and the kind
 * unitJobTypeAtk gives the hit of a technique without a hit kind. */
#define HIT_KIND_NONE 0
#define HIT_KIND_PLAIN 1
#define HIT_KIND_STATUS 5
#define HIT_KIND_NO_INFLICT 6
/* The flags byte of HitRecord.hitMode (bits 8 to 15): bit 8 taken, bit 9 blocked. */
#define HIT_FLAG_TAKEN 0x1
#define HIT_FLAG_BLOCKED 0x2

int attackSefFlag1Check(BattleUnit *attacker, int kind);
void scenarioBatExecDst(BattleUnit **faceTarget, struct HitRecord *hit);
int tecEffectGroupGet(BattleUnit *unit, TecRecord *tec);
int attackEffectsFinished(struct AttackCommand *owner);
void hitPopNumCreate(struct AttackCommand *command, struct HitRecord *hit);
EventQueueEntry *eventQueueFindAttackHit(int ordinal);
int attackHitsShow(EventQueueEntry *entry, struct AttackCommand *command);

/* Bit of the actor's sef entry flags (sefGetEntryFlags) that lets attackHitsShow show the hit panels. */
#define SEF_ENTRY_SHOW_HIT 0x4
/* The hit effects attackHitsShow starts on a target: group 0x136, the low key per step state and for a taken hit. */
#define HIT_EFFECT_GROUP 0x136
#define HIT_EFFECT_TAKEN 0x11
#define HIT_EFFECT_STEP1 0x12
#define HIT_EFFECT_STEP2 0x14
#define HIT_EFFECT_STEP3 0x13

int attackHitsShowAndWait(EventQueueEntry *eventEntry, struct AttackCommand *owner);

/* Effect key ranges of the unit's own sequence: keys 700..733 move up by 100 for a technique of extent below 8. */
#define TEC_EFFECT_SEQUENCE_FIRST 700
#define TEC_EFFECT_SEQUENCE_COUNT 34
#define TEC_EFFECT_SEQUENCE_SHIFT 100

void unitEffectStart(BattleUnit *object, int keyHigh, int keyLow);
void GameMpeg2Play(const char *name);
u8 *resCmdMpeg2(u8 *cursor);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230430);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002304D0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", attackHitsShow);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", attackEffectsFinished);

int attackHitsShowAndWait(EventQueueEntry *eventEntry, struct AttackCommand *owner) {
    attackHitsShow(eventEntry, owner);
    return attackEffectsFinished(owner);
}

typedef int s32;
/* One 0x18-byte record of the list: two flag bytes (cleared by saveScreenClearRecords). */
struct McFileEntry {
    unsigned char unmodeled_00[0x11];
    signed char flagD;                      /* +0x11 */
    signed char flagE;                      /* +0x12 */
    unsigned char unmodeled_13[0x18 - 0x13];
};
struct McFileList {
    unsigned char unmodeled_00[0x1C];
    short cursor;                           /* +0x1C */
    short previousCursor;                   /* +0x1E */
    unsigned char unmodeled_20[0x0C];
    unsigned short portResult[2];           /* +0x2C, indexed by port */
    unsigned char unmodeled_30[0x34 - 0x30];
    struct McFileEntry fileEntries[50];            /* +0x34 */
    unsigned char unmodeled_4E4[0x4E8 - 0x4E4];
    unsigned short fadeBase;                 /* screen +0x5BB04 */
    unsigned char unmodeled_4EA[0x4FB - 0x4EA];
    unsigned char fileStatusBytes[5];               /* screen +0x5BB17..+0x5BB1B, cleared by saveScreenClearStatusBytes */
    unsigned char unmodeled_500[0x502 - 0x500];
    short fadeLevel;                          /* screen +0x5BB1E: the level value saveScreenBeginLevelFade sets */
};
struct McScreen {
    unsigned char unmodeled_000[0x150B0];
    unsigned char scanBuffer[0x5B61C - 0x150B0];   /* +0x150B0: buffer xglMcRequest fills */
    struct McFileList fileList;                             /* +0x5B61C */
    unsigned char port;                             /* +0x5BB20 */
    unsigned char levelIndex[0x5BB23 - 0x5BB21];
    unsigned char uiState;                          /* +0x5BB23 */
    unsigned char requestBits[1];
    signed char flagA;                              /* +0x5BB25 */
    signed char cardBusy;                           /* +0x5BB26 */
    signed char fadeStage;                              /* +0x5BB27 */
    unsigned char fadeProgress[1];
    signed char flagC;                              /* +0x5BB29 */
    unsigned char unmodeled_5BB2A[0x5BB2C - 0x5BB2A];
    signed char pending;                            /* +0x5BB2C */
    unsigned char unmodeled_5BB2D[0x5BB2E - 0x5BB2D];
    signed char displayState;                          /* +0x5BB2E: set to 2 by saveScreenResetDisplayState */
    unsigned char unmodeled_5BB2F[0x5BB40 - 0x5BB2F];
};
struct McCardRequest {
    unsigned char unmodeled_00[4];
    void *screen;
    unsigned char unmodeled_08[4];
    void (*callback)(void);
};
typedef signed char s8;
/* A byte cell whose flag sits 7 bytes in: the screen's flag bytes are addressed from here. */
typedef struct McStatusCell {
    unsigned char unmodeled_00[7];
    s8 statusFlag;
} McStatusCell;
s32 sheetSetIndex(s32);

void saveScreenSetMessage(s32 sheetIndex) {
    sheetSetIndex(0);
    sheetSetIndex(sheetIndex);
}

/*
 * Script command ";mpeg2" (name and handler of its resCommandTable entry; proposed resCmdMpeg2).
 * It plays the movie named by the script text at the cursor and returns the cursor unchanged, as the other
 * resCmd handlers do.
 */
u8 *resCmdMpeg2(u8 *cursor) {
    GameMpeg2Play(cursor);
    return cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002306C0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002306F8);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230720);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230818);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230888);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230938);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002309C8);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230A58);

extern struct McScreen *gSaveScreen;
s32 counterEntryAdvancePhase();
void saveScreenHandleError(struct McScreen *screen, unsigned short cardResult);

void saveScreenHandleError(struct McScreen *screen, unsigned short cardResult) {
    gSaveScreen->requestBits[0] &= 0xFE;
    counterEntryAdvancePhase(&gSaveScreen->scanBuffer[0x46560]);
}

void saveScreenSetRequestFlag(void) {
    gSaveScreen->scanBuffer[0x4656B] = 0x80;
    gSaveScreen->requestBits[0] |= 2;
}

void saveScreenBeginLevelFade(void) {
    gSaveScreen->fadeStage = 1;
    gSaveScreen->fadeProgress[0] = 0x80;
    gSaveScreen->fileList.fadeLevel = (gSaveScreen->levelIndex[0] * 0x670) - gSaveScreen->fileList.fadeBase;
}

void saveScreenStepFlagB(void) {
    if (((s8) gSaveScreen->fadeProgress[0] >= 0) && (gSaveScreen->fadeStage == 1)) {
        gSaveScreen->fadeStage = 2;
        return;
    }
    gSaveScreen->fadeStage = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230CA8);

void saveScreenClearStatusBytes(void) {
    s32 statusCellOffset;
    s32 statusCellIndex;

    statusCellIndex = 0;
    do {
        statusCellOffset = statusCellIndex + 0x5BB10;
        statusCellIndex += 1;
        /* byte store at the screen's absolute address 0x5BB17 + index: this spelling keeps the codegen */
        ((McStatusCell *) &gSaveScreen->unmodeled_000[statusCellOffset])->statusFlag = 0;
    } while (statusCellIndex < 5);
}

void saveScreenClearStatusBytes();

void saveScreenClearRecords(void) {
    s32 fileEntryIndex;

    for (fileEntryIndex = 0; fileEntryIndex < 50; fileEntryIndex++) {
        gSaveScreen->fileList.fileEntries[fileEntryIndex].flagD = 0;
        gSaveScreen->fileList.fileEntries[fileEntryIndex].flagE = 0;
    }
    saveScreenClearStatusBytes();
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230DE8);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00230F08);

void func_002E7348(void *start, void *end);
int sceGsSyncPath(int mode, int timeout);
void xglDmaDirectNormal(unsigned int channel, unsigned int address, unsigned int count);
int xglJpegEncode(JpegEncodeRequest *request);
extern XglRenderState sRender;
extern unsigned long long D_003E6F80[24];

int saveThumbnailEncode(int quality, unsigned char *jpegBuffer)
{
    JpegEncodeRequest encodeRequest;
    unsigned long long bitbltWord;

    bitbltWord = 0x24020000 | (sRender.vram.draw_base << 5);
    D_003E6F80[10] = bitbltWord | (0xC800ULL << 19);
    D_003E6F80[8] = 0x801C0;
    func_002E7348(D_003E6F80, D_003E6F80 + 24);
    sceGsSyncPath(0, 0);
    xglDmaDirectNormal(2, (unsigned int) D_003E6F80, 12);
    sceGsSyncPath(0, 0);

    memset(&encodeRequest, 0, sizeof(encodeRequest));
    encodeRequest.outputBuffer = jpegBuffer;
    encodeRequest.frameBufferBase = 0x3800;
    encodeRequest.frameBufferWidth = 8;
    encodeRequest.width = 128;
    encodeRequest.height = 112;
    encodeRequest.originX = 0;
    encodeRequest.originY = 0;
    if (quality <= 0) {
        encodeRequest.luminanceQuality = 10;
        encodeRequest.chrominanceQuality = 10;
    } else {
        encodeRequest.luminanceQuality = quality;
        encodeRequest.chrominanceQuality = quality;
    }
    encodeRequest.samplingMode = 0;
    xglJpegEncode(&encodeRequest);
    return encodeRequest.outputSize;
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00231090);

s32 xglJpegDecode();
void saveThumbnailDecode(s32 encodedJpeg, s32 thumbnail) {
    s32 decodeRequest[8];

    memset(&decodeRequest[0], 0, 0x20U);
    decodeRequest[0] = thumbnail + 0x58;
    decodeRequest[1] = encodedJpeg;
    xglJpegDecode(&decodeRequest[0]);
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00231338);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002313A0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002314F8);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002315F0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00231690);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002317B0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00231B48);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00231E40);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00232018);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00232520);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00232C50);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00232D80);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00233440);

s32 func_00232D80();

void saveScreenDrawLevel(void) {
    s32 scaled;
    s32 level;

    scaled = (gSaveScreen->fileList.fadeLevel - 0x3F0) * gSaveScreen->fadeProgress[0];
    level = (scaled >> 7) + 0x3F0;
    func_00232D80(gSaveScreen->levelIndex[0], level >> 4, 0x80);
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00233740);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00233898);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00233DB8);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234118);

struct McScreen * memPoolAlloc();
s32 bufferPointersRewind();
s32 func_00213C18();
s32 pagedTablePoolInit();

s32 saveScreenCreate(void) {
    struct McScreen *screen;

    func_00213C18();
    pagedTablePoolInit();
    screen = memPoolAlloc(0x5BB40);
    gSaveScreen = screen;
    memset(screen, 0, 0x5BB40U);
    bufferPointersRewind();
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002344C0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234538);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234910);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234998);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234AB0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234C28);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234D68);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00234F38);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00235060);

extern s32 D_004B4F8C[];

void saveScreenResetDisplayState(void) {
    D_004B4F8C[0] = 0;
    gSaveScreen->displayState = 2;
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00235220);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00235480);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00235498);

s32 xglFontSetAlphaScale();
s32 func_0021E390();

void saveScreenDrawLabel(s32 alpha) {
    xglFontSetAlphaScale(alpha & 0xFF);
    func_0021E390(0x30, 0x78, &gSaveScreen->scanBuffer[0x46524], 0);
}

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002357F8);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_002358D0);

INCLUDE_ASM("asm/main/nonmatchings/res_get_path", func_00235A40);

/* The list object of the memory-card screen block (its cursor, per-port results). */









/* The screen block gSaveScreen points to, at its absolute offsets. */

















/* The request block of xglMcRequest (data at +4, callback at +12). */







extern struct McScreen *gSaveScreen;
extern EventQueueEntry func_002357F8[];
extern struct AttackCommand func_002358D0[];
extern unsigned char func_00235220[];
extern unsigned char func_00235480[];
void func_00235A40(void);
void func_00221A88(void *obj, int a, int b);
void addClampedCounter(void *obj, short value);
void func_00221B40(void *obj, int value);
void mcStateSetupRequest(int a, int b);
void seVoiceStartDefault(unsigned int handle);
void saveScreenHandleError(struct McScreen *screen, unsigned short cardResult);
int func_00213700(int id);
int func_002306C0(int a, int b, int c);
void saveScreenSetMessage(int sheetIndex);
void func_00230818(int a, int b, void *c, void *d);
int xglMcRequest(int port, int command, struct McCardRequest *cardRequest, void *buffer);
void xglMcReset(void);

void saveSlotListUpdate(int listLoadRequested)
{
    struct McCardRequest cardRequest;

    if (listLoadRequested != 0) {
        gSaveScreen->uiState = 0;
        saveScreenSetMessage(func_002306C0(func_00213700(0x291), gSaveScreen->port + 1, 0));
        mcStateSetupRequest(0, 0);
        gSaveScreen->cardBusy = 1;
        func_00221A88(&gSaveScreen->fileList, 198, 8);
        addClampedCounter(&gSaveScreen->fileList, gSaveScreen->fileList.cursor);
        gSaveScreen->pending = 0;
        if (gSaveScreen->fileList.portResult[gSaveScreen->port] == 2) {
            gSaveScreen->uiState = 1;
            return;
        }
        cardRequest.screen = gSaveScreen;
        cardRequest.callback = func_00235A40;
        xglMcRequest(gSaveScreen->port, 8, &cardRequest, gSaveScreen->scanBuffer);
        return;
    }
    switch (gSaveScreen->uiState) {
    case 0: {
        struct McScreen *screen = gSaveScreen;
        unsigned short cardResult = screen->fileList.portResult[screen->port];

        if ((unsigned short) (cardResult - 1) >= 2 && cardResult != 0xFF) {
            screen->flagA = 0;
            gSaveScreen->cardBusy = 0;
            gSaveScreen->fadeStage = 0;
            saveScreenHandleError(screen, cardResult);
            gSaveScreen->flagC = 0;
            xglMcReset();
            func_00230818(func_002306C0(func_00213700(0x28C), gSaveScreen->port + 1, 0), 1,
                          func_00235220, func_00235480);
        }
        break;
    }
    case 1:
        if (gSaveScreen->fileList.cursor == gSaveScreen->fileList.previousCursor) {
            gSaveScreen->cardBusy = 0;
            if (gSaveScreen->pending != 0) {
                seVoiceStartDefault(9);
                attackHitsShowAndWait(func_002357F8, func_002358D0);
            } else {
                func_00230818(func_00213700(0x33C), 1, func_00235220, func_00235480);
            }
        }
        break;
    case 2:
        gSaveScreen->cardBusy = 0;
        func_00230818(func_002306C0(func_00213700(0x28C), gSaveScreen->port + 1, 0), 1,
                      func_00235220, func_00235480);
        break;
    }
    func_00221B40(&gSaveScreen->fileList, 1);
}
