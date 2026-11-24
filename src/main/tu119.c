#include "common.h"
#include "shared.h"
#include "gs.h"
#include "vu0_macro.h"
#include "main/xgl_2.h"

#include "quadword.h"
typedef int s32;

typedef long long s64;

/* The 4-byte word at 0x08 of a link: sefLinkedObjectSetWord and objectSetLinkedValue use the s16 at 0x0A. */
/* The 4-byte word at 0x08 of a link; its s16 half (0x0A) is the linked value. */
typedef union LinkedValueSlot {
    s32 linkedWordValue;
    struct {
        u8 unk_00[2];
        s16 linkedValue; /* 0x0A */
    } linkedHalfValue;
} LinkedValueSlot;

/* The link key at 0x5C: the keyed path stores the whole word, the lookup reads the s16. */
/* The link key at 0x5C: the keyed path stores the whole word, the lookup reads the s16. */
typedef union LinkedChildKey {
    s32 keyWord;
    s16 key; /* 0x5C: key for func_001F9A68 */
} LinkedChildKey;

/* A linked object: an effect effectRecord or a group actor seen through its link fields. Delegated objects route the value
 * writes to the child object found by childKey. */
typedef struct LinkedObject {
    s32 delegated;               /* 0x00: nonzero routes value writes to the child object */
    s32 linkBits;                /* 0x04: bit 0x400000 is set or cleared by objectSetLinkedFlag */
    LinkedValueSlot linkedValue; /* 0x08 */
    u8 unk_0C[4];
    f32 linkedFloat10;           /* 0x10: stored by sefLinkedObjectSetLinkedFloat when delegated */
    u8 unk_14[0x18];
    f32 childFloat;              /* 0x2C: written on the child when delegated */
    u8 unk_30[0x1C];
    f32 ownFloat;                /* 0x4C */
    u8 unk_50[8];
    s32 linkedWord58;            /* 0x58: stored by sefLinkedObjectSetWord when not delegated */
    LinkedChildKey childKey;     /* 0x5C */
    s32 pendingFlags;            /* 0x60 */
    s32 pendingValue;            /* 0x64 */
    u8 unk_68[0xC];
    f32 linkedFloat74;           /* 0x74: stored by sefLinkedObjectSetLinkedFloat when not delegated */
    u8 unk_78[0xC6];
    s16 linkedTargetSlot;               /* 0x13E */
} LinkedObject;

void objectSetLinkedValue(LinkedObject *linkedObject, s32 value);
void objectSetLinkedFlag(LinkedObject *linkedObject, s32 value);

void objectApplyLinkedFlags(LinkedObject *linkedObject, s32 flagMask, s32 linkedValue) {
    if (linkedObject != 0) {
        if (flagMask & 0x400000) {
            objectSetLinkedFlag(linkedObject, linkedValue == 0);
        }
        if (flagMask & 0x200000) {
            objectSetLinkedValue(linkedObject, linkedValue);
        }
    }
}

/* A float and its bit pattern, read through the same four bytes. */
typedef union SefFloatBits {
    f32 asFloat;
    s32 asWord;
} SefFloatBits;

/* One effect effectRecord (0x150 bytes) of sefEffectRecords. The float view (this type) and the vector view
 * (SefVectorRecord) overlay 0x10..0x4F; they stay apart because their field types decide the emitted order. */
typedef struct SefEffectRecord {
    u8 unk_00[4];
    u32 effectFlags;          /* +0x04: bit 0x04000000 is read by sefBufferOwnedEffects */
    s16 modelIndex;     /* +0x08: index into D_00475604 */
    u8 unk_0A[2];
    s32 frames;         /* +0x0C */
    SefFloatBits posX;  /* +0x10: centre of the source rectangle, built by sefSetRecordRect */
    SefFloatBits posY;  /* +0x14 */
    u8 unk_18[0x30 - 0x18];
    SefFloatBits sizeX; /* +0x30: source rectangle width / 32 */
    SefFloatBits sizeY; /* +0x34: source rectangle height / 32 */
    u8 unk_38[0x4C - 0x38];
    SefFloatBits scale; /* +0x4C */
    s32 inUse;          /* +0x50: zero for a free effectRecord */
    u8 unk_54[4];
    s32 groupKey;       /* +0x58: compared with the group key argument */
    s32 unk_5C;
    u8 unk_60[0x70 - 0x60];
    s32 ownerKey;       /* +0x70: owner id compared with the owner argument */
    u8 unk_74[0x13E - 0x74];
    s16 effectSlot;           /* +0x13E: index of the spawned effect */
    u8 unk_140[0x10];
} SefEffectRecord;

typedef struct SefGroupActor {
    unsigned char unk_00[0x04];
    s32 groupKey; /* +0x04 */
    unsigned char unk_08[0x14 - 0x08];
    s32 ownerKey; /* +0x14 */
} SefGroupActor;

extern s32 sefEffectRecordCount;
extern SefEffectRecord *sefEffectRecords;
SefGroupActor *func_001F9998(s32 index);



/* One entry of the model table D_00475604: its key is compared with the effectRecord's model key. */
typedef struct SefModelEntry {
    u8 unk_00[0x20];
    s32 key; /* 0x20 */
    u8 unk_24[0x13B8];
} SefModelEntry;

extern SefModelEntry D_00475604[];
void sefApplyLinkedFlagsByKey(s32 key, s32 flagMask, s32 linkedValue);

/* Applies objectApplyLinkedFlags to the effect vectorRecords and the group actors of one key. A key with bits 28-29 equal to 3
 * selects the vectorRecords whose model key (D_00475604 at modelIndex) is the key; otherwise the vectorRecords whose owner is the key
 * are selected (every owner when the key is negative). The owner matches the actors in the same way. */
void sefApplyLinkedFlagsByKey(s32 key, s32 flagMask, s32 linkedValue)
{
    s32 i;
    SefGroupActor *actor;

    if (key > 0 && ((key >> 28) & 3) == 3) {
        for (i = 0; i < sefEffectRecordCount; i++) {
            if (sefEffectRecords[i].inUse != 0) {
                if (D_00475604[sefEffectRecords[i].modelIndex].key == key) {
                    objectApplyLinkedFlags((LinkedObject *) &sefEffectRecords[i], flagMask, linkedValue);
                }
            }
        }
        return;
    }
    for (i = 0; i < sefEffectRecordCount; i++) {
        if (sefEffectRecords[i].inUse != 0) {
            if (key < 0 || sefEffectRecords[i].ownerKey == key) {
                objectApplyLinkedFlags((LinkedObject *) &sefEffectRecords[i], flagMask, linkedValue);
            }
        }
    }
    for (i = 0; i < 40; i++) {
        actor = func_001F9998(i);
        if (actor != 0 && (actor->ownerKey == key || key < 0)) {
            objectApplyLinkedFlags((LinkedObject *) actor, flagMask, linkedValue);
        }
    }
}


