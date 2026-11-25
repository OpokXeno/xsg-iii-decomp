#include "common.h"
#include "main/Packet.h"
#include "gs.h"
#include "main/tu007.h"
#include "main/xgl_2.h"

#include "quadword.h"
#include "vu0_macro.h"
typedef int s32;

typedef long long s64;

typedef struct DrawEnvRecord {
    s32 workBase;
    s32 frameBase;
    s32 red;
    s32 green;
    s32 blue;
    s32 blendFix;   /* +0x14: FIX value of GS ALPHA */
    s32 blendMode;   /* +0x18: 0 to 7, picks the GS ALPHA equation */
    s32 sourceRect[4];
    s32 destinationRect[4];
    s32 blockWidth;
    s32 blockHeight;
    s32 unk44;
    s32 unk48;
    s32 unk4C;
    s64 unk50;
    s64 unk58;
    s64 unk60;
    union {
        u64 all;
        s32 pageOffset;
        struct {
            s32 z;
            s32 bits;
        } word;
    } flags; /* +0x68 */
} DrawEnvRecord;

/* The draw-environment drawRecord pool (struct completed below with the xgl filter code). */
static struct DrawEnvRecord gDrawArea;
void drawAreaSetWindow(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

struct DrawEnvRecord *drawEnvGetRecord(void) {
    return &gDrawArea;
}

#define DRAW_FLAG_EDGES (0xFULL << 32)       /* bits 32..35: redraw an edge line */
#define DRAW_FLAG_EDGE(edge) (1ULL << (32 + (edge)))
#define DRAW_FLAG_INSETS (0xFULL << 40)      /* bits 40..43: shrink the source rectangle */
#define DRAW_FLAG_INSET(edge) (1ULL << (40 + (edge)))
#define DRAW_FLAG_COLOR (1ULL << 48)         /* use the record's own colour */
/* Origin added to every 12.4 vertex coordinate (X2 uses the same values for its gauge origin; a lead, not X3 proof). */
#define DRAW_VERTEX_ORIGIN_X 0x6FF8
#define DRAW_VERTEX_ORIGIN_Y 0x71F7

void xglPacketBufferInit(s32 spr_address);
void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3);
void xglPacketAppendQuad(s64 highDoubleword, s64 lowDoubleword);
void xglPacketFinishTag(void);
void xglPacketSendDirect(s32 address, s32 qwordCount);
DrawEnvRecord *drawEnvGetRecord(void);
void drawEnvSendRecordPacket(void);

/* Sends the draw environment packet of drawRecord: GS state, the work texture sprite and, when flagged,
 * its edge rectangles (DRAW_FLAG_EDGES) or inset source rectangle (DRAW_FLAG_INSETS). */
void drawEnvSendRecordPacket(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, drawRecord->unk50);
    xglPacketAppendQuad(GS_TEST_1, drawRecord->unk58);
    xglPacketAppendQuad(GS_CLAMP_1, drawRecord->unk60);
    xglPacketAppendQuad(GS_FRAME_1, (s64) drawRecord->frameBase | ((s64) drawRecord->blockHeight << 16) |
                                  ((s64) drawRecord->unk48 << 24) | ((s64) 0xFF000000U << 32));
    switch (drawRecord->blendMode) {
    case 5:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x58 | ((s64) drawRecord->blendFix << 32));
        break;
    case 6:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x52 | ((s64) drawRecord->blendFix << 32));
        break;
    case 7:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x98 | ((s64) drawRecord->blendFix << 32));
        break;
    case 4:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x54 | ((s64) drawRecord->blendFix << 32));
        break;
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x68 | ((s64) drawRecord->blendFix << 32));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x62 | ((s64) drawRecord->blendFix << 32));
        break;
    case 3:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0xA8 | ((s64) drawRecord->blendFix << 32));
        break;
    case 0:
    default:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x64 | ((s64) drawRecord->blendFix << 32));
        break;
    }
    xglPacketAppendQuad(GS_TEX0_1, (s64) (drawRecord->workBase * 32) | ((s64) drawRecord->blockWidth << 14) |
                               ((s64) drawRecord->unk44 << 20) | ((s64) 10 << 26) | ((s64) 9 << 30) |
                               ((s64) 1 << 34));
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    if (drawRecord->flags.all & DRAW_FLAG_COLOR) {
        xglPacketAppendQuad(GS_RGBAQ, (s64) drawRecord->red | ((s64) drawRecord->green << 8) | ((s64) drawRecord->blue << 16) |
                                   ((s64) 0x80 << 24) | ((s64) 0x3F800000 << 32));
    } else {
        xglPacketAppendQuad(GS_RGBAQ, (s64) 0x80 | ((s64) 0x80 << 8) | ((s64) 0x80 << 16) | ((s64) 0x80 << 24) |
                                   ((s64) 0x3F800000 << 32));
    }
    if (drawRecord->flags.all & DRAW_FLAG_EDGES) {
        DrawEnvRecord original = *drawRecord;

        if (drawRecord->flags.all & DRAW_FLAG_EDGE(0)) {
            drawRecord->destinationRect[1]++;
            xglPacketAppendQuad(GS_PRIM, 0x152);
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[0] * 16) |
                                       ((s64) (original.sourceRect[1] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[0] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[1] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[2] * 16) |
                                       ((s64) (original.sourceRect[1] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[2] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[1] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
        }
        if (drawRecord->flags.all & DRAW_FLAG_EDGE(1)) {
            drawRecord->destinationRect[3]--;
            xglPacketAppendQuad(GS_PRIM, 0x152);
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[0] * 16) |
                                       ((s64) (original.sourceRect[3] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[0] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[3] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[2] * 16) |
                                       ((s64) (original.sourceRect[3] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[2] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[3] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
        }
        if (drawRecord->flags.all & DRAW_FLAG_EDGE(2)) {
            drawRecord->destinationRect[0]++;
            xglPacketAppendQuad(GS_PRIM, 0x152);
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[0] * 16) |
                                       ((s64) (original.sourceRect[1] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[0] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[1] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[0] * 16) |
                                       ((s64) (original.sourceRect[3] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[0] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[3] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
        }
        if (drawRecord->flags.all & DRAW_FLAG_EDGE(3)) {
            drawRecord->destinationRect[2]--;
            xglPacketAppendQuad(GS_PRIM, 0x152);
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[2] * 16) |
                                       ((s64) (original.sourceRect[1] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[2] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[1] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
            xglPacketAppendQuad(GS_UV, (s64) (original.sourceRect[2] * 16) |
                                       ((s64) (original.sourceRect[3] * 16) << 16));
            xglPacketAppendQuad(GS_XYZF2, (s64) (original.destinationRect[2] * 16 + DRAW_VERTEX_ORIGIN_X) |
                                       ((s64) (original.destinationRect[3] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                                       ((s64) original.flags.word.z << 32));
        }
    } else if (drawRecord->flags.all & DRAW_FLAG_INSETS) {
        if (drawRecord->flags.all & DRAW_FLAG_INSET(0)) {
            drawRecord->sourceRect[1]++;
        }
        if (drawRecord->flags.all & DRAW_FLAG_INSET(1)) {
            drawRecord->sourceRect[3]--;
        }
        if (drawRecord->flags.all & DRAW_FLAG_INSET(2)) {
            drawRecord->sourceRect[0]++;
        }
        if (drawRecord->flags.all & DRAW_FLAG_INSET(3)) {
            drawRecord->sourceRect[2]--;
        }
    }
    xglPacketAppendQuad(GS_PRIM, 0x156);
    xglPacketAppendQuad(GS_UV, (s64) (drawRecord->sourceRect[0] * 16) | ((s64) (drawRecord->sourceRect[1] * 16) << 16));
    xglPacketAppendQuad(GS_XYZF2, (s64) (drawRecord->destinationRect[0] * 16 + DRAW_VERTEX_ORIGIN_X) |
                               ((s64) (drawRecord->destinationRect[1] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                               ((s64) drawRecord->flags.word.z << 32));
    xglPacketAppendQuad(GS_UV, (s64) (drawRecord->sourceRect[2] * 16) | ((s64) (drawRecord->sourceRect[3] * 16) << 16));
    xglPacketAppendQuad(GS_XYZF2, (s64) (drawRecord->destinationRect[2] * 16 + DRAW_VERTEX_ORIGIN_X) |
                               ((s64) (drawRecord->destinationRect[3] * 16 + DRAW_VERTEX_ORIGIN_Y) << 16) |
                               ((s64) drawRecord->flags.word.z << 32));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
    drawRecord->flags.word.bits = 0;
}

typedef struct XglDrawBuffers {
    u8 unk00[8];
    u16 depthBase; /* +0x08: ZBUF page */
    u16 moviePage; /* +0x0A: page of the movie frame buffer */
    u16 unk0C;
    u16 workBase;    /* +0x0E: page of the work texture */
    u16 unk10;       /* +0x10 */
    u16 frameBase;   /* +0x12: page of the frame being drawn */
    u16 unk14;       /* +0x14 */
    u16 displayBase; /* +0x16: page of the displayed frame */
    u16 largeBase;   /* +0x18 */
    u8 unmodeled_1A[0x38 - 0x1A];
    void (*sendFrame)(s32 packet); /* +0x38: slotSendFrame */
    u8 unmodeled_3C[0x64 - 0x3C];
    void (*setFixedValue)(s32 fixedValue); /* +0x64: slotSetFixedValue */
} XglDrawBuffers;

extern XglDrawBuffers D_004C8390;
void func_001D5398();

/* Draw area: work page = display page, frame page = work page, 8x8 blocks, no edge or inset flags. */
void func_001D5398(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.displayBase;
    drawRecord->frameBase = D_004C8390.workBase;
    drawRecord->blockWidth = 8;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawEnvSendRecordPacket();
}

void func_001D53E0();

/* Draw area for the half draw: work = display, frame = unk10 page, 8x4 blocks, edge flags. */
void func_001D53E0(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.displayBase;
    drawRecord->frameBase = D_004C8390.unk10;
    drawRecord->blockWidth = 8;
    drawRecord->blockHeight = 4;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_EDGES;
    drawEnvSendRecordPacket();
}

s32 xglRenderMovieIsActive();
void drawEnvSetFieldArea(void);

/* Draw area for the field: a movie frame uses the movie page (10 wide), otherwise the large page. */
void drawEnvSetFieldArea(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    if (xglRenderMovieIsActive()) {
        drawRecord->blockWidth = 10;
        drawRecord->unk44 = 2;
        drawRecord->workBase = D_004C8390.moviePage;
    } else {
        drawRecord->blockWidth = 8;
        drawRecord->unk44 = 0;
        drawRecord->workBase = D_004C8390.largeBase;
    }
    drawRecord->blockHeight = 8;
    drawRecord->unk48 = 0;
    drawRecord->frameBase = D_004C8390.workBase;
    drawEnvSendRecordPacket();
}

void func_001D54C8();

/* Draw area: work = work page, frame = frame page, 8x4 blocks, edge flags. */
void func_001D54C8(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.workBase;
    drawRecord->frameBase = D_004C8390.frameBase;
    drawRecord->blockWidth = 8;
    drawRecord->blockHeight = 4;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_EDGES;
    drawEnvSendRecordPacket();
}

void func_001D5528();

/* Draw area for the last half draw: work = work, frame = unk14 page, 8x4, edge flags. */
void func_001D5528(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.workBase;
    drawRecord->frameBase = D_004C8390.unk14;
    drawRecord->blockWidth = 8;
    drawRecord->blockHeight = 4;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_EDGES;
    drawEnvSendRecordPacket();
}

/* Draw area: work = work page, frame = unk10 page, 8x4 blocks, edge flags. */
void func_001D5588(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.workBase;
    drawRecord->frameBase = D_004C8390.unk10;
    drawRecord->blockWidth = 8;
    drawRecord->blockHeight = 4;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_EDGES;
    drawEnvSendRecordPacket();
}

/* Draw area: work = work page, frame = display page, 8x8 blocks, no flags. */
void func_001D55E8(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.workBase;
    drawRecord->frameBase = D_004C8390.displayBase;
    drawRecord->blockWidth = 8;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawEnvSendRecordPacket();
}

/* Draw area: work = unk14 page, frame = frame page, 4x4 blocks, no flags. */
void func_001D5630(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.unk14;
    drawRecord->frameBase = D_004C8390.frameBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 4;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawEnvSendRecordPacket();
}

/* Draw area: work = unk14 page, frame = display page, 4x8 blocks, inset flags. */
void func_001D5678(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.unk14;
    drawRecord->frameBase = D_004C8390.displayBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_INSETS;
    drawEnvSendRecordPacket();
}

void func_001D56D8();

/* Draw area: work = unk10 page, frame = display page, 4x8 blocks, inset flags. */
void func_001D56D8(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.unk10;
    drawRecord->frameBase = D_004C8390.displayBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_INSETS;
    drawEnvSendRecordPacket();
}

void func_001D5738();

/* Draw area: work = unk10 page, frame = work page, 4x8 blocks, inset flags. */
void func_001D5738(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.unk10;
    drawRecord->frameBase = D_004C8390.workBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_INSETS;
    drawEnvSendRecordPacket();
}

/* Draw area: work = unk10 page, frame = frame page, 4x4 blocks, no flags. */
void func_001D5798(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.unk10;
    drawRecord->frameBase = D_004C8390.frameBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 4;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawEnvSendRecordPacket();
}

/* Draw area: work = frame page, frame = work page, 4x8 blocks, inset flags. */
void func_001D57E0(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.frameBase;
    drawRecord->frameBase = D_004C8390.workBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_INSETS;
    drawEnvSendRecordPacket();
}

/* Draw area: work = frame page, frame = display page, 4x8 blocks, inset flags. */
void func_001D5840(void) {
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = D_004C8390.frameBase;
    drawRecord->frameBase = D_004C8390.displayBase;
    drawRecord->blockWidth = 4;
    drawRecord->blockHeight = 8;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->flags.all |= DRAW_FLAG_INSETS;
    drawEnvSendRecordPacket();
}

void drawEnvSetLarge(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x200, 0x1C0);
    func_001D5398();
}

void drawEnvSetHalfDraw(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D53E0();
}

void drawEnvSetField(void) {
    if (xglRenderMovieIsActive() != 0) {
        drawAreaSetWindow(0x80, 3, 0, 0, 0x280, 0x1BF, 0, 0, 0x200, 0x1BF);
    } else {
        drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1BF, 0, 0, 0x200, 0x1BF);
    }
    drawEnvSetFieldArea();
}

void drawEnvSetHalfDisplay(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D56D8();
}

void drawEnvSetHalfDisplayNext(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D5738();
}

void drawEnvSetHalfDrawNext(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D54C8();
}

void drawEnvSetHalfDrawLast(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D5528();
}

void drawEnvSetHalfDrawRepeat(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D5528();
}

/* Draws a flat-coloured rectangle (startX,startY)-(endX,endY) on a draw page with the given alpha. */
void drawRectOnPage(s32 pageId, s32 alpha, s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 width;

    width = 0x200;
    if (pageId == D_004C8390.frameBase || pageId == D_004C8390.unk14 || pageId == D_004C8390.unk10) {
        width = 0x100;
    }
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, 0x30000);
    xglPacketAppendQuad(GS_ZBUF_1, D_004C8390.depthBase | (0x9880LL << 17));
    xglPacketAppendQuad(GS_FRAME_1, pageId | ((s64) width << 10) | 0x00FFFFFF00000000ULL);
    xglPacketAppendQuad(GS_RGBAQ, ((s64) alpha << 24) | 0x3F80000000000000ULL);
    xglPacketAppendQuad(GS_PRIM, 6);
    {
        s32 xFixed = (startX << 4) + DRAW_VERTEX_ORIGIN_X;
        s32 yFixed = (startY << 4) + DRAW_VERTEX_ORIGIN_Y;

        xglPacketAppendQuad(GS_XYZF2, (s64) xFixed | ((s64) yFixed << 16));
    }
    {
        s32 xFixed = (endX << 4) + DRAW_VERTEX_ORIGIN_X;
        s32 yFixed = (endY << 4) + DRAW_VERTEX_ORIGIN_Y;

        xglPacketAppendQuad(GS_XYZF2, (s64) xFixed | ((s64) yFixed << 16));
    }
    xglPacketAppendQuad(GS_FRAME_1, (s64) (0x80000 | pageId) | 0xFF00000000000000ULL);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Draws the rectangle (startX,startY)-(endX,endY) on a draw page; the frame word is the page with its width. */
void func_001D5CD0(s32 pageId, s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 width;

    width = 0x200;
    if (pageId == D_004C8390.frameBase || pageId == D_004C8390.unk14 || pageId == D_004C8390.unk10) {
        width = 0x100;
    }
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, 0x30000);
    xglPacketAppendQuad(GS_ZBUF_1, D_004C8390.depthBase | (0x9880ULL << 17));
    xglPacketAppendQuad(GS_FRAME_1, pageId | ((s64) width << 10) | 0x00FFFFFF00000000ULL);
    xglPacketAppendQuad(GS_RGBAQ, 0xFE00ULL << 46);
    xglPacketAppendQuad(GS_PRIM, 6);
    xglPacketAppendQuad(GS_XYZF2, ((startX << 4) + DRAW_VERTEX_ORIGIN_X) | ((s64) ((startY << 4) + DRAW_VERTEX_ORIGIN_Y) << 16));
    xglPacketAppendQuad(GS_XYZF2, ((endX << 4) + DRAW_VERTEX_ORIGIN_X) | ((s64) ((endY << 4) + DRAW_VERTEX_ORIGIN_Y) << 16));
    xglPacketAppendQuad(GS_FRAME_1, (pageId | 0x80000) | (0xFF00ULL << 48));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Same packet as func_001D5CD0 with the FBA register set to 1 before the rectangle and 0 after it. */
void func_001D5E60(s32 pageId, s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 width;

    width = 0x200;
    if (pageId == D_004C8390.frameBase || pageId == D_004C8390.unk14 || pageId == D_004C8390.unk10) {
        width = 0x100;
    }
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_FBA_1, 1);
    xglPacketAppendQuad(GS_TEST_1, 0x30000);
    xglPacketAppendQuad(GS_ZBUF_1, D_004C8390.depthBase | (0x9880ULL << 17));
    xglPacketAppendQuad(GS_FRAME_1, pageId | ((s64) width << 10) | 0x00FFFFFF00000000ULL);
    xglPacketAppendQuad(GS_RGBAQ, 0xFE00ULL << 46);
    xglPacketAppendQuad(GS_PRIM, 6);
    xglPacketAppendQuad(GS_XYZF2, ((startX << 4) + DRAW_VERTEX_ORIGIN_X) | ((s64) ((startY << 4) + DRAW_VERTEX_ORIGIN_Y) << 16));
    xglPacketAppendQuad(GS_XYZF2, ((endX << 4) + DRAW_VERTEX_ORIGIN_X) | ((s64) ((endY << 4) + DRAW_VERTEX_ORIGIN_Y) << 16));
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_FRAME_1, (pageId | 0x80000) | (0xFF00ULL << 48));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Draws the rectangle (startX,startY)-(endX,endY) at depth z on a draw page. */
void drawRectOnPageDepth(s32 pageId, s32 depth, s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 width;
    s64 depthBits;

    width = 0x200;
    if (pageId == D_004C8390.frameBase || pageId == D_004C8390.unk14 || pageId == D_004C8390.unk10) {
        width = 0x100;
    }
    depthBits = (s64) depth << 32;
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_FBA_1, 1);
    xglPacketAppendQuad(GS_TEST_1, 0x70000);
    xglPacketAppendQuad(GS_ZBUF_1, D_004C8390.depthBase | (0x9880LL << 17));
    xglPacketAppendQuad(GS_FRAME_1, pageId | ((s64) width << 10) | 0x00FFFFFF00000000ULL);
    xglPacketAppendQuad(GS_RGBAQ, 0x3F80000000000000ULL);
    xglPacketAppendQuad(GS_PRIM, 6);
    {
        s32 xFixed = (startX << 4) + DRAW_VERTEX_ORIGIN_X;
        s32 yFixed = (startY << 4) + DRAW_VERTEX_ORIGIN_Y;

        xglPacketAppendQuad(GS_XYZF2, (s64) xFixed | ((s64) yFixed << 16) | depthBits);
    }
    {
        s32 xFixed = (endX << 4) + DRAW_VERTEX_ORIGIN_X;
        s32 yFixed = (endY << 4) + DRAW_VERTEX_ORIGIN_Y;

        xglPacketAppendQuad(GS_XYZF2, (s64) xFixed | ((s64) yFixed << 16) | depthBits);
    }
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_FRAME_1, (s64) (0x80000 | pageId) | 0xFF00000000000000ULL);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Draws the rectangle (startX,startY)-(endX,endY) filled with colour (0xRRGGBB) on a draw page. */
void drawRectOnPageColour(s32 pageId, u32 color, s32 startX, s32 startY, s32 endX, s32 endY) {
    s32 width;
    s32 red;
    s32 green;
    s32 blue;

    red = (color >> 16) & 0xFF;
    green = (color >> 8) & 0xFF;
    blue = color & 0xFF;
    width = 0x200;
    if (pageId == D_004C8390.frameBase || pageId == D_004C8390.unk14 || pageId == D_004C8390.unk10) {
        width = 0x100;
    }
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, 0x30000);
    xglPacketAppendQuad(GS_ZBUF_1, D_004C8390.depthBase | (0x9880LL << 17));
    xglPacketAppendQuad(GS_FRAME_1, pageId | ((s64) width << 10) | 0xFF00000000000000ULL);
    xglPacketAppendQuad(GS_RGBAQ, ((s64) blue << 16) | ((s64) green << 8) | red | 0x3F80000000000000ULL);
    xglPacketAppendQuad(GS_PRIM, 6);
    {
        s32 xFixed = (startX << 4) + DRAW_VERTEX_ORIGIN_X;
        s32 yFixed = (startY << 4) + DRAW_VERTEX_ORIGIN_Y;

        xglPacketAppendQuad(GS_XYZF2, (s64) xFixed | ((s64) yFixed << 16));
    }
    {
        s32 xFixed = (endX << 4) + DRAW_VERTEX_ORIGIN_X;
        s32 yFixed = (endY << 4) + DRAW_VERTEX_ORIGIN_Y;

        xglPacketAppendQuad(GS_XYZF2, (s64) xFixed | ((s64) yFixed << 16));
    }
    xglPacketAppendQuad(GS_FRAME_1, (s64) (0x80000 | pageId) | 0xFF00000000000000ULL);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

extern s32 gPacketBufferBase;
extern s32 gPacketEntryCount;
extern s32 gTagQuadCount;

typedef struct PacketEntry {
    s32 word0;
    s32 word1;
    s32 word2;
    s32 word3;
} PacketEntry;

typedef struct XglObject {
    s32 unk00;
    s32 depth;       /* +0x04: z of every vertex */
    s32 centerX;     /* +0x08 */
    s32 centerY;     /* +0x0C */
    s32 wavelength;  /* +0x10 */
    s32 amplitude;   /* +0x14 */
    s32 extentX;     /* +0x18 */
    s32 extentY;     /* +0x1C */
    s32 phaseStep;   /* +0x20 */
    s32 blendAmount; /* +0x24: fixed alpha */
    s32 blendMode;   /* +0x28 */
    s32 halfHeight;  /* +0x2C: draws to the half-height page */
    u8 unk30[0x0C];
    s32 modeId; /* +0x3C: 1 displaces along the radius, 2 along both axes */
    s32 phaseX; /* +0x40 */
    s32 phaseY; /* +0x44 */
    u8 unk48[0x08];
    s32 flags; /* +0x50: either of bits 3 and 4 stops the phase */
} XglObject;

/* Starts an empty packet at spr_address (the scratchpad buffer). */
void xglPacketBufferInit(s32 spr_address)
{
    extern s32 gPacketCursor;

    gPacketCursor = spr_address;
    gPacketEntryCount = 0;
    gPacketBufferBase = spr_address;
}

void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3) {
    extern void *gPacketCursor;
    extern void *gPacketLastTag;
    s32 *packetCursor;

    packetCursor = gPacketCursor;
    packetCursor[0] = word0;
    packetCursor[1] = word1;
    packetCursor[2] = word2;
    packetCursor[3] = word3;
    gPacketCursor = packetCursor + 4;
    gPacketLastTag = packetCursor;
    gTagQuadCount = 0;
    gPacketEntryCount += 1;
}

/* Appends one quadword of four integer words to the packet and counts it in the open tag. */
void xglPacketAppendWordQuad(s32 word0, s32 word1, s32 word2, s32 word3)
{
    extern s32 *gPacketCursor;
    s32 *packetCursor;

    packetCursor = gPacketCursor;
    packetCursor[0] = word0;
    packetCursor[1] = word1;
    packetCursor[2] = word2;
    packetCursor[3] = word3;
    gPacketCursor = packetCursor + 4;
    gPacketEntryCount += 1;
    gTagQuadCount += 1;
}

/* Appends one quadword of four floats to the packet and counts it in the open tag. */
void xglPacketAppendFloatQuad(f32 x, f32 y, f32 z, f32 w)
{
    extern f32 *gPacketCursor;
    f32 *packetCursor;
    s32 entryCountAfter;
    s32 nextTagQuadCount;

    packetCursor = gPacketCursor;
    entryCountAfter = gPacketEntryCount + 1;
    nextTagQuadCount = gTagQuadCount + 1;
    packetCursor[0] = x;
    packetCursor[1] = y;
    packetCursor[2] = z;
    packetCursor[3] = w;
    gPacketCursor = packetCursor + 4;
    gPacketEntryCount = entryCountAfter;
    gTagQuadCount = nextTagQuadCount;
}

/* Appends one GIF A+D quadword: the data in the low doubleword, the register in the high one. */
void xglPacketAppendQuad(s64 highDoubleword, s64 lowDoubleword)
{
    extern void *gPacketCursor;
    s64 *packetCursor = gPacketCursor;
    s32 oldPacketEntryCount;
    s32 nextPacketEntryCount;
    s32 nextTagQuadCount;
    s32 tagQuadCountStore;
    s64 *nextPacketCursor;

    oldPacketEntryCount = gPacketEntryCount;
    nextTagQuadCount = gTagQuadCount + 1;
    nextPacketCursor = packetCursor + 2;
    nextPacketEntryCount = oldPacketEntryCount + 1;
    packetCursor[0] = lowDoubleword;
    tagQuadCountStore = nextTagQuadCount;
    /* kept: the do-while(0) block decides the emitted order of the tail stores. */
    do {
        packetCursor[1] = highDoubleword;
        gPacketCursor = nextPacketCursor;
        gPacketEntryCount = nextPacketEntryCount;
        gTagQuadCount = tagQuadCountStore;
    } while (0);
}

void xglPacketFinishTag(void) {
    extern s32 *gPacketLastTag;
    *gPacketLastTag |= gTagQuadCount;
}

void sceVif1PkAddDirectDataN(XglPacket *packet, s32 address, s32 qwordCount);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkOpenDirectHLCode(XglPacket *packet, unsigned int count);
XglPacket *xglPacketGetCurrent();

void xglPacketSendDirect(s32 address, s32 qwordCount) {
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, (address != 0) ? address : gPacketBufferBase,
                            (qwordCount != 0) ? qwordCount : gPacketEntryCount);
    sceVif1PkCloseDirectHLCode(packet);
}

s32 xglPacketGetEntryCount(void) {
    return gPacketEntryCount;
}

s32 xglPacketGetQuadCount(void) {
    return gTagQuadCount;
}

void xglPacketSetEntryWords(PacketEntry *packetEntry, s32 word0, s32 word1, s32 word2, s32 word3) {
    packetEntry->word0 = word0;
    packetEntry->word1 = word1;
    packetEntry->word2 = word2;
    packetEntry->word3 = word3;
}

void xglPacketSetEntryQuad(s64 *packetEntry, s64 highDoubleword, s64 lowDoubleword) {
    packetEntry[0] = lowDoubleword;
    packetEntry[1] = highDoubleword;
}

/* Converts hue (degrees), saturation and value (0-255) to RGB (0-255): the standard HSV sector formula. */
void colorHsvToRgb(s32 hue, u8 saturation, u8 maxChannel, s32 *red, s32 *green, s32 *blue)
{
    s32 angle;
    s32 sector;
    f32 fraction;
    s32 minChannel;
    s32 fallingChannel;
    s32 risingChannel;

    angle = hue % 360;
    sector = angle / 60;
    fraction = (f32)angle / 60.0f - (f32)sector;
    minChannel = maxChannel * (255 - saturation);
    minChannel >>= 8;
    fallingChannel = maxChannel * (255.0f - saturation * fraction);
    fallingChannel >>= 8;
    risingChannel = maxChannel * (255.0f - saturation * (1.0f - fraction));
    risingChannel >>= 8;
    switch (sector) {
    case 0:
        *red = maxChannel;
        *green = risingChannel;
        *blue = minChannel;
        break;
    case 1:
        *red = fallingChannel;
        *green = maxChannel;
        *blue = minChannel;
        break;
    case 2:
        *red = minChannel;
        *green = maxChannel;
        *blue = risingChannel;
        break;
    case 3:
        *red = minChannel;
        *green = fallingChannel;
        *blue = maxChannel;
        break;
    case 4:
        *red = risingChannel;
        *green = minChannel;
        *blue = maxChannel;
        break;
    case 5:
        *red = maxChannel;
        *green = minChannel;
        *blue = fallingChannel;
        break;
    }
}

void xglPacketSendFixedHeader(void)
{
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 0xE, 0);
    xglPacketAppendQuad(GS_SCISSOR_1, (((0x01BF0000L << 0x10) | 0x1FF) << 0x10));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

XglCamera *xglStudioSelectGetActiveCamera(int studioIndex);

/* Sends GS_SCISSOR_1 for the active camera window: startX = left, startY = top, endX = right - 1, endY = bottom - 1. */
void drawEnvSetCameraScissor(void) {
    XglCamera *camera;
    s32 bottom;
    s32 right;
    s32 top;
    s32 left;

    camera = xglStudioSelectGetActiveCamera(0);
    bottom = camera->screen.window.component[3];
    left = camera->screen.window.component[0];
    right = camera->screen.window.component[2];
    top = camera->screen.window.component[1];
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_SCISSOR_1, ((s64) (bottom - 1) << 48) | ((s64) (right - 1) << 16) | ((s64) top << 32) | left);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

extern f32 D_00550FFC;

void xglSetOffsetValue(f32 offsetAmount) {
    D_00550FFC = offsetAmount;
}

f32 func_001AEE50();
f32 func_001AEF08();

s32 xglIsOffsetWithinLimits(void) {
    f32 x_limit;
    f32 y_limit;
    f32 remaining;
    s32 x_in_range;
    s32 y_in_range;

    x_limit = func_001AEE50();
    y_limit = func_001AEF08();
    y_in_range = 1;
    remaining = 85.0f - D_00550FFC;
    x_in_range = 1;
    if (!(remaining < x_limit)) {
        x_in_range = 0;
    }
    if (!(remaining < y_limit)) {
        y_in_range = 0;
    }
    return x_in_range | y_in_range;
}

/* Head of the studio block: the flags word at +0x00 and the counter at +0x18. */
typedef struct StudioBlockHead {
    long long studioFlags;
    unsigned char unmodeled_08[0x10];
    s32 unmodeled_18;
} StudioBlockHead;

extern s64 gSbStudioBlock[];
#define STUDIO_BLOCK_HEAD (*(StudioBlockHead *) gSbStudioBlock)

/* True when the studio block says the filter passes must be skipped (flag 1 set with word 6 clear) or the offset is within limits. */
s32 xglFilterShouldSkip(void)
{
    s32 skipFilter;

    skipFilter = 0;
    if (((s32)(STUDIO_BLOCK_HEAD.studioFlags & 1) && STUDIO_BLOCK_HEAD.unmodeled_18 == 0) || xglIsOffsetWithinLimits()) {
        skipFilter = 1;
    }
    return skipFilter;
}

s32 interpolateLinearFixed16(s32 mapInput, s32 mapMin, s32 mapMax, s32 scaledMin, s32 scaledMax) {
    f32 mapOffset;
    f32 scaledSpan;
    f32 mapSpan;

    mapOffset = mapInput - mapMin;
    scaledSpan = scaledMax - scaledMin;
    mapSpan = mapMax - mapMin;
    return ((mapOffset * (scaledSpan / mapSpan)) + scaledMin) * 16.0f;
}

s32 interpolateLinear(s32 mapInput, s32 mapMin, s32 mapMax, s32 scaledMin, s32 scaledMax) {
    f32 mapOffset;
    f32 scaledSpan;
    f32 mapSpan;

    mapOffset = mapInput - mapMin;
    scaledSpan = scaledMax - scaledMin;
    mapSpan = mapMax - mapMin;
    return (mapOffset * (scaledSpan / mapSpan)) + scaledMin;
}

typedef struct WindowCopy {
    s32 unmodeled_00;
    s32 alpha;       /* +0x04 */
    s32 blendMode;   /* +0x08 */
    s32 insetLeft;   /* +0x0C */
    s32 insetRight;  /* +0x10 */
    s32 insetTop;    /* +0x14 */
    s32 insetBottom; /* +0x18 */
    s32 offsetX;     /* +0x1C */
    s32 offsetY;     /* +0x20 */
    s32 unmodeled_24[6];
    s32 source; /* +0x3C: 0 copies the shown frame */
    s32 unmodeled_40[4];
    u32 unmodeled_50 : 2;
    u32 large : 1;
    u32 unmodeled_bits : 29;
} WindowCopy;

void drawEnvSetLarge(void);
s32 xglFilterShouldSkip(void);

/* Screen filter 4, BLUR in the game's filter name list.
 * Draws the studio copy window: the source is the display page (copy->source == 0) or the work page,
 * with insets, offset, alpha and blend mode; skipped when xglFilterShouldSkip(). */
void filterBlurDraw(WindowCopy *copy) {
    XglCamera *camera;
    u32 insetLeft;
    u32 insetRight;
    u32 insetTop;
    u32 insetBottom;
    s32 offsetX;
    s32 offsetY;
    s32 alpha;
    s32 blendMode;
    s32 fromDisplay;
    s32 textureLeft;
    s32 textureTop;
    s32 textureRight;
    s32 textureBottom;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;

    fromDisplay = copy->source == 0;
    alpha = (u8) copy->alpha;
    blendMode = copy->blendMode;
    insetLeft = (u8) copy->insetLeft;
    insetRight = (u8) copy->insetRight;
    insetTop = (u8) copy->insetTop;
    insetBottom = (u8) copy->insetBottom;
    offsetX = copy->offsetX;
    offsetY = copy->offsetY;
    camera = xglStudioSelectGetActiveCamera(0);
    if (xglFilterShouldSkip()) {
        return;
    }
    if (copy->large || !fromDisplay) {
        drawEnvSetLarge();
    }
    if (xglRenderMovieIsActive()) {
        if (fromDisplay) {
            textureLeft = (u32) camera->screen.window.vector.x * 1.25 * 16.0 + (insetLeft << 4);
            textureRight = (u32) camera->screen.window.vector.z * 1.25 * 16.0 - (insetRight << 4);
        } else {
            textureLeft = ((u32) camera->screen.window.vector.x + insetLeft) << 4;
            textureRight = ((u32) camera->screen.window.vector.z - insetRight) << 4;
        }
    } else {
        textureLeft = ((u32) camera->screen.window.vector.x + insetLeft) << 4;
        textureRight = ((u32) camera->screen.window.vector.z - insetRight) << 4;
    }
    textureTop = ((u32) camera->screen.window.vector.y + insetTop) << 4;
    textureBottom = ((u32) camera->screen.window.vector.w - insetBottom) << 4;
    left = ((u32) camera->screen.window.vector.x << 4) + (offsetX << 4) + DRAW_VERTEX_ORIGIN_X;
    right = ((u32) camera->screen.window.vector.z << 4) + (offsetX << 4) + DRAW_VERTEX_ORIGIN_X;
    top = ((u32) camera->screen.window.vector.y << 4) + (offsetY << 4) + DRAW_VERTEX_ORIGIN_Y;
    bottom = ((u32) camera->screen.window.vector.w << 4) + (offsetY << 4) + DRAW_VERTEX_ORIGIN_Y;
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, (s64) D_004C8390.depthBase | ((s64) GS_PSMZ24 << 24) | ((s64) 1 << 32));
    xglPacketAppendQuad(GS_TEST_1, 0x30000);
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    if (copy->large) {
        xglPacketAppendQuad(GS_FRAME_1, (s64) D_004C8390.workBase | ((s64) 8 << 16) | ((s64) 0xFF000000U << 32));
    } else {
        xglPacketAppendQuad(GS_FRAME_1, (s64) D_004C8390.displayBase | ((s64) 8 << 16) | ((s64) 0xFF000000U << 32));
    }
    switch (blendMode) {
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x68 | ((s64) alpha << 32));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x62 | ((s64) alpha << 32));
        break;
    case 3:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0xA8 | ((s64) alpha << 32));
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, (s64) 0x64 | ((s64) alpha << 32));
        break;
    }
    if (xglRenderMovieIsActive()) {
        if (fromDisplay) {
            xglPacketAppendQuad(GS_TEX0_1, (s64) (D_004C8390.moviePage * 32) | ((s64) 10 << 14) | ((s64) 2 << 20) |
                                       ((s64) 10 << 26) | ((s64) 9 << 30) | ((s64) 1 << 34));
        } else {
            xglPacketAppendQuad(GS_TEX0_1, (s64) (D_004C8390.workBase * 32) | ((s64) 8 << 14) | ((s64) 0 << 20) |
                                       ((s64) 9 << 26) | ((s64) 9 << 30) | ((s64) 1 << 34));
        }
    } else {
        if (fromDisplay) {
            xglPacketAppendQuad(GS_TEX0_1, (s64) (D_004C8390.largeBase * 32) | ((s64) 8 << 14) | ((s64) 0 << 20) |
                                       ((s64) 9 << 26) | ((s64) 9 << 30) | ((s64) 1 << 34));
        } else {
            xglPacketAppendQuad(GS_TEX0_1, (s64) (D_004C8390.workBase * 32) | ((s64) 8 << 14) | ((s64) 0 << 20) |
                                       ((s64) 9 << 26) | ((s64) 9 << 30) | ((s64) 1 << 34));
        }
    }
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_RGBAQ, (s64) 0x80 | ((s64) 0x80 << 8) | ((s64) 0x80 << 16) | ((s64) 0x80 << 24) |
                               ((s64) 0x3F800000 << 32));
    xglPacketAppendQuad(GS_PRIM, 0x156);
    xglPacketAppendQuad(GS_UV, (s64) textureLeft | ((s64) textureTop << 16));
    xglPacketAppendQuad(GS_XYZF2, (s64) left | ((s64) top << 16));
    xglPacketAppendQuad(GS_UV, (s64) textureRight | ((s64) textureBottom << 16));
    xglPacketAppendQuad(GS_XYZF2, (s64) right | ((s64) bottom << 16));
    if (copy->large) {
        xglPacketAppendQuad(GS_TEX0_1, (s64) (D_004C8390.workBase * 32) | ((s64) 8 << 14) | ((s64) 0 << 20) |
                                   ((s64) 9 << 26) | ((s64) 9 << 30) | ((s64) 1 << 34));
        xglPacketAppendQuad(GS_ALPHA_1, 0x54);
        xglPacketAppendQuad(GS_FRAME_1, (s64) D_004C8390.displayBase | ((s64) 8 << 16) | ((s64) 0xFF000000U << 32));
        xglPacketAppendQuad(GS_PRIM, 0x156);
        xglPacketAppendQuad(GS_UV, 0);
        xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
        xglPacketAppendQuad(GS_UV, 0x1C002000);
        xglPacketAppendQuad(GS_XYZF2, (s64) 0x8FF8 | ((s64) 0x8DF7 << 16));
    }
    xglPacketAppendQuad(GS_FRAME_1, (s64) D_004C8390.displayBase | ((s64) 8 << 16) | ((s64) 0xFF000000U << 32));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

