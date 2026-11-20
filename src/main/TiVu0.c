#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/TiVu0", func_002A15B0);

#include "shared.h"

void assertFailed(const char *message, const char *file, int line);
extern const char D_005512B8[];
extern const char D_00542B90[];

/* Unconditional assert(0) at TiVu0.euc.c line 802: the expression text is "0" and the file name is "TiVu0.euc.c". */
void func_002A15F0(void)
{
    assertFailed(D_005512B8, D_00542B90, 802);
}

/* Unconditional assert(0) at TiVu0.euc.c line 821: the expression text is "0" and the file name is "TiVu0.euc.c". */
void func_002A1618(void)
{
    assertFailed(D_005512B8, D_00542B90, 821);
}
