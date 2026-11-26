#include "common.h"
#include "shared.h"
#include "main/sound_driver.h"

/* Kept as its own helper: merging it with the QuadwordCopy macro renumbers the compiler's labels. */
static inline void QuadwordCopyT0(Vector4 *destination, Vector4 *source) {
    __asm__ __volatile__("lq $2,0(%1)\n sq $2,0(%0)" : : "r"(destination),"r"(source) : "$2","memory");
}

typedef int s32;

typedef short s16;

typedef unsigned short u16;

typedef float f32;

typedef unsigned char u8;

typedef signed char s8;

/* The sound cue state at gSoundCueStorage: the listener, a list of 64 cue records (ambient tracks, fades,
 * positional sources, area zones, sound slots) and the bookkeeping of the current ambient track. */

/* Kinds of SoundCueRecord. */
enum {
    SOUND_CUE_FREE = 0,
    SOUND_CUE_TRACK = 2,         /* a started voice followed until it ends */
    SOUND_CUE_VOLUME_FADE = 3,
    SOUND_CUE_PAN_FADE = 4,
    SOUND_CUE_POINT_SOURCE = 5,  /* volume and pan follow the distance to a point */
    SOUND_CUE_LINE_SOURCE = 6,   /* the same for the nearest point of a segment */
    SOUND_CUE_AREA_AMBIENT = 7,  /* ambient voice of a map area */
    SOUND_CUE_SOUND_SLOT = 8,    /* one of the numbered sound slots */
    SOUND_CUE_LISTENER = 9       /* where the listener is placed */
};

/* Operations of soundCueCommand besides pushing a record of the given kind. */
enum {
    SOUND_CUE_OP_RELEASE = 1,
    SOUND_CUE_OP_LISTENER = 9,
    SOUND_CUE_OP_REFRESH_LISTENER = 10,
    SOUND_CUE_OP_SELECT_BANK = 11
};

/* One 0x30-byte cue record; the kind selects the view of the union at +0x0C. */
typedef struct SoundCueRecord {
    u8 kind;                     /* +0x00 */
    u8 soundNumber;              /* +0x01 */
    u16 voiceHandle;             /* +0x02 */
    u16 recordId;                /* +0x04: n + 0x200 for a pushed record */
    u8 unk_06[6];
    union {
        s32 slotStartArg;        /* kind 8: handed to soundSlotStartNumbered */
        struct {
            s32 targetId;        /* +0x0C: slot object kind, or -3 for the position below */
            Vector4 pos;         /* +0x10: w = audible radius */
        } listener;
        struct {
            u16 startVolume;     /* +0x0C */
            u16 endVolume;       /* +0x0E */
            s16 fadeTick;        /* +0x10: starts negative, the fade runs once it is above 0 */
            s16 durationTicks;   /* +0x12 */
            u8 stopWhenDone;     /* +0x14 */
        } fade;
        struct {
            u8 volumeLow;        /* +0x0C */
            u8 volumeHigh;       /* +0x0D */
            u8 isMono;           /* +0x0E */
            u8 voicePreStarted;  /* +0x0F */
            Vector4 pos;         /* +0x10: w = radius */
            Vector4 dir;         /* +0x20: x = 256 / radius for a point, the axis for a line */
        } positional;
        struct {
            u8 soundNumber;      /* +0x0C */
            u8 areaId;           /* +0x0D */
            u8 voiceLevel;       /* +0x0E */
            u8 voicePan;         /* +0x0F */
            u8 zoneFlags;        /* +0x10: bit 0 fades the voice in */
            u8 isActive;         /* +0x11 */
        } zone;
    } u;
} SoundCueRecord;

/* One word of the voice table (gSoundRecordBlock +0x530), as bytes or as a whole. */
typedef union SoundVoiceWord {
    s32 word;
    struct {
        u8 soundNumber;
        u8 voiceHandleByte;
        u8 volume;
        u8 pan;
    } voiceByteView;
} SoundVoiceWord;

/* A voice saved by soundCueSnapshotSave: the voice table word with the handle byte replaced. */
typedef struct SavedVoice {
    u8 soundNumber;
    u8 voiceHandleByte;  /* 0xFF marks a sound slot (soundNumber - 1 is the slot) */
    u8 volume;
    u8 pan;
} SavedVoice;

/* The same four bytes as a halfword-aligned element: the save loop addresses the table through it
 * (kept apart: the alignment of the element type decides the induction pointer of each loop). */
typedef struct SavedVoiceHalfAligned {
    u8 soundNumber;
    u8 voiceHandleByte;
    s16 volumeAndPan;
} SavedVoiceHalfAligned;

/* The saved voices as bytes (restore) or as halfword-aligned elements (save). */
typedef union SavedVoiceTable {
    SavedVoice voices[32];
    SavedVoiceHalfAligned halfAlignedVoices[32];
} SavedVoiceTable;

/* The cue records and the bookkeeping that follows them. */
typedef struct SoundCueList {
    SoundCueRecord cueRecords[64];   /* +0x20 */
    u8 cueUpdateEnabled;             /* +0xC20 */
    u8 listenerMode;                 /* +0xC21: 0 follows the player */
    s16 listenerTargetId;            /* +0xC22: -1 when no listener record is set */
    f32 audibleRadius;               /* +0xC24 */
    s8 activeAreaId;                 /* +0xC28 */
    s8 probedAreaId;                 /* +0xC29 */
    u16 ambientHandle;               /* +0xC2A */
    u8 ambientFlags;                 /* +0xC2C */
    u8 ambientSoundNumber;           /* +0xC2D */
    u8 unk_C2E[2];
    SavedVoiceTable savedVoices;     /* +0xC30 */
    s32 savedSlotStartArgs[4];       /* +0xCB0 */
    u8 voiceGroupTags[6];            /* +0xCC0 */
    u8 selectedBank;                 /* +0xCC6 */
    u8 unk_CC7;
    u8 slotGroupTags[4];             /* +0xCC8 */
    u8 unk_CCC[4];
} SoundCueList;