typedef struct DefocusParams {
    u8 unmodeled_00[4];
    s32 frameMark;
    s32 centerX;
    s32 centerY;
    s32 width;
    s32 height;
    s32 tagBits;
    s32 field1C;
    s32 field20;
    s32 field24;
    s32 field28;
    s32 type;
} DefocusParams;

extern s64 D_00407DB0[];
extern u16 movieFramebufferBasePage[];
void sceVif1PkAddDataN(s32 packet, s32 address, s32 qwordCount);

typedef struct GsWords {
    s32 w0;
    s32 w1;
    s32 w2;
    s32 w3;
} GsWords;

static inline void setWords(GsWords *dst, s32 w0, s32 w1, s32 w2, s32 w3) {
    dst->w0 = w0;
    dst->w1 = w1;
    dst->w2 = w2;
    dst->w3 = w3;
}

/* Defocus pass: fills the scratchpad with the screen quad packet words and sends them through VIF1. */
/* Defocus pass: fills the scratchpad with the screen quad packet words and sends them through VIF1. */
/* Defocus pass: fills the scratchpad with the screen quad packet words and sends them through VIF1. */
/* Scratchpad view of the packet words written by the defocus pass. */
#define SCRATCHPAD_DOUBLEWORDS ((u64 *) XGL_SCRATCHPAD)
#define SCRATCHPAD_WORDS ((GsWords *) 0x70000000)

/* Screen filter 5, MOSAIC in the game's filter name list.
 * Defocus pass: fills the scratchpad with the screen quad packet words and sends them through VIF1. */
void filterMosaicDraw(DefocusParams *params, s32 packet) {
    s32 depth = params->frameMark;
    s32 centerX = params->centerX;
    s32 centerY = params->centerY;
    s32 width = params->width;
    s32 height = params->height;
    s32 tagBits = params->tagBits;
    s32 red = params->field1C;
    s32 green = params->field20;
    s32 blue = params->field24;
    s32 alpha = params->field28;
    s32 type = params->type;
    s32 halfWidth;
    s32 halfHeight;

    switch (type) {
    case 1:
        D_00407DB0[10] = 0x48;
        break;
    case 2:
        D_00407DB0[10] = 0x42;
        break;
    default:
        D_00407DB0[10] = 0x44;
        break;
    }
    if (xglFilterShouldSkip() != 0) {
        return;
    }
    D_00407DB0[8] = (s64) (movieFramebufferBasePage[0] << 5) | 0x24020000 | ((s64) 0x9000 << 18);
    D_00407DB0[12] = ((s64) (tagBits ^ 0x3FF) << 4) | ((s64) (tagBits ^ 0x3FF) << 24) | 0xF;
    halfWidth = width >> 1;
    halfHeight = height >> 1;
    setWords(SCRATCHPAD_WORDS + 0, 0, 0, 0, 0x51000006);
    setWords(SCRATCHPAD_WORDS + 1, 0x8001, 0x50AB4000, 0x53531, 0);
    setWords(SCRATCHPAD_WORDS + 2, red, green, blue, alpha);
    setWords(SCRATCHPAD_WORDS + 3, (centerX - halfWidth) << 4, (centerY - halfHeight) << 4, 0, 0);
    setWords(SCRATCHPAD_WORDS + 4, ((centerX - halfWidth) << 4) + 0x6FF8, ((centerY - halfHeight) << 4) + 0x71F7, depth,
             0);
    setWords(SCRATCHPAD_WORDS + 5, (centerX + halfWidth) << 4, (centerY + halfHeight) << 4, 0, 0);
    setWords(SCRATCHPAD_WORDS + 6, ((centerX + halfWidth) << 4) + 0x6FF8, ((centerY + halfHeight) << 4) + 0x71F7, depth,
             0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddDataN(packet, (s32) D_00407DB0, 0x1C);
    sceVif1PkAddDataN(packet, SCRATCHPAD_WORDS, 0x1C);
}

/* Screen quad of the filter passes: the rectangle and its colour, read from the caller's record. */
typedef struct ScreenQuad {
    u8 unmodeled_00[4];
    s32 hiWord;
    s32 centerX;
    s32 centerY;
    s32 width;
    s32 height;
    s32 gsWordA;
    s32 gsWordB;
    s32 gsWordC;
    u8 unmodeled_24[0x2C];
    u32 flags;
} ScreenQuad;

/* Appends the two corners of a sprite centred on (x, y), at depth z, in 12.4 fixed point. */
static inline void drawSprite(s32 x, s32 y, s32 width, s32 height, s32 depth) {
    s32 halfWidth;
    s32 halfHeight;

    halfWidth = width >> 1;
    halfHeight = height >> 1;
    xglPacketAppendQuad(4, (s64) (((x - halfWidth) << 4) + 0x6FF8) |
                               ((s64) (((y - halfHeight) << 4) + 0x71F7) << 16) | ((s64) depth << 32));
    xglPacketAppendQuad(4, (s64) (((x + halfWidth) << 4) + 0x6FF8) |
                               ((s64) (((y + halfHeight) << 4) + 0x71F7) << 16) | ((s64) depth << 32));
}

/* Sets the RGBAQ colour of the following sprite (alpha 0x80, Q = 1.0). */
static inline void setColor(s32 red, s32 green, s32 blue) {
    xglPacketAppendQuad(1, (s64) red | ((s64) green << 8) | ((s64) blue << 16) | ((s64) 0x80 << 24) |
                               ((s64) 0x3F800000 << 32));
}

/* Screen filter 6, NEGAPOSI in the game's filter name list.
 * Draws the screen quad into the frame buffer page chosen by quad->flags bit 2 (set: work page). */
void filterNegaposiDraw(ScreenQuad *quad) {
    s32 red;
    s32 blue;
    s32 green;
    s32 x;
    s32 y;
    s32 depth;
    s32 width;
    s32 height;

    x = quad->centerX;
    y = quad->centerY;
    depth = quad->hiWord;
    width = quad->width;
    height = quad->height;
    red = quad->gsWordA;
    green = quad->gsWordB;
    blue = quad->gsWordC;
    if (xglFilterShouldSkip() != 0) {
        return;
    }
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
    xglPacketAppendQuad(76, ((quad->flags >> 2) & 1)
                                ? ((s64) D_004C8390.workBase | 0xFF00000000080000ULL)
                                : ((s64) D_004C8390.displayBase | 0xFF00000000080000ULL));
    xglPacketAppendQuad(66, 0x80000000A4ULL);
    setColor(red, green, blue);
    xglPacketAppendQuad(0, 70);
    drawSprite(x, y, width, height, depth);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* The screen filter settings passed to the filter passes. */
typedef struct ScreenFilter {
    u8 unk00[4];
    s32 hue;          /* +0x04: degrees */
    s32 saturation;   /* +0x08 */
    s32 value;        /* +0x0C */
    s32 blendFix;     /* +0x10: FIX value of GS ALPHA */
    s32 blendMode;    /* +0x14 */
    u8 unk18[0x50 - 0x18];
    u32 unk50_0 : 2;
    u32 additiveBlend : 1; /* +0x50 bit 2 */
} ScreenFilter;

/* VIF MSCAL: start the VU1 microprogram at the given address (in 8-byte units). */
#define XGL_VIF_MSCAL(address) (0x14000000 | ((u32) (address) >> 3))
/* VIF FLUSHA. */
#define XGL_VIF_FLUSHA 0x11000000
/* IEEE-754 bits of 1.0f, the Q (perspective) word of RGBAQ. */
#define XGL_FLOAT_ONE_BITS 0x3F800000
/* Scratchpad start of the packet buffer. */
#define XGL_SCRATCHPAD 0x70000000
/* One word of the packet being built in the scratchpad. */
#define SCRATCHPAD_WORD(index) (((u32 *) XGL_SCRATCHPAD)[index])

extern s32 D_005166D0[];
extern u8 D_005166D4[];
extern u8 D_00000000[];
extern u8 D_40[];
void sceVif1PkRef(XglPacket *packet, void *data, s32 qwordCount, s32 opt1, s32 opt2, s32 irq);
void sceVif1PkOpenUpkCode(XglPacket *packet, s32 vuAddress, s32 unpackCommand, s32 cycleLength, s32 writeLength);
void sceVif1PkAddUpkData128N(XglPacket *packet, s32 address, s32 qwordCount);
void sceVif1PkCloseUpkCode(XglPacket *packet);
void sceVif1PkAddCode(XglPacket *packet, s32 code);

/* Queues one frame copy drawRecord: the work and frame pages, the blend, and the source and destination
 * rectangles of the copy in blocks of blockWidth x blockHeight. */
static inline void xglFilterQueueCopy(s32 workBase, s32 frameBase, s32 blendFix, s32 blendMode,
                                      s32 blockWidth, s32 blockHeight, s32 sourceX0, s32 sourceY0, s32 sourceY1,
                                      s32 destinationX0, s32 destinationY0, s32 destinationX1, s32 destinationY1)
{
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = workBase;
    drawRecord->frameBase = frameBase;
    drawRecord->blockWidth = blockWidth;
    drawRecord->blockHeight = blockHeight;
    drawRecord->unk44 = 0;
    drawRecord->unk48 = 0;
    drawRecord->blendFix = blendFix;
    drawRecord->blendMode = blendMode;
    drawRecord->sourceRect[0] = sourceX0;
    drawRecord->sourceRect[1] = sourceY0;
    drawRecord->sourceRect[2] = 0x200;
    drawRecord->sourceRect[3] = sourceY1;
    drawRecord->destinationRect[0] = destinationX0;
    drawRecord->destinationRect[1] = destinationY0;
    drawRecord->destinationRect[2] = destinationX1;
    drawRecord->destinationRect[3] = destinationY1;
    drawEnvSendRecordPacket();
}

/* Draws the filtered half of the frame back onto the displayed page: one sprite from screen row top to row
 * bottom (GS XYZ, 1/16 pixel), clipped to the active camera window. */
static inline void xglFilterDrawSprite(ScreenFilter *filter, s32 top, s32 bottom)
{
    XglCamera *camera;
    s32 scissorLeft;
    s32 scissorTop;
    s32 scissorRight;
    s32 scissorBottom;

    camera = xglStudioSelectGetActiveCamera(0);
    scissorBottom = camera->screen.window.vector.w;
    scissorRight = camera->screen.window.vector.z;
    scissorTop = camera->screen.window.vector.y;
    scissorLeft = camera->screen.window.vector.x;
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(scissorLeft, scissorRight - 1, scissorTop, scissorBottom - 1));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.frameBase * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    if (filter->additiveBlend) {
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 1, 1, 0));
    } else {
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
    }
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, 0x156);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, top, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0xE00));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, bottom, 0));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Screen filter 9, MONOTONE in the game's filter name list.
 * Uploads the colour filter: a hue-tinted gray ramp CLUT, then draws the frame through it with the VU1 sprite program. */
void filterMonotoneDraw(ScreenFilter *filter, XglPacket *vifPacket) {
    s32 hue;
    u8 saturation;
    u8 brightness;
    u8 blendFix;
    s32 blendMode;
    s32 red;
    s32 green;
    s32 blue;
    s32 clutIndex;

    hue = filter->hue % 360;
    saturation = filter->saturation;
    brightness = filter->value;
    blendFix = filter->blendFix;
    blendMode = filter->blendMode;
    if (xglFilterShouldSkip()) {
        return;
    }
    colorHsvToRgb(hue, saturation, brightness, &red, &green, &blue);
    drawEnvSetLarge();
    sceVif1PkRef(vifPacket, D_005166D4, D_005166D0[0], 0, 0, 0);
    sceVif1PkCnt(vifPacket, 0);

    sceVif1PkOpenUpkCode(vifPacket, 0x40, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendFloatQuad(256.0f, 64.0f, 0.0f, 0.0f);
    xglPacketAppendWordQuad(0x20, 0x38, 0, 0);
    xglPacketAppendWordQuad(0x000201F3, 0x000A01F3, 0x001201F3, 0x001A01F3);
    xglPacketAppendWordQuad(0x002201F3, 0x002A01F3, 0x003201F3, 0x003A01F3);
    xglPacketAppendWordQuad(0x004201F3, 0x004A01F3, 0x005201F3, 0x005A01F3);
    xglPacketAppendWordQuad(0x006201F3, 0x006A01F3, 0x007201F3, 0x007A01F3);
    xglPacketAppendWordQuad(0x008201F3, 0x008A01F3, 0x009201F3, 0x009A01F3);
    xglPacketAppendWordQuad(0x00A201F3, 0x00AA01F3, 0x00B201F3, 0x00BA01F3);
    xglPacketAppendWordQuad(0x00C201F3, 0x00CA01F3, 0x00D201F3, 0x00DA01F3);
    xglPacketAppendWordQuad(0x00E201F3, 0x00EA01F3, 0x00F201F3, 0x00FA01F3);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);

    /* The gray ramp CLUT, in the GS CSM1 order (entries 8-15 and 16-23 of each block of 32 swapped). */
    sceVif1PkOpenUpkCode(vifPacket, 0x60, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(4, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_BITBLTBUF, GS_SET_BITBLTBUF(0, 0, 0, D_004C8390.frameBase * 32, 4, 0));
    xglPacketAppendQuad(GS_TRXPOS, 0);
    xglPacketAppendQuad(GS_TRXREG, GS_SET_TRXREG(16, 16));
    xglPacketAppendQuad(GS_TRXDIR, 0);
    xglPacketAppendEntry(0x40, 0x08000000, 0, 0);
    for (clutIndex = 0; clutIndex < 256; clutIndex += 8) {
        switch (clutIndex & 0x18) {
        case 0:
        case 0x18:
            xglPacketAppendWordQuad(clutIndex * 0x01010101, (clutIndex + 1) * 0x01010101,
                                    (clutIndex + 2) * 0x01010101, (clutIndex + 3) * 0x01010101);
            xglPacketAppendWordQuad((clutIndex + 4) * 0x01010101, (clutIndex + 5) * 0x01010101,
                                    (clutIndex + 6) * 0x01010101, (clutIndex + 7) * 0x01010101);
            break;
        case 0x10:
            xglPacketAppendWordQuad((clutIndex - 8) * 0x01010101, (clutIndex - 7) * 0x01010101,
                                    (clutIndex - 6) * 0x01010101, (clutIndex - 5) * 0x01010101);
            xglPacketAppendWordQuad((clutIndex - 4) * 0x01010101, (clutIndex - 3) * 0x01010101,
                                    (clutIndex - 2) * 0x01010101, (clutIndex - 1) * 0x01010101);
            break;
        case 0x08:
            xglPacketAppendWordQuad((clutIndex + 8) * 0x01010101, (clutIndex + 9) * 0x01010101,
                                    (clutIndex + 10) * 0x01010101, (clutIndex + 11) * 0x01010101);
            xglPacketAppendWordQuad((clutIndex + 12) * 0x01010101, (clutIndex + 13) * 0x01010101,
                                    (clutIndex + 14) * 0x01010101, (clutIndex + 15) * 0x01010101);
            break;
        }
    }
    xglPacketAppendEntry(GIF_TAG_EOP | 2, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.displayBase * 32, 8, 0x1B, 9, 9, 1, 1,
                                               D_004C8390.frameBase * 32, 0, 0, 0, 1));
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);
    sceVif1PkAddCode(vifPacket, XGL_VIF_MSCAL(D_00000000));
    sceVif1PkAddCode(vifPacket, XGL_VIF_FLUSHA);

    /* Pass 1: the frame through the CLUT into the work page. */
    sceVif1PkOpenUpkCode(vifPacket, 0xC0, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP | 0x12, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(0, 511, 0, 895));
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(1, 0, 0, 1, 0, 0, 1, 1));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.frameBase, 8, 0, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 10, 1, 1, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEX1_1, 0);
    xglPacketAppendQuad(GS_TEXA, GS_SET_RGBAQ(0, 0, 0, 0, 0x80));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    xglPacketAppendQuad(GS_PRIM, 0x116);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.frameBase, 8, 2, 0));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 2, 9, 10, 1, 1, 0, 0, 0, 0, 0));
    xglPacketAppendEntry(GIF_TAG_EOP | 1, 0x408B4000, 0x5353, 0);
    xglPacketAppendFloatQuad(0.0f, 0.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28664.0f, 29175.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(0.0f, 0.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28664.0f, 29175.0f, 0.0f, 0.0f);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);

    /* Pass 2: the tint, blended over the frame. */
    sceVif1PkOpenUpkCode(vifPacket, 0xE0, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP | 9, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.frameBase, 8, 0, 0xFF00FFFF));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(1, 2, 2, 0, 0x26));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x0F, 0x80, XGL_FLOAT_ONE_BITS));
    xglPacketAppendQuad(GS_PRIM, 0x156);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);

    sceVif1PkOpenUpkCode(vifPacket, 0xF0, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP | 0xA, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.frameBase, 8, 0, 0xFF00FFFF));
    xglPacketAppendQuad(GS_TEX0_1,
                        GS_SET_TEX0(D_004C8390.frameBase * 32, 8, 0x1B, 9, 9, 1, 1, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x4B));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    xglPacketAppendQuad(GS_PRIM, 0x156);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0xE00));
    xglPacketAppendQuad(GS_XYZF2, 0x7FF78FF8);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);

    /* Pass 3: the filter colour through the VU1 sprite program. */
    sceVif1PkOpenUpkCode(vifPacket, 0x100, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP | 3, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.frameBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_TEX0_1,
                        GS_SET_TEX0(D_004C8390.frameBase * 32, 16, 0x13, 10, 10, 1, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, 0x80, XGL_FLOAT_ONE_BITS));
    xglPacketAppendEntry(GIF_TAG_EOP | 1, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendEntry(GIF_TAG_EOP | 1, 0xC08B4000, 0x53535353, 0x5353);
    xglPacketAppendFloatQuad(128.0f, 0.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28664.0f, 29175.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(384.0f, 32.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28920.0f, 29207.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(64.0f, 64.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28664.0f, 29207.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(192.0f, 96.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28792.0f, 29239.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(320.0f, 64.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28792.0f, 29207.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(448.0f, 96.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28920.0f, 29239.0f, 0.0f, 0.0f);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);

    /* The top half of the frame. */
    sceVif1PkOpenUpkCode(vifPacket, 0x50, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0xE00));
    xglPacketAppendQuad(GS_XYZF2, 0x7FF78FF8);
    xglPacketAppendFloatQuad(0.0f, 0.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28792.0f, 29175.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(128.0f, 7168.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28920.0f, 36343.0f, 0.0f, 0.0f);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);
    sceVif1PkAddCode(vifPacket, XGL_VIF_MSCAL(D_40));
    sceVif1PkAddCode(vifPacket, XGL_VIF_FLUSHA);
    xglFilterQueueCopy(D_004C8390.workBase, D_004C8390.frameBase, blendFix, blendMode, 8, 8, 0, 0, 0xE0, 0,
                       0, 0x200, 0xE0);
    xglFilterDrawSprite(filter, 0x71F7, 0x7FF7);

    /* The bottom half of the frame. */
    sceVif1PkOpenUpkCode(vifPacket, 0x50, 0x6C, 1, 1);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0, 0xE00));
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x1C00));
    xglPacketAppendQuad(GS_XYZF2, 0x7FF78FF8);
    xglPacketAppendFloatQuad(0.0f, 7168.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28792.0f, 29175.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(128.0f, 14336.0f, 0.0f, 0.0f);
    xglPacketAppendFloatQuad(28920.0f, 36343.0f, 0.0f, 0.0f);
    sceVif1PkAddUpkData128N(vifPacket, XGL_SCRATCHPAD, xglPacketGetEntryCount());
    sceVif1PkCloseUpkCode(vifPacket);
    sceVif1PkAddCode(vifPacket, XGL_VIF_MSCAL(D_40));
    sceVif1PkAddCode(vifPacket, XGL_VIF_FLUSHA);
    xglFilterQueueCopy(D_004C8390.workBase, D_004C8390.frameBase, blendFix, blendMode, 8, 8, 0, 0xE0,
                       0x1C0, 0, 0, 0x200, 0xE0);
    xglFilterDrawSprite(filter, 0x7FF7, 0x8DF7);
}

struct DrawEnvRecord *drawEnvGetRecord(void);

/* One timed wipe pass of the screen filter: the window alpha ramps from 128 down to 0 over `duration` frames,
 * then the window layout chosen by `kind` is set up. unmodeled_XX fields are not yet read by the code. */
typedef struct WipePass {
    u8 unk00;
    u8 state;
    u8 unk02;
    u8 unk03;
    u32 duration;
    s32 kind;
    s32 nextPass;
    u8 unmodeled_10[0x30];
    s32 elapsed;
    u8 unmodeled_44[0xC];
    u32 unk50_0 : 1;
    u32 done : 1;
    u32 unk50_2 : 2;
    u32 paused : 1;
    u32 unk50_6 : 27;
} WipePass;


/* Sets the blend and the source and destination rectangles of the draw environment record. */
static inline void setWin(s32 blendFix, s32 blendMode, s32 sourceX, s32 sourceY, s32 sourceW, s32 sourceH,
                          s32 destinationX, s32 destinationY, s32 destinationW, s32 destinationH) {
    DrawEnvRecord *record = drawEnvGetRecord();

    record->blendFix = blendFix;
    record->blendMode = blendMode;
    record->sourceRect[0] = sourceX;
    record->sourceRect[1] = sourceY;
    record->sourceRect[2] = sourceW;
    record->sourceRect[3] = sourceH;
    record->destinationRect[0] = destinationX;
    record->destinationRect[1] = destinationY;
    record->destinationRect[2] = destinationW;
    record->destinationRect[3] = destinationH;
}

/* Sets the work and frame pages, the block size, the blend and the rectangles of the draw environment record. */
static inline void setWindowPages(s32 workBase, s32 frameBase, s32 blockWidth, s32 blockHeight, s32 drawFlags44,
                                  s32 drawFlags48, s32 blendFix, s32 blendMode, s32 sourceX, s32 sourceY,
                                  s32 sourceW, s32 sourceH, s32 destinationX, s32 destinationY,
                                  s32 destinationW, s32 destinationH) {
    DrawEnvRecord *record = drawEnvGetRecord();

    record->workBase = workBase;
    record->blockWidth = blockWidth;
    record->unk44 = drawFlags44;
    record->frameBase = frameBase;
    record->blockHeight = blockHeight;
    record->unk48 = drawFlags48;
    record->blendFix = blendFix;
    record->blendMode = blendMode;
    record->sourceRect[0] = sourceX;
    record->sourceRect[1] = sourceY;
    record->sourceRect[2] = sourceW;
    record->sourceRect[3] = sourceH;
    record->destinationRect[0] = destinationX;
    record->destinationRect[1] = destinationY;
    record->destinationRect[2] = destinationW;
    record->destinationRect[3] = destinationH;
}

/* Screen filter 10, CROSS FADE in the game's filter name list. */
void filterCrossFadeDraw(WipePass *pass) {
    s32 elapsed;
    u32 duration;
    u32 kind;
    s32 alpha;
    s32 done;
    s32 paused;
    s32 *elapsedPtr;
    s32 *nextPtr;
    nextPtr = &pass->nextPass;
    elapsedPtr = &pass->elapsed;
again:
    elapsed = *elapsedPtr;
    duration = pass->duration;
    kind = pass->kind;
    alpha = 128.0f - (f32) elapsed / (f32) duration * 128.0f;
    done = elapsed == duration;
    paused = pass->paused;
    switch (pass->state) {
    case 0:
        xglPacketSendFixedHeader();
        switch (kind) {
        case 0:
        default:
            drawEnvSetLarge();
            break;
        case 1:
            drawEnvSetHalfDraw();
            break;
        case 2:
            setWindowPages(D_004C8390.displayBase, D_004C8390.unk10, 8, 8, 0, 2, 128, 3, 0, 0, 0x200, 0x1C0,
                           0, 0, 0x200, 0xE0);
            drawEnvGetRecord()->flags.all |= (u64) 1 << 32;
            drawEnvGetRecord()->flags.all |= (u64) 1 << 33;
            drawEnvSendRecordPacket();
            break;
        case 3:
            setWindowPages(D_004C8390.displayBase, D_004C8390.unk10, 8, 4, 0, 2, 128, 3, 0, 0, 0x200, 0x1C0,
                           0, 0, 0x100, 0x1C0);
            drawEnvGetRecord()->flags.all |= (u64) 1 << 34;
            drawEnvGetRecord()->flags.all |= (u64) 1 << 35;
            drawEnvSendRecordPacket();
            break;
        }
        drawEnvSetCameraScissor();
    case 1:
        if (done) {
            if (*nextPtr == 0) {
                pass->done = 1;
            }
            break;
        }
        if (!paused) {
            (*elapsedPtr)++;
        }
        if (xglFilterShouldSkip()) {
            break;
        }
        switch (kind) {
        case 0:
        default:
            setWin(alpha, 0, 0, 0, 0x200, 0x1C0, 0, 0, 0x200, 0x1C0);
            func_001D55E8();
            break;
        case 1:
            setWin(alpha, 0, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
            func_001D56D8();
            break;
        case 2:
            setWindowPages(D_004C8390.unk10, D_004C8390.displayBase, 8, 8, 2, 0, alpha, 0, 0, 0, 0x200, 0xE0, 0,
                           0, 0x200, 0x1C0);
            drawEnvGetRecord()->flags.all |= (u64) 1 << 40;
            drawEnvGetRecord()->flags.all |= (u64) 1 << 41;
            drawEnvSendRecordPacket();
            break;
        case 3:
            setWindowPages(D_004C8390.unk10, D_004C8390.displayBase, 4, 8, 2, 0, alpha, 0, 0, 0, 0x100, 0x1C0, 0,
                           0, 0x200, 0x1C0);
            drawEnvGetRecord()->flags.all |= (u64) 1 << 42;
            drawEnvGetRecord()->flags.all |= (u64) 1 << 43;
            drawEnvSendRecordPacket();
            break;
        }
        break;
    }
    if (*nextPtr != 0) {
        pass->state = 0;
        *elapsedPtr = 0;
        *nextPtr = 0;
        goto again;
    }
}

typedef struct WaveFilter {
    u8 unmodeled_00[4];
    s32 depth;       /* +0x04: z of every sprite */
    s32 amplitudeX;  /* +0x08 */
    s32 phaseStepX;  /* +0x0C */
    s32 offsetStepX; /* +0x10 */
    s32 amplitudeY;  /* +0x14 */
    s32 phaseStepY;  /* +0x18 */
    s32 offsetStepY; /* +0x1C */
    s32 blendAmount; /* +0x20: fixed alpha */
    s32 mode;        /* +0x24 */
    u8 unmodeled_28[0x40 - 0x28];
    s32 offsetX; /* +0x40: 12-bit phase of the x wave */
    s32 offsetY; /* +0x44: 12-bit phase of the y wave */
    u8 unmodeled_48[0x50 - 0x48];
    u32 unmodeled_50 : 3;
    u32 frozen : 1; /* either bit stops the wave's phase */
    u32 stopped : 1;
    u32 unmodeled_50_5 : 27;
} WaveFilter;

typedef struct WaveSprite {
    PacketEntry firstTexel;
    PacketEntry firstPoint;
    PacketEntry secondTexel;
    PacketEntry secondPoint;
} WaveSprite;

#define WAVE_COLUMNS 16
#define WAVE_ROWS 14
#define WAVE_CELL 512 /* 32 pixels in 12.4 fixed point */
#define WAVE_ORIGIN_X 0x6FF8
#define WAVE_ORIGIN_Y 0x71F7
#define WAVE_PIXEL_FIXED(pixels) ((pixels) << 4)

static inline u32 waveClamp(u32 value, u32 low, u32 high) {
    u32 result;

    if (value < low) {
        result = low;
    } else {
        result = (high < value) ? high : value;
    }
    return result;
}

static inline void waveSetTag(PacketEntry *tag, s32 word0, s32 word1, s32 word2, s32 word3) {
    tag->word0 = word0;
    tag->word1 = word1;
    tag->word2 = word2;
    tag->word3 = word3;
}

f32 xglSin(f32 angle);

/* Screen filter 11, SWELL2 in the game's filter name list.
 * Wave pass: advances the two 12-bit wave phases and sends a 16 x 14 grid of sprites whose corners
 * are displaced by the wave, clamped to the window. */
void filterSwell2Draw(WaveFilter *filter, s32 packet) {
    s32 depth = filter->depth;
    s32 amplitudeX = filter->amplitudeX;
    s32 phaseStepX = filter->phaseStepX;
    s32 offsetStepX = filter->offsetStepX;
    s32 amplitudeY = filter->amplitudeY;
    s32 phaseStepY = filter->phaseStepY;
    s32 offsetStepY = filter->offsetStepY;
    s32 blendAmount = filter->blendAmount;
    s32 mode = filter->mode;
    s32 *offsetX = &filter->offsetX;
    s32 *offsetY = &filter->offsetY;
    s32 windowLeft;
    s32 windowTop;
    s32 windowRight;
    s32 windowBottom;
    s32 column;
    s32 row;
    f32 phaseX;
    f32 phaseY;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    XglCamera *camera;
    u32 *columnOffsets = (u32 *) XGL_SCRATCHPAD;          /* 17 column offsets */
    u32 *rowOffsets = (u32 *) (XGL_SCRATCHPAD + 0x44); /* 17 row offsets */
    WaveSprite *sprite = (WaveSprite *) (XGL_SCRATCHPAD + 0x80);
    s32 nextColumn;
    s32 i;

    camera = xglStudioSelectGetActiveCamera(0);
    windowLeft = camera->screen.window.vector.x;
    windowTop = camera->screen.window.vector.y;
    windowRight = camera->screen.window.vector.z;
    windowBottom = camera->screen.window.vector.w;
    if (!(filter->frozen || filter->stopped)) {
        *offsetX = (*offsetX + offsetStepX) & 0xFFF;
        *offsetY = (*offsetY + offsetStepY) & 0xFFF;
    }
    if (xglFilterShouldSkip()) {
        return;
    }
    drawEnvSetLarge();
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_TEST_1, 0x70000);
    switch (mode) {
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, ((s64) blendAmount << 32) | 0x68);
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, ((s64) blendAmount << 32) | 0x62);
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, ((s64) blendAmount << 32) | 0x64);
        break;
    }
    xglPacketAppendQuad(GS_TEX0_1, (D_004C8390.workBase << 5) | 0x24020000 | ((s64) 0xC800 << 19));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);

    waveSetTag((PacketEntry *) XGL_SCRATCHPAD, 0x80E0, 0x40AF4000, 0x5353, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD, 1);

    phaseX = (f32) *offsetX * 0.0015343555f;
    phaseY = (f32) *offsetY * 0.0015343555f;
    for (i = 0; i < 17; i++) {
        columnOffsets[i] = (u32) ((f32) amplitudeX * xglSin(phaseX));
        rowOffsets[i] = (u32) ((f32) amplitudeY * xglSin(phaseY));
        phaseX += (f32) (255 - phaseStepX) * 3.1415927f / 255.0f;
        phaseY += (f32) (255 - phaseStepY) * 3.1415927f / 255.0f;
    }
    /* The edges of the screen do not move. */
    columnOffsets[WAVE_COLUMNS] = 0;
    columnOffsets[0] = 0;
    rowOffsets[WAVE_ROWS] = 0;
    rowOffsets[0] = 0;

    left = WAVE_PIXEL_FIXED(windowLeft);
    top = WAVE_PIXEL_FIXED(windowTop);
    right = WAVE_PIXEL_FIXED(windowRight);
    bottom = WAVE_PIXEL_FIXED(windowBottom);
    for (column = 0; column < WAVE_COLUMNS; column++) {
        for (row = 0; row < WAVE_ROWS; row++) {
            nextColumn = column * WAVE_CELL + WAVE_CELL;
            sprite->firstTexel.word0 = waveClamp(column * WAVE_CELL + columnOffsets[column], left, right);
            sprite->firstTexel.word1 = waveClamp(WAVE_PIXEL_FIXED(row * 32) + rowOffsets[row], top, bottom);
            sprite->firstTexel.word2 = 0;
            sprite->firstTexel.word3 = 0;
            sprite->firstPoint.word0 = column * WAVE_CELL + WAVE_ORIGIN_X;
            sprite->firstPoint.word1 = WAVE_PIXEL_FIXED(row * 32) + WAVE_ORIGIN_Y;
            sprite->firstPoint.word2 = depth;
            sprite->firstPoint.word3 = 0;
            sprite->secondTexel.word0 = waveClamp(nextColumn + columnOffsets[column + 1], left, right);
            sprite->secondTexel.word1 =
                waveClamp(WAVE_PIXEL_FIXED((row + 1) * 32) + rowOffsets[row + 1], top, bottom);
            sprite->secondTexel.word2 = 0;
            sprite->secondTexel.word3 = 0;
            sprite->secondPoint.word0 = WAVE_PIXEL_FIXED((column + 1) * 32) + WAVE_ORIGIN_X;
            sprite->secondPoint.word1 = (row + 1) * WAVE_CELL + WAVE_ORIGIN_Y;
            sprite->secondPoint.word2 = depth;
            sprite->secondPoint.word3 = 0;
            sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD + 0x80, 4);
        }
    }
    sceVif1PkCloseDirectHLCode(packet);
}

u32 xglSRand(void);

typedef struct ScreenWarpParams {
    u8 unmodeled_00[4];
    s32 alpha;       /* +0x04: copied into every tile */
    s32 originX;     /* +0x08 */
    s32 originY;     /* +0x0C */
    s32 step;        /* +0x10: tile spacing, at least 4 */
    s32 enabled;     /* +0x14: zero skips the warp */
    s32 mode;        /* +0x18: selects the GS alpha word of the header */
    s32 movieActive; /* +0x1C: nonzero when the movie draw environments are used */
} ScreenWarpParams;

static f32 gXglWarpRandomScales[512];
extern s32 gXglWarpGifTag[];

typedef struct WarpVertex {
    s32 u;
    s32 v;
    s32 unmodeled_08;
    s32 unmodeled_0C;
    s32 x;
    s32 y;
    s32 alpha;
    s32 unmodeled_1C;
} WarpVertex;

#define DRAW_ENV_SET_BLOCK(record, destX, destY, destW, destH, srcW, srcH) \
    (record) = drawEnvGetRecord(); \
    (record)->blendFix = 0x80; \
    (record)->blendMode = 3; \
    (record)->sourceRect[0] = 0; \
    (record)->sourceRect[1] = 0; \
    (record)->sourceRect[2] = (srcW); \
    (record)->sourceRect[3] = (srcH); \
    (record)->destinationRect[0] = (destX); \
    (record)->destinationRect[1] = (destY); \
    (record)->destinationRect[2] = (destW); \
    (record)->destinationRect[3] = (destH)

#define WARP_SCREEN_WIDTH 0x200
#define WARP_SCREEN_HEIGHT 0x1C0

/* Screen filter 12, WAVY BLUR in the game's filter name list.
 * Screen warp: four GS draw passes over the 512x448 screen, then a warp strip
 * of one centre vertex and one vertex per step along the screen edges, sent as
 * one VIF direct packet. */
