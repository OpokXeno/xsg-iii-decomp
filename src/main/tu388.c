#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
extern u8 *D_00550F84;
extern s8 gVsyncsPerFrame;
s32 func_002CAE70();
s32 func_002CB218();
s32 func_002CB3A0();
extern unsigned char func_002DB120[];

s32 runGuardedStepFallback(void) {
    s32 guardedStepResult;
    s32 savedFlag;

    savedFlag = gVsyncsPerFrame;
    gVsyncsPerFrame = 1;
    func_002CB218();
    guardedStepResult = func_002CAE70();
    func_002CB3A0();
    if (guardedStepResult != 0) {
        guardedStepResult = 0;
        D_00550F84 = func_002DB120;
    }
    gVsyncsPerFrame = savedFlag;
    return guardedStepResult;
}

extern unsigned char stateKeyTableRoot[];
s32 func_001B5F28();
void xcBlockClear(void *block);
s32 func_001B70F0();

void tableSubmitRequest(void) {
    s32 packet[8];

    xcBlockClear(&packet[0]);
    packet[1] = 0x0900000A;
    packet[2] = (s32) stateKeyTableRoot;
    func_001B70F0(&packet[0]);
    func_001B5F28(0, 0, 0);
}

s32 tableGetWord(u8 *table, s32 offset);
s32 func_002C7C50(u8 *table, s32 offset);

s32 stateSlotKeyLookup(s32 slotKey) {
    s32 slotKeyTableOffset;
    slotKeyTableOffset = tableGetWord(stateKeyTableRoot, 0x19);
    return func_002C7C50(&stateKeyTableRoot[slotKeyTableOffset], slotKey);
}

s32 groupKeyLookup(s32 groupId, s32 key) {
    s32 groupKeyTableOffset;
    groupKeyTableOffset = tableGetWord(stateKeyTableRoot, 1);
    return func_002C7C50(&stateKeyTableRoot[groupKeyTableOffset], key + (groupId * 8));
}

s32 groupKeySecondWordGet(s32 groupId) {
    s32 groupKeyTableOffset;
    groupKeyTableOffset = tableGetWord(stateKeyTableRoot, 1);
    return tableGetWord(&stateKeyTableRoot[groupKeyTableOffset], (groupId * 8) + 4);
}

s32 groupPairKeyValueGet(s32 groupId, s32 pairMember) {
    u8 *groupPairKeyTable;

    groupPairKeyTable = &stateKeyTableRoot[tableGetWord(stateKeyTableRoot, 3)];
    if (pairMember == 0) {
        return func_002C7C50(groupPairKeyTable, (groupId * 4) + 1);
    }
    if (pairMember == 1) {
        return func_002C7C50(groupPairKeyTable, (groupId * 4) + 3);
    }
    return 0;
}

s32 groupPairKeyAuxWordGet(s32 groupId) {
    s32 groupPairKeyTableOffset;
    groupPairKeyTableOffset = tableGetWord(stateKeyTableRoot, 3);
    return tableGetWord(&stateKeyTableRoot[groupPairKeyTableOffset], (groupId * 4) + 2);
}

void groupSlotKeyRecordFill(s32 *slotAttributes, s32 groupId, s32 slotNumber) {
    s32 slotKeyOffset;
    u8 *groupSlotKeyTable;

    groupSlotKeyTable = &stateKeyTableRoot[tableGetWord(stateKeyTableRoot, (groupId * 2) + 1)];
    slotKeyOffset = slotNumber * 8;
    slotAttributes[1] = func_002C7C50(groupSlotKeyTable, slotKeyOffset + 1);
    slotAttributes[2] = func_002C7C50(groupSlotKeyTable, slotKeyOffset + 2);
    slotAttributes[3] = tableGetWord(groupSlotKeyTable, slotKeyOffset + 3);
    slotAttributes[4] = tableGetWord(groupSlotKeyTable, slotKeyOffset + 4);
    slotAttributes[5] = tableGetWord(groupSlotKeyTable, slotKeyOffset + 5);
    slotAttributes[6] = tableGetWord(groupSlotKeyTable, slotKeyOffset + 6);
    slotAttributes[7] = tableGetWord(groupSlotKeyTable, slotKeyOffset + 7);
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CAC10);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CAD80);

