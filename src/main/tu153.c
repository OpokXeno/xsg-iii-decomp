#include "common.h"

#include "main/sv_get_image_item_142.h"
typedef int s32;

/* gBattleResetTableStorage: 0x50-byte block cleared and seeded by eBattleResetState; unk_00 and unk_28 hold the seeded words. */
typedef struct BattleResetTable {
    s32 unk_00;
    unsigned char unk_04[0x24];
    s32 unk_28;
    unsigned char unk_2C[0x24];
} BattleResetTable;

extern unsigned char gBattleResetTableStorage[];
#define gBattleResetTable (*(BattleResetTable *) gBattleResetTableStorage)
extern s32 D_00551550;
void eBattleWinReset(void);
void srsSetEffectFlag(s32 lookup, s32 value);
void srsSetEffectIndexFlag(s32 lookup, s32 value);
void *memset(void *destination, s32 value, unsigned int size);

void eBattleResetState(void) {
    memset(gBattleResetTableStorage, 0, 0x50);
    gBattleResetTable.unk_00 = 0xA1C000;
    gBattleResetTable.unk_28 = 0xA4E000;
    D_00551550 = 0;
    eBattleWinReset();
    srsSetEffectFlag(0, 1);
    srsSetEffectIndexFlag(0, 1);
}

s32 func_001B60B8(s32);
s32 xglCdControlThread();

void xglCdWaitControl(void) {
    while (func_001B60B8(3) != 0) {
        xglCdControlThread();
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_0020FD50);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_0020FE10);

#include "main/sv_get_image_item_142.h"


extern SvTypeList svResourceTypeLists[];
void srsFormatEffectStem(int effectNo, char *buffer, int flag);
unsigned int func_0030E8E4(char *text);
int func_0030EBD0(const char *left, const char *right, unsigned int length);
/* Finds the entry of a mapper whose name starts with the stem of effectId; *effectTypeKind receives the record's type kind. */
SvTypeListEntry *svFindEffectEntryByStem(int mapperIndex, int effectId, int *effectTypeKind);

#define SV_MAPPER_COUNT 44        /* mapper slots in svResourceTypeLists */
#define SV_TYPE_ENTRY_COUNT 140   /* entries of one SvTypeList */

/* Returns the first mapper from index 2 whose record holds an entry for effectNo, 0 for the skipped effect kinds and -1 when none does. */
int svFindEffectMapperIndex(int effectNo)
{
    int effectKind;
    unsigned int effectIndex;
    int mapperIndex;

    effectKind = (effectNo >> 28) & 3;
    effectIndex = (effectNo >> 12) & 0x3FF;
    if (effectKind == 0 && (effectIndex - 300) < 100) {
        return 0;
    }
    if (effectKind == 1 && effectIndex == 100) {
        return 0;
    }
    for (mapperIndex = 2; mapperIndex < SV_MAPPER_COUNT; mapperIndex++) {
        if (svResourceTypeLists[mapperIndex].used != 0 && svFindEffectEntryByStem(mapperIndex, effectNo, 0) != 0) {
            return mapperIndex;
        }
    }
    return -1;
}

/* Finds the entry of a mapper whose name starts with the stem of effectId; *effectTypeKind receives the record's type kind. */
SvTypeListEntry *svFindEffectEntryByStem(int mapperIndex, int effectId, int *effectTypeKind)
{
    SvTypeList *mapperTypeList;
    SvTypeListEntry *matchedEntry;
    char stem[0x100];
    int mapperEntryIndex;

    if (effectId == 0) {
        return 0;
    }
    mapperTypeList = &svResourceTypeLists[mapperIndex];
    srsFormatEffectStem(effectId, stem, 1);
    for (mapperEntryIndex = 0; mapperEntryIndex < SV_TYPE_ENTRY_COUNT; mapperEntryIndex++) {
        if (mapperTypeList->entries[mapperEntryIndex].used != 0 && mapperTypeList->entries[mapperEntryIndex].resourceKind == 5 && mapperTypeList->entries[mapperEntryIndex].name != (char *) -1) {
            if (func_0030EBD0(stem, mapperTypeList->entries[mapperEntryIndex].name, func_0030E8E4(stem)) == 0) {
                if (effectTypeKind != 0) {
                    *effectTypeKind = mapperTypeList->typeKind;
                }
                return &mapperTypeList->entries[mapperEntryIndex];
            }
        }
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210098);

s32 srsEffectNameGet();
s32 func_00210098(s32, s32);

/* Sets the value of the effect named by the current effect key (srsEffectNameGet). */
void srsSetNamedEffectValue(s32 unused_arg0, s32 effectValue) {
    s32 effectName;
    effectName = srsEffectNameGet();
    func_00210098(effectName, effectValue);
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210260);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002102C0);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002102F8);

s32 func_0020F790();
s32 func_002102F8(s32, s32);

void srsSetEffectIndexFlag(s32 useCurrentEffect, s32 effectFlagValue) {
    s32 effectIndex;

    effectIndex = -1;
    if (useCurrentEffect != 0) {
        effectIndex = func_0020F790();
    }
    func_002102F8(effectIndex, effectFlagValue);
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210480);

s32 func_00210480(s32, s32);

s32 srsSetEffectHandleValueResult(s32 effectHandleIndex, s32 unused_arg1, s32 effectHandleValue) {
    return func_00210480(effectHandleIndex, effectHandleValue);
}

void srsSetEffectHandleValue(s32 effectHandleIndex, s32 unused_arg1, s32 effectHandleValue) {
    func_00210480(effectHandleIndex, effectHandleValue);
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002105B8);

s32 func_002105B8(s32, s32);

void srsSetEffectFlag(s32 useCurrentEffect, s32 effectFlagValue) {
    s32 effectIndex;

    effectIndex = -1;
    if (useCurrentEffect != 0) {
        effectIndex = func_0020F790();
    }
    func_002105B8(effectIndex, effectFlagValue);
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002107B8);