void filterWavyBlurDraw(ScreenWarpParams *params, s32 packet) {
    s32 alpha;
    s32 originX;
    s32 originY;
    s32 step;
    s32 enabled;
    s32 mode;
    s32 movieActive;
    XglCamera *camera;
    s32 windowLeft;
    s32 windowTop;
    s32 windowRight;
    s32 windowBottom;
    s32 mirrorShiftX;
    s32 mirrorShiftY;
    DrawEnvRecord *record;
    WarpVertex *vertex;
    WarpVertex *firstVertex;
    s32 *packetWords;
    u32 randomRange;
    f32 *scaleCursor;
    s32 vertexCount;
    s32 scaleIndex;
    s32 i;
    s32 j;

    mirrorShiftX = 0;
    mirrorShiftY = 0;
    alpha = params->alpha;
    step = params->step;
    originX = params->originX;
    originY = params->originY;
    enabled = params->enabled;
    mode = params->mode;
    movieActive = params->movieActive;
    camera = xglStudioSelectGetActiveCamera(0);
    windowLeft = camera->screen.window.vector.x;
    windowTop = camera->screen.window.vector.y;
    windowRight = camera->screen.window.vector.z;
    windowBottom = camera->screen.window.vector.w;
    if (step < 4) {
        step = 4;
    }
    if (enabled == 0) {
        return;
    }
    if (xglFilterShouldSkip() != 0) {
        return;
    }
    randomRange = 11U;
    scaleCursor = gXglWarpRandomScales;
    for (scaleIndex = 511; scaleIndex >= 0; scaleIndex--) {
        *scaleCursor = ((s32) (xglSRand() % randomRange)) * 0.01f + 0.9;
        scaleCursor++;
    }
    scaleIndex = 0;

    /* The origin is pulled back inside the window, mirrored across its edge. */
    if (originX < windowLeft) {
        s32 mirroredOriginX = windowLeft - (windowRight - windowLeft);

        mirrorShiftX = 0x100;
        if (originX < mirroredOriginX) {
            originX = mirroredOriginX;
        }
    }
    if (windowRight < originX) {
        s32 mirroredOriginX = windowRight + (windowRight - windowLeft);

        mirrorShiftX = 0;
        if (mirroredOriginX < originX) {
            originX = mirroredOriginX;
        }
    }
    if (originY < windowTop) {
        s32 mirroredOriginY = windowTop - (windowBottom - windowTop);

        mirrorShiftY = 0xE0;
        if (originY < mirroredOriginY) {
            originY = mirroredOriginY;
        }
    }
    if (windowBottom < originY) {
        s32 mirroredOriginY = windowBottom + (windowBottom - windowTop);

        mirrorShiftY = 0;
        if (mirroredOriginY < originY) {
            originY = mirroredOriginY;
        }
    }

    xglPacketSendFixedHeader();
    if (movieActive != 0) {
        if (xglRenderMovieIsActive() != 0) {
            DRAW_ENV_SET_BLOCK(record, 0, 0, 0x100, 0xE0, 0x100, 0xE0);
            func_001D5738();
            DRAW_ENV_SET_BLOCK(record, 0x100, 0, 0x200, 0xE0, 0x100, 0xE0);
            func_001D5738();
            DRAW_ENV_SET_BLOCK(record, 0, 0xE0, 0x100, 0x1C0, 0x100, 0xE0);
            func_001D5738();
            DRAW_ENV_SET_BLOCK(record, 0x100, 0xE0, 0x200, 0x1C0, 0x100, 0xE0);
            func_001D5738();
        } else {
            DRAW_ENV_SET_BLOCK(record, 0, 0, 0x100, 0xE0, 0x200, 0x1C0);
            drawEnvSetFieldArea();
            DRAW_ENV_SET_BLOCK(record, 0x100, 0, 0x200, 0xE0, 0x200, 0x1C0);
            drawEnvSetFieldArea();
            DRAW_ENV_SET_BLOCK(record, 0, 0xE0, 0x100, 0x1C0, 0x200, 0x1C0);
            drawEnvSetFieldArea();
            DRAW_ENV_SET_BLOCK(record, 0x100, 0xE0, 0x200, 0x1C0, 0x200, 0x1C0);
            drawEnvSetFieldArea();
        }
    } else {
        DRAW_ENV_SET_BLOCK(record, 0, 0, 0x100, 0xE0, 0x200, 0x1C0);
        func_001D5398();
        DRAW_ENV_SET_BLOCK(record, 0x100, 0, 0x200, 0xE0, 0x200, 0x1C0);
        func_001D5398();
        DRAW_ENV_SET_BLOCK(record, 0, 0xE0, 0x100, 0x1C0, 0x200, 0x1C0);
        func_001D5398();
        DRAW_ENV_SET_BLOCK(record, 0x100, 0xE0, 0x200, 0x1C0, 0x200, 0x1C0);
        func_001D5398();
    }

    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(windowLeft, windowRight - 1, windowTop, windowBottom - 1));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    switch (mode) {
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, enabled));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, enabled));
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, enabled));
        break;
    }
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_TEST_1, 0x50000);
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);

    /* Vertex 0 is the centre; the packet tag takes the first 16 bytes of scratchpad. */
    vertex = (WarpVertex *) (XGL_SCRATCHPAD + 0x10);
    vertexCount = 1;
    vertex->u = (mirrorShiftX * 2 + originX) * 8;
    vertex->v = (mirrorShiftY * 2 + originY) * 8;
    vertex->unmodeled_08 = 0;
    vertex->unmodeled_0C = 0;
    vertex->x = originX * 16 + 0x6FF8;
    vertex->y = originY * 16 + 0x71F7;
    vertex->alpha = alpha;
    vertex->unmodeled_1C = 0;
    vertex++;

    /* Top edge, left to right. */
    for (i = 0; i < WARP_SCREEN_WIDTH; i += step) {
        f32 scale = gXglWarpRandomScales[scaleIndex];
        u32 vertexU;
        u32 vertexV;

        scaleIndex++;
        vertexU = (u32) (((f32) originX + (f32) (i - originX) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftX * 16));
        vertex->u = vertexU;
        vertexV = (u32) (((f32) originY + (f32) (-originY) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftY * 16));
        vertex->v = vertexV;
        vertex->unmodeled_08 = 0;
        vertex->unmodeled_0C = 0;
        vertex->x = i * 16 + 0x6FF8;
        vertex->y = 0x71F7;
        vertex->alpha = alpha;
        vertex->unmodeled_1C = 0;
        vertex++;
        vertexCount++;
    }

    /* Right edge, top to bottom. */
    for (j = 0; j < WARP_SCREEN_HEIGHT; j += step) {
        f32 scale = gXglWarpRandomScales[scaleIndex];
        u32 vertexU;
        u32 vertexV;

        scaleIndex++;
        vertexU = (u32) (((f32) originX + (f32) (WARP_SCREEN_WIDTH - originX) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftX * 16));
        vertex->u = vertexU;
        vertexV = (u32) (((f32) originY + (f32) (j - originY) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftY * 16));
        vertex->v = vertexV;
        vertex->unmodeled_08 = 0;
        vertex->unmodeled_0C = 0;
        vertex->x = 0x8FF8;
        vertex->y = j * 16 + 0x71F7;
        vertex->alpha = alpha;
        vertex->unmodeled_1C = 0;
        vertex++;
        vertexCount++;
    }

    /* Bottom edge, right to left. */
    for (i = WARP_SCREEN_WIDTH; i > 0; i -= step) {
        f32 scale = gXglWarpRandomScales[scaleIndex];
        u32 vertexU;
        u32 vertexV;

        scaleIndex++;
        vertexU = (u32) (((f32) originX + (f32) (i - originX) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftX * 16));
        vertex->u = vertexU;
        vertexV = (u32) (((f32) originY + (f32) (WARP_SCREEN_HEIGHT - originY) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftY * 16));
        vertex->v = vertexV;
        vertex->unmodeled_08 = 0;
        vertex->unmodeled_0C = 0;
        vertex->x = i * 16 + 0x6FF8;
        vertex->y = 0x8DF7;
        vertex->alpha = alpha;
        vertex->unmodeled_1C = 0;
        vertex++;
        vertexCount++;
    }

    /* Left edge, bottom to top. */
    for (j = WARP_SCREEN_HEIGHT - step; j > 0; j -= step) {
        f32 scale = gXglWarpRandomScales[scaleIndex];
        u32 vertexU;
        u32 vertexV;

        scaleIndex++;
        vertexU = (u32) (((f32) originX + (f32) (-originX) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftX * 16));
        vertex->u = vertexU;
        vertexV = (u32) (((f32) originY + (f32) (j - originY) * scale) * 16.0f * 0.5f + (f32) (mirrorShiftY * 16));
        vertex->v = vertexV;
        vertex->unmodeled_08 = 0;
        vertex->unmodeled_0C = 0;
        vertex->x = 0x6FF8;
        vertex->y = j * 16 + 0x71F7;
        vertex->alpha = alpha;
        vertex->unmodeled_1C = 0;
        vertex++;
        vertexCount++;
    }

    /* The strip closes on the first edge vertex (vertex 1). */
    vertexCount++;
    packetWords = (s32 *) XGL_SCRATCHPAD;
    packetWords[0] = gXglWarpGifTag[0] | vertexCount;
    packetWords[1] = gXglWarpGifTag[1];
    packetWords[2] = gXglWarpGifTag[2];
    packetWords[3] = gXglWarpGifTag[3];
    packetWords += 4;
    /* The closing vertex repeats vertex 1, read as words. */
    vertex->u = packetWords[8];
    vertex->v = packetWords[9];
    vertex->unmodeled_08 = packetWords[10];
    vertex->unmodeled_0C = packetWords[11];
    vertex->x = packetWords[12];
    vertex->y = packetWords[13];
    vertex->alpha = packetWords[14];
    vertex->unmodeled_1C = packetWords[15];

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD, vertexCount * 2 + 1);
    sceVif1PkCloseDirectHLCode(packet);
}

typedef struct RadialBlur {
    s32 unmodeled_00;
    s32 depth;       /* +0x04 */
    s32 centerX;     /* +0x08 */
    s32 centerY;     /* +0x0C */
    s32 radius;      /* +0x10 */
    s32 sectorCount; /* +0x14 */
    s32 stretch;     /* +0x18: rim stretch range, in 1/10000 */
    u8 red;          /* +0x1C */
    u8 unmodeled_1D[3];
    u8 green; /* +0x20 */
    u8 unmodeled_21[3];
    u8 blue; /* +0x24 */
    u8 unmodeled_25[3];
    u8 alpha; /* +0x28 */
    u8 unmodeled_29[3];
    s32 blendMode; /* +0x2C */
    s32 field;     /* +0x30: nonzero copies through the field buffers */
} RadialBlur;

/* A packet quadword moved as one 128-bit value. */
typedef unsigned int PacketQuadword __attribute__((mode(TI)));

#define RADIAL_BLUR_PACKET ((PacketEntry *) XGL_SCRATCHPAD)
#define RADIAL_BLUR_ENTRY(index) ((s64 *) (RADIAL_BLUR_PACKET + (index)))

f32 xglCos(f32 angle);

static inline void radialBlurCopySet(s32 blendFix, s32 blendMode, s32 left, s32 top, s32 right,
                                     s32 bottom) {
    DrawEnvRecord *record = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();

    record->blendFix = blendFix;
    record->blendMode = blendMode;
    record->sourceRect[0] = left;
    record->sourceRect[1] = top;
    record->sourceRect[2] = right;
    record->sourceRect[3] = bottom;
    record->destinationRect[0] = 0x100;
    record->destinationRect[1] = 0;
    record->destinationRect[2] = 0x200;
    record->destinationRect[3] = 0x100;
}

/* Screen filter 38, WAVY BLUR2 in the game's filter name list.
 * Radial blur: copies the current frame into the work buffer under a blended
 * quad, then builds a fan of rim vertices around the blur centre and sends it
 * as one GS packet. */
void filterWavyBlur2Draw(RadialBlur *blur) {
    s32 depth;
    s32 centerX;
    s32 centerY;
    s32 radius;
    s32 sectorCount;
    s32 stretch;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 blendMode;
    s32 field;
    s32 windowLeft;
    s32 windowTop;
    s32 windowRight;
    s32 windowBottom;
    s32 texelU;
    s32 texelV;
    s32 clampedSectorCount;
    s32 sectorIndex;
    s32 texelOrigin;
    PacketEntry *packet;
    PacketEntry *firstRimEntry;
    PacketEntry *entry;
    s32 fixedCenterX;
    s32 fixedCenterY;
    XglCamera *camera;
    f32 stretchScale;
    u32 packetSize;

    depth = blur->depth;
    centerX = blur->centerX;
    centerY = blur->centerY;
    radius = blur->radius;
    sectorCount = blur->sectorCount;
    stretch = blur->stretch;
    red = blur->red;
    green = blur->green;
    blue = blur->blue;
    alpha = blur->alpha;
    blendMode = blur->blendMode;
    field = blur->field;
    camera = xglStudioSelectGetActiveCamera(0);
    windowLeft = camera->screen.window.vector.x;
    windowTop = camera->screen.window.vector.y;
    windowRight = camera->screen.window.vector.z;
    windowBottom = camera->screen.window.vector.w;
    if (xglFilterShouldSkip()) {
        return;
    }
    xglPacketSendFixedHeader();
    if (field) {
        if (xglRenderMovieIsActive()) {
            radialBlurCopySet(0x80, 3, windowLeft / 2, windowTop / 2, windowRight / 2, windowBottom / 2);
            func_001D5738();
        } else {
            radialBlurCopySet(0x80, 3, windowLeft, windowTop, windowRight, windowBottom);
            drawEnvSetFieldArea();
        }
    } else {
        radialBlurCopySet(0x80, 3, windowLeft, windowTop, windowRight, windowBottom);
        func_001D5398();
    }

    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(windowLeft, windowRight - 1, windowTop, windowBottom - 1));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0x101, 0x1FF, 0x101, 0x1FF));
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    switch (blendMode) {
    case 3:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, (s64) alpha));
        break;
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 0, 1, 0x80));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 0, 1, 0x80));
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0x80));
        break;
    }
    xglPacketAppendQuad(GS_TEX0_1,
                        GS_SET_TEX0(D_004C8390.workBase * 32 - 0x800, 8, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_TEST_1, 0x50000);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);

    fixedCenterX = centerX << 4;
    fixedCenterY = centerY << 4;
    /* kept: both field arms store 0x1000; the branch decides the emitted code. */
    if (field) {
        texelOrigin = 0x1000;
    } else {
        texelOrigin = 0x1000;
    }
    packet = RADIAL_BLUR_PACKET;
    texelU = interpolateLinearFixed16(centerX, windowLeft, windowRight, 0, 0x100) + texelOrigin;
    texelV = interpolateLinearFixed16(centerY, windowTop, windowBottom, 0, 0x100) + texelOrigin;
    xglPacketSetEntryWords(RADIAL_BLUR_PACKET, GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketSetEntryQuad(RADIAL_BLUR_ENTRY(1), GS_PRIM, 0x15D);
    xglPacketSetEntryQuad(RADIAL_BLUR_ENTRY(2), GS_RGBAQ,
                          GS_SET_RGBAQ(red, green, blue, alpha, XGL_FLOAT_ONE_BITS));
    xglPacketSetEntryQuad(RADIAL_BLUR_ENTRY(3), GS_UV, GS_SET_UV((s64) texelU, (s64) texelV));
    xglPacketSetEntryQuad(RADIAL_BLUR_ENTRY(4), GS_XYZF2,
                          GS_SET_XYZ(fixedCenterX + 0x6FF8, fixedCenterY + 0x71F7, depth));
    xglPacketSetEntryQuad(RADIAL_BLUR_ENTRY(5), GS_RGBAQ,
                          GS_SET_RGBAQ(red, green, blue, 0, XGL_FLOAT_ONE_BITS));
    entry = RADIAL_BLUR_PACKET + 6;
    firstRimEntry = entry;

    stretchScale = stretch ? (f32) stretch / 10000.0f : 0.0f;
    if (sectorCount <= 500) {
        clampedSectorCount = sectorCount;
        if (clampedSectorCount < 3) {
            clampedSectorCount = 3;
        }
    } else {
        clampedSectorCount = 500;
    }
    radius <<= 4;
    windowLeft <<= 4;
    windowRight <<= 4;
    windowTop <<= 4;
    windowBottom <<= 4;
    for (sectorIndex = 0; sectorIndex < clampedSectorCount; sectorIndex++) {
        f32 angle;
        f32 cosine;
        f32 sine;
        f32 rimScale;
        s32 rimX;
        s32 rimY;
        s32 rimU;
        s32 rimV;
        s32 randomStretch;

        angle = 6.283184f / (f32) clampedSectorCount * (f32) sectorIndex;
        sine = xglSin(angle);
        cosine = xglCos(angle);
        randomStretch = (s32) xglSRand() % (__builtin_abs(stretch) + 1);
        rimScale = (f32) randomStretch * stretchScale + 1.0f;
        rimX = (f32) radius * cosine;
        rimY = (f32) radius * sine;
        texelU = (f32) rimX * rimScale;
        texelV = (f32) rimY * rimScale;
        texelV += fixedCenterY;
        rimX += fixedCenterX;
        rimY += fixedCenterY;
        rimU = interpolateLinear(texelU + fixedCenterX, windowLeft, windowRight, 0, 0x1000);
        rimV = interpolateLinear(texelV, windowTop, windowBottom, 0, 0x1000);
        xglPacketSetEntryQuad((s64 *) entry, GS_UV, GS_SET_UV((s64) (rimU + 0x1000), (s64) (rimV + 0x1000)));
        entry++;
        xglPacketSetEntryQuad((s64 *) entry, GS_XYZF2, GS_SET_XYZ(rimX + 0x6FF8, rimY + 0x71F7, depth));
        entry++;
    }

    /* Close the fan with the first rim vertex. */
    *(PacketQuadword *) entry = *(PacketQuadword *) firstRimEntry;
    entry++;
    firstRimEntry++;
    *(PacketQuadword *) entry = *(PacketQuadword *) firstRimEntry;
    entry++;
    packetSize = (u8 *) entry - (u8 *) packet;
    packet->word0 = (packet->word0 & 0x8000) | ((packetSize - 0x10) >> 4);
    xglPacketSendDirect((s32) packet, packetSize >> 4);
}


#define SLOT_SIZE 0x54

typedef struct SlotControlBlock {
    u8 hookIndex; /* +0x00: index of the release hook in gSlotReleaseHooks */
    u8 unk01;
    u8 allocIndex; /* +0x02 */
    u8 slotId;     /* +0x03: key compared by func_001E7890, 0 when the record is free */
    s32 issuedCount;
    u8 unmodeled_08[0x40 - 0x08];
    s32 doneCount;
    u8 unmodeled_44[0x50 - 0x44];
    u32 controlFlags; /* +0x50 */
} SlotControlBlock;

static SlotControlBlock gFixedSlots[5];

typedef struct {
    u8 p[0x60];
    f32 win[4];
} CAM;

typedef struct {
    s32 unmodeled_00;
    s32 depth;
    s32 offsetX;
    s32 offsetY;
    s32 unmodeled_10[2];
    s32 alpha;
    s32 mode;
    s32 field;
} ScreenQuadRecord;

/* One VIF direct vertex: window corner in fixed point, depth in the z word. */
typedef struct {
    s32 u;
    s32 v;
    s32 unmodeled_08;
    s32 unmodeled_0C;
    s32 x;
    s32 y;
    s32 depth;
    s32 unmodeled_1C;
} QuadVertex;

void drawEnvSetHalfDisplayNext(void);
void drawEnvSetField(void);

static inline void xglPacketBeginGif(void) {
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
}

static inline void xglPacketFinishAndSend(void) {
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

#define SET4(p,a,b,c,d) ((s32*)(p))[0]=(a); ((s32*)(p))[1]=(b); ((s32*)(p))[2]=(c); ((s32*)(p))[3]=(d);

/* Quad vertex slots follow the 16-byte tag, 0x20 bytes apart. */
#define QUAD_VERTEX_SLOT(slot) ((QuadVertex *) (XGL_SCRATCHPAD + 0x10 + (slot) * 0x20))

/* Screen filter 14, DIR BLUR in the game's filter name list.
 * Screen quad: draws the frame through a blended quad covering the window,
 * shifted by the record's offset and placed at its depth. */
void filterDirBlurDraw(ScreenQuadRecord *quad, s32 packet) {
    s32 depth = quad->depth;
    s32 offsetX = quad->offsetX;
    s32 offsetY = quad->offsetY;
    s32 alpha = quad->alpha;
    s32 mode = quad->mode;
    s32 field = quad->field;
    CAM *camera = xglStudioSelectGetActiveCamera(0);
    s32 windowLeft;
    s32 windowTop;
    s32 windowRight;
    s32 windowBottom;
    s32 offsetXY[2];
    QuadVertex *vertex;

    windowLeft = camera->win[0];
    windowTop = camera->win[1];
    windowRight = camera->win[2];
    windowBottom = camera->win[3];
    if (xglFilterShouldSkip()) {
        return;
    }
    if (((u8 *) gFixedSlots)[3 * SLOT_SIZE + 1] == 0) {
        if (xglRenderMovieIsActive()) {
            if (field) {
                drawEnvSetHalfDisplayNext();
            } else {
                drawEnvSetLarge();
            }
        } else {
            if (field) {
                drawEnvSetField();
            } else {
                drawEnvSetLarge();
            }
        }
    } else {
        if (field) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }
    }
    offsetXY[0] = offsetX;
    offsetXY[1] = -offsetY;
    xglPacketBeginGif();
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_CLAMP_1, 1);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    switch (mode) {
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, alpha));
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, alpha));
        break;
    }
    xglPacketAppendQuad(GS_TEX0_1, (D_004C8390.workBase * 32) | 0x24020000 | ((s64) 0xC800 << 19));
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_TEST_1, 0x50000);
    xglPacketFinishAndSend();
    vertex = (QuadVertex *) XGL_SCRATCHPAD;
    SET4(vertex, 0x8000, 0x20AE4000, 0x53, 0)
    vertex = QUAD_VERTEX_SLOT(0);
    vertex->u = windowLeft * 16;
    vertex->v = windowTop * 16;
    vertex->unmodeled_08 = 0;
    vertex->unmodeled_0C = 0;
    vertex->x = (windowLeft + offsetXY[0]) * 16 + 0x6FF8;
    vertex->y = (windowTop + offsetXY[1]) * 16 + 0x71F7;
    vertex->depth = depth;
    vertex->unmodeled_1C = 0;
    vertex = QUAD_VERTEX_SLOT(1);
    vertex->u = windowRight * 16;
    vertex->v = windowTop * 16;
    vertex->unmodeled_08 = 0;
    vertex->unmodeled_0C = 0;
    vertex->x = (windowRight + offsetXY[0]) * 16 + 0x6FF8;
    vertex->y = (windowTop + offsetXY[1]) * 16 + 0x71F7;
    vertex->depth = depth;
    vertex->unmodeled_1C = 0;
    vertex = QUAD_VERTEX_SLOT(2);
    vertex->u = windowLeft * 16;
    vertex->v = windowBottom * 16;
    vertex->unmodeled_08 = 0;
    vertex->unmodeled_0C = 0;
    vertex->x = (windowLeft + offsetXY[0]) * 16 + 0x6FF8;
    vertex->y = (windowBottom + offsetXY[1]) * 16 + 0x71F7;
    vertex->depth = depth;
    vertex->unmodeled_1C = 0;
    vertex = QUAD_VERTEX_SLOT(3);
    vertex->u = windowRight * 16;
    vertex->v = windowBottom * 16;
    vertex->unmodeled_08 = 0;
    vertex->unmodeled_0C = 0;
    vertex->x = (windowRight + offsetXY[0]) * 16 + 0x6FF8;
    vertex->y = (windowBottom + offsetXY[1]) * 16 + 0x71F7;
    vertex->depth = depth;
    vertex->unmodeled_1C = 0;
    SCRATCHPAD_WORD(0) = (SCRATCHPAD_WORD(0) & 0x8000) | 4;
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD, 9);
    sceVif1PkCloseDirectHLCode(packet);
}

typedef struct TwistRingFilter {
    s32 unk00;
    s32 depth;      /* +0x04: z of every vertex */
    s32 centerX;    /* +0x08 */
    s32 centerY;    /* +0x0C */
    s32 radiusX;    /* +0x10: inner radius */
    s32 radiusY;    /* +0x14 */
    s32 width;      /* +0x18: width of the ring */
    s32 twist;      /* +0x1C: largest twist, in degrees */
    s32 noise;      /* +0x20: range of the random radius offset */
    s32 red;        /* +0x24 */
    s32 green;      /* +0x28 */
    s32 blue;       /* +0x2C */
    s32 alpha;      /* +0x30 */
    s32 blendMode;  /* +0x34 */
    s32 halfHeight; /* +0x38: draws to the half-height page */
} TwistRingFilter;

typedef struct TwistRingVertex {
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 texelX;
    s32 texelY;
    s32 unk18;
    s32 unk1C;
    s32 gsX;
    s32 gsY;
    s32 depth;
    s32 unk2C;
} TwistRingVertex;

#define TWIST_RING_SLICES 36
#define TWIST_RING_SLICE_ANGLE 0.17453289f /* 10 degrees */
#define TWIST_RING_PI 3.1415927f
#define TWIST_RING_TWIST(phase, twist) (xglSin(phase) * (f32) (twist) / 180.0f * TWIST_RING_PI)

static inline void twistRingSetTag(PacketEntry *tag, s32 word0, s32 word1, s32 word2, s32 word3) {
    tag->word0 = word0;
    tag->word1 = word1;
    tag->word2 = word2;
    tag->word3 = word3;
}

static inline void twistRingSetCount(PacketEntry *tag, s32 count) {
    tag->word0 &= 0x8000;
    tag->word0 |= count;
}

static inline s32 twistRingTexel(s32 value, s32 low, s32 high) {
    s32 minimum = (low << 4) + 16;
    s32 result;

    if (value < minimum) {
        result = minimum;
    } else {
        s32 maximum = (high << 4) - 16;

        result = value;
        if (maximum < result) {
            result = maximum;
        }
    }
    return result;
}

static inline s32 twistRingGsCoordinate(s32 value, s32 low, s32 high, s32 bias) {
    if (low <= value) {
        return (((high < value) ? high : value) << 4) + bias;
    }
    return (low << 4) + bias;
}

static inline void twistRingSetCopy(s32 blendFix, s32 blendMode, s32 sourceLeft, s32 sourceTop,
                                    s32 sourceRight, s32 sourceBottom, s32 left, s32 top, s32 right,
                                    s32 bottom) {
    DrawEnvRecord *record = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();

    record->blendFix = blendFix;
    record->blendMode = blendMode;
    record->sourceRect[0] = sourceLeft;
    record->sourceRect[1] = sourceTop;
    record->sourceRect[2] = sourceRight;
    record->sourceRect[3] = sourceBottom;
    record->destinationRect[0] = left;
    record->destinationRect[1] = top;
    record->destinationRect[2] = right;
    record->destinationRect[3] = bottom;
}

/* Screen filter 13, TORUS in the game's filter name list. */
void filterTorusDraw(TwistRingFilter *filter, s32 vifPacket) {
    s32 depth;
    s32 centerX;
    s32 centerY;
    s32 radiusX;
    s32 radiusY;
    s32 width;
    s32 twist;
    s32 noise;
    s32 red;
    s32 green;
    s32 blue;
    s32 filterAlpha;
    s32 blendMode;
    s32 halfHeight;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 radiusNoise[TWIST_RING_SLICES + 2];
    s32 slice;
    s32 blurPass;

    depth = filter->depth;
    centerX = filter->centerX;
    centerY = filter->centerY;
    radiusX = filter->radiusX;
    radiusY = filter->radiusY;
    width = filter->width;
    twist = filter->twist;
    noise = filter->noise;
    red = filter->red;
    green = filter->green;
    blue = filter->blue;
    filterAlpha = filter->alpha;
    blendMode = filter->blendMode;
    halfHeight = filter->halfHeight;

    {
        XglCamera *camera = xglStudioSelectGetActiveCamera(0);

        left = (s32) camera->screen.window.vector.x;
        top = (s32) camera->screen.window.vector.y;
        right = (s32) camera->screen.window.vector.z;
        bottom = (s32) camera->screen.window.vector.w;
    }
    if (xglFilterShouldSkip()) {
        return;
    }
    if (noise < 0) {
        return;
    }
    xglPacketSendFixedHeader();
    if (xglRenderMovieIsActive()) {
        if (halfHeight != 0) {
            drawEnvSetHalfDisplayNext();
        } else {
            drawEnvSetLarge();
        }
    } else {
        if (halfHeight != 0) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }
    }

    for (blurPass = 0; blurPass < 3; blurPass++) {
        twistRingSetCopy(0x81, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
        func_001D54C8();
        gDrawArea.unk58 = GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2);
        gDrawArea.flags.pageOffset = depth;
        twistRingSetCopy(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
        func_001D57E0();
        gDrawArea.unk58 = GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
    }

    {
        TwistRingVertex *vertex;
        s32 vertexCount;
        f32 bandWidth;
        s32 bandCount = 3;
        s32 band;

        xglPacketBufferInit(XGL_SCRATCHPAD);
        xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
        switch (blendMode) {
        case 1:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 0, 1, 0));
            break;
        case 2:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 0, 1, 0));
            break;
        default:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0));
            break;
        }
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        vertex = (TwistRingVertex *) XGL_SCRATCHPAD;
        xglPacketAppendQuad(GS_TEX0_1,
                            GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
        xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(0, 511, 0, 447));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);
        drawEnvSetCameraScissor();

        bandWidth = (f32) width / (f32) bandCount;
        twistRingSetTag((PacketEntry *) vertex, 0x8000, 0x30AE4000, 0x531, 0);
        for (slice = 0; slice < TWIST_RING_SLICES; slice++) {
            if (noise != 0) {
                radiusNoise[slice] = ((s32 (*)(void)) xglSRand)() % noise;
            } else {
                radiusNoise[slice] = 0;
            }
        }
        radiusNoise[TWIST_RING_SLICES] = radiusNoise[0];
        radiusNoise[TWIST_RING_SLICES + 1] = radiusNoise[1];

        for (slice = 0; slice < TWIST_RING_SLICES; slice++) {
            f32 sine = xglSin((f32) slice * TWIST_RING_SLICE_ANGLE);
            f32 cosine = xglCos((f32) slice * TWIST_RING_SLICE_ANGLE);
            f32 nextSine = xglSin((f32) (slice + 1) * TWIST_RING_SLICE_ANGLE);
            f32 nextCosine = xglCos((f32) (slice + 1) * TWIST_RING_SLICE_ANGLE);

            vertexCount = 0;
            vertex = (TwistRingVertex *) (XGL_SCRATCHPAD + 0x10);
            for (band = 0; band <= bandCount; band++) {
                s32 alpha = filterAlpha;
                f32 bandRadiusX = (f32) (radiusX + width / 2) - (f32) (bandCount - band) * bandWidth;
                f32 bandRadiusY = (f32) (radiusY + width / 2) - (f32) (bandCount - band) * bandWidth;
                f32 phase = TWIST_RING_PI / (f32) bandCount * (f32) band;
                s32 x = (s32) ((bandRadiusX + (f32) radiusNoise[slice]) * cosine);
                s32 y = (s32) ((bandRadiusY + (f32) radiusNoise[slice]) * sine);
                s32 nextX = (s32) ((bandRadiusX + (f32) radiusNoise[slice + 1]) * nextCosine);
                s32 nextY = (s32) ((bandRadiusY + (f32) radiusNoise[slice + 1]) * nextSine);
                f32 twistSine = xglSin((f32) slice * TWIST_RING_SLICE_ANGLE + TWIST_RING_TWIST(phase, twist));
                f32 twistCosine =
                    xglCos((f32) slice * TWIST_RING_SLICE_ANGLE + TWIST_RING_TWIST(phase, twist));
                f32 nextTwistSine =
                    xglSin((f32) (slice + 1) * TWIST_RING_SLICE_ANGLE + TWIST_RING_TWIST(phase, twist));
                f32 nextTwistCosine =
                    xglCos((f32) (slice + 1) * TWIST_RING_SLICE_ANGLE + TWIST_RING_TWIST(phase, twist));
                s32 offsetX = (s32) (bandRadiusX * twistCosine) << 4;
                s32 offsetY = (s32) (bandRadiusY * twistSine) << 4;
                s32 nextOffsetX = (s32) (bandRadiusX * nextTwistCosine) << 4;
                s32 nextOffsetY = (s32) (bandRadiusY * nextTwistSine) << 4;

                x += centerX;
                y += centerY;
                nextX += centerX;
                nextY += centerY;
                if (band == 0 || band == bandCount) {
                    alpha = 0;
                }

                vertex->red = red;
                vertex->green = green;
                vertex->blue = blue;
                vertex->alpha = alpha;
                vertex->texelX = twistRingTexel((centerX << 4) + offsetX, left, right);
                vertex->texelY = twistRingTexel((centerY << 4) + offsetY, top, bottom);
                vertex->unk18 = 0;
                vertex->unk1C = 0;
                vertex->gsX = twistRingGsCoordinate(x, left, right, 0x6FF8);
                vertex->gsY = twistRingGsCoordinate(y, top, bottom, 0x71F7);
                vertex->depth = depth;
                vertex->unk2C = 0;
                vertex++;
                vertexCount++;
                vertex->red = red;
                vertex->green = green;
                vertex->blue = blue;
                vertex->alpha = alpha;
                vertex->texelX = twistRingTexel((centerX << 4) + nextOffsetX, left, right);
                vertex->texelY = twistRingTexel((centerY << 4) + nextOffsetY, top, bottom);
                vertex->unk18 = 0;
                vertex->unk1C = 0;
                vertex->gsX = twistRingGsCoordinate(nextX, left, right, 0x6FF8);
                vertex->gsY = twistRingGsCoordinate(nextY, top, bottom, 0x71F7);
                vertex->depth = depth;
                vertex->unk2C = 0;
                vertex++;
                vertexCount++;
            }

            twistRingSetCount((PacketEntry *) XGL_SCRATCHPAD, vertexCount);
            sceVif1PkCnt(vifPacket, 0);
            sceVif1PkOpenDirectHLCode(vifPacket, 0);
            sceVif1PkAddDirectDataN(
                vifPacket, XGL_SCRATCHPAD, vertexCount * 3 + 1);
            sceVif1PkCloseDirectHLCode(vifPacket);
        }
    }
}

void xglObjectRippleDraw(XglObject *object, s32 vifPacket);

/* Screen filter 17, RIPPLE in the game's filter name list. */
void filterRippleDraw(XglObject *object, s32 vifPacket) {
    object->modeId = 1;
    xglObjectRippleDraw(object, vifPacket);
}

/* Screen filter 18, RIPPLE2 in the game's filter name list. */
void filterRipple2Draw(XglObject *object, s32 vifPacket) {
    object->modeId = 2;
    xglObjectRippleDraw(object, vifPacket);
}

typedef struct RingRecord {
    s32 texelX;
    s32 texelY;
    s32 color0;
    s32 color1;
    s32 gsX;
    s32 gsY;
    s32 depth;
    s32 zero;
} RingRecord;

#define RING_SLICES 18
#define RING_SLICE_ANGLE 0.34906578f /* 20 degrees */
#define RING_TWO_PI 6.283184f
#define RING_HALF_PI 1.5707964f
#define RING_FAR 1024.0f

static inline void ringSetTag(RingRecord *tag, s32 word0, s32 word1, s32 word2, s32 word3) {
    tag->texelX = word0;
    tag->texelY = word1;
    tag->color0 = word2;
    tag->color1 = word3;
}

static inline void ringSetCount(RingRecord *tag, s32 count) {
    tag->texelX = (tag->texelX & 0x8000) | count;
}

static inline s32 ringClamp(s32 value, s32 low, s32 high) {
    s32 result;

    if (value < low) {
        result = low;
    } else {
        result = value;
        if (high < result) {
            result = high;
        }
    }
    return result;
}

static inline s32 ringGsCoordinate(s32 value, s32 low, s32 high, s32 bias) {
    if (low <= value) {
        return (((high < value) ? high : value) << 4) + bias;
    }
    return (low << 4) + bias;
}

void xglObjectRippleDraw(XglObject *object, s32 vifPacket) {
    s32 depth;
    s32 centerX;
    s32 centerY;
    s32 wavelength;
    s32 amplitude;
    s32 extentX;
    s32 extentY;
    s32 phaseStep;
    s32 blendAmount;
    s32 blendMode;
    s32 halfHeight;
    s32 modeId;
    s32 ringStep;
    s32 ringCount;

    depth = object->depth;
    centerX = object->centerX;
    centerY = object->centerY;
    wavelength = object->wavelength;
    amplitude = object->amplitude;
    extentX = object->extentX;
    extentY = object->extentY;
    phaseStep = object->phaseStep;
    blendAmount = object->blendAmount;
    blendMode = object->blendMode;
    halfHeight = object->halfHeight;
    modeId = object->modeId;
    ringStep = wavelength / 8;
    if (ringStep < 6) {
        ringStep = 6;
    }
    if (wavelength < ringStep + 1) {
        return;
    }
    ringCount = ((extentX < extentY) ? extentY : extentX) / ringStep;

    {
        s32 left;
        s32 right;
        s32 top;
        s32 bottom;
        float scaleX;
        float scaleY;
        s32 periodX;
        s32 periodY;
        s32 amplitudeX;
        s32 amplitudeY;
        s32 phaseStepX;
        s32 phaseStepY;
        s32 *phaseX;
        s32 *phaseY;
        s32 ring;
        float ringStepX;
        float ringStepY;
        s32 slice;
        RingRecord *cursor;
        s32 vertexCount;
        float angle;
        float nextAngle;
        float sine;
        float cosine;
        float nextSine;
        float nextCosine;

        {
            XglCamera *camera = xglStudioSelectGetActiveCamera(0);

            left = (s32) camera->screen.window.vector.x;
            top = (s32) camera->screen.window.vector.y;
            right = (s32) camera->screen.window.vector.z;
            bottom = (s32) camera->screen.window.vector.w;
        }
        scaleX = (extentX < extentY) ? (float) extentX / (float) extentY : 1.0f;
        scaleY = (extentY < extentX) ? (float) extentY / (float) extentX : 1.0f;
        periodX = (s32) ((float) wavelength * scaleX) << 5;
        periodY = (s32) ((float) wavelength * scaleY) << 5;
        phaseStepX = (s32) ((float) phaseStep * scaleX);
        phaseStepY = (s32) ((float) phaseStep * scaleY);
        amplitudeX = (s32) ((float) amplitude * scaleX);
        amplitudeY = (s32) ((float) amplitude * scaleY);

        phaseX = &object->phaseX;
        phaseY = &object->phaseY;
        if ((object->flags & 0x18) == 0) {
            *phaseX += phaseStepX;
            *phaseY += phaseStepY;
            if (*phaseX >= periodX) {
                *phaseX -= periodX;
            }
            if (*phaseX <= -periodX) {
                *phaseX += periodX;
            }
            if (*phaseY >= periodY) {
                *phaseY -= periodY;
            }
            if (*phaseY <= -periodY) {
                *phaseY += periodY;
            }
        }

        if (xglFilterShouldSkip()) {
            return;
        }
        xglPacketSendFixedHeader();
        if (xglRenderMovieIsActive() != 0) {
            if (halfHeight != 0) {
                drawEnvSetHalfDisplayNext();
            } else {
                drawEnvSetLarge();
            }
        } else {
            if (halfHeight != 0) {
                drawEnvSetField();
            } else {
                drawEnvSetLarge();
            }
        }

        xglPacketBufferInit(XGL_SCRATCHPAD);
        xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(left, right - 1, top, bottom - 1));
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_CLAMP_1, 0);
        switch (blendMode) {
        case 1:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, blendAmount));
            break;
        case 2:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, blendAmount));
            break;
        default:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, blendAmount));
            break;
        }
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        cursor = (RingRecord *) XGL_SCRATCHPAD;
        xglPacketAppendQuad(GS_TEX0_1,
                            GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);

        ringStepX = (float) ringStep * scaleX;
        ringStepY = (float) ringStep * scaleY;
        ringSetTag(cursor, 0x8000, 0x20AE4000, 0x53, 0);

        for (slice = 0; slice < RING_SLICES; slice++) {
            angle = (float) slice * RING_SLICE_ANGLE;
            sine = xglSin(angle);
            cosine = xglCos(angle);
            nextAngle = (float) (slice + 1) * RING_SLICE_ANGLE;
            nextSine = xglSin(nextAngle);
            nextCosine = xglCos(nextAngle);

            vertexCount = 0;
            cursor = (RingRecord *) (XGL_SCRATCHPAD + 0x10);
            for (ring = 0; ring <= ringCount; ring++) {
                s32 nextOffsetY;
                s32 offsetX;
                s32 offsetY;
                s32 nextOffsetX;
                s32 ringX;
                s32 nextRingX;
                s32 ringY;
                s32 nextRingY;
                float ringIndex;
                float radiusX;
                float radiusY;
                float sineX;
                float damping;
                float waveX;
                float sineY;
                float waveY;
                s32 maxTexelX;
                s32 maxTexelY;

                maxTexelX = (right << 4) - 16;
                maxTexelY = (bottom << 4) - 16;
                ringIndex = (float) ring;
                nextOffsetY = 0;
                nextOffsetX = 0;
                offsetY = 0;
                offsetX = 0;
                radiusX = ringIndex * ringStepX;
                radiusY = ringIndex * ringStepY;
                if (RING_FAR < radiusX) {
                    continue;
                }
                if (RING_FAR < radiusY) {
                    continue;
                }

                sineX = xglSin((RING_TWO_PI / (float) periodX) *
                               ((float) (-*phaseX) + radiusX * 16.0f + (float) periodX));
                damping = (RING_HALF_PI / (float) ringCount) * ringIndex;
                waveX = ((float) (amplitudeX << 4) * sineX) * xglCos(damping);
                sineY = xglSin((RING_TWO_PI / (float) periodY) *
                               ((float) (-*phaseY) + radiusY * 16.0f + (float) periodY));
                waveY = ((float) (amplitudeY << 4) * sineY) * xglCos(damping);

                switch (modeId) {
                case 1:
                default:
                    if (ring != 0) {
                        offsetX = (s32) (cosine * waveX);
                        offsetY = (s32) (sine * waveY);
                        nextOffsetX = (s32) (nextCosine * waveX);
                        nextOffsetY = (s32) (nextSine * waveY);
                    }
                    break;
                case 2:
                    nextOffsetY = (s32) waveY;
                    nextOffsetX = (s32) waveX;
                    offsetX = nextOffsetX;
                    offsetY = nextOffsetY;
                    break;
                }

                ringX = (s32) (radiusX * cosine);
                ringY = (s32) (radiusY * sine);
                nextRingX = (s32) (radiusX * nextCosine);
                nextRingY = (s32) (radiusY * nextSine);
                ringX += centerX;
                ringY += centerY;
                nextRingX += centerX;
                nextRingY += centerY;
                if (ringX < -ringStep || ringStep + 0x200 < ringX) {
                    if (nextRingX < -ringStep || ringStep + 0x200 < nextRingX) {
                        continue;
                    }
                }
                if (ringY < -ringStep || ringStep + 0x1C0 < ringY) {
                    if (nextRingY < -ringStep || ringStep + 0x1C0 < nextRingY) {
                        continue;
                    }
                }

                cursor->texelX = ringClamp((ringX << 4) + offsetX, (left << 4) + 16, maxTexelX);
                cursor->texelY = ringClamp((ringY << 4) + offsetY, (top << 4) + 16, maxTexelY);
                cursor->color0 = 0xFF;
                cursor->color1 = 0xFF;
                cursor->gsX = ringGsCoordinate(ringX, left, right, 0x6FF8);
                cursor->gsY = ringGsCoordinate(ringY, top, bottom, 0x71F7);
                cursor->depth = depth;
                cursor->zero = 0;
                cursor++;
                vertexCount++;
                cursor->texelX = ringClamp((nextRingX << 4) + nextOffsetX, (left << 4) + 16, maxTexelX);
                cursor->texelY = ringClamp((nextRingY << 4) + nextOffsetY, (top << 4) + 16, maxTexelY);
                cursor->color0 = 0xFF;
                cursor->color1 = 0xFF;
                cursor->gsX = ringGsCoordinate(nextRingX, left, right, 0x6FF8);
                cursor->gsY = ringGsCoordinate(nextRingY, top, bottom, 0x71F7);
                cursor->depth = depth;
                cursor->zero = 0;
                cursor++;
                vertexCount++;
            }

            if (vertexCount != 0) {
                ringSetCount((RingRecord *) XGL_SCRATCHPAD, vertexCount);
                sceVif1PkCnt(vifPacket, 0);
                sceVif1PkOpenDirectHLCode(vifPacket, 0);
                sceVif1PkAddDirectDataN(
                    vifPacket, XGL_SCRATCHPAD, vertexCount * 2 + 1);
                sceVif1PkCloseDirectHLCode(vifPacket);
            }
        }
    }
}

