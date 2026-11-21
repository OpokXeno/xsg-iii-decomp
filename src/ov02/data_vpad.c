#include "common.h"
#include "shared.h"

/* Rebuilds the key list: zeroes the buffer, copies the lookup key of every live unit into it in table order, then walks it. */ void keyBuffClear(void);
void counterSetNonNegative(int mode);
void dataVPadReset(void);
void menuWepSpPanelReset(void);
/* Uninitialized storage of this file, in address order (only this file names these labels). */
extern int keyBuff[8];
extern int keyBuffIdx;
extern int menuWepSpPanelCursor;
extern int menuWepSpPanelRowCount;
extern int padMode;
extern short padData;

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89128);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A891F0);

#include "ov02/tu060.h"
int sceneStateDispatch(int);
int func_002ACAB0(int);
int stateFlag0010Set(int);
int stateFlag0080Set(int);
void vpadActorSet(int value);
int *func_00A88F60(void *, int);
int func_00A89400();

void battleUnitVPadEnable(struct BattleUnit *battleUnit) {
    func_00A89400();
    func_002ACAB0(1);
    stateFlag0010Set(1);
    stateFlag0080Set(1);
    vpadActorSet((int) battleUnit->motionActor);
    sceneStateDispatch(*func_00A88F60(battleUnit, 0));
}

/* One 0x10-byte record of the slot table gVPadSlots (splat name, no config symbol); only the word at +0x00 is read here. */
typedef struct VPadSlot {
    int active;                         /* +0x00 */
    unsigned char pad_04[0xC];
} VPadSlot;
extern VPadSlot gVPadSlots[];
extern unsigned char D_00ABF8C8[];
int func_00A88E78();
int func_00A89128();

signed char vpadSlotValueGet(int unitId) {
    int vpadSlotIndex;

    vpadSlotIndex = func_00A89128();
    if (gVPadSlots[vpadSlotIndex].active != 0) {
        func_00A88E78(unitId, 0, vpadSlotIndex, 0);
        return D_00ABF8C8[vpadSlotIndex];
    }
    return 0;
}

void vpadDisableChannels(void) {
    func_002ACAB0(0);
    stateFlag0010Set(0);
    stateFlag0080Set(0);
    vpadActorSet(0);
}

int sceneStateBlocksClear();

void vpadClearPending(void) {
    sceneStateBlocksClear();
}

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89400);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89768);

int func_00A89970();
int func_00A89A40();
int func_00A89EC8();

void vpadTickAll(void) {
    func_00A89970();
    func_00A89A40();
    func_00A89EC8();
}

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A897C8);

int stateFlag0008Set(int);
int stateFlag0020Set(int);

void xglStudioMainCameraEnable(void) {
    stateFlag0008Set(1);
    stateFlag0020Set(1);
}

void xglStudioMainCameraMove(void) {
    stateFlag0008Set(0);
    stateFlag0020Set(0);
}

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89878);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89970);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89A40);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89AB0);

/* Virtual pad reset: pad mode and pad data, then the two counter pairs (prototype in battle_init.h). */
void dataVPadReset(void) {
    counterSetNonNegative(0);
    keyBuffClear();
    menuWepSpPanelReset();
}

/* Zero-filled 128-byte key list written by keyBuffClear; splat name, no config symbol. */
extern int gBattleUnitLookupKeys[32];

#include "ov02/tu060.h"


void *memset(void *destination, int value, unsigned int count);
struct BattleUnit *battleUnitGet(int index);
int battleUnitCount(void);
void func_002AD3D0(int *list);

/* Rebuilds the key list: zeroes the buffer, copies the lookup key of every live unit into it in table order, then walks it. */
void keyBuffClear(void) {
    int *keyListCursor;
    int i;
    struct BattleUnit *battleUnit;

    memset(gBattleUnitLookupKeys, 0, 0x80);
    keyListCursor = gBattleUnitLookupKeys;
    for (i = 0; i < battleUnitCount(); i++) {
        battleUnit = battleUnitGet(i);
        if (battleUnit != 0 && !(battleUnit->targetingFlags & 0x8000)) {
            *keyListCursor = battleUnit->lookupKeyRecord->lookupKey;
            keyListCursor++;
        }
    }
    func_002AD3D0(gBattleUnitLookupKeys);
}

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89C68);

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89D68);

int func_002ACA80(int);

void vpadActionEnable(void) {
    func_002ACA80(1);
}

void vpadActionDisable(void) {
    func_002ACA80(0);
}

int menuWepSpPanelSetActive(int, short, int);

void vpadBindLookupKey(struct BattleUnit *battleUnit, int wepSpPanelIndex, int mode) {
    menuWepSpPanelSetActive(wepSpPanelIndex, battleUnit->lookupKeyRecord->lookupKey, mode != 1);
}

int menuWepSpPanelSetLimitState(int);
int menuWepSpPanelSetLimitIndex(int);
int func_00A819D8();

/* Selects the 0x12-threshold flag of the record at 0x2B0 (1 when its 16-bit count is at least 0x12),
 * then calls menuWepSpPanelSetLimitState or menuWepSpPanelSetLimitIndex with it, chosen by func_00A819D8. */
void vpadApplyCountLimit(struct BattleUnit *battleUnit)
{
    int limitReached;

    limitReached = 0;
    limitReached = (battleUnit->lookupKeyRecord->lookupKey < 0x12) == limitReached;
    if (func_00A819D8() != 0)
    {
        menuWepSpPanelSetLimitState(limitReached);
        return;
    }
    menuWepSpPanelSetLimitIndex(limitReached);
}

INCLUDE_ASM("asm/nonmatchings/ov02/data_vpad", func_00A89EC8);