typedef struct SoundCueState {
    AlignedVector4 listenerPos;      /* +0x00 */
    AlignedVector4 panReferencePos;  /* +0x10 */
    SoundCueList cues;               /* +0x20 */
} SoundCueState;

extern SoundCueState gSoundCueStorage;

#define gSoundCue (gSoundCueStorage)

/* The slot object found by RgDispModelFindByKey: state bits at +0x00, position at +0x40. */
typedef struct SlotObjectProbe {
    s32 slotStateBits;
    u8 unmodeled_04[0x3C];
    Vector4 position;
} SlotObjectProbe;

/* The studio camera block: its position at +0xD0. */
typedef struct CameraPositionView {
    u8 unmodeled_00[0xD0];
    Vector4 position;
} CameraPositionView;

SlotObjectProbe *RgDispModelFindByKey();
s32 mapStaticObjectProbe(Vector4 *position, f32 *outDistance);
CameraPositionView *xglStudioGetCamera2();
void xglVectorInter(Vector4 *interpolated, const Vector4 *startVector, const Vector4 *endVector, f32 ratio);

/* Places the listener: at the player (or the listener record, or the camera) and keeps the area
 * of the player; the pan reference sits halfway between camera and player. */
void soundCueListenerUpdate() {
    Vector4 playerPos;
    s32 i;
    SlotObjectProbe *player = RgDispModelFindByKey(0);
    if (player) {
        s32 areaId;
        QuadwordCopyT0(&playerPos, &player->position);
        areaId = mapStaticObjectProbe(&playerPos, &playerPos.w);
        if (gSoundCue.cues.probedAreaId != areaId) {
            gSoundCue.cues.activeAreaId = areaId;
        }
        gSoundCue.cues.probedAreaId = areaId;
    } else {
        gSoundCue.cues.activeAreaId = 0;
    }
    {
        CameraPositionView *camera = xglStudioGetCamera2(0);
        gSoundCue.cues.audibleRadius = 10.0f;
        if (player && (player->slotStateBits & 0x100)) {
            gSoundCue.cues.audibleRadius = 40.0f;
        }
        gSoundCue.cues.listenerTargetId = -1;
        if (gSoundCue.cues.listenerMode == 0 && player) {
            QuadwordCopyT0(&gSoundCue.listenerPos, &player->position);
            xglVectorInter(&gSoundCue.panReferencePos, &camera->position, &player->position, 0.5f);
        } else {
            Vector4 *listenerSource = &camera->position;
            for (i = 0; i < 64; i++) {
                SoundCueRecord *cueRecord = &gSoundCue.cues.cueRecords[i];
                if (cueRecord->kind == SOUND_CUE_LISTENER) {
                    if (cueRecord->u.listener.targetId >= 0) {
                        listenerSource = &RgDispModelFindByKey(cueRecord->u.listener.targetId)->position;
                    } else if (cueRecord->u.listener.targetId == -3) {
                        listenerSource = &cueRecord->u.listener.pos;
                    }
                    gSoundCue.cues.audibleRadius = cueRecord->u.listener.pos.w;
                    gSoundCue.cues.listenerTargetId = cueRecord->u.listener.targetId;
                    break;
                }
            }
            gSoundCue.listenerPos.x = gSoundCue.panReferencePos.x = listenerSource->x;
            gSoundCue.listenerPos.y = gSoundCue.panReferencePos.y = listenerSource->y;
            gSoundCue.listenerPos.z = gSoundCue.panReferencePos.z = listenerSource->z;
        }
    }
    gSoundCue.listenerPos.w = gSoundCue.panReferencePos.w = 1.0f;
}

void func_0018E698(int, int);

/* Cue list entry point: pushes a record of the given kind, or runs one of the SOUND_CUE_OP_*
 * operations. Returns the record (0 for the operations that return none). */
SoundCueRecord *soundCueCommand(int cueOperation, int operationArg) {
    SoundCueRecord *cueRecord;
    u16 voiceHandle;
    int i;
    int usedRecordCount;

    if (cueOperation == SOUND_CUE_OP_RELEASE && operationArg != 0) {
        cueRecord = gSoundCue.cues.cueRecords;
        i = 0;
        do {
            voiceHandle = cueRecord->voiceHandle;
            if (voiceHandle == operationArg || cueRecord->recordId == operationArg) {
                cueRecord->kind = SOUND_CUE_FREE;
                if (voiceHandle != 0) {
                    func_0018F098(voiceHandle);
                }
                return 0;
            }
            i++;
            cueRecord++;
        } while (i < 64);
        func_0018F098(operationArg);
        return 0;
    }
    if (cueOperation == SOUND_CUE_OP_LISTENER) {
        if (operationArg >= 0) {
            gSoundCue.cues.listenerMode = operationArg;
            return 0;
        }
        cueRecord = gSoundCue.cues.cueRecords;
        for (i = 0; i < 64; i++) {
            if (cueRecord->kind == SOUND_CUE_LISTENER) {
                return cueRecord;
            }
            cueRecord++;
        }
    }
    if (cueOperation == SOUND_CUE_OP_REFRESH_LISTENER) {
        soundCueListenerUpdate();
        return 0;
    }
    if (cueOperation == SOUND_CUE_OP_SELECT_BANK) {
        if (gSoundCue.cues.selectedBank != operationArg) {
            gSoundCue.cues.selectedBank = operationArg;
            func_0018E698(operationArg + 0x8000, 8);
        }
        return 0;
    }
    cueRecord = gSoundCue.cues.cueRecords;
    usedRecordCount = 0;
    if (cueRecord->kind != SOUND_CUE_FREE) {
        do {
            usedRecordCount++;
            cueRecord++;
        } while (usedRecordCount < 64 && cueRecord->kind != SOUND_CUE_FREE);
    }
    cueRecord->kind = cueOperation;
    cueRecord->voiceHandle = operationArg;
    cueRecord->recordId = usedRecordCount + 0x200;
    return cueRecord;
}


