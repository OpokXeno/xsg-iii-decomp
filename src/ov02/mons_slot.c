#include "common.h"
#include "shared.h"

unsigned short xglSRand(void);
int ov01RollPercent(int percent);
int rnd(int max);

#include "main/tu352.h"
#include "main/xenoActor.h"
#include "main/xgl_2.h"
#include "ov02/tu018.h"

MonsSlot *monsSlotClaim(void *initWord10);








struct XenoActor;






/* Indices of XenoActor.rsrc: the model pack (.pxy, xenoActor.c reads it as PxyHeader *), the texture (.txy), the
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

struct MonsSlot;
struct MonsSlot *monsSlotCreate(void);

/*
 * monsSlotNew zeroes the actor's position through a float array, an alias-0 access that keeps the stores after the
 * slot's pointer store. Making XenoActor.position a union would give main's code alias set 0 too and change
 * uwamonoPlayerUpdate and Get_Off_Ladder, so this one site reaches the position through a view that starts like the
 * actor (struct XenoActor of main/xenoActor.h, +0x10).
 */
typedef struct MonsActorPositionView {
    unsigned char unmodeled_00[0x10];
    Vector4Elements position; /* +0x10 */
} MonsActorPositionView;

/*
 * D_00ABD330 is the one-entry table of MonsSlot record pointers and D_00ABD334 its count (scaffold names, splat
 * D_ symbols): monsSlotInsert fills the entry and bumps the count, monsSlotRemove clears it and drops the count.
 */
extern struct MonsSlot *D_00ABD330;
extern int D_00ABD334;
struct MonsSlot *monsSlotNew();
void monsSlotEmptyUpdate(void);
int monsSlotInsert(struct MonsSlot *record);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88A08);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88A18);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88AE0);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88B38);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88B40);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88B50);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88B88);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88BA8);

/*
 * monsSlotCreate: makes a new MonsSlot with
 * monsSlotNew and inserts it with monsSlotInsert. Returns the record, or 0
 * when monsSlotNew fails. Callers: ov01StreamOpen, ov01StepOpenHandles.
 */
struct MonsSlot *monsSlotCreate(void) {
    struct MonsSlot *slot;

    slot = monsSlotNew();
    if (slot != 0) {
        monsSlotInsert(slot);
    }
    return slot;
}

void func_00A88C58(struct MonsSlot *slot);

struct MonsSlot *monsSlotNew(void *initWord10)
{
    struct MonsSlot *slot;

    slot = monsSlotClaim(initWord10);
    if (slot == 0) {
        return slot;
    }
    slot->initWord10 = initWord10;
    slot->callback = 0;
    slot->initWord08 = 0;
    slot->initWord0C = 0;
    func_00A88C58(slot);
    return slot;
}

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88C58);

void monsSlotReleaseRecord(void);
void XenoActorRelease(struct XenoActor *actor);
int monsSlotRemove(struct MonsSlot *slot);

int monsSlotRelease(struct MonsSlot *slot)
{
    monsSlotReleaseRecord();
    XenoActorRelease(slot->actor);
    return monsSlotRemove(slot);
}

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88CE0);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88CF8);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88D10);

INCLUDE_ASM("asm/nonmatchings/ov02/mons_slot", func_00A88D20);

int monsActiveSlotsGet(struct MonsSlot ***table)
{
    *table = &D_00ABD330;
    return 1;
}

int monsSlotInsert(struct MonsSlot *record)
{
    struct MonsSlot **entry = &D_00ABD330;
    int *count = &D_00ABD334;

    if (*entry == 0) {
        *entry = record;
        *count = *count + 1;
        return 1;
    }
    return 0;
}

int monsSlotRemove(struct MonsSlot *slot)
{
    struct MonsSlot **entry = &D_00ABD330;
    int *count = &D_00ABD334;

    if (*entry == slot) {
        *entry = 0;
        *count = *count - 1;
        return 1;
    }
    return 0;
}

/* rnd: uniform integer in 0..max. The same name and signature are extern in the other ov01 TUs and in X1. */
int rnd(int max)
{
    return xglSRand() % (max + 1);
}

/* ov01RollPercent: 1 with probability percent/100 (rnd(99) + 1 is 1..100). */
int ov01RollPercent(int percent)
{
    if (percent < rnd(99) + 1) {
        return 0;
    }
    return 1;
}
