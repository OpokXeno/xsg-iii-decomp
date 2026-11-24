#include "common.h"
#include "shared.h"
#include "main/tu122.h"
#include "main/tu132.h"
#include "main/xenoActor.h"
#include "main/tu124.h"
#include "main/Packet.h"
#include "main/xgl_2.h"
#include "vu0_macro.h"
#include "quadword.h"
#include "main/sef_135.h"
#include "main/sv_get_image_item_142.h"
#include "gs.h"
#define SDV_KEY_END 0x1000        /* the sequence has ended */
#define SDV_KEY_WAIT_HIT 0x4000   /* the row waits for the hit signal */
#define SDV_KEY_FRAME_MASK 0x3FFF /* frame of a hit-wait row */

/* Set when the battle reports a hit (sdata). */
extern int gSdvHitReceived;

/* Returns the current row when the sequence reaches it (and advances to the next row), else -1. */
int sdvCheckSeqKey(SdvSequence *sequence)
{
    int reachedRow = -1;
    short key = sequence->keys[sequence->position * sequence->stride];
    SefLinkedContext *battleRequest = sefGetBattleData();
    int hitFrame = key & SDV_KEY_FRAME_MASK;

    if ((key & SDV_KEY_WAIT_HIT) && gSdvHitReceived != 0 && sequence->elapsed >= hitFrame) {
        if (battleRequest->waitHit == 0) {
            sequence->position++;
        } else {
            reachedRow = sequence->position++;
        }
    } else if (sequence->elapsed >= key) {
        reachedRow = sequence->position++;
    }
    return reachedRow;
}

/* Per-tick frame delta (sdata); index 0 is the step added to the sequence's elapsed time. */
extern float gSdvFrameDelta[];

/* Steps the sequence's frame counter (in thousandths, capped at 4095) and returns the row it reaches. */
int sdvStepSequence(SdvSequence *sequence)
{
    int thousandths;

    if (sequence->keys[sequence->position * sequence->stride] == SDV_KEY_END) {
        return SDV_KEY_END;
    }
    thousandths = (sequence->elapsed + gSdvFrameDelta[0] + 0.0001f) * 1000.0f;
    sequence->elapsed = thousandths / 1000.0f;
    if (sequence->elapsed > 4095.0f) {
        sequence->elapsed = 4095.0f;
    }
    return sdvCheckSeqKey(sequence);
}

/* Returns the row the sequence reaches (and advances past it), else -1. */
int sdvProgressKey(short *keys, short *position, int stride, int elapsed)
{
    int reachedRow = -1;
    int key = keys[*position * stride];
    SefLinkedContext *battleRequest = sefGetBattleData();
    int waitsForHit = key & SDV_KEY_WAIT_HIT;

    key &= SDV_KEY_FRAME_MASK;
    if (key == SDV_KEY_END) {
        return SDV_KEY_END;
    }
    if (elapsed >= key) {
        if (waitsForHit) {
            unsigned char waitHit = battleRequest->waitHit;

            if (gSdvHitReceived == 0 && waitHit == 0) {
                (*position)++;
            } else {
                reachedRow = (*position)++;
            }
        } else {
            reachedRow = (*position)++;
        }
    }
    return reachedRow;
}

/* One row of a shadow key sequence: the flags pick the actor, the others are the shadow values. */
typedef struct SdvShadowKey {
    unsigned char unmodeled_00[2];
    short actorFlags;            /* +0x02 */
    signed char shadowCount; /* +0x04 */
    signed char moveState;      /* +0x05 */
    short velocity;         /* +0x06 tenths */
    short shadowValue;      /* +0x08 hundredths */
    short angle;            /* +0x0A hundredths */
    short shadowValue3;     /* +0x0C hundredths */
} SdvShadowKey;

#define SDV_KEY_SOURCE 0x100   /* the request's source actor */
#define SDV_KEY_TARGET 0x200   /* the request's target actor */

extern float gSdvHundredthScale;
extern float gSdvTenthScale;

/* Steps a shadow key sequence and copies the row it reaches into the shadow fields of the
 * request's actor that the row's flags pick. */
void sdvExecShadowSeq(SdvSequence *sequence)
{
    unsigned int reachedRow;
    SefLinkedContext *battleRequest;
    XenoActor *actor;
    SdvShadowKey *shadowKey;
    int actorFlags;
    int moveState;
    int shadowCount;
    int shadowValue3Hundredths;
    int velocityTenths;
    int shadowValueHundredths;
    int angleHundredths;
    float velocity;
    float shadowValue;
    float angle;
    float shadowValue3;

    if (sequence == 0 || sequence->keys == 0) {
        return;
    }
    reachedRow = sdvStepSequence(sequence);
    if (reachedRow >= SDV_KEY_END) {
        return;
    }
    battleRequest = sefGetBattleData();
    actor = 0;
    shadowKey = (SdvShadowKey *)(sequence->keys + reachedRow * sequence->stride);
    shadowValue3Hundredths = shadowKey->shadowValue3;
    velocityTenths = shadowKey->velocity;
    shadowValueHundredths = shadowKey->shadowValue;
    angleHundredths = shadowKey->angle;
    actorFlags = shadowKey->actorFlags;
    moveState = shadowKey->moveState;
    shadowCount = shadowKey->shadowCount;
    velocity = velocityTenths * gSdvTenthScale;
    shadowValue = shadowValueHundredths * gSdvHundredthScale;
    angle = angleHundredths * gSdvHundredthScale;
    shadowValue3 = shadowValue3Hundredths * gSdvHundredthScale;
    if (actorFlags & SDV_KEY_TARGET) {
        actor = battleRequest->target;
    } else if (actorFlags & SDV_KEY_SOURCE) {
        actor = battleRequest->source;
    }
    if (actor != 0) {
        actor->sweepPhase = angle;
        actor->shadowValue = shadowValue;
        actor->sweepSpeed = velocity;
        actor->shadowCount = shadowCount;
        actor->shadowValue3 = shadowValue3;
        actor->moveState = moveState;
    }
}

void tiWindClearFirstSlot(void);
void tiWindResetSlots(void);
void tiWindReleaseSlots(int key, int subkey);
void tiWindDirSetEnabled(u32 index, int enabled);
void tiWindPointSetEnabled(u32 index, int enabled);

/* One wind slot: the effect key and sub-key that own it and the kind of its wind (the wind key
 * table's kind: 0 directional, else point). Four slots, the same count as the directional and
 * point wind tables of TiWindBase. */
typedef struct TiWindSlot {
    int key;
    unsigned short subkey;
    short windKind;
} TiWindSlot;

extern TiWindSlot tiWindSlots[4];
void *memset(void *destination, int value, unsigned int size);

/* Stores key, subkey and windKind in the first empty wind slot (key 0) and returns
 * its index, or -1 when all four slots are in use. */
int tiWindAcquireSlot(int key, unsigned short subkey, int windKind)
{
    TiWindSlot *windSlot;
    int i;

    windSlot = tiWindSlots;
    for (i = 0; i < 4; i++) {
        if (windSlot->key == 0) {
            windSlot->key = key;
            windSlot->subkey = subkey;
            windSlot->windKind = windKind;
            return i;
        }
        windSlot++;
    }
    return -1;
}

/* Releases every wind slot owned by key and subkey (key 0 matches all slots):
 * each one is disabled and cleared. */
void tiWindReleaseSlots(int key, int subkey)
{
    TiWindSlot *windSlot;
    int i;

    windSlot = tiWindSlots;
    for (i = 0; i < 4; i++) {
        if (key == 0 || (windSlot->key == key && windSlot->subkey == (subkey & 0xFFFF))) {
            tiWindDirSetEnabled(i, 0);
            tiWindPointSetEnabled(i, 0);
            windSlot->key = 0;
            windSlot->subkey = 0;
            windSlot->windKind = 0;
        }
        windSlot++;
    }
}

/* Index of the wind slot whose key and subkey match, or -1. */
int tiWindFindSlot(int key, unsigned short subkey)
{
    TiWindSlot *windSlot;
    int i;

    windSlot = tiWindSlots;
    for (i = 0; i < 4; i++) {
        if (windSlot->key == key && windSlot->subkey == subkey) {
            return i;
        }
        windSlot++;
    }
    return -1;
}

/* Clears the first wind slot only (sizeof of one entry). */
void tiWindClearFirstSlot(void)
{
    memset(tiWindSlots, 0, sizeof(tiWindSlots[0]));
}

/* Releases all wind slots and clears the first one. */
void tiWindResetSlots(void)
{
    tiWindReleaseSlots(0, 0);
    tiWindClearFirstSlot();
}

typedef struct TiWindKey {
    short frame;            /* +0x00 */
    short positionKind;     /* +0x02 sefGetPosition kind of the offset */
    short positionOffset[3];        /* +0x04 */
    short rangeUpper;       /* +0x0A phase bounds */
    short rangeLower;       /* +0x0C */
    short radius;           /* +0x0E negative: global wind; 0 or TIWIND_RADIUS_OFF and beyond: wind off */
    short direction[3];     /* +0x10 directional wind only */
} TiWindKey;

#define TIWIND_RADIUS_OFF 20000

/* The key rows of a sequence seen as halfword runs (SdvSequence.keys) or as TiWindKey rows. */
typedef union TiWindRowView {
    short *halves;
    TiWindKey *rows;
} TiWindRowView;

/* A vector the code writes component by component through .vector. */
typedef union SdvVectorView {
    Vector4 vector;
} SdvVectorView;

int sdvStepSequence(SdvSequence *sequence);
void sefGetPosition(Vector4 *dest, const Vector4 *source, int kind);
void tiWindPointSetPosition(u32 index, const float *position);
void tiWindPointSetSpeed(u32 index, float speed);
void tiWindPointSetRange(u32 index, float upper, float lower);
void tiWindPointSetRadius(u32 index, float radius, int global);
void tiWindDirSetPosition(u32 index, const float *position);
void tiWindDirSetDirection(u32 index, float x, float y, float z);
void tiWindDirSetSpeed(u32 index, float speed);
void tiWindDirSetRange(u32 index, float upper, float lower);
void tiWindDirSetRadius(u32 index, float radius, int global);
Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);

#define TIWIND_BLEND(member) \
    activeRow->member = (nextRow->member == currentRow->member ? currentRow->member \
        : (currentRow->member + (((nextRow->member - currentRow->member) * blendT + 0x80) >> 8)) & 0xFFFF)

/* Steps a wind key sequence and applies the key it reaches (interpolated towards the next key)
 * to the wind slot of key and subkey, acquiring one when needed: the point or directional wind
 * of the table's kind is placed, given its speed, range and radius and enabled; a radius of 0
 * or TIWIND_RADIUS_OFF and beyond disables it and releases the slot. */
void tiWindUpdateKeyframes(SdvSequence *sequence, int effectKey, unsigned short effectSubkey)
{
    TiWindKey blendedKey;
    SdvVectorView windVector;
    TiWindRowView keyRowView;
    AlignedVector4 windPosition;
    TiWindKey *activeRow;
    TiWindKey *currentRow;
    TiWindKey *nextRow;
    int keyRowIndex;
    int found;
    int windKind;
    int speed;
    int elapsed;
    int rowFrame;
    int nextRowFrame;
    int blendT;
    int windSlotIndex;
    int enabled;
    int radius;

    if (sequence == 0 || sequence->keys == 0) {
        return;
    }
    keyRowIndex = sdvStepSequence(sequence);
    if (keyRowIndex < 0) {
        keyRowIndex = sequence->position - 1;
    }
    if (keyRowIndex < 0) {
        keyRowIndex = 0;
    }
    if (keyRowIndex == 4096) {
        return;
    }
    found = 0;
    activeRow = &blendedKey;
    keyRowView.halves = sequence->keys + keyRowIndex * sequence->stride;
    currentRow = keyRowView.rows;
    nextRow = currentRow + 1;
    elapsed = sequence->elapsed;
    speed = sequence->keys[-2];
    windKind = sequence->keys[-1];
    rowFrame = currentRow->frame;
    if (rowFrame != 4096 && elapsed >= rowFrame) {
        nextRowFrame = nextRow->frame;
        if (nextRowFrame >= 4096) {
            activeRow = currentRow;
            found = 1;
        } else if (rowFrame != nextRowFrame) {
            blendT = ((elapsed - rowFrame) << 8) / (nextRowFrame - rowFrame);
            TIWIND_BLEND(positionOffset[0]);
            TIWIND_BLEND(positionOffset[1]);
            TIWIND_BLEND(positionOffset[2]);
            TIWIND_BLEND(radius);
            TIWIND_BLEND(rangeUpper);
            TIWIND_BLEND(rangeLower);
            TIWIND_BLEND(direction[0]);
            TIWIND_BLEND(direction[1]);
            TIWIND_BLEND(direction[2]);
            found = 1;
            activeRow->positionKind = currentRow->positionKind;
        }
    }
    if (found == 0) {
        return;
    }
    windSlotIndex = tiWindFindSlot(effectKey, effectSubkey);
    if (windSlotIndex < 0) {
        windSlotIndex = tiWindAcquireSlot(effectKey, effectSubkey, windKind);
        if (windSlotIndex < 0) {
            return;
        }
    }
    windVector.vector.x = activeRow->positionOffset[0] * 0.01f;
    windVector.vector.y = activeRow->positionOffset[1] * 0.01f;
    windVector.vector.z = activeRow->positionOffset[2] * 0.01f;
    windVector.vector.w = 1.0f;
    sefGetPosition(&windPosition, &windVector.vector, activeRow->positionKind);
    if (windKind != 0) {
        tiWindDirSetEnabled(windSlotIndex, 0);
        tiWindPointSetPosition(windSlotIndex, &windPosition.x);
        tiWindPointSetSpeed(windSlotIndex, speed * 0.01f);
        tiWindPointSetRange(windSlotIndex, activeRow->rangeUpper * 0.01f, activeRow->rangeLower * 0.01f);
        enabled = 0;
        radius = activeRow->radius;
        if (radius != 0 && radius < TIWIND_RADIUS_OFF) {
            enabled = 1;
        }
        if (enabled) {
            if (radius < 0) {
                tiWindPointSetRadius(windSlotIndex, 0.0f, 1);
            } else {
                tiWindPointSetRadius(windSlotIndex, radius * 0.01f, 0);
            }
            tiWindPointSetEnabled(windSlotIndex, 1);
        } else {
            tiWindPointSetEnabled(windSlotIndex, 0);
            tiWindReleaseSlots(effectKey, effectSubkey);
        }
    } else {
        tiWindPointSetEnabled(windSlotIndex, 0);
        tiWindDirSetPosition(windSlotIndex, &windPosition.x);
        windVector.vector.x = activeRow->direction[0] * 0.01f;
        windVector.vector.y = activeRow->direction[1] * 0.01f;
        windVector.vector.z = activeRow->direction[2] * 0.01f;
        windVector.vector.w = 1.0f;
        MMathNormalizeVector2(&windVector.vector, &windVector.vector);
        tiWindDirSetDirection(windSlotIndex, windVector.vector.x, windVector.vector.y, windVector.vector.z);
        tiWindDirSetSpeed(windSlotIndex, speed * 0.01f);
        tiWindDirSetRange(windSlotIndex, activeRow->rangeUpper * 0.01f, activeRow->rangeLower * 0.01f);
        enabled = 0;
        radius = activeRow->radius;
        if (radius != 0 && radius < TIWIND_RADIUS_OFF) {
            enabled = 1;
        }
        if (enabled) {
            if (radius < 0) {
                tiWindDirSetRadius(windSlotIndex, 0.0f, 1);
            } else {
                tiWindDirSetRadius(windSlotIndex, radius * 0.01f, 0);
            }
            tiWindDirSetEnabled(windSlotIndex, 1);
        } else {
            tiWindDirSetEnabled(windSlotIndex, 0);
            tiWindReleaseSlots(effectKey, effectSubkey);
        }
    }
}

/* The camera request: its search bits and override id choose the camera. */
typedef struct CameraRequest {
    unsigned char unmodeled_00[40];
    int cameraSearchBits;                    /* +0x28: bits 8 and 9 select the camera search */
    int overrideId;                 /* +0x2c */
} CameraRequest;

/* A 64-bit state word; bit 4 set makes the selection return 0. */
extern unsigned long long gCameraSelectFlags[];
int func_0030DD10(void);
int scGetImmAdrImmIdx2(int scriptBase, unsigned short *cameraEntryTable, int index);
short scGetImmNumIdx(unsigned short *cameraEntryTable, int index);

/* Picks one camera script by a random roll weighted by the four weights, or the override camera when the request
 * search bits or override id select it; loads its four key channels into schedule and returns the camera (0 when
 * none). */
