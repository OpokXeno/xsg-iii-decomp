#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
typedef float f32;
extern unsigned char gBattleRecordBuffer[];
extern unsigned char gBattleRecordBufferBaseStorage[];
/* The word at gBattleRecordBufferBaseStorage: the record base address. */
#define gBattleRecordBufferBase (*(f32 **) gBattleRecordBufferBaseStorage)
extern f32 *gBattleRecordCursor;
void *memset(void *s, s32 c, u32 n);

void battleRecordReset(void) {
    memset(gBattleRecordBuffer, 0, 0x180);
    gBattleRecordCursor = gBattleRecordBufferBase;
}

s32 recordBufferIsFull(void)
{
  f32 *cursor;
  f32 *threshold;
  cursor = gBattleRecordCursor;
  threshold = gBattleRecordBufferBase + 0x16C / sizeof(f32);
  if (cursor < threshold)
  {
    return 0;
  }
  return 1;
}

s32 battleSetActorSlots(s32 first, s32 second);
s32 vmArgGet(s32 vm, s32 index);

s32 vmOpCallWithTwoArgs(s32 vm) {
    s32 actorSlotA;
    s32 actorSlotB;

    actorSlotA = vmArgGet(vm, 1);
    actorSlotB = vmArgGet(vm, 2);
    battleSetActorSlots(actorSlotA, actorSlotB);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025DBD8);

extern s32 gBattleRecordPayloadWords[];

f32 *recordAdvance(f32 *recordCursor) {
    f32 recordTypeId;

    recordTypeId = recordCursor[0];
    if (!(!(recordTypeId <= 0.0f) && !(recordTypeId >= 10.0f))) {
        return recordCursor;
    }
    return recordCursor + gBattleRecordPayloadWords[(s32) recordTypeId] + 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025DCE0);

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025DE40);

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025DFA0);

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025E028);

s32 recordBufferIsFull();
f32 *recordAdvance(f32 *record);

s32 vmOpRecordAppend(s32 vm)
{
  f32 appendedValue;
  f32 *cursor;
  if (recordBufferIsFull() == 0)
  {
    *gBattleRecordCursor = 1.0f;
    appendedValue = (f32) vmArgGet(vm, 1);
    cursor = gBattleRecordCursor;
    cursor[1] = appendedValue;
    gBattleRecordCursor = recordAdvance(cursor);
  }
  do { return 1; } while (0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025E130);

s32 textEndScene();

s32 vmOpBattleStep(void) {
    textEndScene();
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu242", func_0025E340);
