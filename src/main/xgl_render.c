#include "common.h"
#include "shared.h"
/* The EE scratchpad (16 KB), where the GS queue and talk packets are built before they are sent. */
#define SCRATCHPAD_BASE 0x70000000

void soundApplySettings(void);
void xglDmaDirectNormal(unsigned int channel, unsigned int address, unsigned int count);

/* GS PMODE register: display circuit enables and the alpha blend value. */
typedef struct GsPmode {
    unsigned int firstCircuitEnable : 1;
    unsigned int secondCircuitEnable : 1;
    unsigned int crtmd : 3;
    unsigned int mmod : 1;
    unsigned int amod : 1;
    unsigned int slbg : 1;
    unsigned int alp : 8;
    unsigned int unmodeled_16 : 16;
    unsigned int unmodeled_32 : 32;
} GsPmode;

typedef struct GsDispfb {
    unsigned int fbp : 9;
    unsigned int fbw : 6;
    unsigned int psm : 5;
    unsigned int unmodeled_20 : 12;
    unsigned int dbx : 11;
    unsigned int dby : 11;
    unsigned int unmodeled_54 : 10;
} GsDispfb;

typedef struct GsDisplay {
    unsigned int dx : 12;
    unsigned int dy : 11;
    unsigned int magh : 4;
    unsigned int magv : 2;
    unsigned int unmodeled_29 : 3;
    unsigned int dw : 12;
    unsigned int dh : 11;
    unsigned int unmodeled_55 : 9;
} GsDisplay;

typedef union GsDispfbRegister {
    u64 packed;
    GsDispfb dispfbFields;
} GsDispfbRegister;

typedef union GsDisplayRegister {
    u64 packed;
    GsDisplay displayFields;
} GsDisplayRegister;

/* GS EXTDATA register: the sampling window of the external video input (EXTBUF is a plain 64-bit word). */
typedef struct GsExtdata {
    unsigned int sx : 12;
    unsigned int sy : 11;
    unsigned int smph : 4;
    unsigned int smpv : 2;
    unsigned int unmodeled_29 : 3;
    unsigned int ww : 12;
    unsigned int wh : 11;
    unsigned int unmodeled_55 : 9;
} GsExtdata;

typedef union GsExtdataRegister {
    u64 packed;
    GsExtdata extdataFields;
} GsExtdataRegister;

#include "main/Packet.h"
#include "main/xgl_render.h"

/*
 * GS clear-environment block (ClearEnv, 0x006457C0, size 0x60). xglRenderClearDepth
 * and xglRenderClearFrame each write only the 8-byte tag at +0x10 (a
 * GS-register-shaped value selecting what a queued clear targets);
 * xglRenderClearColor splits a packed 0xAABBGGRR colour into the four
 * one-byte-per-word fields at +0x20..+0x2c. Bytes no claimed function
 * touches stay unmodeled.
 */
typedef struct GsClearEnv {
    u32 gif_tag_low;
    u32 gif_tag_high;
    u32 gif_registers_low;
    u32 gif_registers_high;
    u64 clearAlphaDepthTest;
    u64 clearAlphaDepthTestAddress;
    u32 color_r;
    u32 color_g;
    u32 color_b;
    u32 color_a;
    u32 left;
    u32 top;
    unsigned char unmodeled_38[0x08];
    u32 right;
    u32 bottom;
    unsigned char unmodeled_48[0x08];
    u64 restoreAlphaDepthTest;
    u64 restoreAlphaDepthTestAddress;
} GsClearEnv;

typedef union GsClearPacket {
    GsClearEnv env;
    AlignedVector4 quadwords[6];
} GsClearPacket;

extern GsClearPacket ClearEnv;

/*
 * GS display-environment block (0x004A9000, size 0x50). xglRenderDispOff/On
 * only touch the 8-byte flag word at +0x00 (bit 0x2 gates display output).
 */
