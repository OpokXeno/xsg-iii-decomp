#include "common.h"
#include "shared.h"
#include "main/sef_131.h"
#include "main/tu045.h"
#include "main/tu132.h"
#include "main/tu352.h"
#include "main/xgl_2.h"
#include "vu0_macro.h"
#include "gs.h"
#include "main/tu118.h"
#include "main/xenoActor.h"
#include "main/sef_135.h"
#include "main/tu122.h"
#include "main/tu124.h"
#include "quadword.h"

/* >>> CHUNK sef_131 */
#define SefEffectRecord SefEffectRecord_sef_131

/* Row-major 4x4 float matrix. */
/* A track key whose time is 0x400 ends the track: no key follows it. */
#define SEF_TRACK_END_KEY 0x1000
/* Pending bit of gameWork.control that holds the effect draw passes back (the X1 SEF_DRAW_FLAG_PENDING value). */
#define SEF_DRAW_FLAG_PENDING 0x04000000

void svDrawScene3D(void);
void vu0NormalizeVf20(void);
Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);

/* Triangle wave record of the camera shake: value is the current amplitude, step the amount it decays
 * per cycle; period and phase are in frames, hold delays the decay, remaining counts the cycles left. */
typedef struct CameraWave {
    float value;                                /* +0x00 */
    float step;                                 /* +0x04 */
    unsigned short period;                      /* +0x08 */
    unsigned short phase;                       /* +0x0A */
    unsigned short hold;                        /* +0x0C */
    unsigned short remaining;                   /* +0x0E */
} CameraWave;

/* One of the four camera slots (0 look-at, 1 position and angles, 2 perspective, 3 roll): a timed
 * transition set by cameraSlotSetup. */
typedef struct CameraSlot {
    unsigned char unmodeled_0000[0x2];
    unsigned char blendMode;                    /* +0x02: stored by cameraSlotSetup when >= 0; slot 1 blends the position when 0, the angles otherwise */
    unsigned char easeMode;                     /* +0x03: stored by cameraSlotSetup when >= 0; the mode of cameraEase */
    float progress;                             /* +0x04 */
    float duration;                             /* +0x08: the frames of the transition as a float; a slot is active when nonzero */
    float fraction;                             /* +0x0C: the eased fraction of the transition, 1.0 when the frames were 0 */
} CameraSlot;

/* A camera range block (0x70 bytes): the anchor, the range mode and the staged vectors. The camera
 * records (cameraCfSlots) keep one each; the current camera is the one at gameWork+0x180. */
typedef struct CameraRange {
    int anchorIndex;                            /* +0x00: -1 when no anchor */
    short anchorSubIndex;                       /* +0x04: -1 when no anchor */
    unsigned char unmodeled_0006[0x2];
    unsigned char lockX;                        /* +0x08 */
    unsigned char lockY;                        /* +0x09 */
    unsigned char lockZ;                        /* +0x0A */
    unsigned char probeMode;                    /* +0x0B */
    unsigned char rangeMode;                    /* +0x0C */
    unsigned char unmodeled_000D[0x2];
    unsigned char rangeFlag;                    /* +0x0F */
    float northAngle;                           /* +0x10: the camera yaw the stick directions are relative to */
    unsigned char unmodeled_0014[0xC];
    Vector4 offset;                             /* +0x20: the look-at offset above the anchor */
    Vector4 angleLimits;                        /* +0x30: pitch min and max, yaw min and max */
    Vector4 lockPosition;                       /* +0x40: the camera position, w is the perspective */
    Vector4 lookAt;                             /* +0x50: the look-at point, w is the roll */
    Vector4 angles;                             /* +0x60: x pitch, y yaw, w the distance to the look-at point */
} CameraRange;

/* The camera values a slot transition interpolates: position, look-at point and angles (the layout of
 * the current camera at gameWork+0x1C0). */
typedef struct CameraPose {
    Vector4 position;                           /* +0x00: w is the perspective */
    Vector4 lookAt;                             /* +0x10: w is the roll */
    Vector4 angles;                             /* +0x20: x pitch, y yaw, w distance */
} CameraPose;

/* The camera state of the game work block (gameWork+0xD0): the flags, the shake, the four slots, the
 * current camera and the values its transitions start from and end at. The offsets in the comments are
 * gameWork offsets. */
typedef struct GameWorkCamera {
    unsigned short flags;                       /* +0xD0: CAMERA_FLAG_* bits (game_camera.c) */
    unsigned char cameraType;                   /* +0xD2: the camera type of the active record */
    unsigned char cameraSubType;                /* +0xD3: cleared with typeState when the camera type changes */
    signed char recordIndex;                    /* +0xD4: the camera record in use */
    signed char mode;                           /* +0xD5: set by GameCameraChangeID */
    unsigned char allSlotsFlagA;                /* +0xD6 */
    unsigned char allSlotsFlagB;                /* +0xD7: also the anchor pending flag of the range update */
    unsigned char previousRecordIndex;          /* +0xD8 */
    unsigned char anchorLocked;                 /* +0xD9 */
    unsigned char previousCameraType;           /* +0xDA */
    unsigned char unmodeled_00DB[0x1];
    signed char trackIndex;                     /* +0xDC: set by cameraSetIndex */
    unsigned char unmodeled_00DD[0x3];
    unsigned long long typeState[2];            /* +0xE0: cleared (as one quadword) when the camera type changes */
    unsigned char shakeFlags;                   /* +0xF0 */
    unsigned char unmodeled_00F1[0xF];
    CameraWave waves[3];                        /* +0x100 */
    float sway[4];                              /* +0x130 */
    CameraSlot slot[4];                         /* +0x140: 0 look-at, 1 position and angles, 2 perspective, 3 roll */
    CameraRange range;                          /* +0x180: the current camera, laid out like a camera record range */
    CameraPose from;                            /* +0x1F0: the values a slot transition starts from */
    CameraPose to;                              /* +0x220: the values a slot transition ends at */
    Vector4 fromOffset;                         /* +0x250: the offset the position transition starts from */
    Vector4 anchorOffset;                       /* +0x260: the anchor offset vector */
    float anchorBase[3];                        /* +0x270 */
    unsigned char unmodeled_027C[0x4];
} GameWorkCamera;

/* One entry of the elevator slope table. */
typedef struct GameWorkSlope {
    float x;
    float z;
    float push;
    float speed;
} GameWorkSlope;

/* The light sets of the scene, one per light set. */
typedef struct GameWorkLights {
    float direction[16][3][4];          /* +0x000: light types 1-3 */
    float color[16][3][4];              /* +0x300 */
    float ambient[16][4];               /* +0x600: light type 0 */
    float type4Direction[16][4];        /* +0x700: light type 4; w 0 disables it */
    float type4Value[16];               /* +0x800: light type 4, 0.2 by default */
} GameWorkLights;

/* One 256-bit map flag set. */
typedef struct MapBits {
    unsigned int bits[8];
} MapBits;

/* The resource record gameWork.active points at, handed to mapResourceLoadCallback. */
typedef struct MainActive {
    PxyHeader *object;          /* +0x00 */
    int jpegData;               /* +0x04 */
    unsigned char unmodeled_08[0x04];
    struct MapFixupBlock *block; /* +0x0C: the collision block mapLoadBlock registers */
    unsigned char *imageHead;   /* +0x10 */
    int imageOffset;            /* +0x14 */
    unsigned char unmodeled_18[0x08];
    int *jpegStream;            /* +0x20 */
    unsigned char *pending;     /* +0x24: decoded texture */
    short offsetY;              /* +0x28 */
    short offsetX;              /* +0x2A */
    unsigned short height;      /* +0x2C */
    unsigned short width;       /* +0x2E */
    ScreenVector4 view;         /* +0x30: the backdrop origin projected by mapBackdropDraw (x is the row, y the column) */
} MainActive;

struct SbImageHeader;

/* The game work block (0x1B30 bytes, one object owned by game_data.c): the scene state, the camera, the
 * player parameters, the light sets and the event flags. The offsets in the comments are gameWork
 * offsets. control, camera.flags and anchorIndex are plain words; a function that depends on reading
 * one of them with alias set 0 (after the stores before it) reads it through a local one-member union
 * view. */
