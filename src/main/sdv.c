#include "common.h"
#include "shared.h"

void sdvInitSpecialWork(void);
void *memset(void *destination, int value, unsigned int count);

extern unsigned char sdvSpecialBuf[];

typedef int s32;
extern s32 D_005511EC;

void sdvStoreValue(s32 storedValue) {
    D_005511EC = storedValue;
}

void sdvInitSpecialWork(void)
{
    memset(&sdvSpecialBuf, 0, 0x10);
}

INCLUDE_ASM("asm/main/nonmatchings/sdv", func_00193850);
