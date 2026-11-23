#include "common.h"
#include "gs.h"
#include "main/xgl_2.h"

typedef int s32;
/* The draw-environment drawRecord pool (struct completed below with the xgl filter code). */
extern struct DrawEnvRecord gDrawArea;
void drawAreaSetWindow(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

struct DrawEnvRecord *drawEnvGetRecord(void) {
    return &gDrawArea;
}

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D4C40);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5398);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D53E0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5440);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D54C8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5528);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5588);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D55E8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5630);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5678);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D56D8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5738);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5798);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D57E0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5840);

void func_001D5398();

void drawEnvSetLarge(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x200, 0x1C0);
    func_001D5398();
}

void func_001D53E0();

void drawEnvSetHalfDraw(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D53E0();
}

s32 xglRenderMovieIsActive();
s32 func_001D5440();

void drawEnvSetField(void) {
    if (xglRenderMovieIsActive() != 0) {
        drawAreaSetWindow(0x80, 3, 0, 0, 0x280, 0x1BF, 0, 0, 0x200, 0x1BF);
    } else {
        drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1BF, 0, 0, 0x200, 0x1BF);
    }
    func_001D5440();
}

void func_001D56D8();

void drawEnvSetHalfDisplay(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D56D8();
}

void func_001D5738();

void drawEnvSetHalfDisplayNext(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x100, 0xE0, 0, 0, 0x200, 0x1C0);
    func_001D5738();
}

void func_001D54C8();

void drawEnvSetHalfDrawNext(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D54C8();
}

void func_001D5528();

void drawEnvSetHalfDrawLast(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D5528();
}

void drawEnvSetHalfDrawRepeat(void) {
    drawAreaSetWindow(0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0, 0x100, 0xE0);
    func_001D5528();
}

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5B28);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5CD0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5E60);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D5FF8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D61A8);

typedef long long s64;
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
    u8 unk_00[0x3C];
    s32 modeId;
} XglObject;

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
 do { packetCursor[1] = highDoubleword; gPacketCursor = nextPacketCursor; gPacketEntryCount = nextPacketEntryCount; gTagQuadCount = tagQuadCountStore; } while (0);
}

void xglPacketFinishTag(void) {
    extern s32 *gPacketLastTag;
    *gPacketLastTag |= gTagQuadCount;
}

s32 sceVif1PkAddDirectDataN(s32 packet, s32 address, s32 qword_count);
s32 sceVif1PkCloseDirectHLCode(s32 packet);
s32 sceVif1PkCnt(s32 packet, s32 count);
s32 sceVif1PkOpenDirectHLCode(s32 packet, s32 flags);
s32 xglPacketGetCurrent();

void xglPacketSendDirect(s32 address, s32 qwordCount) {
    s32 packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenDirectHLCode(packet, 0);
    sceVif1PkAddDirectDataN(packet, (address != 0) ? address : gPacketBufferBase, (qwordCount != 0) ? qwordCount : gPacketEntryCount);
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


void xglPacketBufferInit(s32 spr_address);
void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3);
void xglPacketAppendQuad(s64 highDoubleword, s64 lowDoubleword);
void xglPacketFinishTag(void);
void xglPacketSendDirect(s32 address, s32 qwordCount);

void xglPacketSendFixedHeader(void)
{
  xglPacketBufferInit(0x70000000);
  xglPacketAppendEntry(0x8000, 0x10000000, 0xE, 0);
  xglPacketAppendQuad(0x40, (((0x01BF0000L << 0x10) | 0x1FF) << 0x10));
  xglPacketFinishTag();
  xglPacketSendDirect(0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D66C0);

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

extern s64 gSbStudioBlock[];

/* True when the studio block says the filter passes must be skipped (flag 1 set with word 6 clear) or the offset is within limits. */
s32 xglFilterShouldSkip(void)
{
    s32 skipFilter;

    skipFilter = 0;
    if (((s32)(gSbStudioBlock[0] & 1) && ((s32 *)gSbStudioBlock)[6] == 0) || xglIsOffsetWithinLimits()) {
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

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D6928);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D7180);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D7440);

