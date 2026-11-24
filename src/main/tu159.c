#include "common.h"
#include "shared.h"

typedef signed char s8;

extern u8 *gFontEscapeBuffer;

typedef int s32;

s32 unsignedDecimalDigits(u32 number);

/* Copies text into gFontEscapeBuffer (at most 255 characters, then a 0). A byte with the high bit set is a lead byte and
 * is copied with the byte after it. Each placeholder byte becomes the next digit of number, most significant first; once
 * the digits are used up it becomes the 7 control byte. */
u8 *fontFormatPlaceholderNumber(s8 *text, s32 number, s8 placeholder) {
    u8 *outputCursor = gFontEscapeBuffer;
    s32 outputLength = 0;
    s32 digitsLeft = unsignedDecimalDigits(number);

    while (*text != 0 && outputLength < 255) {
        if (*text & 0x80) {
            *outputCursor++ = *text++;
            outputLength++;
            *outputCursor = *text;
        } else if (*text == placeholder) {
            if (digitsLeft != 0) {
                s32 scaledNumber = number;
                s32 placeIndex;

                for (placeIndex = 1; placeIndex < digitsLeft; placeIndex++) {
                    scaledNumber /= 10;
                }
                digitsLeft--;
                *outputCursor = scaledNumber % 10 + '0';
            } else {
                *outputCursor = 7;
            }
        } else {
            *outputCursor = *text;
        }
        text++;
        outputCursor++;
        outputLength++;
    }
    *outputCursor = 0;
    return gFontEscapeBuffer;
}

extern void *gXcFirstSection;
extern void *gXcSecondSection;

void xcClearPayloadSections(void) {
    gXcFirstSection = 0;
    gXcSecondSection = 0;
}

extern unsigned char D_003C75C0[];

typedef struct XcPayloadSections {
    /* 0x00 */ s32 firstSectionOffset;
    /* 0x04 */ s32 hasFirstSection;
    /* 0x08 */ s32 secondSectionOffset;
    /* 0x0C */ s32 hasSecondSection;
} XcPayloadSections;

s32 xcLocatePayload(XcPayloadSections **payload, void *source);

s32 xcLoadPayloadSections(void) {
    XcPayloadSections *sections;
    s32 secondSectionOffset;
    void *firstSection;
    void *secondSection;

    xcLocatePayload(&sections, D_003C75C0);
    secondSection = 0;
    firstSection = 0;
    if (sections->hasFirstSection != 0) {
        firstSection = (u8 *) sections + sections->firstSectionOffset;
    }
    secondSectionOffset = sections->hasSecondSection;
    gXcFirstSection = firstSection;
    if (secondSectionOffset != 0) {
        secondSectionOffset = sections->secondSectionOffset;
        secondSection = (u8 *) sections + secondSectionOffset;
    }
    gXcSecondSection = secondSection;
    return secondSectionOffset;
}
