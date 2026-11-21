/*
 * OV02 original TU 0: 0x00a80000..0x00a81558 (33 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef short s16;

typedef struct AdvanceBlock {
    s32 advanceStatus;
    unsigned char unk_04[0x4];
    s32 pending;
    s32 advanceState;
    unsigned char unk_10[0x2C];
} AdvanceBlock;

typedef struct AdvanceEntry {
    unsigned char unk_00[0xC];
    s32 advanceFlags;
} AdvanceEntry;

typedef struct ActorLevelStats {
    unsigned char unk_00[0x3E];
    s16 level;
} ActorLevelStats;

typedef struct ActorLevelOwner {
    unsigned char unk_000[0x2B0];
    ActorLevelStats *levelStats;
} ActorLevelOwner;

extern AdvanceBlock gAdvanceBlock;
void *memset(void *, s32, s32);

void advanceInit(void) {
    memset(&gAdvanceBlock, 0, 0x3C);
    gAdvanceBlock.pending = 1;
    gAdvanceBlock.advanceState = 0;
}

AdvanceBlock *advanceBlockGet(void) {
    return &gAdvanceBlock;
}

s32 func_00A800A8();
s32 func_00A80B98();
void contextAdvanceState();
s32 func_00A81220();

void advanceUpdate(void)
{
  func_00A80B98();
  if (gAdvanceBlock.pending != 0)
  {
    func_00A800A8();
  }
  if (gAdvanceBlock.advanceState != 0)
  {
    contextAdvanceState();
  }
  func_00A81220();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A800A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80120);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A801D8);

extern AdvanceEntry *gAdvanceCurrentEntry;
s32 func_00A802E0(void *);
s32 func_00A80318(void *);
s32 func_00A80360(void *);
s32 func_00A80418(void *);
s32 func_00A819D8(void *);
s32 func_00A8C4F8(s32);

void advanceRunCurrentEntry(void) {
    AdvanceEntry *currentAdvanceEntry;

    currentAdvanceEntry = gAdvanceCurrentEntry;
    if (func_00A819D8(currentAdvanceEntry) != 0) {
        func_00A802E0(currentAdvanceEntry);
        return;
    }
    if (currentAdvanceEntry->advanceFlags & 4) {
        if (func_00A80360(currentAdvanceEntry) != 0) {
            func_00A80318(currentAdvanceEntry);
        }
    } else if (!(func_00A8C4F8(0) & 0xA) && (func_00A80418(currentAdvanceEntry) != 0)) {
        func_00A80318(currentAdvanceEntry);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A802E0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80318);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80360);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80418);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80B98);

s32 func_00A854D8(void *, s32, s32);
s32 vpadBindLookupKey(void *, s32, s32);
s32 battleUnitTableStore();
s32 seVoiceStartDefaultCall(s32);

void actorApplyLevelMode(ActorLevelOwner *levelOwner, s32 vpadBindMode)
{
  s32 levelValue;
  int isBelowLevelLimit;
  s32 isAtOrAboveLevelLimit;
  levelValue = levelOwner->levelStats->level;
  isBelowLevelLimit = levelValue < 0x12;
  isAtOrAboveLevelLimit = isBelowLevelLimit == 0;
  battleUnitTableStore();
  vpadBindLookupKey(levelOwner, isAtOrAboveLevelLimit, vpadBindMode);
  func_00A854D8(levelOwner, 0x136, 0x12);
  seVoiceStartDefaultCall(0x20);
}

extern s32 gAdvancePhase;
extern s32 D_00ABCD0C;
void battleUnitVPadEnable();

void advanceReset(void) {
    battleUnitVPadEnable();
    gAdvancePhase = 0;
    D_00ABCD0C = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80D30);

void contextSetPhaseNine(void) {
    gAdvancePhase = 9;
}

void advanceSetPhaseOne(void) {
    gAdvancePhase = 1;
}

void advanceSetPhaseTwo(void) {
    gAdvancePhase = 2;
}

void advanceSetPhaseThree(void) {
    gAdvancePhase = 3;
}

void contextSetPhaseFive(void) {
    gAdvancePhase = 5;
}

void contextSetPhaseFour(void) {
    gAdvancePhase = 4;
}

void contextSetPhaseSix(void) {
    gAdvancePhase = 6;
}

void contextSetPhaseSeven(void) {
    gAdvancePhase = 7;
}

void contextSetPhaseEight(void) {
    gAdvancePhase = 8;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A80F60);

void contextStartAdvance();
s32 contextPollAdvance();
s32 func_00A811B8();

void contextAdvanceState(void)
{
  AdvanceBlock *advanceBlock = &gAdvanceBlock;
  AdvanceBlock **advanceBlockPtr;
  advanceBlockPtr = &advanceBlock;
  switch (advanceBlock->advanceState)
  {
    case 1:
      contextStartAdvance();
      (*advanceBlockPtr)->advanceState = 2;

    case 2:
      if (contextPollAdvance() != 0)
    {
      case 3:
        func_00A811B8();

    }
      return;

  }

}

s32 func_00A8FED0();

void contextStartAdvance(void) {
    func_00A8FED0();
}

extern s32 gAdvancePollResult;
s32 monsSlotQueueEmpty();
s32 func_00A91400();

s32 contextPollAdvance(void) {
    if (func_00A91400() == 0) {
        if (monsSlotQueueEmpty() != 0) {
            gAdvancePollResult = 3;
            return 1;
        }
        gAdvancePollResult = 1;
        goto done;
    }
done:
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A811B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A81220);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A81248);

INCLUDE_ASM("asm/nonmatchings/ov02/tu000", func_00A812E0);

s32 advanceStatusGet(void) {
    return gAdvanceBlock.advanceStatus;
}