int sdvSelectCamera(SdvCameraSchedule *schedule, int scriptBase, unsigned short *cameraEntryTable, CameraRequest *cameraRequest)
{
    int cameraWeights[5];
    int cameraScripts[5];
    int overrideIds[1];
    int cameraRoll;
    int selectedCamera;
    int cumulativeWeight;
    int i;
    int searchFlagBits;
    int useOverrideCamera;
    unsigned short *cameraChannelTable;

    selectedCamera = 0;
    cameraRoll = func_0030DD10() % 100;
    if (cameraEntryTable == 0 || scriptBase == 0) {
        return 0;
    }
    for (i = 0; i < 5; i++) {
        cameraScripts[i] = scGetImmAdrImmIdx2(scriptBase, cameraEntryTable, i * 2);
        cameraWeights[i] = scGetImmNumIdx(cameraEntryTable, i * 2 + 1);
    }
    useOverrideCamera = 0;
    searchFlagBits = cameraRequest->cameraSearchBits & 0x300;
    if (searchFlagBits == 0x300 || searchFlagBits == 0x200 || searchFlagBits == 0x100) {
        useOverrideCamera = 1;
    }
    /* The id list is terminated by its first non-positive entry; it starts with -1. */
    overrideIds[0] = -1;
    for (i = 0; overrideIds[i] > 0; i++) {
        if (cameraRequest->overrideId == overrideIds[i]) {
            useOverrideCamera = 1;
            break;
        }
    }
    if (((gCameraSelectFlags[0] >> 4) & 1) != 0) {
        return 0;
    }
    if (useOverrideCamera != 0 && cameraScripts[4] != 0) {
        selectedCamera = cameraScripts[4];
    } else {
        cumulativeWeight = 0;
        for (i = 0; i < 4; i++) {
            cumulativeWeight += cameraWeights[i];
            if (cameraRoll < cumulativeWeight && cameraScripts[i] != 0) {
                selectedCamera = cameraScripts[i];
                break;
            }
        }
    }
    if (selectedCamera != 0) {
        cameraChannelTable = (unsigned short *) selectedCamera;
        schedule->channels[0].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 0);
        schedule->channels[1].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 1);
        schedule->channels[2].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 2);
        schedule->channels[3].keys = (short *) scGetImmAdrImmIdx2(scriptBase, cameraChannelTable, 3);
    }
    schedule->camera = selectedCamera;
    schedule->animation = 0;
    return selectedCamera;
}

typedef struct SdvCameraTransform {
    unsigned char unmodeled_00[0x50];
    float scale;  /* +0x50 */
    short scaled; /* +0x54: nonzero when scale applies */
} SdvCameraTransform;

typedef struct SdvCameraPoint {
    Vector4 point;  /* +0x00 */
    Vector4 placementVector; /* +0x10: placement x, y, z and w = 1 */
} SdvCameraPoint;

extern SdvCameraTransform gSdvCameraPartTransform;
int sefInitPartTransform(SdvCameraTransform *part, int kind, int scaleKey100, int scaleKey200);

/* Builds one camera point from the three placement values, scaled by 10 when flags bit 1 is set and by 0.01; flags
 * bit 0 first initializes the part transform gSdvCameraPartTransform from the battle request, which also gives the scale. The point
 * is then converted by sefGetPosition for kind. */
void sdvBuildCameraPoint(SdvCameraPoint *point, int positionKind, short *placement, int placementFlags) {
    int placementIsTens;
    SefLinkedContext *battleRequest;
    float partScale;

    battleRequest = sefGetBattleData();
    placementIsTens = (placementFlags >> 1) & 1;
    placementFlags &= 1;
    partScale = 1.0f;
    memset(point, 0, sizeof(*point));
    if (placementFlags) {
        sefInitPartTransform(&gSdvCameraPartTransform, positionKind, (int) battleRequest->source, (int) battleRequest->target);
        if (gSdvCameraPartTransform.scaled != 0) {
            partScale = gSdvCameraPartTransform.scale;
        }
    }
    if (placementIsTens != 0) {
        point->placementVector.x = (float) (placement[0] * 10);
    } else {
        point->placementVector.x = (float) placement[0];
    }
    if (placementIsTens != 0) {
        point->placementVector.y = (float) (placement[1] * 10);
    } else {
        point->placementVector.y = (float) placement[1];
    }
    if (placementIsTens != 0) {
        point->placementVector.z = (float) (placement[2] * 10);
    } else {
        point->placementVector.z = (float) placement[2];
    }
    point->placementVector.w = 1.0f;
    Vu0SefScaleVectorXYZ(&point->placementVector, &point->placementVector, 0.01f);
    Vu0SefScaleVectorXYZ(&point->placementVector, &point->placementVector, partScale);
    sefGetPosition(&point->point, &point->placementVector, positionKind);
}

int sdvProgressKey(short *keys, short *position, int stride, int elapsed);
void cameraIndexedApply(int cameraHandle, int cameraValue, int cameraValueIndex);
void cameraIndexedUpdate(int cameraHandle, int cameraValue, int cameraValueIndex);
void cameraIndexedSample(int cameraHandle, int cameraValue, int cameraValueIndex);
void cameraIndexedBlend(int cameraHandle, int cameraValueIndex, float weight);
void cameraIndexedFade(int cameraHandle, int cameraValueIndex, float amount);
void func_00267398(int wave, int value, Vector4 *amplitude, Vector4 *phase, int cycles, int remaining);
void func_002674C0(int wave, int value, Vector4 *amplitude, Vector4 *phase, int cycles, int remaining);

/* sdvBuildCameraPoint takes four parameters; the callers below also pass a fifth argument, a zero word the callee ignores. */
typedef void (*SdvCameraPointBuilder)(SdvCameraPoint *point, int kind, short *placement, int flags, int zero);

/* Plays the key row of one camera channel at frame: kinds 0 and 1 apply camera points (the row and the next one), kind 2
 * applies the two blend angles, kind 3 applies up to three wave entries. Returns 0 when the channel is at its end row,
 * else 1. */
int sdvCameraChannelPlay(int channelKind, SdvScheduleChannel *channel, float frame) {
    int keySizes[4] = {8, 8, 4, 16};
    int keySize;
    short *keys;
    int keyRowIndex;
    short *key;
    short *nextKey;
    int remaining;
    int frameSpan;
    SdvCameraPoint nextPoint;
    SdvCameraPoint point;

    keySize = keySizes[channelKind];
    keys = channel->keys;
    keyRowIndex = sdvProgressKey(keys, (short *) channel, keySize, (int) frame);
    if (keyRowIndex == SDV_KEY_END) {
        return 0;
    }
    if (keyRowIndex < 0) {
        return 1;
    }
    key = keys + keyRowIndex * keySize;
    nextKey = key + keySize;
    frameSpan = (nextKey[0] & SDV_KEY_FRAME_MASK) - (key[0] & SDV_KEY_FRAME_MASK);
    remaining = frameSpan;
    if (remaining < 0) {
        return 1;
    }
    if ((nextKey[0] & SDV_KEY_FRAME_MASK) > SDV_KEY_END) {
        return 1;
    }
    if ((nextKey[0] & SDV_KEY_FRAME_MASK) == SDV_KEY_END) {
        if (keyRowIndex != 0) {
            return 1;
        }
        remaining = 0;
    }
    if (channelKind == 1) {
        if (keyRowIndex == 0) {
            ((SdvCameraPointBuilder) sdvBuildCameraPoint)(&point, key[7], key + 4, key[3], 0);
            cameraIndexedApply((int) &point, 0, 0);
        }
        if (remaining != 0) {
            ((SdvCameraPointBuilder) sdvBuildCameraPoint)(&nextPoint, key[7], nextKey + 4, key[3], 0);
            cameraIndexedApply((int) &nextPoint, remaining, key[1] & 0xFF);
        }
    } else if (channelKind == 0) {
        if (keyRowIndex == 0) {
            ((SdvCameraPointBuilder) sdvBuildCameraPoint)(&point, key[7], key + 4, key[3], 0);
            cameraIndexedUpdate((int) &point, 0, 0);
        }
        if (remaining != 0) {
            ((SdvCameraPointBuilder) sdvBuildCameraPoint)(&nextPoint, key[7], nextKey + 4, key[3], 0);
            if ((unsigned short) key[1] & 0x100) {
                cameraIndexedSample((int) &nextPoint, remaining, (unsigned short) key[1] & 0xFF);
            } else {
                cameraIndexedUpdate((int) &nextPoint, remaining, (unsigned short) key[1] & 0xFF);
            }
        }
    } else if (channelKind == 2) {
        if (keyRowIndex == 0) {
            float piValue = 3.14159265f;
            cameraIndexedBlend(0, 0, (float) key[2] / 180.0f * piValue);
            cameraIndexedFade(0, 0, (float) key[3] / 180.0f * piValue);
        }
        if (remaining != 0) {
            float piValue = 3.14159265f;
            cameraIndexedBlend(remaining, key[1] & 0xFF, (float) nextKey[2] / 180.0f * piValue);
            cameraIndexedFade(remaining, key[1] & 0xFF, (float) nextKey[3] / 180.0f * piValue);
        }
    } else if (channelKind == 3) {
        int waveTable[18] = {1, 2, 3, 11, 12, 13, 4, 5, 6, 2, 4, 8, 18, 20, 24, 34, 36, 40};
        Vector4 waveAmplitude;
        Vector4 wavePhase;
        int i = 0;
        int j;
        int waveKind = 0;

        if (key[1] != 0) {
            /* kept: the goto loop; do/while spellings changed the emitted code */
        nextWaveEntry: {
            float amplitudeX;
            float amplitudeY;
            float amplitudeZ;
            float phaseX;
            float phaseY;
            float phaseZ;
            float cycles;

            if ((key[i * 5 + 1] & 2) != 0) {
                amplitudeX = (float) key[i * 5 + 2] / 100.0f / 180.0f * 3.14159265f;
                amplitudeY = (float) key[i * 5 + 3] / 100.0f / 180.0f * 3.14159265f;
                amplitudeZ = (float) key[i * 5 + 4] / 100.0f / 180.0f * 3.14159265f;
                phaseX = (float) nextKey[i * 5 + 2] / 100.0f / 180.0f * 3.14159265f;
                phaseY = (float) nextKey[i * 5 + 3] / 100.0f / 180.0f * 3.14159265f;
                phaseZ = (float) nextKey[i * 5 + 4] / 100.0f / 180.0f * 3.14159265f;
                cycles = key[i * 5 + 5];
            } else {
                amplitudeX = (float) key[i * 5 + 2] / 100.0f;
                amplitudeY = (float) key[i * 5 + 3] / 100.0f;
                amplitudeZ = (float) key[i * 5 + 4] / 100.0f;
                phaseX = (float) nextKey[i * 5 + 2] / 100.0f;
                phaseY = (float) nextKey[i * 5 + 3] / 100.0f;
                phaseZ = (float) nextKey[i * 5 + 4] / 100.0f;
                cycles = key[i * 5 + 5];
            }
            for (j = 0; j < 9; j++) {
                if (waveTable[9 + j] == key[i * 5 + 1]) {
                    waveKind = waveTable[j];
                    break;
                }
            }
            wavePhase.w = 1.0f;
            waveAmplitude.x = amplitudeX;
            waveAmplitude.y = amplitudeY;
            waveAmplitude.z = amplitudeZ;
            waveAmplitude.w = 1.0f;
            wavePhase.x = phaseX;
            wavePhase.y = phaseY;
            wavePhase.z = phaseZ;
            if (keyRowIndex == 0) {
                func_00267398(i, waveKind, &waveAmplitude, &wavePhase, (int) cycles, frameSpan);
            } else {
                func_002674C0(i, waveKind, &waveAmplitude, &wavePhase, (int) cycles, frameSpan);
            }
            i++;
        }
            if (i < 3) {
                waveKind = 0;
                if (key[i * 5 + 1] != 0) {
                    goto nextWaveEntry;
                }
            }
        }
    }
    return 1;
}

extern long long gSbStudioBlock[];
void sbStudioCameraReset(void);

typedef struct SdvCameraFlags {
    u32 active : 1;
    u32 done : 1;
    u32 unused : 30;
} SdvCameraFlags;

typedef struct SdvStudioBlock {
    u64 enabled : 1;
    u64 unmodeled_00_high : 63;
    u64 unmodeled_08;
    u64 unmodeled_10_bit0 : 1;
    u64 ready : 1;
    u64 studioMode : 2;
    u64 unmodeled_10_bit4 : 1;
    u64 changed : 1;
    u64 unmodeled_10_high : 58;
} SdvStudioBlock;

/* The studio flags word (gSbStudioBlock) as bit fields. */
#define gSbStudioBlockView (*(SdvStudioBlock *)(*(unsigned long long (*)[])&gSbStudioBlock))

/* Runs the four camera channels of schedule at frame: channels 0 to 2 OR their done results into the return value.
 * Does nothing while the studio block is in mode 2 or not ready, or when the schedule has no camera. Marks the studio
 * block changed when it is on and a channel is active. */
int sdvCameraScheduleUpdate(SdvCameraSchedule *cameraSchedule, float frame) {
    SdvCameraFlags channelFlags;

    *(u32 *) &channelFlags = 0;
    if (gSbStudioBlockView.studioMode == 2 || gSbStudioBlockView.ready == 0 || cameraSchedule == 0 || cameraSchedule->camera == 0) {
        return 0;
    }
    if (frame == 0.0f) {
        sbStudioCameraReset();
    }
    if (cameraSchedule->channels[1].keys != 0) {
        channelFlags.active = 1;
        channelFlags.done = sdvCameraChannelPlay(0, &cameraSchedule->channels[0], frame);
        if (cameraSchedule->channels[1].keys != 0) {
            channelFlags.active = 1;
            channelFlags.done |= sdvCameraChannelPlay(1, &cameraSchedule->channels[1], frame);
        }
    }
    if (cameraSchedule->channels[2].keys != 0) {
        channelFlags.active = 1;
        channelFlags.done |= sdvCameraChannelPlay(2, &cameraSchedule->channels[2], frame);
    }
    if (cameraSchedule->channels[3].keys != 0) {
        channelFlags.active = 1;
        sdvCameraChannelPlay(3, &cameraSchedule->channels[3], frame);
    }
    if (gSbStudioBlockView.enabled && channelFlags.active) {
        gSbStudioBlockView.changed = 1;
    }
    return channelFlags.done;
}

struct XglPacket;
XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);
void sceVif1PkRef(XglPacket *packet, const void *data, unsigned int qwc, unsigned int a3, unsigned int a4,
                  unsigned int a5);

typedef unsigned int XglQuadword __attribute__((mode(TI)));

/* The header sdvDrawAlter puts in front of each strip it sends by reference: sdvSetStripHeader clears its first
   two quadwords, fills the UNPACK code, primitive count and strip flag, and ORs the primitive count into the
   loop count at +0x20 (the third quadword is not cleared). */
typedef union SdvStripHeader {
    XglQuadword quad[3];
    struct {
        unsigned int unmodeled_00[3];
        unsigned int unpack;           /* +0x0C VIF UNPACK code for the whole strip */
        unsigned int primitiveCount;   /* +0x10 */
        unsigned int strip;            /* +0x14 sdvDrawAlter passes 1 with its count / 2, else 0 with count / 3 */
        unsigned int unmodeled_18[2];
        unsigned short loopCount;      /* +0x20 ORed with the primitive count */
    } layout;
} SdvStripHeader;

/* Keeps the physical address of an EE pointer: DMA references take physical addresses. */
#define SDV_PHYSICAL_ADDRESS_MASK 0x0FFFFFFF

void sefMemZero(void *dest, int size);
void gifStateClearModeHook(void);
void slotReleaseChecked(int slot, int flag);

/* One 128-byte record of the table at gSdvSpecialRecords; sdvFindFreeSpecialWork takes the first record whose word at +0x04 is zero. */
typedef struct SdvSpecialRecord {
    unsigned char unmodeled_00[0x04];
    unsigned int inUse;
    unsigned char unmodeled_08[0x78];
} SdvSpecialRecord;

extern SdvSpecialRecord gSdvSpecialRecords[];

/* One alter record, 0xE70 bytes; sdvInitAlter zeroes all of it. Only the two bytes sdvDestroyAlter reads are named. */
typedef struct SdvAlterHeader {
    unsigned char unmodeled_00[8];
    unsigned short active;
    unsigned char unmodeled_0A[0xE1E - 0xE1A];
} SdvAlterHeader;

typedef struct SdvAlter {
    unsigned char unmodeled_000[0xAF2];
    short owner;               /* +0xAF2 compared with the owner argument of sdvDestroyOwnerAlters */
    unsigned char unmodeled_AF4[0xE10 - 0xAF4];
    SdvAlterHeader alterHeader;
    signed char specialSlot;   /* +0xE1E: the special record sdvClearSpecialRecord clears */
    signed char ownedSlot;     /* +0xE1F: first argument of slotReleaseChecked in sdvDestroyAlter */
    unsigned char unmodeled_E20[0xE70 - 0xE20];
} SdvAlter;

