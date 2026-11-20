#include "common.h"
#include "shared.h"

#include "main/res_get_path.h"
#include "main/tu352.h"
#include "main/xenoActor.h"
#include "ov02/tu012.h"
#include "ov02/tu060.h"






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




void XenoActorSetMode(struct XenoActor *actor, int mode);
void XenoActorEnableChannel(struct XenoActor *actor);
void XenoActorEnableChannelB(struct XenoActor *actor);
void XenoActorSetChannelValue(struct XenoActor *actor, float value);
void XenoActorSetChannelValueB(struct XenoActor *actor, float value);
void XenoActorSetSelector(struct XenoActor *actor, int selector);

#define DATA_LOOKUP_CAPACITY 12 /* items in a DataLookup block */




struct BattleUnit;

/* A unit has three child units (and three slot flags, one per child); the command table has 40 handlers. */
#define UNIT_CHILD_COUNT 3


struct FadeTask;





int unitCurrentPhase(BattleUnit *object);
void unitSetPhase(BattleUnit *object, int phase);
void unitSetSelector(BattleUnit *object, int selector);
void unitRefreshPhase(BattleUnit *object);
struct XenoLight;
int partyUnitHpLow(struct BattleUnit *owner);
int unitCmdFinish(BattleUnit *object, Ov01Command *step);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99230);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99278);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A992D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99390);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99450);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99490);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99528);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99540);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99560);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A995D0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99618);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99638);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A996B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A996F0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99730);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99798);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A997E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99860);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99960);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99978);

int func_00A83BE0(void *p);

/* Tail-calls func_00A83BE0 on the command queue at +0x10 of the unit. */
int func_00A99990(BattleUnit *object)
{
    return func_00A83BE0(&object->commandQueue);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A999A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99A88);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99A90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99B10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99B90);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99BE8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99C40);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99CA0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99D18);

/* Finishes at once: unitRefreshPhase, clears the active flag, reports done. */
int unitCmdFinish(BattleUnit *object, Ov01Command *unusedStep)
{
    unitRefreshPhase(object);
    object->flags &= ~1;
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99E50);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99E78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A99ED8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A080);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A100);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A210);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A4A0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A508);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A610);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A628);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A6C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A760);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A820);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A8C0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu061", func_00A9A928);
