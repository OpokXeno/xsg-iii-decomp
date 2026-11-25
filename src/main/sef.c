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
#include "main/sef_132.h"

/* Row-major 4x4 float matrix. */
/* A track key whose time is 0x400 ends the track: no key follows it. */
#define SEF_TRACK_END_KEY 0x1000
/* The same end time as a frame count for the float compares of the keyframe cursors. */
#define SEF_TRACK_END_TIME 4096.0f
/* Pending bit of gameWork.control that holds the effect draw passes back (the X1 SEF_DRAW_FLAG_PENDING value). */
#define SEF_DRAW_FLAG_PENDING 0x04000000

void svDrawScene3D(void);
void vu0NormalizeVf20(void);
Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);

/* Triangle wave record of the camera shake: value is the current amplitude, step the amount it decays
 * per cycle; period and wavePhase are in frames, hold delays the decay, remaining counts the cycles left. */
typedef struct CameraWave {
    float amplitude;                            /* +0x00 */
    float decayPerCycle;                        /* +0x04 */
    unsigned short period;                      /* +0x08 */
    unsigned short wavePhase;                       /* +0x0A */
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
    Vector4 lookAtOffset;                             /* +0x20: the look-at offset above the anchor */
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
    unsigned short cameraFlags;                       /* +0xD0: CAMERA_FLAG_* bits (game_camera.c) */
    unsigned char cameraType;                   /* +0xD2: the camera type of the active record */
    unsigned char cameraSubType;                /* +0xD3: cleared with typeState when the camera type changes */
    signed char activeRecordIndex;                    /* +0xD4: the camera record in use */
    signed char changeMode;                           /* +0xD5: set by GameCameraChangeID */
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
    CameraSlot transitionSlots[4];                         /* +0x140: 0 look-at, 1 position and angles, 2 perspective, 3 roll */
    CameraRange currentCameraRange;                          /* +0x180: the current camera, laid out like a camera record range */
    CameraPose transitionFrom;                            /* +0x1F0: the values a slot transition starts from */
    CameraPose transitionTo;                              /* +0x220: the values a slot transition ends at */
    Vector4 transitionFromOffset;                         /* +0x250: the offset the position transition starts from */
    Vector4 anchorOffset;                       /* +0x260: the anchor offset vector */
    float anchorBase[3];                        /* +0x270 */
    unsigned char unmodeled_027C[0x4];
} GameWorkCamera;

/* One entry of the elevator slope table. */
typedef struct GameWorkSlope {
    float x;
    float z;
    float push;
    float travelSpeed;
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
    PxyModelHeader *resourceHeader;          /* +0x00 */
    int jpegData;               /* +0x04 */
    unsigned char unmodeled_08[0x04];
    struct MapFixupBlock *collisionBlock; /* +0x0C: the collision block mapLoadBlock registers */
    unsigned char *imageHead;   /* +0x10 */
    int imageOffset;            /* +0x14 */
    unsigned char unmodeled_18[0x08];
    int *jpegStream;            /* +0x20 */
    unsigned char *pending;     /* +0x24: decoded texture */
    short offsetY;              /* +0x28 */
    short offsetX;              /* +0x2A */
    unsigned short height;      /* +0x2C */
    unsigned short width;       /* +0x2E */
    ScreenVector4 backdropOrigin;         /* +0x30: the backdrop origin projected by mapBackdropDraw (x is the row, y the column) */
} MainActive;

struct SbImageHeader;

/* The game work block (0x1B30 bytes, one object owned by game_data.c): the scene state, the camera, the
 * player parameters, the light sets and the event flags. The offsets in the comments are gameWork
 * offsets. control, camera.cameraFlags and anchorIndex are plain words; a function that depends on reading
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
    short nextKeyIndex;                              /* +0x02 */
    unsigned short sampleValue;                            /* +0x04 */
    unsigned char unmodeled_006[2];
} SefKeyframe;

/* A track's cursor: the current key index, the sampled value and the
 * playback frame (float). */
typedef struct SefKeyCursor {
    short keyIndex;                                  /* +0x00 */
    unsigned short sampleValue;                            /* +0x02 */
    float frame;                                     /* +0x04 */
} SefKeyCursor;

/* The 2D draw flag word: sefDrawEffect2D reads it before its camera and end-of-3D-mode calls. */
extern int sefDraw2DFlag[];
/* The 3D draw flag word: sefDrawEffect3D draws the 3D scene only while it is non-zero. */
extern int sefDraw3DFlag[];

/* One 0x2C0-byte entry of the table at sefPtAllocatorState. head and tail are the free-slot ring indices of
 * sefPtFreeSlots and are used only through entry 0 (the ring control block); the getters return the entry's data,
 * which starts after a 16-byte prefix. */
typedef struct SefPtRingControl {
    int freeHead;
    int freeTail;
    unsigned char unmodeled_08[0x10 - 0x08];
    unsigned char allocatorBytes[0x2C0 - 0x10];
} SefPtRingControl;

extern SefPtRingControl *sefPtAllocatorState;
extern int sefPtAllocCount;
extern short sefPtFreeSlots[1104];

/* A track cursor: the key index it is on, the running time and the
 * interpolated samples the track readers store. */
typedef struct SefTrackCursor {
    short keyIndex;
    short sampled_s16;
    float time;
    float sampled;
} SefTrackCursor;

extern f32 sefTrackFrameStep[];
SefTrackKey *sefAdvanceTrackCursor(SefTrackKey *keys, SefTrackCursor *cursor);
extern s16 sefInitialized;
void sefDrawEffect3DCallback(void);
void sefDrawEffect3D(void);

/* One 0x18-byte record of the entry count table: four counters, their sum at +0x10 and an unused word.
 * The record after the last entry (index 44) holds the running sums of the counters and of the totals. */
typedef struct SefEffectTally {
    unsigned int laneCounts[4];                     /* +0x00 */
    unsigned int grandTotal;                         /* +0x10 */
    unsigned int unmodeled_14;                  /* +0x14 */
} SefEffectTally;

extern SefEffectTally sefEffectTallies[];

/* Address of entry index in the table sefEffectTallies (SefEffectTally, 0x18 bytes each). */
SefEffectTally *sefEffectTallyAt(int tallyIndex)
{
    return &sefEffectTallies[tallyIndex];
}

/* Returns the 2D draw flag word that sefDrawEffect2D reads. */
int sefGetDraw2DFlag(void) {
    return sefDraw2DFlag[0];
}

/* Returns the 3D draw flag word that sefDrawEffect3D reads. */
int sefGetDraw3DFlag(void) {
    return sefDraw3DFlag[0];
}

typedef struct SefTrackKey16 {
    unsigned char unmodeled_0000[0xC];
    short key;                                  /* +0x0C */
    signed char nextKeyIndex;                           /* +0x0E */
    unsigned char unmodeled_000F[0x1];
} SefTrackKey16;

typedef struct SefEffectSlot {
    short lineHandles[32];                          /* +0x00 */
    unsigned char lineTags[32];                     /* +0x40 */
    int id;                                         /* +0x60 */
    unsigned int flags;                             /* +0x64 */
    short effectKind;                               /* +0x68 */
    signed char lineCount;                          /* +0x6A */
} SefEffectSlot;

typedef struct SefColorTrackCursor {
    SefTrackCursor track;                            /* +0x00 */
    unsigned char unmodeled_00c[0x10 - 0x0c];
    int color[4];                                    /* +0x10 */
} SefColorTrackCursor;

typedef struct SefAnalyzeRecord {
    short time;                                 /* +0x00 */
    unsigned char unmodeled_02[1];
    unsigned char flags;                        /* +0x03 */
    short imageId;                              /* +0x04 */
    short altImageId;                           /* +0x06 */
    short secondaryImageId;                             /* +0x08 */
    short secondaryPaletteId;                          /* +0x0A */
    unsigned char unmodeled_0C[2];
} SefAnalyzeRecord;

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

typedef struct SvTypeListEntry {
    void *pixelData;                                 /* +0x00 */
    unsigned char unmodeled_04[0x0C - 0x04];
    unsigned short resourceKind;                /* +0x0C */
    unsigned char unmodeled_0E[0x12 - 0x0E];
    unsigned short texBasePointer;                 /* +0x12 */
    unsigned short bitsPerPixel;                /* +0x14 */
    unsigned short bufferWidthPixels;           /* +0x16 */
    unsigned short width;                       /* +0x18 */
    unsigned short height;                      /* +0x1A */
    unsigned short textureWidthExponent;        /* +0x1C */
    unsigned short textureHeightExponent;       /* +0x1E */
} SvTypeListEntry;

typedef struct SefPtAllocatorEntry {
    unsigned char unmodeled_00[0x10];
    unsigned char allocatorBytes[0x270 - 0x10];
} SefPtAllocatorEntry;

extern s32 sefDrawEnabledFlag[];

/* Returns the draw enabled flag. */
s32 sefGetDrawEnabledFlag(void) {
    return sefDrawEnabledFlag[0];
}

void sefSumEffectTallies(void)
{
    SefEffectTally *tallyTable;
    SefEffectTally *tallyBase;
    unsigned int *cursor;
    int i;

    tallyTable = sefEffectTallies;
    tallyTable[44].laneCounts[0] = 0;
    tallyTable[44].laneCounts[1] = 0;
    tallyTable[44].laneCounts[2] = 0;
    tallyTable[44].laneCounts[3] = 0;
    tallyTable[44].grandTotal = 0;
    tallyBase = tallyTable;
    tallyTable = tallyTable + 44;
    cursor = &tallyBase[0].grandTotal;
    for (i = 0; i < 44; i++) {
        unsigned int *tallyTotal = cursor;
        unsigned int laneTwoCount = tallyTotal[-2];
        unsigned int evenLaneSum = tallyTotal[-4] + laneTwoCount;
        unsigned int oddLaneSum = tallyTotal[-1] + tallyTotal[-3];

        *tallyTotal = evenLaneSum + oddLaneSum;
        tallyBase[44].laneCounts[0] += tallyTotal[-4];
        tallyBase[44].laneCounts[1] += tallyTotal[-3];
        tallyBase[44].laneCounts[3] += tallyTotal[-1];
        tallyBase[44].laneCounts[2] += tallyTotal[-2];
        tallyBase[44].grandTotal += *tallyTotal;
        cursor += 6;
    }
}

/* Table of atan values indexed by the ratio of the two arguments, 1024 entries. */
extern float *srsAtanTable;

/* Two-argument arctangent: the angle of (x, y) in radians, from the table over the ratio of the smaller
 * argument to the larger one, with the quadrant corrections applied. */
float srsAtan2(float x, float y)
{
    int y_is_positive = y >= 0.0f;
    int x_is_positive = x >= 0.0f;
    float atan2Angle = 0.0f;
    float angle;

    if (y == 0.0f) {
        if (x != 0.0f) {
            if (x_is_positive) {
                atan2Angle = 1.5707964f;
            } else {
                atan2Angle = -1.5707964f;
            }
        }
    } else if (x == 0.0f) {
        if (!y_is_positive) {
            atan2Angle = 3.1415927f;
        }
    } else {
        if (!y_is_positive) {
            y = -y;
        }
        if (!x_is_positive) {
            x = -x;
        }

        if (x <= y) {
            angle = srsAtanTable[(int)(x * 1024.0f / y)];
            if (y_is_positive) {
                if (!x_is_positive) {
                    angle = -angle;
                }
            } else if (x_is_positive) {
                angle = 3.1415927f - angle;
            } else {
                angle += -3.1415927f;
            }
        } else {
            angle = srsAtanTable[(int)(y * 1024.0f / x)];
            if (y_is_positive) {
                if (x_is_positive) {
                    angle = 1.5707964f - angle;
                } else {
                    angle += -1.5707964f;
                }
            } else if (x_is_positive) {
                angle += 1.5707964f;
            } else {
                angle = -1.5707964f - angle;
            }
        }
        atan2Angle = angle;
    }
    return atan2Angle;
}

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
extern int sefRequestKeyBlock[];

/* Clears the particle allocator state and the request key block, then fills the free-slot ring with 0..count-1. */
void sevInitPtAllocator(void)
{
    int i;
    int allocatorCount;

    sefMemZero(sefPtAllocatorState, 0xA7950);
    sefMemZero(sefRequestKeyBlock, 736);
    allocatorCount = sefPtAllocCount;
    for (i = 0; i < allocatorCount; i++) {
        sefPtFreeSlots[i] = i;
    }
}

/* Takes the next free slot index from the sefPtFreeSlots ring (head side); returns -1 when the ring is empty. */
int sevAllocPtAllocator(void)
{
    SefPtRingControl *ring = sefPtAllocatorState;
    int head = ring->freeHead;
    int nextHead = head + 1;
    int allocatedSlot;

    if (!(nextHead < sefPtAllocCount)) {
        nextHead = 0;
    }
    if (nextHead == ring->freeTail) {
        return -1;
    }
    allocatedSlot = sefPtFreeSlots[head];
    ring->freeHead = nextHead;
    return allocatedSlot;
}

void sevFreePtAllocator(int freedSlot)
{
    SefPtRingControl *ring = sefPtAllocatorState;
    int tail = ring->freeTail;
    int nextTail;

    if (freedSlot < 0) {
        return;
    }
    if (!(freedSlot < sefPtAllocCount)) {
        return;
    }
    if (ring->freeHead == tail) {
        return;
    }
    nextTail = tail + 1;
    sefPtFreeSlots[tail] = freedSlot;
    ring->freeTail = nextTail;
    if (!(nextTail < sefPtAllocCount)) {
        ring->freeTail = 0;
    }
}

extern unsigned char sefKeyframeStepBlock[];

/* The step table sefKeyframeStepBlock: the float step of the keyframe cursors at +0xC. */
typedef struct SefCursorStepBlock {
    unsigned char unmodeled_00[0xC];            /* +0x00..0x0B: not read by this unit */
    f32 frameStep;                              /* +0x0C */
} SefCursorStepBlock;

#define sefStepTable (*(SefCursorStepBlock *) sefKeyframeStepBlock)

/* One 8-byte key of a keyframe table: the time is read unsigned, link is the signed index of the next key (-1 ends
 * the track) and value is the sample the key holds. */
typedef struct SefKeyframeEntry {
    unsigned short time;                        /* +0x00 */
    short nextKeyIndex;                                 /* +0x02 */
    float sampleValue;                                /* +0x04 */
} SefKeyframeEntry;

/* Advances a keyframe cursor by one frame step and returns the key it is on afterwards. When the step reaches the
 * next key, the cursor moves to it, or to the key its link names. */
SefKeyframeEntry *sefAdvanceKeyframeCursor(SefKeyframeEntry *keyTable, SefTrackCursor *cursor)
{
    f32 jumpTime;
    f32 frameTime;
    s32 linkedIndex;
    SefKeyframeEntry *currentKey;

    frameTime = cursor->time + sefStepTable.frameStep;
    currentKey = &keyTable[cursor->keyIndex];
    cursor->time = frameTime;
    if ((frameTime < SEF_TRACK_END_TIME) && (currentKey[1].time <= frameTime)) {
        currentKey++;
        linkedIndex = currentKey->nextKeyIndex;
        if (linkedIndex >= 0) {
            cursor->keyIndex = (u16) currentKey->nextKeyIndex;
            currentKey = &keyTable[linkedIndex];
            jumpTime = currentKey->time + sefStepTable.frameStep;
            cursor->time = jumpTime;
        } else {
            cursor->keyIndex = (u16) cursor->keyIndex + 1;
        }
    }
    return currentKey;
}

SefTrackKey16 *sefAdvanceVectorKeyframeCursor(SefTrackKey16 *keyTable, SefTrackCursor *cursor) {
    f32 frameTime;
    s32 linkedIndex;
    SefTrackKey16 *currentKey;

    frameTime = cursor->time + *sefTrackFrameStep;
    currentKey = &keyTable[cursor->keyIndex];
    cursor->time = frameTime;
    if ((frameTime < SEF_TRACK_END_TIME) && (currentKey[1].key <= frameTime)) {
        currentKey++;
        linkedIndex = currentKey->nextKeyIndex;
        if (linkedIndex >= 0) {
            cursor->keyIndex = linkedIndex;
            currentKey = &keyTable[linkedIndex];
            cursor->time = currentKey->key;
        } else {
            cursor->keyIndex = (u16) cursor->keyIndex + 1;
        }
    }
    return currentKey;
}

/* Advances a track cursor by one frame step and returns the key it is on afterwards. When the step reaches the
 * next key, the cursor moves to it, or to the key its link names. */
SefTrackKey *sefAdvanceTrackCursor(SefTrackKey *keys, SefTrackCursor *cursor)
{
    f32 frameTime;
    int wholeFrame;
    SefTrackKey *currentKey;
    short nextKeyIndex;

    frameTime = cursor->time + sefTrackFrameStep[0];
    currentKey = &keys[cursor->keyIndex];
    wholeFrame = frameTime;
    cursor->time = frameTime;
    if (wholeFrame < SEF_TRACK_END_KEY && currentKey->key != SEF_TRACK_END_KEY && wholeFrame >= (currentKey + 1)->key) {
        currentKey++;
        nextKeyIndex = (short)currentKey->nextKeyIndex;
        if (nextKeyIndex >= 0) {
            cursor->keyIndex = currentKey->nextKeyIndex;
            currentKey = &keys[nextKeyIndex];
            cursor->time = currentKey->key;
        } else {
            cursor->keyIndex = (u16) cursor->keyIndex + 1;
        }
    }
    return currentKey;
}

/* Advances a vector track cursor by one frame step: the same walk as sefAdvanceTrackCursor over 16-byte keys
 * (key time at +0x0C, the signed link at +0x0E). */
SefTrackKey16 *sefAdvanceVectorTrackCursor(SefTrackKey16 *keys, SefTrackCursor *cursor)
{
    f32 frameTime;
    int wholeFrame;
    SefTrackKey16 *currentKey;
    signed char nextKeyIndex;

    frameTime = cursor->time + sefTrackFrameStep[0];
    currentKey = &keys[cursor->keyIndex];
    wholeFrame = frameTime;
    cursor->time = frameTime;
    if (wholeFrame < SEF_TRACK_END_KEY && currentKey->key != SEF_TRACK_END_KEY && wholeFrame >= (currentKey + 1)->key) {
        currentKey++;
        nextKeyIndex = currentKey->nextKeyIndex;
        if (nextKeyIndex >= 0) {
            cursor->keyIndex = nextKeyIndex;
            currentKey = &keys[nextKeyIndex];
            cursor->time = currentKey->key;
        } else {
            cursor->keyIndex = (u16) cursor->keyIndex + 1;
        }
    }
    return currentKey;
}

/* Number of point handles in one local effect record. */
#define SEF_LINE_PARTICLE_COUNT 0x100

void *memset(void *destination, int value, unsigned int size);
void sefFreeEffectData(SefEffectSlot *slot, unsigned int index);

/* Picks the four keys around keys[0] for a Hermite segment: the previous key (keys[0] itself at the first key),
 * the next key and the key after it, which follows the link of keys[1] when it has one. A key whose time is 4096
 * is the end of the track, so the next key then stays keys[0]. */
void MMathHermiteKeyNeighbors(SefHermiteKey *keys, SefHermiteKey **outPreviousKey, SefHermiteKey **outCurrentKey, SefHermiteKey **outNextKey, SefHermiteKey **outAfterNextKey)
{
    SefHermiteKey *previousKey;
    SefHermiteKey *nextKey;
    SefHermiteKey *afterNextKey;

    previousKey = &keys[-1];
    if (keys[0].key == 0) {
        previousKey = keys;
    }
    nextKey = &keys[1];
    afterNextKey = &keys[2];
    if (keys[1].key == SEF_TRACK_END_KEY) {
        nextKey = keys;
        afterNextKey = keys;
    } else if (keys[1].nextKeyIndex >= 0) {
        afterNextKey = &keys[keys[1].nextKeyIndex];
    }
    if (keys[2].key == SEF_TRACK_END_KEY && keys[1].nextKeyIndex < 0) {
        afterNextKey = nextKey;
    }
    *outPreviousKey = previousKey;
    *outCurrentKey = keys;
    *outNextKey = nextKey;
    *outAfterNextKey = afterNextKey;
}

/* Maps the elapsed time of a segment of the given duration to an eased fraction in 0..1: ease mode 1 is an
 * in-out quartic, 2 an ease-in square, 3 an ease-out square, any other mode is linear. */
float sefEaseCurve(int easeMode, float time, float duration)
{
    float scale;
    float halfDuration;
    float doubled;
    float rate;

    scale = 1.0f / duration;
    switch (easeMode) {
    case 1:
        doubled = scale + scale;
        halfDuration = duration * 0.5f;
        if (time <= halfDuration) {
            rate = time * doubled;
            rate = rate * rate * rate * rate * 0.5f;
        } else {
            rate = 1.0f - (time - halfDuration) * doubled;
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

/* The curve kind (low nibble of curveFlags) that selects the Hermite segment. */
#define SEF_CURVE_KIND_HERMITE 1
/* The low nibble of curveFlags is the curve kind; the high nibble is the ease mode. */
#define SEF_CURVE_KIND_MASK 0xF
#define SEF_CURVE_EASE_SHIFT 4

/* A vector track cursor and the vector it samples into. */
typedef struct SefVectorTrackCursor {
    SefTrackCursor track; /* +0x00 */
    unsigned char unmodeled_00C[0x10 - 0x0C];
    Vector4 value; /* +0x10 */
} SefVectorTrackCursor;

void MMathCalcHermitePrm(Vector4 *prm, Vector4 *tangent, SefHermiteKey *prev, SefHermiteKey *cur, SefHermiteKey *next, SefHermiteKey *after);
void MMathCalcHermite(Vector4 *value, Vector4 *prm, Vector4 *tangent, SefHermiteKey *cur, SefHermiteKey *next, float fraction);
void func_0025A000(Vector4 *value, SefHermiteKey *key, SefHermiteKey *nextKey, float fraction);

/* Samples the vector track of cursor into cursor->value: advances the track, then eases the segment that starts at
 * the current key (a plain segment, or a Hermite one when its curve kind is SEF_CURVE_KIND_HERMITE). Returns 0
 * when the track is already at its end key, 1 otherwise. */
int sefSampleVectorCurveTrack(SefHermiteKey *keys, SefVectorTrackCursor *cursor)
{
    Vector4 *trackValue;
    SefHermiteKey *curveKey;
    SefHermiteKey *previousKey;
    SefHermiteKey *currentKey;
    SefHermiteKey *nextKey;
    SefHermiteKey *afterNextKey;
    Vector4 prm;
    Vector4 tangent;
    int keyStartTime;
    float keySpan;
    float easedFraction;

    trackValue = &cursor->value;
    if ((&keys[cursor->track.keyIndex])[1].key == SEF_TRACK_END_KEY) {
        return 0;
    }
    curveKey = (SefHermiteKey *) sefAdvanceVectorKeyframeCursor((SefTrackKey16 *) keys, &cursor->track);
    keyStartTime = curveKey->key;
    keySpan = (float) ((curveKey + 1)->key - keyStartTime);
    if (keySpan != 0.0f) {
        easedFraction = sefEaseCurve(curveKey->curveFlags >> SEF_CURVE_EASE_SHIFT,
                                     cursor->track.time - (float) keyStartTime, keySpan);
        switch (curveKey->curveFlags & SEF_CURVE_KIND_MASK) {
        case 0:
        default:
            func_0025A000(trackValue, curveKey, curveKey + 1, easedFraction);
            break;
        case SEF_CURVE_KIND_HERMITE:
            MMathHermiteKeyNeighbors(curveKey, &previousKey, &currentKey, &nextKey, &afterNextKey);
            MMathCalcHermitePrm(&prm, &tangent, previousKey, currentKey, nextKey, afterNextKey);
            MMathCalcHermite(trackValue, &prm, &tangent, currentKey, nextKey, easedFraction);
            break;
        }
    }
    return 1;
}

/* One 16-byte key of a vector track: the key time is the short at +0x0C and the link the signed byte at +0x0E. */
/* A colour track's cursor: the track cursor, then the four integer colour lanes written at +0x10. */
/* Samples a colour track at the cursor's time: the two keys the cursor spans are interpolated per lane,
 * scaled by the track's colour, clamped to 255 and stored as integers in the cursor's colour lanes. */
void sefSampleColorTrack(SefTrackKey16 *keys, SefColorTrackCursor *cursor, const Vector4 *scale)
{
    SefTrackKey16 *key;
    int *lanes;
    short startKey;
    short endKey;
    float fraction;

    key = &keys[cursor->track.keyIndex];
    if ((key + 1)->key != SEF_TRACK_END_KEY) {
        lanes = cursor->color;
        key = sefAdvanceVectorTrackCursor(keys, &cursor->track);
        startKey = key->key;
        endKey = (key + 1)->key;
        if (startKey != endKey) {
            fraction = (cursor->track.time - startKey) / (endKey - startKey);
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

    key = &keys[cursor->keyIndex];
    if ((key + 1)->key == SEF_TRACK_END_KEY) {
        sample = key->sampleValue;
    } else {
        key = sefAdvanceTrackCursor(keys, cursor);
        if (key->key == (key + 1)->key) {
            return;
        }
        sample = ((key + 1)->sampleValue - key->sampleValue)
               * ((cursor->time - key->key) / ((key + 1)->key - key->key))
               + key->sampleValue;
    }
    cursor->sampled = sample;
}

void sefSampleTrackTruncated(SefTrackKey *keys, SefTrackCursor *cursor)
{
    SefTrackKey *key;
    f32 sample;

    key = &keys[cursor->keyIndex];
    if (key->key == SEF_TRACK_END_KEY || (key + 1)->key == SEF_TRACK_END_KEY) {
        return;
    }
    key = sefAdvanceTrackCursor(keys, cursor);
    if (key->key == (key + 1)->key) {
        return;
    }
    sample = ((key + 1)->sampleValue - key->sampleValue)
           * ((cursor->time - key->key) / ((key + 1)->key - key->key))
           + key->sampleValue;
    cursor->sampled_s16 = sample;
}

void sefSampleKeyframeTrack(SefKeyframeEntry *keyTable, SefTrackCursor *cursor) {
    cursor->sampled = sefAdvanceKeyframeCursor(keyTable, cursor)->sampleValue;
}

void sefSampleKeyframeValue(SefKeyframe *keys, SefKeyCursor *cursor) {
    cursor->sampleValue = ((SefKeyframe *)sefAdvanceTrackCursor((SefTrackKey *)keys, (SefTrackCursor *)cursor))->sampleValue;
}

/* Builds in destination the rotation that turns axis onto the normalized direction (MMathMatrixBetweenVectors);
 * returns 0 without touching destination for a zero direction. */
int sefSetSlotDirection(Matrix4 *directionMatrix, const Vector4 *direction, const Vector4 *axis)
{
    Vector4 normalized;
    int isZeroDirection;

    isZeroDirection = ((*(int *)&direction->z | *(u64 *)direction) == 0);
    if (isZeroDirection) {
        return 0;
    }
    MMathNormalizeVector2(&normalized, direction);
    MMathMatrixBetweenVectors(directionMatrix, axis, &normalized);
    return 1;
}

int sefSetSlotDirectionBetween(void *slotMatrix, Vector4 *minuend, Vector4 *subtrahend, const Vector4 *axis) {
    Vector4 delta;

    Vu0SefSubVectorXYZ(&delta, minuend, subtrahend);
    return sefSetSlotDirection(slotMatrix, &delta, axis);
}

/* result = the normalized (first - second) when flags has 0x400, else the
 * normalized (second - first); xyz only. The normalizing subroutine works on
 * vf20 in place. */
void sefNormalizeDifference(Vector4 *normalizedOut, const Vector4 *firstPoint, const Vector4 *secondPoint, int directionFlags) {
    __asm__ __volatile__(
        "lqc2 vf20, 0(%0)\n"
        "lqc2 vf1, 0(%1)\n"
        :
        : "r"(firstPoint), "r"(secondPoint)
        : "memory");
    if ((directionFlags & 0x400) != 0) {
        __asm__ __volatile__("vsub.xyz vf20, vf20, vf1" : : : "memory");
    } else {
        __asm__ __volatile__("vsub.xyz vf20, vf1, vf20" : : : "memory");
    }
    vu0NormalizeVf20();
    __asm__ __volatile__("sqc2 vf20, 0(%0)" : : "r"(normalizedOut) : "memory");
}

typedef struct SefAlignedMatrix4 {
    AlignedVector4 r[4];
} SefAlignedMatrix4;

/* Same 16-byte key as SefHermiteKey, read as float points: kept apart, the float fields decide the emitted order. */
typedef struct SefHermiteKeyPoint {
    float x;
    float y;
    float z;
    short key;
    signed char next;
    unsigned char cv;
} SefHermiteKeyPoint;

/* Same object as SefMotionObject, with AlignedVector4 fields: kept apart, the field types decide the emitted order. */
typedef struct SefMotionKeyObject {
    unsigned char unmodeled_00[0x50];
    AlignedVector4 angles;
    unsigned char unmodeled_60[0x10];
    AlignedVector4 origin;
    unsigned char unmodeled_80[0x30];
    AlignedVector4 target;
    unsigned char unmodeled_C0[0x1DC - 0xC0];
    float time;
} SefMotionKeyObject;

/* The floor and key view of sefMotionFloorCurve, the same global as SefMotionCurve in this region. */
typedef struct SefMotionFloorView {
    unsigned char unmodeled_00[12];
    float floorY;
    unsigned char unmodeled_10[8];
    SefHermiteKeyPoint *keys;
    unsigned char unmodeled_1C[4];
} SefMotionFloorView;

extern SefMotionFloorView sefMotionFloorCurve;
extern float vu0CopyMatrixWithTranslation(AlignedVector4*,SefAlignedMatrix4*,AlignedVector4*,AlignedVector4*);
extern void vu0RotateCurrentYXZ(AlignedVector4*);

#define SefHermiteKeyNeighbors ((void(*)(SefHermiteKeyPoint*,SefHermiteKeyPoint**,SefHermiteKeyPoint**,SefHermiteKeyPoint**,SefHermiteKeyPoint**))MMathHermiteKeyNeighbors)
#define SefEaseCurveFn ((float(*)(int,float,float))sefEaseCurve)
#define SefCopyPointZeroW(dest,source) __asm__ __volatile__("lqc2 vf1,0(%5)\n vmove.w vf1,vf0\n sqc2 vf1,0(%4)" : "=m"((dest)->x),"=m"((dest)->y),"=m"((dest)->z),"=m"((dest)->w) : "r"((dest)),"r"(((AlignedVector4*)(source))),"m"(((AlignedVector4*)(source))->x),"m"(((AlignedVector4*)(source))->y),"m"(((AlignedVector4*)(source))->z),"m"(((AlignedVector4*)(source))->w) : "memory")

static inline void SefStoreVf20Vector(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf20,0(%4)" : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w) : "r"(dest));
}

#define SefLoadMatrixVf26To29(matrix) __asm__ __volatile__("lqc2 vf26,0(%0)\n lqc2 vf27,16(%0)\n lqc2 vf28,32(%0)\n lqc2 vf29,48(%0)" : : "r"((matrix)))
#define SefScaleVf28ByLength(length) __asm__ __volatile__("mfc1 $8,%0\n qmtc2 $8,vf1\n vmulx vf28,vf28,vf1x" : : "f"((length)) : "$8","memory")
#define SefAccumulateMatrixColumns(point) __asm__ __volatile__("lqc2 vf1,0(%0)\n vmulax ACC,vf26,vf1x\n vmadday ACC,vf27,vf1y\n vmaddaz ACC,vf28,vf1z\n vmaddw vf20,vf29,vf1w" : : "r"((point)))
#define SefZeroVf0Store(dest) __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"((dest)->x),"=m"((dest)->y),"=m"((dest)->z),"=m"((dest)->w) : "r"((dest)) : "memory")
#define SefTransformByObjectAngles(keyObject,length,point) { SefLoadMatrixVf26To29(&SEF_SCRATCHPAD(SefAlignedMatrix4, 0x10)); vu0RotateCurrentYXZ(&(keyObject)->angles); SefScaleVf28ByLength(length); SefAccumulateMatrixColumns(point); }

static inline void SefClampPointToFloor(AlignedVector4 *point) {
    if (point->y < sefMotionFloorCurve.floorY) {
        point->y = sefMotionFloorCurve.floorY;
    }
}

static inline void SefClampFloatToFloor(float *value) {
    if (*value < sefMotionFloorCurve.floorY) {
        *value = sefMotionFloorCurve.floorY;
    }
}

#define SefCopyPointZeroWMemory(dest,source) __asm__ __volatile__("lqc2 vf1,0(%1)\n vmove.w vf1,vf0\n sqc2 vf1,0(%0)" :  : "r"((dest)),"r"(((AlignedVector4*)(source))) : "memory")

static inline void SefStoreVf20Pointer(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf20,0(%4)" : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w) : "r"(dest));
}

/* Evaluates the key track of keyObject at its time into the point at 0x700001D0: a single key or the end of the
 * track copies that key's point, two keys interpolate, and the Hermite case uses four neighbour keys. transform is applied
 * to the points, and the result is clamped to the floor. */
void sefEvaluateKeyTrackPoint(SefMotionKeyObject *keyObject, SefAlignedMatrix4 *transform) {
    SefHermiteKeyPoint *curveKeys;
    SefHermiteKeyPoint *firstKey;
    AlignedVector4 outPoints[2];
    AlignedVector4 stagePoint;
    SefHermiteKeyPoint *neighborKeys[4];
    AlignedVector4 hermitePoints[4];
    AlignedVector4 tangents[2];
    AlignedVector4 neighborPoint;
    float matrixLength;
    float fraction;
    int j;
    int frame;
    int foundIndex;
    int keyCount;
    int index;
    int useInterp;
    int i;
    curveKeys = sefMotionFloorCurve.keys;
    frame = keyObject->time;
    matrixLength =
        vu0CopyMatrixWithTranslation(&SEF_SCRATCHPAD(AlignedVector4, 0x10), transform, &keyObject->origin, &keyObject->target);
    foundIndex = -1;
    useInterp = 1;
    index = 0;
    keyCount = 0;
    for (; curveKeys[index].key != 0x1000; index++) {
        if (frame < curveKeys[index].key && foundIndex < 0) {
            foundIndex = index - 1;
        }
        keyCount++;
    }
    if (keyCount <= 0) {
        SefZeroVf0Store((&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)));
        useInterp = 0;
    } else if (keyCount == 1) {
        SefCopyPointZeroW((&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)), curveKeys);
        useInterp = 0;
    }
    if (frame < curveKeys[0].key) {
        keyCount = 1;
    }
    if (foundIndex < 0 || foundIndex == keyCount - 1) {
        SefCopyPointZeroW((&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)), &curveKeys[keyCount - 1]);
        useInterp = 0;
    }
    if (useInterp) {
        firstKey = (SefHermiteKeyPoint *) ((foundIndex << 4) + (int) curveKeys);
        fraction =
            SefEaseCurveFn(firstKey->cv >> 4, keyObject->time - firstKey->key, firstKey[1].key - firstKey->key);
        switch (firstKey->cv & 0xf) {
        case 0:
        default: {
            int pointIndex = 0;
            for (i = 0; 1;) {
                SefCopyPointZeroWMemory(&stagePoint, &firstKey[pointIndex]);
                SefLoadMatrixVf26To29(&SEF_SCRATCHPAD(SefAlignedMatrix4, 0x10));
                vu0RotateCurrentYXZ(&keyObject->angles);
                SefScaleVf28ByLength(matrixLength);
                {
                    AlignedVector4 *dest = &outPoints[i];
                    SefAccumulateMatrixColumns(&stagePoint);
                    SefStoreVf20Pointer(dest);
                }
                SefClampFloatToFloor(&outPoints[i].y);
                pointIndex++;
                i++;
                if (!(i < 2)) {
                    break;
                }
            }
        }
            func_0025A000((&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)), &outPoints[0], &outPoints[1], fraction);
            return;
        case 1:
            SefHermiteKeyNeighbors(firstKey, &neighborKeys[0], &neighborKeys[1], &neighborKeys[2], &neighborKeys[3]);
            for (j = 0; j < 4; j++) {
                SefCopyPointZeroW(&neighborPoint, neighborKeys[j]);
                SefTransformByObjectAngles(keyObject, matrixLength, &neighborPoint);
                SefStoreVf20Vector(&hermitePoints[j]);
                {
                    float *yPoint = &hermitePoints[j].y;
                    if (*yPoint < sefMotionFloorCurve.floorY) {
                        *yPoint = sefMotionFloorCurve.floorY;
                    }
                }
            }
            MMathCalcHermitePrm(&tangents[0], &tangents[1], &hermitePoints[0], &hermitePoints[1], &hermitePoints[2], &hermitePoints[3]);
            MMathCalcHermite((float *) (&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)), &tangents[0], &tangents[1], &hermitePoints[1], &hermitePoints[2],
                             fraction);
            return;
        }
    }
    SefTransformByObjectAngles(keyObject, matrixLength, (&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)));
    SefStoreVf20Vector((&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)));
    SefClampPointToFloor((&SEF_SCRATCHPAD(AlignedVector4, 0x1D0)));
}

typedef struct SefMotionCurve {
    unsigned char unmodeled_00[0x08];
    float blendLimit; /* +0x08 */
    float floorY;     /* +0x0C */
    unsigned char unmodeled_10[0x16 - 0x10];
    short hermiteEnabled; /* +0x16 */
    SefHermiteKey *keys;  /* +0x18 */
} SefMotionCurve;

