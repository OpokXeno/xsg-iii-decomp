#include "common.h"

#include "shared.h"
#include "vu0_macro.h"
#include "gs.h"
#include "main/xgl_2.h"
typedef struct FontCellCursor {
    unsigned short x;
    unsigned short y;
} FontCellCursor;

typedef struct DirectQueueQuad {
    unsigned long long lo;
    unsigned long long hi;
} DirectQueueQuad;

typedef int s32;

typedef struct FontDrawObject {
    unsigned char unmodeled_00[0x40];
    s32 slotSource;
    s32 passMode;
    s32 passCell;
    unsigned char unmodeled_4c[4];
    s32 flags50;
} FontDrawObject;

/* The video mode page words the font code reads; one object, D_004C8390. */
typedef struct VideoModePages {
    u16 unmodeled_00[4];
    u16 zBufferPage;
    u16 unmodeled_0A[2];
    u16 texturePage;
    unsigned char unmodeled_10[2];
    u16 unmodeled_12;
} VideoModePages;

extern VideoModePages D_004C8390;
extern int xglDirectQueueCount;
extern DirectQueueQuad *xglDirectQueueBuffer;
void fontQueueFrameBuffer(s32 value, s32 zero);
void fontQueueSpriteReset(void);
void fontQueueColorState(s32 color);
void nmlPacketGsInit(void);
void xglDirectQueueSubmitRef(void);
s32 xglFontSetAlphaScale(s32);
s32 slotKindFromMode(s32 slotSource, s32 mode);
void func_0021F3D0(FontCellCursor *cursor, s32 slotSource, s32 index, s32 flags);
s32 fontQueueMovieFrameBuffer(void);
s32 fontObjectCellsDraw(s32 drawObjectHandle, s32 pass);

/* Draws the cell grid of one font draw object: a GIF header packet, then one cell per slot cell of two slot modes. */
s32 fontObjectCellsDraw(s32 drawObjectHandle, s32 drawPass) {
    FontDrawObject *drawObject = (FontDrawObject *) drawObjectHandle;
    FontCellCursor cursor;
    unsigned long long *gifWrite;
    unsigned long long *gifStart;
    s32 queueIndex;
    s32 slotMode;
    s32 passIndex;
    s32 cellIndex;
    s32 cellsDrawn;
    s32 slotCellCount;
    s32 isPassCell;
    s32 slotSource;
    s32 slotCellIndex;
    s32 gifOriginX;

    slotMode = drawPass != 0 ? 2 : 0;
    fontQueueFrameBuffer(D_004C8390.texturePage, 0);
    fontQueueSpriteReset();
    xglFontSetAlphaScale(128);
    fontQueueColorState(0x80808080);
    nmlPacketGsInit();
    gifOriginX = D_004C8390.unmodeled_12 << 5;
    queueIndex = xglDirectQueueCount;
    gifWrite = (unsigned long long *) &xglDirectQueueBuffer[queueIndex];
    gifStart = gifWrite;
    gifWrite[1] = 6;
    gifWrite[0] = 0x29340000 | gifOriginX | ((unsigned long long) gifOriginX << 37) | ((unsigned long long) 0x20000006 << 32);
    gifWrite += 2;
    xglDirectQueueCount = queueIndex + (((unsigned int) gifWrite - (unsigned int) gifStart) >> 4);
    xglDirectQueueSubmitRef();
    cursor.x = 0x73F8;
    cursor.y = 0x7478;
    cellsDrawn = 0;
    for (passIndex = 0; passIndex < 2; passIndex++, slotMode++) {
        slotCellCount = slotKindFromMode(drawObject->slotSource, slotMode);
        for (cellIndex = 0; cellIndex < slotCellCount; cellIndex++) {
            if (drawPass == 0 || cellIndex != 0) {
                slotSource = drawObject->slotSource;
                slotCellIndex = slotMode * 8 + cellIndex;
                isPassCell = 0;
                if (drawObject->passMode == slotMode) {
                    isPassCell = drawObject->passCell == cellIndex;
                }
                func_0021F3D0(&cursor, slotSource, slotCellIndex, isPassCell | (drawObject->flags50 * 2));
            }
            cellsDrawn++;
            cursor.x += 0x800;
            if ((cellsDrawn & 3) == 0) {
                cursor.x = 0x73F8;
                cursor.y += 0x500;
            }
        }
    }
    return fontQueueMovieFrameBuffer();
}