/* Fades the current ambient voice out over 10 ticks when its flags ask for it, otherwise stops it. */
void soundCueAmbientFadeOut(void) {
    SoundCueRecord *fadeRecord;

    if (gSoundCue.cues.ambientHandle != 0) {
        if ((gSoundCue.cues.ambientFlags & 2) != 0) {
            unsigned char startVolume;
            unsigned char stopWhenDone;

            fadeRecord = soundCueCommand(SOUND_CUE_VOLUME_FADE, gSoundCue.cues.ambientHandle);
            startVolume = gSoundCueVolumeTable[(unsigned char) gSoundCue.cues.ambientHandle * 4];
            stopWhenDone = 1;
            fadeRecord->u.fade.durationTicks = 10;
            fadeRecord->u.fade.startVolume = startVolume;
            fadeRecord->u.fade.stopWhenDone = stopWhenDone;
            fadeRecord->u.fade.fadeTick = 0;
            fadeRecord->u.fade.endVolume = 0;
        } else {
            func_0018F098(gSoundCue.cues.ambientHandle);
        }
        gSoundCue.cues.ambientHandle = 0;
        gSoundCue.cues.ambientSoundNumber = 0;
    }
}

/* The scene root block: the camera yaw at +0xA4 (a float, read the same way by tu075). */
typedef struct SceneRootView {
    u8 unmodeled_00[0xA4];
    float cameraYaw;
} SceneRootView;

extern SceneRootView *gRgSceneRoot[];

#define TWO_PI 6.2831855f

float xglAtan2(float, float);

/* Turns a horizontal offset (listener to source, rotated by the camera yaw) into a pan angle
 * from 0 (straight ahead) to 0x7F (behind). */
int soundPanAngleFromOffset(float offsetX, float offsetZ) {
    float angle;
    int panAngle;

    angle = xglAtan2(offsetZ, offsetX) - gRgSceneRoot[0]->cameraYaw;
    while (angle > TWO_PI) {
        angle -= TWO_PI;
    }
    while (angle < 0.0f) {
        angle += TWO_PI;
    }
    panAngle = (int) (angle * 256.0f / TWO_PI);
    if (panAngle > 0x80) {
        panAngle = 0x100 - panAngle;
    }
    if (panAngle > 0x7F) {
        panAngle = 0x7F;
    }
    return panAngle;
}

int func_0018EDD8(int voice, int volume, int pan);

/* kept as an inline helper: spelling the test inline changes the emitted order */
static inline int soundCueHasVoice(SoundCueRecord *cueRecord) {
    return cueRecord->voiceHandle != 0;
}

/* Keeps the voice of a positional record in step with its volume and pan: when the record does
 * not own a pre-started voice, a voice is started as soon as the volume is above 0 and stopped when it is 0. */
void soundCueVoiceSync(SoundCueRecord *cueRecord, int voicePreStarted, int volume, int pan) {
    if (!voicePreStarted) {
        if (!volume) {
            if (cueRecord->voiceHandle) {
                func_0018F098(cueRecord->voiceHandle);
                cueRecord->voiceHandle = 0;
            }
        } else {
            if (!soundCueHasVoice(cueRecord)) {
                cueRecord->voiceHandle = func_0018EDD8(cueRecord->soundNumber, volume, pan);
            }
        }
    }
    if (soundCueHasVoice(cueRecord)) {
        func_0018EDD8(cueRecord->voiceHandle, volume, pan);
    }
}

/* The voice table of the sound block: one SoundVoiceWord per voice, from +0x530. The cue code
 * indexes it with a handle byte. */
typedef struct SoundVoiceView {
    u8 unmodeled_00[0x530];
    SoundVoiceWord voices[64];
} SoundVoiceView;

extern SoundVoiceView gSoundRecordBlock;

#define gSoundVoices (gSoundRecordBlock.voices)

/* The game loop runtime block: its flag word at +0x00 (bit 0x100 lets the positional cues run). */
typedef struct GameLoopFlagsView {
    s32 loopFlagBits;
    u8 unmodeled_04[0x200];
} GameLoopFlagsView;

extern GameLoopFlagsView gGameLoopRuntimeStorage;
int soundSlotStatus(int);
float xglPointLength(void *, void *);

/* Updates every cue record of the list once per frame: ambient tracks end with their voice, fades
 * step, positional sources follow the listener and area zones start or replace the ambient voice. */