typedef struct GsDispEnv {
    union {
        u64 pmodeWord;
        GsPmode pmode;
    };
    unsigned char unmodeled_08[8];
    GsDispfbRegister firstDispfb;          /* +0x10 */
    GsDisplayRegister firstDisplay;        /* +0x18 */
    unsigned char unmodeled_20[8];
    GsDispfbRegister secondDispfb;          /* +0x28 */
    GsDisplayRegister secondDisplay;        /* +0x30 */
    u64 extbuf;                        /* +0x38 */
    GsExtdataRegister extdata;         /* +0x40 */
    u64 extwrite;                      /* +0x48 */
} GsDispEnv;

extern GsDispEnv DispEnv;
/* Small-data scalars of xgl_render.c (.sdata): the pxy tick scale (1 doubles the tick), the VSync interval, the render loop count and
 * the scan line interpolation mode. */
extern signed char pxyTickScaleDouble;
extern signed char xglRenderVSyncInterval;
extern int LoopCount;
extern int ScanLineInterpolate;
/* Glyph digit strokes of the pxy debug draw (.data after the hooks). */
extern unsigned char pxyGlyphDigitTable[252];
void xglRenderClearDepth(void);
void xglRenderClearFrame(void);
void xglRenderFrame(void);
struct XglPacket;
extern XglRenderState sRender;
void xglPacketMove(void);
void xglPacketSend(void);
XglPacket *xglPacketGetCurrent(void);
void xglPacketInterpolate(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkOpenDirectHLCode(XglPacket *packet, unsigned int count);
void sceVif1PkAddDirectDataN(XglPacket *packet, const void *data, unsigned int qwc);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);
void xglPadRead(void);
void xglFontFlush(void);
void gifStateUpdateDrawHook(void);
void nmlModelFlushClear(void);
void pxyBeginFrame(void);
void pxyFlush(void);
void xglRenderFrameBeginFlushEIrq(void);
void xglRenderFrameEndFlushEIrq(void);
void sefDrawEffect(void);

typedef unsigned int XglQuadword __attribute__((mode(TI)));

#define GIF_REG_RGBAQ 1
#define GIF_REG_AD 14
#define SCE_GS_TEST_1 0x47
#define XGL_DMA_CHANNEL_GIF 2
/* EE hardware registers (GS privileged, DMAC, VIF). */
#define GS_DISPFB1 0x12000070
#define GS_DISPLAY1 0x12000080
#define GS_EXTBUF 0x120000B0
#define GS_EXTDATA 0x120000C0
#define GS_EXTWRITE 0x120000D0
#define GS_CSR 0x12001000
/* A 64-bit GS privileged register, written through its address. */
#define GS_REG(address) (*(volatile u64 *) (address))
#define D1_CHCR ((volatile u32 *) 0x10009000)
#define D2_CHCR ((volatile u32 *) 0x1000A000)
#define D_ENABLEW ((volatile u32 *) 0x1000F590)
#define GS_TEST_CLEAR_DEPTH 0x00032001 /* alpha test never passes but writes Z only */
#define GS_TEST_CLEAR_FRAME 0x00030003
#define RENDER_CLEAR_QWORDS 6

/*
 * Packed GS registers of the draw-environment block below, laid out as
 * the GS manual defines them (bit positions from the first field).
 */
typedef struct GsFrame {
    u64 fbp : 9;
    u64 unmodeled_09 : 7;
    u64 fbw : 6;
    u64 unmodeled_22 : 2;
    u64 psm : 6;
    u64 unmodeled_30 : 2;
    u64 fbmsk : 32;
} GsFrame;

typedef struct GsZbuf {
    u64 zbp : 9;
    u64 unmodeled_09 : 15;
    u64 psm : 4;
    u64 unmodeled_28 : 4;
    u64 zmsk : 1;
    u64 unmodeled_33 : 31;
} GsZbuf;

typedef struct GsXyoffset {
    u64 ofx : 16;
    u64 unmodeled_16 : 16;
    u64 ofy : 16;
    u64 unmodeled_48 : 16;
} GsXyoffset;