void sefApplyLinkedFlagsByGroup(LinkedObject *linkedObject, s32 ownerKey, s32 flagMask, s32 linkedValue);

/* Applies objectApplyLinkedFlags to the effect vectorRecords and the group actors that belong to linkedObject (its address is
 * the group key) and to owner index (every owner when index < 0). */
void sefApplyLinkedFlagsByGroup(LinkedObject *linkedObject, s32 ownerKey, s32 flagMask, s32 linkedValue)
{
    s32 groupKey = (s32) linkedObject;
    s32 i;
    SefEffectRecord *effectRecord;
    SefGroupActor *actor;

    for (i = 0; i < sefEffectRecordCount; i++) {
        effectRecord = (SefEffectRecord *) (i * sizeof(SefEffectRecord) + (s32) sefEffectRecords);
        if (effectRecord->inUse != 0 && effectRecord->groupKey == groupKey && (effectRecord->ownerKey == ownerKey || ownerKey < 0)) {
            objectApplyLinkedFlags((LinkedObject *) effectRecord, flagMask, linkedValue);
        }
    }
    for (i = 0; i < 40; i++) {
        actor = func_001F9998(i);
        if (actor != 0 && actor->groupKey == groupKey && (actor->ownerKey == ownerKey || ownerKey < 0)) {
            objectApplyLinkedFlags((LinkedObject *) actor, flagMask, linkedValue);
        }
    }
}

void objectApplyLinkedValue(LinkedObject *linkedObject, s32 linkedValue) {
    sefApplyLinkedFlagsByKey(linkedObject, 0x200000, linkedValue);
}

void objectApplyLinkedValueIndexed(LinkedObject *linkedObject, s32 ownerKey, s32 linkedValue) {
    sefApplyLinkedFlagsByGroup(linkedObject, ownerKey, 0x200000, linkedValue);
}

void objectApplyLinkedFlag(LinkedObject *linkedObject, s32 linkEnabled) {
    sefApplyLinkedFlagsByKey(linkedObject, 0x400000, linkEnabled);
}

void objectApplyLinkedFlagIndexed(LinkedObject *linkedObject, s32 ownerKey, s32 linkEnabled) {
    sefApplyLinkedFlagsByGroup(linkedObject, ownerKey, 0x400000, linkEnabled);
}


/* Returns 1 when an effect effectRecord in use or a group actor has both the group key and the owner key, 0 otherwise. */
s32 sefGroupHasMatch(s32 groupKey, s32 ownerKey)
{
    s32 i;
    SefEffectRecord *effectRecord;
    SefGroupActor *actor;

    if (groupKey != 0 && ownerKey > 0) {
        i = 0;
        if (sefEffectRecordCount > 0) {
            effectRecord = sefEffectRecords;
            for (; i < sefEffectRecordCount; i++) {
                if (effectRecord->inUse == 0) {
                    effectRecord++;
                    continue;
                }
                if (effectRecord->ownerKey == ownerKey) {
                    if (effectRecord->groupKey == groupKey) {
                        return 1;
                    }
                }
                effectRecord++;
            }
        }
        for (i = 0; i < 40; i++) {
            actor = func_001F9998(i);
            if (actor != 0 && actor->ownerKey == ownerKey && actor->groupKey == groupKey) {
                return 1;
            }
        }
    }
    return 0;
}


void sefReleaseSlotsByKey(s32 index, s32 flag);
void sefEffectInstanceRelease(SefGroupActor *actor);
s32 sefReleaseEffectRecord();

/* Releases the effect vectorRecords in use and the group actors that have the group key and the owner key. */
void sefReleaseGroupMatches(s32 groupKey, s32 ownerKey)
{
    s32 i;
    SefGroupActor *actor;

    if (groupKey != 0 && ownerKey > 0) {
        i = 0;
        if (sefEffectRecordCount > 0) {
            for (; i < sefEffectRecordCount; i++) {
                if (sefEffectRecords[i].inUse != 0) {
                    if (sefEffectRecords[i].ownerKey == ownerKey) {
                        if (sefEffectRecords[i].groupKey == groupKey) {
                            sefReleaseSlotsByKey(i, 1);
                            sefReleaseEffectRecord(i);
                        }
                    }
                }
            }
        }
        for (i = 0; i < 40; i++) {
            actor = func_001F9998(i);
            if (actor != 0 && actor->ownerKey == ownerKey && actor->groupKey == groupKey) {
                sefEffectInstanceRelease(actor);
            }
        }
    }
}

typedef struct VuQuadword {
    s32 lanes[4];
} VuQuadword;

void *vuLaneMaskQuadsWrite(void *quadStorage)
{
  VuQuadword *quads;
  void *laneMaskQuad;
  quads = quadStorage;
  quads[0].lanes[0] = 0;
  quads[0].lanes[1] = 1;
  quads[0].lanes[2] = 1;
  quads[0].lanes[3] = 0;
  __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&quads[1]) : "memory");
  laneMaskQuad = &quads[2];
  __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w" : : : "memory");
  __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(laneMaskQuad) : "memory");
  __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&quads[3]) : "memory");
  return laneMaskQuad;
}

s32 func_001F6698();

void sceneTickSubsystem(void) {
    func_001F6698();
}

s32 sefResetEffectWorld();
void srsSetEffectFlag(s32, s32);

void sceneInitSubsystems(void) {
    sefResetEffectWorld();
    srsSetEffectFlag(0, 1);
}

/* A 16-byte vector of the effect vectorRecords and buffers (aligned like sceVu0FVECTOR). */
typedef struct SefVector {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} __attribute__((aligned(16))) SefVector;

/* One buffered effect of gEffectBuffer: the corners and spawn keys of an owned effectRecord, saved by
 * sefBufferOwnedEffects and respawned by sefReplayBufferedEffects. */
typedef struct SefBufferedEffect {
    SefVector vectors[4];
    void *spawnKey;
    void *spawnKey2;
    s32 owner;
    s32 effectSlot;
    s32 frames;
    u8 fixedTimingFlag; /* nonzero: replayed for 20 frames at speed 10 instead of frames / 1000 */
} SefBufferedEffect;

extern SefBufferedEffect gEffectBuffer[40];
void clearEffectTable(void);
void *memset(void *outputVector, s32 fillValue, unsigned int size);

void sceneClearBufferAndInit(void) {
    memset(gEffectBuffer, 0, 0xF00);
    clearEffectTable();
}

/* The vector view of an effect effectRecord: the four corners v[] (0x10..0x4F) overlay the float view SefEffectRecord.
 * They stay apart because the field types decide the emitted order. */