s32 _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);
s32 xglFontScaleColorAlpha(unsigned int color);

typedef union GifRecord {
    struct {
        unsigned long value;
        unsigned long reg;
    } fields;
    unsigned long words[2];
} GifRecord;

#define GIF_PUT(record, registerValue, registerAddress) \
    (record)->words[0] = (registerValue);               \
    (record)->words[1] = (registerAddress);             \
    (record)++
#define FLOAT_WORD(value) (*(s32 *) &(value))
#define GS_STView(st) ((unsigned long) FLOAT_WORD((st).x) | (unsigned long) FLOAT_WORD((st).y) << 32)
#define GS_XYZ(vertex) ((unsigned long) (vertex).x | (unsigned long) (vertex).y << 16 | (unsigned long) (vertex).z << 32)
#define GS_RGBAQView(rgba, vertex) ((unsigned long) (rgba) | (unsigned long) FLOAT_WORD((vertex).w) << 32)

/* Draws one 128x80 font glyph quad at texel (texelU, texelV) with the grey level as its colour, clipped as a whole. */
void xglFontGlyphQuadDraw(const Vector4 *position, s32 texelU, s32 texelV, s32 grayLevel) {
    Vector4 corner[4];
    ScreenVector4 screen[4];
    Vector4 st[4];
    GifRecord *record;
    GifRecord *gifRecordsFrom;
    float *stWord;
    float *screenW;
    s32 i;
    s32 queueCount;
    unsigned int rgba;

    corner[0].x = position->x - 1.6f;
    corner[0].y = position->y + 1.0f;
    corner[0].z = position->z;
    corner[0].w = position->w;
    corner[1].x = position->x + 1.6f;
    corner[1].y = position->y + 1.0f;
    corner[1].z = position->z;
    corner[1].w = position->w;
    corner[2].x = position->x + 1.6f;
    corner[2].y = position->y - 1.0f;
    corner[2].z = position->z;
    corner[2].w = position->w;
    corner[3].x = position->x - 1.6f;
    corner[3].y = position->y - 1.0f;
    corner[3].z = position->z;
    corner[3].w = position->w;
    if (_CurRotTransPersClipScreen(&screen[0], &corner[0]) != 0) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[1], &corner[1]) != 0) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[2], &corner[2]) != 0) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[3], &corner[3]) != 0) {
        return;
    }
    st[0].x = st[3].x = (float) texelU / 512.0f;
    st[1].x = st[2].x = ((float) texelU + 128.0f) / 512.0f;
    st[0].y = st[1].y = (float) texelV / 512.0f;
    st[2].y = st[3].y = ((float) texelV + 80.0f) / 512.0f;
    stWord = &st[0].y;
    screenW = &screen[0].w;
    for (i = 3; i >= 0; i--) {
        stWord[-1] *= *screenW;
        stWord[0] *= *screenW;
        stWord += 4;
        screenW += 4;
    }
    rgba = xglFontScaleColorAlpha((long) grayLevel | ((long) grayLevel << 16 | (long) grayLevel << 8) | 0x80000000L);
    nmlPacketGsInit();
    queueCount = xglDirectQueueCount;
    record = (GifRecord *) &xglDirectQueueBuffer[queueCount];
    gifRecordsFrom = record;
    GIF_PUT(record,
            10UL | (unsigned long) texelU << 4 | (unsigned long) (texelU + 127) << 14 | (unsigned long) texelV << 24 |
                (unsigned long) (texelV + 79) << 34,
            8);
    GIF_PUT(record, 0x5D, 0);
    GIF_PUT(record, GS_RGBAQView(rgba, screen[0]), 1);
    GIF_PUT(record, GS_STView(st[0]), 2);
    GIF_PUT(record, GS_XYZ(screen[0]), 5);
    GIF_PUT(record, GS_RGBAQView(rgba, screen[1]), 1);
    GIF_PUT(record, GS_STView(st[1]), 2);
    GIF_PUT(record, GS_XYZ(screen[1]), 5);
    GIF_PUT(record, GS_RGBAQView(rgba, screen[2]), 1);
    GIF_PUT(record, GS_STView(st[2]), 2);
    GIF_PUT(record, GS_XYZ(screen[2]), 5);
    GIF_PUT(record, GS_RGBAQView(rgba, screen[3]), 1);
    GIF_PUT(record, GS_STView(st[3]), 2);
    GIF_PUT(record, GS_XYZ(screen[3]), 5);
    xglDirectQueueCount = queueCount + (((unsigned int) record - (unsigned int) gifRecordsFrom) >> 4);
    xglDirectQueueSubmitRef();
}

