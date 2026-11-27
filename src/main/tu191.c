#include "common.h"
#include "gs.h"
#include "main/tu162.h"
#define GS_TEX2_1 0x16
#define GIF_PUT(cursor, registerValue, registerAddress) \
    (cursor)->packedValue = (registerValue);            \
    (cursor)->gsRegister = (registerAddress);           \
    (cursor)++

typedef struct GifRecord {
    u64 packedValue;
    u64 gsRegister;
} GifRecord;

extern u16 gFontClutBase[];

typedef struct GifBufferPointer {
    GifRecord *gifRecords;
} GifBufferPointer;

extern GifRecord *xglDirectQueueBuffer;

typedef int s32;

extern s32 xglDirectQueueCount;
void nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
s32 xglFontGetColorIndex(void);

#define QUEUE_BUFFER (((GifBufferPointer *) &xglDirectQueueBuffer)->gifRecords)

/* Draws one 16 x 16 cell of the font atlas at (x, y); cells are three per row, 16 texels apart. */
void fontDrawAtlasCell(u16 x, u16 y, s32 cellIndex) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    s32 atlasU;
    s32 atlasV;

    atlasU = cellIndex % 3 * 16 + 180;
    atlasV = cellIndex / 3 * 16 + 87;
    nmlPacketGsInit();
    gifStart = gifCursor = &QUEUE_BUFFER[xglDirectQueueCount];
    GIF_PUT(gifCursor, GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 0x338, 0, 0, 0, 1),
            GS_TEX2_1);
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, atlasU, atlasU + 15, atlasV, atlasV + 15), GS_CLAMP_1);
    atlasU <<= 4;
    atlasV <<= 4;
    GIF_PUT(gifCursor, GS_SET_UV(atlasU, atlasV), GS_UV);
    GIF_PUT(gifCursor, GS_SET_XYZ(x, y, xglFontGetColorIndex()), GS_XYZ2);
    atlasU += 256;
    atlasV += 256;
    GIF_PUT(gifCursor, GS_SET_UV(atlasU, atlasV), GS_UV);
    GIF_PUT(gifCursor, GS_SET_XYZ(x + 256, y + 256, xglFontGetColorIndex()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

#define GS_XYZ(x, y, z) GS_SET_XYZ(x, y, (u32) (z))

typedef struct ScreenPoint {
    u16 x;
    u16 y;
} ScreenPoint;

/* Textured quad over the rectangle pos .. pos + (offsetX, offsetY), atlas cell at u 143..159, v 32..48;
 * cornerIndex rotates the corner order. */
void fontDrawRotatedSpriteU143(ScreenPoint *pos, s32 cornerIndex, s32 offsetX, s32 offsetY) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    ScreenPoint corners[2];
    u8 cornerColumn[] = {0, 1, 1, 0};
    u8 cornerRow[] = {0, 0, 1, 1};

    corners[0].x = pos->x;
    corners[0].y = pos->y;
    corners[1].x = pos->x + offsetX;
    corners[1].y = pos->y + offsetY;
    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 780, 0, 0, 0, 1),
            GS_TEX2_1);
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, 143, 159, 32, 48), GS_CLAMP_1);
    GIF_PUT(gifCursor, GS_SET_UV(0x8F0, 0x200), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xA00, 0x200), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xA00, 0x310), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0x8F0, 0x310), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Textured quad like fontDrawRotatedSpriteU143 with the atlas cell at u 160..172, v 32..48. */
void fontDrawRotatedSpriteU160(ScreenPoint *pos, s32 cornerIndex, s32 offsetX, s32 offsetY) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    ScreenPoint corners[2];
    u8 cornerColumn[] = {0, 1, 1, 0};
    u8 cornerRow[] = {0, 0, 1, 1};

    corners[0].x = pos->x;
    corners[0].y = pos->y;
    corners[1].x = pos->x + offsetX;
    corners[1].y = pos->y + offsetY;
    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 780, 0, 0, 0, 1),
            GS_TEX2_1);
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, 160, 172, 32, 48), GS_CLAMP_1);
    GIF_PUT(gifCursor, GS_SET_UV(0xA00, 0x200), GS_UV);
    cornerIndex = cornerIndex & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xAD0, 0x200), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xAD0, 0x310), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xA00, 0x310), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Textured quad like fontDrawRotatedSpriteU143 with the atlas cell at u 232..255, v 119..137. */