/* The 49 alter records (0xE70 bytes each) start at gSdvAlters. */
extern SdvAlter gSdvAlters[];

typedef struct ApM4 {
    AlignedVector4 rows[4];
} ApM4;

typedef struct ApI4 {
    int x;
    int y;
    int z;
    int w;
} __attribute__((aligned(16))) ApI4;

/* A screen vertex: a float vector, or the integer view that sdvProjectEffectPoint writes. */
typedef union ScreenVertex {
    AlignedVector4 vector;
    ApI4 ints;
} ScreenVertex;

/* The active studio camera: its view matrix sits at +0x470. */
typedef struct StudioCamera {
    unsigned char unmodeled_000[0x470];
    ApM4 matrix;
} StudioCamera;

/* The two scale keys of the battle request that sefInitPartTransform takes. */
typedef struct PartScaleKeys {
    int scaleKey100;
    int scaleKey200;
} PartScaleKeys;

/* gSdvCameraPartTransform seen as the part transform used by sdvProjectEffectPoint: matrix, local translation, scale and scaled flag. */
typedef struct PartTransform {
    ApM4 matrix;
    AlignedVector4 localTranslation;
    float scale;
    short scaled;
} PartTransform;

extern AlignedVector4 D_0053E9C0;
extern StudioCamera *xglStudioGetActiveCamera(void);
extern void sefGetCameraPos(ApM4*);
extern void MMathMulMatrix(ApM4*,ApM4*,ApM4*);
extern void MMathApplyMatrix(AlignedVector4*,ApM4*,AlignedVector4*);
extern void sefBuildEffectMatrix(AlignedVector4*,AlignedVector4*,AlignedVector4*,ApM4*);
extern void sefApplyMatrixVector(AlignedVector4*,ApM4*,AlignedVector4*);
extern void MMathScaleMatrixRotS(ApM4*,ApM4*,float);
extern void MMathScaleMatrix(ApM4*,ApM4*,AlignedVector4*);
extern int MMathRotTransPersClip(ScreenVertex*,StudioCamera*,ApM4*,AlignedVector4*);

static inline void apIntVectorCopy(ApI4 *outInts, ApI4 *inInts) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(outInts->x), "=m"(outInts->y), "=m"(outInts->z), "=m"(outInts->w)
                         : "r"(outInts), "r"(inInts)
                         : "$8");
}

static inline void apVectorAdd(AlignedVector4 *outVector, AlignedVector4 *lhsVector, AlignedVector4 *rhsVector) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z)
                         : "r"(outVector), "r"(lhsVector), "r"(rhsVector));
}

static inline void apVectorSub(AlignedVector4 *outVector, AlignedVector4 *lhsVector, AlignedVector4 *rhsVector) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vsub.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z)
                         : "r"(outVector), "r"(lhsVector), "r"(rhsVector));
}

static inline void apVectorScale(AlignedVector4 *outVector, AlignedVector4 *inVector, float scale) {
    __asm__ __volatile__(
        "lqc2 vf1,0(%4)\n mfc1 $8,%5\n qmtc2.ni $8,vf2\n vmulx.xyz vf1,vf1,vf2x\n sqc2 vf1,0(%3)"
        : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z)
        : "r"(outVector), "r"(inVector), "f"(scale)
        : "$8");
}

static inline void apVectorFromMatrixTranslation(AlignedVector4 *outVector, ApM4 *matrix) {
    __asm__ __volatile__("lqc2 vf1,48(%5)\n vmove.w vf1,vf0\n sqc2 vf1,0(%4)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z), "=m"(outVector->w)
                         : "r"(outVector), "r"(matrix));
}

static inline void apMatrixTranslationSet(ApM4 *matrix, AlignedVector4 *inVector) {
    __asm__ __volatile__("lqc2 vf1,0(%5)\n vmove.w vf1,vf0\n sqc2 vf1,48(%4)"
                         : "=m"(matrix->rows[3].x), "=m"(matrix->rows[3].y), "=m"(matrix->rows[3].z), "=m"(matrix->rows[3].w)
                         : "r"(matrix), "r"(inVector));
}

static inline void apIntToFloatVector(AlignedVector4 *outVector, ApI4 *inInts) {
    __asm__ __volatile__("lqc2 vf1,0(%5)\n vitof0.xyzw vf1,vf1\n sqc2 vf1,0(%4)"
                         : "=m"(outVector->x), "=m"(outVector->y), "=m"(outVector->z), "=m"(outVector->w)
                         : "r"(outVector), "r"(inInts));
}

#define intAbs(x) __builtin_abs(x)

/* The scheduler that sdv reads: bit 24 of flags enables the effect matrix of sdvProjectEffectPoint; the three vectors build it. */
typedef struct SefScheduler {
    unsigned char unmodeled_00[4];
    int flags;                       /* +0x04 */
    unsigned char unmodeled_08[8];
    AlignedVector4 effectOffset;     /* +0x10: added to the point */
    AlignedVector4 effectAxes;       /* +0x20: first input of sefBuildEffectMatrix */
    AlignedVector4 effectScale;      /* +0x30: scale of the effect matrix */
} SefScheduler;

SefScheduler *sefGetNowScheduler(void);
int sdvProjectEffectPoint(ScreenVertex *screenVertex, ApI4 *intPoint, int kind, int depth, AlignedVector4 *screenSize, int usePartTransform, ApM4 *localMatrix);

/* Projects one point: kind 8 writes a pixel position from intPoint and depth and returns 1. Otherwise the point goes
 * to world space (through localMatrix, else sefGetPosition), is placed by the part transform or the scheduler effect
 * matrix and is projected into screenVertex. Returns 0 when clipped, else 1 with screenSize holding the projected
 * x and y extent. */
int sdvProjectEffectPoint(ScreenVertex *screenVertex, ApI4 *intPoint, int positionKind, int depth, AlignedVector4 *screenSize, int usePartTransform, ApM4 *localMatrix) {
    AlignedVector4 worldPos;
    AlignedVector4 scaledPoint;
    ApM4 cameraPos;
    ApM4 modelView;
    ScreenVertex secondVertex;
    ApM4 effectMatrix;
    StudioCamera *camera;
    PartScaleKeys *scaleKeys;
    SefScheduler *scheduler;
    ApM4 *viewMatrix;
    int schedulerEffectFlag;
    int useLocalMatrix;
    ApM4 *cameraPosPtr;
    if (positionKind == 8) {
        apIntVectorCopy(&screenVertex->ints, intPoint);
        screenVertex->ints.x += 0x100;
        screenVertex->ints.y += 0xE0;
        screenVertex->ints.z = depth << 8;
        return 1;
    }
    camera = xglStudioGetActiveCamera();
    scaleKeys = (PartScaleKeys *) sefGetBattleData();
    scheduler = sefGetNowScheduler();
    viewMatrix = &camera->matrix;
    schedulerEffectFlag = scheduler != 0 ? scheduler->flags & 0x1000000 : 0;
    useLocalMatrix = localMatrix != 0 && (positionKind & 0x100);
    apIntToFloatVector(&scaledPoint, intPoint);
    apVectorScale(&scaledPoint, &scaledPoint, 0.01f);
    cameraPosPtr = &cameraPos;
    scaledPoint.w = 1.0f;
    sefGetCameraPos(cameraPosPtr);
    MMathMulMatrix(&modelView, viewMatrix, cameraPosPtr);
    if (useLocalMatrix) {
        MMathApplyMatrix(&worldPos, localMatrix, &scaledPoint);
    } else {
        sefGetPosition(&worldPos, &scaledPoint, positionKind);
    }
    if (scheduler != 0) {
        if (usePartTransform) {
            PartTransform *partTransform = &gSdvCameraPartTransform;
            sefInitPartTransform(partTransform, positionKind, scaleKeys->scaleKey100, scaleKeys->scaleKey200);
            if (schedulerEffectFlag) {
                sefBuildEffectMatrix(&scheduler->effectAxes, &scheduler->effectOffset, &scheduler->effectScale, &effectMatrix);
            }
            if (useLocalMatrix) {
                apVectorFromMatrixTranslation(&partTransform->localTranslation, localMatrix);
                apMatrixTranslationSet(&partTransform->matrix, &partTransform->localTranslation);
            }
            if (partTransform->scaled != 0) {
                apVectorSub(&worldPos, &worldPos, &partTransform->localTranslation);
                if (schedulerEffectFlag) {
                    sefApplyMatrixVector(&worldPos, &effectMatrix, &worldPos);
                    apVectorAdd(&partTransform->matrix.rows[3], &partTransform->matrix.rows[3], &effectMatrix.rows[3]);
                }
                MMathMulMatrix(cameraPosPtr, viewMatrix, &partTransform->matrix);
                MMathScaleMatrixRotS(&modelView, &modelView, partTransform->scale);
                viewMatrix = cameraPosPtr;
            }
        } else if (schedulerEffectFlag) {
            apVectorAdd(&worldPos, &worldPos, &scheduler->effectOffset);
            MMathScaleMatrix(&modelView, &modelView, &scheduler->effectScale);
        }
    }
    MMathApplyMatrix(&modelView.rows[3], viewMatrix, &worldPos);
    if (MMathRotTransPersClip(screenVertex, camera, &modelView, &D_0053E9C0) != 0) {
        return 0;
    }
    MMathRotTransPersClip(&secondVertex, camera, &modelView, screenSize);
    screenSize->x = intAbs(screenVertex->ints.x - secondVertex.ints.x) >> 4;
    screenSize->y = intAbs(screenVertex->ints.y - secondVertex.ints.y) >> 4;
    screenVertex->ints.x = (screenVertex->ints.x >> 4) - 0x700;
    screenVertex->ints.y = (screenVertex->ints.y >> 4) - 0x720;
    if (depth != 0) {
        screenVertex->ints.z += depth << 8;
        if (screenVertex->ints.z > 0xF00000) {
            screenVertex->ints.z = 0xF00000;
        }
    }
    return 1;
}

/* Clears every special record and the gif mode hook state. */
void sdvResetSpecialWork(void)
{
    memset(gSdvSpecialRecords, 0, 4096);
    gifStateClearModeHook();
}

/* The first special record from 1 whose inUse word is zero, or 0 when all are taken. */
int sdvFindFreeSpecialWork(void)
{
    SdvSpecialRecord *specialRecords = gSdvSpecialRecords;
    SdvSpecialRecord *specialRecord;
    int freeSpecialSlot;

    specialRecord = &specialRecords[1];
    for (freeSpecialSlot = 1; freeSpecialSlot < 32; freeSpecialSlot++, specialRecord++) {
        if (specialRecord->inUse == 0) {
            return freeSpecialSlot;
        }
    }
    return 0;
}

/* Clears the record of one special slot; slot 0 is kept. */
void sdvClearSpecialRecord(int specialSlot)
{
    if (specialSlot > 0) {
        memset(&gSdvSpecialRecords[specialSlot], 0, sizeof(SdvSpecialRecord));
    }
}

/* One special record as the effect code reads it: the first word is unmodeled, the second is the key table of the
   record while it plays. Kept apart from SdvSpecialRecord: the field types decide the emitted order. */
typedef struct SdvSpecialSlot {
    int unmodeled_00;
    void *keyTable;
    unsigned char unmodeled_08[0x78];
} SdvSpecialSlot;

/* The owner of up to six special records of one key sequence: the head matches SdvSequence (keys at +0x04, row
   count at +0x08, row stride at +0x09); the special ids and their channels follow. */
typedef struct SdvSlotOwner {
    s16 unmodeled_00;
    signed char unmodeled_02[2];
    s16 *keys;
    signed char count;
    signed char stride;
    signed char unmodeled_0A[2];
    signed char specialIds[6];
    signed char channels[6];
} SdvSlotOwner;

#define SDV_SPECIAL_SLOTS ((SdvSpecialSlot *) gSdvSpecialRecords)
#define SDV_SLOT_RELEASE ((void (*)(int, int)) slotReleaseChecked)
#define SDV_CLEAR_SPECIAL_RECORD ((void (*)(int)) sdvClearSpecialRecord)

/* The special record of an id, or null when the id is out of range. */
static inline SdvSpecialSlot *specialSlotAt(int id) {
    if ((unsigned int) id >= 32) {
        return 0;
    }
    return &SDV_SPECIAL_SLOTS[id];
}

/* Gives back the special records of an owner: each id above zero releases its slot when the record still plays,
   then the id and its channel are cleared. */
void sdvReleaseOwnerSpecials(SdvSlotOwner *owner) {
    int i;
    signed char *ownerCountRef;

    if (owner == 0 || owner->keys == 0) {
        return;
    }
    ownerCountRef = &owner->count;
    i = 0;
    if (*ownerCountRef <= 0) {
        return;
    }
    do {
        int id = owner->specialIds[i];

        if (id > 0) {
            SdvSpecialSlot *slot = specialSlotAt(id);

            if (slot->keyTable != 0) {
                slot->keyTable = 0;
                SDV_SLOT_RELEASE(owner->channels[i], 1);
                SDV_CLEAR_SPECIAL_RECORD(id);
            }
            owner->specialIds[i] = 0;
            owner->channels[i] = 0;
        }
        i++;
        if (i >= owner->count) {
            break;
        }
    } while (i < 6);
}


void *func_002046F8(void *buffer, const void *table, int mode);
void func_00206058(AlignedVector4 *out, void *in, int zero, int selector);
extern const Matrix4 MMathUnitMatrix;

/* A vector seen as its aligned quadword or as four floats. */
typedef union EffectVectorView {
    AlignedVector4 vector;
    f32 floats[4];
} EffectVectorView;

/* The key of an effect entry: the table selector and the transform of its vector, then the three vector values. */
typedef struct EffectKey {
    s16 unmodeled_00;
    s16 tableSelector;
    s16 transform;
    s16 vectorX;
    s16 vectorY;
    s16 vectorZ;
} EffectKey;

/* One effect entry of a special record: its matrix (from the table, or the unit matrix) and its integer vector. */
typedef struct EffectEntry {
    int unmodeled_00;
    EffectKey *key;
    unsigned char unmodeled_08[0x20];
    s16 unmodeled_28;
    s16 matrixFound;
    unsigned char unmodeled_2C[4];
    ApM4 matrix;
    ApI4 intVector;
} EffectEntry;

/* Copies the four matrix quadwords from source to destination (lq/sq through $8-$11). */
static inline void effectCopyMatrix(ApM4 *destination, ApM4 *source) {
    __asm__ __volatile__("lq $8, 0(%1)\n lq $9, 16(%1)\n lq $10, 32(%1)\n lq $11, 48(%1)\n"
                         "sq $8, 0(%0)\n sq $9, 16(%0)\n sq $10, 32(%0)\n sq $11, 48(%0)"
                         :
                         : "r"(destination), "r"(source)
                         : "$8", "$9", "$10", "$11", "memory");
}

/* Stores the unit matrix (MMathUnitMatrix columns through vf1-vf3, vf0 as the fourth) into destination. */
static inline void effectStoreUnitMatrix(ApM4 *destination) {
    __asm__ __volatile__("lqc2 vf1, 32(%1)\n lqc2 vf2, 16(%1)\n lqc2 vf3, 0(%1)\n"
                         "sqc2 vf0, 48(%0)\n sqc2 vf1, 32(%0)\n sqc2 vf2, 16(%0)\n sqc2 vf3, 0(%0)"
                         :
                         : "r"(destination), "r"(&MMathUnitMatrix)
                         : "memory");
}

/* Converts the vector source to integers (vftoi0) and stores it into destination. */
static inline void effectVectorToInt(ApI4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lqc2 vf1, 0(%1)\n vftoi0.xyzw vf1, vf1\n sqc2 vf1, 0(%0)"
                         :
                         : "r"(destination), "r"(source)
                         : "memory");
}

/* Fills the matrix and the integer vector of an effect entry from its key: the matrix is the table's, or the unit
   matrix when the table has none; transform 0x16 clears the vector. */
void sdvSetupEffectEntry(EffectEntry *effectEntry) {
    AlignedVector4 transformed;
    EffectVectorView keyVector;
    int transform;
    int tableSelector;
    ApM4 *tableMatrix;

    effectEntry->unmodeled_28 = 0;
    effectEntry->matrixFound = 0;
    effectEntry->unmodeled_00 = 0;
    if (effectEntry->key != 0) {
        tableSelector = effectEntry->key->tableSelector;
        transform = effectEntry->key->transform;
        tableMatrix = func_002046F8(&keyVector.vector, (*(char (*)[]) & D_0053E9C0), tableSelector);
        if (tableMatrix != 0) {
            effectEntry->matrixFound = 1;
            effectCopyMatrix(&effectEntry->matrix, tableMatrix);
        } else {
            effectEntry->matrixFound = 0;
            effectStoreUnitMatrix(&effectEntry->matrix);
        }
        if (transform != 0x16) {
            keyVector.floats[0] = effectEntry->key->vectorX;
            keyVector.floats[1] = effectEntry->key->vectorY;
            keyVector.floats[2] = effectEntry->key->vectorZ;
            func_00206058(&transformed, &keyVector.vector, 0, transform);
            effectVectorToInt(&effectEntry->intVector, &transformed);
        } else {
            sefMemZero(&effectEntry->intVector, 16);
        }
    }
}

