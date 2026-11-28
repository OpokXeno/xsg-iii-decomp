#include "common.h"

#include "shared.h"
#include "main/Packet.h"
extern float gSdvFrameDelta[];

/* One 16-byte key of a Curve16: its time (+0x0C) and the index of the next key (+0x0E, negative when there is none). */
typedef struct CurveKey16 {
    unsigned char unmodeled_00[0x0C];
    short time;
    signed char next;
    unsigned char unmodeled_0F[0x01];
} CurveKey16;

/* A curve whose keys are 16-byte CurveKey16 records; the keys pointer sits at +0x40. */
typedef struct Curve16 {
    unsigned char unmodeled_00[0x40];
    CurveKey16 *keys;
} Curve16;

/* Playback state of a Curve16 attached to an effect track: time (+0x30), key index (+0x44), curve (+0x58). */
typedef struct CurveTrack16 {
    unsigned char unmodeled_00[0x30];
    float time;
    unsigned char unmodeled_34[0x10];
    short keyIndex;
    unsigned char unmodeled_46[0x12];
    Curve16 *curve;
} CurveTrack16;

/*
 * Advances a 16-byte-keyed curve track by one frame and returns the key it now sits on
 * (0 when the track has no key index yet). A key whose successor is negative only moves the index forward.
 */
CurveKey16 *sefAdvanceCurveTrack16(CurveTrack16 *track) {
    CurveKey16 *currentKey;
    signed char nextIndex;
    float trackTime;
    float overshoot;

    currentKey = 0;
    if (track->keyIndex >= 0) {
        currentKey = &track->curve->keys[track->keyIndex];
        trackTime = track->time;
        trackTime += gSdvFrameDelta[0];
        if (trackTime < 4096.0f) {
            overshoot = trackTime - (float) currentKey[1].time;
            if (overshoot > 0.0f) {
                currentKey = currentKey + 1;
                nextIndex = currentKey->next;
                if (nextIndex >= 0) {
                    track->keyIndex = nextIndex;
                    currentKey = &track->curve->keys[nextIndex];
                    trackTime = (float) currentKey->time + overshoot;
                } else {
                    track->keyIndex = track->keyIndex + 1;
                }
            }
        }
        track->time = trackTime;
    }
    return currentKey;
}

/* One keyframe of a Curve: a float value, the key time and the index of the next key. */
typedef struct Keyframe {
    float value;
    short time;
    short next;
} Keyframe;

/* A curve of Keyframe records; the keys pointer sits at +0x44. */
typedef struct Curve {
    unsigned char unmodeled_00[0x44];
    Keyframe *keys;
} Curve;

/* Playback state of a Curve (time at +0x34, key index at +0x46, curve at +0x58). */
typedef struct CurveTrack {
    unsigned char unmodeled_00[0x34];
    float time;
    unsigned char unmodeled_38[0x0E];
    short keyIndex;
    unsigned char unmodeled_48[0x10];
    Curve *curve;
} CurveTrack;

/*
 * Advances a keyframe curve track by one frame (same steps as sefAdvanceCurveTrack16 on Keyframe records).
 * The track keeps its own copy of the layout: its field offsets differ from the other track types.
 */
void func_001F79B8(CurveTrack *track) {
    Keyframe *currentKey;
    short nextIndex;
    float trackTime;
    float overshoot;

    trackTime = track->time;
    trackTime += gSdvFrameDelta[0];
    currentKey = &track->curve->keys[track->keyIndex];
    if (trackTime < 4096.0f) {
        overshoot = trackTime - (float) currentKey[1].time;
        if (overshoot > 0.0f) {
            nextIndex = currentKey[1].next;
            if (nextIndex >= 0) {
                track->keyIndex = currentKey[1].next;
                trackTime = (float) track->curve->keys[nextIndex].time + overshoot;
            } else {
                track->keyIndex = track->keyIndex + 1;
            }
        }
    }
    track->time = trackTime;
}

/* A Curve whose keys pointer sits at +0x4C instead of +0x44 (kept apart: the offsets decide the access). */
typedef struct ShiftedCurve {
    unsigned char unmodeled_00[0x4C];
    Keyframe *keys;
} ShiftedCurve;

/* Playback state of a ShiftedCurve track (time at +0x3C, key index at +0x4E, curve at +0x5C). */
typedef struct ShiftedCurveTrack {
    unsigned char unmodeled_00[0x3C];
    float time;
    unsigned char unmodeled_40[0x0A];
    short keyIndex;
    unsigned char unmodeled_4C[0x0C];
    ShiftedCurve *curve;
} ShiftedCurveTrack;

/* Same frame step as func_001F79B8 for a ShiftedCurveTrack (the layout differs, so the body is not shared). */
void func_001F7A60(ShiftedCurveTrack *track) {
    Keyframe *currentKey;
    short nextIndex;
    float trackTime;
    float overshoot;

    trackTime = track->time;
    trackTime += gSdvFrameDelta[0];
    currentKey = &track->curve->keys[track->keyIndex];
    if (trackTime < 4096.0f) {
        overshoot = trackTime - (float) currentKey[1].time;
        if (overshoot > 0.0f) {
            nextIndex = currentKey[1].next;
            if (nextIndex >= 0) {
                track->keyIndex = currentKey[1].next;
                trackTime = (float) track->curve->keys[nextIndex].time + overshoot;
            } else {
                track->keyIndex = track->keyIndex + 1;
            }
        }
    }
    track->time = trackTime;
}

/* One vector keyframe: four float components, the key time, the next key index and padding to 0x20 bytes. */
typedef struct VectorKeyframe {
    float value[4];
    short time;
    short next;
    unsigned char unmodeled_14[0x0C];
} VectorKeyframe;

/* A curve of VectorKeyframe records; the keys pointer sits at +0x48. */
typedef struct VectorCurve {
    unsigned char unmodeled_00[0x48];
    VectorKeyframe *keys;
} VectorCurve;

/* Playback state of a VectorCurve track (time at +0x38, key index at +0x4A, curve at +0x5C). */
typedef struct VectorCurveTrack {
    unsigned char unmodeled_00[0x38];
    float time;
    unsigned char unmodeled_3C[0x0C];
    short keyIndex;
    unsigned char unmodeled_4A[0x0E];
    VectorCurve *curve;
} VectorCurveTrack;

/* Same frame step as func_001F79B8 for a VectorCurveTrack (the layout differs, so the body is not shared). */
void func_001F7B08(VectorCurveTrack *track) {
    VectorKeyframe *currentKey;
    short nextIndex;
    float trackTime;
    float overshoot;

    trackTime = track->time;
    trackTime += gSdvFrameDelta[0];
    currentKey = &track->curve->keys[track->keyIndex];
    if (trackTime < 4096.0f) {
        overshoot = trackTime - (float) currentKey[1].time;
        if (overshoot > 0.0f) {
            nextIndex = currentKey[1].next;
            if (nextIndex >= 0) {
                track->keyIndex = currentKey[1].next;
                trackTime = (float) track->curve->keys[nextIndex].time + overshoot;
            } else {
                track->keyIndex = track->keyIndex + 1;
            }
        }
    }
    track->time = trackTime;
}

/* A 4x4 matrix stored as four AlignedVector4 rows. */
typedef struct Matrix4Rows {
    AlignedVector4 rows[4];
} Matrix4Rows;

/* Header of a curve data block: unmodeled bytes, then a pointer to the header table of shorts (kind at h[-2]). */
typedef struct CurveData {
    unsigned char unmodeled_00[0x40];
    short *header;
} CurveData;

/* An effect object that carries a curve: input and output vectors, a time, the curve data and the keyframe entries. */
typedef struct CurveObject {
    AlignedVector4 inputQuat;
    AlignedVector4 position;
    AlignedVector4 output;
    float time;
    unsigned char unmodeled_54[0x24];
    CurveData *data;
} CurveObject;

