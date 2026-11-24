#include "common.h"
#include "shared.h"

int scGetImmAdrImmIdx2(int scriptBase, unsigned short *immediateTable, int immediateIndex);
short scGetImmNumIdx(short *immediateTable, int immediateIndex);
int scOfsToAdr(int encodedOffset);

#define SCRIPT_ADDRESS_NONE 0xFFFF    /* absent table entry or address */

typedef int s32;

typedef struct ScRequest {
    s32 key; /* +0x00 */
    unsigned char unmodeled_04[0x28];
    s32 effect; /* +0x2C */
    unsigned char unmodeled_30[0x50];
} ScRequest;

static inline void QuadwordCopyT0(AlignedVector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8", "memory");
}

static inline void Vu0SefScaleVectorXYZ(AlignedVector4 *destination, AlignedVector4 *left, f32 scale) {
    __asm__ __volatile__(
        "lqc2 vf1,0(%3)\n\tmfc1 $8,%4\n\tqmtc2 $8,vf2\n\tvmulx.xyz vf1,vf1,vf2x\n\tsqc2 vf1,0(%5)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
        : "r"(left), "f"(scale), "r"(destination)
        : "$8", "memory");
}

static inline void Vu0SefAddVectorXYZ(AlignedVector4 *destination, AlignedVector4 *left, Vector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%3)\n\tlqc2 vf2,0(%4)\n\tvadd.xyz vf1,vf1,vf2\n\tsqc2 vf1,0(%5)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(left), "r"(right), "r"(destination)
                         : "memory");
}

typedef struct ScEffectSource ScEffectSource;

typedef struct ScEffectObject {
    unsigned int flags;
    unsigned char unmodeled_04[0x04];
    s32 optionWord;
    unsigned char unmodeled_0C[0x04];
    unsigned int stateFlags;
    unsigned char unmodeled_14[0x10];
    ScEffectSource *source;
    unsigned char unmodeled_28[0x11E];
    s16 index;
    unsigned char unmodeled_148[0x24];
    f32 time;
} ScEffectObject;

typedef struct ScEffectSource {
    unsigned char unmodeled_00[0x20];
    unsigned char probe[0x190];
    Vector4 offsetA;
    Vector4 offsetB;
    unsigned char unmodeled_1D0[0x40];
    Vector4 vector210;
    unsigned char unmodeled_220[0x2CC];
    s32 flags4EC;
} ScEffectSource;

typedef struct ScSpawnedEffect {
    unsigned char unmodeled_00[0x14];
    f32 height;
    unsigned char unmodeled_18[0x28];
    DoublewordAlignedVector4 position;
} ScSpawnedEffect;

extern s32 scSoundCueTable[2];
extern s32 scEffectSpawnCounter;
s32 mapStaticObjectProbe(void *position, f32 *distance);
void sefObjectSetStatePlay(void *effectObject);
ScSpawnedEffect *sefSpawnEffectFromRequest(ScRequest *request);
s32 wordTableGet(unsigned int wordIndex);
s32 XenoActorMulVector(s32 actor, s32 value, void *out, s32 count);
s32 soundCueSeCommand(s32, s32, s32, s32);

/* A float and its word bits: the store through this union decides the emitted code. */
typedef union FloatWordUnion {
    f32 asFloat;
    s32 asWord;
} FloatWordUnion;

/* Updates one effect object: probes its source, then spawns its effect or plays a sound cue. The state bits (1, 2, 4)
 * and the object flags (0x100, 0x2000, 0x8000) pick the branch. */