Vector4 *MMathQuatToMatrix(Matrix4 matrix, const void *quaternion);
void vu0SetRotationFromDirection(Matrix4 matrix, const Vector4 *target, const Vector4 *origin);
void xglMatrix2Quaternion(Vector4 *quaternion, Matrix4 matrix);
void *vu0NormalizeXyzwTo(void *normalizedVector, void *inputVector);
float MMathCalcLength(const Vector4 *vector);
void MMathEvalHermiteKeys(Vector4 *destination, SefHermiteKey *keys, float time);
float func_0025A910(const Vector4 *a, const Vector4 *b);
void rotationMatrixToQuaternion(void *output, void *input);
extern s32 sefVectorMode;
extern SefMotionObject *D_005510A4;
extern const Vector4 sefDefaultAxis[2];
extern const Vector4 sefDefaultPoint;

static inline void sefBlendBasis(Vector4 *basis, float blend) {
    __asm__ __volatile__("mfc1 $8, %1\n\tvmr32.xyzw $vf13, $vf0\n\tlqc2 $vf11, 0(%2)\n\tlqc2 $vf12, 0(%2)\n\t"
                         "qmtc2 $8, $vf4\n\tvsub.xyz $vf11, $vf13, $vf11\n\tvmulax.xyz ACC, $vf11, $vf4x\n\t"
                         "vmaddw.xyz $vf11, $vf12, $vf0w\n\tsqc2 $vf11, 0(%0)"
                         :
                         : "r"(&SEF_SCRATCHPAD(Vector4, 0x220)), "f"(blend), "r"(basis)
                         : "$8", "memory");
}

typedef struct SefRequestKeys {
    int firstKey; /* +0x00 */
    unsigned char unmodeled_04[0x70 - 0x04];
    Matrix4 slotMatrix; /* +0x70 */
} SefRequestKeys;

#define SefSubVectorXYZ4(destination, minuend, subtrahend) \
    __asm__ __volatile__("lqc2 $vf1, 0(%4)\n\tlqc2 $vf2, 0(%5)\n\tvsub.xyz $vf1, $vf1, $vf2\n\tsqc2 $vf1, 0(%6)" \
        : "=m"((destination)->x), "=m"((destination)->y), "=m"((destination)->z), "=m"((destination)->w) \
        : "r"(minuend), "r"(subtrahend), "r"(destination) : "memory")
#define SefSetOrigin4(destination) \
    __asm__ __volatile__("sqc2 $vf0, 0(%4)" \
        : "=m"((destination)->x), "=m"((destination)->y), "=m"((destination)->z), "=m"((destination)->w) \
        : "r"(destination) : "memory")
#define SefStoreVf20(destination) \
    __asm__ __volatile__("sqc2 $vf20, 0(%4)" \
        : "=m"((destination)->x), "=m"((destination)->y), "=m"((destination)->z), "=m"((destination)->w) \
        : "r"(destination) : "memory")
#define SefCopyVector4(destination, source) \
    __asm__ __volatile__("lq $8, 0(%5)\n\tsq $8, 0(%4)" \
        : "=m"((destination)->x), "=m"((destination)->y), "=m"((destination)->z), "=m"((destination)->w) \
        : "r"(destination), "r"(source) : "$8", "memory")

/* Computes the step result out for a motion object. The rotation is rebuilt from the quaternion; a Hermite
 * curve (curve->hermiteEnabled) returns 0 after its own step, otherwise the basis is blended and the target difference
 * is normalized by kind, then the step length is applied and the quaternion is rewritten. */
int sefComputeMotionStep(Vector4 *out, SefMotionObject *object, int kind, float blend) {
    Matrix4 rotation;
    Matrix4 *selectedMatrix = 0;
    int basisRefreshed = 0;
    SefMotionCurve *curve;
    SefRequestKeys *request = (SefRequestKeys *) sefRequestKeyBlock;
    Vector4 *basis = &object->basis;
    Vector4 *quaternion;
    short keyClass;
    void *keyTrack;
    Vector4 *resultVector;
    SefRequestKeys *keys;
    float frameStep;

    frameStep = (object->speedB + object->speedA) * *sefTrackFrameStep;
    keyTrack = *(void **) ((unsigned char *) SEF_SCRATCHPAD(void *, 0x0) + 0x664);
    keyClass = keyTrack != 0 ? *(short *) ((unsigned char *) keyTrack - 0x12) : 0;
    keys = (SefRequestKeys *) sefRequestKeyBlock;
    if (keyClass == 0x3000 && sefVectorMode == 0) {
        frameStep *= MMathCalcLength((Vector4 *) keys->slotMatrix);
    }
    if (SEF_SCRATCHPAD(s32, 0x2B0) != 0) {
        frameStep *= SEF_SCRATCHPAD(float, 0x27C);
    }
    quaternion = &object->quaternion;
    MMathQuatToMatrix(rotation, quaternion);
    curve = &sefMotionFloorCurve;
    if (curve->hermiteEnabled != 0) {
        Vector4 *hermitePoint;
        float hermiteLength;

        if (!(object->flags & 2)) {
            object->flags |= 2;
            vu0SetRotationFromDirection(rotation, &object->target, &object->origin);
        }
        if (curve->blendLimit != 0.0f) {
            sefEvaluateKeyTrackPoint(object, rotation);
            hermitePoint = &SEF_SCRATCHPAD(Vector4, 0x1D0);
            SefSubVectorXYZ4(out, hermitePoint, &object->previousTarget);
            SefCopyVector4(&object->delta, out);
            QuadwordCopyT0(&object->previousTarget, hermitePoint);
            rotationMatrixToQuaternion(quaternion, rotation);
            vu0NormalizeXyzwTo(quaternion, quaternion);
        done:
            return 0;
        }
        hermiteLength = vu0CopyMatrixWithTranslation(
            &SEF_SCRATCHPAD(Vector4, 0x10), rotation, &object->origin, &object->target);
        MMathEvalHermiteKeys(&SEF_SCRATCHPAD(Vector4, 0x1D0), curve->keys, object->time);
        __asm__ __volatile__(
            "lqc2 $vf26, 0(%0)\n\tlqc2 $vf27, 16(%0)\n\tlqc2 $vf28, 32(%0)\n\tlqc2 $vf29, 48(%0)"
            :
            : "r"(&SEF_SCRATCHPAD(Vector4, 0x10))
            : "memory");
        vu0RotateCurrentYXZ(&object->angles);
        __asm__ __volatile__("mfc1 $8, %0\n\tqmtc2.ni $8, $vf1\n\tvmulx.xyzw $vf28, $vf28, $vf1x"
                             :
                             : "f"(hermiteLength)
                             : "$8", "memory");
        __asm__ __volatile__(
            "lqc2 $vf1, 0(%0)\n\tvmulax.xyzw ACC, $vf26, $vf1x\n\tvmadday.xyzw ACC, $vf27, $vf1y\n\t"
            "vmaddaz.xyzw ACC, $vf28, $vf1z\n\tvmaddw.xyzw $vf20, $vf29, $vf1w"
            :
            : "r"(&SEF_SCRATCHPAD(Vector4, 0x1D0)));
        hermitePoint = &SEF_SCRATCHPAD(Vector4, 0x1D0);
        __asm__ __volatile__("sqc2 $vf20, 0(%0)" : : "r"(hermitePoint) : "memory");
        SefSubVectorXYZ4(out, hermitePoint, &object->previousTarget);
        SefCopyVector4(&object->delta, out);
        QuadwordCopyT0(&object->previousTarget, hermitePoint);
        xglMatrix2Quaternion(quaternion, rotation);
        vu0NormalizeXyzwTo(quaternion, quaternion);
        goto done;
    }
    if (frameStep == 0.0f) {
        SefSetOrigin4(out);
        return 1;
    }
    if (blend != 0.0f && kind != 0) {
        sefBlendBasis(basis, blend);
    } else {
        QuadwordCopyT0(&SEF_SCRATCHPAD(Vector4, 0x220), basis);
    }
    if (blend != 0.0f || !(object->flags & 2)) {
        object->flags |= 2;
        basisRefreshed = 1;
        if (kind >= 32) {
            if (SEF_SCRATCHPAD(s32, 0x2D4) != 0) {
                sefNormalizeDifference(&SEF_SCRATCHPAD(Vector4, 0x220), &object->target, &object->previousTarget,
                                       SEF_SCRATCHPAD(s32, 0x2D4));
                resultVector = &SEF_SCRATCHPAD(Vector4, 0x220);
                if (blend != 0.0f) {
                    if (func_0025A910(&object->target, &object->previousTarget) < frameStep * frameStep) {
                        SefSubVectorXYZ4(resultVector, &object->target, &object->previousTarget);
                    }
                }
            } else {
                selectedMatrix = &rotation;
                if (blend != 0.0f && func_0025A910(&object->target, &object->previousTarget) < frameStep * frameStep) {
                    selectedMatrix = 0;
                    SefSubVectorXYZ4(&SEF_SCRATCHPAD(Vector4, 0x220), &object->target, &object->previousTarget);
                } else {
                    vu0SetRotationFromDirection(*selectedMatrix, &object->target, &object->previousTarget);
                }
            }
        } else if (kind == 16) {
            SefMotionObject *linked = D_005510A4;

            if (linked != 0) {
                if (SEF_SCRATCHPAD(s32, 0x2D4) != 0) {
                    sefNormalizeDifference(&SEF_SCRATCHPAD(Vector4, 0x220), &linked->delta, &sefDefaultPoint,
                                           SEF_SCRATCHPAD(s32, 0x2D4));
                } else {
                    selectedMatrix = &rotation;
                    if (sefSetSlotDirection(selectedMatrix, &linked->delta, &sefDefaultAxis) != 0) {
                        selectedMatrix = 0;
                        SefSetOrigin4(&SEF_SCRATCHPAD(Vector4, 0x220));
                    }
                }
            }
        } else if (kind == 12) {
            if (request->firstKey != 0) {
                selectedMatrix = &request->slotMatrix;
            }
        }
    } else {
        if (SEF_SCRATCHPAD(s32, 0x2D4) != 0) {
            MMathNormalizeVector2(&SEF_SCRATCHPAD(Vector4, 0x220), &object->delta);
        } else if (kind >= 9) {
            selectedMatrix = &rotation;
        }
    }
    __asm__ __volatile__("lqc2 $vf20, 0(%0)" : : "r"(&SEF_SCRATCHPAD(Vector4, 0x220)) : "memory");
    if (selectedMatrix != 0) {
        __asm__ __volatile__(
            "lqc2 $vf26, 0(%0)\n\tlqc2 $vf27, 16(%0)\n\tlqc2 $vf28, 32(%0)\n\tlqc2 $vf29, 48(%0)\n\t"
            "vmulax.xyzw ACC, $vf26, $vf20x\n\tvmadday.xyzw ACC, $vf27, $vf20y\n\t"
            "vmaddaz.xyzw ACC, $vf28, $vf20z\n\tvmaddw.xyzw $vf20, $vf29, $vf0w"
            :
            : "r"(selectedMatrix)
            : "memory");
    }
    if (frameStep < 0.0f && !basisRefreshed) {
        __asm__ __volatile__("mfc1 $8, %0\n\tqmtc2.ni $8, $vf1\n\tvmulx.xyz $vf20, $vf20, $vf1x"
                             :
                             : "f"(-frameStep)
                             : "$8", "memory");
    } else {
        __asm__ __volatile__("mfc1 $8, %0\n\tqmtc2.ni $8, $vf1\n\tvmulx.xyz $vf20, $vf20, $vf1x"
                             :
                             : "f"(frameStep)
                             : "$8", "memory");
    }
    SefStoreVf20(&object->delta);
    __asm__ __volatile__("sqc2 $vf20, 0(%0)" : : "r"(out) : "memory");
    xglMatrix2Quaternion(quaternion, rotation);
    return 1;
}

Vector4 *MMathApplyMatrix(Vector4 *out, Matrix4 *matrix, const Vector4 *vector);

/* Copies point to savedBefore, transforms point by matrix in place, then copies the result to savedAfter. */
void sefTransformPointSaved(Vector4 *point, Matrix4 *matrix, Vector4 *savedBefore, Vector4 *savedAfter) {
    QuadwordCopyT0(savedBefore, point);
    MMathApplyMatrix(point, matrix, point);
    QuadwordCopyT0(savedAfter, point);
}

/* Transforms point by matrix when it differs from reference on x, y or z; otherwise copies value into point. */
void sefTransformPointIfMoved(Vector4 *point, Vector4 *reference, Matrix4 *matrix, Vector4 *value) {
    if (point->x != reference->x || point->y != reference->y || point->z != reference->z) {
        MMathApplyMatrix(point, matrix, point);
        return;
    }
    QuadwordCopyT0(point, value);
}

typedef struct SefMotionState {
    u8 unmodeled_00[0xA0];
    AlignedVector4 pos;
    AlignedVector4 pos2;
    u8 unmodeled_C0[0x10];
    AlignedVector4 vel;
    u8 unmodeled_E0[0x1D8 - 0xE0];
    short lastFrame;
    short unmodeled_1DA;
    f32 time;
    u8 unmodeled_1E0[0x18];
    s32 flags; /* +0x1F8, bit 0: resting on the floor */
} SefMotionState;

typedef struct SefKeyframeStepParams {
    s32 unmodeled_00;
    f32 unmodeled_04;
    f32 unmodeled_08;
    f32 timeStep;
} SefKeyframeStepParams;

typedef union SefCpuBlockWord {
    s32 i;
    f32 f;
} SefCpuBlockWord;

extern AlignedVector4 D_0076C440;
extern AlignedVector4 D_0076C450;
extern AlignedVector4 D_0076C460;
extern AlignedVector4 D_0076C470;

#define SefKeyframeStepView (*(SefKeyframeStepParams (*))&sefKeyframeStepBlock)

static inline void SefAddVec3(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z)
                         : "r"(dest), "r"(lhs), "r"(rhs));
}

static inline void SefScaleVec3(AlignedVector4 *dest, AlignedVector4 *source, f32 scale) {
    __asm__ __volatile__(
        "lqc2 vf1,0(%4)\n mfc1 $8,%5\n qmtc2.ni $8,vf2\n vmulx.xyz vf1,vf1,vf2x\n sqc2 vf1,0(%3)"
        : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z)
        : "r"(dest), "r"(source), "f"(scale)
        : "$8");
}

static inline void SefZeroVec4Store(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w) : "r"(dest));
}

static inline void SefAddVec3Memory(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 vf1,0(%1)\n lqc2 vf2,0(%2)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%0)"
                         :
                         : "r"(dest), "r"(lhs), "r"(rhs)
                         : "memory");
}

#define SefCpuBlockInt(o) (((SefCpuBlockWord *)(0x70000000+o))->i)
#define SefCpuBlockFloat(o) (((SefCpuBlockWord *)(0x70000000+o))->f)
#define SefMotionTempVector (&SEF_SCRATCHPAD(AlignedVector4, 0x1E0))

/* Advances one motion state by one step: adds the delta from sefComputeMotionStep (kind, blend) to the position,
 * applies the step velocity and the per-frame drop once per whole frame, then clamps to the floor. */
void sefAdvanceMotionState(SefMotionState *state, s32 kind, f32 blend) {
    AlignedVector4 velocityStep;

    state->time += SefKeyframeStepView.timeStep;
    if (SefCpuBlockInt(0x2B0) != 0) {
        sefTransformPointSaved(&state->pos, &SEF_SCRATCHPAD(M4, 0x50), &D_0076C440, &D_0076C450);
    }
    if (SefCpuBlockInt(0x2B4) != 0) {
        sefTransformPointSaved(&state->pos, &SEF_SCRATCHPAD(M4, 0xD0), &D_0076C440, &D_0076C450);
        sefTransformPointSaved(&state->pos2, &SEF_SCRATCHPAD(M4, 0x150), &D_0076C460, &D_0076C470);
    }
    if (!(state->flags & 1)) {
        AlignedVector4 *tempDelta = SefMotionTempVector;
        if (sefComputeMotionStep((Vector4 *) tempDelta, (SefMotionObject *) state, kind, blend) == 0) {
            if (SefCpuBlockInt(0x2B0) != 0) {
                MMathApplyMatrix(
                    &state->pos, &SEF_SCRATCHPAD(M4, 0x90), &state->pos);
            }
            if (SefCpuBlockInt(0x2B4) != 0) {
                MMathApplyMatrix(
                    &state->pos, &SEF_SCRATCHPAD(M4, 0x110), &state->pos);
                MMathApplyMatrix(
                    &state->pos2, &SEF_SCRATCHPAD(M4, 0x190), &state->pos2);
            }
            return;
        }
        if (SefCpuBlockFloat(0x274) >= 0.0f || state->pos.y != SefCpuBlockFloat(0x278)) {
            SefAddVec3(&state->pos, &state->pos, tempDelta);
        }
    } else {
        SefZeroVec4Store(SefMotionTempVector);
    }
    SefScaleVec3(&velocityStep, &state->vel, SefKeyframeStepView.timeStep);
    if (SefCpuBlockInt(0x280) != 0) {
        state->pos.y -= SefCpuBlockFloat(0x270);
    } else {
        s32 frame = (s32) state->time;
        if (frame != state->lastFrame) {
            state->vel.y -= SefCpuBlockFloat(0x270);
            state->lastFrame = frame;
        }
    }
    SefAddVec3(&state->pos, &state->pos, &velocityStep);
    if (SefCpuBlockInt(0x2B0) != 0) {
        sefTransformPointIfMoved(&state->pos, &D_0076C450, &SEF_SCRATCHPAD(M4, 0x90), &D_0076C440);
    }
    if (SefCpuBlockInt(0x2B4) != 0) {
        sefTransformPointIfMoved(&state->pos, &D_0076C450, &SEF_SCRATCHPAD(M4, 0x110), &D_0076C440);
        sefTransformPointIfMoved(&state->pos2, &D_0076C470, &SEF_SCRATCHPAD(M4, 0x190), &D_0076C460);
    }
    if (SefCpuBlockFloat(0x274) > 0.0f) {
        if (state->pos.y <= SefCpuBlockFloat(0x278)) {
            if (!(state->flags & 1)) {
                SefAddVec3Memory(&state->vel, SefMotionTempVector, &state->vel);
            }
            state->flags |= 1;
            state->pos.y = SefCpuBlockFloat(0x278);
            SefScaleVec3(&state->vel, &state->vel, SefCpuBlockFloat(0x274));
            if (state->vel.y < 0.0f) {
                state->vel.y = -state->vel.y;
            }
        }
    } else if (SefCpuBlockFloat(0x274) < 0.0f) {
        if (state->pos.y <= SefCpuBlockFloat(0x278)) {
            state->pos.y = SefCpuBlockFloat(0x278);
        }
    }
}

typedef struct SefMatrixEntry {
    int key;           /* +0x00 */
    int matrixIndex;   /* +0x04 */
    float position[3]; /* +0x08 */
    float rotation[3]; /* +0x14: x, y, z */
} SefMatrixEntry;

typedef struct SefMatrixTable {
    unsigned char unmodeled_00[0x8];
    unsigned int count; /* +0x08 */
    unsigned char unmodeled_0C[0x10 - 0x0C];
    SefMatrixEntry entries[1]; /* +0x10, the table continues past this first entry */
} SefMatrixTable;

typedef struct SefRotateBlock {
    Matrix4 matrix; /* +0x00 */
    Vector4 source; /* +0x40 */
    unsigned char unmodeled_50[0x10];
    Vector4 normalized; /* +0x60 */
    unsigned char unmodeled_70[0xD8 - 0x70];
    struct SefMatrixTable *table; /* +0xD8 */
    unsigned char unmodeled_DC[0xF0 - 0xDC];
    Matrix4 *matrices; /* +0xF0 */
    unsigned char unmodeled_F4[0x198 - 0xF4];
    float base; /* +0x198 */
    unsigned char unmodeled_19C[0x1A0 - 0x19C];
    Vector4 angles; /* +0x1A0 */
    unsigned char unmodeled_1B0[0x1F8 - 0x1B0];
    int flags; /* +0x1F8 */
} SefRotateBlock;

Matrix4 *MMathScaleMatrixRotS(Matrix4 destination, const Matrix4 source, float scale);
void xglCurMatrixSet(const Matrix4 matrix);
void xglCurMatrixGet(Matrix4 matrix);
void xglCurMulTranslate(const float position[3]);
void xglCurMulRotateX(float angle);
void xglCurMulRotateY(float angle);
void xglCurMulRotateZ(float angle);
void tiVu0LoadIdentityMatrix(void);
/* Loads table entry matrixKey of block into the current matrix (scaled to unit length when scaleFlag is set),
 * applies the entry's translation and Z, Y, X rotations, and stores the result in out. Returns 0 when the key is absent. */
int func_001FB6A0(SefRotateBlock *block, int matrixKey, Matrix4 out, int scaleFlag) {
    SefMatrixEntry *entry;
    Matrix4 scaledMatrix;
    float matrixLength;
    int i;
    SefMatrixEntry *entries;

    if (block->table == 0) {
        return 0;
    }
    entries = block->table->entries;
    entry = 0;
    for (i = 0; i < block->table->count; i++) {
        if (entries[i].key == matrixKey) {
            entry = &entries[i];
            break;
        }
    }
    if (entry == 0) {
        return 0;
    }
    if (scaleFlag != 0) {
        matrixLength = MMathCalcLength(block->matrices[entry->matrixIndex]);
        if (0.00001f <= matrixLength && matrixLength < 1.0f) {
            MMathScaleMatrixRotS(scaledMatrix, block->matrices[entry->matrixIndex], 1.0f / matrixLength);
            xglCurMatrixSet(scaledMatrix);
        } else {
            xglCurMatrixSet(block->matrices[entry->matrixIndex]);
        }
    } else {
        tiVu0LoadIdentityMatrix();
    }
    if (entry->position[0] != 0.0f || entry->position[1] != 0.0f || entry->position[2] != 0.0f) {
        xglCurMulTranslate(entry->position);
    }
    if (entry->rotation[2] != 0.0f) {
        xglCurMulRotateZ(entry->rotation[2]);
    }
    if (entry->rotation[1] != 0.0f) {
        xglCurMulRotateY(entry->rotation[1]);
    }
    if (entry->rotation[0] != 0.0f) {
        xglCurMulRotateX(entry->rotation[0]);
    }
    xglCurMatrixGet(out);
    return 1;
}

typedef struct SefDirectionState {
    M4 matrix;
    unsigned char unmodeled_40[0x20];
    AlignedVector4 quaternion;
    AlignedVector4 q70;
    AlignedVector4 v80;
    AlignedVector4 direction;
    AlignedVector4 va0;
    AlignedVector4 vb0;
    AlignedVector4 vc0;
    AlignedVector4 vd0;
    AlignedVector4 ve0;
    unsigned char unmodeled_F0[0x1F8 - 0xF0];
    int flags; /* +0x1F8 */
} SefDirectionState;

typedef struct SefRequestKeyParams {
    unsigned char *w0;
    unsigned char *w4;
    unsigned char *w8;
    unsigned char *wc;
    unsigned char *w10;
    unsigned char unmodeled_14[0x5C];
    M4 m70;
    M4 mb0;
    unsigned char unmodeled_F0[0x100];
    M4 m1f0;
} SefRequestKeyParams;

typedef struct SefCurrentRecord {
    unsigned char unmodeled_00[0x694];
    int flags; /* +0x694 */
} SefCurrentRecord;

typedef struct SefLinkedBlock {
    unsigned char unmodeled_00[0xA0];
    AlignedVector4 va0;
    unsigned char unmodeled_B0[0x30];
    AlignedVector4 ve0;
} SefLinkedBlock;

extern void quaternionFromToRotation(AlignedVector4*,AlignedVector4*,AlignedVector4*);
int sefDirectionToAngles(Vector4 *destination, const Vector4 *from, const Vector4 *to);
extern void vu0CrossWithWeights(AlignedVector4*,AlignedVector4*,AlignedVector4*);

#define SefRequestKeyBlockView (*(SefRequestKeyParams (*))&sefRequestKeyBlock)
#define M_MATH_NORMALIZE_VECTOR2 ((void(*)(AlignedVector4*,AlignedVector4*))MMathNormalizeVector2)

static inline void SefCopyVec4(AlignedVector4 *dest, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w)
                         : "r"(dest), "r"(source)
                         : "$8");
}

static inline void SefAddVec3Pair(AlignedVector4 *dest, AlignedVector4 *lhs, AlignedVector4 *rhs) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z)
                         : "r"(dest), "r"(lhs), "r"(rhs));
}

static inline void SefZeroMatrixTranslation(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(dest->x), "=m"(dest->y), "=m"(dest->z), "=m"(dest->w) : "r"(dest));
}

static inline void SefZeroVf0StoreMemory(AlignedVector4 *dest) {
    __asm__ __volatile__("sqc2 vf0,0(%0)" : : "r"(dest) : "memory");
}

static inline int SefIsZeroVector(AlignedVector4 *vector) {
    return (*(int *) &vector->z | *(long *) vector) == 0;
}

static inline void SefSetAxisFromDirection(AlignedVector4 *out, AlignedVector4 *direction) {
    AlignedVector4 normalized;
    M_MATH_NORMALIZE_VECTOR2(&normalized, direction);
    quaternionFromToRotation(out, &sefDefaultAxis, &normalized);
}

#define SefCopyMatrixVf26To28(dest,source) do { M4 *sourceMatrix=(source); M4 *destMatrix=(dest); __asm__ __volatile__("lqc2 vf26,0(%0)\n lqc2 vf27,16(%0)\n lqc2 vf28,32(%0)" : : "r"(sourceMatrix) ); __asm__ __volatile__("sqc2 vf26,0(%0)\n sqc2 vf27,16(%0)\n sqc2 vf28,32(%0)\n sqc2 vf0,48(%0)" : : "r"(destMatrix) ); } while(0)
#define SefDirectionTempVector (&SEF_SCRATCHPAD(AlignedVector4, 0x1E0))

extern void rotationMatrixNormalizedToQuaternion(AlignedVector4*,M4*);
extern void vu0QuatToMatrix(M4*,AlignedVector4*);
void sefGetSourceMatrix(void *matrix, const void *source);
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);

/* Recomputes state->direction and state->matrix for kind: the direction comes from the key block, the linked
 * object or the quaternion, selector picks the source, and the matrix is rebuilt from the quaternion. */
void func_001FB8A0(SefDirectionState *state, int selector, int kind, int flags) {
    AlignedVector4 direction;
    AlignedVector4 quaternion;
    M4 deltaMatrix;
    int first = 0;
    int useDeltaMatrix;

    if (kind == 0x4000 || kind == 0x2000) {
        first = 1;
    }
    useDeltaMatrix = 0;
    SefCopyVec4(&direction, &state->direction);
    SefCopyVec4(&quaternion, &state->quaternion);
    if (first || (flags & 2) || !(state->flags & 4)) {
        SefRequestKeyParams *keyBlock = &SefRequestKeyBlockView;
        state->flags |= 4;
        SefZeroVf0StoreMemory(&direction);
        if (kind == 0x2000) {
            SefCopyVec4(SefDirectionTempVector, &state->vd0);
            if (!(state->flags & 1)) {
                SefAddVec3Pair(SefDirectionTempVector, SefDirectionTempVector, &state->ve0);
            }
            if (!SefIsZeroVector(SefDirectionTempVector)) {
                SefSetAxisFromDirection(&direction, SefDirectionTempVector);
            }
        } else if (kind == 0x3000) {
            if ((SEF_SCRATCHPAD(SefCurrentRecord *, 0x0))->flags & 0x40000) {
                ((void (*)(M4 *)) sefGetSourceMatrix)(&deltaMatrix);
                rotationMatrixNormalizedToQuaternion(&direction, &deltaMatrix);
            } else if (sefVectorMode == 0) {
                state->flags |= 0x8000;
                ((void (*)(M4 *)) sefGetSourceMatrix)(&deltaMatrix);
                SefZeroMatrixTranslation(&deltaMatrix.r[3]);
                useDeltaMatrix = 1;
            }
        } else if (kind == 0x3001) {
            unsigned char *keyBase = 0;
            if (selector & 0x100) {
                keyBase = keyBlock->w0;
                selector -= 0x101;
            } else if (selector & 0x200) {
                keyBase = keyBlock->w4;
                selector -= 0x201;
            } else if (selector & 0x400) {
                keyBase = keyBlock->w8;
                selector -= 0x401;
            } else if (selector & 0x800) {
                keyBase = keyBlock->wc;
                selector -= 0x801;
            } else if (selector & 0x1000) {
                keyBase = keyBlock->w10;
                selector -= 0x1001;
            }
            if (func_001FB6A0(keyBase + 0x560, selector, &deltaMatrix, 1) != 0) {
                MMathMulMatrix(&deltaMatrix, (M4 *) (keyBase + 0xB0), &deltaMatrix);
                state->flags |= 0x8000;
                SefZeroMatrixTranslation(&deltaMatrix.r[3]);
                useDeltaMatrix = 1;
            }
        } else if (selector == 0x10) {
            if (((SefLinkedBlock *) D_005510A4) != 0) {
                if (flags & 1) {
                    SefCopyVec4(&direction, &state->v80);
                } else if (!SefIsZeroVector(&((SefLinkedBlock *) D_005510A4)->ve0)) {
                    SefSetAxisFromDirection(&direction, &((SefLinkedBlock *) D_005510A4)->ve0);
                }
            }
        } else if (selector == 0x100 && kind == selector) {
            SefCopyMatrixVf26To28(&deltaMatrix, &keyBlock->m70);
            useDeltaMatrix = 1;
        } else if (selector == 0x200 && kind == selector) {
            rotationMatrixToQuaternion(&direction, &keyBlock->mb0);
        } else if (selector == 0x241 && kind == selector) {
            rotationMatrixToQuaternion(&direction, &keyBlock->m1f0);
        } else if (selector >= 0x20) {
            AlignedVector4 *anchor = &state->va0;
            if (!first) {
                anchor = &SEF_SCRATCHPAD(AlignedVector4, 0x230);
            }
            if (kind == 0x10) {
                if (((SefLinkedBlock *) D_005510A4) != 0) {
                    anchor = &((SefLinkedBlock *) D_005510A4)->va0;
                }
            }
            if (selector == 0x21) {
                sefDirectionToAngles(&direction, &state->vb0, anchor);
            } else {
                sefDirectionToAngles(
                    &direction, &SEF_SCRATCHPAD(AlignedVector4, 0x240), anchor);
            }
        } else {
            vu0QuatToMatrix(&state->matrix, &quaternion);
            return;
        }
    } else if (selector < 0xD) {
        vu0QuatToMatrix(&state->matrix, &quaternion);
        return;
    }
    {
        M4 quatMatrix;
        AlignedVector4 crossed;
        if (useDeltaMatrix) {
            vu0QuatToMatrix(&quatMatrix, &state->quaternion);
            MMathMulMatrix(&state->matrix, &deltaMatrix, &quatMatrix);
            rotationMatrixToQuaternion(&direction, &deltaMatrix);
            vu0NormalizeXyzwTo(&state->direction, &direction);
        } else {
            vu0NormalizeXyzwTo(&state->direction, &direction);
            vu0CrossWithWeights(&crossed, &state->direction, &quaternion);
            vu0QuatToMatrix(&state->matrix, &crossed);
        }
    }
}

extern Vector4 *D_00454170[];
void vu0RotateVectorAboutY(Vector4 *destination, Vector4 *source, float scale);
void vu0RotateVectorAboutX(Vector4 *destination, Vector4 *source, float scale);
void vu0RotateVectorAboutZ(Vector4 *destination, Vector4 *source, float scale);
void quaternionFromAngleY(Vector4 *destination, float scale);
void vu0SetCurrentRotationZ(float angle);

/* Rotates the block's source vector by its Y, X and Z angles and stores the normalized result. With
   computeNormalized == 0 it sets the current VU0 rotation from base + angles.z and loads the
   rotation matrix into vf26..vf29. */
void sefRotateBlockUpdate(SefRotateBlock *rotateBlock, int computeNormalized) {
    Vector4 rotated;
    Vector4 *angles = &rotateBlock->angles;

    if (computeNormalized != 0) {
        if ((rotateBlock->flags & 0x8) != 0) {
            QuadwordCopyT0(&rotated, &rotateBlock->source);
            if (D_00454170[0] != 0) {
                vu0CrossWithWeights(&rotated, &rotated, D_00454170[0]);
            }
            vu0RotateVectorAboutY(&rotated, &rotated, angles->y);
            vu0RotateVectorAboutX(&rotated, &rotated, angles->x);
            vu0RotateVectorAboutZ(&rotated, &rotated, angles->z);
        } else {
            if (D_00454170[0] != 0) {
                vu0RotateVectorAboutY(&rotated, D_00454170[0], angles->y);
                vu0RotateVectorAboutX(&rotated, &rotated, angles->x);
                vu0RotateVectorAboutZ(&rotated, &rotated, angles->z);
            } else {
                quaternionFromAngleY(&rotated, angles->y);
                vu0RotateVectorAboutX(&rotated, &rotated, angles->x);
                vu0RotateVectorAboutZ(&rotated, &rotated, angles->z);
            }
        }
        vu0NormalizeXyzwTo(&rotateBlock->normalized, &rotated);
    } else {
        vu0SetCurrentRotationZ(rotateBlock->base + angles->z);
        __asm__ __volatile__("sqc2 vf26, 0(%0)\n"
                             "sqc2 vf27, 16(%0)\n"
                             "sqc2 vf28, 32(%0)\n"
                             "sqc2 vf29, 48(%0)\n"
                             :
                             : "r"(rotateBlock)
                             : "memory");
    }
}

extern int D_00454178[];
u32 MMathRandomNext(void);

typedef struct SkSefCurveKey {
    s16 key; /* +0x00, 0x1000 ends the curve */
    s8 link; /* +0x02 */
    u8 unmodeled_03;
    s16 unmodeled_04[4];
    s16 group; /* +0x0C */
} SkSefCurveKey;

typedef struct SkSefCurveSegment {
    u8 unmodeled_00[24];
    f32 value; /* +0x18 */
    u8 unmodeled_1C[4];
} SkSefCurveSegment;

typedef struct SkSefCurveCursor {
    s32 index;
    s32 p;
    SkSefCurveSegment *segment_a;
    SkSefCurveSegment *segment_b;
} SkSefCurveCursor;

typedef struct SkSefCurveTrack {
    u8 unmodeled_00[0x150];
    SkSefCurveCursor cursor; /* +0x150 */
    u8 unmodeled_170[0x58];
    union {
        f32 value;
        s32 bits;
    } time;
} SkSefCurveTrack;

typedef struct SkSefCurveState {
    u8 unmodeled_00[0x17C];
    SkSefCurveKey *keys; /* +0x17C */
    u8 unmodeled_180[0x20];
    SkSefCurveSegment segments_a[30];
    SkSefCurveSegment segments_b[30];
} SkSefCurveState;

/* Advances one curve track by delta. A negative advance restarts it at a random keyIndex of the first group. Returns
   1 or 0x1000 when the track entered a new key segment, otherwise 0. */
s32 sefCurveTrackAdvance(SkSefCurveState *curveState, SkSefCurveTrack *track, s32 advance, f32 delta) {
    SkSefCurveCursor *cursor = &track->cursor;
    f32 time = track->time.value;
    s32 frame = (s32) time;
    s32 curveFlags = D_00454178[0];
    SkSefCurveKey *keys = curveState->keys;
    s32 keyIndex = cursor->index;
    SkSefCurveKey *currentKey = keys + keyIndex;
    s32 result = 0;
    SkSefCurveSegment *segmentsA = curveState->segments_a;
    SkSefCurveSegment *segmentsB = curveState->segments_b;
    u16 scanKeyTime;
    s16 currentKeyTime;
    s32 link;
    s32 nextKey;
    SkSefCurveSegment *segmentA;
    SkSefCurveSegment *segmentB;
    if (advance < 0) {
        s32 groupLength = 1;
        s32 i = 0;
        u32 randomOffset;
        currentKey = keys;
        if (currentKey[i].group != -1) {
            while (currentKey[i].group == currentKey[i + 1].group) {
                if (currentKey[i + 1].key == 0x1000) {
                    break;
                }
                groupLength++;
                i++;
            }
        }
        randomOffset = MMathRandomNext() % groupLength;
        cursor->index = i;
        segmentA = &segmentsA[randomOffset];
        segmentB = &segmentsB[randomOffset];
        cursor->segment_b = segmentB;
        cursor->segment_a = segmentA;
        track->time.value = 0.0f;
        return 0;
    }
    currentKeyTime = currentKey->key;
    scanKeyTime = currentKey->key;
    if (currentKeyTime == 0x1000) {
        return 0;
    }
    if ((curveFlags & 8) != 0) {
        for (;; keyIndex++, currentKey = keys + keyIndex, scanKeyTime = currentKey->key) {
            s32 scanLink = currentKey->link;
            if ((s16) scanKeyTime == 0x1000) {
                return 0;
            }
            if (scanLink >= 0) {
                if (currentKey[1].key == 0x1000) {
                    continue;
                }
                return 0;
            }
            nextKey = keyIndex + 1;
            break;
        }
        result = 1;
        cursor->index = nextKey;
        cursor->segment_a = &segmentsA[nextKey];
        cursor->segment_b = &segmentsB[nextKey];
    } else if (cursor->segment_a->value <= (f32) frame) {
        link = currentKey->link;
        if (link >= 0) {
            s32 groupLength = 1;
            s32 j = link;
            u32 randomOffset;
            if (keys[j].group != -1) {
                while (keys[j].group == keys[j + 1].group) {
                    if (keys[j + 1].key == 0x1000) {
                        break;
                    }
                    groupLength++;
                    j++;
                }
            }
            randomOffset = MMathRandomNext() % groupLength;
            cursor->index = j;
            result = 0x1000;
            cursor->segment_a = &segmentsA[link + randomOffset];
            cursor->segment_b = &segmentsB[link + randomOffset];
            track->time.value = segmentsA[link + randomOffset].value - segmentsA[0].value;
            track->time.value = track->time.value + delta;
            return 0x1000;
        } else if (link == -1) {
            if (currentKey[1].key != 0x1000) {
                s32 groupLength = 1;
                s32 j = keyIndex + 1;
                s32 oldIndex;
                u32 randomOffset;
                if (keys[j].group != -1) {
                    while (keys[j].group == keys[j + 1].group) {
                        if (keys[j + 1].key == 0x1000) {
                            break;
                        }
                        groupLength++;
                        j++;
                    }
                }
                randomOffset = MMathRandomNext() % groupLength;
                oldIndex = cursor->index;
                cursor->index = j;
                segmentA = &segmentsA[oldIndex + randomOffset + 1];
                segmentB = &segmentsB[oldIndex + randomOffset + 1];
                cursor->segment_b = segmentB;
                cursor->segment_a = segmentA;
                track->time.value = track->time.value + delta;
                return 1;
            }
        } else if (link != -2) {
            if (currentKey[1].key == 0x1000) {
                return 0;
            }
        }
    }
    track->time.value = track->time.value + delta;
    return result;
}

