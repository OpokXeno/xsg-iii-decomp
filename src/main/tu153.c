#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_0020FCB8);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_0020FD20);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_0020FD50);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_0020FE10);

#include "main/sv_get_image_item_142.h"


extern SvTypeList D_00475600[];
void srsFormatEffectStem(int effectNo, char *buffer, int flag);
unsigned int func_0030E8E4(char *text);
int func_0030EBD0(char *left, char *right, unsigned int length);
/* Finds the entry of a mapper whose name starts with the stem of effectId; *typeOut receives the record's type kind. */
SvTypeListEntry *svFindTypeListEntry(int mapperIndex, int effectId, int *typeOut);

/* Returns the first mapper from index 2 whose record holds an entry for effectNo, 0 for the skipped effect kinds and -1 when none does. */
int svFindEffectMapperIndex(int effectNo)
{
    int effectKind;
    int effectIndex;
    int mapperIndex;

    effectKind = (effectNo >> 28) & 3;
    effectIndex = (effectNo >> 12) & 0x3FF;
    if (effectKind == 0 && (unsigned int) (effectIndex - 300) < 100) {
        return 0;
    }
    if (effectKind == 1 && effectIndex == 100) {
        return 0;
    }
    for (mapperIndex = 2; mapperIndex < 44; mapperIndex++) {
        if (D_00475600[mapperIndex].used != 0 && svFindTypeListEntry(mapperIndex, effectNo, 0) != 0) {
            return mapperIndex;
        }
    }
    return -1;
}

/* Finds the entry of a mapper whose name starts with the stem of effectId; *typeOut receives the record's type kind. */
SvTypeListEntry *svFindTypeListEntry(int mapperIndex, int effectId, int *typeOut)
{
    SvTypeList *typeList;
    SvTypeListEntry *entry;
    char stem[0x100];
    int entryIndex;

    if (effectId == 0) {
        return 0;
    }
    typeList = &D_00475600[mapperIndex];
    srsFormatEffectStem(effectId, stem, 1);
    for (entryIndex = 0; entryIndex < 140; entryIndex++) {
        if (typeList->entries[entryIndex].used != 0 && typeList->entries[entryIndex].resourceKind == 5 && typeList->entries[entryIndex].name != (char *) -1) {
            if (func_0030EBD0(stem, typeList->entries[entryIndex].name, func_0030E8E4(stem)) == 0) {
                if (typeOut != 0) {
                    *typeOut = typeList->typeKind;
                }
                return &typeList->entries[entryIndex];
            }
        }
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210098);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210230);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210260);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002102C0);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002102F8);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210448);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210480);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210588);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002105A0);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002105B8);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_00210780);

INCLUDE_ASM("asm/main/nonmatchings/tu153", func_002107B8);