void fontDrawRotatedSpriteU232(ScreenPoint *pos, s32 cornerIndex, s32 offsetX, s32 offsetY) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    ScreenPoint corners[2];
    u8 cornerColumn[] = {0, 1, 1, 0};
    u8 cornerRow[] = {0, 0, 1, 1};

    corners[0].x = pos->x;
    corners[0].y = pos->y;
    corners[1].x = pos->x + offsetX;
    corners[1].y = pos->y + offsetY;
    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 816, 0, 0, 0, 1),
            GS_TEX2_1);
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, 232, 255, 119, 137), GS_CLAMP_1);
    GIF_PUT(gifCursor, GS_SET_UV(0xE80, 0x770), GS_UV);
    cornerIndex = (cornerIndex + 3) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0x1000, 0x770), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0x1000, 0x8A0), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xE80, 0x8A0), GS_UV);
    cornerIndex = (cornerIndex + 1) & 3;
    GIF_PUT(gifCursor,
            GS_XYZ(corners[cornerColumn[cornerIndex]].x, corners[cornerRow[cornerIndex]].y, xglFontGetColorIndex()),
            GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

typedef struct QueueCounter {
    s32 queuedRecords;
} QueueCounter;

#define QUEUE_COUNT (((QueueCounter *) &xglDirectQueueCount)->queuedRecords)

static inline void queueCommit(GifRecord *gifCursor, GifRecord *gifStart) {
    QUEUE_COUNT += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Draws the rectangle rect as a textured font cell: cells are six per row, 30 texels wide (16 for cell 16). */
void fontDrawRectCell(FontRect *rect, s32 cell) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    s32 atlasU;
    s32 atlasV;
    s32 width;
    s32 height;

    atlasU = cell % 6 * 30;
    atlasV = cell / 6 * 32 + 54;
    width = cell == 16 ? 16 : 30;
    height = (cell >= 12 && cell <= 16) ? 21 : 32;
    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[QUEUE_COUNT];
    GIF_PUT(gifCursor, GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 0x31C, 0, 0, 0, 1),
                   GS_TEX2_1);
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, atlasU, atlasU + width - 1, atlasV, atlasV + height - 1), GS_CLAMP_1);
    atlasU <<= 4;
    atlasV <<= 4;
    height <<= 4;
    width <<= 4;
    GIF_PUT(gifCursor, GS_SET_UV(atlasU, atlasV), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->top, xglFontGetColorIndex()), GS_XYZ2);
    atlasU += width;
    atlasV += height;
    GIF_PUT(gifCursor, GS_SET_UV(atlasU, atlasV), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->bottom, xglFontGetColorIndex()), GS_XYZ2);
    queueCommit(gifCursor, gifStart);
}

/* Builds the rectangle of font cell `cell` at (x, y) and draws it with fontDrawRectCell. */
void fontDrawRectOfCell(u16 x, u16 y, int cell) {
    FontRect rect;

    rect.left = x;
    rect.top = y;
    if (cell == 16) {
        rect.right = x + 256;
    } else {
        rect.right = x + 480;
    }
    if (cell - 12 < 5u) {
        rect.bottom = y + 336;
    } else {
        rect.bottom = y + 512;
    }
    fontDrawRectCell(&rect, cell);
}

extern u16 D_004C8398[];

