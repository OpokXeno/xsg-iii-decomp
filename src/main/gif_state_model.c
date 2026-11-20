#include "common.h"
#include "shared.h"

#include "quadword.h"

#include "main/gifState.h"

extern GifStatusBlock gifStatus;

void gifStateSetDepth(int value);
void nmlModelSetMulColor(Vector4 *color);
void nmlModelUseSubWindow(unsigned int index, int enabled);
void gifStateSetDecrease(f32 decrease);
void gifStateSetCenter(int centerX, int centerY);
void gifStateSetEffectAlpha(int alphaType, int alpha);
void gifStateSetFbp(int fbp);
void gifStateSetBlur(int blur);

#define NML_WINDOW_COUNT 4

/* Sets the model multiply colour (a quadword copy). */
void nmlModelSetMulColor(Vector4 *color)
{
    QuadwordCopy(&gifStatus.mulColor, color);
}

/* Sets the alpha type and the alpha value of the effect (gifStatus +0x0C/+0x10). */
void gifStateSetEffectAlpha(int alphaType, int alpha) {
    gifStatus.effectAlphaType = alphaType;
    gifStatus.effectAlpha = alpha;
}

/* Sets the ratio step of the effect: the draw functions lower the ratio by it each frame. */
void gifStateSetDecrease(f32 decrease) {
    gifStatus.ratioStep = decrease;
}

/* Sets the centre of the effect (gifStatus +0x04/+0x08). */
void gifStateSetCenter(int centerX, int centerY) {
    gifStatus.centerX = centerX;
    gifStatus.centerY = centerY;
}

/* Stores the Z of the camera quad (gifStatus +0x1C). */
void gifStateSetDepth(int value) {
    gifStatus.depth = value;
}

/* Stores the frame buffer base of the effect. */
void gifStateSetFbp(int fbp) {
    gifStatus.fbp = fbp;
}

/* Stores the blur amount of the effect. */
void gifStateSetBlur(int blur) {
    gifStatus.blur = blur;
}

void nmlModelUseSubWindow(unsigned int index, int enabled) {
    if (index < NML_WINDOW_COUNT) {
        gifStatus.subWindow[index] = enabled;
    }
}

void nmlModelSetSubWindowValue(u32 index, f32 value) {
    if (index < 4U) {
        gifStatus.subWindowValue[index] = value;
    }
}
