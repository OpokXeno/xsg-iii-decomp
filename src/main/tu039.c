#include "common.h"
#include "gs.h"
#include "main/xgl_render.h"

/* One A+D entry of the direct GS queue: the register value and its address (the queue head reuses the words as
 * the GIF tag). */
typedef union XglDirectQueueEntry {
    struct {
        u64 registerValue;
        u64 registerAddress;
    };
    u32 tagAndHeaderWords[4];
} XglDirectQueueEntry;

extern XglDirectQueueEntry *xglDirectQueueBuffer;
extern int xglDirectQueueCount;
extern XglRenderStateBlock D_004C8390;

void nmlPacketGsInit(void);
void nmlPacketAddGsFlush(void);
void nmlPacketAddGsFlushWide(void);

/* GS primitive coordinates are 12.4 fixed point around the 2048 origin; Q = 1.0f as raw bits. */
#define GS_ORIGIN 0x800
#define GS_Q_ONE 0x3F800000
/* XYZ2 vertex at (x, y) in pixels. */
#define XY(x, y) GS_SET_XYZ((x) << 4, (y) << 4, 0)
/* Half the frame size. */
#define SCREEN_HALF_WIDTH ((s16) (D_004C8390.frame_width / 2))
#define SCREEN_HALF_HEIGHT ((s16) (D_004C8390.frame_height / 2))
/* FRAME buffer width of the frame (64-pixel units). */
#define SCREEN_FBW ((s16) (D_004C8390.frame_width / 64))

INCLUDE_ASM("asm/main/nonmatchings/tu039", func_0019D6F0);

INCLUDE_ASM("asm/main/nonmatchings/tu039", func_0019D990);

/* X2 pxyFillFrameAlpha80: sets the frame alpha to 0x80 over the whole screen, then restores the frame mask and the
 * depth test. */
void func_0019DBF8(u32 frameMask, u32 frameBase)
{
    int queueIndex;
    u64 *cursor;
    u64 *start;

    nmlPacketGsInit();
    queueIndex = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueIndex];
    start = cursor;
    GS_PUT(cursor, GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.zbuf_base, 1, 1));
    GS_PUT(cursor, GS_FRAME_1, GS_SET_FRAME(frameBase, SCREEN_FBW, D_004C8390.psm, 0xFFFFFF));
    GS_PUT(cursor, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    GS_PUT(cursor, GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, GS_Q_ONE));
    GS_PUT(cursor, GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    GS_PUT(cursor, GS_XYZF2, XY(GS_ORIGIN - SCREEN_HALF_WIDTH, GS_ORIGIN - SCREEN_HALF_HEIGHT));
    GS_PUT(cursor, GS_XYZF2, XY(GS_ORIGIN + SCREEN_HALF_WIDTH, GS_ORIGIN + SCREEN_HALF_HEIGHT));
    GS_PUT(cursor, GS_FRAME_1, GS_SET_FRAME(frameBase, SCREEN_FBW, D_004C8390.psm, frameMask));
    GS_PUT(cursor, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
    xglDirectQueueCount = queueIndex + (((u32) cursor - (u32) start) >> 4);
    nmlPacketAddGsFlushWide();
}

/* X2 pxyModulateFrameByAlpha: multiplies the frame colour by its alpha (a blended full-screen sprite). */
void func_0019DDB0(int unusedFrameBase, float unusedScale)
{
    int queueIndex;
    u64 *p;
    u64 *q;

    nmlPacketGsInit();
    queueIndex = xglDirectQueueCount;
    p = (u64 *) &xglDirectQueueBuffer[queueIndex];
    q = p;
    GS_PUT(p, GS_ALPHA_1, GS_SET_ALPHA(1, 2, 1, 2, 0));
    GS_PUT(p, GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.zbuf_base, 1, 1));
    GS_PUT(p, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    GS_PUT(p, GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, GS_Q_ONE));
    GS_PUT(p, GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 1, 0, 0, 0, 0));
    GS_PUT(p, GS_XYZF2, XY(GS_ORIGIN - SCREEN_HALF_WIDTH, GS_ORIGIN - SCREEN_HALF_HEIGHT));
    GS_PUT(p, GS_XYZF2, XY(GS_ORIGIN + SCREEN_HALF_WIDTH, GS_ORIGIN + SCREEN_HALF_HEIGHT));
    GS_PUT(p, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
    xglDirectQueueCount = queueIndex + (((u32) p - (u32) q) >> 4);
    nmlPacketAddGsFlushWide();
}

INCLUDE_ASM("asm/main/nonmatchings/tu039", func_0019DF00);
