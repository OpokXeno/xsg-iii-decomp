#include "common.h"

typedef int s32;

extern s32 gSbNativeValueSlotIndex;
extern s32 gSbNativeValueRepeatRequest;
extern s32 gSbNativeValueSlots[];
void *memset(void *s, int c, unsigned int n);

void sbNativeValueSlotReset(void) {
    gSbNativeValueSlotIndex = 0;
    gSbNativeValueRepeatRequest = 0;
    memset(gSbNativeValueSlots, 0, 0x20);
}

void sbNativeValueSlotSet(s32 nativeStateRoutine, s32 repeatRequest) {
    gSbNativeValueRepeatRequest = repeatRequest;
    gSbNativeValueSlots[gSbNativeValueSlotIndex] = nativeStateRoutine;
}

typedef unsigned long long u64;

/* Per-slot frame handed to the slot handler (16 bytes, contents unknown). */
typedef struct SbNativeValueFrame {
    u64 registersLow;
    u64 registersHigh;
} SbNativeValueFrame;

extern SbNativeValueFrame gSbNativeValueFrames[];

s32 sbNativeValueSlotPush(s32 nativeStateRoutine, s32 repeatRequest) {
    s32 topSlotIndex = gSbNativeValueSlotIndex;
    s32 pushedSlotIndex;
    s32 *repeatRequestPtr = &gSbNativeValueRepeatRequest;

    if (topSlotIndex >= 7) {
        return 0;
    }
    pushedSlotIndex = topSlotIndex + 1;
    gSbNativeValueFrames[pushedSlotIndex] = gSbNativeValueFrames[pushedSlotIndex - 1];
    gSbNativeValueSlotIndex = pushedSlotIndex;
    gSbNativeValueSlots[pushedSlotIndex] = nativeStateRoutine;
    *repeatRequestPtr = repeatRequest;
    return 1;
}

s32 sbNativeValueSlotPop(s32 repeatRequest) {
    gSbNativeValueRepeatRequest = repeatRequest;
    if (gSbNativeValueSlotIndex != 0) {
        gSbNativeValueSlotIndex--;
    } else {
        if (gSbNativeValueSlots[0] == 0) {
            return 0;
        }
        gSbNativeValueSlots[0] = 0;
    }
    return 1;
}

s32 sbNativeValueSlotRun(void) {
    s32 topSlotIndex = gSbNativeValueSlotIndex;

    if (gSbNativeValueSlots[topSlotIndex] == 0) {
        return 0;
    }
    do {
        s32 runningSlotIndex = gSbNativeValueSlotIndex;

        gSbNativeValueRepeatRequest = 0;
        ((void (*)(SbNativeValueFrame *)) gSbNativeValueSlots[runningSlotIndex])(&gSbNativeValueFrames[runningSlotIndex]);
    } while (gSbNativeValueRepeatRequest != 0);
    return 1;
}
