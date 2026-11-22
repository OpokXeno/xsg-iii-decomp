/*
 * OV02 original TU 59: 0x00a982e8..0x00a98ec8 (16 functions)
 */
#include "common.h"

/* One 232-byte entry of the two lookup pools of this TU; its lookup key is the halfword at +62. */
typedef struct BattlePoolLookup {
    unsigned char unmodeled_00[8];
    int word56;
    unsigned short half60;
    short key;
    unsigned char unmodeled_10[64];
    unsigned char unmodeled_50[2];
    unsigned short half130;
    unsigned char unmodeled_54[4];
    unsigned short half136;
    unsigned char unmodeled_5C[4];
    unsigned short half142;
    unsigned char unmodeled_60[88];
} BattlePoolLookup;

typedef struct BattlePoolEntry {
    unsigned char unmodeled_00[6];
    short unk_06;
    short unk_08;
    short unk_0A;
    short unk_0C;
    unsigned char unmodeled_0E[2];
    signed char unk_10;
    signed char unk_11;
    signed char unk_12;
    signed char unk_13;
    unsigned char unk_14;
    unsigned char unmodeled_15[27];
    BattlePoolLookup keyBlock;
} BattlePoolEntry;

/* Owner record; its pool entry pointer sits at +688. */
typedef struct BattlePoolOwner {
    unsigned char unmodeled_00[688];
    BattlePoolEntry *poolEntry;
    unsigned char unmodeled_AC[92];
} BattlePoolOwner;

BattlePoolEntry *battleEntrySlot(int key);
BattlePoolEntry *battleEntryFindByKey(int key);
BattlePoolEntry *ov02GetExpRecord(int key);
int ov02ExpRecordFlagIsSet(int key, int index);
BattlePoolEntry *ov02PoolEntryGet(int index);
void func_00A876C8(int key, int index);
void func_00A988E0(BattlePoolEntry *entry, int keyA, int keyB);
void calcPara2OrgSub(BattlePoolOwner *owner);
void *memset(void *dst, int value, unsigned int size);

typedef int s32;

/* Keys below BATTLE_KEY_LIMIT live in pool A (10 entries); the others in pool B (5 entries). */
#define BATTLE_KEY_LIMIT 18
#define BATTLE_POOL_A_COUNT 10
#define BATTLE_POOL_B_COUNT 5

void battleEntryNoop(void) {

}

extern BattlePoolEntry gBattleKeySlots[];

BattlePoolEntry * battleEntrySlot(int key) {
    if (!(key < BATTLE_KEY_LIMIT)) {
        return 0;
    }
    return gBattleKeySlots + (key - 1);
}

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98330);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A984B8);

s32 func_00A866C0();
s32 func_00A92318();
s32 func_00A984B8();
void battleEntryCopy(BattlePoolEntry *keySlot, BattlePoolEntry *ownerPoolEntry);

void unitSideInitEntry(BattlePoolOwner *owner, s32 key) {
    BattlePoolEntry *ownerEntry;
    BattlePoolEntry *keySlot;

    ownerEntry = owner->poolEntry;
    if (key < BATTLE_KEY_LIMIT) {
        keySlot = battleEntrySlot(key);
        battleEntryCopy(keySlot, ownerEntry);
    } else {
        func_00A984B8(ownerEntry, key);
    }
    func_00A866C0(owner);
    func_00A92318(owner);
}

extern BattlePoolEntry gBattlePoolA[];
extern BattlePoolEntry gBattlePoolB[];

/* Clears both pools. */
void battleEntryClearPools(void) {
    memset(gBattlePoolA, 0, 0x910U);
    memset(gBattlePoolB, 0, 0x488U);
}

extern BattlePoolEntry gBattlePoolA[];

BattlePoolEntry *battleEntryFindByKey(int key)
{
    int candidateIndex;

    for (candidateIndex = 0; candidateIndex < BATTLE_POOL_A_COUNT; candidateIndex++) {
        if (gBattlePoolA[candidateIndex].keyBlock.key == key) {
            return &gBattlePoolA[candidateIndex];
        }
    }
    return 0;
}

extern BattlePoolEntry gBattlePoolB[];

