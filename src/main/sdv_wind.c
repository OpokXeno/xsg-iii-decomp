#include "common.h"
#include "shared.h"

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

#include "main/tu122.h"

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

/* A vector stored component by component through a one-member union (X2's SdvVectorView). */
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
    frameKey->member = (nextRow->member == row->member ? row->member \
        : (row->member + (((nextRow->member - row->member) * t + 0x80) >> 8)) & 0xFFFF)

/* Steps a wind key sequence and applies the key it reaches (interpolated towards the next key)
 * to the wind slot of key and subkey, acquiring one when needed: the point or directional wind
 * of the table's kind is placed, given its speed, range and radius and enabled; a radius of 0
 * or TIWIND_RADIUS_OFF and beyond disables it and releases the slot. */
void tiWindUpdateKeyframes(SdvSequence *sequence, int key, unsigned short subkey)
{
    TiWindKey blendedKey;
    SdvVectorView windVector;
    TiWindRowView keyRowView;
    AlignedVector4 position;
    TiWindKey *frameKey;
    TiWindKey *row;
    TiWindKey *nextRow;
    int keyRowIndex;
    int found;
    int windKind;
    int speed;
    int elapsed;
    int frame;
    int nextFrame;
    int t;
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
    frameKey = &blendedKey;
    keyRowView.halves = sequence->keys + keyRowIndex * sequence->stride;
    row = keyRowView.rows;
    nextRow = row + 1;
    elapsed = sequence->elapsed;
    speed = sequence->keys[-2];
    windKind = sequence->keys[-1];
    frame = row->frame;
    if (frame != 4096 && elapsed >= frame) {
        nextFrame = nextRow->frame;
        if (nextFrame >= 4096) {
            frameKey = row;
            found = 1;
        } else if (frame != nextFrame) {
            t = ((elapsed - frame) << 8) / (nextFrame - frame);
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
            frameKey->positionKind = row->positionKind;
        }
    }
    if (found == 0) {
        return;
    }
    windSlotIndex = tiWindFindSlot(key, subkey);
    if (windSlotIndex < 0) {
        windSlotIndex = tiWindAcquireSlot(key, subkey, windKind);
        if (windSlotIndex < 0) {
            return;
        }
    }
    windVector.vector.x = frameKey->positionOffset[0] * 0.01f;
    windVector.vector.y = frameKey->positionOffset[1] * 0.01f;
    windVector.vector.z = frameKey->positionOffset[2] * 0.01f;
    windVector.vector.w = 1.0f;
    sefGetPosition(&position, &windVector.vector, frameKey->positionKind);
    if (windKind != 0) {
        tiWindDirSetEnabled(windSlotIndex, 0);
        tiWindPointSetPosition(windSlotIndex, &position.x);
        tiWindPointSetSpeed(windSlotIndex, speed * 0.01f);
        tiWindPointSetRange(windSlotIndex, frameKey->rangeUpper * 0.01f, frameKey->rangeLower * 0.01f);
        enabled = 0;
        radius = frameKey->radius;
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
            tiWindReleaseSlots(key, subkey);
        }
    } else {
        tiWindPointSetEnabled(windSlotIndex, 0);
        tiWindDirSetPosition(windSlotIndex, &position.x);
        windVector.vector.x = frameKey->direction[0] * 0.01f;
        windVector.vector.y = frameKey->direction[1] * 0.01f;
        windVector.vector.z = frameKey->direction[2] * 0.01f;
        windVector.vector.w = 1.0f;
        MMathNormalizeVector2(&windVector.vector, &windVector.vector);
        tiWindDirSetDirection(windSlotIndex, windVector.vector.x, windVector.vector.y, windVector.vector.z);
        tiWindDirSetSpeed(windSlotIndex, speed * 0.01f);
        tiWindDirSetRange(windSlotIndex, frameKey->rangeUpper * 0.01f, frameKey->rangeLower * 0.01f);
        enabled = 0;
        radius = frameKey->radius;
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
            tiWindReleaseSlots(key, subkey);
        }
    }
}
