#include "common.h"

#include "gs.h"
typedef union XglDirectQueueEntry {
    struct {
        u64 registerValue;
        u64 registerAddress;
    };
    u32 tagAndHeaderWords[4];
} XglDirectQueueEntry;

extern XglDirectQueueEntry *xglDirectQueueBuffer;
extern int xglDirectQueueCount;

typedef int s32;

extern s32 gFontStateMask;
void nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
int fontColorModulateRgba(int mask);

typedef unsigned int u32;

/* Font colour setup: stores the font state mask, queues ALPHA and RGBAQ (the converted colour) and submits the queue. */
void fontQueueColorState(int mask) {
    int queueCount;
    u64 *cursor;
    u64 *queueStart;
    int fontColor;

    gFontStateMask = mask;
    fontColor = fontColorModulateRgba(mask);
    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueCount];
    queueStart = cursor;
    cursor[0] = GS_SET_ALPHA(0, 1, 0, 1, 0x80);
    cursor[1] = GS_ALPHA_1;
    cursor += 2;
    cursor[1] = GS_RGBAQ;
    cursor[0] = (u32) fontColor;
    cursor += 2;
    xglDirectQueueCount = queueCount + (((u32) cursor - (u32) queueStart) >> 4);
    xglDirectQueueSubmitRef();
}

/* The same ALPHA and RGBAQ queue as fontQueueColorState, with the mask itself as the colour. Sets gFontStateMask last. */
void fontQueueColorStateRaw(int mask) {
    int queueCount;
    u64 *cursor;
    u64 *queueStart;

    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueCount];
    queueStart = cursor;
    cursor[0] = GS_SET_ALPHA(0, 1, 0, 1, 0x80);
    cursor[1] = GS_ALPHA_1;
    cursor += 2;
    cursor[1] = GS_RGBAQ;
    cursor[0] = (u32) mask;
    cursor += 2;
    xglDirectQueueCount = queueCount + (((u32) cursor - (u32) queueStart) >> 4);
    xglDirectQueueSubmitRef();
    gFontStateMask = mask;
}

extern u16 D_004C8398[];

typedef struct QueueRecord {
    u64 value;
    u64 reg;
} QueueRecord;

/* A one-int view: reads the int global through a struct so the access keeps its own alias set. */
typedef struct SmallInt {
    int value;
} SmallInt;

/* The direct queue entry count (xglDirectQueueCount) through the SmallInt view. */
#define QUEUE_COUNT (((SmallInt *) &xglDirectQueueCount)->value)

/* The next free record of the direct queue. */
static inline QueueRecord *queueBegin(void) {
    return (QueueRecord *) xglDirectQueueBuffer + QUEUE_COUNT;
}

