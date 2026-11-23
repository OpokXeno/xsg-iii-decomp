#include "common.h"
#include "shared.h"
#define GS_TEX0_1 0x06
#define GS_CLAMP_1 0x08
#define GS_TEX1_1 0x14
#define GS_TEXFLUSH 0x3F
#define GS_SCISSOR_1 0x40
#define GS_ALPHA_1 0x42
#define GS_TEST_1 0x47
#define GS_FBA_1 0x4A
#define GS_FRAME_1 0x4C
#define GS_ZBUF_1 0x4E
/* GIFtag register descriptor of an A+D (address plus data) register. */
#define GIF_AD 0x0E
/* GIFtag EOP bit (bit 15 of the NLOOP/EOP word). */
#define GIF_TAG_EOP 0x8000
#define GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld) \
    ((u64)(tbp) | ((u64)(tbw) << 14) | ((u64)(psm) << 20) | ((u64)(tw) << 26) | ((u64)(th) << 30) \
     | ((u64)(tcc) << 34) | ((u64)(tfx) << 35) | ((u64)(cbp) << 37) | ((u64)(cpsm) << 51) \
     | ((u64)(csm) << 55) | ((u64)(csa) << 56) | ((u64)(cld) << 61))
#define GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
    ((u64)(lcm) | ((u64)(mxl) << 2) | ((u64)(mmag) << 5) | ((u64)(mmin) << 6) | ((u64)(mtba) << 9) \
     | ((u64)(l) << 19) | ((u64)(k) << 32))
#define GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
    ((u64)(wms) | ((u64)(wmt) << 2) | ((u64)(minu) << 4) | ((u64)(maxu) << 14) | ((u64)(minv) << 24) \
     | ((u64)(maxv) << 34))
#define GS_SET_SCISSOR(x0, x1, y0, y1) ((u64)(x0) | ((u64)(x1) << 16) | ((u64)(y0) << 32) | ((u64)(y1) << 48))
#define GS_SET_ALPHA(a, b, c, d, fix) \
    ((u64)(a) | ((u64)(b) << 2) | ((u64)(c) << 4) | ((u64)(d) << 6) | ((u64)(fix) << 32))
#define GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
    ((u64)(ate) | ((u64)(atst) << 1) | ((u64)(aref) << 4) | ((u64)(afail) << 12) | ((u64)(date) << 14) \
     | ((u64)(datm) << 15) | ((u64)(zte) << 16) | ((u64)(ztst) << 17))
#define GS_SET_FRAME(fbp, fbw, psm, fbmsk) \
    ((u64)(fbp) | ((u64)(fbw) << 16) | ((u64)(psm) << 24) | ((u64)(fbmsk) << 32))
#define GS_SET_ZBUF(zbp, psm, zmsk) ((u64)(zbp) | ((u64)(psm) << 24) | ((u64)(zmsk) << 32))
#define GS_PRIM 0x00
#define GS_RGBAQ 0x01
#define GS_UV 0x03
#define GS_XYZF2 0x04
#define GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
    ((u64)(prim) | ((u64)(iip) << 3) | ((u64)(tme) << 4) | ((u64)(fge) << 5) | ((u64)(abe) << 6) \
     | ((u64)(aa1) << 7) | ((u64)(fst) << 8) | ((u64)(ctxt) << 9) | ((u64)(fix) << 10))
#define GS_SET_RGBAQ(r, g, b, a, q) \
    ((u64)(r) | ((u64)(g) << 8) | ((u64)(b) << 16) | ((u64)(a) << 24) | ((u64)(q) << 32))
#define GS_SET_UV(u, v) ((u64)(u) | ((u64)(v) << 16))
#define GS_SET_XYZ(x, y, z) ((u64)(x) | ((u64)(y) << 16) | ((u64)(z) << 32))
/* Pixel storage modes. */
#define GS_PSMCT32 0x00
#include "main/Packet.h"
#include "main/gifState.h"
#include "main/xgl_render.h"

extern GifStatusBlock gifStatus;
struct XglPacket;
void gifStateAddReg(int gifAddress, u64 gifData);
void gifStateReset(void);
void gifStateFlush(void);
unsigned short xglSRand(void);
void assertFailed(const char *message, const char *file, int line);
XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddDataN(XglPacket *packet, const void *data, unsigned int qwc);
void sceVif1PkOpenDirectHLCode(XglPacket *packet, unsigned int count);
void sceVif1PkAddDirectDataN(XglPacket *packet, const void *data, unsigned int qwc);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);
extern XglRenderState sRender;
void gifStateSetupContext(XglPacket *packet, signed char alphaFix);
extern int gifStateOriginX;
extern int gifStateOriginY;

/* The 512x448 frame buffer in pixels. */
#define GIF_STATE_WIDTH 512
#define GIF_STATE_HEIGHT 448
/* The quarter-size copy of the page: 128x112 pixels. */
#define GIF_QUARTER_WIDTH (GIF_STATE_WIDTH / 4)
#define GIF_QUARTER_HEIGHT (GIF_STATE_HEIGHT / 4)

/* One quadword of gifStatePacket: a data/address pair (A+D) or four raw words (the tag). */
typedef union GifStateQword {
    u64 adPair[2];                /* A+D: the data, then the register address */
    u32 rawBits[4];
} GifStateQword;

/* The setup packet: 15 quadwords (60 words) sent as one transfer; its qwords 6 and 7 get the frame and the
 * texture of the page each call. */
extern GifStateQword gifStateSetupPacket[15];

#define GIF_SETUP_PACKET_WORDS 60

float sinf(float angle);
float cosf(float angle);
extern int gifStateRingCount;
/* The GIF tag template of the ring (the vertex count is or-ed in); no symbol name yet. */
extern u32 gifRingGifTag[4];

/* Scratchpad RAM staging area of the ring packet. */
#define GIF_SCRATCHPAD 0x70000000
#define GIF_X(x) ((gifStateOriginX + (x)) * 16 - 8)
#define GIF_Y(y) ((gifStateOriginY + (y)) * 16 - 9)

/* The packet the redraws build on the scratchpad: the GIF tag, then vertices of UV and XYZ2 (as many as
 * fit its 16 KB). */
typedef struct GifVertex {
    u32 uv[4];
    u32 xyz[4];
} GifVertex;

#define GIF_PACKET_VERTICES ((0x4000 - 0x10) / sizeof(GifVertex))

typedef struct GifPacket {
    u32 tag[4];
    GifVertex vertex[GIF_PACKET_VERTICES];
} GifPacket;

#define GIF_PACKET ((GifPacket *) 0x70000000)

/* Spacing of the jitter grid in pixels. */
extern int gifStateGridStep;
/* GIF tag templates of the first strip and of each redrawn row (the vertex count is or-ed in). */
extern u32 gifGridFirstRowTag[4];
extern u32 gifGridRowTag[4];
/* The GIF tag template of the fade quad (the vertex count is or-ed in). */
extern u32 gifFadeQuadTag[4];
/*
 * GIF state packet. gifStateNumPacket (numPacket) counts the register pairs queued in
 * gifStatePacket; entry 0 is the GIF tag word, the pairs follow in packed mode (A+D).
 * The assert text "numPacket < (STATEPACKET_MAX - 1)" names both.
 */