void soundCueUpdate() {
    int i;
    int fadeTickCount;
    int fadeValue;
    int hasAmbientCue;
    int ambientInArea;
    if (gSoundCue.cues.cueUpdateEnabled == 0) {
        return;
    }
    soundCueListenerUpdate();
    {
        unsigned short ambientHandle = gSoundCue.cues.ambientHandle;
        if (ambientHandle != 0 && gSoundVoices[ambientHandle & 0xff].voiceByteView.soundNumber == 0) {
            gSoundCue.cues.ambientHandle = 0;
        }
    }
    hasAmbientCue = 0;
    ambientInArea = 0;
    for (i = 0; i < 64; i++) {
        SoundCueRecord *cueRecord = &gSoundCue.cues.cueRecords[i];
        int cueKind;
        cueKind = cueRecord->kind;
        switch (cueKind) {
        case SOUND_CUE_TRACK: {
            if (cueRecord->voiceHandle == 0 || gSoundVoices[(cueRecord->voiceHandle & 0xff)].voiceByteView.soundNumber == 0) {
                cueRecord->kind = SOUND_CUE_FREE;
                gSoundCue.cues.ambientHandle = 0;
                gSoundCue.cues.ambientSoundNumber = 0;
            } else {
                if (gSoundCue.cues.ambientHandle != cueRecord->voiceHandle) {
                    if (gSoundCue.cues.ambientHandle != 0 &&
                        cueRecord->u.zone.soundNumber != gSoundVoices[(gSoundCue.cues.ambientHandle & 0xff)].voiceByteView.soundNumber) {
                        soundCueAmbientFadeOut();
                    }
                    gSoundCue.cues.ambientHandle = cueRecord->voiceHandle;
                    gSoundCue.cues.ambientSoundNumber = cueRecord->u.zone.soundNumber;
                }
                hasAmbientCue = 1;
                ambientInArea = 1;
            }
            break;
        }
        case SOUND_CUE_VOLUME_FADE:
        case SOUND_CUE_PAN_FADE:
            if (gSoundVoices[(cueRecord->voiceHandle & 0xff)].voiceByteView.soundNumber == 0) {
                cueRecord->kind = SOUND_CUE_FREE;
                break;
            }
            if (++cueRecord->u.fade.fadeTick <= 0) {
                break;
            }
            fadeTickCount = cueRecord->u.fade.fadeTick;
            if (cueRecord->u.fade.durationTicks < fadeTickCount) {
                cueRecord->kind = SOUND_CUE_FREE;
                fadeValue = cueRecord->u.fade.endVolume;
                if (cueKind == SOUND_CUE_VOLUME_FADE && cueRecord->u.fade.stopWhenDone) {
                    func_0018F098(cueRecord->voiceHandle);
                    break;
                }
            } else {
                fadeValue = cueRecord->u.fade.startVolume +
                            ((((cueRecord->u.fade.endVolume - cueRecord->u.fade.startVolume) << 8) * fadeTickCount /
                              cueRecord->u.fade.durationTicks) >> 8);
            }
            switch (cueKind) {
            case SOUND_CUE_VOLUME_FADE:
                func_0018EDD8(cueRecord->voiceHandle, fadeValue, -1);
                break;
            case SOUND_CUE_PAN_FADE:
                func_0018EDD8(cueRecord->voiceHandle, -1, fadeValue);
                break;
            }
            break;
        case SOUND_CUE_POINT_SOURCE: {
            int volume;
            int pan;
            float distance;
            if (cueRecord->u.positional.voicePreStarted != 0 && gSoundVoices[(cueRecord->voiceHandle & 0xff)].word == 0) {
                cueRecord->kind = SOUND_CUE_FREE;
                break;
            }
            if (!(gGameLoopRuntimeStorage.loopFlagBits & 0x100)) {
                break;
            }
            distance = xglPointLength(&gSoundCue.listenerPos, &cueRecord->u.positional.pos);
            pan = -1;
            volume = 0;
            if (distance < cueRecord->u.positional.pos.w) {
                volume = cueRecord->u.positional.volumeLow +
                         (((cueRecord->u.positional.volumeHigh - cueRecord->u.positional.volumeLow) *
                           (int) (distance * cueRecord->u.positional.dir.x)) >> 8);
                if (cueRecord->u.positional.isMono == 0) {
                    pan = soundPanAngleFromOffset(-(cueRecord->u.positional.pos.x - gSoundCue.panReferencePos.x),
                                        cueRecord->u.positional.pos.z - gSoundCue.panReferencePos.z);
                }
            }
            soundCueVoiceSync(cueRecord, cueRecord->u.positional.voicePreStarted, volume, pan);
            break;
        }
        case SOUND_CUE_LINE_SOURCE: {
            AlignedVector4 segmentProbe[2];
            int volume;
            int pan;
            float nearestDistance;
            if (cueRecord->u.positional.voicePreStarted != 0 && gSoundVoices[(cueRecord->voiceHandle & 0xff)].word == 0) {
                cueRecord->kind = SOUND_CUE_FREE;
                break;
            }
            if (!(gGameLoopRuntimeStorage.loopFlagBits & 0x100)) {
                break;
            }
            asm volatile("lqc2 vf2,0(%1)\n"
                         "lqc2 vf3,0(%2)\n"
                         "lqc2 vf4,0(%3)\n"
                         "vsub.xyz vf8,vf3,vf2\n"
                         "vmul.xyz vf1,vf8,vf8\n"
                         "vaddy.x vf1,vf1,vf1y\n"
                         "vaddz.x vf1,vf1,vf1z\n"
                         "vmul.xyz vf5,vf8,vf4\n"
                         "vaddy.x vf5,vf5,vf5y\n"
                         "vaddz.x vf5,vf5,vf5z\n"
                         "vmul.x vf6,vf5,vf5\n"
                         "vsub.x vf6,vf1,vf6\n"
                         "vsqrt Q,vf6x\n"
                         "vadda.xyz ACC,vf0,vf2\n"
                         "vmaddw.xyz vf10,vf4,vf4w\n"
                         "vaddx.y vf5,vf5,vf5x\n"
                         "vaddz.y vf1,vf5,vf5z\n"
                         "vsub.xyz vf9,vf10,vf3\n"
                         "vmul.xyz vf7,vf9,vf4\n"
                         "vaddx.y vf7,vf7,vf7x\n"
                         "vaddz.y vf5,vf7,vf7z\n"
                         "vmul.y vf1,vf1,vf5\n"
                         "vaddq.x vf1,vf0,Q\n"
                         "vmul.xyz vf5,vf8,vf8\n"
                         "vmul.xyz vf6,vf9,vf9\n"
                         "vaddy.x vf5,vf5,vf5y\n"
                         "vaddy.x vf6,vf6,vf6y\n"
                         "vaddz.x vf5,vf5,vf5z\n"
                         "vaddz.x vf6,vf6,vf6z\n"
                         "vsqrt Q,vf5x\n"
                         "vwaitq \n"
                         "vaddq.z vf1,vf0,Q\n"
                         "vsqrt Q,vf6x\n"
                         "vsub.z vf5,vf0,vf4\n"
                         "vmulz.x vf5,vf8,vf5z\n"
                         "vmulx.z vf5,vf8,vf4x\n"
                         "vaddz.x vf5,vf5,vf5z\n"
                         "vmulx.w vf10,vf0,vf5x\n"
                         "vwaitq \n"
                         "vmulq.w vf1,vf0,Q\n"
                         "sqc2 vf1,0(%0)\n"
                         "sqc2 vf10,16(%0)\n"
                         :
                         : "r"(segmentProbe), "r"(&cueRecord->u.positional.pos), "r"(&gSoundCue.listenerPos), "r"(&cueRecord->u.positional.dir)
                         : "memory");
            volume = 0;
            pan = -1;
            nearestDistance = 0.0f;
            if (segmentProbe[0].x < cueRecord->u.positional.pos.w && segmentProbe[0].y >= 0.0f) {
                nearestDistance = segmentProbe[0].x;
                if (cueRecord->u.positional.isMono == 0) {
                    {
                        float axisX;
                        float axisZ;
                        if (segmentProbe[1].w < 0.0f) {
                            axisX = cueRecord->u.positional.dir.x;
                            axisZ = cueRecord->u.positional.dir.z;
                        } else {
                            axisX = -cueRecord->u.positional.dir.x;
                            axisZ = -cueRecord->u.positional.dir.z;
                        }
                        pan = soundPanAngleFromOffset(axisZ, axisX);
                    }
                }
            } else if (segmentProbe[0].z < cueRecord->u.positional.pos.w) {
                nearestDistance = segmentProbe[0].z;
                if (cueRecord->u.positional.isMono == 0) {
                    pan = soundPanAngleFromOffset(-(cueRecord->u.positional.pos.x - gSoundCue.listenerPos.x),
                                        cueRecord->u.positional.pos.z - gSoundCue.listenerPos.z);
                }
            } else if (segmentProbe[0].w < cueRecord->u.positional.pos.w) {
                nearestDistance = segmentProbe[0].w;
                if (cueRecord->u.positional.isMono == 0) {
                    pan = soundPanAngleFromOffset(-(segmentProbe[1].x - gSoundCue.listenerPos.x),
                                        segmentProbe[1].z - gSoundCue.listenerPos.z);
                }
            }
            if (nearestDistance > 0.0f) {
                volume = cueRecord->u.positional.volumeLow +
                         (((cueRecord->u.positional.volumeHigh - cueRecord->u.positional.volumeLow) *
                           (int) (nearestDistance * 256.0f / cueRecord->u.positional.pos.w)) >> 8);
            }
            soundCueVoiceSync(cueRecord, cueRecord->u.positional.voicePreStarted, volume, pan);
            break;
        }
        case SOUND_CUE_AREA_AMBIENT: {
            int ambientVoiceHandle;
            SoundCueRecord *fadeRecord;
            int startVolume;
            if (!(gGameLoopRuntimeStorage.loopFlagBits & 0x100)) {
                break;
            }
            hasAmbientCue = 1;
            if (cueRecord->u.zone.areaId != gSoundCue.cues.probedAreaId) {
                break;
            }
            ambientInArea = 1;
            if (cueRecord->u.zone.isActive == 0 && cueRecord->u.zone.areaId != gSoundCue.cues.activeAreaId) {
                break;
            }
            if (gSoundCue.cues.ambientSoundNumber != cueRecord->u.zone.soundNumber) {
                soundCueAmbientFadeOut();
                startVolume = cueRecord->u.zone.voiceLevel;
                if (cueRecord->u.zone.zoneFlags & 1) {
                    startVolume = 0;
                }
                ambientVoiceHandle = func_0018EDD8(cueRecord->u.zone.soundNumber, startVolume, cueRecord->u.zone.voicePan);
                gSoundCue.cues.ambientHandle = ambientVoiceHandle;
                gSoundCue.cues.ambientFlags = cueRecord->u.zone.zoneFlags;
                gSoundCue.cues.ambientSoundNumber = cueRecord->u.zone.soundNumber;
                if (cueRecord->u.zone.zoneFlags & 1) {
                    fadeRecord = soundCueCommand(SOUND_CUE_VOLUME_FADE, ambientVoiceHandle);
                    fadeRecord->u.fade.startVolume = gSoundVoices[ambientVoiceHandle & 0xff].voiceByteView.volume;
                    fadeRecord->u.fade.endVolume = cueRecord->u.zone.voiceLevel;
                    fadeRecord->u.fade.fadeTick = 0;
                    fadeRecord->u.fade.durationTicks = 10;
                    fadeRecord->u.fade.stopWhenDone = 0;
                }
            }
            cueRecord->u.zone.isActive = 0;
            break;
        }
        case SOUND_CUE_SOUND_SLOT:
            if (soundSlotStatus(cueRecord->voiceHandle) >= 5) {
                cueRecord->kind = SOUND_CUE_FREE;
            }
            break;
        case SOUND_CUE_FREE:
        case 1:
        case SOUND_CUE_LISTENER:
            break;
        }
    }
    if (hasAmbientCue && !ambientInArea) {
        soundCueAmbientFadeOut();
    }
}


