#include "common.h"

typedef signed char s8;
typedef short s16;
typedef int s32;

typedef struct SwitchState {
    s16 enabled;
    s16 disabled;
    s32 activeSwitchIndex;
} SwitchState;

void setSwitchState(SwitchState *movieSwitch, s32 switchIndex) {
    if (switchIndex != 0) {
        movieSwitch->activeSwitchIndex = switchIndex;
        movieSwitch->enabled = 1;
        movieSwitch->disabled = 0;
        return;
    }
    movieSwitch->enabled = 0;
    movieSwitch->disabled = 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu286", func_0026E5C8);

INCLUDE_ASM("asm/main/nonmatchings/tu286", func_0026E648);
