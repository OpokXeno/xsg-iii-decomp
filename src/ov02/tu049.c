/*
 * OV02 original TU 49: 0x00a936b8..0x00a93ea0 (8 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A936B8);

typedef int s32;

/* The words of a battle unit past its fade field (+0x300), which battleUnitResetCounters clears. Their roles are not known. */
typedef struct BattleUnitCounters {
    unsigned char unmodeled_000[0x504];
    s32 resetWord504;                   /* +0x504 */
    s32 resetWord508;                   /* +0x508 */
    s32 resetWord50C;                   /* +0x50C */
    unsigned char unmodeled_510[0x4];
    s32 resetWord514;                   /* +0x514 */
    unsigned char unmodeled_518[0x10];
    s32 resetWord528;                   /* +0x528 */
    unsigned char unmodeled_52C[0x4];
    s32 resetWord530;                   /* +0x530 */
} BattleUnitCounters;

void battleUnitResetCounters(BattleUnitCounters *battleUnit) {
    battleUnit->resetWord530 = 0;
    battleUnit->resetWord514 = 0;
    battleUnit->resetWord528 = 0;
    battleUnit->resetWord504 = 0;
    battleUnit->resetWord508 = 0;
    battleUnit->resetWord50C = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93770);

s32 seVoiceCursorSetEnable(s32);

void battleDropRequestArm(void) {
    seVoiceCursorSetEnable(1);
}

s32 func_00A8C4F8(s32);

s32 battleDropRequestConsume(void) {
    if (!(func_00A8C4F8(1) & 0x60)) {
        return 0;
    }
    seVoiceCursorSetEnable(0);
    return 1;
}

#define UNIT_FLAG_DROP_FIRST 0x20

#include "ov02/tu060.h"

typedef struct DropRecord {
    unsigned char unmodeled_00[13];
    unsigned char firstCategory; /* +0x0D: 0 when there is no first drop */
    unsigned char unmodeled_0E[2];
    unsigned char firstId; /* +0x10 */
} DropRecord;


DropRecord *ov02DropRecordGet(short key);
char *stringTableGet(int textId);
char *func_00A85208(int category, int id);
void ov02AdjustRollUp(int category, int id);
void ov02SetEventFlag(char *text);
int battleRollPercent(int percent);
char *strcpy(char *destination, const char *source);
char *func_0030E3D0(char *destination, const char *source);
extern char battleDropMessage[];
extern int D_00AC34A0;

void battleUnitRollFirstDrop(BattleUnit *battleUnit, int percent)
{
    DropRecord *dropRecord;
    char *name;
    int category;
    int id;

    dropRecord = ov02DropRecordGet(battleUnit->lookupKeyRecord->lookupKey);
    category = dropRecord->firstCategory;
    id = dropRecord->firstId;
    if (category == 0 || (battleUnit->targetingFlags & UNIT_FLAG_DROP_FIRST) != 0) {
        category = 0;
    }
    if (category == 0) {
        strcpy(battleDropMessage, stringTableGet(39));
    } else if (battleRollPercent(percent) != 0) {
        battleUnit->targetingFlags |= UNIT_FLAG_DROP_FIRST;
        ov02AdjustRollUp(category, id);
        strcpy(battleDropMessage, stringTableGet(36));
        name = func_00A85208(category, id);
        func_0030E3D0(battleDropMessage, name);
        func_0030E3D0(battleDropMessage, stringTableGet(37));
    } else {
        strcpy(battleDropMessage, stringTableGet(38));
    }
    ov02SetEventFlag(battleDropMessage);
    D_00AC34A0 = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93E00);

struct BattleStats {
    unsigned char unmodeled_00[0x72];
    s16 actionValue;                       /* +0x72: passed to actionDispatchDefault */
};

s32 actionDispatchDefault();
s32 unitSetDrawFlagTree();

void battleUnitSetActorFlag2000(BattleUnit *battleUnit, s32 actionSlot) {
    s32 actorFlags;
    struct XenoActor *actor;
    struct BattleStats *battleStats;

    unitSetDrawFlagTree(battleUnit);
    actor = battleUnit->motionActor;
    battleStats = battleUnit->stats;
    actorFlags = actor->resetFlags | 0x2000;
    actor->resetFlags = actorFlags;
    actionDispatchDefault(actionSlot, battleStats->actionValue);
}
