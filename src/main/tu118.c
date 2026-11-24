#include "common.h"
#include "main/tu118.h"
#include "main/sv_get_image_item_142.h"
#include "quadword.h"

typedef int s32;

/* One effect record of sefEffectRecords, 0x150 bytes each (sef_135.h has the full layout); only the fields below are read here. */
typedef struct SefEffectRecord {
    unsigned char unmodeled_00[0x04];
    s32 noReleaseFlags;                    /* +0x04: 0x04000000 marks a record that is not released here */
    s16 resourceType;               /* +0x08: index into svResourceTypeLists */
    unsigned char unmodeled_0A[0x50 - 0x0A];
    s32 inUse;                      /* +0x50: zero for a free record */
    unsigned char unmodeled_54[0x58 - 0x54];
    s32 groupKey;                   /* +0x58: group key compared with the key argument of sefRunGroupEffectFrames */
    unsigned char unmodeled_5C[0x70 - 0x5C];
    s32 ownerKey;                   /* +0x70: owner id the table at sefListedOwnerKeys lists */
    unsigned char unmodeled_74[0x150 - 0x74];
} SefEffectRecord;

extern s32 sefEffectRecordCount;

extern SefEffectRecord *sefEffectRecords;
s32 sefReleaseEffectRecord(s32);

/* Calls sefReleaseEffectRecord for every in-use effect record whose flags04 0x04000000 bit is clear. */
void sefReleaseUnflaggedRecords(void) {
    s32 i;

    for (i = 0; i < sefEffectRecordCount; i++) {
        if (sefEffectRecords[i].inUse != 0 &&
            (sefEffectRecords[i].noReleaseFlags & 0x04000000) == 0) {
            sefReleaseEffectRecord(i);
        }
    }
}

/* sefListedOwnerKeys: owner keys, -1 terminated. */
extern s32 sefListedOwnerKeys[];

/* Calls sefReleaseEffectRecord for every in-use effect record whose ownerKey is not in the sefListedOwnerKeys list. */
void sefReleaseUnlistedOwnerRecords(void) {
    s32 i;
    s32 ownerIndex;
    s32 isUnlisted;

    for (i = 0; i < sefEffectRecordCount; i++) {
        if (sefEffectRecords[i].inUse != 0) {
            isUnlisted = 1;
            for (ownerIndex = 0; sefListedOwnerKeys[ownerIndex] != -1; ownerIndex++) {
                if (sefEffectRecords[i].ownerKey == sefListedOwnerKeys[ownerIndex]) {
                    isUnlisted = 0;
                }
            }
            if (isUnlisted == 1) {
                sefReleaseEffectRecord(i);
            }
        }
    }
}

extern SvTypeList svResourceTypeLists[];
void *func_001F9998(s32 index);
s32 sefEffectInstanceRelease();
s32 sefReleaseSlotsByKey();

/* One actor of the table func_001F9998 indexes. */
typedef struct SefGroupActor {
    unsigned char unmodeled_00[0x04];
    s32 groupKey;                   /* +0x04: group key compared with the key argument of sefRunGroupEffectFrames */
    unsigned char unmodeled_08[0x14 - 0x08];
    s32 ownerKey;                   /* +0x14: owner id compared with the group key */
    unsigned char unmodeled_18[0x5C - 0x18];
    s16 actorId;                    /* +0x5C: id func_001F9A68 maps to a spawn output */
    signed char unmodeled_5E;
    signed char resourceType;       /* +0x5F: index into svResourceTypeLists */
} SefGroupActor;

extern s32 D_00551050;
extern s32 D_00551058;

/*
 * Releases the in-use effect records and the actors of pendingKey's resource type.
 * A negative pendingKey matches every record and actor.
 */
void sefReleaseByPendingCount(s32 pendingKey) {
    s32 i;

    for (i = 0; i < sefEffectRecordCount; i++) {
        if (sefEffectRecords[i].inUse != 0) {
            if (pendingKey < 0 ||
                svResourceTypeLists[sefEffectRecords[i].resourceType]
                        .pendingCount == pendingKey) {
                sefReleaseSlotsByKey(i, 1);
                sefReleaseEffectRecord(i);
            }
        }
    }
    for (i = 0; i < 40; i++) {
        SefGroupActor *actor = func_001F9998(i);
        if (actor != 0) {
            if (pendingKey < 0 || svResourceTypeLists[actor->resourceType].pendingCount == pendingKey) {
                sefEffectInstanceRelease(actor);
            }
        }
    }
}

