#include "common.h"
#include "main/battle_shared.h"

BattlePoolEntry gBattleKeySlots[BATTLE_UNIT_COUNT] = { { { 0 } } };

Ov02ExpRecord gOv02ExpRecords[BATTLE_UNIT_COUNT] = { { { 0 } } };

int partyRecordTable[PARTY_RECORD_WORDS] = { 0 };

int enemyRecordTable[ENEMY_RECORD_WORDS] = { 0 };

BattleStatusBlock D_004CC320 = { { 0 } };

Ov02BattleCounterBlock gOv02BattleCounters = { { 0 } };

unsigned char ov02DataSectionTable[BATTLE_DATA_BYTES] = { 0 };
