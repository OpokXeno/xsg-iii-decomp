#include "common.h"

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;

/* The second scratch buffer starts this many bytes after the base address held in gEffectScratchBase. */
#define EFFECT_RECORD_BUFFER_OFFSET 0x100

/* One 16-byte record of the list at +0x2D4; byte 0x0F is cleared when a record is appended. */
typedef struct EffectRecordHead {
    u8 assetName[0xF];
    s8 nameTerminator;
} EffectRecordHead;

/* The lists the three append helpers below fill (struct size 0x304, the memset in effectListsClear). */
typedef struct EffectLookupLists {
    u8 unk_000[0x1C8];
    s32 matchIdCount;             /* +0x1C8 */
    s32 matchIds[0x20];          /* +0x1CC */
    s32 effectIndexCount;             /* +0x24C */
    s32 effectIndexes[0x20];           /* +0x250 */
    s32 effectRecordCount;            /* +0x2D0 */
    EffectRecordHead effectRecords[3];    /* +0x2D4 */
} EffectLookupLists;

void effectListsClear(EffectLookupLists *effectLists) {
    memset(effectLists, 0, 0x304);
}

INCLUDE_ASM("asm/main/nonmatchings/tu058", func_001B7CA8);

INCLUDE_ASM("asm/main/nonmatchings/tu058", func_001B7DE0);

INCLUDE_ASM("asm/main/nonmatchings/tu058", func_001B7EC8);

INCLUDE_ASM("asm/main/nonmatchings/tu058", func_001B80B8);

INCLUDE_ASM("asm/main/nonmatchings/tu058", func_001B8208);

extern unsigned char gEffectRecordTable[];
extern u8 *gEffectScratchBase;
s32 func_001B6CA8(void *table, s32 key);
s32 func_0030E250(s32 id, void *table, u8 *base, u8 *dest);

void lookupListsAddMatch(EffectLookupLists *effectLists, s32 id)
{
  s32 *matchCountSlot;
  s32 matchIndex;
  s32 nextMatchCount;
  s32 matchId;

  func_0030E250(id, gEffectRecordTable, gEffectScratchBase, gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET);
  if ((effectLists->matchIdCount < 0x20) && (((matchId = func_001B6CA8(gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET, 0x03000000)), matchId != 0) || ((matchId = func_001B6CA8(gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET, 0x03010000)), matchId != 0)))
  {
    matchCountSlot = &effectLists->matchIdCount;
    matchIndex = *matchCountSlot;
    nextMatchCount = matchIndex + 1;
    effectLists->matchIds[matchIndex] = matchId;
    *matchCountSlot = nextMatchCount;
  }
}

s32 srsParseEffectFileName(void *dest);

void effectListsAddEffectIndex(EffectLookupLists *effectLists, s32 id)
{
  s32 *effectIndexCountSlot;
  s32 effectIndex;
  s32 appendIndex;
  s32 nextEffectIndexCount;

  func_0030E250(id, gEffectRecordTable, gEffectScratchBase, gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET);
  if (effectLists->effectIndexCount < 0x20)
  {
    effectIndex = srsParseEffectFileName(gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET);
    appendIndex = *(effectIndexCountSlot = &effectLists->effectIndexCount);
    nextEffectIndexCount = appendIndex + 1;
    effectLists->effectIndexes[appendIndex] = effectIndex;
    *effectIndexCountSlot = nextEffectIndexCount;
  }
}

void func_0030ED88(void *dest, void *source, s32 size);

void effectListsAddRecord(EffectLookupLists *effectLists, s32 id)
{
  s32 *effectRecordCountSlot;
  s32 currentEffectRecordCount;
  s32 appendIndex;
  s32 nextEffectRecordCount;

  func_0030E250(id, gEffectRecordTable, gEffectScratchBase, gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET);
  currentEffectRecordCount = effectLists->effectRecordCount;
  if (currentEffectRecordCount <= 0)
  {
    func_0030ED88(&effectLists->effectRecords[currentEffectRecordCount], gEffectScratchBase + EFFECT_RECORD_BUFFER_OFFSET, 0x10);
    effectRecordCountSlot = &effectLists->effectRecordCount;
    appendIndex = *effectRecordCountSlot;
    nextEffectRecordCount = appendIndex + 1;
    effectLists->effectRecords[appendIndex].nameTerminator = 0;
    *effectRecordCountSlot = nextEffectRecordCount;
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu058", func_001B8438);
