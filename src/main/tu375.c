#include "common.h"

typedef signed char s8;
typedef int s32;
typedef unsigned char u8;
typedef float f32;

typedef struct SceneLinkState {
    /* 0x00 */ s32 cursor;
    /* 0x04 */ s32 count;
    /* 0x08 */ u8 unk_08[0x10];
    /* 0x18 */ s32 counterLo;
    /* 0x1C */ s32 counterHi;
    /* 0x20 */ s32 enabled;
    /* 0x24 */ s8 mode;
    /* 0x25 */ s8 phase;
} SceneLinkState;

extern unsigned char gSceneLinkStateStorage[];
#define gSceneLinkState (*(SceneLinkState *) &gSceneLinkStateStorage)

void sceneLinkStateClear(void) {
    gSceneLinkState.cursor = 0;
    gSceneLinkState.count = 0;
    gSceneLinkState.counterLo = 0;
    gSceneLinkState.counterHi = 0;
    gSceneLinkState.enabled = 0;
    gSceneLinkState.phase = 0;
    gSceneLinkState.mode = 0;
}

void sceneLinkStateSetEnabled(void)
{
    gSceneLinkState.counterLo = 0;
    gSceneLinkState.counterHi = 0;
    gSceneLinkState.enabled = 1;
}

void sceneLinkStateClearCounters(void)
{
    gSceneLinkState.counterLo = 0;
    gSceneLinkState.counterHi = 0;
    gSceneLinkState.enabled = 0;
}

extern s8 gSceneLinkPhase;

void sceneLinkSetFlag45(s8 value) {
    gSceneLinkPhase = value;
}

extern s8 gSceneLinkMode;

void sceneLinkSetFlag44(s8 value) {
    gSceneLinkMode = value;
}

INCLUDE_ASM("asm/main/nonmatchings/tu375", func_002C6B50);

INCLUDE_ASM("asm/main/nonmatchings/tu375", func_002C6C00);

extern unsigned char sceneLinkTable[];
s32 func_001B5F28();
void xcBlockClear(void *block);
s32 func_001B70F0();

void sceneLinkSubmitPacket(void) {
    s32 packet[8];

    xcBlockClear(&packet[0]);
    packet[1] = 0x09000005;
    packet[2] = (s32) sceneLinkTable;
    func_001B70F0(&packet[0]);
    func_001B5F28(0, 0, 0);
}

s32 tableGetWord();
s32 func_002C7C50();

s32 sceneLinkLookupEntry(s32 key) {
    s32 subtableOffset;
    subtableOffset = tableGetWord(sceneLinkTable, 1);
    return func_002C7C50(&sceneLinkTable[subtableOffset], key);
}

void vecSetXYZ(f32 *vector, f32 x, f32 y, f32 z) {
    vector[0] = x;
    vector[1] = y;
    vector[2] = z;
}

INCLUDE_ASM("asm/main/nonmatchings/tu375", func_002C6EA0);