typedef struct GameWork {
    unsigned char *workEnd;                     /* +0x00: WorkEnd, saved when the field loop starts */
    unsigned char unmodeled_0004[0x2];
    unsigned short loadStage;                   /* +0x06: 1 once the save is read, 2 once the party models are loaded */
    int control;                                /* +0x08: CONTROL_* bits */
    unsigned char inputDelay;                   /* +0x0C: frames until the pause (16 after a toggle, 2 after a ladder exit) or the movie skip (32, 64) is accepted again */
    unsigned char unmodeled_000D[0x3];
    int sceneFlags;                             /* +0x10: bit 0 is set when an encounter starts */
    int nextMap;                                /* +0x14: the requested next map, -1 when none */
    int mapId;                                  /* +0x18: the current map */
    int entryKey;                               /* +0x1C: -1 none, -2 the party position, else an entry key */
    int resourceId;                             /* +0x20: the current resource id (chapter id) */
    unsigned char unmodeled_0024[0x1];
    unsigned char elevatorMask;                 /* +0x25: the copy of elvActiveMask (elevator.c) saved with the scene */
    unsigned char unmodeled_0026[0x2];
    MainActive *active;                  /* +0x28: the resource record being loaded */
    unsigned char *resBuffer;                   /* +0x2C: GameResBufferAlloc result, the loaded image */
    GameWorkSlope slopes[8];                    /* +0x30: the elevator slope table */
    int mapLoaded;                              /* +0xB0 */
    int keepOffMask;                            /* +0xB4: bit n set when elvKeepOffFlags[n] is */
    struct SbImageHeader *imageEnd;             /* +0xB8: the SBL library image behind the loaded script image */
    unsigned char unmodeled_00BC[0x4];
    union { Vector4 mapColor; };                /* +0xC0: setColor, reset to 1.0 at each scene start; a one-member union: the original accesses it with alias set 0 */
    GameWorkCamera camera;                      /* +0xD0 */
    unsigned char unmodeled_0280[0x10];
    unsigned char sceneFlag;                    /* +0x290: cleared at each scene start */
    unsigned char unmodeled_0291[0x3F];
    float frameScale;                           /* +0x2D0: elapsed frames, scales the per-frame movement */
    float frameScaleSaved;                      /* +0x2D4: copy of frameScale made by uwamonoLoadCurrent */
    float playerParams[5][2];                   /* +0x2D8: setPlayer arguments 4 to 8 (percent) in [i][0]; [i][1] is the saved copy */
    int climbFrames;                            /* +0x300: setPlayer argument 9, the ladder-step divisor */
    int climbFramesSaved;                       /* +0x304 */
    float formScale;                            /* +0x308: the current record scale.x */
    float formSize;                             /* +0x30C: the current record extent.x */
    int replayStatus;                           /* +0x310: ground status recorded with the input */
    int replayStickX;                           /* +0x314 */
    int replayStickY;                           /* +0x318 */
    short encounterTarget;                      /* +0x31C */
    unsigned char unmodeled_031E[0x2];
    short encounterReaction;                    /* +0x320: -1 none */
    unsigned char unmodeled_0322[0x2];
    int encounterFlags;                         /* +0x324 */
    unsigned char unmodeled_0328[0x4];
    int encounterRecord;                        /* +0x32C: record index of the current encounter */
    short fadeTimer;                            /* +0x330: frames left of the rider fade */
    unsigned char vibrationTarget;              /* +0x332: the vibration fade target */
    unsigned char vibrationValue;               /* +0x333: the vibration fade current value */
    int vibrationHold;                          /* +0x334: hold countdown of the fade */
    int vibrationFramesLeft;                    /* +0x338: steps left in the fade */
    int vibrationFramesTotal;                   /* +0x33C: steps the fade was started with */
    unsigned char unmodeled_0340[0x4];
    void *releaseHandle;                        /* +0x344 */
    DoublewordAlignedVector4 fogDist;           /* +0x348: setFog near, far, min, max */
    unsigned char unmodeled_0358[0x8];
    Vector4 fogColor;                           /* +0x360: setFog color */
    int shadowCasterMode;                       /* +0x370: setItai argument 2 (default 1), the mode of pxyDrawShadowCaster */
    float gravity;                              /* +0x374: setPlayer argument 10 (ten-thousandths) */
    float gravitySaved;                         /* +0x378 */
    float bounceDivisor;                        /* +0x37C: setPlayer argument 11 (percent) */
    float bounceDivisorSaved;                   /* +0x380 */
    GameWorkLights lights;                      /* +0x384 */
    unsigned char unmodeled_0BC4[0xC0];
    GameWorkLights savedLights;                 /* +0xC84 */
    unsigned char unmodeled_14C4[0x1C];
    MapBits savedMapBits[2];                    /* +0x14E0 */
    signed char eventFlags[512];                /* +0x1520: one flag per kind 8 event index */
    signed char rowFlags[2][512];               /* +0x1720: key row flags, [0] while control bit 11 is clear, [1] while set */
    signed char panelMode;                      /* +0x1B20: the kind of the last panel command */
    unsigned char unmodeled_1B21[0x3];
    int panelValue;                             /* +0x1B24: counted down per frame and shown as a timecode; ov01's
                                                 * battleEndChk ends a time-limited battle at 0 */
    unsigned char unmodeled_1B28[0x8];
} GameWork;

extern GameWork gameWork;
int MMathMatrixBetweenVectors(Matrix4 *destination, const Vector4 *from, const Vector4 *to);

/* One 8-byte key of a keyframe track: frame and next are the key index
 * the cursor steps to; value is the sample the cursor takes from it. */
typedef struct SefKeyframe {
    short frame;                                     /* +0x00 */
    short next;                                      /* +0x02 */
    unsigned short value;                            /* +0x04 */
    unsigned char unmodeled_006[2];
} SefKeyframe;

/* A track's cursor: the current key index, the sampled value and the
 * playback frame (float). */
typedef struct SefKeyCursor {
    short index;                                     /* +0x00 */
    unsigned short value;                            /* +0x02 */
    float frame;                                     /* +0x04 */
} SefKeyCursor;

/* The 2D draw flag word: sefDrawEffect2D reads it before its camera and end-of-3D-mode calls. */
extern int sefDraw2DFlag[];
/* The 3D draw flag word: sefDrawEffect3D draws the 3D scene only while it is non-zero. */
extern int sefDraw3DFlag[];

/* One 0x2C0-byte entry of the table at sefPtAllocatorState. head and tail are the free-slot ring indices of
 * sefPtFreeSlots and are used only through entry 0 (the ring control block); the getters return the entry's data,
 * which starts after a 16-byte prefix. */
typedef struct SefSlot {
    int head;
    int tail;
    unsigned char unmodeled_08[0x10 - 0x08];
    unsigned char data[0x2C0 - 0x10];
} SefSlot;

extern SefSlot *sefPtAllocatorState;
extern int sefPtAllocCount;
extern short sefPtFreeSlots[1104];

/* A track cursor: the key index it is on, the running time and the
 * interpolated samples the track readers store. */
typedef struct SefTrackCursor {
    short index;
    short sampled_s16;
    float time;
    float sampled;
} SefTrackCursor;

extern f32 D_00453EBC[];
SefTrackKey *sefAdvanceTrackCursor(SefTrackKey *keys, SefTrackCursor *cursor);
extern s16 sefInitialized;
void sefDrawEffect3DCallback(void);
void sefDrawEffect3D(void);

/* One 0x18-byte record of the entry count table: four counters, their sum at +0x10 and an unused word.
 * The record after the last entry (index 44) holds the running sums of the counters and of the totals. */
typedef struct SefEntryCount {
    unsigned int counts[4];                     /* +0x00 */
    unsigned int total;                         /* +0x10 */
    unsigned int unmodeled_14;                  /* +0x14 */
} SefEntryCount;

extern SefEntryCount D_0076C000[];

/* Address of entry index in the table D_0076C000 (SefEntryCount, 0x18 bytes each). */
SefEntryCount *sefEntryCountsEntryGet(int index)
{
    return &D_0076C000[index];
}

int sefGetDraw2DFlag(void) {
    return sefDraw2DFlag[0];
}

int sefGetDraw3DFlag(void) {
    return sefDraw3DFlag[0];
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001F9BB8);

