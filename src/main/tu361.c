#include "common.h"

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;
s32 stateTailReset();

void sceneStateTailReset(void) {
    stateTailReset();
}

s32 func_002BC690();

void sceneStateFlush(void) {
    func_002BC690();
}

s32 func_002BCBB8();

void sceneStateSettle(void) {
    func_002BCBB8();
}

s32 func_002BEB80();
s32 func_002BEF50();
s32 drawSetFlagA(s32);
s32 drawSetFlagB(s32);

void sceneStateSetup(s32 sceneSlot, s32 drawFlagB, s32 drawFlagA) {
    func_002BEB80();
    drawSetFlagA(drawFlagA);
    drawSetFlagB(drawFlagB);
    func_002BEF50(sceneSlot);
}

void sceneStateClear(void) {
    func_002BEB80();
    func_002BEF50(0);
}

s32 func_002C45E8();

void sceneTableSettle(void) {
    func_002C45E8();
}

s32 seVoiceSlotsReset();

void seVoiceEntriesReset(void) {
    seVoiceSlotsReset();
}

extern s32 gVPadActor[];

void vpadActorSet(s32 actor) {
    gVPadActor[0] = actor;
}

INCLUDE_ASM("asm/main/nonmatchings/tu361", func_002AEBE8);

INCLUDE_ASM("asm/main/nonmatchings/tu361", func_002AEC38);

/* The state block at D_005C1240; only the two fields read here are laid out. */
typedef struct SceneState {
    u8 unk_0000[0xE60];
    s32 cellUpdateEnabled;          /* 0xE60 */
    u8 unk_E64[0xC];
    u8 cell[0x10];             /* 0xE70, only its address is used */
} SceneState;

extern SceneState D_005C1240;
s32 func_002CFFF0(void *, s32, s32);

void sceneCellUpdateIfEnabled(void)
{
  if (D_005C1240.cellUpdateEnabled != 0)
  {
    func_002CFFF0(D_005C1240.cell, 0x2A, 0);
  }
}
