/*
 * OV02 original TU 4: 0x00a837c0..0x00a839a8 (5 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu004", func_00A837C0);

#include "main/xenoActor.h"
#include "ov02/tu060.h"

typedef signed char s8;
typedef int s32;

/* The part of a battle unit's stats block read here: the free-slot count at +0x3C. */
struct BattleStats {
    unsigned char unmodeled_000[0x3C];
    short freeSlotsLeft;                /* +0x3C */
};

/* One slot record returned by the ov02ItemEntryGet family; only the flags byte is read here. */
typedef struct BattleSlot {
    unsigned char unmodeled_00[6];
    unsigned char usableFlags;          /* +0x06: bit 0x80 makes the usable count threshold one higher */
} BattleSlot;

s32 entryCostGet();
s32 func_00A837C0();
BattleSlot *ov02ItemEntryGet(s32);

s32 unitSlotUsable(BattleUnit *battleUnit, s32 itemNumber)
{
  BattleSlot *itemSlotDetail;
  s32 slotUsableFlags;
  s32 flagMask;
  itemSlotDetail = ov02ItemEntryGet(itemNumber);
  if (!(battleUnit->stats->freeSlotsLeft >= entryCostGet(battleUnit, itemSlotDetail)))
  {
    return 0;
  }
  slotUsableFlags = itemSlotDetail->usableFlags;
  flagMask = 0x80;
  return func_00A837C0(battleUnit, itemSlotDetail) > ((slotUsableFlags & flagMask) != 0);
}

BattleSlot *ov02SlotEntryGet(s32);

s32 unitRecordSlotUsable(BattleUnit *battleUnit, s32 slotNumber)
{
    BattleSlot *slotDetail;

    slotDetail = ov02SlotEntryGet(slotNumber);
    if (!(battleUnit->stats->freeSlotsLeft >= entryCostGet(battleUnit, slotDetail))) {
        return 0;
    }
    return func_00A837C0(battleUnit, slotDetail) > 0;
}

/* The part of a boost gauge read here: the current stock at +0x01 (boost_gauge.c owns the full record). */
struct BoostGauge {
    unsigned char unmodeled_00;
    signed char stock;                  /* +0x01 */
};

struct BoostGauge *func_00A815D8(BattleUnit *battleUnit);
BattleSlot *ov02ActionEntryGet(s32);

s32 unitStockSlotUsable(BattleUnit *battleUnit, s32 actionNumber)
{
    BattleSlot *actionSlotDetail;
    s32 boostStock;

    actionSlotDetail = ov02ActionEntryGet(actionNumber);
    boostStock = func_00A815D8(battleUnit)->stock;
    if (!(boostStock >= entryCostGet(battleUnit, actionSlotDetail))) {
        return 0;
    }
    return func_00A837C0(battleUnit, actionSlotDetail) > 0;
}

BattleSlot *ov02BattleSlotGet(s32 index);

s32 unitStatUsable(BattleUnit *battleUnit, s32 statNumber)
{
    BattleSlot *statSlotDetail;

    statSlotDetail = ov02BattleSlotGet(statNumber);
    if (func_00A837C0(battleUnit, statSlotDetail) <= 0) {
        return 0;
    }
    return func_00A837C0(battleUnit, statSlotDetail) > 0;
}