typedef struct GsScissor {
    u64 scissorMinX : 11;
    u64 unmodeled_11 : 5;
    u64 scissorMaxX : 11;
    u64 unmodeled_27 : 5;
    u64 scissorMinY : 11;
    u64 unmodeled_43 : 5;
    u64 scissorMaxY : 11;
    u64 unmodeled_59 : 5;
} GsScissor;

typedef struct GsAlphaDepthTest {
    u64 ate : 1;
    u64 atst : 3;
    u64 aref : 8;
    u64 afail : 2;
    u64 date : 1;
    u64 datm : 1;
    u64 zte : 1;
    u64 ztst : 2;
    u64 unmodeled_19 : 45;
} GsAlphaDepthTest;

typedef struct GsTexa {
    u64 texelAlphaForZero : 8;
    u64 unmodeled_08 : 7;
    u64 aem : 1;
    u64 unmodeled_16 : 16;
    u64 texelAlphaForOne : 8;
    u64 unmodeled_40 : 24;
} GsTexa;

typedef struct GsDimx {
    u64 dm00 : 3;
    u64 unmodeled_03 : 1;
    u64 dm01 : 3;
    u64 unmodeled_07 : 1;
    u64 dm02 : 3;
    u64 unmodeled_11 : 1;
    u64 dm03 : 3;
    u64 unmodeled_15 : 1;
    u64 dm10 : 3;
    u64 unmodeled_19 : 1;
    u64 dm11 : 3;
    u64 unmodeled_23 : 1;
    u64 dm12 : 3;
    u64 unmodeled_27 : 1;
    u64 dm13 : 3;
    u64 unmodeled_31 : 1;
    u64 dm20 : 3;
    u64 unmodeled_35 : 1;
    u64 dm21 : 3;
    u64 unmodeled_39 : 1;
    u64 dm22 : 3;
    u64 unmodeled_43 : 1;
    u64 dm23 : 3;
    u64 unmodeled_47 : 1;
    u64 dm30 : 3;
    u64 unmodeled_51 : 1;
    u64 dm31 : 3;
    u64 unmodeled_55 : 1;
    u64 dm32 : 3;
    u64 unmodeled_59 : 1;
    u64 dm33 : 3;
    u64 unmodeled_63 : 1;
} GsDimx;

typedef struct GsPrmodecont {
    u64 ac : 1;
    u64 unmodeled_01 : 63;
} GsPrmodecont;

typedef struct GsColclamp {
    u64 clamp : 1;
    u64 unmodeled_01 : 63;
} GsColclamp;

typedef struct GsDthe {
    u64 dthe : 1;
    u64 unmodeled_01 : 63;
} GsDthe;

typedef struct GsGifRegisters {
    u64 firstRegisterDescriptor : 4;
    u64 unmodeled_04 : 60;
} GsGifRegisters;

typedef struct GsGifTagControl {
    u64 nloop : 15;
    u64 eop : 1;
    u64 unmodeled_16 : 30;
    u64 pre : 1;
    u64 prim : 11;
    u64 flg : 2;
    u64 nreg : 4;
} GsGifTagControl;

/*
 * GS draw-environment block (0x004A8F00, size 0x100): a GIF A+D packet of
 * fifteen register writes, each an 8-byte value followed by its register
 * address, as sceGsPutDrawEnv sends it.
 */
typedef struct GsDrawEnv {
    GsGifTagControl gif_tag;
    GsGifRegisters gif_registers;
    GsFrame firstFrame;
    u64 firstFrameAddress;
    GsZbuf firstZbuf;
    u64 firstZbufAddress;
    GsXyoffset firstXyoffset;
    u64 firstXyoffsetAddress;
    GsScissor firstScissor;
    u64 firstScissorAddress;
    GsAlphaDepthTest firstAlphaDepthTest;
    u64 firstAlphaDepthTestAddress;
    GsFrame secondFrame;
    u64 secondFrameAddress;
    GsZbuf secondZbuf;
    u64 secondZbufAddress;
    GsXyoffset secondXyoffset;
    u64 secondXyoffsetAddress;
    GsScissor secondScissor;
    u64 secondScissorAddress;
    GsAlphaDepthTest secondAlphaDepthTest;
    u64 secondAlphaDepthTestAddress;
    GsPrmodecont prmodecont;
    u64 prmodecont_address;
    GsTexa texa;
    u64 texa_address;
    GsColclamp colclamp;
    u64 colclamp_address;
    GsDthe dthe;
    u64 dthe_address;
    GsDimx dimx;
    u64 dimx_address;
} GsDrawEnv;

