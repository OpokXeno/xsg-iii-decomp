#include "common.h"

typedef unsigned char u8;

typedef int s32;

extern u8 gBattleModeActors[];

extern s32 gTextActorEntries[];

s32 textSubmitRandomEntry(void *table, s32 first, s32 second);

void textSubmitSelectedActors(void)
{
    textSubmitRandomEntry(gTextActorEntries, gBattleModeActors[1], gBattleModeActors[2]);
}

/* gBattleChatterControlStorage is gBattleChatterControl; byte 0 is chatterFlags (bit 0x1 set here). */
extern u8 gBattleChatterControlStorage[];

extern s32 gTextModeEntries[];

void stateSetModeAndMark(s32 textMode) {
    textSubmitRandomEntry(gTextModeEntries, textMode, (textMode >= 3) ? 0 : 3);
    gBattleChatterControlStorage[0] |= 1;
}

extern s32 gTextValueEntries[];

void textSubmitValueEntry(s32 textValue) {
    textSubmitRandomEntry(gTextValueEntries, textValue, textValue);
}

extern s32 D_003E2C08[];

s32 textSubmitRandomEntry();

void textPlayDefaultTable(void) {
    textSubmitRandomEntry(D_003E2C08, 3, 3);
}

extern s32 D_003E2C80[];

void textPlayTableSizeA(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2C80, actorSlot, actorSlot);
}

extern s32 D_003E2D68[];

void textPlayTableSizeB(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2D68, actorSlot, actorSlot);
}

extern s32 D_003E2EC8[];

void textPlayTableSizeC(s32 actorSlot) {
    textSubmitRandomEntry(D_003E2EC8, actorSlot, actorSlot);
}

extern s32 D_003E3028[];

void textPlayTableSizeD(s32 actorSlot) {
    textSubmitRandomEntry(D_003E3028, actorSlot, actorSlot);
}

extern s32 D_003E30A8[];

void textPlayTableSizeE(s32 actorSlot) {
    textSubmitRandomEntry(D_003E30A8, actorSlot, actorSlot);
}

extern s32 D_003E30F0[];

void textPlayTableSizeF(s32 actorSlot) {
    textSubmitRandomEntry(D_003E30F0, actorSlot, actorSlot);
}

void battleSetPhase(s32 phase);

void textResetMode(void) {
    battleSetPhase(0);
}