/* Snapshots the voices the cue list depends on into savedVoices (and the start argument of each
 * sound slot), then calls func_0018F1D0. soundCueSnapshotRestore starts them again. */
void soundCueSnapshotSave(void) {
    int i;
    int savedCount = 0;
    SoundCueList *cueList;

    gSoundCue.cues.cueUpdateEnabled = 0;
    for (i = 0; i < 32; i++) {
        gSoundCue.cues.savedVoices.halfAlignedVoices[i].soundNumber = 0;
    }
    cueList = &gSoundCue.cues;
    for (i = 0; i < 64; i++) {
        switch (cueList->cueRecords[i].kind) {
        case SOUND_CUE_TRACK:
        case SOUND_CUE_POINT_SOURCE:
        case SOUND_CUE_LINE_SOURCE: {
            /* kept: the char read of the handle byte is reloaded after the word store */
            *(s32 *) &gSoundCue.cues.savedVoices.halfAlignedVoices[savedCount] =
                gSoundVoices[*(unsigned char *) &cueList->cueRecords[i].voiceHandle].word;
            gSoundCue.cues.savedVoices.halfAlignedVoices[savedCount].voiceHandleByte = cueList->cueRecords[i].voiceHandle;
            savedCount++;
        } break;
        case SOUND_CUE_SOUND_SLOT:
            (cueList->savedVoices.halfAlignedVoices + savedCount)->soundNumber = cueList->cueRecords[i].voiceHandle + 1;
            cueList->savedVoices.halfAlignedVoices[savedCount].voiceHandleByte = -1;
            gSoundCue.cues.savedSlotStartArgs[cueList->cueRecords[i].voiceHandle] = cueList->cueRecords[i].u.slotStartArg;
            break;
        }
    }
    func_0018F1D0();
}