typedef struct SefVectorRecord {
    u8 unk_00[4];
    u32 effectFlags;            /* +0x04: bit 0x04000000 becomes the fixedTimingFlag of the buffered copy */
    u8 unk_08[4];
    s32 frames;           /* +0x0C */
    SefVector vectors[4]; /* +0x10 */
    s32 inUse;            /* +0x50 */
    u8 unk_54[4];
    void *spawnKey;       /* +0x58 */
    void *spawnKey2;      /* +0x5C */
    u8 unk_60[0x10];
    s32 ownerKey;         /* +0x70 */
    u8 unk_74[0x13E - 0x74];
    s16 effectSlot;             /* +0x13E */
    u8 unk_140[0x10];
} SefVectorRecord;

void sceneClearBufferAndInit(void);
void sefCompactActiveEffectRecords(void);

static inline void sefCopyVector(SefVector *destination, const SefVector *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8", "memory");
}

/* Saves the owned effect vectorRecords (in use, owner > 0) into gEffectBuffer: the corners, spawn keys, owner, slot, frames
 * and the 0x04000000 flag, then runs sefCompactActiveEffectRecords. */
void sefBufferOwnedEffects(void)
{
    s32 i;
    s32 copiedRecordCount;
    SefVectorRecord *vectorRecords;

    sceneClearBufferAndInit();
    vectorRecords = (SefVectorRecord *) sefEffectRecords;
    copiedRecordCount = 0;
    for (i = 0; i < 40; i++) {
        if (vectorRecords[i].inUse != 0 && vectorRecords[i].ownerKey > 0) {
            sefCopyVector(&gEffectBuffer[copiedRecordCount].vectors[0], &vectorRecords[i].vectors[0]);
            sefCopyVector(&gEffectBuffer[copiedRecordCount].vectors[1], &vectorRecords[i].vectors[1]);
            sefCopyVector(&gEffectBuffer[copiedRecordCount].vectors[2], &vectorRecords[i].vectors[2]);
            sefCopyVector(&gEffectBuffer[copiedRecordCount].vectors[3], &vectorRecords[i].vectors[3]);
            gEffectBuffer[copiedRecordCount].owner = vectorRecords[i].ownerKey;
            gEffectBuffer[copiedRecordCount].effectSlot = vectorRecords[i].effectSlot;
            gEffectBuffer[copiedRecordCount].frames = vectorRecords[i].frames;
            gEffectBuffer[copiedRecordCount].spawnKey = vectorRecords[i].spawnKey;
            gEffectBuffer[copiedRecordCount].spawnKey2 = vectorRecords[i].spawnKey2;
            gEffectBuffer[copiedRecordCount].fixedTimingFlag = (vectorRecords[i].effectFlags & 0x4000000) >> 26;
            copiedRecordCount++;
        }
    }
    sefCompactActiveEffectRecords();
}

/* The argument block of the spawn: its four corners sit at 0x40 and the owner at 0x2C. */
typedef struct SefEffectRequest {
    void *spawnKey;
    void *spawnKey2;
    u8 unk_08[0x20];
    void *unk_28; /* set to D_0053E8F0 by sefSpawnLinkedEffect */
    s32 owner;
    u8 unk_30[4];
    s32 effectSlot;
    u8 unk_38[8];
    SefVector vectors[4];
} SefEffectRequest;

void sefObjectSetStatePlay(f32 *spawnRequest, f32 scale);
SefEffectRecord *sefSpawnEffectFromRequest(f32 *spawnRequest);
void sefRunEffectFrames(void *owner, f32 frames, f32 speed);
void sefRunCompactedEffects(void);

/* Replays the buffered effects of gEffectBuffer: each one with an owner is respawned from its corners and keys and runs
 * for its frames (20 frames at speed 10 when fixedTimingFlag is set). Then sefRunCompactedEffects runs. */
void sefReplayBufferedEffects(void)
{
    s32 i;
    SefEffectRequest spawnRequest;
    void *spawned;

    /* kept: sefObjectSetStatePlay is called with the spawnRequest only (its scale argument is not set here) */
    ((void (*)(SefEffectRequest *)) sefObjectSetStatePlay)(&spawnRequest);
    for (i = 0; i < 40; i++) {
        s32 owner = gEffectBuffer[i].owner;

        if (owner > 0) {
            sefCopyVector(&spawnRequest.vectors[0], &gEffectBuffer[i].vectors[0]);
            sefCopyVector(&spawnRequest.vectors[1], &gEffectBuffer[i].vectors[1]);
            sefCopyVector(&spawnRequest.vectors[2], &gEffectBuffer[i].vectors[2]);
            sefCopyVector(&spawnRequest.vectors[3], &gEffectBuffer[i].vectors[3]);
            spawnRequest.spawnKey = gEffectBuffer[i].spawnKey;
            spawnRequest.spawnKey2 = gEffectBuffer[i].spawnKey2;
            spawnRequest.owner = owner;
            spawnRequest.effectSlot = gEffectBuffer[i].effectSlot;
            spawned = sefSpawnEffectFromRequest((f32 *) &spawnRequest);
            if (spawned != 0) {
                if (gEffectBuffer[i].fixedTimingFlag != 0) {
                    sefRunEffectFrames(spawned, 20.0f, 10.0f);
                } else {
                    sefRunEffectFrames(spawned, (f32) gEffectBuffer[i].frames / 1000.0f, 1.0f);
                }
            }
            continue;
        }
        break;
    }
    sefRunCompactedEffects();
}


s32 sefSetSlotValueByKey(s16, s32);

/* Sets the link word: when the object is not delegated it is kept in linkedWord58 and passed to sefSetSlotValueByKey with the
 * link index; a delegated object keeps it in linkBits. */
void sefLinkedObjectSetWord(LinkedObject *linkedObject, s32 linkedWordValue)
{
    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            linkedObject->linkedWord58 = linkedWordValue;
            sefSetSlotValueByKey(linkedObject->linkedTargetSlot, linkedWordValue);
        } else {
            linkedObject->linkBits = linkedWordValue;
        }
    }
}

void sefSetEntryValueById(s32 linkedTargetSlot, s32 value);

/* Sets the link key word: when the object is not delegated the word at 0x5C is written and passed to sefSetEntryValueById
 * with the link index; a delegated object stores it at 0x08 (the linked value word). */
void sefLinkedObjectSetChildKey(LinkedObject *linkedObject, s32 linkedWordValue)
{
    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            linkedObject->childKey.keyWord = linkedWordValue;
            sefSetEntryValueById(linkedObject->linkedTargetSlot, linkedWordValue);
        } else {
            linkedObject->linkedValue.linkedWordValue = linkedWordValue;
        }
    }
}

void objectSetPendingFlags(LinkedObject *linkedObject, s32 newPendingFlags, s32 newPendingValue) {
    if ((linkedObject != 0) && (linkedObject->delegated == 0)) {
        linkedObject->pendingFlags = newPendingFlags;
        linkedObject->pendingValue = newPendingValue;
    }
}