/*
 * Releases the effect records and actors whose owner is ownerKey. A negative
 * ownerKey matches all of them; a key with the bit pattern tested below goes
 * to sefReleaseByPendingCount instead.
 */
void sefReleaseOwnerEffects(s32 ownerKey) {
    s32 i;

    if (ownerKey < 0) {
        D_00551050 = 0;
        D_00551058 = 0;
    }
    if ((ownerKey > 0 && ((ownerKey >> 28) & 3) == 3) || (ownerKey & 0x3ff) == 0) {
        sefReleaseByPendingCount(ownerKey);
        return;
    }
    for (i = 0; i < sefEffectRecordCount; i++) {
        if (sefEffectRecords[i].inUse != 0 &&
            (ownerKey < 0 || sefEffectRecords[i].ownerKey == ownerKey)) {
            sefReleaseSlotsByKey(i, 1);
            sefReleaseEffectRecord(i);
        }
    }
    for (i = 0; i < 40; i++) {
        SefGroupActor *actor = func_001F9998(i);
        if (actor != 0 && (actor->ownerKey == ownerKey || ownerKey < 0)) {
            sefEffectInstanceRelease(actor);
        }
    }
}

/* The object an effect handle names: kind 0 marks an effect object, any other value an actor. */
typedef struct SefEffectObject {
    s32 handleClass;                /* +0x00 */
    unsigned char unmodeled_04[0x24 - 0x04];
    s32 playState;                  /* +0x24: 0, 1 or 2 (set by the sefObjectSetState functions below) */
    unsigned char unmodeled_28[0x30 - 0x28];
    f32 resetFloat30;               /* +0x30: 1.0f after sefEffectObjectReset */
    s32 resetIndex34;               /* +0x34: -1 after sefEffectObjectReset */
    unsigned char unmodeled_38[0x3A - 0x38];
    s16 resetShort3A;               /* +0x3A: zero after sefEffectObjectReset */
    s16 resetShort3C;               /* +0x3C: zero after sefEffectObjectReset */
    unsigned char unmodeled_3E[0x40 - 0x3E];
    Vector4 resetVector40;          /* +0x40: zero vector */
    union {
        Vector4 resetVector50;      /* +0x50: zero vector */
        s32 resetVector50Word;                 /* +0x50: first word, tested as a flag by func_001EA050 */
    };
    Vector4 resetVector60;          /* +0x60: (1, 1, 1, 1) */
    Vector4 resetVector70;          /* +0x70: (1, 1, 1, 1) */
    unsigned char unmodeled_80[0x13E - 0x80];
    s16 effectIndex;                /* +0x13E: index into sefEffectRecords */
    unsigned char unmodeled_140[0x14A - 0x140];
    u8 flag14A;                     /* +0x14A */
} SefEffectObject;

/* Releases the effect or actor a handle names: an effect object (handleClass 0) frees its record when in use. */
void sefEffectHandleRelease(SefEffectObject *effectObject)
{
    s32 effectIndex;

    if (effectObject != 0) {
        if (effectObject->handleClass == 0) {
            effectIndex = effectObject->effectIndex;
            if (sefEffectRecords[effectIndex].inUse != 0) {
                sefReleaseSlotsByKey(effectIndex, 1);
                sefReleaseEffectRecord(effectIndex);
            }
        } else {
            sefEffectInstanceRelease(effectObject);
        }
    }
}

extern SefEntryTable sefEffectEntryTable;
int sefEffectFlagTest(int mask);
void sefRunEffectStep(void);
void sefInstallDrawCallbacks(void);

/* Per-frame effect update; gGameLoopRuntimeStorage flag 0x400 resets the draw state instead. */
void sefFrameUpdate(void)
{
    if (gGameLoopRuntimeStorage.flags & 0x400) {
        sefResetDrawState();
        return;
    }
    if (sefEffectFlagTest(1) == 0) {
        sefRunEffectStep();
    }
    sefInstallDrawCallbacks();
}