void xglSleep(void);
int soundSlotSetVolume(int, int, int);
void soundSlotStartNumbered(int, int);
void soundSlotClearLoopFlag(int);

/* Re-enables the cue list and starts again every voice and sound slot that soundCueSnapshotSave saved. */
void soundCueSnapshotRestore(void) {
    int i;

    gSoundCue.cues.cueUpdateEnabled = 1;
    gSoundCue.cues.probedAreaId = gSoundCue.cues.activeAreaId = -1;
    gSoundCue.cues.ambientSoundNumber = 0;
    func_0018F1D0();
    xglSleep();
    for (i = 0; i < 32; i++) {
        if (gSoundCue.cues.savedVoices.voices[i].soundNumber != 0) {
            if (gSoundCue.cues.savedVoices.voices[i].voiceHandleByte == 0xFF) {
                int restoredSlot = gSoundCue.cues.savedVoices.voices[i].soundNumber - 1;

                soundSlotSetVolume(restoredSlot, -1, 0);
                soundSlotStartNumbered(restoredSlot, gSoundCue.cues.savedSlotStartArgs[restoredSlot]);
                soundSlotClearLoopFlag(restoredSlot);
            } else {
                func_0018EDD8(
                    (((int) gSoundCue.cues.savedVoices.voices[i].voiceHandleByte + 256) << 16) +
                        gSoundCue.cues.savedVoices.voices[i].soundNumber,
                    gSoundCue.cues.savedVoices.voices[i].volume,
                    gSoundCue.cues.savedVoices.voices[i].pan);
            }
        }
    }
}

extern u8 gSoundBankTagTable[];
static u8 gSeVoiceStartDelay;
static u8 gSeVoiceStartParity;
void seVoiceStartDriver();

/* Voice cue command of the sound effect player. With group 0 it plays or controls the music-bank
 * voices: mode -2 selects the bank of soundId, -3 stops voices 64..89, other negative modes count down the
 * delayed start, a mode of 0 or more starts the voice of the bank group holding soundId.
 * With a group above 0 it plays a positional voice at pos, louder the nearer it is to the listener
 * (audible radius), and mode -3 stops the four voices of the group. */
