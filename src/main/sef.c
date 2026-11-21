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

int sefGetDraw2DFlag(void) {
    return sefDraw2DFlag[0];
}

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
typedef struct SefHermiteKey {
    unsigned char unmodeled_00[12];
    short key;                                  /* +12 */
    signed char nextKeyIndex;                            /* +14 */
    unsigned char unmodeled_0F;
} SefHermiteKey;
typedef struct SefVectorTrackKey {
    unsigned char unmodeled_0000[0xC];
    short key;                                  /* +0x0C */
    signed char nextKeyIndex;                           /* +0x0E */
    unsigned char unmodeled_000F[0x1];
} SefVectorTrackKey;
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
typedef struct SefEffectRecord {
    unsigned char unmodeled_00[0x08];
    short entryId;                              /* +0x08 */
} SefEffectRecord;
typedef struct SefEntryTableShort {
    SefEntry entries[24];
    unsigned char unmodeled_20700[0x10];
    int entryCount;                             /* +0x20710 */
} SefEntryTableShort;
typedef struct SefPtAllocatorEntry {
    unsigned char unmodeled_00[0x10];
    unsigned char allocatorBytes[0x270 - 0x10];
} SefPtAllocatorEntry;
typedef unsigned int Quadword __attribute__((mode(TI)));
typedef union SefQuadwordView {
    Quadword quadword;
    unsigned int wordLanes[4];
} SefQuadwordView;
typedef struct SefNoiseTable {
    f32 samples[1024];
    unsigned char unmodeled_1000[12];
    int sampleCursor;
} SefNoiseTable;
typedef struct SefLinkedObjectRecord {
    unsigned char unmodeled_000[0x155C];
    unsigned char flags;             /* +0x155C */
} SefLinkedObjectRecord;
typedef struct SefPartPlacement {
    Matrix4 matrix;                  /* +0x000 */
    unsigned char unmodeled_040[0xA0 - 0x40];
    Vector4 translation;             /* +0x0A0 */
    unsigned char unmodeled_0B0[0xC0 - 0xB0];
    Vector4 scale;                   /* +0x0C0 */
    unsigned char unmodeled_0D0[0x200 - 0xD0];
    Vector4 point;                   /* +0x200 */
} SefPartPlacement;
typedef struct SefGlobalTransform {
    Matrix4 matrix;                  /* +0x00 */
    Vector4 position;                /* +0x40 */
    float scale;                     /* +0x50 */
    short scaled;                    /* +0x54: nonzero when scale applies */
    short kind;                      /* +0x56 */
} SefGlobalTransform;
typedef struct SefGroupTimer {
    float center[4];               /* +0x00: x, y, z, w of the group centre */
    int owner;                     /* +0x10: owner id compared by sefGetGroupCenter and sefRegisterGroupCenter */
    short remaining;               /* +0x14: frames left; zero or less means the entry is free */
    unsigned short key;            /* +0x16: second id of the group, compared as an unsigned short */
    unsigned char unmodeled_18[8]; /* +0x18..0x1F: not read by the code in this unit */
} SefGroupTimer;
typedef signed char s8;
typedef int s32;
extern s32 sefDrawEnabledFlag[];

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
extern unsigned char sefRequestKeyBlock[];

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

