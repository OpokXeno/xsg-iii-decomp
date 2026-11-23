#include "common.h"
#include "main/Packet.h"
#include "gs.h"
#include "main/xgl_2.h"

typedef int s32;

void gifStateFlush(void);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkOpenDirectHLCode(XglPacket *packet, unsigned int count);
void gifStateReset(void);
void gifStateAddReg(int gifAddress, u64 gifData);
XglCamera *xglStudioSelectGetActiveCamera(int studio_index);
s32 xglFilterShouldSkip(void);

/* Scaffold u16 words at 0x004C8390 (no config name): page base pointers of the frame buffers. */
extern u16 D_004C8390[];
/* The pages of D_004C8390 the blur passes use (xgl_render's depth, work, frame and display bases). */
#define BLUR_DEPTH_PAGE D_004C8390[4]
#define BLUR_WORK_PAGE D_004C8390[7]
#define BLUR_FRAME_PAGE D_004C8390[9]
#define BLUR_DISPLAY_PAGE D_004C8390[11]
/* IEEE-754 bits of 1.0f, the Q word of RGBAQ. */
#define BLUR_FLOAT_ONE_BITS 0x3F800000

/* The 512x448 frame buffer in pixels, centred on the 2048 GS origin. */
#define BLUR_WIDTH 512
#define BLUR_HEIGHT 448
/* The shrunk copy: a quarter of the frame in each direction, never smaller than the minimum. */
#define BLUR_SHRUNK_WIDTH 128
#define BLUR_SHRUNK_HEIGHT 112
#define BLUR_MIN_SHRUNK_WIDTH 64
#define BLUR_MIN_SHRUNK_HEIGHT 56
/* Camera window corners in GS coordinates. */
#define BLUR_WINDOW_LEFT_X(camera) BLUR_X((int) (camera)->screen.window.vector.x)
#define BLUR_WINDOW_TOP_Y(camera) BLUR_Y((int) (camera)->screen.window.vector.y)
#define BLUR_WINDOW_RIGHT_X(camera) BLUR_X((int) (camera)->screen.window.vector.z)
#define BLUR_WINDOW_BOTTOM_Y(camera) BLUR_Y((int) (camera)->screen.window.vector.w)
#define BLUR_X(x) (((2048 - BLUR_WIDTH / 2 + (x)) << 4) - 8)
#define BLUR_Y(y) (((2048 - BLUR_HEIGHT / 2 + (y)) << 4) - 9)

/* Opens a packet that blends into page BLUR_WORK_PAGE from page BLUR_FRAME_PAGE at alpha. */
void blurBeginBlendPass(XglPacket *packet, int alpha)
{
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
void blurQueueSprite(XglPacket *packet, unsigned int width, unsigned int height, int offsetX, int offsetY, int alpha)
{
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(offsetX), BLUR_Y(offsetY), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(width * 16, height * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(offsetX + BLUR_SHRUNK_WIDTH), BLUR_Y(offsetY + BLUR_SHRUNK_HEIGHT), 0));
}

/* Where the pass copies are offset: radial = eight neighbours, diagonal = four corners, horizontal/vertical = sides. */
typedef enum BlurOffsetPattern {
    BLUR_PATTERN_RADIAL,
    BLUR_PATTERN_DIAGONAL,
    BLUR_PATTERN_HORIZONTAL,
    BLUR_PATTERN_VERTICAL
} BlurOffsetPattern;

/* Parameter block of xglRadialBlurApply. */
typedef struct BlurParams {
    s32 unusedWord00;
    s32 alpha;     /* +0x04 */
    s32 offsetPattern; /* +0x08: BlurOffsetPattern */
    s32 passCount; /* +0x0C */
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
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS)); \
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0)); \
        gifStateAddReg(GS_UV, GS_SET_UV(0, 0)); \
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0)); \
        gifStateAddReg(GS_UV, GS_SET_UV(BLUR_SHRUNK_WIDTH * 16, BLUR_SHRUNK_HEIGHT * 16)); \
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(width), BLUR_Y(height), 0)); \
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha)); \
        gifStateFlush(); \
        sceVif1PkCloseDirectHLCode(packet); \
    }

