#include "common.h"

#include "shared.h"
typedef int s32;

/* One slot instance block: emitter presets, read and written as the fields below. */
typedef struct SlotInstancePresets {
    u8 unk_00[0x28];
    f32 extentX;
    f32 extentY;
    f32 scale;
    f32 spread;
    f32 angle;
    f32 width;
    f32 height;
    s16 particleCount;
    s16 particleStep;
    s16 lifetime;
} SlotInstancePresets;

void *slotInstanceFetch(s32 slotNumber, s32 valueSelector, s32 summedLength);

/* Writes the default presets (extent, scale, spread, angle, size, count, step and lifetime) of the slot instance
   fetched with summed length 7. */
void slotInstancePresetsReset(s32 slotNumber) {
    SlotInstancePresets *presets;

    presets = slotInstanceFetch(slotNumber, 0, 7);
    presets->extentX = 6.0f;
    presets->extentY = 10.0f;
    presets->scale = 0.02f;
    presets->spread = 0.11f;
    presets->angle = 0.7853982f;
    presets->width = 5.0f;
    presets->height = 8.0f;
    presets->particleCount = 30;
    presets->particleStep = 2;
    presets->lifetime = 100;
}

void *slotBindingFetch(s32 slotNumber, s32 kind, s32 scalePercent, u32 flags);
void slotStateReset(void *binding);
void func_0027A138(void *binding);
void streamBeginRecord(s32 slotNumber, s32 unused1, s32 unused2, s32 mode);
void streamFlushNoop(void);
void func_0027B450(s32 slotNumber, s32 unused1, s32 unused2, s32 value);
void func_0027B590(void *stream20);
void streamPutFloat(f32 value);

typedef struct SlotStream {
    u8 unmodeled_00[0x20];
    u8 unmodeled_20[0x54 - 0x20];
    f32 scale54;
} SlotStream;

typedef struct SlotBinding {
    u32 bindingFlags;
    SlotStream *stream;
    u8 unmodeled_08[0x0E - 0x08];
    u16 flags0E;
} SlotBinding;

typedef struct SlotPresetBlock {
    u8 unmodeled_00[4];
    s32 presetFlagB;
    u8 unmodeled_08[0x50 - 0x08];
    s16 minusOne50;
} SlotPresetBlock;

typedef struct SlotPresetHeader {
    u8 unmodeled_00[0x14];
    s32 zeroPresetWord14;
    u8 unmodeled_18[0x31 - 0x18];
    u8 zeroPresetWord31;
    u8 zeroPresetWord32;
} SlotPresetHeader;

/* Resets the slot binding, then for kinds 1 to 14 emits the stream record of that kind. Flag bit 0 selects
   0x02000000 over 0x01000000 in the binding word and bit 1 sets 0x04000000. Returns 1 for kind 0 and for an
   emitted record, 0 otherwise. scalePercent is in hundredths. */
