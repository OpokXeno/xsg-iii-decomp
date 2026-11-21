#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
void movieClearPending(void);

/* Parameters of the movie record at D_003E3D30. */
typedef struct MovieParams {
    f32 originX;
    f32 originY;
    s32 displayMode;
} MovieParams;

extern MovieParams D_003E3D30[];

void setMovieParams(s32 displayMode, f32 originX, f32 originY) {
    extern void movieClearPending();
    f32 storedOriginX;
    f32 storedOriginY;
    s32 storedDisplayMode;

    storedDisplayMode = displayMode;
    storedOriginY = originY;
    storedOriginX = originX;
    movieClearPending();
    D_003E3D30[0].originX = storedOriginX;
    D_003E3D30[0].originY = storedOriginY;
    D_003E3D30[0].displayMode = storedDisplayMode;
}

INCLUDE_ASM("asm/main/nonmatchings/tu294", func_002738E8);

INCLUDE_ASM("asm/main/nonmatchings/tu294", func_00273920);

extern s32 gMoviePending;
extern f32 gMovieFrameAlpha;

void movieClearPending(void) {
    gMoviePending = 0;
}

void movieFrameUpdate(void);
extern unsigned char D_003E3D78[];
extern void (*gFrameUpdateHooks[])(void);
s32 textTagInit();
void movieResetCurrentIndex();
void movieClearWorkBuffer();
void movieRegisterNatives();
void func_0026C620();
void movieCounterSlotsClear();
void movieClearCurrent();
void rgSingletonClearFlag();
s32 memset();

void movieInit(void) {
    extern s8 gMovieActive;
    func_0026C620();
    movieClearWorkBuffer();
    movieResetCurrentIndex();
    textTagInit();
    rgSingletonClearFlag();
    movieCounterSlotsClear();
    movieClearCurrent();
    movieRegisterNatives();
    memset(D_003E3D78, 0, 0xA0);
    gMovieFrameAlpha = 0.0f;
    gMovieActive = 0;
    gFrameUpdateHooks[0] = movieFrameUpdate;
}

void movieInit(void);

void movieBegin(void) {
    extern s8 gMovieActive;
    extern void movieInit();
    movieInit();
    gMovieActive = 1;
}

extern s32 gGameLoopRuntimeStorage[];
void followSlotsUpdate();
void textStepCounterAdvance();
void movieUpdateAll();

void movieStepIfActive(void) {
    movieUpdateAll();
    if (!(gGameLoopRuntimeStorage[0] & 0x4000)) {
        followSlotsUpdate();
        textStepCounterAdvance();
    }
}

void func_00269BA8();
void movieSwitchUpdate();
void func_0026C6B8();
void func_00273C68();
void func_00273D08();

void movieFrameUpdate(void) {
    extern u8 gMovieActive;
    movieSwitchUpdate();
    func_00273C68();
    func_00273D08();
    func_0026C6B8();
    if (gMovieActive == 0) {
        func_00269BA8();
    }
}

s32 xglPacketScratchFlush();
s32 xglPacketScratchGetBatch();
s32 movieFrameSetFade(s32, s32, f32);
s32 gsMakeFrame();
s32 xglPacketScratchAddQuad();
s32 xglPacketScratchBegin();

void drawPendingMovieFrame(void) {
    extern f32 gMovieFrameAlpha;
    s32 frame_x;
    s32 frame_y;
    s32 frame_packet;

    if (gMovieFrameAlpha != 0.0f) {
        xglPacketScratchBegin();
        xglPacketScratchGetBatch(&frame_x, &frame_y);
        frame_packet = gsMakeFrame(0);
        xglPacketScratchAddQuad(0x4C, frame_packet);
        movieFrameSetFade(frame_x, frame_y, gMovieFrameAlpha);
        xglPacketScratchFlush();
        gMovieFrameAlpha = 0.0f;
    }
}

void movieLayerShutdown();

void movieShutdown(void) {
    gFrameUpdateHooks[0] = 0;
    movieLayerShutdown();
    movieCounterSlotsClear();
}

INCLUDE_ASM("asm/main/nonmatchings/tu294", func_00273B78);

void movieSetFrameAlpha(f32 alpha) {
    extern f32 gMovieFrameAlpha;
    gMovieFrameAlpha = alpha;
}

extern unsigned char gMoviePathBuffer[];
s32 GameMpeg2Play();
s32 moviePlaySetup();

s32 playMpeg2(s32 movieFormat, s32 movieFormatArg) {
    GameMpeg2Play(gMoviePathBuffer, movieFormat, movieFormatArg);
    return moviePlaySetup(gMoviePathBuffer, 0, 0, 0);
}

void func_0026CE88();

void movieSetFlagByte(s32 movieSlotId, s32 movieSlotByteOffset, s32 movieFlagValue) {
    func_0026CE88(movieSlotId, movieSlotByteOffset, movieFlagValue & 0xFF);
}

void moviePlayLocalized();

void movieDecoderReset(void) {
    moviePlayLocalized();
}

INCLUDE_ASM("asm/main/nonmatchings/tu294", func_00273C68);

INCLUDE_ASM("asm/main/nonmatchings/tu294", func_00273D08);
