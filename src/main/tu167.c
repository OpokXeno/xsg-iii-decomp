#include "common.h"
#include "main/xgl_render.h"

typedef int s32;
typedef signed char s8;
typedef short s16;
typedef unsigned char u8;

/* Object whose s16 value at 0x54 is stepped toward its target by stepValueTowardTarget. */
typedef struct SteppedObject {
    u8 unk_00[0x54];
    s16 steppedValue;
} SteppedObject;
s32 setSteppedTarget(s16 *, s32);

void steppedObjectRaise(SteppedObject *steppedObject) {
    setSteppedTarget(&steppedObject->steppedValue, 0x80);
}

s32 *xglStudioGetCamera2(s32);

void steppedObjectLower(SteppedObject *steppedObject) {
    setSteppedTarget(&steppedObject->steppedValue, 0);
    *xglStudioGetCamera2(1) = 0;
}

s32 stepValueTowardTarget();

void steppedObjectUpdateColors(SteppedObject *steppedObject)
{
  s32 scaled_17;
  s32 scaled_13;
  stepValueTowardTarget(&steppedObject->steppedValue);
  gEnvFlags.color.red = (steppedObject->steppedValue * 0x17) >> 6;
  scaled_17 = steppedObject->steppedValue * 0x17;
  gEnvFlags.color.green = scaled_17 >> 6;
  scaled_13 = steppedObject->steppedValue * 0x13;
  gEnvFlags.color.blue = scaled_13 >> 6;
}

s32 optionalBlocksDraw();

void steppedObjectStepSecondary(s32 objectAddress) {
    stepValueTowardTarget(objectAddress + 0x58);
    optionalBlocksDraw();
}