extern unsigned int gifStateNumPacket;
extern GifStateQword gifStatePacket[128];

#define GIF_TAG_NREG_1 0x10000000

/* The assert text and the file name of the packet queue asserts (no config name: splat names). */
extern const char gifStatePacketAssert[40];
extern const char gifStateSourceFile[24];

/* The queue size the assert names: numPacket < (STATEPACKET_MAX - 1); the compare is against 63 in this build. */
#define STATEPACKET_MAX 0x40

/* Scaffold u16 words at 0x004C8390 (no config name): the frame-buffer pages of the render state, in 32-word
 * units (tu119 reads the same words as outlinePage +0x0E, glowSourcePage +0x12 and screenPage +0x16). */
extern u16 D_004C8390[];
#define GIF_PAGE_OUTLINE D_004C8390[7]
#define GIF_PAGE_GLOW_SOURCE D_004C8390[9]
#define GIF_PAGE_SCREEN D_004C8390[11]

/* The frame register and the texture register that address a page (PSMCT32, 512 pixels wide, 512x512 texture). */
#define GIF_PAGE_FRAME(page) GS_SET_FRAME(page, 8, GS_PSMCT32, 0xFF000000)
#define GIF_PAGE_TEX0(page) GS_SET_TEX0((page) << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0)
/* Alpha blend source * fixed alpha + destination * (1 - fixed alpha). */
#define GIF_BLEND_FIXED_ALPHA(fixedAlpha) GS_SET_ALPHA(0, 2, 2, 1, fixedAlpha)
/* Vertex colors: 0x80 is the neutral texture modulation; Q is 1.0f. */
#define GIF_RGBAQ_NEUTRAL GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000)
#define GIF_RGBAQ_BLACK GS_SET_RGBAQ(0, 0, 0, 0, 0x3F800000)

/* Writes the setup packet for the page each call (its frame and texture words), sends it, then queues the
 * context registers: clamp, frame, the alpha blend picked by the effect state, texture, test and z buffer. */
void gifStateSetupContext(XglPacket *packet, signed char alphaFix)
{
    gifStateSetupPacket[6].adPair[0] = GS_SET_FRAME(D_004C8390[7], 8, GS_PSMCT32, 0);
    switch (gifStatus.fbp) {
    case 0:
        gifStateSetupPacket[7].adPair[0] = GIF_PAGE_TEX0(GIF_PAGE_SCREEN);
        break;
    case 1:
        gifStateSetupPacket[7].adPair[0] = GS_SET_TEX0(D_004C8390[12] << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0);
        break;
    default:
        gifStateSetupPacket[7].adPair[0] = GIF_PAGE_TEX0(GIF_PAGE_SCREEN);
        break;
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateFlush();
    sceVif1PkAddDataN(packet, gifStateSetupPacket, GIF_SETUP_PACKET_WORDS);
    gifStateReset();
    gifStateAddReg(GS_FBA_1, 1);
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_SCREEN));
    if (gifStatus.state == 1) {
        switch (gifStatus.effectAlphaType) {
        case 1:
            gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 0, 1, 0));
            break;
        case 2:
            gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(2, 0, 0, 1, 0));
            break;
        case 0:
        default:
            gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0));
            break;
        }
    } else {
        switch (gifStatus.effectAlphaType) {
        case 1:
            gifStateAddReg(GS_ALPHA_1, ((((long long) alphaFix) << 56) >> 24) | GIF_BLEND_FIXED_ALPHA(0));
            break;
        case 2:
            gifStateAddReg(GS_ALPHA_1, ((((long long) alphaFix) << 56) >> 24) | GS_SET_ALPHA(2, 0, 2, 1, 0));
            break;
        case 0:
            gifStateAddReg(GS_ALPHA_1, ((((long long) alphaFix) << 56) >> 24) | GS_SET_ALPHA(0, 1, 2, 1, 0));
            break;
        default:
            gifStateAddReg(GS_ALPHA_1, ((((long long) alphaFix) << 56) >> 24) | GS_SET_ALPHA(0, 1, 2, 1, 0));
            break;
        }
    }
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, GIF_STATE_WIDTH - 1, 0, GIF_STATE_HEIGHT - 1));
    gifStateAddReg(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(D_004C8390[7] << 5, 8, GS_PSMCT32, 9, 9, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, 0x31, 1));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Draws the shown page again as a ring of sprite points around the effect centre, in two passes: the
 * outer points at the sub-window radii, then the second ring at the radius of subWindow[1] plus subWindow[3]
 * with its ratio-scaled start angle. Nothing is drawn while the ratio is 0; then the ratio is lowered by its
 * step. */
