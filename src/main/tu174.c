#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu174", func_00220618);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef unsigned char u8;
extern s8 D_0055143C;
extern s8 D_0055143D;
extern s8 D_0055143E;
extern s16 D_00551440;
extern s16 D_00551442;
extern s32 gSheetIndex;
extern s16 gSheetDrawLimit;
extern s8 D_0055144A;
extern s8 gSheetTargetOffset;
extern s32 gSheetPanOffset;
extern s32 gSheetWidth;
extern s32 gSheetNeedsScroll;

void sheetStateReset(void) {
    gSheetIndex = 0;
    gSheetDrawLimit = 0;
    D_0055144A = 0;
    gSheetTargetOffset = 0;
    gSheetPanOffset = 0;
    gSheetWidth = 0;
    gSheetNeedsScroll = 0;
    D_00551440 = 0;
    D_00551442 = 0;
    D_0055143D = 0;
    D_0055143E = 0;
    D_0055143C = 0;
}

s32 func_00213268(s32);
s32 func_002132C8(s32);
s32 func_002164C8();

void sheetSetIndex(s32 newSheetIndex) {
    if (gSheetIndex != newSheetIndex) {
        gSheetIndex = newSheetIndex;
        if (gSheetDrawLimit >= 0x320) {
            gSheetDrawLimit = 0x320;
        }
        if (newSheetIndex != 0) {
            func_002164C8();
            gSheetTargetOffset = func_00213268(newSheetIndex) * 0x14;
            gSheetWidth = func_002132C8(newSheetIndex);
        } else {
            D_0055143C = 0;
            gSheetWidth = 0;
            gSheetTargetOffset = -0x18;
        }
        gSheetPanOffset = 0;
        if (gSheetWidth >= 0x1E1) {
            gSheetNeedsScroll = 1;
            return;
        }
        gSheetNeedsScroll = 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu174", func_002209C0);

s32 func_00220618(s32, s32, s32, s16, s32);

void sheetDrawFrame(s32 position) {
    if (gSheetNeedsScroll != 0) {
        func_00220618(0x18 - gSheetPanOffset, position - D_0055144A, gSheetIndex, gSheetDrawLimit, 1);
        func_00220618((gSheetWidth - gSheetPanOffset) + 0x38, position - D_0055144A, gSheetIndex, gSheetDrawLimit, 0);
        return;
    }
    func_00220618(0x18, position - D_0055144A, gSheetIndex, gSheetDrawLimit, 1);
}

s32 sheetGetIndex(void) {
    return gSheetIndex;
}

void sheetLatchTarget(void) {
    D_0055144A = gSheetTargetOffset;
}