/* Attaches key to the first free entry of the pool it selects; keys below 18 first reuse the entry that already holds them. */
BattlePoolEntry *battleEntryAcquire(void *unused, int key)
{
    BattlePoolEntry *acquiredEntry;
    int candidateIndex;

    acquiredEntry = 0;
    if (key < BATTLE_KEY_LIMIT) {
        acquiredEntry = battleEntryFindByKey(key);
        if (acquiredEntry != 0) {
            return acquiredEntry;
        }
        for (candidateIndex = 0; candidateIndex < BATTLE_POOL_A_COUNT; candidateIndex++) {
            if (gBattlePoolA[candidateIndex].keyBlock.key == 0) {
                gBattlePoolA[candidateIndex].keyBlock.key = key;
                acquiredEntry = &gBattlePoolA[candidateIndex];
                break;
            }
        }
    } else {
        for (candidateIndex = 0; candidateIndex < BATTLE_POOL_B_COUNT; candidateIndex++) {
            if (gBattlePoolB[candidateIndex].keyBlock.key == 0) {
                gBattlePoolB[candidateIndex].keyBlock.key = key;
                acquiredEntry = &gBattlePoolB[candidateIndex];
                break;
            }
        }
    }
    return acquiredEntry;
}

/* Clears the key of the owner's pool entry; returns 1 when the owner has one, else 0. */
int battleEntryRelease(BattlePoolOwner *owner)
{
    BattlePoolEntry *ownerPoolEntry;

    ownerPoolEntry = owner->poolEntry;
    if (ownerPoolEntry != 0) {
        ownerPoolEntry->keyBlock.key = 0;
        return 1;
    }
    return 0;
}

/* Copies a pool entry and applies the per-key fix-up of keys 13..16. */
void battleEntryCopy(BattlePoolEntry *keySlot, BattlePoolEntry *ownerPoolEntry)
{
    *ownerPoolEntry = *keySlot;
    switch (ownerPoolEntry->keyBlock.key) {
    case 13:
        func_00A988E0(ownerPoolEntry, 7, 6);
        return;
    case 14:
        func_00A988E0(ownerPoolEntry, 3, 4);
        return;
    case 15:
        func_00A988E0(ownerPoolEntry, 5, 1);
        return;
    case 16:
        func_00A988E0(ownerPoolEntry, 2, 0);
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A988E0);

/* Attaches the pool entry of every key from 1 to 17 to a local owner and copies it. */
void calcPara2Org(void)
{
    BattlePoolOwner owner;
    BattlePoolEntry *foundEntry;
    int key;

    for (key = 1; key < BATTLE_KEY_LIMIT; key++) {
        foundEntry = battleEntryFindByKey(key);
        if (foundEntry != 0) {
            owner.poolEntry = foundEntry;
            calcPara2OrgSub(&owner);
        }
    }
}

/* Copies the owner's pool entry into the entry allocated for its key; a zero word56 becomes 1. */
void calcPara2OrgSub(BattlePoolOwner *owner)
{
    BattlePoolEntry *ownerPoolEntry;
    BattlePoolEntry *keySlotEntry;
    int ownerWord56;

    ownerPoolEntry = owner->poolEntry;
    keySlotEntry = battleEntrySlot(ownerPoolEntry->keyBlock.key);
    ownerWord56 = ownerPoolEntry->keyBlock.word56;
    keySlotEntry->keyBlock.word56 = ownerWord56;
    if (ownerWord56 == 0) {
        keySlotEntry->keyBlock.word56 = 1;
    }
    keySlotEntry->keyBlock.half60 = ownerPoolEntry->keyBlock.half60;
    keySlotEntry->keyBlock.half130 = ownerPoolEntry->keyBlock.half130;
    keySlotEntry->keyBlock.half142 = ownerPoolEntry->keyBlock.half142;
    keySlotEntry->keyBlock.half136 = ownerPoolEntry->keyBlock.half136;
}

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98D58);

s32 func_00A98D58();

s32 unitSideGet(s32 unitId) {
    return func_00A98D58(unitId, 0);
}

void unitSideTouch(s32 unitId) {
    func_00A98D58(unitId, 1);
}