typedef struct SwKs {
    s16 *k0;
    s16 *k4;
    s16 *k8;
    s16 *kc;
    s16 *k10;
    s16 *k14;
    s16 *k18;
} SwKs;

typedef struct SwLd {
    u8 a[0x90];
    AlignedVector4 drift;
    u8 a1[0x28];
    union {
        u16 b;
        s16 v;
    } Swdirm;
    u8 a2[0x1E];
    union {
        s32 b;
    } SwkE8;
    u8 a3[0x74];
    s16 *keys[8];
    u64 img[2];
    u8 b[0x2C];
    u16 imf;
    s16 imk;
    u8 c[0x4A0];
    SwKs ks;
    u8 d[0x14];
    s32 fl0;
    s32 fl1;
    u8 e[0x206];
    u16 use;
} SwLd;

typedef struct SwMo {
    void *k0;
    f32 k4;
    f32 k8;
    f32 kc;
} SwMo;

typedef struct SwKb {
    u8 a[0x70];
    AlignedVector4 v;
} SwKb;

typedef struct SwRc {
    u8 a[8];
    s16 k8;
} SwRc;

short func_00206A50(SefRunEntry *entries);
void func_00206AA8(Matrix4 *destination, Vector4 *pointAOut, Vector4 *pointBOut, int kindA, int kindB);
void MMathInvertAffineMatrix(AlignedVector4*,AlignedVector4*);

#define SwS(o) (*(s32*)(sc+(o)))
#define SwSF(o) (*(f32*)(sc+(o)))
#define SwSK(o) (*(s16**)(sc+(o)))
#define SwSP(o) (*(void**)(sc+(o)))
#define SwSV(o) ((AlignedVector4*)(sc+(o)))

static inline void SwCopyM(AlignedVector4 *destination, const AlignedVector4 *source) {
    __asm__ __volatile__(
        "lq $8,0(%1)\n\tlq $9,16(%1)\n\tlq $10,32(%1)\n\tlq $11,48(%1)\n\tsq $8,0(%0)\n\tsq $9,16(%0)\n\tsq $10,32(%0)\n\tsq $11,48(%0)"
        :
        : "r"(destination), "r"(source)
        : "$8", "$9", "$10", "$11", "memory");
}

extern SefEffectEntryRecord *sefCurrentEffectRecord;
Matrix4 *func_002046F8(Vector4 *dest, const Vector4 *source, int kind);

/* Fills the scratchpad parameter block (offsets 0x230..0x2D8) from the effect record and the key
   tables of one load, then builds the matrices the effect needs. */
void sefSwLoadParameterBlock(SwLd *swLoad) {
    u8 *sc = &SEF_SCRATCHPAD(u8, 0x0);
    SwKs *keyPointers = &swLoad->ks;
    u64 *imageWords = swLoad->img;
    s16 *angleKeys;
    s32 dirmBits;
    s32 flags1;
    AlignedVector4 pointA;
    AlignedVector4 pointB;
    AlignedVector4 quat;
    s16 **keyTable;
    SwSK(0x250) = keyPointers->k0;
    SwS(0x284) = ((SwRc *) ((void *) sefCurrentEffectRecord))->k8;
    angleKeys = keyPointers->k4;
    SwSP(0x26C) = &swLoad->drift;
    SwSK(0x254) = angleKeys;
    SwSP(0) = swLoad;
    SwSP(0x268) = imageWords;
    SwSK(0x258) = keyPointers->k18;
    SwSK(0x25C) = keyPointers->kc;
    SwSK(0x260) = keyPointers->k8;
    SwSK(0x264) = swLoad->keys[4];
    SwS(0x288) = angleKeys != 0 ? angleKeys[-2] : 0;
    SwS(0x28C) = SwSK(0x254) != 0 ? SwSK(0x254)[-9] : 0;
    SwS(0x290) = SwSK(0x254) != 0 ? SwSK(0x254)[-1] : 0;
    dirmBits = swLoad->Swdirm.b;
    SwS(0x29C) = (u16) ((s16 *) swLoad->keys[0])[-2] - 16U < 2;
    SwS(0x298) = dirmBits == SwS(0x288);
    SwS(0x294) = dirmBits;
    flags1 = swLoad->fl1;
    SwS(0x2A0) = flags1 & 1;
    SwS(0x2A4) = (flags1 & 0x1000) && keyPointers->k14 != 0;
    SwS(0x2A8) = swLoad->imk != 4 || SwS(0x290) != 0;
    SwS(0x2AC) = (imageWords[1] | imageWords[0]) != 0;
    SwS(0x2B0) = (*(u64 *) &swLoad->fl0 & 0x800100000000UL) == 0x800100000000UL;
    SwS(0x2B4) = flags1 & 0x20000;
    SwS(0x2B8) = swLoad->SwkE8.b & 1;
    SwS(0x2BC) = swLoad->SwkE8.b & 2;
    SwS(0x2C0) = swLoad->SwkE8.b & 0x10;
    SwS(0x2C4) = swLoad->SwkE8.b & 8;
    SwS(0x2C8) = swLoad->SwkE8.b & 0x20;
    {
        u32 k0Value = (u32) (*(SwMo *) &sefMotionFloorCurve).k0;
        SwS(0x280) = k0Value;
    }
    SwSF(0x270) = (*(SwMo *) &sefMotionFloorCurve).k4;
    SwSF(0x274) = (*(SwMo *) &sefMotionFloorCurve).k8;
    SwSF(0x278) = (*(SwMo *) &sefMotionFloorCurve).kc;
    SwS(0x2D8) = swLoad->fl1 & 8;
    SwS(0x2CC) = swLoad->imf & 0x10;
    SwS(0x2D0) = -1;
    SwS(0x2DC) = swLoad->use;
    if (keyPointers->k14 != 0) {
        SwS(0x2D0) = keyPointers->k14[-1];
    }
    SwS(0x2D4) = swLoad->fl1 & 0xC00;
    if (SwS(0x28C) >= 32) {
        func_002046F8(SwSV(0x230), 0, SwS(0x28C));
    }
    if (SwS(0x288) >= 32) {
        func_002046F8(SwSV(0x240), 0, SwS(0x288));
    }
    keyTable = (s16 **) swLoad->keys;
    if (SwS(0x2B0)) {
        {
            SwKb *keyBlock = (SwKb *) sefRequestKeyBlock;
            f32 length = MMathCalcLength(&keyBlock->v);
            SwSF(0x27C) = length;
            SwSF(0x270) *= length;
        }
        {
            s32 v = func_00206A50(keyTable[0]);
            func_00206AA8(SwSV(0x50), &pointA, &pointB, keyTable[0][-2], v);
        }
        MMathInvertAffineMatrix(SwSV(0x90), SwSV(0x50));
    }
    if (SwS(0x2B4)) {
        sefGetSourceMatrix(SwSV(0xD0), (const void *) (s32) SwS(0x288));
        rotationMatrixNormalizedToQuaternion(&quat, SwSV(0xD0));
        vu0QuatToMatrix(SwSV(0xD0), &quat);
        SwCopyM(SwSV(0x150), SwSV(0xD0));
        func_002046F8(SwSV(0x100), 0, keyTable[0][-2]);
        func_002046F8(SwSV(0x180), 0, keyTable[2][-2]);
        if (SwSF(0x10C) == 0.0f) {
            SwSF(0x10C) = 1.0f;
        }
        if (SwSF(0x18C) == 0.0f) {
            SwSF(0x18C) = 1.0f;
        }
        MMathInvertAffineMatrix(SwSV(0x110), SwSV(0xD0));
        MMathInvertAffineMatrix(SwSV(0x190), SwSV(0x150));
    }
}

typedef unsigned int u128 __attribute__((mode(TI)));

typedef union SxCol {
    s32 lane[4];
    u128 q;
} SxCol;

typedef struct SxRes {
    u8 a[8];
    s32 k8;
    u8 b[0x12];
    s16 k1e;
} SxRes;

typedef struct SxPt {
    u8 a[0xA0];
    AlignedVector4 pos;
    AlignedVector4 vB0;
    AlignedVector4 scale;
    AlignedVector4 vD0;
    AlignedVector4 vE0;
    AlignedVector4 offset;
    AlignedVector4 baseScale;
    AlignedVector4 colorScale;
    AlignedVector4 v120;
    SxCol color;
    s16 start;
    s16 p142;
    u8 cur[4];
    u8 c[0x10];
    struct SxRes *res;
    u8 d[4];
    AlignedVector4 drift;
    u8 t170[0xC];
    f32 speed;
    AlignedVector4 scaleValue;
    u8 t190[0x20];
    u8 t1B0[8];
    f32 age;
    f32 life;
    AlignedVector4 dir;
    u8 t1D0[8];
    s16 frame;
    s16 link;
    f32 time;
    SxCol colorValue;
    u8 t1F0[8];
    s32 flags;
    u8 e[0x14];
    u8 t210[8];
    f32 spin;
    u8 t21C[0xC];
    u8 t228[2];
    s16 alpha;
    u8 f[0x34];
} SxPt;

typedef struct SxEnt {
    u8 hdr[0x10];
    SxPt d;
} SxEnt;

typedef struct SxKeys {
    u8 a[0x10];
    s32 spin;
    s32 part;
} SxKeys;

typedef struct SxLd {
    u8 a[0x190];
    void *ev;
    u8 b[0x4CC];
    SxKeys keys;
    u8 c[0x14];
    s32 time;
    u8 d[0xC];
    s16 h[256];
} SxLd;

typedef struct SxRc {
    u8 a[0x10];
    AlignedVector4 p;
    AlignedVector4 q;
    AlignedVector4 s;
    u8 b[0x30];
    s32 k70;
    u8 c[0xCA];
    s16 k13e;
} SxRc;

typedef struct SxBlk {
    f32 a[2];
    f32 scale;
    f32 step;
} SxBlk;

typedef union {
    s32 i;
    f32 f;
} SxW;

extern s32 sefActiveLineIndex;
extern s32 D_005510A0;
extern SxPt *D_005510A4_1FC620;
void func_00207490(AlignedVector4 *p, s32 *c, f32 rad);

typedef struct SqEv {
    s16 key;
    s16 life;
    s16 mode;
    s16 blend;
    s16 rgba[4];
    s16 parts[2];
    s16 maxS;
    s16 shrink;
    s16 spread;
    s16 ring;
    s16 img[2][2];
    f32 f24;
    f32 f28;
    s16 k2c;
    s16 k2e;
    s16 k30;
    s16 k32;
    s16 k34;
    s16 k36;
    f32 f38;
    s16 k3c[2];
} SqEv;

void sefQueueEffectEvents(s16 t, u8 *cur, SqEv *ev, void *sef, s32 x, s32 owner);

#define SEF_KEYFRAME_STEP_BLOCKView (*(SxBlk (*))&sefKeyframeStepBlock)
#define SEF_PT_ALLOC_COUNT (*(s32 (*))&sefPtAllocCount)
#define SEF_PT_ALLOCATOR_STATE ((SxEnt *)sefPtAllocatorState)
#define SEF_SAMPLE_COLOR_TRACK ((void(*)(s32,void*,AlignedVector4*))sefSampleColorTrack)
#define SEF_SAMPLE_TRACK_TRUNCATED ((void(*)(s32,void*))sefSampleTrackTruncated)
#define SEF_SAMPLE_TRACK ((void(*)(s32,void*))sefSampleTrack)
#define SEF_ADD_VECTOR_FRACTION ((void(*)(AlignedVector4*,s32))sefAddVectorFraction)
#define SxS(o) (((SxW*)(sc+(o)))->i)
#define SxSV(o) ((AlignedVector4*)(sc+(o)))

static inline SxPt *Sxgp(s32 handle) {
    if (handle < 0 || handle >= SEF_PT_ALLOC_COUNT) {
        return 0;
    }
    return &SEF_PT_ALLOCATOR_STATE[handle].d;
}

static inline SxPt *Sxgl(s32 list) {
    if (sefActiveLineIndex >= 0 && list >= 0) {
        return Sxgp(list);
    }
    return 0;
}

static inline void SxCopyQ(AlignedVector4 *destination, const AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

static inline void SxMul(AlignedVector4 *destination, const AlignedVector4 *left, const AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 $vf1,0(%5)\n\tlqc2 $vf2,0(%6)\n\tvmul.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void SxAdd(AlignedVector4 *destination, const AlignedVector4 *left, const AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 $vf1,0(%5)\n\tlqc2 $vf2,0(%6)\n\tvadd.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void SxSub(AlignedVector4 *destination, const AlignedVector4 *left, const AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 $vf1,0(%5)\n\tlqc2 $vf2,0(%6)\n\tvsub.xyz $vf1,$vf1,$vf2\n\tsqc2 $vf1,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(left), "r"(right));
}

void sefTransformPartPoint(SefPartPlacement *placement, Vector4 *resultPoint, int transformMode, const Vector4 *offsetVector);
float sefPlacePart(SefPartPlacement *placement, Vector4 *resultPoint, int placeMode);
void sefFreeLocalData(SefLocalData *lineData, unsigned int pointIndex);

/* Advances one live particle of the load by its step: samples its tracks, moves it through its link and
   returns 1 while it is alive. A dead particle is freed and returns 0. */
s32 sefAdvanceParticle(SxLd *swLoad, s32 index, s32 frame) {

    s32 handle;
    SxPt *part;
    SxPt *linkedPart;
    SxKeys *partKeys;
    f32 spin;
    s32 flags;
    s32 link;
    u8 *sc = &SEF_SCRATCHPAD(u8, 0x0);
    handle = swLoad->h[index];
    if (handle >= 0) {
        part = Sxgp(handle);
        partKeys = &swLoad->keys;
        if (part == 0) {
            return 0;
        }
        SxS(0x2DC)--;
        SEF_KEYFRAME_STEP_BLOCKView.step = SEF_KEYFRAME_STEP_BLOCKView.scale * part->speed;
        if (part->life < 8192.0f) {
            part->life -= SEF_KEYFRAME_STEP_BLOCKView.step;
        }
        if (part->life >= 0.0f) {
            spin = 0.0f;
            flags = SxS(0x298);
            if (SxS(0x2A4)) {
                sefTransformPartPoint(part, SxSV(0x1F0),
                    SxS(0x2D0), 0);
            } else {
                SxCopyQ(SxSV(0x1F0), &part->pos);
            }
            if (!SxS(0x2C4)) {
                SEF_SAMPLE_COLOR_TRACK(SxS(0x260), part->t1D0, &part->colorScale);
            }
            if (SxS(0x25C)) {
                SEF_SAMPLE_TRACK_TRUNCATED(SxS(0x25C), part->t228);
            }
            if (!SxS(0x2C8)) {
                SEF_SAMPLE_TRACK(SxS(0x258), part->t21C);
            }
            if (partKeys->part) {
                sefSampleVectorCurveTrack(partKeys->part, part->t1F0);
            }
            if (!SxS(0x2B8) && sefSampleVectorCurveTrack(SxS(0x254), part->t190)) {
                sefRotateBlockUpdate(part, SxS(0x2A8));
            }
            if (!SxS(0x2BC) && sefSampleVectorCurveTrack(SxS(0x264), part->t1B0)) {
                M_MATH_NORMALIZE_VECTOR2(&part->dir, &part->dir);
            }
            link = part->link;
            D_005510A0 = link;
            linkedPart = Sxgl(link);
            D_005510A4_1FC620 = linkedPart;
            if (linkedPart != 0 && linkedPart->life == 0.0f) {
                D_005510A0 = -1;
                part->link = -1;
                D_005510A4_1FC620 = 0;
            }
            if (!SxS(0x2C0) && sefSampleVectorCurveTrack(SxS(0x250), part->t170)) {
                SxMul(&part->scale, &part->scaleValue, &part->baseScale);
            }
            part->color.q = part->colorValue.q;
            part->color.lane[3] = part->alpha;
            if (SxS(0x2AC)) {
                SEF_ADD_VECTOR_FRACTION(&part->drift, SxS(0x268));
            }
            if (SxS(0x2CC)) {
                func_00207490(SxSV(0x210), part->color.lane,
                              part->scaleValue.x * sefPlacePart(part, SxSV(0x210), SxS(0x2D0)));
            }
            if (SxS(0x2D8)) {
                sefCurveTrackAdvance(swLoad, part, 0,
                    SEF_KEYFRAME_STEP_BLOCKView.step);
            } else {
                part->age += SEF_KEYFRAME_STEP_BLOCKView.step;
            }
            if (partKeys->spin) {
                SEF_SAMPLE_TRACK(partKeys->spin, part->t210);
                spin = part->spin;
                if (spin != 0.0f) {
                    flags |= 2;
                }
            }
            if (SxS(0x2A0)) {
                if (SxS(0x29C) && D_005510A4_1FC620 != 0) {
                    if (part->flags & 0x10) {
                        SxAdd(&part->pos, &part->pos, &D_005510A4_1FC620->offset);
                    } else {
                        part->flags |= 0x10;
                    }
                } else if (part->flags & 0x10) {
                    SxAdd(&part->pos, &part->pos, SEF_SCRATCHPAD(AlignedVector4 *, 0x26C));
                } else {
                    part->flags |= 0x10;
                }
            }
            ((void (*)(SxPt *, s32, s32, f32)) sefAdvanceMotionState)(part, SxS(0x294), (s32) part->life, spin);
            if (SxS(0x2A8)) {
                if (SxS(0x2A0)) {
                    part->flags &= ~4;
                }
                func_001FB8A0(part, SxS(0x288), SxS(0x28C), flags);
            }
            if (SxS(0x2A4)) {
                sefTransformPartPoint(part, SxSV(0x200),
                    SxS(0x2D0), 0);
                SxSub(&part->offset, SxSV(0x200), SxSV(0x1F0));
            } else {
                SxSub(&part->offset, &part->pos, SxSV(0x1F0));
            }
            if (swLoad->ev != 0 && part->res->k1e == 6) {
                sefQueueEffectEvents(
                    (s16) ((f32) swLoad->time / 1000.0f - (f32) part->start), part->cur, swLoad->ev, part->res->k8,
                    ((SxRc *) sefCurrentEffectRecord)->k13e, handle);
            }
            if (((SxRc *) sefCurrentEffectRecord)->k70 == 0x1006402A) {
                SxCopyQ(&part->pos, &((SxRc *) sefCurrentEffectRecord)->p);
                SxCopyQ(&part->scale, &((SxRc *) sefCurrentEffectRecord)->s);
            }
            return 1;
        }
        part->life = 0.0f;
        sefFreeLocalData(swLoad, index);
    }
    return 0;
}

typedef struct SefEffectObject {
    unsigned char unmodeled_000[0x160];
    short *entryData; /* +0x160; the kind is the short at entryData[-2] */
    unsigned char unmodeled_164[0x1BC - 0x164];
    unsigned short flags1BC; /* +0x1BC */
    short mode;              /* +0x1BE */
    unsigned char unmodeled_1C0[0x688 - 0x1C0];
    float scale; /* +0x688 */
    unsigned char unmodeled_68C[0x694 - 0x68C];
    int flags694; /* +0x694 */
    unsigned char unmodeled_698[0x89E - 0x698];
    unsigned short active; /* +0x89E */
} SefEffectObject;

/* Runs the object's particles for this frame, then adds the alive count (scaled by the lane factor in mode 4)
   to the tally lane chosen by the object kind and flags. Returns 1 when the key block flags pass the 0x880 test,
   otherwise the alive count. */
int sefEffectObjectTick(SefEffectObject *object) {
    int aliveCount;
    int particleSlot;
    int tallyWeight;
    int laneFactor;
    int isKindEight;
    char *keyBlock;

    aliveCount = 0;
    if (object->active != 0) {
        particleSlot = 0;
        ((void (*)(void)) sefSwLoadParameterBlock)();
        if (SEF_SCRATCHPAD(int, 0x2DC) > 0) {
            do {
                aliveCount += sefAdvanceParticle(object, particleSlot, 0);
                particleSlot++;
                if (particleSlot >= 0x100) {
                    break;
                }
            } while (SEF_SCRATCHPAD(int, 0x2DC) > 0);
        }
        if (object->mode == 4) {
            isKindEight = 0;
            if (object->entryData[-2] == 8 || SEF_SCRATCHPAD(int, 0x294) == 8) {
                isKindEight = 1;
            }
            laneFactor = (SEF_SCRATCHPAD(int, 0x290) != 0) ? 17 : 14;
            laneFactor = ((object->flags1BC & 0x4000) != 0) ? laneFactor * 2 : laneFactor;
            laneFactor = laneFactor * aliveCount;
            tallyWeight = laneFactor;
            if (isKindEight != 0) {
                {
                    unsigned int *laneBase = (unsigned int *) sefEffectTallies;
                    laneBase = laneBase + 1;
                    laneBase[SEF_SCRATCHPAD(int, 0x284) * 6] += tallyWeight;
                }
            } else if ((object->flags694 & 0x4000) != 0) {
                {
                    unsigned int *laneBase = (unsigned int *) sefEffectTallies;
                    laneBase = laneBase + 2;
                    laneBase[SEF_SCRATCHPAD(int, 0x284) * 6] += tallyWeight;
                }
            } else {
                {
                    unsigned int *laneBase = (unsigned int *) sefEffectTallies;
                    laneBase = laneBase + 0;
                    laneBase[SEF_SCRATCHPAD(int, 0x284) * 6] += tallyWeight;
                }
            }
        } else {
            {
                unsigned int *laneBase = (unsigned int *) sefEffectTallies;
                laneBase = laneBase + 3;
                laneBase[SEF_SCRATCHPAD(int, 0x284) * 6] += aliveCount;
            }
        }
    }
    keyBlock = (char *) sefRequestKeyBlock;
    if ((*(unsigned long long *) (keyBlock + 0x2B8) & 0x880) == 0x80) {
        return 1;
    }
    if ((*(unsigned int *) (keyBlock + 0x2B8) & 0x800) != 0) {
        if (aliveCount <= 0) {
            object->scale = -0.1f;
        }
    }
    return aliveCount;
}

extern SefLocalData *sefLineDataRecords;
int sefAllocLineData(void);

void sefFreeLocalData(SefLocalData *lineData, unsigned int pointIndex)
{
    if (pointIndex < SEF_LINE_PARTICLE_COUNT) {
        sevFreePtAllocator(lineData->pointHandles[pointIndex]);
        lineData->pointHandles[pointIndex] = -1;
        if (lineData->pointCount != 0) {
            lineData->pointCount--;
        }
    }
}

int sefAllocLocalData(SefLocalData *lineData)
{
    int pointIndex;
    int scanEnd;
    int pointHandle;

    pointIndex = lineData->cursor;
    scanEnd = (pointIndex - 1) & (SEF_LINE_PARTICLE_COUNT - 1);
    if (lineData->allocScanFlag == 0) {
        if (lineData->pointHandles[pointIndex] >= 0) {
            return -1;
        }
    } else {
        for (; pointIndex != scanEnd; pointIndex = (pointIndex + 1) & (SEF_LINE_PARTICLE_COUNT - 1)) {
            if (lineData->pointHandles[pointIndex] < 0) {
                break;
            }
        }
        if (pointIndex == scanEnd) {
            return -1;
        }
    }
    pointHandle = sevAllocPtAllocator();
    if (pointHandle < 0) {
        return -1;
    }
    lineData->pointHandles[pointIndex] = pointHandle;
    lineData->cursor = (pointIndex + 1) & (SEF_LINE_PARTICLE_COUNT - 1);
    lineData->pointCount++;
    return pointIndex;
}

void sefDestroyLocalData(SefLocalData *lineData)
{
    int i;

    for (i = 0; i < SEF_LINE_PARTICLE_COUNT && lineData->pointCount != 0; i++) {
        if (lineData->pointHandles[i] >= 0) {
            sefFreeLocalData(lineData, i);
        }
    }
}

typedef struct SefEffectEntryRecordView {
    unsigned char unmodeled_00[0x04];
    unsigned int flags; /* +0x04 */
    short entryId;      /* +0x08 */
} SefEffectEntryRecordView;

void sefMemZero(void *zeroedMemory, int byteLength);

typedef struct SefLineScript {
    unsigned short tables[2]; /* +0x00: the key and line tables */
    float gravity;            /* +0x04 */
    float height;             /* +0x08 */
    short collision;          /* +0x0C */
    short noAttach;           /* +0x0E */
} SefLineScript;

typedef struct SefLineData {
    float matrix[4][4]; /* +0x000 */
    unsigned char unmodeled_040[0x50 - 0x40];
    Vector4 direction; /* +0x050 */
    unsigned char unmodeled_060[0x70 - 0x60];
    Vector4 noise;    /* +0x070 */
    Vector4 position; /* +0x080 */
    unsigned char unmodeled_090[0xA8 - 0x90];
    float scaleTime; /* +0x0A8 */
    unsigned char unmodeled_0AC[0xB0 - 0xAC];
    Vector4 positionValue; /* +0x0B0 */
    unsigned char unmodeled_0C0[0xC8 - 0xC0];
    short directionMode; /* +0x0C8 */
    unsigned char unmodeled_0CA[0xD0 - 0xCA];
    Vector4 velocityValue; /* +0x0D0 */
    unsigned char unmodeled_0E0[0xE8 - 0xE0];
    int constantKeys; /* +0x0E8 */
    unsigned char unmodeled_0EC[0x110 - 0xEC];
    Vector4 rotationValue; /* +0x110 */
    unsigned char unmodeled_120[0x130 - 0x120];
    Vector4 scaleValue; /* +0x130 */
    unsigned char unmodeled_140[0x160 - 0x140];
    SefTrackKey16 *keys[7];   /* +0x160 */
    SefAnalyzeRecord *images; /* +0x17C */
    Vector4 imageOffset;      /* +0x180 */
    SefTrackKey16 *extraKeys; /* +0x190 */
    unsigned char unmodeled_194[0x1A0 - 0x194];
    SefImagePacket packets[30]; /* +0x1A0 */
    unsigned char unmodeled_560[0x660 - 0x560];
    SefTrackKey16 *scaleKeys;  /* +0x660 */
    SefTrackKey16 *angleKeys;  /* +0x664 */
    SefTrackKey16 *colorKeys;  /* +0x668 */
    SefTrackKey *alphaKeys;    /* +0x66C */
    void *spinKeys;            /* +0x670 */
    void *partKeys;            /* +0x674 */
    SefTrackKey *sizeKeys;     /* +0x678 */
    short pointLife;           /* +0x67C */
    short speedVariation;      /* +0x67E */
    float gravity;             /* +0x680 */
    float height;              /* +0x684 */
    float startFrame;          /* +0x688 */
    int frame;                 /* +0x68C */
    int emitFrame;             /* +0x690 */
    int inUse;                 /* +0x694 */
    short startTime;           /* +0x698 */
    short startFrameShort;     /* +0x69A */
    short pointHandles[0x100]; /* +0x69C */
    unsigned char unmodeled_89C[0x89C - 0x69C - 0x200];
    short cursor;            /* +0x89C */
    short pointCount;        /* +0x89E */
    short count;             /* +0x8A0 */
    short collision;         /* +0x8A2 */
    short reserved8A4;       /* +0x8A4 */
    unsigned char link;      /* +0x8A6 */
    unsigned char mode;      /* +0x8A7 */
    unsigned char kind;      /* +0x8A8 */
    unsigned char imageMode; /* +0x8A9 */
    unsigned char unmodeled_8AA[0x8B0 - 0x8AA];
} SefLineData;

static s32 D_00551514;
extern s32 D_004749E0[];
extern s32 D_0045417C[9];

#define SEF_QUARTER_PI 0.7853982f

extern const DoublewordAlignedVector4 D_0053EF10;
extern const DoublewordAlignedVector4 D_0053EF20;
int scGetImmAdrImmIdx2(int scriptBase, unsigned short *immediateTable, int immediateIndex);
int MMathIsVectorEqual(const Vector4 *first, const Vector4 *second);
void quaternionFromAngleZ(SefLineData *line, f32 angle);
void sefMemFillWords(void *fillMemory, int fillPattern, int fillByteLength);
void sefNoiseNextVector(Vector4 *pos);
void sefGetPosition(Vector4 *resultPoint, const Vector4 *sourcePoint, int pointKind);

static inline void sefCopyKeyVectorW0(Vector4 *destination, const SefTrackKey16 *key) {
    __asm__("lqc2 $vf2,0(%5)\n\tvmove.w $vf2w,$vf0w\n\tsqc2 $vf2,0(%4)"
            : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
            : "r"(destination), "r"(key)
            : "memory");
}

static inline void sefZeroMatrixRow0(SefLineData *line) {
    __asm__("sqc2 $vf0,0(%0)" : : "r"(line) : "memory");
}

typedef struct SefAngleHeaderView {
    unsigned char unmodeled_000[0x664];
    unsigned short *angleHeader; /* +0x664 */
} SefAngleHeaderView;

static inline int sefGetAngleTag(SefLineData *line) {
    if (line->angleKeys != 0) {
        return ((short *) line->angleKeys)[-9];
    }
    return 0;
}

static inline int sefIsVectorNonZero(DoublewordVector4 *vector) {
    return (vector->words[1] | vector->words[0]) != 0;
}

#define SEF_IMAGE_PACKET_COUNT 30
#define SEF_ALT_PACKET_COUNT 8

/* The animation object: its record list at +0x17C, the 30 image packets at +0x1A0 and the 8 alternate
 * packets after them at +0x560. */
typedef struct SefAnimObject {
    unsigned char unmodeled_0000[0x17C];
    SefAnalyzeRecord *animRecordTable;               /* +0x17C */
    unsigned char unmodeled_0180[0x1A0 - 0x180];
    SefImagePacket packets[SEF_IMAGE_PACKET_COUNT]; /* +0x1A0 */
    SefImagePacket altPackets[SEF_ALT_PACKET_COUNT]; /* +0x560 */
} SefAnimObject;

/* Initializes one effect line: clears its state, takes the key and line table pointers from the script and
   derives its start frame, gravity, collision, rotation, scale and direction from the first key of each track. */
void sefInitLine(SefLineData *line, s32 startTime, s32 startFrame, SefLineScript *lineScript, s32 lineLink,
                   s32 requestCount, s32 inUseFlags, s32 lineKind) {
    unsigned short *keyTable;
    unsigned short *lineTable;
    s32 countIsZero;
    s32 collisionScaled;
    s32 inUseBits;
    f32 gravityValue;
    f32 heightValue;
    s16 collisionValue;
    s16 noAttach;
    SefAnalyzeRecord *images;
    s32 hasOrigin;
    f32 axisAngle;
    s32 angleTag;
    SefTrackKey16 **keyArray;

    keyTable = (unsigned short *) scGetImmAdrImmIdx2(D_00551514, lineScript->tables, 0);
    lineTable = (unsigned short *) scGetImmAdrImmIdx2(D_00551514, lineScript->tables, 1);
    gravityValue = lineScript->gravity;
    heightValue = lineScript->height;
    collisionValue = lineScript->collision;
    noAttach = lineScript->noAttach;
    sefMemZero(line, 0x8B0);
    sefMemFillWords(line->pointHandles, -1, sizeof(line->pointHandles));
    line->cursor = 0;
    line->link = lineLink;
    line->pointCount = 0;
    line->count = 0;
    line->startTime = startTime;
    line->startFrameShort = startFrame;
    line->frame = 0;
    countIsZero = requestCount == 0;
    line->mode = countIsZero;
    if (startFrame == 0 || lineKind == 12) {
        line->mode = countIsZero | 2;
    }
    line->gravity = (D_004749E0[0] == 0) ? 0.0f : gravityValue;
    if (D_004749E0[1] != 0) {
        collisionScaled = (s32) (f32) collisionValue;
    } else {
        collisionScaled = 0;
    }
    inUseBits = inUseFlags | 0x20;
    line->collision = collisionScaled;
    line->height = heightValue;
    line->emitFrame = -1000;
    line->kind = lineKind;
    line->inUse = line->inUse | inUseBits;
    line->reserved8A4 = 0;
    if (startFrame != 0) {
        line->startFrame = (f32) (startFrame - startTime);
    }
    if (noAttach != 0) {
        line->inUse |= 0x100;
    }
    line->keys[0] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 0);
    line->keys[1] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 1);
    line->keys[2] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 2);
    line->keys[3] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 3);
    line->keys[4] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 4);
    line->keys[5] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 5);
    line->keys[6] = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, (unsigned short *) keyTable, 6);
    line->extraKeys = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, lineTable, 12);
    sefNoiseNextVector(&line->noise);
    Vu0SefMulVectorXYZ(&line->noise, &line->noise, (Vector4 *) &line->keys[0][-1]);
    if (line->keys[1] != 0) {
        sefCopyKeyVectorW0(&line->rotationValue, line->keys[1]);
    }
    if (line->keys[3] != 0) {
        sefCopyKeyVectorW0(&line->scaleValue, line->keys[3]);
    }
    if (line->keys[0] != 0) {
        sefCopyKeyVectorW0(&line->positionValue, line->keys[0]);
        if (func_00206A50(line->keys[0]) >= 0) {
            line->inUse |= 0x8000;
        }
    }
    if (line->keys[2] != 0) {
        sefCopyKeyVectorW0(&line->velocityValue, line->keys[2]);
        if (func_00206A50(line->keys[2]) >= 0) {
            line->inUse |= 0x10000;
        }
    }
    if ((s16) lineTable[0] == 0x2000) {
        ((SefEffectEntryRecordView *) sefCurrentEffectRecord)->flags |= 0x4000000;
    }
    line->pointLife = lineTable[0];
    line->speedVariation = lineTable[2];
    line->scaleKeys = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, lineTable, 4);
    line->angleKeys = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, lineTable, 5);
    line->colorKeys = (SefTrackKey16 *) scGetImmAdrImmIdx2(D_00551514, lineTable, 6);
    line->alphaKeys = (SefTrackKey *) scGetImmAdrImmIdx2(D_00551514, lineTable, 7);
    line->partKeys = (void *) scGetImmAdrImmIdx2(D_00551514, lineTable, 8);
    line->spinKeys = (void *) scGetImmAdrImmIdx2(D_00551514, lineTable, 9);
    line->sizeKeys = (SefTrackKey *) scGetImmAdrImmIdx2(D_00551514, lineTable, 11);
    images = (SefAnalyzeRecord *) scGetImmAdrImmIdx2(D_00551514, lineTable, 10);
    line->imageMode = ((unsigned char *) images)[-20];
    line->images = images;
    sefZeroMatrixRow0(line);
    if (line->angleKeys != 0) {
        Vector4 *origin;

        origin = (Vector4 *) &line->angleKeys[-2];
        hasOrigin = (*(s32 *) &origin->z | *(u64 *) origin) != 0;
        if (hasOrigin) {
            DoublewordAlignedVector4 positiveDirection;
            DoublewordAlignedVector4 negativeDirection;
            positiveDirection = D_0053EF10;
            negativeDirection = D_0053EF20;
            if (MMathIsVectorEqual(origin, &positiveDirection)) {
                axisAngle = SEF_QUARTER_PI;
                line->inUse |= 0x40;
                quaternionFromAngleZ(line, axisAngle);
            } else if (MMathIsVectorEqual(origin, &negativeDirection)) {
                axisAngle = -SEF_QUARTER_PI;
                line->inUse |= 0x40;
                quaternionFromAngleZ(line, axisAngle);
            } else if (((int (*)(SefLineData *line, const Vector4 *axes, const Vector4 *origin))
                            quaternionFromToRotation)(line, &sefDefaultAxis, (Vector4 *) &line->angleKeys[-2]) != 0) {
                line->inUse |= 0x40;
            }
            vu0NormalizeXyzwTo(line, line);
        }
        if (line->angleKeys[1].key == SEF_TRACK_END_KEY) {
            line->constantKeys |= 0x1;
        }
    }
    if (line->sizeKeys != 0) {
        if (line->sizeKeys[1].key == SEF_TRACK_END_KEY) {
            line->constantKeys |= 0x20;
        }
        line->scaleTime = line->sizeKeys[-1].sampleValue;
    }
    if (line->alphaKeys != 0 && line->alphaKeys[1].key == SEF_TRACK_END_KEY) {
        line->constantKeys |= 0x40;
    }
    if (line->scaleKeys != 0 && line->scaleKeys[1].key == SEF_TRACK_END_KEY) {
        line->constantKeys |= 0x10;
    }
    if (line->colorKeys != 0 && line->colorKeys[1].key == SEF_TRACK_END_KEY) {
        line->constantKeys |= 0x8;
    }
    if (line->keys[4] != 0) {
        Vector4 axis;

        line->directionMode = line->keys[4][-1].key;
        if (line->keys[4][1].key == SEF_TRACK_END_KEY) {
            sefCopyKeyVectorW0(&axis, line->keys[4]);
            if (*(u64 *) &axis == 0) {
                if (axis.z == 1.0f) {
                    line->inUse |= 0x400;
                } else if (axis.z == -1.0f) {
                    line->inUse |= 0x800;
                }
            }
            MMathNormalizeVector2(&line->direction, &axis);
            line->constantKeys |= 0x2;
        }
    }
    if (line->images != 0) {
        line->imageOffset.x = ((Vector4 *) line->images)[-1].x;
        line->imageOffset.y = ((Vector4 *) line->images)[-1].y;
        line->imageOffset.z = ((Vector4 *) line->images)[-1].z;
        line->imageOffset.w = ((Vector4 *) line->images)[-1].w;
        if (sefIsVectorNonZero((DoublewordVector4 *) &line->imageOffset)) {
            line->inUse |= 0x200;
        }
    }
    if (sefAnalyzeAnim((struct SefAnimObject *) line) >= 2) {
        line->inUse |= 0x8;
    }
    if (line->packets[0].flags & 0x20) {
        line->inUse |= 0x4000;
    }
    if (line->packets[0].flags & 0x80) {
        line->inUse |= 0x80;
    }
    keyArray = &line->keys[0];
    angleTag = sefGetAngleTag(line);
    if (D_0045417C[0] != 0 && angleTag == 0x3000) {
        if (line->inUse & 1) {
            if (!(((SefAngleHeaderView *) line)->angleHeader[-1] & 7)) {
                line->inUse |= 0x20000;
                if ((s16) line->packets[0].resourceKind != 6) {
                    line->inUse |= 0x40000;
                }
            }
            if (!(line->inUse & 0x20000)) {
                line->inUse |= 0x6;
            }
        }
    }
    {
        unsigned int positionMode;

        positionMode = (unsigned short) keyArray[0][-1].key - 0x10;
        if (positionMode >= 2U) {
            s32 pointKind;

            if (!(line->inUse & 0x28000)) {
                pointKind = (short) keyArray[0][-1].key;
            } else {
                pointKind = 0;
            }
            sefVectorMode = line->inUse & 0x6;
            sefGetPosition(&line->position, 0, pointKind);
            sefVectorMode = 0;
        }
    }
}