void sefSumEntryCounts(void)
{
    SefEntryCount *counts;
    SefEntryCount *sum;
    unsigned char *cursor;
    int i;

    counts = D_0076C000;
    counts[44].counts[0] = 0;
    counts[44].counts[1] = 0;
    counts[44].counts[2] = 0;
    counts[44].counts[3] = 0;
    counts[44].total = 0;
    sum = counts;
    counts = counts + 44;
    cursor = (unsigned char *)sum + 0x10;
    for (i = 0; i < 44; i++) {
        unsigned int *total = (unsigned int *)cursor;
        unsigned int w2 = total[-2];
        unsigned int first_pair = total[-4] + w2;
        unsigned int second_pair = total[-1] + total[-3];

        *total = first_pair + second_pair;
        sum[44].counts[0] += total[-4];
        sum[44].counts[1] += total[-3];
        sum[44].counts[3] += total[-1];
        sum[44].counts[2] += total[-2];
        sum[44].total += *total;
        cursor += 24;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sef", srsAtan2);

/* vector takes the fraction of vector + addend, all four lanes:
 * sum - (float) (int) sum. */
void sefAddVectorFraction(Vector4 *vector, void *addend)
{
    __asm__ __volatile__(
        "lqc2 vf1, 0(%0)\n"
        "lqc2 vf2, 0(%1)\n"
        "vadd.xyzw vf1, vf1, vf2\n"
        "vftoi0.xyzw vf3, vf1\n"
        "vitof0.xyzw vf3, vf3\n"
        "vsub.xyzw vf3, vf1, vf3\n"
        "sqc2 vf3, 0(%0)\n"
        :
        : "r"(vector), "r"(addend)
        : "memory");
}

void sefMemZero(void *dest, int size);
/* The request buffer zeroed by sevInitPtAllocator (0x340 bytes). */
extern unsigned char sefRequestBuffer[];

void sevInitPtAllocator(void)
{
    int i;
    int count;

    sefMemZero(sefPtAllocatorState, 0xA7950);
    sefMemZero(sefRequestBuffer, 736);
    count = sefPtAllocCount;
    for (i = 0; i < count; i++) {
        sefPtFreeSlots[i] = i;
    }
}

/* Takes the next free slot index from the sefPtFreeSlots ring (head side); returns -1 when the ring is empty. */
int sevAllocPtAllocator(void)
{
    SefSlot *ring = sefPtAllocatorState;
    int head = ring->head;
    int next = head + 1;
    int handle;

    if (!(next < sefPtAllocCount)) {
        next = 0;
    }
    if (next == ring->tail) {
        return -1;
    }
    handle = sefPtFreeSlots[head];
    ring->head = next;
    return handle;
}

void sevFreePtAllocator(int handle)
{
    SefSlot *ring = sefPtAllocatorState;
    int tail = ring->tail;
    int next;

    if (handle < 0) {
        return;
    }
    if (!(handle < sefPtAllocCount)) {
        return;
    }
    if (ring->head == tail) {
        return;
    }
    next = tail + 1;
    sefPtFreeSlots[tail] = handle;
    ring->tail = next;
    if (!(next < sefPtAllocCount)) {
        ring->tail = 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001F9F48);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001F9FF0);

SefTrackKey *sefAdvanceTrackCursor(SefTrackKey *keys, SefTrackCursor *cursor)
{
    f32 time;
    int frame;
    SefTrackKey *key;
    short next_index;

    time = cursor->time + D_00453EBC[0];
    key = &keys[cursor->index];
    frame = (int)time;
    cursor->time = time;
    if (frame < SEF_TRACK_END_KEY && key->key != SEF_TRACK_END_KEY && frame >= (key + 1)->key) {
        key++;
        next_index = (short)key->link;
        if (next_index >= 0) {
            cursor->index = key->link;
            key = &keys[next_index];
            cursor->time = (f32)key->key;
        } else {
            cursor->index = (s16)((u16)cursor->index + 1);
        }
    }
    return key;
}

/* One 16-byte key of a vector track: the key time is the short at +0x0C and the link the signed byte at +0x0E. */
typedef struct SefTrackKey16 {
    unsigned char unmodeled_0000[0xC];
    short key;                                  /* +0x0C */
    signed char link;                           /* +0x0E */
    unsigned char unmodeled_000F[0x1];
} SefTrackKey16;

SefTrackKey16 *sefAdvanceVectorTrackCursor(SefTrackKey16 *keys, SefTrackCursor *cursor)
{
    f32 time;
    int frame;
    SefTrackKey16 *key;
    signed char next_index;

    time = cursor->time + D_00453EBC[0];
    key = &keys[cursor->index];
    frame = (int)time;
    cursor->time = time;
    if (frame < SEF_TRACK_END_KEY && key->key != SEF_TRACK_END_KEY && frame >= (key + 1)->key) {
        key++;
        next_index = key->link;
        if (next_index >= 0) {
            cursor->index = next_index;
            key = &keys[next_index];
            cursor->time = (f32)key->key;
        } else {
            cursor->index = (s16)((u16)cursor->index + 1);
        }
    }
    return key;
}

/* Number of point handles in one local effect record. */
#define SEF_LINE_PARTICLE_COUNT 0x100

/* The fields of a local effect record that the point-handle functions of this unit read and write:
 * the handle array and the count of handles still held. */
typedef struct SefLocalPointRecord {
    unsigned char unmodeled_0000[0x69C];
    short pointHandles[SEF_LINE_PARTICLE_COUNT];    /* +0x69C */
    unsigned char unmodeled_089C[0x89E - 0x89C];
    unsigned short pointCount;                      /* +0x89E: decremented when a handle is freed */
} SefLocalPointRecord;

/* The fields of an effect slot that the slot init and destroy functions of this unit touch: the id,
 * the flags word (bit 31 cleared on init), the kind and the line count. */
typedef struct SefEffectSlot {
    short lineHandles[32];                          /* +0x00 */
    unsigned char lineTags[32];                     /* +0x40 */
    int id;                                         /* +0x60 */
    unsigned int flags;                             /* +0x64 */
    short kind;                                     /* +0x68 */
    unsigned char lineCount;                        /* +0x6A */
} SefEffectSlot;

/* One 16-byte entry of the table that func_001FA1C0 reads; the names are provisional until the callers give them. */
typedef struct SefEntry16 {
    unsigned char unmodeled_00[12];
    short field12;                                  /* +12 */
    signed char field14;                            /* +14 */
    unsigned char unmodeled_0F;
} SefEntry16;

void *memset(void *destination, int value, unsigned int size);
void sefFreeEffectData(SefEffectSlot *slot, int index);

void func_001FA1C0(SefEntry16 *entries, SefEntry16 **out0, SefEntry16 **out1, SefEntry16 **out2, SefEntry16 **out3)
{
    SefEntry16 *head;
    SefEntry16 *mid;
    SefEntry16 *tail;

    head = &entries[-1];
    if (entries[0].field12 == 0) {
        head = entries;
    }
    mid = &entries[1];
    tail = &entries[2];
    if (entries[1].field12 == 4096) {
        mid = entries;
        tail = entries;
    } else if (entries[1].field14 >= 0) {
        tail = &entries[entries[1].field14];
    }
    if (entries[2].field12 == 4096 && entries[1].field14 < 0) {
        tail = mid;
    }
    *out0 = head;
    *out1 = entries;
    *out2 = mid;
    *out3 = tail;
}

float sefEaseCurve(int mode, float time, float duration)
{
    float scale;
    float half;
    float doubled;
    float rate;

    scale = 1.0f / duration;
    switch (mode) {
    case 1:
        doubled = scale + scale;
        half = duration * 0.5f;
        if (time <= half) {
            rate = time * doubled;
            rate = rate * rate * rate * rate * 0.5f;
        } else {
            rate = 1.0f - (time - half) * doubled;
            rate = 1.0f - rate * rate * rate * rate * 0.5f;
        }
        break;
    case 2:
        rate = time * scale;
        rate = rate * rate;
        break;
    case 3:
        rate = 1.0f - time * scale;
        rate = 1.0f - rate * rate;
        break;
    default:
        rate = time * scale;
        break;
    }
    if (rate < 0.0f) {
        rate = 0.0f;
    }
    if (rate > 1.0f) {
        rate = 1.0f;
    }
    return rate;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FA338);

/* One 16-byte key of a vector track: the key time is the short at +0x0C and the link the signed byte at +0x0E. */
typedef struct SefVectorTrackKey {
    unsigned char unmodeled_0000[0xC];
    short key;                                  /* +0x0C */
    signed char link;                           /* +0x0E */
    unsigned char unmodeled_000F[0x1];
} SefVectorTrackKey;

/* A colour track's cursor: the track cursor, then the four integer colour lanes written at +0x10. */
typedef struct SefColorTrackCursor {
    SefTrackCursor track;                            /* +0x00 */
    unsigned char unmodeled_00c[0x10 - 0x0c];
    int color[4];                                    /* +0x10 */
} SefColorTrackCursor;

/* Samples a colour track at the cursor's time: the two keys the cursor spans are interpolated per lane,
 * scaled by the track's colour, clamped to 255 and stored as integers in the cursor's colour lanes. */
void sefSampleColorTrack(SefVectorTrackKey *keys, SefColorTrackCursor *cursor, const Vector4 *scale)
{
    SefVectorTrackKey *key;
    int *lanes;
    short start;
    short end;
    float fraction;

    key = &keys[cursor->track.index];
    if ((key + 1)->key != SEF_TRACK_END_KEY) {
        lanes = cursor->color;
        key = sefAdvanceVectorTrackCursor(keys, &cursor->track);
        start = key->key;
        end = (key + 1)->key;
        if (start != end) {
            fraction = (cursor->track.time - (float)start) / (float)(end - start);
            __asm__ __volatile__(
                "lqc2 vf1, 0(%0)\n\t"
                "lqc2 vf2, 16(%0)\n\t"
                "lqc2 vf5, 0(%2)\n\t"
                "mfc1 $9, %3\n\t"
                "mfc1 $8, %4\n\t"
                "vsub.w vf2, vf2, vf2\n\t"
                "qmtc2 $9, vf3\n\t"
                "qmtc2 $8, vf4\n\t"
                "vsub.xyz vf2, vf2, vf1\n\t"
                "vmulx.xyz vf2, vf2, vf3x\n\t"
                "vadd.xyz vf2, vf2, vf1\n\t"
                "vmul.xyzw vf1, vf5, vf2\n\t"
                "vminix.xyz vf2, vf1, vf4x\n\t"
                "vftoi0.xyzw vf2, vf2\n\t"
                "sqc2 vf2, 0(%1)"
                :
                : "r"(key), "r"(lanes), "r"(scale), "f"(fraction), "f"(255.0f)
                : "$8", "$9", "memory");
        }
    }
}

void sefSampleTrack(SefTrackKey *keys, SefTrackCursor *cursor)
{
    SefTrackKey *key;
    f32 sample;

    key = &keys[cursor->index];
    if ((key + 1)->key == SEF_TRACK_END_KEY) {
        sample = key->value;
    } else {
        key = sefAdvanceTrackCursor(keys, cursor);
        if (key->key == (key + 1)->key) {
            return;
        }
        sample = ((key + 1)->value - key->value)
               * ((cursor->time - (f32)key->key) / (f32)((key + 1)->key - key->key))
               + key->value;
    }
    cursor->sampled = sample;
}

void sefSampleTrackTruncated(SefTrackKey *keys, SefTrackCursor *cursor)
{
    SefTrackKey *key;
    f32 sample;

    key = &keys[cursor->index];
    if (key->key == SEF_TRACK_END_KEY || (key + 1)->key == SEF_TRACK_END_KEY) {
        return;
    }
    key = sefAdvanceTrackCursor(keys, cursor);
    if (key->key == (key + 1)->key) {
        return;
    }
    sample = ((key + 1)->value - key->value)
           * ((cursor->time - (f32)key->key) / (f32)((key + 1)->key - key->key))
           + key->value;
    cursor->sampled_s16 = (short)sample;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FA6B8);

void sefSampleKeyframeValue(SefKeyframe *keys, SefKeyCursor *cursor) {
    cursor->value = ((SefKeyframe *)sefAdvanceTrackCursor((SefTrackKey *)keys, (SefTrackCursor *)cursor))->value;
}

/* Builds in destination the rotation that turns axis onto the normalized direction (MMathMatrixBetweenVectors);
 * returns 0 without touching destination for a zero direction. */
int sefSetSlotDirection(Matrix4 *destination, const Vector4 *direction, const Vector4 *axis)
{
    Vector4 normalized;
    int is_zero;

    is_zero = ((*(int *)&direction->z | *(u64 *)direction) == 0);
    if (is_zero) {
        return 0;
    }
    MMathNormalizeVector2(&normalized, direction);
    MMathMatrixBetweenVectors(destination, axis, &normalized);
    return 1;
}

void sefSetSlotDirectionBetween(void *state, Vector4 *minuend, Vector4 *subtrahend, const Vector4 *axis) {
    Vector4 delta;

    Vu0SefSubVectorXYZ(&delta, minuend, subtrahend);
    sefSetSlotDirection(state, &delta, axis);
}

/* result = the normalized (first - second) when flags has 0x400, else the
 * normalized (second - first); xyz only. The normalizing subroutine works on
 * vf20 in place. */
void sefNormalizeDifference(Vector4 *result, const Vector4 *first, const Vector4 *second, int flags) {
    __asm__ __volatile__(
        "lqc2 vf20, 0(%0)\n"
        "lqc2 vf1, 0(%1)\n"
        :
        : "r"(first), "r"(second)
        : "memory");
    if ((flags & 0x400) != 0) {
        __asm__ __volatile__("vsub.xyz vf20, vf20, vf1" : : : "memory");
    } else {
        __asm__ __volatile__("vsub.xyz vf20, vf1, vf20" : : : "memory");
    }
    vu0NormalizeVf20();
    __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(result) : "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FA808);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FAC58);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FB220);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FB268);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FB2E8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FB6A0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FB8A0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FBD18);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FBE38);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FC1B0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FC620);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FCC28);

extern SefLocalData *sefLineDataRecords;
int sefAllocLineData(void);

void sefFreeLocalData(SefLocalData *data, unsigned int index)
{
    if (index < SEF_LINE_PARTICLE_COUNT) {
        sevFreePtAllocator(data->pointHandles[index]);
        data->pointHandles[index] = -1;
        if (data->pointCount != 0) {
            data->pointCount--;
        }
    }
}

int sefAllocLocalData(SefLocalData *data)
{
    int index;
    int end;
    int handle;

    index = data->cursor;
    end = (index - 1) & (SEF_LINE_PARTICLE_COUNT - 1);
    if (data->allocScanFlag == 0) {
        if (data->pointHandles[index] >= 0) {
            return -1;
        }
    } else {
        for (; index != end; index = (index + 1) & (SEF_LINE_PARTICLE_COUNT - 1)) {
            if (data->pointHandles[index] < 0) {
                break;
            }
        }
        if (index == end) {
            return -1;
        }
    }
    handle = sevAllocPtAllocator();
    if (handle < 0) {
        return -1;
    }
    data->pointHandles[index] = handle;
    data->cursor = (index + 1) & (SEF_LINE_PARTICLE_COUNT - 1);
    data->pointCount++;
    return index;
}

void sefDestroyLocalData(SefLocalPointRecord *data)
{
    int i;

    for (i = 0; i < SEF_LINE_PARTICLE_COUNT && data->pointCount != 0; i++) {
        if (data->pointHandles[i] >= 0) {
            sefFreeLocalData(data, i);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FD000);

/* The count of local effect records in use slots: the first free one is claimed and its index returned. */
extern int D_0055106C;

int sefAllocLineData(void)
{
    SefLocalData *line;
    int index;

    index = 0;
    if (D_0055106C > 0) {
        line = sefLineDataRecords;
        do {
            if (line->inUse == 0) {
                line->inUse = 32;
                return index;
            }
            line++;
        } while (++index < D_0055106C);
    }
    return -1;
}

void sefFreeLineData(int lineIndex)
{
    SefLocalData *line;

    if (lineIndex >= 0 && lineIndex < D_0055106C) {
        line = 0;
        if (lineIndex >= 0) {
            line = sefLineDataRecords + lineIndex;
        }
        sefDestroyLocalData((SefLocalPointRecord *)line);
        line->inUse = 0;
    }
}

/* The 8-byte-aligned image packets of an animation object (the GS texture set-up of one frame). */
/* An animation list ends at the record whose time is 4096. */
#define SEF_LIST_END 4096
#define SEF_IMAGE_PACKET_COUNT 30
#define SEF_ALT_PACKET_COUNT 8
#define SEF_IMAGE_PACKET_SCALE 0.005f
#define SEF_GS_PSMT8 0x13
#define SEF_GS_PSMT4 0x14

/* One 14-byte entry of an object's animation list: time is the key, 4096 ends the list; the flags byte
 * is read from the list head only. */
typedef struct SefAnalyzeRecord {
    short time;                                 /* +0x00 */
    unsigned char unmodeled_02[1];
    unsigned char flags;                        /* +0x03 */
    short imageId;                              /* +0x04 */
    short altImageId;                           /* +0x06 */
    short imageId2;                             /* +0x08 */
    short altImageId2;                          /* +0x0A */
    unsigned char unmodeled_0C[2];
} SefAnalyzeRecord;

/* One 32-byte image packet. */
typedef struct SefImagePacket {
    float scaledWidth;                          /* +0x00 */
    float scaledHeight;                         /* +0x04 */
    const void *image;                          /* +0x08 */
    const void *palette;                        /* +0x0C */
    unsigned long long tex0;                    /* +0x10 */
    float time;                                 /* +0x18 */
    unsigned short flags;                       /* +0x1C */
    unsigned short resourceKind;                /* +0x1E */
} SefImagePacket;

/* The image mapper entry read through svGetImageListItem (owner: sv_get_image_item_142.c). */
typedef struct SvTypeListEntry {
    void *data;                                 /* +0x00 */
    unsigned char unmodeled_04[0x0C - 0x04];
    unsigned short resourceKind;                /* +0x0C */
    unsigned char unmodeled_0E[0x12 - 0x0E];
    unsigned short basePointer;                 /* +0x12 */
    unsigned short bitsPerPixel;                /* +0x14 */
    unsigned short bufferWidthPixels;           /* +0x16 */
    unsigned short width;                       /* +0x18 */
    unsigned short height;                      /* +0x1A */
    unsigned short textureWidthExponent;        /* +0x1C */
    unsigned short textureHeightExponent;       /* +0x1E */
} SvTypeListEntry;

/* The current effect record (sefCurrentRecord): only the entry id at +0x08 is read here. */
typedef struct SefEffectRecord {
    unsigned char unmodeled_00[0x08];
    short entryId;                              /* +0x08 */
} SefEffectRecord;

extern SefEffectRecord *sefCurrentRecord;
SvTypeListEntry *svGetImageListItem(int type, unsigned int flags);

/* The animation object: its record list at +0x17C, the 30 image packets at +0x1A0 and the 8 alternate
 * packets after them at +0x560. */
typedef struct SefAnimObject {
    unsigned char unmodeled_0000[0x17C];
    SefAnalyzeRecord *records;                  /* +0x17C */
    unsigned char unmodeled_0180[0x1A0 - 0x180];
    SefImagePacket packets[SEF_IMAGE_PACKET_COUNT]; /* +0x1A0 */
    SefImagePacket altPackets[SEF_ALT_PACKET_COUNT]; /* +0x560 */
} SefAnimObject;

int sefAnalyzeAnim(SefAnimObject *object)
{
    int packetCount;
    SefAnalyzeRecord *records;
    SefImagePacket *packet;
    SefImagePacket *altPacket;
    short *altPaletteId;
    int psm;

    records = object->records;
    packetCount = 0;
    if (records->time != SEF_LIST_END) {
        altPacket = object->altPackets;
        packet = object->packets;
        do {
            int flags;
            short mapperIndex;
            float animScale;
            SvTypeListEntry *image;
            SvTypeListEntry *palette;

            flags = records->flags;
            mapperIndex = sefCurrentRecord->entryId;
            if ((flags & 0x10) != 0) {
                if (records[packetCount].imageId == 0 || records[packetCount].altImageId == 0) {
                    memset(packet, 0, 0x20);
                    packet->resourceKind = 4;
                    packet->flags = flags;
                    break;
                }
            }
            if ((records[packetCount].imageId & 0x100) != 0) {
                image = svGetImageListItem(0, records[packetCount].imageId);
                palette = svGetImageListItem(0, records[packetCount].altImageId);
            } else {
                image = svGetImageListItem(mapperIndex, records[packetCount].imageId);
                palette = svGetImageListItem(mapperIndex, records[packetCount].altImageId);
            }
            if (image != 0 && palette != 0) {
                if (packetCount < SEF_IMAGE_PACKET_COUNT) {
                    animScale = SEF_IMAGE_PACKET_SCALE;
                    packet->flags = flags;
                    packet->time = (float)records[packetCount].time;
                    packet->scaledWidth = (float)image->width * animScale;
                    packet->scaledHeight = (float)image->height * animScale;
                    packet->resourceKind = image->resourceKind;
                    psm = image->bitsPerPixel == 8 ? SEF_GS_PSMT8 : SEF_GS_PSMT4;
                    packet->image = image->data;
                    packet->palette = palette->data;
                    packet->tex0 = GS_SET_TEX0(image->basePointer, image->bufferWidthPixels >> 6,
                        psm, image->textureWidthExponent, image->textureHeightExponent, 1, 0,
                        palette->basePointer, 0, 0, 0, 1);
                    if (records[packetCount].imageId2 != 0 &&
                        *(altPaletteId = &records[packetCount].altImageId2) != 0) {
                        image = svGetImageListItem(mapperIndex, records[packetCount].imageId2);
                        palette = svGetImageListItem(mapperIndex, *altPaletteId);
                        if (image != 0 && palette != 0) {
                            if (packetCount < SEF_ALT_PACKET_COUNT) {
                                packet->flags |= 0x4000;
                                packet[SEF_IMAGE_PACKET_COUNT].image = image->data;
                                packet[SEF_IMAGE_PACKET_COUNT].scaledWidth = (float)image->width * animScale;
                                packet[SEF_IMAGE_PACKET_COUNT].scaledHeight = (float)image->height * animScale;
                                packet[SEF_IMAGE_PACKET_COUNT].resourceKind = image->resourceKind;
                                packet[SEF_IMAGE_PACKET_COUNT].palette = palette->data;
                                psm = image->bitsPerPixel == 8 ? SEF_GS_PSMT8 : SEF_GS_PSMT4;
                                packet[SEF_IMAGE_PACKET_COUNT].tex0 = GS_SET_TEX0(image->basePointer,
                                    image->bufferWidthPixels >> 6, psm, image->textureWidthExponent,
                                    image->textureHeightExponent, 1, 0, palette->basePointer, 0, 0, 0, 1);
                            }
                        } else {
                            memset(altPacket, 0, 0x20);
                        }
                    }
                }
            } else {
                memset(packet, 0, 0x20);
            }
            altPacket++;
            packet++;
            packetCount++;
        } while (records[packetCount].time != SEF_LIST_END);
    }
    return packetCount;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FDBE0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FE030);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FE6F0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FF1A0);

void sefInitEffectData(SefEffectSlot *slot, int id, int kind)
{
    unsigned int flags;

    memset(slot, -1, 64);
    slot->id = id;
    flags = slot->flags & 0x7FFFFFFF;
    slot->kind = kind;
    slot->flags = flags;
    slot->lineCount = 0;
}

void sefDestroyEffectData(SefEffectSlot *slot)
{
    int i;

    if (slot->id != 0) {
        for (i = 0; i < 32; i++) {
            sefFreeEffectData(slot, i);
        }
    }
    slot->kind = 0;
    slot->id = 0;
    slot->flags = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FF7B0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FF838);

SefLocalData *sefAllocEffectData(SefEffectSlot *slot, int tag)
{
    short *handle;
    int lineSlot;
    int lineHandle;

    lineSlot = (signed char)slot->lineCount;
    if ((unsigned int)lineSlot >= 32) {
        return 0;
    }
    handle = &slot->lineHandles[lineSlot];
    if (*handle >= 0) {
        return 0;
    }
    lineHandle = sefAllocLineData();
    if (lineHandle < 0) {
        return 0;
    }
    *handle = lineHandle;
    slot->lineTags[lineSlot] = tag;
    slot->lineCount++;
    return &sefLineDataRecords[lineHandle];
}

void sefFreeEffectData(SefEffectSlot *slot, int index)
{
    short *handle;

    if ((unsigned int)index < 32) {
        handle = &slot->lineHandles[index];
        if (*handle >= 0) {
            sefFreeLineData(*handle);
            *handle = -1;
        }
    }
}

void sefOrEntryFlags(int key, int flags);
void sefClearEntryFlags(int key, int flags);

void sefApplySlotFlags(int wordMask, int flagBits, int setBits)
{
    void (*applyBits)(int, int);
    int *requestWords;

    requestWords = (int *)sefRequestBuffer;
    applyBits = setBits ? sefOrEntryFlags : sefClearEntryFlags;
    if (wordMask == 0) {
        applyBits(requestWords[0], flagBits);
        return;
    }
    if (wordMask & 1) {
        applyBits(requestWords[0], flagBits);
    }
    if (wordMask & 4) {
        applyBits(requestWords[1], flagBits);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FFA50);

/* The line data pool: the splat label of the pool, owned by the bss of main/tu127. */
extern unsigned char D_00660FA0[];

void sefClearLineDataRecords(void)
{
    memset(D_00660FA0, 0, 0x6978);
}

/* The record block of this unit (the splat label of main/sef_131 bss that sefLineDataRecords points to). */
extern unsigned char D_00667920[];

void sefResetPool(void)
{
    memset(D_00667920, 0, 0x5C4E0);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00200180);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00200210);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002002C8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00200538);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002005F0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00200878);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002008D8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00200FC0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002010C8);

/* The eight-word scheduler entry func_002010C8 (still INCLUDE_ASM); the parameter types are provisional. */
int func_002010C8(int table, int request, int offset, int loadTag, int delayFrames, int reuseIndex, int flag, int extra);

int sefCreateScheduler(int table, int request, int offset, int loadTag, int reuseIndex)
{
    return func_002010C8(table, request, offset, loadTag, 0, reuseIndex, 1, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201508);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201690);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002017E0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201808);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201898);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002018E8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201C30);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201EA0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00202110);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002022F0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00202A78);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00202B30);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00202CC8);