void gifStateDrawRing(XglPacket *packet)
{
    int trailAlpha;
    int ringAlpha;
    float angleStep;
    float angle = 0.0f;
    float cosine;
    float sine;
    u32 *cursor;
    u32 *ring;
    u32 *gifTag;
    unsigned int i;
    int outerVertexCount;
    int innerVertexCount;
    int innerSpan;
    int trailSpan;
    int outerRadius;
    int outerX;
    int outerY;
    int trailX;
    int trailY;
    int blurX;
    int blurY;

    angleStep = 6.2831855f / gifStateRingCount;
    if (gifStatus.ratio == 0.0f) {
        return;
    }
    if (gifStatus.subWindow[0] == 0) {
        trailAlpha = gifStatus.ratio * gifStatus.effectAlpha;
        ringAlpha = 0;
    } else {
        ringAlpha = gifStatus.ratio * gifStatus.effectAlpha;
        trailAlpha = 0;
    }
    gifStatus.ratio -= gifStatus.ratioStep;
    outerRadius = gifStatus.subWindow[1];
    trailSpan = gifStatus.subWindow[2];
    innerSpan = gifStatus.subWindow[3];
    if (gifStatus.ratio < 0.0f) {
        gifStatus.ratio = 0.0f;
    }
    gifStateSetupContext(packet, 0);

    /* The outer ring: per angleStep one point at the radius outerRadius and one at outerRadius + trailSpan pulled in by the blur. */
    outerVertexCount = 0;
    cursor = GIF_PACKET->vertex[0].uv;
    angle = 0.0f;
    for (i = 0; i < 2 * gifStateRingCount; i += 2) {
        cosine = cosf(angle);
        sine = sinf(angle);
        outerX = cosine * outerRadius + gifStatus.centerX;
        outerY = sine * outerRadius + gifStatus.centerY;
        trailX = cosine * (outerRadius + trailSpan) + gifStatus.centerX;
        trailY = sine * (outerRadius + trailSpan) + gifStatus.centerY;
        angle += angleStep;
        outerX = outerX < 0 ? 0 : outerX;
        outerY = outerY < 0 ? 0 : outerY;
        outerX = outerX >= GIF_STATE_WIDTH ? GIF_STATE_WIDTH - 1 : outerX;
        outerY = outerY >= GIF_STATE_HEIGHT ? GIF_STATE_HEIGHT - 1 : outerY;
        cursor[0] = 0x80;
        cursor[1] = 0x80;
        cursor[2] = 0x80;
        cursor[3] = ringAlpha;
        cursor[6] = 0;
        cursor[7] = 0;
        cursor[4] = outerX << 4;
        cursor[5] = outerY << 4;
        cursor[8] = GIF_X(outerX);
        cursor[9] = GIF_Y(outerY);
        cursor[10] = gifStatus.depth;
        cursor[11] = 0;
        cursor += 12;

        cursor[0] = 0x80;
        cursor[1] = 0x80;
        cursor[2] = 0x80;
        cursor[3] = trailAlpha;
        cosine *= gifStatus.blur;
        blurX = trailX - (int) cosine;
        blurY = trailY - (int) (sine * gifStatus.blur);
        blurY = blurY < 0 ? 0 : blurY;
        blurX = blurX < 0 ? 0 : blurX;
        blurY = blurY >= GIF_STATE_HEIGHT ? GIF_STATE_HEIGHT - 1 : blurY;
        blurX = blurX >= GIF_STATE_WIDTH ? GIF_STATE_WIDTH - 1 : blurX;
        cursor[6] = 0;
        cursor[7] = 0;
        cursor[5] = blurY << 4;
        cursor[4] = blurX << 4;
        cursor[8] = GIF_X(trailX);
        cursor[9] = GIF_Y(trailY);
        cursor[10] = gifStatus.depth;
        cursor[11] = 0;
        cursor += 12;
        outerVertexCount += 2;
    }
    outerVertexCount += 2;
    i = 0;
    innerVertexCount = 0;
    gifTag = GIF_PACKET->tag;
    gifTag[0] = gifRingGifTag[0] | outerVertexCount;
    gifTag[1] = gifRingGifTag[1];
    gifTag[2] = gifRingGifTag[2];
    gifTag[3] = gifRingGifTag[3];
    /* Close the strip: the first two points again. */
    gifTag += 4;
    cursor[0] = gifTag[0];
    cursor[1] = gifTag[1];
    cursor[2] = gifTag[2];
    cursor[3] = gifTag[3];
    cursor[4] = gifTag[4];
    cursor[5] = gifTag[5];
    cursor[6] = gifTag[6];
    cursor[7] = gifTag[7];
    cursor[8] = gifTag[8];
    cursor[9] = gifTag[9];
    cursor[10] = gifTag[10];
    cursor[11] = gifTag[11];
    cursor += 12;
    gifTag += 12;
    cursor[0] = gifTag[0];
    cursor[1] = gifTag[1];
    cursor[2] = gifTag[2];
    cursor[3] = gifTag[3];
    cursor[4] = gifTag[4];
    cursor[5] = gifTag[5];
    cursor[6] = gifTag[6];
    cursor[7] = gifTag[7];
    cursor[8] = gifTag[8];
    cursor[9] = gifTag[9];
    cursor[10] = gifTag[10];
    cursor[11] = gifTag[11];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, GIF_PACKET, outerVertexCount * 3 + 1);
    sceVif1PkCloseDirectHLCode(packet);

    ring = GIF_PACKET->vertex[0].uv;
    /* Move each outer point onto its inner neighbour. */
    for (; i < 2 * gifStateRingCount + 2; i += 2) {
        ring[0] = ring[12];
        ring[1] = ring[13];
        ring[2] = ring[14];
        ring[3] = ring[15];
        ring[4] = ring[16];
        ring[5] = ring[17];
        ring[6] = ring[18];
        ring[7] = ring[19];
        ring[8] = ring[20];
        ring[9] = ring[21];
        ring[10] = ring[22];
        ring[11] = ring[23];
        ring += 24;
    }

    /* The inner ring: the outer points again at the radius outerRadius + innerSpan, from the ratio-scaled start angle. */
    angle = angleStep * gifStatus.ratio;
    ring = GIF_PACKET->vertex[0].uv;
    for (i = 0; i < 2 * gifStateRingCount; i += 2) {
        int innerX;
        int innerY;

        ring += 12;
        cosine = cosf(angle);
        sine = sinf(angle);
        cosine *= outerRadius + innerSpan;
        cosine += gifStatus.centerX;
        innerY = sine * (outerRadius + innerSpan) + gifStatus.centerY;
        innerX = cosine;
        angle += angleStep;
        innerY = innerY < 0 ? 0 : innerY;
        innerX = innerX < 0 ? 0 : innerX;
        innerX = innerX >= GIF_STATE_WIDTH ? GIF_STATE_WIDTH - 1 : innerX;
        innerY = innerY >= GIF_STATE_HEIGHT ? GIF_STATE_HEIGHT - 1 : innerY;
        ring[3] = ringAlpha;
        ring[5] = innerY << 4;
        ring[4] = innerX << 4;
        ring[8] = GIF_X(innerX);
        ring[9] = GIF_Y(innerY);
        ring[10] = gifStatus.depth;
        ring += 12;
        innerVertexCount += 2;
    }
    innerVertexCount += 2;
    gifTag = GIF_PACKET->tag;
    gifTag[0] = gifRingGifTag[0] | innerVertexCount;
    gifTag[1] = gifRingGifTag[1];
    gifTag[2] = gifRingGifTag[2];
    gifTag[3] = gifRingGifTag[3];
    gifTag += 4;
    ring[0] = gifTag[0];
    ring[1] = gifTag[1];
    ring[2] = gifTag[2];
    ring[3] = gifTag[3];
    ring[4] = gifTag[4];
    ring[5] = gifTag[5];
    ring[6] = gifTag[6];
    ring[7] = gifTag[7];
    ring[8] = gifTag[8];
    ring[9] = gifTag[9];
    ring[10] = gifTag[10];
    ring[11] = gifTag[11];
    ring += 12;
    gifTag += 12;
    ring[0] = gifTag[0];
    ring[1] = gifTag[1];
    ring[2] = gifTag[2];
    ring[3] = gifTag[3];
    ring[4] = gifTag[4];
    ring[5] = gifTag[5];
    ring[6] = gifTag[6];
    ring[7] = gifTag[7];
    ring[8] = gifTag[8];
    ring[9] = gifTag[9];
    ring[10] = gifTag[10];
    ring[11] = gifTag[11];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, GIF_PACKET, innerVertexCount * 3 + 1);
    sceVif1PkCloseDirectHLCode(packet);
}

#include "main/xgl_2.h"

XglCamera *xglStudioSelectGetActiveCamera(int studio_index);
void tiVu0LoadIdentityMatrix(void);
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurMulRotateZ(float angle);
void xglCurMatrixGet(Matrix4 matrix);
void xglMatrixInverse(Matrix *destination, Matrix *source);
void xglVectorScale(Vector4 *destination, const Vector4 *source, float scale);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
/* The GIF tag template of the camera quad (the vertex count is or-ed in). */
extern u32 gifCameraQuadTag[4];
/* Gp-relative float cell the radial blur keeps its minimum scale in. */
extern float gifBlurMinScale;
/* The scaffold float cells the radial blur reads: 0.9 and 0.01. */
extern const float gifBlurScaleFloor;
extern const float gifBlurScaleStep;
/* The per-edge scales of the radial blur (512 floats; the splat label ends before the table does). */
extern float gifBlurScales[];
/* The GIF tag template of the radial blur fan (the vertex count is or-ed in). */
extern u32 gifBlurFanTag[4];