void soundCueSeCommand(s32 cueGroup, s32 cueMode, s32 soundId, Vector4 *pos) {
    s32 bankTag;
    u8 bank;
    s32 groupTag;
    s32 voiceHandleBase;
    s32 i;
    s32 j;
    s32 volume;
    f32 radius;
    f32 distance;
    if (cueGroup == 0) {
        if (-2 == cueMode) {
            s32 newBank;
            newBank = gSoundBankTagTable[(u8) (soundId - 1)] & 3;
            if (gSoundCue.cues.selectedBank != newBank) {
                gSoundCue.cues.selectedBank = newBank;
                func_0018E698(0x8000 + newBank, 8);
            }
            return;
        }
        if (cueMode == -3) {
            for (cueMode = 64; cueMode < 90; cueMode++) {
                func_0018F098(cueMode);
            }
            return;
        }
        if (cueMode < 0) {
            if (gSeVoiceStartDelay != 0 && --gSeVoiceStartDelay == 0) {
                seVoiceStartDriver(gSeVoiceStartParity + 64);
            }
            return;
        }
        bankTag = gSoundBankTagTable[(u8) (soundId - 1)];
        if (4 <= cueMode) {
            cueMode -= 4;
        }
        bank = bankTag & 3;
        bankTag = bankTag & 0xf8;
        for (i = 0; i < 6; ++i) {
            if (gSoundCue.cues.voiceGroupTags[i] == bankTag) {
                if (gSoundCue.cues.selectedBank != bank) {
                    gSoundCue.cues.selectedBank = bank;
                    func_0018E698(bank + 0x8000, 8);
                }
                seVoiceStartDriver(cueMode + i * 4 + 0x42);
                break;
            }
        }
        gSeVoiceStartDelay = 4;
        gSeVoiceStartParity = cueMode & 1;
    } else {
        if (cueMode == -3) {
            func_0018F098(cueGroup * 4 + 181);
            func_0018F098(cueGroup * 4 + 182);
            func_0018F098(cueGroup * 4 + 183);
            func_0018F098(cueGroup * 4 + 184);
            return;
        }
        if (0 > cueMode) {
            return;
        }
        radius = gSoundCue.cues.audibleRadius;
        if (4 <= cueMode) {
            cueMode -= 4;
            if (cueMode == 1) {
                radius = -1.0f;
            }
        }
        distance = xglPointLength(&gSoundCue.listenerPos, pos);
        if (0.0f > radius || distance < radius) {
            volume = 127;
            if (radius >= 0.0f) {
                volume = 127 - (((s32) (distance * 256.0f / radius) * 127) >> 8);
            }
            if (-2 != gSoundCue.cues.listenerTargetId) {
                /* kept: the original computes the pan angle and then plays with a fixed pan of 64 */
                soundPanAngleFromOffset(-(pos->x - gSoundCue.panReferencePos.x), pos->z - gSoundCue.panReferencePos.z);
            }
            if (volume >= 1) {
                groupTag = gSoundBankTagTable[(u8) (soundId - 1)];
                groupTag = groupTag & 0xf8;
                voiceHandleBase = 4 * cueGroup;
                voiceHandleBase += cueMode;
                cueMode = voiceHandleBase;
                for (j = 0; j < 4; j++) {
                    if (gSoundCue.cues.slotGroupTags[j] == groupTag) {
                        groupTag = cueMode + j;
                        func_0018EDD8(groupTag + 181, volume, 64);
                        j = 4; /* kept: the dead store before the break decides the loop exit code */
                        break;
                    }
                }
            }
        }
    }
}

/* One bank name of gSoundBankNameTable: a 3-character name and the 6 group tags that belong to it. */
typedef struct SoundBankTagEntry {
    signed char *bankName;
    char groupTags[8];
} SoundBankTagEntry;

extern SoundBankTagEntry gSoundBankNameTable[];
void func_0018E940();

/* Sets two driver parameters (7 and 14) and copies the group tags of the bank named bankName
 * into voiceGroupTags. */
void soundBankTagsSet(int driverParamFirst, int driverParamSecond, signed char *bankName) {
    int i;
    int j;

    if (driverParamFirst) {
        func_0018E940(7, driverParamFirst);
    }
    if (driverParamSecond) {
        func_0018E940(14, driverParamSecond);
    }
    i = 0;
    if (!gSoundBankNameTable[i].bankName) {
        return;
    }
again: /* kept: a plain for loop gives different code */
    do {
        if (gSoundBankNameTable[i].bankName[0] == bankName[0] && gSoundBankNameTable[i].bankName[1] == bankName[1] &&
            gSoundBankNameTable[i].bankName[2] == bankName[2]) {
            for (j = 0; j < 6; j++) {
                gSoundCue.cues.voiceGroupTags[j] = gSoundBankNameTable[i].groupTags[j];
            }
            return;
        }
    } while (0);
    i++;
    if (gSoundBankNameTable[i].bankName) {
        goto again;
    }
}

typedef unsigned int u32;

/* The cue list resource (func_002A6FA8): items point into a key table. */
typedef struct CueListItem {
    u16 keyIndex;
    u8 unk_02[14];
} CueListItem;

typedef struct CueKeyEntry {
    u32 key;
    u32 unk_04;
} CueKeyEntry;

typedef struct CueList {
    u16 cueItemCount;
    u8 unk_02[10];
    CueKeyEntry *keyTable;
    CueListItem cueItems[1];
} CueList;

CueList *func_002A6FA8();
void *memset(void*,int,unsigned);
void soundCueReset();

/* Resets the cue list: stops the positional voices, keeps the ambient voice when an ambient
 * record existed, clears the state and fills slotGroupTags with the sorted distinct group tags
 * of the cue list resource (unused slots get 0xFF). */
