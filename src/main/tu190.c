#include "common.h"

#include "gs.h"
typedef signed char s8;

typedef unsigned char u8;

typedef unsigned int u32;

typedef int s32;

const s8 glyphIndexTable[24] = {
    -1, 0, 6, 3, 4, 1, 5, 2,
    8, 9, 7, -1, -1, 10, 11, 12,
    13, -1, 0, 0, 0, 0, 0, 0,
};

s8 glyphIndexGet(u32 glyphCode) {
    if (!(glyphCode < 0x16U)) {
        return -1;
    }
    return glyphIndexTable[glyphCode];

}

const u8 glyphPairOffsetTable[16] = {
    40, 44, 56, 60, 72, 76, 88, 92,
    84, 80, 0, 0, 0, 0, 0, 0,
};

s32 glyphPairOffsetGet(s32 glyphIndex) {
    return glyphPairOffsetTable[glyphIndex >> 1] + 0x320;
}

const u8 glyphAttrTable[24] = {
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 0, 0, 0, 0,
};

u8 glyphAttrGet(s32 glyphIndex) {
    return glyphAttrTable[glyphIndex];
}

const u8 D_00538E40[32] = {
    10, 10, 10, 10, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

u8 glyphAltAttrGet(s32 glyphIndex) {
    return D_00538E40[glyphIndex];
}

extern u16 gFontClutBase[];

typedef struct GifRecord {
    u64 packedValue;
    u64 gsRegister;
} GifRecord;

#define GIF_PUT(cursor, registerValue, registerAddress) \
    (cursor)->packedValue = (registerValue);            \
    (cursor)->gsRegister = (registerAddress);           \
    (cursor)++

extern GifRecord *xglDirectQueueBuffer;
extern s32 xglDirectQueueCount;
void xglDirectQueueSubmitRef(void);

typedef struct QueueCounter {
    s32 queuedRecords;
} QueueCounter;

#define QUEUE_COUNT (((QueueCounter *) &xglDirectQueueCount)->queuedRecords)
#define GS_TEX2_1 0x16

/* Adds the records written since queueWriteStart to the direct queue count, then submits the queue. */
static inline void queueCommit(GifRecord *gifCursor, GifRecord *queueWriteStart) {
    QUEUE_COUNT += ((u32) gifCursor - (u32) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
}

int xglFontGetStateMask(void);
void nmlPacketGsInit(void);
int xglFontGetColorIndex(void);
void fontQueueColorState(s32 mask);
void fontQueueSprite(u16 startX, u16 startY, u16 endX, u16 endY);
void fontQueueLine(u16 startX, u16 startY, u16 endX, u16 endY);
/* Draws one font cell at origin: a textured glyph sprite when glyphIndex >= 0, otherwise a plain
 * colour-state sprite; then the outline lines of the cell. */
void fontDrawCell(u16 *origin, s32 glyphIndex, s32 rowBase, s32 height);

void fontDrawCell(u16 *origin, s32 glyphIndex, s32 rowBase, s32 height) {
    s32 savedMask = xglFontGetStateMask();
    s32 atlasU;
    s32 atlasV;
    GifRecord *gifCursor;
    GifRecord *queueWriteStart;

    if (glyphIndex >= 0) {
        atlasU = (glyphIndex & 7) * 32 + 256;
        atlasV = (glyphIndex >> 3) * 64 + rowBase;
        nmlPacketGsInit();
        queueWriteStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
        GIF_PUT(gifCursor,
                GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, gFontClutBase[0] * 32 + glyphPairOffsetGet(glyphIndex), 0, 0,
                            0, 1),
                GS_TEX2_1);
        GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
        GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, atlasU, atlasU + 31, atlasV, atlasV + height - 1), GS_CLAMP_1);
        atlasU <<= 4;
        atlasV <<= 4;
        height <<= 4;
        GIF_PUT(gifCursor, GS_SET_UV(atlasU, atlasV), GS_UV);
        GIF_PUT(gifCursor, GS_SET_XYZ(origin[0], origin[1], xglFontGetColorIndex()), GS_XYZ2);
        atlasU += 512;
        atlasV += height;
        GIF_PUT(gifCursor, GS_SET_UV(atlasU, atlasV), GS_UV);
        GIF_PUT(gifCursor, GS_SET_XYZ(origin[0] + 512, origin[1] + height, xglFontGetColorIndex()),
                GS_XYZ2);
        queueCommit(gifCursor, queueWriteStart);
    } else {
        height <<= 4;
        fontQueueColorState(0x80000000);
        fontQueueSprite(origin[0], origin[1], origin[0] + 512, origin[1] + height);
    }
    fontQueueColorState(0x80000000);
    fontQueueLine(origin[0], origin[1], origin[0] + 512, origin[1]);
    fontQueueLine(origin[0], origin[1] + 16, origin[0] + 512, origin[1] + 16);
    fontQueueLine(origin[0], origin[1], origin[0], origin[1] + height);
    fontQueueLine(origin[0] + 496, origin[1], origin[0] + 496, origin[1] + height);
    fontQueueLine(origin[0], origin[1] + height - 32, origin[0] + 496, origin[1] + height - 32);
    fontQueueLine(origin[0], origin[1] + height - 16, origin[0] + 496, origin[1] + height - 16);
    fontQueueColorState(savedMask);
}

void glyphDrawByCode(s32 drawTarget, s32 glyphCode) {
    s32 glyphIndex;
    glyphIndex = glyphIndexGet(glyphCode);
    fontDrawCell(drawTarget, glyphIndex, 0, 0x40);
}

void glyphDrawWithAttr(s32 drawTarget, s32 glyphCode) {
    s32 glyphIndex;
    s32 glyphAttr;

    glyphIndex = glyphIndexGet(glyphCode);
    glyphAttr = glyphAttrGet(glyphIndex);
    fontDrawCell(drawTarget, glyphIndex, glyphAttr, 0x30);
}

void glyphDrawWithAltAttr(s32 drawTarget, s32 glyphCode) {
    s32 glyphIndex;
    s32 attr;

    glyphIndex = glyphIndexGet(glyphCode);
    attr = glyphAltAttrGet(glyphIndex);
    fontDrawCell(drawTarget, glyphIndex, attr, 0x2B);
}

void glyphDrawPairSlot(s32 drawTarget, s32 pairNumber) {
    s32 zeroBasedPair;

    zeroBasedPair = pairNumber - 1;
    if (pairNumber != 0) {
        fontDrawCell(drawTarget, (zeroBasedPair >> 1) + 0xE, (~pairNumber & 1) << 5, 0x20);
    }
}
