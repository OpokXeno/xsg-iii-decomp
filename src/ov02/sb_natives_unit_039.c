/*
 * OV02 original TU 39: 0x00a8dee8..0x00a8e120 (9 functions)
 */
#include "common.h"

struct SbThread;
struct BattleUnit;
int vmArgGet(struct SbThread *thread, int argIndex);
struct BattleUnit *unitTblEntryGet(int index);
void func_00A8AF18(struct BattleUnit *unit, int value);

int func_00A8DEE8(struct SbThread *call)
{
    struct BattleUnit *unit;

    unit = unitTblEntryGet(vmArgGet(call, 1));
    func_00A8AF18(unit, vmArgGet(call, 2));
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_039", func_00A8DF40);

struct XenoActor;

#include "ov02/tu060.h"

int vmKeyedEntryGet(struct SbThread *thread, int value);
void XenoActorHidePart(struct XenoActor *actor, int value);

int partsHide(struct SbThread *call)
{
    struct BattleUnit *unit;
    struct XenoActor *actor;
    int value;

    unit = unitTblEntryGet(vmArgGet(call, 1));
    actor = unit->unit;
    value = vmKeyedEntryGet(call, vmArgGet(call, 2));
    XenoActorHidePart(actor, value);
    return 1;
}

void XenoActorShowPart(struct XenoActor *actor, int value);

int partsShow(struct SbThread *call)
{
    struct BattleUnit *unit;
    struct XenoActor *actor;
    int value;

    unit = unitTblEntryGet(vmArgGet(call, 1));
    actor = unit->unit;
    value = vmKeyedEntryGet(call, vmArgGet(call, 2));
    XenoActorShowPart(actor, value);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_039", func_00A8E030);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_039", func_00A8E058);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_039", func_00A8E080);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_039", func_00A8E0D8);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_039", func_00A8E100);