typedef struct FontGifRecord {
    u64 value;
    u64 address;
} FontGifRecord;

typedef union LineGifRecord {
    u64 words[2];
} LineGifRecord;

typedef struct ScreenPoint {
    int x;
    int y;
    int z;
    int w;
} ScreenPoint;

#define GS_POINT(x, y, z) ((u64) (x) | (u64) (y) << 16 | (u64) (z) << 32)
#define DRAW_LINE(points, offsetX, offsetY, rgba)                                                               \
    {                                                                                                           \
        LineGifRecord *record;                                                                                  \
        LineGifRecord *gifRecordsFrom;                                                                             \
        int queueCount;                                                                                         \
                                                                                                                \
        nmlPacketGsInit();                                                                                      \
        queueCount = xglDirectQueueCount;                                                                       \
        gifRecordsFrom = record = (LineGifRecord *) &((FontGifRecord *)xglDirectQueueBuffer)[queueCount];                             \
        GIF_PUT(record, (u64) (u32) (rgba), 1);                                                                \
        GIF_PUT(record, 0x41, 0);                                                                              \
        GIF_PUT(record, GS_POINT((points)[0].x offsetX, (points)[0].y offsetY, (points)[0].z), 5);             \
        GIF_PUT(record, GS_POINT((points)[1].x offsetX, (points)[1].y offsetY, (points)[1].z), 5);             \
        xglDirectQueueCount = queueCount + (((u32) record - (u32) gifRecordsFrom) >> 4);                           \
        xglDirectQueueSubmitRef();                                                                              \
    }

typedef union LineStep {
    int value;
    float unknownFloat;
} LineStep;

/* Draws a two-point line between clipped ends; axis-aligned ends also get fading copies every 8 units. */
void xglFontLineSegmentDraw(Vector4 *ends, u32 rgba, int stepDirMask) {
    ScreenPoint screen[2];
    LineStep stepX[2];
    LineStep stepY[2];
    int i;
    int distance;
    int alpha;

    stepX[0].value = stepX[1].value = stepY[0].value = stepY[1].value = 0;
    if (ends[0].x > ends[1].x) {
        if (stepDirMask & 2) {
            stepX[0].value = -1;
        }
        if (stepDirMask & 1) {
            stepX[1].value = 1;
        }
    }
    if (ends[0].x < ends[1].x) {
        if (stepDirMask & 1) {
            stepX[0].value = 1;
        }
        if (stepDirMask & 2) {
            stepX[1].value = -1;
        }
    }
    if (ends[0].y > ends[1].y) {
        if (stepDirMask & 8) {
            stepY[0].value = -1;
        }
        if (stepDirMask & 4) {
            stepY[1].value = 1;
        }
    }
    if (ends[0].y < ends[1].y) {
        if (stepDirMask & 4) {
            stepY[0].value = 1;
        }
        if (stepDirMask & 8) {
            stepY[1].value = -1;
        }
    }
    for (i = 0; i < 2; i++) {
        ends[i].x += stepX[i].value * 0.8f;
        ends[i].y += stepY[i].value * 0.8f;
    }
    if (_CurRotTransPersClipScreen(&screen[0], &ends[0])) {
        return;
    }
    if (_CurRotTransPersClipScreen(&screen[1], &ends[1])) {
        return;
    }
    alpha = rgba >> 24;
    DRAW_LINE(screen, , , rgba);
    rgba &= 0xFFFFFF;
    distance = 8;
    for (i = 0; i < 7; i++) {
        alpha >>= 1;
        if (ends[0].y == ends[1].y) {
            DRAW_LINE(screen, , +distance, rgba | (alpha << 24));
            DRAW_LINE(screen, , -distance, rgba | (alpha << 24));
        }
        if (ends[0].x == ends[1].x) {
            DRAW_LINE(screen, +distance, , rgba | (alpha << 24));
            DRAW_LINE(screen, -distance, , rgba | (alpha << 24));
        }
        distance += 8;
    }
}