/* One keyframe: its frame and fifteen values. Frame 0x2000 is the always-due marker of SdvSlotEntry; 0x1000 is
   kept as the end marker of a track (not proven beyond the comparisons). */
typedef struct SdvKeyframe {
    unsigned short frame;
    short components[15];
} SdvKeyframe;

/* A key table: twelve header bytes, then its keyframes. */
typedef struct SdvKeyTable {
    unsigned char unmodeled_00[12];
    SdvKeyframe keys[1];
} SdvKeyTable;

/* The track of one special record while its effect plays: the time, the table, the interpolated key and the index
   of the key segment. Kept apart from SdvSpecialSlot: the field types decide the emitted order. */
typedef struct SdvSpecialTrack {
    float time;
    SdvKeyTable *table;
    SdvKeyframe key;
    short keyIndex;
    unsigned short unmodeled_2A;
    unsigned char unmodeled_2C[0x54];
} SdvSpecialTrack;

#define SDV_LERP(start, end, fraction) \
    ((end) == (start) ? (float) (start) : (float) ((end) - (start)) * (fraction) + (float) (start))

/* The track of a special record id, or null when the id is out of range. */
static inline SdvSpecialTrack *specialTrackAt(int id) {
    SdvSpecialTrack *track = 0;

    if ((unsigned) id < 32) {
        track = &((SdvSpecialTrack *) gSdvSpecialRecords)[id];
    }
    return track;
}

/* A keyframe read either as its fields or as sixteen halfwords. */
typedef union SdvKeyframeWords {
    SdvKeyframe key;
    short words[16];
} SdvKeyframeWords;

/* Steps the key segment of a special track to its new time and interpolates its current key. Returns the id while the
   track plays; releases the slot (and returns 0) once the track has run past its last key. */
int sdvStepSpecialTrack(signed char id, signed char slot) {
    SdvSpecialTrack *track;
    SdvKeyframe *key;
    SdvKeyframe *nextKey;
    SdvKeyframe *trailingKey;
    float fraction;

    track = specialTrackAt(id);
    if (track == 0 || track->table == 0) {
        return id;
    }
    {
        SdvKeyframe *keys = track->table->keys;

        key = &keys[track->keyIndex];
        nextKey = key + 1;
        trailingKey = nextKey;
    }
    if (key->frame != 0x1000 && (key->frame == 0x2000 || !(track->time < (float) key->frame))) {
        if (nextKey->frame != 0x2000 && track->time >= (float) key->frame && nextKey->frame >= 0x1000) {
            track->table = 0;
            goto slotReleasePath;
        }
        /* kept: this unreachable release path decides the emitted order of the code after it */
        if (0) {
            track->table = 0;
            slotReleaseChecked(slot, 0);
            sdvClearSpecialRecord(id);
            return 0;
        }
        if (key->frame == 0x2000 && nextKey->frame >= 0x1000) {
            track->key = *key;
            ((SdvKeyframeWords *) &track->key)->words[0] = 0;
        } else if (key->frame != nextKey->frame) {
            fraction = (track->time - (float) key->frame) / (float) (nextKey->frame - key->frame);
            track->key.components[0] = (short) SDV_LERP(key->components[0], nextKey->components[0], fraction);
            track->key.components[1] = (short) SDV_LERP(key->components[1], nextKey->components[1], fraction);
            track->key.components[2] = (short) SDV_LERP(key->components[2], nextKey->components[2], fraction);
            track->key.components[3] = (short) SDV_LERP(key->components[3], nextKey->components[3], fraction);
            track->key.components[4] = (short) SDV_LERP(key->components[4], nextKey->components[4], fraction);
            track->key.components[5] = (short) SDV_LERP(key->components[5], nextKey->components[5], fraction);
            track->key.components[6] = (short) SDV_LERP(key->components[6], nextKey->components[6], fraction);
            track->key.components[7] = (short) SDV_LERP(key->components[7], nextKey->components[7], fraction);
            track->key.components[8] = (short) SDV_LERP(key->components[8], nextKey->components[8], fraction);
            track->key.components[9] = (short) SDV_LERP(key->components[9], nextKey->components[9], fraction);
            track->key.components[10] = (short) SDV_LERP(key->components[10], nextKey->components[10], fraction);
            track->key.components[11] = (short) SDV_LERP(key->components[11], nextKey->components[11], fraction);
            track->key.components[12] = (short) SDV_LERP(key->components[12], nextKey->components[12], fraction);
            track->key.components[13] = (short) SDV_LERP(key->components[13], nextKey->components[13], fraction);
            track->key.components[14] = (short) SDV_LERP(key->components[14], nextKey->components[14], fraction);
        }
    }
    if (!(track->time < 4096.0f) || key->frame == 0x2000) {
        return id;
    }
    track->time = (float) (int) ((track->time + gSdvFrameDelta[0] + 0.0001f) * 1000.0f) / 1000.0f;
    if (track->time >= (float) nextKey->frame) {
        /* kept: the empty do/while decides where the stepping block is scheduled */
        do {
        } while (0);
        goto advanceSegment;
    releaseTrack:
        track->table = 0;
    slotReleasePath:
        slotReleaseChecked(slot, 0);
        sdvClearSpecialRecord(id);
        return 0;
    advanceSegment:
        track->keyIndex++;
        nextKey++;
        if (trailingKey->frame != 0x2000 && track->time >= (float) trailingKey->frame && nextKey->frame >= 0x1000) {
            goto releaseTrack;
        }
        trailingKey++;
        if (track->time >= (float) nextKey->frame) {
            goto advanceSegment;
        }
    }
    return id;
}

typedef signed char s8;

typedef int s32;


/* Same record as specialSlotAt, reached through the signed index of the sequence owners. Both stay: an unused
   static inline is still emitted, so the two definitions are kept. */
static inline SdvSpecialSlot *specialSlotAtSigned(s32 id) {
    if ((u32) id >= 32) {
        return 0;
    }
    return &SDV_SPECIAL_SLOTS[id];
}

int sdvFindFreeSpecialWork(void);
int sdvCheckSeqKey(SdvSequence *sequence);

/* Keeps the special records of a sequence owner fed: each key that comes due takes a free record and starts its
   effect, then every owned record is stepped by sdvStepSpecialTrack. base is the scratchpad base of the key lookup. */
void sdvFeedOwnerSpecials(SdvSlotOwner *owner, s32 base) {
    s32 i;
    u32 t;

    if (owner == 0 || owner->keys == 0) {
        return;
    }
    t = sdvStepSequence((SdvSequence *) owner);
    while (t < 0x1000) {
        for (i = 0; i < 6; i++) {
            if (owner->specialIds[i] <= 0) {
                owner->specialIds[i] = sdvFindFreeSpecialWork();
                owner->channels[i] = 0;
                if (owner->specialIds[i] > 0) {
                    SdvSpecialSlot *slot = specialSlotAtSigned(owner->specialIds[i]);

                    slot->keyTable = ((void *(*) (s32, s16 *, s32)) scGetImmAdrImmIdx2)(base, owner->keys + t * owner->stride, 1);
                    sdvSetupEffectEntry((EffectEntry *) slot);
                    break;
                }
            }
        }
        if (i == 6) {
            break;
        }
        t = sdvCheckSeqKey((SdvSequence *) owner);
    }
    for (i = 0; i < owner->count; i++) {
        if (i >= 6) {
            break;
        }
        owner->specialIds[i] = sdvStepSpecialTrack(owner->specialIds[i], owner->channels[i]);
    }
}

extern s32 D_0055109C;

/* 1 while the address in D_0055109C lies in the window 0x1013FFFF < address <= 0x1014D003, else 0. */
s32 sdvAddressInWindow(void) {
    s32 addressIsInWindow;

    addressIsInWindow = 0;
    if (D_0055109C <= 0x1014D003) {
        addressIsInWindow = D_0055109C > 0x1013FFFF;
    }
    return addressIsInWindow;
}

/* One effect record of a slot event: entries[] start at +0x0C, 32 bytes each, the first halfword is the frame the
   effectEntry becomes due (0x2000: always). */
typedef struct SdvSlotEntry {
    unsigned short frame;
    unsigned char unmodeled_02[0x1E];
} SdvSlotEntry;

typedef struct SdvSlotData {
    unsigned short kind;     /* +0x00 low byte: effect opcode; high byte (signed): target passed to sdvProjectEffectPoint */
    unsigned short mode;     /* +0x02 8: arguments are absolute (not scaled) */
    unsigned char unmodeled_04[8];
    SdvSlotEntry entries[1]; /* +0x0C */
} SdvSlotData;

typedef struct SdvSlotEvent {
    float frame;             /* +0x00 */
    SdvSlotData *data;       /* +0x04 */
    short args[16];          /* +0x08 */
    short entryIndex;        /* +0x28 */
    unsigned short windowed; /* +0x2A nonzero: the window at +0x30 limits the effect */
    unsigned char unmodeled_2C[4];
    unsigned char window[0x40]; /* +0x30 */
    int offset[3];           /* +0x70 added to the event position */
} SdvSlotEvent;

extern short D_00408090[];
int slotSetupMode0(int slot, int opcode, int *setup);
int slotUpdateMode1(int slot, int opcode, int *setup);
void sefGetMatrixPoint(Vector4 *destination, Vector4 *source, int mode);
void MMathInvertAffineMatrix(Matrix4 *out, float *src);
void xglMatrixRotX(float *matrix, float *source, float angle);
void xglMatrixRotY(float *matrix, float *source, float angle);
void xglMatrixRotZ(float *matrix, float *source, float angle);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);
XglCamera *xglStudioSelectGetActiveCamera(int index);

static inline void normalizeInPlace(Vector4 *direction)
{
    MMathNormalizeVector2(direction, direction);
}

/* SdvSlotData.mode: the event arguments are absolute coordinates, not scaled to the located target. */
#define SDV_ALTER_MODE_SINGLE 8

/* SdvSlotEntry.frame: the effectEntry is always due. */
#define SDV_ENTRY_ALWAYS_DUE 0x2000

/* Starts (update == 0) or updates the slot effect of one event: decodes the opcode's arguments into the setup words
   and hands them to the slot system; slot receives the slot number. */
void sdvApplySlotEvent(signed char *slot, SdvSlotEvent *event, int update)
{
    int position[4];
    int located[4];
    float scale[4];
    int setup[15];
    AlignedVector4 vector;
    Matrix4 matrix;
    double percentScale;
    SdvSlotData *effectData;
    SdvSlotEntry *dueEntry;
    SdvSlotEntry *effectEntries;
    short *eventArgs;
    int opcode;
    int targetId;
    int argumentMode;
    void *window;
    int sizeOrDepth;
    unsigned int tableIndex;
    int schedulerBit22;
    int depth;
    long long *studioBlock;
    unsigned short kindWord;
    int windowActive;
    XglCamera *camera;

    if (event == 0) {
        return;
    }
    effectData = event->data;
    if (effectData == 0) {
        return;
    }
    effectEntries = effectData->entries;
    kindWord = effectData->kind;
    eventArgs = event->args;
    dueEntry = &effectEntries[event->entryIndex];
    argumentMode = effectData->mode;
    opcode = kindWord & 0xFF;
    targetId = (short) kindWord >> 8;
    windowActive = sdvAddressInWindow();
    if (event->windowed != 0 && windowActive != 0) {
        window = event->window;
    } else {
        window = 0;
    }
    memset(setup, 0, sizeof(setup));
    if (dueEntry->frame != SDV_ENTRY_ALWAYS_DUE && (float) dueEntry->frame > event->frame) {
        return;
    }
    Vu0SetOrigin(position);

    switch (opcode) {
    case 4:
        setup[0] = eventArgs[1];
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 5:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2] + 0x100;
        setup[2] = eventArgs[3] + 0xE0;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        sizeOrDepth = 1;
        for (tableIndex = 0; tableIndex < 9; tableIndex++) {
            if (D_00408090[tableIndex] >= eventArgs[6]) {
                sizeOrDepth = D_00408090[tableIndex];
                break;
            }
        }
        setup[5] = sizeOrDepth;
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        setup[9] = eventArgs[10];
        setup[10] = eventArgs[11];
        break;
    case 6:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2] + 0x100;
        setup[2] = eventArgs[3] + 0xE0;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 9:
        setup[0] = eventArgs[1];
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        break;
    case 29:
        setup[0] = eventArgs[1];
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        break;
    case 12:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 5.12f;
        scale[1] = 4.48f;
        if (sdvProjectEffectPoint(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            setup[3] = eventArgs[5];
            setup[4] = eventArgs[6];
            setup[5] = eventArgs[7];
            setup[6] = eventArgs[8];
        }
        break;
    case 38:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 100.0f;
        scale[1] = 100.0f;
        if (sdvProjectEffectPoint(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            float factor = scale[0] * 0.0001f;

            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ALTER_MODE_SINGLE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7];
            } else {
                setup[3] = eventArgs[5] * factor;
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7] * factor;
            }
            setup[6] = eventArgs[8];
            setup[7] = eventArgs[9];
            setup[8] = eventArgs[9];
            setup[9] = eventArgs[11];
            setup[10] = eventArgs[12];
            setup[11] = eventArgs[13];
        }
        break;
    case 14:
        vector.x = eventArgs[2] + event->offset[0];
        vector.y = eventArgs[3] + event->offset[1];
        vector.z = eventArgs[4] + event->offset[2];
        vector.w = 1.0f;
        normalizeInPlace(&vector);
        xglVectorScaleXYZ(&vector, &vector, eventArgs[5]);
        camera = xglStudioSelectGetActiveCamera(0);
        sefGetMatrixPoint(&vector, &vector, argumentMode);
        vector.w = 1.0f;
        xglMatrixUnit(matrix);
        xglMatrixRotY(matrix[0], matrix[0], camera->travel.rotation.y);
        xglMatrixRotX(matrix[0], matrix[0], camera->travel.rotation.x);
        xglMatrixRotZ(matrix[0], matrix[0], camera->travel.rotation.z);
        MMathInvertAffineMatrix(&matrix, matrix[0]);
        xglVectorMulMat(&vector, matrix, &vector);
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = vector.x;
        setup[2] = vector.y;
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 13:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 100.0f;
        scale[1] = 100.0f;
        if (sdvProjectEffectPoint(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            float factor = scale[0] * 0.0001f;

            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ALTER_MODE_SINGLE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7];
                setup[7] = eventArgs[9];
            } else {
                setup[3] = eventArgs[5] * factor;
                setup[4] = eventArgs[6] * factor;
                setup[5] = eventArgs[7] * factor;
                setup[7] = eventArgs[9] * factor;
            }
            setup[6] = eventArgs[8];
            setup[8] = eventArgs[10];
            setup[9] = eventArgs[11];
            setup[10] = eventArgs[12];
            setup[11] = eventArgs[13];
            setup[12] = eventArgs[14];
            setup[13] = eventArgs[15];
        }
        break;
    case 17:
    case 18:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        scale[0] = 100.0f;
        scale[1] = 100.0f;
        if (sdvProjectEffectPoint(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            float factor = scale[0] * 0.0001f;

            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ALTER_MODE_SINGLE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
                setup[5] = eventArgs[7];
                setup[6] = eventArgs[8];
                setup[7] = eventArgs[9];
            } else {
                setup[3] = eventArgs[5] * factor;
                setup[4] = eventArgs[6] * factor;
                setup[5] = eventArgs[7] * factor;
                setup[6] = eventArgs[8] * factor;
                setup[7] = eventArgs[9] * factor;
            }
            setup[8] = eventArgs[10];
            setup[9] = eventArgs[11];
            setup[10] = eventArgs[12];
        }
        break;
    case 20:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        break;
    case 25:
        setup[0] = eventArgs[1] + event->offset[0];
        setup[1] = eventArgs[2] + event->offset[1];
        if (argumentMode == SDV_ALTER_MODE_SINGLE) {
            sizeOrDepth = (unsigned short) eventArgs[3] << 8;
        } else {
            sizeOrDepth = eventArgs[3] + event->offset[2];
        }
        setup[2] = sizeOrDepth;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        setup[9] = eventArgs[10];
        setup[10] = eventArgs[11] | (eventArgs[12] << 8) | (eventArgs[13] << 16) | (eventArgs[14] << 24);
        setup[11] = argumentMode | (targetId << 16);
        setup[12] = (int) sefGetNowScheduler();
        setup[14] = eventArgs[15];
        break;
    case 27:
        setup[0] = eventArgs[1] + event->offset[0];
        setup[1] = eventArgs[2] + event->offset[1];
        if (argumentMode == SDV_ALTER_MODE_SINGLE) {
            depth = (unsigned short) eventArgs[3] << 8;
        } else {
            depth = eventArgs[3] + event->offset[2];
        }
        setup[2] = depth;
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        setup[9] = eventArgs[10];
        setup[10] = eventArgs[11];
        setup[11] = eventArgs[12] | (eventArgs[13] << 16);
        setup[12] = eventArgs[14];
        setup[13] = argumentMode | (targetId << 16);
        setup[14] = (int) sefGetNowScheduler();
        break;
    case 21:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        break;
    case 37:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        break;
    case 22:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        percentScale = 0.01;
        scale[0] = eventArgs[6] * percentScale;
        scale[1] = eventArgs[7] * percentScale;
        if (sdvProjectEffectPoint(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            setup[0] = located[2];
            setup[1] = eventArgs[8];
            setup[2] = eventArgs[9];
            setup[3] = eventArgs[10];
            setup[4] = eventArgs[11];
            setup[5] = eventArgs[12];
            setup[6] = eventArgs[13];
            setup[7] = eventArgs[5];
            setup[8] = located[0];
            setup[9] = located[1];
            if (argumentMode == SDV_ALTER_MODE_SINGLE) {
                setup[10] = eventArgs[6];
                setup[11] = eventArgs[7];
            } else {
                setup[10] = scale[0];
                setup[11] = scale[1];
            }
        }
        break;
    case 19:
    case 23:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        break;
    case 11:
    case 24:
        setup[0] = (unsigned short) eventArgs[1] << 8;
        setup[1] = eventArgs[2];
        setup[2] = eventArgs[3];
        setup[3] = eventArgs[4];
        setup[4] = eventArgs[5];
        setup[5] = eventArgs[6];
        setup[6] = eventArgs[7];
        setup[7] = eventArgs[8];
        setup[8] = eventArgs[9];
        break;
    case 26:
        position[0] = eventArgs[2] + event->offset[0];
        position[1] = eventArgs[3] + event->offset[1];
        position[2] = eventArgs[4] + event->offset[2];
        Vu0SetOrigin(scale);
        percentScale = 0.01;
        scale[0] = eventArgs[5] * percentScale;
        scale[1] = eventArgs[6] * percentScale;
        if (sdvProjectEffectPoint(located, position, argumentMode, (unsigned short) eventArgs[1], scale, targetId, window) != 0) {
            setup[0] = located[2];
            setup[1] = located[0];
            setup[2] = located[1];
            if (argumentMode == SDV_ALTER_MODE_SINGLE) {
                setup[3] = eventArgs[5];
                setup[4] = eventArgs[6];
            } else {
                setup[3] = scale[0];
                setup[4] = scale[1];
            }
            setup[5] = eventArgs[7];
            setup[6] = eventArgs[8];
            setup[7] = eventArgs[9];
            setup[8] = eventArgs[10];
        }
        break;
    default:
        return;
    }

    /* copies bit 22 of the scheduler flags into bit 32 of studio block word 5 */
    studioBlock = gSbStudioBlock;
    if (sefGetNowScheduler() != 0) {
        schedulerBit22 = (sefGetNowScheduler()->flags >> 22) & 1;
    } else {
        schedulerBit22 = 0;
    }
    studioBlock[5] = (studioBlock[5] & ~((long long) 1 << 32)) | ((long long) schedulerBit22 << 32);
    if (update != 0) {
        *slot = slotUpdateMode1(*slot, opcode, setup);
    } else {
        *slot = slotSetupMode0(*slot, opcode, setup);
    }
}