/* Number of entries of sefLineDataRecords. */
extern int sefLineDataCount;

/* Claims the first free line record (its inUse flag becomes 32) and returns its index, or -1 when none is free. */
int sefAllocLineData(void)
{
    SefLocalData *line;
    int lineIndex;

    lineIndex = 0;
    if (sefLineDataCount > 0) {
        line = sefLineDataRecords;
        do {
            if (line->inUse == 0) {
                line->inUse = 32;
                return lineIndex;
            }
            line++;
        } while (++lineIndex < sefLineDataCount);
    }
    return -1;
}

/* Destroys the local data of one line record and marks the record free. Indices out of range are ignored. */
void sefFreeLineData(int lineIndex)
{
    SefLocalData *line;

    if (lineIndex >= 0 && lineIndex < sefLineDataCount) {
        line = 0;
        if (lineIndex >= 0) {
            line = sefLineDataRecords + lineIndex;
        }
        sefDestroyLocalData(line);
        line->inUse = 0;
    }
}

/* An animation record list ends at the record whose time is SEF_LIST_END. */
#define SEF_LIST_END 4096
#define SEF_IMAGE_PACKET_SCALE 0.005f
#define SEF_GS_PSMT8 0x13
#define SEF_GS_PSMT4 0x14

SvTypeListEntry *svGetImageListItem(int type, unsigned int flags);

int sefAnalyzeAnim(SefAnimObject *animObject)
{
    int packetCount;
    SefAnalyzeRecord *animRecords;
    SefImagePacket *imagePacket;
    SefImagePacket *altImagePacket;
    short *altPaletteId;
    int psm;

    animRecords = animObject->animRecordTable;
    packetCount = 0;
    if (animRecords->time != SEF_LIST_END) {
        altImagePacket = animObject->altPackets;
        imagePacket = animObject->packets;
        do {
            int packetFlags;
            short mapperIndex;
            float animScale;
            SvTypeListEntry *image;
            SvTypeListEntry *palette;

            packetFlags = animRecords->flags;
            mapperIndex = sefCurrentEffectRecord->entryId;
            if ((packetFlags & 0x10) != 0) {
                if (animRecords[packetCount].imageId == 0 || animRecords[packetCount].altImageId == 0) {
                    memset(imagePacket, 0, 0x20);
                    imagePacket->resourceKind = 4;
                    imagePacket->flags = packetFlags;
                    break;
                }
            }
            if ((animRecords[packetCount].imageId & 0x100) != 0) {
                image = svGetImageListItem(0, animRecords[packetCount].imageId);
                palette = svGetImageListItem(0, animRecords[packetCount].altImageId);
            } else {
                image = svGetImageListItem(mapperIndex, animRecords[packetCount].imageId);
                palette = svGetImageListItem(mapperIndex, animRecords[packetCount].altImageId);
            }
            if (image != 0 && palette != 0) {
                if (packetCount < SEF_IMAGE_PACKET_COUNT) {
                    animScale = SEF_IMAGE_PACKET_SCALE;
                    imagePacket->flags = packetFlags;
                    imagePacket->time = animRecords[packetCount].time;
                    imagePacket->scaledWidth = image->width * animScale;
                    imagePacket->scaledHeight = image->height * animScale;
                    imagePacket->resourceKind = image->resourceKind;
                    psm = image->bitsPerPixel == 8 ? SEF_GS_PSMT8 : SEF_GS_PSMT4;
                    imagePacket->image = image->pixelData;
                    imagePacket->palette = palette->pixelData;
                    imagePacket->tex0 = GS_SET_TEX0(image->texBasePointer, image->bufferWidthPixels >> 6,
                        psm, image->textureWidthExponent, image->textureHeightExponent, 1, 0,
                        palette->texBasePointer, 0, 0, 0, 1);
                    if (animRecords[packetCount].secondaryImageId != 0 &&
                        *(altPaletteId = &animRecords[packetCount].secondaryPaletteId) != 0) {
                        image = svGetImageListItem(mapperIndex, animRecords[packetCount].secondaryImageId);
                        palette = svGetImageListItem(mapperIndex, *altPaletteId);
                        if (image != 0 && palette != 0) {
                            if (packetCount < SEF_ALT_PACKET_COUNT) {
                                imagePacket->flags |= 0x4000;
                                imagePacket[SEF_IMAGE_PACKET_COUNT].image = image->pixelData;
                                imagePacket[SEF_IMAGE_PACKET_COUNT].scaledWidth = image->width * animScale;
                                imagePacket[SEF_IMAGE_PACKET_COUNT].scaledHeight = image->height * animScale;
                                imagePacket[SEF_IMAGE_PACKET_COUNT].resourceKind = image->resourceKind;
                                imagePacket[SEF_IMAGE_PACKET_COUNT].palette = palette->pixelData;
                                psm = image->bitsPerPixel == 8 ? SEF_GS_PSMT8 : SEF_GS_PSMT4;
                                imagePacket[SEF_IMAGE_PACKET_COUNT].tex0 = GS_SET_TEX0(image->texBasePointer,
                                    image->bufferWidthPixels >> 6, psm, image->textureWidthExponent,
                                    image->textureHeightExponent, 1, 0, palette->texBasePointer, 0, 0, 0, 1);
                            }
                        } else {
                            memset(altImagePacket, 0, 0x20);
                        }
                    }
                }
            } else {
                memset(imagePacket, 0, 0x20);
            }
            altImagePacket++;
            imagePacket++;
            packetCount++;
        } while (animRecords[packetCount].time != SEF_LIST_END);
    }
    return packetCount;
}

typedef struct SefEffectRequest {
    u64 tex0[2];
    f32 **mtx;
    u8 rgba[4];
    void *sef;
    s16 owner;
    s16 unk_1E;
    s32 unk_20;
    f32 ring;
    s32 unk_28;
    f32 spread;
    f32 life;
    f32 f34;
    f32 f38;
    f32 minD;
    s16 parts[2];
    s16 maxS;
    s16 shrink;
    s16 blend;
    s16 imgc;
    s16 mode;
    s16 unk_4E;
    s16 layer;
    s16 rec;
    s16 follow;
    s16 frames;
    s16 unk_58;
    s16 unk_5A;
    s16 unk_5C;
    s16 unk_5E;
    f32 f60;
    s16 unk_64;
    s16 unk_66;
} SefEffectRequest;

typedef struct SefEffectRecordView {
    u8 a[8];
    s16 layer;
    u8 b[0x4A];
    s32 unk_54;
    s32 unk_58[2];
    u8 c[0xF0];
} SefEffectRecordView;

typedef struct SefImageInfo {
    u8 a[0x12];
    u16 tbp;
    u16 w;
    u16 w1;
    u8 b[4];
    u16 tw;
    u16 th;
} SefImageInfo;

typedef struct SefScratchpad {
    u8 a[0x694];
    s32 fl;
} SefScratchpad;

s32 sefFindSearchKey(SefSearchOwner *owner, s32 key);
void sdvAllocAlter(SefEffectRequest *, s32, s32);
f32 sefGetEntryScale(s32 id);

#define SEF_PACK_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld) ((u64)(tbp) | ((u64)(tbw) << 14) | ((u64)(psm) << 20) | ((u64)(tw) << 26) | ((u64)(th) << 30) | ((u64)(tcc) << 34) | ((u64)(tfx) << 35) | ((u64)(cbp) << 37) | ((u64)(cpsm) << 51) | ((u64)(csm) << 55) | ((u64)(csa) << 56) | ((u64)(cld) << 61))

extern SefEffectRecord *sefEffectRecords;

/* Queues the effect requests of the event list from *cursor while the time has reached each event key, then
   stores the advanced cursor. When the first event part is 0x141, one request is queued per existing part 0x101 and up. */
void sefQueueEffectEvents(s16 time, u8 *cursor, SqEv *events, void *sef, s32 recIndex, s32 owner) {
    SefEffectRequest req;
    s32 eventIndex;
    s32 partIndex;

    if (events == 0 || cursor == 0) {
        return;
    }
    eventIndex = *cursor;
    while (time >= events[eventIndex].key && events[eventIndex].key < 0x1000) {
        SefEffectRecordView *rec;
        s32 spFlagBit13;
        s32 addr;
        s32 layer;
        s32 imageId;
        s32 paletteId;
        s32 immContext;

        rec = &((SefEffectRecordView *) sefEffectRecords)[recIndex];
        spFlagBit13 = ((SEF_SCRATCHPAD(SefScratchpad *, 0x0))->fl >> 13) & 1;
        layer = rec->layer;
        immContext = rec->unk_54;
        if (events->k3c[0] >= 0) {
            addr = scGetImmAdrImmIdx2(immContext, events->k3c, 0);
        } else {
            addr = 0;
        }
        memset(&req, 0, sizeof(req));
        req.mtx = (f32 **) rec->unk_58;
        req.life = events[eventIndex].life;
        req.parts[0] = events[eventIndex].parts[0];
        req.parts[1] = events[eventIndex].parts[1];
        req.owner = owner;
        req.sef = sef;
        req.maxS = events[eventIndex].maxS > 64 ? 64 : events[eventIndex].maxS;
        req.shrink = events[eventIndex].shrink;
        req.mode = events[eventIndex].mode & 0xF;
        req.follow = events[eventIndex].mode >> 4;
        req.blend = events[eventIndex].blend;
        req.rgba[0] = events[eventIndex].rgba[0];
        req.rgba[1] = events[eventIndex].rgba[1];
        req.rgba[2] = events[eventIndex].rgba[2];
        req.rgba[3] = events[eventIndex].rgba[3];
        req.spread = events[eventIndex].spread;
        req.ring = events[eventIndex].ring;
        req.layer = layer;
        req.rec = recIndex;
        req.frames = events[eventIndex].k2c;
        req.unk_58 = events[eventIndex].k2e;
        req.f34 = events[eventIndex].f24;
        req.unk_5A = events[eventIndex].k30;
        req.f38 = events[eventIndex].f28;
        req.unk_5C = events[eventIndex].k32;
        req.minD = events[eventIndex].k34 * 0.001f;
        req.unk_64 = events[eventIndex].k36;
        req.f60 = events[eventIndex].f38 * 0.01f;
        if (spFlagBit13) {
            if (events[eventIndex].parts[0] & 0x100) {
                req.f60 *= sefGetEntryScale(rec->unk_58[0]);
            } else if (events[eventIndex].parts[0] & 0x200) {
                req.f60 *= sefGetEntryScale(rec->unk_58[1]);
            }
        }
        if (req.follow != 0 && req.maxS > 32) {
            req.maxS = 32;
        }
        for (partIndex = 0; partIndex < 2; partIndex++) {
            imageId = events[eventIndex].img[partIndex][0];
            paletteId = events[eventIndex].img[partIndex][1];
            if (imageId >= 0 && paletteId >= 0) {
                SefImageInfo *image;
                SefImageInfo *palette;
                s32 psm;

                image = (SefImageInfo *) svGetImageListItem(layer, imageId);
                palette = (SefImageInfo *) svGetImageListItem(layer, paletteId);
                if (image != 0 && palette != 0) {
                    psm = image->w == 8 ? SEF_GS_PSMT8 : SEF_GS_PSMT4;
                    req.tex0[req.imgc] = SEF_PACK_TEX0(image->tbp, image->w1 >> 6, psm, image->tw, image->th, 1, 0,
                        palette->tbp, 0, 0, 0, 1);
                    req.imgc++;
                    req.layer = layer;
                }
            }
        }
        if (events->parts[0] == 0x141) {
            for (partIndex = 0; partIndex < 0x33; partIndex++) {
                if (sefFindSearchKey(sef, partIndex) >= 0) {
                    req.parts[0] = partIndex + 0x101;
                    sdvAllocAlter(&req, addr, spFlagBit13);
                }
            }
        } else {
            sdvAllocAlter(&req, addr, spFlagBit13);
        }
        eventIndex++;
    }
    *cursor = eventIndex;
}

typedef union SefKeyVector {
    float f[4];
    short h[8];
    AlignedVector4 v;
} SefKeyVector;

typedef struct SefKeyChannels {
    SefKeyVector *k0;
    SefKeyVector *k4;
    SefKeyVector *k8;
    SefKeyVector *kc;
    SefKeyVector *k10;
    SefKeyVector *k14;
} SefKeyChannels;

typedef struct SefPartModel {
    unsigned char a[0x1E];
    short h1E;
} SefPartModel;

typedef struct SefPart {
    M4 m;
    AlignedVector4 v40;
    AlignedVector4 v50;
    AlignedVector4 v60;
    AlignedVector4 v70;
    AlignedVector4 v80;
    AlignedVector4 v90;
    AlignedVector4 vA0;
    AlignedVector4 vB0;
    AlignedVector4 vC0;
    unsigned char a[0x30];
    AlignedVector4 v100;
    AlignedVector4 v110;
    AlignedVector4 v120;
    unsigned char b[0x10];
    short h140;
    unsigned short h142;
    unsigned char c[0x158 - 0x144];
    SefPartModel *p158;
    unsigned char d[0x178 - 0x15C];
    float f178;
    float f17C;
    AlignedVector4 v180;
    unsigned char e[8];
    float f198;
    float f19C;
    AlignedVector4 v1A0;
    unsigned char f[12];
    union {
        float f;
        int i;
    } Lfu1BC;
    AlignedVector4 v1C0;
    unsigned char g[10];
    union {
        short s;
        unsigned short u;
    } Lfh1DA;
    unsigned char h[4];
    int col[4];
    unsigned char i[8];
    int fl;
    union {
        int i;
        float f;
    } Lfw1FC;
    AlignedVector4 v200;
    unsigned char j[0x224 - 0x210];
    float f224;
    unsigned char k[2];
    short h22A;
    unsigned char l[4];
    unsigned char sl[64];
} SefPart;

typedef struct SefPartPool {
    unsigned char hd[16];
    SefPart d[1];
} SefPartPool;

typedef struct SefLocalEffectData {
    unsigned char a[0x50];
    AlignedVector4 v50;
    unsigned char b[0x48];
    float fA8;
    unsigned char c[0x3C];
    int fE8;
    unsigned char d[0x170 - 0xEC];
    SefKeyVector *k170;
    unsigned char e[0x4A];
    short h1BE;
    unsigned char f[0x660 - 0x1C0];
    SefKeyChannels ks;
    SefKeyVector *k678;
    short h67C;
    short h67E;
    unsigned char g[0x68C - 0x680];
    int w68C;
    int w690;
    int w694;
    int w698;
    short hh[256];
} SefLocalEffectData;

typedef struct SefKeyBlock {
    unsigned char a[0x70];
    AlignedVector4 v;
} SefKeyBlock;

typedef struct SefVectorBits {
    long d;
    int w;
} SefVectorBits;

extern unsigned short D_00551074;
extern AlignedVector4 D_0076BFF0;
void func_00206D78(AlignedVector4 *out, unsigned char *line, float time);
void func_00206F18(AlignedVector4 *out, unsigned char *line, float time);
extern void MMathRotateMatrixYXZ(M4*,int,AlignedVector4*);

#define SEF_PART_ALLOC_COUNT (*(int (*))&sefPtAllocCount)
#define SEF_PART_POOL ((SefPartPool *)sefPtAllocatorState)
#define SEF_REQUEST_KEY_BLOCK (*(SefKeyBlock (*))&sefRequestKeyBlock)
#define SEF_ALLOC_LOCAL_DATA ((int(*)(SefLocalEffectData*))sefAllocLocalData)
#define SEF_MEM_ZERO ((void(*)(void*,int))sefMemZero)
#define MEMSET ((void *(*)(void*,int,unsigned int))memset)

static inline void sefVuStoreZero(AlignedVector4 *destination) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void sefVuStoreOne(AlignedVector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw vf1,vf0,vf0w\n sqc2 vf1,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination));
}

