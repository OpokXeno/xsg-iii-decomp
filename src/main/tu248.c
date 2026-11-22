#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu248", func_002607B8);

typedef int s32;
typedef float f32;
s32 xglMatrixUnit();
f32 func_00260F70(f32, f32, f32);
f32 xglAtan2(f32, f32);
s32 xglMatrixRotY(f32 *, f32 *, f32);
s32 xglVectorMulMat();
s32 xglVectorNormal();

/* Angles out: [0] pitch, [1] yaw, [3] set to 1. The direction is targetPoint - basePoint. */
void xglVectorToAngles(f32 *angles, void *targetPoint, void *basePoint, f32 *pitchLimits, f32 *yawLimits) {
    f32 worldDirection[4];
    f32 yawLocalDirection[4];
    f32 baseMatrix[0x10];
    f32 yawMatrix[0x10];
    f32 yaw;
    f32 clampedYaw;
    f32 pitch;
    f32 clampedPitch;

    __asm__ __volatile__("lqc2 vf20, 0(%0)" : : "r"(targetPoint) : "memory");
    __asm__ __volatile__("lqc2 vf21, 0(%0)" : : "r"(basePoint) : "memory");
    __asm__ __volatile__("vsub.xyzw vf20, vf20, vf21" : : : "memory");
    __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(worldDirection) : "memory");
    xglVectorNormal(&worldDirection[0], &worldDirection[0]);
    yaw = xglAtan2(-worldDirection[0], -worldDirection[2]);
    angles[1] = yaw;
    clampedYaw = func_00260F70(yaw, yawLimits[0], yawLimits[1]);
    angles[1] = clampedYaw;
    xglMatrixUnit(&baseMatrix[0]);
    xglMatrixRotY(&yawMatrix[0], &baseMatrix[0], -angles[1]);
    xglVectorMulMat(&yawLocalDirection[0], &yawMatrix[0], &worldDirection[0]);
    pitch = xglAtan2(yawLocalDirection[1], -yawLocalDirection[2]);
    angles[0] = pitch;
    clampedPitch = func_00260F70(pitch, pitchLimits[0], pitchLimits[1]);
    angles[0] = clampedPitch;
    angles[3] = 1.0f;
}

/* Orders the two values so that *lowerBound <= *upperBound. */
void intPairOrder(s32 *lowerBound, s32 *upperBound) {
    s32 originalHigh;
    s32 originalLow;

    originalLow = *lowerBound;
    originalHigh = *upperBound;
    if (originalHigh < originalLow) {
        *lowerBound = originalHigh;
        *upperBound = originalLow;
    }
}
