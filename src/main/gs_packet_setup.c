#include "common.h"
#include "main/ee_stdarg.h"
#include "shared.h"
#include "gs.h"

void xglPacketScratchAddQuad(long long quadwordHigh, long long quadwordLow);

#include "main/xgl_render.h"

/* The batch: A+D pairs, the first one carries the GIF tag. */
extern GsAdPair *xglScratchQuads;
extern int xglScratchBatchQuadCount;

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264280);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264368);

typedef struct SRenderPages {
    unsigned char unmodeled_00[0x0e];
    unsigned short flip_base;           /* +0x18 of sRender */
    unsigned short unmodeled_10;
    unsigned short secondFrameBase;         /* +0x1C */
    unsigned short unmodeled_14;
    unsigned short display_buffer_base; /* +0x20 */
    unsigned short draw_buffer_base;    /* +0x22 */
} SRenderPages;
typedef struct SRenderState {
    unsigned char unmodeled_00[0x0a];
    SRenderPages pages;                 /* +0x0A */
} SRenderState;
typedef int s32;
typedef signed char s8;
typedef long long s64;
extern unsigned char D_0055A850[];
void *memset(void *dest, s32 value, u32 size);

void gsFlagBlockAClear(void) {
    memset(D_0055A850, 0, 8);
}

void gsFlagBlockASet(s32 enabled) {
    D_0055A850[0] = enabled;
}

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_002643D0);

extern unsigned char D_0055A858[];

void gsFlagBlockBClear(void) {
    memset(D_0055A858, 0, 8);
}

void gsFlagBlockBSet(s32 enabled) {
    D_0055A858[0] = enabled;
}

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264690);

/* The render page bases of sRender (0x004C8386, owned outside this TU): only the halfwords this TU reads are modeled.
 * The layout is the X3 one (flip +0x18, second +0x1C), which the X2 copies of the struct do not use. */















extern SRenderState sRender;

/* GS TEX0 of a 512x512 PSMCT32 frame page (TBW 8, TW and TH 9): framePage 0 is the display page with CBP set to
 * secondFrameBase, framePage 1 the draw page, framePage 2 the flip page; tex_cc goes to TCC, TFX stays 0. */
u64 gsFramePageTexture(int framePage, int tex_cc)
{
    switch (framePage) {
    case 0: {
        SRenderPages *pages = &sRender.pages;

        return GS_SET_TEX0(pages->display_buffer_base << 5, 8, GS_PSMCT32, 9, 9, tex_cc, 0, pages->secondFrameBase, 0, 0, 0, 0);
    }
    case 1:
        return GS_SET_TEX0(sRender.pages.draw_buffer_base << 5, 8, GS_PSMCT32, 9, 9, tex_cc, 0, 0, 0, 0, 0, 0);
    case 2:
        return GS_SET_TEX0(sRender.pages.flip_base << 5, 8, GS_PSMCT32, 9, 9, tex_cc, 0, 0, 0, 0, 0, 0);
    default:
        return 0;
    }
}

/* GS FRAME register value of a 512-pixel-wide (FBW 8) frame page: framePage 0 the display page, 1 the draw page, 2 the flip
 * page; unusedFlag is not read. */
u64 gsMakeFrame(int framePage, int unusedFlag)
{
    switch (framePage) {
    case 0:
        return sRender.pages.display_buffer_base | 0x80000;
    case 1:
        return sRender.pages.draw_buffer_base | 0x80000;
    case 2:
        return sRender.pages.flip_base | 0x80000;
    default:
        return 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264898);

extern unsigned char xglFontDebugTextFormat[];
char *func_0030C440();
s32 func_0030C498();
s32 strlen();
s32 vsprintf();
s32 xglFontDebugPrintf();

void xglFontDebugPrintAt(s32 x, s32 y, const char *format, ...)
{
  s32 formatLength;
  char *text;
  s32 pixelX;
  s32 pixelY;
  pixelX = x;
  pixelY = y;
  formatLength = strlen(format);
  text = func_0030C440(formatLength + 10);
  vsprintf(text, format, ((va_list) __builtin_next_arg(format)) - ((8 - __builtin_args_info(2)) * 8));
  xglFontDebugPrintf(pixelX >> 1, pixelY >> 1, xglFontDebugTextFormat, text);
  func_0030C498(text);
}

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264A88);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264B68);

s32 countWordsUntilZero(s32 *zeroTermWords) {
    s32 *cursor;
    s32 zeroTermWordCount;

    cursor = zeroTermWords;
    zeroTermWordCount = 0;
    if (*cursor != 0) {
        do {
            cursor += 1;
            zeroTermWordCount += 1;
        } while (*cursor != 0);
    }
    return zeroTermWordCount;
}

struct XglPacket *xglPacketGetCurrent(void);
/* The scaffold word (splat name D_005513AC, gp-relative in .sbss): the packet the batch is added to. */
extern struct XglPacket *D_005513AC;

/* Starts a scratchpad batch: the current packet, the scratchpad base, a zero count, then the first quadword (the GIF tag,
 * A+D register 0x0E). */
void xglPacketScratchBegin(void)
{
    D_005513AC = xglPacketGetCurrent();
    xglScratchQuads = (GsAdPair *)0x70000000;
    xglScratchBatchQuadCount = 0;
    xglPacketScratchAddQuad(0xE, 0x1000000000008000LL);
}

s32 sceVif1PkAddDirectDataN(XglPacket *, GsAdPair *, s32);
s32 sceVif1PkCloseDirectHLCode(XglPacket *);
s32 sceVif1PkCnt(XglPacket *, s32, u64);
s32 sceVif1PkOpenDirectHLCode(XglPacket *, s32);

void xglPacketScratchFlush(void) {
    u64 gifTag;

    gifTag = xglScratchQuads->data + (xglScratchBatchQuadCount - 1);
    xglScratchQuads->data = gifTag;
    sceVif1PkCnt(D_005513AC, 0, gifTag);
    sceVif1PkOpenDirectHLCode(D_005513AC, 0);
    sceVif1PkAddDirectDataN(D_005513AC, xglScratchQuads, xglScratchBatchQuadCount);
    sceVif1PkCloseDirectHLCode(D_005513AC);
}

void xglPacketScratchGetBatch(GsAdPair **quadsOut, s32 **batchQuadCountOut) {
    GsAdPair *quads;

    quads = xglScratchQuads;
    *batchQuadCountOut = &xglScratchBatchQuadCount;
    *quadsOut = quads;
}

/* Appends one 128-bit quadword (quadwordHigh, quadwordLow) to the batch (proposed name xglPacketScratchAddQuad). */
void xglPacketScratchAddQuad(long long quadwordHigh, long long quadwordLow)
{
    int newBatchQuadCount;

    newBatchQuadCount = xglScratchBatchQuadCount + 1;
    xglScratchQuads[xglScratchBatchQuadCount].reg = quadwordHigh;
    xglScratchQuads[xglScratchBatchQuadCount].data = quadwordLow;
    xglScratchBatchQuadCount = newBatchQuadCount;
}
