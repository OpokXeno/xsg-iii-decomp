/*
 * OV02 original TU 49: 0x00a936b8..0x00a93ea0 (8 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A936B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93750);

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93770);

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93C78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93C90);

#define UNIT_FLAG_DROP_FIRST 0x20

#include "ov02/tu060.h"

typedef struct DropRecord {
    unsigned char unmodeled_00[13];
    unsigned char firstCategory; /* +0x0D: 0 when there is no first drop */
    unsigned char unmodeled_0E[2];
    unsigned char firstId; /* +0x10 */
} DropRecord;


DropRecord *func_00A84758(short key);
char *func_00A85028(int textId);
char *func_00A85208(int category, int id);
void func_00A865D0(int category, int id);
void func_00A8ABC8(char *text);
int ov01RollPercent(int percent);
char *strcpy(char *destination, const char *source);
char *func_0030E3D0(char *destination, const char *source);
extern char D_00AC3420[];
extern int D_00AC34A0;

void func_00A93CC8(BattleUnit *unit, int percent)
{
    DropRecord *record;
    char *name;
    int category;
    int id;

    record = func_00A84758(unit->poolEntry->lookupKey);
    category = record->firstCategory;
    id = record->firstId;
    if (category == 0 || (unit->flags & UNIT_FLAG_DROP_FIRST) != 0) {
        category = 0;
    }
    if (category == 0) {
        strcpy(D_00AC3420, func_00A85028(39));
    } else if (ov01RollPercent(percent) != 0) {
        unit->flags |= UNIT_FLAG_DROP_FIRST;
        func_00A865D0(category, id);
        strcpy(D_00AC3420, func_00A85028(36));
        name = func_00A85208(category, id);
        func_0030E3D0(D_00AC3420, name);
        func_0030E3D0(D_00AC3420, func_00A85028(37));
    } else {
        strcpy(D_00AC3420, func_00A85028(38));
    }
    func_00A8ABC8(D_00AC3420);
    D_00AC34A0 = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93E00);

INCLUDE_ASM("asm/nonmatchings/ov02/tu049", func_00A93E50);