/* The table of tu118.h without its 0x0C-byte tail: the .extern size recorded in each object differs per TU, so the
 * unification waits for one agreed size. */
typedef struct SefEntryTableShort {
    SefEntry entries[24];
    unsigned char unmodeled_20700[0x10];
    int entryCount;                             /* +0x20710 */
} SefEntryTableShort;

extern SefEntryTableShort D_004541A0;

void sefOrEntryFlags(int key, int flags)
{
    SefEntry *entry;
    int index;

    if (key == 0) {
        return;
    }
    index = 0;
    if (D_004541A0.entryCount <= 0) {
        return;
    }
    if (D_004541A0.entries[0].id == key) {
        D_004541A0.entries[0].flags |= flags;
        return;
    }
next:
    index++;
check:
    if (index >= D_004541A0.entryCount) {
        return;
    }
    entry = &D_004541A0.entries[index];
    if (entry->id != key) {
        goto next;
    }
    entry->flags |= flags;
}

void sefClearEntryFlags(int key, int flags)
{
    SefEntry *entry;
    int index;

    if (key == 0) {
        return;
    }
    entry = &D_004541A0.entries[0];
    index = 0;
    if (D_004541A0.entryCount <= 0) {
        return;
    }
    if (entry->id != key) {
        goto loop;
        do {
            entry = 0;
loop:
            index++;
            if (index >= D_004541A0.entryCount) {
                return;
            }
            entry = &D_004541A0.entries[index];
        } while (entry->id != key);
    }
    entry->flags &= ~flags;
}

