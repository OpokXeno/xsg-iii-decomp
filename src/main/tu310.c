#include "common.h"
#include "main/xgl_render.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef float f32;

/* Record returned by channelTableSelectedEndGet / movieFirstStateGet; movieSlotIndex picks an entry of a movie table. */
typedef struct MovieState
{
  u8 unk00[4];
  s32 movieSlotIndex;
} MovieState;

/* gMovieObject holds the address of the movie object; the buffer at its +0x30 receives the text that GameMpeg2Play formats.
 * Only the address of textBuffer is used; its size is not confirmed. */
typedef struct MovieObject
{
  u8 unk00[0x30];
  u8 textBuffer[1];
} MovieObject;

/* Records of 0xA0 bytes; the parameter word sits at offset 4 of each record. */
typedef struct ChannelParams
{
  u8 unk00[4];
  s32 channelSetting;
  u8 unk08[0x98];
} ChannelParams;

extern s32 gMovieLatchValue05;
extern s32 gMovieLatchValue02;
extern MovieObject *gMovieObject;
extern unsigned char gMovieFormatMain[];
extern unsigned char gMovieFormatDefault[];
extern unsigned char gMovieFormatBank03[];
extern unsigned char gMovieFormatSlotIndex[];
extern unsigned char gMovieFormatFloat[];
extern s32 gMovieFormatArgs[];
extern s32 gMovieLatchTable05[];
extern s32 gMovieLatchTable02[];
extern s32 gMoviePlayBankOneSelector[];
extern s32 gMoviePlayBankOneOffsets[];
extern s32 gMoviePlayBankTwoSelector[];
extern s32 gMoviePlayBankTwoOffsets[];
extern s32 gMoviePlayBankThreeSelector[];
extern s32 gMoviePlayBankThreeOffsets[];
extern s32 gMoviePlayBankFourSelector[];
extern s32 gMoviePlayBankFourOffsets[];
extern s32 gMoviePlayBankFiveSelector[];
extern s32 gMoviePlayBankFiveOffsets[];
extern s32 gMoviePlayBankSixSelector[];
extern s32 gMoviePlayBankSixOffsets[];
extern s32 gMoviePlayFloatBankASelector[];
extern f32 gMoviePlayFloatBankAValues[];
extern s32 gMoviePlayFloatBankBSelector[];
extern f32 gMoviePlayFloatBankBValues[];
extern u16 gMovieAutoSelectFlags[];
extern ChannelParams gMovieChannelsResetA[];
extern ChannelParams gMovieChannelsResetB[];
extern ChannelParams gMovieLatchChannels[];
extern ChannelParams gMovieChannelsResetC[];
extern ChannelParams gMovieChannelsResetD[];
extern ChannelParams gMovieChannelsResetE[];

s32 GameMpeg2Play();
MovieState *movieFirstStateGet();
s32 channelTableSelectedEndGet();

#define movieSelectedState() ((MovieState *) channelTableSelectedEndGet())
s32 func_0028B268();
s32 func_0028B228();
s32 cycleStepARange255();
s32 cycleStepBRange255();
s32 cycleStepARange512();
s32 cycleStepBRange512();
s32 xglCameraSetWindow();
s32 xglStudioSelectGetActiveCamera();
s32 fptodp(f32);
s32 func_00287848();
s32 XenoActorReleaseAll();
s32 func_001E7EC8();

void moviePlayFromStateTable(void) {
    unsigned char *formatString;
    s32 *formatArgTable;
    unsigned char *textBuffer;
    MovieState *auxFirstMovieSlot;

    formatString = gMovieFormatMain;
    formatArgTable = gMovieFormatArgs;
    textBuffer = gMovieObject->textBuffer;
    auxFirstMovieSlot = movieFirstStateGet();
    GameMpeg2Play(textBuffer, formatString, formatArgTable[auxFirstMovieSlot->movieSlotIndex]);
}

void movieCueActivate02(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 2);
}

void movieCueDeactivate02(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 2);
}

void moviePlayStateIndex(void) {
    unsigned char *formatString;
    unsigned char *textBuffer;
    MovieState *auxFirstMovieSlot;

    formatString = gMovieFormatSlotIndex;
    textBuffer = gMovieObject->textBuffer;
    auxFirstMovieSlot = movieFirstStateGet();
    GameMpeg2Play(textBuffer, formatString, auxFirstMovieSlot->movieSlotIndex);
}


typedef struct EnvParams
{
  u8 unk00[4];
  u8 unk04;
  u8 unk05[0xA4 - 0x05];
  u8 unkA4;
  u8 unkA5[0x144 - 0xA5];
  u8 unk144;
} EnvParams;

extern EnvParams gEnvParams[];

void envFlagsCopyFromParams(void)
{
  gEnvFlags.color.red = gEnvParams[0].unk04;
  gEnvFlags.color.green = gEnvParams[0].unkA4;
  gEnvFlags.color.blue = gEnvParams[0].unk144;
}


void envFlagsRefreshA(void) {
    cycleStepARange255();
    envFlagsCopyFromParams();
}

void envFlagsRefreshB(void) {
    cycleStepBRange255();
    envFlagsCopyFromParams();
}

void envParamsResetAndCopy(void)
{
  ChannelParams *channelParams = (ChannelParams *) gEnvParams;
  channelParams[0].channelSetting = 0x30;
  channelParams[1].channelSetting = 0x30;
  channelParams[2].channelSetting = 0x30;
  envFlagsCopyFromParams();
}

extern ChannelParams gEnvCameraParams[];

void envApplyActiveCameraParams(void)
{
  ChannelParams *channelParams;
  s32 activeCamera;

  activeCamera = xglStudioSelectGetActiveCamera(0);
  channelParams = gEnvCameraParams;
  xglCameraSetWindow(activeCamera, channelParams[0].channelSetting, channelParams[1].channelSetting, channelParams[2].channelSetting, channelParams[3].channelSetting);
}

void envCameraRefreshA(void) {
    cycleStepARange512();
    envApplyActiveCameraParams();
}

void envCameraRefreshB(void) {
    cycleStepBRange512();
    envApplyActiveCameraParams();
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287848);

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287880);

void movieNoOpStub01(void) {

}

void movieCueActivateAuto04(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 4, 0x200);
}

void movieCueDeactivateAuto04(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 4, 0x200);
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287938);

void movieNoOpStub02(void) {

}

