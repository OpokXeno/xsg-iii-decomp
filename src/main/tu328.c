#include "common.h"

typedef int s32;

/* Parts entry referenced from the owner at 0x50C; kind is compared against 0x1E by xrgObjectIsPartsKind1E. */
typedef struct XrgPartsEntry {
    unsigned char unk_00[0x10];
    s32 partsKindId;
} XrgPartsEntry;

/* Owner whose parts entry pointer sits at 0x50C. */
typedef struct XrgPartsOwner {
    unsigned char unk_000[0x50C];
    XrgPartsEntry *partsDef;
} XrgPartsOwner;

void XrgPartsSetFlags(s32, s32);

void xrgObjectSetPartsFlags(s32 self, s32 partsFlagMask) {
    XrgPartsSetFlags(partsFlagMask, self + 0x560);
}

void XrgPartsClearFlags(s32, s32);

void xrgObjectClearPartsFlags(s32 self, s32 partsFlagMask) {
    XrgPartsClearFlags(partsFlagMask, self + 0x560);
}

void XrgPartsResetFlags(s32, s32);

void xrgObjectResetPartsFlags(s32 self, s32 partsFlagMask) {
    XrgPartsResetFlags(partsFlagMask, self + 0x560);
}

void XrgPartsSetTime(s32, s32);

void xrgObjectSetPartsTime(s32 self, s32 partsTime) {
    XrgPartsSetTime(partsTime, self + 0x560);
}

void XrgPartsSetWeight(s32, s32);

void xrgObjectSetPartsWeight(s32 self, s32 partsWeight) {
    XrgPartsSetWeight(partsWeight, self + 0x560);
}

s32 xrgObjectIsPartsKind1E(XrgPartsOwner *self) {
    s32 matchesPartsKind;
    XrgPartsEntry *partsDef;

    matchesPartsKind = 0;
    if (self != 0) {
        partsDef = self->partsDef;
        if (partsDef != 0) {
            matchesPartsKind = partsDef->partsKindId == 0x1E;
        }
    }
    return matchesPartsKind;
}

INCLUDE_ASM("asm/main/nonmatchings/tu328", func_00292068);
