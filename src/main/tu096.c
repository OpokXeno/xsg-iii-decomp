#include "common.h"

typedef signed char s8;

typedef unsigned short u16;

typedef int s32;

typedef float f32;

typedef unsigned char u8;

u16 func_0018EDD8();
void * soundCueCommand();
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

u16 soundOriginRecordCreate(s32 vm) {
    SoundOriginRecord *originRecord;
    s32 soundNumber;
    s32 enabled;
    u16 resultId;

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

u16 directionRecordCreate(s32 vm) {
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
    u16 resultId;
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

s32 vmArgTagIsZero(s32, s32);

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