/* Shrinks the camera window into the corner of page BLUR_FRAME_PAGE, darkens page BLUR_WORK_PAGE, then blends the
 * shrunk copy back passCount - 1 times with offsets of the pass number (the pattern picked by mode), shrinking it
 * again after every pass, and finally stretches the result back over the camera window. */
void xglRadialBlurApply(BlurParams *blurParams, XglPacket *packet)
{
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
    fullWidth = (camera->screen.window.vector.z - camera->screen.window.vector.x) / BLUR_WIDTH * 256.0f;
    fullHeight = (camera->screen.window.vector.w - camera->screen.window.vector.y) * (1.0f / BLUR_HEIGHT) * 224.0f;

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_FBA_1, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(BLUR_DEPTH_PAGE, GS_PSMZ24, 1));
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, BLUR_WIDTH - 1, 0, BLUR_HEIGHT - 1));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x80));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_DISPLAY_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
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
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0, BLUR_FLOAT_ONE_BITS));
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
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
                                                camera->screen.window.vector.y, camera->screen.window.vector.w));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, BLUR_SHRUNK_WIDTH, 0, BLUR_SHRUNK_HEIGHT));
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x80));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(BLUR_SHRUNK_WIDTH * 16, BLUR_SHRUNK_HEIGHT * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, BLUR_WIDTH, 0, BLUR_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Parameter block of xglDepthFocusBlurApply. */
typedef struct FocusBlurParams {
    s32 unusedWord00;
    u32 depth;          /* +0x04: Z of the sharp plane */
    u32 passCount;      /* +0x08 */
    s32 alpha;          /* +0x0C: FIX of the final blend */
    s32 depthTest;      /* +0x10: nonzero tests the destination alpha instead of the depth */
    u32 blendMode;      /* +0x14 */
} FocusBlurParams;

/* Marks the pixels behind the sharp plane in the alpha channel of page BLUR_DISPLAY_PAGE, shrinks the frame into
 * page BLUR_FRAME_PAGE, blurs it there passCount + 1 times through page BLUR_WORK_PAGE, and blends the result
 * back over the camera window where the alpha (or the depth) allows it. */
void xglDepthFocusBlurApply(FocusBlurParams *blurParams, XglPacket *packet)
{
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
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_FBA_1, 1);
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_DISPLAY_PAGE, 8, GS_PSMCT32, 0xFFFFFF));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), depth));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), depth));
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(1, 3, 1, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), 0xFFFFFF));
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, BLUR_WIDTH, 0, BLUR_HEIGHT));
        gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0));
        gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_DISPLAY_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(266), BLUR_Y(234), 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
        gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(256), BLUR_Y(224), 0));
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
    } else {
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        gifStateReset();
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_FBA_1, 0);
        gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, BLUR_WIDTH, 0, BLUR_HEIGHT));
        gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, GS_PSMZ24, 1));
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_FRAME_PAGE, 8, GS_PSMCT32, 0));
        gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_DISPLAY_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(266), BLUR_Y(234), 0));
        gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
        gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
        gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
        gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(256), BLUR_Y(224), 0));
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, BLUR_WIDTH, 0, BLUR_HEIGHT));
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(BLUR_WORK_PAGE, 8, GS_PSMCT32, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, BLUR_FLOAT_ONE_BITS));
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
            gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, BLUR_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(330), BLUR_Y(290), 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
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
            gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
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
            gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_FRAME_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, BLUR_FLOAT_ONE_BITS));
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(0), BLUR_Y(0), 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_X(width + 10), BLUR_Y(height + 10), 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
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
            gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(BLUR_WORK_PAGE << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
            gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
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
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
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
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, BLUR_FLOAT_ONE_BITS));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_LEFT_X(camera), BLUR_WINDOW_TOP_Y(camera), depth));
    gifStateAddReg(GS_UV, GS_SET_UV(256 * 16, 224 * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(BLUR_WINDOW_RIGHT_X(camera), BLUR_WINDOW_BOTTOM_Y(camera), depth));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, BLUR_WIDTH, 0, BLUR_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E4348);

INCLUDE_ASM("asm/main/nonmatchings/tu111", func_001E4818);
