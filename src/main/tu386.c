#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;

void sineWavesReset(void) {
    extern s8 gSineWavePhase;
    extern s8 gSineWaveFastValue;
    extern s8 gSineWaveMidValue;
    extern s8 gSineWaveSlowValue;
    gSineWavePhase = 0;
    gSineWaveFastValue = 0;
    gSineWaveMidValue = 0;
    gSineWaveSlowValue = 0;
}

extern f32 gSineWaveAngleStep;
f32 xglSin(f32);

void sineWavesAdvance(void) {
    extern u8 gSineWavePhase;
    extern s8 gSineWaveFastValue;
    extern s8 gSineWaveMidValue;
    extern s8 gSineWaveSlowValue;
    f32 angleStep;
    s32 advancedPhase;

    advancedPhase = gSineWavePhase + 1;
    angleStep = gSineWaveAngleStep;
    gSineWavePhase = advancedPhase;
    gSineWaveFastValue = (s32) (xglSin((advancedPhase & 0x1F) * angleStep * 0.0625f) * 16.0f) + 0x10;
    gSineWaveMidValue = (s32) (xglSin((gSineWavePhase & 0x3F) * angleStep * 0.03125f) * 32.0f) + 0x20;
    gSineWaveSlowValue = (s32) (xglSin((gSineWavePhase & 0x7F) * angleStep * 0.015625f) * 64.0f) + 0x40;
}

u8 sineWaveValue16(void) {
    extern u8 gSineWaveFastValue;
    return gSineWaveFastValue;
}

u8 sineWaveValue32(void) {
    extern u8 gSineWaveMidValue;
    return gSineWaveMidValue;
}

u8 sineWaveValue64(void) {
    extern u8 gSineWaveSlowValue;
    return gSineWaveSlowValue;
}

void sineWavesSetPhase(s8 newPhase) {
    extern s8 gSineWavePhase;
    gSineWavePhase = newPhase;
}

INCLUDE_ASM("asm/main/nonmatchings/tu386", func_002C9458);

INCLUDE_ASM("asm/main/nonmatchings/tu386", func_002C9538);
