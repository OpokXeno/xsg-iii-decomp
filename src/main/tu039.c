#include "common.h"
#include "gs.h"
#include "main/xgl_render.h"
#include "main/xgl_2.h"

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

/* A line colour, one int per channel; func_0019D6F0 clamps a copy of it to 0..255. */
typedef struct PxyLineColor {
    int red;
    int green;
    int blue;
    int alpha;
} PxyLineColor;

extern int pxyCameraEnabled[4];
XglCamera *xglStudioSelectGetActiveCamera(int index);
void _CurMatrixSet(Matrix4 *matrix);
void _CurSetViewScaleTrans(Vector4 *scale, Vector4 *translation);
int _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);
void xglDirectQueueDefer(int sendFlag, int key);

/* The studio cameras a line is drawn for. */
#define PXY_LINE_CAMERAS 4
/* The line's points must be quadword aligned and in main RAM (0x100000 .. 0x1FAFFFF). */
#define PXY_LINE_RAM_START 0x100000
#define PXY_LINE_RAM_LAST 0x1EAFFFF /* bytes from PXY_LINE_RAM_START */
#define PXY_LINE_COLOR_MAX 0xFF
/* PRIM: line strip, flat, untextured. */
#define PXY_PRIM_LINE_STRIP 2
/* The sort key the queued line is deferred with. */
#define PXY_LINE_QUEUE_KEY 10

/* Draws a line between two world-space points for every enabled studio camera: both points are
 * projected with the camera's view, and when neither is clipped the line (depth tested, in the
 * clamped colour) goes into the direct GS queue for that camera. Points that are not quadword
 * aligned or not in main RAM draw nothing. */
void func_0019D6F0(Vector4 *start, Vector4 *end, const PxyLineColor *lineColor)
{
    PxyLineColor color;
    ScreenVector4 screenStart;
    ScreenVector4 screenEnd;
    XglCamera *camera;
    int queueIndex;
    u64 *cursor;
    u64 *queueStart;
    int i;

    color = *lineColor;
    if (((u32) start & 0xF) != 0 || (u32) start - PXY_LINE_RAM_START > PXY_LINE_RAM_LAST || ((u32) end & 0xF) != 0) {
        return;
    }
    color.red = color.red < 0 ? 0 : color.red;
    color.green = color.green < 0 ? 0 : color.green;
    color.blue = color.blue < 0 ? 0 : color.blue;
    color.alpha = color.alpha < 0 ? 0 : color.alpha;
    if (color.red > PXY_LINE_COLOR_MAX) {
        color.red = PXY_LINE_COLOR_MAX;
    }
    if (color.green > PXY_LINE_COLOR_MAX) {
        color.green = PXY_LINE_COLOR_MAX;
    }
    if (color.blue > PXY_LINE_COLOR_MAX) {
        color.blue = PXY_LINE_COLOR_MAX;
    }
    if (color.alpha > PXY_LINE_COLOR_MAX) {
        color.alpha = PXY_LINE_COLOR_MAX;
    }
    for (i = 0; i < PXY_LINE_CAMERAS; i++) {
        camera = xglStudioSelectGetActiveCamera(i);
        if (pxyCameraEnabled[i] != 0 && camera != 0) {
            _CurMatrixSet(&camera->volume[0].viewMatrix);
            _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
            if (_CurRotTransPersClipScreen(&screenStart, start) == 0
                && _CurRotTransPersClipScreen(&screenEnd, end) == 0) {
                nmlPacketGsInit();
                queueIndex = xglDirectQueueCount;
                cursor = (u64 *) &xglDirectQueueBuffer[queueIndex];
                queueStart = cursor;
                GS_PUT(cursor, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
                GS_PUT(cursor, GS_RGBAQ, GS_SET_RGBAQ(color.red, color.green, color.blue, color.alpha, GS_Q_ONE));
                GS_PUT(cursor, GS_PRIM, PXY_PRIM_LINE_STRIP);
                GS_PUT(cursor, GS_XYZF2, GS_SET_XYZ(screenStart.x, screenStart.y, screenStart.z));
                GS_PUT(cursor, GS_XYZF2, GS_SET_XYZ(screenEnd.x, screenEnd.y, screenEnd.z));
                xglDirectQueueCount = queueIndex + (((u32) cursor - (u32) queueStart) >> 4);
                xglDirectQueueDefer(i, PXY_LINE_QUEUE_KEY);
            }
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu039", func_0019D990);

/* Sets the frame alpha to 0x80 over the whole screen, then restores the frame mask and the
 * depth test. */
void pxyFillFrameAlpha80(u32 frameMask, u32 frameBase)
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

/* Multiplies the frame colour by its alpha (a blended full-screen sprite). */
void pxyModulateFrameByAlpha(int unusedFrameBase, float unusedScale)
{
    int queueIndex;
    u64 *cursor;
    u64 *start;

    nmlPacketGsInit();
    queueIndex = xglDirectQueueCount;
    cursor = (u64 *) &xglDirectQueueBuffer[queueIndex];
    start = cursor;
    GS_PUT(cursor, GS_ALPHA_1, GS_SET_ALPHA(1, 2, 1, 2, 0));
    GS_PUT(cursor, GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.zbuf_base, 1, 1));
    GS_PUT(cursor, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    GS_PUT(cursor, GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, GS_Q_ONE));
    GS_PUT(cursor, GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 1, 0, 0, 0, 0));
    GS_PUT(cursor, GS_XYZF2, XY(GS_ORIGIN - SCREEN_HALF_WIDTH, GS_ORIGIN - SCREEN_HALF_HEIGHT));
    GS_PUT(cursor, GS_XYZF2, XY(GS_ORIGIN + SCREEN_HALF_WIDTH, GS_ORIGIN + SCREEN_HALF_HEIGHT));
    GS_PUT(cursor, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
    xglDirectQueueCount = queueIndex + (((u32) cursor - (u32) start) >> 4);
    nmlPacketAddGsFlushWide();
}

INCLUDE_ASM("asm/main/nonmatchings/tu039", func_0019DF00);
