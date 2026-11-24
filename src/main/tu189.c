#include "common.h"

#include "shared.h"
#include "gs.h"
typedef int s32;

#define GS_TEX2_1 0x16

/* One GIF packet entry: the 64-bit register value and the GS register address it is written to. */
typedef struct QueueRecord {
    u64 registerValue;
    u64 registerAddress;
} QueueRecord;

/* One-member view of xglDirectQueueCount (see QUEUE_COUNT). */
typedef struct QueueCountView {
    int queueEntryCount;
} QueueCountView;

extern QueueRecord *xglDirectQueueBuffer;
extern int xglDirectQueueCount;
extern u16 gFontClutBase[];
void nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
s32 xglFontGetColorIndex(void);

#define QUEUE_COUNT (((QueueCountView *) &xglDirectQueueCount)->queueEntryCount)

/* Adds the packets written since queueWriteStart to the queue count, then submits the queue. */
static inline void queueCommit(QueueRecord *cursor, QueueRecord *queueWriteStart) {
    QUEUE_COUNT += ((u32) cursor - (u32) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Draws the 13x17 font-texture cursor sprite at x + 2 * scale, y, scaled by scale. */
void xglFontQueueCursorSprite(s32 x, s32 y, s32 scale) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;
    s32 screenX;
    s32 screenY;
    s32 spriteWidth;
    s32 spriteHeight;
    s32 queueCount;
    u64 tex2Value;

    screenX = (x + scale * 2) & 0xFFFF;
    screenY = y & 0xFFFF;
    spriteWidth = scale * 13;
    spriteHeight = scale * 17;
    nmlPacketGsInit();
    queueCount = QUEUE_COUNT;
    tex2Value = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 0x30C, 0, 0, 0, 1);
    cursor = xglDirectQueueBuffer + queueCount;
    queueWriteStart = cursor;
    cursor->registerValue = tex2Value;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_PRIM_TEXTURED_SPRITE;
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 130, 142, 32, 48);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x820, 0x200);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(screenX, screenY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x8F0, 0x310);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(screenX + spriteWidth, screenY + spriteHeight, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    queueCommit(cursor, queueWriteStart);
}

#define GIF_PUT(cursor, gsValue, gsRegister) \
    (cursor)->registerValue = (gsValue);     \
    (cursor)->registerAddress = (gsRegister); \
    (cursor)++
#define GS_XYZ(x, y, z) GS_SET_XYZ(x, y, (u32) (z))

/* Draws the level gauge: a clamped level (0-40) sets the width of the bar, row (0 or 1) picks its height offset. */
void xglFontQueueLevelGauge(u16 x, u16 y, s32 gaugeLevel, s32 gaugeRow) {
    QueueRecord *packetStart;
    QueueRecord *cursor;

    if (gaugeLevel < 0) {
        gaugeLevel = 0;
    } else if (gaugeLevel > 40) {
        gaugeLevel = 40;
    }
    gaugeLevel = (gaugeLevel + 3) << 4;
    gaugeRow = (gaugeRow & 1) * 11 + 11;
    nmlPacketGsInit();
    packetStart = cursor = &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(cursor,
            GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 808, GS_PSMCT32, 0, 0, 1),
            GS_TEX2_1);
    GIF_PUT(cursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(cursor, GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 180, 225, 54, 64), GS_CLAMP_1);
    GIF_PUT(cursor, GS_SET_UV(0xB40, 0x360), GS_UV);
    GIF_PUT(cursor, GS_XYZ(x, y, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(cursor, GS_SET_UV(0xE20, 0x410), GS_UV);
    GIF_PUT(cursor, GS_XYZ(x + 736, y + 176, xglFontGetColorIndex()), GS_XYZ2);
    GIF_PUT(cursor, GS_PRIM_TEXTURED_SPRITE, GS_PRIM);
    GIF_PUT(cursor,
            GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 180, 180 + (gaugeLevel >> 4) - 1, gaugeRow + 54, gaugeRow + 64),
            GS_CLAMP_1);
    GIF_PUT(cursor, GS_SET_UV(0xB40, (gaugeRow << 4) + 0x360), GS_UV);
    GIF_PUT(cursor, GS_XYZ(x, y, xglFontGetColorIndex() + 1), GS_XYZ2);
    GIF_PUT(cursor, GS_SET_UV(0xB40 + gaugeLevel, (gaugeRow << 4) + 0x410), GS_UV);
    GIF_PUT(cursor, GS_XYZ(x + gaugeLevel, y + 176, xglFontGetColorIndex() + 1), GS_XYZ2);
    xglDirectQueueCount += ((u32) cursor - (u32) packetStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Upper cap of the box frame at (x, y): texel rows 0x200-0x2B0 of the strip. */
void xglFontQueueFrameCapUpper(s32 x, s32 y) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;
    s32 queueCount;
    u64 tex2Value;
    u16 spriteX;
    u16 spriteY;
    u16 rightX;
    u16 bottomY;

    spriteX = x & 0xFFFF;
    spriteY = y & 0xFFFF;
    rightX = spriteX + 176;
    bottomY = spriteY + 1328;
    nmlPacketGsInit();
    queueCount = QUEUE_COUNT;
    tex2Value = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 792, 0, 0, 0, 1);
    cursor = xglDirectQueueBuffer + queueCount;
    queueWriteStart = cursor;
    cursor->registerValue = tex2Value;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0);
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 173, 255, 32, 42);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(0xAD0, 0x200);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(rightX, spriteY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x1000, 0x200);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(rightX, bottomY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x1000, 0x2B0);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(spriteX, bottomY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0xAD0, 0x2B0);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(spriteX, spriteY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    queueCommit(cursor, queueWriteStart);
}