typedef struct FontDrawRecord {
    Vector4 previousPosition; /* +0x00 */
    Vector4 anchor;           /* +0x10 */
    Vector4 previousRotation; /* +0x20 */
    Vector4 offset;           /* +0x30 */
    int slotFlags;
    int unmodeled_44;
    int unmodeled_48;
    int fixedAnchor; /* +0x4C: nonzero takes the fixed anchor (14, -16, 31) */
} FontDrawRecord;

void cameraSlotPosition(float *destination, int first, int second);
int func_002239B0(int a, int b, int c);
int colorModulateByte(int value, u8 alpha);
int xglFontScaleAlpha(int value);

/* Draws one line segment (or its mirrored pair) between two cell rows of a grid, with the row's alpha. */
void xglFontGridSegmentDraw(FontDrawRecord *record, int *from, int *to, int alpha) {
    Vector4 corners[2];
    Vector4 mirror[2];
    int stepDirMask;
    int highlight;
    int modulateLevel;
    u32 rgbaAlpha;
    int rgbaq;
    float midY;
    Vector4 *secondCorner;
    int isLastRow;

    stepDirMask = 3;
    cameraSlotPosition(&corners[0].x, from[0], to[0]);
    secondCorner = &corners[1];
    cameraSlotPosition(&secondCorner->x, from[1], to[1]);
    isLastRow = (to[1] == 7);
    highlight = func_002239B0(record->slotFlags, from[1], to[1]);
    if (!isLastRow) {
        if (to[0] == 0) {
            stepDirMask = 1;
            corners[1].y = corners[0].y;
        }
    }
    modulateLevel = (highlight != 0) ? 160 : 32;
    rgbaAlpha = (u32) xglFontScaleAlpha(colorModulateByte(modulateLevel, alpha));
    if (from[0] != 0) {
        rgbaq = (int) (((u64) rgbaAlpha << 24) | 0x7F285D);
    } else {
        rgbaq = (int) (((u64) rgbaAlpha << 24) | 0x5D317F);
    }
    if (to[1] == 7) {
        rgbaq = (int) (((u64) rgbaAlpha << 24) | 0x73665C);
    }
    if (to[0] == to[1]) {
        mirror[0] = corners[0];
        mirror[1] = *secondCorner;
        midY = (mirror[0].y + mirror[1].y) * 0.5f;
        mirror[1].y = midY;
        if (midY < mirror[0].y) {
            stepDirMask |= 8;
        } else {
            stepDirMask |= 4;
        }
        xglFontLineSegmentDraw(mirror, rgbaq, stepDirMask);
    } else {
        xglFontLineSegmentDraw(corners, rgbaq, stepDirMask);
    }
}

typedef struct FontDrawRecordView {
    Vector4 previousPosition; /* +0x00 */
    Vector4 anchor;           /* +0x10 */
    Vector4 previousRotation; /* +0x20 */
    Vector4 offset;           /* +0x30 */
    int slotFlags;
    int unmodeled_44;
    int unmodeled_48;
    int fixedAnchor; /* +0x4C: nonzero takes the fixed anchor (14, -16, 31) */
    unsigned char unmodeled_50[8];
    short unmodeled_58; /* +0x58: halfword compared against 1024 and 3072 */
} FontDrawRecordView;