static inline void sefVecCopy(AlignedVector4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

static inline void sefVecMul(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vmul.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void sefVecSub(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vsub.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void sefVecScale(AlignedVector4 *destination, AlignedVector4 *left, float factor) {
    __asm__ __volatile__(
        "lqc2 vf1,0(%4)\n mfc1 $8,%5\n qmtc2.ni $8,vf2\n vmulx.xyz vf1,vf1,vf2x\n sqc2 vf1,0(%3)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
        : "r"(destination), "r"(left), "f"(factor)
        : "$8", "memory");
}

static inline void sefVecCopyUnitW(AlignedVector4 *destination, AlignedVector4 *left) {
    __asm__ __volatile__("lqc2 vf2,0(%1)\n vmove.w vf2,vf0\n sqc2 vf2,0(%0)" : : "r"(destination), "r"(left) : "memory");
}

static inline void sefVecToIntColor(int *destination, AlignedVector4 *left, AlignedVector4 *right, float factor) {
    __asm__ __volatile__(
        "mfc1 $8,%4\n lqc2 vf2,0(%2)\n lqc2 vf1,0(%3)\n qmtc2.ni $8,vf3\n vmove.w vf2,vf0\n vmul.xyzw vf1,vf1,vf2\n vminix.xyz vf2,vf1,vf3x\n vftoi0.xyzw vf2,vf2\n sqc2 vf2,0(%1)"
        : "=m"(*destination)
        : "r"(destination), "r"(left), "r"(right), "f"(factor)
        : "$8");
}

static inline int sefVecNonZero(AlignedVector4 *vector) {
    SefVectorBits *p = (SefVectorBits *) vector;
    return (p->w | p->d) != 0;
}

static inline SefPart *sefPtGetPart(int handle) {
    if (handle < 0 || handle >= SEF_PART_ALLOC_COUNT) {
        return 0;
    }
    return &SEF_PART_POOL->d[handle];
}

static inline void sefVf20Load(AlignedVector4 *left) {
    __asm__ __volatile__("lqc2 vf20,0(%0)" : : "r"(left));
}

static inline void sefVf20MulBy(AlignedVector4 *left) {
    __asm__ __volatile__("lqc2 vf1,0(%0)\n vmul.xyz vf20,vf20,vf1" : : "r"(left));
}

static inline void sefVf20Scale(float factor) {
    __asm__ __volatile__("mfc1 $8,%0\n qmtc2.ni $8,vf1\n vmulx.xyz vf20,vf20,vf1x" : : "f"(factor) : "$8");
}

static inline void sefVf20Store(AlignedVector4 *destination) {
    __asm__ __volatile__("sqc2 vf20,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

f32 sefNoiseNext(void);
f32 sefRandf(void);

/* Allocates one part slot of the local effect data for the given kind and fills its transform, colour and key
   vectors from the key blocks of the owner. Returns the slot, or -1 when no local data slot is free. */
int sefSpawnLocalPart(SefLocalEffectData *localData, int kind, float scale) {
    AlignedVector4 rotVec;
    SefKeyBlock *keyBlock;
    M4 rotMatrix;
    SefKeyChannels *keys = &localData->ks;
    int slotIndex;
    SefPart *part;
    SefKeyVector *keyPtr;
    int rotKeyFlag;
    int hasRotation;
    float keyLength;
    slotIndex = SEF_ALLOC_LOCAL_DATA(localData);
    if (slotIndex < 0) {
        return -1;
    }
    part = sefPtGetPart(localData->hh[slotIndex]);
    if (part == 0) {
        return slotIndex;
    }
    SEF_MEM_ZERO(part, 0x270);
    MEMSET(part->sl, -1, 64);
    part->h142 = D_00551074;
    part->Lfh1DA.s = kind;
    part->h140 = (short) (int) ((float) localData->w68C / 1000.0f);
    rotKeyFlag = 0;
    func_00206D78(&part->vA0, localData, scale);
    func_00206F18(&part->vB0, localData, scale);
    sefVecCopy(&part->v120, &D_0076BFF0);
    sefVecCopy(&part->v70, &part->vA0);
    part->f17C = 1.0f;
    part->Lfu1BC.f = (float) localData->h67C;
    if (localData->h67E != 0) {
        part->f17C += sefNoiseNext() * ((float) localData->h67E * 0.01f);
    }
    sefCurveTrackAdvance(localData, part, -1, 1.0f);
    if (keys->k4 != 0) {
        rotKeyFlag = keys->k4->h[-1];
    }
    hasRotation = (part->p158->h1E != 4 || rotKeyFlag != 0);
    if (localData->ks.k4 != 0) {
        sefNoiseNextVector(&rotVec);
        sefVecMul(&rotVec, &rotVec, &localData->ks.k4[-1].v);
        part->f198 = rotVec.z;
        if (hasRotation && sefVecNonZero(&rotVec)) {
            MMathRotateMatrixYXZ(&rotMatrix, 0, &rotVec);
            xglMatrix2Quaternion(&part->v40, &rotMatrix);
            part->fl |= 8;
        }
    }
    if (localData->k678 != 0) {
        part->f224 = localData->k678->f[1];
        if (localData->fA8 != 0.0f) {
            part->f178 = sefRandf() * localData->fA8;
        }
    }
    sefVuStoreZero(&part->v60);
    sefVuStoreZero(&part->v80);
    sefVuStoreZero(&part->v90);
    if (localData->k170 != 0) {
        if (localData->k170->h[-1] != 0) {
            sefNoiseNextVector(&part->v50);
            sefVf20Load(&localData->k170[-1].v);
            sefVf20MulBy(&part->v50);
            sefVf20Scale(0.017453292f);
            sefVf20Store(&part->v50);
        }
        if (localData->fE8 & 2) {
            sefVecCopy(&part->v1C0, &localData->v50);
        }
    } else {
        part->v1C0.w = 1.0f;
    }
    if (keys->k14 != 0) {
        if (keys->k14->h[14] == 0x1000) {
            sefVecCopyUnitW(&part->v200, &keys->k14->v);
        }
        if (keys->k14->h[-1] != 0) {
            part->fl |= 0x20;
        }
    }
    if (keys->k4 != 0 && (localData->fE8 & 1)) {
        sefVecCopyUnitW(&part->v1A0, &keys->k4->v);
        sefRotateBlockUpdate(part, hasRotation);
        if (hasRotation) {
            MMathQuatToMatrix(part, &part->v60);
        }
    }
    if (keys->kc != 0) {
        part->Lfw1FC.i = localData->ks.kc->h[-2];
        if (localData->fE8 & 0x40) {
            part->h22A = (short) (int) keys->kc->f[1];
        }
    }
    part->vC0.w = 1.0f;
    sefVuStoreOne(&part->v100);
    keyPtr = keys->k0;
    if (keyPtr != 0) {
        if (keyPtr[-1].f[1] < 0.0f) {
            if (keyPtr[-1].f[0] != 0.0f) {
                part->v100.x = part->v100.y = part->v100.z = 1.0f - sefNoiseNext() * keyPtr[-1].f[0];
            }
        } else {
            AlignedVector4 noiseOffset;
            sefNoiseNextVector(&noiseOffset);
            sefVecMul(&noiseOffset, &noiseOffset, &keyPtr[-1].v);
            sefVecSub(&part->v100, &part->v100, &noiseOffset);
        }
        sefVecScale(&part->v100, &part->v100, keyPtr->f[-1]);
        if (localData->fE8 & 0x10) {
            {
                AlignedVector4 *keyVec = &keys->k0->v;
                sefVecCopyUnitW(&part->v180, keyVec);
                sefVecMul(&part->vC0, &part->v180, &part->v100);
            }
        }
    } else {
        part->v180.w = 1.0f;
    }
    keyBlock = &SEF_REQUEST_KEY_BLOCK;
    {
        int keyMode = 0;
        if (localData->ks.k4 != 0) {
            keyMode = localData->ks.k4->h[-9];
        }
        keyLength = MMathCalcLength(&keyBlock->v);
        if (keyMode == 0x3000 && (localData->h1BE == 4 || (localData->h1BE == 6 && !(localData->w694 & 1))) && !(sefVectorMode & 4)) {
            sefVecScale(&part->vC0, &part->vC0, keyLength);
            sefVecScale(&part->v100, &part->v100, keyLength);
        }
        if (localData->w694 & 0x8000) {
            sefVecScale(&part->vC0, &part->vC0, keyLength);
            sefVecScale(&part->v100, &part->v100, keyLength);
        }
    }
    sefVuStoreOne(&part->v110);
    keyPtr = localData->ks.k8;
    if (keyPtr != 0) {
        if (keyPtr[-1].f[1] < 0.0f) {
            if (keyPtr[-1].f[0] != 0.0f) {
                part->v110.x = part->v110.y = part->v110.z = 1.0f - sefNoiseNext() * keyPtr[-1].f[0];
            }
        } else {
            AlignedVector4 noiseOffset;
            sefNoiseNextVector(&noiseOffset);
            sefVecMul(&noiseOffset, &noiseOffset, &keyPtr[-1].v);
            sefVecSub(&part->v110, &part->v110, &noiseOffset);
        }
        if (localData->fE8 & 8) {
            sefVecToIntColor(part->col, &keys->k8->v, &part->v110, 255.0f);
        }
    }
    return slotIndex;
}

typedef struct SefKeyEntry {
    short *p;
    int v;
} SefKeyEntry;

typedef struct SefKeyPair {
    SefKeyEntry k[2];
} SefKeyPair;

typedef struct SefLinePart {
    unsigned char a[0xF0];
    AlignedVector4 vF0;
    unsigned char b[0x20];
    AlignedVector4 v120;
    unsigned char c[0x10];
    short h140;
    unsigned char d[0x230 - 0x142];
    short tm[8];
    unsigned char e[0x260 - 0x240];
} SefLinePart;

typedef struct SefLinePartPool {
    unsigned char hd[16];
    SefLinePart d;
} SefLinePartPool;

typedef struct SefLineDataView {
    unsigned char a[0x10];
    AlignedVector4 v10;
    unsigned char b[0x10];
    AlignedVector4 v30;
    unsigned char c[0x30];
    AlignedVector4 v70;
    unsigned char d[0x10];
    AlignedVector4 v90;
    unsigned char e[0x10];
    AlignedVector4 vB0;
    unsigned char f[0x10];
    AlignedVector4 vD0;
    unsigned char g[0x142 - 0xE0];
    short n142;
    unsigned char h[0x150 - 0x144];
    float f150;
    unsigned char i[0x160 - 0x154];
    SefKeyPair ks;
    unsigned char j[0x674 - 0x170];
    signed char *k674;
    unsigned char k[0x688 - 0x678];
    float f688;
    int w68C;
    int w690;
    int fl;
    unsigned short u698;
    unsigned short u69A;
    short hh[256];
    unsigned char m[2];
    unsigned short n89E;
    unsigned char n[7];
    unsigned char u8A7;
    signed char b8A8;
    unsigned char o[7];
} SefLineDataView;

typedef struct SefLineCurrentRecord {
    int w0;
    int fl;
} SefLineCurrentRecord;

typedef struct SefStepBlock {
    float f0;
    float f4;
    float f8;
    float fC;
} SefStepBlock;

typedef struct SefLineVectorBits {
    long d;
    int w;
} SefLineVectorBits;

extern SefLinePart *D_005510A4_1FE6F0;
extern int D_005510A8;
extern SefCameraMatrixBlock sefCameraMatrixBlock;
extern unsigned short D_00453D88[];
extern unsigned short D_00453DC8[];

#define SEF_CURRENT_EFFECT_RECORD ((SefLineCurrentRecord *)sefCurrentEffectRecord)
#define SEF_LINE_PART_ALLOC_COUNT (*(int (*))&sefPtAllocCount)
#define SEF_LINE_PART_POOL ((SefLinePartPool *)sefPtAllocatorState)
#define SEF_LINE_DATA_RECORDS ((SefLineDataView *)sefLineDataRecords)
#define SEF_KEYFRAME_STEP_BLOCK (*(SefStepBlock (*))&sefKeyframeStepBlock)

static inline void sefLineVuStoreZero(AlignedVector4 *destination) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void sefLineVecCopy(AlignedVector4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

static inline void sefLineVecAdd(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vadd.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void sefLineVecSub(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vsub.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline void sefLineVecScale(AlignedVector4 *destination, AlignedVector4 *left, float factor) {
    __asm__ __volatile__(
        "lqc2 vf1,0(%4)\n mfc1 $8,%5\n qmtc2.ni $8,vf2\n vmulx.xyz vf1,vf1,vf2x\n sqc2 vf1,0(%3)"
        : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
        : "r"(destination), "r"(left), "f"(factor)
        : "$8");
}

static inline SefLinePart *sefLinePartGet(int handle) {
    SefLinePart *p = 0;
    if (handle < SEF_LINE_PART_ALLOC_COUNT) {
        p = &SEF_LINE_PART_POOL[handle].d;
    }
    return p;
}

#define SEF_KEY_TYPE_MASKED(l,m,i) (((l)->fl&(m))?0:(l)->ks.k[i].p[-2])

static inline int sefLineVecNonZero(SefLineVectorBits *point) {
    return (point->w | point->d) != 0;
}

int sefGetLinkedFlag(int linkedFlagIndex);

/* Emits the key-sampled parts of one local effect slot over its keyframe steps: through the line records when
   sefActiveLineIndex is set, otherwise through the two key blocks. Returns 0 when the slot has no time left, else 1. */
int sefEmitLocalEffectSteps(SefLineDataView *localData, int slotIndex) {
    AlignedVector4 blendVec;
    SefKeyPair *keys = &localData->ks;
    int stepMs;
    int key0Flag;
    int key1Flag;
    int emitted;
    int repeatCount;
    int elapsedMs;
    int totalMs;
    int activeCount;
    int blendOffset;
    int slotByteOffset;
    int i;
    int partKind;
    int j;
    int key0Value;
    int key1Value;
    int saved0Set;
    int saved1Set;
    int savedFlags;
    SefLineDataView *lineData;
    short *key0Params;
    SefLinePart *part;
    unsigned short *partIds;
    AlignedVector4 *spVec0;
    AlignedVector4 *spVec1;
    unsigned short partId;
    float frac;
    float triangle;
    float timeLeft;
    float timeLeftNoLine;
    float ownerSeconds;
    float lineSeconds;
    float timeLeftNext;
    float ageDiff;
    stepMs = (int) ((localData->f150 + 0.0001f) * 1000.0f);
    key0Params = keys->k[0].p;
    key0Flag = key0Params[-2] == 16 || key0Params[-2] == 17;
    key1Flag = keys->k[1].p[-2] == 16 || keys->k[1].p[-2] == 17;
    emitted = 0;
    if (key0Params[-2] & 0x100) {
        SEF_CURRENT_EFFECT_RECORD->fl |= 0x10;
    }
    D_005510A4_1FE6F0 = 0;
    D_005510A8 = -1;
    sefLineVuStoreZero(&localData->v90);
    if (key0Params[-2] == 1 || key0Params[-2] == 2) {
        sefLineVecCopy(&localData->v90, &sefCameraMatrixBlock.v80);
    } else if (key0Params[-2] == 3 || key0Params[-2] == 7) {
        sefLineVuStoreZero(&localData->v90);
    }
    if ((*(int (*)[4]) & D_00454178)[0] & 0x80) {
        return 1;
    }
    if (localData->b8A8 == 0 && localData->w690 >= 0 && localData->w68C - localData->w690 < stepMs) {
        return 0;
    }
    if (stepMs == 0) {
        return 0;
    }
    repeatCount = localData->n142;
    if (!key0Flag) {
        int initialKey0 = SEF_KEY_TYPE_MASKED(localData, 0x28000, 0);
        sefGetPosition(&D_0076BFF0, 0, initialKey0);
    }
    totalMs = (int) ((SEF_KEYFRAME_STEP_BLOCK.fC + 0.0001f) * 1000.0f);
    if (sefActiveLineIndex >= 0) {
        lineData = &SEF_LINE_DATA_RECORDS[sefActiveLineIndex];
        activeCount = lineData->n89E;
        if (lineData == 0) {
            return 1;
        }
        ownerSeconds = (float) localData->w68C / 1000.0f;
        lineSeconds = (float) lineData->w68C / 1000.0f;
        for (i = 0; i < 256 && activeCount > 0; i++) {
            slotByteOffset = slotIndex * 2 + 0x230;
            partKind = lineData->hh[i];
            if (partKind < 0) {
                continue;
            }
            activeCount--;
            D_005510A0 = partKind;
            timeLeft = localData->f688;
            D_005510A4_1FE6F0 = part = sefActiveLineIndex >= 0 ? sefLinePartGet(partKind) : 0;

            ageDiff = lineSeconds - (float) part->h140;
            if (localData->b8A8 == 12) {
                if (ageDiff <= (float) localData->u698) {
                    continue;
                }
                if ((float) localData->u69A < ageDiff) {
                    continue;
                }
                if (*(short *) ((unsigned char *) part + slotByteOffset) >= 0 &&
                    ownerSeconds - (float) *(short *) ((unsigned char *) part + slotByteOffset) < localData->f150) {
                    continue;
                }
            }
            blendOffset = 1.0f < ageDiff;
            for (elapsedMs = 0; elapsedMs < totalMs; elapsedMs += stepMs) {
                frac = (float) elapsedMs / 1000.0f;
                frac -= (float) (int) frac;
                triangle = frac >= 0.0f ? 1.0f - frac : -frac;
                frac = triangle;
                key0Params = keys->k[0].p;
                if (key0Params[-2] == 17) {
                    {
                        int unsetIndex = -1;
                        if (lineData->k674 != 0) {
                            D_005510A8 = ((short *) lineData->k674)[-1];
                        } else {
                            D_005510A8 = unsetIndex;
                        }
                    }
                }
                if (key0Flag) {
                    if (D_005510A4_1FE6F0 != 0) {
                        if (blendOffset && triangle != 0.0f) {
                            sefLineVecScale(&blendVec, &D_005510A4_1FE6F0->vF0, frac);
                            sefLineVecSub(&D_0076BFF0, &D_005510A4_1FE6F0->v120, &blendVec);
                        } else {
                            sefLineVecCopy(&D_0076BFF0, &D_005510A4_1FE6F0->v120);
                        }
                    } else {
                        sefGetPosition(&D_0076BFF0, 0, lineData->ks.k[0].p[-2]);
                        key0Params = keys->k[0].p;
                    }
                }
                sefGetPosition(&localData->v10, &localData->vB0, key0Params[-2]);
                sefLineVecAdd(&localData->v10, &localData->v10, &localData->v70);
                sefGetPosition(&localData->v30, &localData->vD0, keys->k[1].p[-2]);
                if (key0Flag && D_005510A4_1FE6F0 != 0 && blendOffset) {
                    sefLineVecSub(&localData->v10, &localData->v10, &blendVec);
                    sefLineVecSub(&localData->v30, &localData->v30, &blendVec);
                }
                for (j = 0; j < repeatCount; j++) {
                    sefSpawnLocalPart(localData, partKind, triangle);
                }
                if (emitted == 0) {
                    emitted = 1;
                }
                if (localData->b8A8 == 0 && !(localData->u8A7 & 2)) {
                    timeLeftNext = timeLeft - SEF_KEYFRAME_STEP_BLOCK.f8;
                    if (timeLeft <= 0.0f && timeLeftNext <= 0.0f) {
                        break;
                    }
                    timeLeft = timeLeftNext;
                }
            }
            if (emitted && localData->b8A8 == 12) {
                *(short *) ((unsigned char *) D_005510A4_1FE6F0 + slotByteOffset) = (short) (int) ownerSeconds;
            }
        }
    } else if (localData->b8A8 == 0) {
        key0Value = SEF_KEY_TYPE_MASKED(localData, 0x28000, 0);
        key1Value = SEF_KEY_TYPE_MASKED(localData, 0x30000, 1);
        spVec0 = &SEF_SCRATCHPAD(AlignedVector4, 0x40);
        spVec1 = &SEF_SCRATCHPAD(AlignedVector4, 0x50);
        saved0Set = sefLineVecNonZero((SefLineVectorBits *) spVec0);
        saved1Set = sefLineVecNonZero((SefLineVectorBits *) spVec1);
        timeLeftNoLine = localData->f688;
        for (elapsedMs = 0; elapsedMs < totalMs; elapsedMs += stepMs) {
            savedFlags = sefVectorMode;
            frac = (float) elapsedMs / 1000.0f;
            frac -= (float) (int) frac;
            if (frac < 0.0f) {
                frac = -frac;
            }
            if (!key0Flag) {
                if (key0Value == 63) {
                    sefVectorMode = -1;
                }
                if (0.0f < frac && frac < 1.0f) {
                    if (saved0Set) {
                        sefGetPosition(&localData->v10, &localData->vB0, key0Value);
                        func_0025A000(&localData->v10, &SEF_SCRATCHPAD(AlignedVector4, 0x40), &localData->v10, frac);
                    } else {
                        func_0025A000(&blendVec, &SEF_SCRATCHPAD(AlignedVector4, 0x0), &localData->vB0, frac);
                        sefGetPosition(&localData->v10, &blendVec, key0Value);
                    }
                } else {
                    sefGetPosition(&localData->v10, &localData->vB0, key0Value);
                }
                sefLineVecAdd(&localData->v10, &localData->v10, &localData->v70);
            }
            if (!key1Flag) {
                if (key1Value == 63) {
                    sefVectorMode = -1;
                }
                if (0.0f < frac && frac < 1.0f) {
                    if (saved1Set) {
                        sefGetPosition(&localData->v30, &localData->vD0, key1Value);
                        func_0025A000(&localData->v30, &SEF_SCRATCHPAD(AlignedVector4, 0x50), &localData->v30, frac);
                    } else {
                        func_0025A000(&blendVec, &SEF_SCRATCHPAD(AlignedVector4, 0x10), &localData->vD0, frac);
                        sefGetPosition(&localData->v30, &blendVec, key1Value);
                    }
                } else {
                    sefGetPosition(&localData->v30, &localData->vD0, key1Value);
                }
            }
            if (keys->k[0].p[-2] == 0x141 || keys->k[0].p[-2] == 0x142) {
                partIds = keys->k[0].p[-2] == 0x141 ? D_00453D88 : D_00453DC8;
                for (i = 0; partIds[i] != 0; i++) {
                    partId = partIds[i];
                    if (sefGetLinkedFlag(partId - 257)) {
                        sefGetPosition(&D_0076BFF0, 0, partId);
                        sefGetPosition(&localData->v10, &localData->vB0, partId);
                        sefLineVecAdd(&localData->v10, &localData->v10, &localData->v70);
                        for (j = 0; j < repeatCount; j++) {
                            sefSpawnLocalPart(localData, -1, frac);
                        }
                    }
                }
            } else {
                for (i = 0; i < repeatCount; i++) {
                    sefSpawnLocalPart(localData, -1, frac);
                }
            }
            if (key1Value == 63) {
                sefVectorMode = savedFlags;
            }
            if (emitted == 0) {
                emitted = 1;
            }
            if (!(localData->u8A7 & 2)) {
                timeLeftNext = timeLeftNoLine - SEF_KEYFRAME_STEP_BLOCK.f8;
                if (timeLeftNoLine <= 0.0f && timeLeftNext <= 0.0f) {
                    break;
                }
                timeLeftNoLine = timeLeftNext;
            }
        }
    }
    if (emitted) {
        localData->w690 = localData->w68C;
    }
    return 1;
}
/* A 16-bit list of a script entry table. */
#define SEF_SCRIPT_ENTRY(base, table, index) ((short *(*) (int, void *, int)) scGetImmAdrImmIdx2)(base, table, index)

void sefFreeEffectData(SefEffectSlot *effectSlot, unsigned int lineSlot);

typedef struct SefLineKeyPointers {
    short *k0;
    short *k4;
    short *k8;
    short *kc;
    short *k10;
    short *k14;
} SefLineKeyPointers;

typedef struct SefSlotLineRecord {
    unsigned char a[0x10];
    AlignedVector4 v10;
    AlignedVector4 v20;
    AlignedVector4 v30;
    unsigned char b[0x40];
    AlignedVector4 v80;
    AlignedVector4 v90;
    AlignedVector4 vA0;
    AlignedVector4 vB0;
    AlignedVector4 vC0;
    AlignedVector4 vD0;
    unsigned char c[0x20];
    AlignedVector4 v100;
    AlignedVector4 v110;
    AlignedVector4 v120;
    AlignedVector4 v130;
    float f140;
    float f144;
    float f148;
    float f14C;
    unsigned char d[0x10];
    short *k160;
    short *k164;
    short *k168;
    short *k16C;
    short *k170;
    short *k174;
    short *k178;
    unsigned char f[0x660 - 0x17C];
    SefLineKeyPointers ks;
    unsigned char g[8];
    float speed;
    float unk_684;
    float lifeTime;
    int elapsedMs;
    int w690;
    int lineFlags;
    int w698;
    short hh[256];
    unsigned char h[4];
    unsigned short unk_8A0;
    short unk_8A2;
    unsigned char i[2];
    signed char b8A6;
    unsigned char stateFlags;
    signed char b8A8;
    unsigned char j[7];
} SefSlotLineRecord;

typedef struct SefKeyframeStep {
    float f0;
    float f4;
    float stepSeconds;
    float prevStepSeconds;
} SefKeyframeStep;

typedef struct SefRequestKeyBlockLine {
    unsigned char a[0x2B0];
    SefSlotLineRecord *w2B0;
    int w2B4;
    int w2B8;
} SefRequestKeyBlockLine;

typedef struct SefDrawState {
    int drawFlags;
    float speedStep;
    float percentScale;
    float timeOffset;
    float unk_10;
    short keyMatched;
    short keyValue;
    short *keyTrack;
} SefDrawState;

typedef struct SefItemDraw {
    unsigned char a[0x44];
    float f44;
    float f48;
    float f4C;
    float f50;
    short h54;
} SefItemDraw;

typedef struct SefCurrentRecordTail {
    unsigned char a[0x58];
    int w58;
    int w5C;
} SefCurrentRecordTail;

extern int D_00551040;
extern int svInitItemDrawKind(SefItemDraw*,SefSlotLineRecord*,int,int);

#define SEF_KEYFRAME_STEP_LG (*(SefKeyframeStep (*))&sefKeyframeStepBlock)
#define SEF_REQUEST_KEYS_LG (*(SefRequestKeyBlockLine (*))&sefRequestKeyBlock)
#define SEF_LINE_RECORDS_LG ((SefSlotLineRecord *)sefLineDataRecords)
#define SEF_CURRENT_RECORD_LG ((SefCurrentRecordTail *)sefCurrentEffectRecord)
#define SEF_DRAW_STATE_LG ((SefDrawState *)&sefMotionFloorCurve)
#define SEF_FREE_EFFECT_DATA ((void(*)(short*,int))sefFreeEffectData)
#define SEF_SAMPLE_KEYFRAME_VALUE ((void(*)(short*,float*))sefSampleKeyframeValue)
#define SEF_SAMPLE_KEYFRAME_TRACK ((void(*)(short*,float*))sefSampleKeyframeTrack)

static inline void SefVectorZero(AlignedVector4 *destination) {
    __asm__ __volatile__("sqc2 vf0,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void SefVectorCopy(AlignedVector4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

static inline void SefVectorSub(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vsub.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

static inline unsigned char SefLineStateBit0(SefSlotLineRecord *list) {
    return list->stateFlags & 1;
}

static inline short SefKeyTypeOf(short **factor) {
    return (*factor)[-2];
}

extern SefGlobalTransform gSdvCameraPartTransform;

/* Advances one line record of effect slot tb (lineSlot is its index in the slot): ages its life timer and
 * frees it, with the lines linked to it, when the timer runs out (returns 0). Otherwise it refreshes the
 * draw state and the sampled keyframe tracks and returns 1. */
int sefUpdateLineRecord(short *lineHandles, SefSlotLineRecord *line, int unusedArg, int lineSlot, int mode) {

    AlignedVector4 basePoint;
    int done = 0, i, j, pointKind;
    float lifeDelta;
    SefSlotLineRecord *linkedLine;
    short *stateKey;
    SefItemDraw *itemDraw;
    SEF_KEYFRAME_STEP_LG.prevStepSeconds = SEF_KEYFRAME_STEP_LG.stepSeconds;
    sefVectorMode = line->lineFlags & 6;
    if (!(line->stateFlags & 2) || (SEF_REQUEST_KEYS_LG.w2B8 & 0x800)) {
        lifeDelta = line->lifeTime - SEF_KEYFRAME_STEP_LG.stepSeconds;
        if (line->lifeTime > 0.0f && lifeDelta <= 0.0f) {
            done = 1;
        }
        line->lifeTime = lifeDelta;
        if (lifeDelta < 0.0f && line->elapsedMs != 0) {
            if (SefLineStateBit0(line)) {
                line->lifeTime = 0.0f;
                SEF_FREE_EFFECT_DATA(lineHandles, lineSlot);
                if (SefLineStateBit0(line)) {
                    for (i = 0; i < 32; i++) {
                        if (i != lineSlot && lineHandles[i] >= 0) {
                            SefSlotLineRecord *linkedLine = &SEF_LINE_RECORDS_LG[lineHandles[i]];
                            if (linkedLine->b8A6 == lineSlot) {
                                SEF_FREE_EFFECT_DATA(lineHandles, i);
                            }
                        }
                    }
                }
                return 0;
            } else if (line->unk_8A0 == 0) {
                line->lifeTime = 0.0f;
                SEF_FREE_EFFECT_DATA(lineHandles, lineSlot);
                for (j = 0; j < 32; j++) {
                    if (j != lineSlot && lineHandles[j] >= 0) {
                        linkedLine = &SEF_LINE_RECORDS_LG[lineHandles[j]];
                        if (linkedLine->b8A6 == lineSlot && linkedLine->b8A8 == 12) {
                            linkedLine->lifeTime = -1.0f;
                            linkedLine->stateFlags &= ~2;
                        }
                    }
                }
                return 0;
            }
        }
        if (line->lifeTime < 0.0f) {
            line->lifeTime = -1.0f;
        }
    }
    SEF_DRAW_STATE_LG->drawFlags = line->lineFlags & 0x100;
    SEF_DRAW_STATE_LG->speedStep = line->speed * SEF_KEYFRAME_STEP_LG.stepSeconds * 0.01f;
    SEF_DRAW_STATE_LG->percentScale = (float) line->unk_8A2 * 0.01f;
    SEF_DRAW_STATE_LG->timeOffset = line->unk_684 + SEF_DRAW_STATE_LG->unk_10;
    SEF_DRAW_STATE_LG->keyTrack = line->k170;
    SEF_DRAW_STATE_LG->keyValue = SEF_DRAW_STATE_LG->keyTrack != 0 ? SEF_DRAW_STATE_LG->keyTrack[-1] : 0;
    SEF_REQUEST_KEYS_LG.w2B0 = (line->lineFlags & 0x40) ? line : 0;
    if (line->lineFlags & 0x4000) {
        D_00551040++;
    }
    SefVectorCopy(&SEF_SCRATCHPAD(AlignedVector4, 0x0), &line->vB0);
    SefVectorCopy(&SEF_SCRATCHPAD(AlignedVector4, 0x10), &line->vD0);
    SefVectorCopy(&SEF_SCRATCHPAD(AlignedVector4, 0x20), &line->v110);
    SefVectorCopy(&SEF_SCRATCHPAD(AlignedVector4, 0x30), &line->v130);
    SefVectorCopy(&SEF_SCRATCHPAD(AlignedVector4, 0x40), &line->v10);
    SefVectorCopy(&SEF_SCRATCHPAD(AlignedVector4, 0x50), &line->v30);
    sefSampleVectorCurveTrack(line->k160, &line->vA0);
    sefSampleVectorCurveTrack(line->k168, &line->vC0);
    sefSampleVectorCurveTrack(line->k164, &line->v100);
    sefSampleVectorCurveTrack(line->k16C, &line->v120);
    SEF_SAMPLE_KEYFRAME_VALUE(line->k174, &line->f140);
    SEF_SAMPLE_KEYFRAME_TRACK(line->k178, &line->f148);
    stateKey = line->ks.k4;
    SEF_DRAW_STATE_LG->keyMatched = 0;
    if (stateKey[-2] == 0x241 && stateKey[-9] == 0x241) {
        SEF_DRAW_STATE_LG->keyMatched = 1;
    }
    if (line->lifeTime >= 0.0f || done) {
        sefEmitLocalEffectSteps(line, lineSlot);
    } else if (line->k160[-2] == 1 || line->k160[-2] == 2) {
        SefVectorCopy(&line->v90, &sefCameraMatrixBlock.v80);
    } else if (line->k160[-2] == 3 || line->k160[-2] == 7) {
        SefVectorZero(&line->v90);
    }
    if (SefKeyTypeOf(&line->k160) >= 0x20) {
        pointKind = (line->lineFlags & 0x28000) ? 0 : line->k160[-2];
        SefVectorCopy(&basePoint, &line->v80);
        sefGetPosition(&line->v80, 0, pointKind);
        SefVectorSub(&line->v90, &line->v80, &basePoint);
    }
    itemDraw = &gSdvCameraPartTransform;
    itemDraw->h54 = 0;
    if (line->lineFlags & 0x2000) {
        svInitItemDrawKind(itemDraw, line, SEF_CURRENT_RECORD_LG->w58, SEF_CURRENT_RECORD_LG->w5C);
        if (itemDraw->h54 != 0) {
            SEF_DRAW_STATE_LG->timeOffset += itemDraw->f44 - itemDraw->f44 / itemDraw->f50;
        }
    }
    SEF_SCRATCHPAD(SefSlotLineRecord *, 0x0) = line;
    line->unk_8A0 = ((int (*)(SefSlotLineRecord *, int, int)) sefEffectObjectTick)(line, mode, lineSlot);
    line->elapsedMs += (int) ((SEF_KEYFRAME_STEP_LG.stepSeconds + 0.0001f) * 1000.0f);
    return 1;
}

void sefInitEffectData(SefEffectSlot *effectSlot, int id, int effectKind)
{
    unsigned int effectFlags;

    memset(effectSlot, -1, 64);
    effectSlot->id = id;
    effectFlags = effectSlot->flags & 0x7FFFFFFF;
    effectSlot->effectKind = effectKind;
    effectSlot->flags = effectFlags;
    effectSlot->lineCount = 0;
}

void sefDestroyEffectData(SefEffectSlot *effectSlot)
{
    int i;

    if (effectSlot->id != 0) {
        for (i = 0; i < 32; i++) {
            sefFreeEffectData(effectSlot, i);
        }
    }
    effectSlot->effectKind = 0;
    effectSlot->id = 0;
    effectSlot->flags = 0;
}

extern s32 D_00551058;
extern s32 sefEffectSlotCount;
extern unsigned char sefEffectSlots[];

/* Finds a free effect slot: scans the slot table circularly from the saved cursor D_00551058,
 * claims the first slot whose id is 0 and moves the cursor past it. Returns -1 when none is free. */
s32 sefFindFreeEffectSlot(void) {
    s32 sweepEnd = -1;
    s32 slot = D_00551058;
    s32 sweepStart = slot;

    while (slot != sweepEnd) {
        if (((SefEffectSlot *) sefEffectSlots)[slot].id == 0) {
            D_00551058 = (slot + 1) % sefEffectSlotCount;
            return slot;
        }
        if (sweepEnd != sweepStart) {
            sweepEnd = sweepStart;
        }
        slot = (slot + 1) % sefEffectSlotCount;
    }
    return -1;
}

void *sefGetEffectSlot(s32 effectIndex);

/* Releases an effect slot: frees its 32 line records, clears its header and moves the free-slot cursor here. */
void sefReleaseEffectSlot(s32 effectIndex) {
    SefEffectSlot *effectSlot;
    s32 lineSlot;

    if (effectIndex < 0 && !(effectIndex < sefEffectSlotCount)) {
        return;
    }
    effectSlot = sefGetEffectSlot(effectIndex);
    if (effectSlot == 0) {
        return;
    }
    for (lineSlot = 0; lineSlot < 32; lineSlot++) {
        sefFreeEffectData(effectSlot, lineSlot);
    }
    effectSlot->flags = 0;
    effectSlot->effectKind = 0;
    D_00551058 = effectIndex;
    effectSlot->id = 0;
}

/* Takes the next free line record for an effect slot (up to 32 per slot) and stores its tag.
 * Returns the record, or 0 when the slot is full or no line record is free. */
SefLocalData *sefAllocEffectData(SefEffectSlot *effectSlot, int tag)
{
    short *lineHandlePtr;
    unsigned int lineSlot;
    int lineHandle;

    lineSlot = effectSlot->lineCount;
    if (lineSlot >= 32) {
        return 0;
    }
    lineHandlePtr = &effectSlot->lineHandles[lineSlot];
    if (*lineHandlePtr >= 0) {
        return 0;
    }
    lineHandle = sefAllocLineData();
    if (lineHandle < 0) {
        return 0;
    }
    *lineHandlePtr = lineHandle;
    effectSlot->lineTags[lineSlot] = tag;
    effectSlot->lineCount++;
    return &sefLineDataRecords[lineHandle];
}

/* Releases line slot lineSlot of an effect slot, if it holds a line record. */
void sefFreeEffectData(SefEffectSlot *effectSlot, unsigned int lineSlot)
{
    short *lineHandlePtr;

    if (lineSlot < 32) {
        lineHandlePtr = &effectSlot->lineHandles[lineSlot];
        if (*lineHandlePtr >= 0) {
            sefFreeLineData(*lineHandlePtr);
            *lineHandlePtr = -1;
        }
    }
}

void sefSetEntryFlagsByKey(int key, int flags);
void sefClearEntryFlagsByKey(int key, int flags);

/* Sets or clears flagBits on the request keys selected by wordMask: bit 0 picks key 0, bit 2 picks key 1.
 * A zero mask applies key 0 only. */
void sefApplyRequestKeyBits(int wordMask, int flagBits, int setBits)
{
    void (*applyBits)(int, int);
    int *requestKeys;

    requestKeys = sefRequestKeyBlock;
    applyBits = setBits ? sefSetEntryFlagsByKey : sefClearEntryFlagsByKey;
    if (wordMask == 0) {
        applyBits(requestKeys[0], flagBits);
        return;
    }
    if (wordMask & 1) {
        applyBits(requestKeys[0], flagBits);
    }
    if (wordMask & 4) {
        applyBits(requestKeys[1], flagBits);
    }
}

typedef struct SefEffectEntryFlagsView {
    unsigned char unmodeled_00[0x04];
    unsigned int flags; /* +0x04 */
    short entryId;      /* +0x08 */
} SefEffectEntryFlagsView;

void sefSetEntryFlagsByKey(int key, int flagBits);

typedef struct SefScriptEvent {
    short unk_00;
    short startTime;   /* +0x02: the time the event becomes due */
    short frame;       /* +0x04 */
    short link;        /* +0x06: index in the slot's lineHandles, -1 for none */
    short tag;         /* +0x08 */
    signed char count; /* +0x0A */
    signed char flags; /* +0x0B */
    signed char linkFlag; /* +0x0C */
    signed char kind;  /* +0x0D */
} SefScriptEvent;

typedef struct SefSlotScript {
    short lineHandles[48];          /* +0x00 */
    SefScriptEvent *events;         /* +0x60 */
    int flags;                      /* +0x64 */
    short startTime;                /* +0x68 */
    signed char eventCursor;        /* +0x6A */
    unsigned char unk_6B;           /* +0x6B */
} SefSlotScript;

typedef struct SefLineView {
    unsigned char a[0x160];
    short *k160;
    unsigned char f[0x694 - 0x164];
    int lineFlags;
    unsigned char h[0x8A4 - 0x698];
    unsigned short h8A4;
    signed char b8A6;
    unsigned char stateFlags;
    signed char b8A8;
    unsigned char j[7];
} SefLineView;

typedef struct SefRequestKeyBlock {
    void *w0;
    void *w4;
    unsigned char a[0x18];
    unsigned char b20;
    unsigned char b[0x2B8 - 0x21];
    int w2B8;
} SefRequestKeyBlock;

typedef struct SefSlotCurrentRecord {
    int w0;
    int w4;
} SefSlotCurrentRecord;

extern void svSceneFlagsClear(int);
extern void sbStudioCameraResetIfFlagged(void);
extern unsigned long gCameraSelectFlags;
extern float gSdvFrameDelta[];
extern int D_004749E8[];

#define SEF_REQUEST_KEYS_FA50 (*(SefRequestKeyBlock (*))&sefRequestKeyBlock)
#define SEF_LINE_RECORDS_FA50 ((SefLineView *)sefLineDataRecords)
#define SEF_CURRENT_RECORD_FA50 ((SefSlotCurrentRecord *)((SefEffectEntryFlagsView *)sefCurrentEffectRecord))
#define SEF_SET_ENTRY_FLAGS_BY_KEY ((void(*)(void*,int))sefSetEntryFlagsByKey)
#define SEF_ALLOC_LINE ((SefLineView *(*)(SefSlotScript*,int))sefAllocEffectData)

static inline SefSlotScript *SefLineSlotAt(int handle) {
    if (handle >= 0 && handle < sefEffectSlotCount) {
        return (SefSlotScript *) sefEffectSlots + handle;
    }
    return 0;
}

static inline SefLineView *SefSlotLineAt(int handle) {
    if (handle < 0) {
        return 0;
    }
    return &SEF_LINE_RECORDS_FA50[handle];
}

extern int gSdvHitReceived;

/* Runs the timed script of the effect slot *slotHandle at time now: starts every script event that is due
 * (allocating a line record for each), advances the slot's line records and frees the slot when no record is
 * left. Returns 1 while the slot still runs, 0 once it has been released. */
int sefRunEffectSlotScript(short *slotHandle, int now, int unusedArg, int mode) {
    SefSlotScript *s;
    int stopRequested;
    int requestHit;
    int elapsed;
    int liveCount;
    int i;
    int lineHandle;
    SefScriptEvent *ev;
    int frame;
    int link;
    int startTime;
    int tag;
    int count;
    int eventFlags;
    int linkFlag;
    int kind;
    short *script;
    short *entry;
    SefLineView *line;
    int entryKind;

    s = SefLineSlotAt(*slotHandle);
    if (s == 0) {
        return 0;
    }
    s->flags &= 0x7FFFFFFF;
    if (s->events == 0) {
        return 0;
    }
    if ((int) ((gCameraSelectFlags >> 2) & 3) == 0 && SEF_REQUEST_KEYS_FA50.b20 == 0 &&
        !(s->flags & 0x40000000) && (float) s->startTime + gSdvFrameDelta[0] * -5.0f <= (float) now && (s->flags & 1)) {
        SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 0x40000000);
        s->flags |= 0x40000000;
    }
    if (now >= s->startTime) {
        stopRequested = SEF_REQUEST_KEYS_FA50.w2B8 & 0x2000000;
        requestHit = SEF_REQUEST_KEYS_FA50.b20;
        if (s->flags != 0) {
            if (s->flags & 0x10) {
                s->flags &= ~0x10;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 0x10);
            } else if (s->flags & 8) {
                s->flags &= ~8;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 8);
            }
            if (s->flags & 4) {
                s->flags &= ~4;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 4);
            }
            if (s->flags & 2) {
                s->flags &= ~2;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 2);
                if (stopRequested) {
                    svSceneFlagsClear(1);
                    SEF_CURRENT_RECORD_FA50->w4 |= 0x40;
                    s->flags &= ~2;
                    SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w0, 2);
                    SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w0, 0x8000000);
                    SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 0x8000000);
                    sbStudioCameraResetIfFlagged();
                }
            }
            if (s->flags & 1) {
                s->flags &= ~1;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 1);
                if (!requestHit) {
                    sefReleaseEffectSlot(*slotHandle);
                    *slotHandle = -1;
                    return 0;
                }
                gSdvHitReceived = 1;
            }
            if (s->flags & 0x10000000) {
                s->flags &= ~0x10000000;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 0x10000000);
            }
            if (s->flags & 0x40) {
                s->flags &= ~0x40;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 0x40);
            }
            if (s->flags & 0x20000000) {
                s->flags &= ~0x20000000;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w4, 0x20000000);
            }
            if (s->flags & 0x20) {
                s->flags &= ~0x20;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w0, 0x20);
            }
            if (s->flags & 0x80) {
                s->flags &= ~0x80;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w0, 0x80);
            } else if (s->flags & 0x100) {
                s->flags &= ~0x100;
                SEF_SET_ENTRY_FLAGS_BY_KEY(SEF_REQUEST_KEYS_FA50.w0, 0x100);
            }
        }
        elapsed = now - s->startTime;
        if ((unsigned int) s->events >= 3) {
            liveCount = 0;
            while (1) {
                ev = &s->events[s->eventCursor];
                link = ev->link;
                startTime = ev->startTime;
                tag = ev->tag;
                count = ev->count;
                eventFlags = ev->flags;
                linkFlag = ev->linkFlag;
                kind = ev->kind;
                frame = ev->frame;
                if (frame == 0) {
                    SEF_CURRENT_RECORD_FA50->w4 |= 0x4000000;
                }
                if (startTime >= 0x1000) {
                    break;
                }
                if (startTime <= elapsed) {
                    if (s->eventCursor >= 32) {
                        break;
                    }
                    script = ((short *(*) (void *, void *, int) ) scGetImmAdrImmIdx2)((*(void **) &D_00551514),
                                                                                  s->events, s->eventCursor * 7);
                    if (script == 0) {
                        break;
                    }
                    if (D_004749E8[0] == 0) {
                        entry = ((short *(*) (void *, void *, int) ) scGetImmAdrImmIdx2)((*(void **) &D_00551514),
                                                                                     script, 0);
                        entry = ((short *(*) (void *, void *, int) ) scGetImmAdrImmIdx2)((*(void **) &D_00551514),
                                                                                     entry, 0);
                        entryKind = entry[-2];
                        if (entryKind == 0x143 || entryKind == 0x243 || entryKind == 0x242) {
                            s->eventCursor++;
                            continue;
                        }
                    }
                    line = SEF_ALLOC_LINE(s, tag);
                    if (line == 0) {
                        break;
                    }
                    sefInitLine(line, startTime, frame, script, link, count, eventFlags, kind);
                    if (linkFlag != 0) {
                        line->lineFlags |= 0x2000;
                        if (link >= 0) {
                            SefLineView *linkedLine = SefSlotLineAt(s->lineHandles[link]);
                            if (linkedLine != 0) {
                                line->h8A4 = linkedLine->k160[-2];
                            }
                        }
                    }
                    if (line->k160[-2] == 0x11 && link >= 0) {
                        SefLineView *linkedLine = SefSlotLineAt(s->lineHandles[link]);
                        if (linkedLine != 0) {
                            linkedLine->lineFlags |= 0x1000;
                        }
                    }
                    continue;
                }
                liveCount = startTime != 0x1000;
                break;
            }
            s->flags |= 0x80000000;
            for (i = 0; i < 32; i++) {
                lineHandle = s->lineHandles[i];
                if (lineHandle >= 0) {
                    line = SEF_LINE_RECORDS_FA50 + lineHandle;
                    link = line->b8A6;
                    if (link >= 0) {
                        link = s->lineHandles[link];
                    }
                    (*(int *) &D_00551074) = lineHandle;
                    sefActiveLineIndex = link;
                    if (sefUpdateLineRecord(s->lineHandles, line, elapsed, i, mode)) {
                        liveCount++;
                    }
                }
            }
            if (liveCount == 0) {
                sefReleaseEffectSlot(*slotHandle);
                *slotHandle = -1;
                return 0;
            }
        } else {
            sefReleaseEffectSlot(*slotHandle);
            *slotHandle = -1;
            return 0;
        }
        return s->events != 0;
    }
    return 1;
}

void sefClearLineDataRecords(void)
{
    memset(sefEffectSlots, 0, 0x6978);
}

/* The record block of this unit (the splat label of main/sef_131 bss that sefLineDataRecords points to). */
extern unsigned char sefLineDataBlock[];

void sefClearLineRecordBlock(void)
{
    memset(sefLineDataBlock, 0, 0x5C4E0);
}

extern int sefEffectRecordCount;

typedef struct SefEffectRecordTail {
    unsigned char unmodeled_00[0xFC];
    signed char unk_FC[2]; /* +0xFC */
    unsigned char unmodeled_FE[0x143 - 0xFE];
    signed char unk_143; /* +0x143 */
    unsigned char unmodeled_144[0x150 - 0x144];
} SefEffectRecordTail;

/* Clears the record table header (4 bytes) and sets the two unknown fields at +0xFC and +0x143 of every record to -1. */
void sefClearEffectRecordTails(void) {
    int i;

    sefMemZero(sefEffectRecords, 4);
    for (i = 0; i < sefEffectRecordCount; i++) {
        ((SefEffectRecordTail *) sefEffectRecords)[i].unk_143 = -1;
        memset(((SefEffectRecordTail *) sefEffectRecords)[i].unk_FC, -1, 2);
    }
}

typedef struct SefAllocRecord {
    s32 a[3];
    s32 flags;
    u8 b[0x40];
    s32 owner;
    u8 c[0xEF];
    s8 unk_143;
    u8 d[12];
} SefAllocRecord;

extern s32 D_00551050;

/* Claims the next free alloc record (owner == 0) for owner id, scanning circularly from D_00551050; returns its index or -1. */
s32 sefClaimAllocRecord(s32 ownerId) {

    s32 end = (D_00551050 + sefEffectRecordCount - 1) % sefEffectRecordCount;
    s32 i = D_00551050;

    while (i != end) {
        i = i % sefEffectRecordCount;
        if (((SefAllocRecord *) sefEffectRecords)[i].owner == 0) {
            ((SefAllocRecord *) sefEffectRecords)[i].owner = ownerId;
            ((SefAllocRecord *) sefEffectRecords)[i].flags = 0;
            ((SefAllocRecord *) sefEffectRecords)[i].unk_143 = -1;
            D_00551050 = (i + 1) % sefEffectRecordCount;
            return i;
        }
        i++;
    }
    return -1;
}

typedef struct SefEffectRecordOwner {
    s32 a;
    s32 flags;
    s16 model;
    u8 u0a[2];
    s32 c;
    u8 u10[0x40];
    s32 used;
    u8 u54[0x1C];
    s32 own;
    u8 u74[0x4C];
    u8 c0[0x24];
    u8 e4[0x18];
    s16 slots[32];
    u8 u13c[8];
    s8 sl[6];
    u8 e[6];
} SefEffectRecordOwner;

typedef struct SvResourceTypeList {
    s32 res;
    u8 u[0x1C];
    s32 k20;
    s32 k24;
    s16 cnt;
    s8 fl;
    u8 p[0x13DC - 0x2B];
} SvResourceTypeList;

typedef struct SbStudioFlags {
    u64 p[2];
    u64 lo : 2;
    u64 mode : 2;
    u64 hi : 60;
} SbStudioFlags;

extern SvResourceTypeList svResourceTypeLists[];
extern SbStudioFlags gSbStudioBlock;
extern void sdvReleaseOwnerSpecials(void*);
extern void sdvDestroyOwnerAlters(s32);
extern void sefReleaseSlotsByKey(s32,s32);
extern void sdvReleaseShadowBlock(void*);
extern void srsReleaseEffectRecordsByResource(s32,s32);
extern SefEffectRecordOwner *sefEffectRecords_2002C8[1];
void sefClearEntryTableSlot(s8 slotNumber);

/* Releases alloc record recordIndex when it is in use: frees its slots and owner data, clears the record and drops one use of its resource type (releasing the resource at the last use). */
void sefReleaseEffectRecord(s32 recordIndex) {

    s32 j;
    SefEffectRecordOwner *records;
    if (recordIndex < 0 || recordIndex >= sefEffectRecordCount) {
        return;
    }
    records = sefEffectRecords_2002C8[0];
    if (records[recordIndex].used == 0) {
        return;
    }
    sdvReleaseOwnerSpecials(records[recordIndex].c0);
    for (j = 0; j < 32; j++) {
        sefReleaseEffectSlot(sefEffectRecords_2002C8[0][recordIndex].slots[j]);
        sefEffectRecords_2002C8[0][recordIndex].slots[j] = -1;
    }
    sdvDestroyOwnerAlters(recordIndex);
    sefReleaseSlotsByKey(recordIndex, 0);
    sdvReleaseShadowBlock(sefEffectRecords_2002C8[0][recordIndex].e4);
    sefEffectRecords_2002C8[0][recordIndex].used = 0;
    sefEffectRecords_2002C8[0][recordIndex].c = 0;
    sefEffectRecords_2002C8[0][recordIndex].flags = 0;
    sefEffectRecords_2002C8[0][recordIndex].own = 0;
    if (gSbStudioBlock.mode == 0) {
        sefClearEntryTableSlot(sefEffectRecords_2002C8[0][recordIndex].sl[0]);
        sefClearEntryTableSlot(sefEffectRecords_2002C8[0][recordIndex].sl[3]);
        sefClearEntryTableSlot(sefEffectRecords_2002C8[0][recordIndex].sl[1]);
        sefClearEntryTableSlot(sefEffectRecords_2002C8[0][recordIndex].sl[4]);
        sefClearEntryTableSlot(sefEffectRecords_2002C8[0][recordIndex].sl[2]);
        sefClearEntryTableSlot(sefEffectRecords_2002C8[0][recordIndex].sl[5]);
        sefEffectRecords_2002C8[0][recordIndex].sl[0] = -1;
        sefEffectRecords_2002C8[0][recordIndex].sl[3] = -1;
        sefEffectRecords_2002C8[0][recordIndex].sl[1] = -1;
        sefEffectRecords_2002C8[0][recordIndex].sl[4] = -1;
        sefEffectRecords_2002C8[0][recordIndex].sl[2] = -1;
        sefEffectRecords_2002C8[0][recordIndex].sl[5] = -1;
    }
    svResourceTypeLists[sefEffectRecords_2002C8[0][recordIndex].model].cnt--;
    if (svResourceTypeLists[sefEffectRecords_2002C8[0][recordIndex].model].cnt <= 0) {
        svResourceTypeLists[sefEffectRecords_2002C8[0][recordIndex].model].cnt = 0;
        if (gSbStudioBlock.mode == 0) {
            if (svResourceTypeLists[sefEffectRecords_2002C8[0][recordIndex].model].fl != 0) {
                srsReleaseEffectRecordsByResource(svResourceTypeLists[sefEffectRecords_2002C8[0][recordIndex].model].res, 1);
            }
        }
    }
}

typedef struct SefScanRecord {
    s32 unmodeled_00;
    s32 flags; /* +0x04: 0x4000000 marks a key hit */
} SefScanRecord;

typedef struct SefScanKey {
    s16 unmodeled_00;
    s16 time; /* +0x02 */
    s16 kind; /* +0x04 */
    s16 unmodeled_06[4];
} SefScanKey;

/* Sets the hit flag 0x4000000 on record when the key list is a hit: a key of kind 0, or a key whose table entry
 * holds 0x2000 at immediate 1. The scan ends at the end key (time 0x1000). */
void sefScanRecordKeysHit(SefScanRecord *record, SefScanKey *keys) {
    s32 j;

    for (j = 0;; j++) {
        s16 kind = keys[j].kind;

        if (keys[j].time >= 0x1000) {
            return;
        }
        if (kind == 0) {
            record->flags |= 0x4000000;
            return;
        }
        {
            s16 *entry = SEF_SCRIPT_ENTRY(D_00551514, keys, j * 7);

            if (entry != 0) {
                if (*SEF_SCRIPT_ENTRY(D_00551514, entry, 1) == 0x2000) {
                    record->flags |= 0x4000000;
                    return;
                }
            }
        }
    }
}

typedef struct SefEffectSlotHeader {
    u8 a[0x60];
    s32 k60;
    s32 k64;
    u8 b[3];
    u8 k6b;
} SefEffectSlotHeader;

typedef struct SefSpawnItem {
    s16 id;
    s16 p2;
    s32 k4;
    s8 k8;
    u8 k9;
    u8 ka;
    u8 kb;
} SefSpawnItem;

typedef struct SefSpawnRecord {
    u8 a[0xFC];
    s16 slots[32];
    u8 b[7];
    u8 k143;
    u8 c[12];
} SefSpawnRecord;

static inline SefEffectSlotHeader *SefSpawnSlotAt(s32 i) {
    if (i >= 0 && i < sefEffectSlotCount) {
        return &((SefEffectSlotHeader *) sefEffectSlots)[i];
    }
    return 0;
}

/* Spawns the items of the spawn list item into record recordIndex: allocates one effect slot per item (up to 32, stopping at an item with id < 0), starts its key list and returns the count spawned. Remaining slot handles are set to -1. */
s32 sefSpawnRecordItems(SefSpawnItem *item, s32 recordIndex, s32 flag) {
    SefSpawnRecord *record = &((SefSpawnRecord *) sefEffectRecords)[recordIndex];
    s16 *slotHandles = record->slots;
    SefSpawnItem *itemTable = item;
    s32 spawnCount = 0;
    s32 spawned;
    s32 slotIndex;
    SefEffectSlotHeader *slotHeader;
    s32 applyK4;
    s32 keyList;
    if (item->id >= 0) {
        do {
            slotIndex = sefFindFreeEffectSlot();
            slotHeader = SefSpawnSlotAt(slotIndex);
            if (slotHeader == 0) {
                break;
            }
            slotHandles[spawnCount] = slotIndex;
            applyK4 = 1;
            item = &itemTable[spawnCount];
            if (spawnCount >= 32 || item->id < 0) {
                break;
            }
            if (item->ka >= 100) {
                keyList = scGetImmAdrImmIdx2(D_00551514, item, 1);
            } else if (item->ka == 0 || MMathRandomNext() % 100 >= item->ka) {
                keyList = 0;
                applyK4 = 0;
            } else {
                keyList = scGetImmAdrImmIdx2(D_00551514, item, 1);
            }
            if (flag != 0) {
                if ((item->k4 & 3) == 0) {
                    keyList = 0;
                }
            }
            sefInitEffectData((SefEffectSlot *) slotHeader, keyList != 0 ? keyList : 1, item->id);
            if (keyList != 0 && gSbStudioBlock.mode == 0) {
                sefScanRecordKeysHit(record, keyList);
            }
            if (applyK4) {
                slotHeader->k64 = item->k4;
            }
            slotHeader->k6b = item->k9;
            if (item->k8 >= 0) {
                ((SefSpawnRecord *) sefEffectRecords)[slotIndex].k143 = itemTable[item->k8].k8;
            }
            spawnCount++;
        } while (item->id >= 0);
    }
    spawned = spawnCount;
    for (; spawnCount < 32; spawnCount++) {
        slotHandles[spawnCount] = -1;
    }
    return spawned;
}