void runGuardedStep(void) {
    s32 savedFlag;

    savedFlag = gVsyncsPerFrame;
    gVsyncsPerFrame = 1;
    func_002CB218();
    func_002CAE70();
    func_002CB3A0();
    gVsyncsPerFrame = savedFlag;
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CAE70);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CB148);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CB218);

/* Shared block at D_005FA500; only the offsets read or written by the routines below are named. */
typedef struct TuState {
    s32 jpegSource;          /* 0x00 */
    s32 jpegDestination;         /* 0x04 */
    s32 initValue08;          /* 0x08 */
    s32 slotKeyThreeLookup;             /* 0x0C */
    s32 initValue10;          /* 0x10 */
    s32 slotKeyTwoLookup;             /* 0x14 */
    s32 initValue18;          /* 0x18 */
    s32 slotKeyOneLookup;             /* 0x1C */
    s32 initValue20;          /* 0x20 */
    s32 slotKeyFourLookup;             /* 0x24 */
    s32 initValue28;          /* 0x28 */
    s32 initValue2C;          /* 0x2C */
    s32 initValue30;          /* 0x30 */
    s32 slotKeyElevenLookup;             /* 0x34 */
    s32 initValue38;          /* 0x38 */
    s32 slotKeyThirteenLookup;             /* 0x3C */
    s32 initValue40;          /* 0x40 */
    s32 initValue44;          /* 0x44 */
    s32 initValue48;          /* 0x48 */
    s32 slotKeyFourteenLookup;             /* 0x4C */
    s32 initValue50;          /* 0x50 */
    s32 slotKeyFifteenLookup;             /* 0x54 */
    s32 initValue58;          /* 0x58 */
    s32 slotKeySixteenLookup;             /* 0x5C */
    s32 initValue60;          /* 0x60 */
    s32 initValue64;          /* 0x64 */
    s32 initValue68;          /* 0x68 */
    s32 slotKeyTwentySixLookup;             /* 0x6C */
    s32 initValue70;          /* 0x70 */
    s32 initValue74;          /* 0x74 */
    u8 unk_78[0x348];
    s32 fadeInFrame;              /* 0x3C0 */
    u8 unk_3C4[0x4];
    s32 (*frameCallback)();       /* 0x3C8 */
    void (*decodeTickCallback)(void);  /* 0x3CC */
    s32 sheetIndex;            /* 0x3D0 */
    s32 groupOneKeyOneLookup;          /* 0x3D4 */
    u8 unk_3D8[0x4];
    s32 unk_3DC;
    u8 unk_3E0[0x30];
    s32 pendingCount;         /* 0x410 */
    u8 unk_414[0x4];
    s8 decodePhase;                 /* 0x418 */
    u8 unk_419[0x2];
    s8 frameCallbackFlag;          /* 0x41B */
} TuState;

extern unsigned char D_005FA500[];
/* Typed view of the shared block; the one place the extern is cast. */
#define TU_STATE ((TuState *) D_005FA500)
s32 sheetSetIndex();
s32 func_002209C0();
s32 func_00220E00();
s32 func_002C72D8();
s32 sineWavesAdvance();
void advanceCounter();

s8 runFrameCallbackAndGetFlag(void)
{
  TuState *tuState = TU_STATE;
  s32 (*frameCallback)();

  sineWavesAdvance();
  func_002C72D8();
  frameCallback = tuState->frameCallback;
  tuState->frameCallbackFlag = 0;
  frameCallback();
  advanceCounter();
  func_00220E00();
  sheetSetIndex(tuState->sheetIndex);
  func_002209C0();
  return tuState->frameCallbackFlag;
}

s32 fontQueueColorState(s32);
s32 xglFontSetColorIndex();
s32 xglFontGetColorIndex();
s32 xglFontGetStateMask();
s32 mcStateSeekEntry(s32);

