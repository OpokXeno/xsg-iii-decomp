#include "common.h"
#include "shared.h"

typedef int s32;
typedef signed char s8;

float xglAtan2(float y, float x);

void cameraAnglesBetween(Vector4 *angles, const Vector4 *from, const Vector4 *to);

/* Angles, in radians. */
#define CAMERA_PI 3.1415927f

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F3C0);

s32 xglCameraMove(void *camera);
void *xglStudioGetCamera2(s32 index);

void cameraSyncMove(void) {
    void *camera;
    camera = xglStudioGetCamera2(0);
    xglCameraMove(camera);
}

/* The camera mode block: the mode at +0, an auxiliary byte at +1 copied to the flag at +2. */
typedef struct CameraModeState {
    unsigned char modeId;                 /* +0x00 */
    unsigned char pendingFlag;                  /* +0x01 */
    s8 cameraFlag;                            /* +0x02: -1 in camera mode 1 */
    unsigned char unk03[0x30 - 0x03];
} CameraModeState;

/* One camera entry (40 of them): the origin of its probe is the Vector4 at +0x20. */
typedef struct CameraEntry {
    unsigned char unk00[0x20];
    f32 probeOrigin[4];                 /* +0x20 */
    unsigned char unk30[0xD0 - 0x30];
} CameraEntry;

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
typedef struct CurrentCameraSlot {
    unsigned char unk000[0x6C4];
    s32 cameraFlags;                       /* +0x6C4: bit 0x100 is tested by cameraCurrentFlagged */
} CurrentCameraSlot;

#define cameraModeState (*(CameraModeState *) gCameraModeState)
#define cameraBlock (*(CameraBlock *) gCameraBlock)
#define cameraProbe (*(CameraProbe *) gCameraProbe)
#define cameraResourceHandle (*(s32 *) gCameraResourceHandle)
#define cameraFloatState (*(f32 *) gCameraFloatState)

extern unsigned char gCameraModeState[];
extern CurrentCameraSlot *gCurrentCameraSlot;
extern unsigned char gCameraBlock[];
extern unsigned char gCameraProbe[];
void cameraRegisterBcamNatives();
void func_0025F3C0();
void func_0025F688();
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
    func_0025F688(0);
    func_0025F3C0();
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

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F558);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F5C0);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F688);

void cameraGetSavedPair(s32 *outSavedFirst, s32 *outSavedSecond)
{
    *outSavedFirst = cameraBlock.savedFirst;
    *outSavedSecond = cameraBlock.savedSecond;
}

extern unsigned char gCameraResourceHandle[];
s32 func_002A6FA8();

void cameraLoadResource(s32 resourceId) {
    s32 resourceHandle;

    resourceHandle = func_002A6FA8(0x600, resourceId);
    if (resourceHandle != 0) {
        cameraResourceHandle = resourceHandle;
    }
}

extern unsigned char gCameraFloatState[];

void cameraSetFloatState(f32 value) {
    cameraFloatState = value;
}

void cameraSetMode(s32 newModeId) {
    cameraModeState.modeId = newModeId;
    if (newModeId == 1) {
        cameraModeState.cameraFlag = -1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F750);

/* The studio camera view that xglStudioGetCamera2 returns: the angles at +0xA0 and the target at +0xD0. */
typedef struct StudioCameraView {
    unsigned char unk00[0xA0];
    Vector4 angles;                     /* +0xA0 */
    unsigned char unkB0[0xD0 - 0xB0];
    Vector4 lookAtPos;                     /* +0xD0 */
} StudioCameraView;

void cameraComputeAngles(Vector4 *fromPos) {
    StudioCameraView *camera;

    camera = xglStudioGetCamera2(0);
    cameraAnglesBetween(&camera->angles, fromPos, &camera->lookAtPos);
}

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F7D0);

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

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F9A0);

extern unsigned char gCameraSlotPairs[];

u8 *cameraGetStateBlock(void) {
    return gCameraSlotPairs;
}

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025F9E8);

s32 func_0025F5C0();

void cameraStepPending(void) {
    func_0025F5C0();
}

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FA68);

s32 func_00261050();
s32 func_00264E60();
s32 func_00265FA8();
s32 func_00266008();
s32 cameraSlotsUpdate();

void cameraUpdateAll(void) {
    func_00265FA8();
    func_00264E60();
    func_00266008();
    cameraSlotsUpdate();
    func_00261050();
}

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FB78);

INCLUDE_ASM("asm/main/nonmatchings/game_camera", func_0025FC70);

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