void scEffectObjectUpdate(ScEffectObject *effectObject) {
    ScEffectSource *effectSource;
    ScRequest spawnRequest;
    AlignedVector4 actorPosition;
    AlignedVector4 spawnOffset;
    ScSpawnedEffect *spawnedEffect;
    f32 probeDistance;
    s32 probeResult;
    s32 spawnMode;
    s32 spawnCueId;
    s32 objectIndex;
    s32 stateFlags;
    spawnCueId = 0;
    spawnMode = -1;
    probeDistance = 0.0f;
    if (effectObject == 0) {
        return;
    }
    effectSource = effectObject->source;
    stateFlags = effectObject->stateFlags;
    objectIndex = effectObject->index;
    if (effectSource == 0) {
        return;
    }
    probeResult = mapStaticObjectProbe(effectSource->probe, &probeDistance);
    if (probeResult >= 77) {
        probeDistance = 0.0f;
    } else if (probeResult < 71) {
        probeDistance = 0.0f;
    }
    if (effectObject->flags & 0x100) {
        if (stateFlags & 1) {
            scEffectSpawnCounter = (scEffectSpawnCounter + 1) & 3;
            if (scEffectSpawnCounter == 0 && effectObject->time != 0.0f) {
                spawnCueId = scSoundCueTable[1];
                goto spawnEffect;
            }
        } else if ((stateFlags & 2) == 0) {
            if ((stateFlags & 4) == 0) {
                goto spawnEffect;
            }
            if (effectSource->flags4EC & 0x20000) {
                spawnMode = 1;
            } else if (effectSource->flags4EC & 0x40000) {
                spawnMode = 0;
            }
            if (spawnMode != -1) {
                goto playCues;
            }
            return;
        }
    } else {
        if (effectSource->flags4EC & 0x20000) {
            spawnMode = 1;
        } else if (effectSource->flags4EC & 0x40000) {
            spawnMode = 0;
        }
        if (spawnMode == -1) {
            if (stateFlags & 1) {
                soundCueSeCommand(0, -2, probeResult, 0);
            }
            return;
        }
        spawnCueId = scSoundCueTable[2 + spawnMode];
    spawnEffect:
        sefObjectSetStatePlay(&spawnRequest);
        spawnRequest.key = (s32) effectSource;
        spawnRequest.effect = 0;
        if ((spawnRequest.effect = wordTableGet(probeResult)) != 0 &&
            XenoActorMulVector((s32) effectSource, spawnCueId, &actorPosition, 1) != 0) {
            spawnedEffect = sefSpawnEffectFromRequest(&spawnRequest);
            if (spawnedEffect != 0) {
                QuadwordCopyT0(&spawnOffset, &effectSource->vector210);
                Vu0SefScaleVectorXYZ(&spawnOffset, &spawnOffset, 1.8f);
                Vu0SefAddVectorXYZ(&spawnOffset, &spawnOffset, &effectSource->offsetA);
                Vu0SefAddVectorXYZ(&spawnOffset, &spawnOffset, &effectSource->offsetB);
                Vu0SefScaleVectorXYZ(&spawnOffset, &spawnOffset, 0.5f);
                spawnOffset.w = 1.0f;
                spawnedEffect->position = spawnOffset;
                if (probeDistance != 0.0f) {
                    ((FloatWordUnion *) &spawnedEffect->height)->asFloat = probeDistance - actorPosition.y;
                }
            }
        }
    }
playCues:
    if (effectObject->flags & 0x100) {
        if (stateFlags & 1) {
            if ((effectObject->flags & 0x2000) == 0) {
                if (effectObject->optionWord & 0x40000) {
                    soundCueSeCommand(0, 4, probeResult, 0);
                } else if (effectObject->optionWord & 0x80000) {
                    soundCueSeCommand(0, 5, probeResult, 0);
                } else {
                    soundCueSeCommand(0, -2, probeResult, 0);
                }
            }
        } else if (stateFlags & 2) {
            if (effectObject->optionWord & 0x40000) {
                soundCueSeCommand(objectIndex + 1, 4, probeResult, (s32) effectSource->probe);
            } else if (effectObject->optionWord & 0x80000) {
                soundCueSeCommand(objectIndex + 1, 5, probeResult, (s32) effectSource->probe);
            }
        } else if (stateFlags & 4) {
            if (objectIndex >= 0) {
                soundCueSeCommand(objectIndex + 1, 0, probeResult, (s32) effectSource->probe);
            }
        }
    } else {
        if ((effectObject->flags & 0x2000) == 0) {
            if (effectObject->flags & 0x8000) {
                spawnMode |= 2;
            }
            if (stateFlags & 1) {
                soundCueSeCommand(0, scSoundCueTable[4 + spawnMode], probeResult, 0);
            } else if (stateFlags & 6) {
                if (objectIndex >= 0) {
                    soundCueSeCommand(objectIndex + 1, 0, probeResult, (s32) effectSource->probe);
                }
            }
        }
    }
    /* kept: this empty label closes the function body and fixes the emitted block order */
end:;
}

