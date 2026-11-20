/*
 * OV02 original TU 59: 0x00a982e8..0x00a98ec8 (16 functions)
 */
#include "common.h"

/* One 232-byte entry of the two lookup pools of this TU; its lookup key is the halfword at +62. */
typedef struct Ov02PoolLookup {
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
} Ov02PoolLookup;

typedef struct Ov02PoolEntry {
    unsigned char unmodeled_00[6];
    short half6;
    short half8;
    short half10;
    short half12;
    unsigned char unmodeled_0E[2];
    signed char byte16;
    signed char byte17;
    signed char byte18;
    signed char byte19;
    unsigned char byte20;
    unsigned char unmodeled_15[27];
    Ov02PoolLookup lookup;
} Ov02PoolEntry;

/* Owner record; its pool entry pointer sits at +688. */
typedef struct Ov02Owner {
    unsigned char unmodeled_00[688];
    Ov02PoolEntry *entry;
    unsigned char unmodeled_AC[92];
} Ov02Owner;

Ov02PoolEntry *func_00A982F0(int key);
Ov02PoolEntry *battleEntryFindByKey(int key);
Ov02PoolEntry *func_00A87550(int key);
int func_00A87740(int key, int index);
Ov02PoolEntry *func_00A84488(int index);
void func_00A876C8(int key, int index);
void func_00A988E0(Ov02PoolEntry *entry, int keyA, int keyB);
void calcPara2OrgSub(Ov02Owner *owner);
void *memset(void *dst, int value, unsigned int size);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A982E8);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A982F0);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98330);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A984B8);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98590);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98600);

extern Ov02PoolEntry D_00AC3C60[];

Ov02PoolEntry *battleEntryFindByKey(int key)
{
    int index;

    for (index = 0; index < 10; index++) {
        if (D_00AC3C60[index].lookup.key == key) {
            return &D_00AC3C60[index];
        }
    }
    return 0;
}

extern Ov02PoolEntry D_00AC4570[];

/* Attaches key to the first free entry of the pool it selects; keys below 18 first reuse the entry that already holds them. */
Ov02PoolEntry *battleEntryAcquire(void *unused, int key)
{
    Ov02PoolEntry *entry;
    int index;

    entry = 0;
    if (key < 18) {
        entry = battleEntryFindByKey(key);
        if (entry != 0) {
            return entry;
        }
        for (index = 0; index < 10; index++) {
            if (D_00AC3C60[index].lookup.key == 0) {
                D_00AC3C60[index].lookup.key = key;
                entry = &D_00AC3C60[index];
                break;
            }
        }
    } else {
        for (index = 0; index < 5; index++) {
            if (D_00AC4570[index].lookup.key == 0) {
                D_00AC4570[index].lookup.key = key;
                entry = &D_00AC4570[index];
                break;
            }
        }
    }
    return entry;
}

/* Clears the key of the owner's pool entry; returns 1 when the owner has one, else 0. */
int battleEntryRelease(Ov02Owner *owner)
{
    Ov02PoolEntry *entry;

    entry = owner->entry;
    if (entry != 0) {
        entry->lookup.key = 0;
        return 1;
    }
    return 0;
}

void func_00A98770(Ov02PoolEntry *src, Ov02PoolEntry *dst)
{
    *dst = *src;
    switch (dst->lookup.key) {
    case 13:
        func_00A988E0(dst, 7, 6);
        return;
    case 14:
        func_00A988E0(dst, 3, 4);
        return;
    case 15:
        func_00A988E0(dst, 5, 1);
        return;
    case 16:
        func_00A988E0(dst, 2, 0);
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A988E0);

/* Attaches the pool entry of every key from 1 to 17 to a local owner and copies it. */
void calcPara2Org(void)
{
    Ov02Owner owner;
    Ov02PoolEntry *entry;
    int key;

    for (key = 1; key < 18; key++) {
        entry = battleEntryFindByKey(key);
        if (entry != 0) {
            owner.entry = entry;
            calcPara2OrgSub(&owner);
        }
    }
}

/* Copies the owner's pool entry into the entry allocated for its key; a zero word56 becomes 1. */
void calcPara2OrgSub(Ov02Owner *owner)
{
    Ov02PoolEntry *src;
    Ov02PoolEntry *dst;
    int word56;

    src = owner->entry;
    dst = func_00A982F0(src->lookup.key);
    word56 = src->lookup.word56;
    dst->lookup.word56 = word56;
    if (word56 == 0) {
        dst->lookup.word56 = 1;
    }
    dst->lookup.half60 = src->lookup.half60;
    dst->lookup.half130 = src->lookup.half130;
    dst->lookup.half142 = src->lookup.half142;
    dst->lookup.half136 = src->lookup.half136;
}

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98D58);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98E98);

INCLUDE_ASM("asm/nonmatchings/ov02/calc", func_00A98EB0);
