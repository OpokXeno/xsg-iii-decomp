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

/* The 16-byte special work at sdvSpecialBuf as func_00193850 fills it (sdvInitSpecialWork clears it). */
typedef struct SdvSpecialWork {
    short values[3];          /* +0x00 */
    unsigned char unmodeled_06[2];
    unsigned char bytes[4];   /* +0x08 */
    int active;               /* +0x0C: set to 1 */
} SdvSpecialWork;

#define sdvSpecialWork (*(SdvSpecialWork *)sdvSpecialBuf)
void func_00193850(int value0, int value1, int value2, int byte0, int byte1, int byte2, int byte3)
{
    sdvSpecialWork.values[0] = value0;
    sdvSpecialWork.values[1] = value1;
    sdvSpecialWork.values[2] = value2;
    sdvSpecialWork.bytes[0] = byte0;
    sdvSpecialWork.bytes[1] = byte1;
    sdvSpecialWork.bytes[2] = byte2;
    sdvSpecialWork.bytes[3] = byte3;
    sdvSpecialWork.active = 1;
}