int tiMRPackageGetNodeMatrix(Vector4 *block, void *arg);
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
extern const Matrix4 MMathUnitMatrix;

/* Multiplies the actor's local matrix into out when apply is set; an actor without a block gets the identity and returns 0. */
int sefActorMulLocalMatrix(XenoActor *actor, void *arg, Matrix4 *out, int apply)
{
    if (actor == 0) {
        Vu0CopyUnitMatrix(out, &MMathUnitMatrix);
        return 0;
    }
    if (tiMRPackageGetNodeMatrix(&actor->shadowB, arg) == 0) {
        return 0;
    }
    if (apply != 0) {
        MMathMulMatrix(out, &actor->localMatrix, out);
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203048);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002030E8);

SefEntry *sefFindEntryById(int key)
{
    SefEntryTableShort *table;
    SefEntry *entry;
    int i;

    if (key != 0) {
        table = &D_004541A0;
        for (i = 0; i < table->entryCount; i++) {
            entry = &table->entries[i];
            if (entry->id == key) {
                return entry;
            }
        }
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203360);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203390);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002033C8);

void func_001ED2E8(void);
void sdvExecAlters(void);
void func_001F8680(void);
extern int D_00551084;
extern int D_00551088;

void sefExecStep(void)
{
    memset(D_0076C000, 0, sizeof(SefEntryCount) * 45);
    func_001ED2E8();
    sdvExecAlters();
    func_001F8680();
    sefSumEntryCounts();
    D_00551084 = 0;
    D_00551088 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203450);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002034F0);

extern int D_0055107C;
s16 func_001F52C0(void);
void sefGetCameraPos(Matrix4 *dest);
void svDrawScene2D(void);

void sefDrawEffect2D(void)
{
    if (D_0055107C != 0) {
        if (sefGetDraw2DFlag() != 0 || func_001F52C0() != 0) {
            sefGetCameraPos(sefCameraMatrix);
            svDrawScene2D();
        }
    }
}

int func_001F52D0(void);
void func_0020E6F8(void);

void func_00203568(void)
{
    if (D_0055107C != 0) {
        if (sefGetDraw3DFlag() != 0 || func_001F52D0() != 0) {
            sefGetCameraPos(sefCameraMatrix);
            func_0020E6F8();
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002035C0);

INCLUDE_ASM("asm/main/nonmatchings/sef", sefDrawEffect3D);

void sefDrawEffect3DCallback(void) {
    sefDrawEffect3D();
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203668);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203690);

#undef SefEffectRecord
/* <<< CHUNK sef_131 */

/* >>> CHUNK sef_132 */
#define SefTrackKey16 SefTrackKey16_sef_132

/* Row-major 4x4 float matrix. */

#define SEF_TABLE_RECORD_SIZE 0xA70

SefEffectRecord *sefGetScheduler(void);

struct SefLocalData;
struct SefGroupObject;
struct SefLocalData *sefGetLineAdr(int entryIndex);

/* Particles per line-data record. */
#define SEF_LINE_PARTICLE_COUNT 0x100
#define SEF_LINE_DATA_STRIDE 0x8B0

/* The request buffer: 0x340 bytes (SefRequestBuffer view); +0x100 (0x8046F0) and sefRequestParentMatrixPointer (+0x300) are
 * labels inside it. */
extern unsigned char sefRequestBuffer[0x340];

typedef struct SefTrackKey16 {
    float value[3];                                  /* +0x00 */
    short key;
    signed char link;
    unsigned char unmodeled_00f[1];
} SefTrackKey16;

#define sefCurrentRecord (*(SefEffectRecord **)&sefCurrentRecord)
extern SefLocalData *sefLineDataRecords;
extern SefEffectRecord *sefEffectRecords;

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002037D0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002037F8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203858);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002038A8);

