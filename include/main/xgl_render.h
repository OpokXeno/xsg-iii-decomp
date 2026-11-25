/* header_harvest: hand-maintained (X3 express integration; tools/header_harvest.py leaves it alone). */
#ifndef INCLUDE_MAIN_XGL_RENDER_H
#define INCLUDE_MAIN_XGL_RENDER_H

/* Types owned by main/tu034 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"
#include "main/Packet.h"

/* One GS A+D pair of a packet built in the scratchpad: the register data, then the register address. */
typedef struct GsAdPair {
    u64 data;
    u64 reg;
} GsAdPair;

/* One row of asDispReso: the display geometry sceGsSetDefDispEnv and the draw environment take. */
typedef struct XglRenderReso {
    short depth_psm;                   /* +0x00 */
    short frame_psm;                   /* +0x02: sRenderPsm */
    short frame_width;                 /* +0x04 */
    short frame_height;                /* +0x06 */
} XglRenderReso;

typedef void (*XglRenderFadeCallback)(XglPacket *packet);

/* One row of asVramBase: the GS frame-buffer pages of a resolution. Offset comments are relative to sRender. */
typedef struct XglRenderVram {
    unsigned short depth_base;         /* sRender+0x08 */
    unsigned short unmodeled_0a[3];
    u16 width;                         /* sRender+0x10: the display page */
    u16 height;                        /* sRender+0x12: the draw page */
    unsigned short flip_base;          /* sRender+0x14: sRenderFrameBase */
    unsigned short draw_base;          /* sRender+0x16: res_get_path.c builds the jpeg bitblt word from it (page << 5) */
    unsigned short unmodeled_18;
    unsigned short second_base;        /* sRender+0x1A: the second page the movie player uploads movie strips to; the talk and
                                          text frames read it as the font texture base (in 32-word units) */
    unsigned short unmodeled_1c[2];
} XglRenderVram;

/* sRender (0x00645820): the render state of xgl_render.c, which fills it from asDispReso and asVramBase. The map names
 * sRenderPsm (+0x02), sRenderFrameBase (+0x14), sRenderDisplayBase (+0x20). The width and height at +0x04 and +0x06 are
 * the main frame size. Other files still read it through local views of their own (game_movie.c, game_defocus.c, ...). */
typedef struct XglRenderState {
    XglRenderReso reso;                /* +0x00 */
    XglRenderVram vram;                /* +0x08 */
    u16 display_buffer_base;           /* +0x20: sRenderDisplayBase */
    u16 draw_buffer_base;              /* +0x22 */
    /* +0x24: the per-frame callbacks xgl_render.c's final packet pass calls with the frame packet; recordRenderAll takes
     * slot 6. */
    XglRenderFadeCallback final_packet_callback[8];
    /* +0x44: battleSceneInit sets it to ov01DrawAll and ov01SceneEnd clears it. */
    XglRenderFadeCallback fade_callback;
    u32 render_status;                 /* +0x48 */
    u32 frame_delta;                   /* +0x4c */
    u32 frame_count;                   /* +0x50 */
} XglRenderState;

/* Callbacks xglRenderFinalPacket runs on the frame packet. */
#define RENDER_FINAL_CALLBACK_COUNT 21

/* The X3 render state at 0x004C8390 (0x88 bytes with the fade gain and level). XglRenderState above is
 * X2's layout and belongs to sRender only. */
typedef struct XglRenderStateBlock {
    short depth_psm;                    /* +0x00 */
    short psm;                          /* +0x02 */
    short frame_width;                  /* +0x04 */
    short frame_height;                 /* +0x06 */
    unsigned short zbuf_base;           /* +0x08: the Z-buffer page xglRenderDrawEnvMove copies into both zbuf fields */
    unsigned short dispfb_base;         /* +0x0A */
    unsigned short prev_dispfb_base;    /* +0x0C */
    /* +0x0E: the third full-screen page (X2 work_buffer_base; tu163.c reads it as movieBackdropBasePage) */
    unsigned short work_buffer_base;
    unsigned short aux_page;            /* +0x10: a VRAM page func_0019A6F0 sets; no reader in this build */
    unsigned short texture_base;        /* +0x12: the texture buffer page (Packet.c's texture transfer) */
    unsigned short aux_page2;           /* +0x14: a VRAM page func_0019A6F0 sets; no reader in this build */
    /* +0x16: the page shown (X2 sRenderDisplayBase); swapped with flip_base every frame; read by the drop-shadow
     * packets of XenoActorUnpack.c */
    unsigned short display_buffer_base;
    unsigned short flip_base;           /* +0x18: the next display page (X2 sRenderFlipBase) */
    unsigned char unmodeled_1a[2];      /* +0x1A..+0x1B */
    XglRenderFadeCallback finalPacketCallbacks[RENDER_FINAL_CALLBACK_COUNT]; /* +0x1C: run by xglRenderFinalPacket */
    unsigned char unmodeled_70[4];      /* +0x70..+0x73 */
    unsigned char render_status;        /* +0x74: cleared by the base swap (X2 xglRenderStatus) */
    unsigned char frame_delta;          /* +0x75 */
    unsigned char path_busy_flag;       /* +0x76: cleared by the render reset (X2 xglRenderPathBusyFlag) */
    unsigned char unmodeled_77;         /* +0x77 */
    unsigned short frame_count;         /* +0x78: advanced by frame_delta on every base swap */
    unsigned short vsync_tick_count;    /* +0x7A: VSyncs since the last render move (X2 xglRenderVSyncTickCount) */
    /* +0x7C: half the ticks minus one, at least 0 (X2 xglRenderSkippedFrameCount) */
    short skipped_frame_count;
    unsigned char unmodeled_7e[2];      /* +0x7E..+0x7F */
    /* +0x80 / +0x81: the video mode xglRenderScanLineInit resets the GS with (PAL, else 480p when
     * progressiveMode is set, else NTSC interlaced); xglRenderStart sets progressiveMode. */
    unsigned char palMode;              /* +0x80 */
    unsigned char progressiveMode;      /* +0x81 */
    /* +0x82: the base swap sends the movie frame instead of swapping pages */
    unsigned char movieMode;
    unsigned char movieFullColor;       /* +0x83: the movie frame is PSMCT32 without dithering (else PSMCT16) */
    unsigned char syncBacklogFull;      /* +0x84 */
    unsigned char unmodeled_85;         /* +0x85 */
    unsigned char fade_gain;            /* +0x86 */
    unsigned char fade_level;           /* +0x87 */
} XglRenderStateBlock;

/* The clear packet at gEnvFlags (0x004C8240, 0x150 = 0x15 quadwords, sent whole by the base swap in movie
 * mode): one quadword, then the GIF packet that xglRenderMove sends by DMA from +0x10 (the
 * ClearEnv label). Its A+D writes include the TEST value that xglRenderClearDepth / xglRenderClearFrame select
 * (+0x20) and the clear colour, a packed RGBAQ (+0x40). */
typedef struct XglClearPacket {
    unsigned char unmodeled_00[0x10];
    u64 gifTag[2];                      /* +0x10: the GIF tag, start of the packet xglRenderMove sends */
    u64 test;                           /* +0x20: GS TEST_1 value of the clear */
    unsigned char unmodeled_28[0x18];
    struct {
        unsigned char red;              /* +0x40 */
        unsigned char green;            /* +0x41 */
        unsigned char blue;             /* +0x42 */
        unsigned char alpha;            /* +0x43 */
    } color;
    unsigned char unmodeled_44[0x10C];
} XglClearPacket;

extern XglClearPacket gEnvFlags;

#endif