/* One keyframe entry of the object's curve: position, frame, two easing bytes (rate in the high nibble of e). */
typedef struct CurveEntry {
    float x;
    float y;
    float z;
    short frame;
    unsigned char b;
    unsigned char ease;
} CurveEntry;

extern Matrix4Rows sefCameraMatrixBlock;
extern void vu0QuatToMatrix(void*,void*);
extern float sefEaseCurve(int,float,float);
extern void MMathLerpKeyPoint(void*,void*,void*,float);
extern void MMathHermiteKeyNeighbors(void*,void*,void*,void*,void*);
extern void MMathCalcHermitePrm(void*,void*,void*,void*,void*,void*);
extern void MMathCalcHermite(void*,void*,void*,void*,void*,float);
extern void MMathApplyMatrix(void*,void*,void*);
extern int func_002046F8(void*,void*,int);

/* Copies the 4 quadwords of a matrix (lq/sq, $8-$11 clobbered). */
static inline void CopyMatrixRows(Matrix4Rows *destination, Matrix4Rows *source) {
    __asm__ __volatile__(
        "lq $8,0(%1)\n lq $9,16(%1)\n lq $10,32(%1)\n lq $11,48(%1)\n sq $8,0(%0)\n sq $9,16(%0)\n sq $10,32(%0)\n sq $11,48(%0)"
        :
        : "r"(destination), "r"(source)
        : "$8", "$9", "$10", "$11", "memory");
}

