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

#endif
