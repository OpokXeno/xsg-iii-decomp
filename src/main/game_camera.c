#include "common.h"
#include "shared.h"

typedef int s32;

typedef signed char s8;

static unsigned char gCameraFallbackPos[16];
static unsigned char gCameraFallbackAngles[16];
static DoublewordAlignedVector4 gCameraSavedPos;
static DoublewordAlignedVector4 gCameraSavedAngles;
static unsigned char gCameraBlock[0x2080];
static unsigned char gCameraSlotPairs[3216]; /* gCameraFloatState is its last word (+3212) */
static unsigned char gCameraProbe[28];
static unsigned char gCameraResourceHandle[20];
static f32 gCameraSavedFloat;

float xglAtan2(float y, float x);
void cameraAnglesBetween(Vector4 *angles, const Vector4 *from, const Vector4 *to);

/* Angles, in radians. */
#define CAMERA_PI 3.1415927f

void *xglStudioGetCamera2(s32 index);

typedef struct StudioCameraView {
    unsigned int active;      /* +0x00 */
    unsigned int cameraState; /* +0x04 */
    unsigned char unk08[0x94 - 0x08];
    unsigned char unk94[0xA0 - 0x94]; /* +0x94: its address is stored by cameraSlotsSelect */
    Vector4 angles;                   /* +0xA0 */
    unsigned char unkB0[0xD0 - 0xB0];
    Vector4 lookAtPos; /* +0xD0 */
} StudioCameraView;

extern void *gCameraSourcePointers[];
extern f32 gCameraDefaultFloat;

/* The three camera slots of gCameraSourcePointers: look-at, angles and the float slot. */
typedef struct CameraSlotPointers {
    Vector4 *lookAt;
    Vector4 *angles;
    f32 *cameraFloatSlot;
} CameraSlotPointers;

#define cameraSlots (*(CameraSlotPointers *) gCameraSourcePointers)
void cameraSlotsSelect();

/* Points the three camera slots of gCameraSourcePointers (look-at, angles, float) at the studio camera
 * while its state is 4, else at the private fallbacks. */
void cameraSlotsSelect(void) {
    StudioCameraView *camera;

    camera = xglStudioGetCamera2(0);
    if (camera->cameraState == 4) {
        cameraSlots.lookAt = &camera->lookAtPos;
        cameraSlots.angles = &camera->angles;
        cameraSlots.cameraFloatSlot = (f32 *) camera->unk94;
    } else {
        cameraSlots.lookAt = gCameraFallbackPos;
        cameraSlots.angles = gCameraFallbackAngles;
        cameraSlots.cameraFloatSlot = &gCameraDefaultFloat;
    }
}

s32 xglCameraMove(void *camera);

void cameraSyncMove(void) {
    void *camera;
    camera = xglStudioGetCamera2(0);
    xglCameraMove(camera);
}

/* The camera mode block: the mode at +0, an auxiliary byte at +1 copied to the flag at +2. */
typedef struct CameraModeState {
    s8 modeId;                 /* +0x00 */
    s8 pendingFlag;                             /* +0x01: the pending camera index */
    s8 cameraFlag;                            /* +0x02: -1 in camera mode 1 */
    unsigned char unk03[0x30 - 0x03];
} CameraModeState;

/* One camera entry (40 of them): the origin of its probe is the Vector4 at +0x20. */
typedef struct CameraEntry {
    unsigned char unk00[0x20];
    f32 probeOrigin[4];                 /* +0x20 */
    unsigned char unk30[0xD0 - 0x30];
} CameraEntry;

typedef struct CameraObject {
    unsigned char motionKind; /* +0x00: 1..5 select the motion */
    unsigned char unmodeled_01[3];
    float distance; /* +0x04 */
    unsigned char unmodeled_08[8];
    unsigned char motion[0x30];     /* +0x10 */
    unsigned char cameraView[0x60]; /* +0x40: the view, up to +0xA0 */
    Vector4 viewPointA0;            /* +0xA0 */
    Vector4 viewPointB0;            /* +0xB0: x is tested by cameraUpdateMode, w is passed as +0xBC */
} CameraObject;

