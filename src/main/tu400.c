#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E3850);

typedef float f32;
extern int D_004C83D4[];

void mainCallbackHookClear(void) {
    D_004C83D4[0] = 0;
}

int effectPoolClearTails();

void effectPoolTailsClearAll(void) {
    effectPoolClearTails();
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E39D0);

typedef struct MainGlobals {
    unsigned char unk_0000[0x5048];
    int sequenceFlags;
    unsigned char unk_504C[0x18];
    int running;
    int sequenceResetA;
    unsigned char unk_506C[0x4];
    int sequenceResetB;
} MainGlobals;

extern MainGlobals D_00609040[];

void mainSequenceStart(void)
{
    MainGlobals *globals = D_00609040;
    int nextSequenceFlags;

    globals->running = 1;
    nextSequenceFlags = globals->sequenceFlags | 1;
    globals->sequenceFlags = nextSequenceFlags;
    globals->sequenceResetA = 0;
    globals->sequenceResetB = 0;
}

extern int D_0060E0A0[];

int mainGetStatusA(void) {
    return D_0060E0A0[0];
}

extern int D_0060E0A8[];

int mainGetStatusB(void) {
    return D_0060E0A8[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E3A60);

extern f32 gSinePhaseStep;
extern f32 gSinePhaseLimit;
extern f32 D_005514FC;
int sheetSetIndex(int);
int func_002209C0();
int func_00220E00();
int func_002AF830(int);
int mrPackUpdateModes();
int mrPackCurrentValue();
int func_002BA858(int);
int func_002BB4E8();
int func_002BC7D8();
int func_002BD7A8(int);
int func_002BF8F8();
int func_002C20A0();
int func_002C3700();
int seVoiceSlotsRefresh();
int func_002C47A0();
int func_002C61E8();
int func_002C6B50();
int sineWavesAdvance();
int func_002E3A60();
int func_002E51F0();

void mainFrameUpdate(void) {
    f32 advancedSinePhase;
    int sheetIndex;

    advancedSinePhase = D_005514FC + gSinePhaseStep;
    if (gSinePhaseLimit < advancedSinePhase) {
        D_005514FC = 0.0f;
    } else {
        D_005514FC = advancedSinePhase;
    }
    sineWavesAdvance();
    func_002E3A60();
    mrPackUpdateModes();
    func_002BB4E8();
    func_002AF830(1);
    func_002E51F0();
    func_002BA858(1);
    sheetIndex = mrPackCurrentValue();
    sheetSetIndex(sheetIndex);
    func_002209C0();
    func_002BD7A8(0);
    func_002BC7D8();
    func_002C20A0();
    seVoiceSlotsRefresh();
    func_002BF8F8();
    func_002C47A0();
    func_002C3700();
    func_00220E00();
    func_002C6B50();
    func_002C61E8();
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E3D60);

int mrPackGetCurrentField();

void mainReadPackCurrentField(void) {
    mrPackGetCurrentField();
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E4090);

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E40F8);

int njStateBindEntry();

void mainSelectEntry(void) {
    njStateBindEntry();
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E4178);

int mrPackArmModeA();
int func_002E5008();

void mainActorPrepare(void) {
    func_002E5008();
    mrPackArmModeA();
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E4290);

int playbackModeSet(signed char);

void mainSetPlaybackMode(signed char playbackMode) {
    playbackModeSet(playbackMode);
}

int slotSetDisplayModeAll(signed char);

void mainSetPlaybackFlag(signed char playbackFlag) {
    slotSetDisplayModeAll(playbackFlag);
}

int mrPackClearStates();

void mainClearPackStates(void) {
    mrPackClearStates();
}

int mrPackClearStateA();

void mainClearPackStateA(void) {
    mrPackClearStateA();
}

int mrPackClearStateB();

void mainClearPackStateB(void) {
    mrPackClearStateB();
}

int func_002B4370();

void mainTickSubsystemC(void) {
    func_002B4370();
}

int mrPackStepCurrent();

void mainStepPackCurrent(void) {
    mrPackStepCurrent();
}

INCLUDE_ASM("asm/main/nonmatchings/tu400", func_002E45D0);
