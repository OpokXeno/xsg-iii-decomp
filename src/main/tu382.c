#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu382", func_002C8D68);

typedef int s32;
typedef unsigned char u8;
typedef float f32;

typedef struct SubHost {
    u8 unk_00[0xC0];
    f32 pointX;
    f32 pointY;
    u8 unk_C8[0x8];
    u8 subString[0];
} SubHost;

s32 njStringSetCounter(s32);
s32 njStringSetPosition(void *, f32, f32);

void subHostResetString(s32 hostAddress) {
    njStringSetCounter(hostAddress + 0xD0);
}

void setSubObjectPoint(SubHost *host, s32 stringX, s32 stringY, f32 pointX, f32 pointY) {
    host->pointX = pointX;
    host->pointY = pointY;
    njStringSetPosition(host->subString, stringX, stringY);
}