/* The fan's edge points are this far apart at least. */
#define GIF_BLUR_MIN_STEP 4

/* Draws the shown page again as a triangle fan from the effect centre to points around the screen edge, every
 * subWindow[0] pixels; each edge point samples the texture at a random scale between 0.9 and 1.0 of the ratio
 * towards the centre, so the picture blurs radially. The scales are drawn anew and the ratio lowered by its
 * step. Nothing is drawn while the ratio is 0. */
void gifStateDrawRadialBlur(XglPacket *packet)
{
    u32 *cursor;
    u32 *fanTag = 0;
    int scaleIndex;
    int fanVertexCount;
    int edgeSpacing;
    int edgePos;
    float edgeScale = 0.0f;

    if (gifStatus.ratio == edgeScale) {
        return;
    }
    gifBlurMinScale = gifStatus.ratio * gifBlurScaleFloor;
    for (edgePos = 0; edgePos < 512; edgePos++) {
        gifBlurScales[edgePos] = (xglSRand() % 10U) * gifBlurScaleStep + gifBlurMinScale;
    }
    scaleIndex = 0;
    fanVertexCount = 1;
    edgePos = 0;
    gifStateSetupContext(packet, gifStatus.effectAlpha * gifStatus.ratio);
    edgeSpacing = gifStatus.subWindow[0];
    if (edgeSpacing < GIF_BLUR_MIN_STEP) {
        edgeSpacing = GIF_BLUR_MIN_STEP;
    }
    cursor = GIF_PACKET->vertex[0].uv;
    cursor[0] = gifStatus.centerX << 4;
    cursor[1] = gifStatus.centerY << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = ((gifStateOriginX + gifStatus.centerX) << 4) - 8;
    cursor[5] = ((gifStateOriginY + gifStatus.centerY) << 4) - 9;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor = GIF_PACKET->vertex[1].uv;
    do {
        edgeScale = gifBlurScales[scaleIndex++];
        cursor[0] = (gifStatus.centerX + (edgePos - gifStatus.centerX) * edgeScale) * 16.0f;
        cursor[1] = (gifStatus.centerY + (0 - gifStatus.centerY) * edgeScale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        fanVertexCount++;
        cursor[4] = ((gifStateOriginX + edgePos) << 4) - 8;
        edgePos += edgeSpacing;
        cursor[5] = ((gifStateOriginY + 0) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    } while (edgePos < GIF_STATE_WIDTH);
    for (edgePos = 0; edgePos < GIF_STATE_HEIGHT; edgePos += edgeSpacing) {
        edgeScale = gifBlurScales[scaleIndex++];
        cursor[0] = (gifStatus.centerX + ((GIF_STATE_WIDTH - 1) - gifStatus.centerX) * edgeScale) * 16.0f;
        cursor[1] = (gifStatus.centerY + (edgePos - gifStatus.centerY) * edgeScale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        fanVertexCount++;
        cursor[4] = ((gifStateOriginX + (GIF_STATE_WIDTH - 1)) << 4) - 8;
        cursor[5] = ((gifStateOriginY + edgePos) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    }
    for (edgePos = GIF_STATE_WIDTH - 1; edgePos >= 0; edgePos -= edgeSpacing) {
        edgeScale = gifBlurScales[scaleIndex++];
        cursor[0] = (gifStatus.centerX + (edgePos - gifStatus.centerX) * edgeScale) * 16.0f;
        cursor[1] = (gifStatus.centerY + ((GIF_STATE_HEIGHT - 1) - gifStatus.centerY) * edgeScale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        fanVertexCount++;
        cursor[4] = ((gifStateOriginX + edgePos) << 4) - 8;
        cursor[5] = ((gifStateOriginY + (GIF_STATE_HEIGHT - 1)) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    }
    for (edgePos = GIF_STATE_HEIGHT - 1; edgePos >= 0; edgePos -= edgeSpacing) {
        edgeScale = gifBlurScales[scaleIndex++];
        cursor[0] = (gifStatus.centerX + (0 - gifStatus.centerX) * edgeScale) * 16.0f;
        cursor[1] = (gifStatus.centerY + (edgePos - gifStatus.centerY) * edgeScale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        fanVertexCount++;
        cursor[4] = ((gifStateOriginX + 0) << 4) - 8;
        cursor[5] = ((gifStateOriginY + edgePos) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    }
    fanVertexCount++;
    fanTag = GIF_PACKET->tag;
    fanTag[0] = gifBlurFanTag[0] | fanVertexCount;
    fanTag[1] = gifBlurFanTag[1];
    fanTag[2] = gifBlurFanTag[2];
    fanTag[3] = gifBlurFanTag[3];
    /* The fan closes on its first edge point: past the tag come vertex 0 (the centre) and vertex 1, whose
     * eight words go once more to the end. */
    fanTag += 4;
    cursor[0] = fanTag[8];
    cursor[1] = fanTag[9];
    cursor[2] = fanTag[10];
    cursor[3] = fanTag[11];
    cursor[4] = fanTag[12];
    cursor[5] = fanTag[13];
    cursor[6] = fanTag[14];
    cursor[7] = fanTag[15];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, GIF_PACKET, fanVertexCount * 2 + 1);
    sceVif1PkCloseDirectHLCode(packet);
    gifStatus.ratio -= gifStatus.ratioStep;
    if (gifStatus.ratio < 0.0f) {
        gifStatus.ratio = 0.0f;
    }
}

/* Draws the shown page again as a 512x448 sprite, offset on the screen by the blur direction turned into
 * the active camera's view and scaled by the blur amount, with the effect alpha faded by the ratio; then
 * lowers the ratio by its step unless the decrease gate is set. Nothing is drawn once the ratio is down to 0. */
void gifStateDrawCameraQuad(XglPacket *packet)
{
    Vector4 blurVector;
    Vector4 blurOffset;
    Matrix matrix;
    int corners[4][2];
    XglCamera *camera = xglStudioSelectGetActiveCamera(0);
    u32 *cursor;
    u32 *gifTag = 0;
    int shiftX;
    int shiftY;

    if (gifStatus.ratio <= 0.0f) {
        return;
    }
    tiVu0LoadIdentityMatrix();
    xglCurMulRotateY(camera->travel.rotation.y);
    xglCurMulRotateX(camera->travel.rotation.x);
    xglCurMulRotateZ(camera->travel.rotation.z);
    xglCurMatrixGet(matrix.rows);
    xglMatrixInverse(&matrix, &matrix);
    xglVectorScale(&blurVector, &gifStatus.mulColor, gifStatus.blur);
    xglVectorMulMat(&blurOffset, matrix.rows, &blurVector);
    shiftX = -blurOffset.x;
    shiftY = blurOffset.y;
    corners[0][0] = shiftX;
    corners[0][1] = shiftY;
    corners[1][0] = shiftX;
    corners[1][1] = shiftY;
    corners[2][0] = shiftX;
    corners[2][1] = shiftY;
    corners[3][0] = shiftX;
    corners[3][1] = shiftY;
    gifStateSetupContext(packet, gifStatus.ratio * gifStatus.effectAlpha);
    cursor = GIF_PACKET->vertex[0].uv;
    cursor[0] = 0;
    cursor[1] = 0;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = ((gifStateOriginX + corners[0][0]) << 4) - 8;
    cursor[5] = ((gifStateOriginY + corners[0][1]) << 4) - 9;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor = GIF_PACKET->vertex[1].uv;
    cursor[0] = (GIF_STATE_WIDTH - 1) << 4;
    cursor[1] = 0;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = ((gifStateOriginX + corners[1][0] + (GIF_STATE_WIDTH - 1)) << 4) - 8;
    cursor[5] = ((gifStateOriginY + corners[1][1]) << 4) - 9;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor = GIF_PACKET->vertex[2].uv;
    cursor[0] = 0;
    cursor[1] = (GIF_STATE_HEIGHT - 1) << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = ((gifStateOriginX + corners[2][0]) << 4) - 8;
    cursor[5] = ((gifStateOriginY + corners[2][1] + (GIF_STATE_HEIGHT - 1)) << 4) - 9;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor = GIF_PACKET->vertex[3].uv;
    cursor[0] = (GIF_STATE_WIDTH - 1) << 4;
    cursor[1] = (GIF_STATE_HEIGHT - 1) << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = ((gifStateOriginX + corners[3][0] + (GIF_STATE_WIDTH - 1)) << 4) - 8;
    cursor[5] = ((gifStateOriginY + corners[3][1] + (GIF_STATE_HEIGHT - 1)) << 4) - 9;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    gifTag = GIF_PACKET->tag;
    gifTag[0] = gifCameraQuadTag[0] | 4;
    gifTag[1] = gifCameraQuadTag[1];
    gifTag[2] = gifCameraQuadTag[2];
    gifTag[3] = gifCameraQuadTag[3];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, GIF_PACKET, 9);
    sceVif1PkCloseDirectHLCode(packet);
    if (gifStatus.decreaseGate == 0) {
        gifStatus.ratio -= gifStatus.ratioStep;
    }
}

/* Draws the shown page as a grid of vertices whose texture coordinates are shifted by a random amount scaled by
 * the ratio (never on the left column or the top row): the first pass builds a strip of the first two rows,
 * every later row copies the lower vertex of the strip above and adds a new one, each strip sent as its own
 * packet. The ratio is then lowered by its step and clamped at 0. The cursor walks the scratchpad packet
 * eight words (one vertex) at a time; the second vertex of the first pass tests the left column through a
 * flag, which the original keeps as a separate compare. */
void gifStateDrawJitterGrid(XglPacket *packet)
{
    u32 *cursor;
    int gridX;
    int gridY;
    int stripVertexCount = 0;
    int jitter;
    int leftColumn;
    u32 *tag = 0;
    u32 *rowTag = 0;

    if (gifStatus.ratio != 0.0f) {
        gridY = 0;
        gridX = gridY;
        stripVertexCount = gridX;
        cursor = GIF_PACKET->vertex[0].uv;
        gifStateSetupContext(packet, gifStatus.effectAlpha * gifStatus.ratio);
        if (gifStatus.blur == 0) {
            gifStatus.blur = 1;
        }
        do {
            if (gridX == 0) {
                jitter = 0;
            } else {
                jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
            }
            cursor[0] = (gridX + jitter) << 4;
            cursor[1] = gridY << 4;
            cursor[2] = 0;
            cursor[3] = 0;
            cursor[4] = ((gifStateOriginX + gridX) << 4) - 8;
            cursor[5] = ((gifStateOriginY + gridY) << 4) - 9;
            cursor[6] = gifStatus.depth;
            cursor[7] = 0;
            stripVertexCount++;
            cursor += 8;
            gridY += gifStateGridStep;
            leftColumn = gridX == 0;
            if (leftColumn) {
                jitter = gridX;
            } else {
                jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
            }
            cursor[0] = (gridX + jitter) << 4;
            cursor[1] = gridY << 4;
            cursor[2] = 0;
            cursor[3] = 0;
            cursor[4] = ((gifStateOriginX + gridX) << 4) - 8;
            cursor[5] = ((gifStateOriginY + gridY) << 4) - 9;
            cursor[6] = gifStatus.depth;
            cursor[7] = 0;
            stripVertexCount++;
            cursor += 8;
            gridY -= gifStateGridStep;
            gridX += gifStateGridStep;
        } while (gridX < GIF_STATE_WIDTH);
        tag = GIF_PACKET->tag;
        tag[0] = gifGridFirstRowTag[0] | stripVertexCount;
        tag[1] = gifGridFirstRowTag[1];
        tag[2] = gifGridFirstRowTag[2];
        tag[3] = gifGridFirstRowTag[3];
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        sceVif1PkAddDirectDataN(packet, GIF_PACKET, stripVertexCount * 2 + 1);
        sceVif1PkCloseDirectHLCode(packet);

        for (gridY += gifStateGridStep; gridY < GIF_STATE_HEIGHT; gridY += gifStateGridStep) {
            gridX = 0;
            stripVertexCount = 0;
            cursor = GIF_PACKET->vertex[0].uv;
            do {
                cursor[0] = cursor[8];
                cursor[1] = cursor[9];
                cursor[2] = cursor[10];
                cursor[3] = cursor[11];
                cursor[4] = cursor[12];
                cursor[5] = cursor[13];
                cursor[6] = cursor[14];
                cursor[7] = cursor[15];
                stripVertexCount++;
                cursor += 8;
                gridY += gifStateGridStep;
                if (gridX == 0) {
                    jitter = 0;
                } else {
                    jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
                }
                cursor[0] = (gridX + jitter) << 4;
                if (gridY == 0) {
                    jitter = 0;
                } else {
                    jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
                }
                cursor[1] = (gridY + jitter) << 4;
                cursor[2] = 0;
                cursor[3] = 0;
                cursor[4] = ((gifStateOriginX + gridX) << 4) - 8;
                cursor[5] = ((gifStateOriginY + gridY) << 4) - 9;
                cursor[6] = gifStatus.depth;
                cursor[7] = 0;
                stripVertexCount++;
                cursor += 8;
                gridX += gifStateGridStep;
                gridY -= gifStateGridStep;
            } while (gridX < GIF_STATE_WIDTH);
            rowTag = GIF_PACKET->tag;
            rowTag[0] = gifGridRowTag[0] | stripVertexCount;
            rowTag[1] = gifGridRowTag[1];
            rowTag[2] = gifGridRowTag[2];
            rowTag[3] = gifGridRowTag[3];
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            sceVif1PkAddDirectDataN(packet, GIF_PACKET, stripVertexCount * 2 + 1);
            sceVif1PkCloseDirectHLCode(packet);
        }
        gifStatus.ratio -= gifStatus.ratioStep;
        if (gifStatus.ratio < 0.0f) {
            gifStatus.ratio = 0.0f;
        }
    }
}

/* Draws the shown page again as a 512x448 sprite whose corners move by the sub-window offsets scaled by
 * the ratio, with the effect alpha faded by the ratio; then lowers the ratio by its step. Nothing is
 * drawn once the ratio is down to 0. */
void gifStateDrawFadeQuad(XglPacket *packet)
{
    u32 *cursor;
    u32 *gifTag = 0;
    int vertexCount;

    if (gifStatus.ratio <= 0.0f) {
        return;
    }
    gifStateSetupContext(packet, gifStatus.ratio * gifStatus.effectAlpha);
    cursor = GIF_PACKET->vertex[0].uv;
    vertexCount = 0;
    cursor[0] = 0;
    cursor[1] = 0;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + gifStatus.subWindow[0] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + gifStatus.subWindow[1] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor += 8;
    vertexCount++;
    cursor[0] = (GIF_STATE_WIDTH - 1) << 4;
    cursor[1] = 0;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + (GIF_STATE_WIDTH - 1) + gifStatus.subWindow[0] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + gifStatus.subWindow[1] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor += 8;
    vertexCount++;
    cursor[0] = 0;
    cursor[1] = (GIF_STATE_HEIGHT - 1) << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + gifStatus.subWindow[2] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + (GIF_STATE_HEIGHT - 1) + gifStatus.subWindow[3] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor += 8;
    vertexCount++;
    cursor[0] = (GIF_STATE_WIDTH - 1) << 4;
    cursor[1] = (GIF_STATE_HEIGHT - 1) << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + (GIF_STATE_WIDTH - 1) + gifStatus.subWindow[2] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + (GIF_STATE_HEIGHT - 1) + gifStatus.subWindow[3] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    vertexCount++;
    gifTag = GIF_PACKET->tag;
    gifTag[0] = gifFadeQuadTag[0] | vertexCount;
    gifTag[1] = gifFadeQuadTag[1];
    gifTag[2] = gifFadeQuadTag[2];
    gifTag[3] = gifFadeQuadTag[3];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, gifTag, vertexCount * 2 + 1);
    sceVif1PkCloseDirectHLCode(packet);
    gifStatus.ratio -= gifStatus.ratioStep;
}

void gifStateDrawNone(XglPacket *packet) {
}

/* Three small-data words the timing setters below write (no config name yet: splat names): the echo pattern
 * (switch of gifStateDrawEchoBlur), the numerator of its alpha (alpha = gifStateEchoAlphaBase / gifStateEchoPassCount) and the pass count. */
extern int gifStateEchoPattern;
extern int gifStateEchoAlphaBase;
extern int gifStateEchoPassCount;

void gifStateResetTimings(void)
{
    gifStateEchoAlphaBase = 90;
    gifStateEchoPassCount = 6;
    gifStateEchoPattern = 0;
}

void gifStateSetTimings(int echoPattern, int echoAlphaBase, int echoPassCount)
{
    gifStateEchoPattern = echoPattern;
    gifStateEchoAlphaBase = echoAlphaBase;
    if (echoPassCount <= 0) {
        gifStateEchoPassCount = 1;
    } else {
        gifStateEchoPassCount = echoPassCount;
    }
}

/* Queues the quad the shown page is drawn as: the setup registers, the texture of the display page, and four
 * vertices at the origin of the page and its half-size corner, sent as one transfer. */
void gifStateDrawQuad(XglPacket *packet, unsigned int width, unsigned int height, int offsetX, int offsetY, int alpha)
{
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 100, width + 99, 100, height + 99));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(alpha));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(offsetX * 16 + 1600, offsetY * 16 + 1600));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_UV, GS_SET_UV((width + offsetX) * 16 + 1600, (height + offsetY) * 16 + 1600));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(128), GIF_Y(112), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Queues the shown page as one quad like gifStateDrawQuad, its texture window one texel wider and the far texel
 * corner moved back by the offset instead of forward. */
void gifStateDrawPageQuadInset(XglPacket *packet, unsigned int width, unsigned int height, int offsetX, int offsetY, int alpha)
{
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 100, width + 100, 100, height + 100));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(alpha));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(offsetX * 16 + 1600, offsetY * 16 + 1600));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_UV, GS_SET_UV((width - offsetX) * 16 + 1600, (height - offsetY) * 16 + 1600));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(GIF_QUARTER_WIDTH), GIF_Y(GIF_QUARTER_HEIGHT), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Copies the other page back at full size: black border strips below and right of the width x height area,
 * then the 128x112 page as one textured sprite. */
