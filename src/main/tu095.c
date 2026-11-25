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

typedef signed char s8;

typedef unsigned short u16;

typedef float f32;

typedef unsigned char u8;

f32 vmArgFloatGet();

s32 vmArgGetOrDefault();

s32 vmArgGet();

/* Record of kind 5 built by soundOriginRecordCreate. */
typedef struct SoundOriginRecord {
    u8 unk_00;
    s8 soundNumber; /* 0x01 */
    u8 unk_02[2];
    u16 resultId; /* 0x04 */
    u8 unk_06[6];
    s8 voiceLevel;    /* 0x0C */
    s8 paramB;        /* 0x0D */
    s8 directionMode; /* 0x0E */
    s8 enabled;       /* 0x0F */
    f32 originX;      /* 0x10 */
    f32 originY;      /* 0x14 */
    f32 originZ;      /* 0x18 */
    f32 extra;        /* 0x1C */
    f32 extraScale;   /* 0x20, 256.0f / extra */
} SoundOriginRecord;

s32 soundOriginRecordCreate(s32 vm) {
    SoundOriginRecord *originRecord;
    s32 soundNumber;
    s32 enabled;
    s32 resultId;

    soundNumber = vmArgGet(vm, 1);
    enabled = vmArgGetOrDefault(vm, 9, 0);
    if (enabled != 0) {
        resultId = func_0018EDD8(soundNumber, 0, 0x40);
    } else {
        resultId = 0;
    }
    originRecord = soundCueCommand(5, resultId);
    originRecord->soundNumber = soundNumber;
    originRecord->originX = vmArgFloatGet(vm, 2);
    originRecord->originY = vmArgFloatGet(vm, 3);
    originRecord->originZ = vmArgFloatGet(vm, 4);
    originRecord->extra = vmArgFloatGet(vm, 5);
    originRecord->voiceLevel = vmArgGetOrDefault(vm, 6, 0x7F);
    originRecord->paramB = vmArgGetOrDefault(vm, 7, 0);
    originRecord->directionMode = vmArgGetOrDefault(vm, 8, 0);
    originRecord->enabled = enabled;
    originRecord->extraScale = 256.0f / originRecord->extra;
    if (resultId == 0) {
        resultId = originRecord->resultId;
    }
    return resultId;
}

s32 xglVectorLength();

struct DirectionVector {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
};

/* Record of kind 6 built by directionRecordCreate. */
typedef struct SoundDirectionRecord {
    u8 unk_00;
    s8 soundNumber;                        /* 0x01 */
    u8 unk_02[2];
    u16 resultId;                   /* 0x04 */
    u8 unk_06[6];
    s8 voiceLevel;                      /* 0x0C */
    s8 paramB;                      /* 0x0D */
    s8 directionMode;                        /* 0x0E */
    s8 enabled;                     /* 0x0F */
    f32 originX;                    /* 0x10 */
    f32 originY;                    /* 0x14 */
    f32 originZ;                    /* 0x18 */
    f32 extra;                      /* 0x1C */
    f32 dirX;                       /* 0x20, normalized */
    f32 dirY;                       /* 0x24 */
    f32 dirZ;                       /* 0x28 */
    f32 length;                     /* 0x2C */
} SoundDirectionRecord;

/* Record of kind 7 built by soundParamRecordCreate. */
typedef struct SoundParamRecord {
    u8 unk_00[0xC];
    s8 soundNumber;                      /* 0x0C */
    s8 value2;                      /* 0x0D */
    s8 voiceLevel;                      /* 0x0E */
    s8 voicePan;                      /* 0x0F */
    s8 value5;                      /* 0x10 */
    s8 isActive;                        /* 0x11 */
} SoundParamRecord;