typedef union GsDrawPacket {
    GsDrawEnv env;
    XglQuadword quadwords[16];
} GsDrawPacket;

extern GsDrawPacket DrawEnv;
extern void SyncDCache(void *start, void *end);
extern void sceGsPutDrawEnv(GsDrawEnv *env);
extern int xglRenderGlobalFadeColor;
/* 0 by xglRenderCpUDispOff, 1 by xglRenderCpUDispOn */
extern int xglRenderClearSkip;
extern int xglRenderMoveCount;
/* The GS VSync interrupt callback increments this counter while
 * xglRenderSyncMove polls it on the render thread. */
extern volatile u32 VSyncCount;
extern s16 sRenderPsm;
extern u16 sRenderDisplayBase;
int FlushCache(int);
extern u16 sRenderFlipBase;
extern int xglRenderVSyncTickCount[];
void xglRenderSyncMove(void);
void xglRenderDrawEnvMove(void);
/*
 * dispEnvDispfb1..dispEnvExtwrite are fields of DispEnv (0x00645740, GsDispEnv above):
 * firstDispfb (+0x10), firstDisplay (+0x18), secondDispfb (+0x28), secondDisplay (+0x30),
 * extbuf (+0x38), extdata (+0x40), extwrite (+0x48). dispEnvTailWork lies just past
 * the 0x50 bytes of GsDispEnv.
 */
extern GsDispfbRegister dispEnvDispfb1;
extern GsDisplayRegister dispEnvDisplay1;
extern GsDispfbRegister dispEnvDispfb2;
extern GsDisplayRegister dispEnvDisplay2;
extern u64 dispEnvExtbuf;
extern u64 dispEnvExtdata;
extern u64 dispEnvExtwrite;
extern u64 dispEnvCacheEnd[];
extern u16 sRenderDispfbBase;
void sceGsPutDispEnv(GsDispEnv *env);
void xglRenderDispEnvMove(void);
void xglRenderClearEnvMove(void);
extern s16 sRenderPrevDispfbBase;
extern int xglRenderStatus[];
extern int xglRenderFrameDelta[];
extern int xglRenderFrameCount[];
void xglRenderSwapBase(void);
extern int sceGsResetGraph(int, int, int, int);
extern int xglRenderSkippedFrameCount[];
void sceGsResetPath(void);
void sceGsSyncV(int);
void xglRenderFinalPacket(void);
/* xglRenderGlobalFadeCallback: cleared by xglRenderGlobalFadeInit; referenced by xglRenderGlobalFade (the fade draw). */
extern int xglRenderGlobalFadeCallback[];
void xglRenderGlobalFade(void);
void xglRenderMove(void);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_00199CE0);

typedef struct XglRenderStateBlock {
    unsigned char unmodeled_00[2];
    short psm;                          /* +0x02 */
    unsigned char unmodeled_04[4];
    unsigned short zbuf_base;           /* +0x08: the Z-buffer page xglRenderDrawEnvMove copies into both zbuf fields */
    unsigned short dispfb_base;         /* +0x0A */
    unsigned short prev_dispfb_base;    /* +0x0C */
    unsigned char unmodeled_0e[0x67];   /* +0x0E..+0x74 */
    unsigned char frame_delta;          /* +0x75 */
    unsigned char unmodeled_76[0x0e];   /* +0x76..+0x83 */
    unsigned char syncBacklogFull;              /* +0x84 */
} XglRenderStateBlock;

