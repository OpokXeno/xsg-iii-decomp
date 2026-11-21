/*
 * OV02 original TU 39: 0x00a8dee8..0x00a8e120 (9 functions)
 */
#include "common.h"

struct SbThread;
struct BattleUnit;
int vmArgGet(struct SbThread *thread, int argIndex);
struct BattleUnit *unitTblEntryGet(int index);
void unitSetupPhaseActor(struct BattleUnit *unit, int value);

int sbNativeUnitSetupPhase(struct SbThread *call)
{
    struct BattleUnit *battleUnit;

    battleUnit = unitTblEntryGet(vmArgGet(call, 1));
    unitSetupPhaseActor(battleUnit, vmArgGet(call, 2));
    return 1;
}

typedef int s32;
s32 *advanceBlockGet();

s32 battleContextTailGet(void) {
    return advanceBlockGet()[14];
}

struct XenoActor;

#include "ov02/tu060.h"

int vmKeyedEntryGet(struct SbThread *thread, int value);
void XenoActorHidePart(struct XenoActor *actor, int value);

int partsHide(struct SbThread *call)
{
    struct BattleUnit *battleUnit;
    struct XenoActor *actor;
    int partName;

    battleUnit = unitTblEntryGet(vmArgGet(call, 1));
    actor = battleUnit->motionActor;
    partName = vmKeyedEntryGet(call, vmArgGet(call, 2));
    XenoActorHidePart(actor, partName);
    return 1;
}

void XenoActorShowPart(struct XenoActor *actor, int value);

int partsShow(struct SbThread *call)
{
    struct BattleUnit *battleUnit;
    struct XenoActor *actor;
    int partName;

    battleUnit = unitTblEntryGet(vmArgGet(call, 1));
    actor = battleUnit->motionActor;
    partName = vmKeyedEntryGet(call, vmArgGet(call, 2));
    XenoActorShowPart(actor, partName);
    return 1;
}

s32 sceneMaskSet(s32);
s32 soundGroupOnClearFlag();

s32 sbNativeSuspendEnter(void) {
    sceneMaskSet(0);
    soundGroupOnClearFlag();
    return 1;
}

s32 soundGroupOffSetFlag();

s32 sbNativeSuspendLeave(void) {
    soundGroupOffSetFlag();
    sceneMaskSet(1);
    return 1;
}

s32 func_00A8AD20(BattleUnit *, s32);

s32 sbNativeUnitSetMode(struct SbThread *call) {
    BattleUnit *battleUnit;
    int battleUnitIndex;
    int battleUnitMode;

    battleUnitIndex = vmArgGet(call, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    battleUnitMode = vmArgGet(call, 2);
    func_00A8AD20(battleUnit, battleUnitMode);
    return 1;
}

s32 unitActorKeyNext(BattleUnit *);

void sbNativeUnitRefreshPhase(struct SbThread *call) {
    int battleUnitIndex;
    struct BattleUnit *battleUnit;
    battleUnitIndex = vmArgGet(call, 1);
    battleUnit = unitTblEntryGet(battleUnitIndex);
    unitActorKeyNext(battleUnit);
}

s32 unitLiveNumGet(s32);

void sbNativeUnitLiveNumRead(struct SbThread *call) {
    int battleUnitIndex;
    battleUnitIndex = vmArgGet(call, 1);
    unitLiveNumGet(battleUnitIndex);
}