SefLinkedContext *sefGetBattleData(void)
{
    return (SefLinkedContext *) sefRequestBuffer;
}

/* One 0x270-byte entry of the table at sefPtAllocatorState (the stride the getters step by); data is the array at +0x10. */
typedef struct SefPtAllocatorEntry {
    unsigned char unmodeled_00[0x10];
    unsigned char data[0x270 - 0x10];
} SefPtAllocatorEntry;

#define sefPtAllocatorState (*(SefPtAllocatorEntry **)&sefPtAllocatorState)
extern int sefPtAllocCount;

unsigned char *sevGetPtAllocator2(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= sefPtAllocCount) {
        return 0;
    }
    return sefPtAllocatorState[slotIndex].data;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203928);

/* Returns line-data record entryIndex, or null for a negative index. */
SefLocalData *sefGetLineAdr(int entryIndex)
{
    if (entryIndex < 0) {
        return 0;
    }
    return &sefLineDataRecords[entryIndex];
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203990);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002039E0);

/* Returns the base of the effect record table. */
SefEffectRecord *sefGetScheduler(void)
{
    return sefEffectRecords;
}

/* Returns the record most recently stored by sefDestroyRecordEffects (the current record). */
SefEffectRecord *sefGetNowScheduler(void)
{
    return sefCurrentRecord;
}

unsigned char *sefGetParentLine(int checkedIndex, int slotIndex)
{
    unsigned char *data;

    if (checkedIndex >= 0 && slotIndex >= 0) {
        data = 0;
        if (slotIndex < sefPtAllocCount) {
            data = sefPtAllocatorState[slotIndex].data;
        }
        return data;
    }
    return 0;
}

#undef SefTrackKey16
#undef sefCurrentRecord
#undef sefPtAllocatorState
/* <<< CHUNK sef_132 */

/* >>> CHUNK tu133 */

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203A68);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203A88);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203AF8);

/* <<< CHUNK tu133 */

/* >>> CHUNK sef_134 */

/* Row-major 4x4 float matrix. */
void sefGetCubePosBtm(Vector4 *pos, const Vector4 *extent);
void sefGetCubePosTop(Vector4 *pos, const Vector4 *extent);
void sefMemZero(void *dest, int size);
void sefMemFillWords(void *dest, int value, int size);
void sefNoiseInit(void);
void sefGetCirclePos(Vector4 *pos, float radius);
void sefMergeMatrixPos(Matrix4 *dest, Matrix4 *matrix, const Vector4 *position);
void sefGetCameraPos(Matrix4 *dest);
int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *out);
float sefNoiseNext(void);
float sefRandf(void);
float sefRandAngle(void);
/* Three random coordinates in [-0.5, 0.5), drawn in x, y, z order; w is set to 1. */
void sefNoiseNextVector(Vector4 *pos);
float xglSin(float angle);
float xglCos(float angle);

/* One 128-bit quadword (sq/lq width). */
typedef unsigned int Quadword __attribute__((mode(TI)));

extern const Matrix4 MMathUnitMatrix;
void MMathRandomSeed(int seed);
float MMathMakeRandom(void);
Matrix4 *MMathRotateMatrixZYX(Matrix4 *out, Matrix4 *matrix, const Vector4 *angles);
Matrix4 *MMathScaleMatrix(Matrix4 *out, Matrix4 *matrix, const Vector4 *scale);
struct XglCamera;

struct XglCamera *xglStudioGetActiveCamera(void);
extern const Vector4 sefDefaultOffset;

typedef union SefQuadBlock {
    Quadword quad;
    unsigned int words[4];
} SefQuadBlock;

/* The LCG of the seed: multiplier 0xF82D and increment 0x303B; the high 15 bits (SEF_RAND_MASK) are the sample. */
#define SEF_RAND_MULTIPLIER 0xF82Du
#define SEF_RAND_INCREMENT 0x303Bu
#define SEF_RAND_MASK 0x7FFF
#define SEF_NOISE_SCALE 32768.0f

/* LCG state of sefRandf, stored at sefRandSeed. */
extern unsigned int sefRandSeed;
/* Set to 1 once sefNoiseInit has filled the noise table. */
extern int sefNoiseReady;

/* 1024 uniform noise samples in [-0.5, 0.5) with a read index. sefNoiseNextVector reads three samples at a time and
 * can read the three floats after the 1024th, which sefNoiseInit also fills (1027 samples in all). */
typedef struct SefNoiseTable {
    f32 samples[1024];
    unsigned char unmodeled_1000[12];
    int nextIndex;
} SefNoiseTable;

extern SefNoiseTable sefNoiseTable;
/* Single-precision 2*pi, read from the scaffold literal sefTwoPi of this unit. */
extern f32 sefTwoPi;
extern unsigned int sefRandVectorSeeds[4];
extern unsigned int sefRandVectorIncrements[4];
extern f32 sefCircleAngleScale;

/* Stores value into each of the size / 4 words starting at dest. */
void sefMemFillWords(void *dest, int value, int size) {
    int *word;
    int count;

    word = dest;
    count = size >> 2;
    if (count > 0) {
        do {
            count -= 1;
            *word = value;
            word += 1;
        } while (count != 0);
    }
}

void sefMemZero(void *dest, int size)
{
    SefQuadBlock *blocks = dest;
    int block_count = size >> 4;
    int word_count = (size & 0xf) >> 2;
    SefQuadBlock *tailBlock = blocks + block_count;
    int i;

    for (i = 0; i < block_count; i++) {
        blocks[i].quad = 0;
    }
    for (i = 0; i < word_count; i++) {
        tailBlock->words[i] = 0;
    }
}

/* Fills the noise table once with uniform samples in [-0.5, 0.5) and resets its read index to zero. */
void sefNoiseInit(void)
{
    f32 sample;
    f32 *cursor;
    int i;

    if (sefNoiseReady == 0) {
        sefNoiseReady = 1;
        MMathRandomSeed(0);
        sefNoiseTable.nextIndex = 0;
        cursor = (f32 *)&sefNoiseTable;
        for (i = 0; i < 1027; i++) {
            sample = MMathMakeRandom() - 0.5f;
            *cursor = sample;
            cursor++;
        }
    }
}

/* Random float in [0, 1): one LCG step, the high 15 bits scaled by 2^-15. */
f32 sefRandf(void)
{
    int bits;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    bits = (sefRandSeed >> 16) & SEF_RAND_MASK;
    return (f32)bits * (1.0f / 32768.0f);
}

/* Returns the next noise sample and advances the read index of the noise table. */
f32 sefNoiseNext(void)
{
    int index = sefNoiseTable.nextIndex;

    sefNoiseTable.nextIndex = (index + 1) & 0x3FF;
    return sefNoiseTable.samples[index];
}

/* Random float in [0.75, 1): one LCG step of sefRandSeed, the high 15 bits scaled by 2^-15, times 0.25, plus 0.75. */
f32 sefRandfUpperQuarter(void)
{
    unsigned int seed;

    seed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    sefRandSeed = seed;
    return (f32) (int) ((seed >> 16) & SEF_RAND_MASK) * 3.0517578125e-05f * 0.25f + 0.75f;
}

/* Random angle in [0, 2*pi): the same LCG step as sefRandf, scaled by 2^-15 and then by 2*pi. */
f32 sefRandAngle(void)
{
    int bits;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    bits = (sefRandSeed >> 16) & 0x7FFF;
    return (f32)bits * (1.0f / 32768.0f) * sefTwoPi;
}

/* Random vector: steps the x, y and z seeds of the vector LCG (times 0xF82D, then all four lanes plus the increments in one paddw), keeps the high 15 bits of each lane and converts them to floats in out. */
void sefRandVector(Vector4 *out)
{
    unsigned int *laneSeeds;
    Quadword mask;                  /* the 128-bit literal of the original: 0x7FFF in x, y and z, but only 1 in w */

    laneSeeds = sefRandVectorSeeds;
    mask = 0x100007FFF00007FFF00007FFFULL;
    laneSeeds[0] *= 0xF82D;
    laneSeeds[1] *= 0xF82D;
    laneSeeds[2] *= 0xF82D;
    __asm__ __volatile__(
        "lq $8, 0(%1)\n"
        "lq $9, 0(%2)\n"
        "paddw $8, $8, $9\n"
        "sq $8, 0(%1)\n"
        "psrlw $8, $8, 16\n"
        "pand $8, $8, %3\n"
        "qmtc2 $8, vf1\n"
        "vitof0.xyzw vf1, vf1\n"
        "sqc2 vf1, 0(%0)\n"
        : : "r"(out), "r"(laneSeeds), "r"(sefRandVectorIncrements), "r"(mask) : "$8", "$9", "memory");
}

/* Three random coordinates in [-0.5, 0.5), drawn in x, y, z order; w is set to 1. */
void sefNoiseNextVector(Vector4 *pos)
{
    pos->x = MMathMakeRandom() - 0.5f;
    pos->y = MMathMakeRandom() - 0.5f;
    pos->z = MMathMakeRandom() - 0.5f;
    pos->w = 1.0f;
}

/* Writes the position transformed by the matrix into dest (the position's translation merged into the matrix). */
void sefMergeMatrixPos(Matrix4 *dest, Matrix4 *matrix, const Vector4 *position)
{
    QuadwordMergeMatrixPosition(dest, matrix, position);
}

/* Builds the effect matrix in out (the scratchpad when out is null): the identity rows of MMathUnitMatrix are copied
 * first, then a nonzero translation becomes row 3, a nonzero rotation is applied, and a scale that is not all ones.
 * Returns how many of the three steps ran. */