s32 sefSetEntryWeightById(s16, f32);

/* Sets the link float: linkedFloat74 and sefSetEntryWeightById when not delegated, linkedFloat10 when delegated. */
void sefLinkedObjectSetLinkedFloat(LinkedObject *linkedObject, f32 linkedFloatValue)
{
    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            linkedObject->linkedFloat74 = linkedFloatValue;
            sefSetEntryWeightById(linkedObject->linkedTargetSlot, linkedFloatValue);
        } else {
            linkedObject->linkedFloat10 = linkedFloatValue;
        }
    }
}

void sefSetSlotRecordPos(s16, const void *);
LinkedObject *func_001F9A68(s16);

/* Copies the 16-byte position at source: into linkedFloat10 and sefSetSlotRecordPos when not delegated, else to offset 0 of
 * the child object. */
void sefLinkedObjectSetPosition(LinkedObject *linkedObject, const void *position)
{
    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            QuadwordCopyT0(&linkedObject->linkedFloat10, position);
            sefSetSlotRecordPos(linkedObject->linkedTargetSlot, position);
        } else {
            LinkedObject *childObject = func_001F9A68(linkedObject->childKey.key);
            if (childObject != 0) {
                QuadwordCopyT0(childObject, position);
            }
        }
    }
}

/* Copies the 16-byte rotation at source (offset 0x20) of a link that is not delegated. */
void sefLinkedObjectSetRotation(LinkedObject *linkedObject, const void *rotationVector)
{
    if ((linkedObject != 0) && (linkedObject->delegated == 0)) {
        QuadwordCopyT0(&linkedObject->unk_14[0xC], rotationVector);
    }
}

void sefSetSlotRecordVectorQ(s16, const void *);

/* Copies the 16-byte vector at source to offset 0x30: a link that is not delegated keeps it, a delegated one copies it to
 * offset 0x10 of the child object. */
void func_001EAA98(LinkedObject *linkedObject, const void *vector)
{
    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            QuadwordCopyT0(&linkedObject->unk_30[0], vector);
            sefSetSlotRecordVectorQ(linkedObject->linkedTargetSlot, vector);
        } else {
            LinkedObject *childObject = func_001F9A68(linkedObject->childKey.key);
            if (childObject != 0) {
                QuadwordCopyT0(&childObject->linkedFloat10, vector);
            }
        }
    }
}

void sefSetSlotRecordVectorR(s16, const void *);

/* Copies the 16-byte vector at source to offset 0x40: a link that is not delegated keeps it, a delegated one copies it to
 * offset 0x20 of the child object. */
void func_001EAB00(LinkedObject *linkedObject, const void *vector)
{
    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            QuadwordCopyT0(&linkedObject->unk_30[0x10], vector);
            sefSetSlotRecordVectorR(linkedObject->linkedTargetSlot, vector);
        } else {
            LinkedObject *childObject = func_001F9A68(linkedObject->childKey.key);
            if (childObject != 0) {
                QuadwordCopyT0(&childObject->unk_14[0xC], vector);
            }
        }
    }
}

s32 sefSetSlotRecordVectorRW(s16);

void linkedObjectSetFloat(LinkedObject *linkedObject, f32 linkedFloat) {
    LinkedObject *childObject;

    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            linkedObject->ownFloat = linkedFloat;
            sefSetSlotRecordVectorRW(linkedObject->linkedTargetSlot);
            return;
        }
        childObject = func_001F9A68(linkedObject->childKey.key);
        if (childObject != 0) {
            childObject->childFloat = linkedFloat;
        }
    }
}

s32 func_001F7740(s16 linkedTargetSlot, s32 value);

void objectSetLinkedValue(LinkedObject *linkedObject, s32 linkedValue)
{
  s32 newPendingFlags;
  if (linkedObject != 0)
  {
    if (linkedObject->delegated == 0)
    {
      linkedObject->linkedValue.linkedHalfValue.linkedValue = linkedValue;
      func_001F7740(linkedObject->linkedTargetSlot, linkedValue);
    }
    else
    {
      newPendingFlags = (linkedObject->pendingFlags & (~0x100)) | ((linkedValue & 1) << 8);
      linkedObject->pendingFlags = newPendingFlags;
    }
  }
}

s32 func_001F77D8(s16 linkedTargetSlot, s32 value);

void objectSetLinkedFlag(LinkedObject *linkedObject, s32 linkEnabled)
{
  s32 newPendingFlags;
  s32 flagsWithLinked;
  s32 flagsWithoutLinked;
  if (linkedObject != 0)
  {
    if (linkedObject->delegated == 0)
    {
      if (linkEnabled != 0)
      {
        flagsWithLinked = linkedObject->linkBits | 0x400000;
        linkedObject->linkBits = flagsWithLinked;
      }
      else
      {
        flagsWithoutLinked = linkedObject->linkBits & 0xFFBFFFFF;
        linkedObject->linkBits = flagsWithoutLinked;
      }
      func_001F77D8(linkedObject->linkedTargetSlot, linkEnabled);
    }
    else
    {
      newPendingFlags = (linkedObject->pendingFlags & (~0x200)) | ((linkEnabled & 1) << 9);
      linkedObject->pendingFlags = newPendingFlags;
    }
  }
}


extern u8 D_0053E8F0[];


s32 sefRunEffectRecords(SefEffectRecord *effectRecord);

/* Spawns the effect of a linked object: the spawnRequest takes the object's words 0..3 as its source rectangle and the
 * scale. Returns the slot of the spawned effectRecord, or -1 when nothing spawned. */
s32 sefSpawnLinkedEffect(LinkedObject *linkedObject, f32 scale)
{
    f32 spawnRequest[32];
    SefEffectRecord *spawned;
    s32 spawnedSlot;
    s32 *rect = (s32 *) linkedObject;

    sefObjectSetStatePlay(spawnRequest, scale);
    ((SefFloatBits *) spawnRequest)[31].asFloat = scale;
    spawnRequest[16] = ((f32) rect[0] - 256.0f) + (f32) rect[2] * 0.5f;
    spawnRequest[17] = -(((f32) rect[1] - 224.0f) + (f32) rect[3] * 0.5f) * 0.875f;
    spawnRequest[24] = (f32) rect[2] * 0.03125f;
    spawnRequest[25] = (f32) rect[3] * 0.03125f;
    ((SefFloatBits *) spawnRequest)[10].asWord = (s32) D_0053E8F0;

    spawned = sefSpawnEffectFromRequest(spawnRequest);
    if (spawned != 0) {
        sefRunEffectRecords(spawned);
        spawned->effectFlags |= 0x10;
        spawnedSlot = spawned->effectSlot;
    } else {
        spawnedSlot = -1;
    }
    return spawnedSlot;
}