SefKeyframeEntry *sefAdvanceKeyframeCursor(SefKeyframeEntry *keyTable, SefTrackCursor *cursor)
{
    f32 jumpTime;
    f32 frameTime;
    s32 linkedIndex;
    SefKeyframeEntry *currentKey;

    frameTime = cursor->time + sefStepTable.frameStep;
    currentKey = &keyTable[cursor->keyIndex];
    cursor->time = frameTime;
    if ((frameTime < 4096.0f) && (currentKey[1].time <= frameTime)) {
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

SefVectorTrackKey *sefAdvanceVectorKeyframeCursor(SefVectorTrackKey *keyTable, SefTrackCursor *cursor) {
    f32 frameTime;
    s32 linkedIndex;
    SefVectorTrackKey *currentKey;

    frameTime = cursor->time + *sefTrackFrameStep;
    currentKey = &keyTable[cursor->keyIndex];
    cursor->time = frameTime;
    if ((frameTime < 4096.0f) && (currentKey[1].key <= frameTime)) {
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

SefTrackKey *sefAdvanceTrackCursor(SefTrackKey *keys, SefTrackCursor *cursor)
{
    f32 time;
    int frame;
    SefTrackKey *key;
    short nextKeyIndex;

    time = cursor->time + sefTrackFrameStep[0];
    key = &keys[cursor->keyIndex];
    frame = time;
    cursor->time = time;
    if (frame < SEF_TRACK_END_KEY && key->key != SEF_TRACK_END_KEY && frame >= (key + 1)->key) {
        key++;
        nextKeyIndex = (short)key->nextKeyIndex;
        if (nextKeyIndex >= 0) {
            cursor->keyIndex = key->nextKeyIndex;
            key = &keys[nextKeyIndex];
            cursor->time = key->key;
        } else {
            cursor->keyIndex = (u16) cursor->keyIndex + 1;
        }
    }
    return key;
}

/* One 16-byte key of a vector track: the key time is the short at +0x0C and the link the signed byte at +0x0E. */







SefVectorTrackKey *sefAdvanceVectorTrackCursor(SefVectorTrackKey *keys, SefTrackCursor *cursor)
{
    f32 time;
    int frame;
    SefVectorTrackKey *key;
    signed char nextKeyIndex;

    time = cursor->time + sefTrackFrameStep[0];
    key = &keys[cursor->keyIndex];
    frame = time;
    cursor->time = time;
    if (frame < SEF_TRACK_END_KEY && key->key != SEF_TRACK_END_KEY && frame >= (key + 1)->key) {
        key++;
        nextKeyIndex = key->nextKeyIndex;
        if (nextKeyIndex >= 0) {
            cursor->keyIndex = nextKeyIndex;
            key = &keys[nextKeyIndex];
            cursor->time = key->key;
        } else {
            cursor->keyIndex = (u16) cursor->keyIndex + 1;
        }
    }
    return key;
}

/* Number of point handles in one local effect record. */
#define SEF_LINE_PARTICLE_COUNT 0x100


/* The fields of an effect slot that the slot init and destroy functions of this unit touch: the id,
 * the flags word (bit 31 cleared on init), the kind and the line count. */









/* One 16-byte entry of the table that MMathHermiteKeyNeighbors reads; the names are provisional until the callers give them. */







void *memset(void *destination, int value, unsigned int size);
void sefFreeEffectData(SefEffectSlot *slot, int index);

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
    if (keys[1].key == 4096) {
        nextKey = keys;
        afterNextKey = keys;
    } else if (keys[1].nextKeyIndex >= 0) {
        afterNextKey = &keys[keys[1].nextKeyIndex];
    }
    if (keys[2].key == 4096 && keys[1].nextKeyIndex < 0) {
        afterNextKey = nextKey;
    }
    *outPreviousKey = previousKey;
    *outCurrentKey = keys;
    *outNextKey = nextKey;
    *outAfterNextKey = afterNextKey;
}

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

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FA338);

/* One 16-byte key of a vector track: the key time is the short at +0x0C and the link the signed byte at +0x0E. */







/* A colour track's cursor: the track cursor, then the four integer colour lanes written at +0x10. */






/* Samples a colour track at the cursor's time: the two keys the cursor spans are interpolated per lane,
 * scaled by the track's colour, clamped to 255 and stored as integers in the cursor's colour lanes. */
void sefSampleColorTrack(SefVectorTrackKey *keys, SefColorTrackCursor *cursor, const Vector4 *scale)
{
    SefVectorTrackKey *key;
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

void sefSetSlotDirectionBetween(void *slotMatrix, Vector4 *minuend, Vector4 *subtrahend, const Vector4 *axis) {
    Vector4 delta;

    Vu0SefSubVectorXYZ(&delta, minuend, subtrahend);
    sefSetSlotDirection(slotMatrix, &delta, axis);
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

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FD000);

/* The count of local effect records in use slots: the first free one is claimed and its index returned. */
extern int sefLineDataCount;

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











/* One 32-byte image packet. */











/* The image mapper entry read through svGetImageListItem (owner: sv_get_image_item_142.c). */














/* The current effect record (sefCurrentEffectRecord): only the entry id at +0x08 is read here. */





extern SefEffectRecord *sefCurrentEffectRecord;
SvTypeListEntry *svGetImageListItem(int type, unsigned int flags);

/* The animation object: its record list at +0x17C, the 30 image packets at +0x1A0 and the 8 alternate
 * packets after them at +0x560. */
typedef struct SefAnimObject {
    unsigned char unmodeled_0000[0x17C];
    SefAnalyzeRecord *animRecordTable;               /* +0x17C */
    unsigned char unmodeled_0180[0x1A0 - 0x180];
    SefImagePacket packets[SEF_IMAGE_PACKET_COUNT]; /* +0x1A0 */
    SefImagePacket altPackets[SEF_ALT_PACKET_COUNT]; /* +0x560 */
} SefAnimObject;

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

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FDBE0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FE030);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FE6F0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FF1A0);

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

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FF7B0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FF838);

