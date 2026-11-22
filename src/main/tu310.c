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

INCLUDE_ASM("asm/main/nonmatchings/tu310", func_00289168);
