#include "common.h"
#include "shared.h"
/* The EE scratchpad (16 KB), where the GS queue and talk packets are built before they are sent. */
#define SCRATCHPAD_BASE 0x70000000

void soundApplySettings(void);
void xglDmaDirectNormal(unsigned int channel, unsigned int address, unsigned int count);

/* GS PMODE register: display circuit enables and the alpha blend value. */
typedef struct GsPmode {
    unsigned int en1 : 1;
    unsigned int en2 : 1;
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
    u64 value;
    GsDispfb field;
} GsDispfbRegister;

typedef union GsDisplayRegister {
    u64 value;
    GsDisplay field;
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
    u64 value;
    GsExtdata field;
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
    u64 target;
    u64 target_address;
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
    u64 restore_target;
    u64 restore_target_address;
} GsClearEnv;

typedef union GsClearPacket {
    GsClearEnv value;
    AlignedVector4 quadwords[6];
} GsClearPacket;

extern GsClearPacket ClearEnv;

/*
 * GS display-environment block (0x004A9000, size 0x50). xglRenderDispOff/On
 * only touch the 8-byte flag word at +0x00 (bit 0x2 gates display output).
 */
typedef struct GsDispEnv {
    union {
        u64 flags;
        GsPmode pmode;
    };
    unsigned char unmodeled_08[8];
    GsDispfbRegister dispfb1;          /* +0x10 */
    GsDisplayRegister display1;        /* +0x18 */
    unsigned char unmodeled_20[8];
    GsDispfbRegister dispfb2;          /* +0x28 */
    GsDisplayRegister display2;        /* +0x30 */
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
    u64 scax0 : 11;
    u64 unmodeled_11 : 5;
    u64 scax1 : 11;
    u64 unmodeled_27 : 5;
    u64 scay0 : 11;
    u64 unmodeled_43 : 5;
    u64 scay1 : 11;
    u64 unmodeled_59 : 5;
} GsScissor;

typedef struct GsTest {
    u64 ate : 1;
    u64 atst : 3;
    u64 aref : 8;
    u64 afail : 2;
    u64 date : 1;
    u64 datm : 1;
    u64 zte : 1;
    u64 ztst : 2;
    u64 unmodeled_19 : 45;
} GsTest;

