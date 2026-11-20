#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9078);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9100);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E91E8);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9300);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9428);

#include "main/tu118.h"

extern SefEntryTable D_004541A0;

int sefFlagTest(int mask);
void sefExecStep(void);
void func_00203450(void);

void func_001E94B0(void)
{
    if (D_003E46B0.flags & 0x400) {
        func_002034F0();
        return;
    }
    if (sefFlagTest(1) == 0) {
        sefExecStep();
    }
    func_00203450();
}

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9500);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9660);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9770);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9790);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9800);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9828);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9858);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9888);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9F48);

int func_001E9FC8(void *actor)
{
    SefEntryTable *table;
    SefEntry *entry;
    int i;

    if (actor != 0) {
        table = &D_004541A0;
        i = 0;
        if (table->entryCount > 0) {
            do {
                entry = &table->entries[i];
                if (entry->id == (int) actor) {
                    return entry->flags;
                }
                i++;
            } while (i < table->entryCount);
        }
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001EA028);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001EA050);