void gifStateRestorePageBorder(XglPacket *packet, int width, int height)
{
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, GIF_QUARTER_WIDTH, 0, GIF_QUARTER_HEIGHT));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_BLACK);
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(100), GIF_Y(height + 95), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(width + 105), GIF_Y(height + 105), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(width + 105), GIF_Y(100), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(width + 105), GIF_Y(height + 105), 0));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(100), GIF_Y(100), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(width + 100), GIF_Y(height + 100), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

/* Two more small-data words of the echo blur (no config name yet: splat names): the offset step of one echo
 * pass and the divisor of the darkening alpha (the darkening quad uses 40 / divisor). */
extern int gifStateEchoOffsetStep;
extern int gifStateEchoDarkenDivisor;

/* Echo blur of the camera window: copies the window shrunk into the 256x224 corner of another page, darkens it,
 * then draws the copy over itself in four shifted directions per pass with offsets growing every pass (the
 * pattern is picked by gifStateEchoPattern, the pass count is gifStateEchoPassCount), shrinking the copy between passes, and finally
 * stretches the result back over the window. */
void gifStateDrawEchoBlur(XglPacket *packet)
{
    int echoAlpha = 15;
    int fullWidth;
    int fullHeight;
    XglCamera *camera = xglStudioSelectGetActiveCamera(0);
    /* One counter (kept as one variable): the strip column of the first copy, then the echo pass number. */
    int stripOrPass;
    int echoWidth;
    int echoHeight;

    if (gifStateEchoPattern != 0) {
        echoAlpha = gifStateEchoAlphaBase / gifStateEchoPassCount;
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
                                                camera->screen.window.vector.y, camera->screen.window.vector.w));
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, 0x31, 1));
    gifStateAddReg(GS_FBA_1, 1);
    gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
    gifStateAddReg(GS_TEX1_1, GS_SET_TEX1(0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_SCREEN));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    for (stripOrPass = 0; stripOrPass < GIF_STATE_WIDTH; stripOrPass += 64) {
        gifStateAddReg(GS_UV, GS_SET_UV(stripOrPass * 16, camera->screen.window.vector.y * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(stripOrPass), GIF_Y(camera->screen.window.vector.y), 0));
        gifStateAddReg(GS_UV, GS_SET_UV((stripOrPass + 64) * 16, camera->screen.window.vector.w * 16.0f));
        gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(stripOrPass + 64), GIF_Y(camera->screen.window.vector.w), 0));
    }
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, GIF_STATE_WIDTH, 0, GIF_STATE_HEIGHT));
    gifStateAddReg(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    fullWidth = (camera->screen.window.vector.z - camera->screen.window.vector.x) / GIF_STATE_WIDTH * 256.0f;
    fullHeight = (camera->screen.window.vector.w - camera->screen.window.vector.y) * (1.0f / GIF_STATE_HEIGHT) * 224.0f;
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(fullWidth), GIF_Y(fullHeight), 0));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_BLACK);
    /* The original takes the echoWidth for both coordinates of these border strips. */
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(100), GIF_Y(fullWidth + 99), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(fullWidth + 101), GIF_Y(fullWidth + 101), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(fullWidth + 99), GIF_Y(100), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(fullWidth + 101), GIF_Y(fullWidth + 101), 0));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(100), GIF_Y(100), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(fullWidth * 16, fullHeight * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(fullWidth + 100), GIF_Y(fullHeight + 100), 0));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(echoAlpha));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(40 / gifStateEchoDarkenDivisor));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_BLACK);
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(129), GIF_Y(113), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    echoWidth = fullWidth;
    echoHeight = fullHeight;
    switch (gifStateEchoPattern) {
    case 0:
        for (stripOrPass = 1; stripOrPass < 6; stripOrPass++) {
            int passScale = stripOrPass / 2;

            if (passScale == 0) {
                passScale = 1;
            }
            if (stripOrPass >= 3) {
                gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, 0, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, 0, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoOffsetStep * passScale, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoOffsetStep * passScale, echoAlpha);
            }
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, -gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, -gifStateEchoOffsetStep * passScale, echoAlpha);
            echoWidth = fullWidth / stripOrPass;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / stripOrPass;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            if (stripOrPass + 1 < 6) {
                gifStateRestorePageBorder(packet, echoWidth, echoHeight);
            }
        }
        break;
    case 1:
        for (stripOrPass = 1; stripOrPass < gifStateEchoPassCount; stripOrPass++) {
            int passScale = stripOrPass / 2;

            if (passScale == 0) {
                passScale = 1;
            }
            if (stripOrPass >= 3) {
                gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, 0, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, 0, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoOffsetStep * passScale, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoOffsetStep * passScale, echoAlpha);
            }
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, -gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, -gifStateEchoOffsetStep * passScale, echoAlpha);
            echoWidth = fullWidth / stripOrPass;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / stripOrPass;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            if (stripOrPass + 1 < gifStateEchoPassCount) {
                gifStateRestorePageBorder(packet, echoWidth, echoHeight);
            }
        }
        break;
    case 2:
        for (stripOrPass = 1; stripOrPass < gifStateEchoPassCount; stripOrPass++) {
            int passScale = stripOrPass / 2;

            if (passScale == 0) {
                passScale = 1;
            }
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, -gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, -gifStateEchoOffsetStep * passScale, echoAlpha);
            echoWidth = fullWidth / stripOrPass;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / stripOrPass;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            if (stripOrPass + 1 < gifStateEchoPassCount) {
                gifStateRestorePageBorder(packet, echoWidth, echoHeight);
            }
        }
        break;
    case 3:
        for (stripOrPass = 1; stripOrPass < gifStateEchoPassCount; stripOrPass++) {
            int passScale = stripOrPass / 2;

            if (passScale == 0) {
                passScale = 1;
            }
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, 0, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, 0, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, 0, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoOffsetStep * passScale, 0, echoAlpha);
            echoWidth = fullWidth / stripOrPass;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / stripOrPass;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            if (stripOrPass + 1 < gifStateEchoPassCount) {
                gifStateRestorePageBorder(packet, echoWidth, echoHeight);
            }
        }
        break;
    case 4:
        for (stripOrPass = 1; stripOrPass < gifStateEchoPassCount; stripOrPass++) {
            int passScale = stripOrPass / 2;

            if (passScale == 0) {
                passScale = 1;
            }
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoOffsetStep * passScale, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoOffsetStep * passScale, echoAlpha);
            echoWidth = fullWidth / stripOrPass;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / stripOrPass;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            if (stripOrPass + 1 < gifStateEchoPassCount) {
                gifStateRestorePageBorder(packet, echoWidth, echoHeight);
            }
        }
        break;
    case 5:
        for (stripOrPass = 1; stripOrPass < gifStateEchoPassCount; stripOrPass++) {
            int passScale = stripOrPass / 2;

            if (passScale == 0) {
                passScale = 1;
            }
            gifStateDrawPageQuadInset(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, 2.0f * echoAlpha);
            gifStateDrawPageQuadInset(packet, echoWidth, echoHeight, gifStateEchoOffsetStep * passScale, gifStateEchoOffsetStep * passScale, 2.0f * echoAlpha);
            echoWidth = fullWidth / stripOrPass;
            if (echoWidth < 128) {
                echoWidth = 128;
            }
            echoHeight = fullHeight / stripOrPass;
            if (echoHeight < 112) {
                echoHeight = 112;
            }
            if (stripOrPass + 1 < gifStateEchoPassCount) {
                gifStateRestorePageBorder(packet, echoWidth, echoHeight);
            }
        }
        break;
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
                                                camera->screen.window.vector.y, camera->screen.window.vector.w));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, GIF_QUARTER_WIDTH, 0, GIF_QUARTER_HEIGHT));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_SCREEN));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(camera->screen.window.vector.x), GIF_Y(camera->screen.window.vector.y), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(camera->screen.window.vector.z), GIF_Y(camera->screen.window.vector.w), 0));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_SCREEN));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, GIF_STATE_WIDTH, 0, GIF_STATE_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_002561E8);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00256750);