typedef struct GsTexa {
    u64 ta0 : 8;
    u64 unmodeled_08 : 7;
    u64 aem : 1;
    u64 unmodeled_16 : 16;
    u64 ta1 : 8;
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
    u64 regs0 : 4;
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
    GsFrame frame1;
    u64 frame1_address;
    GsZbuf zbuf1;
    u64 zbuf1_address;
    GsXyoffset xyoffset1;
    u64 xyoffset1_address;
    GsScissor scissor1;
    u64 scissor1_address;
    GsTest test1;
    u64 test1_address;
    GsFrame frame2;
    u64 frame2_address;
    GsZbuf zbuf2;
    u64 zbuf2_address;
    GsXyoffset xyoffset2;
    u64 xyoffset2_address;
    GsScissor scissor2;
    u64 scissor2_address;
    GsTest test2;
    u64 test2_address;
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
    GsDrawEnv value;
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
 * dispfb1 (+0x10), display1 (+0x18), dispfb2 (+0x28), display2 (+0x30),
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

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", xglRenderSyncInit);

/* X3 scaffold label 0x004c8390: the fields func_00199DA8 reads and writes, at their asm offsets. */
typedef struct XglRenderSyncFlags {
    unsigned char unmodeled_00[2];
    short psm;                          /* +0x02 */
    unsigned char unmodeled_04[6];
    unsigned short dispfb_base;         /* +0x0A */
    unsigned short prev_dispfb_base;    /* +0x0C */
    unsigned char unmodeled_0e[0x67];   /* +0x0E..+0x74 */
    unsigned char frame_delta;          /* +0x75 */
    unsigned char unmodeled_76[0x0e];   /* +0x76..+0x83 */
    unsigned char flag_84;              /* +0x84 */
} XglRenderSyncFlags;

extern XglRenderSyncFlags D_004C8390;
extern signed char D_00551206;
extern volatile unsigned int D_00551214;
/* Scaffold labels of the callees (config has no name for them). */
void func_001337B8(void);
void func_002F32B8(void);
void func_002F3118(int, int, int, int);
void func_002F38F0(int);
int sceGsSyncPath(int, int);

void func_00199DA8(void)
{
    u64 *packet;
    int flag;
    int pollCount;
    int pathBusy;

    if ((unsigned int) D_00551206 < D_00551214) {
        D_004C8390.frame_delta = 1;
    } else {
        D_004C8390.frame_delta = 0;
    }

    flag = 0;
    pathBusy = 0;
    D_004C8390.flag_84 = 0;
    for (pollCount = 0; pollCount < 0x80; pollCount++) {
        pathBusy = sceGsSyncPath(1, 0);
        if (pathBusy == 0) {
            if (D_00551214 >= 3 && flag == 0) {
                D_004C8390.flag_84 = 1;
            }
            flag = 1;
            if (D_00551214 == D_00551206 || (unsigned int) D_00551206 < D_00551214) {
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
        packet[4] = ((u64) (D_004C8390.dispfb_base << 5) | 0x80000)
            | ((u64) (D_004C8390.prev_dispfb_base << 5) << 32) | ((u64) 0x8000 << 36)
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

    D_00551214 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_00199FE8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A1E8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A280);

/* X3 scaffold labels of the display environment block (0x004c81c0) and of ScanLineInterpolate (0x00551210); config names neither. */
extern GsDispEnv D_004C81C0;
extern int D_00551210;
/* Scaffold labels of the two callees (config has no name for them). */
void func_002E7348(void *start, void *end);
void func_002F3748(GsDispEnv *env);

void xglRenderDispEnvMove(void)
{
    D_004C81C0.dispfb1.field.fbp = sRender.vram.flip_base;
    switch (D_00551210) {
    case 0:
        D_004C81C0.pmode.en1 = 0;
        break;
    case 1:
        D_004C81C0.dispfb2.field.fbp = sRender.vram.flip_base;
        D_004C81C0.pmode.en1 = D_004C81C0.pmode.en2;
        D_004C81C0.pmode.alp = 0x80;
        D_004C81C0.display2.field.dx = D_004C81C0.display1.field.dx;
        D_004C81C0.display2.field.dy = D_004C81C0.display1.field.dy - 1;
        break;
    case 2:
        break;
    }
    func_002E7348(&D_004C81C0, (u8 *) &D_004C81C0 + 88);
    func_002F3748(&D_004C81C0);
    *(volatile u64 *) GS_DISPFB1 = D_004C81C0.dispfb2.value;
    *(volatile u64 *) GS_DISPLAY1 = D_004C81C0.display2.value;
    *(volatile u64 *) GS_EXTBUF = D_004C81C0.extbuf;
    *(volatile u64 *) GS_EXTDATA = D_004C81C0.extdata.value;
    *(volatile u64 *) GS_EXTWRITE = D_004C81C0.extwrite;
    *(volatile u64 *) GS_CSR = 16;
}

void xglRenderClearDepth(void)
{
    ClearEnv.value.target = GS_TEST_CLEAR_DEPTH;
}

void xglRenderClearFrame(void)
{
    ClearEnv.value.target = GS_TEST_CLEAR_FRAME;
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A530);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A558);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A620);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A6C0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A6F0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A798);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019A7C0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AB00);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC10);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC30);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC50);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC60);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC68);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC70);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC80);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AC98);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019ADD8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AE90);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AEF8);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AF08);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019AF10);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", xglRenderGlobalFadeSet);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B020);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B3C0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B480);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", xglRenderMove);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B670);

void xglRenderFrame(void) {
    xglRenderMove();
    soundApplySettings();
    xglPadRead();
}

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B6C0);

INCLUDE_ASM("asm/main/nonmatchings/xgl_render", func_0019B6D0);