SefLocalData *sefAllocEffectData(SefEffectSlot *effectSlot, int tag)
{
    short *lineHandlePtr;
    int lineSlot;
    int lineHandle;

    lineSlot = effectSlot->lineCount;
    if ((unsigned int)lineSlot >= 32) {
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

void sefFreeEffectData(SefEffectSlot *effectSlot, int lineSlot)
{
    short *lineHandlePtr;

    if ((unsigned int)lineSlot < 32) {
        lineHandlePtr = &effectSlot->lineHandles[lineSlot];
        if (*lineHandlePtr >= 0) {
            sefFreeLineData(*lineHandlePtr);
            *lineHandlePtr = -1;
        }
    }
}

void sefSetEntryFlagsByKey(int key, int flags);
void sefClearEntryFlagsByKey(int key, int flags);

void sefApplyRequestKeyBits(int wordMask, int flagBits, int setBits)
{
    void (*applyBits)(int, int);
    int *requestKeys;

    requestKeys = (int *)sefRequestKeyBlock;
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

INCLUDE_ASM("asm/main/nonmatchings/sef", func_001FFA50);

/* The line data pool: the splat label of the pool, owned by the bss of main/tu127. */
extern unsigned char sefEffectSlots[];

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

int sefCreateScheduler(int tableId, int scheduleRequest, int scheduleOffset, int loadTag, int reuseIndex)
{
    return func_002010C8(tableId, scheduleRequest, scheduleOffset, loadTag, 0, reuseIndex, 1, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201508);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201690);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002017E0);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00201808);

extern const Matrix4 MMathUnitMatrix;
void func_00201808(void *destination, const void *source);

void sefSetMatrixOrUnit(void *destinationMatrix, const void *sourceMatrix) {
    if (sourceMatrix != 0) {
        func_00201808(destinationMatrix, sourceMatrix);
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






extern SefEntryTableShort sefEffectEntryTable;

void sefSetEntryFlagsByKey(int key, int flagBits)
{
    SefEntry *foundEntry;
    int effectEntryIndex;

    if (key == 0) {
        return;
    }
    effectEntryIndex = 0;
    if (sefEffectEntryTable.entryCount <= 0) {
        return;
    }
    if (sefEffectEntryTable.entries[0].id == key) {
        sefEffectEntryTable.entries[0].effectEntryFlags |= flagBits;
        return;
    }
nextCandidate:
    effectEntryIndex++;
candidateCheck:
    if (effectEntryIndex >= sefEffectEntryTable.entryCount) {
        return;
    }
    foundEntry = &sefEffectEntryTable.entries[effectEntryIndex];
    if (foundEntry->id != key) {
        goto nextCandidate;
    }
    foundEntry->effectEntryFlags |= flagBits;
}

void sefClearEntryFlagsByKey(int key, int flagBits)
{
    SefEntry *foundEntry;
    int effectEntryIndex;

    if (key == 0) {
        return;
    }
    foundEntry = &sefEffectEntryTable.entries[0];
    effectEntryIndex = 0;
    if (sefEffectEntryTable.entryCount <= 0) {
        return;
    }
    if (foundEntry->id != key) {
        goto candidateLoop;
        do {
            foundEntry = 0;
candidateLoop:
            effectEntryIndex++;
            if (effectEntryIndex >= sefEffectEntryTable.entryCount) {
                return;
            }
            foundEntry = &sefEffectEntryTable.entries[effectEntryIndex];
        } while (foundEntry->id != key);
    }
    foundEntry->effectEntryFlags &= ~flagBits;
}

int tiMRPackageGetNodeMatrix(Vector4 *block, void *arg);
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
extern const Matrix4 MMathUnitMatrix;

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

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203048);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002030E8);

SefEntry *sefFindEntryById(int key)
{
    SefEntryTableShort *effectTable;
    SefEntry *foundEntry;
    int i;

    if (key != 0) {
        effectTable = &sefEffectEntryTable;
        for (i = 0; i < effectTable->entryCount; i++) {
            foundEntry = &effectTable->entries[i];
            if (foundEntry->id == key) {
                return foundEntry;
            }
        }
    }
    return 0;
}

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

extern unsigned char sefEffectOwner[];
extern unsigned char sefSetupArg[];
extern s32 sefSetupPending;
void func_001936F0(void *owner, s32 message, void *argument);
void func_001E8CB8(s32 mode);

void sefFlushPendingEffectSetup(void) {
    if (sefSetupPending != 0) {
        func_001936F0(sefEffectOwner, 0x1709, sefSetupArg);
    }
    func_001E8CB8(1);
}

extern unsigned char sefTeardownArg[];
extern s32 sefTeardownPending;
void func_001E8E70(void);

void sefFlushPendingEffectTeardown(void) {
    if (sefTeardownPending != 0) {
        func_001936F0(sefEffectOwner, 0x1719, sefTeardownArg);
        sefTeardownPending = 0;
    }
    func_001E8E70();
}

void sefAdvanceStepCounter(void);
void sdvExecAlters(void);
void func_001F8680(void);
extern int D_00551084;
extern int gSdvHitReceived;

void sefRunEffectStep(void)
{
    memset(sefEffectTallies, 0, sizeof(SefEffectTally) * 45);
    sefAdvanceStepCounter();
    sdvExecAlters();
    func_001F8680();
    sefSumEffectTallies();
    D_00551084 = 0;
    gSdvHitReceived = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203450);

extern unsigned char D_004C8390[];

/* The block D_004C8390 as far as this unit reads it: four words cleared by sefResetDrawState; the rest is not read here. */
typedef struct SefDrawStateBlock {
    unsigned char unmodeled_00[0x20];           /* +0x00..0x1F: not read by this unit */
    s32 word20;                                 /* +0x20 */
    unsigned char unmodeled_24[0x2C - 0x24];    /* +0x24..0x2B: not read by this unit */
    s32 word2C;                                 /* +0x2C */
    unsigned char unmodeled_30[0x3C - 0x30];    /* +0x30..0x3B: not read by this unit */
    s32 word3C;                                 /* +0x3C */
    unsigned char unmodeled_40[0x5C - 0x40];    /* +0x40..0x5B: not read by this unit */
    s32 word5C;                                 /* +0x5C */
} SefDrawStateBlock;
#define sefDrawStateBlock (*(SefDrawStateBlock *) D_004C8390)

void sefResetDrawState(void)
{
    sefDrawStateBlock.word5C = 0;
    sefDrawStateBlock.word3C = 0;
    sefDrawStateBlock.word20 = 0;
    sefDrawStateBlock.word2C = 0;
}

extern int sefDrawEnabled;
s16 sefGetDraw2DOverride(void);
void sefGetCameraPos(Matrix4 *dest);
void svDrawScene2D(void);

void sefDrawEffect2D(void)
{
    if (sefDrawEnabled != 0) {
        if (sefGetDraw2DFlag() != 0 || sefGetDraw2DOverride() != 0) {
            sefGetCameraPos(sefCameraMatrix);
            svDrawScene2D();
        }
    }
}

int sefGetDraw3DOverride(void);
void svDrawSceneEffects(void);

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

void sefDrawEffect3D(void) {
    extern s32 sefGetDrawEnabledFlag();
    if ((sefDrawEnabled != 0) && (sefGetDrawEnabledFlag() != 0)) {
        svDrawScene3D();
    }
}

void sefDrawEffect3DCallback(void) {
    sefDrawEffect3D();
}

extern s32 sefDraw3DForced;

void sefDrawEffect3DFlagged(void) {
    sefDraw3DForced = 1;
    sefDrawEffect3D();
    sefDraw3DForced = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203690);

#undef SefEffectRecord
/* <<< CHUNK sef_131 */

/* >>> CHUNK sef_132 */
#define SefTrackKey16 SefTrackKey16_sef_132

/* Row-major 4x4 float matrix. */

#define SEF_TABLE_RECORD_SIZE 0xA70

SefEffectRecord *sefGetScheduler(void);

struct SefLocalData;
struct SefRecordOwner;
struct SefLocalData *sefGetLineAdr(int entryIndex);

/* Particles per line-data record. */
#define SEF_LINE_PARTICLE_COUNT 0x100
#define SEF_LINE_DATA_STRIDE 0x8B0

/* The request buffer: 0x340 bytes (SefRequestBuffer view); +0x100 (0x8046F0) and sefRequestParentMatrixPointer (+0x300) are
 * labels inside it. */
extern unsigned char sefRequestKeyBlock[0x340];

typedef struct SefTrackKey16 {
    float value[3];                                  /* +0x00 */
    short key;
    signed char nextKeyIndex;
    unsigned char unmodeled_00f[1];
} SefTrackKey16;

#define sefCurrentEffectRecord (*(SefEffectRecord **)&sefCurrentEffectRecord)
extern SefLocalData *sefLineDataRecords;
extern SefEffectRecord *sefEffectRecords;

/* One 0x40-byte slot of the table at D_004749F0 (0x24 slots, cleared by sefClearEntryTable and sefClearEntryTableSlot). */
typedef struct SefTableSlot {
    unsigned char unmodeled_00[0x40];
} SefTableSlot;

extern SefTableSlot D_004749F0[];

void sefClearEntryTable(void) {
    memset(D_004749F0, 0, 0x900U);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002037F8);

void sefClearEntryTableSlot(s8 slotNumber) {
    s8 signedSlotNumber;
    u32 unsignedSlotNumber;

    signedSlotNumber = slotNumber;
    unsignedSlotNumber = signedSlotNumber & 0xFF;
    if (unsignedSlotNumber < 0x24U) {
        memset(&D_004749F0[signedSlotNumber], 0, 0x40U);
    }
}

s32 sefMakeEffectHandle(s32 handleClass, s32 handleId, s32 handleTag) {
    if (handleClass == 3) {
        return handleId | 0x30000000;
    }
    return (handleClass << 0x1C) | (handleId << 0xC) | handleTag;
}

SefLinkedContext *sefGetBattleData(void)
{
    return (SefLinkedContext *) sefRequestKeyBlock;
}

/* One 0x270-byte entry of the table at sefPtAllocatorState (the stride the getters step by); data is the array at +0x10. */





#define sefPtAllocatorState (*(SefPtAllocatorEntry **)&sefPtAllocatorState)
extern int sefPtAllocCount;

unsigned char *sevGetPtAllocator2(int allocatorIndex)
{
    if (allocatorIndex < 0 || allocatorIndex >= sefPtAllocCount) {
        return 0;
    }
    return sefPtAllocatorState[allocatorIndex].allocatorBytes;
}

extern s32 sefEffectSlotCount;

void *sefGetEffectSlot(s32 effectIndex) {
    if ((effectIndex >= 0) && (effectIndex < sefEffectSlotCount)) {
        return sefEffectSlots + (effectIndex * 0x6C);
    }
    return 0;
}

/* Returns line-data record entryIndex, or null for a negative index. */
SefLocalData *sefGetLineAdr(int lineIndex)
{
    if (lineIndex < 0) {
        return 0;
    }
    return &sefLineDataRecords[lineIndex];
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203990);

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
/* <<< CHUNK sef_132 */

/* >>> CHUNK tu133 */

extern Vector4 sefDefaultAxis;

void sefSetSlotDirectionDefaultAxis(f32 (*directionMatrix)[4][4], Vector4 *direction) {
    sefSetSlotDirection(directionMatrix, direction, &sefDefaultAxis);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00203A88);

void sefSetSlotDirectionBetweenDefaultAxis(void *slotMatrix, Vector4 *minuend, Vector4 *subtrahend) {
    sefSetSlotDirectionBetween(slotMatrix, minuend, subtrahend, &sefDefaultAxis);
}

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
extern const Vector4 sefDefaultPoint;






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






extern SefNoiseTable sefNoiseTable;
/* Single-precision 2*pi, read from the scaffold literal sefTwoPi of this unit. */
extern f32 sefTwoPi;
extern unsigned int sefRandVectorSeeds[4];
extern unsigned int sefRandVectorIncrements[4];
extern f32 sefCircleAngleScale;

/* Stores value into each of the size / 4 words starting at dest. */
void sefMemFillWords(void *fillMemory, int fillPattern, int fillByteLength) {
    int *wordCursor;
    int wordsLeft;

    wordCursor = fillMemory;
    wordsLeft = fillByteLength >> 2;
    if (wordsLeft > 0) {
        do {
            wordsLeft -= 1;
            *wordCursor = fillPattern;
            wordCursor += 1;
        } while (wordsLeft != 0);
    }
}

void sefMemZero(void *zeroedMemory, int byteLength)
{
    SefQuadwordView *quadwordBase = zeroedMemory;
    int quadwordCount = byteLength >> 4;
    int tailWordCount = (byteLength & 0xf) >> 2;
    SefQuadwordView *tailBlock = quadwordBase + quadwordCount;
    int i;

    for (i = 0; i < quadwordCount; i++) {
        quadwordBase[i].quadword = 0;
    }
    for (i = 0; i < tailWordCount; i++) {
        tailBlock->wordLanes[i] = 0;
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
        sefNoiseTable.sampleCursor = 0;
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
    return bits * (1.0f / 32768.0f);
}

/* Returns the next noise sample and advances the read index of the noise table. */
f32 sefNoiseNext(void)
{
    int sampleIndex = sefNoiseTable.sampleCursor;

    sefNoiseTable.sampleCursor = (sampleIndex + 1) & 0x3FF;
    return sefNoiseTable.samples[sampleIndex];
}

/* Random float in [0.75, 1): one LCG step of sefRandSeed, the high 15 bits scaled by 2^-15, times 0.25, plus 0.75. */
f32 sefRandfUpperQuarter(void)
{
    unsigned int nextSeed;

    nextSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    sefRandSeed = nextSeed;
    return (int) ((nextSeed >> 16) & SEF_RAND_MASK) * 3.0517578125e-05f * 0.25f + 0.75f;
}

/* Random angle in [0, 2*pi): the same LCG step as sefRandf, scaled by 2^-15 and then by 2*pi. */
f32 sefRandAngle(void)
{
    int bits;

    sefRandSeed = sefRandSeed * SEF_RAND_MULTIPLIER + SEF_RAND_INCREMENT;
    bits = (sefRandSeed >> 16) & 0x7FFF;
    return bits * (1.0f / 32768.0f) * sefTwoPi;
}

/* Random vector: steps the x, y and z seeds of the vector LCG (times 0xF82D, then all four lanes plus the increments in one paddw), keeps the high 15 bits of each lane and converts them to floats in out. */
void sefRandVector(Vector4 *randomVector)
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
        : : "r"(randomVector), "r"(laneSeeds), "r"(sefRandVectorIncrements), "r"(mask) : "$8", "$9", "memory");
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
void sefMergeMatrixPos(Matrix4 *destinationMatrix, Matrix4 *matrix, const Vector4 *position)
{
    QuadwordMergeMatrixPosition(destinationMatrix, matrix, position);
}

/* Builds the effect matrix in out (the scratchpad when out is null): the identity rows of MMathUnitMatrix are copied
 * first, then a nonzero translation becomes row 3, a nonzero rotation is applied, and a scale that is not all ones.
 * Returns how many of the three steps ran. */
int sefBuildEffectMatrix(const Vector4 *rotation, const Vector4 *translation, const Vector4 *scale, Matrix4 *out)
{
    Matrix4 *destMatrix;
    const Matrix4 *unitMatrix;
    int stepsRun;
    int hasTranslation;
    int hasRotation;

    destMatrix = out;
    if (destMatrix == 0) {
        destMatrix = (Matrix4 *)0x70000000;
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
        : "r" (destMatrix), "r" (unitMatrix)
        : "memory");
    hasTranslation = (*(int *)&translation->z | *(u64 *)translation) != 0;
    if (hasTranslation) {
        QuadwordCopyT0((*destMatrix)[3], translation);
        stepsRun = 1;
    }
    hasRotation = (*(int *)&rotation->z | *(u64 *)rotation) != 0;
    if (hasRotation) {
        MMathRotateMatrixZYX(destMatrix, destMatrix, rotation);
        stepsRun++;
    }
    if (scale != 0 && !(scale->x == 1.0f && scale->y == 1.0f && scale->z == 1.0f)) {
        MMathScaleMatrix(destMatrix, destMatrix, scale);
        stepsRun++;
    }
    return stepsRun;
}

void sefGetCameraPos(Matrix4 *destMatrix)
{
    XglCamera *camera = xglStudioGetActiveCamera();

    if (destMatrix == 0) {
        destMatrix = sefCameraMatrix;
    }
    sefMergeMatrixPos(destMatrix, &camera->travel.transform, &sefDefaultPoint);
}

/* VU0 product of a Vector4 and a column-major matrix: destination = matrix * vector. */
void sefApplyMatrixVector(Vector4 *transformedVector, Matrix4 *matrix, Vector4 *vector)
{
    __asm__ __volatile__("lqc2 vf1, 0(%0)" : : "r"(vector) : "memory");
    __asm__ __volatile__("lqc2 vf2, 0(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf3, 16(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("lqc2 vf4, 32(%0)" : : "r"(matrix) : "memory");
    __asm__ __volatile__("vmulax.xyzw ACC, vf2, vf1x" : : : "memory");
    __asm__ __volatile__("vmadday.xyzw ACC, vf3, vf1y" : : : "memory");
    __asm__ __volatile__("vmaddaz.xyzw ACC, vf4, vf1z" : : : "memory");
    __asm__ __volatile__("vmaddw.xyzw vf1, vf0, vf1w" : : : "memory");
    __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(transformedVector) : "memory");
}

/* Copies one 16-byte vector through t0 with the R5900 quadword load and store. */
static inline void sefCopyQuadword(Vector4 *destVector, const void *sourceVector)
{
    __asm__ __volatile__(
        "lq $8, 0(%2)\n"
        "sq $8, 0(%1)\n"
        : "+m"(*destVector)
        : "r"(destVector), "r"(sourceVector)
        : "$8", "memory");
}

/* The record of key (zero when none). */
SefEntry *sefFindEntryById(int key);

/* Copies the vector of slot of the record found by key into *destination when that slot's flag byte is set, and
 * returns bit 0 of the flag byte (0 without a record). */
int sefCopyRecordSlotVector(Vector4 *destinationVector, int key, int vectorSlot)
{
    SefEntry *foundEntry = sefFindEntryById(key);

    if (foundEntry == 0) {
        return 0;
    }
    if (foundEntry->slotFlags[vectorSlot] != 0) {
        sefCopyQuadword(destinationVector, &foundEntry->slotVectors[vectorSlot]);
    }
    return foundEntry->slotFlags[vectorSlot] & 1;
}

/* Partial view of one linked object record: its flag byte at +0x155C, bit 0 is the in-use bit. */





/* Returns bit 0 of flag byte linkIndex of the linked object table (objectOffset), or 0 without a link. */
int sefGetLinkedFlag(int linkedFlagIndex)
{
    SefLinkedContext *linkedContext = sefGetBattleData();
    int linkedObjectOffset = linkedContext->objectOffset;

    if (linkedObjectOffset == 0) {
        return 0;
    }
    return ((SefLinkedObjectRecord *)(linkedFlagIndex + linkedObjectOffset))->flags & 1;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204080);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204118);

