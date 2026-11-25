#include "common.h"
#include "main/tu007.h"

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

#define PAD_VIBRATION_SMALL_MOTOR 0
#define PAD_VIBRATION_LARGE_MOTOR 1

extern PadRecord PadData[2];

/* The vibration timers (small data; the original loads their address with one gp-relative la): hold counts
 * the frames the small motor keeps running, fadeFramesLeft the remaining steps of the large motor's fade. */
typedef struct PadVibrationTimers {
    s16 hold;
    s16 fadeFramesLeft;
} PadVibrationTimers;

extern unsigned char gPadLatchBlocked[];
static u8 padVibrationFadeStart; /* large motor value at the start of the fade */
static u8 padVibrationFadeEnd; /* large motor value at the end of the fade */
static PadVibrationTimers gPadLatchedValue;
static s16 padVibrationFadeFrames; /* fade length in frames */

void Vibration_Stop(void)
{
    do {
        gPadLatchedValue.hold = 0;
        PadData[0].actuator[PAD_VIBRATION_LARGE_MOTOR] = 0;
    } while (0);
    do {
        gPadLatchedValue.fadeFramesLeft = 0;
        PadData[0].actuator[PAD_VIBRATION_SMALL_MOTOR] = 0;
    } while (0);
}

void padSetLatchedValue(s32 latchedValue) {
    if (gPadLatchBlocked[0] == 0) {
        gPadLatchedValue.hold = latchedValue;
    }
}

void Vibration_Set_Strong(u8 startValue, u8 endValue, s32 framesTotal)
{
    if (gPadLatchBlocked[0] == 0) {
        padVibrationFadeStart = startValue;
        padVibrationFadeEnd = endValue;
        gPadLatchedValue.fadeFramesLeft = framesTotal;
        padVibrationFadeFrames = framesTotal;
    }
}

void VibrationEngine(void)
{
    if (gPadLatchedValue.hold != 0) {
        PadData[0].actuator[PAD_VIBRATION_SMALL_MOTOR] = 1;
        if ((s16)(gPadLatchedValue.hold - 1) < 0) {
            gPadLatchedValue.hold = 0;
        } else {
            gPadLatchedValue.hold = gPadLatchedValue.hold - 1;
        }
    }
    if (gPadLatchedValue.fadeFramesLeft != 0) {
        int fadeStep;

        fadeStep = (padVibrationFadeStart - padVibrationFadeEnd) * gPadLatchedValue.fadeFramesLeft / padVibrationFadeFrames;
        gPadLatchedValue.fadeFramesLeft = gPadLatchedValue.fadeFramesLeft - 1;
        PadData[0].actuator[PAD_VIBRATION_LARGE_MOTOR] = padVibrationFadeEnd + fadeStep;
        if (gPadLatchedValue.fadeFramesLeft < 0) {
            gPadLatchedValue.fadeFramesLeft = 0;
        }
    }
}

/* The movie flag bytes (gMovieFlagBytes, read by scriptMovieFlagByteGet in tu091) start at 0x4B34D4 inside this array. */
signed char D_004B34C0[176] = { 0 };
