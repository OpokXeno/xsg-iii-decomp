#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
extern u8 gStateExchangeEnabled;
extern s8 gStateExchangeOverride;

u8 stateExchangeOff(void) {
    s32 wasEnabled;

    wasEnabled = gStateExchangeEnabled;
    gStateExchangeOverride = 0;
    gStateExchangeEnabled = 0;
    return wasEnabled;
}

u8 stateExchangeOn(void) {
    s32 wasEnabled;

    wasEnabled = gStateExchangeEnabled;
    gStateExchangeOverride = 1;
    gStateExchangeEnabled = 1;
    return wasEnabled;
}

void stateExchangeOverrideClear(void) {
    gStateExchangeOverride = 0;
}

void stateExchangeOverrideSet(void) {
    gStateExchangeOverride = 1;
}

extern unsigned char D_004C8412[];

s32 xglRenderMovieIsActive(void) {
    return D_004C8412[0] != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu033", func_00199BE0);