/* Adds the records between queueWriteStart and point to the queue count, then submits the queue. */
static inline void queueCommit(QueueRecord *point, QueueRecord *queueWriteStart) {
    QUEUE_COUNT += ((u32) point - (u32) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Queues the depth test and Z buffer pair: depthTest set selects the depth compare (ZTST 3) with Z writes enabled;
 * otherwise the test always passes and the Z write is masked. */
void fontQueueDepthTest(int depthTest) {
    QueueRecord *record;
    QueueRecord *queueWriteStart;

    nmlPacketGsInit();
    record = queueBegin();
    queueWriteStart = record;
    if (depthTest) {
        record->value = GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 3);
        record->reg = GS_TEST_1;
        record++;
        record->value = GS_SET_ZBUF(D_004C8398[0], GS_PSMCT24, 0);
        record->reg = GS_ZBUF_1;
        record++;
    } else {
        record->value = GS_TEST_ZTE_ALWAYS;
        record->reg = GS_TEST_1;
        record++;
        record->value = GS_SET_ZBUF(D_004C8398[0], GS_PSMCT24, 1);
        record->reg = GS_ZBUF_1;
        record++;
    }
    queueCommit(record, queueWriteStart);
}

/* Writes one record (data, register address) and advances the cursor. */
#define GS_RECORD_PUT(record, registerValue, registerAddress) \
    (record)->value = (registerValue);                        \
    (record)->reg = (registerAddress);                        \
    (record)++

/* Queues one ALPHA_1 record: blend mode 2 when alphaBlendMode is set, mode 1 otherwise; then submits. */
void fontQueueAlphaBlend(int alphaBlendMode) {
    QueueRecord *record;
    QueueRecord *queueWriteStart;

    nmlPacketGsInit();
    queueWriteStart = record = (QueueRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    if (alphaBlendMode) {
        GS_RECORD_PUT(record, GS_SET_ALPHA(0, 2, 0, 1, 0x80), GS_ALPHA_1);
    } else {
        GS_RECORD_PUT(record, GS_SET_ALPHA(0, 1, 0, 1, 0x80), GS_ALPHA_1);
    }
    xglDirectQueueCount += ((u32) record - (u32) queueWriteStart) >> 4;
    xglDirectQueueSubmitRef();
}

extern s32 gFontColorIndex;

/* Queues the test, Z buffer, ALPHA and RGBAQ state and one sprite (two XYZ2 corners), then resets gFontColorIndex to 1. */
void fontQueueSpriteReset(void) {
    int queueCount;
    u64 *cursor;
    u64 *queueStart;

    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueCount];
    queueStart = cursor;
    cursor[0] = GS_TEST_ZTE_ALWAYS;
    cursor[1] = GS_TEST_1;
    cursor += 2;
    cursor[0] = GS_SET_ZBUF(D_004C8398[0], GS_PSMCT24, 0);
    cursor[1] = GS_ZBUF_1;
    cursor += 2;
    cursor[0] = GS_SET_ALPHA(0, 1, 0, 1, 0x80);
    cursor[1] = GS_ALPHA_1;
    cursor += 2;
    cursor[0] = (u64) 0x8000 << 16;
    cursor[1] = GS_RGBAQ;
    cursor += 2;
    cursor[0] = GS_PRIM_SPRITE;
    cursor[1] = GS_PRIM;
    cursor += 2;
    cursor[0] = GS_SET_XYZ(0x6FF8, 0x71F8, 0);
    cursor[1] = GS_XYZ2;
    cursor += 2;
    cursor[0] = GS_SET_XYZ(0x8FF8, 0x8DF8, 0);
    cursor[1] = GS_XYZ2;
    cursor += 2;
    xglDirectQueueCount = queueCount + (((u32) cursor - (u32) queueStart) >> 4);
    xglDirectQueueSubmitRef();
    fontQueueDepthTest(0);
    gFontColorIndex = 1;
}

/* Same queue as fontQueueSpriteReset with another TEST record (ZTST 1, ALPHA 2 blend), an RGBAQ of 0 and one sprite. */
void fontQueueSpriteBlack(void) {
    int queueCount;
    u64 *cursor;
    u64 *queueStart;

    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueCount];
    queueStart = cursor;
    cursor[0] = GS_SET_TEST(1, 0, 0, 2, 0, 0, 1, 1);
    cursor[1] = GS_TEST_1;
    cursor += 2;
    cursor[0] = GS_SET_ZBUF(D_004C8398[0], GS_PSMCT24, 0);
    cursor[1] = GS_ZBUF_1;
    cursor += 2;
    cursor[0] = GS_SET_ALPHA(0, 1, 0, 1, 0x80);
    cursor[1] = GS_ALPHA_1;
    cursor += 2;
    cursor[0] = 0;
    cursor[1] = GS_RGBAQ;
    cursor += 2;
    cursor[1] = GS_PRIM;
    cursor[0] = GS_PRIM_SPRITE;
    cursor += 2;
    cursor[0] = GS_SET_XYZ(0x6FF8, 0x71F8, 0);
    cursor[1] = GS_XYZ2;
    cursor += 2;
    cursor[0] = GS_SET_XYZ(0x8FF8, 0x8DF8, 0);
    cursor[1] = GS_XYZ2;
    cursor += 2;
    xglDirectQueueCount = queueCount + (((u32) cursor - (u32) queueStart) >> 4);
    xglDirectQueueSubmitRef();
    fontQueueDepthTest(0);
}

typedef struct FontGifRecord {
    unsigned long value;
    unsigned long address;
} FontGifRecord;

/* Queues one FRAME_1 record: the page as FBP, psm at bit 24 and FBW 8 (0x80000); then submits. */
void fontQueueFrameBuffer(unsigned short page, unsigned char psm) {
    FontGifRecord *record;
    FontGifRecord *queueWriteStart;
    int queueCount;

    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    record = (FontGifRecord *) &xglDirectQueueBuffer[queueCount];
    queueWriteStart = record;
    record->value = (unsigned long) page | (unsigned long) psm << 24 | 0x80000;
    record->address = GS_FRAME_1;
    record++;
    xglDirectQueueCount = queueCount + (((unsigned int) record - (unsigned int) queueWriteStart) >> 4);
    xglDirectQueueSubmitRef();
}

typedef union MovieFramebufferPage {
    unsigned short value;
    int words[2];
} MovieFramebufferPage;

extern MovieFramebufferPage movieFramebufferBasePage;

