#include "common.h"
#include "shared.h"

void sdvInitSpecialWork(void);
void *memset(void *destination, int value, unsigned int count);

extern unsigned char sdvSpecialBuf[];

INCLUDE_ASM("asm/main/nonmatchings/sdv", func_00193820);

void sdvInitSpecialWork(void)
{
    memset(&sdvSpecialBuf, 0, 0x10);
}

INCLUDE_ASM("asm/main/nonmatchings/sdv", func_00193850);