/* vertex words start after the 16-byte tag quadword at the scratchpad base */
#define SCRATCHPAD_VERTICES (XGL_SCRATCHPAD + 0x10)

typedef struct ScrollWaveFilter {
    u8 unmodeled_00[4];
    s32 vertexZ;   /* +0x04 */
    s32 size;      /* +0x08: wave period and amplitude scale */
    s32 speedX;    /* +0x0C */
    s32 speedY;    /* +0x10 */
    s32 alpha;     /* +0x14 */
    s32 blendMode; /* +0x18 */
    s32 field;     /* +0x1C */
    u8 unmodeled_20[0x20];
    s32 scrollX; /* +0x40 */
    s32 scrollY; /* +0x44 */
    u8 unmodeled_48[8];
    u32 flags; /* +0x50 */
} ScrollWaveFilter;

typedef struct WaveVertex {
    s32 s;
    s32 t;
    s32 u;
    s32 v;
    s32 x;
    s32 y;
    s32 z;
    s32 w;
} WaveVertex;

static inline s32 waveWrap(s32 value, s32 low, s32 high) {
    s32 scaled;

    if (value < low) {
        scaled = ((low - value) + low) * 16;
    } else {
        scaled = value * 16;
        if (high < value) {
            scaled = (high - value % high) * 16;
        }
    }
    return scaled;
}

static inline s32 waveWrapNext(s32 value, s32 low, s32 high) {
    if (value + 8 < low) {
        return ((low - (value + 8)) + low) * 16;
    }
    if (value + 8 <= high) {
        return (value + 8) * 16;
    }
    return (high - (value + 8) % high) * 16;
}

/* Screen filter 19, DISTORT in the game's filter name list.
 * Scrolling wave filter: advances the scroll unless flags bits 3 and 4 freeze it, then draws the
 * screen as strips whose texture coordinates follow two sine tables. */
void filterDistortDraw(ScrollWaveFilter *filter, s32 packet) {
    s32 vertexZ;
    s32 waveSize;
    s32 period;
    s32 speedX;
    s32 speedY;
    s32 alpha;
    s32 blendMode;
    s32 field;
    s32 *scrollX;
    s32 *scrollY;
    XglCamera *camera;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 columnWave[65 * 8];
    s32 rowWave[57 * 8];
    WaveVertex *vertex;
    s32 vertexCount;
    s32 x;
    s32 y;
    s32 nextY;

    waveSize = filter->size;
    vertexZ = filter->vertexZ;
    period = waveSize * 32;
    scrollX = &filter->scrollX;
    scrollY = &filter->scrollY;
    speedX = filter->speedX;
    speedY = filter->speedY;
    alpha = filter->alpha;
    blendMode = filter->blendMode;
    field = filter->field;
    camera = xglStudioSelectGetActiveCamera(0);
    left = camera->screen.window.vector.x;
    top = camera->screen.window.vector.y;
    right = camera->screen.window.vector.z;
    bottom = camera->screen.window.vector.w;
    if ((filter->flags & 0x18) == 0) { /* bits 3 and 4: frozen */
        *scrollX += speedX;
        *scrollY += speedY;
        if (*scrollX >= period) {
            *scrollX -= period;
        }
        if (*scrollY >= period) {
            *scrollY -= period;
        }
    }
    if (xglFilterShouldSkip()) {
        return;
    }
    if (xglRenderMovieIsActive()) {
        if (field) {
            drawEnvSetHalfDisplayNext();
        } else {
            drawEnvSetLarge();
        }
    } else {
        if (field) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }
    }
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    switch (blendMode) {
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, alpha));
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, alpha));
        break;
    }
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEST_1, 0x50000);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
    for (x = 0; x < 513; x += 8) {
        columnWave[x] = (s32) ((f32) (waveSize / 2) * xglSin((f32) (x + (*scrollX >> 5)) * 6.283184f / (f32) waveSize));
    }
    for (y = 0; y < 449; y += 8) {
        rowWave[y] = (s32) ((f32) (waveSize / 2) * xglSin((f32) (y + (*scrollY >> 5)) * 6.283184f / (f32) waveSize));
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    ((s32 *) XGL_SCRATCHPAD)[0] = 0x8000;
    ((s32 *) XGL_SCRATCHPAD)[1] = 0x20AE4000;
    ((s32 *) XGL_SCRATCHPAD)[2] = 0x53;
    ((s32 *) XGL_SCRATCHPAD)[3] = 0;
    for (y = 0; y < 448; y = nextY) {
        nextY = y + 8;
        vertex = (WaveVertex *) SCRATCHPAD_VERTICES;
        vertexCount = 0;
        for (x = 0; x < 513; x += 8) {
            vertex->s = waveWrap(x + columnWave[x], left, right);
            vertex->t = waveWrap(y + rowWave[y], top, bottom);
            vertex->u = 255;
            vertex->v = 255;
            vertex->x = x;
            vertex->x <<= 4;
            vertex->x += 0x6FF8;
            vertex->y = y * 16 + 0x71F7;
            vertex->z = vertexZ;
            vertex->w = 0;
            vertex++;
            vertexCount++;
            vertex->s = waveWrap(x + columnWave[x], left, right);
            vertex->t = waveWrapNext(y + rowWave[y + 8], top, bottom);
            vertex->u = 255;
            vertex->v = 255;
            vertex->x = x;
            vertex->x <<= 4;
            vertex->x += 0x6FF8;
            vertex->y = nextY * 16 + 0x71F7;
            vertex->z = vertexZ;
            vertex->w = 0;
            vertex++;
            vertexCount++;
        }
        SCRATCHPAD_WORD(0) |= vertexCount;
        sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD, vertexCount * 2 + 1);
    }
    sceVif1PkCloseDirectHLCode(packet);
}

typedef struct GlobalScrollWaveFilter {
    u8 unmodeled_00[4];
    s32 depth;      /* +0x04: the z word of each vertex */
    s32 wavelength; /* +0x08: in pixels; the scroll wraps at wavelength * 32 */
    s32 amplitude;  /* +0x0C: in pixels */
    s32 speedX;     /* +0x10: scroll step, 1/32 pixel */
    s32 speedY;     /* +0x14 */
    s32 alpha;      /* +0x18: the high word of the alpha register */
    s32 blendMode;  /* +0x1C: 1 and 2 select their own blend codes */
    s32 field;      /* +0x20: nonzero draws the half-height variant */
    u8 unmodeled_24[0x50 - 0x24];
    u32 unk50_0 : 3;
    u32 stopped : 1; /* +0x50: bits 3 and 4 freeze the scroll */
    u32 paused : 1;
    u32 unk50_5 : 27;
} GlobalScrollWaveFilter;

#define WAVE_PACKET ((GsWords *) XGL_SCRATCHPAD)

static inline void packetWordsSet(GsWords *words, s32 w0, s32 w1, s32 w2, s32 w3) {
    words->w0 = w0;
    words->w1 = w1;
    words->w2 = w2;
    words->w3 = w3;
}

extern s32 gXglWaveScrollX;
extern s32 gXglWaveScrollY;

#define WAVE_COORDINATE(coordinate, near, far) \
    (((coordinate) < (near) ? (near) - (coordinate) + (near) \
      : (coordinate) <= (far) ? (coordinate) : (far) - (coordinate) % (far)) * 16)

/* Screen filter 20, SWELL in the game's filter name list.
 * Wave filter like filterDistortDraw, but its scroll is kept in the globals gXglWaveScrollX (x) and gXglWaveScrollY (y)
 * and the wavelength is the scroll period. */
void filterSwellDraw(GlobalScrollWaveFilter *filter, s32 packet) {
    s32 shiftByColumn[520];
    s32 shiftByRow[456];
    XglCamera *camera;
    s32 depth;
    s32 wavelength;
    s32 amplitude;
    s32 speedX;
    s32 speedY;
    s32 alpha;
    s32 blendMode;
    s32 field;
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
    s32 scrollLimit;
    s32 x;
    s32 y;
    s32 vertexCount;
    GsWords *entry;

    depth = filter->depth;
    wavelength = filter->wavelength;
    scrollLimit = wavelength << 5;
    amplitude = filter->amplitude;
    speedX = filter->speedX;
    speedY = filter->speedY;
    alpha = filter->alpha;
    blendMode = filter->blendMode;
    field = filter->field;
    camera = xglStudioSelectGetActiveCamera(0);
    left = camera->screen.window.vector.x;
    top = camera->screen.window.vector.y;
    right = camera->screen.window.vector.z;
    bottom = camera->screen.window.vector.w;

    if (!(filter->stopped || filter->paused)) {
        gXglWaveScrollX += speedX;
        gXglWaveScrollY += speedY;
        if (gXglWaveScrollX >= scrollLimit) {
            gXglWaveScrollX -= scrollLimit;
        }
        if (gXglWaveScrollY >= scrollLimit) {
            gXglWaveScrollY -= scrollLimit;
        }
    }
    if (xglFilterShouldSkip()) {
        return;
    }
    if (xglRenderMovieIsActive()) {
        if (field) {
            drawEnvSetHalfDisplayNext();
        } else {
            drawEnvSetLarge();
        }
    } else {
        if (field) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }
    }

    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    switch (blendMode) {
    case 1:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha));
        break;
    case 2:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, alpha));
        break;
    default:
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, alpha));
        break;
    }
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEST_1, 0x50000);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);

    for (x = 0; x <= 512; x += 8) {
        shiftByColumn[x] = (f32) amplitude * xglSin((f32) (x + (gXglWaveScrollX >> 5)) * 6.283184f / (f32) wavelength);
    }
    for (y = 0; y <= 448; y += 8) {
        shiftByRow[y] =
            (f32) amplitude * xglSin((f32) (y + (gXglWaveScrollY >> 5)) * 6.283184f / (f32) wavelength);
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    packetWordsSet(WAVE_PACKET, GIF_TAG_EOP, 0x20AE4000, 0x53, 0);
    for (y = 0; y < 448; y += 8) {
        entry = WAVE_PACKET + 1;
        vertexCount = 0;
        for (x = 0; x <= 512; x += 8) {
            entry->w0 = WAVE_COORDINATE(x + shiftByRow[y], left, right);
            entry->w1 = WAVE_COORDINATE(y + shiftByColumn[x], top, bottom);
            entry->w2 = 0xFF;
            entry->w3 = 0xFF;
            entry[1].w0 = (x << 4) + 0x6FF8;
            entry[1].w1 = (y << 4) + 0x71F7;
            entry[1].w2 = depth;
            entry[1].w3 = 0;
            entry += 2;
            vertexCount++;
            entry->w0 = WAVE_COORDINATE(x + shiftByRow[y + 8], left, right);
            entry->w1 = WAVE_COORDINATE(y + shiftByColumn[x] + 8, top, bottom);
            entry->w2 = 0xFF;
            entry->w3 = 0xFF;
            entry[1].w0 = (x << 4) + 0x6FF8;
            entry[1].w1 = ((y + 8) << 4) + 0x71F7;
            entry[1].w2 = depth;
            entry[1].w3 = 0;
            entry += 2;
            vertexCount++;
        }
        WAVE_PACKET->w0 |= vertexCount;
        sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD, vertexCount * 2 + 1);
    }
    sceVif1PkCloseDirectHLCode(packet);
}

typedef struct FrameDrawParams {
    u8 unk00[4];
    s32 frameBase;
    s32 count;
    s32 blendMode;
    s32 enabled;
} FrameDrawParams;

static inline void drawRecordFill(DrawEnvRecord *record, s32 blendFix, s32 blendMode, s32 srcW, s32 srcH,
                                  s32 dstW, s32 dstH) {
    record->blendFix = blendFix;
    record->blendMode = blendMode;
    record->sourceRect[0] = 0;
    record->sourceRect[1] = 0;
    record->sourceRect[2] = srcW;
    record->sourceRect[3] = srcH;
    record->destinationRect[0] = 0;
    record->destinationRect[1] = 0;
    record->destinationRect[2] = dstW;
    record->destinationRect[3] = dstH;
}

/* Screen filter 21, DOF in the game's filter name list.
 * Frame pass: `count` (at most 10) rounds of a half-size fill and a blended copy back to full size;
 * `enabled` selects the variant that first prepares the work page. */
void filterDofDraw(FrameDrawParams *frameParams) {
    DrawEnvRecord *enabledShrinkRecord;
    DrawEnvRecord *enabledExpandRecord;
    DrawEnvRecord *shrinkRecord;
    DrawEnvRecord *expandRecord;
    DrawEnvRecord *fullSizeRecord;
    s32 enabled;
    s32 frameBase;
    s32 count;
    s32 blendMode;
    s32 i;

    frameBase = frameParams->frameBase;
    count = frameParams->count;
    blendMode = frameParams->blendMode;
    enabled = frameParams->enabled;
    if (xglFilterShouldSkip()) {
        return;
    }
    if (count < 0) {
        return;
    }
    count = (count > 10) ? 10 : count;
    xglPacketSendFixedHeader();
    drawEnvSetLarge();
    if (enabled != 0) {
        func_001D5CD0(D_004C8390.workBase, 0, 0, 0x200, 0x1C0);
        drawRectOnPageDepth(D_004C8390.workBase, frameBase, 0, 0, 0x200, 0x1C0);
        for (i = 0; i < count; i++) {
            enabledShrinkRecord = drawEnvGetRecord();
            drawRecordFill(enabledShrinkRecord, 0x80, 3, 0x200, 0x1C0, 0x100, 0xE0);
            func_001D54C8();
            gDrawArea.unk58 = 0x34000;
            enabledExpandRecord = drawEnvGetRecord();
            drawRecordFill(enabledExpandRecord, blendMode, 0, 0x100, 0xE0, 0x200, 0x1C0);
            func_001D57E0();
            gDrawArea.unk58 = 0x30000;
        }
    } else if (count > 0) {
        for (i = count; i != 0; i--) {
            shrinkRecord = drawEnvGetRecord();
            drawRecordFill(shrinkRecord, 0x81, 3, 0x200, 0x1C0, 0x100, 0xE0);
            func_001D54C8();
            gDrawArea.unk58 = 0x50000;
            gDrawArea.flags.pageOffset = frameBase;
            expandRecord = drawEnvGetRecord();
            drawRecordFill(expandRecord, blendMode, 0, 0x100, 0xE0, 0x200, 0x1C0);
            func_001D57E0();
            gDrawArea.unk58 = 0x30000;
        }
    }
    drawEnvSetCameraScissor();
    fullSizeRecord = drawEnvGetRecord();
    drawRecordFill(fullSizeRecord, 0x80, 3, 0x200, 0x1C0, 0x200, 0x1C0);
    /* kept: the do/while(0) around the last call decides the emitted order */
    do {
        func_001D55E8();
    } while (0);
}

typedef struct NoiseFilter {
    u8 unmodeled_00[4];
    s32 drawFlags;   /* +0x04: copied to the draw area's +0x68 word */
    s32 count;       /* +0x08: points drawn, low 14 bits */
    s32 red;         /* +0x0C: the colour 1, 2, 3 asks for a random colour per point */
    s32 green;       /* +0x10 */
    s32 blue;        /* +0x14 */
    s32 blendAmount; /* +0x18: fixed alpha */
    s32 blendMode;   /* +0x1C */
    s32 shape;       /* +0x20: 0 whole screen, 1 rectangle, 2 ellipse */
    s32 centerX;     /* +0x24 */
    s32 centerY;     /* +0x28 */
    s32 radiusX;     /* +0x2C */
    s32 radiusY;     /* +0x30 */
} NoiseFilter;

#define NOISE_TILE_SIZE 128
#define NOISE_BATCH 500 /* points of one packet, two quadwords each */
#define NOISE_CELL 64   /* side of one screen tile, in pixels */
#define NOISE_ELLIPSE_POINTS 37
#define NOISE_CELL_LOW(cell) ((cell) * NOISE_CELL)
#define NOISE_CELL_HIGH(cell) (((cell) + 1) * NOISE_CELL)
#define NOISE_POINT(x, y) ((s64) (((x) << 4) + 0x6FF8) | ((s64) (((y) << 4) + 0x71F7) << 16))
#define NOISE_SCISSOR(x0, x1, y0, y1) ((s64) (x0) | ((s64) (x1) << 16) | ((s64) (y0) << 32) | ((s64) (y1) << 48))
#define NOISE_TILE(firstUv, secondUv)                                                                     \
    xglPacketAppendQuad(GS_UV, firstUv);                                                                  \
    xglPacketAppendQuad(GS_XYZF2, NOISE_POINT(NOISE_CELL_LOW(column), NOISE_CELL_LOW(row)));               \
    xglPacketAppendQuad(GS_UV, secondUv);                                                                 \
    xglPacketAppendQuad(GS_XYZF2, NOISE_POINT(NOISE_CELL_HIGH(column), NOISE_CELL_HIGH(row)))

static inline s32 noiseRandomBelow(s32 limit) {
    u32 value;

    value = xglSRand();
    value %= limit;
    return (u16) value;
}

static inline void noiseSetEntry(PacketEntry *entry, s32 word0, s32 word1, s32 word2, s32 word3) {
    entry->word0 = word0;
    entry->word1 = word1;
    entry->word2 = word2;
    entry->word3 = word3;
}

static inline void noisePacketBegin(void) {
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
}

static inline void noisePacketEnd(void) {
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

static inline void noiseQueueCopy(s32 blendAmount, s32 blendMode, s32 sourceLeft, s32 sourceTop,
                                  s32 sourceRight, s32 sourceBottom, s32 left, s32 top, s32 right,
                                  s32 bottom) {
    DrawEnvRecord *drawRecord = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();

    drawRecord->blendFix = blendAmount;
    drawRecord->blendMode = blendMode;
    drawRecord->sourceRect[0] = sourceLeft;
    drawRecord->sourceRect[1] = sourceTop;
    drawRecord->sourceRect[2] = sourceRight;
    drawRecord->sourceRect[3] = sourceBottom;
    drawRecord->destinationRect[0] = left;
    drawRecord->destinationRect[1] = top;
    drawRecord->destinationRect[2] = right;
    drawRecord->destinationRect[3] = bottom;
}

/* Screen filter 22, NOISE in the game's filter name list.
 * Noise filter: queues random points (or random colours when red, green and blue are 1, 2, 3) and
 * copies the frame back through one of three shapes: 0 whole screen, 1 rectangle, 2 ellipse. */
void filterNoiseDraw(NoiseFilter *filter, s32 packet) {
    s32 drawFlags;
    s32 pointCount;
    s32 red;
    s32 green;
    s32 blue;
    s32 blendAmount;
    s32 blendMode;
    s32 shape;
    s32 centerX;
    s32 centerY;
    s32 radiusX;
    s32 radiusY;
    u32 pointsLeft;
    s32 queuedPoints = 0;
    PacketEntry *entry;
    s32 row;
    s32 column;
    s32 ellipseSegment;
    s32 x;
    s32 y;
    XglCamera *camera;
    s32 windowLeft;
    s32 windowTop;
    s32 windowRight;
    s32 windowBottom;

    drawFlags = filter->drawFlags;
    pointCount = filter->count & 0x3FFF;
    red = filter->red;
    green = filter->green;
    blue = filter->blue;
    blendAmount = filter->blendAmount;
    blendMode = filter->blendMode;
    shape = filter->shape;
    centerX = filter->centerX;
    centerY = filter->centerY;
    radiusX = filter->radiusX;
    radiusY = filter->radiusY;
    if (xglFilterShouldSkip()) {
        return;
    }
    if (pointCount <= 0) {
        return;
    }
    if (shape != 0) {
        if (radiusX <= 0) {
            return;
        }
        if (radiusY <= 0) {
            return;
        }
    }

    noisePacketBegin();
    xglPacketAppendQuad(GS_SCISSOR_1, NOISE_SCISSOR(0, 511, 0, 447));
    noisePacketEnd();
    drawRectOnPageColour(D_004C8390.frameBase, 0, 0, 0, 0x100,
                                                                         0x100);
    func_001D5CD0(D_004C8390.frameBase, 0, 0, 0x100, 0x100);
    noisePacketBegin();
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, (s64) D_004C8390.depthBase | ((s64) GS_PSMZ24 << 24) | ((s64) 1 << 32));
    xglPacketAppendQuad(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
    xglPacketAppendQuad(GS_FRAME_1, (s64) D_004C8390.frameBase | ((s64) 4 << 16));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    noisePacketEnd();

    entry = (PacketEntry *) XGL_SCRATCHPAD + 1;
    noiseSetEntry((PacketEntry *) XGL_SCRATCHPAD, 0x8000, 0x20004000, 0x51, 0);
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    pointsLeft = pointCount;
    do {
        x = noiseRandomBelow(NOISE_TILE_SIZE + 1);
        y = noiseRandomBelow(NOISE_TILE_SIZE + 1);
        if (red == 1 && green == 2 && blue == 3) {
            entry->word0 = xglSRand() & 0xFF;
            entry->word1 = xglSRand() & 0xFF;
            entry->word2 = xglSRand() & 0xFF;
        } else {
            entry->word0 = red;
            entry->word1 = green;
            entry->word2 = blue;
        }
        entry[0].word3 = 0x80;
        entry[1].word0 = (x << 4) + 0x6FF8;
        entry[1].word1 = (y << 4) + 0x71F7;
        entry[1].word2 = 0;
        entry[1].word3 = 0;
        queuedPoints++;
        entry += 2;
        if ((u32) queuedPoints >= NOISE_BATCH) {
            ((PacketEntry *) XGL_SCRATCHPAD)->word0 |= NOISE_BATCH;
            sceVif1PkAddDirectDataN(
                packet, XGL_SCRATCHPAD, NOISE_BATCH * 2 + 1);
            queuedPoints = 0;
            entry = (PacketEntry *) XGL_SCRATCHPAD + 1;
        }
    } while (--pointsLeft);
    if (queuedPoints) {
        ((PacketEntry *) XGL_SCRATCHPAD)->word0 = (((PacketEntry *) XGL_SCRATCHPAD)->word0 & 0x8000) | queuedPoints;
        sceVif1PkAddDirectDataN(packet, XGL_SCRATCHPAD,
                                                                                      queuedPoints * 2 + 1);
    }
    sceVif1PkCloseDirectHLCode(packet);

    camera = xglStudioSelectGetActiveCamera(0);
    windowLeft = camera->screen.window.vector.x;
    windowTop = camera->screen.window.vector.y;
    windowRight = camera->screen.window.vector.z;
    windowBottom = camera->screen.window.vector.w;
    noisePacketBegin();
    xglPacketAppendQuad(GS_SCISSOR_1,
                        NOISE_SCISSOR(windowLeft, windowRight - 1, windowTop, windowBottom - 1));
    xglPacketAppendQuad(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
    xglPacketAppendQuad(GS_FRAME_1, (s64) D_004C8390.workBase | ((s64) 8 << 16));
    xglPacketAppendQuad(GS_ALPHA_1, 0xA8 | ((s64) 0x80 << 32));
    xglPacketAppendQuad(GS_TEX0_1, (s64) (D_004C8390.frameBase * 32) | ((s64) 4 << 14) | ((s64) 8 << 26) |
                                       ((s64) 8 << 30) | ((s64) 1 << 34));
    xglPacketAppendQuad(GS_TEX1_1, 0x60);
    xglPacketAppendQuad(GS_RGBAQ, 0x3F80000080808080LL);
    xglPacketAppendQuad(GS_PRIM, GS_PRIM_TEXTURED_SPRITE);
    for (row = 0; row < 7; row++) {
        for (column = 0; column < 8; column++) {
            switch ((s32) xglSRand() & 3) {
            case 0:
                NOISE_TILE(0, 0x04000400);
                break;
            case 1:
                NOISE_TILE(0x00000400, 0x04000800);
                break;
            case 2:
                NOISE_TILE(0x04000000, 0x08000400);
                break;
            case 3:
                NOISE_TILE(0x04000400, 0x08000800);
                break;
            }
        }
    }
    noisePacketEnd();

    switch (shape) {
    case 0:
        gDrawArea.unk58 = 0x5000D;
        gDrawArea.flags.pageOffset = drawFlags;
        noiseQueueCopy(blendAmount, blendMode, 0, 0, 512, 448, 0, 0, 512, 448);
        func_001D55E8();
        gDrawArea.unk58 = GS_TEST_ZTE_ALWAYS;
        break;
    case 1: {
        s32 left = centerX - radiusX;
        s32 top = centerY - radiusY;
        s32 right = centerX + radiusX;
        s32 bottom = centerY + radiusY;

        if (left < 0) {
            left = 0;
        }
        if (right > 512) {
            right = 512;
        }
        if (top < 0) {
            top = 0;
        }
        if (bottom > 448) {
            bottom = 448;
        }
        func_001D5E60(D_004C8390.displayBase, 0, 0, 512, 448);
        func_001D5CD0(D_004C8390.displayBase, left, top, right, bottom);
        gDrawArea.unk58 = 0x5400D;
        gDrawArea.flags.pageOffset = drawFlags;
        noiseQueueCopy(blendAmount, blendMode, 0, 0, 512, 448, 0, 0, 512, 448);
        func_001D55E8();
        gDrawArea.unk58 = GS_TEST_ZTE_ALWAYS;
        break;
    }
    case 2: {
        f32 angle;
        f32 sine;
        f32 cosine;

        func_001D5E60(D_004C8390.displayBase, 0, 0, 512, 448);
        noisePacketBegin();
        xglPacketAppendQuad(GS_FBA_1, 0);
        xglPacketAppendQuad(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
        xglPacketAppendQuad(GS_FRAME_1,
                            (s64) D_004C8390.displayBase | ((s64) 8 << 16) | ((s64) 0xFFFFFF << 32));
        switch (blendMode) {
        case 1:
            xglPacketAppendQuad(GS_ALPHA_1, ((s64) blendAmount << 32) | 0x68);
            break;
        case 2:
            xglPacketAppendQuad(GS_ALPHA_1, ((s64) blendAmount << 32) | 0x62);
            break;
        default:
            xglPacketAppendQuad(GS_ALPHA_1, ((s64) blendAmount << 32) | 0x64);
            break;
        }
        xglPacketAppendQuad(GS_RGBAQ, (s64) XGL_FLOAT_ONE_BITS << 32);
        xglPacketAppendQuad(GS_PRIM, 0x45);
        xglPacketAppendQuad(GS_XYZF2, NOISE_POINT(centerX, centerY));
        for (ellipseSegment = 0; ellipseSegment < NOISE_ELLIPSE_POINTS; ellipseSegment++) {
            angle = (f32) ellipseSegment * 0.17453289f; /* 10 degrees per segment */
            sine = xglSin(angle);
            cosine = xglCos(angle);
            x = (s32) ((f32) radiusX * cosine);
            y = (s32) ((f32) radiusY * sine);
            x += centerX;
            y += centerY;
            xglPacketAppendQuad(GS_XYZF2, NOISE_POINT(x, y));
        }
        noisePacketEnd();
        gDrawArea.unk58 = 0x5400D;
        gDrawArea.flags.pageOffset = drawFlags;
        noiseQueueCopy(blendAmount, blendMode, 0, 0, 512, 448, 0, 0, 512, 448);
        func_001D55E8();
        gDrawArea.unk58 = GS_TEST_ZTE_ALWAYS;
        break;
    }
    }
}

typedef struct LineJitterFilter {
    s32 unk00;
    s32 depth;      /* +0x04: z of every vertex */
    s32 vertical;   /* +0x08: 1 = columns shifted vertically, otherwise rows shifted horizontally */
    s32 range;      /* +0x0C: range of the random shift, in pixels */
    s32 chance;     /* +0x10: percentage of shifted lines */
    s32 fixedAlpha; /* +0x14: fixed alpha of the blend */
    s32 blendMode;  /* +0x18 */
    s32 halfHeight; /* +0x1C: draws to the half-height page */
} LineJitterFilter;

typedef struct LineJitterVertex {
    s32 texelX;
    s32 texelY;
    s32 unk08;
    s32 unk0C;
    s32 gsX;
    s32 gsY;
    s32 depth;
    s32 unk1C;
} LineJitterVertex;

#define LINE_JITTER_WIDTH 0x200
#define LINE_JITTER_HEIGHT 0x1C0
#define LINE_JITTER_COLUMNS (LINE_JITTER_WIDTH / 2)
#define LINE_JITTER_ROWS (LINE_JITTER_HEIGHT / 2)
#define LINE_JITTER_GS_X(x) (((x) << 4) + 0x6FF8)
#define LINE_JITTER_GS_Y(y) (((y) << 4) + 0x71F7)
#define LINE_JITTER_ADD_VERTEX(texX, texY, screenX, screenY)                                                           \
    vertex->texelX = (texX) << 4;                                                                                      \
    vertex->texelY = (texY) << 4;                                                                                      \
    vertex->unk08 = 0;                                                                                                 \
    vertex->unk0C = 0;                                                                                                 \
    vertex->gsX = LINE_JITTER_GS_X(screenX);                                                                           \
    vertex->gsY = LINE_JITTER_GS_Y(screenY);                                                                           \
    vertex->depth = depth;                                                                                             \
    vertex->unk1C = 0;                                                                                                 \
    vertex++;                                                                                                          \
    vertexCount++
/* kept: the do/while(0) wrapper decides the emitted order */
#define LINE_JITTER_CLOSE(packet)                                                                                      \
    do {                                                                                                               \
        sceVif1PkCloseDirectHLCode(packet);                                                                            \
    } while (0)

static inline void lineJitterSetTag(PacketEntry *tag, s32 word0, s32 word1, s32 word2, s32 word3) {
    tag->word0 = word0;
    tag->word1 = word1;
    tag->word2 = word2;
    tag->word3 = word3;
}

static inline void lineJitterSetCount(PacketEntry *tag, s32 count) {
    tag->word0 &= 0x8000;
    tag->word0 |= count;
}

/* Screen filter 23, NOISE2 in the game's filter name list. */
void filterNoise2Draw(LineJitterFilter *filter, s32 vifPacket) {
    s32 depth = filter->depth;
    s32 vertical = filter->vertical;
    s32 shiftRange = filter->range;
    s32 chance = filter->chance;
    s32 fixedAlpha = filter->fixedAlpha;
    s32 blendMode = filter->blendMode;
    s32 halfHeight = filter->halfHeight;
    s32 line;

    if (xglFilterShouldSkip()) {
        return;
    }
    if (chance <= 0) {
        return;
    }
    if (shiftRange <= 0) {
        return;
    }
    if (xglRenderMovieIsActive()) {
        if (halfHeight) {
            drawEnvSetHalfDisplayNext();
        } else {
            drawEnvSetLarge();
        }
    } else {
        if (halfHeight) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }
    }

    {
        LineJitterVertex *vertex;
        s32 vertexCount;

        xglPacketBufferInit(XGL_SCRATCHPAD);
        xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_CLAMP_1, 0);
        xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, 0x31, 1));
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        switch (blendMode) {
        case 1:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, fixedAlpha));
            break;
        case 2:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, fixedAlpha));
            break;
        default:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, fixedAlpha));
            break;
        }
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        vertex = (LineJitterVertex *) XGL_SCRATCHPAD;
        xglPacketAppendQuad(GS_TEX0_1,
                            GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);

        vertexCount = 0;
        sceVif1PkCnt(vifPacket, 0);
        sceVif1PkOpenDirectHLCode(vifPacket, 0);
        lineJitterSetTag((PacketEntry *) vertex, 0x8000, 0x20AE4000, 0x53, 0);

        if (vertical == 1) {
            for (line = 0; line < LINE_JITTER_COLUMNS; line++) {
                s32 shift = 0;
                s32 split;
                s32 rest;
                s32 left;
                s32 right;
                s32 height;

                if ((s32) (xglSRand() % 100U) + 1 <= chance) {
                    shift = (s32) xglSRand() % (shiftRange * 2) - shiftRange;
                }
                left = line * 2;
                right = left + 2;
                height = LINE_JITTER_HEIGHT;
                split = 0;
                rest = 0;

                /* The piece that stays on the screen. */
                vertex = (LineJitterVertex *) SCRATCHPAD_VERTICES;
                if (shift > 0) {
                    rest = height - shift;
                    split = shift;
                    LINE_JITTER_ADD_VERTEX(left, split, left, 0);
                    LINE_JITTER_ADD_VERTEX(right, 0, right, 0);
                    LINE_JITTER_ADD_VERTEX(left, LINE_JITTER_HEIGHT, left, rest);
                    LINE_JITTER_ADD_VERTEX(right, LINE_JITTER_HEIGHT, right, LINE_JITTER_HEIGHT);
                } else if (shift < 0) {
                    rest = -shift;
                    split = height + shift;
                    LINE_JITTER_ADD_VERTEX(left, 0, left, rest);
                    LINE_JITTER_ADD_VERTEX(right, 0, right, 0);
                    LINE_JITTER_ADD_VERTEX(left, split, left, LINE_JITTER_HEIGHT);
                    LINE_JITTER_ADD_VERTEX(right, LINE_JITTER_HEIGHT, right, LINE_JITTER_HEIGHT);
                }
                lineJitterSetCount((PacketEntry *) XGL_SCRATCHPAD, vertexCount);
                sceVif1PkAddDirectDataN(
                    vifPacket, XGL_SCRATCHPAD, vertexCount * 2 + 1);
                vertexCount = 0;

                /* The piece that comes back at the other end. */
                vertex = (LineJitterVertex *) SCRATCHPAD_VERTICES;
                if (shift > 0) {
                    LINE_JITTER_ADD_VERTEX(left, 0, left, rest);
                    LINE_JITTER_ADD_VERTEX(right, 0, right, LINE_JITTER_HEIGHT);
                    LINE_JITTER_ADD_VERTEX(left, split, left, LINE_JITTER_HEIGHT);
                } else if (shift < 0) {
                    LINE_JITTER_ADD_VERTEX(left, split, left, 0);
                    LINE_JITTER_ADD_VERTEX(right, LINE_JITTER_HEIGHT, right, 0);
                    LINE_JITTER_ADD_VERTEX(left, LINE_JITTER_HEIGHT, left, rest);
                }
                lineJitterSetCount((PacketEntry *) XGL_SCRATCHPAD, vertexCount);
                sceVif1PkAddDirectDataN(
                    vifPacket, XGL_SCRATCHPAD, vertexCount * 2 + 1);
                vertexCount = 0;
            }
        } else {
            for (line = 0; line < LINE_JITTER_ROWS; line++) {
                s32 shift = 0;
                s32 split;
                s32 rest;
                s32 top;
                s32 bottom;
                s32 width;

                if ((s32) (xglSRand() % 100U) < chance) {
                    shift = (s32) xglSRand() % shiftRange - (shiftRange >> 1);
                }
                top = line * 2;
                bottom = top + 2;
                width = LINE_JITTER_WIDTH;
                split = 0;
                rest = 0;

                /* The piece that stays on the screen. */
                vertex = (LineJitterVertex *) SCRATCHPAD_VERTICES;
                if (shift > 0) {
                    rest = width - shift;
                    split = shift;
                    LINE_JITTER_ADD_VERTEX(split, top, 0, top);
                    LINE_JITTER_ADD_VERTEX(0, bottom, 0, bottom);
                    LINE_JITTER_ADD_VERTEX(LINE_JITTER_WIDTH, top, rest, top);
                    LINE_JITTER_ADD_VERTEX(LINE_JITTER_WIDTH, bottom, LINE_JITTER_WIDTH, bottom);
                } else if (shift < 0) {
                    rest = -shift;
                    split = width + shift;
                    LINE_JITTER_ADD_VERTEX(0, top, rest, top);
                    LINE_JITTER_ADD_VERTEX(0, bottom, 0, bottom);
                    LINE_JITTER_ADD_VERTEX(split, top, LINE_JITTER_WIDTH, top);
                    LINE_JITTER_ADD_VERTEX(LINE_JITTER_WIDTH, bottom, LINE_JITTER_WIDTH, bottom);
                }
                lineJitterSetCount((PacketEntry *) XGL_SCRATCHPAD, vertexCount);
                sceVif1PkAddDirectDataN(
                    vifPacket, XGL_SCRATCHPAD, vertexCount * 2 + 1);
                vertexCount = 0;

                /* The piece that comes back at the other end. */
                vertex = (LineJitterVertex *) SCRATCHPAD_VERTICES;
                if (shift > 0) {
                    LINE_JITTER_ADD_VERTEX(0, top, rest, top);
                    LINE_JITTER_ADD_VERTEX(0, bottom, LINE_JITTER_WIDTH, bottom);
                    LINE_JITTER_ADD_VERTEX(split, top, LINE_JITTER_WIDTH, top);
                } else if (shift < 0) {
                    LINE_JITTER_ADD_VERTEX(split, top, 0, top);
                    LINE_JITTER_ADD_VERTEX(LINE_JITTER_WIDTH, bottom, 0, bottom);
                    LINE_JITTER_ADD_VERTEX(LINE_JITTER_WIDTH, top, rest, top);
                }
                lineJitterSetCount((PacketEntry *) XGL_SCRATCHPAD, vertexCount);
                sceVif1PkAddDirectDataN(
                    vifPacket, XGL_SCRATCHPAD, vertexCount * 2 + 1);
                vertexCount = 0;
            }
        }
        LINE_JITTER_CLOSE(vifPacket);
    }
}

