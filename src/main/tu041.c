#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E510);

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E5B0);

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E7D0);

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E7E0);

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E7F0);

extern unsigned int D_008C2C90[];

void func_0019E808(unsigned int value)
{
    D_008C2C90[0] = value;
}

extern struct {
    unsigned char unmodeled_000[0x300];
    unsigned int flags; /* +0x300 */
} D_008C28B0;

void func_0019E818(void)
{
    D_008C28B0.flags |= 0x400;
}

void func_0019E830(const void *src)
{
    __builtin_memcpy(&D_008C28B0, src, 0x180);
}

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E8E8);

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E8F8);