extern s32 sefKeyframeStepBlock[];
void sefClearEffectTallies(void);
void sefRunEffectRecords(SefEffectObject *effectObject);
void sdvExecOwnerAlters(SefEffectObject *effectObject);
void *sefFindEffectById(s32 index);
void sefEffectGroupStep(void *target);
void sefRunEffectFrames(SefEffectObject *effectObject, f32 frames, f32 frameStep);

/*
 * Runs the effect object for `frames` frames, `frameStep` at a time (with no
 * object, the whole effect system). sefKeyframeStepBlock[0] holds the step in
 * thousandths for the length of the run and is restored afterwards.
 */
void sefRunEffectFrames(SefEffectObject *effectObject, f32 frames, f32 frameStep) {
    f32 elapsed;
    s32 savedStepBlock;

    if (frameStep <= 0.0f) {
        return;
    }
    savedStepBlock = sefKeyframeStepBlock[0];
    sefKeyframeStepBlock[0] = (s32) ((frameStep + 0.0001f) * 1000.0f);
    elapsed = 0.0f;
    if (effectObject == 0) {
        while (elapsed < frames) {
            elapsed += frameStep;
            sefRunEffectStep();
        }
    } else {
        sefClearEffectTallies();
        if (effectObject->handleClass == 0) {
            elapsed = 0.0f;
            while (elapsed < frames) {
                elapsed += frameStep;
                sefRunEffectRecords(effectObject);
                sdvExecOwnerAlters(effectObject);
                sefEffectGroupStep(sefFindEffectById(effectObject->effectIndex));
            }
        } else {
            for (elapsed = 0.0f; elapsed < frames; elapsed += frameStep) {
                sefEffectGroupStep(effectObject);
            }
        }
    }
    sefKeyframeStepBlock[0] = savedStepBlock;
}

/*
 * Runs the records and actors of groupKey for `frames` frames, `frameStep` at a
 * time; a negative ownerKey matches every owner.
 */
void sefRunGroupEffectFrames(s32 groupKey, s32 ownerKey, f32 frames, f32 frameStep) {
    s32 i;
    SefGroupActor *actor;

    if (groupKey != 0) {
        for (i = 0; i < sefEffectRecordCount; i++) {
            if (sefEffectRecords[i].inUse != 0 &&
                sefEffectRecords[i].groupKey == groupKey) {
                if (sefEffectRecords[i].ownerKey == ownerKey || ownerKey < 0) {
                    sefRunEffectFrames((SefEffectObject *) &sefEffectRecords[i], frames, frameStep);
                }
            }
        }
        for (i = 0; i < 40; i++) {
            actor = func_001F9998(i);
            if (actor != 0 && actor->groupKey == groupKey) {
                if (actor->ownerKey == ownerKey || ownerKey < 0) {
                    sefRunEffectFrames((SefEffectObject *) actor, frames, frameStep);
                }
            }
        }
    }
}

void sefEffectStartScaled(SefEffectObject *effectObject, f32 effectScale) {
    sefRunEffectFrames(effectObject, effectScale, 1.0f);
}

static inline void effectVectorZero(Vector4 *vector) {
    __asm__ __volatile__("sqc2 $vf0, 0(%0)" : : "r"(vector) : "memory");
}

static inline void effectVectorSetOne(Vector4 *vector) {
    __asm__ __volatile__("vmaxw.xyzw $vf1, $vf0, $vf0w\n sqc2 $vf1, 0(%0)" : : "r"(vector) : "memory");
}

void *memset();

/*
 * Resets an effect object: clears its first 0x80 bytes, then writes the reset
 * values of SefEffectObject (float 1.0 and index -1, zero shorts and vectors,
 * and (1, 1, 1, 1) in the two last vectors).
 */
void sefEffectObjectReset(SefEffectObject *effectObject) {
    memset(effectObject, 0, 0x80);
    effectObject->resetFloat30 = 1.0f;
    effectObject->resetIndex34 = -1;
    effectObject->resetShort3A = 0;
    effectObject->resetShort3C = 0;
    effectVectorZero(&effectObject->resetVector40);
    effectVectorZero(&effectObject->resetVector50);
    effectVectorSetOne(&effectObject->resetVector60);
    effectVectorSetOne(&effectObject->resetVector70);
}

