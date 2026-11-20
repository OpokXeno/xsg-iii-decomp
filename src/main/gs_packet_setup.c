#include "common.h"
#include "shared.h"
#include "gs.h"

void xglPacketScratchAddQuad(long long high, long long low);

#include "main/xgl_render.h"

/* The batch: A+D pairs, the first one carries the GIF tag. */
extern GsAdPair *xglScratchQuads;
extern int xglScratchQuadCount;

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264280);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264368);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264398);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_002643C0);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_002643D0);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264658);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264680);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264690);

/* The render page bases of sRender (0x004C8386, owned outside this TU): only the halfwords this TU reads are modeled.
 * The layout is the X3 one (flip +0x18, second +0x1C), which the X2 copies of the struct do not use. */
typedef struct SRenderPages {
    unsigned char unmodeled_00[0x0e];
    unsigned short flip_base;           /* +0x18 of sRender */
    unsigned short unmodeled_10;
    unsigned short second_base;         /* +0x1C */
    unsigned short unmodeled_14;
    unsigned short display_buffer_base; /* +0x20 */
    unsigned short draw_buffer_base;    /* +0x22 */
} SRenderPages;

typedef struct SRenderState {
    unsigned char unmodeled_00[0x0a];
    SRenderPages pages;                 /* +0x0A */
} SRenderState;

extern SRenderState sRender;

/* GS TEX0 of a 512x512 PSMCT32 frame page (TBW 8, TW and TH 9): buffer 0 is the display page with CBP set to
 * second_base, buffer 1 the draw page, buffer 2 the flip page; tex_cc goes to TCC, TFX stays 0. */
u64 gsMakeTex0(int buffer, int tex_cc)
{
    switch (buffer) {
    case 0: {
        SRenderPages *pages = &sRender.pages;

        return GS_SET_TEX0(pages->display_buffer_base << 5, 8, GS_PSMCT32, 9, 9, tex_cc, 0, pages->second_base, 0, 0, 0, 0);
    }
    case 1:
        return GS_SET_TEX0(sRender.pages.draw_buffer_base << 5, 8, GS_PSMCT32, 9, 9, tex_cc, 0, 0, 0, 0, 0, 0);
    case 2:
        return GS_SET_TEX0(sRender.pages.flip_base << 5, 8, GS_PSMCT32, 9, 9, tex_cc, 0, 0, 0, 0, 0, 0);
    default:
        return 0;
    }
}

/* GS FRAME register value of a 512-pixel-wide (FBW 8) frame page: buffer 0 the display page, 1 the draw page, 2 the flip
 * page; flag is not read. */
u64 gsMakeFrame(int buffer, int flag)
{
    switch (buffer) {
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

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_002649D0);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264A88);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264B68);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264BA0);

struct XglPacket *xglPacketGetCurrent(void);
/* The scaffold word (splat name D_005513AC, gp-relative in .sbss): the packet the batch is added to. */
extern struct XglPacket *D_005513AC;

/* Starts a scratchpad batch: the current packet, the scratchpad base, a zero count, then the first quadword (the GIF tag,
 * A+D register 0x0E). */
void xglPacketScratchBegin(void)
{
    D_005513AC = xglPacketGetCurrent();
    xglScratchQuads = (GsAdPair *)0x70000000;
    xglScratchQuadCount = 0;
    xglPacketScratchAddQuad(0xE, 0x1000000000008000LL);
}

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264C18);

INCLUDE_ASM("asm/main/nonmatchings/gs_packet_setup", func_00264C70);

/* Appends one 128-bit quadword (high, low) to the batch (proposed name xglPacketScratchAddQuad). */
void xglPacketScratchAddQuad(long long high, long long low)
{
    int newCount;
    GsAdPair *slot;

    newCount = xglScratchQuadCount + 1;
    /* The offset is added to the base as an integer: that operand order gives the original register use. */
    slot = (GsAdPair *)(xglScratchQuadCount * sizeof(GsAdPair) + (int)xglScratchQuads);
    slot->reg = high;
    slot->data = low;
    xglScratchQuadCount = newCount;
}
