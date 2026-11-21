#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu172", func_0021FAB8);

typedef unsigned char u8;
typedef unsigned short u16;

/* Sheet record: the two halfwords this file copies are at 0x02 and 0x36 (source) and 0x1A and 0x1C (destination). */
typedef struct TiSheet {
    u8 unk_00[0x2];
    u16 unk_02;                         /* 0x02 */
    u8 unk_04[0x16];
    u16 unk_1A;                         /* 0x1A */
    u16 frameBase;                         /* 0x1C */
    u8 unk_1E[0x36 - 0x1E];
    u16 unk_36;                         /* 0x36 */
} TiSheet;

void tiSheetCopyFields(TiSheet *destSheet, TiSheet *sourceSheet) {
    destSheet->unk_1A = sourceSheet->unk_02;
    destSheet->frameBase = sourceSheet->unk_36;
}

void tiSheetCopyTrailingField(TiSheet *destSheet, TiSheet *sourceSheet) {
    destSheet->frameBase = sourceSheet->unk_36;
}

INCLUDE_ASM("asm/main/nonmatchings/tu172", func_0021FCE0);