/* Queues a FRAME_1 record for the movie framebuffer base page (FBW 8), then submits. */
void fontQueueMovieFrameBuffer(void) {
    FontGifRecord *record;
    FontGifRecord *queueWriteStart;

    nmlPacketGsInit();
    queueWriteStart = record = (FontGifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    record->value = (unsigned long) movieFramebufferBasePage.value | 0x80000;
    record->address = GS_FRAME_1;
    record++;
    QUEUE_COUNT = xglDirectQueueCount + (((unsigned int) record - (unsigned int) queueWriteStart) >> 4);
    xglDirectQueueSubmitRef();
}

typedef struct DirectQueueBuffer {
    QueueRecord *records;
} DirectQueueBuffer;

#define DIRECT_QUEUE_RECORDS (((DirectQueueBuffer *) &xglDirectQueueBuffer)->records)
#define SCISSOR_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SCISSOR_MIN(a, b) ((a) < (b) ? (a) : (b))

/* Queues SCISSOR_1 for the rectangle (left, top) to (right, bottom): an empty or inverted rectangle becomes 0,0,0,0 and
 * the corners clamp to the frame (right at most 511, bottom at most 447). */
void fontQueueScissor(int scissorLeft, int scissorTop, int scissorRight, int scissorBottom) {
    QueueRecord *record;
    QueueRecord *queueWriteStart;

    if (scissorLeft >= scissorRight || scissorTop >= scissorBottom) {
        scissorLeft = 0;
        scissorTop = 0;
        scissorRight = 0;
        scissorBottom = 0;
    }
    scissorLeft = SCISSOR_MAX(scissorLeft, 0);
    scissorTop = SCISSOR_MAX(scissorTop, 0);
    scissorRight = SCISSOR_MIN(scissorRight, 0x1FF);
    scissorBottom = SCISSOR_MIN(scissorBottom, 0x1BF);
    nmlPacketGsInit();
    queueWriteStart = record = DIRECT_QUEUE_RECORDS + xglDirectQueueCount;
    record->value = (u64) scissorLeft | (u64) scissorRight << 16 | (u64) scissorTop << 32 | (u64) scissorBottom << 48;
    record->reg = GS_SCISSOR_1;
    record++;
    QUEUE_COUNT = xglDirectQueueCount + (((u32) record - (u32) queueWriteStart) >> 4);
    xglDirectQueueSubmitRef();
}

/* Queues SCISSOR_1 for the whole 512x448 frame (maximum corner 511, 447), then submits. */
void fontQueueFullScissor(void) {
    FontGifRecord *record;
    FontGifRecord *queueWriteStart;

    nmlPacketGsInit();
    queueWriteStart = record = (FontGifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    record->value = (unsigned long) (512 - 1) << 16 | (unsigned long) (448 - 1) << 48;
    record->address = GS_SCISSOR_1;
    record++;
    QUEUE_COUNT = xglDirectQueueCount + (((unsigned int) record - (unsigned int) queueWriteStart) >> 4);
    xglDirectQueueSubmitRef();
}

/* Queues a PRIM record (value 0x41) and two XYZ2 corners, (startX, startY) and (endX, endY), with gFontColorIndex as the
 * Z value; then submits. */
void fontQueueLine(u16 startX, u16 startY, u16 endX, u16 endY) {
    int queueCount;
    u64 *cursor;
    u64 *queueStart;
    u64 startCorner;
    u64 endCorner;

    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueCount];
    queueStart = cursor;
    startCorner = GS_SET_XYZ(startX, startY, (u32) gFontColorIndex);
    endCorner = GS_SET_XYZ(endX, endY, (u32) gFontColorIndex);
    cursor[0] = 0x41;
    cursor[1] = GS_PRIM;
    cursor += 2;
    cursor[0] = startCorner;
    cursor[1] = GS_XYZ2;
    cursor += 2;
    cursor[0] = endCorner;
    cursor[1] = GS_XYZ2;
    cursor += 2;
    xglDirectQueueCount = queueCount + (((u32) cursor - (u32) queueStart) >> 4);
    xglDirectQueueSubmitRef();
}

#define FONT_COLOR (((SmallInt *) &gFontColorIndex)->value)

/* Queues a PRIM record (value 0x42) and one XYZ2 vertex per (x, y) pair in points, with the font colour as Z; then
 * commits. */
void fontQueuePoints(u16 *points, int pointCount) {
    QueueRecord *record;
    QueueRecord *queueWriteStart;
    int remaining;
    u16 *point;
    u64 fontColor;

    nmlPacketGsInit();
    record = queueBegin();
    queueWriteStart = record;
    record->value = 0x42;
    record->reg = GS_PRIM;
    record++;
    if (pointCount > 0) {
        point = points;
        remaining = pointCount;
        fontColor = (u32) FONT_COLOR;
        do {
            record->value = (u64) point[0] | ((u64) point[1] << 16) | (fontColor << 32);
            record->reg = GS_XYZ2;
            record++;
            point += 2;
        } while (--remaining != 0);
    }
    queueCommit(record, queueWriteStart);
}

typedef struct ScreenPoint {
    u16 x;
    u16 y;
} ScreenPoint;

/* Queues a 0x4A header, then for each point from the last to the first: the colour of the next 4-bit index of nibbles
 * (looked up in colourTable and converted by fontColorModulateRgba), an RGBAQ record and the XYZ2 vertex of the point. Then
 * submits and restores the font state mask. */
void fontQueueColoredPoints(ScreenPoint *points, u32 *colourTable, u32 nibbles, int pointCount) {
    int i;
    int savedFontMask = gFontStateMask;
    QueueRecord *record;
    QueueRecord *queueWriteStart;

    nmlPacketGsInit();
    record = queueBegin();
    queueWriteStart = record;
    record->value = 0x4A;
    record->reg = GS_PRIM;
    record++;
    for (i = pointCount - 1; i >= 0; i--) {
        record->value = (u32) fontColorModulateRgba(colourTable[nibbles & 0xF]);
        nibbles >>= 4;
        record->reg = GS_RGBAQ;
        record++;
        record->value = (u64) points[i].x | ((u64) points[i].y << 16) | ((u64) (u32) FONT_COLOR << 32);
        record->reg = GS_XYZ2;
        record++;
    }
    queueCommit(record, queueWriteStart);
    fontQueueColorState(savedFontMask);
}

typedef unsigned char u8;

/* Queues a PRIM record (sprite with alpha blend) and two XYZ2 corners, (startX, startY) and (endX, endY), with the font
 * colour as Z; then submits. */
void fontQueueSprite(u16 startX, u16 startY, u16 endX, u16 endY) {
    XglDirectQueueEntry *record;
    XglDirectQueueEntry *firstRecord;
    u64 fontColor;

    nmlPacketGsInit();
    record = &xglDirectQueueBuffer[xglDirectQueueCount];
    firstRecord = record;
    fontColor = (u32) gFontColorIndex;
    record->registerValue = GS_PRIM_SPRITE_ABE;
    record->registerAddress = GS_PRIM;
    record++;
    record->registerValue = (u64) startX | ((u64) startY << 16) | ((u64) fontColor << 32);
    record->registerAddress = GS_XYZ2;
    record++;
    record->registerValue = (u64) endX | ((u64) endY << 16) | ((u64) fontColor << 32);
    record->registerAddress = GS_XYZ2;
    record++;
    xglDirectQueueCount += (u32) ((u8 *) record - (u8 *) firstRecord) >> 4;
    xglDirectQueueSubmitRef();
}

static inline int fontDepthIndexGet(void) {
    return ((SmallInt *) &gFontColorIndex)->value;
}

typedef struct GifRecord {
    u64 value;
    u64 address;
} GifRecord;

typedef struct FontRect {
    u16 left;
    u16 top;
    u16 right;
    u16 bottom;
} FontRect;

/* PRIM values of the packets below: the PRIM type in bits 0-2, IIP (gouraud) in bit 3, ABE (alpha blend) in bit 6. */
#define FONT_PRIM_LINE_ABE 0x41
#define FONT_PRIM_TRISTRIP_ABE 0x44
#define FONT_PRIM_TRISTRIP_GOURAUD_ABE 0x4C
#define FONT_PRIM_TRIFAN_GOURAUD_ABE 0x4D

#define GIF_PUT(gifCursor, registerValue, registerAddress) \
    (gifCursor)->value = (registerValue);                  \
    (gifCursor)->address = (registerAddress);              \
    (gifCursor)++
#define GS_XYZ(x, y, z) GS_SET_XYZ(x, y, (u32) (z))

/* Queues a filled rect with four colours: colours[] indexed by the nibbles of colourNibbles (top-left
 * corner from the highest nibble), then restores the saved font state mask. */
void fontQueueRectFourColors(FontRect *rect, u32 *colours, u32 colourNibbles) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    u32 savedStateMask;

    savedStateMask = gFontStateMask;
    nmlPacketGsInit();
    gifStart = gifCursor =
        (GifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, FONT_PRIM_TRISTRIP_GOURAUD_ABE, GS_PRIM);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[(colourNibbles >> 12) & 0xF]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->top, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[(colourNibbles >> 8) & 0xF]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->top, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[(colourNibbles >> 4) & 0xF]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[colourNibbles & 0xF]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
    fontQueueColorState(savedStateMask);
}