/* Writes effectRecord index from the four words of a source rectangle (left, top, width, height): its centre, its size and the
 * scale, then refreshes the effectRecord through sefRunEffectRecords. */
void sefSetRecordRect(s32 *rect, s32 effectRecordIndex, f32 scale)
{
    sefEffectRecords[effectRecordIndex].posX.asFloat = ((f32) rect[0] - 256.0f) + (f32) rect[2] * 0.5f;
    sefEffectRecords[effectRecordIndex].posY.asFloat = -(((f32) rect[1] - 224.0f) + (f32) rect[3] * 0.5f) * 0.875f;
    sefEffectRecords[effectRecordIndex].sizeX.asFloat = (f32) rect[2] * 0.03125f;
    sefEffectRecords[effectRecordIndex].sizeY.asFloat = (f32) rect[3] * 0.03125f;
    sefEffectRecords[effectRecordIndex].scale.asFloat = scale;
    sefRunEffectRecords(&sefEffectRecords[effectRecordIndex]);
}

s32 sefReleaseOwnerEffects(s32);

void checkResultReport(s32 errorCode) {
    if (errorCode < 0) {
        sefReleaseOwnerEffects(0x1006402A);
        return;
    }
    sefReleaseEffectRecord();
}

extern unsigned int gSceneLoadState;

s32 stateIsReady(void) {
    return gSceneLoadState == 3;
}

extern u8 *D_004C83F0[];
void slotReleaseFixedSlots();
void sceneShatterTransitionUpdate(void);

void sceneCallbackReset(void) {
    gSceneLoadState = 0;
    slotReleaseFixedSlots();
    D_004C83F0[0] = sceneShatterTransitionUpdate;
}

/* The screen-break transition. sceneCallbackReset installs it in the render state's frame hook; gSceneLoadState steps
 * it: 0 builds the shards, 1 shows the still screen for two frames, 2 lets the shards fly until none is left, 3 ends it
 * and clears the hook. The screen is cut into SHATTER_ROWS x SHATTER_COLUMNS hexagons laid on the camera's near plane;
 * each is drawn as a textured triangle fan from a half-size copy of the screen. */
#define SHATTER_ROWS 29
#define SHATTER_COLUMNS 14
#define SHATTER_PI 3.1415927f

/* A vector of the effect. The projected corners are read back through words: MMathRotTransPers leaves GS fixed-point
 * x, y and z in them, and the fourth word of a shard's center corner holds its culled flag. */
typedef union ShatterVector {
    Vector4 vector;
    float floatLanes[4];
    int wordLanes[4];
} ShatterVector;

/* One flying hexagon of the screen-break effect. */
typedef struct ShatterShard {
    ShatterVector position; /* w: the alpha, 128 when built, falling by 10 per frame once the shard has flown off */
    ShatterVector velocity; /* w: the frames left before the shard starts to move, then counts down to -2 */
    ShatterVector spin;     /* added to angle every frame */
    ShatterVector angle;    /* the rotation about x, y and z */
} ShatterShard;

/* The frame-buffer pages and the frame hook of the render state (D_004C8390) this effect reads. */
typedef struct ShatterRenderState {
    unsigned char unk_00[8];
    u16 depthPage;      /* +0x08: ZBUF base */
    unsigned char unk_0a[4];
    u16 outlinePage;    /* +0x0E: the page the outlines and the glow are drawn to */
    u16 copyPage;       /* +0x10: the half-size copy of the screen the shards are textured from */
    u16 glowSourcePage; /* +0x12 */
    u16 glowPage;       /* +0x14 */
    u16 screenPage;     /* +0x16: the shown screen */
    unsigned char unk_18[0x48];
    void (*frameHook)(void); /* +0x60 */
} ShatterRenderState;

extern ShatterRenderState D_004C8390;
extern ShatterShard gShatterShards[SHATTER_ROWS][SHATTER_COLUMNS];
/* The shards still visible in the last frame of state 2. */
extern int gShatterActiveShardCount;
extern const Matrix4 MMathUnitMatrix;
void *memset(void *outputVector, int value, unsigned int size);
void xglPacketBufferInit(s32 spr_address);
void xglPacketAppendEntry(s32 word0, s32 word1, s32 word2, s32 word3);
void xglPacketAppendQuad(s64 reg, s64 value);
void xglPacketFinishTag(void);
void xglPacketSendDirect(s32 address, s32 qwordCount);
int func_0030DD10(void);
u64 func_003098D0(f32 value);
float xglSin(float angle);
float MMathCalcLength(const Vector4 *vector);
float MMathCalcLengthXY(const Vector4 *vector);
Matrix4 *MMathRotateMatrixX(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixY(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathRotateMatrixZ(Matrix4 *outMatrix, Matrix4 *matrix, float angle);
Matrix4 *MMathMulMatrix(Matrix4 *outMatrix, Matrix4 *rhs, Matrix4 *lhs);
Vector4 *MMathApplyMatrix(Vector4 *outVector, Matrix4 *matrix, const Vector4 *vector);
void MMathRotTransPers(Vector4 *outVector, const struct XglCamera *camera, const Matrix4 *matrix,
                       const Vector4 *point);
struct XglCamera *xglStudioGetActiveCamera(void);
void drawRectOnPageColour(s32 page, s32 x, s32 y, s32 z, s32 width, s32 height);
void drawAreaSetExtended(s32 word_00, s32 word_3C, s32 word_44, s32 word_04, s32 word_40, s32 word_48, s32 frameBase,
                         s32 drawMode, s32 origin_x, s32 origin_y, s32 width, s32 height, s32 clip_x, s32 clip_y,
                         s32 draw_width, s32 draw_height);
void drawAreaSetWindow(s32 frameBase, s32 drawMode, s32 origin_x, s32 origin_y, s32 width, s32 height, s32 clip_x,
                       s32 clip_y, s32 draw_width, s32 draw_height);
void drawEnvSendRecordPacket(void);
void func_001D55E8(void);

/* The GS vertex of one projected hexagon corner. */
#define SHATTER_XYZ(point) GS_SET_XYZ((point).wordLanes[0], (point).wordLanes[1], (point).wordLanes[2])

static inline void sefAddVector(Vector4 *outputVector, const Vector4 *leftVector, const Vector4 *rightVector)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%5)\n"
        "lqc2 vf2, 0(%6)\n"
        "vadd.xyz vf1, vf1, vf2\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (outputVector->x), "=m" (outputVector->y), "=m" (outputVector->z), "=m" (outputVector->w)
        : "r" (outputVector), "r" (leftVector), "r" (rightVector)
        : "memory");
}

static inline void sefSubVector(Vector4 *outputVector, const Vector4 *minuend, const Vector4 *subtrahend)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%5)\n"
        "lqc2 vf2, 0(%6)\n"
        "vsub.xyz vf1, vf1, vf2\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (outputVector->x), "=m" (outputVector->y), "=m" (outputVector->z), "=m" (outputVector->w)
        : "r" (outputVector), "r" (minuend), "r" (subtrahend)
        : "memory");
}