/* The frame and depth buffer pages of xgl_render.c's state block (D_004C8390), as this file reads them. */
typedef struct XglDrawBuffers {
    u8 unk00[8];
    u16 depthBase;   /* +0x08: ZBUF page */
    u8 unk0A[4];
    u16 workBase;    /* +0x0E: page of the work texture */
    u8 unk10[2];
    u16 frameBase;   /* +0x12: page of the frame being drawn */
    u8 unk14[2];
    u16 displayBase; /* +0x16: page of the displayed frame */
} XglDrawBuffers;

extern XglDrawBuffers D_004C8390;

/* The screen filter settings passed to the filter passes. */
typedef struct ScreenFilter {
    u8 unk00[4];
    s32 hue;          /* +0x04: degrees */
    s32 saturation;   /* +0x08 */
    s32 value;        /* +0x0C */
    s32 blendMode;    /* +0x10 */
    s32 blendAmount;  /* +0x14 */
    u8 unk18[0x50 - 0x18];
    u32 unk50_0 : 2;
    u32 additiveBlend : 1; /* +0x50 bit 2 */
} ScreenFilter;

/* One draw environment drawRecord of the frame copy queue. */
typedef struct DrawEnvRecord {
    s32 workBase;
    s32 frameBase;
    u8 unk08[0xC];
    s32 blendMode;
    s32 blendAmount;
    s32 sourceRect[4];
    s32 destinationRect[4];
    s32 blockWidth;
    s32 blockHeight;
    s32 unk44;
    s32 unk48;
} DrawEnvRecord;

/* VIF MSCAL: start the VU1 microprogram at the given address (in 8-byte units). */
#define XGL_VIF_MSCAL(address) (0x14000000 | ((u32) (address) >> 3))
/* VIF FLUSHA. */
#define XGL_VIF_FLUSHA 0x11000000
/* IEEE-754 bits of 1.0f, the Q (perspective) word of RGBAQ. */
#define XGL_FLOAT_ONE_BITS 0x3F800000
/* Scratchpad start of the packet buffer. */
#define XGL_SCRATCHPAD 0x70000000

extern s32 D_005166D0[];
extern u8 D_005166D4[];
extern u8 D_00000000[];
extern u8 D_40[];

void sceVif1PkRef(s32 packet, void *data, s32 qwordCount, s32 opt1, s32 opt2, s32 irq);
void sceVif1PkOpenUpkCode(s32 packet, s32 vuAddress, s32 unpackCommand, s32 cycleLength, s32 writeLength);
void sceVif1PkAddUpkData128N(s32 packet, s32 address, s32 qwordCount);
void sceVif1PkCloseUpkCode(s32 packet);
void sceVif1PkAddCode(s32 packet, s32 code);
void drawEnvSetLarge(void);
DrawEnvRecord *drawEnvGetRecord(void);
XglCamera *xglStudioSelectGetActiveCamera(int studioIndex);
void func_001D4C40(void);

/* Queues one frame copy drawRecord: the work and frame pages, the blend, and the source and destination
 * rectangles of the copy in blocks of blockWidth x blockHeight. */