void xglFontRunWithSavedColor(s32 stateEntryId) {
    s32 previousState;
    s32 previousMode;

    previousMode = xglFontGetColorIndex();
    previousState = xglFontGetStateMask();
    xglFontSetColorIndex(0xFFFF01);
    mcStateSeekEntry(stateEntryId);
    fontQueueColorState(previousState);
    xglFontSetColorIndex(previousMode);
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CB340);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CB3A0);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CB3F0);

extern s32 D_005FA580[];
s32 xcClearPayloadSections();
s32 xcLoadPayloadSections();
s32 blockStatusBitsSet();
s32 shardFieldInit();

void xcBlockPointersReload(void) {
    blockStatusBitsSet(0);
    xcClearPayloadSections();
    xcLoadPayloadSections();
    shardFieldInit(D_005FA580);
    D_005FA580[0x340 / 4] = 0;
}

s32 blockStatusAdvance();

void advanceCounter(void)
{
  TuState *tuState;
  s32 nextFadeInFrame;

  blockStatusAdvance();
  tuState = TU_STATE;
  nextFadeInFrame = tuState->fadeInFrame + 1;
  tuState->fadeInFrame = nextFadeInFrame;
}

s32 xglFontSetAlphaScale();
s32 shardFieldUpdate();

void blockFadeInDraw(void)
{
  s32 alphaScale;
  u8 fade;
  xglFontSetColorIndex(0x64);
  alphaScale = 0x80;
  if (TU_STATE->fadeInFrame < 0x10)
  {
    fade = TU_STATE->fadeInFrame;
    alphaScale = (fade * 8) & 0xFF;
  }
  xglFontSetAlphaScale(alphaScale);
  shardFieldUpdate(D_005FA580);
  xglFontSetAlphaScale(0x80);
  xglFontSetColorIndex(0xFFFF01);
}

s32 xcBlockFlagsReset(s32 *);

void xcBlockFlagsRefresh(void) {
    xcBlockFlagsReset(D_005FA580);
}

s32 mcModuleReset();
s32 stateSlotKeyLookup();

void initStateTable(void) {
    s32 slotKeyThreeLookup;
    s32 slotKeyTwoLookup;
    s32 slotKeyOneLookup;
    s32 slotKeyFourLookup;
    s32 slotKeyElevenLookup;
    s32 slotKeyThirteenLookup;
    s32 slotKeyFourteenLookup;
    s32 slotKeyFifteenLookup;
    s32 slotKeySixteenLookup;
    s32 slotKeyTwentySixLookup;

    mcModuleReset();
    TU_STATE->initValue08 = 0x10;
    slotKeyThreeLookup = stateSlotKeyLookup(3);
    TU_STATE->slotKeyThreeLookup = slotKeyThreeLookup;
    TU_STATE->initValue10 = 0x40;
    slotKeyTwoLookup = stateSlotKeyLookup(2);
    TU_STATE->slotKeyTwoLookup = slotKeyTwoLookup;
    TU_STATE->initValue18 = 0x20;
    slotKeyOneLookup = stateSlotKeyLookup(1);
    TU_STATE->initValue20 = 8;
    TU_STATE->slotKeyOneLookup = slotKeyOneLookup;
    slotKeyFourLookup = stateSlotKeyLookup(4);
    TU_STATE->initValue28 = 4;
    TU_STATE->initValue30 = 0x80;
    TU_STATE->slotKeyFourLookup = slotKeyFourLookup;
    TU_STATE->initValue2C = 0;
    slotKeyElevenLookup = stateSlotKeyLookup(0xB);
    TU_STATE->initValue38 = 8;
    TU_STATE->slotKeyElevenLookup = slotKeyElevenLookup;
    slotKeyThirteenLookup = stateSlotKeyLookup(0xD);
    TU_STATE->initValue40 = 4;
    TU_STATE->initValue48 = 0x80;
    TU_STATE->slotKeyThirteenLookup = slotKeyThirteenLookup;
    TU_STATE->initValue44 = 0;
    slotKeyFourteenLookup = stateSlotKeyLookup(0xE);
    TU_STATE->initValue50 = 0x80;
    TU_STATE->slotKeyFourteenLookup = slotKeyFourteenLookup;
    slotKeyFifteenLookup = stateSlotKeyLookup(0xF);
    TU_STATE->initValue58 = 8;
    TU_STATE->slotKeyFifteenLookup = slotKeyFifteenLookup;
    slotKeySixteenLookup = stateSlotKeyLookup(0x10);
    TU_STATE->initValue68 = 8;
    TU_STATE->initValue60 = 4;
    TU_STATE->slotKeySixteenLookup = slotKeySixteenLookup;
    TU_STATE->initValue64 = 0;
    slotKeyTwentySixLookup = stateSlotKeyLookup(0x1A);
    TU_STATE->initValue70 = 4;
    TU_STATE->slotKeyTwentySixLookup = slotKeyTwentySixLookup;
    TU_STATE->initValue74 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CB768);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CBAF8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CBD18);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC100);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC270);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC428);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC5E0);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC6E8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC780);

