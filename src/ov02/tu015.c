/*
 * OV02 original TU 15: 0x00a86bf8..0x00a871b0 (10 functions)
 */
#include "common.h"
#include "ov02/tu060.h"

typedef int s32;
typedef unsigned int u32;
extern XenoActor *gUnitActorTable[];
s32 unitTblGet(s32 tableId, BattleUnit ***units);

XenoActor **unitActorTableBuild(void) {
    BattleUnit **units;
    XenoActor **actorTableCursor;
    s32 battleUnitTotal;
    XenoActor *unitActor;
    s32 writtenCount;
    s32 remaining;
    BattleUnit **unitCursor;
    BattleUnit *battleUnit;

    battleUnitTotal = unitTblGet(2, &units);
    writtenCount = 0;
    if (battleUnitTotal > 0) {
        unitCursor = units;
        remaining = battleUnitTotal;
        actorTableCursor = gUnitActorTable;
        do {
            battleUnit = *unitCursor;
            unitCursor += 1;
            if (battleUnit != 0) {
                unitActor = battleUnit->motionActor;
                writtenCount += 1;
                *actorTableCursor = unitActor;
                actorTableCursor += 1;
            } else {
                *actorTableCursor = 0;
                actorTableCursor += 1;
                writtenCount += 1;
            }
            remaining -= 1;
        } while (remaining != 0);
    }
    gUnitActorTable[writtenCount] = 0;
    return gUnitActorTable;
}

s32 unitPlNumGet();

void unitPlNumUpdate(void) {
    unitPlNumGet();
}

s32 unitEnNumGet();

void unitEnNumUpdate(void) {
    unitEnNumGet();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu015", func_00A86CB8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu015", func_00A86D98);

BattleUnit *unitSideGet();

BattleUnitLookupKeyView *unitGetValueById(s32 unitId) {
    u32 unitPoolId;

    unitPoolId = unitId - 1;
    if (!(unitPoolId < 0xC2U)) {
        return 0;
    }
    return unitSideGet(unitId)->lookupKeyRecord;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu015", func_00A86E28);

s32 entryCostGet(BattleUnit *, s32);
s32 ov02SlotEntryGet(s32);

void unitEntryCostCheck(s32 unused, s32 entryId) {
    BattleUnit *sideUnit;
    s32 costSlotEntry;

    sideUnit = unitSideGet();
    costSlotEntry = ov02SlotEntryGet(entryId);
    entryCostGet(sideUnit, costSlotEntry);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu015", func_00A86F50);

INCLUDE_ASM("asm/nonmatchings/ov02/tu015", func_00A87090);