void func_00204118(void *matrix, const void *source);
void func_0020F2E8(void *output, void *input);

void sefApplySourceMatrix(void *transformedOutput, const void *sourceMatrixBlock) {
    Matrix4 matrix;

    func_00204118(matrix, sourceMatrixBlock);
    func_0020F2E8(transformedOutput, matrix);
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204318);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00204510);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002045C8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002046F8);

/* The point writer of this unit, still hand-written asm: it writes the point of kind into dest from source. */
void func_002046F8(Vector4 *dest, const Vector4 *source, int kind);

/* Kinds 321 and 322 are passed on as kind 256; a NULL source reads sefDefaultPoint. The w component is set to 1.0f. */
void sefGetPosition(Vector4 *resultPoint, const Vector4 *sourcePoint, int pointKind)
{
    if ((unsigned int)(pointKind - 321) < 2) {
        pointKind = 256;
    }
    if (sourcePoint == 0) {
        sourcePoint = &sefDefaultPoint;
    }
    func_002046F8(resultPoint, sourcePoint, pointKind);
    resultPoint->w = 1.0f;
}

void vu0TransformPoint(Vector4 *point);

/* Partial view of a part's transform state: the matrix rows (vf26..vf28), the vector at +0xA0 (loaded into vf29), the scale
 * at +0xC0 and the point at +0x200 that vu0TransformPoint transforms. */










