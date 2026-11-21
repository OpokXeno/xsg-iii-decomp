#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AB950);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;

typedef struct SceneState {
    s32 headWord;                 /* 0x0000, cleared by sceneSystemsInit */
    u8 unmodeled_0004[0xF50];
    s32 counterA;                 /* 0xF54, cleared by sceneStateCountersClear */
    u8 unmodeled_0F58[0x0C];
    s32 counterB;                 /* 0xF64, cleared by sceneStateCountersClear */
    u8 unmodeled_0F68[0x04];
    s32 mask;                     /* 0xF6C, set to 0xFFFF by sceneSystemsInit */
    s32 sceneFlags;               /* 0xF70, bit set/clear pairs below */
} SceneState;

typedef struct SceneLinkPairs {
    s32 firstSelector;
    s32 firstLinkId;
    s32 secondSelector;
    s32 secondLinkId;
} SceneLinkPairs;

extern void (*D_004C83D4[])(void);
extern unsigned char sceneLinkPairBlock[];
extern unsigned char D_005C1240[];
extern f32 sceneTimer;
#define gSceneState (*(SceneState *) D_005C1240)
#define gSceneLinkPairs (*(SceneLinkPairs *) sceneLinkPairBlock)

void sheetStateReset(void);
void mcModuleReset(void);
void sceneFrameUpdate(void);
void func_00220B90(s32 selector, s32 value);
void mcClearSetupStatus(void);
void func_00220D28(void);
void func_002ABB48(void);
void func_002ABF18(void);
void func_002ACDD0(void);
void func_002AD598(void);
void func_002ADD48(void);
void func_002AEBE8(void);
void sceneSharedModeSet(s8 mode);
void func_002AF2A8(void);
void func_002AF730(s32 mode);
void mrPackSystemInit(void);
void sceneStateBlockClear(void);
void effectPoolInit(void);
void auxBufferClearA(void);
void auxBufferClearB(void);
void func_002BD230(void);
void func_002BEB80(void);
void func_002C1C70(s32 mode);
void seVoiceStateInit(void);
void seVoiceSlotsClear(void);
void seVoiceCursorReset(void);
void func_002C6098(void);
void sceneLinkStateClear(void);
void sceneLinkSetFlag45(s32 mode);
s32 sceneLinkLookupEntry(s32 index);
void sineWavesReset(void);

void sceneSystemsInit(void)
{
    s32 firstLinkId;
    s32 secondLinkId;
    mrPackSystemInit();
    gSceneLinkPairs.firstSelector = 8;
    firstLinkId = sceneLinkLookupEntry(0x29);
    gSceneLinkPairs.firstLinkId = firstLinkId;
    gSceneLinkPairs.secondSelector = 4;
    secondLinkId = sceneLinkLookupEntry(0x28);
    gSceneLinkPairs.secondLinkId = secondLinkId;
    func_002AF2A8();
    func_002AF730(0);
    sceneStateBlockClear();
    gSceneState.headWord = 0;
    func_002AD598();
    func_002ADD48();
    effectPoolInit();
    mcModuleReset();
    mcClearSetupStatus();
    func_00220B90(gSceneLinkPairs.firstSelector, gSceneLinkPairs.firstLinkId);
    firstLinkId = gSceneLinkPairs.secondSelector;
    func_00220B90(firstLinkId, gSceneLinkPairs.secondLinkId);
    func_00220D28();
    sheetStateReset();
    func_002BD230();
    auxBufferClearA();
    auxBufferClearB();
    func_002C1C70(0);
    func_002BEB80();
    seVoiceSlotsClear();
    func_002AEBE8();
    seVoiceCursorReset();
    seVoiceStateInit();
    func_002C6098();
    sceneLinkStateClear();
    sceneLinkSetFlag45(0);
    sceneTimer = 0.0f;
    func_002ACDD0();
    gSceneState.mask = 0xFFFF;
    gSceneState.sceneFlags = 0x4100;
    sceneSharedModeSet(0);
    sineWavesReset();
    sceneFrameUpdate();
    func_002ABB48();
    D_004C83D4[0] = func_002ABF18;
}

void sceneHookClear(void) {
    D_004C83D4[0] = 0;
}

void effectPoolClearTails(void);

void sceneHookRunStep(void) {
    effectPoolClearTails();
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ABB48);

extern s32 D_005C21A0[];

s32 sceneFirstWordGet(void) {
    return D_005C21A0[0];
}

void sceneStateCountersClear(void)
{
    gSceneState.counterA = 0;
    gSceneState.counterB = 0;
}

extern s32 D_005C21A4[];

s32 sceneSecondWordGet(void) {
    return D_005C21A4[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ABBA8);

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ABBF0);