static inline void xglFilterQueueCopy(s32 workBase, s32 frameBase, s32 blendMode, s32 blendAmount,
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
    drawRecord->blendMode = blendMode;
    drawRecord->blendAmount = blendAmount;
    drawRecord->sourceRect[0] = sourceX0;
    drawRecord->sourceRect[1] = sourceY0;
    drawRecord->sourceRect[2] = 0x200;
    drawRecord->sourceRect[3] = sourceY1;
    drawRecord->destinationRect[0] = destinationX0;
    drawRecord->destinationRect[1] = destinationY0;
    drawRecord->destinationRect[2] = destinationX1;
    drawRecord->destinationRect[3] = destinationY1;
    func_001D4C40();
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

/* Uploads the colour filter: a hue-tinted gray ramp CLUT, then draws the frame through it with the VU1 sprite program. */
void xglColorFilterApplyClut(ScreenFilter *filter, s32 vifPacket)
{
    s32 hue;
    u8 saturation;
    u8 maxChannel;
    u8 blendMode;
    s32 blendAmount;
    s32 red;
    s32 green;
    s32 blue;
    s32 clutIndex;

    hue = filter->hue % 360;
    saturation = filter->saturation;
    maxChannel = filter->value;
    blendMode = filter->blendMode;
    blendAmount = filter->blendAmount;
    if (xglFilterShouldSkip()) {
        return;
    }
    colorHsvToRgb(hue, saturation, maxChannel, &red, &green, &blue);
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
            xglPacketAppendWordQuad(clutIndex * 0x01010101, (clutIndex + 1) * 0x01010101, (clutIndex + 2) * 0x01010101, (clutIndex + 3) * 0x01010101);
            xglPacketAppendWordQuad((clutIndex + 4) * 0x01010101, (clutIndex + 5) * 0x01010101, (clutIndex + 6) * 0x01010101, (clutIndex + 7) * 0x01010101);
            break;
        case 0x10:
            xglPacketAppendWordQuad((clutIndex - 8) * 0x01010101, (clutIndex - 7) * 0x01010101, (clutIndex - 6) * 0x01010101, (clutIndex - 5) * 0x01010101);
            xglPacketAppendWordQuad((clutIndex - 4) * 0x01010101, (clutIndex - 3) * 0x01010101, (clutIndex - 2) * 0x01010101, (clutIndex - 1) * 0x01010101);
            break;
        case 0x08:
            xglPacketAppendWordQuad((clutIndex + 8) * 0x01010101, (clutIndex + 9) * 0x01010101, (clutIndex + 10) * 0x01010101, (clutIndex + 11) * 0x01010101);
            xglPacketAppendWordQuad((clutIndex + 12) * 0x01010101, (clutIndex + 13) * 0x01010101, (clutIndex + 14) * 0x01010101, (clutIndex + 15) * 0x01010101);
            break;
        }
    }
    xglPacketAppendEntry(GIF_TAG_EOP | 2, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.displayBase * 32, 8, 0x1B, 9, 9, 1, 1, D_004C8390.frameBase * 32, 0, 0, 0, 1));
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
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.frameBase * 32, 8, 0x1B, 9, 9, 1, 1, 0, 0, 0, 0, 0));
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
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.frameBase * 32, 16, 0x13, 10, 10, 1, 0, 0, 0, 0, 0, 0));
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
    xglFilterQueueCopy(D_004C8390.workBase, D_004C8390.frameBase, blendMode, blendAmount,
                       8, 8, 0, 0, 0xE0, 0, 0, 0x200, 0xE0);
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
    xglFilterQueueCopy(D_004C8390.workBase, D_004C8390.frameBase, blendMode, blendAmount,
                       8, 8, 0, 0xE0, 0x1C0, 0, 0, 0x200, 0xE0);
    xglFilterDrawSprite(filter, 0x7FF7, 0x8DF7);
}


INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D87F8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D8D38);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001D9360);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DA050);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DA858);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DAC58);

s32 func_001DB658();

void xglObjectSetModeOne(XglObject *object) {
    object->modeId = 1;
    func_001DB658();
}

void xglObjectSetModeTwo(XglObject *object) {
    object->modeId = 2;
    func_001DB658();
}

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DB658);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DC0F0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DC710);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DCCF0);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DCF70);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DD998);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DE530);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DE9F8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DEA48);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DEAC8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001DF7D8);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001E0630);

INCLUDE_ASM("asm/main/nonmatchings/tu108", func_001E0E10);

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
    drawRecord->blendMode = 0x80;
    drawRecord->blendAmount = 3;
    drawRecord->sourceRect[0] = left;
    drawRecord->sourceRect[1] = top;
    drawRecord->sourceRect[2] = right;
    drawRecord->sourceRect[3] = bottom;
    drawRecord->destinationRect[0] = left;
    drawRecord->destinationRect[1] = 0;
    drawRecord->destinationRect[2] = right;
    drawRecord->destinationRect[3] = 0xE0;
    func_001D4C40();
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

/* Applies the colour filter (brightness, contrast, tint) to the frame with GS direct packets; with the composite flag it
 * works on the work page and blends the result onto the displayed page. */
void xglColorFilterApply(ColorFilter *filter)
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