void movieCueActivateAuto03(void)
{
  func_0028B268(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 3, 0x200);
}

void movieCueDeactivateAuto03(void)
{
  func_0028B228(channelTableSelectedEndGet() + 4, (gMovieAutoSelectFlags[0] & 0x80) ? 0xA : 1, 3, 0x200);
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_002879F8);

void movieNoOpStub03(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287A58);

void movieNoOpStub04(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287A98);

void movieNoOpStub05(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287AE0);

void movieNoOpStub06(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287B28);

void movieNoOpStub07(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287B78);

void movieNoOpStub08(void) {

}

void movieChannelResetA(void)
{
  ChannelParams *channelParams = gMovieChannelsResetA;
  channelParams[0].channelSetting = 0xE0;
  channelParams[1].channelSetting = 3;
  channelParams[2].channelSetting = 0xC0;
  channelParams[3].channelSetting = 0;
}

void movieNoOpStub09(void) {

}

void movieChannelResetB(void)
{
  ChannelParams *channelParams = gMovieChannelsResetB;
  channelParams[0].channelSetting = 0xE0;
  channelParams[1].channelSetting = 0xFF;
  channelParams[2].channelSetting = 0;
}

void moviePlayerLatchStateA(void) {
    ChannelParams *latchChannels;
    s32 selectedSlotA;
    s32 selectedSlotB;
    s32 selectedSlotC;

    latchChannels = gMovieLatchChannels;
    cycleStepARange255();
    latchChannels = gMovieLatchChannels;
    selectedSlotA = movieSelectedState()->movieSlotIndex;
    latchChannels[3].channelSetting = selectedSlotA;
    selectedSlotB = movieSelectedState()->movieSlotIndex;
    latchChannels[4].channelSetting = selectedSlotB;
    selectedSlotC = movieSelectedState()->movieSlotIndex;
    latchChannels[5].channelSetting = selectedSlotC;
}

void moviePlayerLatchStateB(void) {
    ChannelParams *latchChannels;
    s32 selectedSlotA;
    s32 selectedSlotB;
    s32 selectedSlotC;

    latchChannels = gMovieLatchChannels;
    cycleStepBRange255();
    latchChannels = gMovieLatchChannels;
    selectedSlotA = movieSelectedState()->movieSlotIndex;
    latchChannels[3].channelSetting = selectedSlotA;
    selectedSlotB = movieSelectedState()->movieSlotIndex;
    latchChannels[4].channelSetting = selectedSlotB;
    selectedSlotC = movieSelectedState()->movieSlotIndex;
    latchChannels[5].channelSetting = selectedSlotC;
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287CC0);

void movieNoOpStub10(void) {

}

void movieCueActivate02Twin(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 2);
}

void movieCueDeactivate02Twin(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 2);
}

void moviePlayBank01(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatMain, gMoviePlayBankOneOffsets[gMoviePlayBankOneSelector[0]]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287DA8);

void movieNoOpStub11(void) {

}

void movieCueActivate01(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 1);
}

void movieCueDeactivate01(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 1);
}

void moviePlayBank02(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatMain, gMoviePlayBankTwoOffsets[gMoviePlayBankTwoSelector[0]]);
}

void movieChannelResetC(void)
{
  ChannelParams *channelParams = gMovieChannelsResetC;
  channelParams[0].channelSetting = 0xFFFF;
  channelParams[2].channelSetting = 0x14;
  channelParams[3].channelSetting = 5;
  channelParams[4].channelSetting = 0x80;
  channelParams[1].channelSetting = 0;
  channelParams[5].channelSetting = 0;
  channelParams[6].channelSetting = 0;
}

void movieNoOpStub12(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287EE8);

void movieNoOpStub13(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287F40);

void movieNoOpStub14(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00287F88);

void movieNoOpStub15(void) {

}

void movieCueActivate08(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 8);
}

void movieCueDeactivate08(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 8);
}

void moviePlayBank03(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatBank03, gMoviePlayBankThreeOffsets[gMoviePlayBankThreeSelector[0]]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288098);

void movieNoOpStub16(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_002880F0);

void movieNoOpStub17(void) {

}

void movieChannelResetD(void)
{
  ChannelParams *channelParams = gMovieChannelsResetD;
  s32 clearedSetting;
  clearedSetting = 0;
  channelParams[2].channelSetting = 0x80;
  channelParams[4].channelSetting = clearedSetting;
  do { channelParams[0].channelSetting = clearedSetting; channelParams[1].channelSetting = clearedSetting; } while (0);
  channelParams[3].channelSetting = clearedSetting;
}

void movieNoOpStub18(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288168);

void movieNoOpStub19(void) {

}

void movieCueActivate03(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 3);
}

void movieCueDeactivate03(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 3);
}

void moviePlayBank04(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatMain, gMoviePlayBankFourOffsets[gMoviePlayBankFourSelector[0]]);
}

void movieChannelResetE(void)
{
  ChannelParams *channelParams = gMovieChannelsResetE;
  channelParams[0].channelSetting = 0;
  channelParams[1].channelSetting = 0;
  channelParams[2].channelSetting = 0;
}

void movieNoOpStub20(void) {

}

void movieNoOpStub21(void) {

}

void movieNoOpStub22(void) {

}

void movieNoOpStub23(void) {

}

void movieNoOpStub24(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288278);

void movieNoOpStub25(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288318);

void movieNoOpStub26(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_002883B8);

void movieNoOpStub27(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288458);

void movieNoOpStub28(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_002884F8);

void movieNoOpStub29(void) {

}

void movieCueActivate0C(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 0xC);
}

void movieCueDeactivate0C(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 0xC);
}

extern unsigned char gMovieFormatFloat[];

void moviePlayFloatBankA(void)
{
  GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatFloat, fptodp(gMoviePlayFloatBankAValues[gMoviePlayFloatBankASelector[0]]));
}

void movieCueActivate09(void) {
    func_0028B268(channelTableSelectedEndGet() + 4, 1, 0, 9);
}

void movieCueDeactivate09(void) {
    func_0028B228(channelTableSelectedEndGet() + 4, 1, 0, 9);
}

void moviePlayFloatBankB(void)
{
  GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatFloat, fptodp(gMoviePlayFloatBankBValues[gMoviePlayFloatBankBSelector[0]]));
}

extern ChannelParams gMovieSetupChannels[];

