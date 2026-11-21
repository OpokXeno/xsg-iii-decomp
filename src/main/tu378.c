#include "common.h"

typedef int s32;
typedef long long s64;
typedef signed char s8;
extern s64 gMovieAutoSelectFlags;

/* Counter with a [start, end) range. Accessors below read and write these fields; the 0x00 word is cleared
 * by the reset but never read in this TU. */
typedef struct DecodeCounter {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 decodedCount;
    /* 0x08 */ s32 decodedLastIndex;
    /* 0x0C */ s32 decodeStartIndex;
    /* 0x10 */ s32 decodeEndIndex;
    /* 0x14 */ s32 decodeLastIndex;
    /* 0x18 */ s32 decodedCountSaved;
    /* 0x1C */ s32 flag_a;
    /* 0x20 */ s32 flag_b;
    /* 0x24 */ s8 flag_c;
} DecodeCounter;

s32 counterGlobalMaskIsClear(void) {
    return (~gMovieAutoSelectFlags & 0x90C) == 0;
}

void decodeCounterClear(DecodeCounter *decodeCounter) {
    decodeCounter->decodedLastIndex = 0;
    decodeCounter->decodedCount = 0;
    decodeCounter->decodeLastIndex = 0;
    decodeCounter->decodeEndIndex = 0;
    decodeCounter->decodeStartIndex = 0;
    decodeCounter->flag_a = 0;
    decodeCounter->flag_b = 0;
    decodeCounter->decodedCountSaved = 0;
    decodeCounter->unk_00 = 0;
}

void decodeCounterSetFlagC(DecodeCounter *decodeCounter, s8 decodeFlagValue) {
    decodeCounter->flag_c = decodeFlagValue;
}

s32 decodeCounterGetCount(DecodeCounter *decodeCounter) {
    return decodeCounter->decodedCount;
}

s32 decodeCounterGetEnd(DecodeCounter *decodeCounter) {
    return decodeCounter->decodeEndIndex;
}

s32 decodeCounterGetStart(DecodeCounter *decodeCounter) {
    return decodeCounter->decodeStartIndex;
}

void decodeCounterSetFlagA(DecodeCounter *decodeCounter, s8 decodeFlagValue) {
    decodeCounter->flag_a = decodeFlagValue;
}

void decodeCounterSetFlagB(DecodeCounter *decodeCounter, s8 decodeFlagValue) {
    decodeCounter->flag_b = decodeFlagValue;
}

void decodeCounterSetCount(DecodeCounter *decodeCounter, s32 newDecodedCount) {
    decodeCounter->decodedCount = newDecodedCount;
}

void decodeCounterSetDecodedLastIndex(DecodeCounter *decodeCounter, s32 newDecodedCount) {
    decodeCounter->decodedLastIndex = newDecodedCount - 1;
}

void decodeCounterSet(DecodeCounter *decodeCounter, s32 newDecodeStart, s32 newDecodeEnd, s32 newDecodeCount) {
    s32 storedDecodeStart;
    s32 storedDecodeEnd;
    s32 storedDecodeLastIndex;

    storedDecodeStart = newDecodeStart;
    storedDecodeEnd = newDecodeEnd;
    storedDecodeLastIndex = newDecodeCount - 1;
    decodeCounter->decodeLastIndex = storedDecodeLastIndex;
    decodeCounter->decodeEndIndex = storedDecodeEnd;
    decodeCounter->decodeStartIndex = storedDecodeStart;
}

void decodeCounterSetStart(DecodeCounter *decodeCounter, s32 newDecodeStart) {
    decodeCounter->decodeStartIndex = newDecodeStart;
}

void decodeCounterSetEnd(DecodeCounter *decodeCounter, s32 newDecodeEnd) {
    decodeCounter->decodeEndIndex = newDecodeEnd;
}

void decodeCounterSetLastIndex(DecodeCounter *decodeCounter, s32 newDecodeCount) {
    decodeCounter->decodeLastIndex = newDecodeCount - 1;
}

s32 func_002C74D8(void *);

s32 counterRangeSnapshot(DecodeCounter *decodeCounter) {
    s32 decodedCount;

    func_002C74D8(decodeCounter);
    decodedCount = decodeCounter->decodedCount;
    decodeCounter->decodedCountSaved = decodedCount;
    return decodedCount;
}

INCLUDE_ASM("asm/main/nonmatchings/tu378", func_002C74D8);

INCLUDE_ASM("asm/main/nonmatchings/tu378", func_002C79D0);
