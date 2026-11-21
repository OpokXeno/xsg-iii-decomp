#include "common.h"

typedef int s32;
typedef unsigned short u16;

/* State A of the package (D_005D0670): the words the package code reads and clears. Its 0x18-byte entries are read
 * by word index in mrPackCurrentValue (word 0x1D5 + 6 * index is the entry field at +0x754). */
typedef struct MrPackModeAState {
    unsigned char unmodeled_0000[0x840];
    s32 pendingValue;                   /* +0x840: returned by mrPackGetPendingValue while enabled */
    s32 unmodeled_844;                  /* +0x844: cleared by the state reset */
    s32 unmodeled_848;                  /* +0x848: cleared by the state reset */
    s32 currentEntryNumber;                   /* +0x84C: index of the current entry (0x18-byte entries from +0) */
    s32 enabled;                        /* +0x850 */
} MrPackModeAState;

/* Nj plane registry (gNjPlaneRegistryStorage), the package mode B state. Its 0x18-byte entries start at +0x103F4 (word 0x40FD + 6 * index). */
typedef struct NjPlaneRegistry {
    unsigned char unmodeled_0000[0x103F0];
    unsigned char packEntries[0xC00];    /* +0x103F0: cleared by mrPackArmModeA with memset */
    unsigned char unmodeled_10ff0[0x11690 - 0x10FF0];
    s32 pendingValue;                   /* +0x11690: returned by mrPackGetPendingValue while enabled */
    s32 unmodeled_11694;                /* +0x11694: cleared by the state reset */
    s32 unmodeled_11698;                /* +0x11698: cleared by the state reset */
    s32 unmodeled_1169C;                /* +0x1169C: cleared by the state reset */
    unsigned char unmodeled_116a0[0x116A4 - 0x116A0];
    s32 enabled;                        /* +0x116A4 */
} NjPlaneRegistry;

extern s32 mrPackModeAActive;
extern s32 mrPackPendingFlag;
extern s32 mrPackStatusLatch;
extern s32 mrPackModeBActive;
extern u16 D_004B34A2;
extern s32 D_005D0670[];
extern s32 gNjPlaneRegistryStorage[];
#define gMrPackModeAState (*(MrPackModeAState *) D_005D0670)
#define gNjPlaneRegistry (*(NjPlaneRegistry *) gNjPlaneRegistryStorage)

s32 func_002B45F8();
s32 njPlaneBlockInit();
s32 njPlaneNoopStep();
s32 sceneLinkSubmitPacket();

void mrPackSystemInit(void) {
    sceneLinkSubmitPacket();
    func_002B45F8();
    njPlaneBlockInit();
    njPlaneNoopStep();
}

s32 func_002B4A10();
s32 func_002B6078();

void mrPackUpdateModes(void) {
    if (mrPackModeAActive != 0) {
        func_002B4A10();
    }
    if (mrPackModeBActive != 0) {
        func_002B6078();
    }
}

s32 func_002B4C58();

void mrPackTickModeBOff(void) {
    if (mrPackModeBActive == 0) {
        func_002B4C58();
    }
}

s32 func_002B6A80();

void mrPackTickModeB(void) {
    if (mrPackModeBActive != 0) {
        func_002B6A80();
    }
}