s32 slotStreamEmitInitial(s32 slotNumber, u32 kind, s32 scalePercent, u32 flags) {
    SlotBinding *binding;
    SlotPresetHeader *presetHeader;
    SlotPresetBlock *presetBlock;
    s32 streamMode;
    s32 recordId = 0;
    s32 emitRecord;
    s32 presetFlagB;

    streamMode = 0;
    presetFlagB = 0;
    binding = slotBindingFetch(slotNumber, kind, scalePercent, flags);
    slotStateReset(binding);
    func_0027A138(binding);
    emitRecord = 0;
    if (flags & 1) {
        binding->bindingFlags |= 0x02000000;
    } else {
        binding->bindingFlags |= 0x01000000;
    }
    if (flags & 2) {
        binding->bindingFlags |= 0x04000000;
    }
    switch (kind) {
    case 0:
        return 1;
    case 1:
        streamMode = 1;
        recordId = 30;
        emitRecord = 1;
        break;
    case 2:
        streamMode = 1;
        recordId = 32;
        emitRecord = 1;
        presetFlagB = 1;
        break;
    case 3:
        streamMode = 1;
        recordId = 32;
        emitRecord = 1;
        break;
    case 4:
        streamMode = 1;
        recordId = 33;
        emitRecord = 1;
        break;
    case 5:
        streamMode = 1;
        recordId = 33;
        emitRecord = 1;
        presetFlagB = 1;
        break;
    case 6:
        streamMode = 1;
        recordId = 31;
        emitRecord = 1;
        break;
    case 7:
        streamMode = 1;
        recordId = 34;
        emitRecord = 1;
        break;
    case 8:
        streamMode = 1;
        recordId = 35;
        emitRecord = 1;
        break;
    case 9:
        streamMode = 1;
        recordId = 35;
        emitRecord = 1;
        presetFlagB = 1;
        break;
    case 10:
        streamMode = 1;
        recordId = 36;
        emitRecord = 1;
        break;
    case 11:
        streamMode = 1;
        recordId = 37;
        emitRecord = 1;
        break;
    case 12:
        streamMode = 1;
        recordId = 38;
        emitRecord = 1;
        break;
    case 13:
        streamMode = 1;
        recordId = 39;
        emitRecord = 1;
        break;
    case 14:
        streamMode = 1;
        recordId = 40;
        break;
    default:
        break;
    }
    if (emitRecord != 0) {
        streamBeginRecord(slotNumber, 0, 0, streamMode);
        streamFlushNoop();
        func_0027B450(slotNumber, 0, 7, recordId);
        func_0027B590(binding->stream->unmodeled_20);
        streamPutFloat(binding->stream->scale54);
        streamPutFloat(scalePercent * 0.01f);
        streamFlushNoop();
        slotInstancePresetsReset(slotNumber);
        presetBlock = slotInstanceFetch(slotNumber, 0, 7);
        presetHeader = slotInstanceFetch(slotNumber, 0, 6);
        presetBlock->minusOne50 = -1;
        presetBlock->presetFlagB = presetFlagB;
        presetHeader->zeroPresetWord31 = 0;
        presetHeader->zeroPresetWord32 = 0;
        presetHeader->zeroPresetWord14 = 0;
        binding->flags0E = binding->flags0E | 1;
        return 1;
    }
    return 0;
}

/* The six setup words of one record (unk_00 to unk_14). */
typedef struct SlotSetupParams {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
    s32 unk_14;
} SlotSetupParams;

typedef union SlotVector {
    f32 f[4];
    s32 w[4];
} SlotVector;

typedef struct SlotVectorPair {
    SlotVector offsetVector;
    SlotVector angleVector;
} SlotVectorPair;

typedef union SlotPresetKindData {
    f32 f[6];
    s16 h[12];
    s32 w[6];
} SlotPresetKindData;

typedef struct SlotPreset {
    u8 unmodeled_00[0x28];
    f32 extentX;
    f32 extentY;
    f32 scale;
    f32 spread;
    f32 angle;
    f32 width;
    f32 height;
    s16 particleCount;
    s16 particleStep;
    s16 lifetime;
    u8 unmodeled_4A[6];
    SlotPresetKindData presetKindData;
} SlotPreset;

typedef struct SlotPresetEntryList {
    u8 unk_00[0x10];
    f32 unk_10;
    s32 unk_14;
    s16 unk_18[5];
    u8 unk_22[5];
    u8 unk_27[5];
    u8 unk_2C[5];
    u8 usedEntryCount;
} SlotPresetEntryList;

s32 rgDispModelGetActorId(s32 slotNumber);
s32 func_0027B638(s32 actorId);

#define PI 3.14159265f
#define TWO_PI 6.2831855f

/* Stores one setup record (kind 1 to 12) into the slot's preset block. givenParams says which optional words
   were given; kind 1 does nothing. */
