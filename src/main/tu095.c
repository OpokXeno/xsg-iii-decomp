#include "common.h"
#include "shared.h"

typedef int s32;

void *soundCueCommand();
s32 vmArgGet(s32 vm, s32 argIndex);
s32 vmArgGetOrDefault(s32 vm, s32 argIndex, s32 fallbackValue);
s32 vmArgTagIsZero(s32 vm, s32 argIndex);
s32 func_0018F118(s32 voice);
s32 func_0018EDD8(s32 voice, s32 volume, s32 pan);

/* Record of kind 2 handed out by soundCueCommand. */
typedef struct SoundKind2Record {
    /* 0x00 */ u8 unmodeled_00[0xC];
    /* 0x0C */ u8 soundNumber;
} SoundKind2Record;

s32 sbNativePlaySound(s32 vm) {
    s32 soundNumber;
    s32 volume;
    s32 pan = 0xFFFF;
    s32 track;
    s32 soundHandle;
    SoundKind2Record *soundRecord;

    soundNumber = vmArgGet(vm, 1);
    volume = vmArgGetOrDefault(vm, 2, 0x7F);
    if (vmArgTagIsZero(vm, 3) == 0) {
        pan = vmArgGet(vm, 3) + 0x40;
        if (pan < 0) {
            pan = 0;
        }
        if (pan >= 0x80) {
            pan = 0x7F;
        }
    }
    track = vmArgGetOrDefault(vm, 4, 0);
    if (track != 0 && soundNumber < 256) {
        soundHandle = func_0018F118(soundNumber);
        if (soundHandle == 0) {
            soundHandle = func_0018EDD8(soundNumber, volume, pan);
        }
    } else {
        soundHandle = func_0018EDD8(soundNumber, volume, pan);
    }
    if (track != 0) {
        soundRecord = soundCueCommand(2, soundHandle);
        soundRecord->soundNumber = soundNumber;
    }
    return soundHandle;
}

void vmNativeSetKind1Value(s32 vm) {
    s32 soundParam;
    soundParam = vmArgGet(vm, 1);
    soundCueCommand(1, soundParam);
}

/* Record of kind 3 handed out by soundCueCommand. */
typedef struct SoundKind3Record {
    /* 0x00 */ u8 unmodeled_00[0xC];
    /* 0x0C */ s16 soundTableEntry;
    /* 0x0E */ s16 value2;
    /* 0x10 */ s16 negatedValue4;
    /* 0x12 */ s16 value3Plus1;
    /* 0x14 */ signed char value5;
} SoundKind3Record;

extern u8 gSoundCueVolumeTable[];

s32 func_001CDDD8(s32 vm) {
    s32 soundNumber;
    s32 value2;
    s32 value3;
    s32 fallback4;
    s32 value5;
    s32 negatedFallback4;
    s32 soundTableEntry;
    s32 value3Plus1;
    SoundKind3Record *soundRecord;

    soundNumber = vmArgGet(vm, 1);
    value2 = vmArgGet(vm, 2);
    value3 = vmArgGet(vm, 3);
    fallback4 = vmArgGetOrDefault(vm, 4, 0);
    value5 = vmArgGetOrDefault(vm, 5, 0);
    soundRecord = soundCueCommand(3, soundNumber);
    soundTableEntry = gSoundCueVolumeTable[(soundNumber & 0xff) * 4];
    negatedFallback4 = -fallback4;
    value3Plus1 = value3 + 1;
    soundRecord->value3Plus1 = value3Plus1;
    soundRecord->value2 = value2;
    soundRecord->value5 = value5;
    soundRecord->negatedValue4 = negatedFallback4;
    soundRecord->soundTableEntry = soundTableEntry;
    return soundNumber;
}

/* Record of kind 4 handed out by soundCueCommand. */
typedef struct SoundKind4Record {
    /* 0x00 */ u8 unmodeled_00[0xC];
    /* 0x0C */ s16 soundTableEntry;
    /* 0x0E */ s16 value2;
    /* 0x10 */ s16 negatedValue4;
    /* 0x12 */ s16 value3Plus1;
} SoundKind4Record;

extern u8 D_004C756B[];

s32 func_001CDEB0(s32 vm) {
    s32 soundNumber;
    s32 value2;
    s32 value3;
    s32 fallback4;
    s32 negatedFallback;
    s32 soundTableEntry;
    SoundKind4Record *soundRecord;

    soundNumber = vmArgGet(vm, 1);
    value2 = vmArgGet(vm, 2);
    value3 = vmArgGet(vm, 3);
    fallback4 = vmArgGetOrDefault(vm, 4, 0);
    soundRecord = soundCueCommand(4, soundNumber);
    soundTableEntry = D_004C756B[(soundNumber & 0xff) * 4];
    negatedFallback = -fallback4;
    soundRecord->value3Plus1 = value3 + 1;
    soundRecord->value2 = value2;
    soundRecord->negatedValue4 = negatedFallback;
    soundRecord->soundTableEntry = soundTableEntry;
    return soundNumber;
}