typedef struct SdvSlotEventList {
    unsigned char unmodeled_00[8];
    signed char count;
    unsigned char unmodeled_09[3];
    signed char eventIds[6];
    signed char slotIds[6];
} SdvSlotEventList;

/* Starts the slot effects of an event list: each of its first six entries names a special record (its event) and
   the slot that plays it; an id out of range starts nothing. */
void sdvStartSlotEvents(SdvSlotEventList *list) {
    int i;
    unsigned int eventId;
    SdvSlotEvent *event;

    for (i = 0; i < list->count && i < 6; i++) {
        eventId = list->eventIds[i];
        event = 0;
        if (eventId < 32) {
            event = (SdvSlotEvent *) &gSdvSpecialRecords[eventId];
        }
        sdvApplySlotEvent(&list->slotIds[i], event, 0);
    }
}

typedef struct SdvAlterRequest {
    unsigned int tex0[2][2];          /* +0x00 GS TEX0 of the two images, low word first */
    struct SdvSefRequest *sefRequest; /* +0x10 */
    unsigned char rgba[4];            /* +0x14 */
    unsigned int anchor;              /* +0x18 nonzero: the position comes from the anchor */
    short anchorTarget;               /* +0x1C allocator index of the anchor's target; negative for none */
    unsigned char unmodeled_1E[0x24 - 0x1E];
    float length; /* +0x24 */
    unsigned char unmodeled_28[4];
    float imageFrames;     /* +0x2C */
    float life;            /* +0x30 */
    float ringSpacing;     /* +0x34 */
    float height;          /* +0x38 */
    float minDelta;        /* +0x3C */
    short parts[2];        /* +0x40 */
    short maxSamples;      /* +0x44 */
    unsigned short shrink; /* +0x46 */
    short blend;           /* +0x48 */
    short imageCount;      /* +0x4A */
    short mode;            /* +0x4C */
    unsigned char unmodeled_4E[2];
    short layer;         /* +0x50 */
    short owner;         /* +0x52 */
    short follow;        /* +0x54 */
    short spread;        /* +0x56 */
    unsigned short spin; /* +0x58 */
    short timeBase;      /* +0x5A */
    short timeStep;      /* +0x5C */
    unsigned char unmodeled_5E[2];
    float scale60;               /* +0x60 */
    unsigned short unmodeled_64; /* +0x64 */
    unsigned char unmodeled_66[2];
} SdvAlterRequest;

typedef struct SdvAlterTex0View {
    unsigned long long tex0[2];
} SdvAlterTex0View;

typedef struct SdvAlterEvent {
    unsigned char unmodeled_00[2];
    short life;                  /* +0x02 */
    unsigned short mode;         /* +0x04 low nibble the mode, high 12 bits the follow flag */
    unsigned short blend;        /* +0x06 */
    unsigned short rgba[4];      /* +0x08 */
    unsigned short parts[2];     /* +0x10 */
    short maxSamples;            /* +0x14 */
    unsigned short shrink;       /* +0x16 */
    short imageFrames;           /* +0x18 */
    short length;                /* +0x1A */
    short images[2][2];          /* +0x1C */
    float ringSpacing;           /* +0x24 */
    float height;                /* +0x28 */
    unsigned short spread;       /* +0x2C */
    unsigned short spin;         /* +0x2E */
    unsigned short timeBase;     /* +0x30 */
    unsigned short timeStep;     /* +0x32 */
    short minDelta;              /* +0x34 in 1/1000 */
    unsigned short unmodeled_36; /* +0x36 */
    float scale38;               /* +0x38 */
    short scriptEntry;           /* +0x3C */
} SdvAlterEvent;


int sdvAllocAlter(SdvAlterRequest *alterRequest, int specialTag, int unused);
int sefGetLinkedFlag(int linkedPart);
extern unsigned short D_004080A8[];
extern unsigned short D_004080E8[];

/* Starts the alters of one sequence: each key of the sequence fills an SdvAlterRequest from its SdvAlterEvent and
   starts it through sdvAllocAlter. Keys whose first part is 321 or 322 start once per linked part of D_004080A8 or
   D_004080E8 instead. */
void sdvStartSequenceAlters(SdvSequence *sequence, struct SdvSefRequest *sefRequest, int layer, int owner,
                   int scriptBase) {
    SdvAlterRequest alterRequest;
    SdvAlterEvent *keyEvent;
    SvTypeListEntry *imageItem;
    SvTypeListEntry *paletteItem;
    unsigned short *partIds;
    int keyStep;
    int i;
    int partIndex;
    int specialTag;
    short imageIndex;
    short paletteIndex;
    int pixelStorage;
    unsigned short firstPartId;

    if (sequence == 0) {
        return;
    }
    if (sequence->keys == 0) {
        return;
    }
    for (keyStep = sdvStepSequence(sequence); (unsigned int) keyStep < 4096; keyStep = sdvCheckSeqKey(sequence)) {
        keyEvent = (SdvAlterEvent *) &sequence->keys[keyStep * sequence->stride];
        if (keyEvent->scriptEntry >= 0) {
            specialTag = scGetImmAdrImmIdx2(scriptBase, (unsigned short *) &keyEvent->scriptEntry, 0);
        } else {
            specialTag = 0;
        }
        memset(&alterRequest, 0, sizeof(alterRequest));
        alterRequest.life = keyEvent->life;
        alterRequest.parts[0] = keyEvent->parts[0];
        alterRequest.sefRequest = sefRequest;
        alterRequest.parts[1] = keyEvent->parts[1];
        alterRequest.maxSamples = keyEvent->maxSamples > 64 ? 64 : keyEvent->maxSamples;
        alterRequest.shrink = keyEvent->shrink;
        alterRequest.mode = keyEvent->mode & 0xF;
        alterRequest.follow = (short) keyEvent->mode >> 4;
        alterRequest.blend = keyEvent->blend;
        alterRequest.rgba[0] = keyEvent->rgba[0];
        alterRequest.rgba[1] = keyEvent->rgba[1];
        alterRequest.rgba[2] = keyEvent->rgba[2];
        alterRequest.rgba[3] = keyEvent->rgba[3];
        alterRequest.imageFrames = keyEvent->imageFrames;
        alterRequest.length = keyEvent->length;
        alterRequest.layer = layer;
        alterRequest.owner = owner;
        alterRequest.spread = keyEvent->spread;
        alterRequest.ringSpacing = keyEvent->ringSpacing;
        alterRequest.spin = keyEvent->spin;
        alterRequest.height = keyEvent->height;
        alterRequest.timeBase = keyEvent->timeBase;
        alterRequest.timeStep = keyEvent->timeStep;
        alterRequest.minDelta = keyEvent->minDelta * 0.001f;
        alterRequest.scale60 = keyEvent->scale38 * 0.01f;
        alterRequest.unmodeled_64 = keyEvent->unmodeled_36;
        if (alterRequest.follow != 0 && alterRequest.maxSamples > 32) {
            alterRequest.maxSamples = 32;
        }
        for (i = 0; i < 2; i++) {
            imageIndex = keyEvent->images[i][0];
            paletteIndex = keyEvent->images[i][1];
            if (imageIndex >= 0 && paletteIndex >= 0) {
                imageItem = svGetImageListItem(layer, imageIndex);
                paletteItem = svGetImageListItem(layer, paletteIndex);
                if (imageItem != 0 && paletteItem != 0) {
                    pixelStorage = imageItem->bitsPerPixel == 8 ? GS_PSMT8 : GS_PSMT4;
                    ((SdvAlterTex0View *) &alterRequest)->tex0[alterRequest.imageCount] = GS_SET_TEX0(
                        imageItem->basePointer, imageItem->bufferWidthPixels >> 6, pixelStorage, imageItem->textureWidthExponent,
                        imageItem->textureHeightExponent, 1, 0, paletteItem->basePointer, 0, 0, 0, 1);
                    alterRequest.imageCount++;
                    alterRequest.layer = layer;
                }
            }
        }
        firstPartId = keyEvent->parts[0];
        if ((unsigned int) (firstPartId - 321) < 2) {
            partIds = (short) firstPartId == 321 ? D_004080A8 : D_004080E8;
            for (partIndex = 0; partIds[partIndex] != 0; partIndex++) {
                if (sefGetLinkedFlag(partIds[partIndex] - 257) != 0) {
                    alterRequest.parts[0] = partIds[partIndex];
                    sdvAllocAlter(&alterRequest, specialTag, 0);
                }
            }
        } else {
            sdvAllocAlter(&alterRequest, specialTag, 0);
        }
    }
}

void sdvInitAlter(SdvAlter *alter)
{
    sefMemZero(alter, sizeof(*alter));
}

/* The alter header as SdvAlterSpawnView reads it. Proposal: merge with SdvAlterHeader of the owner region. */
typedef struct SdvAlterHeaderFields {
    unsigned char unmodeled_00[4];
    short frame;            /* +0x04 */
    short index;            /* +0x06 */
    unsigned short active;  /* +0x08 */
    unsigned short wrapped; /* +0x0A */
    short serial;           /* +0x0C */
} SdvAlterHeaderFields;

/* The 0xE70-byte alter record as the spawn code (sdvAllocAlter) writes it. Kept apart from SdvAlterBody: the field
   types decide the emitted order. */
typedef struct SdvAlterSpawnView {
    AlignedVector4 orientation; /* +0x000 */
    unsigned char unmodeled_010[0x50 - 0x10];
    SdvVectorView points[2][64];     /* +0x050 */
    AlignedVector4 quats[33];        /* +0x850 */
    AlignedVector4 heading;          /* +0xA60 */
    AlignedVector4 lastDelta;        /* +0xA70 */
    AlignedVector4 position;         /* +0xA80 */
    AlignedVector4 previousPosition; /* +0xA90 */
    SdvAlterRequest request;  /* +0xAA0 */
    unsigned char unmodeled_B08[0xC08 - 0xB08];
    float progress;   /* +0xC08 */
    float angles[32]; /* +0xC0C */
    unsigned char unmodeled_C8C[4];
    float radii[32]; /* +0xC90 */
    unsigned char unmodeled_D10[0xE10 - 0xD10];
    SdvAlterHeaderFields alterHeader;
    signed char specialSlot; /* +0xE1E: the special record sdvClearSpecialRecord clears */
    signed char ownedSlot;   /* +0xE1F: first argument of slotReleaseChecked in sdvDestroyAlter */
    unsigned char unmodeled_E20[0xE60 - 0xE20];
    unsigned char unusedFlag; /* +0xE60 */
    unsigned char unmodeled_E61[0xE70 - 0xE61];
} SdvAlterSpawnView;

extern SdvAlterSpawnView gSdvAlters_1F1220[];
extern int sdvAlterSerial;
extern const AlignedVector4 D_0053E920;
extern const AlignedVector4 D_0053E930;
extern const AlignedVector4 D_0053E950;
float sefRandf(void);
SefEffectRecord *sefGetScheduler(void);
void sefNoiseNextVector(Vector4 *dest);
Matrix4 *MMathRotateMatrixXYZ(Matrix4 *out, Matrix4 *matrix, const Vector4 *angles);
void xglMatrix2Quaternion(Vector4 *dest, Matrix4 matrix);
Vector4 *sevGetPtAllocator2(int index);
int MMathQuatBetweenVectors(Vector4 *destination, const Vector4 *from, const Vector4 *to);
int func_002050A8(Vector4 *position, unsigned int *anchor, int part, int unused);

#define SdvCopyQuadword(destination, source) \
    __asm__ __volatile__("lq $8, 0(%1)\nsq $8, 0(%0)\n" \
                         : : "r" (destination), "r" (source), "m" (*(source)) : "$8", "memory")
#define SdvMulVectorXYZ(destination, first, second) \
    __asm__ __volatile__("lqc2 vf1, 0(%1)\nlqc2 vf2, 0(%2)\nvmul.xyz vf1, vf1, vf2\nsqc2 vf1, 0(%0)\n" \
                         : : "r" (destination), "r" (first), "r" (second), "m" (*(second)) : "memory")
#define SdvSubVectorXYZ(destination, first, second) \
    __asm__ __volatile__("lqc2 vf1, 0(%3)\nlqc2 vf2, 0(%4)\nvsub.xyz vf1, vf1, vf2\nsqc2 vf1, 0(%5)\n" \
                         : "=m" ((destination)->x), "=m" ((destination)->y), "=m" ((destination)->z) \
                         : "r" (first), "r" (second), "r" (destination) : "memory")
#define SDV_ALTER_COUNT 49
#define SDV_SPECIAL_COUNT 32

#define SDV_ALTER_RADIUS_RANDOM 0.1f
#define SDV_ALTER_RADIUS_UNIT 0.001f
#define SDV_ALTER_NOISE_ANGLE 0.17453292f
#define SDV_DEG_TO_RAD 0.017453292f
#define SDV_PI 3.1415927f

#define SEF_RECORD_BUILDS_MATRIX 0x01000000

static inline SdvSpecialRecord *specialRecordAt(unsigned int slot) {
    SdvSpecialRecord *specialRecord = 0;

    if (slot < SDV_SPECIAL_COUNT) {
        specialRecord = &gSdvSpecialRecords[slot];
    }
    return specialRecord;
}

