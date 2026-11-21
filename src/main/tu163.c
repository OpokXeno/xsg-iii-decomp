#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu163", func_002169A0);

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef struct GameLoopRuntime {
    s32 loopFlags;
} GameLoopRuntime;

extern unsigned char gGameLoopRuntimeStorage[];
#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)
s32 xcClearPayloadSections();

void xcBlockFlagsReset(void) {
    s32 remainingFlags;

    remainingFlags = gGameLoopRuntime.loopFlags & ~0x400;
    gGameLoopRuntime.loopFlags = remainingFlags;
    xcClearPayloadSections();
}

extern s32 D_00550C50;

void xcLoadCounterReset(void) {
    D_00550C50 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu163", func_00216C08);

INCLUDE_ASM("asm/main/nonmatchings/tu163", func_00216F38);

INCLUDE_ASM("asm/main/nonmatchings/tu163", func_00217310);

void springFloatStep(f32 *position, f32 *previousPosition, f32 *targetPosition, f32 stiffness) {
    f32 currentPosition;
    f32 nextPosition;

    currentPosition = *position;
    nextPosition = currentPosition + ((currentPosition - *previousPosition) + ((*targetPosition - currentPosition) * stiffness));
    *position = nextPosition;
    *previousPosition = nextPosition;
}

INCLUDE_ASM("asm/main/nonmatchings/tu163", func_00217570);