/* As fontQueueRectFourColors with two colours (firstColour, secondColour) picked by one bit per corner of colourNibbles. */
void fontQueueRectTwoColors(FontRect *rect, u32 firstColour, u32 secondColour, u32 colourNibbles) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    u32 colours[2];
    int savedStateMask;

    savedStateMask = gFontStateMask;
    colours[0] = firstColour;
    colours[1] = secondColour;
    nmlPacketGsInit();
    gifStart = gifCursor =
        (GifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, FONT_PRIM_TRISTRIP_GOURAUD_ABE, GS_PRIM);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[(colourNibbles >> 12) & 1]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->top, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[(colourNibbles >> 8) & 1]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->top, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[(colourNibbles >> 4) & 1]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(colours[colourNibbles & 1]), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
    fontQueueColorState(savedStateMask);
}

/* Queues one triangle-fan packet (PRIM 0x4D) of count points: each point takes the colour
 * colourTable[index] for the 4-bit index in colourIndexes (low nibble first), then the font depth.
 * The points are emitted from the last to the first. Restores the saved font state mask. */
void fontQueueFanPoints(ScreenPoint *points, u32 *colourTable, u32 colourIndexes, int pointCount) {
    int i;
    int savedStateMask = gFontStateMask;
    QueueRecord *cursor;
    QueueRecord *packetStart;

    nmlPacketGsInit();
    cursor = queueBegin();
    packetStart = cursor;
    cursor->value = FONT_PRIM_TRIFAN_GOURAUD_ABE;
    cursor->reg = GS_PRIM;
    cursor++;
    for (i = pointCount - 1; i >= 0; i--) {
        cursor->value = (u32) fontColorModulateRgba(colourTable[colourIndexes & 0xF]);
        colourIndexes >>= 4;
        cursor->reg = GS_RGBAQ;
        cursor++;
        cursor->value = (u64) points[i].x | ((u64) points[i].y << 16) | ((u64) (u32) FONT_COLOR << 32);
        cursor->reg = GS_XYZ2;
        cursor++;
    }
    queueCommit(cursor, packetStart);
    fontQueueColorState(savedStateMask);
}

/* Queues the outline of a rect: its points offset by 16 in the four directions as a line primitive
 * (PRIM 0x41) in the outline colour, then the rect itself in the state colour one depth step further. */
