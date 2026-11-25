#include "common.h"

typedef unsigned char u8;

typedef int s32;

typedef float f32;

extern u8 gBattleModeActors[];

extern void *gTextActorEntries[];

s32 textSubmitRandomEntry(void *table, s32 first, s32 second);

void textSubmitSelectedActors(void)
{
    textSubmitRandomEntry(gTextActorEntries, gBattleModeActors[1], gBattleModeActors[2]);
}

/* gBattleChatterControlStorage is gBattleChatterControl; byte 0 is chatterFlags (bit 0x1 set here). */
extern u8 gBattleChatterControlStorage[];

extern void *gTextModeEntries[];

void stateSetModeAndMark(s32 textMode) {
    textSubmitRandomEntry(gTextModeEntries, textMode, (textMode >= 3) ? 0 : 3);
    gBattleChatterControlStorage[0] |= 1;
}

extern void *gTextValueEntries[];

void textSubmitValueEntry(s32 textValue) {
    textSubmitRandomEntry(gTextValueEntries, textValue, textValue);
}

extern void *D_003E2C08[];

s32 textSubmitRandomEntry();

void textPlayDefaultTable(void) {
    textSubmitRandomEntry(D_003E2C08, 3, 3);
}

extern void *D_003E2C80[];

void textPlayTableSizeA(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2C80, actorSlot, actorSlot);
}

extern void *D_003E2D68[];

void textPlayTableSizeB(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2D68, actorSlot, actorSlot);
}

extern void *D_003E2EC8[];

void textPlayTableSizeC(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2EC8, actorSlot, actorSlot);
}

extern void *D_003E3028[];

void textPlayTableSizeD(s32 actorSlot) {
    textSubmitRandomEntry(D_003E3028, actorSlot, actorSlot);
}

extern void *D_003E30A8[];

void textPlayTableSizeE(s32 actorSlot) {
    textSubmitRandomEntry(D_003E30A8, actorSlot, actorSlot);
}

extern void *D_003E30F0[];

void textPlayTableSizeF(s32 actorSlot) {
    textSubmitRandomEntry(D_003E30F0, actorSlot, actorSlot);
}

void battleSetPhase(s32 phase);

void textResetMode(void) {
    battleSetPhase(0);
}

extern s32 D_003E2330[];