/* Blit parameters written through drawEnvGetRecord(); unk00 holds the leading words. */
typedef struct BlitRecord {
    s32 unk00[5];
    s32 blitA;
    s32 blitB;
    s32 sourceX;
    s32 sourceY;
    s32 sourceWidth;
    s32 sourceHeight;
    s32 destX;
    s32 destY;
    s32 destWidth;
    s32 destHeight;
} BlitRecord;

/* Request block read by filterSpeculerDraw (display-copy pass). */
typedef struct DisplayCopyRequest {
    s32 unk00;
    s32 pageRow;
    s32 blitCount;
    s32 copyPages;
    s32 red;
    s32 green;
    s32 blue;
    s32 argA;
    s32 argB;
    s32 halfField;
    s32 unk28[10];
    u32 unk50Bit0 : 1;
    u32 unk50Bit1 : 1;
    u32 composite : 1;
    u32 unk50High : 29;
} DisplayCopyRequest;

static inline void SetBlitRecord(s32 blitA, s32 blitB, s32 sourceX, s32 sourceY, s32 sourceWidth, s32 sourceHeight, s32 destX, s32 destY, s32 destWidth, s32 destHeight) {
    BlitRecord *record = (BlitRecord *) ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();
    record->blitA = blitA;
    record->blitB = blitB;
    record->sourceX = sourceX;
    record->sourceY = sourceY;
    record->sourceWidth = sourceWidth;
    record->sourceHeight = sourceHeight;
    record->destX = destX;
    record->destY = destY;
    record->destWidth = destWidth;
    record->destHeight = destHeight;
}

/* Screen filter 24, SPECULER in the game's filter name list.
 * Display-copy pass: blits the frame, optionally copies the page buffers, and sends the
   composite packet when request->composite is set. */
void filterSpeculerDraw(DisplayCopyRequest *request) {
    s32 pageRow = request->pageRow;
    s32 blitCount = request->blitCount;
    s32 copyPages = request->copyPages;
    s32 argA = request->argA;
    s32 argB = request->argB;
    s32 halfField = request->halfField;
    s32 invertedColour = ((255 - request->red) << 16) | ((255 - request->green) << 8) | (255 - request->blue);
    s32 i;
    s32 page;
    if (xglFilterShouldSkip()) {
        return;
    }
    if (blitCount < 0) {
        return;
    }
    if (blitCount >= 11) {
        blitCount = 10;
    }
    xglPacketSendFixedHeader();
    if (xglRenderMovieIsActive()) {
        if (halfField) {
            drawEnvSetHalfDisplayNext();
        } else {
            drawEnvSetLarge();
        }
    } else {
        if (halfField) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }
    }
    drawRectOnPageColour(D_004C8390.frameBase, invertedColour, 0, 0, 0x100, 0xE0);
    SetBlitRecord(0x80, 2, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D57E0();
    SetBlitRecord(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D54C8();
    if (request->composite) {
        drawEnvSetLarge();
    }
    if (copyPages) {
        if (request->composite) {
            page = D_004C8390.workBase;
        } else {
            page = D_004C8390.displayBase;
        }
        func_001D5CD0(page, 0, 0, 0x200, 0x1C0);
        if (request->composite) {
            page = D_004C8390.workBase;
        } else {
            page = D_004C8390.displayBase;
        }
        drawRectOnPageDepth(page, pageRow, 0, 0, 0x200, 0x1C0);
        drawEnvSetCameraScissor();
        gDrawArea.unk58 = 0x34000;
        for (i = 0; i < blitCount; i++) {
            SetBlitRecord(argA, argB, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
            if (request->composite) {
                func_001D57E0();
            } else {
                func_001D5840();
            }
        }
        gDrawArea.unk58 = 0x30000;
    } else {
        drawEnvSetCameraScissor();
        gDrawArea.unk58 = 0x50000;
        gDrawArea.flags.pageOffset = pageRow;
        for (i = 0; i < blitCount; i++) {
            SetBlitRecord(argA, argB, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
            if (request->composite) {
                func_001D57E0();
            } else {
                func_001D5840();
            }
        }
        gDrawArea.unk58 = 0x30000;
    }
    if (request->composite) {
        xglPacketBeginGif();
        xglPacketAppendQuad(0x3F, 0);
        xglPacketAppendQuad(0x47, 0x30000);
        xglPacketAppendQuad(6, (D_004C8390.workBase * 32) | 0x24020000 | ((s64) 0xC800 << 19));
        xglPacketAppendQuad(0x42, 0x54);
        xglPacketAppendQuad(0x4C, D_004C8390.displayBase | 0xFF00000000080000ULL);
        xglPacketAppendQuad(0, 0x156);
        xglPacketAppendQuad(3, 0);
        xglPacketAppendQuad(4, 0x71F76FF8);
        xglPacketAppendQuad(3, 0x1C002000);
        xglPacketAppendQuad(4, 0x8DF78FF8ULL);
        xglPacketFinishAndSend();
    }
}


static signed char gXglEffectSlotMask;


/* Claims the lowest free bit of the effect-slot mask gXglEffectSlotMask. Returns the 1-based slot id, or 0 when all eight are used. */
s32 xglEffectSlotAlloc(void) {
    s32 bit;

    for (bit = 0; bit < 8; bit++) {
        if ((gXglEffectSlotMask & (1 << bit)) == 0) {
            gXglEffectSlotMask |= 1 << bit;
            return bit + 1;
        }
    }
    return 0;
}

#define EFFECT_SLOT_BYTES 0x1800

typedef struct SlotPool {
    u8 unk00[0x40];
    s32 slotId; /* +0x40: 1..8 while the pool holds a slot, 0 when empty */
} SlotPool;

static u8 gXglEffectSlots[16400];
void *memset(void *destination, int value, unsigned int size);

/* Releases the effect slot held by pool: clears its particle block and its bit in gXglEffectSlotMask. */
void xglEffectSlotRelease(SlotPool *pool) {
    s32 *slotIdPtr;
    s32 slotId;
    s32 bit;
    u8 flags;
    u8 *slot;

    slotIdPtr = &pool->slotId;
    slotId = *slotIdPtr;
    if (slotId != 0) {
        slot = gXglEffectSlots + slotId * EFFECT_SLOT_BYTES - EFFECT_SLOT_BYTES;
        if (slot != 0) {
            memset(slot, 0, EFFECT_SLOT_BYTES);
            bit = *slotIdPtr - 1;
            flags = gXglEffectSlotMask;
            *slotIdPtr = 0;
            gXglEffectSlotMask = flags & ~(1 << bit);
        }
    }
}

#define EFFECT_DEGREE 0.017453292f
#define EFFECT_PI_OVER_4 0.785398f
#define EFFECT_EMITTER_STATE_MASK 0x18

enum EffectParticleMode {
    PARTICLE_MODE_SPAWN = 0,
    PARTICLE_MODE_FALL = 1,
    PARTICLE_MODE_HOLD = 2,
    PARTICLE_MODE_TURN = 3
};

typedef struct EffectParticle {
    s32 x;    /* +0x00 */
    s32 y;    /* +0x04 */
    s32 z;    /* +0x08 */
    s32 time; /* +0x0C: y << 4 at spawn, then += step */
    unsigned char unk10[0x20 - 0x10];
    signed char angleX; /* +0x20: degrees */
    signed char angleY; /* +0x21: degrees */
    signed char life;   /* +0x22 */
    signed char mode;   /* +0x23: enum EffectParticleMode */
    s16 step;           /* +0x24 */
    unsigned char unk26[0x30 - 0x26];
} EffectParticle;

typedef struct EffectSlot {
    EffectParticle particles[128];
} EffectSlot;

typedef struct EmitterMotion {
    unsigned char unk00[0x58];
    s32 motion[6]; /* +0x58..+0x6C: arguments of func_002018E8 */
} EmitterMotion;

typedef struct ParticleEmitter {
    unsigned char unk00;
    unsigned char staggered; /* +0x01: nonzero = lives are not reseeded */
    unsigned char unk02[2];
    s32 originX;             /* +0x04 */
    s32 originY;             /* +0x08 */
    s32 originZ;             /* +0x0C */
    s32 spreadBase;          /* +0x10 */
    s32 spreadRange;         /* +0x14 */
    u32 extentX;             /* +0x18 */
    u32 count;               /* +0x1C */
    u32 amplitude;           /* +0x20 */
    u32 extentY;             /* +0x24 */
    u32 lifeMax;             /* +0x28 */
    unsigned char colour[4]; /* +0x2C: r, g, b, a */
    u32 pattern;             /* +0x30: high half style, low half segments */
    EmitterMotion *motion;   /* +0x34 */
    unsigned char unk38[4];
    s32 shape; /* +0x3C */
    s32 slot;  /* +0x40: 1..8 from xglEffectSlotAlloc */
    unsigned char unk44[0x50 - 0x44];
    u32 flags; /* +0x50: bits 3 and 4 select the stored state */
} ParticleEmitter;

f32 sefNoiseNext(void);
void func_002018E8(s32, s32, s32, s32, s32, s32);
void sdvProjectEffectPoint(s32 *origin, EffectParticle *particle, s32 segments, s32 z, f32 *scale, s32 style, s32 zero);

static inline s32 ClampInsetWindow(s32 value, s32 low, s32 high) {
    s32 result;

    if (value < (low << 4) + 16) {
        result = (low << 4) + 16;
    } else {
        result = value;
        if ((high << 4) - 16 < result) {
            result = (high << 4) - 16;
        }
    }
    return result;
}

static inline s32 ClampInsetPixel(s32 value, s32 low, s32 high) {
    s32 result;

    if (value < low + 1) {
        result = (low << 4) + 16;
    } else {
        s32 clamped = high - 1 < value ? high - 1 : value;

        result = clamped << 4;
    }
    return result;
}

static inline s32 ClampGsCoord(s32 value, s32 low, s32 high, s32 offset) {
    s32 result;

    if (value >= low) {
        s32 clamped;

        if (high < value) {
            clamped = high;
        } else {
            clamped = value;
        }
        result = (clamped << 4) + offset;
    } else {
        result = (low << 4) + offset;
    }
    return result;
}

/* Screen filter 25, SHIMMER in the game's filter name list.
 * Emits the particle sprites of one emitter: each live particle runs its mode state machine
   (spawn, fall, hold, turn), then its star of nine spokes and its sprite packets are sent. */
void filterShimmerDraw(ParticleEmitter *emitter) {
    s32 originX = emitter->originX;
    s32 originY = emitter->originY;
    s32 originZ = emitter->originZ;
    s32 spreadBase = emitter->spreadBase;
    s32 spreadRange = emitter->spreadRange;
    u32 extentX = emitter->extentX;
    u32 count = emitter->count;
    u32 amplitude = emitter->amplitude;
    u32 extentY = emitter->extentY;
    u32 lifeMax = emitter->lifeMax;
    s32 colourR = emitter->colour[0];
    s32 colourG = emitter->colour[1];
    s32 colourB = emitter->colour[2];
    u32 colourA = emitter->colour[3];
    s32 segments = emitter->pattern & 0xFFFF;
    s32 style = emitter->pattern >> 16;
    EmitterMotion *motion = emitter->motion;
    s32 shape = emitter->shape;
    XglCamera *camera;
    s32 clipLeft;
    s32 clipTop;
    s32 clipRight;
    s32 clipBottom;
    s32 origin[4];
    s32 *slotIdPtr = &emitter->slot;
    f32 scale[4];
    u32 particleIndex;
    s32 spoke;
    EffectParticle *particle;
    f32 spin;
    f32 noise;
    f32 angle;
    f32 sinAngle;
    f32 cosAngle;
    f32 stepAngle;

    camera = xglStudioSelectGetActiveCamera(0);
    clipLeft = (s32) camera->screen.window.vector.x;
    clipTop = (s32) camera->screen.window.vector.y;
    clipRight = (s32) camera->screen.window.vector.z;
    clipBottom = (s32) camera->screen.window.vector.w;
    if ((*slotIdPtr) <= 0) {
        (*slotIdPtr) = xglEffectSlotAlloc();
        if ((*slotIdPtr) <= 0) {
            return;
        }
    }
    if (count - 1 < 128 && lifeMax != 0 && lifeMax <= 128 && extentX != 0) {
        if (xglFilterShouldSkip() == 0) {
            drawEnvSetLarge();
            xglPacketBufferInit(XGL_SCRATCHPAD);
            xglPacketAppendEntry(0x8000, 0x10000000, 0xE, 0);
            xglPacketAppendQuad(0x4C, D_004C8390.displayBase | ((((u64) 0xFF000000 << 0x10) | 8) << 0x10));
            switch (shape) {
            case 1:
                xglPacketAppendQuad(0x42, 0x48);
                break;
            case 2:
                xglPacketAppendQuad(0x42, 0x42);
                break;
            default:
                xglPacketAppendQuad(0x42, 0x44);
                break;
            }
            xglPacketAppendQuad(0x14, 0x60);
            xglPacketAppendQuad(6, 0x24020000 | (D_004C8390.workBase << 5) | ((u64) 0x9000 << 0x12));
            xglPacketAppendQuad(0x47, 0x50000);
            xglPacketAppendQuad(0x4E, D_004C8390.depthBase | ((u64) 0x9880 << 0x11));
            xglPacketFinishTag();
            xglPacketSendDirect(0, 0);
        }
        if (motion != 0) {
            func_002018E8(motion->motion[0], motion->motion[1], motion->motion[2], motion->motion[3],
                          motion->motion[4], motion->motion[5]);
        }
        origin[0] = originX;
        origin[1] = originY;
        origin[2] = originZ;
        origin[3] = 0;

        for (particleIndex = 0; particleIndex < count; particleIndex++) {
            s32 dx;
            s32 dy;
            s32 x;
            s32 y;
            s32 z;

            particle = &((EffectSlot *) &gXglEffectSlots)[(*slotIdPtr) - 1].particles[particleIndex];
            spin = 0.0f;
            if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                if (emitter->staggered == 0) {
                    particle->life = xglSRand() % lifeMax;
                    particle->mode = PARTICLE_MODE_SPAWN;
                }
                particle->life -= 1;
            }
            switch (particle->mode) {
            case PARTICLE_MODE_SPAWN:
                if (particle->life > 0) {
                    continue;
                }
                if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                    noise = sefNoiseNext();
                    particle->x = (s32) ((f32) originX + (noise + noise) * (f32) amplitude);
                    noise = sefNoiseNext();
                    particle->y = (s32) ((f32) originY + (noise + noise) * (f32) amplitude);
                    noise = sefNoiseNext();
                    z = (s32) ((f32) originZ + (noise + noise) * (f32) amplitude);
                    particle->time = particle->y << 4;
                    particle->life = 9;
                    particle->z = z;
                    particle->mode = PARTICLE_MODE_FALL;
                    if (spreadBase != 0) {
                        if (spreadRange != 0) {
                            particle->step =
                                spreadBase + ((s32) (xglSRand() % (spreadRange * 2)) - spreadRange);
                        } else {
                            particle->step = spreadBase;
                        }
                    } else {
                        particle->step = 0;
                    }
                    particle->angleX = (signed char) (xglSRand() % 180);
                    particle->angleY = (signed char) (xglSRand() % 180);
                }
                /* fall through */
            case PARTICLE_MODE_FALL:
                if (particle->life > 0) {
                    if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                        particle->time += particle->step;
                    }
                    angle = (f32) (20 * particle->life) * EFFECT_DEGREE;
                    spin = (xglCos(angle) + 1.0f) * 0.5f;
                    break;
                }
                if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                    particle->mode = PARTICLE_MODE_HOLD;
                    particle->life = lifeMax;
                }
                /* fall through */
            case PARTICLE_MODE_HOLD:
                if (particle->life >= 9) {
                    if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                        particle->time += particle->step;
                    }
                    spin = 1.0f;
                    break;
                }
                if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                    particle->mode = PARTICLE_MODE_TURN;
                }
                /* fall through */
            case PARTICLE_MODE_TURN:
                if (particle->life > 0) {
                    if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) == 0) {
                        particle->time += particle->step;
                    }
                    angle = (f32) (9 - particle->life) * EFFECT_DEGREE * 20.0f;
                    spin = (xglCos(angle) + 1.0f) * 0.5f;
                    break;
                }
                if ((emitter->flags & EFFECT_EMITTER_STATE_MASK) != 0) {
                    continue;
                }
                particle->mode = PARTICLE_MODE_SPAWN;
                particle->life = xglSRand() % 30;
                continue;
            default:
                break;
            }
            if (xglFilterShouldSkip() != 0) {
                continue;
            }
            __asm__ volatile("sqc2 $vf0,0(%0)" : : "r"(scale) : "memory");
            scale[0] = (f32) ((double) extentX * 0.01);
            scale[1] = (f32) ((double) extentY * 0.01);
            if (segments == 8) {
                particle->y = particle->time >> 4;
                sdvProjectEffectPoint(origin, particle, 8, originZ, scale, style, 0);
                particle->z = originZ;
                scale[0] = (f32) extentX;
                scale[1] = (f32) extentY;
            } else {
                particle->y = particle->time >> 4;
                sdvProjectEffectPoint(origin, particle, segments, 0, scale, style, 0);
            }
            x = origin[0];
            y = origin[1];
            cosAngle = xglCos((f32) particle->angleX * EFFECT_DEGREE);
            dx = (s32) (scale[1] * cosAngle);
            dy = (s32) (scale[1] * xglCos((f32) particle->angleY * EFFECT_DEGREE));

            xglPacketBufferInit(XGL_SCRATCHPAD);
            xglPacketAppendEntry(2, 0x10000000, 0xE, 0);
            xglPacketAppendQuad(0x4C, D_004C8390.workBase | ((((u64) 0xFF000000 << 0x10) | 8) << 0x10));
            xglPacketAppendQuad(6, 0x24020000 | (D_004C8390.displayBase << 5) | ((u64) 0x9000 << 0x12));
            xglPacketAppendEntry(0x8000, 0x308EC000, 1329, 0);
            xglPacketAppendWordQuad(colourR, colourG, colourB, (u32) ((f32) colourA * spin));

            xglPacketAppendWordQuad(ClampInsetWindow((x << 4) + dx, clipLeft, clipRight),
                                    ClampInsetWindow((y << 4) + dy, clipTop, clipBottom), 0, 0);
            xglPacketAppendWordQuad(ClampGsCoord(x, clipLeft, clipRight, 0x6FF8),
                                    ClampGsCoord(y, clipTop, clipBottom, 0x71F7), origin[2], 0);

            for (spoke = 0; spoke < 9; spoke++) {
                stepAngle = (f32) spoke * EFFECT_PI_OVER_4;
                sinAngle = xglSin(stepAngle);
                x = (s32) ((f32) origin[0] + scale[0] * sinAngle);
                cosAngle = xglCos(stepAngle);
                y = (s32) ((f32) origin[1] + scale[0] * cosAngle);
                xglPacketAppendWordQuad(colourR, colourG, colourB, 0);
                xglPacketAppendWordQuad(ClampInsetPixel(x, clipLeft, clipRight),
                                        ClampInsetPixel(y, clipTop, clipBottom), 0, 0);
                xglPacketAppendWordQuad(ClampGsCoord(x, clipLeft, clipRight, 0x6FF8),
                                        ClampGsCoord(y, clipTop, clipBottom, 0x71F7), origin[2], 0);
            }

            SCRATCHPAD_WORD(12) |= 0xA;
            xglPacketSendDirect(0, 0);
            SCRATCHPAD_WORD(13) = 0x30AEC000;
            SCRATCHPAD_DOUBLEWORDS[2] = D_004C8390.displayBase | ((((u64) 0xFF000000 << 0x10) | 8) << 0x10);
            SCRATCHPAD_DOUBLEWORDS[4] = 0x24020000 | (D_004C8390.workBase << 5) | ((u64) 0x9000 << 0x12);
            xglPacketSendDirect(0, 0);
        }
    }
}

typedef union EffectVector {
    Vector4 vector;
    s32 words[4];
} EffectVector __attribute__((aligned(16)));

typedef union EffectScreenVector {
    struct {
        s32 x;
        s32 y;
        s32 z;
        s32 w;
    } vector;
    s32 words[4];
} EffectScreenVector __attribute__((aligned(16)));

typedef struct EffectMatrix {
    Vector4 row[4];
} EffectMatrix;

enum MistParticleMode {
    MIST_MODE_SPAWN = 0,
    MIST_MODE_APPEAR = 1,
    MIST_MODE_HOLD = 2,
    MIST_MODE_VANISH = 3
};

typedef struct MistParticle {
    union {
        Vector4 vector;
        f32 component[4];
        s32 words[4];
    } position;         /* +0x00 */
    Vector4 velocity;   /* +0x10 */
    signed char angleX; /* +0x20: degrees */
    signed char angleY; /* +0x21: degrees */
    signed char life;   /* +0x22 */
    signed char mode;   /* +0x23: enum MistParticleMode */
    s16 unk24;
    s16 size; /* +0x26: radius, in hundredths */
    unsigned char unk28[0x30 - 0x28];
} MistParticle;

typedef struct MistSlot {
    MistParticle particles[128];
} MistSlot;

typedef struct MistEmitter {
    unsigned char unk00;
    unsigned char staggered; /* +0x01: nonzero = lives are not reseeded */
    unsigned char unk02[2];
    s32 originX;               /* +0x04 */
    s32 originY;               /* +0x08 */
    s32 originZ;               /* +0x0C */
    s32 velocityX;             /* +0x10 */
    s32 velocityY;             /* +0x14 */
    s32 velocityZ;             /* +0x18 */
    u32 sizeBase;              /* +0x1C */
    u32 sizeRange;             /* +0x20 */
    u32 count;                 /* +0x24 */
    u32 amplitude;             /* +0x28: spread of the spawn points */
    u32 extentY;               /* +0x2C */
    u32 lives;                 /* +0x30: low half total life, high half fade time */
    u32 alpha;                 /* +0x34 */
    u32 pattern;               /* +0x38: low half shape, high half style */
    EmitterMotion *motion;     /* +0x3C */
    s32 slot;                  /* +0x40: 1..8 from xglEffectSlotAlloc */
    unsigned char unk44[0x50 - 0x44];
    union {
        u32 word;
        struct {
            u32 spawning : 1;
            u32 finished : 1;
            u32 unk2 : 1;
            u32 frozen : 2;
            u32 drawn : 1;
        } bits;
    } flags; /* +0x50 */
} MistEmitter;

#define MIST_FLAG_FINISHED 0x02
#define MIST_FLAG_FROZEN 0x18
#define MIST_FLAG_DRAWN 0x20
#define MIST_PI 3.1415927f

static inline void MistAddVector(Vector4 *destination, Vector4 *first, Vector4 *second) {
    Vu0SefAddVectorXYZ(destination, first, second);
}

static inline void MistScaleVector(Vector4 *destination, Vector4 *vector, f32 scale) {
    Vu0SefScaleVectorXYZNI(destination, vector, scale);
}

Vector4 *MMathApplyMatrix(Vector4 *destination, EffectMatrix *matrix, Vector4 *source);
void sefGetSourceMatrix(EffectMatrix *matrix, s32 shape);

/* Draws the mist of one emitter: particles spawn in a cloud around the origin, rise with
   their velocity, hold and vanish, and each one is sent as a nine-spoke sprite. */
static inline s32 MistClampTexel(s32 value, s32 low, s32 high) {
    s32 result;

    if (value < (low << 4) + 16) {
        result = (low << 4) + 16;
    } else {
        result = value;
        if ((high << 4) - 16 < result) {
            result = (high << 4) - 16;
        }
    }
    return result;
}

static inline s32 MistClampPixel(s32 value, s32 low, s32 high) {
    s32 result;

    if (value < low + 1) {
        result = (low << 4) + 16;
    } else {
        s32 clamped = high - 1 < value ? high - 1 : value;

        result = clamped << 4;
    }
    return result;
}

static inline s32 MistClampGsCoord(s32 value, s32 low, s32 high, s32 offset) {
    s32 result;

    if (value >= low) {
        s32 clamped;

        if (high < value) {
            clamped = high;
        } else {
            clamped = value;
        }
        result = (clamped << 4) + offset;
    } else {
        result = (low << 4) + offset;
    }
    return result;
}

void xglMistEmitterDraw(MistEmitter *emitter) {
    s32 originX = emitter->originX;
    s32 originY = emitter->originY;
    s32 originZ = emitter->originZ;
    EffectVector velocity = {
        {(f32) emitter->velocityX, (f32) emitter->velocityY, (f32) emitter->velocityZ, 1.0f}};
    EffectVector offset;
    EffectMatrix matrix;
    AlignedVector4 center;
    f32 scale[4];
    s32 screen[4];
    u32 sizeBase = emitter->sizeBase;
    u32 sizeRange = emitter->sizeRange;
    u32 count = emitter->count;
    u32 amplitude = emitter->amplitude;
    u32 extentY = emitter->extentY;
    u32 totalLife = emitter->lives & 0xFFFF;
    u32 fadeTime = emitter->lives >> 16;
    u32 alpha = emitter->alpha;
    s32 shape = emitter->pattern & 0xFFFF;
    s32 style = emitter->pattern >> 16;
    EmitterMotion *motion = emitter->motion;
    s32 *slotIdPtr = &emitter->slot;
    s32 stopped = !emitter->flags.bits.spawning;
    XglCamera *camera;
    s32 clipLeft;
    s32 clipTop;
    s32 clipRight;
    s32 clipBottom;
    s32 drawCount = 0;
    u32 particleIndex;
    s32 sector;
    MistParticle *particle;
    f32 fade;
    f32 noise;
    f32 angle;
    f32 sine;
    f32 cosine;
    f32 sectorAngle;

    camera = xglStudioSelectGetActiveCamera(0);
    clipLeft = (s32) camera->screen.window.vector.x;
    clipTop = (s32) camera->screen.window.vector.y;
    clipRight = (s32) camera->screen.window.vector.z;
    clipBottom = (s32) camera->screen.window.vector.w;
    emitter->flags.word |= MIST_FLAG_DRAWN;
    if (*slotIdPtr <= 0) {
        *slotIdPtr = xglEffectSlotAlloc();
        if (*slotIdPtr <= 0) {
            return;
        }
    }
    if (count - 1 >= 128) {
        return;
    }
    if (totalLife == 0) {
        return;
    }
    if (totalLife >= 129) {
        return;
    }
    if (sizeBase == 0) {
        return;
    }

    if (xglFilterShouldSkip() == 0) {
        drawEnvSetLarge();
        xglPacketBufferInit(XGL_SCRATCHPAD);
        xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0));
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        xglPacketAppendQuad(GS_TEX0_1,
                            GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, 0x31, 1));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);
    }

    if (motion != 0) {
        func_002018E8(motion->motion[0], motion->motion[1], motion->motion[2], motion->motion[3],
                      motion->motion[4], motion->motion[5]);
        sefGetSourceMatrix(&matrix, shape);
        offset.vector = matrix.row[3];
        matrix.row[3].x = matrix.row[3].y = matrix.row[3].z = 0.0f;
        matrix.row[3].w = 1.0f;
        MistScaleVector(&offset.vector, &offset.vector, 100.0f);
    }
    center.x = (f32) originX;
    center.y = (f32) originY;
    center.z = (f32) originZ;
    center.w = 0.0f;
    MMathApplyMatrix(&center, &matrix, &center);

    for (particleIndex = 0; particleIndex < count; particleIndex++) {
        s32 texelOffsetX;
        s32 texelOffsetY;
        s32 x;
        s32 y;

        particle = &((MistSlot *) &gXglEffectSlots)[(*slotIdPtr) - 1].particles[particleIndex];
        fade = 0.0f;
        if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
            if (emitter->staggered == 0) {
                particle->life = xglSRand() % (totalLife + fadeTime);
                particle->mode = MIST_MODE_SPAWN;
            }
            particle->life -= 1;
        }
        switch (particle->mode) {
        case MIST_MODE_SPAWN:
            if (particle->life > 0 || stopped) {
                continue;
            }
            if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
                noise = sefNoiseNext();
                particle->position.component[0] = center.x + (noise + noise) * (f32) amplitude;
                noise = sefNoiseNext();
                particle->position.component[1] = center.y + (noise + noise) * (f32) amplitude;
                noise = sefNoiseNext();
                particle->position.component[2] = center.z + (noise + noise) * (f32) amplitude;
                particle->life = fadeTime;
                particle->mode = MIST_MODE_APPEAR;
                if (sizeRange != 0) {
                    particle->size = sizeBase + xglSRand() % (sizeRange * 2) - sizeRange;
                } else {
                    particle->size = sizeBase;
                }
                particle->angleX = xglSRand() % 180;
                particle->angleY = xglSRand() % 180;
                MMathApplyMatrix(&particle->velocity, &matrix, &velocity.vector);
                MistAddVector(&particle->position.vector, &particle->position.vector, &offset.vector);
            }
            /* fall through */
        case MIST_MODE_APPEAR:
            if (particle->life > 0) {
                if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
                    MistAddVector(&particle->position.vector, &particle->position.vector,
                                  &particle->velocity);
                }
                angle = MIST_PI / (f32) fadeTime * (f32) particle->life;
                fade = (xglCos(angle) + 1.0f) * 0.5f;
                break;
            }
            if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
                particle->mode = MIST_MODE_HOLD;
                particle->life = totalLife;
            }
            /* fall through */
        case MIST_MODE_HOLD:
            if (particle->life >= fadeTime) {
                if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
                    MistAddVector(&particle->position.vector, &particle->position.vector,
                                  &particle->velocity);
                }
                fade = 1.0f;
                break;
            }
            if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
                particle->mode = MIST_MODE_VANISH;
            }
            /* fall through */
        case MIST_MODE_VANISH:
            if (particle->life > 0) {
                if ((emitter->flags.word & MIST_FLAG_FROZEN) == 0) {
                    MistAddVector(&particle->position.vector, &particle->position.vector,
                                  &particle->velocity);
                }
                angle = MIST_PI / (f32) fadeTime * (f32) (fadeTime - particle->life);
                fade = (xglCos(angle) + 1.0f) * 0.5f;
                break;
            }
            if ((emitter->flags.word & MIST_FLAG_FROZEN) != 0) {
                continue;
            }
            particle->mode = MIST_MODE_SPAWN;
            particle->life = xglSRand() % 30;
            continue;
        default:
            break;
        }
        drawCount++;
        if (xglFilterShouldSkip() != 0) {
            continue;
        }
        memset(screen, 0, 16);
        {
            EffectScreenVector position = {{(s32) particle->position.component[0],
                                            (s32) particle->position.component[1],
                                            (s32) particle->position.component[2], 0}};

            __asm__ volatile("sqc2 $vf0,0(%0)" : : "r"(scale) : "memory");
            scale[0] = (f32) ((double) particle->size * 0.01);
            scale[1] = (f32) ((double) extentY * 0.01);
            if (shape == 8) {
                sdvProjectEffectPoint(screen, position.words, 8, originZ, scale, style, 0);
                scale[0] = (f32) particle->size;
                particle->position.words[2] = originZ;
                scale[1] = (f32) extentY;
            } else {
                sdvProjectEffectPoint(screen, position.words, 0, 0, scale, style, 0);
            }
        }

        x = screen[0];
        y = screen[1];
        cosine = xglCos((f32) particle->angleX * EFFECT_DEGREE);
        texelOffsetX = (s32) (scale[1] * cosine);
        texelOffsetY = (s32) (scale[1] * xglCos((f32) particle->angleY * EFFECT_DEGREE));

        xglPacketBufferInit(XGL_SCRATCHPAD);
        xglPacketAppendEntry(2, 0x10000000, 14, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.workBase, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_TEX0_1,
                            GS_SET_TEX0(D_004C8390.displayBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendEntry(0x8000, 0x308EC000, 0x531, 0);
        xglPacketAppendWordQuad(0x80, 0x80, 0x80, (u32) ((f32) alpha * fade));
        xglPacketAppendWordQuad(MistClampTexel((x << 4) + texelOffsetX, clipLeft, clipRight),
                                MistClampTexel((y << 4) + texelOffsetY, clipTop, clipBottom), 0, 0);
        xglPacketAppendWordQuad(MistClampGsCoord(x, clipLeft, clipRight, 0x6FF8),
                                MistClampGsCoord(y, clipTop, clipBottom, 0x71F7), screen[2], 0);
        for (sector = 0; sector < 9; sector++) {
            sectorAngle = (f32) sector * EFFECT_PI_OVER_4;
            sine = xglSin(sectorAngle);
            x = (s32) ((f32) screen[0] + scale[0] * sine);
            cosine = xglCos(sectorAngle);
            y = (s32) ((f32) screen[1] + scale[0] * cosine);
            xglPacketAppendWordQuad(0x80, 0x80, 0x80, 0);
            xglPacketAppendWordQuad(MistClampPixel(x, clipLeft, clipRight),
                                    MistClampPixel(y, clipTop, clipBottom), 0, 0);
            xglPacketAppendWordQuad(MistClampGsCoord(x, clipLeft, clipRight, 0x6FF8),
                                    MistClampGsCoord(y, clipTop, clipBottom, 0x71F7), screen[2], 0);
        }

        /* Draw it to the work page, then the same packet from the work page to the displayed one. */
        SCRATCHPAD_WORD(12) |= 0xA;
        xglPacketSendDirect(0, 0);
        SCRATCHPAD_WORD(13) = 0x30AEC000;
        SCRATCHPAD_DOUBLEWORDS[2] = GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000);
        SCRATCHPAD_DOUBLEWORDS[4] = GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0);
        xglPacketSendDirect(0, 0);
    }
    if (stopped && drawCount == 0) {
        emitter->flags.word |= MIST_FLAG_FINISHED;
    }
}

typedef struct RingFadeParams {
    s32 header;    /* +0x00: not read by this function */
    s32 tag;       /* +0x04: copied into each vertex record */
    s32 offsetX;   /* +0x08 */
    s32 offsetY;   /* +0x0C */
    s32 ratioX;    /* +0x10 */
    s32 ratioY;    /* +0x14 */
    s32 phase;     /* +0x18: angle multiplier of the fade */
    s32 fadeHigh;  /* +0x1C: high word of the blend quad */
    s32 blendMode; /* +0x20: 1 -> 0x68, 2 -> 0x62, else 0x64 */
    s32 useField;  /* +0x24 */
} RingFadeParams;

static inline s32 ringCoord(s32 value, s32 low, s32 high, s32 bias) {
    s32 result;

    if (value >= low) {
        result = ((value > high) ? high : value) * 16 + bias;
    } else {
        result = low * 16 + bias;
    }
    return result;
}

#define CLAMP(v, lo, hi) (((v) < (lo)) ? (lo) : (((v) > (hi)) ? (hi) : (v)))
#define SCRATCH ((s32 *)XGL_SCRATCHPAD)

/* Screen filter 26, REFRACT in the game's filter name list.
 * Draws the fade ring: a GS state packet, then 36 angle steps of vertex strips over the camera window, one VIF direct
 * transfer per step. */
