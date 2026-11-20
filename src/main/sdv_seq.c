#include "common.h"
#include "shared.h"
#define SDV_KEY_END 0x1000        /* the sequence has ended */
#define SDV_KEY_WAIT_HIT 0x4000   /* the row waits for the hit signal */
#define SDV_KEY_FRAME_MASK 0x3FFF /* frame of a hit-wait row */
#include "main/tu122.h"
#include "main/tu132.h"
#include "main/xenoActor.h"

extern int D_00551088;

/* the hit signal (.sdata) */
/* Returns the current row when the sequence reaches it (and advances to the next row), else -1. */
int sdvCheckSeqKey(SdvSequence *sequence)
{
    int result = -1;
    short key = sequence->keys[sequence->position * sequence->stride];
    SefLinkedContext *request = sefGetBattleData();
    int hitFrame = key & SDV_KEY_FRAME_MASK;

    if ((key & SDV_KEY_WAIT_HIT) && D_00551088 != 0 && sequence->elapsed >= hitFrame) {
        if (request->waitHit == 0) {
            sequence->position++;
        } else {
            result = sequence->position++;
        }
    } else if (sequence->elapsed >= key) {
        result = sequence->position++;
    }
    return result;
}

/* the hit signal (.sdata) */
extern float D_00453EB8[];

/* Steps the sequence's frame counter (in thousandths, capped at 4095) and returns the row it reaches. */
int func_001ED8A0(SdvSequence *sequence)
{
    int thousandths;

    if (sequence->keys[sequence->position * sequence->stride] == SDV_KEY_END) {
        return SDV_KEY_END;
    }
    thousandths = (sequence->elapsed + D_00453EB8[0] + 0.0001f) * 1000.0f;
    sequence->elapsed = thousandths / 1000.0f;
    if (sequence->elapsed > 4095.0f) {
        sequence->elapsed = 4095.0f;
    }
    return sdvCheckSeqKey(sequence);
}

/* Returns the row the sequence reaches (and advances past it), else -1. */
int sdvProgressKey(short *keys, short *position, int stride, int elapsed)
{
    int result = -1;
    int key = keys[*position * stride];
    SefLinkedContext *request = sefGetBattleData();
    int waitsForHit = key & SDV_KEY_WAIT_HIT;

    key &= SDV_KEY_FRAME_MASK;
    if (key == SDV_KEY_END) {
        return SDV_KEY_END;
    }
    if (elapsed >= key) {
        if (waitsForHit) {
            unsigned char waitHit = request->waitHit;

            if (D_00551088 == 0 && waitHit == 0) {
                (*position)++;
            } else {
                result = (*position)++;
            }
        } else {
            result = (*position)++;
        }
    }
    return result;
}


/* One row of a shadow key sequence: the flags pick the actor, the others are the shadow values. */
typedef struct SdvShadowKey {
    unsigned char unmodeled_00[2];
    short flags;            /* +0x02 */
    signed char shadowCount; /* +0x04 */
    signed char state;      /* +0x05 */
    short velocity;         /* +0x06 tenths */
    short shadowValue;      /* +0x08 hundredths */
    short angle;            /* +0x0A hundredths */
    short shadowValue3;     /* +0x0C hundredths */
} SdvShadowKey;

#define SDV_KEY_SOURCE 0x100   /* the request's source actor */
#define SDV_KEY_TARGET 0x200   /* the request's target actor */

extern float D_0054FC5C;
extern float D_0054FC60;

/* Steps a shadow key sequence and copies the row it reaches into the shadow fields of the
 * request's actor that the row's flags pick. */
void sdvExecShadowSeq(SdvSequence *sequence)
{
    int index;
    SefLinkedContext *request;
    XenoActor *actor;
    SdvShadowKey *key;
    int flags;
    int state;
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
    index = func_001ED8A0(sequence);
    if ((unsigned int)index >= SDV_KEY_END) {
        return;
    }
    request = sefGetBattleData();
    actor = 0;
    key = (SdvShadowKey *)(sequence->keys + index * sequence->stride);
    shadowValue3Hundredths = key->shadowValue3;
    velocityTenths = key->velocity;
    shadowValueHundredths = key->shadowValue;
    angleHundredths = key->angle;
    flags = key->flags;
    state = key->state;
    shadowCount = key->shadowCount;
    velocity = velocityTenths * D_0054FC60;
    shadowValue = shadowValueHundredths * D_0054FC5C;
    angle = angleHundredths * D_0054FC5C;
    shadowValue3 = shadowValue3Hundredths * D_0054FC5C;
    if (flags & SDV_KEY_TARGET) {
        actor = request->target;
    } else if (flags & SDV_KEY_SOURCE) {
        actor = request->source;
    }
    if (actor != 0) {
        actor->sweepPhase = angle;
        actor->shadowValue = shadowValue;
        actor->sweepSpeed = velocity;
        actor->shadowCount = shadowCount;
        actor->shadowValue3 = shadowValue3;
        actor->moveState = state;
    }
}