/* Draws a 16 x 16 pixel textured sprite at (x, y); coordinates are 12.4 fixed point (256 = 16 pixels). */
void fontDrawSprite16(u16 x, u16 y) {
    GifRecord *gifCursor;
    GifRecord *gifStart;

    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(
        gifCursor,
        GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (D_004C8398[5] << 5) + 816, GS_PSMCT32, 0, 0, 1),
        GS_TEX2_1);
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, 212, 227, 135, 150), GS_CLAMP_1);
    GIF_PUT(gifCursor, GS_SET_UV(0xD40, 0x870), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(x, y, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xE40, 0x970), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(x + 256, y + 256, xglFontGetColorIndex()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Draws a 17 x 17 pixel textured sprite at (x, y) from another atlas cell (272 = 17 pixels). */
void fontDrawSprite17(u16 x, u16 y) {
    GifRecord *gifCursor;
    GifRecord *gifStart;

    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(
        gifCursor,
        GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (D_004C8398[5] << 5) + 908, GS_PSMCT32, 0, 0, 1),
        GS_TEX2_1);
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, 727, 743, 154, 170), GS_CLAMP_1);
    GIF_PUT(gifCursor, GS_SET_UV(0x2D70, 0x9A0), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(x, y, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0x2E80, 0xAB0), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(x + 272, y + 272, xglFontGetColorIndex()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Draws a frame of textured panels around pos; the panel UVs are three atlas corners. */
void fontDrawPanelFrame(ScreenPoint *pos, u16 offsetX, u16 offsetY) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    FontRect rect;
    u64 uvTopRight = GS_SET_UV(0xC40, 0x770);
    u64 uvBottomRight = GS_SET_UV(0xC40, 0x870);
    u64 uvTopLeft = GS_SET_UV(0xB40, 0x770);

    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(
        gifCursor,
        GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (D_004C8398[5] << 5) + 816, GS_PSMCT32, 0, 0, 1),
        GS_TEX2_1);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, 180, 195, 119, 134), GS_CLAMP_1);
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    rect.left = pos->x - 112;
    rect.top = pos->y - 144;
    rect.right = rect.left + 256;
    rect.bottom = rect.top + 256;
    GIF_PUT(gifCursor, uvTopLeft, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvTopRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvBottomRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xB40, 0x870), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_UV(0xC30, 0x770), GS_UV);
    rect.left = rect.right;
    rect.right = offsetX + pos->x - 144;
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvBottomRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    rect.left = rect.right;
    rect.right = rect.left + 256;
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, uvTopLeft, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvTopRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvBottomRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xB40, 0x870), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    rect.top = rect.bottom;
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_UV(0xB40, 0x860), GS_UV);
    rect.bottom = offsetY + pos->y - 112;
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xC40, 0x860), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    rect.top = rect.bottom;
    rect.bottom = rect.bottom + 256;
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, uvTopLeft, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvTopRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvBottomRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xB40, 0x870), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_UV(0xC30, 0x770), GS_UV);
    rect.right = rect.left;
    rect.left = pos->x + 144;
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvBottomRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    rect.right = rect.left;
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, uvTopLeft, GS_UV);
    rect.left = pos->x - 112;
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvTopRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, uvBottomRight, GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xB40, 0x870), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    rect.bottom = rect.top;
    GIF_PUT(gifCursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_UV(0xB40, 0x860), GS_UV);
    rect.top = pos->y + 112;
    GIF_PUT(gifCursor, GS_XYZ(rect.left, rect.top, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(0xC40, 0x860), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(rect.right, rect.bottom, xglFontGetColorIndex()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

typedef float f32;

extern f32 xglSin(f32);
extern f32 xglCos(f32);

void drawRotatedSquare(u16 *origin, u16 drawSize, int tileColumn, f32 angle);

/* Draws a textured square of drawSize units at origin, rotated by angle; tileColumn picks the atlas column
 * and tileColumn >= 3 mirrors the corner order. */
void drawRotatedSquare(u16 *origin, u16 drawSize, int tileColumn, f32 angle) {
    ScreenPoint corner[4];
    GifRecord *gifCursor;
    GifRecord *gifStart;
    int i;
    int tileU;
    f32 x;
    f32 y;

    corner[0].x = 0;
    corner[0].y = 0;
    corner[2].x = drawSize;
    corner[2].y = drawSize;
    if (tileColumn >= 3) {
        tileColumn = 2;
        corner[1].x = drawSize;
        corner[1].y = 0;
        corner[3].x = 0;
        corner[3].y = drawSize;
    } else {
        corner[1].x = 0;
        corner[1].y = drawSize;
        corner[3].y = 0;
        corner[3].x = drawSize;
    }
    tileU = tileColumn * 40 + 0x284;

    for (i = 0; i < 4; i++) {
        x = corner[i].x;
        y = corner[i].y;
        corner[i].x = origin[0] + (s32) (xglCos(angle) * x - xglSin(angle) * y);
        corner[i].y = origin[1] + (s32) (xglSin(angle) * x + xglCos(angle) * y);
    }

    nmlPacketGsInit();
    gifStart = gifCursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(
        gifCursor,
        GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 904, GS_PSMCT32, 0, 0, 1),
        GS_TEX2_1);
    GIF_PUT(gifCursor, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0), GS_PRIM);
    GIF_PUT(gifCursor, GS_SET_CLAMP(2, 2, tileU, tileU + 0x27, 0x71, 0x98), GS_CLAMP_1);
    tileU = tileU << 4;
    GIF_PUT(gifCursor, GS_SET_UV(tileU + 0x280, 0x990), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(corner[0].x, corner[0].y, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(tileU, 0x990), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(corner[1].x, corner[1].y, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(tileU, 0x710), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(corner[2].x, corner[2].y, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_SET_UV(tileU + 0x280, 0x710), GS_UV);
    GIF_PUT(gifCursor, GS_XYZ(corner[3].x, corner[3].y, xglFontGetColorIndex()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

typedef short s16;

/* Two 16-bit screen coordinates passed to drawRotatedSquare as a pair. */
typedef struct ShortPoint {
    s16 x; /* 0x0 */
    s16 y; /* 0x2 */
} ShortPoint;

/* Draws four layers of drawRotatedSquare at (x, y), one per quarter turn, using the low 16 bits of drawSize as the size. */
void drawScreenPointLayers(s32 x, s32 y, s32 drawSize) {
    ShortPoint point;
    s32 drawSizeU16;

    drawSizeU16 = drawSize & 0xFFFF;
    point.x = x;
    point.y = y;
    drawRotatedSquare(&point.x, drawSizeU16, 0, 0.0f);
    drawRotatedSquare(&point.x, drawSizeU16, 0, 1.5707964f);
    drawRotatedSquare(&point.x, drawSizeU16, 0, 3.14159265f);
    drawRotatedSquare(&point.x, drawSizeU16, 0, 4.712389f);
}

extern f32 gScreenSpinAngle;

/* Advances the spin angle gScreenSpinAngle by 1/256 of a turn (wrapping at 2 pi), draws the four fixed layers
 * at (x, y) and six layers spun by the angle. */
void drawSpinningPointLayers(u16 x, u16 y, u16 drawSize) {
    ShortPoint point;
    f32 halfTurn;

    gScreenSpinAngle += 0.024543693f;
    if (gScreenSpinAngle > 6.2831855f) {
        gScreenSpinAngle -= 6.2831855f;
    }
    drawScreenPointLayers(x, y, drawSize);
    point.x = x;
    point.y = y;
    drawRotatedSquare(&point.x, drawSize, 1, gScreenSpinAngle);
    halfTurn = 3.14159265f;
    drawRotatedSquare(&point.x, drawSize, 1, gScreenSpinAngle + halfTurn);
    drawRotatedSquare(&point.x, drawSize, 3, -gScreenSpinAngle);
    drawRotatedSquare(&point.x, drawSize, 3, -(gScreenSpinAngle + halfTurn));
    drawRotatedSquare(&point.x, drawSize, 2, -(gScreenSpinAngle + 1.5707964f));
    drawRotatedSquare(&point.x, drawSize, 2, -(gScreenSpinAngle + 4.712389f));
}

f32 gScreenSpinAngle = 0.0f;