void _CurMatrixSet(void *matrix);
void _CurSetViewScaleTrans(void *scale, void *trans);
s32 xglFontGetAlphaScale();
int recordSlotFlagTest(int value, int mode);
int func_00223B08(int a, int b, int c);

#define GIF_PUTView(record, registerValue, registerAddress) \
    (record)->value = (registerValue);                  \
    (record)->address = (registerAddress);              \
    (record)++

static inline int gridCellAlpha(FontDrawRecordView *record, int slot, int row, int closedAlpha) {
    int alpha = 0x80;

    if (row == 7) {
        if (recordSlotFlagTest(record->slotFlags, slot)) {
            alpha = record->unmodeled_58;
            if (slot == record->unmodeled_44) {
                if (alpha < 0x400) {
                    alpha = closedAlpha;
                } else if (alpha < 0xC00) {
                    alpha = (0x1400 - alpha) >> 4;
                } else {
                    alpha = 0x80;
                }
            } else {
                alpha = 0x80;
            }
        } else {
            alpha = closedAlpha;
        }
    }
    return alpha;
}

void xglFontGridCellsDraw(FontDrawRecordView *record, int pass);

/* Draws the bordered cell grid of a font draw record for one pass: VU view setup, GIF packets, then the cell quads. */
void xglFontGridCellsDraw(FontDrawRecordView *record, int pass) {
    FontGifRecord *packet;
    FontGifRecord *packetsFrom;
    XglCamera *camera;
    int from[2];
    int to[2];
    Vector4 position;
    int slotPairRound;
    int slot;
    int row;
    int rowCount;
    int alpha;
    int cellIndex;
    int fontAlpha;

    camera = xglStudioGetCamera2(1);
    _CurMatrixSet((u8 *) camera + 0x470);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
    nmlPacketGsInit();
    packetsFrom = packet = &((FontGifRecord *) xglDirectQueueBuffer)[xglDirectQueueCount];
    GIF_PUTView(packet, 0x30000, 0x47);
    GIF_PUTView(packet, (u64) D_004C8390.zBufferPage | 0x101000000, 0x4E);
    GIF_PUTView(packet,
                   (u64) (0x24020000 | D_004C8390.texturePage << 5) |
                       (u64) 0xC800 << 19,
                   6);
    GIF_PUTView(packet, 0x60, 0x14);
    fontAlpha = xglFontGetAlphaScale() & 0xFF;
    GIF_PUTView(packet, (u64) (u32) fontAlpha << 32 | 0x48, 0x42);
    xglDirectQueueCount += ((u32) packet - (u32) packetsFrom) >> 4;
    xglDirectQueueSubmitRef();
    if (!pass) {
        to[0] = to[1] = 1;
        from[0] = 0;
        from[1] = 1;
        slot = 0;
        xglFontGridSegmentDraw(record, from, to, 0x80);
        from[0] = 1;
        from[1] = 0;
        xglFontGridSegmentDraw(record, from, to, 0x80);
        for (slotPairRound = 0; slotPairRound < 2; slotPairRound++, slot++) {
            from[0] = from[1] = slot;
            for (row = 2, rowCount = slotKindFromMode(record->slotFlags, slot); row < rowCount;
                 row++) {
                to[0] = row - 1;
                to[1] = row;
                alpha = gridCellAlpha(record, slot, row, 0x20);
                xglFontGridSegmentDraw(record, from, to, alpha);
            }
            if (!(slot & 1)) {
                to[0] = 0;
                to[1] = 1;
                xglFontGridSegmentDraw(record, from, to, 0x80);
            }
        }
    }
    {
        FontGifRecord *cellPacket;
        FontGifRecord *cellPacketsFrom;

        nmlPacketGsInit();
        slot = pass ? 2 : 0;
        cellPacketsFrom = cellPacket = &((FontGifRecord *) xglDirectQueueBuffer)[xglDirectQueueCount];
        fontAlpha = xglFontGetAlphaScale() & 0xFF;
        GIF_PUTView(cellPacket, (u64) (u32) fontAlpha << 32 | 0x68, 0x42);
        xglDirectQueueCount += ((u32) cellPacket - (u32) cellPacketsFrom) >> 4;
        xglDirectQueueSubmitRef();
    }
    cellIndex = 0;
    for (slotPairRound = 0; slotPairRound < 2; slotPairRound++, slot++) {
        for (row = 0, rowCount = slotKindFromMode(record->slotFlags, slot); row < rowCount;
             row++, cellIndex++) {
            int isOuterSlot = slot < 2;

            if (row >= 7 && !isOuterSlot) {
                continue;
            }
            if (row == 0 && slot > 0) {
                continue;
            }
            cameraSlotPosition(&position.x, slot, row);
            alpha = gridCellAlpha(record, slot, row, 0x14);
            if (func_00223B08(record->slotFlags, slot, row)) {
                xglFontGlyphQuadDraw(&position.x, (cellIndex & 3) * 128, (cellIndex >> 2) * 80, alpha);
            }
        }
    }
}

