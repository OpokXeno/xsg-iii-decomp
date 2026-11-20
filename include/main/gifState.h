#ifndef INCLUDE_MAIN_GIFSTATE_H
#define INCLUDE_MAIN_GIFSTATE_H

/* Types owned by main/tu218 and shared with other TUs (unified 2026-11-20 from the TU-local copies; offsets measured with the compiler). */

#include "shared.h"

/*
 * One GIF status block. The centre pair (+0x04/+0x08) is written by gifStateSetCenter and the
 * effect alpha pair (+0x0C/+0x10) by gifStateSetEffectAlpha; the state word and the ratio are read
 * by gifStateEffectReady.
 */
typedef struct GifStatusBlock {
    int state;
    int centerX;
    int centerY;
    int effectAlphaType;
    int effectAlpha;
    int fbp;                    /* +0x14: gifStateSetFbp */
    int blur;                   /* +0x18: gifStateSetBlur */
    int depth;                  /* +0x1C: the Z of the camera quad (gifStateSetDepth sets it) */
    f32 ratio;
    f32 ratioStep;              /* +0x24: gifStateSetDecrease; the draw functions lower ratio by it */
    int subWindow[4];           /* +0x28: nmlModelUseSubWindow; word 0 is the flag gifStateDrawBreakShards tests,
                                 * gifStateDrawFadeQuad moves its corners by all four */
    f32 subWindowValue[4];      /* +0x38: nmlModelSetSubWindowValue */
    int decreaseGate;           /* +0x48: tested for 0 before the draw functions lower ratio by ratioStep */
    u8 unmodeled_4c[0x50 - 0x4C];
    Vector4 mulColor;           /* +0x50: the model multiply colour nmlModelSetMulColor sets;
                                 * gifStateDrawCameraQuad scales it by blur into its offset */
} GifStatusBlock;

#endif