void mrPackClearPendingFlag(void) {
    mrPackPendingFlag = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu364", func_002B3E70);

void mrPackClearStatusLatch(void) {
    mrPackStatusLatch = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu364", func_002B3F28);

INCLUDE_ASM("asm/main/nonmatchings/tu364", func_002B4010);

void *memset(void *destination, s32 value, unsigned int size);

void mrPackArmModeA(void)
{
    if (gNjPlaneRegistry.enabled != 0)
    {
        gNjPlaneRegistry.enabled = 0;
        mrPackModeAActive = 1;
        memset(gNjPlaneRegistry.packEntries, 0, 0xC00);
    }
}

s32 mrPackStateAIndexGet();
s32 njPlaneCurrentGet();

s32 mrPackCurrentValue(void)
{
    s32 currentPackValue;

    currentPackValue = 0;
    if (gMrPackModeAState.enabled != 0)
    {
        currentPackValue = D_005D0670[0x1D5 + 6 * mrPackStateAIndexGet()];
    }
    else
    {
        if (gNjPlaneRegistry.enabled != 0)
        {
            currentPackValue = gNjPlaneRegistryStorage[0x40FD + 6 * njPlaneCurrentGet()];
        }
    }
    return currentPackValue;
}

s32 func_002B47A0();
s32 func_002B4FE0();

void mrPackStepCurrent(void)
{
    if (mrPackModeAActive != 0)
    {
        func_002B47A0();
        return;
    }
    if (!(mrPackModeBActive != 0))
    {
        return;
    }
    {
        func_002B4FE0();
    }
}

s32 mrPackGetCurrentId(void) {
    s32 currentPackId;

    currentPackId = 0;
    if (mrPackModeAActive != 0) {
        currentPackId = mrPackStateAIndexGet();
    } else if (mrPackModeBActive != 0) {
        currentPackId = njPlaneCurrentGet();
    }
    return currentPackId;
}

s32 stateATailValueGet();
s32 func_002B52A0();

s32 mrPackGetCurrentField(void) {
    s32 currentPackField;

    currentPackField = 0;
    if (mrPackModeAActive != 0) {
        currentPackField = stateATailValueGet();
    } else if (mrPackModeBActive != 0) {
        currentPackField = func_002B52A0();
    }
    return currentPackField;
}

void mrPackClearStates(void)
{
    gMrPackModeAState.currentEntryNumber = 0;
    gMrPackModeAState.unmodeled_848 = 0;
    gMrPackModeAState.unmodeled_844 = 0;
    gNjPlaneRegistry.unmodeled_1169C = 0;
    gNjPlaneRegistry.unmodeled_11698 = 0;
    gNjPlaneRegistry.unmodeled_11694 = 0;
}

void mrPackClearStateA(void)
{
    gMrPackModeAState.currentEntryNumber = 0;
    gMrPackModeAState.unmodeled_848 = 0;
    gMrPackModeAState.unmodeled_844 = 0;
}

void mrPackClearStateB(void)
{
    gNjPlaneRegistry.unmodeled_1169C = 0;
    gNjPlaneRegistry.unmodeled_11698 = 0;
    gNjPlaneRegistry.unmodeled_11694 = 0;
}

s32 func_002B4848();
s32 func_002B5480();

void mrPackFinishCurrent(void)
{
    if (mrPackModeAActive != 0)
    {
        func_002B4848();
        return;
    }
    if (!(mrPackModeBActive != 0))
    {
        return;
    }
    {
        func_002B5480();
    }
}

s32 func_002B48C8();
s32 func_002B55E0();

void mrPackFlushCurrent(void)
{
    if (mrPackModeAActive != 0)
    {
        func_002B48C8();
        return;
    }
    if (!(mrPackModeBActive != 0))
    {
        return;
    }
    {
        func_002B55E0();
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu364", func_002B4370);

s32 mrPackGetCurrentId(void);
extern u16 D_004B34A2;
s32 func_002B4370();
s32 func_002B4928();
s32 func_002B56A8();

void mrPackUpdateByState(void) {
    if (mrPackGetCurrentId() == 0) {
        if (D_004B34A2 & 0x1000) {
            func_002B4370();
            return;
        }
        return;
    }
    if (mrPackModeAActive != 0) {
        func_002B4928();
        return;
    }
    if (mrPackModeBActive == 0) {
        return;
    }
    func_002B56A8();
}

INCLUDE_ASM("asm/main/nonmatchings/tu364", func_002B4468);

s32 func_002B5D00();

void mrPackUpdateModeB(void) {
    if (mrPackModeBActive != 0) {
        func_002B5D00();
    }
}

s32 func_002B5EC0();

void mrPackRefreshModeB(void) {
    if (mrPackModeBActive != 0) {
        func_002B5EC0();
    }
}

s32 mrPackGetPendingValue(void)
{
    if (gMrPackModeAState.enabled != 0)
    {
        return gMrPackModeAState.pendingValue;
    }
    if (!(gNjPlaneRegistry.enabled != 0))
    {
        return 0;
    }
    return gNjPlaneRegistry.pendingValue;
}
