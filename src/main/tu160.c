#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_002138E8);

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213938);

typedef unsigned short u16;
typedef int s32;
typedef signed char s8;
typedef short s16;
typedef unsigned char u8;
extern u16 gStatusBitsLow;
extern u16 gStatusBitsHigh;

void clearStatusBits(s32 clearLowBits, s32 clearHighBits)
{
  s32 lowClearMask;
  s32 highKeepMask;
  s32 lowKeepMask;

  lowClearMask = clearLowBits & 0xFFFF;
  highKeepMask = 0xFFFF;
  gStatusBitsLow &= (lowKeepMask = ~lowClearMask);
  gStatusBitsHigh &= (highKeepMask = ~(clearHighBits & highKeepMask));
}

void setStatusBits(s32 setLowBits, s32 setHighBits) {
    gStatusBitsLow |= setLowBits;
    gStatusBitsHigh |= setHighBits;
}

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_002139E8);

extern unsigned char D_005A6BC0[];

/* Status header record at D_005A6BC0: 16-bit fields at 0x00, 0x02 and 0x06 (0x04 is not read here). */
typedef struct StatusHeader {
    u16 flags0;
    u16 flags2;
    u8 unk_04[2];
    u16 unk_06;
} StatusHeader;
#define gStatusHeader (*(StatusHeader *) D_005A6BC0)

s32 statusHeaderIsReadyMode20(void)
{
    s32 isModeMatch;
    isModeMatch = 0;
    if (gStatusHeader.flags2 == 0x20)
    {
        isModeMatch = gStatusHeader.unk_06 == 0;
    }
    return isModeMatch;
}

s32 statusHeaderIsReadyMode40(void)
{
    s32 isModeMatch;
    isModeMatch = 0;
    if (gStatusHeader.flags2 == 0x40)
    {
        isModeMatch = gStatusHeader.unk_06 == 0;
    }
    return isModeMatch;
}

s32 statusHeaderTestFlags(void)
{
    s32 isBit11Set;
    isBit11Set = 0;
    if ((gStatusHeader.flags0 & 0x10C) == 0x10C)
    {
        isBit11Set = 0;
        isBit11Set = (gStatusHeader.flags2 & 0x800) != isBit11Set;
    }
    return isBit11Set;
}

/* A 16-bit value that steps toward a target: fields 0 and 2 of the record at +0x54/+0x58. */
typedef struct SteppedValue {
    s16 level;
    s16 targetLevel;
} SteppedValue;

void setSteppedValue(SteppedValue *stepped, s16 currentLevel, s16 targetLevel) {
    stepped->level = currentLevel;
    stepped->targetLevel = targetLevel;
}

void setSteppedTarget(SteppedValue *stepped, s16 targetLevel) {
    stepped->targetLevel = targetLevel;
}

extern u8 gVsyncsPerFrame;

void stepValueTowardTarget(SteppedValue *stepped) {
    s32 currentLevel;
    s32 targetLevel;
    s32 stepAmount;
    s32 unsignedLevel;

    currentLevel = stepped->level;
    targetLevel = stepped->targetLevel;
    stepAmount = (gVsyncsPerFrame << 0x18) >> 0x14;
    unsignedLevel = (u16) stepped->level;
    if (currentLevel < targetLevel) {
        if ((targetLevel - currentLevel) >= stepAmount) {
            stepped->level = stepAmount + unsignedLevel;
            return;
        }
        stepped->level = stepped->targetLevel;
        return;
    }
    if (targetLevel < currentLevel) {
        if ((currentLevel - targetLevel) < stepAmount) {
            stepped->level = stepped->targetLevel;
            return;
        }
        stepped->level = unsignedLevel - stepAmount;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213B40);

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213BA8);

extern s32 gFontEscapeBuffer;
extern s32 gFontNumberBuffer;
void bufferCursorClear();
void func_00221C50();
void func_00225780();
void sceneReset();

void initScratchpadBuffers(void) {
    gFontEscapeBuffer = 0x70000000;
    gFontNumberBuffer = 0x70000100;
    bufferCursorClear();
    sceneReset();
    func_00221C50();
    func_00225780();
}

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213C18);

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213C50);

u8 *getStatusHeader(void) {
    return D_005A6BC0;
}

extern s32 gBlockStatusBits;

void blockStatusBitsSet(s32 newStatusBits) {
    gBlockStatusBits = newStatusBits;
}

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213E10);

s32 blockStatusBitsGet(void) {
    return gBlockStatusBits;
}

extern u8 D_00550B88;

u8 statusByte0Get(void) {
    return D_00550B88;
}

extern u8 D_00550B89;

u8 statusByte1Get(void) {
    return D_00550B89;
}

extern u8 D_00550B8A;

u8 statusByte2Get(void) {
    return D_00550B8A;
}

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00213FB8);

INCLUDE_ASM("asm/main/nonmatchings/tu160", func_00214008);