static inline void sefScaleVector(Vector4 *outputVector, const Vector4 *vector, float scale)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%5)\n"
        "mfc1 $8, %6\n"
        "qmtc2.ni $8, vf2\n"
        "vmulx.xyz vf1, vf1, vf2x\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (outputVector->x), "=m" (outputVector->y), "=m" (outputVector->z), "=m" (outputVector->w)
        : "r" (outputVector), "r" (vector), "f" (scale)
        : "$8", "memory");
}

static inline void sefSetOrigin(Vector4 *outputVector)
{
    __asm__ __volatile__(
        "sqc2 vf0, 0(%4)\n"
        : "=m" (outputVector->x), "=m" (outputVector->y), "=m" (outputVector->z), "=m" (outputVector->w)
        : "r" (outputVector)
        : "memory");
}

static inline void sefAccumulateVector(Vector4 *outputVector, const Vector4 *addend)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%4)\n"
        "lqc2 vf2, 0(%5)\n"
        "vadd.xyz vf1, vf1, vf2\n"
        "sqc2 vf1, 0(%4)\n"
        : "=m" (outputVector->x), "=m" (outputVector->y), "=m" (outputVector->z), "=m" (outputVector->w)
        : "r" (outputVector), "r" (addend)
        : "memory");
}

/* Screen shatter transition: case 0 seeds a random velocity and spin per hexagonal shard (SHATTER_ROWS x
 * SHATTER_COLUMNS grid), cases 1/2 draw the captured screen page as shards (1: still, 2: flying and fading)
 * and blur-glow the spawnedSlot; case 3 clears the frame hook. */
