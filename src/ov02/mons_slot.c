#include "common.h"
#include "shared.h"

unsigned short xglSRand(void);
int battleRollPercent(int percent);
int rnd(int max);

#include "main/tu352.h"
#include "main/xenoActor.h"
#include "main/xgl_2.h"
#include "ov02/tu018.h"

MonsActorRecord *monsSlotClaim(void *initArgument);








struct XenoActor;






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




struct XenoActor *XenoActorAlloc(void);

#define ACTOR_STATE_HIDDEN 0x4

struct MonsActorRecord;
struct MonsActorRecord *monsterSpawn(void);

/*
 * monsterClaimInit zeroes the actor's position through a float array, an alias-0 access that keeps the stores after the
 * slot's pointer store. Making XenoActor.position a union would give main's code alias set 0 too and change
 * uwamonoPlayerUpdate and Get_Off_Ladder, so this one site reaches the position through a view that starts like the
 * actor (struct XenoActor of main/xenoActor.h, +0x10).
 */
typedef struct MonsActorPositionView {
    unsigned char unmodeled_00[0x10];
    Vector4Elements position; /* +0x10 */
} MonsActorPositionView;

/*
 * gMonsActiveSlot is the one-entry table of MonsActorRecord record pointers and gMonsActiveSlotCount its count (scaffold names, splat
 * D_ symbols): monsActiveInsert fills the entry and bumps the count, monsActiveRemove clears it and drops the count.
 */
extern struct MonsActorRecord *gMonsActiveSlot;
extern int gMonsActiveSlotCount;
struct MonsActorRecord *monsterClaimInit();
void monsSlotEmptyUpdate(void);
int monsActiveInsert(struct MonsActorRecord *record);

typedef int s32;
typedef long long s64;
extern s32 gMonsReadyCount;

void monsReadyCountReset(void) {
    gMonsReadyCount = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88A18);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88AE0);

s32 monsSlotNullUpdate(void) {
    return 0;
}

s32 monsReadyCountGet(void) {
    return gMonsReadyCount;
}

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88B50);

extern s64 gMonsColor[2];

void monsColorStore(s64 *color) {
    gMonsColor[0] = color[0];
    gMonsColor[1] = color[1];
}

s32 func_0019F1C0(void *);

void monsColorApply(void) {
    func_0019F1C0(gMonsColor);
}

/*
 * monsterSpawn: makes a new MonsActorRecord with
 * monsterClaimInit and inserts it with monsActiveInsert. Returns the record, or 0
 * when monsterClaimInit fails. Callers: battleStreamOpen, battleStepOpenHandles.
 */
struct MonsActorRecord *monsterSpawn(void) {
    struct MonsActorRecord *monster;

    monster = monsterClaimInit();
    if (monster != 0) {
        monsActiveInsert(monster);
    }
    return monster;
}

void func_00A88C58(struct MonsActorRecord *slot);

struct MonsActorRecord *monsterClaimInit(void *initArgument)
{
    struct MonsActorRecord *monster;

    monster = monsSlotClaim(initArgument);
    if (monster == 0) {
        return monster;
    }
    monster->initArgument = initArgument;
    monster->callback = 0;
    monster->initWord08 = 0;
    monster->initWord0C = 0;
    func_00A88C58(monster);
    return monster;
}

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88C58);

void monsSlotReleaseRecord(void);
void XenoActorRelease(struct XenoActor *actor);
int monsActiveRemove(struct MonsActorRecord *slot);

int monsterRelease(struct MonsActorRecord *monster)
{
    monsSlotReleaseRecord();
    XenoActorRelease(monster->actor);
    return monsActiveRemove(monster);
}

void monsSlotShowActor(struct MonsActorRecord *monster) {
    struct XenoActor *actor;

    actor = monster->actor;
    actor->drawFlags &= ~ACTOR_STATE_HIDDEN;
}

void monsSlotHideActor(struct MonsActorRecord *monster) {
    struct XenoActor *actor;

    actor = monster->actor;
    actor->drawFlags |= ACTOR_STATE_HIDDEN;
}

extern s32 D_00AB7AD4;

void monsValueSet(s32 newValue) {
    D_00AB7AD4 = newValue;
}

s32 monsValueGet(void) {
    return D_00AB7AD4;
}

int monsActiveSlotsGet(struct MonsActorRecord ***activeSlotsOut)
{
    *activeSlotsOut = &gMonsActiveSlot;
    return 1;
}

int monsActiveInsert(struct MonsActorRecord *monster)
{
    struct MonsActorRecord **activeSlot = &gMonsActiveSlot;
    int *activeSlotCount = &gMonsActiveSlotCount;

    if (*activeSlot == 0) {
        *activeSlot = monster;
        *activeSlotCount = *activeSlotCount + 1;
        return 1;
    }
    return 0;
}

int monsActiveRemove(struct MonsActorRecord *monster)
{
    struct MonsActorRecord **activeSlot = &gMonsActiveSlot;
    int *activeSlotCount = &gMonsActiveSlotCount;

    if (*activeSlot == monster) {
        *activeSlot = 0;
        *activeSlotCount = *activeSlotCount - 1;
        return 1;
    }
    return 0;
}

/* rnd: uniform integer in 0..upperBound. The same name and signature are extern in the other battle-overlay TUs and in X1. */
int rnd(int upperBound)
{
    return xglSRand() % (upperBound + 1);
}

/* battleRollPercent: 1 with probability percent/100 (rnd(99) + 1 is 1..100). */
int battleRollPercent(int percent)
{
    if (percent < rnd(99) + 1) {
        return 0;
    }
    return 1;
}