int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *out)
{
    Matrix4 *dest;
    const Matrix4 *unitMatrix;
    int stepsRun;
    int hasTranslation;
    int hasRotation;

    dest = out;
    if (dest == 0) {
        dest = (Matrix4 *)0x70000000;
    }
    stepsRun = 0;
    unitMatrix = &MMathUnitMatrix;
    __asm__ __volatile__(
        "lqc2 $vf1, 32(%1)\n"
        "lqc2 $vf2, 16(%1)\n"
        "lqc2 $vf3, 0(%1)\n"
        "sqc2 $vf0, 48(%0)\n"
        "sqc2 $vf1, 32(%0)\n"
        "sqc2 $vf2, 16(%0)\n"
        "sqc2 $vf3, 0(%0)\n"
        :
        : "r" (dest), "r" (unitMatrix)
        : "memory");
    hasTranslation = (*(int *)&translation->z | *(u64 *)translation) != 0;
    if (hasTranslation) {
        QuadwordCopyT0((*dest)[3], translation);
        stepsRun = 1;
    }
    hasRotation = (*(int *)&rotation->z | *(u64 *)rotation) != 0;
    if (hasRotation) {
        MMathRotateMatrixZYX(dest, dest, rotation);
        stepsRun++;
    }
    if (scale != 0 && !(scale->x == 1.0f && scale->y == 1.0f && scale->z == 1.0f)) {
        MMathScaleMatrix(dest, dest, scale);
        stepsRun++;
    }
    return stepsRun;
}

void sefGetCameraPos(Matrix4 *dest)
{
    XglCamera *camera = xglStudioGetActiveCamera();

    if (dest == 0) {
        dest = sefCameraMatrix;
    }
    sefMergeMatrixPos(dest, &camera->travel.transform, &sefDefaultOffset);
}

/* VU0 product of a Vector4 and a column-major matrix: destination = matrix * vector. */
void sefApplyMatrixVector(Vector4 *destination, Matrix4 *matrix, Vector4 *vector)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(vector) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf4, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf2, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf3, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf4, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf1, vf0, vf1w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(destination) : "memory");
}

/* Copies one 16-byte vector through t0 with the R5900 quadword load and store. */
static inline void sefCopyQuadword(Vector4 *dest, const void *src)
{
    __asm__ __volatile__(
        "lq $8, 0(%2)\n"
        "sq $8, 0(%1)\n"
        : "+m"(*dest)
        : "r"(dest), "r"(src)
        : "$8", "memory");
}

/* The record of key (zero when none). */
SefEntry *sefFindEntryById(int key);

/* Copies the vector of slot of the record found by key into *destination when that slot's flag byte is set, and
 * returns bit 0 of the flag byte (0 without a record). */
int sefCopyRecordSlotVector(Vector4 *destination, int key, int slot)
{
    SefEntry *entry = sefFindEntryById(key);

    if (entry == 0) {
        return 0;
    }
    if (entry->slotFlags[slot] != 0) {
        sefCopyQuadword(destination, &entry->slotVectors[slot]);
    }
    return entry->slotFlags[slot] & 1;
}

/* Partial view of one linked object record: its flag byte at +0x155C, bit 0 is the in-use bit. */
typedef struct SefObject {
    unsigned char unmodeled_000[0x155C];
    unsigned char flags;             /* +0x155C */
} SefObject;

/* Returns bit 0 of flag byte linkIndex of the linked object table (objectOffset), or 0 without a link. */
int sefGetLinkedFlag(int linkIndex)
{
    SefLinkedContext *context = sefGetBattleData();
    int offset = context->objectOffset;

    if (offset == 0) {
        return 0;
    }
    return ((SefObject *)(linkIndex + offset))->flags & 1;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204080);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204118);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002042E0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204318);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204510);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002045C8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002046F8);

/* The point writer of this unit, still hand-written asm: it writes the point of kind into dest from source. */
void func_002046F8(Vector4 *dest, const Vector4 *source, int kind);

/* Kinds 321 and 322 are passed on as kind 256; a NULL source reads sefDefaultOffset. The w component is set to 1.0f. */
void sefGetPosition(Vector4 *dest, const Vector4 *source, int kind)
{
    if ((unsigned int)(kind - 321) < 2) {
        kind = 256;
    }
    if (source == 0) {
        source = &sefDefaultOffset;
    }
    func_002046F8(dest, source, kind);
    dest->w = 1.0f;
}

void vu0TransformPoint(Vector4 *point);

/* Partial view of a part's transform state: the matrix rows (vf26..vf28), the vector at +0xA0 (loaded into vf29), the scale
 * at +0xC0 and the point at +0x200 that vu0TransformPoint transforms. */
typedef struct SefPartPlacement {
    Matrix4 matrix;                  /* +0x000 */
    unsigned char unmodeled_040[0xA0 - 0x40];
    Vector4 translation;             /* +0x0A0 */
    unsigned char unmodeled_0B0[0xC0 - 0xB0];
    Vector4 scale;                   /* +0x0C0 */
    unsigned char unmodeled_0D0[0x200 - 0xD0];
    Vector4 point;                   /* +0x200 */
} SefPartPlacement;

/* Transforms the part's point (mode 0: transform, then scale; mode != 0: scale, then transform when mode >= 0), adds the
 * offset's w term when offset is given, and writes the result to out. */
void sefTransformPartPoint(SefPartPlacement *part, Vector4 *out, int mode, const Vector4 *offset)
{
    __asm__ __volatile__(
        "lqc2 vf26, 0(%0)\n"
        "lqc2 vf27, 16(%0)\n"
        "lqc2 vf28, 32(%0)\n"
        "lqc2 vf29, 0(%1)\n"
        :
        : "r"(part), "r"(&part->translation)
        : "memory");
    if (mode != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulx.xyzw vf26, vf26, vf1x\n"
            "vmuly.xyzw vf27, vf27, vf1y\n"
            "vmulz.xyzw vf28, vf28, vf1z\n"
            "vmulw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(&part->scale), "m"(part->scale));
        if (mode >= 0) {
            Vector4 *point = &part->point;
            vu0TransformPoint(point);
        }
    } else {
        vu0TransformPoint(&part->point);
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulx.xyzw vf26, vf26, vf1x\n"
            "vmuly.xyzw vf27, vf27, vf1y\n"
            "vmulz.xyzw vf28, vf28, vf1z\n"
            "vmulw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(&part->scale)
            : "memory");
    }
    if (offset != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulax.xyzw ACC, vf26, vf1x\n"
            "vmadday.xyzw ACC, vf27, vf1y\n"
            "vmaddaz.xyzw ACC, vf28, vf1z\n"
            "vmaddw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(offset)
            : "memory");
    }
    __asm__ __volatile__(
        "sqc2 vf29, 0(%0)\n"
        :
        : "r"(out)
        : "memory");
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002050A8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00205380);

/* Random point on the circle of the given radius in the XZ plane, y zero: the seed steps once, the angle is the
 * sampled fraction of a turn. */
void sefGetCirclePos(Vector4 *pos, f32 radius)
{
    f32 angle;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    angle = (f32)(int)((sefRandSeed >> 16) & SEF_RAND_MASK) / SEF_NOISE_SCALE * sefCircleAngleScale;
    Vu0SetOrigin(pos);
    pos->x = xglCos(angle) * radius;
    pos->z = xglSin(angle) * radius;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00205470);

Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);

/* Unit direction from the three floats x, y and z (w is left as it was), normalized and then scaled by scale in x, y
 * and z. */
void func_00205518(Vector4 *vector, float x, float y, float z, float scale)
{
    vector->x = x;
    vector->y = y;
    vector->z = z;
    MMathNormalizeVector2(vector, vector);
    Vu0SefScaleVectorXYZ(vector, vector, scale);
}

/* Random point of the cube [-0.5, 0.5) x [0, 1) x [-0.5, 0.5) scaled by extent: the bottom-aligned cube. */
void sefGetCubePosBtm(Vector4 *pos, const Vector4 *extent) {
    f32 shiftedY;

    sefNoiseNextVector(pos);
    shiftedY = pos->y + 0.5f;
    pos->y = shiftedY;
    Vu0SefMulVectorXYZ(pos, pos, extent);
}

/* The top-aligned cube: sefGetCubePosBtm with y negated. */
void sefGetCubePosTop(Vector4 *pos, const Vector4 *extent)
{
    sefGetCubePosBtm(pos, extent);
    pos->y = -pos->y;
}

/* Random point of the centred cube [-0.5, 0.5)^3 scaled by the extent vector. */
void sefGetCubePosCenter(Vector4 *destination, Vector4 *scale)
{
    sefNoiseNextVector(destination);
    Vu0SefMulVectorXYZ(destination, destination, scale);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00205638);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00205DF0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206058);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206A50);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206AA8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206C10);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206C80);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206D78);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00206F18);

#define D_004541A0 (*(SefEntryTable *)&D_004541A0)
extern Vector4 D_004748A0;

/* Zeroes the origin, then clears the flags of every entry and the short at +0x1592 of each. */
void sefResetEntryFlags(void)
{
    SefEntry *entry;
    int i;

    Vu0SetOrigin(&D_004748A0);
    for (i = 0; i < D_004541A0.entryCount; i++) {
        entry = &D_004541A0.entries[i];
        entry->flags = 0;
        *(short *) &entry->unmodeled_157C[0x1592 - 0x157C] = 0;
    }
}

/* TU-local view of D_004ABFD0 (the same 0x58-byte transform block as SefPartTransform in
 * sv_get_image_item_143.c; no header defines it yet). */