void filterRefractDraw(RingFadeParams *params, s32 packet) {
    s32 tag = params->tag;
    s32 offsetX = params->offsetX;
    s32 offsetY = params->offsetY;
    s32 ratioX = params->ratioX;
    s32 ratioY = params->ratioY;
    s32 phase = params->phase;
    s32 fadeHigh = params->fadeHigh;
    s32 blendMode = params->blendMode;
    s32 useField = params->useField;
    f32 scaleX;
    f32 scaleY;
    f32 angleStep;
    f32 sinA;
    f32 cosA;
    f32 sinB;
    f32 cosB;
    f32 alpha;
    s32 alphaOn = 1;
    f32 stepX;
    f32 stepY;
    s32 ringSteps;
    s32 camLeft;
    s32 camTop;
    s32 camRight;
    s32 camBottom;
    s32 vertexCount;
    s32 *vertex;
    s32 i;
    s32 j;
    XglCamera *camera;

    if (ratioX < ratioY) {
        scaleX = (f32) ratioX / (f32) ratioY;
    } else {
        scaleX = 1.0f;
    }
    if (ratioY < ratioX) {
        scaleY = (f32) ratioY / (f32) ratioX;
    } else {
        scaleY = 1.0f;
    }
    ringSteps = ((ratioX < ratioY) ? ratioY : ratioX) / 16;

    camera = xglStudioSelectGetActiveCamera(0);
    camLeft = camera->screen.window.vector.x;
    camTop = camera->screen.window.vector.y;
    camRight = camera->screen.window.vector.z;
    camBottom = camera->screen.window.vector.w;

    if (xglFilterShouldSkip() == 0) {
        if (xglRenderMovieIsActive() != 0) {
            if (useField != 0) {
                drawEnvSetHalfDisplayNext();
            } else {
                drawEnvSetLarge();
            }
        } else if (useField != 0) {
            drawEnvSetField();
        } else {
            drawEnvSetLarge();
        }

        xglPacketBufferInit(XGL_SCRATCHPAD);
        xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
        switch (blendMode) {
        case 1:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, fadeHigh));
            break;
        case 2:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, fadeHigh));
            break;
        default:
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, fadeHigh));
            break;
        }
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        vertex = (s32 *) XGL_SCRATCHPAD;
        angleStep = 0.17453289f;
        xglPacketAppendQuad(GS_TEX0_1,
                            GS_SET_TEX0(D_004C8390.workBase << 5, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
        xglPacketAppendQuad(GS_SCISSOR_1, GS_SET_SCISSOR(0, 0x1FF, 0, 0x1BF));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);

        stepX = scaleX * 16.0f;
        stepY = scaleY * 16.0f;
        vertex[0] = 0x8000;
        vertex[1] = 0x20AE4000;
        vertex[2] = 0x53;
        vertex[3] = 0;

        for (i = 0; i < 36; i++) {
            vertex = SCRATCH + 4;
            vertexCount = 0;
            sinA = xglSin((f32) i * angleStep);
            cosA = xglCos((f32) i * angleStep);
            sinB = xglSin((f32) (i + 1) * angleStep);
            cosB = xglCos((f32) (i + 1) * angleStep);
            {
                for (j = 0; j <= ringSteps; j++) {
                    s32 ringAX;
                    s32 ringAY;
                    s32 ringBX;
                    s32 ringBY;
                    s32 fadedAX;
                    s32 fadedAY;
                    s32 fadedBX;
                    s32 fadedBY;

                    ringAX = (s32) ((f32) (ringSteps - j) * stepX * cosA);
                    ringAY = (s32) ((f32) (ringSteps - j) * stepY * sinA);
                    ringBX = (s32) ((f32) (ringSteps - j) * stepX * cosB);
                    ringBY = (s32) ((f32) (ringSteps - j) * stepY * sinB);
                    alpha = 0.0f;
                    if (alphaOn == 1) {
                        alpha = xglCos((f32) phase * 0.012319972f) *
                                (1.0f - xglCos((1.5707964f / (f32) ringSteps) * (f32) j));
                    }
                    fadedAX = (s32) ((f32) (ringAX << 4) + (f32) (ratioX << 4) * cosA * alpha);
                    fadedAY = (s32) ((f32) (ringAY << 4) + (f32) (ratioY << 4) * sinA * alpha);
                    fadedBX = (s32) ((f32) (ringBX << 4) + (f32) (ratioX << 4) * cosB * alpha);
                    fadedBY = (s32) ((f32) (ringBY << 4) + (f32) (ratioY << 4) * sinB * alpha);
                    ringAX += offsetX;
                    ringBX += offsetX;
                    ringBY += offsetY;
                    ringAY += offsetY;

                    if ((((u32) (ringAX + 0x10) < 0x221U) || ((u32) (ringBX + 0x10) < 0x221U)) &&
                        (((u32) (ringAY + 0x10) < 0x1E1U) || ((u32) (ringBY + 0x10) < 0x1E1U))) {
                        vertex[0] = CLAMP(offsetX * 16 + fadedAX, camLeft * 16 + 16, camRight * 16 - 16);
                        vertex[1] = CLAMP(offsetY * 16 + fadedAY, camTop * 16 + 16, camBottom * 16 - 16);
                        vertex[2] = 0;
                        vertex[3] = 0;
                        vertex[4] = ringCoord(ringAX, camLeft, camRight, 0x6FF8);
                        vertex[5] = ringCoord(ringAY, camTop, camBottom, 0x71F7);
                        vertex[6] = tag;
                        vertex[7] = 0;
                        vertex += 8;
                        vertexCount++;
                        vertex[0] = CLAMP(offsetX * 16 + fadedBX, camLeft * 16 + 16, camRight * 16 - 16);
                        vertex[1] = CLAMP(offsetY * 16 + fadedBY, camTop * 16 + 16, camBottom * 16 - 16);
                        vertex[2] = 0;
                        vertex[3] = 0;
                        vertex[4] = ringCoord(ringBX, camLeft, camRight, 0x6FF8);
                        vertex[5] = ringCoord(ringBY, camTop, camBottom, 0x71F7);
                        vertex[6] = tag;
                        vertex[7] = 0;
                        vertex += 8;
                        vertexCount++;
                    }
                }
            }
            SCRATCH[0] = (SCRATCH[0] & 0x8000) | vertexCount;
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            sceVif1PkAddDirectDataN(
                packet, XGL_SCRATCHPAD, (vertexCount * 2) + 1);
            sceVif1PkCloseDirectHLCode(packet);
        }
    }
}

typedef struct FilterRing {
    u8 unk00[0x4];
    s32 frameTag;
    s32 count;
    s32 radiusX;
    s32 radiusY;
} FilterRing;

static inline void queueFilterDraw(s32 blendFix, s32 blendMode, s32 sourceX0, s32 sourceY0, s32 sourceX1,
                                   s32 sourceY1, s32 destinationX0, s32 destinationY0, s32 destinationX1,
                                   s32 destinationY1) {
    DrawEnvRecord *drawRecord;

    drawRecord = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();
    drawRecord->blendFix = blendFix;
    drawRecord->blendMode = blendMode;
    drawRecord->sourceRect[0] = sourceX0;
    drawRecord->sourceRect[1] = sourceY0;
    drawRecord->sourceRect[2] = sourceX1;
    drawRecord->sourceRect[3] = sourceY1;
    drawRecord->destinationRect[0] = destinationX0;
    drawRecord->destinationRect[1] = destinationY0;
    drawRecord->destinationRect[2] = destinationX1;
    drawRecord->destinationRect[3] = destinationY1;
}

/* Draws the filter ring: a 37-point circle packet over the work page, then count passes of two queued draws. */
void func_001E0E10(FilterRing *ring) {
    s32 frameTag;
    s32 count;
    s32 radiusX;
    s32 radiusY;
    s32 i;
    s32 remaining;
    f32 angle;
    f32 sinAngle;
    f32 cosAngle;
    s32 pointX;
    s32 pointY;

    frameTag = ring->frameTag;
    count = ring->count;
    radiusX = ring->radiusX;
    radiusY = ring->radiusY;
    if (xglFilterShouldSkip()) {
        return;
    }
    drawEnvSetLarge();
    drawRectOnPage(D_004C8390.workBase, 192, 0, 0, 512, 448);
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.workBase, 8, 0, 0xFFFFFF));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 128));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(5, 1, 0, 0, 1, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0, 0x3F800000));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x7FF7, 0));
    for (i = 0; i < 37; i++) {
        angle = (f32) i * 0.17453289f;
        sinAngle = xglSin(angle);
        cosAngle = xglCos(angle);
        pointX = (s32) (radiusX * cosAngle);
        pointY = (s32) (radiusY * sinAngle);
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0xC0, 0x3F800000));
        pointY += 224;
        pointY <<= 4;
        pointY += 0x71F7;
        pointX += 256;
        pointX <<= 4;
        pointX += 0x6FF8;
        {
            s64 vertexX = pointX;
            s64 vertexY = pointY;

            vertexY <<= 16;
            xglPacketAppendQuad(GS_XYZF2, vertexX | vertexY);
        }
    }
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
    if (count > 0) {
        remaining = count;
        do {
            remaining--;
            queueFilterDraw(0x81, 3, 0, 0, 512, 448, 0, 0, 256, 224);
            func_001D54C8();
            gDrawArea.unk58 = 0x50000;
            gDrawArea.flags.pageOffset = frameTag;
            queueFilterDraw(0x80, 3, 0, 0, 256, 224, 0, 0, 512, 448);
            func_001D57E0();
            gDrawArea.unk58 = 0x30000;
        } while (remaining != 0);
    }
    queueFilterDraw(0x80, 3, 0, 0, 512, 448, 0, 0, 512, 448);
    do {
        func_001D55E8();
    } while (0);
}

/* The color filter settings: brightness, contrast and a tint color. */
typedef struct ColorFilter {
    u8 unk00[4];
    s32 brightness; /* +0x04: added to the 0x80 (1.0) blend factor */
    s32 contrast;   /* +0x08: > 0 raises, < 0 lowers the contrast */
    s32 red;        /* +0x0C: the hue in degrees when alpha is 0 */
    s32 green;      /* +0x10: the saturation when alpha is 0 */
    s32 blue;       /* +0x14: the value - 0x80 when alpha is 0 */
    s32 alpha;      /* +0x18: 0 = the tint is hue, saturation and value with alpha 0x80 */
    u8 unk1C[0x50 - 0x1C];
    u32 unk50_0 : 2;
    u32 composite : 1; /* +0x50 bit 2: filter the work page, then blend it onto the displayed page */
} ColorFilter;

/* Queues the copy of one column of the source page, rows top to bottom, to the top half of the frame page. */
static inline void xglFilterQueueColumnCopy(s32 workBase, s32 frameBase, s32 left, s32 top, s32 right, s32 bottom)
{
    DrawEnvRecord *drawRecord;

    drawRecord = drawEnvGetRecord();
    drawRecord->workBase = workBase;
    drawRecord->frameBase = frameBase;
    drawRecord->blockWidth = 8;
    drawRecord->unk44 = 0;
    drawRecord->blockHeight = 8;
    drawRecord->unk48 = 0;
    drawRecord->blendFix = 0x80;
    drawRecord->blendMode = 3;
    drawRecord->sourceRect[0] = left;
    drawRecord->sourceRect[1] = top;
    drawRecord->sourceRect[2] = right;
    drawRecord->sourceRect[3] = bottom;
    drawRecord->destinationRect[0] = left;
    drawRecord->destinationRect[1] = 0;
    drawRecord->destinationRect[2] = right;
    drawRecord->destinationRect[3] = 0xE0;
    drawEnvSendRecordPacket();
}

/* Opens a direct GS packet of A+D register writes in the scratchpad. */
static inline void xglFilterBegin(void)
{
    xglPacketBufferInit(XGL_SCRATCHPAD);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
}

/* Closes the packet and sends it to the GS. */
static inline void xglFilterEnd(void)
{
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* One untextured sprite from column left to column right (pixels) and from row top to row bottom (GS, 1/16 pixel). */
#define XGL_FILTER_FILL_SPRITE(left, top, right, bottom)                                                    \
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ((left) * 16 + 0x6FF8, top, 0));                                \
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ((right) * 16 + 0x6FF8, bottom, 0))
/* One textured sprite 64 pixels wide: columns left to right (pixels), rows top to bottom (GS, 1/16 pixel); the
 * texture spans the same columns and rows 0 to textureBottom. */
#define XGL_FILTER_TEXTURE_SPRITE(left, top, right, bottom, textureBottom)                                  \
    xglPacketAppendQuad(GS_UV, GS_SET_UV((left) * 16, 0));                                                  \
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ((left) * 16 + 0x6FF8, top, 0));                                \
    xglPacketAppendQuad(GS_UV, GS_SET_UV(((left) + 0x40) * 16, textureBottom));                             \
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ((right) * 16 + 0x6FF8, bottom, 0))
/* Untextured sprites over the screen width, in strips 64 pixels wide, from row top to row bottom; x is the caller's
 * column counter. */
#define XGL_FILTER_FILL_STRIPS(top, bottom)                                                                 \
    for (x = 0; x < 0x200; x += 0x40) {                                                                    \
        s32 right = x + 0x40;                                                                               \
        XGL_FILTER_FILL_SPRITE(x, top, right, bottom);                                                      \
    }
/* Textured sprites over the screen width in strips 64 pixels wide; the texture spans rows 0 to textureBottom. */
#define XGL_FILTER_TEXTURE_STRIPS(top, textureBottom, bottom)                                               \
    for (x = 0; x < 0x200; x += 0x40) {                                                                    \
        s32 right = x + 0x40;                                                                               \
        XGL_FILTER_TEXTURE_SPRITE(x, top, right, bottom, textureBottom);                                    \
    }
/* Inverts the page: white minus the frame. */
#define XGL_FILTER_INVERT(page)                                                                             \
    {                                                                                                       \
        xglFilterBegin();                                                                                   \
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(page, 8, 0, 0xFF000000));                              \
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 2, 0x80));                                    \
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0x3F800000));                    \
        xglPacketAppendQuad(GS_PRIM, 0x46);                                                                 \
        XGL_FILTER_FILL_STRIPS(0x71F7, 0x8DF7);                                                             \
        xglFilterEnd();                                                                                     \
    }
/* Keeps the part of the page above mid gray: the frame minus 0x80, from row top to row bottom. */
#define XGL_FILTER_THRESHOLD(page, top, bottom)                                                             \
    {                                                                                                       \
        xglFilterBegin();                                                                                   \
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(page, 8, 0, 0xFF000000));                              \
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(1, 0, 2, 2, 0x80));                                    \
        xglPacketAppendQuad(GS_PRIM, 0x146);                                                                \
        XGL_FILTER_FILL_STRIPS(top, bottom);                                                                \
        xglFilterEnd();                                                                                     \
    }
/* Adds (contrast > 0) or subtracts the thresholded texture page, times the contrast, to the destination page. */
#define XGL_FILTER_ADD_SCALED(destinationPage, texturePage, contrast, top, textureBottom, bottom)            \
    {                                                                                                       \
        xglFilterBegin();                                                                                   \
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(destinationPage, 8, 0, 0xFF000000));                   \
        xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0((texturePage) * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));   \
        xglPacketAppendQuad(GS_TEX1_1, 1);                                                                  \
        if ((contrast) > 0) {                                                                               \
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, contrast));                            \
        } else {                                                                                            \
            xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 2, 1, -(contrast)));                         \
        }                                                                                                   \
        xglPacketAppendQuad(GS_PRIM, 0x156);                                                                \
        XGL_FILTER_TEXTURE_STRIPS(top, textureBottom, bottom);                                              \
        xglFilterEnd();                                                                                     \
    }
/* The contrast step on the work page: each half of sourcePage is copied to the frame page, thresholded there and
 * added back onto its half of the work page. */
#define XGL_FILTER_CONTRAST_HALVES(sourcePage, contrast)                                                   \
    {                                                                                                       \
        s32 half;                                                                                           \
        s32 top;                                                                                            \
        s32 bottom;                                                                                         \
                                                                                                            \
        for (half = 0, top = 0; half < 2; half++) {                                                         \
            for (x = 0, bottom = top + 0xE0; x < 0x200; x += 0x40) {                                        \
                xglFilterQueueColumnCopy(sourcePage, D_004C8390.frameBase, x, top, x + 0x40, bottom);       \
            }                                                                                               \
            XGL_FILTER_THRESHOLD(D_004C8390.frameBase, 0x71F7, 0x7FF7);                                     \
            XGL_FILTER_ADD_SCALED(D_004C8390.workBase, D_004C8390.frameBase, contrast, top * 16 + 0x71F7,   \
                                  0xE00, bottom * 16 + 0x71F7);                                             \
            top = bottom;                                                                                   \
        }                                                                                                   \
    }
/* The contrast step on the displayed page itself. */
#define XGL_FILTER_CONTRAST_DIRECT(contrast)                                                                \
    {                                                                                                       \
        drawEnvSetLarge();                                                                                  \
        XGL_FILTER_THRESHOLD(D_004C8390.workBase, 0x71F7, 0x8DF7);                                          \
        XGL_FILTER_ADD_SCALED(D_004C8390.displayBase, D_004C8390.workBase, contrast, 0x71F7, 0x1C00, 0x8DF7); \
    }

/* Screen filter 29, CORRECT in the game's filter name list.
 * Applies the colour filter (brightness, contrast, tint) to the frame with GS direct packets; with the composite flag it
 * works on the work page and blends the result onto the displayed page. */
void filterCorrectDraw(ColorFilter *filter)
{
    s32 hue = filter->red % 360;
    s32 saturation = filter->green & 0xFF;
    s32 maxChannel = (filter->blue + 0x80) & 0xFF;
    s32 alpha = 0x80;
    s32 tintAlpha = filter->alpha;
    s32 red;
    s32 green;
    s32 blue;
    s32 brightness = filter->brightness;
    s32 contrast = filter->contrast;
    s32 page;
    s32 x;

    green = saturation;
    if (tintAlpha != 0) {
        alpha = tintAlpha & 0xFF;
    }
    red = filter->red & 0xFF;
    blue = filter->blue & 0xFF;
    if (xglFilterShouldSkip()) {
        return;
    }
    if (filter->composite) {
        drawEnvSetLarge();
    }

    xglFilterBegin();
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_COLCLAMP, 1);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    xglPacketAppendQuad(GS_TEX1_1, 1);
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    xglFilterEnd();

    if (brightness != 0) {
        page = filter->composite ? D_004C8390.workBase : D_004C8390.displayBase;
        xglFilterBegin();
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(page, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, brightness + 0x80));
        xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(page * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_PRIM, 0x156);
        XGL_FILTER_TEXTURE_STRIPS(0x71F7, 0x1C00, 0x8DF7);
        xglFilterEnd();
    }

    if (contrast != 0) {
        if (filter->composite) {
            XGL_FILTER_CONTRAST_HALVES(D_004C8390.displayBase, contrast);
        } else {
            XGL_FILTER_CONTRAST_DIRECT(contrast);
        }
        if (filter->composite) {
            XGL_FILTER_INVERT(D_004C8390.workBase);
            XGL_FILTER_CONTRAST_HALVES(D_004C8390.workBase, contrast);
            XGL_FILTER_INVERT(D_004C8390.workBase);
        } else {
            XGL_FILTER_INVERT(D_004C8390.displayBase);
            XGL_FILTER_CONTRAST_DIRECT(contrast);
            XGL_FILTER_INVERT(D_004C8390.displayBase);
        }
    }

    if (tintAlpha != 0 ? (red != 0x80 || green != 0x80 || blue != 0x80)
                       : (hue != 0 || saturation != 0 || maxChannel != 0x80)) {
        page = filter->composite ? D_004C8390.workBase : D_004C8390.displayBase;
        if (tintAlpha == 0) {
            colorHsvToRgb(hue, saturation, maxChannel, &red, &green, &blue);
        }
        xglFilterBegin();
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(page, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, alpha));
        xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(page * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_TEX1_1, 1);
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, 0x80, XGL_FLOAT_ONE_BITS));
        xglPacketAppendQuad(GS_PRIM, 0x156);
        XGL_FILTER_TEXTURE_STRIPS(0x71F7, 0x1C00, 0x8DF7);
        xglFilterEnd();
    }

    if (filter->composite) {
        xglFilterBegin();
        xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.workBase * 32, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 1, 1, 0));
        xglPacketAppendQuad(GS_PRIM, 0x156);
        XGL_FILTER_TEXTURE_STRIPS(0x71F7, 0x1C00, 0x8DF7);
        xglFilterEnd();
    }
}

void gifStateFlush(void);
void gifStateReset(void);
void gifStateAddReg(int gifAddress, u64 gifData);
XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

/* The pages of D_004C8390 the blur passes use (xgl_render's depth, work, frame and display bases). */
#define BLUR_DEPTH_PAGE D_004C8390.depthBase
#define BLUR_WORK_PAGE D_004C8390.workBase
#define BLUR_FRAME_PAGE D_004C8390.frameBase
#define BLUR_DISPLAY_PAGE D_004C8390.displayBase
/* The 512x448 frame buffer in pixels, centred on the 2048 GS origin. */
#define BLUR_HEIGHT 448
/* The shrunk copy: a quarter of the frame in each direction, never smaller than the minimum. */
#define BLUR_SHRUNK_WIDTH 128
#define BLUR_SHRUNK_HEIGHT 112
#define BLUR_MIN_SHRUNK_WIDTH 64
#define BLUR_MIN_SHRUNK_HEIGHT 56
#define BLUR_X(x) (((2048 - WAVE_CELL / 2 + (x)) << 4) - 8)
/* Camera window corners in GS coordinates. */
#define BLUR_WINDOW_LEFT_X(camera) BLUR_X((int) (camera)->screen.window.vector.x)
#define BLUR_Y(y) (((2048 - BLUR_HEIGHT / 2 + (y)) << 4) - 9)
#define BLUR_WINDOW_TOP_Y(camera) BLUR_Y((int) (camera)->screen.window.vector.y)
#define BLUR_WINDOW_RIGHT_X(camera) BLUR_X((int) (camera)->screen.window.vector.z)
#define BLUR_WINDOW_BOTTOM_Y(camera) BLUR_Y((int) (camera)->screen.window.vector.w)

/* Opens a packet that blends into page BLUR_WORK_PAGE from page BLUR_FRAME_PAGE at alpha. */
void blurBeginBlendPass(XglPacket *packet, int alpha) {
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_WORK_PAGE, 8, GS_PSMCT32, 0));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
}

void gifStateFlushAndCloseDirect(XglPacket *packet) {
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Queues one textured sprite of width x height texels drawn at (offsetX, offsetY) into the open packet. */
void blurQueueSprite(XglPacket *packet, unsigned int width, unsigned int height, int offsetX, int offsetY,
                     int alpha) {
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(offsetX), BLUR_Y(offsetY), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(width * 16, height * 16));
    gifStateAddReg(GS_XYZF2,
                   GS_SET_XYZ(BLUR_X(offsetX + BLUR_SHRUNK_WIDTH), BLUR_Y(offsetY + BLUR_SHRUNK_HEIGHT), 0));
}

/* Where the pass copies are offset: radial = eight neighbours, diagonal = four corners, horizontal/vertical = sides. */
typedef enum BlurOffsetPattern {
    BLUR_PATTERN_RADIAL,
    BLUR_PATTERN_DIAGONAL,
    BLUR_PATTERN_HORIZONTAL,
    BLUR_PATTERN_VERTICAL
} BlurOffsetPattern;

/* Parameter block of filterSoftFocusDraw. */
typedef struct BlurParams {
    s32 unusedWord00;
    s32 alpha;         /* +0x04 */
    s32 offsetPattern; /* +0x08: BlurOffsetPattern */
    s32 passCount;     /* +0x0C */
} BlurParams;

/* One blur pass tail: the copy size is the full size divided by the pass number (not below the minimum), then the
 * work page is drawn into the frame page at that size, blended at alpha. */
#define BLUR_SHRINK_PASS(packet, fullWidth, fullHeight, passIndex, alpha, width, height) \
    { \
        width = fullWidth / passIndex; \
        if (width < BLUR_MIN_SHRUNK_WIDTH) { \
            width = BLUR_MIN_SHRUNK_WIDTH; \
        } \
        height = fullHeight / passIndex; \
        if (height < BLUR_MIN_SHRUNK_HEIGHT) { \
            height = BLUR_MIN_SHRUNK_HEIGHT; \
        } \
        sceVif1PkCnt(packet, 0); \
        sceVif1PkOpenDirectHLCode(packet, 0); \
        gifStateReset(); \
        gifStateAddReg(GS_TEXFLUSH, 0); \
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x80)); \
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0xFF000000)); \
        gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0)); \
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS)); \
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0)); \
        gifStateAddReg(GS_UV, GS_SET_UV(0, 0)); \
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0)); \
        gifStateAddReg(GS_UV, GS_SET_UV(BLUR_SHRUNK_WIDTH * 16, BLUR_SHRUNK_HEIGHT * 16)); \
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(width), BLUR_Y(height), 0)); \
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha)); \
        gifStateFlush(); \
        sceVif1PkCloseDirectHLCode(packet); \
    }

/* Screen filter 30, SOFTFOCUS in the game's filter name list.
 * Shrinks the camera window into the corner of page BLUR_FRAME_PAGE, darkens page BLUR_WORK_PAGE, then blends the
 * shrunk copy back passCount - 1 times with offsets of the pass number (the pattern picked by offsetPattern), shrinking it
 * again after every pass, and finally stretches the result back over the camera window. */
void filterSoftFocusDraw(BlurParams *blurParams, XglPacket *packet) {
    int offsetPattern;
    int passCount;
    int alpha;
    XglCamera *camera;
    int fullWidth;
    int fullHeight;
    int width;
    int height;
    int passIndex;

    alpha = blurParams->alpha;
    offsetPattern = blurParams->offsetPattern;
    passCount = blurParams->passCount;
    camera = xglStudioSelectGetActiveCamera(0);

    if (xglFilterShouldSkip() != 0) {
        return;
    }
    fullWidth = (camera->screen.window.vector.z - camera->screen.window.vector.x) / WAVE_CELL * 256.0f;
    fullHeight =
        (camera->screen.window.vector.w - camera->screen.window.vector.y) * (1.0f / BLUR_HEIGHT) * 224.0f;

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_FBA_1, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(BLUR_DEPTH_PAGE, GS_PSMZ24, 1));
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL - 1, 0, BLUR_HEIGHT - 1));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x80));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_DISPLAY_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV,
                   GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
    gifStateAddReg(GS_UV,
                   GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(fullWidth), BLUR_Y(fullHeight), 0));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 13));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_WORK_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(148), BLUR_Y(132), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    width = fullWidth;
    height = fullHeight;
    switch (offsetPattern) {
    case BLUR_PATTERN_RADIAL:
        for (passIndex = 1; passIndex < passCount; passIndex++) {
            blurBeginBlendPass(packet, alpha);
            if (passIndex >= 3) {
                blurQueueSprite(packet, width, height, passIndex, 0, alpha);
                blurQueueSprite(packet, width, height, -passIndex, 0, alpha);
                blurQueueSprite(packet, width, height, 0, passIndex, alpha);
                blurQueueSprite(packet, width, height, 0, -passIndex, alpha);
            }
            blurQueueSprite(packet, width, height, passIndex, passIndex, alpha);
            blurQueueSprite(packet, width, height, -passIndex, passIndex, alpha);
            blurQueueSprite(packet, width, height, passIndex, -passIndex, alpha);
            blurQueueSprite(packet, width, height, -passIndex, -passIndex, alpha);
            gifStateFlushAndCloseDirect(packet);
            BLUR_SHRINK_PASS(packet, fullWidth, fullHeight, passIndex, alpha, width, height);
        }
        break;
    case BLUR_PATTERN_DIAGONAL:
        for (passIndex = 1; passIndex < passCount; passIndex++) {
            blurBeginBlendPass(packet, alpha);
            blurQueueSprite(packet, width, height, passIndex, passIndex, alpha);
            blurQueueSprite(packet, width, height, -passIndex, passIndex, alpha);
            blurQueueSprite(packet, width, height, passIndex, -passIndex, alpha);
            blurQueueSprite(packet, width, height, -passIndex, -passIndex, alpha);
            gifStateFlushAndCloseDirect(packet);
            BLUR_SHRINK_PASS(packet, fullWidth, fullHeight, passIndex, alpha, width, height);
        }
        break;
    case BLUR_PATTERN_HORIZONTAL:
        for (passIndex = 1; passIndex < passCount; passIndex++) {
            blurBeginBlendPass(packet, alpha);
            blurQueueSprite(packet, width, height, passIndex, 0, alpha);
            blurQueueSprite(packet, width, height, -passIndex, 0, alpha);
            blurQueueSprite(packet, width, height, passIndex, 0, alpha);
            blurQueueSprite(packet, width, height, -passIndex, 0, alpha);
            gifStateFlushAndCloseDirect(packet);
            BLUR_SHRINK_PASS(packet, fullWidth, fullHeight, passIndex, alpha, width, height);
        }
        break;
    case BLUR_PATTERN_VERTICAL:
        for (passIndex = 1; passIndex < passCount; passIndex++) {
            blurBeginBlendPass(packet, alpha);
            blurQueueSprite(packet, width, height, 0, passIndex, alpha);
            blurQueueSprite(packet, width, height, 0, -passIndex, alpha);
            blurQueueSprite(packet, width, height, 0, passIndex, alpha);
            blurQueueSprite(packet, width, height, 0, -passIndex, alpha);
            gifStateFlushAndCloseDirect(packet);
            BLUR_SHRINK_PASS(packet, fullWidth, fullHeight, passIndex, alpha, width, height);
        }
        break;
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1,
                   GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
                                  camera->screen.window.vector.y, camera->screen.window.vector.w));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, BLUR_SHRUNK_WIDTH, 0, BLUR_SHRUNK_HEIGHT));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x80));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(BLUR_SHRUNK_WIDTH * 16, BLUR_SHRUNK_HEIGHT * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL, 0, BLUR_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Parameter block of filterScreenBlurDraw. */
typedef struct FocusBlurParams {
    s32 unusedWord00;
    u32 depth;     /* +0x04: Z of the sharp plane */
    u32 passCount; /* +0x08 */
    s32 alpha;     /* +0x0C: FIX of the final blend */
    s32 depthTest; /* +0x10: nonzero tests the destination alpha instead of the depth */
    u32 blendMode; /* +0x14 */
} FocusBlurParams;

/* Screen filter 33, SCR BLUR in the game's filter name list.
 * Marks the pixels behind the sharp plane in the alpha channel of page BLUR_DISPLAY_PAGE, shrinks the frame into
 * page BLUR_FRAME_PAGE, blurs it there passCount + 1 times through page BLUR_WORK_PAGE, and blends the result
 * back over the camera window where the alpha (or the depth) allows it. */
void filterScreenBlurDraw(FocusBlurParams *blurParams, XglPacket *packet) {
    u32 depth;
    u32 passCount;
    int alpha;
    int depthTest;
    u32 blendMode;
    XglCamera *camera;
    u32 blurPass;
    int width;
    int height;
    int alphaSourceFix;

    depth = blurParams->depth;
    passCount = blurParams->passCount;
    alpha = blurParams->alpha;
    depthTest = blurParams->depthTest;
    blendMode = blurParams->blendMode;
    camera = xglStudioSelectGetActiveCamera(0);

    if (xglFilterShouldSkip() != 0) {
        return;
    }

    if (depthTest) {
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        gifStateReset();
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFFFFFF));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_XYZF2,
                       GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_FBA_1, 1);
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFFFFFF));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), depth));
        gifStateAddReg(GS_XYZF2,
                       GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), depth));
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(1, 3, 1, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_XYZF2,
                       GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL, 0, BLUR_HEIGHT));
        gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0));
        gifStateAddReg(GS_TEX0_1,
                       GS_SET_TEX0(BLUR_DISPLAY_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(266), BLUR_Y(234), 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
        gifStateAddReg(
            GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(
            GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(256), BLUR_Y(224), 0));
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
    } else {
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        gifStateReset();
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL, 0, BLUR_HEIGHT));
        gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0));
        gifStateAddReg(GS_TEX0_1,
                       GS_SET_TEX0(BLUR_DISPLAY_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(266), BLUR_Y(234), 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
        gifStateAddReg(
            GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(
            GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(256), BLUR_Y(224), 0));
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL, 0, BLUR_HEIGHT));
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_WORK_PAGE, 8, GS_PSMCT32, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(266), BLUR_Y(234), 0));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, 319, 0, 279));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    for (blurPass = 0; blurPass <= passCount; blurPass++) {
        if (depthTest) {
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_WORK_PAGE, 8, GS_PSMCT32, 0));
            gifStateAddReg(GS_TEX0_1,
                           GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, XGL_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(330), BLUR_Y(290), 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(256 * 16, 224 * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(320), BLUR_Y(280), 0));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);

            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0));
            gifStateAddReg(GS_TEX0_1,
                           GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(320 * 16, 280 * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(256), BLUR_Y(224), 0));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
        } else {
            height = 560 / (int) (blurPass + 2);
            width = 640 / (int) (blurPass + 2);
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_WORK_PAGE, 8, GS_PSMCT32, 0));
            gifStateAddReg(GS_TEX0_1,
                           GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, XGL_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(width + 10), BLUR_Y(height + 10), 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(256 * 16, 224 * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0) + width * 16, BLUR_Y(0) + height * 16, 0));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);

            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0));
            gifStateAddReg(GS_TEX0_1,
                           GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(width * 16, height * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(256), BLUR_Y(224), 0));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
        }
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1,
                   GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
                                  camera->screen.window.vector.y, camera->screen.window.vector.w));
    gifStateAddReg(GS_TEXFLUSH, 0);
    if (depthTest) {
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 1, 0, 1, 1));
    } else {
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
    }
    alphaSourceFix = 2;
    switch (blendMode) {
    case 0:
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, alphaSourceFix, 1, alpha));
        break;
    case 1:
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, alphaSourceFix, 1, alpha));
        break;
    case 2:
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(2, 0, alphaSourceFix, 1, alpha));
        break;
    default:
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, alphaSourceFix, 1, alpha));
        break;
    }
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), depth));
    gifStateAddReg(GS_UV, GS_SET_UV(256 * 16, 224 * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), depth));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL, 0, BLUR_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

typedef struct BlurTintParams {
    u8 unmodeled_00[4];
    u8 red; /* +0x04 */
    u8 unmodeled_05[3];
    u8 green; /* +0x08 */
    u8 unmodeled_09[3];
    u8 blue; /* +0x0C */
    u8 unmodeled_0D[3];
    u8 fix; /* +0x10: FIX of ALPHA_1 */
    u8 unmodeled_11[3];
    u8 blendMode; /* +0x14 */
} BlurTintParams;

u64 func_003098D0(f32 value);

/* Screen filter 34, COLOR in the game's filter name list.
 * Draws a copy of the camera window tinted with the red, green and blue fields of tint into the display page. The
 * blend is picked by blendMode (0, 1, 2; other values behave as 0) and uses the fix field as its alpha. */