/* Claims a free alter slot for alterRequest and returns (serial << 16) | slot, or -1 when all SDV_ALTER_COUNT slots are
   busy. Alters with follow set also get their start position, orientation, heading and sample ring here. */
int sdvAllocAlter(SdvAlterRequest *alterRequest, int specialTag, int unused) {
    AlignedVector4 position;
    Matrix4 matrix;
    AlignedVector4 normal;
    AlignedVector4 tangent;
    SdvAlterSpawnView *alter;
    SdvSpecialRecord *specialRecord;
    SefEffectRecord *effect;
    Vector4 *targetPoints;
    int hasTarget;
    int i;
    int k;
    int maxSamples;
    float radius;
    float angle;

    hasTarget = 0;
    if (alterRequest->anchorTarget >= 0) {
        hasTarget = alterRequest->anchor != 0;
    }
    for (i = 0; i < SDV_ALTER_COUNT; i++) {
        if (gSdvAlters_1F1220[i].alterHeader.active == 0) {
            alter = &gSdvAlters_1F1220[i];
            sdvAlterSerial++;
            sdvAlterSerial &= 0xFF;
            alter->request = *alterRequest;
            alter->alterHeader.active = 1;
            alter->alterHeader.serial = sdvAlterSerial;
            alter->alterHeader.frame = 0;
            alter->alterHeader.index = 0;
            alter->progress = 0.0f;
            alter->unusedFlag = unused;
            if (specialTag != 0) {
                alter->specialSlot = sdvFindFreeSpecialWork();
            } else {
                alter->specialSlot = -1;
            }
            alter->ownedSlot = 0;
            if (alter->specialSlot > 0) {
                specialRecord = specialRecordAt(alter->specialSlot);
                specialRecord->inUse = specialTag;
                sdvSetupEffectEntry(specialRecord);
            }
            if (alterRequest->follow != 0) {
                maxSamples = alterRequest->maxSamples;
                radius = sefRandf() * SDV_ALTER_RADIUS_RANDOM;
                if (hasTarget != 0) {
                    func_002050A8(&position, &alterRequest->anchor, alterRequest->parts[0], unused);
                    effect = &sefGetScheduler()[alterRequest->owner];
                    if ((effect->ownerGroupFlags & SEF_RECORD_BUILDS_MATRIX) != 0) {
                        sefBuildEffectMatrix(
                            &effect->unmodeled_20, &effect->unmodeled_10, &effect->unmodeled_30, matrix);
                        MMathApplyMatrix(
                            &position, matrix, &position);
                    }
                } else {
                    sefGetPosition(&position, 0,
                                                                                alterRequest->parts[0]);
                }
                QuadwordCopyT0(&alter->position, &position);
                QuadwordCopyT0(&alter->previousPosition, &position);
                sefNoiseNextVector(&normal);
                normal.x *= SDV_ALTER_NOISE_ANGLE;
                normal.y *= SDV_ALTER_NOISE_ANGLE;
                normal.z *= SDV_ALTER_NOISE_ANGLE;
                MMathRotateMatrixXYZ(&matrix, 0, &normal);
                xglMatrix2Quaternion(&alter->orientation, matrix);
                if (hasTarget != 0) {
                    targetPoints = sevGetPtAllocator2(alterRequest->anchorTarget);
                    SdvSubVectorXYZ(&tangent, &targetPoints[10], &targetPoints[11]);
                    MMathNormalizeVector2(&tangent, &tangent);
                    QuadwordCopyT0(&alter->heading, &tangent);
                    QuadwordCopyT0(&alter->lastDelta, &tangent);
                    for (k = 0; k < SDV_SPECIAL_COUNT; k++) {
                        SdvMulVectorXYZ(&alter->quats[k], &alter->quats[k], &D_0053E950);
                        MMathQuatBetweenVectors(&alter->quats[k], &alter->quats[k], &tangent);
                    }
                } else {
                    QuadwordCopyT0(&alter->heading, &D_0053E920);
                    SdvCopyQuadword(&alter->lastDelta, &D_0053E930);
                    for (k = 0; k < SDV_SPECIAL_COUNT; k++) {
                        SdvMulVectorXYZ(&alter->quats[k], &alter->quats[k], &D_0053E950);
                        MMathQuatBetweenVectors(&alter->quats[k], &alter->quats[k], &D_0053E920);
                    }
                }
                angle = sefRandf() * SDV_PI;
                for (k = 0; k <= maxSamples; k++) {
                    SdvVectorView *samplePoint = &alter->points[0][k];

                    Vu0SetOrigin(samplePoint);
                    Vu0SetOrigin(&alter->points[1][k]);
                    samplePoint->vector.z = (float) -k * alter->request.ringSpacing;
                    alter->angles[k] =
                        (float) (alter->request.spread / maxSamples * k) * SDV_DEG_TO_RAD + angle;
                    alter->radii[k] =
                        radius + (float) (alter->request.timeBase + k * alter->request.timeStep) *
                                     SDV_ALTER_RADIUS_UNIT;
                }
            }
            return (sdvAlterSerial << 16) | i;
        }
    }
    return -1;
}

void sdvDestroyAlter(SdvAlter *alter)
{
    slotReleaseChecked(alter->ownedSlot, 0);
    sdvClearSpecialRecord(alter->specialSlot);
    sefMemZero(alter, sizeof(*alter));
}

/* Destroys the alters of one owner, visited in address order. kept: the byte-offset walk decides the emitted loop. */
void sdvDestroyOwnerAlters(int owner)
{
    SdvAlter *alters;
    SdvAlterHeader *alterHeader;
    SdvAlter *alter;
    int i;
    unsigned int alterByteOffset;

    if (owner < 0) {
        return;
    }
    alterByteOffset = 0;
    alters = gSdvAlters;
    alterHeader = &gSdvAlters[0].alterHeader;
    for (i = 48; i >= 0; i--) {
        if (alterHeader->active != 0) {
            alter = (SdvAlter *)(alterByteOffset + (unsigned int)alters);
            if (alter->owner == owner) {
                sdvDestroyAlter(alter);
            }
        }
        alterHeader = (SdvAlterHeader *)((char *)alterHeader + sizeof(SdvAlter));
        alterByteOffset += sizeof(SdvAlter);
    }
}

typedef union VectorUnion {
    AlignedVector4 vector;
    float f[4];
    long long d[2];
} VectorUnion;

typedef struct SevPtBlock {
    unsigned char unmodeled_00[0x1BC];
    float life;
} SevPtBlock;

typedef struct SdvSchedRecord {
    int state;
    int flags;
    unsigned char unmodeled_08[8];
    AlignedVector4 scale;
    AlignedVector4 rotation;
    AlignedVector4 position;
    unsigned char unmodeled_40[0x74 - 0x40];
    float frameDelta;
    unsigned char unmodeled_78[0x142 - 0x78];
    signed char stopped;
    unsigned char unmodeled_143[0x150 - 0x143];
} SdvSchedRecord;

typedef struct SdvSefTarget {
    unsigned char unmodeled_00[0xB0];
    Matrix4 localMatrix;
} SdvSefTarget;

typedef struct SdvSefRequestWords {
    int words[6];
} SdvSefRequestWords;

/* SdvAlterRequest as the step code reads it: emitterKey and lifeIndex sit where SdvAlterRequest has rgba and anchor. */
typedef struct SdvAlterEmitterRequest {
    unsigned char unmodeled_00[0x10];
    SdvSefRequestWords *sefRequest;
    unsigned char unmodeled_14[4];
    int emitterKey;
    short lifeIndex;
    unsigned char unmodeled_1E[0x24 - 0x1E];
    float length;
    unsigned char unmodeled_28[8];
    float life;
    unsigned char unmodeled_34[8];
    float minDelta;
    short parts[2];
    short maxSamples;
    short shrink;
    unsigned char unmodeled_48[4];
    short mode;
    unsigned char unmodeled_4E[4];
    short owner;
    short follow;
    unsigned char unmodeled_56[2];
    short spin;
    unsigned char unmodeled_5A[6];
} SdvAlterEmitterRequest;

/* The same alter record as the step code (sdvStepAlter) reads it: VectorUnion points and float sample views. */
typedef struct SdvAlterBody {
    AlignedVector4 rotation;
    Matrix4 matrix;
    VectorUnion points[2][64];
    AlignedVector4 quats[33];
    AlignedVector4 heading;
    AlignedVector4 lastDelta;
    AlignedVector4 position;
    AlignedVector4 previousPosition;
    SdvAlterEmitterRequest request;
    float sampleStep;
    short sampleMode;
    unsigned char unmodeled_B06[2];
    float texturePhases[64];
    float progress;
    float angles[32];
    unsigned char unmodeled_C8C[4];
    float radii[32];
    unsigned char unmodeled_D10[4];
    float sampleValues[64];
    short frame;
    unsigned short index;
    unsigned short active;
    unsigned short wrapped;
    short serial;
    signed char specialSlot;
    signed char ownedSlot;
    unsigned char slotFlags[64];
    signed char effectTarget;
    unsigned char unmodeled_E61[0xF];
} SdvAlterBody;

typedef struct KeyframeStepBlock {
    int step;
    float limit;
    float delta;
    float cursor;
} KeyframeStepBlock;

extern KeyframeStepBlock sefKeyframeStepBlock;
void sefMergeMatrixPos(Matrix4 *destination, Matrix4 *source, AlignedVector4 *position);
void func_002018E8(int a, int b, int c, int d, int e, int f);
void vu0VectorSlerp(AlignedVector4 *destination, AlignedVector4 *source, AlignedVector4 *delta, float scale);
void MMathQuatToMatrix(Matrix4 *destination, AlignedVector4 *quaternion);
float MMathCalcLength(AlignedVector4 *vector);
float MMathVectorDotProduct(AlignedVector4 *first, AlignedVector4 *second);

static inline int wrapIndex(int index) {
    if (index < 0) {
        index += 64;
    }
    return index;
}

static inline KeyframeStepBlock *keyframeStepBlock(void) {
    return &sefKeyframeStepBlock;
}

static inline void normalizeVector(VectorUnion *destination) {
    ((AlignedVector4 * (*) (AlignedVector4 * destination, AlignedVector4 * s)) MMathNormalizeVector2)(&destination->vector,
                                                                                            &destination->vector);
}

static inline void vuSubXYZ(void *destination, const void *left, const void *third) {
    __asm__ __volatile__("lqc2 vf1, 0(%5)\n"
                         "lqc2 vf2, 0(%6)\n"
                         "vsub.xyz vf1, vf1, vf2\n"
                         "sqc2 vf1, 0(%4)\n"
                         : "=m"(((float *) (destination))[0]), "=m"(((float *) (destination))[1]), "=m"(((float *) (destination))[2]),
                           "=m"(((float *) (destination))[3])
                         : "r"(destination), "r"(left), "r"(third)
                         : "memory");
}

static inline void vuMulXYZ(void *destination, const void *left, const void *third) {
    __asm__ __volatile__("lqc2 vf1, 0(%5)\n"
                         "lqc2 vf2, 0(%6)\n"
                         "vmul.xyz vf1, vf1, vf2\n"
                         "sqc2 vf1, 0(%4)\n"
                         : "=m"(((float *) (destination))[0]), "=m"(((float *) (destination))[1]), "=m"(((float *) (destination))[2]),
                           "=m"(((float *) (destination))[3])
                         : "r"(destination), "r"(left), "r"(third)
                         : "memory");
}

static inline void vuStoreOrigin(void *destination) {
    __asm__ __volatile__("sqc2 vf0, 0(%4)\n"
                         : "=m"(((float *) (destination))[0]), "=m"(((float *) (destination))[1]), "=m"(((float *) (destination))[2]),
                           "=m"(((float *) (destination))[3])
                         : "r"(destination)
                         : "memory");
}

static inline void copyQuadword(void *destination, const void *left) {
    __asm__ __volatile__("lq $8, 0(%5)\n"
                         "sq $8, 0(%4)\n"
                         : "=m"(((float *) destination)[0]), "=m"(((float *) destination)[1]), "=m"(((float *) destination)[2]),
                           "=m"(((float *) destination)[3])
                         : "r"(destination), "r"(left)
                         : "$8", "memory");
}

static inline int vecNonZero(AlignedVector4 *vector) {
    return (((int *) vector)[2] | ((long long *) vector)[0]) != 0;
}

static inline void setTranslationMatrix(Matrix4 *matrix, AlignedVector4 *point) {
    __asm__ __volatile__("lqc2 $vf1, 0(%1)\n"
                         "vmove.w $vf1, $vf0\n"
                         "lqc2 $vf2, 32(%2)\n"
                         "lqc2 $vf3, 16(%2)\n"
                         "lqc2 $vf4, 0(%2)\n"
                         "sqc2 $vf1, 48(%0)\n"
                         "sqc2 $vf2, 32(%0)\n"
                         "sqc2 $vf3, 16(%0)\n"
                         "sqc2 $vf4, 0(%0)\n"
                         :
                         : "r"(matrix), "r"(point), "r"(&MMathUnitMatrix)
                         : "memory");
}

void sdvStepAlter(SdvAlterBody *alter);

/* Advances one alter through the sample steps of keyframeStepBlock(). It returns at once when its owner record is stopped or
   flagged 0x400000, and destroys the alter when its life is spent and no frames remain. Otherwise it moves the sample
   count, records the part positions along the ring and updates the texture phases and slot flags. */