typedef struct SefGlobalTransform {
    Matrix4 matrix;                  /* +0x00 */
    Vector4 position;                /* +0x40 */
    float scale;                     /* +0x50 */
    short scaled;                    /* +0x54: nonzero when scale applies */
    short kind;                      /* +0x56 */
} SefGlobalTransform;

extern SefGlobalTransform D_004ABFD0;
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
Vector4 *MMathApplyMatrix(Vector4 *out, Matrix4 *matrix, const Vector4 *vector);

/* Places the part: the combined matrix of the placement and the global transform (its translation first offset by the
 * global position when the scale applies), applied to the placement's point when mode >= 0, otherwise the matrix's
 * position row copied to out. Returns the global scale when the scale applies, else 1. */
float sefPlacePart(SefPartPlacement *placement, Vector4 *out, int mode)
{
    Matrix4 combined;
    float scale = 1.0f;
    AlignedVector4 localTranslation;

    if (D_004ABFD0.scaled) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%1)\n"
            "lqc2 vf2, 0(%2)\n"
            "vsub.xyz vf1, vf1, vf2\n"
            "sqc2 vf1, 0(%3)\n"
            : "=m"(localTranslation)
            : "r"(&placement->translation), "r"(&D_004ABFD0.position), "r"(&localTranslation),
              "m"(placement->translation), "m"(D_004ABFD0.position));
        sefMergeMatrixPos(&combined, &placement->matrix, &localTranslation);
        MMathMulMatrix(&combined, &D_004ABFD0.matrix, &combined);
        scale = D_004ABFD0.scale;
    } else {
        sefMergeMatrixPos(&combined, &placement->matrix, &placement->translation);
    }
    if (mode >= 0) {
        Vector4 *point = &placement->point;

        MMathApplyMatrix(out, &combined, point);
        return scale;
    }
    sefCopyQuadword(out, combined[3]);
    return scale;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002071F8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00207490);

#undef D_004541A0
/* <<< CHUNK sef_134 */

/* >>> CHUNK sef_135 */

int sefFlagTest(int mask);

void sefFlagSet(int mask);
void sefTickGroupTimers(void);
void sefGetGroupCenter(float *center, int owner, unsigned short key);

#define SEF_GROUP_ENTRY_COUNT 8

/*
 * One group timer, 0x20 bytes, eight of them in sefGroupTable. sefRegisterGroupCenter fills it for a tracked group
 * (the centre of the group's objects, the owner id and the key that name the group) with 600 frames on the timer;
 * sefTickGroupTimers counts `remaining` down; sefGetGroupCenter reads the centre back by owner and key.
 */
typedef struct SefGroupEntry {
    float center[4];               /* +0x00: x, y, z, w of the group centre */
    int owner;                     /* +0x10: owner id compared by sefGetGroupCenter and sefRegisterGroupCenter */
    short remaining;               /* +0x14: frames left; zero or less means the entry is free */
    unsigned short key;            /* +0x16: second id of the group, compared as an unsigned short */
    unsigned char unmodeled_18[8]; /* +0x18..0x1F: not read by the code in this unit */
} SefGroupEntry;

extern SefGroupEntry sefGroupTable[SEF_GROUP_ENTRY_COUNT];
/* sefFlagWord is one flag word; sefExecEffect tests bit 1 of it through sefFlagTest. */
extern int sefFlagWord;
void sefFlagClear(int mask);

/* Counts every group timer down by one frame, stopping at zero. */
void sefTickGroupTimers(void)
{
    SefGroupEntry *entry;
    int index;

    entry = sefGroupTable;
    for (index = SEF_GROUP_ENTRY_COUNT - 1; index >= 0; index--) {
        if (entry->remaining > 0) {
            entry->remaining = entry->remaining - 1;
        }
        entry++;
    }
}

/*
 * Writes the origin vector (0, 0, 0, 1) to center, then copies the centre of the first table entry whose owner and key
 * match. Leaves center as the origin when no entry matches.
 */
void sefGetGroupCenter(float *center, int owner, unsigned short key)
{
    SefGroupEntry *entry;
    int index;

    Vu0SetOrigin(center);
    entry = sefGroupTable;
    for (index = 0; index < SEF_GROUP_ENTRY_COUNT; index++) {
        if (entry->owner == owner && entry->key == key) {
            QuadwordCopyT0(center, entry->center);
            return;
        }
        entry++;
    }
}

#define SEF_GROUP_MEMBER_MAX 24
#define SEF_GROUP_TIMER_FRAMES 600
#define SEF_RECORD_EXCLUDE_FLAG 0x8000000

/* Effect record count word (gp-24972); the scaffold has no name for it yet. */
extern int D_00551064;

SefEffectRecord *sefGetScheduler(void);
Vector4 *MMathDivVectorS(Vector4 *destination, const Vector4 *source, float scale);

/*
 * Copies a group centre into its timer entry. Both registration paths share one copy block in the original, which
 * gcc only produces when the two copies come from the same asm statement.
 */
static inline void sefGroupStoreCenter(SefGroupEntry *entry, Vector4 *centre)
{
    QuadwordCopyT0(entry->center, centre);
}

/*
 * Gathers the live effect records owned by owner under key. Each distinct object is counted once and its position is
 * summed into the group centre; the centre is averaged when two or more objects contributed. The centre is written to
 * the table entry for (owner, key) if one exists, otherwise to the first free entry, which gets a 600-frame timer.
 */
void sefRegisterGroupCenter(int owner, unsigned short key)
{
    int index;
    int seenCount;
    int seenIndex;
    SefEffectRecord *records;
    SefEffectRecord *record;
    SefGroupObject *object;
    SefGroupObject *seen[SEF_GROUP_MEMBER_MAX];
    AlignedVector4 sum;
    Vector4 *center;
    SefGroupEntry *entry;

    records = sefGetScheduler();
    center = &sum;
    seenCount = 0;
    Vu0SetOrigin(center);
    for (index = 0; index < D_00551064; index++) {
        record = &records[index];
        if (record->inUse == 0 || record->ownerKey != owner) {
            continue;
        }
        object = record->object;
        if (object == 0 || record->value != key) {
            continue;
        }
        if (record->flags & SEF_RECORD_EXCLUDE_FLAG) {
            continue;
        }
        for (seenIndex = 0; seenIndex < seenCount; seenIndex++) {
            if (seen[seenIndex] == record->object) {
                break;
            }
        }
        if (seenIndex != seenCount) {
            continue;
        }
        /* center.xyz += object position; no memory clobber, so the record count load stays hoisted */
        __asm__ __volatile__(
            "lqc2 vf1, 0(%1)\n"
            "lqc2 vf2, 0(%2)\n"
            "vadd.xyz vf1, vf1, vf2\n"
            "sqc2 vf1, 0(%0)\n"
            :
            : "r" (center), "r" (center), "r" (&object->position));
        seen[seenCount] = object;
        seenCount++;
        if (seenCount >= SEF_GROUP_MEMBER_MAX) {
            break;
        }
    }
    if (seenCount >= 2) {
        MMathDivVectorS(center, center, (float) seenCount);
    }

    for (index = 0; index < SEF_GROUP_ENTRY_COUNT; index++) {
        entry = &sefGroupTable[index];
        if (entry->owner == owner && entry->key == key) {
            entry->remaining = SEF_GROUP_TIMER_FRAMES;
            entry->key = key;
            sefGroupStoreCenter(entry, center);
            return;
        }
    }

    for (index = 0; index < SEF_GROUP_ENTRY_COUNT; index++) {
        entry = &sefGroupTable[index];
        if (entry->remaining == 0) {
            entry->remaining = SEF_GROUP_TIMER_FRAMES;
            entry->owner = owner;
            entry->key = key;
            sefGroupStoreCenter(entry, center);
            return;
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00207948);

INCLUDE_ASM("asm/main/nonmatchings/sef", sefFlagSet);

INCLUDE_ASM("asm/main/nonmatchings/sef", sefFlagClear);

int sefFlagTest(int mask) {
    return sefFlagWord & mask;
}

void sefFlagToggle(int mask) {
    sefFlagWord ^= mask;
}

void sefFlagUpdateBit1(int selector) {
    if (selector < 0) {
        sefFlagToggle(1);
        return;
    }
    if (selector > 0) {
        sefFlagSet(1);
        return;
    }
    sefFlagClear(1);
}

/* Counts the records of owner under key that the owner-group flags do not exclude. */
int sefCountOwnerGroupRecords(int owner, unsigned short key)
{
    SefEffectRecord *record;
    int index;
    int count;

    count = 0;
    record = sefGetScheduler();
    for (index = 0; index < D_00551064; index++) {
        if (record->inUse != 0 && record->ownerKey == owner && record->value == key) {
            if ((record->flags & SEF_RECORD_EXCLUDE_FLAG) == 0) {
                count += 1;
            }
        }
        record++;
    }
    return count;
}

/* Clears the mask bits of the records of owner under key, except records that carry SEF_RECORD_EXCLUDE_FLAG. */
void sefClearOwnerGroupFlags(int owner, unsigned short key, int mask)
{
    SefEffectRecord *record;
    int slot;
    int scanLimit;
    int recordCount;
    int keepBit;
    int flags;

    record = sefGetScheduler();
    scanLimit = D_00551064;
    slot = 0;
    if (scanLimit > 0) {
        keepBit = SEF_RECORD_EXCLUDE_FLAG;
        recordCount = scanLimit;
        do {
            if (record->inUse != 0 && record->ownerKey == owner && record->value == key) {
                flags = record->flags;
                if ((flags & keepBit) == 0) {
                    record->flags = flags & ~mask;
                    scanLimit = recordCount;
                }
            }
            slot += 1;
            record++;
        } while (slot < scanLimit);
    }
}

/* <<< CHUNK sef_135 */