void sefObjectSetStateIdle(SefEffectObject *effectObject) {
    sefEffectObjectReset(effectObject);
    effectObject->playState = 0;
}

void sefObjectSetStatePlay(SefEffectObject *effectObject) {
    sefEffectObjectReset(effectObject);
    effectObject->playState = 1;
}

void sefEffectSetStop(SefEffectObject *effectObject) {
    sefEffectObjectReset(effectObject);
    effectObject->playState = 2;
}

extern s32 sefCountOwnerGroupRecords();
extern void sefClearOwnerGroupFlags();
extern void sefRegisterGroupCenter();
extern s32 countFlaggedSefEntries();
extern s32 func_001F6590();
extern s32 sefScheduleEffectRecord();
extern s32 func_0020F790();
extern void svSceneFlagsSet();

typedef long long s64;

/* The key block sefCreateScheduler reads; sefRequestKeyBlock keeps the last key stored with earlyMode 0. */
typedef struct SefSpawnKey {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
    s32 unk_14;
    s32 *groupId;                   /* +0x18: the group id, stored as a pointer (the scheduler reads it so) */
    f32 scale;                      /* +0x1C */
    u16 unk_20;
    u16 unk_22;
    s32 unk_24;
    s32 unk_28;
    s32 unk_2C;
} SefSpawnKey;

/* One spawn request (the argument of sefSpawnEffectFromRequest); the unk_ words keep their offsets. */
typedef struct SefSpawnRequest {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
    s32 unk_14;
    s32 unk_18;
    s32 unk_1C;
    s32 unk_20;
    s32 earlyMode;                  /* +0x24: nonzero takes the vector copy path */
    s32 groupLookupKey;             /* +0x28: when nonzero the group is func_0020F790(groupLookupKey) */
    s32 groupId;                    /* +0x2C */
    f32 scale;                      /* +0x30 */
    s32 schedulerArg;               /* +0x34 */
    u16 spawnCounter;               /* +0x38: copied into the scheduler entry */
    u16 unk_3A;
    u16 unk_3C;
    u16 unk_3E;
    AlignedVector4 vector40;        /* +0x40 */
    AlignedVector4 vector50;        /* +0x50 */
    AlignedVector4 vector60;        /* +0x60 */
    AlignedVector4 vector70;        /* +0x70 */
} SefSpawnRequest;

/* One scheduler entry (0x150 bytes) of the table sefGetScheduler returns. */
typedef struct SefSchedulerEntry {
    s32 unk_00;
    u32 schedFlags;                 /* +0x04 */
    s16 refId;                      /* +0x08: index into sefReferenceCounts for sefIncrementRef */
    s16 unk_0A;
    s32 unk_0C;
    AlignedVector4 vector10;
    AlignedVector4 vector20;
    AlignedVector4 vector30;
    AlignedVector4 vector40;
    char unk_50[0xFC - 0x50];
    s16 subSlotBusy[32];               /* +0xFC: a value of 0 or more marks a busy sub-slot */
    s16 unk_13C;
    s16 unk_13E;
    u16 spawnCounter;               /* +0x140 */
    char unk_142[0x150 - 0x142];
} SefSchedulerEntry;

/* One output record of func_001F9A68 (0x3C bytes). */
typedef struct SefSpawnOutput {
    AlignedVector4 vector00;
    AlignedVector4 vector10;
    AlignedVector4 vector20;
    s32 unk_30;
    s32 unk_34;
    s32 groupId;                    /* +0x38 */
} SefSpawnOutput;

/* A reference count per entry of sefReferenceCounts. */
typedef struct SefReferenceCount {
    char unk_00[8];
    u16 referenceCount;                       /* +0x08 */
    char unk_0A[0x13DC - 10];
} SefReferenceCount;

/* An effect slot of the table sefGetEffectSlot returns: flags at +0x64. */
typedef struct SefEffectSlot {
    char unk_00[0x64];
    u32 effectFlags;                /* +0x64 */
} SefEffectSlot;

/* The studio block (gSbStudioBlock): flags at +0x10. */
typedef struct SbStudioBlock {
    char unk_00[0x10];
    union {
        s64 studioFlags;
        u64 studioFlagsUnsigned;
    };
} SbStudioBlock;

