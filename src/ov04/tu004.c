/*
 * OV04 original TU 4: 0x00a870a8..0x00a8deb0 (40 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A870A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A874F0);

typedef signed char s8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;

/* Screen mode control block at gScreenModeControlStorage (shared with tu005, tu006, tu016, tu020): reset values at 0xFE and 0x100, the cell value last seen by effectCoordToTableOffset at 0x114. */
typedef struct ScreenModeControl {
    u8 unk_00[0xFE];
    s16 unk_FE;
    s16 unk_100;
    u8 unk_102[0x12];
    s16 cachedCellEntry;
} ScreenModeControl;

/* Coordinate of one entry in the 2-byte table at gEntryCellTable: group (0x98 per group), row (0x13 per row), column. */
typedef struct EntryCoord {
    s16 cellGroupIndex;
    s16 rowIndex;
    s16 columnIndex;
} EntryCoord;

/* One 0xC0-byte transform of the pool; the fields 0x30..0x4C are reset by resetTransformPool. */
typedef struct TransformSlot {
    u8 unk_00[0x30];
    f32 unk_30;
    f32 unk_34;
    f32 unk_38;
    f32 unk_3C;
    f32 unk_40;
    f32 unk_44;
    f32 unk_48;
    f32 unk_4C;
    u8 unk_50[0x70];
} TransformSlot;

/* The transform pool at the pointer stored in gTransformPool: 0x540 bytes of header, then 270 transform slots. */
typedef struct TransformPool {
    u8 unk_000[0x540];
    TransformSlot transforms[270];
} TransformPool;

extern s16 *gEntryCellTable;
extern ScreenModeControl gScreenModeControlStorage[];
void effectRestartFromTable();
s32 func_00A9D718(s32, s32, s32);

s32 effectCoordToTableOffset(EntryCoord *coord)
{
  s16 *cellValueAt;
  s32 cached_value;
  s32 rowIndex;
  s32 columnIndex;
  s32 cellByteOffset;
  s32 cellValue;
  ScreenModeControl *screenControl;
  s32 coordEntryIndex;
  rowIndex = coord->rowIndex;
  screenControl = gScreenModeControlStorage;
  columnIndex = coord->columnIndex;
  cached_value = screenControl->cachedCellEntry;
  coordEntryIndex = ((rowIndex * 0x13) + ((coord->cellGroupIndex) * 0x98)) + columnIndex;
  cellByteOffset = coordEntryIndex * 2;
  cellValueAt = gEntryCellTable + coordEntryIndex;
  cellValue = (u16) (*cellValueAt);
  if (cached_value != (*cellValueAt))
  {
    screenControl->cachedCellEntry = cellValue;
    effectRestartFromTable();
    return func_00A9D718(-1, 0, 4);
  }
  return cellByteOffset;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A877B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A87930);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A87A20);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A87A98);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A88190);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A881C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A88550);

extern TransformPool *gTransformPool;

void resetTransformPool(void)
{
  ScreenModeControl *screenControl = gScreenModeControlStorage;
  float zeroScalar;
  int transformIndex;
  TransformSlot *transform;
  transform = gTransformPool->transforms;
  screenControl->unk_100 = 0;
  screenControl->unk_FE = 3;
  for (transformIndex = 0; transformIndex < 270; transformIndex++)
  {
    zeroScalar = 0.0f;
    transform->unk_48 = zeroScalar;
    transform->unk_40 = (transform->unk_44 = zeroScalar);
    transform->unk_4C = 1.0f;
    transform->unk_30 = 0.0f;
    transform->unk_38 = 0.0f;
    transform->unk_3C = 1.0f;
    transform += 1;
  }

}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A886B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A888C8);

extern u16 gEffectIdTable[];
void func_00AA30D0(s16, s32, s32, s32, s32, s32, s32, s32, f32, s32, s32);
void func_00AA3380();

void effectRestartFromTable(void) {
    s16 effectId;

    func_00AA3380(0x15);
    effectId = gEffectIdTable[0] + 0x200;
    func_00AA30D0(effectId, 0x15, 0x7C, 0x24, -1, -1, 0, 0, 1.0f, 0, 0x602F4F5F);
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A0A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A120);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A1F8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A2C0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A3F0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A500);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8A678);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AA60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AC60);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8ACA8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AD00);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8AD68);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8BEA0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C168);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C1D8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C408);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8C7A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8CB18);

extern s32 gEffectRecordBase;

/* Byte pair at 0x24 of the record that gEffectRecordBase + 2 * effectRecordId addresses. */
typedef struct EffectRecordBytes {
    u8 unk_00[0x24];
    u8 effectByteA;
    u8 effectByteB;
} EffectRecordBytes;

void copyEffectRecordPair(s32 effectRecordId, u8 *effectBytePairOut) {
    s32 effectRecordOffset;

    effectRecordOffset = (effectRecordId << 0x10) >> 0xF;
    effectBytePairOut[0] = ((EffectRecordBytes *) (effectRecordOffset + gEffectRecordBase))->effectByteA;
    effectBytePairOut[1] = ((EffectRecordBytes *) (effectRecordOffset + gEffectRecordBase))->effectByteB;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8CBB0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D1B0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D4A0);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D5A8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8D900);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8DA40);

INCLUDE_ASM("asm/nonmatchings/ov04/tu004", func_00A8DB70);
