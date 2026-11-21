#include "common.h"

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

extern unsigned char gEnvFlags[];

/* Three colour channel bytes at gEnvFlags + 0x40, 0x41 and 0x42. */
typedef struct EnvColorChannels {
    u8 unk_00[0x40];
    s8 colorRed;
    s8 colorGreen;
    s8 colorBlue;
} EnvColorChannels;

#define gEnvColorChannels (*(EnvColorChannels *) gEnvFlags)
s32 stepValueTowardTarget();

void steppedObjectUpdateColors(SteppedObject *steppedObject)
{
  s32 scaled_17;
  s32 scaled_13;
  stepValueTowardTarget(&steppedObject->steppedValue);
  gEnvColorChannels.colorRed = (steppedObject->steppedValue * 0x17) >> 6;
  scaled_17 = steppedObject->steppedValue * 0x17;
  gEnvColorChannels.colorGreen = scaled_17 >> 6;
  scaled_13 = steppedObject->steppedValue * 0x13;
  gEnvColorChannels.colorBlue = scaled_13 >> 6;
}

s32 func_002184E0();

void steppedObjectStepSecondary(s32 objectAddress) {
    stepValueTowardTarget(objectAddress + 0x58);
    func_002184E0();
}
