/*
 * OV02 original TU 54: 0x00a958d0..0x00a964f0 (8 functions)
 */
#include "common.h"

typedef int s32;
typedef unsigned char u8;

s32 triangularNumber(s32 lastTerm) {
    s32 i;
    s32 triangleSum;

    triangleSum = 0;
    i = 1;
    if (lastTerm > 0) {
        do {
            triangleSum += i;
            i += 1;
        } while (lastTerm >= i);
    }
    return triangleSum;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu054", func_00A95908);

INCLUDE_ASM("asm/nonmatchings/ov02/tu054", func_00A959A0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu054", func_00A95A58);

INCLUDE_ASM("asm/nonmatchings/ov02/tu054", func_00A95C68);

INCLUDE_ASM("asm/nonmatchings/ov02/tu054", func_00A95E78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu054", func_00A95F70);

s32 func_00A95C68();

/* Header of an item list: the item count at +0x00 and a second counter at +0x38. */
typedef struct ListHeader {
    s32 listLength;             /* 0x00 */
    u8 unk_04[0x34];
    s32 unk_38;                 /* 0x38 */
} ListHeader;

/* Owner whose item list hangs off +0x2B0. */
typedef struct ListOwner {
    u8 unk_00[0x2B0];
    ListHeader *list;           /* 0x2B0 */
} ListOwner;

void listOwnerPickState(ListOwner *owner, s32 *pickedState) {
    s32 listLength;
    s32 isBelowHalf;
    ListHeader *ownerList;

    ownerList = owner->list;
    listLength = ownerList->listLength;
    isBelowHalf = ownerList->unk_38 < (listLength / 2);
    if (isBelowHalf != 0) {
        *pickedState = 7;
        return;
    }
    func_00A95C68(owner, pickedState);
}
