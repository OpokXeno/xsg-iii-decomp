#include "common.h"
#include "shared.h"
#define SDV_KEY_END 0x1000        /* the sequence has ended */
#define SDV_KEY_WAIT_HIT 0x4000   /* the row waits for the hit signal */
#define SDV_KEY_FRAME_MASK 0x3FFF /* frame of a hit-wait row */
#include "main/tu122.h"
#include "main/tu132.h"
#include "main/xenoActor.h"

extern int gSdvHitReceived;

/* the hit signal (.sdata) */
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

/* the hit signal (.sdata) */
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
    SdvShadowKey *key;
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
    key = (SdvShadowKey *)(sequence->keys + reachedRow * sequence->stride);
    shadowValue3Hundredths = key->shadowValue3;
    velocityTenths = key->velocity;
    shadowValueHundredths = key->shadowValue;
    angleHundredths = key->angle;
    actorFlags = key->actorFlags;
    moveState = key->moveState;
    shadowCount = key->shadowCount;
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