void sceneShatterTransitionUpdate(void)
{
    Vector4 nearOrigin;
    Vector4 nearRight;
    Vector4 nearTop;
    Vector4 nearCorner;
    Vector4 nearWidth;
    Vector4 nearHeight;
    Matrix viewMatrix;
    Vector4 grid[SHATTER_ROWS][SHATTER_COLUMNS];
    Matrix4 rotation;
    Vector4 direction;
    Matrix4 turn;
    Vector4 scaled;
    ShatterVector screen[SHATTER_COLUMNS][7];
    int row;
    int column;
    Matrix4 *cameraView;
    struct XglCamera *camera;
    float gridScaleX;
    float gridScaleY;
    float gridX;
    float gridY;
    float glowStep;
    int gridLeft;

    switch (gSceneLoadState) {
    case 0:
        gShatterActiveShardCount = 0;
        xglPacketBufferInit(0x70000000);
        xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FBA_1, 0);
        xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
        xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthPage, 0x31, 1));
        xglPacketAppendQuad(GS_CLAMP_1, 0);
        xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
        xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0, 0x3F800000));
        xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.screenPage << 5, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.copyPage, 4, 0, 0xFF000000));
        xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
        xglPacketAppendQuad(GS_UV, 0);
        xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
        xglPacketAppendQuad(GS_UV, GS_SET_UV(0x2000, 0x1C00));
        xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x7FF8, 0x81F7, 0));
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);
        for (row = 0; row < SHATTER_ROWS; row++) {
            for (column = 0; column < SHATTER_COLUMNS; column++) {
                memset(&direction, 0, sizeof(direction));
                direction.z = -10.0f;
                sefSetOrigin(&gShatterShards[row][column].position.vector);
                sefSetOrigin(&gShatterShards[row][column].velocity.vector);
                sefSetOrigin(&gShatterShards[row][column].angle.vector);
                Vu0CopyUnitMatrix(&turn, &MMathUnitMatrix);
                MMathRotateMatrixY(&turn, &turn, func_0030DD10() % 360 / 180.0f * SHATTER_PI);
                MMathApplyMatrix(&gShatterShards[row][column].velocity.vector, &turn, &direction);
                gShatterShards[row][column].spin.floatLanes[0] = (func_0030DD10() % 100 - 50) / 10.0f / 180.0f * SHATTER_PI;
                gShatterShards[row][column].spin.floatLanes[1] = (func_0030DD10() % 100 - 50) / 10.0f / 180.0f * SHATTER_PI;
                gShatterShards[row][column].spin.floatLanes[2] = (func_0030DD10() % 100 - 50) / 10.0f / 180.0f * SHATTER_PI;
                sefSetOrigin(&direction);
                gridLeft = row * 18;
                if (column & 1) {
                    direction.x = (gridLeft + 9) / 512.0f - 0.5f;
                } else {
                    direction.x = gridLeft / 512.0f - 0.5f;
                }
                direction.y = -((column * 36) / 448.0f - 0.5f);
                sefScaleVector(&direction, &direction, 10.0f);
                sefAccumulateVector(&gShatterShards[row][column].velocity.vector, &direction);
                sefScaleVector(&direction, &direction, 2.0f);
                gShatterShards[row][column].velocity.floatLanes[3] = (int) MMathCalcLengthXY(&direction) + 1;
                gShatterShards[row][column].position.floatLanes[3] = 128.0f;
            }
        }
        gSceneLoadState = 1;
    case 1:
        if (++gShatterActiveShardCount < 2) {
            break;
        }
        gSceneLoadState = 2;
        gShatterActiveShardCount = -1;
    case 2:
        if (gShatterActiveShardCount != 0) {
            gShatterActiveShardCount = 0;
            break;
        }
        gSceneLoadState = 3;
    case 3:
        D_004C8390.frameHook = 0;
        return;
    }

    viewMatrix = xglStudioGetActiveCamera()->travel.transform;
    viewMatrix.rows[3][0] = viewMatrix.rows[3][1] = viewMatrix.rows[3][2] = 0.0f;
    viewMatrix.rows[3][3] = 1.0f;
    nearOrigin = xglStudioGetActiveCamera()->volume[0].vertex[2].vector;
    nearRight = xglStudioGetActiveCamera()->volume[0].vertex[3].vector;
    nearTop = xglStudioGetActiveCamera()->volume[0].vertex[0].vector;
    nearCorner = xglStudioGetActiveCamera()->volume[0].vertex[1].vector;
    sefSubVector(&nearWidth, &nearRight, &nearOrigin);
    sefSubVector(&nearHeight, &nearTop, &nearOrigin);
    gridScaleX = MMathCalcLength(&nearWidth) / 512.0f * 1.01f;
    gridScaleY = MMathCalcLength(&nearHeight) / 448.0f * 1.01f;
    for (row = 0; row < SHATTER_ROWS; row++) {
        for (column = 0; column < SHATTER_COLUMNS; column++) {
            if (column & 1) {
                gridX = (row * 18 + 9) / 512.0f;
            } else {
                gridX = (row * 18) / 512.0f;
            }
            gridY = (column * 36) / 448.0f;
            sefScaleVector(&scaled, &nearWidth, gridX);
            sefAddVector(&grid[row][column], &nearOrigin, &scaled);
            sefScaleVector(&scaled, &nearHeight, gridY);
            sefAddVector(&grid[row][column], &grid[row][column], &scaled);
        }
    }

    xglPacketBufferInit(0x70000000);
    xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
    xglPacketAppendQuad(GS_TEXFLUSH, 0);
    xglPacketAppendQuad(GS_FBA_1, 0);
    xglPacketAppendQuad(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
    xglPacketAppendQuad(GS_CLAMP_1, 0);
    xglPacketAppendQuad(GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthPage, 0x31, 1));
    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0, 0, 0, 0x80, 0x3F800000));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 2, 0x80));
    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.outlinePage, 8, 0, 0xFF000000));
    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 0, 0, 0, 1, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
    xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
    xglPacketAppendQuad(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 0, 1, 0x80));
    xglPacketAppendQuad(GS_TEX0_1, GS_SET_TEX0(D_004C8390.copyPage << 5, 4, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0));
    xglPacketAppendQuad(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
    xglPacketFinishTag();
    xglPacketSendDirect(0, 0);

    for (row = 0; row < SHATTER_ROWS; row++) {
        xglPacketBufferInit(0x70000000);
        xglPacketAppendEntry(0x8000, 0x10000000, GIF_AD, 0);
        xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.screenPage, 8, 0, 0xFF000000));
        for (column = 0; column < SHATTER_COLUMNS; column++) {
            float halfWidth = gridScaleX * 18.0f * 0.5f;
            float halfHeight = gridScaleY * 48.0f * 0.5f;
            float quarterHeight = gridScaleY * 24.0f * 0.5f;
            ShatterVector hexagon[7] = {
                {{0.0f, 0.0f, 0.0f, 1.0f}},
                {{0.0f, halfHeight, 0.0f, 1.0f}},
                {{halfWidth, quarterHeight, 0.0f, 1.0f}},
                {{halfWidth, -quarterHeight, 0.0f, 1.0f}},
                {{0.0f, -halfHeight, 0.0f, 1.0f}},
                {{-halfWidth, -quarterHeight, 0.0f, 1.0f}},
                {{-halfWidth, quarterHeight, 0.0f, 1.0f}},
            };
            Matrix4 worldMatrix;
            Vector4 driftOffset;

            cameraView = &xglStudioGetActiveCamera()->volume[0].viewMatrix;
            switch (gSceneLoadState) {
            case 1:
                xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
                xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(6, 1, 1, 0, 1, 0, 1, 0, 0));
                row = SHATTER_ROWS;
                column = SHATTER_COLUMNS;
                xglPacketAppendQuad(GS_UV, 0);
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x1000, 0x1000));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
                xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(2, 1, 1, 0, 1, 0, 1, 0, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x10, 0x10));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0xFF0, 0x10));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x71F7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0xFF0, 0xFF0));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x8FF8, 0x8DF7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x10, 0xFF0));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x8DF7, 0));
                xglPacketAppendQuad(GS_UV, GS_SET_UV(0x10, 0x10));
                xglPacketAppendQuad(GS_XYZF2, GS_SET_XYZ(0x6FF8, 0x71F7, 0));
                break;
            case 2:
                if (gShatterShards[row][column].velocity.floatLanes[3] > 0.0f) {
                    gShatterShards[row][column].velocity.floatLanes[3] -= 1.0f;
                    if (gShatterShards[row][column].velocity.floatLanes[3] <= 0.0f) {
                        gShatterShards[row][column].velocity.floatLanes[3] = 0.0f;
                    }
                    gShatterActiveShardCount++;
                } else if (gShatterShards[row][column].velocity.floatLanes[3] <= 0.0f
                           && gShatterShards[row][column].velocity.floatLanes[3] >= -2.0f) {
                    gShatterShards[row][column].velocity.floatLanes[3] -= 1.0f;
                } else if (gShatterShards[row][column].position.floatLanes[3] > 0.0f) {
                    gShatterShards[row][column].position.floatLanes[3] -= 10.0f;
                    gShatterActiveShardCount++;
                } else {
                    gShatterShards[row][column].position.floatLanes[3] = 0.0f;
                    continue;
                }
                Vu0CopyUnitMatrix(&rotation, &MMathUnitMatrix);
                if (gSceneLoadState == 2 && gShatterShards[row][column].velocity.floatLanes[3] <= -2.0f) {
                    sefAddVector(&gShatterShards[row][column].angle.vector, &gShatterShards[row][column].angle.vector,
                                       &gShatterShards[row][column].spin.vector);
                    MMathRotateMatrixZ(&rotation, &rotation, gShatterShards[row][column].angle.floatLanes[2]);
                    MMathRotateMatrixX(&rotation, &rotation, gShatterShards[row][column].angle.floatLanes[0]);
                    MMathRotateMatrixY(&rotation, &rotation, gShatterShards[row][column].angle.floatLanes[1]);
                }
                MMathMulMatrix(&worldMatrix, &viewMatrix.rows, &rotation);
                MMathApplyMatrix(&hexagon[0].vector, &worldMatrix, &hexagon[0].vector);
                MMathApplyMatrix(&hexagon[1].vector, &worldMatrix, &hexagon[1].vector);
                MMathApplyMatrix(&hexagon[2].vector, &worldMatrix, &hexagon[2].vector);
                MMathApplyMatrix(&hexagon[3].vector, &worldMatrix, &hexagon[3].vector);
                MMathApplyMatrix(&hexagon[4].vector, &worldMatrix, &hexagon[4].vector);
                MMathApplyMatrix(&hexagon[5].vector, &worldMatrix, &hexagon[5].vector);
                MMathApplyMatrix(&hexagon[6].vector, &worldMatrix, &hexagon[6].vector);
                if (gSceneLoadState == 2 && gShatterShards[row][column].velocity.floatLanes[3] <= -2.0f) {
                    sefAddVector(&gShatterShards[row][column].position.vector, &gShatterShards[row][column].position.vector,
                                       &gShatterShards[row][column].velocity.vector);
                    sefScaleVector(&driftOffset, &gShatterShards[row][column].position.vector, gridScaleX);
                    MMathApplyMatrix(&driftOffset, &viewMatrix.rows, &driftOffset);
                    sefAccumulateVector(&hexagon[0].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[1].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[2].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[3].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[4].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[5].vector, &driftOffset);
                    sefAccumulateVector(&hexagon[6].vector, &driftOffset);
                }
                sefAccumulateVector(&hexagon[0].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[1].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[2].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[3].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[4].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[5].vector, &grid[row][column]);
                sefAccumulateVector(&hexagon[6].vector, &grid[row][column]);
                {
                    int visible;
                    int corner;

                    camera = xglStudioGetActiveCamera();
                    visible = 0;
                    MMathRotTransPers(&screen[column][0].vector, camera, cameraView, &hexagon[0].vector);
                    MMathRotTransPers(&screen[column][1].vector, camera, cameraView, &hexagon[1].vector);
                    MMathRotTransPers(&screen[column][2].vector, camera, cameraView, &hexagon[2].vector);
                    MMathRotTransPers(&screen[column][3].vector, camera, cameraView, &hexagon[3].vector);
                    MMathRotTransPers(&screen[column][4].vector, camera, cameraView, &hexagon[4].vector);
                    MMathRotTransPers(&screen[column][5].vector, camera, cameraView, &hexagon[5].vector);
                    MMathRotTransPers(&screen[column][6].vector, camera, cameraView, &hexagon[6].vector);
                    /* kept: the loop (always marks the shard visible) decides the emitted order */
                    for (corner = 0; corner < 7; corner++) {
                        visible |= 1;
                    }
                    if (visible) {
                        screen[column][0].wordLanes[3] = 0;
                    } else {
                        screen[column][0].wordLanes[3] = 1;
                        continue;
                    }
                    {
                        int textureX;
                        int textureY;
                        u64 centerU;
                        u64 rightU;
                        u64 leftU;
                        u64 firstUv;
                        u64 upperV;
                        u64 lowerV;

                        textureX = (column & 1) ? row * 18 + 9 : row * 18;
                        textureX += 512;
                        textureY = column * 36 + 448;
                        xglPacketAppendQuad(GS_RGBAQ,
                                            GS_SET_RGBAQ(0x80, 0x80, 0x80,
                                                         (int) ((1.0f - xglSin((gShatterShards[row][column].position.floatLanes[3] / 128.0f * 90.0f
                                                                                + 90.0f) / 180.0f * SHATTER_PI))
                                                                * 128.0f),
                                                         0x3F800000));
                        xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(5, 1, 1, 0, 1, 0, 1, 0, 0));
                        centerU = func_003098D0((float) (textureX << 4) * 0.5f + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(centerU, func_003098D0((float) (textureY << 4)
                                                                                   * (256.0f / 448.0f) + 4.0f)));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][0]));
                        firstUv = GS_SET_UV(centerU, func_003098D0((float) ((textureY - 24) << 4) * (256.0f / 448.0f) + 4.0f));
                        xglPacketAppendQuad(GS_UV, firstUv);
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                        rightU = func_003098D0((float) ((textureX + 9) << 4) * 0.5f + 4.0f);
                        upperV = func_003098D0((float) ((textureY - 12) << 4) * (256.0f / 448.0f) + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(rightU, upperV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][2]));
                        lowerV = func_003098D0((float) ((textureY + 12) << 4) * (256.0f / 448.0f) + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(rightU, lowerV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][3]));
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(centerU, func_003098D0((float) ((textureY + 24) << 4)
                                                                                   * (256.0f / 448.0f) + 4.0f)));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][4]));
                        leftU = func_003098D0((float) ((textureX - 9) << 4) * 0.5f + 4.0f);
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(leftU, lowerV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][5]));
                        xglPacketAppendQuad(GS_UV, GS_SET_UV(leftU, upperV));
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][6]));
                        xglPacketAppendQuad(GS_UV, firstUv);
                        xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                    }
                }
                break;
            }
        }
        if (gSceneLoadState == 2) {
            for (column = 0; column < SHATTER_COLUMNS; column++) {
                int alpha = (int) gShatterShards[row][column].position.floatLanes[3];
                int delay = (int) gShatterShards[row][column].velocity.floatLanes[3];
                if (screen[column][0].wordLanes[3] == 0 && delay <= 0 && alpha > 0) {
                    float brightness = alpha / 128.0f;
                    int red = (int) (brightness * 32.0f);
                    int green = (int) (brightness * 127.0f) + 0x80;
                    int blue = (int) (brightness * -95.0f) + 0xFF;
                    xglPacketAppendQuad(GS_RGBAQ, GS_SET_RGBAQ(red, green, blue, alpha, 0x3F800000));
                    xglPacketAppendQuad(GS_FRAME_1, GS_SET_FRAME(D_004C8390.outlinePage, 8, 0, 0xFF000000));
                    xglPacketAppendQuad(GS_PRIM, GS_SET_PRIM(2, 1, 0, 0, 1, 0, 0, 0, 0));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][2]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][3]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][4]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][5]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][6]));
                    xglPacketAppendQuad(GS_XYZF2, SHATTER_XYZ(screen[column][1]));
                }
            }
        }
        xglPacketFinishTag();
        xglPacketSendDirect(0, 0);
    }

    if (gSceneLoadState != 2) {
        return;
    }
    drawRectOnPageColour(D_004C8390.glowSourcePage, 0, 0, 0, 0x100, 0xE0);
    drawAreaSetExtended(D_004C8390.outlinePage, 8, 0, D_004C8390.glowPage, 4, 0, 0x80, 3, 0, 0, 0x200, 0x1C0, 0, 0,
                        0x100, 0xE0);
    drawEnvSendRecordPacket();
    glowStep = 4.0f;
    for (row = 0; row < 5; row++) {
        int alpha = glowStep * (5 - row);
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 1, 1, 0x100, 0xE0,
                            row + 1, row + 1, row + 0x100, row + 0xE0);
        drawEnvSendRecordPacket();
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 0, 1, 0xFF, 0xE0,
                            -row, row + 1, 0xFF - row, row + 0xE0);
        drawEnvSendRecordPacket();
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 1, 0, 0x100, 0xDF,
                            row + 1, -row, row + 0x100, 0xDF - row);
        drawEnvSendRecordPacket();
        drawAreaSetExtended(D_004C8390.glowPage, 4, 0, D_004C8390.glowSourcePage, 4, 0, alpha, 1, 0, 0, 0xFF, 0xDF,
                            -row, -row, 0xFF - row, 0xDF - row);
        drawEnvSendRecordPacket();
    }
    drawAreaSetExtended(D_004C8390.glowSourcePage, 4, 0, D_004C8390.screenPage, 8, 0, 0x80, 1, 1, 1, 0xFF, 0xDF, 1, 1,
                        0x1FF, 0x1BF);
    drawEnvSendRecordPacket();
    drawAreaSetExtended(D_004C8390.glowSourcePage, 4, 0, D_004C8390.screenPage, 8, 0, 0x80, 1, 0, 0, 0xFF, 0xDF, -1, -1,
                        0x1FF, 0x1BF);
    drawEnvSendRecordPacket();
    drawAreaSetWindow(0x80, 1, 0, 0, 0x200, 0x1C0, 0, 0, 0x200, 0x1C0);
    func_001D55E8();
}