typedef struct SefMatrix4Rows {
    AlignedVector4 r[4];
} SefMatrix4Rows;

typedef struct SefScriptItem {
    short a;
    short b;
    short c;
    short d;
    short e;
    short f;
    short g;
} SefScriptItem;

typedef struct SefSlotItemsView {
    unsigned char a[0x60];
    SefScriptItem *items;
    unsigned char b[8];
} SefSlotItemsView;

typedef struct SefSlotEffectRecordView {
    int w0;
    int fl;
    unsigned char a[0x4C];
    int k54;
    void *m58;
    void *m5c;
    unsigned char b[0xFC - 0x60];
    short slots[32];
    unsigned char c[8];
    signed char b144;
    signed char b145;
    signed char b146;
    signed char b147;
    signed char b148;
    signed char b149;
    unsigned char z[6];
} SefSlotEffectRecordView;

typedef struct SefCameraSelectBits {
    unsigned long lo : 2;
    unsigned long mode : 2;
    unsigned long hi : 60;
} SefCameraSelectBits;

s8 sefClaimTableSlot(void);
s32 sefGetBattlePoint(AlignedVector4 *o, s32 i, s32 t);

typedef struct SefProbeLinked {
    unsigned char unmodeled_00[0x40];
    Matrix4 view;
} SefProbeLinked;

typedef struct SefProbeSourceView {
    unsigned char unmodeled_000[0xB0];
    Matrix4 cameraView;
    unsigned char unmodeled_0F0[0x560 - 0xF0];
    unsigned char packageNode[4]; /* +0x560: the node package the point lookup reads */
    unsigned char unmodeled_564[0x650 - 0x564];
    SefProbeLinked *linked;
} SefProbeSourceView;

int sefReadProbePoint(SefProbeSourceView *source, int n, Vector4 *out, int w);

#define SEF_SLOT_ITEMS_TABLE ((SefSlotItemsView *)sefEffectSlots)
#define SEF_MATRIX_BLOCK_AT ((SefMatrix4Rows *(*)(int))sefGetActiveTableSlot)
#define SEF_SET_SLOT_DIRECTION_BETWEEN ((int(*)(SefMatrix4Rows*,AlignedVector4*,AlignedVector4*,AlignedVector4*))sefSetSlotDirectionBetween)

static inline SefSlotItemsView *SefSlotItemsAt(int i) {
    extern int sefEffectSlotCount;
    if (i >= 0 && i < sefEffectSlotCount) {
        return &SEF_SLOT_ITEMS_TABLE[i];
    }
    return 0;
}

static inline void PbCp(AlignedVector4 *destination, AlignedVector4 *source) {
    __asm__ __volatile__("lq $8,0(%5)\n sq $8,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination), "r"(source)
                         : "$8");
}

void sefSetMatrixOrUnit(void *destinationMatrix, const void *sourceMatrix);
int sefActorMulLocalMatrix(XenoActor *actor, void *actorNodeArg, Matrix4 *combinedMatrix, int multiplyLocal);

const Vector4 sefDefaultPoint = { 0.0f, 0.0f, 0.0f, 1.0f };
const Vector4 sefDefaultAxis[2] = {
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 0.0f, -1.0f, 1.0f }
};
const DoublewordAlignedVector4 D_0053EF10 = { 1.0f, 1.0f, 0.0f, 0.0f };
const DoublewordAlignedVector4 D_0053EF20 = { 1.0f, -1.0f, 0.0f, 0.0f };


/* Updates the hit flags and matrices of effect record recordIndex from the slot items of its 32 slots: one-bit flags
 * per key kind, then the slot matrices, direction and scale (not run when the camera select mode is set or the record is flagged 0x1000). */
void sefUpdateRecordHitFlagsAndMatrices(int recordIndex) {

    int func_00200878(int kind) {
        int flagBits = 0;
        switch (kind) {
        case 0x3B:
            flagBits = 1;
            break;
        case 0x3D:
            flagBits = 2;
            break;
        case 0x40:
        case 0x41:
            flagBits = 0x2E;
            break;
        case 0x3E:
            flagBits = 0x10;
            break;
        case 0x3F:
            flagBits = 0x20;
            break;
        }
        return flagBits;
    }
    unsigned char hitFlags = 0;
    int slotIndex;
    int keyIndex;
    SefSlotItemsView *slotItems;
    AlignedVector4 subtrahend;
    AlignedVector4 minuend;
    if ((*(SefCameraSelectBits *) &gCameraSelectFlags).mode != 0) {
        return;
    }
    if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].fl & 0x1000) {
        return;
    }
    for (slotIndex = 0; slotIndex < 32; slotIndex++) {
        slotItems = SefSlotItemsAt(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].slots[slotIndex]);
        if (slotItems != 0 && (unsigned int) slotItems->items >= 3) {
            for (keyIndex = 0; slotItems != 0 && slotItems->items[keyIndex].b < 0x1000; keyIndex++) {
                short *keyEntry1 = ((short *(*) (int, void *, int) ) scGetImmAdrImmIdx2)(
                    ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].k54, slotItems->items, keyIndex * 7);
                if (keyEntry1 != 0) {
                    short *keyEntry2 = ((short *(*) (int, void *, int) ) scGetImmAdrImmIdx2)(
                        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].k54, keyEntry1, 0);
                    short *keyEntry3 = ((short *(*) (int, void *, int) ) scGetImmAdrImmIdx2)(
                        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].k54, keyEntry1, 1);
                    short *keyEntry4 = ((short *(*) (int, void *, int) ) scGetImmAdrImmIdx2)(
                        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].k54, keyEntry2, 0);
                    short *keyEntry5 = ((short *(*) (int, void *, int) ) scGetImmAdrImmIdx2)(
                        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].k54, keyEntry2, 2);
                    short *keyEntry6 = ((short *(*) (int, void *, int) ) scGetImmAdrImmIdx2)(
                        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].k54, keyEntry3, 5);
                    hitFlags |= func_00200878(keyEntry4[-2]);
                    hitFlags |= func_00200878(keyEntry5[-2]);
                    hitFlags |= func_00200878(keyEntry6[-2]);
                    hitFlags |= func_00200878(keyEntry6[-9]);
                    if (hitFlags == 0x3F) {
                        break;
                    }
                }
            }
        }
        if (hitFlags == 0x3F) {
            break;
        }
    }
    if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m58 != 0 && (hitFlags & 0x11)) {
        SefMatrix4Rows *m;
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b144 = sefClaimTableSlot();
        m = SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b144);
        if (m != 0) {
            sefSetMatrixOrUnit(m, ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m58);
        }
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b147 = sefClaimTableSlot();
        m = SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b147);
        if (m != 0) {
            sefActorMulLocalMatrix(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m58,
                                                                         3, m, 1);
        }
    } else {
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b144 = -1;
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b147 = -1;
    }
    if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c != 0 && (hitFlags & 0x22)) {
        SefMatrix4Rows *m;
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b145 = sefClaimTableSlot();
        m = SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b145);
        if (m != 0) {
            sefSetMatrixOrUnit(m, ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c);
        }
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b148 = sefClaimTableSlot();
        m = SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b148);
        if (m != 0) {
            sefActorMulLocalMatrix(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c,
                                                                         3, m, 1);
        }
    } else {
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b145 = -1;
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b148 = -1;
    }
    if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m58 != 0 && ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c != 0 && (hitFlags & 0xC)) {
        SefMatrix4Rows *m;
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b146 = sefClaimTableSlot();
        m = SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b146);
        if (m != 0) {
            sefGetBattlePoint(&minuend, 1, 1);
            sefGetBattlePoint(&subtrahend, 1, 0);
            subtrahend.y = 0.0f;
            minuend.y = 0.0f;
            SEF_SET_SLOT_DIRECTION_BETWEEN(m, &minuend, &subtrahend, &sefDefaultAxis);
            PbCp(&m->r[3],
                 (AlignedVector4 *) ((unsigned char *) SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b145) +
                                     0x30));
        } else {
            ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b146 = -1;
        }
    } else if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c != 0 && (hitFlags & 0xC)) {
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b146 = ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b145;
    } else {
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b146 = -1;
    }
    if (hitFlags != 0) {
        SefMatrix4Rows *m;
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b149 = sefClaimTableSlot();
        m = SEF_MATRIX_BLOCK_AT(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b149);
        if (m != 0) {
            if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m58 != 0) {
                sefReadProbePoint(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m58, 3, &m->r[0], 0);
            }
            if (((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c != 0) {
                float entryScale;
                sefReadProbePoint(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c, 3, &m->r[1], 0);
                entryScale = sefGetEntryScale(((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].m5c);
                ((int *) &m->r[2])[0] = 0;
                ((int *) &m->r[2])[1] = 0;
                m->r[2].z = entryScale * 0.1f;
                ((float *) &m->r[2])[3] = 1.0f;
            }
        } else {
            ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b149 = -1;
        }
    } else {
        ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].b149 = -1;
    }
    ((SefSlotEffectRecordView *) sefEffectRecords)[recordIndex].fl |= 0x1000;
}

typedef struct SefSlotBlock {
    unsigned char unmodeled_00[0xF0];
    short slotHandles[32]; /* +0xF0 of the block (+0xFC of the owner) */
} SefSlotBlock;

typedef struct SefScheduleSlots {
    unsigned char unmodeled_00[0x0C];
    SefSlotBlock block; /* +0x0C */
} SefScheduleSlots;

static inline SefEffectSlot *sefEffectSlotFor(int index) {
    if ((index >= 0) && (index < sefEffectSlotCount)) {
        return (SefEffectSlot *) (sefEffectSlots + (index * 0x6C));
    }
    return 0;
}

int func_00200FC0(SefScheduleSlots *schedule) {
    SefSlotBlock *block;
    int count;
    int bestIndex;
    int i;

    count = 0;
    if (schedule != 0) {
        bestIndex = -1;
        block = &schedule->block;
        for (i = 0; i < 32; i++) {
            SefEffectSlot *slot = sefEffectSlotFor(block->slotHandles[i]);
            if ((slot != 0) && (slot->id != 0) && ((slot->flags & 1) != 0)) {
                count++;
                bestIndex = i;
            }
        }
        if (bestIndex >= 0) {
            SefEffectSlot *slot = sefEffectSlotFor(block->slotHandles[bestIndex]);
            if (slot != 0) {
                slot->flags |= 0x10000000;
            }
        }
    }
    return count;
}

typedef struct SefScheduleOwnerBlock {
    int kind;     /* +0x00: the effect kind */
    float coordB; /* +0x04 */
    unsigned char unmodeled_08[4];
} SefScheduleOwnerBlock;

typedef struct SefScheduleRequest {
    unsigned int word[6];     /* +0x00 */
    SefScheduleOwnerBlock block; /* +0x18 */
} SefScheduleRequest;

typedef struct SefScheduleRecord {
    unsigned int unmodeled_00;              /* +0x00 */
    unsigned int flags;                /* +0x04: 0x2000000 when the slots are loaded */
    short entryId;                     /* +0x08 */
    short gate;                        /* +0x0A */
    float coordA;                      /* +0x0C */
    Vector4 offsetCopy;                /* +0x10 */
    Vector4 rotation;                  /* +0x20 */
    Vector4 scale;                     /* +0x30 */
    Vector4 vertexScale;               /* +0x40 */
    unsigned short *inUse;             /* +0x50 */
    int loadTag;                       /* +0x54 */
    unsigned int requestWord[6];       /* +0x58 */
    SefScheduleOwnerBlock ownerBlock;     /* +0x70 */
    SdvSequence effectSeq;             /* +0x7C */
    SdvSequence alter;                 /* +0x88 */
    unsigned char camera[0xC0 - 0x94]; /* +0x94 */
    SdvSequence special;               /* +0xC0 */
    unsigned char unmodeled_CC[0xD8 - 0xCC];
    SdvSequence wind;     /* +0xD8 */
    SdvSequence tail;     /* +0xE4 */
    unsigned int unmodeled_F0; /* +0xF0 */
    float unmodeled_F4;        /* +0xF4 */
    float cullRadius;     /* +0xF8 */
    unsigned char unmodeled_FC[0x13C - 0xFC];
    short execSteps; /* +0x13C */
    short slotIndex; /* +0x13E */
    unsigned short scalePercent; /* +0x140: scale in 0.01 units, 0 = use the entry scale */
    signed char delayFrames;      /* +0x142 */
    signed char linkValue; /* +0x143 */
    unsigned char unmodeled_144[0x14A - 0x144];
    unsigned char liveSlotCount; /* +0x14A */
    unsigned char unmodeled_14B[0x150 - 0x14B];
} SefScheduleRecord;

#define SEF_RECORD_AT(index) ((SefScheduleRecord *) ((index) * sizeof(SefScheduleRecord) + (int) sefEffectRecords))

extern short gSefExecSteps;
int svFindEffectMapperIndex(int kind);
int sdvSelectCamera(void *camera, int loadTag, unsigned short *cameraValue, void *request);

static inline void sefCopyQuad(Vector4 *destination, const Vector4 *source) {
    __asm__ __volatile__("lq $8,0(%4)\n\tsq $8,0(%5)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(source), "r"(destination)
                         : "$8");
}

static inline void sefZeroQuad(Vector4 *destination) {
    __asm__ __volatile__("sq $0,0(%4)" : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w) : "r"(destination));
}

static inline void sefSetUnitVector(Vector4 *destination) {
    __asm__ __volatile__("vmaxw.xyzw $vf1,$vf0,$vf0w\n\tsqc2 $vf1,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination));
}

static inline void sefBindHandle(SdvSequence *handle, int address, int kind) {
    handle->stride = kind;
    handle->keys = (short *) address;
}

int sefScheduleEffectRecord(unsigned short *table, SefScheduleRequest *request, const Vector4 *offset, int loadTag, int delayFrames, int reuseIndex, int flag, int loadFlags);

/* Schedules one effect record for loadTag: binds its handles from the table, reuses reuseIndex
 * or takes a free record, and resets its state. Returns the record index, or -1. */
int sefScheduleEffectRecord(unsigned short *table, SefScheduleRequest *request, const Vector4 *offset, int loadTag,
                  int delayFrames, int reuseIndex, int flag, int loadFlags) {
    unsigned char *savedLoadTag;
    int windHandle;
    int cameraHandle;
    int specialHandle;
    int alterHandle;
    int effectSeqHandle;
    int tailHandle;
    const Vector4 *source;
    SefScheduleOwnerBlock *block;
    SefScheduleRecord *record;
    int recordIndex;
    int mapperIndex;

    windHandle = scGetImmAdrImmIdx2(loadTag, table, 0);
    cameraHandle = scGetImmAdrImmIdx2(loadTag, table, 1);
    specialHandle = scGetImmAdrImmIdx2(loadTag, table, 2);
    alterHandle = scGetImmAdrImmIdx2(loadTag, table, 3);
    effectSeqHandle = scGetImmAdrImmIdx2(loadTag, table, -1);
    tailHandle = scGetImmAdrImmIdx2(loadTag, table, 5);
    table += 6;
    if (reuseIndex >= 0) {
        SefScheduleRecord *reused;

        recordIndex = reuseIndex;
        reused = SEF_RECORD_AT(recordIndex);
        reused->linkValue = -1;
        reused->inUse = table;
        reused->coordA = 0.0f;
    } else {
        recordIndex = sefClaimAllocRecord(table);
    }
    if (recordIndex < 0) {
        return -1;
    }
    block = &request->block;
    savedLoadTag = *(unsigned char **) &D_00551514;
    D_00551514 = loadTag;
    SEF_RECORD_AT(recordIndex)->ownerBlock = *block;
    record = SEF_RECORD_AT(recordIndex);
    record->requestWord[0] = request->word[0];
    record->requestWord[1] = request->word[1];
    record->requestWord[2] = request->word[2];
    record->requestWord[3] = request->word[3];
    record->requestWord[4] = request->word[4];
    record->requestWord[5] = request->word[5];
    if (record->ownerBlock.coordB == 0.0f) {
        record->ownerBlock.coordB = 1.0f;
    }
    source = offset;
    if (source == 0) {
        source = &sefDefaultPoint;
    }
    sefCopyQuad(&record->offsetCopy, source);
    sefZeroQuad(&record->rotation);
    record->delayFrames = delayFrames;
    record->loadTag = loadTag;
    SEF_RECORD_AT(recordIndex)->flags = flag ? 0x2000000 : 0;
    SEF_RECORD_AT(recordIndex)->unmodeled_F4 = 0.0f;
    SEF_RECORD_AT(recordIndex)->execSteps = gSefExecSteps;
    SEF_RECORD_AT(recordIndex)->unmodeled_00 = 0;
    SEF_RECORD_AT(recordIndex)->slotIndex = recordIndex;
    sefSetUnitVector(&SEF_RECORD_AT(recordIndex)->vertexScale);
    memset(SEF_RECORD_AT(recordIndex)->camera, 0, 44);
    memset(&SEF_RECORD_AT(recordIndex)->special, 0, 24);
    memset(&SEF_RECORD_AT(recordIndex)->alter, 0, 12);
    memset(&SEF_RECORD_AT(recordIndex)->effectSeq, 0, 12);
    memset(&SEF_RECORD_AT(recordIndex)->wind, 0, 12);
    memset(&SEF_RECORD_AT(recordIndex)->tail, 0, 16);
    memset(&((SefScheduleRecord *) sefEffectRecords)[recordIndex].unmodeled_F0, -1, 4);
    sefBindHandle(&SEF_RECORD_AT(recordIndex)->effectSeq, effectSeqHandle, 10);
    sefBindHandle(&SEF_RECORD_AT(recordIndex)->alter, alterHandle, 32);
    sefBindHandle(&SEF_RECORD_AT(recordIndex)->special, specialHandle, 2);
    sefBindHandle(&SEF_RECORD_AT(recordIndex)->wind, windHandle, 11);
    sefBindHandle(&SEF_RECORD_AT(recordIndex)->tail, tailHandle, 0);
    sdvSelectCamera(((SefScheduleRecord *) sefEffectRecords)[recordIndex].camera, loadTag,
                    (unsigned short *) cameraHandle, request);
    sefSetUnitVector(&SEF_RECORD_AT(recordIndex)->scale);
    SEF_RECORD_AT(recordIndex)->gate = 1;
    SEF_RECORD_AT(recordIndex)->cullRadius = 8.0f;
    if (sefSpawnRecordItems(table, recordIndex, loadFlags) == 0) {
        SEF_RECORD_AT(recordIndex)->flags &= ~0x2000000;
    }
    *(unsigned char **) &D_00551514 = savedLoadTag;
    mapperIndex = svFindEffectMapperIndex(block->kind);
    SEF_RECORD_AT(recordIndex)->entryId = mapperIndex;
    SEF_RECORD_AT(recordIndex)->liveSlotCount = func_00200FC0(SEF_RECORD_AT(recordIndex));
    return recordIndex;
}

int sefCreateScheduler(int tableId, int scheduleRequest, int scheduleOffset, int loadTag, int reuseIndex)
{
    return sefScheduleEffectRecord(tableId, scheduleRequest, scheduleOffset, loadTag, 0, reuseIndex, 1, 0);
}

/* Object view written by the jitter: the offset vector and the jitter amount. */
typedef struct SefJitterTarget {
    unsigned char unmodeled_00[0x10];
    AlignedVector4 offset;       /* +0x10 */
    unsigned char unmodeled_20[0x120];
    unsigned short jitterAmount; /* +0x140 */
} SefJitterTarget;


extern f32 MMathMakeRandomSigned(void);

static inline void sefSubVec3Memory(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%3)\n\tlqc2 vf2,0(%4)\n\tvsub.xyz vf1,vf1,vf2\n\tsqc2 vf1,0(%5)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(left), "r"(right), "r"(destination)
                         : "memory");
}

void sefApplyMatrixVector(Vector4 *transformedVector, Matrix4 *matrix, Vector4 *vector);

/* The request state of one effect record: the copied key dwords, the kind word
 * (low byte: record kind, high byte 2: the jitter amount is re-rolled) and the
 * table mask. The same object is the timer source read by func_00201508. */
typedef struct SefRequestState {
    unsigned char unmodeled_00[0x58];
    unsigned long long copy58; /* +0x58: first dword copied to the frame */
    unsigned long long copy60; /* +0x60 */
    unsigned long long copy68; /* +0x68 */
    unsigned long long copy70; /* +0x70 */
    unsigned short kindWord;   /* +0x78: low byte selects the record kind */
    unsigned short mask;       /* +0x7A: 12-bit table mask */
    unsigned char unmodeled_7C[0x140 - 0x7C];
    unsigned short amplitude;  /* +0x140: jitter amplitude */
    unsigned char unmodeled_142[8];
    unsigned char pointCount;  /* +0x14A: two or more points jitter the offset */
} SefRequestState;

void func_00201508(SefRequestState *source, SefJitterTarget *target, s32 pathId) {
    AlignedVector4 delta;
    AlignedVector4 pointA;
    AlignedVector4 pointB;

    if (target != 0) {
        if (source->pointCount >= 2) {
            sefReadProbePoint(pathId, 2, &pointA, 0);
            sefReadProbePoint(pathId, 3, &pointB, 0);
            sefSubVec3Memory(&delta, &pointA, &pointB);
            delta.x = sefNoiseNext() * source->amplitude * 0.01f;
            delta.y *= MMathMakeRandomSigned();
            delta.z = sefRandf() * source->amplitude * 0.01f;
            sefApplyMatrixVector(&target->offset, pathId + 0xB0, &delta);
        }
        if ((source->kindWord >> 8) == 2) {

            *(s32 *) &target->offset.z = 0;
            target->jitterAmount = (u16) ((sefRandf() * 0.1f + 0.9f) * source->amplitude);
        }
    }
}


typedef union SefRequestWords {
    unsigned long long dword;
    struct {
        unsigned int low;
        unsigned int high;
    } word;
} SefRequestWords;

typedef struct SefRequestFrame {
    SefRequestWords first;     /* +0 */
    unsigned long long second; /* +8 */
    unsigned long long third;  /* +16 */
    SefRequestWords key;       /* +24: key passed to sefCreateScheduledRequest */
    union {
        unsigned int word;
        unsigned short half[2];
    } tail; /* +32 */
} SefRequestFrame;

extern unsigned int D_00453DF0[5];

typedef struct PkRec {
    s32 a;
    s32 fl;
    u8 z[0x148];
} PkRec;

typedef struct PkKey {
    s32 w[6];
    s32 key;
    s32 x[5];
} PkKey;

PkRec *sefCreateScheduledRequest(PkKey *q);

/* Runs the effect callbacks of one request state. Kind 2 states (high byte of the
 * kind word is 2) use one fixed key: 0x136006 for kind 1, 0x136007 for kind 3.
 * Other states run func_00201508 for every set bit of the 12-bit mask, with the
 * keys of D_00453DF0. */
void func_00201690(SefRequestState *state) {
    SefRequestFrame frame;
    unsigned int kindLow;
    unsigned int mask;
    unsigned int bit;
    int index;

    kindLow = (unsigned char) state->kindWord;
    if (kindLow == 0) {
        return;
    }
    mask = state->mask & 0xfff;
    memset(&frame, 0, 48);
    frame.first.dword = state->copy58;
    frame.second = state->copy60;
    frame.third = state->copy68;
    frame.key.dword = state->copy70;
    frame.tail.word = *(unsigned int *) &state->kindWord;
    frame.tail.half[0] = 0;
    frame.tail.half[1] = 0;
    if (frame.first.word.high == 0) {
        frame.first.word.high = frame.first.word.low;
    }
    if ((state->kindWord >> 8) == 2) {
        if (kindLow == 1) {
            frame.key.word.low = 0x136006;
        } else if (kindLow == 3) {
            frame.key.word.low = 0x136007;
        } else {
            return;
        }
        func_00201508(state, (SefJitterTarget *) sefCreateScheduledRequest((PkKey *) &frame), frame.first.word.high);
        return;
    }
    if (mask == 0) {
        return;
    }
    bit = 1;
    for (index = 0; index < 5; index++) {
        if ((mask & bit) != 0) {
            frame.key.word.low = D_00453DF0[index];
            func_00201508(state, (SefJitterTarget *) sefCreateScheduledRequest((PkKey *) &frame), frame.first.word.high);
        }
        bit <<= 1;
    }
}

/* Returns 1 when the camera select bits (bits 2-3 of gCameraSelectFlags) are 2,
 * the mode that reads the probe view from the linked camera. */
int sefCameraSelectIsLinked(void) {
    int cameraSelect = ((*(unsigned long long (*)[]) & gCameraSelectFlags)[0] >> 2) & 3;

    return cameraSelect == 2;
}

typedef struct SefProbeSource {
    unsigned char unmodeled_000[0xB0];
    Matrix4 cameraView;
    unsigned char unmodeled_0F0[0x650 - 0xF0];
    SefProbeLinked *linked;
} SefProbeSource;

int sefCopyProbeView(Matrix4 *destination, const SefProbeSource *source);

/* Copies the probe view into destination: the linked camera view when the camera
 * select is 2 and a link exists, else the source camera view. Returns 1 when linked. */
int sefCopyProbeView(Matrix4 *destination, const SefProbeSource *source) {
    Matrix4 *view;
    int found;

    found = 0;
    view = &source->cameraView;
    if (sefCameraSelectIsLinked() != 0 && source->linked != 0) {
        view = &source->linked->view;
        found = 1;
    }
    if (destination != 0) {
        QuadwordCopyMatrixT0(destination, view);
    }
    return found;
}

extern const Matrix4 MMathUnitMatrix;

