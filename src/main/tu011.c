#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef float f32;

typedef struct PadRightStick {
    u8 unk_00[0x64];
    s8 rightX;
    s8 rightY;
} PadRightStick;

void padGetRightStick(PadRightStick *controllerPort, f32 *stickX, f32 *stickY) {
    *stickX = controllerPort->rightX * 0.0078125f;
    *stickY = controllerPort->rightY * 0.0078125f;
}

/* The two vibration motor bytes of PadData's first port record (PadRecord.actuator in XS2's pad.h). */
typedef struct PadActuatorView {
    u8 unmodeled_00[0x50];
    u8 actuator[2]; /* +0x50 */
} PadActuatorView;

#define PAD_VIBRATION_SMALL_MOTOR 0
#define PAD_VIBRATION_LARGE_MOTOR 1

extern PadActuatorView PadData[2];

/* The vibration timers: a two-halfword small-data array (the original loads its address with one gp-relative la).
 * PAD_VIBRATION_HOLD counts the frames the small motor keeps running, PAD_VIBRATION_FADE_FRAMES_LEFT the
 * remaining steps of the large motor's fade. */
#define PAD_VIBRATION_HOLD 0
#define PAD_VIBRATION_FADE_FRAMES_LEFT 1

extern unsigned char gPadLatchBlocked[];
extern s16 gPadLatchedValue[2];
extern s16 D_0055156C; /* fade length in frames */
extern u8 D_00551560; /* large motor value at the start of the fade */
extern u8 D_00551561; /* large motor value at the end of the fade */

void func_00186FB8(void)
{
    do {
        gPadLatchedValue[PAD_VIBRATION_HOLD] = 0;
        PadData[0].actuator[PAD_VIBRATION_LARGE_MOTOR] = 0;
    } while (0);
    do {
        gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] = 0;
        PadData[0].actuator[PAD_VIBRATION_SMALL_MOTOR] = 0;
    } while (0);
}

void padSetLatchedValue(s32 latchedValue) {
    if (gPadLatchBlocked[0] == 0) {
        gPadLatchedValue[PAD_VIBRATION_HOLD] = latchedValue;
    }
}

void func_00186FF0(u8 startValue, u8 endValue, s32 framesTotal)
{
    if (gPadLatchBlocked[0] == 0) {
        D_00551560 = startValue;
        D_00551561 = endValue;
        gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] = framesTotal;
        D_0055156C = framesTotal;
    }
}

void func_00187020(void)
{
    if (gPadLatchedValue[PAD_VIBRATION_HOLD] != 0) {
        PadData[0].actuator[PAD_VIBRATION_SMALL_MOTOR] = 1;
        if ((s16)(gPadLatchedValue[PAD_VIBRATION_HOLD] - 1) < 0) {
            gPadLatchedValue[PAD_VIBRATION_HOLD] = 0;
        } else {
            gPadLatchedValue[PAD_VIBRATION_HOLD] = gPadLatchedValue[PAD_VIBRATION_HOLD] - 1;
        }
    }
    if (gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] != 0) {
        int fadeStep;

        fadeStep = (D_00551560 - D_00551561) * gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] / D_0055156C;
        gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] = gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] - 1;
        PadData[0].actuator[PAD_VIBRATION_LARGE_MOTOR] = D_00551561 + fadeStep;
        if (gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] < 0) {
            gPadLatchedValue[PAD_VIBRATION_FADE_FRAMES_LEFT] = 0;
        }
    }
}