void fontQueueOutline(u16 startX, u16 startY, u16 endX, u16 endY) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    u32 color = gFontStateMask;
    u32 outlineColor = (color & 0xFFFFFF) | ((color >> 26) << 24);

    nmlPacketGsInit();
    gifStart = gifCursor =
        (GifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(gifCursor, FONT_PRIM_LINE_ABE, GS_PRIM);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(outlineColor), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(endX + 16, endY, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(startX + 16, startY, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(startX, startY + 16, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(endX, endY + 16, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(endX - 16, endY, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(startX - 16, startY, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(startX, startY - 16, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(endX, endY - 16, fontDepthIndexGet()), GS_XYZ2);
    GIF_PUT(gifCursor, (u32) fontColorModulateRgba(color), GS_RGBAQ);
    GIF_PUT(gifCursor, GS_XYZ(startX, startY, fontDepthIndexGet() + 1), GS_XYZ2);
    GIF_PUT(gifCursor, GS_XYZ(endX, endY, fontDepthIndexGet() + 1), GS_XYZ2);
    xglDirectQueueCount += ((u32) gifCursor - (u32) gifStart) >> 4;
    xglDirectQueueSubmitRef();
}

/* Queues a two-colour frame of rect: the top and right edges of the ring between rect and the inner
 * rect (inset packs the inner rect's x and y steps in its nibbles) in firstColor, the left and bottom edges in
 * secondColor. Then restores the saved font state mask. */
void fontQueueBevelFrame(FontRect *rect, int inset, int firstColor, int secondColor) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    FontRect inner;
    int savedStateMask;
    int insetX;
    int insetY;

    if (inset != 0) {
        insetX = inset >> 4;
        insetY = (inset & 0xF) << 4;
        if (insetX == 0) {
            insetX = inset;
        }
        insetX <<= 4;
        inner.left = rect->left + insetX;
        inner.top = rect->top + insetY;
        inner.right = rect->right - insetX;
        inner.bottom = rect->bottom - insetY;
        savedStateMask = gFontStateMask;
        nmlPacketGsInit();
        gifStart = gifCursor =
            (GifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
        GIF_PUT(gifCursor, FONT_PRIM_TRISTRIP_ABE, GS_PRIM);
        GIF_PUT(gifCursor, (u32) fontColorModulateRgba(firstColor), GS_RGBAQ);
        GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.left, inner.top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.right, inner.top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.right, inner.bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, FONT_PRIM_TRISTRIP_ABE, GS_PRIM);
        GIF_PUT(gifCursor, (u32) fontColorModulateRgba(secondColor), GS_RGBAQ);
        GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.left, inner.top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.left, inner.bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.right, inner.bottom, fontDepthIndexGet()), GS_XYZ2);
        QUEUE_COUNT = xglDirectQueueCount + (((u32) gifCursor - (u32) gifStart) >> 4);
        xglDirectQueueSubmitRef();
        fontQueueColorState(savedStateMask);
    }
}

u32 colorBlendAverageAlpha(u32 colourA, u32 colourB, u32 alpha);
u32 colorAddSaturated(u32 mask, u32 tint, u32 alpha, u32 blend);

/* Draws a framed rect: the two-colour frame of fontQueueBevelFrame, then the inner rect, filled with
 * colour through fontQueueRectTwoColors when blend is set and through fontQueueSprite otherwise. */
void fontDrawFramedRect(u16 *rect, int inset, u32 colour, u32 blend) {
    u16 inner[4];
    u32 mask;
    u32 tint;
    u32 blended;
    int stepX;
    int dy;
    int dyCopy;

    mask = gFontStateMask;
    stepX = inset >> 4;
    tint = colorAddSaturated(mask, 0x202020, mask >> 24, blend);
    if (stepX == 0) {
        stepX = inset;
    }
    if (blend != 0) {
        blended = colorBlendAverageAlpha(colour, 0, colour >> 24);
    } else {
        blended = colorBlendAverageAlpha(mask, 0, mask >> 24);
    }
    fontQueueBevelFrame(rect, inset, tint, blended);

    if (blend != 0) {
        dy = (inset & 0xF) * 16;
        dyCopy = dy + ((dy & 0xFF) >> 8);
        inner[0] = rect[0] + stepX * 16;
        inner[1] = rect[1] + dyCopy;
        inner[2] = rect[2] - stepX * 16;
        inner[3] = rect[3] - dyCopy;
        fontQueueRectTwoColors(inner, colour, mask, 0x1100);
        fontQueueColorState(mask);
    } else {
        dy = (inset & 0xF) * 16;
        fontQueueSprite(
            (u16) (rect[0] + stepX * 16), (u16) (rect[1] + dy), (u16) (rect[2] - stepX * 16),
            (u16) (rect[3] - dy));
    }
}

/* As fontQueueBevelFrame, with the frame vertices in the reverse order. The vertex order decides the
 * emitted packet, so both stay. */
void fontQueueBevelFrameReversed(FontRect *rect, int inset, int firstColor, int secondColor) {
    GifRecord *gifCursor;
    GifRecord *gifStart;
    FontRect inner;
    int savedStateMask;
    int insetX;
    int insetY;

    if (inset != 0) {
        insetX = inset >> 4;
        insetY = (inset & 0xF) << 4;
        if (insetX == 0) {
            insetX = inset;
        }
        insetX <<= 4;
        inner.left = rect->left + insetX;
        inner.top = rect->top + insetY;
        inner.right = rect->right - insetX;
        inner.bottom = rect->bottom - insetY;
        savedStateMask = gFontStateMask;
        nmlPacketGsInit();
        gifStart = gifCursor =
            (GifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
        GIF_PUT(gifCursor, FONT_PRIM_TRISTRIP_ABE, GS_PRIM);
        GIF_PUT(gifCursor, (u32) fontColorModulateRgba(firstColor), GS_RGBAQ);
        GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.right, inner.top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.left, inner.top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.left, inner.bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, FONT_PRIM_TRISTRIP_ABE, GS_PRIM);
        GIF_PUT(gifCursor, (u32) fontColorModulateRgba(secondColor), GS_RGBAQ);
        GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.right, inner.top, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->right, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.right, inner.bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(rect->left, rect->bottom, fontDepthIndexGet()), GS_XYZ2);
        GIF_PUT(gifCursor, GS_XYZ(inner.left, inner.bottom, fontDepthIndexGet()), GS_XYZ2);
        QUEUE_COUNT = xglDirectQueueCount + (((u32) gifCursor - (u32) gifStart) >> 4);
        xglDirectQueueSubmitRef();
        fontQueueColorState(savedStateMask);
    }
}

/* Draws the tinted box of a font rect, then either its inner rect (blendMode set) or
 * a sprite-box fill of the same rect. inset packs the x inset (high bits) and the y
 * inset (low nibble); both are scaled to GS units (x16). kept: the innerInsetY term
 * ((insetY & 0xFF) >> 8) is zero for every inset, but its ops decide the emitted code. */
void xglFontDrawBoxBlend(u16 *rect, int inset, u32 colour, u32 blendMode) {
    FontRect innerRect;
    u32 stateMask;
    u32 tintColour;
    u32 fillColour;
    int insetX;
    int insetY;
    int innerInsetY;

    stateMask = gFontStateMask;
    insetX = inset >> 4;
    tintColour = colorAddSaturated(stateMask, 0x202020, stateMask >> 24, blendMode);
    if (insetX == 0) {
        insetX = inset;
    }
    if (blendMode != 0) {
        fillColour = colorBlendAverageAlpha(colour, 0, colour >> 24);
    } else {
        fillColour = colorBlendAverageAlpha(stateMask, 0, stateMask >> 24);
    }
    fontQueueBevelFrameReversed(rect, inset, tintColour, fillColour);

    if (blendMode != 0) {
        insetY = (inset & 0xF) * 16;
        innerInsetY = insetY + ((insetY & 0xFF) >> 8);
        innerRect.left = rect[0] + insetX * 16;
        innerRect.top = rect[1] + innerInsetY;
        innerRect.right = rect[2] - insetX * 16;
        innerRect.bottom = rect[3] - innerInsetY;
        fontQueueRectTwoColors(&innerRect, colour, stateMask, 0x1100);
        fontQueueColorState(stateMask);
    } else {
        insetY = (inset & 0xF) * 16;
        fontQueueSprite(
            (rect[0] + insetX * 16), (rect[1] + insetY), (rect[2] - insetX * 16), (rect[3] - insetY));
    }
}

/* Depth (z) of the font vertices: the current font colour index. */
static inline s32 fontVertexDepth(void) {
    return ((SmallInt *) &gFontColorIndex)->value;
}

void xglFontDrawFramedRect(FontRect *rect, s32 overlayLevel, s32 overlayColour);

/* Draws the framed font box of rect: two corner-bevel strips in two colours and a
 * triangle in front (depth + 1), then an overlay pass when overlayLevel is set. */
void xglFontDrawFramedRect(FontRect *rect, s32 overlayLevel, s32 overlayColour) {
    GifRecord *record;
    GifRecord *firstRecord;

    fontQueueDepthTest(1);
    nmlPacketGsInit();
    firstRecord = record = (GifRecord *) &xglDirectQueueBuffer[xglDirectQueueCount];
    GIF_PUT(record, 0x44, GS_PRIM);
    GIF_PUT(record, (u32) fontColorModulateRgba(0x73443C3A), GS_RGBAQ);
    GIF_PUT(record, GS_XYZ(rect->right, rect->top, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right - 0x20, rect->top + 0x20, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->left, rect->top, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->left + 0x20, rect->top + 0x20, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->left, rect->bottom, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->left + 0x20, rect->bottom - 0x20, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, 0x44, GS_PRIM);
    GIF_PUT(record, (u32) fontColorModulateRgba(0x73181514), GS_RGBAQ);
    GIF_PUT(record, GS_XYZ(rect->right, rect->top, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right - 0x20, rect->top + 0x20, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right, rect->bottom - 0x80, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right - 0x20, rect->bottom - 0x90, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right - 0x80, rect->bottom, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right - 0x90, rect->bottom - 0x20, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->left, rect->bottom, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->left + 0x20, rect->bottom - 0x20, fontVertexDepth()), GS_XYZ2);
    GIF_PUT(record, 0x43, GS_PRIM);
    GIF_PUT(record, 0, GS_RGBAQ);
    GIF_PUT(record, GS_XYZ(rect->right, rect->bottom, fontVertexDepth() + 1), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right - 0x80, rect->bottom, fontVertexDepth() + 1), GS_XYZ2);
    GIF_PUT(record, GS_XYZ(rect->right, rect->bottom - 0x80, fontVertexDepth() + 1), GS_XYZ2);
    xglDirectQueueCount += ((u32) record - (u32) firstRecord) >> 4;
    xglDirectQueueSubmitRef();
    fontQueueRectTwoColors(rect, 0x73221E1D, 0x73312A29, 0x11);
    if (overlayLevel != 0) {
        gFontColorIndex += 1;
        fontQueueRectTwoColors(rect, (overlayColour & 0xFFFFFF) | (overlayLevel << 24), overlayColour & 0xFFFFFF, 0x101);
        gFontColorIndex -= 1;
    }
    fontQueueDepthTest(0);
}

typedef signed char s8;

void xglFontDrawDefault(FontRect *rect, s32 level) {
    xglFontDrawFramedRect(rect, level, 0xA8B234);
}

extern u8 gFontAlphaScale;

void xglFontSetAlphaScale(s8 alphaScale) {
    gFontAlphaScale = alphaScale;
}

u8 xglFontGetAlphaScale(void) {
    return gFontAlphaScale;
}

s32 xglFontScaleAlpha(s32 alpha) {
    s32 scaled;

    scaled = ((alpha & 0xFF) * gFontAlphaScale) >> 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    return scaled & 0xFF;
}

s32 xglFontScaleColorAlpha(u32 color) {
    s32 scaled;

    scaled = (color >> 0x18) * gFontAlphaScale;
    scaled >>= 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    return (color & 0xFFFFFF) | (scaled << 0x18);
}

extern u8 gFontColorScale;

void xglFontSetColorScale(u8 colorScale) {
    gFontColorScale = colorScale;
}

u8 xglFontGetColorScale(void) {
    return gFontColorScale;
}

s32 xglFontScaleColorChannel(s32 channelValue) {
    s32 scaled;

    scaled = ((channelValue & 0xFF) * gFontColorScale) >> 7;
    if (scaled >= 0x100) {
        scaled = 0xFF;
    }
    return scaled & 0xFF;
}

void xglFontSetColorIndex(s32 colorIndex) {
    gFontColorIndex = colorIndex;
}

void xglFontColorIndexInc(void) {
    gFontColorIndex += 1;
}

void xglFontColorIndexDec(void) {
    gFontColorIndex -= 1;
}

void xglFontColorIndexAdd(s32 delta) {
    gFontColorIndex += delta;
}

s32 xglFontGetColorIndex(void) {
    return gFontColorIndex;
}

void xglFontPrint(s32 x, s32 y, s32 color, const char *text);

void xglFontPrintIndexed(s32 x, s32 y, const char *text) {
    xglFontPrint(x, y, gFontColorIndex + 2, text);
}

s32 xglFontGetStateMask(void) {
    return gFontStateMask;
}

/* A text cursor: the pen position in GS units (x and y) and the fields the text
 * escapes do not touch. */
typedef struct TextCursor {
    unsigned char unmodeled_00[0x8];
    unsigned short positionX;
    unsigned short positionY;
    unsigned char unmodeled_0C[0x2A - 0xC];
    unsigned char unk_2A;
} TextCursor;

/* kept apart: a whole one-byte struct store has its own alias set (gcc 2.96), so
 * the pending-glyph bytes are stored through this type, not as plain unsigned char. */
typedef struct ByteCell {
    unsigned char value;
} ByteCell;

extern unsigned char gPendingGlyphActive;
extern unsigned char gPendingGlyphKind;
extern unsigned char gPendingGlyphFade;
extern unsigned short gPendingGlyphX;
extern unsigned short gPendingGlyphY;

/* Escape handler for "%button<digit>": records a pending button glyph of kind buttonKind at the
 * cursor (its x and y+4, the cursor byte at 0x2A), then advances the cursor past the glyph
 * (15 units for kinds 4 and up, 20 otherwise). */
void xglFontQueueButtonGlyph(TextCursor *cursor, int buttonKind) {
    ByteCell glyphKind;
    ByteCell glyphPending;
    unsigned short positionX;
    unsigned short positionY;
    int cursorTrailerByte;

    glyphKind.value = buttonKind;
    *(ByteCell *) &gPendingGlyphKind = glyphKind;
    positionY = cursor->positionY;
    positionX = cursor->positionX;
    cursorTrailerByte = cursor->unk_2A;
    gPendingGlyphY = positionY + 4;
    gPendingGlyphFade = cursorTrailerByte;
    gPendingGlyphX = positionX;
    if ((unsigned char) buttonKind >= 4u) {
        cursor->positionX = positionX + 0xF;
    } else {
        cursor->positionX = positionX + 0x14;
    }
    glyphPending.value = 1;
    *(ByteCell *) &gPendingGlyphActive = glyphPending;
}

/* Escape byte that makes the text printer call a handler: it is followed by the
 * 24-bit handler address (3 bytes), three zero bytes and the handler argument. */
#define FONT_ESCAPE_CALL 23

/* Rewrites each "%button<digit>" in text into a FONT_ESCAPE_CALL escape that calls
 * xglFontQueueButtonGlyph; an escape already in place is skipped whole. */
void xglFontExpandButtonEscapes(char *text) {
    if (text == 0) {
        return;
    }
    while (*text != 0) {
        if (*text & 0x80) {
            text++;
        } else if (*text == '%' && text[1] == 'b' && text[2] == 'u' && text[3] == 't' && text[4] == 't' &&
                   text[5] == 'o' && text[6] == 'n' && text[7] != 0) {
            u32 callTarget = (u32) xglFontQueueButtonGlyph;
            u8 buttonDigit = text[7] - '0';

            text[0] = FONT_ESCAPE_CALL;
            text[1] = callTarget >> 16;
            text[2] = callTarget >> 8;
            text[3] = callTarget;
            text[4] = 0;
            text[5] = 0;
            text[6] = 0;
            text[7] = buttonDigit;
            text += 7;
        } else if (*text == FONT_ESCAPE_CALL) {
            text += 7;
        }
        text++;
    }
}

extern char *gFontNumberBuffer;
int xglFontGetSPcodeSize(int opcode, const unsigned char *text);

/* Copies text into gFontNumberBuffer: a control code (below 32) is copied whole, and a
 * literal backslash-n becomes a line feed followed by the code 7. */
char *xglFontCopyWithLineBreaks(char *text) {
    char *cursor = gFontNumberBuffer;

    if (text != 0) {
        while (*text != 0) {
            if ((u8) *text < 32) {
                int remainingParamBytes = xglFontGetSPcodeSize((u8) text[0], text);

                while (remainingParamBytes > 0) {
                    *cursor++ = *text++;
                    remainingParamBytes--;
                }
            } else if (*text == '\\' && text[1] == 'n') {
                *cursor++ = '\n';
                *cursor++ = 7;
                text += 2;
                continue;
            }
            *cursor++ = *text++;
        }
    }
    *cursor = 0;
    return gFontNumberBuffer;
}

/* Same as xglFontCopyWithLineBreaks, but a literal backslash-n becomes a space followed by the code 7. */
char *xglFontCopyWithSpaceBreaks(char *text) {
    char *cursor = gFontNumberBuffer;

    if (text != 0) {
        while (*text != 0) {
            if ((u8) *text < 32) {
                int remainingParamBytes = xglFontGetSPcodeSize((u8) text[0], text);

                while (remainingParamBytes > 0) {
                    *cursor++ = *text++;
                    remainingParamBytes--;
                }
            } else if (*text == '\\' && text[1] == 'n') {
                *cursor++ = ' ';
                *cursor++ = 7;
                text += 2;
                continue;
            }
            *cursor++ = *text++;
        }
    }
    *cursor = 0;
    return gFontNumberBuffer;
}

typedef struct LabelBlob {
    u8 prefix[3];
    u8 labelHi;
    u8 labelLo;
    u8 suffix;
} LabelBlob;

extern LabelBlob D_00538EB0;
extern char xglFontTitleString[];
void xglFontPrintTitle(void);
void xglFontPrintEscapeByte(s32 code);
void xglFontPrintExtendedColor(s32 a, s32 b, s32 c);
void xglFontSetColor(s32 r, s32 g, s32 b);
void xglFontPrintAlphaCommand(s32 a, s32 b, s32 c);
void xglFontResetState(void);
void func_0023BF30(s32 x, s32 y, s32 shift);

/* Prints the title xglFontTitleString at the origin, then text at (x, y) with its escapes. Then it
 * draws the pending pulse glyph (gPendingGlyphActive) when one is set and clears the flag. */
void xglFontPrintTitledText(s32 x, s32 y, char *text, s32 labelValue) {
    LabelBlob labelBlob;
    u8 colorScale;
    s32 stateMask;
    char *escapedText;
    u8 fadeLevel;

    if (text != 0) {
        labelBlob = D_00538EB0;
        colorScale = xglFontGetColorScale();
        stateMask = xglFontGetStateMask();
        xglFontExpandButtonEscapes(text);
        xglFontPrintTitle();
        xglFontPrintEscapeByte(14);
        xglFontPrintExtendedColor(0, 0, 0);
        xglFontSetColorScale(128);
        xglFontSetColor(stateMask & 0xff, (stateMask & 0xff00) >> 8, ((u32) stateMask >> 16) & 0xff);
        xglFontPrintAlphaCommand(68, 0, colorScale);
        xglFontSetColorScale(colorScale);
        xglFontPrintIndexed(0, 0, xglFontTitleString);
        labelValue -= 10;
        labelBlob.labelHi = labelValue >> 8;
        labelBlob.labelLo = labelValue;
        escapedText = xglFontCopyWithLineBreaks(text);
        xglFontPrintIndexed(x, y, escapedText);
        xglFontResetState();
        fadeLevel = gPendingGlyphFade;
        if (gPendingGlyphActive != 0) {
            fontQueueColorState((unsigned long) fadeLevel << 24 | 0x808080);
            func_0023BF30(((gPendingGlyphX * 16) + 0x6FF8) & 0xFFF8, ((gPendingGlyphY * 16) + 0x71F8) & 0xFFF8,
                          1 << gPendingGlyphKind);
            gPendingGlyphActive = 0;
        }
    }
}
