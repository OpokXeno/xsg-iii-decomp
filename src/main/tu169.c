#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_00219C30);

typedef int s32;
typedef short s16;
typedef unsigned short u16;
typedef unsigned int u32;
s32 func_002146C0();
s32 func_0021EA68(s32, s32, s32, s32);
s32 func_00A84A90(s32, s32);
s32 func_00A84B08(s32, s32);
s32 func_00A84B78(s32, s32);
s32 func_00A84C68(s32, s32);

void lookupEntryByKind(s32 context, s32 tag, s32 lookupTableId, s32 lookupEntryKey) {
    s32 foundEntryId;

    foundEntryId = 0;
    switch (lookupTableId) {                                 /* irregular */
    case 2:
        foundEntryId = func_00A84A90(lookupEntryKey, 1);
        break;
    case 1:
        foundEntryId = func_00A84B08(lookupEntryKey, 1);
        break;
    case 0:
        foundEntryId = func_00A84C68(lookupEntryKey, 1);
        break;
    case 3:
        foundEntryId = func_00A84B78(lookupEntryKey, 1);
        break;
    }
    func_002146C0(0x80626262);
    func_0021EA68(context, tag, foundEntryId, 0xE0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021A508);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021A670);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021A7E0);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021A990);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021ABB0);

extern s32 D_00550C74;

void setPendingValue(s32 value) {
    D_00550C74 = value;
}

s32 func_0021ABB0(s32, s32, s32, s32);

void flushPendingValue(void) {
    if (D_00550C74 != 0) {
        func_0021ABB0(0, 0xB, D_00550C74, 0);
    }
}

s32 func_00214FA8();
s32 func_00215E60();
s32 func_00215FD0();
s32 func_0023DA40();

/* A screen rectangle in 16-bit units: origin x/y and size. */
typedef struct ScreenRect {
    u16 x;          /* 0x0 */
    u16 y;          /* 0x2 */
    u16 width;      /* 0x4 */
    u16 height;     /* 0x6 */
} ScreenRect;

void rectDrawLevelFrame(ScreenRect *rect, s32 level, s32 mode)
{
  ScreenRect rectCopy;
  s32 shiftedX;
  u16 rectY;
  u16 rectWidth;
  u16 rectHeight;
  u16 rectX;
  u16 left;
  u16 top;
  u32 levelByte;
  levelByte = level & 0xFF;
  rectY = rect->y;
  rectWidth = rect->width;
  rectHeight = rect->height;
  shiftedX = rect->x + 0x100;
  rectCopy.x = shiftedX;
  rectCopy.y = rectY;
  rectCopy.width = rectWidth;
  rectCopy.height = rectHeight;
  func_00215FD0(&rectCopy, levelByte >> 1, ((mode != 0) ? (0xF98980) : (0xA8B234)));
  rectX = rect->x;
  rectCopy.x = rectX;
  rectCopy.width = rectX + 0xB0;
  func_002146C0(0x73514442);
  func_00215E60(&rectCopy, 2, 0x734B3E3E, 1);
  func_002146C0(0x80000000);
  left = rect->x;
  top = rect->y;
  func_00214FA8((left + 0xB0) & 0xFFFF, top, (left + 0x110) & 0xFFFF, (top + 0x190) & 0xFFFF);
  func_0023DA40((rect->x + 0xC0) & 0xFFFF, (rect->y + 0x30) & 0xFFFF, mode, levelByte);
}

void rectDrawLevelFrameFixed(ScreenRect *rect, s32 level, s32 mode)
{
  ScreenRect rectCopy;
  s32 shiftedX;
  u16 rectHeight;
  u16 rectWidth;
  u16 rectY;
  u16 rectX;
  u16 left;
  u16 top;
  u32 levelByte;
  rectY = rect->y;
  levelByte = level & 0xFF;
  rectWidth = rect->width;
  rectHeight = rect->height;
  shiftedX = rect->x + 0x100;
  rectCopy.x = shiftedX;
  rectCopy.y = rectY;
  rectCopy.width = rectWidth;
  rectCopy.height = rectHeight;
  func_00215FD0(&rectCopy, levelByte >> 1, ((mode != 0) ? (0xF98980) : (0xA8B234)));
  rectX = rect->x;
  rectCopy.x = rectX;
  rectCopy.width = rectX + 0xB0;
  func_002146C0(0x73514442);
  func_00215E60(&rectCopy, 2, 0x734B3E3E, 1);
  func_002146C0(0x80000000);
  left = rect->x;
  top = rect->y;
  func_00214FA8((left + 0xB0) & 0xFFFF, top, (left + 0x110) & 0xFFFF, (top + 0x190) & 0xFFFF);
  func_0023DA40((rect->x + 0xC0) & 0xFFFF, (rect->y + 0x30) & 0xFFFF, mode, 0x80);
}

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021AF70);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021B518);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021B740);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021B948);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021BA80);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021BB68);

extern signed char D_00550C78;

void setPendingByte(signed char value) {
    D_00550C78 = value;
}

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021BD10);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021C5E8);

s32 func_0021BD10();
s32 func_0021C5E8(s32, s32, s32, s32);

void dispatchCodeRange(s32 targetId, u32 messageCode, s32 value, s32 messageFlags) {
    if (messageCode - 1 < 0xCU) {
        func_0021BD10();
    }
    if (messageCode - 0xD >= 5U) {
        return;
    }
    func_0021C5E8(targetId, messageCode, value, messageFlags);
}

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021CB40);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021CB90);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021CFF0);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021D420);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021D620);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021D968);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021DA38);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021DF40);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021E390);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021E9A8);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021EA68);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021EBD0);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021ED38);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021EE90);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021F028);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021F260);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021F368);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021F3D0);

INCLUDE_ASM("asm/main/nonmatchings/tu169", func_0021F6E0);