void *gTextActorEntries[2] = { D_003E2330, 0 };
f32 D_003E2998[2][13] = {
    { 2.0f, 1536.0f, 3.0f, 0.0f, 0.0f, -2.79999995f, 0.0f, 4.0f, 0.0f, 7.0f, 0.0f, -30.0f, 5.0f },
    { 4.0f, 60.0f, 3.0f, 0.899999976f, 0.0f, -4.0f, 0.0f, 4.0f, 1.0f, 20.0f, 11.0f, -55.0f, 0.0f }
};
void *D_003E2A00[2] = { D_003E2998, "r50" };
f32 D_003E2A08[2][13] = {
    { 2.0f, 9728.0f, 3.0f, 0.400000006f, 0.0f, -3.0f, 0.0f, 4.0f, 0.349999994f, 30.0f, 7.0f, 70.0f, 5.0f },
    { 1.0f, 90.0f, 3.0f, 0.899999976f, 0.0f, -3.0f, 0.0f, 4.0f, 1.0f, -50.0f, 8.0f, 65.0f, 0.0f }
};
void *D_003E2A70[2] = { D_003E2A08, "r51" };
f32 D_003E2A78[2][15] = {
    { 2.0f, 9728.0f, 3.0f, 1.0f, 0.0f, -2.0f, 0.0f, 4.0f, 1.0f, 30.0f, 2.0f, -30.0f, 6.0f, 0.0f, 5.0f },
    { 4.0f, 60.0f, 3.0f, 1.0f, 0.0f, -1.0f, 0.0f, 4.0f, 1.0f, -15.0f, 1.5f, -57.0f, 6.0f, 0.122172952f, 0.0f }
};
void *D_003E2AF0[2] = { D_003E2A78, "r52" };
f32 D_003E2AF8[2][13] = {
    { 2.0f, 3584.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f, 8.0f, -4.0f, 25.0f, 5.0f },
    { 1.0f, 80.0f, 3.0f, 1.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f, -9.5f, -4.0f, 25.0f, 0.0f }
};
void *D_003E2B60[2] = { D_003E2AF8, "r53" };
void *gTextModeEntries[6] = { D_003E2A00, D_003E2A70, D_003E2AF0, D_003E2B60, 0, 0 };
f32 D_003E2B80[2][7] = {
    { 2.0f, 17408.0f, 3.0f, 0.0f, 0.0f, -4.0f, -5.0f },
    { 4.0f, 0.0f, 20.0f, 0.0f, 30.0f, 0.0f, 0.0f }
};
void *D_003E2BB8[2] = { D_003E2B80, "r45" };
void *gTextValueEntries[2] = { D_003E2BB8, 0 };
f32 D_003E2BC8[2][7] = {
    { 2.0f, 17408.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 4.0f, 0.0f, 0.0f, 12.0f, -50.0f, 0.0f, 0.0f }
};
void *D_003E2C00[2] = { D_003E2BC8, "r46" };
void *D_003E2C08[2] = { D_003E2C00, 0 };
f32 D_003E2C10[2][13] = {
    { 2.0f, 0.0f, 3.0f, 0.0f, 0.0f, -1.79999995f, 0.0f, 4.0f, 0.0f, 10.0f, -9.0f, -20.0f, 5.0f },
    { 3.0f, 18.0f, 3.0f, 0.0f, 0.0f, -1.0f, 0.0f, 4.0f, 0.0f, 5.0f, -1.79999995f, -10.0f, 0.0f }
};
void *D_003E2C78[2] = { D_003E2C10, "r40" };
void *D_003E2C80[2] = { D_003E2C78, 0 };
f32 D_003E2C88[2][13] = {
    { 2.0f, 512.0f, 3.0f, 0.0f, 0.0f, -2.5f, 0.0f, 4.0f, 0.0f, 1.5f, 2.0f, -15.0f, 5.0f },
    { 4.0f, 35.0f, 3.0f, 0.0f, 0.0f, -2.5f, 0.0f, 4.0f, 0.0f, 5.0f, -6.0f, -25.0f, 0.0f }
};
void *D_003E2CF0[2] = { D_003E2C88, "r47" };
f32 D_003E2CF8[2][13] = {
    { 2.0f, 512.0f, 3.0f, 0.0f, 0.0f, -3.0f, 0.0f, 4.0f, 0.0f, 3.0f, 5.5f, -17.0f, 5.0f },
    { 3.0f, 50.0f, 3.0f, 0.0f, 0.0f, -3.0f, 0.0f, 4.0f, 0.0f, -6.0f, -3.0f, -25.0f, 0.0f }
};
void *D_003E2D60[2] = { D_003E2CF8, "r48" };
void *D_003E2D68[4] = { D_003E2CF0, D_003E2D60, 0, 0 };
f32 D_003E2D78[2][13] = {
    { 2.0f, 75264.0f, 3.0f, 0.0f, 0.0f, -4.0f, 0.0f, 4.0f, 0.0f, 25.0f, 30.0f, -50.0f, 5.0f },
    { 4.0f, 110.0f, 3.0f, 0.0f, 0.0f, -4.0f, 0.0f, 4.0f, 0.0f, -30.0f, -7.0f, -50.0f, 0.0f }
};
void *D_003E2DE0[2] = { D_003E2D78, "r49" };
f32 D_003E2DE8[2][13] = {
    { 2.0f, 75264.0f, 3.0f, 1.0f, 0.0f, -3.0f, 0.0f, 4.0f, 1.0f, 50.0f, 15.0f, -40.0f, 5.0f },
    { 4.0f, 100.0f, 3.0f, 1.0f, 0.0f, -3.0f, 0.0f, 4.0f, 1.0f, -50.0f, 15.0f, -40.0f, 0.0f }
};
void *D_003E2E50[2] = { D_003E2DE8, "r492" };
f32 D_003E2E58[2][13] = {
    { 2.0f, 75264.0f, 3.0f, 0.0f, 0.0f, -3.0f, 0.0f, 4.0f, 0.0f, 30.0f, -3.0f, -30.0f, 5.0f },
    { 4.0f, 100.0f, 3.0f, 0.0f, 0.0f, -3.0f, 0.0f, 4.0f, 0.0f, -20.0f, 20.0f, -65.0f, 0.0f }
};
void *D_003E2EC0[2] = { D_003E2E58, "r493" };
void *D_003E2EC8[4] = { D_003E2DE0, D_003E2E50, D_003E2EC0, 0 };
f32 D_003E2ED8[2][13] = {
    { 2.0f, 1536.0f, 3.0f, 1.0f, 0.0f, -2.5f, 0.0f, 4.0f, 1.0f, 40.0f, -6.5f, -24.0f, 5.0f },
    { 4.0f, 120.0f, 3.0f, 1.0f, 0.0f, -2.0f, 0.0f, 4.0f, 1.0f, -25.0f, -9.0f, -40.0f, 0.0f }
};
void *D_003E2F40[2] = { D_003E2ED8, "r90" };
f32 D_003E2F48[2][13] = {
    { 2.0f, 1536.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 37.0f, 6.0f, -33.0f, 5.0f },
    { 4.0f, 180.0f, 3.0f, 0.0f, 0.0f, -3.0f, 0.0f, 4.0f, 0.0f, -35.0f, 45.0f, -70.0f, 0.0f }
};
void *D_003E2FB0[2] = { D_003E2F48, "r91" };
f32 D_003E2FB8[2][13] = {
    { 2.0f, 512.0f, 3.0f, 0.0f, 0.0f, -5.0f, 0.0f, 4.0f, 0.0f, 6.0f, 2.0f, -40.0f, 5.0f },
    { 4.0f, 50.0f, 3.0f, 0.0f, 0.0f, -2.79999995f, -0.5f, 4.0f, 1.0f, 6.5f, -6.5f, -12.0f, 0.0f }
};
void *D_003E3020[2] = { D_003E2FB8, "r92" };
void *D_003E3028[4] = { D_003E2F40, D_003E2FB0, D_003E3020, 0 };
f32 D_003E3038[2][13] = {
    { 2.0f, 9728.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 30.0f, 30.0f, -30.0f, 5.0f },
    { 4.0f, 255.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, -100.0f, 160.0f, -45.0f, 0.0f }
};
void *D_003E30A0[2] = { D_003E3038, "r95" };
void *D_003E30A8[2] = { D_003E30A0, 0 };
f32 D_003E30B0[2][7] = {
    { 2.0f, 9216.0f, 3.0f, 0.0f, 0.0f, -1.0f, 0.0f },
    { 4.0f, 0.0f, 6.0f, -3.0f, 18.0f, 0.0f, 0.0f }
};
void *D_003E30E8[2] = { D_003E30B0, "r100" };
void *D_003E30F0[2] = { D_003E30E8, 0 };
