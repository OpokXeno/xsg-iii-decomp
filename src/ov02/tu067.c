/*
 * OV02 original TU 67: 0x00a9e738..0x00a9ff28 (23 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9E738);

typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
typedef unsigned short u16;

/* Word 0 of a sound record: the base block id (sbBaseBlockIdGet), or its bytes. */
typedef union SoundRecordHead {
    s32 baseBlockId;
    u8 headerByteArray[4];             /* bytes[3] is the record's table index */
} SoundRecordHead;

/* Word 4 of a sound record: a gain as a float, or the raw word, or its halfwords. */
typedef union SoundRecordGain {
    s32 rawBits;
    f32 gain;
    u16 halves[2];             /* half[1] is read as the table index */
} SoundRecordGain;

/* One sound record (0x130 bytes): fields used by ov02 tu067. */
typedef struct SoundRecord {
    SoundRecordHead head;    /* 0x00 */
    SoundRecordGain gainOrIndex; /* 0x04 */
    s32 playbackMode;                /* 0x08 */
    s32 playbackFlags;               /* 0x0C */
    s32 priority;            /* 0x10 */
    f32 volume;              /* 0x14 */
    u8 unk_18[0xFC];
    u8 *remaining;           /* 0x114: byte counter, read through the pointer */
    u8 unk_118[8];
    f32 threshold;           /* 0x120 */
    s32 playbackState;               /* 0x124 */
    s32 active;              /* 0x128 */
    s32 averageLevel;               /* 0x12C: the running average written by tu065 */
} SoundRecord;

s32 sbBaseBlockIdGet(void);
void *memset(void *dest, s32 value, s32 size);
s32 soundRecordGetLevel(SoundRecord *soundRecord);
extern char gSoundRecordAssertFile[];
extern char gSoundRecordAssertMessage[];
void assertFailed(const char *message, const char *file, int line);

void soundRecordInit(SoundRecord *soundRecord, s32 playbackMode, s32 gain, s32 playbackFlags, s32 priority, f32 volume) {
    s32 baseBlockId;

    memset(soundRecord, 0, 0x18);
    baseBlockId = sbBaseBlockIdGet();
    soundRecord->volume = volume;
    soundRecord->gainOrIndex.rawBits = gain;
    soundRecord->playbackMode = playbackMode;
    soundRecord->playbackFlags = playbackFlags;
    soundRecord->priority = priority;
    soundRecord->head.baseBlockId = baseBlockId;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9E830);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9EBC8);

s32 soundRecordIsActive(SoundRecord *soundRecord) {
    return soundRecord->active;
}

void soundRecordActivate(SoundRecord *soundRecord) {
    soundRecord->active = 1;
    soundRecord->playbackState = 7;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9EC78);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9ED00);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9EF60);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9F190);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9F3D8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9F4F0);

void soundRecordSetState(SoundRecord *soundRecord, s32 playbackState) {
    soundRecord->playbackState = playbackState;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9F6D0);

s32 soundRecordGetLevel(SoundRecord *soundRecord) {
    return soundRecord->averageLevel;
}

s32 soundRecordIsDrained(SoundRecord *soundRecord) {
    if (soundRecordGetLevel(soundRecord) == 0) {
        assertFailed(gSoundRecordAssertMessage, gSoundRecordAssertFile, 0x23D);
    }
    if (!(!(*soundRecord->remaining <= soundRecord->threshold))) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9F7E0);

s32 soundRecordClampGain(SoundRecord *soundRecord) {
    f32 gain;

    gain = soundRecord->gainOrIndex.gain;
    if (gain > 45.0f) {
        soundRecord->gainOrIndex.gain = 45.0f;
        return 1;
    }
    if (gain < 0.0f) {
        soundRecord->gainOrIndex.gain = 0.0f;
        return -1;
    }
    return 0;
}

/* The table is a run of 4-byte entries: the byte two before the entry at (index * 4). */
f32 soundRecordSampleAt(s8 *sampleTable, SoundRecord *soundRecord) {
    return (sampleTable + (soundRecord->gainOrIndex.halves[1] + soundRecord->head.headerByteArray[3]) * 4)[-2];
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9F8B8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9FC88);

s32 soundDeadbandSign(f32 deviation) {
    if (!(!(deviation < -3.0f))) {
        return -1;
    }
    if (!(!(deviation > 3.0f))) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu067", func_00A9FED8);