/* Issues sound command -3 with zero arguments. */
void scQueueDefaultRequest(void) {
    soundCueSeCommand(0, -3, 0, 0);
}

extern s32 scSampleEnabled;

/* Sets bit 0 of the request flags (scSampleEnabled). */
void scSampleEnableSet(void) {
    s32 *requestFlags = &scSampleEnabled;

    *requestFlags |= 1;
}

/* Clears bit 0 of the request flags (scSampleEnabled). */
void scSampleEnableClear(void) {
    s32 *requestFlags = &scSampleEnabled;

    *requestFlags &= ~1;
}

extern s32 scCrossingEffectMode;

typedef struct ScRangePoint {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} __attribute__((aligned(8))) ScRangePoint;

typedef struct ScKeyRecord {
    ScRangePoint rangePoints[2]; /* +0x00: the y of both vectors is set to 10000.0f by scKeyRangeReset */
    s32 key;          /* +0x20 */
    u8 unmodeled_24[0x0C];
} ScKeyRecord;

extern ScKeyRecord scKeyRecords[18];
void *memset(void *, int, unsigned int);

/* Clears the 18-entry key table (scKeyRecords), sets every range y to 10000.0f, stores mode in scCrossingEffectMode and
 * clears bit 0 of the request flags. */
void scKeyTableReset(s32 mode) {
    s32 keyIndex;
    s32 rangeIndex;
    s32 *requestFlags;

    memset(scKeyRecords, 0, sizeof scKeyRecords);
    for (keyIndex = 0; keyIndex < 18; keyIndex++) {
        for (rangeIndex = 0; rangeIndex < 2; rangeIndex++) {
            scKeyRecords[keyIndex].rangePoints[rangeIndex].y = 10000.0f;
        }
    }
    scCrossingEffectMode = mode;
    requestFlags = &scSampleEnabled;
    *requestFlags &= ~1;
}

/* Resets the range y values of the key table entry for key to 10000.0f. */
void scKeyRangeReset(s32 key) {
    s32 keyIndex;
    s32 rangeIndex;

    for (keyIndex = 0; keyIndex < 18; keyIndex++) {
        if (key == scKeyRecords[keyIndex].key) {
            for (rangeIndex = 0; rangeIndex < 2; rangeIndex++) {
                scKeyRecords[keyIndex].rangePoints[rangeIndex].y = 10000.0f;
            }
            return;
        }
    }
}

/* Returns the key table entry for key. Otherwise claims the last free entry for it, or returns 0 when the table is full. */
ScKeyRecord *scKeyRecordGet(s32 key) {
    s32 freeIndex;
    s32 keyIndex;

    freeIndex = -1;
    for (keyIndex = 0; keyIndex < 18; keyIndex++) {
        if (key == scKeyRecords[keyIndex].key) {
            return &scKeyRecords[keyIndex];
        }
        if (scKeyRecords[keyIndex].key == 0) {
            freeIndex = keyIndex;
        }
    }
    if (freeIndex >= 0) {
        scKeyRecords[freeIndex].key = key;
        return &scKeyRecords[freeIndex];
    }
    return 0;
}

s32 sefReadProbePoint(s32, s32, AlignedVector4 *, s32);
void sefObjectSetStateIdle(ScRequest *);

static inline void scSubXYZ(AlignedVector4 *destination, AlignedVector4 *left, ScRangePoint *right) {
    __asm__ __volatile__("lqc2 vf1,0(%3)\n\tlqc2 vf2,0(%4)\n\tvsub.xyz vf1,vf1,vf2\n\tsqc2 vf1,0(%5)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(left), "r"(right), "r"(destination)
                         : "memory");
}