/* Transforms the part's point (mode 0: transform, then scale; mode != 0: scale, then transform when mode >= 0), adds the
 * offset's w term when offset is given, and writes the result to out. */
void sefTransformPartPoint(SefPartPlacement *placement, Vector4 *resultPoint, int transformMode, const Vector4 *offsetVector)
{
    __asm__ __volatile__(
        "lqc2 vf26, 0(%0)\n"
        "lqc2 vf27, 16(%0)\n"
        "lqc2 vf28, 32(%0)\n"
        "lqc2 vf29, 0(%1)\n"
        :
        : "r"(placement), "r"(&placement->translation)
        : "memory");
    if (transformMode != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulx.xyzw vf26, vf26, vf1x\n"
            "vmuly.xyzw vf27, vf27, vf1y\n"
            "vmulz.xyzw vf28, vf28, vf1z\n"
            "vmulw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(&placement->scale), "m"(placement->scale));
        if (transformMode >= 0) {
            Vector4 *point = &placement->point;
            vu0TransformPoint(point);
        }
    } else {
        vu0TransformPoint(&placement->point);
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulx.xyzw vf26, vf26, vf1x\n"
            "vmuly.xyzw vf27, vf27, vf1y\n"
            "vmulz.xyzw vf28, vf28, vf1z\n"
            "vmulw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(&placement->scale)
            : "memory");
    }
    if (offsetVector != 0) {
        __asm__ __volatile__(
            "lqc2 vf1, 0(%0)\n"
            "vmulax.xyzw ACC, vf26, vf1x\n"
            "vmadday.xyzw ACC, vf27, vf1y\n"
            "vmaddaz.xyzw ACC, vf28, vf1z\n"
            "vmaddw.xyzw vf29, vf29, vf1w\n"
            :
            : "r"(offsetVector)
            : "memory");
    }
    __asm__ __volatile__(
        "sqc2 vf29, 0(%0)\n"
        :
        : "r"(resultPoint)
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
    angle = (int)((sefRandSeed >> 16) & SEF_RAND_MASK) / SEF_NOISE_SCALE * sefCircleAngleScale;
    Vu0SetOrigin(pos);
    pos->x = xglCos(angle) * radius;
    pos->z = xglSin(angle) * radius;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00205470);

