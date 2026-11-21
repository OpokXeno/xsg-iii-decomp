#include "common.h"

typedef signed char s8;
typedef int s32;
s8 *func_0030E4FC(s8 *, s32);
void *memcpy(void *, const void *, unsigned int);

void textCopyTagValue(s8 *tagValueOut, s8 **cursor) {
    s8 *tagBody;
    s32 valueLength;
    s8 *tagValueStart;
    s8 *tagValueEnd;

    tagValueEnd = tagValueOut;
    tagBody = *cursor;
    tagBody += 2;
    tagValueStart = tagValueEnd;
    valueLength = func_0030E4FC(tagBody, '>') - tagBody;
    tagValueEnd += valueLength;
    memcpy(tagValueStart, tagBody, valueLength);
    *tagValueEnd = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu275", func_00269930);

INCLUDE_ASM("asm/main/nonmatchings/tu275", func_002699B8);
