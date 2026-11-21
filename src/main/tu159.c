#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu159", func_00213740);

typedef int s32;
typedef unsigned char u8;
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
