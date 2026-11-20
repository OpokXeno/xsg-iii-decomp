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
    TiWindSlot *slot;
    int i;

    slot = tiWindSlots;
    for (i = 0; i < 4; i++) {
        if (slot->key == 0) {
            slot->key = key;
            slot->subkey = subkey;
            slot->windKind = windKind;
            return i;
        }
        slot++;
    }
    return -1;
}

/* Releases every wind slot owned by key and subkey (key 0 matches all slots):
 * each one is disabled and cleared. */
void tiWindReleaseSlots(int key, int subkey)
{
    TiWindSlot *slot;
    int i;

    slot = tiWindSlots;
    for (i = 0; i < 4; i++) {
        if (key == 0 || (slot->key == key && slot->subkey == (subkey & 0xFFFF))) {
            tiWindDirSetEnabled(i, 0);
            tiWindPointSetEnabled(i, 0);
            slot->key = 0;
            slot->subkey = 0;
            slot->windKind = 0;
        }
        slot++;
    }
}

/* Index of the wind slot whose key and subkey match, or -1. */
int tiWindFindSlot(int key, unsigned short subkey)
{
    TiWindSlot *slot;
    int i;

    slot = tiWindSlots;
    for (i = 0; i < 4; i++) {
        if (slot->key == key && slot->subkey == subkey) {
            return i;
        }
        slot++;
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
    short offset[3];        /* +0x04 */
    short rangeUpper;       /* +0x0A phase bounds */
    short rangeLower;       /* +0x0C */
    short radius;           /* +0x0E negative: global wind; 0 or TIWIND_RADIUS_OFF and beyond: wind off */
    short direction[3];     /* +0x10 directional wind only */
} TiWindKey;

#define TIWIND_RADIUS_OFF 20000

/* A vector stored component by component through a one-member union (X2's SdvVectorView). */
typedef union SdvVectorView {
    Vector4 vector;
} SdvVectorView;

int func_001ED8A0(SdvSequence *sequence);
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
    frame_key->member = (next->member == row->member ? row->member \
        : (row->member + (((next->member - row->member) * t + 0x80) >> 8)) & 0xFFFF)

/* Steps a wind key sequence and applies the key it reaches (interpolated towards the next key)
 * to the wind slot of key and subkey, acquiring one when needed: the point or directional wind
 * of the table's kind is placed, given its speed, range and radius and enabled; a radius of 0
 * or TIWIND_RADIUS_OFF and beyond disables it and releases the slot. */
void tiWindUpdateKeyframes(SdvSequence *sequence, int key, unsigned short subkey)
{
    TiWindKey current;
    SdvVectorView offset;
    AlignedVector4 position;
    TiWindKey *frame_key;
    TiWindKey *row;
    TiWindKey *next;
    int index;
    int found;
    int kind;
    int speed;
    int elapsed;
    int frame;
    int nextFrame;
    int t;
    int slot;
    int enabled;
    int radius;

    if (sequence == 0 || sequence->keys == 0) {
        return;
    }
    index = func_001ED8A0(sequence);
    if (index < 0) {
        index = sequence->position - 1;
    }
    if (index < 0) {
        index = 0;
    }
    if (index == 4096) {
        return;
    }
    found = 0;
    frame_key = &current;
    row = (TiWindKey *)(sequence->keys + index * sequence->stride);
    next = row + 1;
    elapsed = sequence->elapsed;
    speed = sequence->keys[-2];
    kind = sequence->keys[-1];
    frame = row->frame;
    if (frame != 4096 && elapsed >= frame) {
        nextFrame = next->frame;
        if (nextFrame >= 4096) {
            frame_key = row;
            found = 1;
        } else if (frame != nextFrame) {
            t = ((elapsed - frame) << 8) / (nextFrame - frame);
            TIWIND_BLEND(offset[0]);
            TIWIND_BLEND(offset[1]);
            TIWIND_BLEND(offset[2]);
            TIWIND_BLEND(radius);
            TIWIND_BLEND(rangeUpper);
            TIWIND_BLEND(rangeLower);
            TIWIND_BLEND(direction[0]);
            TIWIND_BLEND(direction[1]);
            TIWIND_BLEND(direction[2]);
            found = 1;
            frame_key->positionKind = row->positionKind;
        }
    }
    if (found == 0) {
        return;
    }
    slot = tiWindFindSlot(key, subkey);
    if (slot < 0) {
        slot = tiWindAcquireSlot(key, subkey, kind);
        if (slot < 0) {
            return;
        }
    }
    offset.vector.x = frame_key->offset[0] * 0.01f;
    offset.vector.y = frame_key->offset[1] * 0.01f;
    offset.vector.z = frame_key->offset[2] * 0.01f;
    offset.vector.w = 1.0f;
    sefGetPosition(&position, &offset.vector, frame_key->positionKind);
    if (kind != 0) {
        tiWindDirSetEnabled(slot, 0);
        tiWindPointSetPosition(slot, &position.x);
        tiWindPointSetSpeed(slot, speed * 0.01f);
        tiWindPointSetRange(slot, frame_key->rangeUpper * 0.01f, frame_key->rangeLower * 0.01f);
        enabled = 0;
        radius = frame_key->radius;
        if (radius != 0 && radius < TIWIND_RADIUS_OFF) {
            enabled = 1;
        }
        if (enabled) {
            if (radius < 0) {
                tiWindPointSetRadius(slot, 0.0f, 1);
            } else {
                tiWindPointSetRadius(slot, radius * 0.01f, 0);
            }
            tiWindPointSetEnabled(slot, 1);
        } else {
            tiWindPointSetEnabled(slot, 0);
            tiWindReleaseSlots(key, subkey);
        }
    } else {
        tiWindPointSetEnabled(slot, 0);
        tiWindDirSetPosition(slot, &position.x);
        offset.vector.x = frame_key->direction[0] * 0.01f;
        offset.vector.y = frame_key->direction[1] * 0.01f;
        offset.vector.z = frame_key->direction[2] * 0.01f;
        offset.vector.w = 1.0f;
        MMathNormalizeVector2(&offset.vector, &offset.vector);
        tiWindDirSetDirection(slot, offset.vector.x, offset.vector.y, offset.vector.z);
        tiWindDirSetSpeed(slot, speed * 0.01f);
        tiWindDirSetRange(slot, frame_key->rangeUpper * 0.01f, frame_key->rangeLower * 0.01f);
        enabled = 0;
        radius = frame_key->radius;
        if (radius != 0 && radius < TIWIND_RADIUS_OFF) {
            enabled = 1;
        }
        if (enabled) {
            if (radius < 0) {
                tiWindDirSetRadius(slot, 0.0f, 1);
            } else {
                tiWindDirSetRadius(slot, radius * 0.01f, 0);
            }
            tiWindDirSetEnabled(slot, 1);
        } else {
            tiWindDirSetEnabled(slot, 0);
            tiWindReleaseSlots(key, subkey);
        }
    }
}