void filterColorDraw(BlurTintParams *tint, XglPacket *packet) {
    u8 green;
    s32 blendMode;
    u8 red;
    u8 blue;
    u8 fix;
    u64 scissorLow;
    u64 scissorHigh;
    XglCamera *camera;
    XglDrawBuffers *drawBuffers;

    red = tint->red;
    green = tint->green;
    blue = tint->blue;
    fix = tint->fix;
    blendMode = tint->blendMode;
    camera = xglStudioSelectGetActiveCamera(0);
    if (xglFilterShouldSkip() != 0) {
        return;
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    drawBuffers = &D_004C8390;
    scissorLow = func_003098D0(camera->screen.window.vector.x);
    scissorLow |= func_003098D0(camera->screen.window.vector.w) << 48;
    scissorHigh = func_003098D0(camera->screen.window.vector.y) << 32;
    scissorHigh |= func_003098D0(camera->screen.window.vector.z) << 16;
    gifStateAddReg(GS_SCISSOR_1, scissorLow | scissorHigh);
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_FBA_1, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(drawBuffers->depthBase, GS_PSMZ24, 1));
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_CLAMP_1, 0);
    gifStateAddReg(GS_ALPHA_1, 0);
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(drawBuffers->workBase, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(drawBuffers->displayBase << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV((int) camera->screen.window.vector.x * 16,
                                    (int) camera->screen.window.vector.y * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0));
    gifStateAddReg(GS_UV, GS_SET_UV((int) camera->screen.window.vector.z * 16,
                                    (int) camera->screen.window.vector.w * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0));
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 0, 1));

    switch (blendMode) {
    case 0:
        gifStateAddReg(GS_ALPHA_1, ((u64) fix << 32) | 0x64);
        break;
    case 1:
        gifStateAddReg(GS_ALPHA_1, ((u64) fix << 32) | 0x68);
        break;
    case 2:
        gifStateAddReg(GS_ALPHA_1, ((u64) fix << 32) | 0x62);
        break;
    default:
        gifStateAddReg(GS_ALPHA_1, ((u64) fix << 32) | 0x64);
        break;
    }

    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, 0x80, XGL_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV((int) camera->screen.window.vector.x * 16,
                                    (int) camera->screen.window.vector.y * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0));
    gifStateAddReg(GS_UV, GS_SET_UV((int) camera->screen.window.vector.z * 16,
                                    (int) camera->screen.window.vector.w * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, WAVE_CELL, 0, BLUR_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

#define DRAW_BUFFERS (&D_004C8390)

/* A wipe transition: step counts the steps taken towards phaseLimit. */
typedef struct TrackNode {
    u8 unmodeled_00;
    u8 state;
    u8 unmodeled_02[2];
    u32 phaseLimit;
    s32 drawMode;
    s32 wipeKind;
    s32 restartRequested;
    u8 unmodeled_14[44];
    s32 step;
    u8 unmodeled_44[12];
    u32 flags;
} TrackNode;

static inline void drawAreaSetWindowRect(s32 blendFix, s32 blendMode, s32 sourceX, s32 sourceY, s32 sourceW,
                                     s32 sourceH, s32 destinationX, s32 destinationY, s32 destinationW,
                                     s32 destinationH) {
    DrawEnvRecord *drawRecord;

    drawRecord = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();
    drawRecord->blendFix = blendFix;
    drawRecord->blendMode = blendMode;
    drawRecord->sourceRect[0] = sourceX;
    drawRecord->sourceRect[1] = sourceY;
    drawRecord->sourceRect[2] = sourceW;
    drawRecord->sourceRect[3] = sourceH;
    drawRecord->destinationRect[0] = destinationX;
    drawRecord->destinationRect[1] = destinationY;
    drawRecord->destinationRect[2] = destinationW;
    drawRecord->destinationRect[3] = destinationH;
}

static inline void drawAreaQueueFrame(s32 workBase, s32 frameBase, s32 blockWidth, s32 blockHeight, s32 unk44,
                                      s32 unk48, s32 blendFix, s32 blendMode, s32 sourceX, s32 sourceY,
                                      s32 sourceW, s32 sourceH, s32 destinationX, s32 destinationY,
                                      s32 destinationW, s32 destinationH) {
    DrawEnvRecord *drawRecord;

    drawRecord = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();
    drawRecord->workBase = workBase;
    drawRecord->blockWidth = blockWidth;
    drawRecord->unk44 = unk44;
    drawRecord->frameBase = frameBase;
    drawRecord->blockHeight = blockHeight;
    drawRecord->unk48 = unk48;
    drawRecord->blendFix = blendFix;
    drawRecord->blendMode = blendMode;
    drawRecord->sourceRect[0] = sourceX;
    drawRecord->sourceRect[1] = sourceY;
    drawRecord->sourceRect[2] = sourceW;
    drawRecord->sourceRect[3] = sourceH;
    drawRecord->destinationRect[0] = destinationX;
    drawRecord->destinationRect[1] = destinationY;
    drawRecord->destinationRect[2] = destinationW;
    drawRecord->destinationRect[3] = destinationH;
}

/* Screen filter 31, WIPE in the game's filter name list.
 * One frame of a wipe transition over the display page. State 0 sets the draw mode and moves to state 1; state 1
 * draws the wipe rectangle of wipeKind for the current step, then advances the step. A pending restart resets the
 * node to state 0. */
void filterWipeDraw(TrackNode *node, s32 flag) {
    s32 *wipeStep;
    s32 *restartRequest;
    u32 drawMode;
    u32 wipeKind;
    f32 progress;
    s32 reachedEnd;

    wipeStep = &node->step;
    restartRequest = &node->restartRequested;
    drawMode = node->drawMode;
    wipeKind = node->wipeKind;
    progress = (f32) *wipeStep / (f32) node->phaseLimit;
    reachedEnd = *wipeStep == (s32) node->phaseLimit;
    func_001D5CD0(DRAW_BUFFERS->displayBase, 0, 0, 512, 448);

    switch (node->state) {
    case 0:
        switch (drawMode) {
        case 0:
        default:
            drawEnvSetLarge();
            break;
        case 1:
            drawEnvSetHalfDraw();
            break;
        case 2:
            drawAreaQueueFrame(DRAW_BUFFERS->displayBase, DRAW_BUFFERS->unk10, 8, 8, 0, drawMode, 128, 3, 0, 0, 512,
                               448, 0, 0, 512, 224);
            ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)()->flags.all |= 0x8000ULL << 17;
            ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)()->flags.all |= 0x8000ULL << 18;
            drawEnvSendRecordPacket();
            break;
        }
        node->state = 1;
        /* fall through */
    case 1:
        if (reachedEnd) {
            if (*restartRequest == 0) {
                node->flags |= 2;
            }
            break;
        }
        (*wipeStep)++;
        if (xglFilterShouldSkip()) {
            break;
        }
        switch (wipeKind) {
        case 0:
            func_001D5E60(DRAW_BUFFERS->displayBase, 0, 0, (s32) (progress * 512.0f), 448);
            break;
        case 1:
            func_001D5E60(DRAW_BUFFERS->displayBase, (s32) ((1.0f - progress) * 512.0f), 0, 512, 448);
            break;
        case 2:
            func_001D5E60(DRAW_BUFFERS->displayBase, 0, 0, 512, (s32) (progress * 448.0f));
            break;
        case 3:
            func_001D5E60(DRAW_BUFFERS->displayBase, 0, (s32) ((1.0f - progress) * 448.0f), 512, 448);
            break;
        case 4:
            func_001D5E60(DRAW_BUFFERS->displayBase, 0, 0, (s32) (progress * 512.0f), (s32) (progress * 448.0f));
            break;
        case 5:
            func_001D5E60(DRAW_BUFFERS->displayBase, (s32) ((1.0f - progress) * 512.0f), 0, 512,
                          (s32) (progress * 448.0f));
            break;
        case 6:
            func_001D5E60(DRAW_BUFFERS->displayBase, 0, (s32) ((1.0f - progress) * 448.0f), (s32) (progress * 512.0f),
                          448);
            break;
        case 7:
            func_001D5E60(DRAW_BUFFERS->displayBase, (s32) ((1.0f - progress) * 512.0f),
                          (s32) ((1.0f - progress) * 448.0f), 512, 448);
            break;
        case 8:
            func_001D5E60(DRAW_BUFFERS->displayBase, 256 - (s32) (progress * 256.0f), 224 - (s32) (progress * 224.0f),
                          (s32) (progress * 256.0f) + 256, (s32) (progress * 224.0f) + 224);
            break;
        case 9:
            func_001D5E60(DRAW_BUFFERS->displayBase, 256 - (s32) (progress * 256.0f), 0,
                          (s32) (progress * 256.0f) + 256, 448);
            break;
        case 10:
            func_001D5E60(DRAW_BUFFERS->displayBase, 0, 224 - (s32) (progress * 224.0f), 512,
                          (s32) (progress * 224.0f) + 224);
            break;
        default:
            node->flags |= 2;
            return;
        }

        gDrawArea.unk58 = 0x34000ULL;
        switch (drawMode) {
        case 0:
        default:
            drawAreaSetWindowRect(128, 3, 0, 0, 512, 448, 0, 0, 512, 448);
            func_001D55E8();
            break;
        case 1:
            drawAreaSetWindowRect(128, 3, 0, 0, 256, 224, 0, 0, 512, 448);
            func_001D56D8();
            break;
        case 2:
            drawAreaQueueFrame(DRAW_BUFFERS->unk10, DRAW_BUFFERS->displayBase, 8, 8, drawMode, 0, 128, 3, 0, 0, 512,
                               224, 0, 0, 512, 448);
            ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)()->flags.all |= 0x8000ULL << 25;
            ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)()->flags.all |= 0x8000ULL << 26;
            drawEnvSendRecordPacket();
            break;
        }
        gDrawArea.unk58 = 0x30000ULL;
        break;
    }

    if (*restartRequest != 0) {
        node->state = 0;
        *wipeStep = 0;
        *restartRequest = 0;
        filterWipeDraw(node, flag);
    }
}


typedef struct ScreenWipe {
    u8 unmodeled_00[4];
    u8 red; /* +0x04 */
    u8 unmodeled_05[3];
    u8 green; /* +0x08 */
    u8 unmodeled_09[3];
    u8 blue; /* +0x0C */
    u8 unmodeled_0d[3];
    u8 alpha; /* +0x10 */
    u8 unmodeled_11[3];
    unsigned int kind; /* +0x14: ScreenWipeKind */
    int position;      /* +0x18: progress in pixels, or the centre's x */
    int centerY;       /* +0x1C */
    int width;         /* +0x20 */
    int height;        /* +0x24 */
} ScreenWipe;

typedef enum ScreenWipeKind {
    WIPE_FROM_BOTTOM,
    WIPE_FROM_TOP,
    WIPE_FROM_RIGHT,
    WIPE_FROM_LEFT,
    WIPE_CORNER_BOTTOM_LEFT,
    WIPE_CORNER_TOP_LEFT,
    WIPE_CORNER_BOTTOM_RIGHT,
    WIPE_CORNER_TOP_RIGHT,
    WIPE_ELLIPSE,
    WIPE_RECTANGLE,
    WIPE_KIND_COUNT
} ScreenWipeKind;

#define WIPE_PROGRESS_MAX 0x200
#define WIPE_ELLIPSE_STEPS 41
#define WIPE_SIDE_STEPS 10
#define WIPE_PRIM_GOURAUD_STRIP 0x4C

static inline void wipePacketBegin(void) {
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(GIF_TAG_EOP, 0x10000000, GIF_AD, 0);
}

static inline void wipePacketSend(void) {
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

#define WIPE_VERTEX(alpha, x, y) \
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, alpha, XGL_FLOAT_ONE_BITS)); \
    xglPacketAppendQuad(GS_XYZF2, (long long) (((x) << 4) + 0x6FF8) | ((long long) (((y) << 4) + 0x71F7) << 16))

/* Screen filter 32, GRADATION in the game's filter name list.
 * Draws one screen wipe (edge, corner, ellipse or rectangle) over the studio camera's screen. */
void filterGradationDraw(ScreenWipe *wipe) {
    int red;
    int green;
    int blue;
    int alpha;
    unsigned int kind;
    int position;
    int centerY;
    unsigned int halfWidth;
    unsigned int halfHeight;
    XglCamera *camera;
    float angle;
    float sine;
    float cosine;
    int boxLeft;
    int boxTop;
    int boxRight;
    int boxBottom;
    int screenLeft;
    int screenTop;
    int screenRight;
    int screenBottom;

    red = wipe->red;
    green = wipe->green;
    blue = wipe->blue;
    alpha = wipe->alpha;
    kind = wipe->kind;
    position = wipe->position;
    centerY = wipe->centerY;
    halfWidth = wipe->width >> 1;
    halfHeight = wipe->height >> 1;
    camera = xglStudioSelectGetActiveCamera(0);
    screenLeft = camera->screen.window.component[0];
    screenTop = camera->screen.window.component[1];
    screenRight = camera->screen.window.component[2];
    screenBottom = camera->screen.window.component[3];
    if (xglFilterShouldSkip()) {
        return;
    }
    if (alpha == 0) {
        return;
    }
    if (kind >= WIPE_KIND_COUNT) {
        return;
    }
    if ((unsigned int) position > WIPE_PROGRESS_MAX) {
        return;
    }

    wipePacketBegin();
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
    xglPacketAppendQuad(
        GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    xglPacketAppendQuad(
        GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(
        GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0));
    wipePacketSend();

    switch (kind) {
    case WIPE_FROM_BOTTOM:
    case WIPE_FROM_TOP:
    case WIPE_FROM_RIGHT:
    case WIPE_FROM_LEFT: {
        int left;
        int top;
        int right;
        int bottom;
        int alphaTopLeft;
        int alphaBottomLeft;
        int alphaTopRight;
        int alphaBottomRight;

        switch (kind) {
        case WIPE_FROM_BOTTOM:
            left = screenLeft;
            top = screenTop + position;
            right = screenRight;
            bottom = screenBottom;
            alphaTopLeft = alphaTopRight = 0;
            alphaBottomLeft = alphaBottomRight = alpha;
            break;
        case WIPE_FROM_TOP:
            left = screenLeft;
            top = screenTop;
            right = screenRight;
            bottom = screenBottom - position;
            alphaTopLeft = alphaTopRight = alpha;
            alphaBottomLeft = alphaBottomRight = 0;
            break;
        case WIPE_FROM_RIGHT:
            left = screenLeft + position;
            top = screenTop;
            right = screenRight;
            bottom = screenBottom;
            alphaTopLeft = alphaBottomLeft = 0;
            alphaTopRight = alphaBottomRight = alpha;
            break;
        case WIPE_FROM_LEFT:
            left = screenLeft;
            top = screenTop;
            right = screenRight - position;
            bottom = screenBottom;
            alphaTopLeft = alphaBottomLeft = alpha;
            alphaTopRight = alphaBottomRight = 0;
            break;
        default:
            left = top = right = bottom = alphaTopLeft = alphaBottomLeft = alphaTopRight = alphaBottomRight =
                0;
            break;
        }
        wipePacketBegin();
        xglPacketAppendQuad(
            GS_PRIM, WIPE_PRIM_GOURAUD_STRIP);
        WIPE_VERTEX(alphaTopLeft, left, top);
        WIPE_VERTEX(alphaTopRight, right, top);
        WIPE_VERTEX(alphaBottomLeft, left, bottom);
        WIPE_VERTEX(alphaBottomRight, right, bottom);
        wipePacketSend();
        break;
    }
    case WIPE_CORNER_BOTTOM_LEFT:
    case WIPE_CORNER_TOP_LEFT:
    case WIPE_CORNER_BOTTOM_RIGHT:
    case WIPE_CORNER_TOP_RIGHT: {
        int cornerX;
        int cornerY;
        int secondX;
        int secondY;
        int thirdX;
        int thirdY;
        int cornerAlpha;
        int farAlpha;
        int sideWidth;
        int sideHeight;

        sideWidth = (position - 0x100) << 1;
        sideHeight = (float) sideWidth * 0.875f;
        cornerAlpha = alpha;
        farAlpha = 0;
        switch (kind) {
        case WIPE_CORNER_BOTTOM_LEFT:
            cornerX = screenLeft;
            cornerY = screenBottom;
            secondX = screenLeft;
            secondY = screenTop + sideHeight;
            thirdX = screenRight - sideWidth;
            thirdY = screenBottom;
            break;
        case WIPE_CORNER_TOP_LEFT:
            cornerX = screenLeft;
            cornerY = screenTop;
            secondX = screenRight - sideWidth;
            secondY = screenTop;
            thirdX = screenLeft;
            thirdY = screenBottom - sideHeight;
            break;
        case WIPE_CORNER_BOTTOM_RIGHT:
            cornerX = screenRight;
            cornerY = screenBottom;
            secondX = screenLeft + sideWidth;
            secondY = screenBottom;
            thirdX = screenRight;
            thirdY = screenTop + sideHeight;
            break;
        case WIPE_CORNER_TOP_RIGHT:
            cornerX = screenRight;
            cornerY = screenTop;
            secondX = screenRight;
            secondY = screenBottom - sideHeight;
            thirdX = screenLeft + sideWidth;
            thirdY = screenTop;
            break;
        default:
            cornerX = cornerY = secondX = secondY = thirdX = thirdY = cornerAlpha = 0;
            break;
        }
        wipePacketBegin();
        xglPacketAppendQuad(
            GS_PRIM, WIPE_PRIM_GOURAUD_STRIP);
        WIPE_VERTEX(cornerAlpha, cornerX, cornerY);
        WIPE_VERTEX(farAlpha, secondX, secondY);
        WIPE_VERTEX(farAlpha, thirdX, thirdY);
        wipePacketSend();
        break;
    }
    case WIPE_ELLIPSE: {
        int ellipseX;
        int ellipseY;
        int side;
        int step;
        int sideStep;
        int edgeX;
        int edgeY;
        int screenWidth;
        int screenHeight;

        wipePacketBegin();
        xglPacketAppendQuad(
            GS_PRIM, WIPE_PRIM_GOURAUD_STRIP);
        sideStep = WIPE_SIDE_STEPS / 2;
        side = 0;
        for (step = 0; step < WIPE_ELLIPSE_STEPS; step++) {
            screenWidth = screenRight - screenLeft;
            screenHeight = screenBottom - screenTop;
            angle = (float) step * 0.1570796f;
            sine = xglSin(angle);
            cosine = xglCos(angle);
            ellipseX = (int) ((float) halfWidth * cosine) + position;
            ellipseY = (int) ((float) halfHeight * -sine) + centerY;
            if (sideStep >= WIPE_SIDE_STEPS) {
                sideStep = 0;
                side++;
            }
            /* The outer vertex walks the screen's edge: right side up, top, left side down, bottom, right side again. */
            switch (side) {
            case 0:
            case 4:
                edgeX = screenRight;
                edgeY = (float) screenBottom - (float) screenHeight * ((float) sideStep / 10.0f);
                break;
            case 1:
                edgeX = (float) screenRight - (float) screenWidth * ((float) sideStep / 10.0f);
                edgeY = screenTop;
                break;
            case 2:
                edgeX = screenLeft;
                edgeY = (float) screenTop + (float) screenHeight * ((float) sideStep / 10.0f);
                break;
            case 3:
                edgeX = (float) screenLeft + (float) screenWidth * ((float) sideStep / 10.0f);
                edgeY = screenBottom;
                break;
            default:
                edgeX = edgeY = 0;
                break;
            }
            sideStep++;
            WIPE_VERTEX(0, ellipseX, ellipseY);
            WIPE_VERTEX(alpha, edgeX, edgeY);
        }
        wipePacketSend();
        break;
    }
    case WIPE_RECTANGLE:
        boxLeft = position - halfWidth;
        boxTop = centerY - halfHeight;
        boxRight = position + halfWidth;
        boxBottom = centerY + halfHeight;
        wipePacketBegin();
        xglPacketAppendQuad(
            GS_PRIM, WIPE_PRIM_GOURAUD_STRIP);
        WIPE_VERTEX(alpha, screenLeft, screenTop);
        WIPE_VERTEX(0, boxLeft, boxTop);
        WIPE_VERTEX(alpha, screenRight, screenTop);
        WIPE_VERTEX(0, boxRight, boxTop);
        WIPE_VERTEX(alpha, screenRight, screenBottom);
        WIPE_VERTEX(0, boxRight, boxBottom);
        WIPE_VERTEX(alpha, screenLeft, screenBottom);
        WIPE_VERTEX(0, boxLeft, boxBottom);
        WIPE_VERTEX(alpha, screenLeft, screenTop);
        WIPE_VERTEX(0, boxLeft, boxTop);
        wipePacketSend();
        break;
    }
}

typedef struct FadeColor {
    u8 unmodeled_00[4];
    int red;   /* +0x04 */
    int green; /* +0x08 */
    int blue;  /* +0x0C */
    int alpha; /* +0x10 */
    int unmodeled_14[16];
} FadeColor;

/* Screen filter 36, PLANE in the game's filter name list.
 * Draws one full-screen sprite of colour (red, green, blue, alpha) for the fade. */
void filterPlaneDraw(FadeColor *color, int packet) {
    u8 red = (u8) color->red;
    u8 green = (u8) color->green;
    u8 blue = (u8) color->blue;
    u8 alpha = (u8) color->alpha;
    if (xglFilterShouldSkip()) {
        return;
    }
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_TEST_ZTE_ALWAYS);
    xglPacketAppendQuad(
        GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, GS_PSMZ24, 1));
    xglPacketAppendQuad(
        GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(
        GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, alpha));
    xglPacketAppendQuad(
        GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, 0, XGL_FLOAT_ONE_BITS));
    xglPacketAppendQuad(GS_PRIM, GS_PRIM_SPRITE_ABE);
    xglPacketAppendQuad(GS_XYZF2, 0x71F76FF8);
    xglPacketAppendQuad(GS_XYZF2, 0x8DF78FF8);
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

typedef struct FadeClock {
    int frame; /* +0x00 */
    int unmodeled_04[3];
    union {
        unsigned int word; /* +0x10 */
        struct {
            unsigned int state : 4;
            unsigned int paused : 1;
            unsigned int unmodeled_bits : 27;
        } bits;
    } flags;
} FadeClock;

typedef struct FadeRecord {
    int unmodeled_00;
    unsigned int length; /* +0x04 */
    int red;             /* +0x08 */
    int green;           /* +0x0C */
    int blue;            /* +0x10 */
    int alpha;           /* +0x14 */
    int mode;            /* +0x18 */
    int unmodeled_1C[9];
    FadeClock clock; /* +0x40 */
} FadeRecord;

void xglRenderGlobalFadeValueSet(unsigned int color);

/* Screen filter 35, GLOBAL FADE (also 39 PREV FADE and 40 POST FADE) in the game's filter name list.
 * Advances a fade: modes 0 and 1 set the global fade colour; modes 2 to 5 draw a full-screen sprite. */
void filterFadeDraw(FadeRecord *fade, int packet) {
    FadeClock *clock = &fade->clock;
    int frame = clock->frame;
    unsigned int length = fade->length;
    int paused = fade->clock.flags.bits.paused;
    int finished = frame == length;
    int mode = fade->mode;
    int red = (u8) fade->red;
    int green = (u8) fade->green;
    int blue = (u8) fade->blue;
    int alpha = (u8) fade->alpha;
    FadeColor sprite;
    unsigned int color;

    switch (mode) {
    case 0:
    case 1: {
        unsigned int alphaRed;
        unsigned int greenBlue;

        if (mode == 1) {
            alpha = alpha * ((float) frame / length);
        } else {
            alpha = alpha * (1.0f - (float) frame / length);
        }
        alphaRed = (alpha << 24) | red;
        greenBlue = (green << 8) | (blue << 16);
        color = alphaRed | greenBlue;
        if (finished) {
            xglRenderGlobalFadeValueSet(color);
            fade->clock.flags.word |= 2;
        } else {
            xglRenderGlobalFadeValueSet(color);
            if (!paused) {
                clock->frame++;
            }
        }
        break;
    }
    case 2:
    case 3:
    case 4:
    case 5:
        if (mode == 3 || mode == 5) {
            alpha = alpha * ((float) frame / length);
        } else {
            alpha = alpha * (1.0f - (float) frame / length);
        }
        sprite.red = red;
        sprite.green = green;
        sprite.blue = blue;
        sprite.alpha = alpha;
        filterPlaneDraw(&sprite, packet);
        if (finished) {
            if (mode == 2 || mode == 4) {
                fade->clock.flags.word |= 2;
            }
        } else {
            if (!paused) {
                clock->frame++;
            }
        }
        break;
    }
}

typedef struct ScreenEffectArgs {
    u8 unmodeled_00[4];
    int pageOffset;  /* +0x04 */
    int blendSource; /* +0x08 */
    int wholeFrame;  /* +0x0C */
} ScreenEffectArgs;

typedef struct DrawOffset {
    s32 x;
    s32 y;
} DrawOffset;

#define DRAW_MODE_DEFAULT 0x30000
#define DRAW_MODE_OFFSET 0x50000

#define SCREEN_HEIGHT 448
#define HALF_WIDTH 256
#define HALF_HEIGHT 224
#define BLUR_COPIES 4
#define BLUR_STEP 3

static inline void drawEnvPush(s32 blendFix, s32 blendMode, s32 sourceX, s32 sourceY, s32 sourceW,
                               s32 sourceH, s32 destX, s32 destY, s32 destW, s32 destH) {
    DrawEnvRecord *record = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();

    record->blendFix = blendFix;
    record->blendMode = blendMode;
    record->sourceRect[0] = sourceX;
    record->sourceRect[1] = sourceY;
    record->sourceRect[2] = sourceW;
    record->sourceRect[3] = sourceH;
    record->destinationRect[0] = destX;
    record->destinationRect[1] = destY;
    record->destinationRect[2] = destW;
    record->destinationRect[3] = destH;
}

static inline void drawEnvShrink(void) {
    drawEnvPush(128, 3, 0, 0, WAVE_CELL, SCREEN_HEIGHT, 0, 0, HALF_WIDTH, HALF_HEIGHT);
    func_001D54C8();
}

/* Screen filter 37, DOF2 in the game's filter name list.
 * Blurs the large page: four offset copies blended on top, then one full-frame blend. */
void filterDof2Draw(ScreenEffectArgs *effect) {
    s32 pageOffset = effect->pageOffset;
    s32 blurAlpha = effect->blendSource >> 2;
    s32 wholeFrame = effect->wholeFrame;
    s32 i;
    s32 offsetX;
    s32 offsetY;

    if (xglFilterShouldSkip()) {
        return;
    }
    xglPacketSendFixedHeader();
    drawEnvSetLarge();
    if (wholeFrame) {
        func_001D5CD0(D_004C8390.displayBase, 0, 0, WAVE_CELL, SCREEN_HEIGHT);
        drawRectOnPageDepth(D_004C8390.displayBase, pageOffset, 0, 0, WAVE_CELL,
                      SCREEN_HEIGHT);
    }
    drawEnvShrink();
    {
        DrawOffset offsets[BLUR_COPIES] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

        for (i = 0; i < BLUR_COPIES; i++) {
            offsetX = offsets[i].x * BLUR_STEP;
            offsetY = offsets[i].y * BLUR_STEP;
            drawEnvPush(blurAlpha, 0, 0, 0, HALF_WIDTH, HALF_HEIGHT, offsetX, offsetY, offsetX + WAVE_CELL, offsetY + SCREEN_HEIGHT);
            func_001D57E0();
            drawEnvShrink();
        }
    }
    drawEnvSetCameraScissor();
    if (wholeFrame) {
        gDrawArea.unk58 = DRAW_MODE_DEFAULT;
    } else {
        gDrawArea.unk58 = DRAW_MODE_OFFSET;
        gDrawArea.flags.pageOffset = pageOffset;
    }
    drawEnvPush(128, 3, 0, 0, WAVE_CELL, SCREEN_HEIGHT, 0, 0, WAVE_CELL, SCREEN_HEIGHT);
    func_001D55E8();
    gDrawArea.unk58 = DRAW_MODE_DEFAULT;
}

typedef signed char s8;

#define DRAW_ENV_FLAG(bit) ((u64)1 << (bit))

typedef struct DissolveCell {
    s8 speed; /* 0: no bubble */
    s8 alpha;
} DissolveCell;

#define DISSOLVE_ROWS 36
#define DISSOLVE_COLUMNS 19
#define DISSOLVE_ROW_STEP 31
#define DISSOLVE_COLUMN_STEP 27

#define DISSOLVE_TOP (-304)
#define DISSOLVE_RADIUS 17.0f
#define DISSOLVE_CORNERS 7
#define DISSOLVE_CORNER_ANGLE 1.0471973f /* 60 degrees */
#define DISSOLVE_FADE_FRAMES 5

static DissolveCell gXglDissolveCells[DISSOLVE_ROWS * DISSOLVE_COLUMNS];

typedef enum DissolvePhase {
    DISSOLVE_START = 1,
    DISSOLVE_CAPTURE,
    DISSOLVE_RUN,
    DISSOLVE_DONE
} DissolvePhase;

typedef struct DissolveEffect {
    u8 unmodeled_00;
    u8 phase; /* +0x01: DissolvePhase */
    u8 unmodeled_02[0x3E];
    f32 row;         /* +0x40: the row the edge has reached */
    f32 previousRow; /* +0x44 */
    s32 fadeCounter; /* +0x48 */
    s32 isCapturing; /* +0x4C */
    union {
        u32 word;
        struct {
            u32 unmodeled0 : 1;
            u32 isFinished : 1;
            u32 unmodeled2 : 2;
            u32 isPaused : 1;
        } bits;
    } flags; /* +0x50 */
} DissolveEffect;

#define DISSOLVE_FLAG_FINISHED 2

s32 func_0030DD10(void);

static inline void drawEnvCopyPage(s32 workBase, s32 workWidth, s32 workFormat, s32 frameBase, s32 frameWidth,
                                   s32 frameFormat, s32 blendMode, s32 sourceLeft, s32 sourceTop,
                                   s32 sourceRight, s32 sourceBottom, s32 left, s32 top, s32 right,
                                   s32 bottom, s32 blendFix) {
    DrawEnvRecord *record = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();

    record->workBase = workBase;
    record->blockWidth = workWidth;
    record->unk44 = workFormat;
    record->frameBase = frameBase;
    record->blockHeight = frameWidth;
    record->unk48 = frameFormat;
    record->blendMode = blendMode;
    record->blendFix = blendFix;
    record->sourceRect[0] = sourceLeft;
    record->sourceRect[1] = sourceTop;
    record->sourceRect[2] = sourceRight;
    record->sourceRect[3] = sourceBottom;
    record->destinationRect[0] = left;
    record->destinationRect[1] = top;
    record->destinationRect[2] = right;
    record->destinationRect[3] = bottom;
}

static inline void drawEnvCopyPageTinted(s32 workBase, s32 workWidth, s32 workFormat, s32 frameBase,
                                         s32 frameWidth, s32 frameFormat, s32 blendMode, s32 sourceLeft,
                                         s32 sourceTop, s32 sourceRight, s32 sourceBottom, s32 left, s32 top,
                                         s32 right, s32 bottom, s32 red, s32 green, s32 blue, s32 blendFix) {
    DrawEnvRecord *record = ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)();

    record->workBase = workBase;
    record->blockWidth = workWidth;
    record->unk44 = workFormat;
    record->frameBase = frameBase;
    record->blockHeight = frameWidth;
    record->unk48 = frameFormat;
    record->blendMode = blendMode;
    record->red = red;
    record->green = green;
    record->blue = blue;
    record->blendFix = blendFix;
    record->sourceRect[0] = sourceLeft;
    record->sourceRect[1] = sourceTop;
    record->sourceRect[2] = sourceRight;
    record->sourceRect[3] = sourceBottom;
    record->destinationRect[0] = left;
    record->destinationRect[1] = top;
    record->destinationRect[2] = right;
    record->destinationRect[3] = bottom;
    record->flags.all |= DRAW_ENV_FLAG(48);
}

static inline void drawEnvSetFlag(s32 bit) {
    ((DrawEnvRecord * (*) (void) ) drawEnvGetRecord)()->flags.all |= DRAW_ENV_FLAG(bit);
}

static inline void dissolvePacketBegin(void) {
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
}

static inline void dissolvePacketSend(void) {
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

#define DISSOLVE_GREY(alpha) GS_SET_RGBAQ(0x80, 0x80, 0x80, alpha, XGL_FLOAT_ONE_BITS)
#define DISSOLVE_X(x) (((x) << 4) + 0x6FF8)
#define DISSOLVE_Y(y) (((y) << 4) + 0x71F7)
#define DISSOLVE_XYZ(x, y) GS_SET_XYZ(DISSOLVE_X(x), DISSOLVE_Y(y), 0)
#define DISSOLVE_EDGE_VERTEX(alpha, x, y) \
    xglPacketAppendQuad(1, DISSOLVE_GREY(alpha)); \
    xglPacketAppendQuad(4, DISSOLVE_XYZ(x, y));
#define DISSOLVE_MAP_POINT \
    if (pointX < 0) { \
        texU = (-pointX) << 4; \
    } else if (pointX > WAVE_CELL) { \
        texU = (WAVE_CELL - pointX % WAVE_CELL) << 4; \
    } else { \
        texU = pointX << 4; \
    } \
    vertexX = pointX << 4; \
    if (pointY < 0) { \
        texV = (s32)((f32)((-pointY) << 4) * 0.5f); \
        vertexY = pointY << 4; \
    } else if (pointY > SCREEN_HEIGHT) { \
        texV = (s32)((f32)((SCREEN_HEIGHT - pointY % SCREEN_HEIGHT) << 4) * 0.5f); \
        vertexY = pointY << 4; \
    } else { \
        vertexY = pointY << 4; \
        texV = (s32)((f32)vertexY * 0.5f); \
    }

/* Screen filter 41, CUT IN in the game's filter name list.
 * Advances the dissolve: captures the frame, sweeps the edge row down and draws the bubble cells. */
void filterCutInDraw(DissolveEffect *effect) {
    s32 tintRed = 0x80;
    s32 tintGreen = 0xC8;
    s32 tintBlue = 0xC2;
    s32 isPaused;
    f32 *rowPos;
    f32 *previousRowPos;
    s32 *fadeFrames;
    s32 *capturing;
    s32 cellX;
    s32 j;
    s32 k;
    s32 cellAlpha;
    s32 cellY;
    s32 edge;
    s32 rowCount;
    s32 i;
    s32 shade;
    s32 texU;
    s32 texV;
    s32 vertexX;
    s32 vertexY;
    s32 pointX;
    s32 pointY;
    f32 angle;

    rowPos = &effect->row;
    isPaused = effect->flags.bits.isPaused;
    previousRowPos = &effect->previousRow;
    fadeFrames = &effect->fadeCounter;
    capturing = &effect->isCapturing;

    switch (effect->phase) {
    case DISSOLVE_START:
        *rowPos = 0.0f;
        memset(gXglDissolveCells, 0, sizeof(gXglDissolveCells));
        *fadeFrames = DISSOLVE_FADE_FRAMES;
        *capturing = 1;
        effect->phase = DISSOLVE_CAPTURE;
        drawEnvCopyPage(D_004C8390.displayBase, 8, 0,
                        D_004C8390.unk10, 8, 2, 3, 0, 0, WAVE_CELL,
                        SCREEN_HEIGHT, 0, 0, WAVE_CELL, HALF_HEIGHT, 0x80);
        drawEnvSetFlag(32);
        drawEnvSetFlag(33);
        drawEnvSendRecordPacket();
        break;
    case DISSOLVE_CAPTURE:
        if (!isPaused) {
            if (--*fadeFrames == 0) {
                *capturing = 0;
                effect->phase = DISSOLVE_RUN;
            }
        }
        break;
    case DISSOLVE_RUN:
        break;
    case DISSOLVE_DONE:
        effect->flags.word |= DISSOLVE_FLAG_FINISHED;
        return;
    }
    if (xglFilterShouldSkip()) {
        return;
    }
    func_001D5E60(D_004C8390.displayBase, 0, 0, WAVE_CELL, SCREEN_HEIGHT);
    if (*capturing) {
        edge = (s32) *previousRowPos * DISSOLVE_ROW_STEP + 16;
    } else {
        edge = (s32) *rowPos * DISSOLVE_ROW_STEP + 16;
    }
    {
        s32 upperLeft = edge - 397;
        s32 upperRight = edge - 304;
        s32 lowerLeft = edge - 93;

        dissolvePacketBegin();
        xglPacketAppendQuad(0x3F, 0);
        xglPacketAppendQuad(0x4A, 0);
        xglPacketAppendQuad(8, 0);
        xglPacketAppendQuad(
            0x4E, GS_SET_ZBUF(D_004C8390.depthBase, 0x31, 1));
        xglPacketAppendQuad(0x47, 0x30000);
        xglPacketAppendQuad(
            6,
            GS_SET_TEX0(D_004C8390.unk10 * 32, 8, 2, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(0x14, 0x60);
        xglPacketAppendQuad(
            0x4C, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFFFFFF));
        xglPacketAppendQuad(
            0x42, GS_SET_ALPHA(0, 2, 0, 2, 0x80));
        xglPacketAppendQuad(0, 0x14C);
        DISSOLVE_EDGE_VERTEX(0, upperLeft, 0)
        DISSOLVE_EDGE_VERTEX(0x80, upperRight, 0)
        DISSOLVE_EDGE_VERTEX(0, lowerLeft, 513)
        DISSOLVE_EDGE_VERTEX(0x80, edge, 513)
        if (lowerLeft > 0) {
            xglPacketAppendQuad(0, 0x14C);
            xglPacketAppendQuad(
                1, DISSOLVE_GREY(0));
            xglPacketAppendQuad(
                4, DISSOLVE_XYZ(upperLeft, 0));
            xglPacketAppendQuad(
                4, DISSOLVE_XYZ(0, SCREEN_HEIGHT));
            xglPacketAppendQuad(
                4, DISSOLVE_XYZ(lowerLeft, 513));
        }
        if (upperLeft > 0) {
            xglPacketAppendQuad(0, 0x14C);
            xglPacketAppendQuad(
                1, DISSOLVE_GREY(0));
            xglPacketAppendQuad(
                4, DISSOLVE_XYZ(0, 0));
            xglPacketAppendQuad(
                4, DISSOLVE_XYZ(upperLeft, 0));
            xglPacketAppendQuad(
                4, DISSOLVE_XYZ(0, 513));
        }
        dissolvePacketSend();
    }
    shade = 128 - (DISSOLVE_FADE_FRAMES - *fadeFrames) * 10;
    drawEnvCopyPageTinted(D_004C8390.unk10, 8, 2,
                          D_004C8390.displayBase, 8, 0, 4, 0, 0, WAVE_CELL,
                          HALF_HEIGHT, 0, 0, WAVE_CELL, SCREEN_HEIGHT, shade, shade, shade, 0);
    drawEnvSetFlag(40);
    drawEnvSetFlag(41);
    drawEnvSendRecordPacket();

    if (*capturing == 0 && !isPaused) {
        rowCount = (s32) *rowPos - (s32) *previousRowPos;
        for (j = 0; j < DISSOLVE_COLUMNS; j++) {
            s32 step;

            gXglDissolveCells[(s32) *rowPos * DISSOLVE_COLUMNS + j].alpha = 127;
            gXglDissolveCells[(s32) *rowPos * DISSOLVE_COLUMNS + j].speed = func_0030DD10() % 56 + 32;
            for (step = 1; step < rowCount; step++) {
                gXglDissolveCells[((s32) *previousRowPos + step) * DISSOLVE_COLUMNS + j].alpha = 95;
                gXglDissolveCells[((s32) *previousRowPos + step) * DISSOLVE_COLUMNS + j].speed =
                    func_0030DD10() % 56 + 32;
            }
        }
    }
    for (j = 0; j < DISSOLVE_ROWS; j++) {
        cellX = j * DISSOLVE_ROW_STEP + DISSOLVE_TOP;
        dissolvePacketBegin();
        for (k = 0; k < DISSOLVE_COLUMNS; k++) {
            cellX += WAVE_COLUMNS;
            cellY = k * DISSOLVE_COLUMN_STEP;
            cellAlpha = gXglDissolveCells[j * DISSOLVE_COLUMNS + k].alpha;
            if (gXglDissolveCells[j * DISSOLVE_COLUMNS + k].speed != 0) {
                if (*capturing != 0 || isPaused) {
                    cellAlpha += gXglDissolveCells[j * DISSOLVE_COLUMNS + k].speed;
                }
                if (cellAlpha <= 0) {
                    gXglDissolveCells[j * DISSOLVE_COLUMNS + k].alpha -= gXglDissolveCells[j * DISSOLVE_COLUMNS + k].speed;
                    gXglDissolveCells[j * DISSOLVE_COLUMNS + k].speed = 0;
                } else {
                    pointX = cellX;
                    pointY = cellY;
                    DISSOLVE_MAP_POINT(
                        (void (*)(long long highDoubleword, long long lowDoubleword)) xglPacketAppendQuad)(
                        0x42, GS_SET_ALPHA(0, 1, 2, 1, cellAlpha));
                    xglPacketAppendQuad(
                        1, DISSOLVE_GREY(0x80));
                    xglPacketAppendQuad(
                        0, 0x15D);
                    xglPacketAppendQuad(
                        3, GS_SET_UV(texU, texV));
                    xglPacketAppendQuad(
                        4, GS_SET_XYZ(vertexX + 0x6FF8, vertexY + 0x71F7, 0xFFFFFF));
                    for (i = 0; i < DISSOLVE_CORNERS; i++) {
                        angle = (f32) i;
                        angle *= DISSOLVE_CORNER_ANGLE;
                        pointX = (s32) (xglSin(angle) * DISSOLVE_RADIUS);
                        pointY = (s32) (xglCos(angle) * DISSOLVE_RADIUS);
                        pointX += cellX;
                        pointY += cellY;
                        DISSOLVE_MAP_POINT((void (*)(long long highDoubleword, long long lowDoubleword))
                                               xglPacketAppendQuad)(3, GS_SET_UV(texU, texV));
                        xglPacketAppendQuad(
                            4, GS_SET_XYZ(vertexX + 0x6FF8, vertexY + 0x71F7, 0xFFFFFF));
                    }
                    pointX = cellX;
                    pointY = cellY;
                    xglPacketAppendQuad(
                        0x42, GS_SET_ALPHA(0, 1, 2, 1, cellAlpha / 2));
                    xglPacketAppendQuad(
                        1, GS_SET_RGBAQ(tintRed, tintGreen, tintBlue, 0x80, XGL_FLOAT_ONE_BITS));
                    xglPacketAppendQuad(
                        0, 0x14D);
                    xglPacketAppendQuad(
                        4, GS_SET_XYZ(DISSOLVE_X(pointX), DISSOLVE_Y(pointY), 0xFFFFFF));
                    for (i = 0; i < DISSOLVE_CORNERS; i++) {
                        angle = (f32) i;
                        angle *= DISSOLVE_CORNER_ANGLE;
                        pointX = (s32) (xglSin(angle) * DISSOLVE_RADIUS);
                        pointY = (s32) (xglCos(angle) * DISSOLVE_RADIUS);
                        pointX += cellX;
                        pointY += cellY;
                        xglPacketAppendQuad(
                            4, GS_SET_XYZ(DISSOLVE_X(pointX), DISSOLVE_Y(pointY), 0xFFFFFF));
                    }
                    if (*capturing == 0 && !isPaused) {
                        gXglDissolveCells[j * DISSOLVE_COLUMNS + k].alpha -=
                            gXglDissolveCells[j * DISSOLVE_COLUMNS + k].speed;
                    }
                }
            }
        }
        dissolvePacketSend();
    }
    if (*capturing == 0 && !isPaused) {
        *previousRowPos = *rowPos;
        *rowPos += 4.0f;
    }
    if (*rowPos >= 36.0f) {
        effect->phase = DISSOLVE_DONE;
    }
}

/* Screen filter 42, TEST in the game's filter name list. */
void filterTestDraw(void) {
}

/* Screen filter 43, TEST2 in the game's filter name list. */
void filterTest2Draw(void) {
}

extern PadRecord PadData[];

void func_001E8108(s32 enableFlag);
void scratchpadSendCmd0A(s32 sendParams);

/* The packet words are copied to the scratchpad. The empty do/while is a scheduling barrier between the copies and
   the sends: without it the emitted order changes (kept: it decides the emitted order). */
#define SCRATCHPAD_ORDER_BARRIER() do { } while (0)

/* Screen filter 44, TEST3 in the game's filter name list.
 * Copies the 14 words of sendParams to the scratchpad, then sends the stored command 0x0A pairs by pad button:
   held 0x10 calls func_001E8108; pressed 4 sends the pair (30, 2), pressed 1 the pair (30, 3). */
void filterTest3Draw(u32 *sendParams) {
    u32 packetWord1;
    u32 packetWord2;
    u32 packetWord3;
    u32 packetWord4;
    u32 packetWord5;
    u32 packetWord6;
    u32 packetWord7;
    u32 packetWord8;
    u32 packetWord9;
    u32 packetWord10;
    u32 packetWord11;
    u32 packetWord12;
    u32 packetWord13;
    u32 packetWord14;
    u16 held;
    u16 pressed;
    packetWord1 = sendParams[1];
    packetWord2 = sendParams[2];
    packetWord3 = sendParams[3];
    packetWord4 = sendParams[4];
    packetWord5 = sendParams[5];
    packetWord6 = sendParams[6];
    packetWord7 = sendParams[7];
    packetWord8 = sendParams[8];
    packetWord9 = sendParams[9];
    packetWord10 = sendParams[10];
    packetWord11 = sendParams[11];
    packetWord12 = sendParams[12];
    packetWord13 = sendParams[13];
    packetWord14 = sendParams[14];
    SCRATCHPAD_ORDER_BARRIER();
    SCRATCHPAD_WORD(0) = packetWord1;
    SCRATCHPAD_WORD(1) = packetWord2;
    SCRATCHPAD_WORD(2) = packetWord3;
    SCRATCHPAD_WORD(3) = packetWord4;
    SCRATCHPAD_WORD(4) = packetWord5;
    SCRATCHPAD_WORD(5) = packetWord6;
    SCRATCHPAD_WORD(6) = packetWord7;
    SCRATCHPAD_WORD(7) = packetWord8;
    SCRATCHPAD_WORD(8) = packetWord9;
    SCRATCHPAD_WORD(9) = packetWord10;
    SCRATCHPAD_WORD(10) = packetWord11;
    SCRATCHPAD_WORD(11) = packetWord12;
    SCRATCHPAD_WORD(12) = packetWord13;
    SCRATCHPAD_WORD(13) = packetWord14;

    held = PadData[0].held;
    func_001E8108(held & 0x10);
    pressed = PadData[0].pressed;
    if (pressed & 4) {
        s32 firstCommand[2] = { 30, 2 };

        scratchpadSendCmd0A((s32) firstCommand);
    }
    pressed = PadData[0].pressed;
    if (pressed & 1) {
        s32 secondCommand[2] = { 30, 3 };

        scratchpadSendCmd0A((s32) secondCommand);
    }
}

void scratchpadClearSendCmd29(void);
s32 slotStateInactive(void);

/* Screen filter 45, TEST4 in the game's filter name list.
 * Copies the 9 words of sendParams to the scratchpad; pressed 0x80 clears the cmd29 state and held 0x10 makes the
   slot inactive. */
void filterTest4Draw(u32 *sendParams) {
    u32 packetWord1;
    u32 packetWord2;
    u32 packetWord3;
    u32 packetWord4;
    u32 packetWord5;
    u32 packetWord6;
    u32 packetWord7;
    u32 packetWord8;
    u32 packetWord9;
    packetWord1 = sendParams[1];
    packetWord2 = sendParams[2];
    packetWord3 = sendParams[3];
    packetWord4 = sendParams[4];
    packetWord5 = sendParams[5];
    packetWord6 = sendParams[6];
    packetWord7 = sendParams[7];
    packetWord8 = sendParams[8];
    packetWord9 = sendParams[9];
    SCRATCHPAD_ORDER_BARRIER();
    SCRATCHPAD_WORD(0) = packetWord1;
    SCRATCHPAD_WORD(1) = packetWord2;
    SCRATCHPAD_WORD(2) = packetWord3;
    SCRATCHPAD_WORD(3) = packetWord4;
    SCRATCHPAD_WORD(4) = packetWord5;
    SCRATCHPAD_WORD(5) = packetWord6;
    SCRATCHPAD_WORD(6) = packetWord7;
    SCRATCHPAD_WORD(7) = packetWord8;
    SCRATCHPAD_WORD(8) = packetWord9;

    if (PadData[0].pressed & 0x80) {
        scratchpadClearSendCmd29();
    }

    if (PadData[0].held & 0x10) {
        slotStateInactive();
    }
}

/* Screen filter 46, TEST5 in the game's filter name list.
 * Sets the draw environment to a 256x224 window at the origin with a 256x224 clip and a 512x448 draw size, then
   calls func_001D56D8. */
void filterTest5Draw(void) {
    DrawEnvRecord *drawArea;

    drawArea = drawEnvGetRecord();
    drawArea->sourceRect[1] = drawArea->sourceRect[0] = 0;
    drawArea->sourceRect[2] = 0x100;
    drawArea->sourceRect[3] = 0xE0;
    drawArea->blendFix = 0x80;
    drawArea->blendMode = 3;
    drawArea->destinationRect[0] = 0x100;
    drawArea->destinationRect[1] = 0xE0;
    drawArea->destinationRect[2] = 0x200;
    drawArea->destinationRect[3] = 0x1C0;
    func_001D56D8();
}

/* Screen filter 47, TEST6 in the game's filter name list.
 * Builds one GS packet list at the scratchpad: a direct-mode GIF tag, the state registers (test, z, texture, alpha,
   colour, frame, primitive) and two textured quads that copy the screen page into the copy page and back, then sends
   it. */
void filterTest6Draw(void) {
    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, 0x31, 1));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0, 0x3F800000));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.displayBase << 5, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.unk10, 4, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    xglPacketAppendQuad(GS_UV, 0);
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x1C00));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x81F7, 0));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.unk10 << 5, 4, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.displayBase, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x1000, 0x1000));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x7FF7, 0));
    xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x2000));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);
}

