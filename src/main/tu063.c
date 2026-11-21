#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001B9E50);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001B9FD8);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA070);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA1D8);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA310);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA480);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA598);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA678);

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;

/* The pad port the debug menu reads: the held and repeated button halfwords at +0x28 and +0x2C, and a third
 * halfword at +0x6C that the menu ORs into the repeated buttons (its role is not modeled yet). */
typedef struct DebugPadState {
    unsigned char unmodeled_00[0x28];
    u16 held;                       /* +0x28 */
    unsigned char unmodeled_2A[2];
    u16 repeated;                   /* +0x2C */
    unsigned char unmodeled_2E[0x3E];
    u16 unk6C;                      /* +0x6C */
} DebugPadState;

extern s32 D_003E63A0[];
extern char gDebugCursorString[];
extern char gDebugAngleFormat[];
extern char gDebugFlagFormat[];
extern s32 gDebugMenuPage;
extern s32 gDebugAngleFlag;
extern s32 gDebugAngle;
int GameMpeg2Play();
extern DebugPadState PadData[];
s32 func_001B8B58();
void func_001B8BA0();
void xglFontDebugPrintf();

/* Debug menu over the pad: page 0 toggles the mode and the table entry, page 1 steers the angle
 * and applies it through func_001B8BA0; the strings print the mode name, the angle and the flag. */
void debugAngleMenuUpdate(void)
{
  unsigned int angleStep;
  DebugPadState *pad = PadData;
  s32 menuTextBuffer[8];
  s32 menuButtons;
  s32 heldButtons;
  s32 angleDelta;
  s32 angleValid;
  s32 newMenuPage;
  s32 newAngleFlag;
  s32 angle;
  heldButtons = pad->held;
  menuButtons = pad->repeated | pad->unk6C;
  angleDelta = 0;
  if (menuButtons & 0x5000)
  {
    newMenuPage = 1 - gDebugMenuPage;
    gDebugMenuPage = newMenuPage;
  }
  angleValid = func_001B8B58(gDebugAngle, gDebugAngleFlag);
  switch (gDebugMenuPage)
  {
    case 0:
      if (menuButtons & 0xA050)
    {
      newAngleFlag = 1 - gDebugAngleFlag;
      gDebugAngleFlag = newAngleFlag;
    }
      break;

    case 1:
      if (menuButtons & 0xA000)
    {
      if (menuButtons & 0x8000)
      {
        angleStep = 1;
        angleDelta = (!(heldButtons & 0x80)) ? (-angleStep) : (-0xA);
      }
      if (menuButtons & 0x2000)
      {
        angleDelta = (!(heldButtons & 0x80)) ? (gDebugMenuPage) : (0xA);
      }
      angle = (gDebugAngle + angleDelta) & 0x7FF;
      gDebugAngle = angle;
    }
      if (menuButtons & 0x50)
    {
      angleValid = 1 - func_001B8B58(gDebugAngle, gDebugAngleFlag);
      func_001B8BA0(gDebugAngle, gDebugAngleFlag, angleValid);
    }
      break;

  }

  GameMpeg2Play(&menuTextBuffer[0], gDebugCursorString);
  xglFontDebugPrintf(8, (gDebugMenuPage * 8) + 0x18, &menuTextBuffer[0]);
  GameMpeg2Play(&menuTextBuffer[0], gDebugFlagFormat, D_003E63A0[gDebugAngleFlag]);
  xglFontDebugPrintf(0x10, 0x18, &menuTextBuffer[0]);
  GameMpeg2Play(&menuTextBuffer[0], gDebugAngleFormat, gDebugAngle, angleValid);
  xglFontDebugPrintf(0x10, 0x20, &menuTextBuffer[0]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA908);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BA9E8);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BACC0);

INCLUDE_ASM("asm/main/nonmatchings/tu063", func_001BAE58);