void sefSetMatrixOrUnit(void *destinationMatrix, const void *sourceMatrix) {
    if (sourceMatrix != 0) {
        sefCopyProbeView(destinationMatrix, sourceMatrix);
        return;
    }
    __asm__ __volatile__("lqc2 vf1, 32(%0)" : : "r"(MMathUnitMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf2, 16(%0)" : : "r"(MMathUnitMatrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 0(%0)" : : "r"(MMathUnitMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf0, 48(%0)" : : "r"(destinationMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf1, 32(%0)" : : "r"(destinationMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf2, 16(%0)" : : "r"(destinationMatrix) : "memory");
    __asm__ __volatile__("sqc2 vf3, 0(%0)" : : "r"(destinationMatrix) : "memory");
}

typedef struct SefRequestKeyView {
    struct XenoActor *source;    /* +0x000 */
    struct XenoActor *target;    /* +0x004 */
    struct XenoActor *actors[4]; /* +0x008: a2, a3, a4, a5 */
    unsigned char unmodeled_018[0x70 - 0x18];
    float kind256Matrix[4][4];    /* +0x070 */
    float kind512Matrix[4][4];    /* +0x0B0 */
    float actorMatrices[4][4][4]; /* +0x0F0: one matrix per actor a2..a5, 0x40 apart */
    float directionMatrix[4][4];  /* +0x1F0: set from the slot direction, row 3 from kind512 */
    unsigned char unmodeled_230[0x2C8 - 0x230];
    int objectOffset;       /* +0x2C8 */
    int kindTableOffset[5]; /* +0x2CC: tables of the kinds 0, 2, 3, 4 and 6 */
} SefRequestKeyView;

#define SEF_KEY_VIEW ((SefRequestKeyView *) sefRequestKeyBlock)

extern unsigned long long gSbStudioBlock_2018E8[];
extern f32 D_00475388[2];
SefEntry *sefFindEntryById(int key);

/* Fills the request key block (SEF_KEY_VIEW) from the source, the target and four
 * actors: object ids, table offsets, matrices and the direction matrix. The studio
 * mode (bits 2-3 of gSbStudioBlock_2018E8[2]) decides whether actors 2 and 3 count.
 * Sets the scale D_00475388[0]. */
void func_002018E8(struct XenoActor *source, struct XenoActor *target, struct XenoActor *actor0,
                   struct XenoActor *actor1, struct XenoActor *actor2, struct XenoActor *actor3) {
    Vector4 subtrahend;
    Vector4 minuend;

    SEF_KEY_VIEW->source = source;
    SEF_KEY_VIEW->target = target;
    SEF_KEY_VIEW->actors[0] = actor0;
    SEF_KEY_VIEW->actors[1] = actor1;
    if (((int) ((gSbStudioBlock_2018E8[2] >> 2) & 3)) == 0) {
        SEF_KEY_VIEW->actors[2] = actor2;
        SEF_KEY_VIEW->actors[3] = actor3;
    } else {
        SEF_KEY_VIEW->actors[2] = 0;
        SEF_KEY_VIEW->actors[3] = 0;
    }
    SEF_KEY_VIEW->objectOffset = (int) sefFindEntryById((int) source);
    SEF_KEY_VIEW->kindTableOffset[0] =
        ((int) ((gSbStudioBlock_2018E8[2] >> 2) & 3)) != 1 ? (int) sefFindEntryById((int) target) : 0;
    SEF_KEY_VIEW->kindTableOffset[1] = (int) sefFindEntryById((int) actor0);
    SEF_KEY_VIEW->kindTableOffset[2] = (int) sefFindEntryById((int) actor1);
    if (((int) ((gSbStudioBlock_2018E8[2] >> 2) & 3)) == 0) {
        SEF_KEY_VIEW->kindTableOffset[3] = (int) sefFindEntryById((int) actor2);
        SEF_KEY_VIEW->kindTableOffset[4] = (int) sefFindEntryById((int) actor3);
    } else {
        SEF_KEY_VIEW->kindTableOffset[3] = 0;
        SEF_KEY_VIEW->kindTableOffset[4] = 0;
    }
    if (source != 0) {
        sefSetMatrixOrUnit(SEF_KEY_VIEW->kind256Matrix, source);
    } else {
        Vu0CopyUnitMatrix(SEF_KEY_VIEW->kind256Matrix, MMathUnitMatrix);
    }
    sefSetMatrixOrUnit(SEF_KEY_VIEW->kind512Matrix, target);
    sefSetMatrixOrUnit(SEF_KEY_VIEW->actorMatrices[0], actor0);
    sefSetMatrixOrUnit(SEF_KEY_VIEW->actorMatrices[1], actor1);
    if (((int) ((gSbStudioBlock_2018E8[2] >> 2) & 3)) == 0) {
        sefSetMatrixOrUnit(SEF_KEY_VIEW->actorMatrices[2], actor2);
        sefSetMatrixOrUnit(SEF_KEY_VIEW->actorMatrices[3], actor3);
    } else {
        Vu0CopyUnitMatrix(SEF_KEY_VIEW->actorMatrices[2], MMathUnitMatrix);
        Vu0CopyUnitMatrix(SEF_KEY_VIEW->actorMatrices[3], MMathUnitMatrix);
    }
    if (source != 0) {
        sefGetBattlePoint(&minuend, 1, 1);
        sefGetBattlePoint(&subtrahend, 1, 0);
        subtrahend.y = 0;
        minuend.y = 0;
        sefSetSlotDirectionBetween(SEF_KEY_VIEW->directionMatrix, &minuend,
                                   &subtrahend, &sefDefaultAxis);
        {
            SefRequestKeyView *keys = (SefRequestKeyView *) sefRequestKeyBlock;
            QuadwordCopyT0(keys->directionMatrix[3], keys->kind512Matrix[3]);
        }
    } else {
        QuadwordCopyMatrixT0(SEF_KEY_VIEW->directionMatrix,
                             SEF_KEY_VIEW->kind512Matrix);
    }
    if (sefCurrentEffectRecord != 0 &&
        ((SefScheduleRecord *) sefCurrentEffectRecord)->scalePercent != 0) {
        D_00475388[0] =
            (float) ((SefScheduleRecord *) sefCurrentEffectRecord)->scalePercent * 0.01f;
    } else {
        D_00475388[0] = sefGetEntryScale((s32) target) * 0.1f;
    }
}

typedef struct SefRecordScanView {
    unsigned char unmodeled_00[4];
    unsigned int flags; /* +0x04: any of 0x400502 skips the record */
    unsigned char unmodeled_08[2];
    short gate;         /* +0x0A: zero skips the record */
    int distanceSource; /* +0x0C */
    unsigned char unmodeled_10[0x50 - 0x10];
    int active; /* +0x50: zero skips the record */
    unsigned char unmodeled_54[0xFC - 0x54];
    short slotIds[32]; /* +0xFC */
    unsigned char unmodeled_13C[0x142 - 0x13C];
    signed char delayFrames; /* +0x142: positive skips the record */
    unsigned char unmodeled_143[0x150 - 0x143];
} SefRecordScanView;

typedef struct SefLodStateView {
    int levelIndex;    /* +0x00 */
    float levelScaled; /* +0x04 */
} SefLodStateView;

struct XglCamera;

/* Scales the camera for each eligible record of the effect table (not in studio
 * mode 0): the slots of a record whose effect kind the distance reaches set the
 * camera default scale from their scale byte at 0x6B, and the LOD state takes the
 * scaled level. Records with an active slot then run func_00201690. */
void func_00201C30(SefRecordScanView *records) {
    SefRecordScanView *record;
    short *slotIdCursor;
    SefEffectSlot *slot;
    XglCamera *camera;
    int i;
    int k;
    int distance;
    int activeCount;
    const float distanceUnit = 1000.0f;
    float unitScale;

    if (((int) (((*(unsigned long long (*)[]) & gSbStudioBlock)[2] >> 2) & 3)) != 0) {
        return;
    }
    i = 0;
    if (sefEffectRecordCount > 0) {
        record = records;
        do {
            activeCount = 0;
            if (record->active != 0 && record->gate != 0 && (record->flags & 0x400502) == 0 &&
                record->delayFrames <= 0) {
                distance = (int) ((float) record->distanceSource / distanceUnit);
                if (distance >= 4095) {
                    distance = 4092;
                }
                slotIdCursor = record->slotIds;
                for (k = 31; k >= 0; k--, slotIdCursor++) {
                    slot = sefEffectSlotFor(*slotIdCursor);
                    if (slot == 0) {
                        continue;
                    }
                    if (distance < slot->effectKind) {
                        continue;
                    }
                    activeCount += slot->flags & 1;
                    if (((signed char *) slot)[0x6B] < 0) {
                        continue;
                    }
                    unitScale = 1.0f;
                    camera = xglStudioGetCamera2(0);
                    camera->defaultScale = (float) ((signed char *) slot)[0x6B] * 0.01f;
                    camera = xglStudioGetCamera2(0);
                    if (__builtin_fabsf(camera->defaultScale - unitScale) < 0.000001f) {
                        camera = xglStudioGetCamera2(0);
                        camera->defaultScale = unitScale;
                    }
                    camera = xglStudioGetCamera2(0);
                    ((SefLodStateView *) sefKeyframeStepBlock)->levelScaled =
                        (float) (int) (((camera->defaultScale *
                                         ((float) ((SefLodStateView *) sefKeyframeStepBlock)->levelIndex /
                                          distanceUnit)) +
                                        0.0001f) *
                                       distanceUnit) /
                        distanceUnit;
                }
                if (activeCount != 0) {
                    func_00201690(record);
                }
            }
            record++;
        } while (++i < sefEffectRecordCount);
    }
}

typedef struct SefLaneTally {
    unsigned int laneCount0, laneCount1, laneCount2, laneCount3; /* +0x00 */
    unsigned int grandTotal;                                     /* +0x10 */
    unsigned int unmodeled_14;                                   /* +0x14 */
} SefLaneTally;

extern SefLaneTally sefEffectTallies_201EA0[];

typedef struct SefSlotLines {
    s16 lineIndex[32];            /* +0x00: line ids of the slot */
    unsigned char unk_40[0x20];
    unsigned int unk_60;
    unsigned char unk_64[8];
} SefSlotLines;

typedef struct SefLineRecord {
    unsigned char unk_00[0xC8];
    unsigned short unk_0C8;
    unsigned char unk_CA[0x160 - 0xCA];
    s16 *unk_160;
    unsigned char unk_164[0x1BC - 0x164];
    unsigned short unk_1BC;
    s16 unk_1BE;
    unsigned char unk_1C0[0x664 - 0x1C0];
    s16 *unk_664;
    unsigned char unk_668[0x694 - 0x668];
    s32 unk_694;
    unsigned char unk_698[0x8A0 - 0x698];
    unsigned short unk_8A0;
    unsigned char unk_8A2[14];
} SefLineRecord;

typedef struct SefSlotRecord {
    s32 unk_00;
    s32 flags;
    s16 model;
    s16 unk_0A;
    struct {
        s32 timeValue;
        unsigned char unk_10[0xEC];
        s16 slots[32];
    } slotBlock;
} SefSlotRecord;

typedef struct SefRecordVectors {
    u8 a[0x10];
    AlignedVector4 v10;
    AlignedVector4 v20;
    AlignedVector4 v30;
} SefRecordVectors;

#define SEF_LINE_RECORDS ((SefLineRecord *)sefLineDataRecords)
#define SEF_SLOT_LINES ((SefSlotLines *)sefEffectSlots)

static inline SefSlotLines *SefSlotLinesAt(s32 i) {
    extern s32 sefEffectSlotCount;
    if (i >= 0 && i < sefEffectSlotCount) {
        return &SEF_SLOT_LINES[i];
    }
    return 0;
}

int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *effectMatrix);

/* Adds the lane tallies of every line of the record's slots to the tally of its model:
 * one term per line, sized by its header and tail words and entry count. */
void sefAddSlotLaneTallies(SefSlotRecord *record) {
    s32 slotIndex;
    s32 slotLine;

    ((SefRecordVectors *) record)->v10.w = 1.0f;
    if (sefBuildEffectMatrix(
            &((SefRecordVectors *) record)->v20, &((SefRecordVectors *) record)->v10, &((SefRecordVectors *) record)->v30, 0) != 0) {
        record->flags |= 0x1000000;
    } else {
        record->flags &= ~0x1000000;
    }
    for (slotIndex = 0; slotIndex < 32; slotIndex++) {
        SefSlotLines *slot = SefSlotLinesAt(record->slotBlock.slots[slotIndex]);
        if (slot != 0 && slot->unk_60 >= 2) {
            for (slotLine = 0; slotLine < 32; slotLine++) {
                s32 lineId = slot->lineIndex[slotLine];
                SefLineRecord *line;
                if (lineId < 0) {
                    continue;
                }
                line = &SEF_LINE_RECORDS[lineId];
                if (line != 0 && line->unk_8A0 != 0) {
                    if (line->unk_1BE == 4) {
                        s32 isTwoLane = 0;
                        s32 tailWord;
                        s32 laneCount;
                        if (line->unk_160[-2] == 8 || line->unk_0C8 == 8) {
                            isTwoLane = 1;
                        }
                        tailWord = 0;
                        if (line->unk_664 != 0) {
                            tailWord = line->unk_664[-1];
                        }
                        laneCount = tailWord != 0 ? 17 : 14;
                        if (line->unk_1BC & 0x4000) {
                            laneCount *= 2;
                        }
                        laneCount *= line->unk_8A0;
                        if (isTwoLane) {
                            sefEffectTallies_201EA0[record->model].laneCount1 += laneCount;
                        } else if (line->unk_694 & 0x4000) {
                            sefEffectTallies_201EA0[record->model].laneCount2 += laneCount;
                        } else {
                            sefEffectTallies_201EA0[record->model].laneCount0 += laneCount;
                        }
                    } else {
                        sefEffectTallies_201EA0[record->model].laneCount3 += line->unk_8A0;
                    }
                }
            }
        }
    }
}

typedef struct SefSlotLineIndices {
    s16 lineIndex[32];            /* +0x00 */
    unsigned char unk_40[0x2C];
} SefSlotLineIndices;

typedef struct SefLineTimeRecord {
    unsigned char unk_00[0x68C];
    s32 time0;                    /* +0x68C */
    s32 time1;                    /* +0x690 */
    s32 unk_694[2];
    s16 pointIds[256];            /* +0x69C */
    unsigned char unk_89C[0x14];
} SefLineTimeRecord;

typedef struct SefPointRecord {
    unsigned char unk_00[0x140];
    s16 timeValue;                /* +0x140 */
    unsigned char unk_142[0xEE];
    s16 timeValues[32];           /* +0x230 */
} SefPointRecord;

typedef struct SefPointAllocator {
    unsigned char unk_00[16];
    SefPointRecord points[1];
} SefPointAllocator;

typedef struct SefRecordTimeScan {
    unsigned char unk_00[0xC];
    s32 timeValue;                /* +0x0C */
    unsigned char unk_10[0xEC];
    s16 slots[32];                /* +0xFC */
} SefRecordTimeScan;

static inline SefSlotLineIndices *SefSlotAt(s32 i) {
    if (i >= 0 && i < sefEffectSlotCount) {
        return &((SefSlotLineIndices *) sefEffectSlots)[i];
    }
    return 0;
}

static inline SefLineTimeRecord *SefLineAt(s32 i) {
    if (i < 0) {
        return 0;
    }
    return &((SefLineTimeRecord *) (*(SefLineTimeRecord * (*) [1]) & sefLineDataRecords)[0])[i];
}

static inline SefPointRecord *SefPointAt(s32 i) {
    if (i < 0 || i >= sefPtAllocCount) {
        return 0;
    }
    return &((SefPointAllocator *) (*(SefPointAllocator * (*) [1]) & sefPtAllocatorState)[0])->points[i];
}

/* Rebases the effect record's clock and the times of its slots, lines and points by one
 * 5000000 step (the points by 5000) when the clock is above 0xFA0000. */
void sefRebaseRecordTimes(SefRecordTimeScan *record) {
    s32 index;
    s32 pointIndex;
    s32 timeIndex;
    s32 timeShift;

    if (record->timeValue > 0xFA0000) {
        timeShift = 5000000;
        record->timeValue -= timeShift;
        for (index = 0; index < 32; index++) {
            SefSlotLineIndices *slot = SefSlotAt(record->slots[index]);
            if (slot != 0) {
                /* The inner loop reuses index: the match requires this nested form. */
                for (index = 0; index < 32; index++) {
                    SefLineTimeRecord *line = SefLineAt(slot->lineIndex[index]);
                    if (line != 0) {
                        line->time0 -= timeShift;
                        line->time1 -= timeShift;
                        for (pointIndex = 0; pointIndex < 256; pointIndex++) {
                            SefPointRecord *point = SefPointAt(line->pointIds[pointIndex]);
                            if (point != 0) {
                                if (point->timeValue - 5000 < 0) {
                                    point->timeValue = 0;
                                } else {
                                    point->timeValue -= 5000;
                                }
                                for (timeIndex = 0; timeIndex < 32; timeIndex++) {
                                    if (point->timeValues[timeIndex] - 5000 < 0) {
                                        point->timeValues[timeIndex] = 0;
                                    } else {
                                        point->timeValues[timeIndex] -= 5000;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

typedef union SefDwordValue {
    unsigned long q;
    int w;
} SefDwordValue;

typedef struct SefActorBlock {
    unsigned char a[0x24];
    float f24;
} SefActorBlock;

typedef struct SefRunRecord {
    int w0;
    union {
        int w;
        unsigned int u;
    } w4;
    short h8;
    short hA;
    int wC;
    AlignedVector4 v10;
    AlignedVector4 v20;
    unsigned char x30[0x20];
    void *w50;
    void *w54;
    SefActorBlock *w58;
    void *w5C;
    void *w60;
    void *w64;
    void *w68;
    void *w6C;
    unsigned int w70;
    float f74;
    int w78;
    unsigned char x7C[12];
    unsigned char x88[12];
    unsigned char x94[0x2C];
    unsigned char xC0[0x18];
    unsigned char xD8[12];
    unsigned char xE4[16];
    float fF4;
    float fF8;
    short hh[32];
    unsigned short h13C;
    unsigned char x13E[4];
    signed char delayFrames;
    signed char linkValue;
    unsigned char x144[12];
} SefRunRecord;

typedef struct SefRunRecordFlags {
    int w0;
    int w4;
    short h8;
    short hA;
    int wC;
    AlignedVector4 v10;
    AlignedVector4 v20;
    unsigned char x30[0x20];
    void *w50;
    void *w54;
    SefActorBlock *w58;
    void *w5C;
    void *w60;
    void *w64;
    void *w68;
    void *w6C;
    unsigned int w70;
    float f74;
    int w78;
    unsigned char x7C[12];
    unsigned char x88[12];
    unsigned char x94[0x2C];
    unsigned char xC0[0x18];
    unsigned char xD8[12];
    unsigned char xE4[16];
    float fF4;
    float fF8;
    short hh[32];
    unsigned short h13C;
    unsigned char x13E[4];
    signed char delayFrames;
    signed char linkValue;
    unsigned char x144[12];
} SefRunRecordFlags;

typedef struct SefCameraView {
    unsigned char a[0x170];
    M4 m170;
    unsigned char x1B0[0x5F4 - 0x1B0];
    float f5F4;
} SefCameraView;

typedef struct SefKeyframeStepBlock {
    int w0;
    float f4;
    float f8;
    float fC;
} SefKeyframeStepBlock;

typedef struct SefRequestKeyWords {
    void *w0;
    void *w4;
    unsigned char a[0x10];
    unsigned int w18;
    unsigned char b[0x2B8 - 0x1C];
    int w2B8;
    int w2BC;
    float f2C0;
    float f2C4;
} SefRequestKeyWords;

typedef struct SefScaleBlock {
    int w0;
    float f4;
    float f8;
    float fC;
    float f10;
} SefScaleBlock;

typedef struct SefKeyTail {
    SefDwordValue q;
    union {
        int w;
        unsigned int u;
    } w8;
} SefKeyTail;

typedef struct SefStudioFlags {
    unsigned long a0 : 1;
    unsigned long a1 : 63;
    unsigned long q8;
    unsigned long b0 : 1;
    unsigned long b1 : 1;
    unsigned long b2 : 2;
    unsigned long b4 : 1;
    unsigned long b5 : 1;
    unsigned long b6 : 58;
} SefStudioFlags;

extern int D_0055109C;
extern void *D_00551514_2022F0;
extern SefCameraMatrixBlock sefCameraMatrixBlock;
void sefUpdateEntryLights(void);
void sefRebuildEntryTable(void);
void sefRefreshProbePoints(void);
extern void sdvStartSlotEvents(void*,int);
extern int sdvCameraScheduleUpdate(void*,float);
void battleSetPhase(s32 nextBattlePhase);
extern void sdvExecShadowSeq(void*);
extern void tiWindUpdateKeyframes(void*,unsigned int,int);
extern void sdvFeedOwnerSpecials(void*,void*);
extern void sdvStartSequenceAlters(void*,void*,int,int,void*);
extern void func_001F3A98(void*,void*,int);
extern void tiWindReleaseSlots(unsigned int,int);
void func_002071F8(void);

#define SEF_CURRENT_RECORD ((SefRunRecord *)sefCurrentEffectRecord)
#define SEF_KEYFRAME_STEP (*(SefKeyframeStepBlock (*))&sefKeyframeStepBlock)
#define SEF_REQUEST_KEY_WORDS ((int *)sefRequestKeyBlock)
#define XGL_STUDIO_GET_CAMERA2 ((SefCameraView *(*)(int))xglStudioGetCamera2)

static inline SefRunRecord *SefRunRecordArray(void) {
    return (*(SefRunRecord * (*) [1]) & sefEffectRecords)[0];
}

static inline void sefCopyMatrix(M4 *destination, M4 *source) {
    __asm__ __volatile__(
        "lq $8,0(%1)\n lq $9,16(%1)\n lq $10,32(%1)\n lq $11,48(%1)\n sq $8,0(%0)\n sq $9,16(%0)\n sq $10,32(%0)\n sq $11,48(%0)"
        :
        : "r"(destination), "r"(source)
        : "$8", "$9", "$10", "$11", "memory");
}

static inline void sefSubVec3(AlignedVector4 *destination, AlignedVector4 *left, AlignedVector4 *right) {
    __asm__ __volatile__("lqc2 vf1,0(%4)\n lqc2 vf2,0(%5)\n vsub.xyz vf1,vf1,vf2\n sqc2 vf1,0(%3)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z)
                         : "r"(destination), "r"(left), "r"(right));
}

#define SEF_TIME_ROUNDING 0.0001f

static inline int sefSecondsToMs(float x) {
    return (int) ((x + SEF_TIME_ROUNDING) * 1000.0f);
}

static inline float sefRoundSeconds(float x) {
    return (float) sefSecondsToMs(x) / 1000.0f;
}

extern int D_00551084;
extern Vector4 sefOriginVector;
void sefTickGroupTimers(void);
void sefResetEntryFlagsAndOrigin(void);
struct XglCamera *xglStudioGetActiveCamera(void);
void sefGetGroupCenter(float *center, int owner, unsigned short key);

/* Runs one frame of the effect records: ticks each live record (or finishes it when its
 * link value is negative), and returns 1 while some record is still pending. */
int sefRunEffectRecords(SefRunRecord *only) {
    int keyWord;
    int pendingCount = 0;
    int anyPlaying = 0;
    int recordIndex;
    int slotIndex;
    int recordFlag2;
    int activeCount;
    int wholeSeconds;
    SefRunRecord *record;
    SefCameraView *activeCamera;
    float timeSeconds;
    SefRequestKeyWords *keyWords;
    D_00551040 = 0;
    sefVectorMode = 0;
    sefUpdateEntryLights();
    sefRebuildEntryTable();
    sefRefreshProbePoints();
    sefTickGroupTimers();
    sefResetEntryFlagsAndOrigin();
    (*(SefStudioFlags *) &gSbStudioBlock).b5 = 0;
    activeCamera = ((SefCameraView * (*) (void) ) xglStudioGetActiveCamera)();
    if (sefCameraMatrixBlock.w94 != 0) {
        sefCopyMatrix(&sefCameraMatrixBlock.m40, &sefCameraMatrixBlock.m0);
    } else {
        sefCopyMatrix(&sefCameraMatrixBlock.m40, &activeCamera->m170);
        sefCameraMatrixBlock.w94 = 1;
    }
    sefCopyMatrix(&sefCameraMatrixBlock.m0, &activeCamera->m170);
    sefCameraMatrixBlock.m0.r[3].y = 0.0f;
    sefCameraMatrixBlock.p90 = activeCamera->x1B0;
    sefCameraMatrixBlock.m40.r[3].y = 0.0f;
    sefSubVec3(&sefCameraMatrixBlock.v80, &sefCameraMatrixBlock.m0.r[3], &sefCameraMatrixBlock.m40.r[3]);
    SEF_KEYFRAME_STEP.f4 =
        sefRoundSeconds(XGL_STUDIO_GET_CAMERA2(0)->f5F4 * ((float) SEF_KEYFRAME_STEP.w0 / 1000.0f));
    if (only == 0) {
        func_00201C30(SefRunRecordArray());
    }
    for (recordIndex = 0; recordIndex < sefEffectRecordCount; recordIndex++) {
        record = &SefRunRecordArray()[recordIndex];
        keyWord = D_00551084;
        recordFlag2 = ((SefRunRecordFlags *) record)->w4 & 2;
        activeCount = 0;
        if (only != 0 && record != only) {
            continue;
        }
        if (record->w50 != 0) {
            if (((SefRunRecordFlags *) record)->w4 & 0x400000) {
                SEF_CURRENT_RECORD = record;
                sefAddSlotLaneTallies(record);
                if (SefRunRecordArray()[recordIndex].hA != 0) {
                    sdvStartSlotEvents(SefRunRecordArray()[recordIndex].xC0, SefRunRecordArray()[recordIndex].h8);
                }
            } else {
                if (record->delayFrames > 0) {
                    record->delayFrames--;
                    pendingCount++;
                    continue;
                }
                D_0055109C = record->w70;
                D_00551514_2022F0 = record->w54;
                SEF_CURRENT_RECORD = record;
                SEF_KEYFRAME_STEP.f8 = sefRoundSeconds(record->f74 * SEF_KEYFRAME_STEP.f4);
                timeSeconds = (float) record->wC / 1000.0f;
                record->wC += sefSecondsToMs(SEF_KEYFRAME_STEP.f8);
                ((SefKeyTail *) &((SefRequestKeyWords *) SEF_REQUEST_KEY_WORDS)->w18)->q.q = ((SefDwordValue *) &record->w70)->q;
                ((SefKeyTail *) &((SefRequestKeyWords *) SEF_REQUEST_KEY_WORDS)->w18)->w8.w = record->w78;
                keyWords = (SefRequestKeyWords *) SEF_REQUEST_KEY_WORDS;
                keyWords->w2B8 = ((SefRunRecordFlags *) record)->w4;
                keyWords->f2C4 = record->fF4;
                keyWords->w18 = record->w70;
                keyWords->w2BC = recordFlag2;
                func_002018E8(record->w58, record->w5C, record->w60, record->w64, record->w68, record->w6C);
                sefUpdateRecordHitFlagsAndMatrices(recordIndex);
                (*(SefScaleBlock *) &sefMotionFloorCurve).f10 = 0.0f;
                if (((D_0055109C >> 28) & 3) == 1 && record->w58 != 0) {
                    (*(SefScaleBlock *) &sefMotionFloorCurve).f10 = record->w58->f24;
                }
                sefGetGroupCenter(
                    &sefOriginVector, D_0055109C, record->h13C);
                record->v10.w = 1.0f;
                if (sefBuildEffectMatrix(
                        &record->v20, &record->v10, record->x30, 0)) {
                    ((SefRunRecordFlags *) record)->w4 |= 0x1000000;
                } else {
                    ((SefRunRecordFlags *) record)->w4 &= ~0x1000000;
                }
                if (!recordFlag2 && (((SefRunRecordFlags *) record)->w4 & 0x10000000) && timeSeconds > 2.0f) {
                    ((SefRunRecordFlags *) record)->w4 &= ~0x10000000;
                    SEF_SET_ENTRY_FLAGS_BY_KEY(((SefRequestKeyWords *) SEF_REQUEST_KEY_WORDS)->w0, 4);
                }
                wholeSeconds = (int) timeSeconds;
                for (slotIndex = 0; slotIndex < 32; slotIndex++) {
                    if (sefRunEffectSlotScript(&SefRunRecordArray()[recordIndex].hh[slotIndex], wholeSeconds, &keyWord, recordIndex)) {
                        activeCount++;
                    }
                }
                if (!(((SefRunRecordFlags *) &SefRunRecordArray()[recordIndex])->w4 & 0x40)) {
                    anyPlaying |= sdvCameraScheduleUpdate(SefRunRecordArray()[recordIndex].x94, timeSeconds);
                    if (anyPlaying) {
                        if ((*(SefStudioFlags *) &gSbStudioBlock).a0 != 0 || (*(SefStudioFlags *) &gSbStudioBlock).b2 != 1) {
                            if ((*(SefStudioFlags *) &gSbStudioBlock).b0 == 0) {
                                battleSetPhase(1);
                                (*(SefStudioFlags *) &gSbStudioBlock).b0 = 1;
                            }
                        }
                    }
                }
                if (!recordFlag2) {
                    sdvExecShadowSeq(SefRunRecordArray()[recordIndex].x7C);
                    tiWindUpdateKeyframes(SefRunRecordArray()[recordIndex].xD8, record->w70, record->h13C);
                }
                sdvFeedOwnerSpecials(SefRunRecordArray()[recordIndex].xC0, SefRunRecordArray()[recordIndex].w54);
                sdvStartSequenceAlters(SefRunRecordArray()[recordIndex].x88, &SefRunRecordArray()[recordIndex].w58, SefRunRecordArray()[recordIndex].h8, recordIndex, SefRunRecordArray()[recordIndex].w54);
                func_001F3A98(SefRunRecordArray()[recordIndex].xE4, SefRunRecordArray()[recordIndex].w54, (int) timeSeconds);
                if (SefRunRecordArray()[recordIndex].hA != 0) {
                    sdvStartSlotEvents(SefRunRecordArray()[recordIndex].xC0, SefRunRecordArray()[recordIndex].h8);
                }
                if (activeCount == 0) {
                    if (!recordFlag2) {
                        tiWindReleaseSlots(record->w70, record->h13C);
                    }
                    if (SefRunRecordArray()[recordIndex].linkValue < 0 && !(((SefRunRecordFlags *) &SefRunRecordArray()[recordIndex])->w4 & 0x20)) {
                        sefReleaseEffectRecord(recordIndex);
                        continue;
                    }
                    for (slotIndex = 0; slotIndex < 32; slotIndex++) {
                        sefReleaseEffectSlot(SefRunRecordArray()[recordIndex].hh[slotIndex]);
                        SefRunRecordArray()[recordIndex].hh[slotIndex] = -1;
                    }
                    SefRunRecordArray()[recordIndex].wC = (int) (float) SefRunRecordArray()[recordIndex].linkValue;
                    sefSpawnRecordItems(SefRunRecordArray()[recordIndex].w50, recordIndex, 0);
                    recordIndex--;
                    continue;
                }
                sefRebaseRecordTimes(&SefRunRecordArray()[recordIndex]);
                if (!(((SefRunRecordFlags *) &SefRunRecordArray()[recordIndex])->w4 & 0x40)) {
                    pendingCount++;
                }
            }
        }
        SefRunRecordArray()[recordIndex].w4.w &= ~9;
    }
    func_002071F8();
    sefVectorMode = 0;
    return pendingCount > 0;
}

extern SefEntryTableShort sefEffectEntryTable;
void func_001975C8(XenoLight *light);

/* Partial view of one effect entry: its actor object at +0x1550 and its state at +0x1590. */
typedef struct SefEffectEntryObjectView {
    unsigned char unmodeled_000[0x1550];
    XenoActor *object; /* +0x1550 */
    unsigned char unmodeled_1554[0x1590 - 0x1554];
    short state; /* +0x1590 */
    unsigned char unmodeled_1592[0x15A0 - 0x1592];
} SefEffectEntryObjectView;

/* Fields of gCameraSelectFlags. cameraSelect (bits 2-3) is compared with 0 and 1 by the effect code. */
typedef struct CameraSelectFlags {
    unsigned long long unmodeled_a : 2;
    unsigned long long cameraSelect : 2;
    unsigned long long unmodeled_c : 60;
} CameraSelectFlags;

/* Calls func_001975C8 on the light of every live effect entry (object set, state not 2) while cameraSelect is 0. */
void sefUpdateEntryLights(void) {
    int index;

    if (((CameraSelectFlags *) (*(unsigned long long (*)[]) & gCameraSelectFlags))->cameraSelect == 0) {
        for (index = 0; index < sefEffectEntryTable.effectEntryCount; index++) {
            SefEffectEntryObjectView *entry = (SefEffectEntryObjectView *) &sefEffectEntryTable.effectEntries[index];

            if (entry->object != 0 && entry->state != 2) {
                func_001975C8(&entry->object->light);
            }
        }
    }
}

/* Full view of one effect table entry (0x15A0 bytes) with every field this unit reads or writes. The 0x1530 and 0x1540
 * vectors are not in SefEntry (include/main/tu118.h); they are the two vectors sefRebuildEntryTable zeroes. */
typedef struct SefEffectEntryFields {
    unsigned char unmodeled_000[0xCC0];
    AlignedVector4 slotVectors[64]; /* +0xCC0 */
    unsigned char unmodeled_10C0[0x1530 - 0x10C0];
    AlignedVector4 clearedVector0; /* +0x1530 */
    AlignedVector4 clearedVector1; /* +0x1540 */
    int id;               /* +0x1550 */
    int effectEntryFlags; /* +0x1554 */
    union {
        float halfHeight; /* +0x1558 (read as f32 scale by sefGetEntryScale) */
        unsigned char unmodeled_1558[0x155C - 0x1558];
    };
    signed char slotFlags[0x20]; /* +0x155C: bit 0 = slot set */
    unsigned char unmodeled_157C[0x1590 - 0x157C];
    short modeValue;      /* +0x1590 */
    short unmodeled_1592; /* +0x1592 */
    short modeZero;       /* +0x1594 */
    unsigned char unmodeled_1596[0x15A0 - 0x1596];
} SefEffectEntryFields;

/* The effect entry table as a whole: 24 entries, the cleared vector at +0x20700 and the entry count at +0x20710. */
typedef struct SefEffectEntryTableView {
    SefEffectEntryFields effectEntries[24];
    AlignedVector4 clearedVector; /* +0x20700 */
    int effectEntryCount;         /* +0x20710 */
} SefEffectEntryTableView;

#define SEF_EFFECT_ENTRY_VIEW_TABLE (*(SefEffectEntryTableView *) &sefEffectEntryTable)

/* The float at +0x84 of an effect object, the scale that sefRegisterEntry applies when cameraSelect is 0. */
typedef struct EffectObjectScaleView {
    unsigned char unmodeled_00[0x84];
    float scale; /* +0x84 */
} EffectObjectScaleView;

/* Adds or updates the entry of object (its id): a new entry takes mode value and a half height from sefReadProbePoint and
 * its camera scale (object+0x84 when cameraSelect is 0), clamped to [0.25, 4]. Does nothing past 24 entries. */
void sefRegisterEntry(void *object, int value) {
    int index;
    float scratch[2];

    for (index = 0; index < sefEffectEntryTable.effectEntryCount; index++) {
        if (sefEffectEntryTable.effectEntries[index].id == (int) object) {
            break;
        }
    }
    if (index == sefEffectEntryTable.effectEntryCount && index < 24) {
        sefEffectEntryTable.effectEntryCount++;
        sefEffectEntryTable.effectEntries[index].id = (int) object;
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].modeZero = 0;
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].modeValue = value;
        if (sefReadProbePoint(object, 0, scratch, 0) != 0) {
            SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].halfHeight = scratch[1] * 0.5f;
            if ((int) (((*(u64(*)[]) & gCameraSelectFlags)[0] >> 2) & 3) == 0) {
                SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].halfHeight *= ((EffectObjectScaleView *) object)->scale;
            }
        }
        if (SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].halfHeight > 4.0f) {
            SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].halfHeight = 4.0f;
        } else if (SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].halfHeight < 0.25f) {
            SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].halfHeight = 0.25f;
        }
    }
}

/* Partial view of one effect record (0x150 bytes): the flags word at +0x04, the used flag at +0x50 and six object keys at
 * +0x58. */
typedef struct EffectRecordView {
    s32 unmodeled_00;
    s32 flags;
    u8 unmodeled_08[0x48];
    s32 used;
    s32 unmodeled_54;
    s32 objectKeys[6];
    s32 unmodeled_70;
    u8 unmodeled_74[0xDC];
} EffectRecordView;

/* The three object words returned by func_001F9998 for each of the 40 object slots. */
typedef struct EffectObjectKeys {
    s32 unmodeled_00;
    s32 keyB;
    s32 keyC;
} EffectObjectKeys;

/* Writes the 16-byte zero vector with sqc2 vf0 (the zero register). */
static inline void SefStoreZeroVector(AlignedVector4 *destination) {
    __asm__ __volatile__("sqc2 vf0,0(%4)"
                         : "=m"(destination->x), "=m"(destination->y), "=m"(destination->z), "=m"(destination->w)
                         : "r"(destination)
                         : "memory");
}

extern EffectObjectKeys *func_001F9998(s32);

/* Clears the entry table, then registers the object keys of each live record without flag 0x400 and of the 40 object
 * slots of func_001F9998. */
void sefRebuildEntryTable(void) {
    s32 index;
    EffectObjectKeys *object;

    SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntryCount = 0;
    SefStoreZeroVector(&SEF_EFFECT_ENTRY_VIEW_TABLE.clearedVector);
    for (index = 0; index < 24; index++) {
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].id = 0;
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].modeZero = 0;
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].modeValue = 0;
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].unmodeled_1592 = 0;
        SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].effectEntryFlags = 0;
        SefStoreZeroVector(&SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].clearedVector0);
        SefStoreZeroVector(&SEF_EFFECT_ENTRY_VIEW_TABLE.effectEntries[index].clearedVector1);
    }
    for (index = 0; index < sefEffectRecordCount; index++) {
        EffectRecordView *record = &((EffectRecordView *) sefEffectRecords)[index];

        if (record->used != 0 && !(record->flags & 0x400)) {
            if (record->objectKeys[0] != 0) {
                sefRegisterEntry(record->objectKeys[0], 1);
            }
            if (record->objectKeys[1] != 0) {
                sefRegisterEntry(record->objectKeys[1], 0);
            }
            if (record->objectKeys[2] != 0) {
                sefRegisterEntry(record->objectKeys[2], 2);
            }
            if (record->objectKeys[3] != 0) {
                sefRegisterEntry(record->objectKeys[3], 2);
            }
            if (gSbStudioBlock.mode == 0) {
                if (record->objectKeys[4] != 0) {
                    sefRegisterEntry(record->objectKeys[4], 2);
                }
                if (record->objectKeys[5] != 0) {
                    sefRegisterEntry(record->objectKeys[5], 2);
                }
            }
        }
    }
    for (index = 0; index < 40; index++) {
        object = func_001F9998(index);
        if (object != 0) {
            if (object->keyB != 0) {
                sefRegisterEntry((void *) object->keyB, 1);
            }
            if (object->keyC != 0) {
                sefRegisterEntry((void *) object->keyC, 0);
            }
        }
    }
}

/* Partial view of gSbStudioBlock: the 64-bit studio word after its 0x10 bytes of other fields. */
typedef struct SbStudioView {
    u8 unmodeled_00[0x10];
    u64 unmodeledLow : 2;
    u64 mode : 2;
    u64 unmodeledHigh : 60;
} SbStudioView;

/* Sets flagBits in the effect entry whose id is key; the first entry is tested before the scan. */
void sefSetEntryFlagsByKey(int key, int flagBits)
{
    SefEntry *foundEntry;
    int effectEntryIndex;

    if (key == 0) {
        return;
    }
    effectEntryIndex = 0;
    if (sefEffectEntryTable.effectEntryCount <= 0) {
        return;
    }
    if (sefEffectEntryTable.effectEntries[0].id == key) {
        sefEffectEntryTable.effectEntries[0].effectEntryFlags |= flagBits;
        return;
    }
nextEntry:
    effectEntryIndex++;
candidateCheck:
    if (effectEntryIndex >= sefEffectEntryTable.effectEntryCount) {
        return;
    }
    foundEntry = &sefEffectEntryTable.effectEntries[effectEntryIndex];
    if (foundEntry->id != key) {
        goto nextEntry;
    }
    foundEntry->effectEntryFlags |= flagBits;
}

/* Clears flagBits in the effect entry whose id is key; the first entry is tested before the scan. */
void sefClearEntryFlagsByKey(int key, int flagBits)
{
    SefEntry *foundEntry;
    int effectEntryIndex;

    if (key == 0) {
        return;
    }
    foundEntry = &sefEffectEntryTable.effectEntries[0];
    effectEntryIndex = 0;
    if (sefEffectEntryTable.effectEntryCount <= 0) {
        return;
    }
    if (foundEntry->id != key) {
        goto candidateLoop;
        do {
            foundEntry = 0;
candidateLoop:
            effectEntryIndex++;
            if (effectEntryIndex >= sefEffectEntryTable.effectEntryCount) {
                return;
            }
            foundEntry = &sefEffectEntryTable.effectEntries[effectEntryIndex];
        } while (foundEntry->id != key);
    }
    foundEntry->effectEntryFlags &= ~flagBits;
}

int tiMRPackageGetNodeMatrix(Vector4 *block, void *arg);

/* Multiplies the actor's local matrix into out when apply is set; an actor without a block gets the identity and returns 0. */
int sefActorMulLocalMatrix(XenoActor *actor, void *actorNodeArg, Matrix4 *combinedMatrix, int multiplyLocal)
{
    if (actor == 0) {
        Vu0CopyUnitMatrix(combinedMatrix, &MMathUnitMatrix);
        return 0;
    }
    if (tiMRPackageGetNodeMatrix(&actor->shadowB, actorNodeArg) == 0) {
        return 0;
    }
    if (multiplyLocal != 0) {
        MMathMulMatrix(combinedMatrix, &actor->localMatrix, combinedMatrix);
    }
    return 1;
}

s32 tiMRPackageGetNodePoint(void *node, s32 index, Vector4 *out, s32 flag);

/* Reads point pointIndex of the probe source's package node into out. A nonzero useCameraView maps it through the
 * source's camera view; otherwise it is mapped through the inverse of the matrix sefCopyProbeView builds for the source,
 * when that builds one. Returns 0 only when the point cannot be read. */
int sefReadProbePoint(SefProbeSourceView *source, int pointIndex, Vector4 *out, int useCameraView) {
    Matrix4 sourceMatrix;
    Matrix4 inverseMatrix;

    if (tiMRPackageGetNodePoint(source->packageNode, pointIndex, out, 1) == 0) {
        return 0;
    }
    if (useCameraView != 0) {
        MMathApplyMatrix(out, &source->cameraView, out);
    } else {
        if (sefCopyProbeView(&sourceMatrix, source) != 0) {
            MMathInvertAffineMatrix(&inverseMatrix, &sourceMatrix);
            MMathApplyMatrix(out, &inverseMatrix, out);
        }
    }
    return 1;
}

typedef unsigned int EpQuadword __attribute__((mode(TI)));

/* One VU0 vector, as four floats (v) or as one 128-bit quadword (q). */
typedef union EpVector {
    Vector4 lanes;
    EpQuadword quadword;
} EpVector;

/* Four vectors, one per matrix row. */
typedef struct EpMatrix {
    EpVector rows[4];
} EpMatrix;

/* One effect entry as the probe code uses it: 51 local matrices, 51 local points, 51 camera points, a flag byte per
 * point, and the object pointer at +0x1550. */
typedef struct EpEntry {
    EpMatrix localMatrices[0x33];
    EpVector cameraPoints[0x33];
    EpVector localPoints[0x33];
    u8 unmodeled_1320[0x230];
    u8 *object;
    s32 unmodeled_1554;
    s32 unmodeled_1558;
    u8 pointFlags[0x33];
    u8 unmodeled_158F[0x11];
} EpEntry;

/* The effect entry table as the probe code sees it: 24 entries and the entry count at +0x20710. */
typedef struct EpTable {
    EpEntry entries[24];
    u8 pad[0x20710 - 24 * 0x15A0];
    s32 entryCount;
} EpTable;

/* The model that an object points to at +0xD8: modelWord is the test sefRefreshProbePoints makes. */
typedef struct EpModel {
    u8 unmodeled_00[8];
    s32 modelWord;
} EpModel;

typedef struct EpObject {
    u8 unmodeled_00[0xD8];
    EpModel *model;
} EpObject;

/* The studio word of gSbStudioBlock: onFlag is bit 0. */
typedef struct EpStudio {
    unsigned long onFlag : 1;
    unsigned long unmodeledHigh : 63;
} EpStudio;

#define SEF_STUDIO (*(EpStudio *)&(*(SbStudioView *)&gSbStudioBlock))

const EpVector sefStudioPatternEven[16] = {
    { 0.0f, 2.0f, 2.0f, 1.0f },
    { 0.400000006f, 0.800000012f, 2.0f, 1.0f },
    { -0.200000003f, 1.10000002f, 2.0f, 1.0f },
    { 0.400000006f, 0.800000012f, 2.0f, 1.0f },
    { 0.200000003f, 1.10000002f, 2.0f, 1.0f },
    { -0.100000001f, 0.0f, 2.0f, 1.0f },
    { -0.100000001f, 0.600000024f, 2.0f, 1.0f },
    { 0.100000001f, 0.0f, 2.0f, 1.0f },
    { 0.100000001f, 0.600000024f, 2.0f, 1.0f },
    { 0.0f, 0.0f, 2.0f, 1.0f },
    { 0.0f, 0.0f, 2.0f, 1.0f },
    { 0.0f, 0.0f, 2.0f, 1.0f },
    { 0.0f, 0.0f, 2.0f, 1.0f },
    { 0.0f, 1.70000005f, 2.0f, 1.0f },
    { 0.0f, 1.54999995f, 2.1500001f, 1.0f },
    { 0.0f, 1.29999995f, 2.1500001f, 1.0f }
};
const EpVector sefStudioPatternOdd[16] = {
    { 0.0f, 2.0f, -2.0f, 1.0f },
    { 0.400000006f, 0.800000012f, -2.0f, 1.0f },
    { -0.200000003f, 1.10000002f, -2.0f, 1.0f },
    { 0.400000006f, 0.800000012f, -2.0f, 1.0f },
    { 0.200000003f, 1.10000002f, -2.0f, 1.0f },
    { -0.100000001f, 0.0f, -2.0f, 1.0f },
    { -0.100000001f, 0.600000024f, -2.0f, 1.0f },
    { 0.100000001f, 0.0f, -2.0f, 1.0f },
    { 0.100000001f, 0.600000024f, -2.0f, 1.0f },
    { 0.0f, 0.0f, -2.0f, 1.0f },
    { 0.0f, 0.0f, -2.0f, 1.0f },
    { 0.0f, 0.0f, -2.0f, 1.0f },
    { 0.0f, 0.0f, -2.0f, 1.0f },
    { 0.0f, 1.70000005f, -2.0f, 1.0f },
    { 0.0f, 1.54999995f, -1.85000002f, 1.0f },
    { 0.0f, 1.29999995f, -1.85000002f, 1.0f }
};
extern const Matrix4 vu0IdentityMatrix;


#define SEF_EFFECT_ENTRY_TABLE (*(EpTable (*))&sefEffectEntryTable)
#define SEF_ACTOR_MUL_LOCAL_MATRIX ((s32(*)(void *, s32, EpMatrix *, s32))sefActorMulLocalMatrix)

/* True when the x, y and z lanes are all zero (w is not tested). */
static inline s32 EpIsZero(EpVector *vector) {
    return (*(s32 *) &vector->lanes.z | *(long *) vector) == 0;
}

/* Copies one 16-byte vector through $8 with the R5900 quadword load and store. */
static inline void EpCopy(EpVector *destination, const EpVector *source) {
    __asm__ __volatile__("lq $8,0(%5)\n\tsq $8,0(%4)"
                         : "=m"(destination->lanes.x), "=m"(destination->lanes.y), "=m"(destination->lanes.z), "=m"(destination->lanes.w)
                         : "r"(destination), "r"(source)
                         : "$8", "memory");
}

/* Refreshes the probe points of every effect entry with an object: the local matrices and the local and camera points of
 * each of its 51 indices. An entry whose model has no key takes the studio pattern (sefStudioPatternOdd for odd entries,
 * sefStudioPatternEven otherwise) instead, when the studio flag is on. */
void sefRefreshProbePoints(void) {
    s32 effectEntryIndex;
    s32 pointIndex;
    EpVector tmp;

    tmp.lanes.w = 1.0f;
    for (effectEntryIndex = 0; effectEntryIndex < SEF_EFFECT_ENTRY_TABLE.entryCount; effectEntryIndex++) {
        EpEntry *entry = &SEF_EFFECT_ENTRY_TABLE.entries[effectEntryIndex];

        if (entry->object != 0) {
            EpModel *model;
            EpObject *owner = (EpObject *) (entry->object + 0x560);

            memset(entry->pointFlags, 0, 0x33);
            model = owner->model;
            if (model != 0 && model->modelWord != 0) {
                for (pointIndex = 0; pointIndex < 0x33; pointIndex++) {
                    if (SEF_ACTOR_MUL_LOCAL_MATRIX(entry->object, pointIndex, &entry->localMatrices[pointIndex], 1) != 0 &&
                        sefReadProbePoint(entry->object, pointIndex, &entry->localPoints[pointIndex], 0) != 0 &&
                        sefReadProbePoint(entry->object, pointIndex, &entry->cameraPoints[pointIndex], 1) != 0) {
                        entry->pointFlags[pointIndex] = 1;
                        if (EpIsZero(&entry->cameraPoints[pointIndex])) {
                            entry->cameraPoints[pointIndex].lanes.w = 0.0f;
                        }
                    }
                }
            } else if (SEF_STUDIO.onFlag) {
                EpVector *pattern = (effectEntryIndex & 1) ? sefStudioPatternOdd : sefStudioPatternEven;

                for (pointIndex = 0; pointIndex < 16; pointIndex++) {
                    EpCopy(&entry->localMatrices[pointIndex].rows[3], &pattern[pointIndex]);
                    EpCopy(&entry->cameraPoints[pointIndex], &pattern[pointIndex]);
                }
            }
        }
    }
}

