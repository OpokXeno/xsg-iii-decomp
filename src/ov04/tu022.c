/*
 * OV04 original TU 22: 0x00aa4248..0x00aa4484 (3 functions)
 */
#include "common.h"

typedef unsigned char u8;
typedef short s16;
typedef int s32;

typedef struct CdFileReadSlot {
    s16 unk_00;
    u8 unk_02[2];
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
    s32 unk_14;
    u8 unk_18[8];
    u8 fileReadRecord[0x74];
} CdFileReadSlot;

extern CdFileReadSlot xglCdReadFileTable[];
void xglCdFileRequestClear(void *fileReadRecord);

void xglCdReadFileTableClear(void) {
    CdFileReadSlot *fileReadSlot;
    s32 slotNumber;

    fileReadSlot = xglCdReadFileTable;
    for (slotNumber = 0; slotNumber < 4; slotNumber++) {
        fileReadSlot->unk_08 = 0;
        fileReadSlot->unk_0C = 0;
        fileReadSlot->unk_10 = 0;
        fileReadSlot->unk_14 = 0;
        fileReadSlot->unk_00 = 0;
        fileReadSlot->unk_04 = 0;
        xglCdFileRequestClear(fileReadSlot->fileReadRecord);
        fileReadSlot++;
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu022", func_00AA42A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu022", func_00AA43F0);
