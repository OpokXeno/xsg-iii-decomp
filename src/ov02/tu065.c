/*
 * OV02 original TU 65: 0x00a9d888..0x00a9e668 (31 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
typedef short s16;
typedef unsigned int u32;
typedef unsigned char u8;

/* One sound slot record: 0x90 bytes. gSoundSlots is the slot table; gSoundSlotTagStore, gSoundSlotTagLoad and
 * D_00AC5488 are tables of the same stride whose word at +0x00 is `tag`. */
#define SOUND_SLOT_SIZE 0x90

typedef struct SoundSlotRecord {
    s32 tag;                      /* +0x00: the word of the per-slot tables */
    u8 unk_04[0x4C];
    s32 inUse;                   /* +0x50 */
    s32 unk_54;                   /* +0x54: cleared by soundSlotEnable */
    u8 unk_58[0x10];
    s32 enabled;                  /* +0x68: set by soundSlotEnable */
    s32 playState;                    /* +0x6C: 2 and 3 count as finished (soundSlotIsFinished) */
    s32 timer;                    /* +0x70: compared with the limit while playState is 1 */
    u8 unk_74[SOUND_SLOT_SIZE - 0x74];
} SoundSlotRecord;

/* The same memory as gSoundSlots read as one block: the fields after the slot records. */
typedef struct SoundTable {
    u8 unk_00[0x58];
    s32 unk_58;
    u8 unk_5C[0xE8 - 0x5C];
    s32 unk_E8;
    u8 unk_EC[0x124 - 0xEC];
    s32 startConfirmState;              /* +0x124: 1 becomes 2 in soundConfirmStart */
    s32 unk_128;                  /* +0x128: cleared by soundStateReset */
    s32 sampleAverage;            /* +0x12C: written by soundAverageSamples */
    s32 paused;                   /* +0x130: written by soundSetPaused */
} SoundTable;

extern SoundSlotRecord gSoundSlots[];
#define gSoundTable (*(SoundTable *) gSoundSlots)

void soundSlotSetState(SoundSlotRecord *soundSlot, s32 newPlayState) {
    soundSlot->timer = 0;
    soundSlot->playState = newPlayState;
}

void soundAverageSamples(s32 unused_arg0, s32 unused_arg1, s16 *samples)
{
  SoundTable *soundTable;
  s16 *sampleCursor;
  s16 runningAverage;
  s16 sampleValue;
  s32 sampleIndex;
  u32 sampleSum;
  s32 scaledSum;
  soundTable = &gSoundTable;
  runningAverage = samples[0];
  sampleCursor = samples + 1;
  for (sampleIndex = 0; sampleIndex < 7; sampleIndex++)
  {
    sampleValue = *sampleCursor;
    sampleCursor += 1;
    sampleSum = runningAverage + sampleValue;
    scaledSum = (sampleSum + (sampleSum >> 0x1F)) << 0xF;
    runningAverage = scaledSum >> 0x10;
  }

  soundTable->sampleAverage = runningAverage;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9D8E0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9D978);

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9D9A8);

void soundConfirmStart(void)
{
  if (gSoundTable.startConfirmState == 1)
  {
    gSoundTable.startConfirmState = 2;
  }
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9DA28);

void soundSlotSetPause(s32 soundSlotNumber, s32 value);
s32 soundSlotStatus(s32 index);

void soundSetPaused(s32 paused)
{
  SoundTable *soundTable = &gSoundTable;
  if (soundSlotStatus(1) != 0)
  {
    if (paused != 0)
    {
      soundSlotSetPause(1, 1);
    }
    else
    {
      soundSlotSetPause(1, 0);
    }
  }
  soundTable->paused = paused;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9DD70);

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9DFB0);

extern s32 gMoviePlayFormat;
extern s32 gMoviePlayBuffer;
s32 sprintf(s32 *destination, s32 *format, s32 trackNumber);
s32 soundSlotStartNumbered(s32 soundSlotNumber, s32 *buffer);

void soundPlayMovieTrack(s32 *trackNumber) {
    if (trackNumber != 0) {
        sprintf(&gMoviePlayBuffer, &gMoviePlayFormat, *trackNumber);
        soundSlotStartNumbered(1, &gMoviePlayBuffer);
    }
}

void soundSlotStop(s32 soundSlotNumber);

void soundStopPrimarySlot(void) {
    soundSlotStop(1);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9E1F8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9E220);

void soundSlotEnable(SoundSlotRecord *soundSlot) {
    soundSlot->unk_54 = 0;
    soundSlot->enabled = 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9E2B8);

void caption_begin(s8 *text);
s32 func_00A9E5F8(s32 flag);

void captionTableReset(void)
{
  gSoundTable.startConfirmState = 0;
  gSoundTable.unk_128 = 0;
  func_00A9E5F8(0);
  caption_begin(0);
}

void soundStopPrimarySlot();

void soundSlotRequestStop(s32 soundSlotNumber) {
    SoundSlotRecord *soundSlot;

    soundSlot = &gSoundSlots[soundSlotNumber];
    if ((soundSlot->inUse != 0) && (soundSlot->playState != 2)) {
        soundStopPrimarySlot();
        soundSlotSetState(soundSlot, 2);
    }
}

extern SoundSlotRecord gSoundSlotTagStore[];

void soundSlotSetTag(s32 soundSlotNumber, s32 tag) {
    gSoundSlotTagStore[soundSlotNumber].tag = tag;
}

void soundSlotRelease(s32 soundSlotNumber) {
    SoundSlotRecord *soundSlot;

    soundSlot = &gSoundSlots[soundSlotNumber];
    soundSlotSetState(soundSlot, 3);
    soundSlot->inUse = 0;
}

extern SoundSlotRecord gSoundSlotTagLoad[];

s32 soundSlotGetTag(s32 soundSlotNumber) {
    return gSoundSlotTagLoad[soundSlotNumber].tag;
}

s32 soundSlotIsFinished(s32 soundSlotNumber, s32 timerLimit) {
    s32 playState;
    u32 stateMinusTwo;
    SoundSlotRecord *soundSlot;

    soundSlot = &gSoundSlots[soundSlotNumber];
    playState = soundSlot->playState;
    stateMinusTwo = playState - 2;
    if ((stateMinusTwo < 2U) || ((playState == 1) && (soundSlot->timer >= timerLimit))) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9E4D8);

extern SoundSlotRecord D_00AC5488[];

void soundSlotSetTagB(s32 soundSlotNumber, s32 tag) {
    D_00AC5488[soundSlotNumber].tag = tag;
}

void soundResetPlayback(void)
{
  gSoundTable.unk_58 = 0;
  gSoundTable.unk_E8 = 0;
  soundStopPrimarySlot();
}

s32 func_00A9E5A8(s32 flag);
void soundFlagClear();

void soundGroupOnClearFlag(void) {
    func_00A9E5A8(1);
    soundFlagClear();
}

void soundFlagSet();

void soundGroupOffSetFlag(void) {
    func_00A9E5F8(1);
    soundFlagSet();
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9E5A8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu065", func_00A9E5F8);

extern s32 gSoundFlag;

void soundFlagSet(void) {
    gSoundFlag = 1;
}

void soundFlagClear(void) {
    gSoundFlag = 0;
}