/* Returns the effect entry whose id is key, scanning the table in order; null for key 0 or no match. */
SefEntry *sefFindEntryById(int key)
{
    SefEntryTableShort *effectTable;
    SefEntry *foundEntry;
    int i;

    if (key != 0) {
        effectTable = &sefEffectEntryTable;
        for (i = 0; i < effectTable->effectEntryCount; i++) {
            foundEntry = &effectTable->effectEntries[i];
            if (foundEntry->id == key) {
                return foundEntry;
            }
        }
    }
    return 0;
}

/* Returns the halfHeight scale of the entry with id; 1.0 when no entry has that id. */
f32 sefGetEntryScale(s32 id) {
    SefEntry *foundEntry;
    f32 scale;

    foundEntry = sefFindEntryById(id);
    scale = 1.0f;
    if (foundEntry != 0) {
        scale = *(f32 *) foundEntry->unmodeled_1558;
    }
    return scale;
}

extern s32 sefSetupPending;
void func_001936F0(void *owner, s32 message, void *argument);
void sefSetSceneMode(s32 mode);

/* Runs the pending setup callback (message 0x1709) when one is pending, then sets scene mode 1. */
void sefFlushPendingEffectSetup(void) {
    if (sefSetupPending != 0) {
        func_001936F0("seft.euc.c", 0x1709, "sefInitEffectCf(void)\244\362ymInitTypeEffect(YMINIT_CF_TYPE)\244\313\312\321\244\250\244\306\244\257\244\300\244\265\244\244");
    }
    sefSetSceneMode(1);
}

extern s32 sefTeardownPending;
void sefResetEffectWorld(void);

/* Runs the pending teardown callback (message 0x1719) once when pending, then resets the effect world. */
void sefFlushPendingEffectTeardown(void) {
    if (sefTeardownPending != 0) {
        func_001936F0("seft.euc.c", 0x1719, "sefDestroyEffectCf(void)\244\362ymDestroyEffect(void)\244\313\312\321\244\250\244\306\244\257\244\300\244\265\244\244");
        sefTeardownPending = 0;
    }
    sefResetEffectWorld();
}

const Matrix4 vu0IdentityMatrix = {
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

void sefAdvanceStepCounter(void);
void sdvExecAlters(void);
void sefUpdateAllEffectGroups(void);

/* One effect step: clears the frame tallies, advances the step counter, runs the alters and sefUpdateAllEffectGroups, then sums the
 * tallies and clears the frame flags. */
void sefRunEffectStep(void)
{
    memset(sefEffectTallies, 0, sizeof(SefEffectTally) * 45);
    sefAdvanceStepCounter();
    sdvExecAlters();
    sefUpdateAllEffectGroups();
    sefSumEffectTallies();
    D_00551084 = 0;
    gSdvHitReceived = 0;
}

extern unsigned char D_004C8390[];

/* The draw-state block at D_004C8390: four callback words that the effect draw path reads. */
typedef struct SefDrawStateBlock {
    unsigned char unmodeled_00[0x20];           /* +0x00..0x1F: not read by this unit */
    s32 draw3DListCallback;                     /* +0x20 */
    unsigned char unmodeled_24[0x2C - 0x24];    /* +0x24..0x2B: not read by this unit */
    s32 draw2DCallback;                         /* +0x2C */
    unsigned char unmodeled_30[0x3C - 0x30];    /* +0x30..0x3B: not read by this unit */
    s32 draw3DCallback;                         /* +0x3C */
    unsigned char unmodeled_40[0x5C - 0x40];    /* +0x40..0x5B: not read by this unit */
    s32 draw3DFlaggedCallback;                  /* +0x5C */
} SefDrawStateBlock;

#define sefDrawStateBlock (*(SefDrawStateBlock *) D_004C8390)

#define SEF_CAMERA_SELECT_FLAGS (*(CameraSelectFlags *) &gCameraSelectFlags)

void sefDrawEffectList3D(void);
void sefDrawEffect2D(void);
void sefDrawEffect3DFlagged(void);
int sefGetDraw3DOverride(void);
extern void pxyGlowPendingSet(void);

/* Installs the effect draw callbacks in the draw-state block. A pending 3D draw (sefGetDraw3DFlag or
 * sefGetDraw3DOverride) also calls pxyGlowPendingSet, and the flagged 3D callback is installed only when cameraSelect is 1. */
void sefInstallDrawCallbacks(void) {
    int cameraSelect;

    sefDrawStateBlock.draw2DCallback = (s32) sefDrawEffect2D;
    sefDrawStateBlock.draw3DListCallback = (s32) sefDrawEffectList3D;
    if (sefGetDraw3DFlag() != 0 || sefGetDraw3DOverride() != 0) {
        pxyGlowPendingSet();
    }
    sefDrawStateBlock.draw3DCallback = (s32) sefDrawEffect3DCallback;
    cameraSelect = SEF_CAMERA_SELECT_FLAGS.cameraSelect;
    if (cameraSelect == 1) {
        sefDrawStateBlock.draw3DFlaggedCallback = (s32) sefDrawEffect3DFlagged;
    }
}

/* Clears the four draw callbacks. */
void sefResetDrawState(void)
{
    sefDrawStateBlock.draw3DFlaggedCallback = 0;
    sefDrawStateBlock.draw3DCallback = 0;
    sefDrawStateBlock.draw3DListCallback = 0;
    sefDrawStateBlock.draw2DCallback = 0;
}

extern int sefDrawEnabled;
s16 sefGetDraw2DOverride(void);
void sefGetCameraPos(Matrix4 *dest);
void svDrawScene2D(void);

/* Draws the 2D effect scene when drawing is enabled and a 2D draw is requested. */
void sefDrawEffect2D(void)
{
    if (sefDrawEnabled != 0) {
        if (sefGetDraw2DFlag() != 0 || sefGetDraw2DOverride() != 0) {
            sefGetCameraPos(sefCameraMatrix);
            svDrawScene2D();
        }
    }
}

void svDrawSceneEffects(void);

/* Draws the 3D effect scene list when drawing is enabled and a 3D draw is requested. */
void sefDrawEffectList3D(void)
{
    if (sefDrawEnabled != 0) {
        if (sefGetDraw3DFlag() != 0 || sefGetDraw3DOverride() != 0) {
            sefGetCameraPos(sefCameraMatrix);
            svDrawSceneEffects();
        }
    }
}

void svDrawEffectRecords3D(void);

/* Draws the 3D effect records; flags 0x40000400 of the game loop runtime reset the draw state instead. */
void sefDrawEffectRecords3D(void) {
    if (gGameLoopRuntimeStorage.flags & 0x40000400) {
        sefResetDrawState();
        return;
    }
    if (sefDrawEnabled != 0) {
        sefGetCameraPos(sefCameraMatrix);
        svDrawEffectRecords3D();
    }
}

s32 sefGetDrawEnabledFlag(void);

/* Draws the 3D effect scene when drawing is enabled and sefGetDrawEnabledFlag is set. */
void sefDrawEffect3D(void) {
    extern s32 sefGetDrawEnabledFlag();
    if ((sefDrawEnabled != 0) && (sefGetDrawEnabledFlag() != 0)) {
        svDrawScene3D();
    }
}

/* Callback of the 3D draw slot: draws the 3D effect scene. */
void sefDrawEffect3DCallback(void) {
    sefDrawEffect3D();
}

extern s32 sefDraw3DForced;

/* Callback of the flagged 3D draw slot: one sefDrawEffect3D call with sefDraw3DForced set. */
void sefDrawEffect3DFlagged(void) {
    sefDraw3DForced = 1;
    sefDrawEffect3D();
    sefDraw3DForced = 0;
}

extern s32 sefSpawnMode;
extern s32 scScriptLookup(s32,s32 *,s32,s32);

#define SEF_PK_REQUEST_KEY (*(PkKey (*))&sefRequestKeyBlock)
#define SEF_CREATE_SCHEDULER ((s32(*)(s32,PkKey *,s32,s32,s32))sefCreateScheduler)

/* Creates a scheduler record for a positive request key: the key is copied into the request block, scScriptLookup gives the
 * slot range it needs, and the new record gets flags 0x140, loses 0x10000000 and gains 0x800000 when sefSpawnMode is
 * 0x3D. Returns the record, or null when nothing was created. */
PkRec *sefCreateScheduledRequest(PkKey *requestKey) {
    s32 count;
    s32 schedulerRangeBase;
    PkRec *record = 0;

    if (requestKey->key > 0) {
        SEF_PK_REQUEST_KEY = *requestKey;
    }
    if (requestKey->key <= 0) {
        return 0;
    }
    schedulerRangeBase = scScriptLookup(SEF_PK_REQUEST_KEY.key, &count, 0, 0);
    if (schedulerRangeBase == 0) {
        return 0;
    }
    schedulerRangeBase = SEF_CREATE_SCHEDULER(schedulerRangeBase + count * 2, &SEF_PK_REQUEST_KEY, 0, schedulerRangeBase, -1);
    if (schedulerRangeBase >= 0) {
        record = &((PkRec *) sefEffectRecords)[schedulerRangeBase];
        record->fl |= 0x140;
        record->fl &= ~0x10000000;
        if (sefSpawnMode == 0x3D) {
            record->fl |= 0x800000;
        }
    }
    return record;
}

/* <<< CHUNK sef_131 */
#define SefTrackKey16 SefTrackKey16_sef_132
/* Row-major 4x4 float matrix. */
#define SEF_TABLE_RECORD_SIZE 0xA70

SefEffectRecord *sefGetScheduler(void);
struct SefLocalData;
struct SefLocalData *sefGetLineAdr(int effectEntryIndex);

#define SEF_LINE_DATA_STRIDE 0x8B0
#define sefCurrentEffectRecord (*(SefEffectRecord **)&sefCurrentEffectRecord)

/* One 0x40-byte slot of the table at gSefDirectionSlots (0x24 slots). activeScale at +0x3C is the in-use value: 0 marks a free
 * slot. */
typedef struct SefEffectTableSlot {
    unsigned char unmodeled_00[0x3C];
    f32 activeScale; /* +0x3C */
} SefEffectTableSlot;

extern SefEffectTableSlot gSefDirectionSlots[];

/* Clears the whole 0x900-byte slot table. */
void sefClearEntryTable(void) {
    memset(gSefDirectionSlots, 0, 0x900U);
}

/* The same table as the linked alias D_004749F0_2037F8. */
extern SefEffectTableSlot D_004749F0_2037F8[];

/* Claims the first free slot (activeScale 0 becomes 1.0) and returns its index, or -1 when all 36 slots are in use. */
s8 sefClaimTableSlot(void) {
    s32 index;

    for (index = 0; index < 36; index++) {
        if (D_004749F0_2037F8[index].activeScale == 0.0f) {
            D_004749F0_2037F8[index].activeScale = 1.0f;
            return index;
        }
    }
    return -1;
}

/* Clears the 0x40-byte slot slotNumber of the slot table; slot numbers from 36 up are ignored. */
void sefClearEntryTableSlot(s8 slotNumber) {
    s8 signedSlotNumber;
    u32 unsignedSlotNumber;

    signedSlotNumber = slotNumber;
    unsignedSlotNumber = signedSlotNumber & 0xFF;
    if (unsignedSlotNumber < 0x24U) {
        memset(&gSefDirectionSlots[signedSlotNumber], 0, 0x40U);
    }
}

/* Packs an effect handle: class 3 is the id with 0x30000000 set; other classes pack class, id and tag. */
s32 sefMakeEffectHandle(s32 handleClass, s32 handleId, s32 handleTag) {
    if (handleClass == 3) {
        return handleId | 0x30000000;
    }
    return (handleClass << 0x1C) | (handleId << 0xC) | handleTag;
}

/* Returns the linked context stored in the request key block. */
SefLinkedContext *sefGetBattleData(void)
{
    return (SefLinkedContext *) sefRequestKeyBlock;
}

/* One 0x270-byte entry of the table at sefPtAllocatorState (the stride the getters step by); data is the array at +0x10. */
#define sefPtAllocatorState (*(SefPtAllocatorEntry **)&sefPtAllocatorState)

/* Returns the byte block of allocator allocatorIndex, or null when the index is out of range. */
unsigned char *sevGetPtAllocator2(int allocatorIndex)
{
    if (allocatorIndex < 0 || allocatorIndex >= sefPtAllocCount) {
        return 0;
    }
    return sefPtAllocatorState[allocatorIndex].allocatorBytes;
}

/* Returns effect slot effectIndex (0x6C bytes each), or null when the index is out of range. */
void *sefGetEffectSlot(s32 effectIndex) {
    if ((effectIndex >= 0) && (effectIndex < sefEffectSlotCount)) {
        return sefEffectSlots + (effectIndex * 0x6C);
    }
    return 0;
}

/* Returns line-data record effectEntryIndex, or null for a negative index. */
SefLocalData *sefGetLineAdr(int lineIndex)
{
    if (lineIndex < 0) {
        return 0;
    }
    return &sefLineDataRecords[lineIndex];
}

extern SefEffectTableSlot D_004749F0_203990[];

/* Returns the table slot at index (a byte below 0x24) when its activeScale is set, else null. */
void *sefGetActiveTableSlot(int index) {
    s8 slotNumber;
    u32 unsignedSlotNumber;
    SefEffectTableSlot *slot;

    slotNumber = index;
    unsignedSlotNumber = slotNumber & 0xFF;
    if (unsignedSlotNumber < 0x24U) {
        if (D_004749F0_203990[slotNumber].activeScale != 0.0f) {
            slot = &D_004749F0_203990[slotNumber];
            return slot;
        }
    }
    return 0;
}

/* Clears the 0x438-byte effect tally array. */
void sefClearEffectTallies(void) {
    memset(sefEffectTallies, 0, 0x438U);
}

/* Returns the base of the effect record table. */
SefEffectRecord *sefGetScheduler(void)
{
    return sefEffectRecords;
}

/* Returns the record most recently stored by sefDestroyRecordEffects (the current record). */
SefEffectRecord *sefGetNowScheduler(void)
{
    return sefCurrentEffectRecord;
}

/* Returns the byte block of allocator allocatorIndex as the parent line, or null for a negative index. */
unsigned char *sefGetParentLine(int checkedIndex, int allocatorIndex)
{
    unsigned char *parentLineBytes;

    if (checkedIndex >= 0 && allocatorIndex >= 0) {
        parentLineBytes = 0;
        if (allocatorIndex < sefPtAllocCount) {
            parentLineBytes = sefPtAllocatorState[allocatorIndex].allocatorBytes;
        }
        return parentLineBytes;
    }
    return 0;
}

#undef SefTrackKey16
#undef sefCurrentEffectRecord
#undef sefPtAllocatorState

/* Sets the slot direction matrix so that sefDefaultAxis points along direction. */
void sefSetSlotDirectionDefaultAxis(f32 (*directionMatrix)[4][4], Vector4 *direction) {
    sefSetSlotDirection(directionMatrix, direction, &sefDefaultAxis);
}

/* Writes into directionMatrix the rotation from sefDefaultAxis to the normalized direction (quaternionFromToRotation) and returns 1;
 * a zero direction leaves the matrix alone and returns 0. */
int sefSetSlotDirectionFromVector(Matrix4 *directionMatrix, const Vector4 *direction) {
    Vector4 normalized;
    int isZeroDirection;

    isZeroDirection = ((*(int *) &direction->z | *(u64 *) direction) == 0);
    if (isZeroDirection == 0) {
        MMathNormalizeVector2(&normalized, direction);
        quaternionFromToRotation(directionMatrix, &sefDefaultAxis, &normalized);
        return 1;
    }
    return 0;
}

/* Sets the slot direction matrix from sefDefaultAxis to the difference minuend - subtrahend. */
void sefSetSlotDirectionBetweenDefaultAxis(void *slotMatrix, Vector4 *minuend, Vector4 *subtrahend) {
    sefSetSlotDirectionBetween(slotMatrix, minuend, subtrahend, &sefDefaultAxis);
}

/* <<< CHUNK tu133 */
void sefMemFillWords(void *dest, int value, int size);
float sefNoiseNext(void);
float sefRandf(void);

/* The pools the .sdata pointers address: effect records, effect slots, line data and the part ring with its entries. */
SefEffectRecord D_0065DB20[40];
unsigned char sefEffectSlots[250 * 0x6C];
unsigned char sefLineDataBlock[170 * 0x8B0];
unsigned char D_006C3E00[0x10 + 1100 * 0x270];
short sefPtFreeSlots[1104];
AlignedVector4 D_0076BFF0;
/* The draw flags the getters return are fields of the last tally. */
SefEffectTally sefEffectTallies[45];
AlignedVector4 D_0076C440;
AlignedVector4 D_0076C450;
AlignedVector4 D_0076C460;
AlignedVector4 D_0076C470;

/* The arc tangent of n / 1024, n = 0..1024; srsAtan2 reads it through the uncached mirror. */
float D_00452D80[1025] = {
    0.0f, 0.000977f, 0.001953f, 0.00293f, 0.003906f, 0.004883f, 0.005859f, 0.006836f,
    0.007812f, 0.008789f, 0.009765f, 0.010742f, 0.011718f, 0.012695f, 0.013671f, 0.014647f,
    0.015624f, 0.0166f, 0.017576f, 0.018553f, 0.019529f, 0.020505f, 0.021481f, 0.022457f,
    0.023433f, 0.024409f, 0.025385f, 0.026361f, 0.027337f, 0.028313f, 0.029288f, 0.030264f,
    0.03124f, 0.032215f, 0.033191f, 0.034166f, 0.035142f, 0.036117f, 0.037092f, 0.038068f,
    0.039043f, 0.040018f, 0.040993f, 0.041968f, 0.042942f, 0.043917f, 0.044892f, 0.045866f,
    0.046841f, 0.047815f, 0.048789f, 0.049764f, 0.050738f, 0.051712f, 0.052686f, 0.053659f,
    0.054633f, 0.055607f, 0.05658f, 0.057554f, 0.058527f, 0.0595f, 0.060473f, 0.061446f,
    0.062419f, 0.063392f, 0.064364f, 0.065337f, 0.066309f, 0.067281f, 0.068253f, 0.069225f,
    0.070197f, 0.071169f, 0.07214f, 0.073112f, 0.074083f, 0.075054f, 0.076025f, 0.076996f,
    0.077967f, 0.078937f, 0.079908f, 0.080878f, 0.081848f, 0.082818f, 0.083788f, 0.084757f,
    0.085727f, 0.086696f, 0.087665f, 0.088634f, 0.089603f, 0.090572f, 0.09154f, 0.092509f,
    0.093477f, 0.094445f, 0.095413f, 0.09638f, 0.097348f, 0.098315f, 0.099282f, 0.100249f,
    0.101215f, 0.102182f, 0.103148f, 0.104114f, 0.10508f, 0.106046f, 0.107012f, 0.107977f,
    0.108942f, 0.109907f, 0.110872f, 0.111836f, 0.1128f, 0.113764f, 0.114728f, 0.115692f,
    0.116655f, 0.117619f, 0.118582f, 0.119544f, 0.120507f, 0.121469f, 0.122431f, 0.123393f,
    0.124355f, 0.125316f, 0.126278f, 0.127239f, 0.128199f, 0.12916f, 0.13012f, 0.13108f,
    0.13204f, 0.132999f, 0.133959f, 0.134918f, 0.135876f, 0.136835f, 0.137793f, 0.138751f,
    0.139709f, 0.140666f, 0.141624f, 0.142581f, 0.143537f, 0.144494f, 0.14545f, 0.146406f,
    0.147361f, 0.148317f, 0.149272f, 0.150227f, 0.151181f, 0.152136f, 0.15309f, 0.154043f,
    0.154997f, 0.15595f, 0.156903f, 0.157855f, 0.158808f, 0.15976f, 0.160711f, 0.161663f,
    0.162614f, 0.163565f, 0.164515f, 0.165465f, 0.166415f, 0.167365f, 0.168314f, 0.169263f,
    0.170212f, 0.17116f, 0.172108f, 0.173056f, 0.174004f, 0.174951f, 0.175898f, 0.176844f,
    0.17779f, 0.178736f, 0.179682f, 0.180627f, 0.181572f, 0.182516f, 0.18346f, 0.184404f,
    0.185348f, 0.186291f, 0.187234f, 0.188177f, 0.189119f, 0.190061f, 0.191002f, 0.191943f,
    0.192884f, 0.193825f, 0.194765f, 0.195705f, 0.196644f, 0.197583f, 0.198522f, 0.199461f,
    0.200399f, 0.201336f, 0.202274f, 0.203211f, 0.204147f, 0.205083f, 0.206019f, 0.206955f,
    0.20789f, 0.208825f, 0.209759f, 0.210693f, 0.211627f, 0.21256f, 0.213493f, 0.214426f,
    0.215358f, 0.216289f, 0.217221f, 0.218152f, 0.219083f, 0.220013f, 0.220943f, 0.221872f,
    0.222801f, 0.22373f, 0.224658f, 0.225586f, 0.226514f, 0.227441f, 0.228367f, 0.229294f,
    0.23022f, 0.231145f, 0.23207f, 0.232995f, 0.233919f, 0.234843f, 0.235767f, 0.23669f,
    0.237612f, 0.238535f, 0.239456f, 0.240378f, 0.241299f, 0.242219f, 0.24314f, 0.244059f,
    0.244979f, 0.245898f, 0.246816f, 0.247734f, 0.248652f, 0.249569f, 0.250486f, 0.251402f,
    0.252318f, 0.253233f, 0.254149f, 0.255063f, 0.255977f, 0.256891f, 0.257804f, 0.258717f,
    0.25963f, 0.260542f, 0.261453f, 0.262364f, 0.263275f, 0.264185f, 0.265095f, 0.266004f,
    0.266913f, 0.267821f, 0.268729f, 0.269637f, 0.270544f, 0.27145f, 0.272357f, 0.273262f,
    0.274167f, 0.275072f, 0.275976f, 0.27688f, 0.277784f, 0.278687f, 0.279589f, 0.280491f,
    0.281392f, 0.282293f, 0.283194f, 0.284094f, 0.284994f, 0.285893f, 0.286791f, 0.28769f,
    0.288587f, 0.289485f, 0.290381f, 0.291278f, 0.292173f, 0.293069f, 0.293963f, 0.294858f,
    0.295752f, 0.296645f, 0.297538f, 0.29843f, 0.299322f, 0.300214f, 0.301105f, 0.301995f,
    0.302885f, 0.303774f, 0.304663f, 0.305552f, 0.30644f, 0.307327f, 0.308214f, 0.3091f,
    0.309986f, 0.310872f, 0.311757f, 0.312641f, 0.313525f, 0.314409f, 0.315291f, 0.316174f,
    0.317056f, 0.317937f, 0.318818f, 0.319698f, 0.320578f, 0.321458f, 0.322336f, 0.323215f,
    0.324092f, 0.32497f, 0.325846f, 0.326723f, 0.327598f, 0.328474f, 0.329348f, 0.330222f,
    0.331096f, 0.331969f, 0.332842f, 0.333714f, 0.334585f, 0.335456f, 0.336327f, 0.337197f,
    0.338066f, 0.338935f, 0.339803f, 0.340671f, 0.341538f, 0.342405f, 0.343271f, 0.344137f,
    0.345002f, 0.345867f, 0.346731f, 0.347594f, 0.348457f, 0.34932f, 0.350182f, 0.351043f,
    0.351904f, 0.352764f, 0.353624f, 0.354483f, 0.355342f, 0.3562f, 0.357057f, 0.357914f,
    0.358771f, 0.359627f, 0.360482f, 0.361337f, 0.362191f, 0.363045f, 0.363898f, 0.36475f,
    0.365602f, 0.366454f, 0.367305f, 0.368155f, 0.369005f, 0.369854f, 0.370703f, 0.371551f,
    0.372398f, 0.373245f, 0.374092f, 0.374938f, 0.375783f, 0.376628f, 0.377472f, 0.378316f,
    0.379159f, 0.380001f, 0.380843f, 0.381684f, 0.382525f, 0.383365f, 0.384205f, 0.385044f,
    0.385883f, 0.386721f, 0.387558f, 0.388395f, 0.389231f, 0.390067f, 0.390902f, 0.391736f,
    0.39257f, 0.393403f, 0.394236f, 0.395068f, 0.3959f, 0.396731f, 0.397562f, 0.398391f,
    0.399221f, 0.40005f, 0.400878f, 0.401705f, 0.402532f, 0.403359f, 0.404184f, 0.40501f,
    0.405834f, 0.406658f, 0.407482f, 0.408305f, 0.409127f, 0.409949f, 0.41077f, 0.41159f,
    0.41241f, 0.41323f, 0.414049f, 0.414867f, 0.415684f, 0.416501f, 0.417318f, 0.418134f,
    0.418949f, 0.419764f, 0.420578f, 0.421391f, 0.422204f, 0.423016f, 0.423828f, 0.424639f,
    0.42545f, 0.42626f, 0.427069f, 0.427878f, 0.428686f, 0.429493f, 0.4303f, 0.431107f,
    0.431912f, 0.432717f, 0.433522f, 0.434326f, 0.435129f, 0.435932f, 0.436734f, 0.437536f,
    0.438337f, 0.439137f, 0.439937f, 0.440736f, 0.441534f, 0.442332f, 0.44313f, 0.443926f,
    0.444722f, 0.445518f, 0.446313f, 0.447107f, 0.447901f, 0.448694f, 0.449486f, 0.450278f,
    0.45107f, 0.45186f, 0.45265f, 0.45344f, 0.454229f, 0.455017f, 0.455805f, 0.456592f,
    0.457378f, 0.458164f, 0.458949f, 0.459734f, 0.460518f, 0.461301f, 0.462084f, 0.462866f,
    0.463648f, 0.464429f, 0.465209f, 0.465989f, 0.466768f, 0.467546f, 0.468324f, 0.469101f,
    0.469878f, 0.470654f, 0.47143f, 0.472204f, 0.472979f, 0.473752f, 0.474525f, 0.475298f,
    0.476069f, 0.47684f, 0.477611f, 0.478381f, 0.47915f, 0.479919f, 0.480687f, 0.481454f,
    0.482221f, 0.482988f, 0.483753f, 0.484518f, 0.485283f, 0.486046f, 0.486809f, 0.487572f,
    0.488334f, 0.489095f, 0.489856f, 0.490616f, 0.491375f, 0.492134f, 0.492893f, 0.49365f,
    0.494407f, 0.495164f, 0.495919f, 0.496674f, 0.497429f, 0.498183f, 0.498936f, 0.499689f,
    0.500441f, 0.501192f, 0.501943f, 0.502693f, 0.503443f, 0.504192f, 0.50494f, 0.505688f,
    0.506435f, 0.507181f, 0.507927f, 0.508673f, 0.509417f, 0.510161f, 0.510905f, 0.511647f,
    0.512389f, 0.513131f, 0.513872f, 0.514612f, 0.515352f, 0.516091f, 0.516829f, 0.517567f,
    0.518304f, 0.519041f, 0.519777f, 0.520512f, 0.521247f, 0.521981f, 0.522715f, 0.523447f,
    0.52418f, 0.524911f, 0.525642f, 0.526373f, 0.527102f, 0.527832f, 0.52856f, 0.529288f,
    0.530015f, 0.530742f, 0.531468f, 0.532193f, 0.532918f, 0.533642f, 0.534366f, 0.535089f,
    0.535811f, 0.536533f, 0.537254f, 0.537975f, 0.538694f, 0.539414f, 0.540132f, 0.54085f,
    0.541568f, 0.542284f, 0.543001f, 0.543716f, 0.544431f, 0.545145f, 0.545859f, 0.546572f,
    0.547284f, 0.547996f, 0.548707f, 0.549418f, 0.550128f, 0.550837f, 0.551546f, 0.552254f,
    0.552962f, 0.553668f, 0.554375f, 0.55508f, 0.555785f, 0.55649f, 0.557194f, 0.557897f,
    0.558599f, 0.559301f, 0.560003f, 0.560703f, 0.561403f, 0.562103f, 0.562802f, 0.5635f,
    0.564198f, 0.564895f, 0.565591f, 0.566287f, 0.566982f, 0.567676f, 0.56837f, 0.569064f,
    0.569756f, 0.570449f, 0.57114f, 0.571831f, 0.572521f, 0.573211f, 0.5739f, 0.574588f,
    0.575276f, 0.575963f, 0.57665f, 0.577336f, 0.578021f, 0.578706f, 0.57939f, 0.580073f,
    0.580756f, 0.581439f, 0.58212f, 0.582801f, 0.583482f, 0.584162f, 0.584841f, 0.58552f,
    0.586198f, 0.586875f, 0.587552f, 0.588228f, 0.588903f, 0.589578f, 0.590253f, 0.590927f,
    0.5916f, 0.592272f, 0.592944f, 0.593615f, 0.594286f, 0.594956f, 0.595626f, 0.596295f,
    0.596963f, 0.597631f, 0.598298f, 0.598964f, 0.59963f, 0.600295f, 0.60096f, 0.601624f,
    0.602287f, 0.60295f, 0.603612f, 0.604274f, 0.604935f, 0.605595f, 0.606255f, 0.606914f,
    0.607573f, 0.608231f, 0.608888f, 0.609545f, 0.610201f, 0.610857f, 0.611512f, 0.612166f,
    0.61282f, 0.613473f, 0.614126f, 0.614778f, 0.615429f, 0.61608f, 0.61673f, 0.61738f,
    0.618029f, 0.618677f, 0.619325f, 0.619972f, 0.620619f, 0.621265f, 0.62191f, 0.622555f,
    0.623199f, 0.623843f, 0.624486f, 0.625128f, 0.62577f, 0.626411f, 0.627052f, 0.627692f,
    0.628332f, 0.62897f, 0.629609f, 0.630246f, 0.630883f, 0.63152f, 0.632156f, 0.632791f,
    0.633426f, 0.63406f, 0.634694f, 0.635327f, 0.635959f, 0.636591f, 0.637222f, 0.637852f,
    0.638482f, 0.639112f, 0.639741f, 0.640369f, 0.640996f, 0.641623f, 0.64225f, 0.642876f,
    0.643501f, 0.644126f, 0.64475f, 0.645373f, 0.645996f, 0.646619f, 0.647241f, 0.647862f,
    0.648482f, 0.649102f, 0.649722f, 0.650341f, 0.650959f, 0.651577f, 0.652194f, 0.65281f,
    0.653426f, 0.654042f, 0.654657f, 0.655271f, 0.655884f, 0.656497f, 0.65711f, 0.657722f,
    0.658333f, 0.658944f, 0.659554f, 0.660164f, 0.660773f, 0.661381f, 0.661989f, 0.662596f,
    0.663203f, 0.663809f, 0.664415f, 0.66502f, 0.665624f, 0.666228f, 0.666831f, 0.667434f,
    0.668036f, 0.668638f, 0.669239f, 0.669839f, 0.670439f, 0.671038f, 0.671637f, 0.672235f,
    0.672833f, 0.67343f, 0.674026f, 0.674622f, 0.675217f, 0.675812f, 0.676406f, 0.677f,
    0.677593f, 0.678185f, 0.678777f, 0.679368f, 0.679959f, 0.680549f, 0.681139f, 0.681728f,
    0.682317f, 0.682904f, 0.683492f, 0.684079f, 0.684665f, 0.685251f, 0.685836f, 0.68642f,
    0.687005f, 0.687588f, 0.688171f, 0.688753f, 0.689335f, 0.689916f, 0.690497f, 0.691077f,
    0.691657f, 0.692236f, 0.692814f, 0.693392f, 0.693969f, 0.694546f, 0.695122f, 0.695698f,
    0.696273f, 0.696848f, 0.697422f, 0.697995f, 0.698568f, 0.699141f, 0.699712f, 0.700284f,
    0.700854f, 0.701425f, 0.701994f, 0.702563f, 0.703132f, 0.7037f, 0.704267f, 0.704834f,
    0.7054f, 0.705966f, 0.706532f, 0.707096f, 0.70766f, 0.708224f, 0.708787f, 0.70935f,
    0.709912f, 0.710473f, 0.711034f, 0.711594f, 0.712154f, 0.712713f, 0.713272f, 0.71383f,
    0.714388f, 0.714945f, 0.715502f, 0.716058f, 0.716613f, 0.717168f, 0.717723f, 0.718277f,
    0.71883f, 0.719383f, 0.719935f, 0.720487f, 0.721038f, 0.721589f, 0.722139f, 0.722689f,
    0.723238f, 0.723786f, 0.724334f, 0.724882f, 0.725429f, 0.725975f, 0.726521f, 0.727066f,
    0.727611f, 0.728156f, 0.728699f, 0.729243f, 0.729785f, 0.730328f, 0.730869f, 0.731411f,
    0.731951f, 0.732491f, 0.733031f, 0.73357f, 0.734109f, 0.734646f, 0.735184f, 0.735721f,
    0.736257f, 0.736793f, 0.737329f, 0.737864f, 0.738398f, 0.738932f, 0.739465f, 0.739998f,
    0.74053f, 0.741062f, 0.741593f, 0.742124f, 0.742654f, 0.743184f, 0.743713f, 0.744242f,
    0.74477f, 0.745298f, 0.745825f, 0.746352f, 0.746878f, 0.747403f, 0.747928f, 0.748453f,
    0.748977f, 0.749501f, 0.750024f, 0.750546f, 0.751068f, 0.75159f, 0.752111f, 0.752631f,
    0.753151f, 0.753671f, 0.75419f, 0.754708f, 0.755226f, 0.755744f, 0.756261f, 0.756777f,
    0.757293f, 0.757809f, 0.758324f, 0.758838f, 0.759352f, 0.759865f, 0.760378f, 0.760891f,
    0.761403f, 0.761914f, 0.762425f, 0.762936f, 0.763446f, 0.763955f, 0.764464f, 0.764973f,
    0.76548f, 0.765988f, 0.766495f, 0.767001f, 0.767507f, 0.768013f, 0.768518f, 0.769022f,
    0.769526f, 0.77003f, 0.770533f, 0.771036f, 0.771538f, 0.772039f, 0.77254f, 0.773041f,
    0.773541f, 0.774041f, 0.77454f, 0.775038f, 0.775537f, 0.776034f, 0.776531f, 0.777028f,
    0.777524f, 0.77802f, 0.778515f, 0.77901f, 0.779504f, 0.779998f, 0.780491f, 0.780984f,
    0.781477f, 0.781968f, 0.78246f, 0.782951f, 0.783441f, 0.783931f, 0.784421f, 0.78491f,
    0.785398f,
};
unsigned short D_00453D88[32] = {
    0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B,
    0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112, 0x113,
    0x114, 0x115, 0x116, 0x117, 0x118, 0x119, 0x11A, 0x11B,
    0x11C, 0x11D, 0x11E, 0x11F, 0x0, 0x0, 0x0, 0x0
};
unsigned short D_00453DC8[20] = {
    0x106, 0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x10E,
    0x10F, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116,
    0x0, 0x0, 0x0, 0x0
};
unsigned int D_00453DF0[5] = { 0x136004, 0x136001, 0x136003, 0x136002, 0x136005 };
SefCameraMatrixBlock sefCameraMatrixBlock = { 0 };
/* The frame step in milliseconds, in seconds, scaled and for the tracks (gSdvFrameDelta and sefTrackFrameStep are its last words). */
unsigned char sefKeyframeStepBlock[16] = { 0 };
/* The request key block (sevInitPtAllocator clears its 736 bytes). */
int sefRequestKeyBlock[184] = { 0 };
SefEntryTableShort sefEffectEntryTable = { 0 };
unsigned char sefGroupTimers[256] = { 0 };
SefMotionFloorView sefMotionFloorCurve = { 0 };
s32 D_004749E0[2] = { 0 };
int D_004749E8[2] = { 0 };
SefEffectTableSlot gSefDirectionSlots[36] = { 0 };
unsigned char D_004752F0[144] = { 0 };
s32 D_00551040 = 0;
s32 sefVectorMode = 0;
s32 sefDraw3DForced = 0;
SefEffectRecord *sefEffectRecords = D_0065DB20;
s32 D_00551050 = 0;
void *D_00551054 = sefEffectSlots;
s32 D_00551058 = 0;
SefLocalData *sefLineDataRecords = (SefLocalData *) sefLineDataBlock;
SefPtRingControl *sefPtAllocatorState = (SefPtRingControl *) D_006C3E00;
int sefEffectRecordCount = 40;
int sefEffectSlotCount = 250;
int sefLineDataCount = 170;
int sefPtAllocCount = 1100;
unsigned short D_00551074 = 0xFFFF;
unsigned short D_00551076 = 0xFFFF;
s32 sefActiveLineIndex = -1;
int sefDrawEnabled = 0;
int D_00551080 = 0;
int D_00551084 = 0;
int gSdvHitReceived = 0;
float *srsAtanTable = (float *) ((char *) D_00452D80 + 0x20000000); /* uncached */
s32 sefSetupPending = 1;
s32 sefTeardownPending = 1;
SefEffectEntryRecord *sefCurrentEffectRecord = 0;
int D_0055109C = 0;
s32 D_005510A0 = 0;
SefMotionObject *D_005510A4 = 0;
int D_005510A8 = 0;