s32 slotPresetApplySetup(s32 slotNumber, s32 kind, SlotSetupParams *setupParams, SlotSetupParams *givenParams) {
    switch (kind) {
    case 1:
        break;
    case 2: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);

        preset->extentX = setupParams->unk_00 * 0.01f;
        preset->extentY = setupParams->unk_04 * 0.01f;
        preset->scale = setupParams->unk_08 * 0.01f / 30.0f;
        preset->spread = setupParams->unk_0C * 0.01f / 30.0f;
        break;
    }
    case 3: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);

        preset->angle = setupParams->unk_00 * PI * 0.5f / 180.0f;
        preset->width = setupParams->unk_04 * 0.01f;
        preset->height = setupParams->unk_08 * 0.01f;
        break;
    }
    case 4: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);

        if (givenParams->unk_00) {
            preset->particleCount = setupParams->unk_00;
        } else {
            preset->particleCount = -1;
        }
        preset->particleStep = setupParams->unk_04;
        if (givenParams->unk_08) {
            preset->lifetime = setupParams->unk_08;
        } else {
            preset->lifetime = -1;
        }
        break;
    }
    case 5: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);
        SlotVector *vectorEntry = slotInstanceFetch(slotNumber, 0, 9);

        if (setupParams->unk_00 > 0) {
            vectorEntry = &vectorEntry[setupParams->unk_00 - 1];
            vectorEntry->f[0] = setupParams->unk_04 * 0.01f;
            vectorEntry->f[1] = setupParams->unk_08 * 0.01f;
            vectorEntry->f[2] = setupParams->unk_0C * 0.01f;
            if (preset->presetKindData.h[0] < setupParams->unk_00) {
                preset->presetKindData.h[0] = setupParams->unk_00;
            }
        }
        break;
    }
    case 6: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);
        SlotVectorPair *vectorPairSlot = slotInstanceFetch(slotNumber, 0, 9);

        if (setupParams->unk_00 > 0) {
            vectorPairSlot = &vectorPairSlot[setupParams->unk_00 - 1];
            vectorPairSlot->offsetVector.f[0] = setupParams->unk_04 * 0.01f;
            vectorPairSlot->offsetVector.f[1] = setupParams->unk_08 * 0.01f;
            vectorPairSlot->offsetVector.f[2] = setupParams->unk_0C * 0.01f;
            vectorPairSlot->angleVector.w[0] = 0;
            vectorPairSlot->angleVector.w[1] = 0;
            if (preset->presetKindData.h[0] < setupParams->unk_00) {
                preset->presetKindData.h[0] = setupParams->unk_00;
            }
        }
        break;
    }
    case 7: {
        SlotVectorPair *vectorPairSlot = slotInstanceFetch(slotNumber, 0, 9);

        if (setupParams->unk_00 > 0) {
            vectorPairSlot = &vectorPairSlot[setupParams->unk_00 - 1];
            vectorPairSlot->angleVector.w[0] = setupParams->unk_04;
            vectorPairSlot->angleVector.w[1] = setupParams->unk_08;
            vectorPairSlot->angleVector.f[2] = setupParams->unk_0C * 0.01f * PI / 180.0f;
            if (vectorPairSlot->angleVector.f[2] <= -PI) {
                vectorPairSlot->angleVector.f[2] += TWO_PI;
            }
            if (vectorPairSlot->angleVector.f[2] >= PI) {
                vectorPairSlot->angleVector.f[2] -= TWO_PI;
            }
        }
        break;
    }
    case 8: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);

        if (setupParams->unk_00 == 0) {
            preset->presetKindData.w[2] = 1000;
        } else {
            s32 actorId = func_0027B638(rgDispModelGetActorId(setupParams->unk_00));

            if (actorId == -1) {
                /* Kept: an unknown actor halts here (the original loops forever). */
                for (;;) {
                }
            }
            preset->presetKindData.w[2] = actorId;
        }
        preset->presetKindData.f[0] = setupParams->unk_04 * 0.01f;
        preset->presetKindData.f[1] = setupParams->unk_08 * 0.01f;
        break;
    }
    case 9: {
        f32 inner;
        f32 outer;
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);

        preset->presetKindData.f[0] = setupParams->unk_00 * 0.01f;
        preset->presetKindData.f[1] = setupParams->unk_04 * 0.01f;
        inner = setupParams->unk_08 * 0.01f;
        outer = setupParams->unk_0C * 0.01f;
        if (inner == outer) {
            preset->presetKindData.f[2] = inner;
            preset->presetKindData.f[3] = 1.0f;
        } else if (inner <= outer) {
            preset->presetKindData.f[2] = outer;
            preset->presetKindData.f[3] = inner / outer;
        } else {
            preset->presetKindData.f[2] = outer;
            preset->presetKindData.f[3] = (inner - outer + outer) / outer;
        }
        preset->presetKindData.f[4] = setupParams->unk_10 * 0.01f * PI / 180.0f;
        preset->presetKindData.w[5] = setupParams->unk_14;
        break;
    }
    case 10: {
        SlotPreset *preset = slotInstanceFetch(slotNumber, 0, 7);

        preset->presetKindData.h[6] = setupParams->unk_00 * 0.1f;
        preset->presetKindData.h[7] = setupParams->unk_04 * 0.1f;
        preset->presetKindData.h[8] = setupParams->unk_08;
        preset->presetKindData.f[5] = setupParams->unk_0C * 0.01f;
        break;
    }
    case 11: {
        s32 i;
        SlotPresetEntryList *presetEntryList = slotInstanceFetch(slotNumber, 0, 6);

        presetEntryList->unk_10 = setupParams->unk_00 * 0.01f;
        presetEntryList->unk_14 = setupParams->unk_04;
        presetEntryList->usedEntryCount = 0;
        for (i = 0; i < 5; i++) {
            presetEntryList->unk_22[i] = 0;
            presetEntryList->unk_18[i] = 0;
        }
        break;
    }
    case 12: {
        s32 entryCursor;
        SlotPresetEntryList *presetEntryList = slotInstanceFetch(slotNumber, 0, 6);

        entryCursor = 5;
        if (presetEntryList->usedEntryCount < 5) {
            entryCursor = presetEntryList->usedEntryCount;
            presetEntryList->usedEntryCount = entryCursor + 1;
        }
        presetEntryList->unk_2C[entryCursor] = setupParams->unk_00;
        if (givenParams->unk_00) {
            presetEntryList->unk_27[entryCursor] = setupParams->unk_04;
        } else {
            presetEntryList->unk_27[entryCursor] = -1;
        }
        break;
    }
    }
    return 1;
}