/* The camera table block: 40 entries, then the init bytes and the saved pair at +0x2D04 and +0x2D08. */
typedef struct CameraBlock {
    CameraEntry cameraEntries[0x28];          /* +0x0000 */
    s8 initCameraMode;                        /* +0x2080 */
    s8 initCameraFlag;                        /* +0x2081 */
    unsigned char unk2082[0x2D04 - 0x2082];
    s32 savedFirst;                     /* +0x2D04 */
    s32 savedSecond;                    /* +0x2D08 */
    unsigned char unk2D0C[0x2D10 - 0x2D0C];
} CameraBlock;

/* The probe block that the map probes take: the record pair, the bit word at +0x0A and the kind byte at +0x24. */
typedef struct CameraProbe {
    s32 probeRecord;                         /* +0x00 */
    s32 previousProbeRecord;                 /* +0x04 */
    unsigned char unk08[0x0A - 0x08];
    s16 bits;                           /* +0x0A */
    unsigned char unk0C[0x24 - 0x0C];
    unsigned char probeKind;                 /* +0x24 */
} CameraProbe;

/* The current camera slot: the flag word at +0x6C4. */
typedef struct CameraView {
    long long first;
    long long second;
} CameraView;

typedef struct CurrentCameraSlot {
    unsigned char unk000[0x20];
    CameraView probeOriginView;                 /* +0x20: copied into the pending entry by cameraTransitionUpdate */
    unsigned char unk030[0x6C4 - 0x30];
    s32 cameraFlags;                       /* +0x6C4: bit 0x100 is tested by cameraCurrentFlagged */
} CurrentCameraSlot;

#define cameraModeState (*(CameraModeState *) gCameraModeState)
#define cameraBlock (*(CameraBlock *) gCameraBlock)
#define cameraProbe (*(CameraProbe *) gCameraProbe)
#define cameraResourceHandle (*(s32 *) gCameraResourceHandle)
#define cameraFloatState (*(f32 *) gCameraFloatState)

extern unsigned char gCameraFloatState[];

extern unsigned char gCameraModeState[];
extern unsigned char gCameraFallbackEntry[];
static CurrentCameraSlot *gCurrentCameraSlot;
void cameraRegisterBcamNatives();
void cameraSetStudioState();
void cameraLoadResource();
void cameraBlockClear();
void cameraRegisterEffcamNatives();
void cameraRegisterNativeOwner();
void cameraTablesClear();
void cameraAuxReset();
void cameraPendingClear();
void cameraWorkBufferAlloc();
void cameraSlotsClear();
void func_002A70E8();
void memset();

void cameraInit(void) {
    memset(gCameraModeState, 0, 0x30);
    cameraModeState.cameraFlag = -1;
    cameraModeState.modeId = 1;
    memset(gCameraBlock, 0, 0x2D10);
    gCurrentCameraSlot = 0;
    func_002A70E8(gCameraProbe);
    cameraProbe.previousProbeRecord = 0;
    cameraProbe.probeRecord = 0;
    cameraProbe.bits = 0x4C00;
    cameraLoadResource(-1);
    cameraSetStudioState(0);
    cameraSlotsSelect();
    cameraRegisterNativeOwner();
    cameraRegisterEffcamNatives();
    cameraRegisterBcamNatives();
    cameraWorkBufferAlloc();
    cameraAuxReset();
    cameraBlockClear();
    cameraPendingClear();
    cameraTablesClear();
    cameraSlotsClear();
    cameraBlock.initCameraMode = 6;
    cameraBlock.initCameraFlag = 0;
}

void cameraSetCurrent(CurrentCameraSlot *camera) {
    gCurrentCameraSlot = camera;
}

void cameraUpdateAll(void);
void cameraUpdateMode(void);

/* Per-frame camera update: refreshes the slots, runs cameraUpdateAll (mode -1) or cameraUpdateMode,
 * then cameraSyncMove, only while a mode and a current slot are set. */