Vector4 *MMathNormalizeVector2(Vector4 *destination, const Vector4 *source);

/* Unit direction from the three floats x, y and z (w is left as it was), normalized and then scaled by scale in x, y
 * and z. */
void sefSetUnitDirectionScaled(Vector4 *vector, float x, float y, float z, float scale)
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
void sefGetCubePosCenter(Vector4 *destinationPoint, Vector4 *scale)
{
    sefNoiseNextVector(destinationPoint);
    Vu0SefMulVectorXYZ(destinationPoint, destinationPoint, scale);
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

#define sefEffectEntryTable (*(SefEntryTable *)&sefEffectEntryTable)
extern Vector4 sefOriginVector;

/* Zeroes the origin, then clears the flags of every entry and the short at +0x1592 of each. */
void sefResetEntryFlagsAndOrigin(void)
{
    SefEntry *effectEntry;
    int i;

    Vu0SetOrigin(&sefOriginVector);
    for (i = 0; i < sefEffectEntryTable.effectEntryCount; i++) {
        effectEntry = &sefEffectEntryTable.effectEntries[i];
        effectEntry->effectEntryFlags = 0;
        *(short *) &effectEntry->unmodeled_157C[0x1592 - 0x157C] = 0;
    }
}

/* TU-local view of D_004ABFD0 (the same 0x58-byte transform block as SefPartTransform in
 * sv_get_image_item_143.c; no header defines it yet). */








extern SefGlobalTransform D_004ABFD0;
Matrix4 *MMathMulMatrix(Matrix4 *out, Matrix4 *rhs, Matrix4 *lhs);
Vector4 *MMathApplyMatrix(Vector4 *out, Matrix4 *matrix, const Vector4 *vector);

/* Places the part: the combined matrix of the placement and the global transform (its translation first offset by the
 * global position when the scale applies), applied to the placement's point when mode >= 0, otherwise the matrix's
 * position row copied to out. Returns the global scale when the scale applies, else 1. */
float sefPlacePart(SefPartPlacement *placement, Vector4 *resultPoint, int placeMode)
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
    if (placeMode >= 0) {
        Vector4 *point = &placement->point;

        MMathApplyMatrix(resultPoint, &combined, point);
        return scale;
    }
    sefCopyQuadword(resultPoint, combined[3]);
    return scale;
}

