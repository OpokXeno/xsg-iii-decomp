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

void soundSlotClearLoopFlag(s32 soundSlotNumber);
void *soundCueCommand(s32 op, s32 value);

/* The record soundCueCommand hands out for op 8: its word at 0x0C takes the movie path of the slot. */
typedef struct SoundSlotRecord {
    /* 0x00 */ unsigned char unmodeled_00[0xC];
    /* 0x0C */ SoundSlotPath *moviePath;
} SoundSlotRecord;

/* Script native: unpauses sound slot arg 1; mode 2 also attaches the slot movie path, modes 1 and 2 clear its loop flag. */
s32 soundSlotMovieControl(s32 vm) {
    s32 soundSlot;
    s32 loopMode;
    SoundSlotRecord *pausedSlotRecord;

    soundSlot = vmArgGetOrDefault(vm, 1, 1);
    loopMode = vmArgGetOrDefault(vm, 2, 0);
    soundSlotSetPause(soundSlot, 0);
    switch (loopMode) {
    case 2:
        pausedSlotRecord = soundCueCommand(8, soundSlot);
        pausedSlotRecord->moviePath = &gSoundSlotPaths[soundSlot];
    case 1:
        soundSlotClearLoopFlag(soundSlot);
        break;
    case 0:
    default:
        break;
    }
    return 1;
}

/* Script native: volume level (arg 1) and ramp step (arg 2) for the sound slot in arg 3 (default 1). */
void soundSlotSetVolumeVm(s32 vm) {
    s32 volumeLevel;
    s32 soundSlot;
    s32 rampStep;

    soundSlot = vmArgGetOrDefault(vm, 3, 1);
    volumeLevel = vmArgGet(vm, 1);
    rampStep = vmArgGet(vm, 2);
    soundSlotSetVolume(soundSlot, volumeLevel, rampStep);
}

/* One sound soundRecord of the table soundCueCommand hands out (the op codes 9, 0xA and 0xB). */
typedef struct SoundRecord {
    /* 0x00 */ unsigned char unmodeled_00[0xC];
    /* 0x0C */ s32 id;
    /* 0x10 */ f32 position[3];
    /* 0x1C */ f32 scale;
} SoundRecord;

f32 vmArgFloatGetOrDefault(s32, s32, f32);

void soundRecordSelect(s32 vm) {
    s32 recordId;

    recordId = vmArgGet(vm, 1);
    soundCueCommand(9, recordId);
}

void soundRecordDefine(s32 vm) {
    f32 scale;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    s32 id;
    SoundRecord *soundRecord;

    soundRecord = soundCueCommand(9, -1);
    id = vmArgGetOrDefault(vm, 1, -3);
    soundRecord->id = id;
    scale = vmArgFloatGetOrDefault(vm, 2, 10.0f);
    soundRecord->scale = scale;
    positionX = vmArgFloatGetOrDefault(vm, 3, 0.0f);
    soundRecord->position[0] = positionX;
    positionY = vmArgFloatGetOrDefault(vm, 4, 0.0f);
    soundRecord->position[1] = positionY;
    positionZ = vmArgFloatGetOrDefault(vm, 5, 0.0f);
    soundRecord->position[2] = positionZ;
    soundCueCommand(0xA, 0);
}

void soundRecordApply(s32 vm) {
    s32 recordId;

    recordId = vmArgGet(vm, 1);
    soundCueCommand(0xB, recordId);
}

#include "shared.h"

s32 mapStaticObjectProbe(Vector4 *position, f32 *outDistance);
s32 soundCueSeCommand(s32, s32, s32, s32);
void scQueueDefaultRequest(void);
void *RgDispModelFindByKey(s32 mode);

/* The slot object RgDispModelFindByKey hands out: the probe reads the position at 0x40 of it. */
typedef struct MapProbeObject {
    /* 0x00 */ unsigned char unmodeled_00[0x40];
    /* 0x40 */ Vector4 position;
} MapProbeObject;

/* Script native: probes the map under the player slot and queues a request chosen by arg 1 (0 default, 1/2 with hit). */
s32 sbNativeMapProbeRequest(s32 vm) {
    s32 probeMode;
    MapProbeObject *playerSlot;
    s32 hit;
    f32 distance;

    probeMode = vmArgGet(vm, 1);
    playerSlot = RgDispModelFindByKey(0);
    if (playerSlot == 0) {
        return 0;
    }
    hit = mapStaticObjectProbe(&playerSlot->position, &distance);
    switch (probeMode) {
    case 0:
        scQueueDefaultRequest();
        break;
    case 1:
        soundCueSeCommand(0, 0, hit, 0);
        break;
    case 2:
        soundCueSeCommand(0, 1, hit, 0);
        break;
    default:
        return 0;
    }
    return 1;
}
