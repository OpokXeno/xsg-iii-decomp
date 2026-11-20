#include "common.h"
#include "shared.h"

int scGetImmAdrImmIdx2(int base, unsigned short *table, int index);
short scGetImmNumIdx(short *table, int index);
int scOfsToAdr(int offset);

#define SCRIPT_ADDRESS_NONE 0xFFFF    /* absent table entry or address */

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECA60);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECE70);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECE98);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECEB0);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECEC8);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECF70);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ECFF0);

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ED068);

extern int D_00551034;
void func_002022F0(int arg);

void func_001ED2E8(void)
{
    D_00551034 += 1;
    func_002022F0(0);
}

int scGetImmAdrImmIdx2(int base, unsigned short *table, int index)
{
    int offset;
    int address;

    offset = table[index];
    if (offset == SCRIPT_ADDRESS_NONE ||offset == -1) {
        offset = 0;
    }
    address = 0;
    if (offset != 0) {
        address = base + offset * 2;
    }
    return address;
}

short scGetImmNumIdx(short *table, int index)
{
    return table[index];
}

INCLUDE_ASM("asm/main/nonmatchings/sc_get", func_001ED360);

int scOfsToAdr(int offset)
{
    if (offset == SCRIPT_ADDRESS_NONE ||offset == -1) {
        offset = 0;
    }
    return offset;
}