/* Screen filter 48, TEST7 in the game's filter name list.
 * Empty step: no body in the original. */
void filterTest7Draw(void) {
}

void *memset(void *destination, int value, unsigned int blockCount);
void slotSetFixedValue(s32 fixedValue);
void slotSendFrame(s32 packet);

static SlotControlBlock D_0061BEE0[1];
static SlotControlBlock D_0061B018[32];
static SlotControlBlock *D_0061BF38[32];
static SlotControlBlock *D_0061BFB8[8];
static SlotControlBlock D_0061BA98[8];

/* Clears the slot tables and the draw-area record, then installs the two callbacks of D_004C8390. */
void func_001E7268(void) {
    D_00550FFC = 0;
    memset(D_0061B018, 0, SLOT_SIZE * 32);
    memset(D_0061BF38, 0, sizeof(SlotControlBlock *) * 32);
    memset(D_0061BA98, 0, SLOT_SIZE * 8);
    memset(D_0061BFB8, 0, sizeof(SlotControlBlock *) * 8);
    memset(gFixedSlots, 0, SLOT_SIZE * 5);
    memset(D_0061BEE0, 0, SLOT_SIZE);
    memset(&gDrawArea, 0, sizeof(gDrawArea));
    gDrawArea.unk58 = 0x30000;
    gDrawArea.unk60 = 0;
    gDrawArea.unk50 = ((unsigned long long) 0x9880 << 17) | D_004C8390.depthBase;
    D_004C8390.sendFrame = slotSendFrame;
    D_004C8390.setFixedValue = slotSetFixedValue;
}

typedef void (*SlotHandler)(SlotControlBlock *slotControl, s32 fixedValue);

extern SlotHandler gSlotHandlers[];

void func_001E7358(SlotControlBlock *slotControl, s32 fixedValue);

/* Runs the handler of one slot record (hookIndex 1..48 in gSlotHandlers) and updates its control flags. */
void func_001E7358(SlotControlBlock *slotControl, s32 fixedValue) {
    u32 controlFlags;

    if (slotControl == 0) {
        return;
    }
    if (slotControl->hookIndex == 0) {
        return;
    }
    if (slotControl->hookIndex >= 49) {
        return;
    }
    if (gSlotHandlers[slotControl->hookIndex] == 0) {
        return;
    }
    drawEnvSetCameraScissor();
    gSlotHandlers[slotControl->hookIndex](slotControl, fixedValue);
    if (slotControl->unk01 == 0) {
        slotControl->unk01++;
    }
    controlFlags = slotControl->controlFlags & ~1;
    slotControl->controlFlags = controlFlags;
    if ((STUDIO_BLOCK_HEAD.studioFlags & 1) != 0 && STUDIO_BLOCK_HEAD.unmodeled_18 != 0 && slotControl->hookIndex != 27) {
        slotControl->controlFlags = controlFlags | 1;
    }
    slotControl->controlFlags &= ~8;
}

#define SCRATCHPAD_BASE 0x70000000
#define SCRATCHPAD_PARAMS ((s32 *) SCRATCHPAD_BASE)
/* Word 5 of the scratchpad parameter block: the sendMode of the send. */
#define SCRATCHPAD_MODE SCRATCHPAD_PARAMS[5]

/* Release hook called for a record before the record is cleared; gSlotReleaseHooks[hookIndex]. */
typedef void (*SlotHook)(void);

extern SlotHook gSlotReleaseHooks[];
void slotSendCommand(s32 command, s32 sendParams);
void slotCallReleaseHookAndClear(SlotControlBlock *slotControl);
void slotRelease(s32 slotIndex);

void slotCallReleaseHookAndClear(SlotControlBlock *slotControl) {
    SlotHook releaseHook;

    if (slotControl != 0) {
        releaseHook = gSlotReleaseHooks[slotControl->hookIndex];
        if (releaseHook != 0) {
            releaseHook();
        }
        memset(slotControl, 0, SLOT_SIZE);
    }
}

void slotSetFixedValue(s32 fixedValue) {
    u32 controlFlags;

    func_001E7358(&gFixedSlots[4], fixedValue);
    controlFlags = gFixedSlots[4].controlFlags;
    if ((controlFlags >> 1) & 1) {
        slotRelease(4);
    }
}

void slotReleaseChecked(s32 slotIndex, s32 forceRelease);
void slotReleaseKind8Checked(s32 slotIndex);

s32 stateIsReady(void);
extern unsigned long long D_00407FC0[];
extern unsigned long long D_00408020[];

/* Builds the active studio camera window packet and sends the slot records of the frame through VIF1. */
void slotSendFrame(s32 packet) {
    u8 *packetWords;
    const XglCamera *camera;
    unsigned long long packed;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 i;
    s32 j;
    s32 busy;
    s32 notReady;
    SlotControlBlock *slotRecord;

    packetWords = D_00407FC0;
    camera = xglStudioSelectGetActiveCamera(0);
    x0 = camera->screen.window.component[0];
    y0 = camera->screen.window.component[1];
    x1 = camera->screen.window.component[2];
    y1 = camera->screen.window.component[3];
    packed =
        ((unsigned long long) (long long) (x0) | ((unsigned long long) (long long) (x1 - 1) << 16) |
         ((unsigned long long) (long long) (y0) << 32) | ((unsigned long long) (long long) (y1 - 1) << 48));
    {
        unsigned long long *pkt = (unsigned long long *) D_00407FC0;
        pkt[10] = packed;
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, pkt, 24);
    }

    for (i = 0; i < 32; i++) {
        if (D_0061BF38[i] == 0) {
            break;
        }
        if (D_0061BF38[i]->hookIndex == 27 || (D_0061BF38[i]->controlFlags & 1)) {
            func_001E7358(D_0061BF38[i], packet);
        }
    }
    for (i = 0; i < 8; i++) {
        if (D_0061BFB8[i] == 0) {
            break;
        }
        func_001E7358(D_0061BFB8[i], packet);
    }
    for (i = 0; i < 5; i++) {
        if (i != 4) {
            func_001E7358(&gFixedSlots[i], packet);
        }
    }
    func_001E7358(&D_0061BEE0[0], packet);

    for (i = 0; i < 32; i++) {
        if ((D_0061B018[i].controlFlags >> 1) & 1) {
            slotReleaseChecked(D_0061B018[i].slotId, 0);
        }
    }
    for (i = 0; i < 8; i++) {
        if ((D_0061BA98[i].controlFlags >> 1) & 1) {
            slotReleaseKind8Checked(D_0061BA98[i].slotId);
        }
    }
    for (i = 0; i < 5; i++) {
        if (((gFixedSlots[i].controlFlags >> 1) & 1) && i != 4) {
            slotRelease(i);
        }
    }
    if ((D_0061BEE0[0].controlFlags >> 1) & 1) {
        slotReleaseChecked(D_0061BEE0[0].slotId, 0);
    }

    if (xglRenderMovieIsActive() != 0) {
        busy = 0;
        for (j = 0; j < 5; j++) {
            if (gFixedSlots[j].hookIndex != 0) {
                busy = 1;
                break;
            }
        }
        busy |= stateIsReady() == 0;
        if (!busy) {
            xglPacketSendFixedHeader();
            drawEnvSetHalfDraw();
            drawEnvSetCameraScissor();
        }
    }

    {
        unsigned long long *pkt = (unsigned long long *) D_00408020;
        pkt[8] = ((unsigned long long) 0xFF << 56) | ((unsigned long long) 0x8 << 16) |
                 movieFramebufferBasePage[0];
        pkt[10] = ((unsigned long long) 0x1BF << 48) | ((unsigned long long) 0x1FF << 16);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddDataN(packet, pkt, 24);
    }
}

SlotControlBlock *func_001E7890(s32 slotIndex, s32 entryCount, u8 *slotTable);

/* Returns the record in a table of 0x54-byte blocks whose slotId equals slotIndex, or 0. */
SlotControlBlock *func_001E7890(s32 slotIndex, s32 entryCount, u8 *slotTable) {
    SlotControlBlock *tableEntry;
    s32 i;

    if (slotIndex == 0) {
        return 0;
    }
    i = 0;
    if (entryCount > 0) {
        tableEntry = (SlotControlBlock *) slotTable;
        slotTable += 3;
        do {
            if (*slotTable == slotIndex) {
                return tableEntry;
            }
            tableEntry++;
            slotTable += SLOT_SIZE;
            i++;
        } while (i < entryCount);
    }
    return 0;
}

/* Claims a free block of blocks for kind, registers it in table, and returns it (0 when none is free). */
SlotControlBlock *slotClaimRecord(s32 slotKind, s32 blockCount, SlotControlBlock *blocks, SlotControlBlock **slotTable) {
    s32 blockIndex;
    s32 tableIndex;

    if (slotKind == 0) {
        return 0;
    }
    blockIndex = 0;
    if (blockCount > 0) {
        for (; blockIndex < blockCount; blockIndex++) {
            if (blocks[blockIndex].slotId == 0) {
                for (tableIndex = 0; tableIndex < blockCount; tableIndex++) {
                    if (slotTable[tableIndex] == 0) {
                        slotTable[tableIndex] = &blocks[blockIndex];
                        blocks[blockIndex].slotId = slotKind;
                        blocks[blockIndex].allocIndex = blockIndex;
                        return &blocks[blockIndex];
                    }
                }
            }
        }
    }
    return 0;
}

void func_001E7990(s32 slotKind, SlotControlBlock *slotControl, SlotControlBlock **slotTable);

void func_001E7990(s32 slotKind, SlotControlBlock *slotControl, SlotControlBlock **slotTable) {
    SlotControlBlock **tableEntry;
    s32 readIndex;
    s32 skipIndex;
    s32 writeIndex;

    if (slotControl == 0) {
        return;
    }
    tableEntry = slotTable;
    writeIndex = 0;
    if (slotKind > 0 && slotTable[0] != 0) {
        readIndex = (slotTable[0] == slotControl);
        slotTable[0] = slotTable[readIndex];
    loop:
        readIndex++;
        skipIndex = readIndex + 1;
        tableEntry++;
        writeIndex++;
        if (readIndex >= slotKind || slotTable[readIndex] == 0) {
            goto done;
        }
        if (*tableEntry == slotControl) {
            readIndex++;
        }
        *tableEntry = slotTable[readIndex];
        goto loop;
    }
done:
    slotTable[writeIndex] = 0;
}

s32 slotFindFreeId(void) {
    SlotControlBlock *slotTable;
    s32 slotId;
    s32 entryIndex;

    slotTable = D_0061B018;
    for (slotId = 1; slotId < 32; slotId++) {
        for (entryIndex = 0; entryIndex < 32; entryIndex++) {
            if (slotTable[entryIndex].slotId == slotId) {
                goto found;
            }
        }
        return slotId;
    found:;
    }
    return 0;
}

typedef struct SlotSendBlock {
    s32 words[15];
} SlotSendBlock;

/* The 0x54-byte slot block viewed as a send record: sendBlock at +0x04 is kept apart from SlotControlBlock.issuedCount
   because the field types decide the emitted order. */
typedef struct SlotSendRecord {
    u8 hookIndex; /* +0x00: index of the release hook in gSlotReleaseHooks */
    u8 unk01;
    u8 allocIndex;           /* +0x02 */
    u8 slotId;               /* +0x03: 0 when the record is free */
    SlotSendBlock sendBlock; /* +0x04: 60-byte send block */
    s32 doneCount;
    u8 unmodeled_44[0x50 - 0x44];
    u32 controlFlags; /* +0x50 */
} SlotSendRecord;

typedef struct SlotGlobalFlags {
    u32 enabled : 1;
    u32 rest : 31;
    s32 unmodeled_04[7];
} SlotGlobalFlags;

extern SlotGlobalFlags gSlotDelayEnabled;

/* Bitfield view of a slot record (controlFlags at +0x50); the same layout as SlotSendRecord up to +0x40. */
typedef struct SlotControlBits {
    u8 hookIndex;
    u8 unk01;
    u8 allocIndex;
    u8 slotId;
    SlotSendBlock sendBlock;
    s32 unmodeled_40[4];
    u32 active : 1;
    u32 sendMode : 1;
    u32 bit2 : 1;
    u32 delayed : 1;
    u32 bit4 : 1;
    u32 bit5 : 1;
    u32 rest : 26;
} SlotControlBits;

s32 slotSetupRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode);

/* Stores a send for subId in the D_0061B018 record of id (claiming one when absent); returns the slot id. */
s32 slotSetupRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode) {
    SlotControlBits *slotControl = (SlotControlBits *) func_001E7890(id, 32, (u8 *) D_0061B018);

    if (subId == 0) {
        return 0;
    }
    if (slotControl == 0) {
        if (id == 0) {
            id = slotFindFreeId();
        }
        slotControl = (SlotControlBits *) slotClaimRecord(id, 32, D_0061B018, D_0061BF38);
        if (slotControl == 0) {
            return 0;
        }
    }
    if (sendParams == 0) {
        slotReleaseChecked(id, 0);
    }
    slotControl->hookIndex = subId;
    slotControl->sendMode = sendMode;
    slotControl->active = 1;
    slotControl->delayed = gSlotDelayEnabled.enabled;
    if (subId == 41) {
        slotControl->bit5 = 1;
    }
    slotControl->sendBlock = *sendParams;
    return id;
}

s32 slotSetupMode0(s32 slotIndex, s32 setupValue, s32 valueCount) {
    return slotSetupRecord(slotIndex, setupValue, valueCount, 0);
}

void slotSetupMode1(s32 slotIndex, s32 setupValue, s32 valueCount) {
    slotSetupRecord(slotIndex, setupValue, valueCount, 1);
}

s32 func_001E7C50(s32 id, s32 hook, SlotSendBlock *sendParams, s32 sendMode);

/* Finds the record of slotRecord id (or claims a free one when id is 0 or new), stores hook, sendParams and sendMode,
   then runs its update. Returns the slotRecord id, or 0 when no record is free. */
s32 func_001E7C50(s32 id, s32 hook, SlotSendBlock *sendParams, s32 sendMode) {
    SlotControlBits *record;
    SlotControlBits *freeRecord;
    s32 recordIndex;
    s32 i;
    s32 packet;

    packet = xglPacketGetCurrent();
    if (hook) {
        record = 0;
        freeRecord = 0;
        recordIndex = 0;
        if (id == 0) {
            id = slotFindFreeId();
        }
        for (i = 0; i < 32; i++) {
            record = (SlotControlBits *) &D_0061B018[i];
            if (record->slotId == id) {
                recordIndex = i;
                break;
            }
            if (record->slotId == 0) {
                freeRecord = record;
                recordIndex = i;
            }
        }
        if (i == 32) {
            record = freeRecord;
        }
        if (record == 0) {
            return 0;
        }
        record->hookIndex = hook;
        record->allocIndex = recordIndex;
        record->slotId = id;
        record->bit2 = sendMode;
        record->active = 1;
        if (sendParams) {
            record->sendBlock = *sendParams;
        } else {
            memset(&record->sendBlock, 0, sizeof(SlotSendBlock));
        }
        func_001E7358((SlotControlBlock *) record, packet);
    }
    return id;
}

void slotUpdateMode0(s32 slotIndex, s32 updateValue, s32 valueCount) {
    func_001E7C50(slotIndex, updateValue, valueCount, 0);
}

void slotUpdateMode1(s32 slotIndex, s32 updateValue, s32 valueCount) {
    func_001E7C50(slotIndex, updateValue, valueCount, 1);
}

void slotReleaseChecked(s32 slotIndex, s32 forceRelease)
{
    SlotControlBlock **slotControlPtr;
    SlotControlBlock *slotControl;

    slotControlPtr = &slotControl;
    slotControl = func_001E7890(slotIndex, 0x20, D_0061B018);
    if (*slotControlPtr != 0 && (forceRelease != 0 || ((*slotControlPtr)->controlFlags & 0x22) != 0x20)) {
        func_001E7990(0x20, *slotControlPtr, D_0061BF38);
        slotCallReleaseHookAndClear(slotControl);
    }
}

void func_001E8550(void);

/* Releases the D_0061BF38 records unless one is pinned (flag 5 set, flag 1 clear), clears both pools, then runs func_001E8550. */
void slotReleaseAllPools(void) {
    s32 i;
    SlotControlBlock *slotControl;
    u32 controlFlags;

    for (i = 0; i < 32; i++) {
        slotControl = D_0061BF38[i];
        if (slotControl != 0) {
            controlFlags = slotControl->controlFlags;
            if (((controlFlags >> 1) & 1) == 0 && ((controlFlags >> 5) & 1) != 0) {
                return;
            }
            slotCallReleaseHookAndClear(slotControl);
        }
    }
    memset(D_0061B018, 0, SLOT_SIZE * 32);
    memset(D_0061BF38, 0, sizeof(SlotControlBlock *) * 32);
    func_001E8550();
}

void slotSendPoolRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode);

/* Stores a send for subId in the D_0061BA98 pool record of id (claiming one when absent). */
void slotSendPoolRecord(s32 id, s32 subId, SlotSendBlock *sendParams, s32 sendMode) {
    SlotSendRecord *slotControl;
    u32 controlFlags;

    slotControl = (SlotSendRecord *) func_001E7890(id, 8, (u8 *) D_0061BA98);
    if (subId == 0) {
        return;
    }
    if (slotControl == 0) {
        slotControl = (SlotSendRecord *) slotClaimRecord(id, 32, D_0061BA98, D_0061BFB8);
        if (slotControl == 0) {
            return;
        }
    }
    controlFlags = slotControl->controlFlags;
    slotControl->hookIndex = subId;
    controlFlags &= ~2;
    controlFlags |= (sendMode & 1) << 1;
    controlFlags |= 1;
    slotControl->controlFlags = controlFlags;
    if (sendParams != 0) {
        slotControl->sendBlock = *sendParams;
    } else {
        memset(&slotControl->sendBlock, 0, 60);
    }
}

void scratchpadSendMode0(s32 id, s32 subId, s32 sendParams) {
    slotSendPoolRecord(id, subId, sendParams, 0);
}

void scratchpadSendMode1(s32 id, s32 subId, s32 sendParams) {
    slotSendPoolRecord(id, subId, sendParams, 1);
}

s32 slotRecord1CountDiff(void);

/* Sets or clears bit 4 of the controlFlags of gFixedSlots[1] when that slot runs hook 10 with pending counts. */
void func_001E8108(s32 enableFlag) {
    u32 bit;

    if (gFixedSlots[1].hookIndex == 10 && slotRecord1CountDiff()) {
        bit = (u32) enableFlag > 0;
        gFixedSlots[1].controlFlags = (gFixedSlots[1].controlFlags & ~0x10) | (bit << 4);
    }
}

void scratchpadSendCmd0A(s32 sendParams) {
    slotSendCommand(0xA, sendParams);
}

void scratchpadSendCmd1F(s32 sendParams) {
    slotSendCommand(0x1F, sendParams);
}

void scratchpadLoadParams(s32 *sendParams) {
    SCRATCHPAD_PARAMS[0] = sendParams[0];
    SCRATCHPAD_PARAMS[1] = sendParams[1];
    SCRATCHPAD_PARAMS[2] = sendParams[2];
    SCRATCHPAD_PARAMS[3] = sendParams[3];
    SCRATCHPAD_PARAMS[4] = sendParams[4];
}

void scratchpadSendCmd23Mode0(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 0;
    slotSendCommand(0x23, SCRATCHPAD_BASE);
}

void scratchpadSendCmd23Mode1(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 1;
    slotSendCommand(0x23, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode2(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 2;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode3(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 3;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

void scratchpadSendCmd27Mode4(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 4;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

/* Sends the parameter block as command 0x27 in mode 5. */
void func_001E82E8(s32 *sendParams) {
    scratchpadLoadParams(sendParams);
    SCRATCHPAD_MODE = 5;
    slotSendCommand(0x27, SCRATCHPAD_BASE);
}

typedef struct SlotSend2 {
    s32 w[2];
} SlotSend2;

typedef struct SlotSend3 {
    s32 w[3];
} SlotSend3;

typedef struct SlotSend6 {
    s32 w[6];
} SlotSend6;


typedef struct SlotSendView {
    u8 hookIndex;
    u8 busy;
    u8 unmodeled_02[2];
    union {
        SlotSend6 six;
        struct {
            SlotSend3 three;
            s32 wasBusy;
        } withThree;
        struct {
            SlotSend2 two;
            s32 wasBusy;
        } withTwo;
        s32 words[15];
    } send;
    s32 doneCount;
    s32 unmodeled_44[4];
} SlotSendView;


/* Copies a send payload into the slot view that matches command: 0x27/0x28 the fixed slots, 0x1F/0x0A D_0061BD8C, 0x23 D_0061BDE0, 0x29 marks D_0061BE34 busy. */
void slotSendCommand(s32 command, s32 sendParams) {
    SlotSendView *slotRecord;

    if (sendParams == 0) {
        return;
    }
    switch (command) {
    case 0x27:
        slotRecord = (SlotSendView *) &gFixedSlots[0];
        goto tail;
    case 0x28:
        slotRecord = (SlotSendView *) &gFixedSlots[4];
        goto tail;
    case 0x1F:
        slotRecord = (SlotSendView *) &gFixedSlots[1];
        slotRecord->hookIndex = command;
        slotRecord->send.withThree.three = *(SlotSend3 *) sendParams;
        slotRecord->send.withThree.wasBusy = slotRecord->busy != 0;
        break;
    case 0x0A:
        slotRecord = (SlotSendView *) &gFixedSlots[1];
        slotRecord->hookIndex = command;
        slotRecord->send.withTwo.two = *(SlotSend2 *) sendParams;
        slotRecord->send.withTwo.wasBusy = slotRecord->busy != 0;
        break;
    case 0x23:
        slotRecord = (SlotSendView *) &gFixedSlots[2];
    tail:
        slotRecord->hookIndex = command;
        slotRecord->doneCount = 0;
        slotRecord->send.six = *(SlotSend6 *) sendParams;
        break;
    case 0x29:
        slotRecord = (SlotSendView *) &gFixedSlots[3];
        slotRecord->hookIndex = command;
        slotRecord->busy = 1;
        break;
    default:
        break;
    }
}

void slotReleaseSlot1(void) {
    slotRelease(1);
}

void slotRelease(s32 slotIndex) {
    SlotControlBlock *slotControl;

    slotControl = &gFixedSlots[slotIndex];
    if (slotControl != 0) {
        slotCallReleaseHookAndClear(slotControl);
    }
}

/* Releases the five fixed slots gFixedSlots[0..4]. */
void slotReleaseFixedSlots(void) {
    s32 slotIndex;

    for (slotIndex = 0; slotIndex < 5; slotIndex++) {
        slotRelease(slotIndex);
    }
}

void slotReleaseKind8Checked(s32 slotIndex)
{
    u32 controlFlags;
    SlotControlBlock **slotControlPtr;
    SlotControlBlock *slotControl;

    slotControlPtr = &slotControl;
    slotControl = func_001E7890(slotIndex, 8, D_0061BA98);
    if (*slotControlPtr != 0) {
        controlFlags = (*slotControlPtr)->controlFlags;
        if ((((controlFlags >> 5) & 1) == 0) || ((controlFlags >> 1) & 1)) {
            func_001E7990(8, *slotControlPtr, D_0061BFB8);
            slotCallReleaseHookAndClear(slotControl);
        }
    }
}

/* Releases the D_0061BFB8 records unless one is pinned (flag 5 set, flag 1 clear), then clears the D_0061BA98 pool. */
void func_001E8550(void) {
    s32 i;
    SlotControlBlock *slotControl;
    u32 controlFlags;

    for (i = 0; i < 8; i++) {
        slotControl = D_0061BFB8[i];
        if (slotControl != 0) {
            controlFlags = slotControl->controlFlags;
            if (((controlFlags >> 5) & 1) != 0 && ((controlFlags >> 1) & 1) == 0) {
                return;
            }
            slotCallReleaseHookAndClear(slotControl);
        }
    }
    memset(D_0061BA98, 0, SLOT_SIZE * 8);
    memset(D_0061BFB8, 0, sizeof(SlotControlBlock *) * 8);
}

typedef union RecordWord {
    s32 word;
    u8 byte;
} RecordWord;

typedef struct RecordWords5 {
    RecordWord words[5];
} RecordWords5;

typedef struct RecordWords6 {
    RecordWords5 head;
    s32 extra;
} RecordWords6;

typedef struct FixedRecord {
    u8 slotKind;            /* +0x00 */
    RecordWords6 block; /* +0x04 */
    s32 tail;           /* +0x1C */
    u8 unmodeled_20[0x50 - 0x20];
    s32 flags; /* +0x50 */
} FixedRecord;

/* Typed view of the fixed slotRecord record D_0061BEE0 (the slotRecord-control block of the fixed record). */
#define gFixedRecord (*(FixedRecord *) D_0061BEE0)

void slotFixedRecordSetHead(const RecordWords5 *src) {
    u8 byte;

    gFixedRecord.slotKind = 29;
    gFixedRecord.flags |= 3;
    gFixedRecord.block.head = *src;
    gFixedRecord.tail = 0;
    byte = gFixedRecord.block.head.words[2].byte;
    gFixedRecord.block.extra = 0;
    gFixedRecord.block.head.words[2].word = (s32) (byte * 1.412f);
}

typedef struct SlotPayload {
    s32 words[6];
} SlotPayload;

/* Second view of the fixed record, kept apart from FixedRecord: the field types decide the emitted order
   (six plain words here, a union of bytes and words in FixedRecord.block). */
typedef struct FixedRecordPayload {
    u8 slotKind;             /* +0x00 */
    SlotPayload payload; /* +0x04 */
    s32 tail;            /* +0x1C */
    u8 unmodeled_20[0x50 - 0x20];
    s32 flags; /* +0x50 */
} FixedRecordPayload;

#define gFixedRecordPayload (*(FixedRecordPayload *) D_0061BEE0)

/* Stores a 6-word payload into the fixed record and marks it with slotKind 29 and flags 3. */
void slotFixedRecordSetPayload(const SlotPayload *src) {
    gFixedRecordPayload.slotKind = 29;
    gFixedRecordPayload.flags |= 3;
    gFixedRecordPayload.payload = *src;
    gFixedRecordPayload.tail = 0;
}

s32 slotRecord1CountDiff(void)
{
    return gFixedSlots[1].issuedCount - gFixedSlots[1].doneCount;
}

s32 slotRecord2CountDiff(void)
{
    return gFixedSlots[2].issuedCount - gFixedSlots[2].doneCount;
}

void scratchpadClearSendCmd29(void) {
    memset(SCRATCHPAD_PARAMS, 0, 0x38);

    slotSendCommand(0x29, SCRATCHPAD_BASE);
}

s32 slotStateActive(void) {
    u32 stateMinusOne;

    stateMinusOne = (((SlotSendView *) &gFixedSlots[3])->busy + 0xFF) & 0xFF;
    return stateMinusOne < 2U;
}

s32 slotStateInactive(void) {
    u32 stateMinusOne;

    stateMinusOne = (((SlotSendView *) &gFixedSlots[3])->busy + 0xFF) & 0xFF;
    return stateMinusOne >= 2U;
}

void slotSetEnableBit(s32 slotIndex, s32 isEnabled) {
    s32 controlFlags;
    s32 flagBit;
    SlotControlBlock *slotControl;

    flagBit = isEnabled;
    slotControl = func_001E7890(slotIndex, 0x20, D_0061B018);
    flagBit = flagBit != 0;
    flagBit *= 0x10;
    if (slotControl != 0) {
        controlFlags = (slotControl->controlFlags & ~0x10) | flagBit;
        slotControl->controlFlags = controlFlags;
    }
}

/* Sets or clears bit 4 of the controlFlags of every pool record and fixed slot from isEnabled. */
void slotSetAllEnableBits(s32 isEnabled) {
    SlotControlBlock *slotRecord;
    s32 busy;
    s32 flagBits;
    s32 clearMask;
    s32 keepMask;
    s32 i;

    busy = isEnabled != 0;
    flagBits = busy << 4;
    clearMask = ~0x10;
    slotRecord = D_0061B018;
    for (i = 31; i >= 0; i--, slotRecord++) {
        if (slotRecord->slotId != 0) {
            slotRecord->controlFlags = (slotRecord->controlFlags & clearMask) | flagBits;
        }
    }
    slotRecord = gFixedSlots;
    keepMask = ~0x10;
    for (i = 4; i >= 0; i--, slotRecord++) {
        slotRecord->controlFlags = (slotRecord->controlFlags & keepMask) | ((u8) busy << 4);
    }
}

void slotHookNoop(void) {

}

/* Initialized data of the TU (.data and .sdata), in address order. */
s64 D_00407DB0[14] = {
    0, 0x5100000600000000LL, 0x5000000000008001LL, 0xEEEEELL, 0, 20, 0x70000LL, 71,
    0, 6, 0, 66, 0, 8,
};

s32 gXglWarpGifTag[4] = {
    32768, 548323328, 83, 0,
};

SlotHandler gSlotHandlers[50] = {
    0, 0, 0, 0,
    filterBlurDraw, filterMosaicDraw, filterNegaposiDraw, 0,
    0, filterMonotoneDraw, filterCrossFadeDraw, filterSwell2Draw,
    filterWavyBlurDraw, filterTorusDraw, filterDirBlurDraw, 0,
    0, filterRippleDraw, filterRipple2Draw, filterDistortDraw,
    filterSwellDraw, filterDofDraw, filterNoiseDraw, filterNoise2Draw,
    filterSpeculerDraw, filterShimmerDraw, filterRefractDraw, xglMistEmitterDraw,
    func_001E0E10, filterCorrectDraw, filterSoftFocusDraw, filterWipeDraw,
    filterGradationDraw, filterScreenBlurDraw, filterColorDraw, filterFadeDraw,
    filterPlaneDraw, filterDof2Draw, filterWavyBlur2Draw, filterFadeDraw,
    filterFadeDraw, filterCutInDraw, filterTestDraw, filterTest2Draw,
    filterTest3Draw, filterTest4Draw, filterTest5Draw, filterTest6Draw,
    filterTest7Draw, 0,
};

SlotHook gSlotReleaseHooks[50] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, xglEffectSlotRelease, 0, xglEffectSlotRelease,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0,
};

unsigned long long D_00407FC0[12] = {
    0x0000000000000000ULL, 0x5100000500000000ULL, 0x4000000000008001ULL, 0x000000000000EEEEULL,
    0x0000000000000000ULL, 0x000000000000003FULL, 0x0000000131000000ULL, 0x000000000000004EULL,
    0x000006FC007FC005ULL, 0x0000000000000008ULL, 0x0000000000000000ULL, 0x0000000000000040ULL,
};

unsigned long long D_00408020[12] = {
    0x0000000000000000ULL, 0x5100000500000000ULL, 0x4000000000008001ULL, 0x000000000000EEEEULL,
    0x0000000000000000ULL, 0x000000000000003FULL, 0x0000000031000000ULL, 0x000000000000004EULL,
    0x0000000000000000ULL, 0x000000000000004CULL, 0x0000000000000000ULL, 0x0000000000000040ULL,
};

s32 gPacketBufferBase = 0;
void *gPacketCursor = 0;
s32 *gPacketLastTag = 0;
s32 gPacketEntryCount = 0;
s32 gTagQuadCount = 0;
s32 gXglWaveScrollX = 0;
s32 gXglWaveScrollY = 0;

const char D_0053E838[40] = "\x1b" "[31m\x1b" "[47m\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "[%d]=%d \x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x00" "\x1b" "[0m\n";

static u8 D_00620060[32752];