void cameraUpdateFrame(void) {
    cameraSlotsSelect();
    if (cameraModeState.modeId != 0 && gCurrentCameraSlot != 0) {
        if (cameraModeState.modeId == -1) {
            cameraUpdateAll();
        } else {
            cameraUpdateMode();
        }
        cameraSyncMove();
    }
}

/* captureCamera != 0 copies the three camera slots into the saved values;
 * captureCamera == 0 writes the saved values back into the slots. */
void cameraCaptureOrRestoreSlots(s32 captureCamera);

void cameraCaptureOrRestoreSlots(s32 captureCamera) {
    if (captureCamera != 0) {
        gCameraSavedPos = *cameraSlots.lookAt;
        gCameraSavedAngles = *cameraSlots.angles;
        gCameraSavedFloat = *cameraSlots.cameraFloatSlot;
    } else {
        *cameraSlots.lookAt = gCameraSavedPos;
        *cameraSlots.angles = gCameraSavedAngles;
        *cameraSlots.cameraFloatSlot = gCameraSavedFloat;
    }
}

/* Sets the studio camera state: 4 (the state cameraSlotsSelect tests) or 2 when useFallbackCamera is set. */
void cameraSetStudioState(s32 useFallbackCamera) {
    StudioCameraView *camera;

    if (useFallbackCamera == 0) {
        camera = xglStudioGetCamera2(0);
        camera->cameraState = 4;
    } else {
        camera = xglStudioGetCamera2(0);
        camera->cameraState = 2;
    }
}

void cameraGetSavedPair(s32 *outSavedFirst, s32 *outSavedSecond)
{
    *outSavedFirst = cameraBlock.savedFirst;
    *outSavedSecond = cameraBlock.savedSecond;
}

s32 func_002A6FA8();

void cameraLoadResource(s32 resourceId) {
    s32 resourceHandle;

    resourceHandle = func_002A6FA8(0x600, resourceId);
    if (resourceHandle != 0) {
        cameraResourceHandle = resourceHandle;
    }
}

void cameraSetFloatState(f32 cameraFloatNudge) {
    cameraFloatState = cameraFloatNudge;
}

void cameraSetMode(s32 newModeId) {
    cameraModeState.modeId = newModeId;
    if (newModeId == 1) {
        cameraModeState.cameraFlag = -1;
    }
}

/* Sets the studio camera's look-at position. */
void cameraSetLookAt(const Vector4 *lookAtPos) {
    StudioCameraView *camera;

    camera = xglStudioGetCamera2(0);
    camera->lookAtPos = *lookAtPos;
}

void cameraComputeAngles(Vector4 *fromPos) {
    StudioCameraView *camera;

    camera = xglStudioGetCamera2(0);
    cameraAnglesBetween(&camera->angles, fromPos, &camera->lookAtPos);
}

void xglMatrixStackUnit(void);
void xglMatrixStackTrans(const Vector4 *vector);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackSave(Matrix4 matrix);

/* Computes the world position of an orbiting camera: pivotPosition, turned by orbitAngles
 * (y then x), then moved orbitAngles->w along z. The result goes to outPosition. */
void cameraOrbitPosition(Vector4 *outPosition, const Vector4 *pivotPosition, const Vector4 *orbitAngles) {
    Vector4 distanceOffset;
    Vector4 pivotPoint;
    Matrix4 matrix;
    float homogeneousW;

    distanceOffset.z = orbitAngles->w;
    homogeneousW = 1.0f;
    pivotPoint.x = pivotPosition->x;
    pivotPoint.y = pivotPosition->y;
    distanceOffset.x = 0.0f;
    distanceOffset.y = 0.0f;
    distanceOffset.w = homogeneousW;
    pivotPoint.z = pivotPosition->z;
    pivotPoint.w = homogeneousW;
    xglMatrixStackUnit();
    xglMatrixStackTrans(&pivotPoint);
    xglMatrixStackRotY(orbitAngles->y);
    xglMatrixStackRotX(orbitAngles->x);
    xglMatrixStackTrans(&distanceOffset);
    xglMatrixStackSave(matrix);
    outPosition->x = matrix[3][0];
    outPosition->y = matrix[3][1];
    outPosition->z = matrix[3][2];
}