void movieSetupChannels(void)
{
  ChannelParams *channelParams = gMovieSetupChannels;
  channelParams[0].channelSetting = 1;
  channelParams[1].channelSetting = 6;
  channelParams[2].channelSetting = 4;
  channelParams[3].channelSetting = 0;
  envParamsResetAndCopy();
  func_00287848();
}

void movieCueActivate05Latch(void) {
    s32 selectedTableEnd;
    s32 movieLatchValue;

    selectedTableEnd = channelTableSelectedEndGet();
    func_0028B268(selectedTableEnd + 4, 1, 0, 5);
    movieLatchValue = gMovieLatchTable05[movieSelectedState()->movieSlotIndex];
    gMovieLatchValue05 = movieLatchValue;
}

void movieCueDeactivate05Latch(void) {
    s32 selectedTableEnd;
    s32 movieLatchValue;

    selectedTableEnd = channelTableSelectedEndGet();
    func_0028B228(selectedTableEnd + 4, 1, 0, 5);
    movieLatchValue = gMovieLatchTable05[movieSelectedState()->movieSlotIndex];
    gMovieLatchValue05 = movieLatchValue;
}

void moviePlayBank05(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatMain, gMoviePlayBankFiveOffsets[gMoviePlayBankFiveSelector[0]]);
}

void movieCueActivate02Latch(void) {
    s32 selectedTableEnd;
    s32 movieLatchValue;

    selectedTableEnd = channelTableSelectedEndGet();
    func_0028B268(selectedTableEnd + 4, 1, 0, 2);
    movieLatchValue = gMovieLatchTable02[movieSelectedState()->movieSlotIndex];
    gMovieLatchValue02 = movieLatchValue;
}

void movieCueDeactivate02Latch(void) {
    s32 selectedTableEnd;
    s32 movieLatchValue;

    selectedTableEnd = channelTableSelectedEndGet();
    func_0028B228(selectedTableEnd + 4, 1, 0, 2);
    movieLatchValue = gMovieLatchTable02[movieSelectedState()->movieSlotIndex];
    gMovieLatchValue02 = movieLatchValue;
}

void moviePlayBank06(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatMain, gMoviePlayBankSixOffsets[gMoviePlayBankSixSelector[0]]);
}

void movieStateSetActive(void) {
    movieSelectedState()->movieSlotIndex = 1;
}

extern unsigned char gMovieFormatDefault[];

void moviePlayDefault(void) {
    GameMpeg2Play(gMovieObject->textBuffer, gMovieFormatDefault);
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288988);

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288AC0);

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288D78);

s32 movieReleaseActors(void) {
    XenoActorReleaseAll();
    func_001E7EC8();
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00288E30);

void *memset(void *destination, int value, unsigned int size);
s32 slotSetupMode0(s32 slotIndex, s32 setupValue, s32 *values);
void slotReleaseChecked(s32 slotIndex, s32 forceRelease);
extern ChannelParams D_003EC9C0[];
extern ChannelParams D_003ED160[];
extern ChannelParams D_003ED860[];
extern ChannelParams D_003EE280[];
extern ChannelParams D_003EE980[];
extern ChannelParams D_003EF120[];
extern ChannelParams D_003EF960[];
extern ChannelParams D_003F02E0[];
extern ChannelParams D_003F1E40[];
extern ChannelParams D_003F2F78[];
extern ChannelParams D_003F3AD8[];
extern ChannelParams D_003F43B8[];
extern ChannelParams D_003F4FB8[];
extern ChannelParams D_003F5960[];
extern ChannelParams D_003F66C0[];
extern ChannelParams D_003F7110[];
extern s32 D_003F8670[];
extern ChannelParams D_003F86B0[];
extern s32 D_003F9C10[];
extern ChannelParams D_003F9C50[];
extern s32 D_003FB1B0[];
extern ChannelParams D_003FB1F0[];
extern s32 D_003FC750[];
extern ChannelParams D_003FC790[];
extern s32 D_003FDCF0[];
extern ChannelParams D_003FDD30[];
extern s32 D_003FF290[];
extern ChannelParams D_003FF2D0[];
extern s32 D_00400830[];
extern ChannelParams gMovieChannelEnable[];
extern u16 gPadDataPressed[];

/* Debug/test table driver: for each of 28 channels, when its enable word in gMovieChannelEnable is set, gather the
 * channel's parameter words into setupArgs (some copied through to second tables) and start the slot with
 * slotSetupMode0; START on pad 0 calls the channel's no-op stub; a disabled channel releases its slot. */
#define PAD_START 0x0800

/* A channel whose setting is a long value: read through its long view, which does not
 * alias the s32 argument block. */
#define CHANNEL_LONG_SETTING(table, index) (*(long *)&(table)[index].channelSetting)