/* Samples key's target position into its two range points. In mode 0 or 3 a crossing of a sampled plane spawns
 * the effect of that mode (0x13601F or 0x136020). */
void scKeyRangeSample(s32 key) {
    ScRequest spawnRequest;
    AlignedVector4 targetPosition;
    AlignedVector4 rangeOffset;
    ScRangePoint *rangePoint;
    ScKeyRecord *keyRecord;
    s32 crossingFlags;
    s32 effectId;
    s32 rangeIndex;
    f32 horizontalDistance;
    f32 rangeY;
    if (key != 0 && (scSampleEnabled & 1)) {
        switch (scCrossingEffectMode) {
        case 0:
            effectId = 0x13601F;
            break;
        case 3:
            effectId = 0x136020;
            break;
        default:
            effectId = 0;
            break;
        }
        if (effectId != 0) {
            sefObjectSetStateIdle(&spawnRequest);
            spawnRequest.key = key;
            spawnRequest.effect = 0;
            crossingFlags = 0;
            keyRecord = scKeyRecordGet(key);
            rangePoint = keyRecord->rangePoints;
            for (rangeIndex = 0; rangeIndex < 2; rangePoint++, rangeIndex++) {
                spawnRequest.effect = effectId;
                if (sefReadProbePoint(key, scSoundCueTable[rangeIndex], &targetPosition, 1) != 0) {
                    rangeY = 0.0f;
                    scSubXYZ(&rangeOffset, &targetPosition, rangePoint);
                    rangeOffset.y = rangeY;
                    horizontalDistance = __builtin_sqrtf(rangeOffset.x * rangeOffset.x + rangeOffset.z * rangeOffset.z);
                    rangeY = rangePoint->y;
                    if (rangeY != 10000.0f) {
                        if (scCrossingEffectMode == 0) {
                            if ((targetPosition.y <= 0.0f && rangeY > 0.0f) ||
                                (targetPosition.y <= 0.0f && rangeY <= 0.0f && horizontalDistance > 0.5f)) {
                                crossingFlags |= 1;
                            }
                        }
                        if (scCrossingEffectMode == 3) {
                            if ((targetPosition.y > 0.0f && rangeY <= 0.0f) || (targetPosition.y <= 0.0f && rangeY > 0.0f) ||
                                (targetPosition.y <= 0.0f && rangeY <= 0.0f && horizontalDistance > 0.5f)) {
                                crossingFlags |= 1;
                            }
                        }
                    }
                    *rangePoint = *(ScRangePoint *) &targetPosition;
                }
            }
            if (crossingFlags != 0) {
                sefSpawnEffectFromRequest(&spawnRequest);
            }
        }
    }
}

extern int gSefExecSteps;
void sefRunEffectRecords(int arg);

/* Advances the SEF step counter and then calls sefRunEffectRecords(0). */
void sefAdvanceStepCounter(void)
{
    gSefExecSteps += 1;
    sefRunEffectRecords(0);
}

/* Returns the address of the halfword offset at immediateTable[immediateIndex], relative to scriptBase. The absent
 * marker (SCRIPT_ADDRESS_NONE or -1) and a zero offset give 0. */
int scGetImmAdrImmIdx2(int scriptBase, unsigned short *immediateTable, int immediateIndex)
{
    int halfwordOffset;
    int address;

    halfwordOffset = immediateTable[immediateIndex];
    if (halfwordOffset == SCRIPT_ADDRESS_NONE || halfwordOffset == -1) {
        halfwordOffset = 0;
    }
    address = 0;
    if (halfwordOffset != 0) {
        address = scriptBase + halfwordOffset * 2;
    }
    return address;
}

/* Returns the immediate number stored at immediateTable[immediateIndex]. */
short scGetImmNumIdx(short *immediateTable, int immediateIndex)
{
    return immediateTable[immediateIndex];
}

typedef struct ScScript {
    u8 unknown_00[0x10];
    s16 scriptKind;
    u16 scriptWords[8];
} ScScript;