extern SbStudioBlock gSbStudioBlock;
extern s32 sefSpawnMode;
extern s32 sefOwnerGroupScale;
extern s32 sefOutputLinkEnabled;
extern u16 gSefExecSteps;
extern SefReferenceCount sefReferenceCounts[];
extern SefSpawnKey sefRequestKeyBlock;
SefSchedulerEntry *sefGetScheduler(void);
s16 *scScriptLookup(s32, s32 *, s32, s32 *);
s32 sefCreateScheduler(s16 *, SefSpawnKey *, s32, s16 *, s32);
SefSpawnOutput *func_001F9A68(s32);
SefEffectSlot *sefGetEffectSlot(s32);

/* Adds one reference to the sefReferenceCounts entry at index. */
static inline void sefIncrementRef(s32 referenceIndex) {
    SefReferenceCount *referenceSlot = &sefReferenceCounts[referenceIndex];
    referenceSlot->referenceCount++;
}

/* Copies the request's vectors into the output record and stores the group. */
static inline void copyRequestVectorsToOutput(SefSpawnOutput *spawnOutput, SefSpawnRequest *spawnRequest, s32 groupId) {
    QuadwordCopyT0(&spawnOutput->vector00, &spawnRequest->vector40);
    QuadwordCopyT0(&spawnOutput->vector10, &spawnRequest->vector60);
    QuadwordCopyT0(&spawnOutput->vector20, &spawnRequest->vector70);
    spawnOutput->groupId = groupId;
}

/*
 * Output link used by two spawn modes: when sefOutputLinkEnabled is set, func_001F6590
 * picks an actor, and that actor's output record receives the request.
 */
#define SPAWN_LINK_OUTPUT() \
    if (sefOutputLinkEnabled) { \
        schedIndex = func_001F6590(sefOutputLinkEnabled, schedTable, schedIndex, groupId, &requestKey); \
        if (schedIndex >= 0) { \
            actor = func_001F9998(schedIndex); \
            spawnOutput = func_001F9A68(actor->actorId); \
            if (spawnOutput) { \
                copyRequestVectorsToOutput(spawnOutput, spawnRequest, groupId); \
            } \
        } \
    }

/*
 * Spawns the effect a request describes: picks the scheduler table of its
 * group, creates the scheduler entry for the current spawn mode (sefSpawnMode),
 * then either stores the vectors of an early request in the entry or keeps
 * the key block for the next request. Returns the entry, or the actor when no
 * entry was made.
 */