void xglVectorLength(float *destination, const Vector4 *vector);
extern DoublewordAlignedVector4 gDefaultSpringOffset;

/* One spring step of a point toward its anchor: the scaled offset to the anchor, then a 0.998 Verlet update from previous. */
void xglFontSpringStep(Vector4 *point, Vector4 *previous, const Vector4 *anchor, float scale) {
    DoublewordAlignedVector4 toAnchor;
    float length;

    Vu0SubVectorXYZ(&toAnchor, anchor, point);
    xglVectorLength(&length, &toAnchor);
    if (length > 0.002f) {
        toAnchor.x *= scale;
        toAnchor.y *= scale;
        toAnchor.z *= scale;
    } else {
        toAnchor = gDefaultSpringOffset;
    }
    point->x += toAnchor.x;
    point->y += toAnchor.y;
    point->z += toAnchor.z;
    point->x += (point->x - previous->x) * 0.998f;
    point->y += (point->y - previous->y) * 0.998f;
    point->z += (point->z - previous->z) * 0.998f;
    *previous = *point;
}

void xglFontDrawRecordAnchorUpdate(FontDrawRecord *record);

/* Sets the record's anchor (fixed, or sampled from its cell) and steps the camera travel position and rotation toward it. */
void xglFontDrawRecordAnchorUpdate(FontDrawRecord *record) {
    XglCamera *camera;
    float sample[2];
    float scale;

    camera = xglStudioGetCamera2(1);
    if (record->fixedAnchor != 0) {
        record->anchor.x = 14.0f;
        record->anchor.y = -16.0f;
        do { /* kept: this empty do/while(0) decides the emitted store order */
        } while (0);
        record->anchor.z = 31.0f;
        record->offset.x = 0.5f;
    } else {
        cameraSlotPosition(sample, record->unmodeled_44, record->unmodeled_48);
        record->anchor.x = sample[0];
        record->anchor.y = sample[1] - 1.0f;
        record->anchor.z = 16.0f;
        record->offset.x = 0.0f;
    }
    record->offset.y = 0.0f;
    record->offset.z = 0.0f;
    do { /* kept: this empty do/while(0) decides the emitted store order */
    } while (0);
    scale = 0.03f;
    xglFontSpringStep(&camera->travel.position, &record->previousPosition, &record->anchor, scale);
    xglFontSpringStep(&camera->travel.rotation, &record->previousRotation, &record->offset, scale);
}

void fontQueueSpriteBlack(void);

/* Runs the two passes of a font draw object, keeping the alpha scale saved around the cell grid draws. */
void xglFontDrawObjectPasses(s32 drawObjectHandle) {
    s32 savedAlphaScale;

    savedAlphaScale = xglFontGetAlphaScale();
    xglFontDrawRecordAnchorUpdate(drawObjectHandle);
    fontObjectCellsDraw(drawObjectHandle, 0);
    xglFontSetAlphaScale(savedAlphaScale);
    xglFontGridCellsDraw(drawObjectHandle, 0);
    fontObjectCellsDraw(drawObjectHandle, 1);
    xglFontSetAlphaScale(savedAlphaScale);
    xglFontGridCellsDraw(drawObjectHandle, 1);
    fontQueueSpriteBlack();
}
