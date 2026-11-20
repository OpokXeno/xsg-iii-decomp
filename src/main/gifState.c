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

/* One quadword of gifStatePacket: a data/address pair (A+D) or four raw words (the tag). */
typedef union GifStateQword {
    u64 pair[2];                /* A+D: the data, then the register address */
    u32 word[4];
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
extern int gifStateNumPacket;
extern GifStateQword gifStatePacket[128];

#define GIF_TAG_NREG_1 0x10000000

/* The assert text and the file name of the packet queue asserts (no config name: splat names). */
extern const char D_00539B68[40];
extern const char D_00539B90[24];

/* The queue size the assert names: numPacket < (STATEPACKET_MAX - 1); the compare is against 63 in this build. */
#define STATEPACKET_MAX 0x40

/* Scaffold u16 words at 0x004C8390 (no config name): +0x0E the flip base, +0x16 the display base, +0x18 the draw
 * base. */
extern u16 D_004C8390[];

/* Writes the setup packet for the page each call (its frame and texture words), sends it, then queues the
 * context registers: clamp, frame, the alpha blend picked by the effect state, texture, test and z buffer. */
void gifStateSetupContext(XglPacket *packet, signed char alphaFix)
{
    gifStateSetupPacket[6].pair[0] = GS_SET_FRAME(D_004C8390[7], 8, GS_PSMCT32, 0);
    switch (gifStatus.fbp) {
    case 0:
        gifStateSetupPacket[7].pair[0] = GS_SET_TEX0(D_004C8390[11] << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0);
        break;
    case 1:
        gifStateSetupPacket[7].pair[0] = GS_SET_TEX0(D_004C8390[12] << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0);
        break;
    default:
        gifStateSetupPacket[7].pair[0] = GS_SET_TEX0(D_004C8390[11] << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0);
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
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(D_004C8390[11], 8, GS_PSMCT32, 0xFF000000));
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
            gifStateAddReg(GS_ALPHA_1, ((((long long) alphaFix) << 56) >> 24) | GS_SET_ALPHA(0, 2, 2, 1, 0));
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
    float step;
    float angle = 0.0f;
    float cosine;
    float sine;
    u32 *cursor;
    u32 *ring;
    u32 *gifTag;
    unsigned int i;
    int count;
    int innerCount;
    int sub3;
    int sub2;
    int sub1;
    int x;
    int y;
    int x2;
    int y2;
    int u;
    int v;

    step = 6.2831855f / gifStateRingCount;
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
    sub1 = gifStatus.subWindow[1];
    sub2 = gifStatus.subWindow[2];
    sub3 = gifStatus.subWindow[3];
    if (gifStatus.ratio < 0.0f) {
        gifStatus.ratio = 0.0f;
    }
    gifStateSetupContext(packet, 0);

    /* The outer ring: per step one point at the radius sub1 and one at sub1 + sub2 pulled in by the blur. */
    count = 0;
    cursor = (void *) (GIF_SCRATCHPAD + 0x10);
    angle = 0.0f;
    for (i = 0; i < 2 * gifStateRingCount; i += 2) {
        cosine = cosf(angle);
        sine = sinf(angle);
        x = cosine * sub1 + gifStatus.centerX;
        y = sine * sub1 + gifStatus.centerY;
        x2 = cosine * (sub1 + sub2) + gifStatus.centerX;
        y2 = sine * (sub1 + sub2) + gifStatus.centerY;
        angle += step;
        x = x < 0 ? 0 : x;
        y = y < 0 ? 0 : y;
        x = x >= GIF_STATE_WIDTH ? GIF_STATE_WIDTH - 1 : x;
        y = y >= GIF_STATE_HEIGHT ? GIF_STATE_HEIGHT - 1 : y;
        cursor[0] = 0x80;
        cursor[1] = 0x80;
        cursor[2] = 0x80;
        cursor[3] = ringAlpha;
        cursor[6] = 0;
        cursor[7] = 0;
        cursor[4] = x << 4;
        cursor[5] = y << 4;
        cursor[8] = GIF_X(x);
        cursor[9] = GIF_Y(y);
        cursor[10] = gifStatus.depth;
        cursor[11] = 0;
        cursor += 12;

        cursor[0] = 0x80;
        cursor[1] = 0x80;
        cursor[2] = 0x80;
        cursor[3] = trailAlpha;
        cosine *= gifStatus.blur;
        u = x2 - (int) cosine;
        v = y2 - (int) (sine * gifStatus.blur);
        v = v < 0 ? 0 : v;
        u = u < 0 ? 0 : u;
        v = v >= GIF_STATE_HEIGHT ? GIF_STATE_HEIGHT - 1 : v;
        u = u >= GIF_STATE_WIDTH ? GIF_STATE_WIDTH - 1 : u;
        cursor[6] = 0;
        cursor[7] = 0;
        cursor[5] = v << 4;
        cursor[4] = u << 4;
        cursor[8] = GIF_X(x2);
        cursor[9] = GIF_Y(y2);
        cursor[10] = gifStatus.depth;
        cursor[11] = 0;
        cursor += 12;
        count += 2;
    }
    count += 2;
    i = 0;
    innerCount = 0;
    gifTag = (void *) GIF_SCRATCHPAD;
    gifTag[0] = gifRingGifTag[0] | count;
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
    sceVif1PkAddDirectDataN(packet, (void *) GIF_SCRATCHPAD, count * 3 + 1);
    sceVif1PkCloseDirectHLCode(packet);

    ring = (void *) (GIF_SCRATCHPAD + 0x10);
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

    /* The inner ring: the outer points again at the radius sub1 + sub3, from the ratio-scaled start angle. */
    angle = step * gifStatus.ratio;
    ring = (void *) (GIF_SCRATCHPAD + 0x10);
    for (i = 0; i < 2 * gifStateRingCount; i += 2) {
        int innerX;
        int innerY;

        ring += 12;
        cosine = cosf(angle);
        sine = sinf(angle);
        cosine *= sub1 + sub3;
        cosine += gifStatus.centerX;
        innerY = sine * (sub1 + sub3) + gifStatus.centerY;
        innerX = cosine;
        angle += step;
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
        innerCount += 2;
    }
    innerCount += 2;
    gifTag = (void *) GIF_SCRATCHPAD;
    gifTag[0] = gifRingGifTag[0] | innerCount;
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
    sceVif1PkAddDirectDataN(packet, (void *) GIF_SCRATCHPAD, innerCount * 3 + 1);
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
extern float D_00550CF0;
/* The scaffold float cells the radial blur reads: 0.9 and 0.01. */
extern const float D_0054F6E4;
extern const float D_0054F6E8;
/* The per-edge scales of the radial blur (512 floats; the splat label ends before the table does). */
extern float D_005A6C60[];
/* The GIF tag template of the radial blur fan (the vertex count is or-ed in). */
extern u32 D_003EB740[4];

/* The fan's edge points are this far apart at least. */
#define GIF_BLUR_MIN_STEP 4

/* Draws the shown page again as a triangle fan from the effect centre to points around the screen edge, every
 * subWindow[0] pixels; each edge point samples the texture at a random scale between 0.9 and 1.0 of the ratio
 * towards the centre, so the picture blurs radially. The scales are drawn anew and the ratio lowered by its
 * step. Nothing is drawn while the ratio is 0. */
void gifStateDrawRadialBlur(XglPacket *packet)
{
    u32 *cursor;
    u32 *tag = 0;
    int k;
    int count;
    int step;
    int pos;
    float scale = 0.0f;

    if (gifStatus.ratio == scale) {
        return;
    }
    D_00550CF0 = gifStatus.ratio * D_0054F6E4;
    for (pos = 0; pos < 512; pos++) {
        D_005A6C60[pos] = (xglSRand() % 10U) * D_0054F6E8 + D_00550CF0;
    }
    k = 0;
    count = 1;
    pos = 0;
    gifStateSetupContext(packet, gifStatus.effectAlpha * gifStatus.ratio);
    step = gifStatus.subWindow[0];
    if (step < GIF_BLUR_MIN_STEP) {
        step = GIF_BLUR_MIN_STEP;
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
        scale = D_005A6C60[k++];
        cursor[0] = (gifStatus.centerX + (pos - gifStatus.centerX) * scale) * 16.0f;
        cursor[1] = (gifStatus.centerY + (0 - gifStatus.centerY) * scale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        count++;
        cursor[4] = ((gifStateOriginX + pos) << 4) - 8;
        pos += step;
        cursor[5] = ((gifStateOriginY + 0) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    } while (pos < GIF_STATE_WIDTH);
    for (pos = 0; pos < GIF_STATE_HEIGHT; pos += step) {
        scale = D_005A6C60[k++];
        cursor[0] = (gifStatus.centerX + ((GIF_STATE_WIDTH - 1) - gifStatus.centerX) * scale) * 16.0f;
        cursor[1] = (gifStatus.centerY + (pos - gifStatus.centerY) * scale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        count++;
        cursor[4] = ((gifStateOriginX + (GIF_STATE_WIDTH - 1)) << 4) - 8;
        cursor[5] = ((gifStateOriginY + pos) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    }
    for (pos = GIF_STATE_WIDTH - 1; pos >= 0; pos -= step) {
        scale = D_005A6C60[k++];
        cursor[0] = (gifStatus.centerX + (pos - gifStatus.centerX) * scale) * 16.0f;
        cursor[1] = (gifStatus.centerY + ((GIF_STATE_HEIGHT - 1) - gifStatus.centerY) * scale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        count++;
        cursor[4] = ((gifStateOriginX + pos) << 4) - 8;
        cursor[5] = ((gifStateOriginY + (GIF_STATE_HEIGHT - 1)) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    }
    for (pos = GIF_STATE_HEIGHT - 1; pos >= 0; pos -= step) {
        scale = D_005A6C60[k++];
        cursor[0] = (gifStatus.centerX + (0 - gifStatus.centerX) * scale) * 16.0f;
        cursor[1] = (gifStatus.centerY + (pos - gifStatus.centerY) * scale) * 16.0f;
        cursor[2] = 0;
        cursor[3] = 0;
        count++;
        cursor[4] = ((gifStateOriginX + 0) << 4) - 8;
        cursor[5] = ((gifStateOriginY + pos) << 4) - 9;
        cursor[6] = gifStatus.depth;
        cursor[7] = 0;
        cursor += 8;
    }
    count++;
    tag = GIF_PACKET->tag;
    tag[0] = D_003EB740[0] | count;
    tag[1] = D_003EB740[1];
    tag[2] = D_003EB740[2];
    tag[3] = D_003EB740[3];
    /* The fan closes on its first edge point: past the tag come vertex 0 (the centre) and vertex 1, whose
     * eight words go once more to the end. */
    tag += 4;
    cursor[0] = tag[8];
    cursor[1] = tag[9];
    cursor[2] = tag[10];
    cursor[3] = tag[11];
    cursor[4] = tag[12];
    cursor[5] = tag[13];
    cursor[6] = tag[14];
    cursor[7] = tag[15];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, GIF_PACKET, count * 2 + 1);
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
    Vector4 scaled;
    Vector4 offset;
    Matrix matrix;
    int corners[4][2];
    XglCamera *camera = xglStudioSelectGetActiveCamera(0);
    u32 *cursor;
    u32 *gifTag = 0;
    int x;
    int y;

    if (gifStatus.ratio <= 0.0f) {
        return;
    }
    tiVu0LoadIdentityMatrix();
    xglCurMulRotateY(camera->travel.rotation.y);
    xglCurMulRotateX(camera->travel.rotation.x);
    xglCurMulRotateZ(camera->travel.rotation.z);
    xglCurMatrixGet(matrix.rows);
    xglMatrixInverse(&matrix, &matrix);
    xglVectorScale(&scaled, &gifStatus.mulColor, gifStatus.blur);
    xglVectorMulMat(&offset, matrix.rows, &scaled);
    x = -offset.x;
    y = offset.y;
    corners[0][0] = x;
    corners[0][1] = y;
    corners[1][0] = x;
    corners[1][1] = y;
    corners[2][0] = x;
    corners[2][1] = y;
    corners[3][0] = x;
    corners[3][1] = y;
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
    int x;
    int y;
    int count = 0;
    int jitter;
    int leftColumn;
    u32 *tag = 0;
    u32 *rowTag = 0;

    if (gifStatus.ratio != 0.0f) {
        y = 0;
        x = y;
        count = x;
        cursor = GIF_PACKET->vertex[0].uv;
        gifStateSetupContext(packet, gifStatus.effectAlpha * gifStatus.ratio);
        if (gifStatus.blur == 0) {
            gifStatus.blur = 1;
        }
        do {
            if (x == 0) {
                jitter = 0;
            } else {
                jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
            }
            cursor[0] = (x + jitter) << 4;
            cursor[1] = y << 4;
            cursor[2] = 0;
            cursor[3] = 0;
            cursor[4] = ((gifStateOriginX + x) << 4) - 8;
            cursor[5] = ((gifStateOriginY + y) << 4) - 9;
            cursor[6] = gifStatus.depth;
            cursor[7] = 0;
            count++;
            cursor += 8;
            y += gifStateGridStep;
            leftColumn = x == 0;
            if (leftColumn) {
                jitter = x;
            } else {
                jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
            }
            cursor[0] = (x + jitter) << 4;
            cursor[1] = y << 4;
            cursor[2] = 0;
            cursor[3] = 0;
            cursor[4] = ((gifStateOriginX + x) << 4) - 8;
            cursor[5] = ((gifStateOriginY + y) << 4) - 9;
            cursor[6] = gifStatus.depth;
            cursor[7] = 0;
            count++;
            cursor += 8;
            y -= gifStateGridStep;
            x += gifStateGridStep;
        } while (x < GIF_STATE_WIDTH);
        tag = GIF_PACKET->tag;
        tag[0] = gifGridFirstRowTag[0] | count;
        tag[1] = gifGridFirstRowTag[1];
        tag[2] = gifGridFirstRowTag[2];
        tag[3] = gifGridFirstRowTag[3];
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        sceVif1PkAddDirectDataN(packet, GIF_PACKET, count * 2 + 1);
        sceVif1PkCloseDirectHLCode(packet);

        for (y += gifStateGridStep; y < GIF_STATE_HEIGHT; y += gifStateGridStep) {
            x = 0;
            count = 0;
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
                count++;
                cursor += 8;
                y += gifStateGridStep;
                if (x == 0) {
                    jitter = 0;
                } else {
                    jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
                }
                cursor[0] = (x + jitter) << 4;
                if (y == 0) {
                    jitter = 0;
                } else {
                    jitter = (int) ((float) (xglSRand() % gifStatus.blur) * gifStatus.ratio);
                }
                cursor[1] = (y + jitter) << 4;
                cursor[2] = 0;
                cursor[3] = 0;
                cursor[4] = ((gifStateOriginX + x) << 4) - 8;
                cursor[5] = ((gifStateOriginY + y) << 4) - 9;
                cursor[6] = gifStatus.depth;
                cursor[7] = 0;
                count++;
                cursor += 8;
                x += gifStateGridStep;
                y -= gifStateGridStep;
            } while (x < GIF_STATE_WIDTH);
            rowTag = GIF_PACKET->tag;
            rowTag[0] = gifGridRowTag[0] | count;
            rowTag[1] = gifGridRowTag[1];
            rowTag[2] = gifGridRowTag[2];
            rowTag[3] = gifGridRowTag[3];
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            sceVif1PkAddDirectDataN(packet, GIF_PACKET, count * 2 + 1);
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
    int count;

    if (gifStatus.ratio <= 0.0f) {
        return;
    }
    gifStateSetupContext(packet, gifStatus.ratio * gifStatus.effectAlpha);
    cursor = GIF_PACKET->vertex[0].uv;
    count = 0;
    cursor[0] = 0;
    cursor[1] = 0;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + gifStatus.subWindow[0] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + gifStatus.subWindow[1] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor += 8;
    count++;
    cursor[0] = (GIF_STATE_WIDTH - 1) << 4;
    cursor[1] = 0;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + (GIF_STATE_WIDTH - 1) + gifStatus.subWindow[0] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + gifStatus.subWindow[1] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor += 8;
    count++;
    cursor[0] = 0;
    cursor[1] = (GIF_STATE_HEIGHT - 1) << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + gifStatus.subWindow[2] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + (GIF_STATE_HEIGHT - 1) + gifStatus.subWindow[3] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    cursor += 8;
    count++;
    cursor[0] = (GIF_STATE_WIDTH - 1) << 4;
    cursor[1] = (GIF_STATE_HEIGHT - 1) << 4;
    cursor[2] = 0;
    cursor[3] = 0;
    cursor[4] = (gifStateOriginX + (GIF_STATE_WIDTH - 1) + gifStatus.subWindow[2] * gifStatus.ratio) * 16.0f - 8.0f;
    cursor[5] = (gifStateOriginY + (GIF_STATE_HEIGHT - 1) + gifStatus.subWindow[3] * gifStatus.ratio) * 16.0f - 9.0f;
    cursor[6] = gifStatus.depth;
    cursor[7] = 0;
    count++;
    gifTag = GIF_PACKET->tag;
    gifTag[0] = gifFadeQuadTag[0] | count;
    gifTag[1] = gifFadeQuadTag[1];
    gifTag[2] = gifFadeQuadTag[2];
    gifTag[3] = gifFadeQuadTag[3];
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, gifTag, count * 2 + 1);
    sceVif1PkCloseDirectHLCode(packet);
    gifStatus.ratio -= gifStatus.ratioStep;
}

void gifStateDrawNone(XglPacket *packet) {
}

/* Three small-data words the timing setters below write (no config name yet: splat names). */
extern int D_00550CFC;
extern int D_00550D00;
extern int D_00550D04;

void func_00254920(void)
{
    D_00550D00 = 90;
    D_00550D04 = 6;
    D_00550CFC = 0;
}

void func_00254938(int first, int second, int third)
{
    D_00550CFC = first;
    D_00550D00 = second;
    if (third <= 0) {
        D_00550D04 = 1;
    } else {
        D_00550D04 = third;
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
    gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, alpha));
    gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(D_004C8390[9], 8, GS_PSMCT32, 0xFF000000));
    gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(D_004C8390[7] << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 0));
    gifStateAddReg(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
    gifStateAddReg(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
    gifStateAddReg(GS_UV, GS_SET_UV(offsetX * 16 + 1600, offsetY * 16 + 1600));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(0), GIF_Y(0), 0));
    gifStateAddReg(GS_UV, GS_SET_UV((width + offsetX) * 16 + 1600, (height + offsetY) * 16 + 1600));
    gifStateAddReg(GS_XYZF2, GS_SET_XYZ(GIF_X(128), GIF_Y(112), 0));
    gifStateFlush();
    sceVif1PkCloseDirectHLCode(packet);
}

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00254B40);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00254D28);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00254F88);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_002561E8);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00256750);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00256CE8);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00257CF8);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_002580A0);

