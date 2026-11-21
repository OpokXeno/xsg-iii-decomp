#include "common.h"

typedef unsigned char u8;
typedef int s32;
extern u8 gBattleModeActors[];
extern s32 gTextActorEntries;
s32 textSubmitRandomEntry(void *table, s32 first, s32 second);

void textSubmitSelectedActors(void)
{
    textSubmitRandomEntry(&gTextActorEntries, gBattleModeActors[1], gBattleModeActors[2]);
}

/* gBattleChatterControlStorage is gBattleChatterControl; byte 0 is chatterFlags (bit 0x1 set here). */
extern u8 gBattleChatterControlStorage;
extern s32 gTextModeEntries;

void stateSetModeAndMark(s32 textMode) {
    textSubmitRandomEntry(&gTextModeEntries, textMode, (textMode >= 3) ? 0 : 3);
    gBattleChatterControlStorage |= 1;
}

extern s32 gTextValueEntries;

void textSubmitValueEntry(s32 textValue) {
    textSubmitRandomEntry(&gTextValueEntries, textValue, textValue);
}
