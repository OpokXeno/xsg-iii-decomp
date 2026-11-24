#include "common.h"

typedef int s32;
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