/* Empties the queue and writes the tag header: one register per loop, A+D descriptor. */
void gifStateReset(void) {
    gifStateNumPacket = 0;
    gifStatePacket[0].word[0] = GIF_TAG_EOP;
    gifStatePacket[0].word[1] = GIF_TAG_NREG_1;
    gifStatePacket[0].word[2] = GIF_AD;
    gifStatePacket[0].word[3] = 0;
}

/* Queues the A+D pair (gifAddress, gifData) as the next quadword of gifStatePacket (asserts while the
 * packet is not full). */
void gifStateAddReg(int gifAddress, u64 gifData)
{
    if ((unsigned int) gifStateNumPacket >= STATEPACKET_MAX - 1) {
        assertFailed(D_00539B68, D_00539B90, 37);
    }
    gifStateNumPacket++;
    gifStatePacket[gifStateNumPacket].pair[0] = gifData;
    gifStatePacket[gifStateNumPacket].pair[1] = gifAddress;
}

/* Queues four raw words as one quadword of gifStatePacket (asserts while the packet is not full). */
void gifStateAddRawQword(u32 word0, u32 word1, u32 word2, u32 word3)
{
    if ((unsigned int) gifStateNumPacket >= STATEPACKET_MAX - 1) {
        assertFailed(D_00539B68, D_00539B90, 50);
    }
    gifStateNumPacket++;
    gifStatePacket[gifStateNumPacket].word[0] = word0;
    gifStatePacket[gifStateNumPacket].word[1] = word1;
    gifStatePacket[gifStateNumPacket].word[2] = word2;
    gifStatePacket[gifStateNumPacket].word[3] = word3;
}

/* Sends the queued pairs as one direct VIF transfer of count+1 quadwords (tag + pairs). */
void gifStateFlush(void) {
    XglPacket *packet;
    int count;
    int tag;
    unsigned int byteCount;

    packet = xglPacketGetCurrent();
    count = gifStateNumPacket;
    tag = count | GIF_TAG_EOP;
    byteCount = (count * 0x10) + 0x10;
    if (count != 0) {
        gifStatePacket[0].word[0] = tag;
        sceVif1PkAddDirectDataN(packet, gifStatePacket, byteCount >> 4);
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

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_00258698);

INCLUDE_ASM("asm/main/nonmatchings/gifState", func_002586A8);