void soundCueReset() {
    u32 groupKeys[32];
    s32 i;
    s32 j;
    s32 cueItemIndex;
    s32 hadAmbientCue = 0;
    u16 savedAmbientHandle;
    u8 savedAmbientSoundNumber;
    CueList *cueList;
    CueListItem *cueItem;
    CueKeyEntry *keyTable;
    s32 cueItemCount;
    u32 key;
    u32 groupTag;
    u32 *keyEnd;
    u32 *scan;
    u32 *tagScan;
    u32 *tagEnd;
    for (i = 0; i < 64; i++) {
        SoundCueRecord *cueRecord = &gSoundCue.cues.cueRecords[i];
        switch (cueRecord->kind) {
        case SOUND_CUE_TRACK:
        case SOUND_CUE_AREA_AMBIENT:
            hadAmbientCue = 1;
            break;
        case SOUND_CUE_POINT_SOURCE:
        case SOUND_CUE_LINE_SOURCE:
            func_0018F098(cueRecord->voiceHandle);
            break;
        case SOUND_CUE_VOLUME_FADE:
        case SOUND_CUE_PAN_FADE:
        case SOUND_CUE_SOUND_SLOT:
            break;
        }
    }
    savedAmbientHandle = gSoundCue.cues.ambientHandle;
    savedAmbientSoundNumber = gSoundCue.cues.ambientSoundNumber;
    memset(&gSoundCue, 0, sizeof(gSoundCue));
    gSoundCue.cues.cueUpdateEnabled = 1;
    gSoundCue.cues.ambientHandle = savedAmbientHandle;
    gSoundCue.cues.probedAreaId = gSoundCue.cues.activeAreaId = -1;
    if (hadAmbientCue) {
        gSoundCue.cues.ambientSoundNumber = savedAmbientSoundNumber;
    }
    gSoundCue.cues.audibleRadius = -1.0f;
    gSoundCue.cues.selectedBank = -1;
    groupKeys[0] = 0;
    keyEnd = groupKeys;
    cueList = func_002A6FA8(0x700, -1);
    if (cueList) {
        cueItem = cueList->cueItems;
        keyTable = cueList->keyTable;
        for (cueItemIndex = 0; cueItemIndex < cueList->cueItemCount; cueItemIndex++, cueItem++) {
            key = keyTable[cueItem->keyIndex].key;
            for (scan = groupKeys; scan < keyEnd; scan++) {
                if (*scan == key) {
                    break;
                }
            }
            if (scan >= keyEnd) {
                *keyEnd++ = key;
            }
        }
    }
    for (scan = groupKeys, tagEnd = groupKeys; scan < keyEnd;) {
        groupTag = gSoundBankTagTable[(u8) *scan++] & 0xf8;
        tagScan = groupKeys;
        while (tagScan < tagEnd) {
            if (*tagScan++ == groupTag) {
                tagScan = 0;
                break;
            }
        }
        if (tagScan) {
            *tagEnd++ = groupTag;
        }
    }
    scan = groupKeys;
    while (scan + 1 < tagEnd) {
        if (scan[0] > scan[1]) {
            u32 swapped = scan[0];
            scan[0] = scan[1];
            scan[1] = swapped;
            scan = groupKeys;
        } else {
            scan++;
        }
    }
    for (scan = groupKeys, j = 0; scan < tagEnd; j++) {
        gSoundCue.cues.slotGroupTags[j] = *scan++;
    }
    for (; j < 4; j++) {
        gSoundCue.cues.slotGroupTags[j] = -1;
    }
}

extern s16 D_005A621A[];

void subsystemResetClearState(void) {
    soundCueReset();
    D_005A621A[0] = 0;
}

/* Bank names of gSoundBankNameTable. */
signed char D_005509C8[8] = "MEV";
signed char D_005509D0[8] = "MIK";
signed char D_005509D8[8] = "HAK";
signed char D_005509E0[8] = "DYU";
signed char D_005509E8[8] = "MER";
signed char D_005509F0[8] = "RAV";
signed char D_005509F8[8] = "HOS";
signed char D_00550A00[8] = "CIT";
signed char D_00550A08[8] = "DOU";
signed char D_00550A10[8] = "KYO";
signed char D_00550A18[8] = "MOR";
signed char D_00550A20[8] = "KYM";
signed char D_00550A28[8] = "REN";
signed char D_00550A30[8] = "ZDA";
signed char D_00550A38[8] = "ZCF";
signed char D_00550A40[8] = "ELS";
signed char D_00550A48[8] = "SYU";
signed char D_00550A50[8] = "MUJ";
signed char D_00550A58[8] = "UMD";

/* The bank name table: a name pointer and the 8 group tags of each bank, zero terminated. */
SoundBankTagEntry gSoundBankNameTable[20] = {
    { D_00550A58, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A50, { 0x00, 0x10, 0x50, 0x70, 0x80, 0x88, 0x00, 0x00 } },
    { D_00550A48, { 0x00, 0x10, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A40, { 0x00, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A38, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A30, { 0x00, 0x08, 0x30, 0x38, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A28, { 0x00, 0x48, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A20, { 0x00, 0x18, 0x50, 0x58, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A18, { 0x00, 0x18, 0x48, 0x50, 0x58, 0x00, 0x00, 0x00 } },
    { D_00550A10, { 0x00, 0x10, 0x18, 0x48, 0x50, 0x58, 0x00, 0x00 } },
    { D_00550A08, { 0x00, 0x08, 0x48, 0x50, 0x00, 0x00, 0x00, 0x00 } },
    { D_00550A00, { 0x00, 0x10, 0x48, 0x50, 0x58, 0x00, 0x00, 0x00 } },
    { D_005509F8, { 0x00, 0x30, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_005509F0, { 0x00, 0x08, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_005509E8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_005509E0, { 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { D_005509D8, { 0x00, 0x20, 0x28, 0x88, 0x00, 0x00, 0x00, 0x00 } },
    { D_005509D0, { 0x00, 0x08, 0x38, 0x40, 0x48, 0x50, 0x00, 0x00 } },
    { D_005509C8, { 0x00, 0x10, 0x30, 0x70, 0x88, 0x00, 0x00, 0x00 } },
    { 0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
};

u8 gSoundBankTagTable[96] = {
    0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12, 0x18, 0x19, 0x1A, 0x20, 0x21, 0x22, 0x28,
    0x29, 0x2A, 0x30, 0x31, 0x32, 0x38, 0x39, 0x3A, 0x40, 0x41, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x49, 0x4A, 0x50, 0x51, 0x52, 0x58, 0x59,
    0x5A, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x70, 0x71, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x79, 0x7A, 0x80, 0x81, 0x82, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x89, 0x8A, 0x00, 0x00, 0x00,
};

/* Sound cue state, the sbss flags, and the bss of this TU. */
SoundCueState gSoundCueStorage;
