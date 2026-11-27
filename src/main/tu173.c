#include "common.h"
#include "main/tu173.h"

typedef signed char s8;
typedef int s32;

typedef struct TiFrameBox {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
} TiFrameBox;

s32 sheetFramePosition(TiFramePos *position, TiFrameSheet *sheet, s32 animationTick) {
    s32 frame;
    s32 columns;

    frame = animationTick - sheet->frameBase;
    if ((frame < 0) || (columns = sheet->columns, ((frame < (columns * sheet->rows)) == 0))) {
        return 0;
    }
    if (position != 0) {
        position->x = (sheet->originX + ((frame % columns) * (sheet->cellWidthA + sheet->cellWidthB + sheet->cellWidthC)) + 0x20);
        position->y = (sheet->originY + ((frame / sheet->columns) * 0x160) + 0xF0);
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu173", func_00220130);

s32 xglFontSetColor();
s32 xglFontPrintEscapeByte();
s32 xglFontPrintExtendedColor();
s32 xglFontPrintTitle();
s32 xglFontPrintIndexed();
s32 sheetFramePosition();

void sheetFrameDrawText(TiFrameSheet *sheet, s32 animationTick, u32 color, s32 text)
{
  TiFramePos pos;
  s32 offsetX;
  u32 packedColor;

  packedColor = color;
  if ((text != 0) && (sheetFramePosition(&pos, sheet, animationTick) != 0))
  {
    xglFontPrintTitle();
    xglFontPrintEscapeByte(0xF);
    xglFontPrintExtendedColor(0, 0, 0);
    xglFontSetColor(packedColor & 0xFF, (packedColor >> 8) & 0xFF, (packedColor >> 0x10) & 0xFF);
    offsetX = pos.x - 0x6FF8;
    xglFontPrintIndexed((offsetX >> 4) + 7, (pos.y - 0x71F8) >> 4, text);
  }
}

s32 fontQueueColorState();
s32 xglFontDrawDigitsLower();

void sheetFrameDrawValue(TiFrameSheet *sheet, s32 animationTick, s32 mask, s32 displayValue)
{
  TiFramePos pos;
  s32 middleCellWidth;
  int shift;

  if (sheetFramePosition(&pos, sheet, animationTick) != 0)
  {
    fontQueueColorState(mask);
    middleCellWidth = sheet->cellWidthB;
    shift = 1;
    xglFontDrawDigitsLower((((pos.x + ((sheet->valueColumn) * 0x50)) + (sheet->cellWidthA + (middleCellWidth >> shift))) - 0x40) & 0xFFFF, (pos.y + 0x20) & 0xFFFF, displayValue, 0x10);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu173", func_00220350);

s32 func_0021B740();

void sheetFrameDrawBox(TiFrameSheet *sheet)
{
  TiFrameBox box;
  s8 rows;
  u16 top;
  s32 left;
  s32 yOffset;
  s16 frameBase;

  rows = sheet->rows;
  top = sheet->originY;
  left = sheet->originX + ((sheet->cellWidthA + sheet->cellWidthB + sheet->cellWidthC) * sheet->columns);
  box.right = left + 0x100;
  yOffset = (rows * 0x160) + 0x1F0;
  box.bottom = top + yOffset;
  frameBase = sheet->frameBase;
  box.left = left;
  box.top = top;
  func_0021B740(&box.left, sheet->entryCount, rows, frameBase);
}

INCLUDE_ASM("asm/main/nonmatchings/tu173", func_00220488);

s32 xglFontQueueHighlightPanel();

void sheetFrameDrawMarker(TiFrameSheet *sheet, s32 animationTick) {
    TiFramePos pos;

    if (sheetFramePosition(&pos, sheet, animationTick) != 0) {
        xglFontQueueHighlightPanel((sheet->originX + 0xFFC0) & 0xFFFF, (pos.y + 0x20) & 0xFFFF, 0, 0x80);
    }
}