INCLUDE_ASM("asm/main/nonmatchings/sef", func_002071F8);

INCLUDE_ASM("asm/main/nonmatchings/sef", func_00207490);

#undef sefEffectEntryTable
/* <<< CHUNK sef_134 */

/* >>> CHUNK sef_135 */

int sefEffectFlagTest(int mask);

void sefEffectFlagSet(int mask);
void sefTickGroupTimers(void);
void sefGetGroupCenter(float *center, int owner, unsigned short key);

#define SEF_GROUP_ENTRY_COUNT 8

/*
 * One group timer, 0x20 bytes, eight of them in sefGroupTimers. sefRegisterGroupCenter fills it for a tracked group
 * (the centre of the group's objects, the owner id and the key that name the group) with 600 frames on the timer;
 * sefTickGroupTimers counts `remaining` down; sefGetGroupCenter reads the centre back by owner and key.
 */








extern SefGroupTimer sefGroupTimers[SEF_GROUP_ENTRY_COUNT];
/* sefEffectFlags is one flag word; sefExecEffect tests bit 1 of it through sefEffectFlagTest. */
extern int sefEffectFlags;
void sefEffectFlagClear(int mask);

/* Counts every group timer down by one frame, stopping at zero. */
void sefTickGroupTimers(void)
{
    SefGroupTimer *groupTimer;
    int timerIndex;

    groupTimer = sefGroupTimers;
    for (timerIndex = SEF_GROUP_ENTRY_COUNT - 1; timerIndex >= 0; timerIndex--) {
        if (groupTimer->remaining > 0) {
            groupTimer->remaining = groupTimer->remaining - 1;
        }
        groupTimer++;
    }
}

/*
 * Writes the origin vector (0, 0, 0, 1) to center, then copies the centre of the first table entry whose owner and key
 * match. Leaves center as the origin when no entry matches.
 */
void sefGetGroupCenter(float *center, int owner, unsigned short key)
{
    SefGroupTimer *groupTimer;
    int timerIndex;

    Vu0SetOrigin(center);
    groupTimer = sefGroupTimers;
    for (timerIndex = 0; timerIndex < SEF_GROUP_ENTRY_COUNT; timerIndex++) {
        if (groupTimer->owner == owner && groupTimer->key == key) {
            QuadwordCopyT0(center, groupTimer->center);
            return;
        }
        groupTimer++;
    }
}

#define SEF_GROUP_MEMBER_MAX 24
#define SEF_GROUP_TIMER_FRAMES 600
#define SEF_RECORD_EXCLUDE_FLAG 0x8000000

/* Effect record count word (gp-24972); the scaffold has no name for it yet. */
extern int sefEffectRecordCount;

SefEffectRecord *sefGetScheduler(void);
Vector4 *MMathDivVectorS(Vector4 *destination, const Vector4 *source, float scale);

/*
 * Copies a group centre into its timer entry. Both registration paths share one copy block in the original, which
 * gcc only produces when the two copies come from the same asm statement.
 */
static inline void sefGroupStoreCenter(SefGroupTimer *groupTimer, Vector4 *centre)
{
    QuadwordCopyT0(groupTimer->center, centre);
}

/*
 * Gathers the live effect records owned by owner under key. Each distinct object is counted once and its position is
 * summed into the group centre; the centre is averaged when two or more objects contributed. The centre is written to
 * the table entry for (owner, key) if one exists, otherwise to the first free entry, which gets a 600-frame timer.
 */