typedef int s32;

typedef signed char s8;

extern volatile unsigned int gVsyncsSinceFlip;
extern unsigned char func_00199CE0[];
s32 func_002F47D8();

void xglRenderSyncInit(void) {
    func_002F47D8(func_00199CE0);
    gVsyncsSinceFlip = 0;
}

/* X3 scaffold label 0x004c8390: the fields xglRenderSyncWait reads and writes, at their asm offsets. */
extern XglRenderStateBlock D_004C8390;
extern signed char gVsyncsPerFrame;
/* Scaffold labels of the callees (config has no name for them). */
void func_001337B8(void);
void func_002F32B8(void);
void func_002F3118(int, int, int, int);
void func_002F38F0(int);
int sceGsSyncPath(int, int);

void xglRenderSyncWait(void)
{
    u64 *packet;
    int pathIdleSeen;
    int pollCount;
    int pathBusy;

    if ((unsigned int) gVsyncsPerFrame < gVsyncsSinceFlip) {
        D_004C8390.frame_delta = 1;
    } else {
        D_004C8390.frame_delta = 0;
    }

    pathIdleSeen = 0;
    pathBusy = 0;
    D_004C8390.syncBacklogFull = 0;
    for (pollCount = 0; pollCount < 0x80; pollCount++) {
        pathBusy = sceGsSyncPath(1, 0);
        if (pathBusy == 0) {
            if (gVsyncsSinceFlip >= 3 && pathIdleSeen == 0) {
                D_004C8390.syncBacklogFull = 1;
            }
            pathIdleSeen = 1;
            if (gVsyncsSinceFlip == gVsyncsPerFrame || (unsigned int) gVsyncsPerFrame < gVsyncsSinceFlip) {
                break;
            }
        }
        func_001337B8();
    }

    if (pathBusy != 0) {
        *D_ENABLEW = 0x10000;
        *D1_CHCR = 0;
        *D2_CHCR = 0;
        *D_ENABLEW = 0;
        func_002F32B8();
        func_002F3118(1, 0, 0, 0);
        sceGsSyncPath(0, 0);

        packet = (u64 *) SCRATCHPAD_BASE;
        packet[0] = 0x1000000000008006ULL;
        packet[1] = GIF_REG_AD;
        packet[2] = 0;
        packet[3] = 0x3F;
        packet[4] = ((D_004C8390.dispfb_base << 5) | 0x80000)
            | ((u64) (D_004C8390.prev_dispfb_base << 5) << 32) | (0x8000ULL << 36)
            | ((u64) D_004C8390.psm << 24) | ((u64) D_004C8390.psm << 56);
        packet[5] = 0x50;
        packet[6] = 0;
        packet[7] = 0x51;
        packet[8] = (0x1C0ULL << 32) | 0x200;
        packet[9] = 0x52;
        packet[10] = 2;
        packet[11] = 0x53;
        packet[12] = 0;
        packet[13] = 0x3F;

        /* sync: the packet stores must reach memory before the DMAC reads the scratchpad. */
        __asm__ volatile("sync" ::: "memory");
        FlushCache(0);
        xglDmaDirectNormal(2, (unsigned int) packet, 7);
        sceGsSyncPath(0, 0);
        func_002F38F0(0);
    }

    gVsyncsSinceFlip = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_00199FE8);

/* X3 scaffold label of the draw-environment packet (0x004c80c0); config names it only by address. */
extern GsDrawPacket gXglRenderDrawPacket;
/* Scaffold labels of the two callees (config has no name for them, and the sync and put calls keep them). */
void func_002E7348(void *start, void *end);
void func_002F3808(GsDrawEnv *env);

