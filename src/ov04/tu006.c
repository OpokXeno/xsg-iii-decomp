/*
 * OV04 original TU 6: 0x00a90940..0x00a974e0 (38 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A90940);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A90EB8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A91330);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A919C8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A91E28);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A92240);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A92738);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A92918);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A92B68);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A93390);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A93408);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A935C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A93860);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A93998);

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
/* Screen mode control block at gScreenModeControlStorage as this TU uses it: the screen mode halfword at +0x16, the countdown halfword at +0x1A and a table
 * of halfwords at +0xA6..+0xFB (the slot cleared by ov04SlotsClearUpper and ov04SlotsClearLower). */
typedef struct ScreenModeControl {
    u8 unk_00[0x16];
    s16 screenMode;                 /* +0x16 */
    u8 unk_18[0x1A - 0x18];
    s16 countdownTicks;                /* +0x1A */
    u8 unk_1C[0xA6 - 0x1C];
    s16 slots[(0xFC - 0xA6) / 2];   /* +0xA6 */
} ScreenModeControl;
extern s16 gScreenModeControlStorage[];
#define gScreenModeControl (*(ScreenModeControl *) gScreenModeControlStorage)
void func_00A989B0(s32, s32, s32, s32, s32, s32, s32, f32);
void func_00A9D718(s32, s32, s32);

void ov04EnterModeD(void)
{
  gScreenModeControl.screenMode = 0xD;
  gScreenModeControl.countdownTicks = 0;
  func_00A989B0(0x389, 0x2A, 0x150, 0x160, 0x20, 0, -1, 1.0f);
  func_00A989B0(0x38A, 0x2F, 0x150, 0x180, 0x20, 0, -1, 1.0f);
  func_00A989B0(0x38B, 0x34, 0x130, 0x1A2, 0x20, 0, -1, 1.0f);
  func_00A9D718(-1, 5, 0x1A);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A93C88);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A94188);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A94560);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A94E78);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A94FD0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A95278);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A95540);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A95758);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A958F8);

s32 func_00AA30D0(s32, s32, s32, s32, s32, s32, s32, s32, f32, s32, s32);

void ov04EnterMode24(void)
{
  func_00AA30D0(0x2FC, 0x19, 0x60, 0x10, -1, -1, 0, 0, 2.0f, 4, 0x74003F2F);
  func_00AA30D0(0x1A3, 0x1A, 0x10, 0x40, -1, -1, 0, 0, 1.0f, 5, -1);
  gScreenModeControl.screenMode = 0x24;
  gScreenModeControl.countdownTicks = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A95F38);

void ov04EnterMode23(void)
{
  func_00AA30D0(0x2FB, 0x16, 0x40, 0x10, -1, -1, 0, 0, 2.0f, 4, 0x74003F2F);
  func_00AA30D0(0x1AC, 0x17, 0x10, 0x40, -1, -1, 0, 0, 1.0f, 5, 0x741F343F);
  gScreenModeControl.screenMode = 0x23;
  gScreenModeControl.countdownTicks = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A96300);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A96628);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A96830);

extern s16 D_00AA9336;
s32 gsFlagBlockBSet(s32 flag);

void ov04EnterMode21(void) {
    func_00AA30D0(0x2FE, 0xB, 0x40, 0x10, -1, -1, 0, 0, 1.0f, 4, 0x74003F2F);
    func_00AA30D0(0x24C, 0xB, 8, 0x40, -1, -1, 0, 0, 1.0f, 5, 0x802F4F5F);
    D_00AA9336 = 0x21;
    gsFlagBlockBSet(1);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A96930);

void ov04SlotsClearUpper(void)
{
  s32 i;
  s16 *cursor;
  cursor = gScreenModeControlStorage;
  cursor = cursor + 0x7D;
  for (i = 0; i < 32; i++)
  {
    *cursor = 0;
    cursor -= 1;
  }

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A969B8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A969F8);

void ov04SlotsClearLower(void)
{
  s32 i;
  s16 *cursor;
  cursor = gScreenModeControlStorage;
  cursor = cursor + 0x5D;
  for (i = 0; i < 11; i++)
  {
    *cursor = 0;
    cursor -= 1;
  }

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A96B80);

INCLUDE_ASM("asm/nonmatchings/ov04/tu006", func_00A97170);