extern s32 sefOutputLinkEnabled;
extern s32 sefOwnerGroupScale;
extern s32 sefSpawnMode;
s32 svFindEffectMapperIndex(s32);
s32 srsNormalizeEffectKey(s32);
ScScript **svFindEffectEntryByStem(s32, s32, s32);
s32 srsGetEffectSlot(s32);

/* Reads one script u16 into DEST; the absent marker (0xFFFF or -1) reads as 0. The caller declares scriptWord. */
#define SC_READ_INDEX(DEST, SRC) { s32 entryIndex; scriptWord = *(SRC); entryIndex = scriptWord; if (scriptWord == 0xffff || scriptWord == -1) entryIndex = 0; DEST = entryIndex; }

/* Looks up effect key effectKey and returns its script, or 0. Writes the resolved index to *outValue (0 when the kind
 * sets none). sefSpawnMode gets the script kind; sefOutputLinkEnabled gets the link address for kinds 0x40 and 0x49. */
ScScript *scScriptLookup(s32 effectKey, s32 *outValue, s32 selector, s32 entryVariant) {
    s32 mapperIndex = svFindEffectMapperIndex(effectKey);
    ScScript **effectEntry;
    ScScript *script;
    s32 resultIndex;
    s32 scriptWord;
    *outValue = 0;
    sefOutputLinkEnabled = 0;
    sefSpawnMode = 0;
    if (mapperIndex < 0) {
        return 0;
    }
    effectEntry = svFindEffectEntryByStem(mapperIndex, srsNormalizeEffectKey(effectKey), entryVariant);
    if (effectEntry != 0) {
        script = *effectEntry;
        if (script != 0) {
            resultIndex = 0;
            sefSpawnMode = script->scriptKind;
            if ((u16) (script->scriptKind - 0x3c) < 2) {
                SC_READ_INDEX(resultIndex, &script->scriptWords[0])
            } else if (script->scriptKind == 0x3e || script->scriptKind == 0x41) {
                s32 ownerGroupWord = (s16) script->scriptWords[0];
                if ((ownerGroupWord & 0x4000) == 0) {
                    sefOwnerGroupScale = ownerGroupWord & 0x3fff;
                } else {
                    sefOwnerGroupScale = ownerGroupWord;
                }
                SC_READ_INDEX(resultIndex, &script->scriptWords[selector + 1])
            } else if (script->scriptKind == 0x3f) {
                s32 slotIndex = srsGetEffectSlot(effectKey);
                if (slotIndex >= 0 && slotIndex < (s16) script->scriptWords[0]) {
                    SC_READ_INDEX(resultIndex, &script->scriptWords[slotIndex + 1])
                }
            } else if (script->scriptKind == 0x49) {
                sefOutputLinkEnabled = (s32) &script->scriptWords[0];
                return script;
            } else if (script->scriptKind == 0x40) {
                SC_READ_INDEX(*outValue, &script->scriptWords[0])
                SC_READ_INDEX(resultIndex, &script->scriptWords[1])
                if (resultIndex != 0) {
                    sefOutputLinkEnabled = (s32) script + resultIndex * 2;
                }
                return script;
            } else if (script->scriptKind == 0x42) {
                s32 ownerGroupWord = (s16) script->scriptWords[0];
                if ((ownerGroupWord & 0x4000) == 0) {
                    sefOwnerGroupScale = ownerGroupWord & 0x3fff;
                } else {
                    sefOwnerGroupScale = ownerGroupWord;
                }
                SC_READ_INDEX(*outValue, &script->scriptWords[selector + 1])
                SC_READ_INDEX(resultIndex, &script->scriptWords[3])
                if (resultIndex != 0) {
                    sefOutputLinkEnabled = (s32) script + resultIndex * 2;
                }
                return script;
            }
            if (resultIndex == 0) {
                return 0;
            }
            *outValue = resultIndex;
            return script;
        }
    }
    return 0;
}

/* Returns encodedOffset, with the absent marker (SCRIPT_ADDRESS_NONE or -1) mapped to 0. */
int scOfsToAdr(int encodedOffset)
{
    if (encodedOffset == SCRIPT_ADDRESS_NONE || encodedOffset == -1) {
        encodedOffset = 0;
    }
    return encodedOffset;
}