/* Offset step of the copies drawn by gifStateDrawCustomEchoBlur (no config name yet: splat name). */
extern int gifStateEchoPassOffsetStep;

/* Echo blur of the camera window with explicit parameters (the counterpart of gifStateDrawEchoBlur): shrinks the window
 * into the corner of another page, darkens it, then draws the copy over itself echoPassCount - 1 times with
 * offsets growing by gifStateEchoPassOffsetStep per pass (the shifts picked by echoPattern), shrinking it again after every pass,
 * and finally stretches the result back over the window. */
void gifStateDrawCustomEchoBlur(XglPacket *packet, int echoPattern, int echoAlpha, int echoPassCount)
{
    XglCamera *camera = xglStudioSelectGetActiveCamera(0);
    int fullWidth = (camera->screen.window.vector.z - camera->screen.window.vector.x) / GIF_STATE_WIDTH * 256.0f;
    int fullHeight = (camera->screen.window.vector.w - camera->screen.window.vector.y) * (1.0f / GIF_STATE_HEIGHT) * 224.0f;
    int echoWidth;
    int echoHeight;
    int passIndex;

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_TEXFLUSH, 0);
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(1, 1, 0, 0, 0, 0));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_SCREEN));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.x * 16.0f, camera->screen.window.vector.y * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(camera->screen.window.vector.z * 16.0f, camera->screen.window.vector.w * 16.0f));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(fullWidth), GIF_Y(fullHeight), 0));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(echoAlpha));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(40 / gifStateEchoDarkenDivisor));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_GLOW_SOURCE));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_BLACK);
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(148), GIF_Y(132), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);

    echoWidth = fullWidth;
        echoHeight = fullHeight;
    switch (echoPattern) {
    case 0:
        for (passIndex = 1; passIndex < echoPassCount; passIndex++) {
            if (passIndex >= 3) {
                gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, 0, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, 0, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
                gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            }
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            echoWidth = fullWidth / passIndex;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / passIndex;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
            gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
            gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
            gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(echoWidth), GIF_Y(echoHeight), 0));
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(echoAlpha));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
        }
        break;
    case 1:
        for (passIndex = 1; passIndex < echoPassCount; passIndex++) {
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            echoWidth = fullWidth / passIndex;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / passIndex;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
            gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
            gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
            gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(echoWidth), GIF_Y(echoHeight), 0));
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(echoAlpha));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
        }
        break;
    case 2:
        for (passIndex = 1; passIndex < echoPassCount; passIndex++) {
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, 0, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, 0, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, gifStateEchoPassOffsetStep * passIndex, 0, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, -gifStateEchoPassOffsetStep * passIndex, 0, echoAlpha);
            echoWidth = fullWidth / passIndex;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / passIndex;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
            gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
            gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
            gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(echoWidth), GIF_Y(echoHeight), 0));
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(echoAlpha));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
        }
        break;
    case 3:
        for (passIndex = 1; passIndex < echoPassCount; passIndex++) {
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            gifStateDrawQuad(packet, echoWidth, echoHeight, 0, -gifStateEchoPassOffsetStep * passIndex, echoAlpha);
            echoWidth = fullWidth / passIndex;
            if (echoWidth < 64) {
                echoWidth = 64;
            }
            echoHeight = fullHeight / passIndex;
            if (echoHeight < 56) {
                echoHeight = 56;
            }
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateReset();
            gifStateAddReg(GS_TEXFLUSH, 0);
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
            gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_GLOW_SOURCE));
            gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
            gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
            gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
            gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
            gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
            gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(echoWidth), GIF_Y(echoHeight), 0));
            gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(echoAlpha));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
        }
        break;
    }

    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    gifStateReset();
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(camera->screen.window.vector.x, camera->screen.window.vector.z,
                                                camera->screen.window.vector.y, camera->screen.window.vector.w));
    gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(2, 2, 0, GIF_QUARTER_WIDTH, 0, GIF_QUARTER_HEIGHT));
    gifStateAddReg(GS_ALPHA_1, GIF_BLEND_FIXED_ALPHA(0x80));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_SCREEN));
    gifStateAddReg(GS_TEX0_1, GIF_PAGE_TEX0(GIF_PAGE_OUTLINE));
    gifStateAddReg(GS_RGBAQ, GIF_RGBAQ_NEUTRAL);
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(0, 0));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(camera->screen.window.vector.x), GIF_Y(camera->screen.window.vector.y), 0));
    gifStateAddReg(GS_UV, GS_SET_UV(GIF_QUARTER_WIDTH * 16, GIF_QUARTER_HEIGHT * 16));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(camera->screen.window.vector.z), GIF_Y(camera->screen.window.vector.w), 0));
    gifStateAddReg(GS_FRAME_1, GIF_PAGE_FRAME(GIF_PAGE_SCREEN));
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(0, GIF_STATE_WIDTH, 0, GIF_STATE_HEIGHT));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00257CF8);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_002580A0);

