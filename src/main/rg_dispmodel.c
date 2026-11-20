#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BF848);

void *memset(void *destination, int value, unsigned int size);
/* Scaffold data of rg_dispmodel, named by its splat label (no config/symbols name exists). */
extern int D_00551400;
extern unsigned char D_0055E5C0[142800];

/* Clears the active count and the whole resource slot table. */
void sefResetResourceTable(void)
{
    D_00551400 = 0;
    memset(D_0055E5C0, 0, sizeof(D_0055E5C0));
}

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BF8F0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BF970);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BFBB8);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BFC50);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BFDA0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001BFDE8);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0040);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C02C0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C02E0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C03B8);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", RgDispModelSetLocal);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C04D0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0570);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0628);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0688);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C06F0);

INCLUDE_ASM("asm/main/nonmatchings/rg_dispmodel", func_001C0720);