void movieChannelSlotsUpdate(void)
{
    s32 setupArgs[15];

    memset(setupArgs, 0, sizeof(setupArgs));
    if (gMovieChannelEnable[0].channelSetting != 0) {
        setupArgs[0] = D_003EC9C0[0].channelSetting;
        setupArgs[1] = D_003EC9C0[1].channelSetting;
        setupArgs[2] = D_003EC9C0[2].channelSetting;
        setupArgs[3] = D_003EC9C0[3].channelSetting;
        setupArgs[4] = D_003EC9C0[4].channelSetting;
        setupArgs[5] = D_003EC9C0[5].channelSetting;
        setupArgs[6] = D_003EC9C0[6].channelSetting;
        setupArgs[7] = D_003EC9C0[7].channelSetting;
        slotSetupMode0(204, 4, setupArgs);
    } else {
        slotReleaseChecked(4, 0);
    }
    if (gMovieChannelEnable[1].channelSetting != 0) {
        setupArgs[0] = D_003ED160[0].channelSetting << 8;
        setupArgs[1] = D_003ED160[1].channelSetting + 256;
        setupArgs[2] = D_003ED160[2].channelSetting + 224;
        setupArgs[3] = D_003ED160[3].channelSetting;
        setupArgs[4] = D_003ED160[4].channelSetting;
        setupArgs[5] = D_003ED160[5].channelSetting;
        setupArgs[6] = CHANNEL_LONG_SETTING(D_003ED160, 6);
        slotSetupMode0(212, 12, setupArgs);
    } else {
        slotReleaseChecked(12, 0);
    }
    if (gMovieChannelEnable[2].channelSetting != 0) {
        setupArgs[0] = D_003ED860[0].channelSetting << 8;
        setupArgs[1] = D_003ED860[1].channelSetting + 256;
        setupArgs[2] = D_003ED860[2].channelSetting + 224;
        setupArgs[3] = D_003ED860[3].channelSetting;
        setupArgs[4] = D_003ED860[4].channelSetting;
        setupArgs[5] = D_003ED860[5].channelSetting;
        setupArgs[6] = CHANNEL_LONG_SETTING(D_003ED860, 6);
        setupArgs[7] = CHANNEL_LONG_SETTING(D_003ED860, 7);
        setupArgs[8] = CHANNEL_LONG_SETTING(D_003ED860, 8);
        setupArgs[9] = CHANNEL_LONG_SETTING(D_003ED860, 9);
        setupArgs[10] = CHANNEL_LONG_SETTING(D_003ED860, 10);
        setupArgs[11] = CHANNEL_LONG_SETTING(D_003ED860, 11);
        slotSetupMode0(238, 38, setupArgs);
    } else {
        slotReleaseChecked(38, 0);
    }
    if (gMovieChannelEnable[3].channelSetting != 0) {
        setupArgs[0] = D_003EE280[0].channelSetting << 8;
        setupArgs[1] = D_003EE280[1].channelSetting;
        setupArgs[2] = D_003EE280[2].channelSetting;
        setupArgs[3] = D_003EE280[3].channelSetting;
        setupArgs[4] = D_003EE280[4].channelSetting;
        setupArgs[5] = D_003EE280[5].channelSetting;
        setupArgs[6] = CHANNEL_LONG_SETTING(D_003EE280, 6);
        slotSetupMode0(219, 19, setupArgs);
    } else {
        slotReleaseChecked(19, 0);
    }
    if (gMovieChannelEnable[4].channelSetting != 0) {
        setupArgs[0] = D_003EE980[0].channelSetting << 8;
        setupArgs[1] = D_003EE980[1].channelSetting;
        setupArgs[2] = D_003EE980[2].channelSetting;
        setupArgs[3] = D_003EE980[3].channelSetting;
        setupArgs[4] = D_003EE980[4].channelSetting;
        setupArgs[5] = D_003EE980[5].channelSetting;
        setupArgs[6] = D_003EE980[6].channelSetting;
        setupArgs[7] = CHANNEL_LONG_SETTING(D_003EE980, 7);
        slotSetupMode0(220, 20, setupArgs);
    } else {
        slotReleaseChecked(20, 0);
    }
    if (gMovieChannelEnable[5].channelSetting != 0) {
        setupArgs[0] = D_003EF120[0].channelSetting << 8;
        setupArgs[1] = D_003EF120[1].channelSetting;
        setupArgs[2] = D_003EF120[2].channelSetting;
        setupArgs[3] = D_003EF120[3].channelSetting;
        setupArgs[4] = D_003EF120[4].channelSetting;
        setupArgs[5] = D_003EF120[5].channelSetting;
        setupArgs[6] = D_003EF120[6].channelSetting;
        setupArgs[7] = CHANNEL_LONG_SETTING(D_003EF120, 7);
        setupArgs[8] = CHANNEL_LONG_SETTING(D_003EF120, 8);
        slotSetupMode0(211, 11, setupArgs);
    } else {
        slotReleaseChecked(11, 0);
    }
    if (gMovieChannelEnable[6].channelSetting != 0) {
        setupArgs[0] = D_003EF960[0].channelSetting << 8;
        setupArgs[1] = D_003EF960[1].channelSetting + 256;
        setupArgs[2] = D_003EF960[2].channelSetting + 224;
        setupArgs[3] = D_003EF960[3].channelSetting;
        setupArgs[4] = D_003EF960[4].channelSetting;
        setupArgs[5] = D_003EF960[5].channelSetting;
        setupArgs[6] = D_003EF960[6].channelSetting;
        setupArgs[7] = D_003EF960[7].channelSetting;
        setupArgs[8] = D_003EF960[8].channelSetting;
        setupArgs[9] = D_003EF960[9].channelSetting;
        setupArgs[10] = CHANNEL_LONG_SETTING(D_003EF960, 10);
        slotSetupMode0(217, 17, setupArgs);
    } else {
        slotReleaseChecked(17, 0);
    }
    if (gMovieChannelEnable[7].channelSetting != 0) {
        setupArgs[0] = D_003F02E0[0].channelSetting << 8;
        setupArgs[1] = D_003F02E0[1].channelSetting + 256;
        setupArgs[2] = D_003F02E0[2].channelSetting + 224;
        setupArgs[3] = D_003F02E0[3].channelSetting;
        setupArgs[4] = D_003F02E0[4].channelSetting;
        setupArgs[5] = D_003F02E0[5].channelSetting;
        setupArgs[6] = D_003F02E0[6].channelSetting;
        setupArgs[7] = D_003F02E0[7].channelSetting;
        setupArgs[8] = D_003F02E0[8].channelSetting;
        setupArgs[9] = D_003F02E0[9].channelSetting;
        setupArgs[10] = CHANNEL_LONG_SETTING(D_003F02E0, 10);
        slotSetupMode0(218, 18, setupArgs);
    } else {
        slotReleaseChecked(18, 0);
    }
    if (gMovieChannelEnable[8].channelSetting != 0) {
        setupArgs[0] = gMovieChannelsResetA[0].channelSetting << 8;
        setupArgs[1] = gMovieChannelsResetA[1].channelSetting;
        setupArgs[2] = gMovieChannelsResetA[2].channelSetting;
        setupArgs[3] = gMovieChannelsResetA[3].channelSetting;
        slotSetupMode0(221, 21, setupArgs);
    } else {
        slotReleaseChecked(21, 0);
    }
    if (gMovieChannelEnable[9].channelSetting != 0) {
        setupArgs[0] = gMovieChannelsResetB[0].channelSetting << 8;
        setupArgs[1] = gMovieChannelsResetB[1].channelSetting;
        setupArgs[2] = gMovieChannelsResetB[2].channelSetting;
        slotSetupMode0(237, 37, setupArgs);
    } else {
        slotReleaseChecked(37, 0);
    }
    if (gMovieChannelEnable[10].channelSetting != 0) {
        setupArgs[0] = gMovieLatchChannels[0].channelSetting << 8;
        setupArgs[1] = gMovieLatchChannels[1].channelSetting;
        setupArgs[2] = gMovieLatchChannels[2].channelSetting;
        setupArgs[3] = gMovieLatchChannels[3].channelSetting;
        setupArgs[4] = gMovieLatchChannels[4].channelSetting;
        setupArgs[5] = gMovieLatchChannels[5].channelSetting;
        setupArgs[6] = gMovieLatchChannels[7].channelSetting;
        setupArgs[7] = gMovieLatchChannels[8].channelSetting;
        setupArgs[8] = gMovieLatchChannels[9].channelSetting;
        slotSetupMode0(224, 24, setupArgs);
    } else {
        slotReleaseChecked(24, 0);
    }
    if (gMovieChannelEnable[11].channelSetting != 0) {
        setupArgs[0] = D_003F1E40[0].channelSetting << 8;
        setupArgs[1] = D_003F1E40[1].channelSetting;
        setupArgs[2] = D_003F1E40[2].channelSetting;
        setupArgs[3] = D_003F1E40[3].channelSetting;
        setupArgs[4] = D_003F1E40[4].channelSetting;
        setupArgs[5] = D_003F1E40[5].channelSetting;
        setupArgs[6] = D_003F1E40[6].channelSetting;
        setupArgs[7] = D_003F1E40[7].channelSetting;
        setupArgs[8] = D_003F1E40[8].channelSetting;
        setupArgs[9] = D_003F1E40[9].channelSetting;
        setupArgs[10] = D_003F1E40[10].channelSetting;
        setupArgs[11] = D_003F1E40[11].channelSetting;
        slotSetupMode0(222, 22, setupArgs);
    } else {
        slotReleaseChecked(22, 0);
    }
    if (gMovieChannelEnable[12].channelSetting != 0) {
        setupArgs[0] = gMovieChannelsResetC[0].channelSetting << 8;
        setupArgs[1] = gMovieChannelsResetC[1].channelSetting;
        setupArgs[2] = gMovieChannelsResetC[2].channelSetting;
        setupArgs[3] = gMovieChannelsResetC[3].channelSetting;
        setupArgs[4] = gMovieChannelsResetC[4].channelSetting;
        setupArgs[5] = gMovieChannelsResetC[5].channelSetting;
        setupArgs[6] = gMovieChannelsResetC[6].channelSetting;
        slotSetupMode0(223, 23, setupArgs);
    } else {
        slotReleaseChecked(23, 0);
    }
    if (gMovieChannelEnable[13].channelSetting != 0) {
        setupArgs[0] = D_003F2F78[0].channelSetting << 8;
        setupArgs[1] = D_003F2F78[1].channelSetting + 256;
        setupArgs[2] = D_003F2F78[2].channelSetting + 224;
        setupArgs[3] = D_003F2F78[3].channelSetting;
        setupArgs[4] = D_003F2F78[4].channelSetting;
        setupArgs[5] = D_003F2F78[5].channelSetting;
        setupArgs[6] = D_003F2F78[6].channelSetting;
        setupArgs[7] = D_003F2F78[7].channelSetting;
        setupArgs[8] = D_003F2F78[8].channelSetting;
        setupArgs[9] = D_003F2F78[9].channelSetting;
        setupArgs[10] = D_003F2F78[10].channelSetting;
        setupArgs[11] = D_003F2F78[11].channelSetting;
        setupArgs[12] = D_003F2F78[12].channelSetting;
        setupArgs[13] = D_003F2F78[13].channelSetting;
        slotSetupMode0(213, 13, setupArgs);
    } else {
        slotReleaseChecked(13, 0);
    }
    if (gMovieChannelEnable[14].channelSetting != 0) {
        setupArgs[0] = D_003F3AD8[0].channelSetting << 8;
        setupArgs[1] = D_003F3AD8[1].channelSetting + 256;
        setupArgs[2] = D_003F3AD8[2].channelSetting + 224;
        setupArgs[3] = D_003F3AD8[3].channelSetting;
        setupArgs[4] = D_003F3AD8[4].channelSetting;
        setupArgs[5] = D_003F3AD8[5].channelSetting;
        setupArgs[6] = D_003F3AD8[6].channelSetting;
        setupArgs[7] = D_003F3AD8[7].channelSetting;
        setupArgs[8] = D_003F3AD8[8].channelSetting;
        setupArgs[9] = D_003F3AD8[9].channelSetting;
        slotSetupMode0(226, 26, setupArgs);
    } else {
        slotReleaseChecked(26, 0);
    }
    if (gMovieChannelEnable[15].channelSetting != 0) {
        setupArgs[0] = D_003F43B8[0].channelSetting;
        setupArgs[1] = D_003F43B8[1].channelSetting;
        setupArgs[2] = D_003F43B8[2].channelSetting << 8;
        setupArgs[3] = D_003F43B8[3].channelSetting;
        setupArgs[4] = D_003F43B8[4].channelSetting;
        setupArgs[5] = D_003F43B8[5].channelSetting;
        setupArgs[6] = D_003F43B8[6].channelSetting;
        setupArgs[7] = D_003F43B8[7].channelSetting;
        setupArgs[8] = D_003F43B8[8].channelSetting;
        setupArgs[9] = D_003F43B8[9].channelSetting;
        setupArgs[10] = D_003F43B8[10].channelSetting | (D_003F43B8[11].channelSetting << 8) | (D_003F43B8[12].channelSetting << 16) | (D_003F43B8[13].channelSetting << 24);
        setupArgs[11] = 8;
        setupArgs[14] = D_003F43B8[14].channelSetting;
        slotSetupMode0(225, 25, setupArgs);
    } else {
        slotReleaseChecked(25, 0);
    }
    if (gMovieChannelEnable[16].channelSetting != 0) {
        setupArgs[0] = D_003F4FB8[0].channelSetting << 8;
        setupArgs[1] = D_003F4FB8[1].channelSetting + 256;
        setupArgs[2] = D_003F4FB8[2].channelSetting + 224;
        setupArgs[3] = D_003F4FB8[3].channelSetting;
        setupArgs[4] = D_003F4FB8[4].channelSetting;
        setupArgs[5] = gMoviePlayBankThreeOffsets[D_003F4FB8[5].channelSetting];
        setupArgs[6] = D_003F4FB8[6].channelSetting;
        setupArgs[7] = D_003F4FB8[7].channelSetting;
        setupArgs[8] = D_003F4FB8[8].channelSetting;
        setupArgs[9] = D_003F4FB8[9].channelSetting;
        setupArgs[10] = D_003F4FB8[10].channelSetting;
        slotSetupMode0(205, 5, setupArgs);
    } else {
        slotReleaseChecked(5, 0);
    }
    if (gMovieChannelEnable[17].channelSetting != 0) {
        setupArgs[0] = D_003F5960[0].channelSetting << 8;
        setupArgs[1] = D_003F5960[1].channelSetting + 256;
        setupArgs[2] = D_003F5960[2].channelSetting + 224;
        setupArgs[3] = D_003F5960[3].channelSetting;
        setupArgs[4] = D_003F5960[4].channelSetting;
        setupArgs[5] = D_003F5960[5].channelSetting;
        setupArgs[6] = D_003F5960[6].channelSetting;
        setupArgs[7] = D_003F5960[7].channelSetting;
        slotSetupMode0(206, 6, setupArgs);
    } else {
        slotReleaseChecked(6, 0);
    }
    if (gMovieChannelEnable[18].channelSetting != 0) {
        setupArgs[0] = gMovieChannelsResetD[0].channelSetting;
        setupArgs[1] = gMovieChannelsResetD[1].channelSetting;
        setupArgs[2] = gMovieChannelsResetD[2].channelSetting;
        setupArgs[3] = gMovieChannelsResetD[3].channelSetting;
        setupArgs[4] = gMovieChannelsResetD[4].channelSetting;
        slotSetupMode0(209, 9, setupArgs);
    } else {
        slotReleaseChecked(9, 0);
    }
    if (gMovieChannelEnable[19].channelSetting != 0) {
        setupArgs[0] = D_003F66C0[0].channelSetting;
        setupArgs[1] = D_003F66C0[1].channelSetting;
        setupArgs[2] = D_003F66C0[2].channelSetting;
        setupArgs[3] = D_003F66C0[3].channelSetting;
        setupArgs[4] = D_003F66C0[4].channelSetting;
        setupArgs[5] = 0;
        setupArgs[6] = D_003F66C0[5].channelSetting;
        slotSetupMode0(229, 29, setupArgs);
    } else {
        slotReleaseChecked(29, 0);
    }
    if (gMovieChannelEnable[20].channelSetting != 0) {
        setupArgs[0] = gMovieChannelsResetE[0].channelSetting;
        setupArgs[1] = gMovieChannelsResetE[1].channelSetting;
        setupArgs[2] = gMovieChannelsResetE[2].channelSetting;
        slotSetupMode0(230, 30, setupArgs);
    } else {
        slotReleaseChecked(30, 0);
    }
    if (gMovieChannelEnable[21].channelSetting != 0) {
        setupArgs[0] = D_003F7110[0].channelSetting;
        setupArgs[1] = D_003F7110[1].channelSetting;
        setupArgs[2] = D_003F7110[2].channelSetting;
        setupArgs[3] = D_003F7110[3].channelSetting;
        setupArgs[4] = D_003F7110[4].channelSetting;
        setupArgs[5] = D_003F7110[5].channelSetting;
        setupArgs[6] = D_003F7110[6].channelSetting;
        setupArgs[7] = D_003F7110[7].channelSetting;
        setupArgs[8] = D_003F7110[8].channelSetting;
        setupArgs[9] = D_003F7110[9].channelSetting;
        setupArgs[10] = D_003F7110[10].channelSetting;
        setupArgs[11] = D_003F7110[11].channelSetting;
        setupArgs[12] = D_003F7110[12].channelSetting;
        setupArgs[13] = D_003F7110[13].channelSetting;
        setupArgs[14] = D_003F7110[14].channelSetting;
        D_003F8670[0] = D_003F7110[15].channelSetting;
        D_003F8670[1] = D_003F7110[16].channelSetting;
        D_003F8670[2] = D_003F7110[17].channelSetting;
        D_003F8670[3] = D_003F7110[18].channelSetting;
        D_003F8670[4] = D_003F7110[19].channelSetting;
        D_003F8670[5] = D_003F7110[20].channelSetting;
        D_003F8670[6] = D_003F7110[21].channelSetting;
        D_003F8670[7] = D_003F7110[22].channelSetting;
        D_003F8670[8] = D_003F7110[23].channelSetting;
        D_003F8670[9] = D_003F7110[24].channelSetting;
        D_003F8670[10] = D_003F7110[25].channelSetting;
        D_003F8670[11] = D_003F7110[26].channelSetting;
        D_003F8670[12] = D_003F7110[27].channelSetting;
        D_003F8670[13] = D_003F7110[28].channelSetting;
        D_003F8670[14] = D_003F7110[29].channelSetting;
        slotSetupMode0(242, D_003F7110[30].channelSetting, setupArgs);
    } else {
        slotReleaseChecked(42, 0);
    }
    if (gMovieChannelEnable[22].channelSetting != 0) {
        setupArgs[0] = D_003F86B0[0].channelSetting;
        setupArgs[1] = D_003F86B0[1].channelSetting;
        setupArgs[2] = D_003F86B0[2].channelSetting;
        setupArgs[3] = D_003F86B0[3].channelSetting;
        setupArgs[4] = D_003F86B0[4].channelSetting;
        setupArgs[5] = D_003F86B0[5].channelSetting;
        setupArgs[6] = D_003F86B0[6].channelSetting;
        setupArgs[7] = D_003F86B0[7].channelSetting;
        setupArgs[8] = D_003F86B0[8].channelSetting;
        setupArgs[9] = D_003F86B0[9].channelSetting;
        setupArgs[10] = D_003F86B0[10].channelSetting;
        setupArgs[11] = D_003F86B0[11].channelSetting;
        setupArgs[12] = D_003F86B0[12].channelSetting;
        setupArgs[13] = D_003F86B0[13].channelSetting;
        setupArgs[14] = D_003F86B0[14].channelSetting;
        D_003F9C10[0] = D_003F86B0[15].channelSetting;
        D_003F9C10[1] = D_003F86B0[16].channelSetting;
        D_003F9C10[2] = D_003F86B0[17].channelSetting;
        D_003F9C10[3] = D_003F86B0[18].channelSetting;
        D_003F9C10[4] = D_003F86B0[19].channelSetting;
        D_003F9C10[5] = D_003F86B0[20].channelSetting;
        D_003F9C10[6] = D_003F86B0[21].channelSetting;
        D_003F9C10[7] = D_003F86B0[22].channelSetting;
        D_003F9C10[8] = D_003F86B0[23].channelSetting;
        D_003F9C10[9] = D_003F86B0[24].channelSetting;
        D_003F9C10[10] = D_003F86B0[25].channelSetting;
        D_003F9C10[11] = D_003F86B0[26].channelSetting;
        D_003F9C10[12] = D_003F86B0[27].channelSetting;
        D_003F9C10[13] = D_003F86B0[28].channelSetting;
        D_003F9C10[14] = D_003F86B0[29].channelSetting;
        slotSetupMode0(243, D_003F86B0[30].channelSetting, setupArgs);
        if (gPadDataPressed[0] & PAD_START) {
            movieNoOpStub24();
        }
    } else {
        slotReleaseChecked(43, 0);
    }
    if (gMovieChannelEnable[23].channelSetting != 0) {
        setupArgs[0] = D_003F9C50[0].channelSetting;
        setupArgs[1] = D_003F9C50[1].channelSetting;
        setupArgs[2] = D_003F9C50[2].channelSetting;
        setupArgs[3] = D_003F9C50[3].channelSetting;
        setupArgs[4] = D_003F9C50[4].channelSetting;
        setupArgs[5] = D_003F9C50[5].channelSetting;
        setupArgs[6] = D_003F9C50[6].channelSetting;
        setupArgs[7] = D_003F9C50[7].channelSetting;
        setupArgs[8] = D_003F9C50[8].channelSetting;
        setupArgs[9] = D_003F9C50[9].channelSetting;
        setupArgs[10] = D_003F9C50[10].channelSetting;
        setupArgs[11] = D_003F9C50[11].channelSetting;
        setupArgs[12] = D_003F9C50[12].channelSetting;
        setupArgs[13] = D_003F9C50[13].channelSetting;
        setupArgs[14] = D_003F9C50[14].channelSetting;
        D_003FB1B0[0] = D_003F9C50[15].channelSetting;
        D_003FB1B0[1] = D_003F9C50[16].channelSetting;
        D_003FB1B0[2] = D_003F9C50[17].channelSetting;
        D_003FB1B0[3] = D_003F9C50[18].channelSetting;
        D_003FB1B0[4] = D_003F9C50[19].channelSetting;
        D_003FB1B0[5] = D_003F9C50[20].channelSetting;
        D_003FB1B0[6] = D_003F9C50[21].channelSetting;
        D_003FB1B0[7] = D_003F9C50[22].channelSetting;
        D_003FB1B0[8] = D_003F9C50[23].channelSetting;
        D_003FB1B0[9] = D_003F9C50[24].channelSetting;
        D_003FB1B0[10] = D_003F9C50[25].channelSetting;
        D_003FB1B0[11] = D_003F9C50[26].channelSetting;
        D_003FB1B0[12] = D_003F9C50[27].channelSetting;
        D_003FB1B0[13] = D_003F9C50[28].channelSetting;
        D_003FB1B0[14] = D_003F9C50[29].channelSetting;
        slotSetupMode0(244, D_003F9C50[30].channelSetting, setupArgs);
        if (gPadDataPressed[0] & PAD_START) {
            movieNoOpStub25();
        }
    } else {
        slotReleaseChecked(44, 0);
    }
    if (gMovieChannelEnable[24].channelSetting != 0) {
        setupArgs[0] = D_003FB1F0[0].channelSetting;
        setupArgs[1] = D_003FB1F0[1].channelSetting;
        setupArgs[2] = D_003FB1F0[2].channelSetting;
        setupArgs[3] = D_003FB1F0[3].channelSetting;
        setupArgs[4] = D_003FB1F0[4].channelSetting;
        setupArgs[5] = D_003FB1F0[5].channelSetting;
        setupArgs[6] = D_003FB1F0[6].channelSetting;
        setupArgs[7] = D_003FB1F0[7].channelSetting;
        setupArgs[8] = D_003FB1F0[8].channelSetting;
        setupArgs[9] = D_003FB1F0[9].channelSetting;
        setupArgs[10] = D_003FB1F0[10].channelSetting;
        setupArgs[11] = D_003FB1F0[11].channelSetting;
        setupArgs[12] = D_003FB1F0[12].channelSetting;
        setupArgs[13] = D_003FB1F0[13].channelSetting;
        setupArgs[14] = D_003FB1F0[14].channelSetting;
        D_003FC750[0] = D_003FB1F0[15].channelSetting;
        D_003FC750[1] = D_003FB1F0[16].channelSetting;
        D_003FC750[2] = D_003FB1F0[17].channelSetting;
        D_003FC750[3] = D_003FB1F0[18].channelSetting;
        D_003FC750[4] = D_003FB1F0[19].channelSetting;
        D_003FC750[5] = D_003FB1F0[20].channelSetting;
        D_003FC750[6] = D_003FB1F0[21].channelSetting;
        D_003FC750[7] = D_003FB1F0[22].channelSetting;
        D_003FC750[8] = D_003FB1F0[23].channelSetting;
        D_003FC750[9] = D_003FB1F0[24].channelSetting;
        D_003FC750[10] = D_003FB1F0[25].channelSetting;
        D_003FC750[11] = D_003FB1F0[26].channelSetting;
        D_003FC750[12] = D_003FB1F0[27].channelSetting;
        D_003FC750[13] = D_003FB1F0[28].channelSetting;
        D_003FC750[14] = D_003FB1F0[29].channelSetting;
        slotSetupMode0(245, D_003FB1F0[30].channelSetting, setupArgs);
        if (gPadDataPressed[0] & PAD_START) {
            movieNoOpStub26();
        }
    } else {
        slotReleaseChecked(45, 0);
    }
    if (gMovieChannelEnable[25].channelSetting != 0) {
        setupArgs[0] = D_003FC790[0].channelSetting;
        setupArgs[1] = D_003FC790[1].channelSetting;
        setupArgs[2] = D_003FC790[2].channelSetting;
        setupArgs[3] = D_003FC790[3].channelSetting;
        setupArgs[4] = D_003FC790[4].channelSetting;
        setupArgs[5] = D_003FC790[5].channelSetting;
        setupArgs[6] = D_003FC790[6].channelSetting;
        setupArgs[7] = D_003FC790[7].channelSetting;
        setupArgs[8] = D_003FC790[8].channelSetting;
        setupArgs[9] = D_003FC790[9].channelSetting;
        setupArgs[10] = D_003FC790[10].channelSetting;
        setupArgs[11] = D_003FC790[11].channelSetting;
        setupArgs[12] = D_003FC790[12].channelSetting;
        setupArgs[13] = D_003FC790[13].channelSetting;
        setupArgs[14] = D_003FC790[14].channelSetting;
        D_003FDCF0[0] = D_003FC790[15].channelSetting;
        D_003FDCF0[1] = D_003FC790[16].channelSetting;
        D_003FDCF0[2] = D_003FC790[17].channelSetting;
        D_003FDCF0[3] = D_003FC790[18].channelSetting;
        D_003FDCF0[4] = D_003FC790[19].channelSetting;
        D_003FDCF0[5] = D_003FC790[20].channelSetting;
        D_003FDCF0[6] = D_003FC790[21].channelSetting;
        D_003FDCF0[7] = D_003FC790[22].channelSetting;
        D_003FDCF0[8] = D_003FC790[23].channelSetting;
        D_003FDCF0[9] = D_003FC790[24].channelSetting;
        D_003FDCF0[10] = D_003FC790[25].channelSetting;
        D_003FDCF0[11] = D_003FC790[26].channelSetting;
        D_003FDCF0[12] = D_003FC790[27].channelSetting;
        D_003FDCF0[13] = D_003FC790[28].channelSetting;
        D_003FDCF0[14] = D_003FC790[29].channelSetting;
        slotSetupMode0(246, D_003FC790[30].channelSetting, setupArgs);
        if (gPadDataPressed[0] & PAD_START) {
            movieNoOpStub27();
        }
    } else {
        slotReleaseChecked(46, 0);
    }
    if (gMovieChannelEnable[26].channelSetting != 0) {
        setupArgs[0] = D_003FDD30[0].channelSetting;
        setupArgs[1] = D_003FDD30[1].channelSetting;
        setupArgs[2] = D_003FDD30[2].channelSetting;
        setupArgs[3] = D_003FDD30[3].channelSetting;
        setupArgs[4] = D_003FDD30[4].channelSetting;
        setupArgs[5] = D_003FDD30[5].channelSetting;
        setupArgs[6] = D_003FDD30[6].channelSetting;
        setupArgs[7] = D_003FDD30[7].channelSetting;
        setupArgs[8] = D_003FDD30[8].channelSetting;
        setupArgs[9] = D_003FDD30[9].channelSetting;
        setupArgs[10] = D_003FDD30[10].channelSetting;
        setupArgs[11] = D_003FDD30[11].channelSetting;
        setupArgs[12] = D_003FDD30[12].channelSetting;
        setupArgs[13] = D_003FDD30[13].channelSetting;
        setupArgs[14] = D_003FDD30[14].channelSetting;
        D_003FF290[0] = D_003FDD30[15].channelSetting;
        D_003FF290[1] = D_003FDD30[16].channelSetting;
        D_003FF290[2] = D_003FDD30[17].channelSetting;
        D_003FF290[3] = D_003FDD30[18].channelSetting;
        D_003FF290[4] = D_003FDD30[19].channelSetting;
        D_003FF290[5] = D_003FDD30[20].channelSetting;
        D_003FF290[6] = D_003FDD30[21].channelSetting;
        D_003FF290[7] = D_003FDD30[22].channelSetting;
        D_003FF290[8] = D_003FDD30[23].channelSetting;
        D_003FF290[9] = D_003FDD30[24].channelSetting;
        D_003FF290[10] = D_003FDD30[25].channelSetting;
        D_003FF290[11] = D_003FDD30[26].channelSetting;
        D_003FF290[12] = D_003FDD30[27].channelSetting;
        D_003FF290[13] = D_003FDD30[28].channelSetting;
        D_003FF290[14] = D_003FDD30[29].channelSetting;
        slotSetupMode0(247, D_003FDD30[30].channelSetting, setupArgs);
        if (gPadDataPressed[0] & PAD_START) {
            movieNoOpStub28();
        }
    } else {
        slotReleaseChecked(47, 0);
    }
    if (gMovieChannelEnable[27].channelSetting != 0) {
        setupArgs[0] = D_003FF2D0[0].channelSetting;
        setupArgs[1] = D_003FF2D0[1].channelSetting;
        setupArgs[2] = D_003FF2D0[2].channelSetting;
        setupArgs[3] = D_003FF2D0[3].channelSetting;
        setupArgs[4] = D_003FF2D0[4].channelSetting;
        setupArgs[5] = D_003FF2D0[5].channelSetting;
        setupArgs[6] = D_003FF2D0[6].channelSetting;
        setupArgs[7] = D_003FF2D0[7].channelSetting;
        setupArgs[8] = D_003FF2D0[8].channelSetting;
        setupArgs[9] = D_003FF2D0[9].channelSetting;
        setupArgs[10] = D_003FF2D0[10].channelSetting;
        setupArgs[11] = D_003FF2D0[11].channelSetting;
        setupArgs[12] = D_003FF2D0[12].channelSetting;
        setupArgs[13] = D_003FF2D0[13].channelSetting;
        setupArgs[14] = D_003FF2D0[14].channelSetting;
        D_00400830[0] = D_003FF2D0[15].channelSetting;
        D_00400830[1] = D_003FF2D0[16].channelSetting;
        D_00400830[2] = D_003FF2D0[17].channelSetting;
        D_00400830[3] = D_003FF2D0[18].channelSetting;
        D_00400830[4] = D_003FF2D0[19].channelSetting;
        D_00400830[5] = D_003FF2D0[20].channelSetting;
        D_00400830[6] = D_003FF2D0[21].channelSetting;
        D_00400830[7] = D_003FF2D0[22].channelSetting;
        D_00400830[8] = D_003FF2D0[23].channelSetting;
        D_00400830[9] = D_003FF2D0[24].channelSetting;
        D_00400830[10] = D_003FF2D0[25].channelSetting;
        D_00400830[11] = D_003FF2D0[26].channelSetting;
        D_00400830[12] = D_003FF2D0[27].channelSetting;
        D_00400830[13] = D_003FF2D0[28].channelSetting;
        D_00400830[14] = D_003FF2D0[29].channelSetting;
        slotSetupMode0(248, D_003FF2D0[30].channelSetting, setupArgs);
        if (gPadDataPressed[0] & PAD_START) {
            movieNoOpStub29();
        }
    } else {
        slotReleaseChecked(48, 0);
    }
}