extern f32 sceneTimerStep;
extern f32 sceneTimerLimit;
void sheetSetIndex(s32 index);
void func_002209C0(void);
void func_00220E00(void);
void func_002ABBA8(void);
void func_002ABBF0(void);
void func_002AD640(void);
void func_002AE030(void);
void func_002AEC38(void);
void func_002AF830(s32 mode);
void mrPackUpdateModes(void);
s32 mrPackCurrentValue(void);
void func_002BA858(s32 mode);
void func_002BB4E8(void);
void func_002BC7D8(void);
void func_002BCD20(void);
void func_002BD7A8(s32 mode);
void func_002BF8F8(void);
void func_002C20A0(void);
void func_002C3700(void);
void seVoiceSlotsRefresh(void);
void func_002C47A0(void);
void func_002C61E8(void);
void func_002C6B50(void);
void sineWavesAdvance(void);

void sceneFrameUpdate(void) {
    f32 timer;
    s32 currentSheetIndex;

    timer = sceneTimer + sceneTimerStep;
    if (sceneTimerLimit < timer) {
        sceneTimer = 0.0f;
    } else {
        sceneTimer = timer;
    }
    sineWavesAdvance();
    func_002ABBF0();
    mrPackUpdateModes();
    func_002BB4E8();
    func_002AF830(0);
    func_002BA858(0);
    func_002AD640();
    func_002AE030();
    func_002BF8F8();
    func_002BD7A8(1);
    func_002BC7D8();
    func_002C20A0();
    func_002BCD20();
    seVoiceSlotsRefresh();
    func_002AEC38();
    func_002C47A0();
    func_002C3700();
    func_00220E00();
    currentSheetIndex = mrPackCurrentValue();
    sheetSetIndex(currentSheetIndex);
    func_002209C0();
    func_002C6B50();
    func_002C61E8();
    func_002ABBA8();
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ABF18);

extern s32 sceneMaskBits[];

void sceneMaskSet(s32 maskOn) {
    if (maskOn != 0) {
        sceneMaskBits[0] = 0xFFFF;
        return;
    }
    sceneMaskBits[0] = 0;
}

void mrPackGetCurrentField(void);

void sceneStateTick(void) {
    mrPackGetCurrentField();
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AC328);

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AC3B0);

void mrPackArmModeA(void);

void stateFlag0002ClearAndReset(void)
{
    gSceneState.sceneFlags &= 0xFFFD;
    mrPackArmModeA();
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AC498);

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AC520);

void mrPackClearStates(void);

void sceneStateBlocksClear(void) {
    mrPackClearStates();
}

void func_002B4370(void);

void sceneStateRelease(void) {
    func_002B4370();
}

void mrPackStepCurrent(void);

void sceneStateDispatch(void) {
    mrPackStepCurrent();
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AC5F8);

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002AC690);

void playbackModeSet(s8 value);

void sceneDataByteSet(s8 sceneByte) {
    playbackModeSet(sceneByte);
}

void slotSetDisplayModeAll(s8 value);

void sceneCellBytesSet(s8 cellByte) {
    slotSetDisplayModeAll(cellByte);
}

void counterSet(s32 value);

void counterSetNonNegative(s32 counterAmount) {
    counterSet((counterAmount < 0) ? 0 : counterAmount);
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ACA80);

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ACAB0);

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ACAE8);

void stateFlag1000Set(s32 bitOn)
{
    if (bitOn != 0)
    {
        gSceneState.sceneFlags |= 0x1000;
        return;
    }
    gSceneState.sceneFlags &= 0xEFFF;
}

void stateFlag0008Set(s32 bitOn)
{
    if (bitOn != 0)
    {
        gSceneState.sceneFlags |= 8;
        return;
    }
    gSceneState.sceneFlags &= 0xFFF7;
}

void stateFlag0010Set(s32 bitOn)
{
    if (bitOn != 0)
    {
        gSceneState.sceneFlags |= 0x10;
        return;
    }
    gSceneState.sceneFlags &= 0xFFEF;
}

void stateFlag0020Set(s32 bitOn)
{
    if (bitOn != 0)
    {
        gSceneState.sceneFlags |= 0x20;
        return;
    }
    gSceneState.sceneFlags &= 0xFFDF;
}

void func_002BB450(s32 enable);

void stateFlag0040Set(s32 bitOn)
{
    if (bitOn != 0)
    {
        gSceneState.sceneFlags |= 0x40;
        return;
    }
    gSceneState.sceneFlags &= 0xFFBF;
    func_002BB450(bitOn);
}

void stateFlag0080Set(s32 bitOn)
{
    if (bitOn != 0)
    {
        gSceneState.sceneFlags |= 0x80;
        return;
    }
    gSceneState.sceneFlags &= 0xFF7F;
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ACD50);

void slotUsedFlagSet(s32 index, s32 value);

void sceneIndexFlagSet(s32 slotNumber) {
    slotUsedFlagSet(slotNumber, 1);
}

void sceneRecordFlagWordSet(s32 index, s32 value);

void sceneRecordFlagSet(s32 recordNumber) {
    sceneRecordFlagWordSet(recordNumber, 1);
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ACDD0);

void sceneStatusFlagIsOne(void);

void scenePollStatus(void) {
    sceneStatusFlagIsOne();
}

INCLUDE_ASM("asm/main/nonmatchings/tu357", func_002ACE40);