void xglRenderDrawEnvMove(void)
{
    gXglRenderDrawPacket.env.firstFrame.fbp = D_004C8390.prev_dispfb_base;
    gXglRenderDrawPacket.env.secondFrame.fbp = D_004C8390.prev_dispfb_base;
    gXglRenderDrawPacket.env.firstZbuf.zbp = D_004C8390.zbuf_base;
    gXglRenderDrawPacket.env.secondZbuf.zbp = D_004C8390.zbuf_base;
    func_002E7348(&gXglRenderDrawPacket, &gXglRenderDrawPacket + 1);
    func_002F3808(&gXglRenderDrawPacket.env);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A280);

/* The display environment block (0x004c81c0). */
extern GsDispEnv xglRenderDispEnvBlock;
void func_002F3748(GsDispEnv *env);

void xglRenderDispEnvMove(void)
{
    xglRenderDispEnvBlock.firstDispfb.dispfbFields.fbp = sRender.vram.flip_base;
    switch (ScanLineInterpolate) {
    case 0:
        xglRenderDispEnvBlock.pmode.firstCircuitEnable = 0;
        break;
    case 1:
        xglRenderDispEnvBlock.secondDispfb.dispfbFields.fbp = sRender.vram.flip_base;
        xglRenderDispEnvBlock.pmode.firstCircuitEnable = xglRenderDispEnvBlock.pmode.secondCircuitEnable;
        xglRenderDispEnvBlock.pmode.alp = 0x80;
        xglRenderDispEnvBlock.secondDisplay.displayFields.dx = xglRenderDispEnvBlock.firstDisplay.displayFields.dx;
        xglRenderDispEnvBlock.secondDisplay.displayFields.dy = xglRenderDispEnvBlock.firstDisplay.displayFields.dy - 1;
        break;
    case 2:
        break;
    }
    func_002E7348(&xglRenderDispEnvBlock, (u8 *) &xglRenderDispEnvBlock + 88);
    func_002F3748(&xglRenderDispEnvBlock);
    GS_REG(GS_DISPFB1) = xglRenderDispEnvBlock.secondDispfb.packed;
    GS_REG(GS_DISPLAY1) = xglRenderDispEnvBlock.secondDisplay.packed;
    GS_REG(GS_EXTBUF) = xglRenderDispEnvBlock.extbuf;
    GS_REG(GS_EXTDATA) = xglRenderDispEnvBlock.extdata.packed;
    GS_REG(GS_EXTWRITE) = xglRenderDispEnvBlock.extwrite;
    GS_REG(GS_CSR) = 16;
}

void xglRenderClearDepth(void)
{
    ClearEnv.env.clearAlphaDepthTest = GS_TEST_CLEAR_DEPTH;
}

void xglRenderClearFrame(void)
{
    ClearEnv.env.clearAlphaDepthTest = GS_TEST_CLEAR_FRAME;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A530);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A558);

void xglRenderSyncInit();

void xglRenderScanLineInit(void)
{
  *((volatile s32 *) 0x10003820) = 0;
  *((volatile s32 *) 0x10003C20) = 0;
  ScanLineInterpolate = 1;
  func_002F38F0(0);
  if (D_004C8390.unmodeled_76[0xA] == 0)
  {
    if (!(D_004C8390.unmodeled_76[0xB] != 0))
    {
      func_002F3118(0, 1, 2, 0);
    }
    else
    {
      func_002F3118(0, 0, 0x50, 1);
    }
  }
  else
  {
    func_002F3118(0, 1, 3, 0);
  }
  xglRenderSyncInit();
}

extern unsigned char xglRenderStartFlag[];
s32 func_0019A280();
void xglRenderScanLineInit();
void xglRenderDispEnvFlagSet();
s32 xglIntcEnable();

void xglRenderStart(s32 renderStartMode) {
    xglRenderStartFlag[0] = renderStartMode;
    xglRenderScanLineInit();
    xglIntcEnable(5);
    func_0019A280();
    xglRenderDispEnvFlagSet();
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A6F0);

/* The fade bytes sit just past XglRenderStateBlock, which must keep its 0x86-byte size (the object records it). */
typedef struct XglRenderFadeView {
  unsigned char unmodeled_00[0x86];
  unsigned char fade_gain;            /* +0x86 */
  unsigned char fade_level;           /* +0x87 */
} XglRenderFadeView;