void sdvStepAlter(SdvAlterBody *alter) {
    AlignedVector4 delta;
    Matrix4 localMatrix;
    Matrix4 inverse;
    Matrix4 effectMatrix;
    VectorUnion sinFirst;
    VectorUnion sinValue;
    VectorUnion axisA;
    VectorUnion axisB;
    SdvAlterEmitterRequest *alterRequest = &alter->request;
    AlignedVector4 *position;
    AlignedVector4 *previousPosition;
    SdvSefRequestWords *sefRequest;
    SdvSefRequestWords *sefBuffer;
    float *pointFloats;

    SdvSchedRecord *emitterRecord;
    SdvSchedRecord *followRecord;
    int hidden = 0;
    int follow = alterRequest->follow;
    int hasEmitter = 0;
    int partCount;
    float length;
    float stepTime;
    float alignment;
    int i;
    int sampleSlot;
    int prevIndex;
    int lastSampleSlot;
    int prevPrevIndex;

    if (alterRequest->emitterKey != 0) {
        if (alterRequest->lifeIndex >= 0) {
            hasEmitter = 1;
        }
    }
    if (alterRequest->owner >= 0) {
        {
            SdvSchedRecord *owner = ((SdvSchedRecord * (*) ()) sefGetScheduler)() + alterRequest->owner;
            if (owner->flags & 0x80) {
                hidden = 1;
            }
            if (owner->flags & 0x400000) {
                return;
            }
            if (owner->stopped > 0) {
                return;
            }
            gSdvFrameDelta[0] = owner->frameDelta;
        }
    }
    if (hasEmitter) {
        if (((SevPtBlock * (*) (int index)) sevGetPtAllocator2)(alterRequest->lifeIndex)->life <= 0.0f) {
            sdvDestroyAlter((SdvAlter *) alter);
            return;
        }
    }
    for (stepTime = 0.0f; stepTime < keyframeStepBlock()->limit; stepTime += 1.0f) {
        alter->specialSlot =
            sdvStepSpecialTrack(alter->specialSlot, alter->ownedSlot);
        if (alterRequest->life != 8192.0f) {
            alterRequest->life -= keyframeStepBlock()->delta;
        }
        if (hidden) {
            if (alter->frame > 0) {
                alter->frame--;
            }
        } else if (alterRequest->life > 0.0f) {
            if (alter->frame < 64 && alter->frame < alter->request.maxSamples) {
                alter->frame++;
            }
        } else {
            alter->frame -= alter->request.shrink;
        }
        if (alter->frame > 0) {
            sefRequest = alterRequest->sefRequest;
            sefBuffer = ((SdvSefRequestWords * (*) (void) ) sefGetBattleData)();
            sefBuffer->words[0] = sefRequest->words[0];
            sefBuffer->words[1] = sefRequest->words[1];
            sefBuffer->words[2] = sefRequest->words[2];
            sefBuffer->words[3] = sefRequest->words[3];
            sefBuffer->words[4] = sefRequest->words[4];
            sefBuffer->words[5] = sefRequest->words[5];
            func_002018E8(sefRequest->words[0], sefRequest->words[1], sefRequest->words[2], sefRequest->words[3],
                          sefRequest->words[4], sefRequest->words[5]);
            if (follow) {
                position = &alter->position;
                previousPosition = &alter->previousPosition;
                copyQuadword(previousPosition, position);
                if (hasEmitter) {
                    func_002050A8(
                        position, &alterRequest->emitterKey, alterRequest->parts[0], alter->effectTarget);
                    followRecord = ((SdvSchedRecord * (*) ()) sefGetScheduler)() + alterRequest->owner;
                    if (followRecord->flags & 0x1000000) {
                        sefBuildEffectMatrix(&followRecord->rotation, &followRecord->scale, &followRecord->position, &effectMatrix);
                        MMathApplyMatrix(position, &effectMatrix, position);
                    }
                } else {
                    sefGetPosition(position, 0, alterRequest->parts[0]);
                }
                vuSubXYZ(&delta, previousPosition, position);
                if (sefRequest->words[0] != 0) {
                    sefMergeMatrixPos(&alter->matrix, &((SdvSefTarget *) sefRequest->words[0])->localMatrix,
                                      position);
                } else {
                    setTranslationMatrix(&alter->matrix, position);
                }
                QuadwordCopyMatrixT0(localMatrix, &alter->matrix);
                vuStoreOrigin(localMatrix[3]);
                MMathInvertAffineMatrix(&inverse, &localMatrix);
                MMathQuatToMatrix(&localMatrix, &alter->rotation);
                MMathMulMatrix(&alter->matrix, &alter->matrix, &localMatrix);
                MMathApplyMatrix(&delta, &inverse, &delta);
                length = MMathCalcLength(&delta);
                ((AlignedVector4 * (*) (AlignedVector4 * d, AlignedVector4 * s))
                     MMathNormalizeVector2)(&delta, &delta);
                if (length < alter->request.minDelta) {
                    QuadwordCopyT0(&delta, &alter->lastDelta);
                } else {
                    QuadwordCopyT0(&alter->lastDelta, &delta);
                }
                vu0VectorSlerp(&alter->heading, &alter->heading, &delta, (float) alterRequest->parts[1] * 0.01f);
                for (i = 32; i > 0; i--) {
                    QuadwordCopyT0(&alter->quats[i], &alter->quats[i - 1]);
                }
                vuMulXYZ(&delta, &alter->heading, &D_0053E950);
                MMathQuatBetweenVectors(&alter->quats[0], &delta,
                                                                             &D_0053E920);
                Vu0Sin(sinFirst.f[0], alter->angles[0]);
                length = sinFirst.f[0] * alter->radii[0];
                pointFloats = &alter->points[1][0].vector.y;
                for (i = 0; i < 32; i++) {
                    alter->angles[i] += (float) -alter->request.spin * 0.017453292f;
                    pointFloats[i * 4 - 1] = 0.0f;
                    pointFloats[i * 4 + 1] = 0.0f;
                    Vu0Sin(sinValue.f[0], alter->angles[i]);
                    pointFloats[i * 4] = sinValue.f[0] * alter->radii[i];
                    Vu0SefAddVectorXYZ(&alter->points[1][i], &alter->points[1][i], &alter->points[0][i]);
                    pointFloats[i * 4] -= length;
                }
                for (i = 0; i < alter->frame; i++) {
                    {
                        int newestIndex = alter->index - 1;
                        sampleSlot = newestIndex - i;
                        if (sampleSlot < 0) {
                            sampleSlot += 64;
                        }
                    }
                    if (alter->sampleMode == 0) {
                        alter->sampleValues[sampleSlot] += alter->sampleStep * keyframeStepBlock()->delta;
                    } else {
                        alter->sampleValues[sampleSlot] = alter->sampleStep * keyframeStepBlock()->delta;
                    }
                    alter->points[0][sampleSlot].f[1] -= alter->sampleValues[sampleSlot];
                }
            } else {
                partCount = 2;
                if (alterRequest->mode == 8) {
                    partCount = 1;
                }
                if (hasEmitter) {
                    emitterRecord = ((SdvSchedRecord * (*) ()) sefGetScheduler)() + alterRequest->owner;
                    if (emitterRecord->flags & 0x1000000) {
                        sefBuildEffectMatrix(&emitterRecord->rotation, &emitterRecord->scale, &emitterRecord->position, &effectMatrix);
                        MMathApplyMatrix(&alter->position, &effectMatrix, &alter->position);
                    }
                    for (i = 0; i < partCount; i++) {
                        ((void (*)(AlignedVector4 *destination, int *alterRequest, int part,
                                   int slot)) func_002050A8)(&alter->points[i][alter->index].vector,
                                                             &alterRequest->emitterKey, alterRequest->parts[i],
                                                             alter->effectTarget);
                        if (emitterRecord->flags & 0x1000000) {
                            MMathApplyMatrix(&alter->points[i][alter->index].vector, &effectMatrix,
                                             &alter->points[i][alter->index].vector);
                        }
                    }
                    alter->sampleValues[alter->index] = 0.0f;
                } else {
                    for (i = 0; i < partCount; i++) {
                        sefGetPosition(&alter->points[i][alter->index].vector, 0, alterRequest->parts[i]);
                    }
                    alter->sampleValues[alter->index] = 0.0f;
                }
                for (i = 0; i < alter->frame; i++) {
                    {
                        int newestIndex = alter->index - 1;
                        sampleSlot = newestIndex - i;
                        if (sampleSlot < 0) {
                            sampleSlot += 64;
                        }
                    }
                    if (alter->sampleMode == 0) {
                        alter->sampleValues[sampleSlot] += alter->sampleStep * keyframeStepBlock()->delta;
                    } else {
                        alter->sampleValues[sampleSlot] = alter->sampleStep * keyframeStepBlock()->delta;
                    }
                    alter->points[0][sampleSlot].f[1] -= alter->sampleValues[sampleSlot];
                    alter->points[1][sampleSlot].f[1] -= alter->sampleValues[sampleSlot];
                }
            }
            if (alterRequest->mode >= 3 && alterRequest->mode <= 7) {
                float progress = alter->progress;

                alter->progress = progress + alter->request.length / (float) alter->request.maxSamples;
                if (alterRequest->mode == 3) {
                    if (alter->wrapped == 0) {
                        alter->texturePhases[alter->index] = progress;
                    } else {
                        alter->texturePhases[alter->index] = 4.0f;
                    }
                }
                if (alterRequest->mode == 4) {
                    alter->texturePhases[alter->index] = progress;
                    if (alter->progress > 1.0f) {
                        alter->progress -= 1.0f;
                        for (i = 0; i < alter->frame; i++) {
                            sampleSlot = wrapIndex(alter->index - i);
                            alter->texturePhases[sampleSlot] -= 1.0f;
                        }
                    }
                }
            }
            if (alterRequest->mode == 2) {
                alter->progress -= alter->request.length / (2.0f * (float) alter->request.maxSamples);
                if (alter->progress < -1.0f) {
                    alter->progress += 1.0f;
                    for (i = 0; i < alter->frame; i++) {
                        sampleSlot = wrapIndex(alter->index - i);
                        alter->texturePhases[sampleSlot] += 1.0f;
                    }
                }
            }
            if (alter->frame >= 2) {
                if (alter->wrapped) {
                    lastSampleSlot = alter->index;
                    prevIndex = lastSampleSlot - 1;
                    if (prevIndex < 0) {
                        prevIndex = 63;
                    }
                    prevPrevIndex = prevIndex - 1;
                    if (prevPrevIndex < 0) {
                        prevPrevIndex = 63;
                    }
                } else {
                    lastSampleSlot = alter->index;
                    prevIndex = lastSampleSlot - 1;
                    if (prevIndex < 0) {
                        prevIndex = 0;
                    }
                    prevPrevIndex = prevIndex - 1;
                    if (prevPrevIndex < 0) {
                        prevPrevIndex = 0;
                    }
                }
                vuSubXYZ(&axisA, alter->points[0] + prevIndex, alter->points[0] + prevPrevIndex);
                normalizeVector(&axisA);
                vuSubXYZ(&axisB, alter->points[0] + lastSampleSlot, alter->points[0] + prevIndex);
                normalizeVector(&axisB);
                {
                    double dot = MMathVectorDotProduct(&axisA.vector, &axisB.vector);
                    if (dot < 0) {
                        dot = -dot;
                    }
                    alignment = dot;
                }
                if (alignment >= 1.0f) {
                    alter->slotFlags[prevPrevIndex] = alter->slotFlags[prevIndex] = 1;
                } else if (alignment > 0.95f) {
                    alter->slotFlags[prevPrevIndex] = alter->slotFlags[prevIndex] = 3;
                } else if (alignment > 0.85f) {
                    alter->slotFlags[prevPrevIndex] = alter->slotFlags[prevIndex] = 6;
                } else if (alignment > 0.3f) {
                    alter->slotFlags[prevPrevIndex] = alter->slotFlags[prevIndex] = 8;
                } else if (alignment != 0.0f || vecNonZero(&axisA.vector)) {
                    alter->slotFlags[prevPrevIndex] = alter->slotFlags[prevIndex] = 8;
                } else {
                    alter->slotFlags[prevPrevIndex] = alter->slotFlags[prevIndex] = 1;
                }
            }
            if (alter->sampleStep == 0.0f || !(alterRequest->life <= 0.0f)) {
                alter->index++;
            }
            if (alter->index >= 64) {
                alter->wrapped = 1;
                alter->index &= 63;
            }
        } else if (alterRequest->life < 0.0f) {
            sdvDestroyAlter((SdvAlter *) alter);
        }
    }
}

/* VIF codes appended to the packet: FLUSH and MSCNT. */
#define SDV_VIF_CODE_FLUSH 0x11000000
#define SDV_VIF_CODE_MSCNT 0x17000000

/* Resets the VIF packet counter and appends the FLUSH and MSCNT codes to the current packet. */
void sdvPacketFlushMscnt(void)
{
    XglPacket *packet;

    packet = xglPacketGetCurrent();
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, SDV_VIF_CODE_FLUSH);
    sceVif1PkAddCode(packet, SDV_VIF_CODE_MSCNT);
}

/* Clears and fills a strip stripHeader, then references its quadwords in the current packet by physical address.
   kept: the mask is applied before the packet calls, which decides the emitted order. */
void sdvSetStripHeader(SdvStripHeader *stripHeader, int unused, int qwc, int primitiveCount, int strip)
{
    XglPacket *packet;
    void *physical;

    physical = (void *)((unsigned int)stripHeader & SDV_PHYSICAL_ADDRESS_MASK);
    packet = xglPacketGetCurrent();
    sdvPacketFlushMscnt();
    stripHeader->quad[0] = 0;
    stripHeader->quad[1] = 0;
    stripHeader->layout.unpack = ((qwc - 1) << 16) | 0x6C008000;
    stripHeader->layout.primitiveCount = primitiveCount;
    stripHeader->layout.strip = strip;
    stripHeader->layout.loopCount |= primitiveCount;
    sceVif1PkRef(packet, physical, qwc, 0, 0, 0);
}

/* The draw view of an alter request: the fields sdvDrawAlters and sdvAlterDraw read. */
typedef struct SdvAlterDrawRequest {
    unsigned int tex0[2][2];       /* +0x00 GS TEX0 of the two images, low word first */
    unsigned char unmodeled_10[4]; /* +0x10 */
    unsigned char rgba[4];         /* +0x14 */
    unsigned char unmodeled_18[0x24 - 0x18];
    float length;          /* +0x24 */
    float imageTime;       /* +0x28 frames the current image has been shown */
    float imageFrames;     /* +0x2C frames each image is shown */
    float life;            /* +0x30 */
    float ringSpacing;     /* +0x34 */
    float height;          /* +0x38 */
    float minDelta;        /* +0x3C */
    short parts[2];        /* +0x40 */
    short maxSamples;      /* +0x44 */
    unsigned short shrink; /* +0x46 */
    short blend;           /* +0x48 */
    short imageCount;      /* +0x4A */
    short alterMode;            /* +0x4C */
    short image;           /* +0x4E the image drawn, 0..imageCount-1 */
    short layer;           /* +0x50 */
    short owner;           /* +0x52 index of the owning effect record */
    short follow;          /* +0x54 nonzero: the record trails the part of parts[0] */
    short spread;          /* +0x56 */
    short spin;            /* +0x58 */
    short timeBase;        /* +0x5A */
    short timeStep;        /* +0x5C */
    unsigned char unmodeled_5E[2];
    float lifeFade; /* +0x60 nonzero: the strip fades by the alterRequest life */
    unsigned char unmodeled_64[4];
} SdvAlterDrawRequest;

/* The draw view of the alter header (the strip counters sdvDrawAlters reads). */
typedef struct SdvAlterDrawHeader {
    unsigned char unmodeled_00[4];
    short frame;           /* +0x04 (alter +0xE14) segments drawn by sdvDrawAlter */
    unsigned short ringCursor;  /* +0x06 (alter +0xE16) the point written this frame */
    unsigned short active; /* +0x08 */
    unsigned char unmodeled_0A[0xE1E - 0xE1A];
} SdvAlterDrawHeader;

/* The draw view of one alter record (0xE70 bytes), shared by the three draw functions. */
typedef struct SdvAlterDraw {
    unsigned char unmodeled_000[0x10];
    Matrix4 matrix;           /* +0x010 */
    Vector4 points[2][64];    /* +0x050 two 64-point rings */
    AlignedVector4 quats[33]; /* +0x850 heading quaternions, newest first */
    AlignedVector4 unmodeled_A60[4];
    SdvAlterDrawRequest request; /* +0xAA0 */
    float texturePhases[64];        /* +0xB08 */
    float progress;                 /* +0xC08 */
    unsigned char unmodeled_C0C[0xE10 - 0xC0C];
    SdvAlterDrawHeader alterHeader;
    signed char specialSlot; /* +0xE1E: the special record sdvClearSpecialRecord clears */
    signed char ownedSlot;   /* +0xE1F: first argument of slotReleaseChecked in sdvDestroyAlter */
    unsigned char segmentPackets[0xE70 - 0xE20]; /* +0xE20 packets per segment */
} SdvAlterDraw;

/* GIF tag and the GS state registers that sdvDrawAlter loads for each strip. */
typedef struct SdvAlterEnvironment {
    u64 giftag[2];
    DrawStateRegister tex0;
    DrawStateRegister test;
    DrawStateRegister tex1;
    DrawStateRegister clamp;
    DrawStateRegister zbuf;
    DrawStateRegister alpha;
} SdvAlterEnvironment;

/* One 16-byte vertex or tag word, viewed as the form the packet needs. */
typedef union SdvAlterQuad {
    XglQuadword quad;
    float f[4];
    Vector4 vector;
    unsigned int rgba[4];
    u64 words[2];
} SdvAlterQuad;

#define SDV_GIF_TAG(prim, registerCount) \
    (((u64)(registerCount) << 60) | ((u64)(prim) << 47) | ((u64)1 << 46) | GIF_TAG_EOP)
#define SDV_PRIM_LINE_STRIP 0x4A
#define SDV_PRIM_TRIANGLE_STRIP 0x4C
#define SDV_PRIM_TEXTURED 0x10
#define SDV_ALTER_MODE_REPEAT 3

#define SDV_ALTER_FOLLOW_LINE 2
#define SDV_UNCACHED 0x20000000

extern SdvAlterEnvironment sdvAlterEnvironment;
extern unsigned char D_00001B70[];
extern unsigned char D_00001BD0[];
extern u64 sdvAlphaBlendTable[4];
extern unsigned short D_004C8390[];
void *xglPacketReserve(int drawnSegments);
void xglPacketBufferInit(unsigned int address);
void xglPacketAppendEntry(u64 a, u64 b, int c, int d);
void xglPacketAppendQuad(unsigned int reg, u64 value);
void xglPacketFinishTag(void);
unsigned int xglPacketGetEntryCount(void);
void sceVif1PkOpenUpkCode(XglPacket *packet, unsigned int a, unsigned int b, unsigned int c, unsigned int d);
void sceVif1PkAddUpkData128N(XglPacket *packet, void *data, unsigned int drawnSegments);
void sceVif1PkCloseUpkCode(XglPacket *packet);

/* The part of an effect record (0x150 bytes) that sdvDrawAlter reads: its color scale. */
typedef struct SdvEffectRecord {
    unsigned char unmodeled_00[0x40];
    float colorScale[4]; /* +0x40 */
    unsigned char unmodeled_50[0x150 - 0x50];
} SdvEffectRecord;

void sefPacketFlush(void);
void sdvPacketFlushMscnt(void);
Vector4 *MMathClampVectorXYZ(Vector4 *outVector, const Vector4 *inputVector, float minimum, float maximum);
void MMathCalcHermitePrm(Vector4 *tangent_start, Vector4 *tangent_end, const Vector4 *point_prev,
                         const Vector4 *point_cur, const Vector4 *point_next, const Vector4 *point_last);
float *MMathCalcHermite(float *outPoint, Vector4 *tangent_first, Vector4 *tangent_second, const Vector4 *point_a,
                        const Vector4 *point_b, float parameter);