/* Empties the queue and writes the tag header: one register per loop, A+D descriptor. */
void gifStateReset(void) {
    gifStateNumPacket = 0;
    gifStatePacket[0].rawBits[0] = GIF_TAG_EOP;
    gifStatePacket[0].rawBits[1] = GIF_TAG_NREG_1;
    gifStatePacket[0].rawBits[2] = GIF_AD;
    gifStatePacket[0].rawBits[3] = 0;
}

/* Queues the A+D pair (gifAddress, gifData) as the next quadword of gifStatePacket (asserts while the
 * packet is not full). */
void gifStateAddReg(int gifAddress, u64 gifData)
{
    if (gifStateNumPacket >= STATEPACKET_MAX - 1) {
        assertFailed(gifStatePacketAssert, gifStateSourceFile, 37);
    }
    gifStateNumPacket++;
    gifStatePacket[gifStateNumPacket].adPair[0] = gifData;
    gifStatePacket[gifStateNumPacket].adPair[1] = gifAddress;
}

/* Queues four raw words as one quadword of gifStatePacket (asserts while the packet is not full). */
void gifStateAddRawQword(u32 qwordLowBits, u32 qwordLowMidBits, u32 qwordHighMidBits, u32 qwordHighBits)
{
    if (gifStateNumPacket >= STATEPACKET_MAX - 1) {
        assertFailed(gifStatePacketAssert, gifStateSourceFile, 50);
    }
    gifStateNumPacket++;
    gifStatePacket[gifStateNumPacket].rawBits[0] = qwordLowBits;
    gifStatePacket[gifStateNumPacket].rawBits[1] = qwordLowMidBits;
    gifStatePacket[gifStateNumPacket].rawBits[2] = qwordHighMidBits;
    gifStatePacket[gifStateNumPacket].rawBits[3] = qwordHighBits;
}