/* Writes the angles from `to` to `from`: x the pitch, y the yaw (plus pi), w the length of the difference. */
void cameraAnglesBetween(Vector4 *angles, const Vector4 *fromPos, const Vector4 *toPos)
{
    float dx = fromPos->x - toPos->x;
    float dy = fromPos->y - toPos->y;
    float dz = fromPos->z - toPos->z;

    angles->y = xglAtan2(dx, dz) + CAMERA_PI;
    dx = dx * dx;
    dz = dz * dz;
    angles->x = xglAtan2(dy, __builtin_sqrtf(dx + dz));
    angles->w = __builtin_sqrtf(dx + dy * dy + dz);
}

void *cameraGetEntry(s32 cameraIndex) {
    void *cameraEntry;

    cameraEntry = 0;
    if (cameraIndex >= 0) {
        if (cameraIndex < 0x28) {
            cameraEntry = &cameraBlock.cameraEntries[cameraIndex];
        }
    }
    return cameraEntry;
}

/* The entry of the pending camera, or the fallback table gCameraFallbackEntry when that entry is empty (first byte 0). */
CameraObject *cameraPendingEntry(void) {
    CameraObject *cameraEntry;

    cameraEntry = cameraGetEntry(cameraModeState.pendingFlag);
    if (cameraEntry->motionKind != 0) {
        return cameraEntry;
    }
    return (CameraObject *) gCameraFallbackEntry;
}

u8 *cameraGetStateBlock(void) {
    return gCameraSlotPairs;
}

/* The 2-byte pair record of two cameras in gCameraSlotPairs (the order does not matter), or 0 for the same camera. */
u8 *cameraPairRecord(s32 firstCamera, s32 secondCamera) {
    if (firstCamera == secondCamera) {
        return 0;
    }
    if (firstCamera < secondCamera) {
        return &gCameraSlotPairs[2 + 2 * (40 * firstCamera + secondCamera)];
    }
    return &gCameraSlotPairs[2 + 2 * (40 * secondCamera + firstCamera)];
}

void cameraStepPending(s32 captureCamera) {
    cameraCaptureOrRestoreSlots(captureCamera);
}

s32 cameraCurrentFlagged(void);
void cameraCopyAuxToFlag(void);
void cameraTransitionUpdate(void);


void cameraObjectUpdate(CameraObject *cameraObject);
void func_002658B8();
s32 func_00261050();
s32 func_00265FA8();
s32 func_00266008();
s32 cameraSlotsUpdate();

/* Mode-driven camera update, skipped while the current slot has flag 0x100: transition, object
 * update of the pending camera, slot update, the float nudge, then the post-step. */
void cameraUpdateMode(void) {
    CameraObject *cameraEntry;

    if (cameraCurrentFlagged() == 0) {
        cameraTransitionUpdate();
        func_00265FA8();
        cameraEntry = cameraPendingEntry();
        cameraObjectUpdate(cameraEntry);
        func_00266008();
        /* kept: the call through void (*)() decides the emitted call */
        ((void (*)()) cameraSlotsUpdate)();
        if (cameraFloatState != 0.0f) {
            cameraSlots.angles->z += cameraFloatState;
        }
        func_00261050();
        cameraCopyAuxToFlag();
        if (cameraEntry->viewPointB0.x == 0.0f) {
            if (cameraEntry->viewPointB0.w == 0.0f) {
                return;
            }
        }
        func_002658B8(&cameraEntry->viewPointA0, &cameraEntry->viewPointB0.x, &cameraEntry->viewPointB0.w);
    }
}

s32 func_00264E60();

void cameraUpdateAll(void) {
    func_00265FA8();
    func_00264E60();
    func_00266008();
    cameraSlotsUpdate();
    func_00261050();
}


