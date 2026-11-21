#include "common.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef signed char s8;

extern unsigned char gMoviePathFormat[];
void GameMpeg2Play(void *moviePath, const unsigned char *format, s32 movieName);
s32 vmArgGetOrDefault(s32 vm, s32 argIndex, s32 defaultValue);
void soundSlotStartNumbered(s32 index, void *moviePath);
s32 soundSlotSetPause(s32 index, s32 value);
s32 soundSlotSetVolume(s32 index, s32 level, s32 extra);
s32 vmArgGet(s32 vm, s32 argIndex);
s32 vmKeyedEntryGet(s32 vm, s32 key);

/* One 64-byte movie moviePath of the sound slot table at gSoundSlotPaths (one per slot index). */
typedef struct SoundSlotPath {
    unsigned char unmodeled_00[0x40];
} SoundSlotPath;

extern SoundSlotPath gSoundSlotPaths[];

void soundSlotPlayMovie(s32 vm) {
    s32 movieName;
    s32 soundSlot;
    SoundSlotPath *moviePath;
    s32 movieKey;

    movieKey = vmArgGet(vm, 1);
    movieName = vmKeyedEntryGet(vm, movieKey);
    soundSlot = vmArgGetOrDefault(vm, 2, 1);
    moviePath = &gSoundSlotPaths[soundSlot];
    GameMpeg2Play(moviePath, gMoviePathFormat, movieName);
    soundSlotSetPause(soundSlot, 1);
    soundSlotSetVolume(soundSlot, -1, 0);
    soundSlotStartNumbered(soundSlot, moviePath);
}

INCLUDE_ASM("asm/main/nonmatchings/tu097", func_001CE438);

void soundSlotSetVolumeVm(s32 vm) {
    s32 volumeLevel;
    s32 soundSlot;
    s32 rampStep;

    soundSlot = vmArgGetOrDefault(vm, 3, 1);
    volumeLevel = vmArgGet(vm, 1);
    rampStep = vmArgGet(vm, 2);
    soundSlotSetVolume(soundSlot, volumeLevel, rampStep);
}

void *func_001CED38(s32 op, s32 value);

/* One sound soundRecord of the table func_001CED38 hands out (the op codes 9, 0xA and 0xB). */
typedef struct SoundRecord {
    /* 0x00 */ unsigned char unmodeled_00[0xC];
    /* 0x0C */ s32 id;
    /* 0x10 */ f32 position[3];
    /* 0x1C */ f32 scale;
} SoundRecord;

f32 func_001D41E0(s32, s32, f32);

void soundRecordSelect(s32 vm) {
    s32 recordId;

    recordId = vmArgGet(vm, 1);
    func_001CED38(9, recordId);
}

void soundRecordDefine(s32 vm) {
    f32 scale;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    s32 id;
    SoundRecord *soundRecord;

    soundRecord = func_001CED38(9, -1);
    id = vmArgGetOrDefault(vm, 1, -3);
    soundRecord->id = id;
    scale = func_001D41E0(vm, 2, 10.0f);
    soundRecord->scale = scale;
    positionX = func_001D41E0(vm, 3, 0.0f);
    soundRecord->position[0] = positionX;
    positionY = func_001D41E0(vm, 4, 0.0f);
    soundRecord->position[1] = positionY;
    positionZ = func_001D41E0(vm, 5, 0.0f);
    soundRecord->position[2] = positionZ;
    func_001CED38(0xA, 0);
}

void soundRecordApply(s32 vm) {
    s32 recordId;

    recordId = vmArgGet(vm, 1);
    func_001CED38(0xB, recordId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu097", func_001CE650);
