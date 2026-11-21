#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu209", func_0024E348);

INCLUDE_ASM("asm/main/nonmatchings/tu209", func_0024E380);

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

/* A table whose first record sits at actorTable + firstRecordDelta (actorRecordsEnd). */
typedef struct ActorRecordTable {
    u8 unk_00[0x4C];
    s32 firstRecordDelta;          /* +0x4C */
} ActorRecordTable;

/* One 0xD4-byte record; variableDataSize is added to the end of the record to reach the next one. */
typedef struct ActorRecord {
    u8 unk_00[0x90];
    s32 variableDataSize;             /* +0x90 */
    u8 unk_94[0x40];
} ActorRecord;

/* A table whose first slot sits at base + firstSlotDelta (tiSlotTableEnd). */
typedef struct TiSlotTable {
    u8 unk_00[0x14];
    s32 firstSlotDelta;          /* +0x14 */
} TiSlotTable;

/* One 0x20-byte slot; variableDataSize is added to the end of the slot to reach the next one. */
typedef struct TiSlot {
    u8 unk_00[0x10];
    s32 variableDataSize;             /* +0x10 */
    u8 unk_14[0x0C];
} TiSlot;

u8 *actorRecordsEnd(ActorRecordTable *actorTable, u32 recordSteps)
{
  s32 stride;
  u32 i;
  u8 *cursor;
  i = 0;
  cursor = (u8 *) actorTable + actorTable->firstRecordDelta;
  if (recordSteps != 0)
  {
    stride = 0xD4;
    do
    {
      i += 1;
      cursor = (cursor + stride) + ((ActorRecord *) cursor)->variableDataSize;
    }
    while (i < recordSteps);
  }
  return cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/tu209", func_0024E3F0);

u8 *tiSlotTableEnd(TiSlotTable *base, s32 unused_arg1, u32 slotSteps)
{
  u32 i;
  u8 *cursor;
  s32 stride;
  i = 0;
  cursor = (u8 *) base + base->firstSlotDelta;
  if (slotSteps != 0)
  {
    stride = 0x20;
    do
    {
      i += 1;
      cursor = (cursor + stride) + ((TiSlot *) cursor)->variableDataSize;
    }
    while (i < slotSteps);
  }
  return cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/tu209", func_0024E460);

INCLUDE_ASM("asm/main/nonmatchings/tu209", func_0024E4A8);