void *sefSpawnEffectFromRequest(SefSpawnRequest *spawnRequest) {
    SefSpawnKey requestKey;
    s32 schedTableOffset;
    s32 schedTableFlag;
    s32 otherSchedOffset;
    SefSchedulerEntry *ownerGroupSlot;
    s32 ownerGroupCount;
    SefSchedulerEntry *schedSlot;
    SefGroupActor *actor;
    s32 groupId;
    SefSchedulerEntry *sched;
    s16 *schedTable;
    s16 *otherSchedTable;
    u32 updatedFlags;
    s32 schedIndex;
    s32 spawnMode;
    s32 anySlotBusy;
    s32 subSlotNumber;
    SefSpawnOutput *spawnOutput;
    SefEffectSlot *effectSlot;
    SefSchedulerEntry *schedEntry;

    schedSlot = 0;
    actor = 0;
    ownerGroupSlot = 0;
    if (spawnRequest->groupLookupKey) {
        groupId = func_0020F790(spawnRequest->groupLookupKey);
    } else {
        groupId = spawnRequest->groupId;
    }
    if (spawnRequest->earlyMode && ((groupId >> 12) & 0x3FF) == 200) {
        return 0;
    }
    sched = sefGetScheduler();
    schedIndex = -1;
    schedTableFlag = 0;
    schedTable = scScriptLookup(groupId, &schedTableOffset, 1, &schedTableFlag);
    if (!schedTable) {
        return 0;
    }
    memset(&requestKey, 0, sizeof(requestKey));
    requestKey.unk_00 = spawnRequest->unk_00;
    requestKey.unk_04 = spawnRequest->unk_04;
    requestKey.unk_08 = spawnRequest->unk_08;
    requestKey.unk_0C = spawnRequest->unk_0C;
    requestKey.unk_10 = spawnRequest->unk_10;
    requestKey.unk_14 = spawnRequest->unk_14;
    requestKey.groupId = (s32 *) groupId;
    requestKey.scale = spawnRequest->scale;
    requestKey.unk_20 = spawnRequest->unk_3A;
    requestKey.unk_22 = spawnRequest->unk_3C;
    requestKey.unk_24 = spawnRequest->unk_18;
    requestKey.unk_28 = spawnRequest->unk_1C;
    requestKey.unk_2C = spawnRequest->unk_20;
    gSbStudioBlock.studioFlags &= ~0x10;
    spawnMode = sefSpawnMode;
    if (spawnMode == 64) {
        if (schedTableOffset) {
            schedIndex = sefCreateScheduler(schedTable + schedTableOffset, &requestKey, 0, schedTable, spawnRequest->schedulerArg);
            if (schedIndex >= 0) {
                schedSlot = sched + schedIndex;
                subSlotNumber = 0;
                anySlotBusy = 0;
                for (; subSlotNumber < 32; subSlotNumber++) {
                    if (schedSlot->subSlotBusy[subSlotNumber] >= 0) {
                        anySlotBusy = 1;
                        break;
                    }
                }
                if (!anySlotBusy) {
                    sefIncrementRef(schedSlot->refId);
                    schedSlot = 0;
                    sefReleaseEffectRecord(schedIndex);
                    schedIndex = -1;
                }
            }
        }
        SPAWN_LINK_OUTPUT()
    } else if (spawnMode == 62 || spawnMode == 65 || spawnMode == 66) {
        ownerGroupCount = sefCountOwnerGroupRecords(groupId, gSefExecSteps);
        if (sefSpawnMode == 65) {
            svSceneFlagsSet(1);
        }
        if (ownerGroupCount == 0) {
            otherSchedTable = scScriptLookup(groupId, &otherSchedOffset, 0, 0);
            if (otherSchedTable) {
                schedIndex = sefCreateScheduler(otherSchedTable + otherSchedOffset, &requestKey, 0, otherSchedTable, -1);
                if (schedIndex >= 0) {
                    schedEntry = sched + schedIndex;
                    if (countFlaggedSefEntries(schedEntry) == 0) {
                        schedEntry->schedFlags = (schedEntry->schedFlags | 0x08000000) & 0xFDFFFFFF;
                    } else {
                        schedEntry->schedFlags |= 0x0A000000;
                    }
                    ownerGroupSlot = &sched[schedIndex];
                    if (sefSpawnMode == 65) {
                        updatedFlags = ownerGroupSlot->schedFlags | 0x00802000;
                        ownerGroupSlot->schedFlags = updatedFlags;
                    }
                }
            }
            if (sefSpawnMode == 66) {
                SPAWN_LINK_OUTPUT()
            }
        } else {
            sefClearOwnerGroupFlags(groupId, gSefExecSteps, 0x02000000);
            gSbStudioBlock.studioFlags |= 0x10;
        }
        if (schedTableOffset) {
            schedIndex = sefScheduleEffectRecord(schedTable + schedTableOffset, &requestKey, 0, schedTable, sefOwnerGroupScale * ownerGroupCount, -1, 1, 0);
            if (schedIndex >= 0) {
                schedSlot = sched + schedIndex;
                if (sefSpawnMode == 65) {
                    schedSlot->schedFlags |= 0x00804000;
                }
                effectSlot = sefGetEffectSlot(schedSlot->subSlotBusy[0]);
                if (effectSlot && schedTableFlag) {
                    effectSlot->effectFlags |= 0x20000000;
                }
            }
        }
        sefRegisterGroupCenter(groupId, gSefExecSteps);
    } else {
        schedIndex = sefCreateScheduler(schedTable + schedTableOffset, &requestKey, 0, schedTable, spawnRequest->schedulerArg);
        if (schedIndex >= 0) {
            schedSlot = sched + schedIndex;
            if (sefSpawnMode == 61) {
                schedSlot->schedFlags |= 0x00800000;
            }
        }
    }
    if (schedSlot) {
        sefIncrementRef(schedSlot->refId);
    }
    if (ownerGroupSlot) {
        sefIncrementRef(ownerGroupSlot->refId);
    }
    if (actor) {
        sefIncrementRef(actor->resourceType);
    }
    if (schedSlot == 0) {
        return actor;
    }
    schedSlot->spawnCounter = spawnRequest->spawnCounter;
    if (spawnRequest->earlyMode == 0) {
        sefRequestKeyBlock.unk_00 = spawnRequest->unk_00;
        sefRequestKeyBlock.unk_20 = spawnRequest->unk_3A;
        sefRequestKeyBlock.unk_04 = spawnRequest->unk_04;
        sefRequestKeyBlock.unk_22 = spawnRequest->unk_3C;
        sefRequestKeyBlock.unk_08 = spawnRequest->unk_08;
        sefRequestKeyBlock.groupId = (s32 *) groupId;
        sefRequestKeyBlock.scale = spawnRequest->scale;
        sefRequestKeyBlock.unk_0C = spawnRequest->unk_0C;
        sefRequestKeyBlock.unk_10 = spawnRequest->unk_10;
        sefRequestKeyBlock.unk_14 = spawnRequest->unk_14;
    } else {
        QuadwordCopyT0(&schedSlot->vector10, &spawnRequest->vector40);
        QuadwordCopyT0(&schedSlot->vector20, &spawnRequest->vector50);
        QuadwordCopyT0(&schedSlot->vector30, &spawnRequest->vector60);
        QuadwordCopyT0(&schedSlot->vector40, &spawnRequest->vector70);
        {
            updatedFlags = schedSlot->schedFlags | 0x12;
            schedSlot->schedFlags = updatedFlags;
        }
    }
    return schedSlot;
}

