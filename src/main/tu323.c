#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu323", func_0028B228);

INCLUDE_ASM("asm/main/nonmatchings/tu323", func_0028B268);

typedef float f32;

void floatStepCycle(f32 *cyclicValue, f32 increment, f32 minimum, f32 maximum) {
    f32 valueBeforeStep;
    f32 valueAfterStep;
    f32 boundary;
    f32 steppedValue;

    boundary = maximum;
    valueBeforeStep = *cyclicValue;
    valueAfterStep = valueBeforeStep + increment;
    steppedValue = minimum;
    if (boundary < valueAfterStep) {
        if (valueBeforeStep != boundary) {
            steppedValue = boundary;
        }
        *cyclicValue = steppedValue;
        return;
    }
    if (valueAfterStep < minimum) {
        if (valueBeforeStep != minimum) {
            boundary = minimum;
        }
        *cyclicValue = boundary;
        return;
    }
    *cyclicValue = valueAfterStep;
}
