#include "common.h"

typedef int s32;
typedef float f32;

/* One 0xD0-byte effect slot of the table at effectSlotTable (24 entries; the caller of effectSlotRelease tests active). */
typedef struct EffectSlot {
    s32 active;                     /* +0x00: 1 while the slot is live */
    unsigned char unk_04[0xC];
    s32 tagFlags;                   /* +0x10: bit 0 is tested by the caller; stepStateResetToPending sets 0x800 */
    s32 tagRef;                     /* +0x14: cleared by stepStateResetToPending */
    unsigned char unk_18[0xC0 - 0x18];
    s32 effectKind;                 /* +0xC0 */
    f32 level;                      /* +0xC4 */
    f32 alpha;                      /* +0xC8 */
    f32 unk_CC;                     /* +0xCC: 1000.0f on start */
} EffectSlot;

s32 rgDispModelGetActorId(s32);

void effectSlotStart(EffectSlot *effectSlot, s32 effectKind)
{
  EffectSlot **slotAlias;
  effectSlot->effectKind = effectKind;
  slotAlias = &effectSlot;
  (*slotAlias)->active = 1;
  (*slotAlias)->level = 5.0f;
  effectSlot->alpha = 1.0f;
  (*slotAlias)->unk_CC = 1000.0f;
  rgDispModelGetActorId(effectKind);
}

INCLUDE_ASM("asm/main/nonmatchings/tu268", func_00268070);

s32 stepStateResetToPending(void *);
void *memset(void *destination, s32 value, unsigned int size);

void effectSlotRelease(EffectSlot *effectSlot) {
    stepStateResetToPending(&effectSlot->tagFlags);
    memset(effectSlot, 0, 0xD0);
}

void effectSlotSetLevel(EffectSlot *effectSlot, f32 newLevel) {
    effectSlot->level = newLevel;
}

void effectSlotSetAlpha(EffectSlot *effectSlot, f32 newAlpha) {
    effectSlot->alpha = newAlpha;
}

f32 effectPulseCurve(f32 pulsePhase) {
    f32 rampValue;

    rampValue = (pulsePhase * 1.5f) - 0.25f;
    if (rampValue <= 0.0f) {
        rampValue = 0.0f;
    }
    if (rampValue >= 1.0f) {
        rampValue = 1.0f;
    }
    return (2.0f * (1.0f - (rampValue * rampValue))) - 1.0f;
}

INCLUDE_ASM("asm/main/nonmatchings/tu268", func_00268408);

INCLUDE_ASM("asm/main/nonmatchings/tu268", func_00268488);

INCLUDE_ASM("asm/main/nonmatchings/tu268", func_002684B8);