/* Lower cap of the box frame at (x, y): the same strip as xglFontQueueFrameCapUpper on texel rows 0x2B0-0x360. */
void xglFontQueueFrameCapLower(s32 x, s32 y) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;
    s32 queueCount;
    u64 tex2Value;
    u16 spriteX;
    u16 spriteY;
    u16 rightX;
    u16 bottomY;

    spriteX = x & 0xFFFF;
    spriteY = y & 0xFFFF;
    rightX = spriteX + 176;
    bottomY = spriteY + 1328;
    nmlPacketGsInit();
    queueCount = QUEUE_COUNT;
    tex2Value = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 792, 0, 0, 0, 1);
    cursor = xglDirectQueueBuffer + queueCount;
    queueWriteStart = cursor;
    cursor->registerValue = tex2Value;
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0);
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 173, 255, 43, 53);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(0xAD0, 0x2B0);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(rightX, spriteY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x1000, 0x2B0);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(rightX, bottomY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x1000, 0x360);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(spriteX, bottomY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0xAD0, 0x360);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(spriteX, spriteY, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    queueCommit(cursor, queueWriteStart);
}

/* Screen rectangle of a queued fill: left, top, right, bottom in pixels. */
typedef struct FontRect {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
} FontRect;

void fontQueueColorState(int mask);
void fontQueueSprite(u16 startX, u16 startY, u16 endX, u16 endY);
void fontQueueRectTwoColors(FontRect *rect, u32 firstColour, u32 secondColour, u32 colourNibbles);

/* Red bar at (x, y) for the given row count: the two caps (xglFontQueueFrameCapUpper, xglFontQueueFrameCapLower) and the rows between them. */
void xglFontQueueBarRed(u16 x, u16 y, s32 barRows) {
    FontRect rect;
    s32 halfRows;
    s32 extraRows;
    u16 bottomY;

    halfRows = barRows >> 1;
    extraRows = halfRows - 53;
    bottomY = (y + (barRows << 4)) & 0xFFFF;
    rect.left = x + 112;
    rect.right = x + 176;
    fontQueueColorState(0x804A4AE1);
    if (extraRows > 0) {
        fontQueueSprite(rect.left, y, rect.right, (y + (extraRows << 4)) & 0xFFFF);
        fontQueueSprite(rect.left, (bottomY - (extraRows << 4)) & 0xFFFF, rect.right, bottomY);
    }
    rect.top = y + (extraRows << 4);
    rect.bottom = y + (halfRows << 4);
    fontQueueRectTwoColors(&rect, 0x804A4AE1, 0x004A4AE1, 0x11);
    rect.top = bottomY - (halfRows << 4);
    rect.bottom = bottomY - (extraRows << 4);
    fontQueueRectTwoColors(&rect, 0x804A4AE1, 0x004A4AE1, 0x1100);
    fontQueueColorState(0x80808080);
    xglFontQueueFrameCapUpper(x, (y + (((barRows - 83) >> 1) << 4)) & 0xFFFF);
}