static inline void sdvCopyVector(float *outVector, float *inVector) {
    __asm__ __volatile__("lq $8, 0(%5)\n"
                         "sq $8, 0(%4)\n"
                         : "=m"(outVector[0]), "=m"(outVector[1]), "=m"(outVector[2]),
                           "=m"(outVector[3])
                         : "r"(outVector), "r"(inVector)
                         : "$8");
}

static inline int sdvAlterIsFlat(const SdvAlterDrawRequest *alterRequest) {
    return alterRequest->follow == SDV_ALTER_FOLLOW_LINE;
}

static inline int sdvFollowIsFlat(int follow) {
    return follow == SDV_ALTER_FOLLOW_LINE;
}

static inline SdvSlotEvent *sdvSpecialEventAt(int specialIndex) {
    SdvSlotEvent *event = 0;

    if ((unsigned int) specialIndex < 32) {
        event = (SdvSlotEvent *) &gSdvSpecialRecords[specialIndex];
    }
    return event;
}

/* Draws one alter record: its ring of points (or the heading quaternions when it follows a part) becomes a
   strip of textured or line vertices, appended to the current packet with its strip stripHeader. */
void sdvAlterDraw(SdvAlterDraw *alter) {
    Vector4 tangents[2];
    float edges[2][9][4];
    AlignedVector4 positions[32];
    SdvAlterQuad color;
    SdvAlterQuad giftag;
    SdvAlterQuad texcoords[2];
    Matrix4 heading;
    int previousPoint;
    int lastPoint;
    XglPacket *packet;
    SdvAlterQuad *vertex;
    Vector4 *turned;
    Vector4 *points;
    void *buffer;
    float progress;
    float lengthStep;
    float alpha;
    float alphaStep;
    float parameterStep;
    float parameter;
    int ringIndex;
    int packets;
    int sides;
    int side;
    int step;
    int nextPoint;
    int i;
    int reserveSize;
    int vertexCount;
    int newestIndex;
    int segmentCount;
    SdvAlterDrawRequest *alterRequest;
    SdvStripHeader *stripHeader;
    int reservedTotal;
    int follow;
    int drawAsLines;
    SdvEffectRecord *record;
    int hasSpecial;
    float alphaValue;

    alterRequest = &alter->request;
    reserveSize = 0;
    newestIndex = alter->alterHeader.ringCursor - 1;
    segmentCount = alter->alterHeader.frame;
    reservedTotal = 0;
    progress = alter->progress;
    packet = xglPacketGetCurrent();
    follow = alterRequest->follow;
    drawAsLines = 0;
    if (sdvFollowIsFlat(follow) || alterRequest->alterMode == SDV_ALTER_MODE_SINGLE) {
        drawAsLines = 1;
    }
    record = (SdvEffectRecord *) sefGetScheduler() + alter->request.owner;
    hasSpecial = alter->specialSlot > 0;
    texcoords[0].f[0] = 0.0f;
    texcoords[0].f[1] = 0.0f;
    texcoords[0].f[2] = 1.0f;
    texcoords[0].f[3] = 0.0f;
    texcoords[1].quad = texcoords[0].quad;
    texcoords[1].f[1] = 1.0f;
    if (segmentCount < 2) {
        return;
    }
    for (i = 0; i < segmentCount; i++) {
        reserveSize += alter->segmentPackets[i] * 6 + 9;
    }
    buffer = xglPacketReserve(reserveSize);
    stripHeader = (SdvStripHeader *) ((unsigned int) buffer + SDV_UNCACHED);
    if (buffer == 0) {
        return;
    }
    color.rgba[0] = alter->request.rgba[0];
    color.rgba[1] = alter->request.rgba[1];
    color.rgba[2] = alter->request.rgba[2];
    color.rgba[0] = (unsigned int) ((float) color.rgba[0] * record->colorScale[0]);
    color.rgba[1] = (unsigned int) ((float) color.rgba[1] * record->colorScale[1]);
    color.rgba[2] = (unsigned int) ((float) color.rgba[2] * record->colorScale[2]);
    MMathClampVectorXYZ(&color.vector, &color.vector, 0.0f, 255.0f);
    if (alterRequest->lifeFade != 0.0f && alterRequest->life < 0.0f) {
        return;
    }
    alpha = 0.0f;
    lengthStep = alter->request.length / (float) alter->request.maxSamples;
    progress += lengthStep * (float) (segmentCount - 1);
    if (alterRequest->imageCount != 0) {
        alterRequest->imageTime += 1.0f;
        if (alterRequest->imageTime >= alterRequest->imageFrames) {
            alterRequest->imageTime = 0.0f;
            alterRequest->image++;
            if (alterRequest->image >= alterRequest->imageCount) {
                alterRequest->image = 0;
            }
        }
        giftag.words[1] = GS_ST | (GS_RGBAQ << 4) | (GS_XYZ2 << 8);
        giftag.words[0] = SDV_GIF_TAG(SDV_PRIM_TRIANGLE_STRIP | SDV_PRIM_TEXTURED, 3);
    } else {
        giftag.words[1] = GS_ST | (GS_RGBAQ << 4) | (GS_XYZ2 << 8);
        giftag.words[0] = SDV_GIF_TAG(SDV_PRIM_TRIANGLE_STRIP, 3);
    }
    if (drawAsLines) {
        giftag.words[1] = GS_RGBAQ | (GS_XYZ2 << 4);
        giftag.words[0] = SDV_GIF_TAG(SDV_PRIM_LINE_STRIP, 2);
    }
    sdvAlterEnvironment.tex0.data =
        alterRequest->imageCount != 0 ? ((unsigned long long *) alter->request.tex0)[alterRequest->image] : 0;
    sdvAlterEnvironment.clamp.data = alterRequest->alterMode == SDV_ALTER_MODE_REPEAT ? 5 : 4;
    sdvAlterEnvironment.alpha.data = sdvAlphaBlendTable[alterRequest->blend];
    sceVif1PkCnt(packet, 0);
    if (hasSpecial) {
        xglPacketBufferInit(0x70000000);
        xglPacketAppendEntry(0x8000, 0x10000000, 14, 0);
        xglPacketAppendQuad(74, 0);
        xglPacketAppendQuad(71, 0x30000);
        xglPacketAppendQuad(66, 168);
        xglPacketAppendQuad(76, D_004C8390[11] | 0xFFFFFF00080000ULL);
        xglPacketAppendQuad(0, 326);
        xglPacketAppendQuad(1, 0xFE00ULL << 46);
        xglPacketAppendQuad(4, 0x71F76FF8ULL);
        xglPacketAppendQuad(4, 0x8DF78FF8ULL);
        xglPacketAppendQuad(76, D_004C8390[11] | 0x80000ULL);
        xglPacketFinishTag();
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenUpkCode(packet, 0, 0x6C, 1, 1);
        sceVif1PkAddUpkData128N(packet, (void *) 0x70000000, xglPacketGetEntryCount());
        sceVif1PkCloseUpkCode(packet);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddCode(packet, SDV_VIF_CODE_FLUSH);
        sceVif1PkAddCode(packet, ((unsigned int) D_00001BD0 >> 3) | 0x14000000);
        sefPacketFlush();
    }
    vertexCount = 0;
    sceVif1PkOpenUpkCode(packet, 0, 0x6C, 4, 4);
    sceVif1PkAddUpkData128N(packet, &sdvAlterEnvironment, 7);
    sceVif1PkCloseUpkCode(packet);
    vertex = (SdvAlterQuad *) (stripHeader + 1);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, SDV_VIF_CODE_FLUSH);
    sceVif1PkAddCode(packet, ((unsigned int) D_00001B70 >> 3) | 0x14000000);
    if (follow != 0) {
        for (i = 0; i < segmentCount; i++) {
            MMathQuatToMatrix(heading, &alter->quats[i]);
            ((Vector4 * (*) (Vector4 * outVector, Matrix4 * matrix, const Vector4 *vector))
                 MMathApplyMatrix)(&positions[i], &heading, &alter->points[1][i]);
        }
    }
    for (i = segmentCount - 2; i >= 0; i--) {
        int currentPoint;
        int isFlat;

        packets = alter->segmentPackets[segmentCount - 2 - i];
        parameterStep = 1.0f / (float) packets;
        alphaValue = (float) alter->request.rgba[3] * record->colorScale[3];
        if (alterRequest->lifeFade != 0.0f) {
            int halfSamples = alterRequest->maxSamples / 2;
            if (alterRequest->life < (float) halfSamples) {
                alphaValue = alphaValue * alterRequest->life / (float) halfSamples;
            }
        }
        alphaStep = alphaValue / (float) (segmentCount - 1) / (float) packets;
        lengthStep = alter->request.length / (float) alter->request.maxSamples;
        lengthStep /= (float) packets;
        if (follow != 0) {
            if (i == 0) {
                previousPoint = 0;
                currentPoint = 0;
            } else {
                previousPoint = i - 1;
                currentPoint = i;
            }
            if (i >= segmentCount - 2) {
                nextPoint = i + 1;
                lastPoint = nextPoint;
            } else {
                nextPoint = i + 1;
                lastPoint = i + 2;
            }
        } else {
            ringIndex = (newestIndex - i) & 0x3F;
            if (i == 0) {
                previousPoint = ringIndex;
                currentPoint = ringIndex;
            } else {
                previousPoint = (ringIndex + 1) & 0x3F;
                currentPoint = ringIndex;
            }
            if (i >= segmentCount - 2) {
                nextPoint = (ringIndex - 1) & 0x3F;
                lastPoint = nextPoint;
            } else {
                nextPoint = (ringIndex - 1) & 0x3F;
                lastPoint = (ringIndex - 2) & 0x3F;
            }
        }
        if (follow != 0) {
            isFlat = sdvAlterIsFlat(alterRequest);
            turned = positions;
            MMathCalcHermitePrm(&tangents[0], &tangents[1], &turned[previousPoint], &turned[currentPoint],
                                &turned[nextPoint], &turned[lastPoint]);
            parameter = 0.0f;
            for (step = 0; step <= packets; step++) {
                MMathCalcHermite(edges[0][step], &tangents[0], &tangents[1], &turned[currentPoint], &turned[nextPoint],
                                 parameter);
                sdvCopyVector(edges[1][step], edges[0][step]);
                if (!isFlat) {
                    edges[0][step][1] += alter->request.height;
                    edges[1][step][1] -= alter->request.height;
                }
                parameter += parameterStep;
            }
        } else {
            sides = alterRequest->alterMode == SDV_ALTER_MODE_SINGLE ? 1 : 2;
            for (side = 0; side < sides; side++) {
                points = alter->points[side];
                MMathCalcHermitePrm(&tangents[0], &tangents[1], &points[previousPoint], &points[currentPoint],
                                    &points[nextPoint], &points[lastPoint]);
                parameter = 0.0f;
                for (step = 0; step <= packets; step++) {
                    MMathCalcHermite(edges[side][step], &tangents[0], &tangents[1], &points[currentPoint],
                                     &points[nextPoint], parameter);
                    parameter += parameterStep;
                }
            }
        }
        if (alterRequest->alterMode >= 3 && alterRequest->alterMode <= 7) {
            progress = alter->texturePhases[currentPoint];
            lengthStep = (alter->texturePhases[nextPoint] - progress) / (float) packets;
        }
        for (step = packets; step >= 0; step--) {
            color.rgba[3] = (unsigned int) alpha;
            if (drawAsLines) {
                vertex[0].quad = color.quad;
                if (sdvAlterIsFlat(alterRequest)) {
                    ((Vector4 * (*) (Vector4 * outVector, Matrix4 * matrix, const Vector4 *vector))
                         MMathApplyMatrix)(&vertex[1].vector, &alter->matrix, (Vector4 *) edges[0][step]);
                } else {
                    vertex[1].quad = *(XglQuadword *) edges[0][step];
                }
                vertex += 2;
                vertexCount += 2;
            } else {
                for (side = 0; side < 2; side++) {
                    vertex[0].quad = side != 0 ? texcoords[1].quad : texcoords[0].quad;
                    vertex[0].f[0] = progress;
                    vertex[1].quad = color.quad;
                    if (follow != 0) {
                        ((Vector4 * (*) (Vector4 * outVector, Matrix4 * matrix, const Vector4 *vector))
                             MMathApplyMatrix)(&vertex[2].vector, &alter->matrix,
                                               (Vector4 *) edges[side][step]);
                    } else {
                        vertex[2].quad = *(XglQuadword *) edges[side][step];
                    }
                    vertex += 3;
                    vertexCount += 3;
                }
            }
            if (step > 0) {
                progress -= lengthStep;
                alpha += alphaStep;
            }
        }
        reservedTotal += reserveSize;
        stripHeader->quad[2] = giftag.quad;
        if (drawAsLines) {
            sdvSetStripHeader(stripHeader, 0, vertexCount + 3, vertexCount / 2, 1);
        } else {
            sdvSetStripHeader(stripHeader, 0, vertexCount + 3, vertexCount / 3, 0);
        }
        stripHeader = (SdvStripHeader *) vertex;
        vertex += 3;
        vertexCount = 0;
    }
    if (reservedTotal > 0) {
        sdvPacketFlushMscnt();
        sefPacketFlush();
        if (hasSpecial) {
            sdvApplySlotEvent(&alter->ownedSlot, sdvSpecialEventAt(alter->specialSlot), 1);
        }
    }
}

void sdvInitAlters(void)
{
    SdvAlter *alter;
    int i;

    alter = gSdvAlters;
    for (i = 0; i < SDV_ALTER_COUNT; i++) {
        sdvInitAlter(alter);
        alter++;
    }
}

void sdvDestroyAlters(void)
{
    SdvAlter *alter;
    int i;

    alter = gSdvAlters;
    for (i = 0; i < SDV_ALTER_COUNT; i++) {
        sdvDestroyAlter(alter);
        alter++;
    }
}

void sdvExecAlters(void)
{
    int i;

    for (i = 0; i < SDV_ALTER_COUNT; i++) {
        if (gSdvAlters[i].alterHeader.active != 0) {
            sdvStepAlter(&gSdvAlters[i]);
        }
    }
}

/* The work block of an owner whose alters sdvExecOwnerAlters runs; only its owner id is named. */
typedef struct SdvOwnerWork {
    unsigned char unmodeled_000[0x13E];
    short ownerId;             /* +0x13E compared with SdvAlter.owner */
} SdvOwnerWork;

void sdvExecOwnerAlters(SdvOwnerWork *ownerWork) {
    SdvAlter *alter;
    s32 i;

    alter = gSdvAlters;
    for (i = 0; i < SDV_ALTER_COUNT; i++) {
        if ((alter->alterHeader.active != 0) && (alter->owner == ownerWork->ownerId)) {
            sdvStepAlter(alter);
        }
        alter++;
    }
}

extern SdvAlterDraw gSdvAlters_sdvDrawAlters[];
extern s32 svEffectSceneFlag;
void svLoadDrawViewMatrix(void);
void svBuildDrawStatePacket(void);

/* Draws the alters of one layer whose owning effect record has its flag at +0x0A set. The scene state is
   built once, before the first alter drawn. */
void sdvDrawAlters(s32 layer) {
    SefEffectRecord *record;
    s32 i;
    s32 stateBuilt = 0;

    if (svEffectSceneFlag == 0 && (((s32) (gSbStudioBlock[0] & 1)) == 0 || (gSbStudioBlock[4] >> 32) != 0)) {
        for (i = 0; i < SDV_ALTER_COUNT; i++) {
            if (gSdvAlters_sdvDrawAlters[i].alterHeader.active != 0 &&
                gSdvAlters_sdvDrawAlters[i].request.layer == layer) {
                record = sefGetScheduler() + gSdvAlters_sdvDrawAlters[i].request.owner;
                if (record != 0 && *(short *) &record->unmodeled_08[2] != 0) {
                    if (!stateBuilt) {
                        stateBuilt = 1;
                        svLoadDrawViewMatrix();
                        svBuildDrawStatePacket();
                    }
                    sdvAlterDraw(&gSdvAlters_sdvDrawAlters[i]);
                }
            }
        }
    }
}

extern SdvAlterDraw gSdvAlters_1F35E0[];

/* Draws the alters that belong to owner, under the same scene gate as sdvDrawAlters. */
void sdvAlterDrawOwned(s32 owner) {
    s32 i;
    s32 stateBuilt = 0;

    if (svEffectSceneFlag == 0 && (((s32) (gSbStudioBlock[0] & 1)) == 0 || (gSbStudioBlock[4] >> 32) != 0)) {
        for (i = 0; i < SDV_ALTER_COUNT; i++) {
            if (gSdvAlters_1F35E0[i].alterHeader.active != 0 && gSdvAlters_1F35E0[i].request.owner == owner) {
                if (!stateBuilt) {
                    stateBuilt = 1;
                    svLoadDrawViewMatrix();
                    svBuildDrawStatePacket();
                }
                sdvAlterDraw(&gSdvAlters_1F35E0[i]);
            }
        }
    }
}