#define D_004C8390_FADE (*(XglRenderFadeView *) &D_004C8390)

s32 xglRenderFadeAlpha(void)
{
  s32 scaled;
  scaled = (D_004C8390_FADE.fade_level * D_004C8390_FADE.fade_gain) >> 7;
  return (scaled >= 0x100) ? (0xFF) : (scaled);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A7C0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AB00);

void xglRenderDispEnvFlagClear(void) {
    xglRenderDispEnvBlock.pmodeWord &= ~2;
}

void xglRenderDispEnvFlagSet(void) {
    xglRenderDispEnvBlock.pmodeWord |= 2;
}

extern s8 xglRenderLocked;

void xglRenderLockSet(void) {
    xglRenderLocked = 1;
}

void xglRenderLockClear(void) {
    xglRenderLocked = 0;
}

extern s8 xglRenderHeld;

void xglRenderHoldClear(void) {
    xglRenderHeld = 0;
}

void xglRenderHoldSet(void) {
    xglRenderHeld = 1;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC80);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC98);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019ADD8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AE90);

extern s32 D_004C8400[];
extern s32 xglRenderGlobalFadeValue;

void xglRenderGlobalFadeReset(void) {
    xglRenderGlobalFadeValue = 0;
    D_004C8400[0] = 0;
}

s32 xglRenderGlobalFadeValueGet(void) {
    return xglRenderGlobalFadeValue;
}

void xglRenderGlobalFadeValueSet(s32 fadeValue) {
    xglRenderGlobalFadeValue = fadeValue;
}

void xglRenderGlobalFadeSet(float red, float green, float blue, float alpha)
{
    if (red < 0.0f) {
        red = 0.0f;
    }
    if (green < 0.0f) {
        green = 0.0f;
    }
    if (blue < 0.0f) {
        blue = 0.0f;
    }
    if (alpha < 0.0f) {
        alpha = 0.0f;
    }
    if (1.0f < red) {
        red = 1.0f;
    }
    if (1.0f < green) {
        green = 1.0f;
    }
    if (1.0f < blue) {
        blue = 1.0f;
    }
    if (1.0f < alpha) {
        alpha = 1.0f;
    }
    xglRenderGlobalFadeValue = ((int) (red * 255.0f) & 0xff) + (((int) (green * 255.0f) & 0xff) << 8) + (((int) (blue * 255.0f) & 0xff) << 16) + (((int) (alpha * 128.0f) & 0xff) << 24);
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B020);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B3C0);

extern s32 xglRenderFadeGifWord[];
extern unsigned char gXglRenderFadeDisabled[];
s32 xglRenderFadeAlpha();
s32 sceVif1PkRef();

void xglRenderGlobalFadeSend(void) {
    s32 alpha;
    XglPacket *packet;

    if (gXglRenderFadeDisabled[0] == 0) {
        alpha = xglRenderFadeAlpha();
        xglRenderFadeGifWord[0] = alpha;
        packet = xglPacketGetCurrent();
        /* the GIF packet starts 0x44 bytes (17 words) before the fade word */
        sceVif1PkRef(packet, &xglRenderFadeGifWord[-0x11], 0x15, 0, 0, 0);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", xglRenderMove);

extern s8 xglRenderMovieActive;
s32 func_0019ADD8();
s32 moviePlayerInit();

void xglRenderMovieBegin(void) {
    gXglRenderFadeDisabled[0] = 1;
    xglRenderMovieActive = 1;
    func_0019ADD8();
    moviePlayerInit();
}

void xglRenderFrame(void) {
    xglRenderMove();
    soundApplySettings();
    xglPadRead();
}

extern s32 xglRenderIoActive;

void xglRenderIoBegin(void) {
    xglRenderIoActive = 1;
}

void xglRenderIoEnd(void) {
    xglRenderIoActive = 0;
}