/* Blue bar: the same layout as xglFontQueueBarRed with the blue colour state and the lower cap xglFontQueueFrameCapLower. */
void xglFontQueueBarBlue(u16 x, u16 y, s32 barRows) {
    FontRect rect;
    s32 halfRows;
    s32 extraRows;
    u16 bottomY;

    halfRows = barRows >> 1;
    extraRows = halfRows - 53;
    bottomY = (y + (barRows << 4)) & 0xFFFF;
    rect.left = x + 112;
    rect.right = x + 176;
    fontQueueColorState(0x80C36866);
    if (extraRows > 0) {
        fontQueueSprite(rect.left, y, rect.right, (y + (extraRows << 4)) & 0xFFFF);
        fontQueueSprite(rect.left, (bottomY - (extraRows << 4)) & 0xFFFF, rect.right, bottomY);
    }
    rect.top = y + (extraRows << 4);
    rect.bottom = y + (halfRows << 4);
    fontQueueRectTwoColors(&rect, 0x80C36866, 0x00C36866, 0x11);
    rect.top = bottomY - (halfRows << 4);
    rect.bottom = bottomY - (extraRows << 4);
    fontQueueRectTwoColors(&rect, 0x80C36866, 0x00C36866, 0x1100);
    fontQueueColorState(0x80808080);
    xglFontQueueFrameCapLower(x, (y + (((barRows - 83) >> 1) << 4)) & 0xFFFF);
}

void fontQueueAlphaBlend(int alphaBlendMode);

/* Alpha-blended shade sprite of width x height pixels at (x, y). */
void xglFontQueueShadeSprite(u16 x, u16 y, u16 width, u16 height) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;

    fontQueueAlphaBlend(1);
    nmlPacketGsInit();
    cursor = xglDirectQueueBuffer + xglDirectQueueCount;
    queueWriteStart = cursor;
    cursor->registerValue = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 812, 0, 0, 0, 1);
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0);
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 226, 255, 54, 85);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    cursor->registerValue = GS_SET_UV(0xE20, 0x360);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(x, y, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(0x1000, 0x560);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(x + width, y + height, xglFontGetColorIndex());
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    xglDirectQueueCount += ((u32) cursor - (u32) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
    fontQueueAlphaBlend(0);
}

typedef long long s64;

/* Highlight panel at (x, y): a shade (xglFontQueueShadeSprite) and a 48x352 sprite whose colour depends on alpha and highlighted. */
void xglFontQueueHighlightPanel(u16 x, u16 y, s32 highlighted, u8 alpha) {
    QueueRecord *cursor;
    QueueRecord *queueWriteStart;
    s32 highlightIndex;

    highlightIndex = highlighted != 0 ? 3 : 0;
    if (highlightIndex != 0) {
        fontQueueColorState(((s64) ((alpha >> 3) + (alpha >> 4)) << 24) | 0x00EA0A83);
        xglFontQueueShadeSprite(x - 112, y - 176, 256, 656);
    } else {
        fontQueueColorState(((s64) ((alpha >> 3) + (alpha >> 4)) << 24) | 0x006E7E51);
        xglFontQueueShadeSprite(x - 112, y - 176, 256, 656);
    }
    fontQueueColorState((s64) alpha << 24 | 0x00808080);
    nmlPacketGsInit();
    cursor = xglDirectQueueBuffer + xglDirectQueueCount;
    queueWriteStart = cursor;
    cursor->registerValue = GS_SET_TEX0(0, 0, GS_PSMT8, 0, 0, 0, 0, (gFontClutBase[0] << 5) + 828, 0, 0, 0, 1);
    cursor->registerAddress = GS_TEX2_1;
    cursor++;
    cursor->registerValue = GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0);
    cursor->registerAddress = GS_PRIM;
    cursor++;
    cursor->registerValue = GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, highlightIndex + 228, highlightIndex + 230, 87, 108);
    cursor->registerAddress = GS_CLAMP_1;
    cursor++;
    highlightIndex <<= 4;
    cursor->registerValue = GS_SET_UV(highlightIndex + 0xE40, 0x570);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(x, y, xglFontGetColorIndex() + 1);
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    cursor->registerValue = GS_SET_UV(highlightIndex + 0xE70, 0x6D0);
    cursor->registerAddress = GS_UV;
    cursor++;
    cursor->registerValue = GS_SET_XYZ(x + 48, y + 352, xglFontGetColorIndex() + 1);
    cursor->registerAddress = GS_XYZ2;
    cursor++;
    xglDirectQueueCount += ((u32) cursor - (u32) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
}
