#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu057", func_001B77A8);

typedef short s16;
typedef unsigned char u8;
typedef int s32;
typedef unsigned short u16;
extern s16 D_00551414;

s16 soundStateGetCounter(void) {
    return D_00551414;
}

extern s16 D_00551410;
extern s16 D_00551412;
extern s16 D_00551416;
extern s32 D_00551418;
extern s32 D_0055141C;
extern s32 D_00551420;
extern unsigned char D_005A63B0[];
void soundSlotsReset();
void *memset();

void soundStateReset(void) {
    memset(D_005A63B0, 0, 0x100);
    D_00551410 = 0;
    D_00551412 = 0;
    D_00551414 = 0;
    D_00551416 = 0;
    D_00551418 = 0;
    D_0055141C = 0;
    D_00551420 = 0;
    soundSlotsReset();
}

s32 soundSlotStop(s32);

void soundStateClearCounters(void) {
    D_00551410 = 0;
    D_00551412 = 0;
    D_00551414 = 0;
    D_00551416 = 0;
    D_00551418 = 0;
    soundSlotStop(0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu057", func_001B78B0);

INCLUDE_ASM("asm/main/nonmatchings/tu057", func_001B7A60);

INCLUDE_ASM("asm/main/nonmatchings/tu057", func_001B7B98);

s32 func_001B77A8();

s32 soundDriverSendOpcode2(void) {
    s32 command;

    command = 2;
    func_001B77A8(&command);
    return 1;
}

/* A command record for func_001B77A8: the opcode, then two 16-bit parameters. */
struct SoundCommand {
    /* 0x0 */ s32 opcode;
    /* 0x4 */ s16 param_a;
    /* 0x6 */ s16 driverMode;
};

s32 soundDriverSetMode(s32 requestedMode)
{
  struct SoundCommand command;
  s32 driverMode;
  driverMode = requestedMode;
  if (driverMode < 0)
  {
    if (requestedMode)
    {
      driverMode = 2;
    }
    else
    {
      driverMode = 2;
    }
  }
  command.opcode = 4;
  command.param_a = 0;
  command.driverMode = 0;
  func_001B77A8(&command.opcode);
  command.opcode = 3;
  func_001B77A8(&command.opcode);
  command.opcode = 4;
  command.driverMode = driverMode;
  command.param_a = D_0055141C;
  func_001B77A8(&command.opcode);
  return 1;
}