extern s32 D_004749E0[];
void scKeyTableReset(s32 mode);

/* Calls scKeyTableReset(mode), then sets the three D_004749E0 words: mode 1 clears them all, mode 2 clears word 2 and sets words 0 and 1, any other mode sets them all. */
void func_001E9F48(s32 mode) {
    scKeyTableReset(mode);
    switch (mode) {
    case 1:
        D_004749E0[0] = 0;
        D_004749E0[1] = 0;
        D_004749E0[2] = 0;
        break;
    case 2:
        D_004749E0[2] = 0;
        D_004749E0[0] = 1;
        D_004749E0[1] = 1;
        break;
    default:
        D_004749E0[0] = 1;
        D_004749E0[1] = 1;
        D_004749E0[2] = 1;
        break;
    }
}

/* Returns the effectEntryFlags of the sef entry whose id is entryId, or 0 when there is none. */
int sefActorFlagsGet(int entryId)
{
    SefEntryTable *effectEntryTable;
    SefEntry *effectEntry;
    int i;

    if (entryId != 0) {
        effectEntryTable = &sefEffectEntryTable;
        i = 0;
        if (effectEntryTable->effectEntryCount > 0) {
            do {
                effectEntry = &effectEntryTable->effectEntries[i];
                if (effectEntry->id == entryId) {
                    return effectEntry->effectEntryFlags;
                }
                i++;
            } while (i < effectEntryTable->effectEntryCount);
        }
    }
    return 0;
}

/* Returns the effect object's flag byte (+0x14A), or 0 for a non-effect handle. */
u8 sefEffectFlagGet(SefEffectObject *effectObject) {
    if ((effectObject != 0) && (effectObject->handleClass == 0)) {
        return effectObject->flag14A;
    }
    return 0U;
}

s32 objectHasFlag5E();
s32 func_001F7870();

/* Returns nonzero when an effect object has its word50 flag or its record flag set; actors use objectHasFlag5E. */
s32 func_001EA050(SefEffectObject *effectObject) {
    s32 isFlagged;

    isFlagged = 0;
    if (effectObject != 0) {
        if (effectObject->handleClass == 0) {
            isFlagged = effectObject->resetVector50Word != 0;
            if (isFlagged == 0) {
                isFlagged = func_001F7870(effectObject->effectIndex);
            }
        } else {
            isFlagged = objectHasFlag5E(effectObject);
        }
    }
    return isFlagged;
}