s32 directionRecordCreate(s32 vm) {
    struct DirectionVector direction;
    f32 length;
    f32 deltaZ;
    f32 normX;
    f32 normY;
    f32 normZ;
    f32 lengthStored;
    f32 extra;
    f32 originX;
    f32 originY;
    f32 originZ;
    f32 endX;
    f32 endY;
    s32 soundNumber;
    s32 enabled;
    s32 directionMode;
    s32 resultId;
    SoundDirectionRecord *directionRecord;

    soundNumber = vmArgGet(vm, 1);
    enabled = vmArgGetOrDefault(vm, 0xB, 0);
    if (enabled != 0) {
        resultId = func_0018EDD8(soundNumber, 0, 0x40);
    } else {
        resultId = 0;
    }
    directionRecord = soundCueCommand(6, resultId);
    directionRecord->soundNumber = soundNumber;
    originX = vmArgFloatGet(vm, 2);
    originY = vmArgFloatGet(vm, 3);
    originZ = vmArgFloatGet(vm, 4);
    endX = vmArgFloatGet(vm, 5);
    endY = vmArgFloatGet(vm, 6);
    deltaZ = vmArgFloatGet(vm, 7) - originZ;
    direction.x = endX - originX;
    direction.z = deltaZ;
    direction.y = endY - originY;
    xglVectorLength(&length, &direction.x);
    directionRecord->originX = originX;
    directionRecord->originY = originY;
    directionRecord->originZ = originZ;
    normX = direction.x / length;
    directionRecord->dirX = normX;
    normY = direction.y / length;
    directionRecord->dirY = normY;
    normZ = direction.z / length;
    directionRecord->dirZ = normZ;
    lengthStored = length;
    directionRecord->length = lengthStored;
    extra = vmArgFloatGet(vm, 8);
    directionRecord->extra = extra;
    directionRecord->voiceLevel = vmArgGetOrDefault(vm, 9, 0x7F);
    directionRecord->paramB = vmArgGetOrDefault(vm, 0xA, 0);
    directionMode = vmArgGetOrDefault(vm, 0xC, 0);
    directionRecord->enabled = enabled;
    directionRecord->directionMode = directionMode;
    if (resultId == 0) {
        resultId = directionRecord->resultId;
    }
    return resultId;
}

void soundParamRecordCreate(s32 vm) {
    s32 soundNumber;
    s32 value2;
    s32 voiceLevel;
    s32 voicePan;
    s32 value5;
    SoundParamRecord *soundParamRecord;

    voicePan = 0xFFFF;
    soundNumber = vmArgGet(vm, 1);
    value2 = vmArgGet(vm, 2);
    voiceLevel = vmArgGetOrDefault(vm, 3, 0x7F);
    if (vmArgTagIsZero(vm, 4) == 0) {
        voicePan = vmArgGet(vm, 4) + 0x40;
        if (voicePan < 0) {
            voicePan = 0;
        }
        if (voicePan >= 0x80) {
            voicePan = 0x7F;
        }
    }
    value5 = vmArgGetOrDefault(vm, 5, 3);
    soundParamRecord = soundCueCommand(7, 0);
    soundParamRecord->soundNumber = soundNumber;
    soundParamRecord->value2 = value2;
    soundParamRecord->voiceLevel = voiceLevel;
    soundParamRecord->voicePan = voicePan;
    soundParamRecord->value5 = value5;
    soundParamRecord->isActive = 1;
}


void GameMpeg2Play(void *moviePath, const unsigned char *format, s32 movieName);

void soundSlotStartNumbered(s32 index, void *moviePath);

s32 soundSlotSetPause(s32 index, s32 value);

s32 soundSlotSetVolume(s32 index, s32 level, s32 extra);

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
    GameMpeg2Play(moviePath, "mev/%s", movieName);
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

s32 sbNativeStoreValueArg(s32 vm);
s32 sbNativeStoreValueAndStartFx(s32 vm);
s32 soundOriginRecordCreate(s32 vm);
s32 directionRecordCreate(s32 vm);
void soundParamRecordCreate(s32 vm);
void soundSlotPlayMovie(s32 vm);
s32 soundSlotMovieControl(s32 vm);
void soundSlotSetVolumeVm(s32 vm);
void soundRecordSelect(s32 vm);
void soundRecordDefine(s32 vm);
void soundRecordApply(s32 vm);
s32 sbNativeMapProbeRequest(s32 vm);

/* One record of the sound native table: the script name, the native and its argument count. */
typedef struct SoundNativeRecord {
    const char *name;
    int (*func)();
    s32 argCount;
} SoundNativeRecord;

SoundNativeRecord gSbSoundNativeTable[17] = {
    { "setBGM", sbNativeStoreValueArg, 1 },
    { "changeBGM", sbNativeStoreValueAndStartFx, 1 },
    { "se", sbNativePlaySound, 1 },
    { "seStop", vmNativeSetKind1Value, 1 },
    { "seFade", func_001CDDD8, 3 },
    { "sePan", func_001CDEB0, 3 },
    { "sePos", soundOriginRecordCreate, 5 },
    { "seLine", directionRecordCreate, 8 },
    { "seID", soundParamRecordCreate, 2 },
    { "adxLoad", soundSlotPlayMovie, 1 },
    { "adxPlay", soundSlotMovieControl, 0 },
    { "adxFade", soundSlotSetVolumeVm, 2 },
    { "panMode", soundRecordSelect, 1 },
    { "panPos", soundRecordDefine, 1 },
    { "fx", soundRecordApply, 1 },
    { "roboFoot", sbNativeMapProbeRequest, 1 },
    { 0, 0, 0 },
};

SoundSlotPath gSoundSlotPaths[4];
