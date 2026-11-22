#include "common.h"
#include "shared.h"
#include "main/sdv.h"

void sdvInitSpecialWork(void);
void *memset(void *destination, int value, unsigned int count);

typedef int s32;
extern s32 D_005511EC;

void sdvStoreValue(s32 storedValue) {
    D_005511EC = storedValue;
}

void sdvInitSpecialWork(void)
{
    memset(&sdvSpecialBuf, 0, sizeof(sdvSpecialBuf));
}

/* Sets the camera pad override: the buttons and the four stick axes the camera pad updates read
 * instead of the pads; active is stored last. */
void func_00193850(int held, int pressed, int repeated, int rightX, int rightY, int leftX, int leftY)
{
    sdvSpecialBuf.held = held;
    sdvSpecialBuf.pressed = pressed;
    sdvSpecialBuf.repeated = repeated;
    sdvSpecialBuf.rightX = rightX;
    sdvSpecialBuf.rightY = rightY;
    sdvSpecialBuf.leftX = leftX;
    sdvSpecialBuf.leftY = leftY;
    sdvSpecialBuf.active = 1;
}