void func_00260FD0(u8 *pair);
u8 cameraEntryProbe(CameraEntry *cameraEntry);

/* Camera transition: records the pending camera from the probe and copies the current slot's view
 * into its entry; when the camera changed, saves the pair of cameras and runs func_00260FD0 on its record. */
void cameraTransitionUpdate(void) {
    CameraObject *cameraEntry;
    u8 *cameraSlotPair;

    cameraModeState.pendingFlag = cameraEntryProbe((CameraEntry *) gCurrentCameraSlot);
    cameraEntry = cameraGetEntry(cameraModeState.pendingFlag);
    if (cameraEntry == 0 || cameraEntry->motionKind == 0) {
        cameraEntry = (CameraObject *) gCameraFallbackEntry;
    }
    /* kept: the 16-byte copies go through CameraView (two doublewords) */
    *(CameraView *) &cameraEntry->viewPointA0 = *(CameraView *) cameraEntry->cameraView =
        gCurrentCameraSlot->probeOriginView;
    if (cameraModeState.cameraFlag != -1 && cameraModeState.cameraFlag != cameraModeState.pendingFlag) {
        cameraSlotPair = cameraPairRecord(cameraModeState.cameraFlag, cameraModeState.pendingFlag);
        if (cameraSlotPair != 0 && cameraSlotPair[0] != 0) {
            cameraBlock.savedFirst = cameraModeState.cameraFlag;
            cameraBlock.savedSecond = cameraModeState.pendingFlag;
        } else {
            cameraSlotPair = (u8 *) &cameraBlock.initCameraMode;
            cameraBlock.savedSecond = 0;
            cameraBlock.savedFirst = 0;
        }
        func_00260FD0(cameraSlotPair);
    }
}

void func_00261DA0(unsigned char *view, float distance);
void func_002662B0(unsigned char *view);
void func_002660B8(unsigned char *motion);
void objectAdvanceToTarget(unsigned char *motion);
void objectAdvanceRotated(unsigned char *motion);
void objectAdvanceAndAim(unsigned char *motion);
void func_00265878(unsigned char *motion);
void cameraUpdateRefDistance(unsigned char *view);

/* Updates one camera object: its view and distance, the motion picked by motionKind (1..5),
 * then its reference distance. */
void cameraObjectUpdate(CameraObject *cameraObject) {
    func_00261DA0(cameraObject->cameraView, cameraObject->distance);
    func_002662B0(cameraObject->cameraView);
    switch (cameraObject->motionKind) {
    case 1:
        func_002660B8(cameraObject->motion);
        break;
    case 2:
        objectAdvanceToTarget(cameraObject->motion);
        break;
    case 3:
        objectAdvanceRotated(cameraObject->motion);
        break;
    case 4:
        objectAdvanceAndAim(cameraObject->motion);
        break;
    case 5:
        func_00265878(cameraObject->motion);
        break;
    }
    cameraUpdateRefDistance(cameraObject->cameraView);
}

void cameraCopyAuxToFlag(void)
{
    cameraModeState.cameraFlag = cameraModeState.pendingFlag;
}

s32 func_002AA490();

u8 cameraEntryProbe(CameraEntry *cameraEntry) {
    func_002AA490(cameraEntry->probeOrigin, 0, gCameraProbe);
    return cameraProbe.probeKind;
}

s32 cameraCurrentFlagged(void)
{
    if ((gCurrentCameraSlot->cameraFlags & 0x100) != 0)
    {
        return 1;
    }
    return 0;
}

unsigned char gCameraFallbackEntry[208] = {
    3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    232, 102, 223, 190, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    186, 184, 50, 63, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    36, 95, 13, 196, 36, 95, 13, 68,
    36, 95, 13, 196, 36, 95, 13, 68,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 128, 63,
    0, 0, 0, 0, 0, 0, 0, 0,
    232, 102, 223, 190, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 32, 65, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char gCameraModeState[48] = { 0 };

f32 gCameraDefaultFloat = 0.698131204f;