/* Copies one vector with a single lq/sq through $8. */
static inline void CopyVector(AlignedVector4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

/* Adds two vectors in VU0 (vf1 and vf2) and stores the sum to d. */
static inline void AddVector(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

/*
 * Updates a curve object for one frame: it evaluates the curve track into a position, eases between
 * the two keyframes around it, and writes the result to object->output through the rotation or the
 * offset matching the object kind. The camera matrix is saved and restored around kinds 4..6.
 */
void func_001F7BB0(CurveObject *object) {
    Matrix4Rows savedCamera;
    AlignedVector4 value;
    AlignedVector4 tangents[2];
    AlignedVector4 *hermitePoints[4];
    CurveEntry *entry;
    float span;
    float eased;

    entry = ((CurveEntry * (*) (CurveObject *) ) sefAdvanceCurveTrack16)(object);
    if (object->data->header[-2] >= 4 && object->data->header[-2] <= 6) {
        CopyMatrixRows(&savedCamera, &sefCameraMatrixBlock);
        vu0QuatToMatrix(&sefCameraMatrixBlock, object);
        CopyVector(&sefCameraMatrixBlock.rows[3], &object->position);
    }
    if (entry != 0) {
        span = (float) (entry[1].frame - entry->frame);
        if (span != 0.0f) {
            eased = sefEaseCurve(entry->ease >> 4, object->time - entry->frame, span);
            switch (entry->ease & 15) {
            case 0:
            default:
                MMathLerpKeyPoint(&value, entry, entry + 1, eased);
                break;
            case 1:
                MMathHermiteKeyNeighbors(entry, &hermitePoints[0], &hermitePoints[1], &hermitePoints[2], &hermitePoints[3]);
                MMathCalcHermitePrm(&tangents[0], &tangents[1], hermitePoints[0], hermitePoints[1], hermitePoints[2], hermitePoints[3]);
                MMathCalcHermite(&value, &tangents[0], &tangents[1], hermitePoints[1], hermitePoints[2], eased);
                break;
            }
        }
    } else {
        CopyVector(&value, (AlignedVector4 *) object->data->header);
    }
    {
        if (object->data->header[-2] == 0x3B || object->data->header[-2] == 0x3C) {
            vu0QuatToMatrix(&savedCamera, object);
            CopyVector(&savedCamera.rows[3], &object->position);
            MMathApplyMatrix(&object->output, &savedCamera, &value);
        } else if (object->data->header[-2] == 0x39 || object->data->header[-2] == 0x3A || object->data->header[-2] == 0x144) {
            AddVector(&object->output, &value, &object->position);
        } else {
            func_002046F8(&object->output, &value, object->data->header[-2]);
        }
    }
    if (object->data->header[-2] >= 4 && object->data->header[-2] <= 6) {
        CopyMatrixRows(&sefCameraMatrixBlock, &savedCamera);
    }
}

/*
 * Evaluates a Curve track into result: linear interpolation between the current key and the next one,
 * or the next key's value when the two keys share a time.
 */
void func_001F7E38(float *result, CurveTrack *track) {
    Keyframe *currentKey;
    float weight;

    currentKey = &track->curve->keys[track->keyIndex];
    weight = currentKey[1].time - currentKey[0].time;
    if (weight != 0.0f) {
        weight = (track->time - currentKey[0].time) / weight;
        *result = currentKey[1].value * weight + currentKey[0].value * (1.0f - weight);
    } else {
        *result = currentKey[1].value;
    }
}

/* Same interpolation as func_001F7E38 for a ShiftedCurveTrack (kept as its own copy: the layout differs). */
void func_001F7EC8(float *result, ShiftedCurveTrack *track) {
    Keyframe *currentKey;
    float weight;

    currentKey = &track->curve->keys[track->keyIndex];
    weight = currentKey[1].time - currentKey[0].time;
    if (weight != 0.0f) {
        weight = (track->time - currentKey[0].time) / weight;
        *result = currentKey[1].value * weight + currentKey[0].value * (1.0f - weight);
    } else {
        *result = currentKey[1].value;
    }
}

/* Four signed 32-bit lanes, the integer form of a Vector4 (used by the VU0 float-to-int store). */
typedef struct IntVector4 {
    int x;
    int y;
    int z;
    int w;
} IntVector4;

/*
 * Writes out = the vector curve track evaluated at its current frame, as four integers (vftoi0).
 * The weight is (time - key time) / span; with a zero span only the next key's vector is stored.
 */
void func_001F7F58(IntVector4 *out, VectorCurveTrack *track) {
    VectorKeyframe *currentKey;
    Vector4 weights;
    float span;
    float fraction;

    currentKey = &track->curve->keys[track->keyIndex];
    span = currentKey[1].time - currentKey[0].time;
    if (span != 0.0f) {
        fraction = (track->time - currentKey[0].time) / span;
        weights.x = fraction;
        __asm__ __volatile__("lqc2 vf1, 0(%1)\n"
                             "lqc2 vf2, 0(%2)\n"
                             "mfc1 $8, %3\n"
                             "qmtc2 $8, vf3\n"
                             "vsub.xyzw vf2, vf2, vf1\n"
                             "vmulx.xyzw vf2, vf2, vf3x\n"
                             "vadd.xyzw vf2, vf2, vf1\n"
                             "vftoi0.xyzw vf1, vf2\n"
                             "sqc2 vf1, 0(%0)"
                             :
                             : "r"(out), "r"(currentKey), "r"(&currentKey[1]), "f"(fraction), "m"(weights)
                             : "$8", "memory");
    } else {
        __asm__ __volatile__("lqc2 vf1, 0(%1)\n"
                             "vftoi0.xyzw vf1, vf1\n"
                             "sqc2 vf1, 0(%0)"
                             :
                             : "r"(out), "r"(&currentKey[1])
                             : "memory");
    }
}

/* Flag word of an EffectGroup: the same 32 bits seen as unsigned flags or as a signed value. */
typedef union EffectFlags {
    unsigned int word;
    int signedWord;
} EffectFlags;

/* One particle of an effect entry: its life, three track key indices (+0x46, +0x48, +0x4A), a use-curves flag at +0x4E and the next particle. */
typedef struct EffectParticle {
    unsigned char unmodeled_00[0x40];
    float life;
    short unmodeled_44;
    short trackKeyIndex[3];
    short unmodeled_4C;
    short usesCurves;
    short unmodeled_50;
    short unmodeled_52;
    struct EffectParticle *next;
} EffectParticle;

/* One effect entry (0x70 bytes): last emission time (+0x34), start and end times, period, flags and particle list. */
typedef struct EffectEntry {
    unsigned char unmodeled_00[0x34];
    float lastEmitTime;
    unsigned char unmodeled_38[0x52 - 0x38];
    short startTime;
    short endTime;
    short period;
    unsigned long long flags;
    EffectParticle *particles;
    unsigned char unmodeled_68[0x70 - 0x64];
} EffectEntry;

/* An effect group: up to 32 entry slot ids in ids[], the active count n, the group time in ms and its rate. */
typedef struct EffectGroup {
    int unmodeled_00;
    int callArg0;
    int callArg1;
    int timeMs;
    float rate;
    int unmodeled_14;
    short ids[32];
    unsigned char unmodeled_58[0x5E - 0x58];
    signed char activeCount;
    unsigned char unmodeled_5F;
    EffectFlags flags;
} EffectGroup;

/* Counters of the effect table head (gSefEffectTable at +0x494B4): entries with and without the 0x10000 flag. */
typedef struct EffectCounters {
    unsigned char unmodeled_00[0x494B4];
    unsigned short countWithFlag;
    unsigned short countWithoutFlag;
    unsigned char unmodeled_494B8[0x4950C - 0x494B8];
} EffectCounters;

/* The studio block (gSbStudioBlock): two 64-bit words, two single bits, the 2-bit studio mode and the high bits. */
typedef struct StudioBlock {
    unsigned long long unmodeled_00;
    unsigned long unmodeled_08;
    unsigned long long unmodeled_flag0 : 1;
    unsigned long long unmodeled_flag1 : 1;
    unsigned long long mode : 2;
    unsigned long long unmodeled_hi : 60;
} StudioBlock;

/* Per-step block: the step in ms, the step in seconds (rounded) and that step scaled by the group rate. */
typedef struct KeyframeStepBlock {
    int stepMs;
    float stepSeconds;
    float scaledStepSeconds;
} KeyframeStepBlock;

/* The studio camera: its time scale sits at +0x5F4. */
typedef struct StudioCamera {
    unsigned char unmodeled_00[0x5F4];
    float timeScale;
} StudioCamera;

extern EffectEntry gSefEffectPool[400];
extern EffectCounters gSefEffectTable;
extern StudioBlock gSbStudioBlock;
extern KeyframeStepBlock sefKeyframeStepBlock;
extern StudioCamera *xglStudioGetCamera2(int);
extern void func_002018E8(int,int,int,int,int,int);
extern void sefEffectPoolRelease(int);
extern void func_001F62F8(EffectEntry*);
extern void sefEffectNodeRelease(EffectParticle*);
extern void sefTakeKeys(EffectParticle*);
extern void sefEffectInstanceRelease(EffectGroup*);

/* Entry record i of gSefEffectPool, or 0 when the index is out of range (400 records). */
static inline EffectEntry *EffectEntryAt(int index) {
    EffectEntry *entry;
    if ((unsigned int) index < 400) {
        entry = &gSefEffectPool[index];
    } else {
        entry = 0;
    }
    return entry;
}

/* Counts one entry in the flagged or the unflagged counter of the table head (flag 0x10000). */
static inline void CountEffectEntry(EffectEntry *entry) {
    if (entry->flags & 0x10000) {
        gSefEffectTable.countWithFlag++;
    } else {
        gSefEffectTable.countWithoutFlag++;
    }
}

/*
 * Steps one effect group by one frame: the group's entries are emitted (func_001F62F8), their particles are
 * aged by the scaled step and then updated or freed. Entries that are out of time are released (sefEffectPoolRelease).
 * A group with flag 0x200 only counts its live entries; a group whose entries are all gone is released (sefEffectInstanceRelease).
 */
void sefEffectGroupStep(EffectGroup *group) {
    int i;
    int activeLeft;
    float timeSeconds;
    EffectEntry *entry;
    EffectParticle *particle;
    EffectParticle *previous;
    int period;
    float delta;

    if (group == 0) {
        return;
    }
    if (group->flags.word & 0x200) {
        int remaining = group->activeCount;
        int j;
        for (j = 0; j < 32 && remaining > 0; j++) {
            EffectEntry *counted = EffectEntryAt(group->ids[j]);
            if (counted != 0) {
                remaining--;
                if (group->flags.word & 0x100) {
                    CountEffectEntry(counted);
                }
            }
        }
        return;
    }
    activeLeft = group->activeCount;
    timeSeconds = (float) group->timeMs / 1000.0f;
    sefKeyframeStepBlock.stepSeconds =
        (float) (int) ((xglStudioGetCamera2(0)->timeScale * ((float) sefKeyframeStepBlock.stepMs / 1000.0f) + 0.0001f) *
                       1000.0f) /
        1000.0f;
    sefKeyframeStepBlock.scaledStepSeconds = sefKeyframeStepBlock.stepSeconds * group->rate;
    func_002018E8(group->callArg0, group->callArg1, 0, 0, 0, 0);
    for (i = 0; i < 32 && activeLeft > 0; i++) {
        entry = EffectEntryAt(group->ids[i]);
        if (entry == 0) {
            continue;
        }
        activeLeft--;
        if (entry->endTime <= timeSeconds && entry->endTime != 0x2000 && entry->particles == 0) {
            sefEffectPoolRelease(group->ids[i]);
            group->ids[i] = -1;
            if (--group->activeCount < 0) {
                group->activeCount = 0;
                break;
            }
            continue;
        }
        if (group->flags.word & 0x100) {
            CountEffectEntry(entry);
        }
        if (entry->startTime <= timeSeconds && (timeSeconds < entry->endTime || entry->endTime == 0x2000)) {
            period = entry->period;
            if (period == -1) {
                if (timeSeconds == entry->startTime) {
                    func_001F62F8(entry);
                } else if (entry->particles == 0) {
                    sefEffectPoolRelease(group->ids[i]);
                    group->ids[i] = period;
                    if (--group->activeCount < 0) {
                        group->activeCount = 0;
                        break;
                    }
                    continue;
                }
            } else if (gSbStudioBlock.mode != 0) {
                if (period <= timeSeconds - entry->lastEmitTime) {
                    func_001F62F8(entry);
                    entry->lastEmitTime = timeSeconds;
                }
            } else {
                if (entry->lastEmitTime < 0.0f) {
                    delta = timeSeconds - entry->startTime;
                    if (delta == 0.0f) {
                        delta = period;
                    }
                } else {
                    delta = timeSeconds - entry->lastEmitTime;
                }
                while (entry->period <= delta) {
                    func_001F62F8(entry);
                    entry->lastEmitTime = timeSeconds;
                    delta -= entry->period;
                }
            }
        }
        particle = entry->particles;
        previous = 0;
        while (particle != 0) {
            particle->life -= sefKeyframeStepBlock.scaledStepSeconds;
            if (particle->life < 0.0f) {
                if (gSbStudioBlock.mode != 0) {
                    sefEffectNodeRelease(particle);
                    if (previous != 0) {
                        previous->next = 0;
                    } else {
                        entry->particles = 0;
                    }
                    break;
                } else {
                    EffectParticle *dead = particle;
                    if (previous != 0) {
                        previous->next = particle->next;
                    } else {
                        entry->particles = particle->next;
                    }
                    particle = particle->next;
                    sefEffectNodeRelease(dead);
                }
            } else {
                if (particle->usesCurves != 0) {
                    func_001F7BB0((CurveObject *) particle);
                    if (particle->trackKeyIndex[0] >= 0) {
                        func_001F79B8((CurveTrack *) particle);
                    }
                    if (particle->trackKeyIndex[1] >= 0) {
                        func_001F7B08((VectorCurveTrack *) particle);
                    }
                    if (particle->trackKeyIndex[2] >= 0) {
                        func_001F7A60((ShiftedCurveTrack *) particle);
                    }
                } else {
                    sefTakeKeys(particle);
                }
                previous = particle;
                particle = previous->next;
            }
        }
    }
    if (group->activeCount <= 0) {
        sefEffectInstanceRelease(group);
        return;
    }
    group->timeMs += (int) ((sefKeyframeStepBlock.scaledStepSeconds + 0.0001f) * 1000.0f);
}

/* Key of an effect table entry: its id (+0x08), active flag (+0x0E) and tag (+0x0F). */
typedef struct EntryTableKey {
    unsigned char unmodeled_00[0x08];
    short id;
    unsigned char unmodeled_0A[0x04];
    signed char active;
    signed char tag;
} EntryTableKey;

/* The same key as the flag view reads it: the slot is a short at +0x0C. */
typedef struct EntryTableKeySlot {
    unsigned char unmodeled_00[0x08];
    short id;
    unsigned char unmodeled_0A[0x02];
    short slot;
    signed char active;
    signed char tag;
} EntryTableKeySlot;

/* Flags of an entry: the low byte, bits 8 and 9, a 2-bit field and the high 20 bits. */
typedef union EntryFlags {
    unsigned int word;
    struct {
        unsigned int lo : 8;
        unsigned int b8 : 1;
        unsigned int b9 : 1;
        unsigned int b10 : 2;
        unsigned int hi : 20;
    } bits;
} EntryFlags;

/* One effect table entry (0x64 bytes): its key at +0x50 and its flags at +0x60. */
typedef struct EntryTableItem {
    unsigned char unmodeled_00[0x50];
    EntryTableKey key;
    unsigned int flags;
} EntryTableItem;

/* The same entry read through the flag bits (kept apart: the field types decide the emitted order). */
typedef struct EntryTableItemBits {
    unsigned char unmodeled_00[0x50];
    EntryTableKeySlot key;
    EntryFlags flags;
} EntryTableItemBits;

/* The effect table at gSefEffectTable: 40 entries from +0x47180, then the gate and two counters. */
typedef struct EntryTable {
    unsigned char unmodeled_00[0x47180];
    EntryTableItem entries[40];
    unsigned char unmodeled_48120[0x1392];
    short gate;
    short unmodeled_cb4;
    short unmodeled_cb6;
} EntryTable;

/* The same table read with the flag-bit entries. */
typedef struct EntryTableView {
    unsigned char unmodeled_00[0x47180];
    EntryTableItemBits entries[40];
    unsigned char unmodeled_48120[0x1392];
    short gate;
    short unmodeled_cb4;
    short unmodeled_cb6;
} EntryTableView;

/* The entry at index when it is active (index below 40), otherwise 0. */
static inline EntryTableItem *EntryAtIndex(unsigned int index) {
    if (index < 0x28U && (*(EntryTable *) &gSefEffectTable).entries[index].key.active != 0) {
        return &(*(EntryTable *) &gSefEffectTable).entries[index];
    }
    return 0;
}

/*
 * Steps the active effect groups of the table once: at most 'gate' of its 40 entries are updated
 * through sefEffectGroupStep. The two counters are cleared first.
 */
void sefUpdateAllEffectGroups(void) {
    int remaining;
    int i;

    (*(EntryTable *) &gSefEffectTable).unmodeled_cb4 = 0;
    (*(EntryTable *) &gSefEffectTable).unmodeled_cb6 = 0;
    remaining = (*(EntryTable *) &gSefEffectTable).gate;
    for (i = 0; i < 40 && remaining > 0; i++) {
        EntryTableItem *entry = EntryAtIndex(i);

        if (entry != 0) {
            remaining--;
            sefEffectGroupStep(entry);
        }
    }
}

/* A record of the effect array gSefEffectPool as the pool code sees it: a kind byte (+0x5E) and the head pointer (+0x60). */
typedef struct EffectPool {
    unsigned char unmodeled_00[0x5E];
    unsigned char kind;
    unsigned char unmodeled_5F[0x01];
    void *head;
    unsigned char unmodeled_64[0x0C];
} EffectPool;

/* One request written by func_001F8750: the slot, the pool kind and an on flag. */
typedef struct EffectRequest {
    short slot;
    short kind;
    short on;
} EffectRequest;

/* The slot view of an entry: the 32 pool slot ids (shorts) after the 0x18-byte header. */
typedef struct EffectSlotView {
    unsigned char unmodeled_00[0x18];
    short slots[32];
} EffectSlotView;

/* The pool record at index (400 records), or 0 when the index is out of range. */
static inline EffectPool *EffectPoolAt(unsigned int index) {
    if (index < 400) {
        return &(*(EffectPool(*)[]) & gSefEffectPool)[index];
    }
    return 0;
}

/* The active entry with this id, when the table gate is open; otherwise 0. */
static inline EntryTableItem *FindEntryById(int id) {
    int i;

    if ((*(EntryTable *) &gSefEffectTable).gate != 0) {
        for (i = 0; i < 40; i++) {
            if ((*(EntryTable *) &gSefEffectTable).entries[i].key.id == id &&
                (*(EntryTable *) &gSefEffectTable).entries[i].key.active != 0) {
                return &(*(EntryTable *) &gSefEffectTable).entries[i];
            }
        }
    }
    return 0;
}

/*
 * Lists the pool slots of the entry with this id whose flags read 0x900 under mask 0xD00 into requests,
 * starting at index count. Returns the new count.
 */
int func_001F8750(int id, EffectRequest *requests, int count) {
    EntryTableItem *entry = FindEntryById(id);
    EffectSlotView *view;
    int i;

    if (entry != 0 && (entry->flags & 0xD00) == 0x900) {
        view = (EffectSlotView *) entry;
        for (i = 0; i < 32; i++) {
            EffectPool *pool = EffectPoolAt((unsigned int) view->slots[i]);

            if (pool != 0 && pool->head != 0) {
                requests[count].slot = view->slots[i];
                requests[count].kind = pool->kind;
                requests[count].on = 1;
                count++;
            }
        }
    }
    return count;
}

/* One record of the sprite table gSefEffectTransforms: four 16-byte vectors (position, q, r and s). */
typedef struct EntryRecord {
    float pos[4];
    float q[4];
    float r[4];
    float s[4];
} EntryRecord;

extern EntryRecord gSefEffectTransforms[];

/* One hit of the depth list: the entry index, its kind (2 for these hits) and the clip depth. */
typedef struct EntryHit {
    short index;
    short kind;
    float depth;
} EntryHit;

extern EntryHit svVisibleSlots[];
extern int svVisibleEntryCount;
void sefTransformClipPoint(AlignedVector4 *outPoint, EntryRecord *inPoint);

/* The entry at index when it is active, read through the flag-bit table view. */
static inline EntryTableItemBits *EntryAtIndexBits(unsigned int index) {
    return (index < 0x28U && (*(EntryTableView *) &gSefEffectTable).entries[index].key.active != 0)
               ? &(*(EntryTableView *) &gSefEffectTable).entries[index]
               : 0;
}

/* The sprite record of a slot (below 40) whose w component is set, otherwise 0. */
static inline EntryRecord *RecordAtSlot(unsigned int slot) {
    if (slot < 0x28U && gSefEffectTransforms[slot].pos[3] != 0.0f) {
        return &gSefEffectTransforms[slot];
    }
    return 0;
}

/*
 * Appends one hit per active entry with flag 0x100 whose sprite record is set: the hit gets the entry
 * index and the clipped depth of the record. The count continues from svVisibleEntryCount.
 */
void func_001F8860(void) {
    AlignedVector4 clipPoint;
    EntryTableView *table;
    EntryTableItemBits *entry;
    EntryRecord *record;
    int hitCount;
    int i;
    EntryHit *nextHit;

    table = &gSefEffectTable;
    if (table->gate != 0) {
        hitCount = svVisibleEntryCount;
        nextHit = &svVisibleSlots[hitCount];
        for (i = 0; i < 40; i++) {
            entry = EntryAtIndexBits(i);
            if (entry != 0 && entry->key.active != 0 && (entry->flags.word & 0x100) != 0) {
                record = RecordAtSlot(entry->key.slot);
                if (record != 0) {
                    hitCount++;
                    sefTransformClipPoint(&clipPoint, record);
                    nextHit->index = i;
                    nextHit->kind = 2;
                    nextHit->depth = clipPoint.z;
                    nextHit++;
                }
            }
        }
        svVisibleEntryCount = hitCount;
    }
}

/* The VU object of an effect: a 64-bit word copied into the packet (+0x20), the index table header (+0x48), scale bytes and a flag. */
typedef struct VuObject {
    unsigned char unmodeled_00[0x20];
    unsigned long long packetWord2;
    unsigned char unmodeled_28[0x20];
    short *header;
    unsigned char unmodeled_50[0xF];
    unsigned char scaleS;
    unsigned char scaleT;
    unsigned char drawFlag;
} VuObject;

/* The screen parameters at D_004C8390: packed into the packet with the scene flag, as shorts and unsigned words. */
typedef struct VuScreenParams {
    short unmodeled_00;
    short unmodeled_02;
    short unmodeled_04;
    short unmodeled_06;
    short unmodeled_08;
    short unmodeled_0A;
    short unmodeled_0C;
    unsigned short sceneLowWord;
    short unmodeled_10;
    short unmodeled_12;
    short unmodeled_14;
    unsigned short plainLowWord;
} VuScreenParams;

/* The camera of the studio: two vectors at +0x70 and +0x80 and the camera matrix at +0x470. */
typedef struct VuCamera {
    unsigned char unmodeled_00[0x70];
    AlignedVector4 vectorA;
    AlignedVector4 vectorB;
    unsigned char unmodeled_90[0x470 - 0x90];
    Matrix4Rows matrix;
} VuCamera;

/* A light: its position and its colour components (colour[3] unused). */
typedef struct VuLight {
    AlignedVector4 position;
    float colour[4];
} VuLight;

/* The upload block D_00408180: seven GS register pairs; the packet references the first eleven words. */
typedef struct VuUploadBlock {
    unsigned long long words[14];
} VuUploadBlock;

/* The VU draw block written into the packet (13 quadwords): tag, mode and light flags, scales, the matrix and the vectors. */
typedef struct VuDrawBlock {
    unsigned int tag[4];
    int mode;
    int hasLight;
    int unmodeled_18;
    int unmodeled_1C;
    float scaleS;
    float scaleT;
    int unmodeled_28;
    int unmodeled_2C;
    Matrix4Rows matrix;
    AlignedVector4 vectorA;
    AlignedVector4 vectorB;
    AlignedVector4 lightPosition;
    AlignedVector4 lightColour;
} VuDrawBlock;

/* The second block of the draw packet: its tag words, a tag quadword and the register list. */
typedef struct VuTailBlock {
    unsigned int tag[4];
    unsigned long long tagWord;
    unsigned long long regs;
} VuTailBlock;

extern VuUploadBlock D_00408180;
extern VuScreenParams D_004C8390;
extern unsigned long long sdvAlphaBlendTable[];
extern int svEffectSceneFlag;
extern unsigned char D_1FC0[];
extern void *xglPacketReserve(int);
extern void *xglPacketGetCurrent(void);
extern VuCamera *xglStudioGetActiveCamera(void);
extern void sceVif1PkCnt(void*,unsigned int);
extern void sceVif1PkOpenUpkCode(void*,unsigned int,unsigned int,unsigned int,unsigned int);
extern void sceVif1PkAddUpkData128N(void*,void*,unsigned int);
extern void sceVif1PkCloseUpkCode(void*);
extern void sceVif1PkRef(void*,void*,unsigned int,unsigned int,unsigned int,unsigned int);
extern void sceVif1PkAddCode(void*,unsigned int);

/* Copies the 4 quadwords of a matrix: the same asm as CopyMatrixRows, kept as its own definition (label order). */
static inline void CopyMatrixRowsVu(Matrix4Rows *destination, Matrix4Rows *source) {
    __asm__ __volatile__(
        "lq $8,0(%1)\n lq $9,16(%1)\n lq $10,32(%1)\n lq $11,48(%1)\n sq $8,0(%0)\n sq $9,16(%0)\n sq $10,32(%0)\n sq $11,48(%0)"
        :
        : "r"(destination), "r"(source)
        : "$8", "$9", "$10", "$11", "memory");
}

/* Copies one vector through $8: the same asm as CopyVector, kept as its own definition (label order). */
static inline void CopyVectorVu(AlignedVector4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

/* Writes the zero vector (sqc2 vf0) to d. */
static inline void ZeroVector(AlignedVector4 *destination) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

/* Writes the vector (0, 0, 0, 1) to d (vmaxw against vf0 with w). */
static inline void SetVectorOne(AlignedVector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw vf1,vf0,vf0w\n sqc2 vf1,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination));
}

/*
 * Builds the draw packet of one VU effect object: the upload block, the scene data, the optional light and
 * the camera matrix go into a draw block, followed by a tail block. Returns 0 when the packet is full.
 */
int sefBuildDrawPacket(VuObject *object, VuLight *light, int mode) {
    VuDrawBlock *drawBlock;
    VuTailBlock *tailBlock;
    void *packet;
    VuCamera *camera;
    Matrix4Rows *cameraMatrix;

    drawBlock = xglPacketReserve(13);
    if (drawBlock == 0) {
        return 0;
    }
    packet = xglPacketGetCurrent();
    camera = xglStudioGetActiveCamera();
    cameraMatrix = &camera->matrix;
    D_00408180.words[2] = object->packetWord2;
    D_00408180.words[4] = object->drawFlag != 0 ? 5 : 0;
    D_00408180.words[8] = sdvAlphaBlendTable[object->header[-1]];
    if (svEffectSceneFlag != 0) {
        D_00408180.words[10] = (unsigned long long) D_004C8390.sceneLowWord | ((long long) (D_004C8390.unmodeled_04 / 64) << 16) |
                               ((long long) D_004C8390.unmodeled_02 << 24);
    } else {
        D_00408180.words[10] = (unsigned long long) D_004C8390.plainLowWord | ((long long) (D_004C8390.unmodeled_04 / 64) << 16) |
                               ((long long) D_004C8390.unmodeled_02 << 24);
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkOpenUpkCode(packet, 0, 0x6C, 1, 1);
    sceVif1PkAddUpkData128N(packet, &D_00408180, 6);
    sceVif1PkCloseUpkCode(packet);
    drawBlock->tag[0] = 0x11000000;
    drawBlock->tag[1] = 0;
    drawBlock->tag[2] = 0x1000101;
    drawBlock->tag[3] = 0x6C0A0006;
    drawBlock->mode = mode;
    drawBlock->hasLight = light != 0;
    drawBlock->scaleS = object->scaleS * 0.005f;
    drawBlock->scaleT = object->scaleT * 0.005f;
    drawBlock->unmodeled_28 = 0;
    drawBlock->unmodeled_2C = 0;
    CopyMatrixRowsVu(&drawBlock->matrix, cameraMatrix);
    CopyVectorVu(&drawBlock->vectorA, &camera->vectorB);
    CopyVectorVu(&drawBlock->vectorB, &camera->vectorA);
    *(int *) &drawBlock->vectorA.w = 0;
    *(int *) &drawBlock->vectorB.w = 0;
    if (light != 0) {
        CopyVectorVu(&drawBlock->lightPosition, &light->position);
        drawBlock->lightColour.x = light->colour[0];
        drawBlock->lightColour.y = light->colour[1];
        drawBlock->lightColour.z = light->colour[2];
        drawBlock->lightColour.w = (light->colour[0] + light->colour[1] + light->colour[2]) / 3.0f;
    } else {
        ZeroVector(&drawBlock->lightPosition);
        SetVectorOne(&drawBlock->lightColour);
    }
    sceVif1PkCnt(packet, 0);
    sceVif1PkRef(packet, drawBlock, 11, 0, 0, 0);
    tailBlock = (VuTailBlock *) (drawBlock + 1);
    sceVif1PkCnt(packet, 0);
    sceVif1PkAddCode(packet, ((unsigned int) D_1FC0 >> 3) | 0x14000000);
    sceVif1PkAddCode(packet, 0x11000000);
    tailBlock->tag[0] = 0x11000000;
    tailBlock->tag[1] = 0;
    tailBlock->tag[2] = 0x1000101;
    tailBlock->tag[3] = 0x6C01001E;
    tailBlock->tagWord = 0x902A400000008001ULL;
    tailBlock->regs = 0x525252512ULL;
    sceVif1PkCnt(packet, 0);
    sceVif1PkRef(packet, tailBlock, 2, 0, 0, 0);
    return 1;
}

/* A 128-bit quadword as the compiler's 128-bit integer type. */
typedef int Quadword128 __attribute__((mode(TI)));

/* A quadword seen as one 128-bit value or as four 32-bit words. */
typedef union QuadwordView {
    Quadword128 quadword;
    int words[4];
} QuadwordView;

/* A VIF segment packet: the tag quadword and the body quadword (first and second words). */
typedef struct SegmentPacket {
    int tag[4];
    int body[4];
} SegmentPacket;

extern unsigned char D_20B8[];

/*
 * Writes one segment packet of count quadwords: a tag, a zeroed body quadword, the two words of the
 * body, then the upload that references D_20B8 (sceVif1PkRef). The packet ends with the D_20B8 code.
 */
void func_001F8D38(SegmentPacket *packet, int count, int firstWord, int secondWord) {
    void *packetCurrent = xglPacketGetCurrent();
    packet->tag[0] = 0x11000000;
    packet->tag[1] = 0;
    packet->tag[2] = 0x01000101;
    packet->tag[3] = ((count - 1) << 16) | 0x6C008000;
    ((QuadwordView *) packet->body)->quadword = 0;
    packet->body[0] = firstWord;
    packet->body[1] = secondWord;
    sceVif1PkCnt(packetCurrent, 0);
    sceVif1PkRef(packetCurrent, packet, count, 0, 0, 0);
    sceVif1PkCnt(packetCurrent, 0);
    sceVif1PkAddCode(packetCurrent, 0x14000000 | ((unsigned int) D_20B8 >> 3));
    sceVif1PkAddCode(packetCurrent, 0x11000000);
}

/* The animation of a line effect: its fade rate (+0x00) and the animation kind (+0x04). The anim pointer points one past it. */
typedef struct LineAnim {
    float fade;
    short kind;
    short unmodeled_06;
} LineAnim;

/* A node of a line effect's trail: its position and vector, the birth time, the key indices of its three angle and vector curves, the key count and the first key. */
typedef struct LineNode {
    unsigned char unmodeled_00[16];
    AlignedVector4 pos;
    AlignedVector4 v20;
    unsigned char unmodeled_30[16];
    float birthTime;
    short unmodeled_44;
    short angleAKeyIndex;
    short vectorKeyIndex;
    short angleBKeyIndex;
    short firstKey;
    short keyCount;
    unsigned char unmodeled_56[4];
    struct LineNode *next;
} LineNode;

/* A line effect (pool record, 0x400-entry array gSefEffectPool): texture rates, duration, the animation and the angle arrays, the binding and the head node. */
typedef struct LineEffect {
    unsigned char unmodeled_00[0x28];
    float texRateU;
    float texRateV;
    float duration;
    unsigned char unmodeled_34[8];
    int interpolateKeys;
    LineAnim *anim;
    float *angles;
    AlignedVector4 **offsetVectors;
    float *alphaValues;
    unsigned char unmodeled_54[8];
    union {
        unsigned long bits;
        short kind;
    } binding;
    LineNode *head;
    unsigned char unmodeled_6C[12];
} LineEffect;

/* A line record of D_004516B0_1F8E28: a position with its w component, and the colour, scale and bias vectors. */
typedef struct LineRecord {
    float pos[4];
    AlignedVector4 colour;
    AlignedVector4 scale;
    AlignedVector4 bias;
} LineRecord;

/* One key of the line key table: its position, angle and the vector used with it. */
typedef struct LineKey {
    float x;
    float y;
    float z;
    float angle;
    AlignedVector4 pos;
} LineKey;

/* One entry of a line table: its slot, active flag and tag (the resource type). */
typedef struct LineEntry {
    unsigned char unmodeled_00[0x5c];
    short slot;
    signed char active;
    signed char tag;
    unsigned char unmodeled_61[4];
} LineEntry;

/* The line table: 5000 keys, then 40 entries. */
typedef struct LineTable {
    LineKey keys[5000];
    unsigned char unmodeled_keys[0x20080];
    LineEntry entries[40];
} LineTable;

/* A resource type list: its reference count (+0x28). */
typedef struct ResourceTypeList {
    unsigned char unmodeled_00[0x28];
    short refs;
    unsigned char unmodeled_2A[0x13DC - 0x2A];
} ResourceTypeList;

/* One drawn line segment packet: the tag words, the alpha, the texture coordinates, the node, the offsets and the matrix. */
typedef struct LineSegment {
    unsigned int tag[4];
    float alpha;
    float texU;
    float texV;
    float pad;
    AlignedVector4 node;
    AlignedVector4 offsets;
    Matrix4Rows transform;
} LineSegment;

/* Four 32-bit lanes of an offset vector, as integers or as a vector. */
typedef union IntVectorView {
    int i[4];
    AlignedVector4 v;
} IntVectorView;

/* A quadword as a vector, floats, ints or the 128-bit value. */
typedef union PacketQuadword {
    Quadword128 q;
    AlignedVector4 v;
    float f[4];
    int i[4];
} PacketQuadword;

extern LineRecord D_004516B0_1F8E28[];
extern ResourceTypeList svResourceTypeLists[];
extern Matrix4Rows vu0IdentityMatrix;
extern Matrix4Rows sefCameraMatrix;
extern unsigned char D_2048[];
extern void *sefGetBattleData(void);
extern void svLoadImageList(void*);
extern void svRefMapperPacket(void);
extern void vu0RotateCurrentZ(float);
extern void vu0CurMatrixMulRight(Matrix4Rows*);

/* Copies one vector through $8: the same asm as CopyVector, kept as its own definition (label order). */
static inline void CopyVectorLe(AlignedVector4 *destination, const AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

/* Loads the identity matrix into vf26-vf29 (vu0IdentityMatrix). */
static inline void LoadIdentityMatrix(void) {
    __asm__ __volatile__("lqc2 vf26,0(%0)\n lqc2 vf27,16(%0)\n lqc2 vf28,32(%0)\n vmove vf29,vf0"
                         :
                         : "r"(&vu0IdentityMatrix));
}

/* The argument is unused: the identity is always loaded from vu0IdentityMatrix. */
#define LoadIdentity(m) LoadIdentityMatrix()

/* Stores the current VU0 matrix (vf26-vf29) to m. */
static inline void StoreCurrentMatrix(void *matrix) {
    __asm__ __volatile__("sqc2 vf26,0(%0)\n sqc2 vf27,16(%0)\n sqc2 vf28,32(%0)\n sqc2 vf29,48(%0)"
                         :
                         : "r"(matrix)
                         : "memory");
}

/* Transforms the vector at d by the matrix at m (vf7 through vmulax/vmadday/vmaddz), in place. */
#define TransformVectorByMatrix(d,m) __asm__ __volatile__("lqc2 vf7,0(%0)\n lqc2 vf17,0(%1)\n lqc2 vf18,16(%1)\n lqc2 vf19,32(%1)\n vmulax ACC,vf17,vf7x\n vmadday ACC,vf18,vf7y\n vmaddz vf7,vf19,vf7z\n sqc2 vf7,0(%0)" : : "r"(d),"r"(m) : "memory")

/* The line entry of the table at index (below 40) when it is active, otherwise 0. */
static inline LineEntry *LineEntryAt(int index) {
    return ((unsigned int) index < 40U && (*(LineTable *) &gSefEffectTable).entries[index].active)
               ? &(*(LineTable *) &gSefEffectTable).entries[index]
               : 0;
}

/* The battle matrix when the battle data is present (row block at +0x70 of the battle data), otherwise 0. */
static inline Matrix4Rows *BattleMatrix(void) {
    int *b = sefGetBattleData();
    return b[0] ? (Matrix4Rows *) (b + 28) : 0;
}

/* The line effect at index (400 records), or 0 when the index is out of range. */
static inline LineEffect *LineEffectAt(unsigned int index) {
    return index < 400 ? &(*(LineEffect(*)[]) & gSefEffectPool)[index] : 0;
}

/* Converts a vector to four integers (vftoi0) and stores them at d. */
static inline void ConvertVectorToInts(int *destination, const AlignedVector4 *source) {
    __asm__ __volatile__("lqc2 vf1,0(%5)\n vftoi0 vf1,vf1\n sqc2 vf1,0(%4)"
                         : "=m"(destination[0]), "=m"(destination[1]), "=m"(destination[2]), "=m"(destination[3])
                         : "r"(destination), "r"(source));
}

/*
 * Draws one line effect (the pool record at poolIndex): it binds the resource type of the entry, then
 * builds one packet per trail node with its keys, the angle and the texture coordinates.
 * lastResourceType holds the resource type last loaded (updated when a new one is bound).
 */
void sefLineEffectDraw(unsigned int poolIndex, int *lastResourceType) {
    Matrix4Rows localMatrix;
    IntVectorView offsetVec;
    AlignedVector4 scratchVec;
    float rollAngle;
    float alphaAngle;
    LineRecord *record;
    int perKeyAngles;
    int slot;
    Matrix4Rows *transform;
    int segmentQuads;
    int k;
    int keyIndex;
    LineEffect *line;
    LineEntry *lineEntry;
    LineNode *headNode;
    LineNode *node;
    AlignedVector4 *packetQuads;
    LineSegment *segment;
    void *packet;
    float age;
    float lifeFraction;
    float dropOffset;
    float texU;
    float texV;
    int quadCount;
    int segmentCount;

    line = LineEffectAt(poolIndex);
    if (line == 0) {
        return;
    }
    headNode = line->head;
    if (headNode == 0) {
        return;
    }
    if (svEffectSceneFlag) {
        if (!(line->binding.bits & 0x10000)) {
            return;
        }
    } else if (line->binding.bits & 0x10000) {
        return;
    }
    lineEntry = LineEntryAt(line->binding.kind);
    slot = lineEntry->slot;
    if ((unsigned int) slot < 40U) {
        record = &D_004516B0_1F8E28[slot];
        if (D_004516B0_1F8E28[slot].pos[3] == 0.0f) {
            record = 0;
        }
    } else {
        record = 0;
    }
    perKeyAngles = 1;
    if (line->angles[-1] == 0.0f) {
        perKeyAngles = 0;
    }
    transform = 0;
    node = headNode;
    if (!sefBuildDrawPacket((VuObject *) line, (VuLight *) record, perKeyAngles)) {
        return;
    }
    if (lastResourceType && lineEntry->tag && lineEntry->tag != *lastResourceType && svResourceTypeLists[lineEntry->tag].refs > 0) {
        svLoadImageList(&svResourceTypeLists[lineEntry->tag]);
        svRefMapperPacket();
        *lastResourceType = lineEntry->tag;
    }
    if (line->anim[-1].kind == 256) {
        transform = BattleMatrix();
    }
    if (headNode->angleAKeyIndex == -1) {
        rollAngle = line->angles[0] / 180.0f * 3.14159265f;
    }
    if (headNode->angleBKeyIndex == -1) {
        alphaAngle = line->alphaValues[0];
    }
    segmentQuads = 2;
    if (perKeyAngles) {
        segmentQuads = 6;
    }
    for (; node != 0; node = node->next) {
        if (node->keyCount == 0) {
            continue;
        }
        packetQuads = xglPacketReserve(node->keyCount * (segmentQuads + 2) + 8);
        age = line->duration - node->birthTime;
        lifeFraction = age / line->duration;
        dropOffset = age * ((age - 1.0f) * line->anim[-1].fade) * 0.5f;
        if (packetQuads == 0) {
            break;
        }
        if (lifeFraction < 0.0f) {
            lifeFraction = 0.0f;
        } else if (lifeFraction > 1.0f) {
            lifeFraction = 1.0f;
        }
        if (node->vectorKeyIndex >= 0) {
            func_001F7F58(offsetVec.i, (VectorCurveTrack *) node);
        } else {
            ConvertVectorToInts(offsetVec.i, (AlignedVector4 *) line->offsetVectors);
        }
        if (record->scale.x != 1.0f || record->scale.y != 1.0f || record->scale.z != 1.0f || record->scale.w != 1.0f) {
            offsetVec.i[0] *= record->scale.x;
            offsetVec.i[1] *= record->scale.y;
            offsetVec.i[2] *= record->scale.z;
            offsetVec.i[3] *= record->scale.w;
        }
        if (node->angleAKeyIndex >= 0) {
            func_001F7E38(&rollAngle, (CurveTrack *) node);
            rollAngle = rollAngle / 180.0f * 3.14159265f;
        }
        if (node->angleBKeyIndex >= 0) {
            func_001F7EC8(&alphaAngle, (ShiftedCurveTrack *) node);
        }
        if (line->anim[-1].kind == 59 || line->anim[-1].kind == 60) {
            vu0QuatToMatrix(&localMatrix, node);
            CopyVectorLe(&localMatrix.rows[3], &node->pos);
            transform = &localMatrix;
        }
        segmentCount = 0;
        keyIndex = node->firstKey;
        texU = age * line->texRateU;
        texV = age * line->texRateV;
        texU -= (int) texU;
        texV -= (int) texV;
        if (texU < 0.0f) {
            texU += 1.0f;
        }
        packet = xglPacketGetCurrent();
        segment = (LineSegment *) packetQuads;
        segment->tag[0] = 0x11000000;
        segment->tag[1] = 0;
        segment->tag[2] = 0x1000101;
        segment->alpha = alphaAngle;
        segment->texU = texU;
        segment->texV = texV;
        CopyVectorLe(&segment->node, &node->v20);
        CopyVectorLe(&segment->offsets, (AlignedVector4 *) offsetVec.i);
        quadCount = 4;
        if (!perKeyAngles) {
            LoadIdentity(&vu0IdentityMatrix);
            vu0RotateCurrentZ(rollAngle);
            vu0CurMatrixMulRight(&sefCameraMatrix);
            StoreCurrentMatrix(&segment->transform);
            quadCount = 8;
        }
        segment->tag[3] = ((quadCount - 1) << 16) | 0x6C000011;
        sceVif1PkCnt(packet, 0);
        sceVif1PkRef(packet, packetQuads, quadCount, 0, 0, 0);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddCode(packet, ((unsigned int) D_2048 >> 3) | 0x14000000);
        sceVif1PkAddCode(packet, 0x11000000);
        packetQuads += quadCount;
        ((PacketQuadword *) packetQuads)[0].q = 0;
        ((PacketQuadword *) packetQuads)[1].q = 0;
        quadCount = 2;
        for (k = 0; k < node->keyCount; k++) {
            LineKey *key = &(*(LineTable *) &gSefEffectTable).keys[keyIndex];
            if (perKeyAngles) {
                LoadIdentity(&vu0IdentityMatrix);
                vu0RotateCurrentZ(rollAngle + key->angle);
                vu0CurMatrixMulRight(&sefCameraMatrix);
                StoreCurrentMatrix(&packetQuads[quadCount]);
                quadCount += 4;
            }
            if (line->interpolateKeys) {
                CopyVectorLe(&scratchVec, &key->pos);
                scratchVec.y -= dropOffset;
                MMathLerpKeyPoint(&packetQuads[quadCount], key, &scratchVec, lifeFraction);
            } else {
                CopyVectorLe(&packetQuads[quadCount], (AlignedVector4 *) key);
                packetQuads[quadCount].y -= dropOffset;
            }
            if (transform) {
                TransformVectorByMatrix(&packetQuads[quadCount], transform);
            }
            quadCount++;
            ((PacketQuadword *) &packetQuads[quadCount])->q = 0;
            packetQuads[quadCount].x = key->pos.w;
            quadCount++;
            segmentCount++;
            if (quadCount >= 250) {
                func_001F8D38((SegmentPacket *) packetQuads, quadCount, segmentCount, segmentQuads);
                packetQuads += quadCount;
                ((PacketQuadword *) packetQuads)[0].q = 0;
                ((PacketQuadword *) packetQuads)[1].q = 0;
                quadCount = 2;
                segmentCount = 0;
            }
            keyIndex = (keyIndex + 1) % 5000;
        }
        if (segmentCount) {
            func_001F8D38((SegmentPacket *) packetQuads, quadCount, segmentCount, segmentQuads);
        }
    }
}

typedef struct EffectSlotGroup {
    unsigned char a[0x18];
    short slots[32];
    unsigned char p[6];
    signed char used;
    unsigned char q;
    int flags;
} EffectSlotGroup;

typedef struct EffectObject {
    unsigned char a[0x5E];
    unsigned char pri;
    unsigned char c;
    int k60;
    unsigned char d[12];
} EffectObject;

typedef struct EffectWorkTable {
    unsigned char a[0x47180];
    EffectSlotGroup g[40];
} EffectWorkTable;

typedef struct EffectSortKey {
    int id;
    int key;
} EffectSortKey;

static inline EffectSlotGroup *EffectGroupAt(int i) {
    if ((unsigned int) i < 40 && ((EffectWorkTable *) &gSefEffectTable)->g[i].used != 0) {
        return &((EffectWorkTable *) &gSefEffectTable)->g[i];
    }
    return 0;
}

static inline EffectObject *EffectObjectAt(int i) {
    if ((unsigned int) i < 400) {
        return &(*(EffectObject(*)[]) & gSefEffectPool)[i];
    }
    return 0;
}

/* Draws the effects of group groupIndex: unsorted when its 0xC00 mode is 0, otherwise sorted by object priority (insertion sort of the active slots). */
void sefDrawEffectGroup(int key, void *drawArg, int drawMode) {
    int i;
    int j;
    int count;
    int sortIndex;
    EffectSlotGroup *group;
    EffectSortKey *sortList = (EffectSortKey *) 0x70000000;
    EffectSortKey *sortTail;
    group = EffectGroupAt(key);
    if (group == 0) {
        return;
    }
    if ((group->flags & 0xC00) == 0) {
        for (i = 0; i < 32; i++) {
            sefLineEffectDraw(group->slots[i], drawArg);
        }
    } else if ((group->flags & 0xC00) == 0x400 || ((group->flags & 0xC00) == 0x800 && drawMode == 0)) {
        count = 0;
        for (i = 0; i < 32; i++) {
            EffectObject *object = EffectObjectAt(group->slots[i]);
            if (object != 0 && object->k60 != 0) {
                sortList[count].id = group->slots[i];
                sortList[count].key = object->pri;
                count++;
            }
        }
        sortTail = &sortList[count];
        for (sortIndex = 1; sortIndex < count; sortIndex++) {
            *sortTail = sortList[sortIndex];
            for (j = sortIndex; j > 0 && sortList[j - 1].key > sortTail->key; j--) {
                sortList[j] = sortList[j - 1];
            }
            sortList[j] = *sortTail;
        }
        for (i = 0; i < count; i++) {
            sefLineEffectDraw(sortList[i].id, drawArg);
        }
    }
}

/* Draws the groups of every active entry with flag 0x100 whose tag matches key, passing arg through unused. */
void func_001F98C8(int key, int arg) {
    int i;

    for (i = 0; i < 40; i++) {
        EntryTableItem *entry = EntryAtIndex(i);

        if (entry != 0 && (entry->flags & 0x100) != 0 && entry->key.tag == key) {
            sefDrawEffectGroup(i, &key, arg);
        }
    }
}

typedef struct EffectTableItemRaw {
    unsigned char unmodeled_00[0x5E];
    signed char active;
    unsigned char unmodeled_5F[0x05];
} EffectTableItemRaw;

extern EffectTableItemRaw D_004081F0_1F9998[];

/* The raw table entry at index (below 40) when it is active, otherwise 0. */
EffectTableItemRaw *func_001F9998(unsigned int index) {
    if (index < 0x28U && D_004081F0_1F9998[2912 + index].active != 0) {
        return &D_004081F0_1F9998[2912 + index];
    }
    return 0;
}

typedef struct EffectTableGate {
    unsigned char unmodeled_00[0x47180];
    EntryTableItem entries[40];
    unsigned char unmodeled_48120[0x1392];
    short gate;
} EffectTableGate;

/* The active entry with this id, when the table gate is open; otherwise 0. */
EntryTableItem *sefFindEffectById(int key) {
    int i;

    if ((*(EffectTableGate *) &gSefEffectTable).gate != 0) {
        for (i = 0; i < 40; i++) {
            if ((*(EffectTableGate *) &gSefEffectTable).entries[i].key.id == key &&
                (*(EffectTableGate *) &gSefEffectTable).entries[i].key.active != 0) {
                return &(*(EffectTableGate *) &gSefEffectTable).entries[i];
            }
        }
    }
    return 0;
}

/* The sprite record at index (below 40) whose w component is set, otherwise 0. */
EntryRecord *func_001F9A68(unsigned int index) {
    if (index < 40 && gSefEffectTransforms[index].pos[3] != 0.0f) {
        return &gSefEffectTransforms[index];
    }
    return 0;
}

typedef unsigned int u32;

/* One 128-byte record of the table at gSdvSpecialRecords (the same record sdv_alter.c indexes); inUse is its word at +0x04. */
typedef struct SdvSpecialRecord {
    unsigned char unmodeled_00[0x04];
    unsigned int inUse;
    unsigned char unmodeled_08[0x78];
} SdvSpecialRecord;

extern SdvSpecialRecord gSdvSpecialRecords[];

SdvSpecialRecord *sdvGetSpecialRecord(u32 specialRecordIndex) {
    if (!(specialRecordIndex < 0x20U)) {
        return 0;
    }
    return &gSdvSpecialRecords[specialRecordIndex];
}

VuUploadBlock D_00408180 = {
    { 0x1000000000008005ULL, 14, 0, 6, 0, 8, 0x5000DULL, 71, 0, 66, 0, 76, 0, 0 }
};