/* Returns the record size for each kind (4, 5, 6 or 8); 0 for an unknown kind. */
s32 slotSetupRecordSize(s32 reserved, s32 kind) {
    switch (kind) {
    case 1:
    case 11:
    case 12:
        return 4;
    case 3:
    case 4:
    case 8:
        return 5;
    case 2:
    case 5:
    case 6:
    case 7:
    case 10:
        return 6;
    case 9:
        return 8;
    default:
        return 0;
    }
}

s32 func_00286880(void *obj, s32 value);
s32 func_002869C0(void *obj, f32 scale, s32 value);

typedef struct SlotPlaybackRecord {
    s32 recordSlotNo;
    s32 level;
    s32 count;
    s32 span;
    s32 enabled;
} SlotPlaybackRecord;

typedef struct SlotPlayback {
    u8 unmodeled_00[0x4EC];
    u32 flags;
    f32 position;
    u8 unmodeled_4F4[0x570 - 0x4F4];
    u8 motionState[4];
    f32 rate;
    u8 unmodeled_578[0x580 - 0x578];
    s32 phaseCount;
    s32 phaseSpan;
} SlotPlayback;

void tiMRStateSetPosition(void *state, f32 position);

/* Applies a slot record to a playback: the rate from level, the phase count, position and span, and the enabled bit in flags. */
void slotPlaybackApplyRecord(SlotPlayback *playback, u8 *slotBytes, SlotPlaybackRecord *playbackRecord) {
    f32 scale;
    u32 flags;

    if (playbackRecord->enabled != 0) {
        scale = func_00286880(playback, slotBytes[playbackRecord->recordSlotNo]);
        if (10.0f < scale) {
            scale = 10.0f;
        }
    } else {
        scale = 0.0f;
    }
    if (playbackRecord->recordSlotNo >= 50) {
        func_002869C0(playback, scale, playbackRecord->recordSlotNo);
    } else {
        func_002869C0(playback, scale, slotBytes[playbackRecord->recordSlotNo]);
    }
    playback->rate = playbackRecord->level * 0.01f;
    if (playbackRecord->count != 0) {
        playback->phaseCount = playbackRecord->count - 1;
        tiMRStateSetPosition(playback->motionState, playbackRecord->count - 1.0f);
        playback->position = playbackRecord->count - 1.0f;
    }
    if (playbackRecord->span != 0) {
        playback->phaseSpan = playbackRecord->span - 1;
    }
    flags = playback->flags & ~1u;
    playback->flags = flags;
    if (playbackRecord->enabled != 0) {
        playback->flags = flags & ~2u;
    } else {
        playback->flags = flags | 2u;
    }
}

