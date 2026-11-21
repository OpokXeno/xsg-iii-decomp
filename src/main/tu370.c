#include "common.h"

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* 0x1C-byte draw entry. DrawObject embeds three of them (leftEntry, middleEntry, rightEntry). */
typedef struct DrawEntry {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 spriteIndex;
    /* 0x0C */ f32 scale;
    /* 0x10 */ f32 spriteOffset;
    /* 0x14 */ f32 alpha;
    /* 0x18 */ s8 drawFlag;
    /* 0x19 */ u8 unk_19[0x3];
} DrawEntry;

/* Per-drawObject state: position, a header block, a draw mode, three draw entries and two sprite ids. */
typedef struct DrawObject {
    /* 0x00 */ f32 posX;
    /* 0x04 */ f32 posY;
    /* 0x08 */ u8 unk_08[0x8];
    /* 0x10 */ u8 stringHeader[0x100];
    /* 0x110 */ s32 drawMode;
    /* 0x114 */ DrawEntry leftEntry;
    /* 0x130 */ DrawEntry middleEntry;
    /* 0x14C */ DrawEntry rightEntry;
    /* 0x168 */ s32 spriteIdA;
    /* 0x16C */ u8 unk_16C[0x4];
    /* 0x170 */ s32 spriteIdB;
} DrawObject;

/* One 0x190-byte record of the two draw tables (gDrawTableA, gDrawTableB). */
typedef struct DrawTableRecord {
    /* 0x00 */ s32 pairAFirst;
    /* 0x04 */ s32 pairASecond;
    /* 0x08 */ s32 pairBFirst;
    /* 0x0C */ s32 pairBSecond;
    /* 0x10 */ u8 unk_10[0x180];
} DrawTableRecord;

void drawEntryReset(DrawEntry *drawEntry) {
    drawEntry->y = 0;
    drawEntry->x = 0;
    drawEntry->spriteIndex = 0;
    drawEntry->alpha = 0;
    drawEntry->scale = 0;
    drawEntry->spriteOffset = 0;
    drawEntry->drawFlag = 0;
}

void drawEntrySetPosition(DrawEntry *drawEntry, s32 x, s32 y) {
    drawEntry->x = x;
    drawEntry->y = y;
}

void drawEntrySetValue(DrawEntry *drawEntry, s32 spriteIndex) {
    drawEntry->spriteIndex = spriteIndex;
    drawEntry->spriteOffset = spriteIndex - 2;
}

void drawEntrySetAlpha(DrawEntry *drawEntry, s32 alpha) {
    drawEntry->alpha = alpha;
}

void drawEntrySetScale(DrawEntry *drawEntry, s32 scale) {
    drawEntry->scale = scale;
}

void drawEntrySetFlag(DrawEntry *drawEntry, s8 drawFlag) {
    drawEntry->drawFlag = drawFlag;
}

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002BE480);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002BEB80);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002BEF50);

extern unsigned char gDrawFlagA[];

void drawSetFlagA(s32 drawFlag) {
    gDrawFlagA[0] = drawFlag;
}

extern unsigned char gDrawFlagB[];

void drawSetFlagB(s32 drawFlag) {
    gDrawFlagB[0] = drawFlag;
}

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002BF8F8);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002BFA70);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C0000);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C05E0);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C0670);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C07B0);

s32 func_002146C0(s32 value);
s32 func_0023C588(s32, s32, s32, s32);
s32 xglFontDrawDigits(s32, s32, s32, s32);
void drawEntrySetAlpha(DrawEntry *drawEntry, s32 alpha);
s32 func_002BE480(DrawEntry *drawEntry);
s32 sceneLinkLookupEntry(s32 index);
s32 njStringSetCounter(u8 *, s32);
s32 func_002D8358(u8 *);

void drawObjectRefresh(DrawObject *drawObject) {
    DrawEntry *leftEntry;
    DrawEntry *middleEntry;
    u8 *stringHeader;
    DrawEntry *rightEntry;
    s32 leftSpriteIndex;
    s32 middleSpriteIndex;
    s32 rightSpriteIndex;

    leftEntry = &drawObject->leftEntry;
    middleEntry = &drawObject->middleEntry;
    stringHeader = drawObject->stringHeader;
    drawEntrySetAlpha(leftEntry, 0x80);
    rightEntry = &drawObject->rightEntry;
    drawEntrySetAlpha(middleEntry, 0x80);
    drawEntrySetAlpha(rightEntry, 0x80);
    func_002BE480(leftEntry);
    func_002BE480(middleEntry);
    func_002BE480(rightEntry);
    func_002146C0(0x80322955);
    leftSpriteIndex = sceneLinkLookupEntry(2);
    func_0023C588((((s32) drawObject->posX * 0x10) + 0x78A8) & 0xFFF8, (((s32) drawObject->posY * 0x10) + 0x7228) & 0xFFF8, leftSpriteIndex, 0xD);
    func_002146C0(0x80545934);
    middleSpriteIndex = sceneLinkLookupEntry(0);
    func_0023C588((((s32) drawObject->posX * 0x10) + 0x7C68) & 0xFFF8, (((s32) drawObject->posY * 0x10) + 0x7228) & 0xFFF8, middleSpriteIndex, 0xD);
    func_002146C0(0x80562E2D);
    rightSpriteIndex = sceneLinkLookupEntry(1);
    func_0023C588((((s32) drawObject->posX * 0x10) + 0x80D8) & 0xFFF8, (((s32) drawObject->posY * 0x10) + 0x7228) & 0xFFF8, rightSpriteIndex, 0xD);
    func_002146C0(0x80808080);
    xglFontDrawDigits((((s32) drawObject->posX * 0x10) + 0x8058) & 0xFFF8, (((s32) drawObject->posY * 0x10) + 0x7208) & 0xFFF8, drawObject->spriteIdA, 0xF);
    xglFontDrawDigits((((s32) drawObject->posX * 0x10) + 0x8418) & 0xFFF8, (((s32) drawObject->posY * 0x10) + 0x7208) & 0xFFF8, drawObject->spriteIdB, 0xF);
    njStringSetCounter(stringHeader, drawObject->drawMode);
    func_002D8358(stringHeader);
}

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C0B68);

extern DrawTableRecord gDrawTableA[];

void drawTableASetPairB(s32 drawTableIndex, s32 pairFirst, s32 pairSecond) {
    DrawTableRecord *drawTableRecord;

    drawTableRecord = &gDrawTableA[drawTableIndex];
    drawTableRecord->pairBFirst = pairFirst;
    drawTableRecord->pairBSecond = pairSecond;
}

extern DrawTableRecord gDrawTableB[];

void drawTableBSetPairA(s32 drawTableIndex, s32 pairFirst, s32 pairSecond) {
    DrawTableRecord *drawTableRecord;

    drawTableRecord = &gDrawTableB[drawTableIndex];
    drawTableRecord->pairAFirst = pairFirst;
    drawTableRecord->pairASecond = pairSecond;
}

void drawTableBSetPairB(s32 drawTableIndex, s32 pairFirst, s32 pairSecond) {
    DrawTableRecord *drawTableRecord;

    drawTableRecord = &gDrawTableB[drawTableIndex];
    drawTableRecord->pairBFirst = pairFirst;
    drawTableRecord->pairBSecond = pairSecond;
}

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C0C68);

INCLUDE_ASM("asm/main/nonmatchings/tu370", func_002C0CD8);
