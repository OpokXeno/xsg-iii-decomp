#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267108);

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_002671F0);

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267228);

typedef unsigned char u8;

typedef int s32;

typedef signed char s8;

typedef float f32;

/* One camera cameraSlot: 0x60 bytes, the first byte is the active flag. */
typedef struct CameraSlot {
    s8 active;
    u8 unk_01[0x5F];
} CameraSlot;

extern CameraSlot gCameraSlots[];

void *memset(void *, int, unsigned int);

void cameraSlotsClear(void) {
    memset(gCameraSlots, 0, 0x240);
}

s32 func_00266008();

s32 func_00267680();

void cameraSlotsUpdate(void)
{
  s32 cameraSlotIndex;
  CameraSlot *cameraSlot;
  cameraSlot = gCameraSlots;
  for (cameraSlotIndex = 0; cameraSlotIndex < 6; cameraSlotIndex++)
  {
    if (cameraSlot->active != 0)
    {
      func_00266008();
      if (func_00267680(cameraSlot) != 0)
      {
        cameraSlot->active = 0;
      }
    }
    cameraSlot++;
  }

}

void cameraSlotsClear();

void cameraSlotsReset(void) {
    cameraSlotsClear();
}

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267398);

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_002674C0);

f32 floatWrapToPeriod(f32 value, f32 period) {
    f32 remainder;

    remainder = value;
    if (period <= remainder) {
        do {
            remainder -= period;
        } while (period <= remainder);
    }
    return remainder;
}

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267680);

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267998);

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_002679F8);

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267A58);

extern s32 D_00550858;

void battleClearFlag(void) {
    D_00550858 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu265", func_00267AD8);