void sefRegisterGroupCenter(int owner, unsigned short key)
{
    int scanIndex;
    int seenCount;
    int seenIndex;
    SefEffectRecord *effectRecords;
    SefEffectRecord *effectRecord;
    SefRecordOwner *groupMember;
    SefRecordOwner *seen[SEF_GROUP_MEMBER_MAX];
    AlignedVector4 centerSum;
    Vector4 *center;
    SefGroupTimer *groupTimer;

    effectRecords = sefGetScheduler();
    center = &centerSum;
    seenCount = 0;
    Vu0SetOrigin(center);
    for (scanIndex = 0; scanIndex < sefEffectRecordCount; scanIndex++) {
        effectRecord = &effectRecords[scanIndex];
        if (effectRecord->inUse == 0 || effectRecord->ownerKey != owner) {
            continue;
        }
        groupMember = effectRecord->ownedObject;
        if (groupMember == 0 || effectRecord->groupKey != key) {
            continue;
        }
        if (effectRecord->ownerGroupFlags & SEF_RECORD_EXCLUDE_FLAG) {
            continue;
        }
        for (seenIndex = 0; seenIndex < seenCount; seenIndex++) {
            if (seen[seenIndex] == effectRecord->ownedObject) {
                break;
            }
        }
        if (seenIndex != seenCount) {
            continue;
        }
        /* center.xyz += groupMember position; no memory clobber, so the effectRecord count load stays hoisted */
        __asm__ __volatile__(
            "lqc2 vf1, 0(%1)\n"
            "lqc2 vf2, 0(%2)\n"
            "vadd.xyz vf1, vf1, vf2\n"
            "sqc2 vf1, 0(%0)\n"
            :
            : "r" (center), "r" (center), "r" (&groupMember->position));
        seen[seenCount] = groupMember;
        seenCount++;
        if (seenCount >= SEF_GROUP_MEMBER_MAX) {
            break;
        }
    }
    if (seenCount >= 2) {
        MMathDivVectorS(center, center, seenCount);
    }

    for (scanIndex = 0; scanIndex < SEF_GROUP_ENTRY_COUNT; scanIndex++) {
        groupTimer = &sefGroupTimers[scanIndex];
        if (groupTimer->owner == owner && groupTimer->key == key) {
            groupTimer->remaining = SEF_GROUP_TIMER_FRAMES;
            groupTimer->key = key;
            sefGroupStoreCenter(groupTimer, center);
            return;
        }
    }

    for (scanIndex = 0; scanIndex < SEF_GROUP_ENTRY_COUNT; scanIndex++) {
        groupTimer = &sefGroupTimers[scanIndex];
        if (groupTimer->remaining == 0) {
            groupTimer->remaining = SEF_GROUP_TIMER_FRAMES;
            groupTimer->owner = owner;
            groupTimer->key = key;
            sefGroupStoreCenter(groupTimer, center);
            return;
        }
    }
}

extern unsigned char sefOverrideFlag[];

u8 *sefGetFogOverrideBlock(void) {
    return sefOverrideFlag;
}

void sefEffectFlagSet(s32 mask) {
    sefEffectFlags |= mask;
}

void sefEffectFlagClear(s32 mask) {
    sefEffectFlags &= ~mask;
}

int sefEffectFlagTest(int mask) {
    return sefEffectFlags & mask;
}

void sefFlagToggle(int mask) {
    sefEffectFlags ^= mask;
}

void sefEffectFlagUpdateFromSign(int selector) {
    if (selector < 0) {
        sefFlagToggle(1);
        return;
    }
    if (selector > 0) {
        sefEffectFlagSet(1);
        return;
    }
    sefEffectFlagClear(1);
}

/* Counts the records of owner under key that the owner-group flags do not exclude. */
int sefCountOwnerGroupRecords(int owner, unsigned short key)
{
    SefEffectRecord *effectRecord;
    int scanIndex;
    int matchCount;

    matchCount = 0;
    effectRecord = sefGetScheduler();
    for (scanIndex = 0; scanIndex < sefEffectRecordCount; scanIndex++) {
        if (effectRecord->inUse != 0 && effectRecord->ownerKey == owner && effectRecord->groupKey == key) {
            if ((effectRecord->ownerGroupFlags & SEF_RECORD_EXCLUDE_FLAG) == 0) {
                matchCount += 1;
            }
        }
        effectRecord++;
    }
    return matchCount;
}

/* Clears the mask bits of the records of owner under key, except records that carry SEF_RECORD_EXCLUDE_FLAG. */
void sefClearOwnerGroupFlags(int owner, unsigned short key, int mask)
{
    SefEffectRecord *effectRecord;
    int scanIndex;
    int scanLimit;
    int recordLimit;
    int keepBit;
    int recordBits;

    effectRecord = sefGetScheduler();
    scanLimit = sefEffectRecordCount;
    scanIndex = 0;
    if (scanLimit > 0) {
        keepBit = SEF_RECORD_EXCLUDE_FLAG;
        recordLimit = scanLimit;
        do {
            if (effectRecord->inUse != 0 && effectRecord->ownerKey == owner && effectRecord->groupKey == key) {
                recordBits = effectRecord->ownerGroupFlags;
                if ((recordBits & keepBit) == 0) {
                    effectRecord->ownerGroupFlags = recordBits & ~mask;
                    scanLimit = recordLimit;
                }
            }
            scanIndex += 1;
            effectRecord++;
        } while (scanIndex < scanLimit);
    }
}

/* <<< CHUNK sef_135 */