s32 func_002E0858();

void decodeStateTickA(void) {
    func_002E0858();
}

s32 groupKeyLookup();
s32 func_002CB768();
s32 func_002CC8F8();
void decodeStateTickB(void);
s32 decodeStateInit();
s32 decodeSetFlag();

void initDecodeState(void)
{
  s32 groupOneKeyOneLookup;
  decodeStateInit();
  groupOneKeyOneLookup = groupKeyLookup(1, 1);
  TU_STATE->groupOneKeyOneLookup = groupOneKeyOneLookup;
  TU_STATE->decodeTickCallback = decodeStateTickB;
  TU_STATE->frameCallback = func_002CC8F8;
  TU_STATE->unk_3DC = 0;
  decodeSetFlag(0);
  func_002CB768(9);
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CC8F8);

s32 decodeUpdate();

void decodeStateTickB(void) {
    decodeUpdate();
}

s32 func_001B6520();

void allocateBuffers(void) {
    s32 jpegSource;
    s32 jpegDestination;

    jpegSource = func_001B6520(0x9800);
    TU_STATE->jpegSource = jpegSource;
    jpegDestination = func_001B6520(0x25000);
    TU_STATE->jpegDestination = jpegDestination;
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CCA50);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CCAC8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CCCB8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CCE40);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CCF60);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CD120);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CD4D8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CD690);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CD7D0);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CD9B0);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CDAF8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CDC88);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CDDF8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CDF90);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CE278);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CE568);

void func_002CCF60();
void func_002CE278();
void func_002CE568();
s32 func_002CE7E0();
void func_002CED08();

void advanceState(void)
{
  TuState *tuState;

  func_002CCF60();
  func_002CE278();
  func_002CE568();
  func_002CED08();
  tuState = TU_STATE;
  if ((tuState->decodePhase == 2) && (func_002CE7E0() != 0))
  {
    tuState->decodePhase = 3;
  }
  func_002CE7E0();
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CE7E0);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CE9C8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CEAA0);

s32 masterFlagTest();

s32 masterFlagIsSet(void) {
    return masterFlagTest() != 0;
}

extern unsigned char jpegDecodeStatus[];
s32 func_001B60B8();
s32 memset();
s32 xglJpegDecode();

void runJpegDecode(void)
{
  TuState *tuState = TU_STATE;
  s32 jpegRequest[8];
  s32 pollStatus;
  s32 jpegSource;

  if (tuState->pendingCount <= 0)
  {
    jpegDecodeStatus[0] = 0;
    return;
  }
  pollStatus = func_001B60B8(0);
  if (pollStatus == 1)
  {
    memset(&jpegRequest[0], 0, 0x20);
    jpegSource = tuState->jpegSource;
    jpegRequest[1] = tuState->jpegDestination;
    jpegRequest[0] = jpegSource;
    xglJpegDecode(&jpegRequest[0]);
    jpegDecodeStatus[0] = pollStatus;
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CEBC8);

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CED08);

extern unsigned char decodeIndexCurrent[];

void decodeIndexClear(void) {
    decodeIndexCurrent[0] = -1;
}

void decodeIndexSet(s32 decodeIndex) {
    decodeIndexCurrent[0] = decodeIndex;
}

INCLUDE_ASM("asm/main/nonmatchings/tu388", func_002CF038);