/* Sends the queued pairs as one direct VIF transfer of count+1 quadwords (tag + pairs). */
void gifStateFlush(void) {
    XglPacket *packet;
    int queuedPairs;
    int tagWord;
    unsigned int transferBytes;

    packet = xglPacketGetCurrent();
    queuedPairs = gifStateNumPacket;
    tagWord = queuedPairs | GIF_TAG_EOP;
    transferBytes = (queuedPairs * 0x10) + 0x10;
    if (queuedPairs != 0) {
        gifStatePacket[0].rawBits[0] = tagWord;
        sceVif1PkAddDirectDataN(packet, gifStatePacket, transferBytes >> 4);
        gifStateNumPacket = 0;
    }
}

/* GS TEXFLUSH with no data. */
void gifStateTexFlush(void) {
    gifStateAddReg(GS_TEXFLUSH, 0);
}

/* GS ALPHA_1: (Cs - Cd) * FIX + Cd with a fixed alpha. */
void gifStateSetAlphaFix(int fixedAlpha) {
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, fixedAlpha));
}

/* Sets the drawing scissor rectangle (SCISSOR_1). */
void gifStateSetScissor(int minX, int minY, int maxX, int maxY)
{
    gifStateAddReg(GS_SCISSOR_1, GS_SET_SCISSOR(minX, maxX, minY, maxY));
}

/* GS ZBUF_1 at page 0 with the Z format code 0x31 in the PSM field. */
void gifStateSetZBufFormat(void) {
    gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, 0x31, 0));
}

extern int D_00550E30;

void gifStateEnableFlag(void) {
    D_00550E30 = 1;
}

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_002586A8);