typedef struct SlotIndexedByte {
    u32 recordSlotNo;
    u8 updatedByte;
} SlotIndexedByte;

s32 func_00286A60(void *obj, s32 value);

/* Stores the new byte at recordSlotNo (below 32). When func_00286A60 accepts the old byte, calls func_002869C0 with 0.0f and the new byte. */
void slotPlaybackApplyIndexedByte(SlotPlayback *playback, u8 *slotBytes, SlotIndexedByte *indexedByte) {
    u8 oldValue;

    if (indexedByte->recordSlotNo < 32) {
        oldValue = slotBytes[indexedByte->recordSlotNo];
        slotBytes[indexedByte->recordSlotNo] = indexedByte->updatedByte;
        if (func_00286A60(playback, oldValue)) {
            /* kept: the do/while(0) decides the emitted order. */
            do {
                func_002869C0(playback, 0.0f, slotBytes[indexedByte->recordSlotNo]);
            } while (0);
        }
    }
}

typedef struct SlotDrawSpec {
    s32 kind;
    s32 key;
    s32 unk_08;
    s32 unk_0C;
    union {
        s32 unk_10;
        u16 unk_10Half;
    } tail;
    s32 unk_14;
    s32 unk_18;
    s32 unk_1C;
} SlotDrawSpec;

typedef union SlotVec3 {
    f32 v[3];
} SlotVec3;

typedef union SlotHalfBits {
    u16 unsignedValue;
    s16 signedValue;
} SlotHalfBits;

typedef struct SlotDrawState {
    u8 unmodeled_00[8];
    u16 flags;
    s16 actor;
    u16 unk_0C;
    SlotHalfBits unk_0E;
    SlotVec3 vecA;
    u8 unmodeled_1C[4];
    SlotVec3 vecB;
} SlotDrawState;

/* Fills the draw state from a spec: kind 0 sets vector A and the actor, kind 1 sets vector B and the flag bits. */
void slotDrawStateFromSpec(SlotDrawState *drawState, SlotDrawSpec *spec) {
    s32 kind;
    s32 key;

    kind = spec->kind;
    switch (kind) {
    case 0: {
        SlotVec3 *vec = &drawState->vecA;

        drawState->flags |= 1;
        key = spec->key;
        if (key == -1) {
            drawState->actor = key;
        } else if (key == 0) {
            drawState->actor = 1000;
        } else {
            drawState->actor = func_0027B638(rgDispModelGetActorId(key));
        }
        vec->v[0] = spec->unk_08 * 0.01f;
        vec->v[1] = spec->unk_0C * 0.01f;
        vec->v[2] = spec->tail.unk_10 * 0.01f;
        break;
    }
    case 1: {
        SlotVec3 *vec = &drawState->vecB;

        vec->v[0] = spec->key * 0.01f;
        vec->v[1] = spec->unk_08 * 0.01f;
        vec->v[2] = spec->unk_0C * 0.01f;
        drawState->unk_0E.unsignedValue = 0;
        drawState->unk_0C = spec->tail.unk_10;
        drawState->flags = 0;
        if (kind == spec->unk_14) {
            drawState->flags = 0x10;
        }
        if (spec->unk_18 == 0) {
            drawState->flags = drawState->flags | 2;
        } else if (kind == spec->unk_18) {
            drawState->flags = drawState->flags | 4;
        } else if (spec->unk_18 == 2) {
            drawState->flags = drawState->flags | 8;
        }
        if (spec->unk_1C == 1) {
            drawState->flags = drawState->flags | 0x20;
        } else if (spec->unk_1C == 2) {
            drawState->flags = drawState->flags | 0x60;
        } else if (spec->unk_1C == 3) {
            drawState->flags = drawState->flags | 0xa0;
        }
        break;
    }
    }
}
