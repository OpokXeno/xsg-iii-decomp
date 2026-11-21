/*
 * OV02 original TU 51: 0x00a947e8..0x00a94c68 (3 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu051", func_00A947E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu051", func_00A949B0);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;

/* Battle entry header: the first slot number of the entry sits at +0x14. */
typedef struct BattleEntry {
    unsigned char unk_00[0x14];
    signed char firstItemSlotNumber;
} BattleEntry;

/* One item of an entry: its kind and count, the two bytes at the start of the item record. */
typedef struct BattleItem {
    unsigned char itemId;
    unsigned char quantity;
} BattleItem;

u8 battleCurrentRosterEntryCountGet(void);
s32 battleCurrentRosterEntryIdGet(s32);
s32 *advanceBlockGet();
s32 func_00A85160(u8 itemId, u8 quantity);
s32 *sceneStateGet(s32);
s32 func_00A87E08(s32, s32);
BattleItem *func_00A88248(s32, s32);
void ov02AddRecordPoints(s32, s32);
BattleEntry *battleEntrySlot(s32);

void battleAwardEntryItems(void)
{
  s32 entry_id;
  s32 awardedItemCount;
  s32 itemAwardBase;
  s32 remaining;
  s32 nextItemSlotNumber;
  s32 awardIndex;
  s32 recordPointAward;
  s32 itemSlotNumber;
  s32 firstItemSlotNumber;
  s32 *awardStats;
  BattleItem *awardedItem;
  if ((*advanceBlockGet()) != 2)
  {
    awardIndex = 0;
    awardStats = sceneStateGet(2);
    while (awardIndex < battleCurrentRosterEntryCountGet())
    {
      entry_id = battleCurrentRosterEntryIdGet(awardIndex);
      firstItemSlotNumber = battleEntrySlot(entry_id)->firstItemSlotNumber;
      if (awardIndex >= 3)
      {
        itemAwardBase = (awardStats[0] * 0x50) / 100;
        recordPointAward = (awardStats[1] * 0x50) / 100;
      }
      else
      {
        itemAwardBase = awardStats[0];
        recordPointAward = awardStats[1];
      }
      ov02AddRecordPoints(entry_id, recordPointAward);
      awardedItemCount = func_00A87E08(entry_id, itemAwardBase);
      if (awardedItemCount > 0)
      {
        nextItemSlotNumber = firstItemSlotNumber + 1;
        remaining = awardedItemCount;
        do
        {
          itemSlotNumber = nextItemSlotNumber;
          nextItemSlotNumber += 1;
          awardedItem = func_00A88248(entry_id, itemSlotNumber);
          if (awardedItem != 0)
          {
            func_00A85160(awardedItem->itemId, awardedItem->quantity);
          }
          remaining -= 1;
          itemSlotNumber = nextItemSlotNumber;
        }
        while (remaining != 0);
      }
      awardIndex += 1;
    }

  }
}
