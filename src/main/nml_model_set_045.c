#include "common.h"
#include "shared.h"
#include "main/tu045.h"
#include "main/tu141.h"
#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
void xglMatrixUnit(Matrix4 destination);
struct PxyDrawPartRange;
union Matrix;
struct PxyShadow;
struct PxyHideMask;
void *nmlPacketSetAttributeData16N(void *matrices, int count);
void nmlModelDirectSend(int key, void *packetData, int qwc);
void nmlModelDirectSendFull(int key, void *packetData, int qwc, int sendFlag, int sendState);
void pxyAbortModel(void);
struct PxyModelHeader;
void *memset(void *destination, int value, unsigned int size);

/* pxySetClipPlaneWithAngle fills it (at most two entries, five quadwords allocated). It is the PxyClipPlanes of a packet
 * with float-array rows: indexing a row as v[i] folds the field offset into the address the way the original does, a
 * vector member does not (pxyClipPlanes, the table under construction, is the only user). */
/* A plane or angle row as pxySetClipPlaneMasked reads it: words 0..2 are compared as integers. */
typedef union PxyRowBits {
    Vector4Elements row;
    u32 rowWords[4];
} PxyRowBits;

typedef struct PxyPlaneTable {
    PxyRowBits plane[2]; /* +0x00: plane normal and distance */
    PxyRowBits angle[2]; /* +0x20: rotX, rotY, then the next rotX, rotY */
} PxyPlaneTable;

#define PXY_SLOT_COUNT 4 /* ring of recently drawn packets */

typedef struct PxyDrawSlot {
    PxyPacket *packet;
    int batch;
    int translucentBatch;
    int unk0C;
} PxyDrawSlot;

typedef union PxyBestRecord {
    struct {
        float best;  /* +0x00 */
        int bestIndex;   /* +0x04 */
        int bestCount;   /* +0x08 */
        int unk0C;   /* +0x0c: not read here */
    };
    int rawWords[4];
    u64 doublewords[2]; /* the records are doubleword aligned */
} PxyBestRecord;

typedef struct PxyModelRenderState { XglLightSet light; u8 unk180[0x300 - 0x180]; u32 renderStatus; /* +0x300: nmlRenderStatus, the flag word of the model code */ u8 unk304[0x314 - 0x304]; PxyModelHeader *modelHeader; /* +0x314: pxyCurrentHeader */ u8 unk318[0x394 - 0x318]; int renderLevel; /* +0x394: pxyRenderLevel */ u8 unk398[0x3AC - 0x398]; int visibleOverride; /* +0x3ac */ Matrix *motionGlobalMatrices; /* +0x3b0 */ s16 clipPlaneCount; /* +0x3b4 */ u16 clipPlaneMaskedCount; /* +0x3b6 */ PxyPlaneTable *clipPlanes; /* +0x3b8 */ Matrix *globalMatrices; /* +0x3bc: pxyGlobalMatrices, one 0x40-byte matrix per joint */ float clipPlaneMaskedX; /* +0x3c0 */ u32 *clipPlaneMask; /* +0x3c4 */ float clipPlaneMaskedY; /* +0x3c8 */ float clipPlaneMaskedZ; /* +0x3cc */ u8 unk3D0[0x3E4 - 0x3D0]; int stateFlags3E4; /* +0x3e4: bit 0x8 is set by pxyStateSetFlag3E4Bit3 */ u8 unk3E8[0x480 - 0x3E8]; u32 cameraVisible[4]; /* +0x480: per camera, nonzero when the model is visible */ u8 unk490[0x4E0 - 0x490]; float trackValues[8][4]; /* +0x4e0: result stored at index k + 1 */ PxyBestRecord bestRecords[13]; /* +0x560: written by pxyEvaluateTracks, read by pxyCalcFogFactors */ void (*drawCallbacks[16])(int); /* +0x630: registered by pxyRegisterDrawCallback, called with the camera index */ u8 unk670[0x698 - 0x670]; u32 drawFlags; /* +0x698: the pxy flag word, bit 0x1000 is set by pxyForceCameraVisibleSet */ u8 unk69C[0x6A8 - 0x69C]; int freezeAnimation; /* +0x6a8: pxyForcedVisibilityRequested */ u8 unk6AC[0x6EC - 0x6AC]; int modelHiddenFlag; /* +0x6ec: set when the model is hidden */ u8 unk6F0[0x740 - 0x6F0]; Vector4 boundSphere; /* +0x740: pxyBoundSphere, centre and radius */ u8 unk750[0x820 - 0x750]; struct PxyBatch *unk820[0xA0]; /* +0x820 */ u8 unkAA0[0x49C0 - 0xAA0]; PxyDrawSlot recentDrawSlots[PXY_SLOT_COUNT]; /* +0x49c0 */ u8 unk4A00[0x10880 - 0x4A00]; int flagSearchHeaders[256]; /* +0x10880: headers of the flags-0x10 search, count at +0x5c0; bound from the object extent up to pxyDrawEntryTable at 0x97A980 */ } PxyModelRenderState;

typedef struct PxyBatch {
    s16 firstDrawEntry; /* +0x00: first entry in pxyDrawEntryTable */
    s16 drawEntryCount;
    float depth;
    u32 batchFlags;
    PxyPacket *packet; /* +0x0c */
    s16 unk10;
    s16 nextBatch; /* +0x12: index of the next batch, < 0 at the end */
    struct PxyBatch *translucent; /* +0x14 */
    u8 unk18[8];
} PxyBatch;

/* One 16-byte record of the pxy entry table pxyEntryQueue. */
typedef struct PxyPacketRequest {
    void *packetData; /* +0x00: the packet data, copied to the attribute buffer when the key is even */
    unsigned short quadwordCount; /* +0x04: low half of the second argument */
    unsigned char groupBit; /* +0x06: bit index into pxyEntryGroupMask */
    unsigned char sendFlag; /* +0x07 */
    int sendState; /* +0x08 */
    unsigned char row; /* +0x0C: key / 12 */
    unsigned char unmodeled_0d[3];
} PxyPacketRequest;

extern PxyPacketRequest pxyEntryQueue[400];
extern unsigned char pxyResetWork[48];
void xglVectorClear(Vector4 *destination);
extern int nmlRenderStatus;
extern int pxyActiveCameraIndex;
extern int pxyShadowMode;
extern PxyHideMask *pxyHideMask;
extern PxyModelHeader *pxyCurrentHeader;
extern PxyDrawPartRange *pxyPartRanges;
extern void *pxyHeaderTable58;
extern void *pxyHeaderTable6C;
extern PxyPart *pxyParts;
extern void *pxyDefaultPartMatrices;
extern void *pxyDefaultJointMatrices;
extern Matrix *pxyJointMatrices;
extern void *pxyHeaderTable64;
extern u8 *pxyPartVisibilityFlags;
extern TxyBlock *pxyTxyBlock;
extern void *pxyHeaderTable3C;
extern PxyModelHeader *pxyParentHeader;
extern float pxyReflTransparency;
extern float pxyTransparency;
extern float pxyCircleShadowScale;
extern void *pxyHeaderTable74;
extern void *pxyHeaderTable78;
extern Matrix *pxyPartMatrices;
extern f32 pxyFocusX;
extern f32 pxyFocusY;
extern void *pxyGeometryFogFactors;
extern XglLightSet *pxyCurrentLight;
extern PxyShadow *pxyCurrentShadow;
extern int pxyRenderLevel;
extern float pxyModelScaleMax;
extern int pxyClipEnabled;
extern Matrix *pxyShadowMatrix;
extern int pxyVisibleOverride;
extern Matrix *pxyMotionGlobalMatrices;
extern s16 pxyClipPlaneCount;
extern PxyPlaneTable *pxyClipPlanes;
extern Matrix *pxyGlobalMatrices;
extern u32 *pxyClipPlaneMask;
extern f32 pxyShadowHeightLower;
extern void *pxySecondPartMask;
extern f32 pxyShadowHeightUpper;
extern u8 *pxyColorIndexSource;
extern int pxyStateFlags;
extern float pxyFadeAlpha;
extern Matrix nmlModelDrawMatrix;
extern Matrix nmlModelDrawInverseMatrix;
extern AlignedVector4 pxyModelScale;
extern int pxyColorLimitR;
extern int pxyColorLimitG;
extern int pxyColorLimitB;
extern int pxyColorLimitA;
extern Vector4 pxyModelPosition;
extern int pxyLatchedActorState;
extern int pxyLatchedSecondState;
extern float nmlModelSortOffset;
extern int pxyForcedVisibilityRequested;
extern f32 pxyShadowScale;
void pxyResetGlobals(void);
extern int pxyEntryGroupMask;
extern int pxyEntryCount;

typedef int s32;

extern s32 pxyFlushDoneFlag;

#include "quadword.h"


void pxyAbortModel(void) {
    pxyResetGlobals();
}

typedef signed char s8;
extern unsigned char D_008C2FF0[];

u8 *pxyGetBoundSphere(void) {
    return D_008C2FF0;
}

extern s32 pxyModelHiddenFlag[];

s32 pxyIsModelHidden(void) {
    return pxyModelHiddenFlag[0];
}

extern s32 D_008C2F84[];
void pxyInitModuleState();
void pxyPacketDisablePending();

void pxyResetAndDisablePending(void) {
    pxyInitModuleState();
    D_008C2F84[0] = 0;
    pxyPacketDisablePending();
}

extern PxyModelRenderState gPxyModelRenderStateStorage;

void pxySetTexMapDropAll(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x400;
}

void pxyEnableFrameMaskPass(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x800;
}

void pxyForceCameraVisibleSet(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x1000;
}

void pxyGlowPendingSet(void) {
    gPxyModelRenderStateStorage.drawFlags |= 4;
}

extern s32 pxyModelFlags[];

s32 pxyGlowPendingGet(void) {
    return pxyModelFlags[0] & 4;
}

void pxyStateSetFlag3E4Bit3(void)
{
  gPxyModelRenderStateStorage.stateFlags3E4 |= 8;
}

s32 pxyWaterReflectionPendingGet(void) {
    return pxyModelFlags[0] & 1;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F678);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F778);

void *xglPacketReserve(int count);

void pxySetClipPlaneMasked(PxyRowBits *plane, PxyRowBits *angle, int maskedPlaneCount, float x, float y, float z, u32 *planeMask) {
    if (plane->rowWords[0] != 0 || plane->rowWords[1] != 0 || plane->rowWords[2] != 0 || angle->rowWords[0] != 0 || angle->rowWords[1] != 0 || angle->rowWords[2] != 0) {
        PxyModelRenderState *renderState = &gPxyModelRenderStateStorage;
        PxyPlaneTable *planes;

        if (renderState->clipPlanes == 0) {
            renderState->clipPlanes = xglPacketReserve(5);
        }
        planes = renderState->clipPlanes;
        planes->plane[0] = *plane;
        renderState->clipPlaneCount = 1;
        planes->angle[0] = *angle;
        renderState->clipPlaneMaskedCount = maskedPlaneCount;
        renderState->clipPlaneMask = planeMask;
        renderState->clipPlaneMaskedZ = z;
        renderState->clipPlaneMaskedX = x;
        renderState->renderStatus |= 0x8000;
        renderState->clipPlaneMaskedY = y;
        renderState->renderLevel |= 8;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019FA20);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019FB28);

extern f32 pxyTrackValueWindow[];

/* One track record: the kind and mode bytes are the only fields read here. */
typedef struct PxyTrackRecord {
    u8 unk00[0x18];
    s8 trackValueKind; /* +0x18 */
    s8 trackValueMode; /* +0x19 */
} PxyTrackRecord;

void pxyTrackValueFromRecord(s32 unused_arg0, PxyTrackRecord *trackRecord, s32 trackIndex)
{
  f32 *trackValueSlot;
  f32 zeroKindTrackValue;
  f32 matchedKindValue;
  s32 trackValueMode;
  s32 trackValueKind;
  s32 trackValueIndex;
  trackValueMode = trackRecord->trackValueMode;
  if (trackValueMode == 1)
  {
    trackValueKind = trackRecord->trackValueKind;
    trackValueIndex = trackIndex + 1;
    trackValueSlot = &pxyTrackValueWindow[trackValueIndex];
    if (trackValueKind == 0)
    {
      zeroKindTrackValue = pxyTrackValueWindow[-98]; /* gPxyModelRenderState +0x368 */
      *trackValueSlot = zeroKindTrackValue;
      return;
    }
    if (trackValueKind == trackValueMode)
    {
      matchedKindValue = pxyTrackValueWindow[-97]; /* gPxyModelRenderState +0x36c */
      *trackValueSlot = matchedKindValue;
    }
  }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A0A10);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A0ED0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1140);

struct XglCamera *xglStudioSelectGetActiveCamera(int cameraIndex);
void _CurMatrixSet(Matrix4 *matrix);
void _CurSetViewScaleTrans(Vector4 *scale, Vector4 *translation);
int _CurRotTransPersClipScreen(ScreenVector4 *screenOut, const Vector4 *point);
float tanf(float x);

float pxyCalcFadeMetric(int cameraIndex, const Vector4 *point, float scale) {
    ScreenVector4 screen;
    float fadeMetric;
    XglCamera *camera;

    fadeMetric = 0.0f;
    camera = xglStudioSelectGetActiveCamera(cameraIndex);
    if (camera == 0) {
        return fadeMetric;
    }
    _CurMatrixSet(&camera->volume[0].viewMatrix);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
    _CurRotTransPersClipScreen(&screen, point);
    fadeMetric = (float)screen.z * scale / tanf(camera->travel.fov * 0.5f);
    if (fadeMetric < 0.0f) {
        fadeMetric = 10000000.0f;
    }
    return fadeMetric;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A12A0);

extern int pxyCameraEnabled[4];
void func_00196200(Matrix4 destination);
void _ModelCalcClipInitMat1(float (*planeSet)[4], Matrix4 matrix);
int _ModelCalcClipMat1(Vector4 *point);
int xglCullingPollCheck(XglCamera *camera, Vector4 *sphere);

void pxyCalcCameraVisibility(void)
{
    int i;
    int anyVisible;
    Matrix4 clipMatrix;
    PxyModelHeader *modelHeader;
    int forceMask;
    XglCamera *camera;
    int visible;

    anyVisible = 0;
    modelHeader = gPxyModelRenderStateStorage.modelHeader;
    forceMask = (gPxyModelRenderStateStorage.renderStatus & 0x200000) | (gPxyModelRenderStateStorage.drawFlags & 0x1000);
    func_00196200(clipMatrix);
    for (i = 0; i < 4; i++) {
        camera = xglStudioSelectGetActiveCamera(i);
        if (pxyCameraEnabled[i] != 0 && camera != 0) {
            if ((gPxyModelRenderStateStorage.renderStatus & 0x800) || (gPxyModelRenderStateStorage.renderLevel & 0x8) || (modelHeader->flags & 0x88) || forceMask != 0) {
                gPxyModelRenderStateStorage.cameraVisible[i] = 1;
            } else {
                _ModelCalcClipInitMat1(camera->volume[0].planeSet[0], clipMatrix);
                visible = _ModelCalcClipMat1(&gPxyModelRenderStateStorage.boundSphere);
                if (visible && (modelHeader->flags & 2)) {
                    visible = xglCullingPollCheck(camera, &gPxyModelRenderStateStorage.boundSphere) == 0;
                }
                if ((gPxyModelRenderStateStorage.cameraVisible[i] = visible)) {
                    anyVisible = 1;
                } else if ((gPxyModelRenderStateStorage.renderLevel & 0x80) && gPxyModelRenderStateStorage.globalMatrices != 0 && !(gPxyModelRenderStateStorage.drawFlags & 0x10)) {
                    int j;

                    _ModelCalcClipInitMat1(camera->volume[0].planeSet[0], clipMatrix);
                    for (j = 0; j < modelHeader->nJointMatrix; j++) {
                        if (_ModelCalcClipMat1(&gPxyModelRenderStateStorage.globalMatrices[j].vectors[3])) {
                            gPxyModelRenderStateStorage.cameraVisible[i] = 1;
                            anyVisible = 1;
                            break;
                        }
                    }
                }
            }
            if (gPxyModelRenderStateStorage.renderStatus & 0x2) {
                gPxyModelRenderStateStorage.cameraVisible[i] = 1;
            }
        }
    }
    if (anyVisible == 0) {
        if (!(gPxyModelRenderStateStorage.renderStatus & 0x800) && !(gPxyModelRenderStateStorage.renderLevel & 0x8) && !(modelHeader->flags & 0x88) && forceMask == 0) {
            gPxyModelRenderStateStorage.renderStatus |= 0x10;
        }
    }
    if (gPxyModelRenderStateStorage.freezeAnimation != 0 && gPxyModelRenderStateStorage.visibleOverride == 0) {
        gPxyModelRenderStateStorage.renderStatus |= 0x10;
    }
    if (gPxyModelRenderStateStorage.renderStatus & 0x10) {
        gPxyModelRenderStateStorage.modelHiddenFlag = 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1688);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A17F8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1B10);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1D18);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3638);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3D90);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3E30);

void nmlModelDirectSend(int key, void *packetData, int qwc)
{
    nmlModelDirectSendFull(key, packetData, qwc, 0, 0);
}

void nmlModelDirectSendWithState(s32 key, void *packetData, s32 qwc, s32 sendState) {
    nmlModelDirectSendFull(key, packetData, qwc, 0, sendState);
}

void nmlModelDirectSendWithFlag(s32 key, void *packetData, s32 qwc, s32 sendFlag) {
    nmlModelDirectSendFull(key, packetData, qwc, sendFlag, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", nmlModelDirectSendFull);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A4058);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A40C8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A5108);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A52C8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A57A0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A5D30);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A62A8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A6660);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A69D8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A7690);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A8280);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A9E00);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA1B0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA580);

extern s32 gPxyDirtyFlag;
s32 func_001AA580();

void pxyStateConsumeDirty(void) {
    if (gPxyDirtyFlag != 0) {
        gPxyModelRenderStateStorage.drawFlags |= 0x100;
        func_001AA580();
        gPxyDirtyFlag = 0;
    }
}

void pxyFinishInit();
void pxyInitPacketSlots();
void pxyInitGlobals();
s32 func_0019E410();

void pxyInitModuleState(void) {
    pxyInitPacketSlots();
    pxyInitGlobals();
    func_0019E410();
    pxyFinishInit();
}

void xglPacketAddVifFlushIrq(void);

void pxyVif1EmptyPacketStart(void) {
    xglPacketAddVifFlushIrq();
}

void pxyVif1EmptyPacketRestart(void) {
    xglPacketAddVifFlushIrq();
}

extern s32 D_008C2F78[];

s32 pxyWorkInt2F78Get(void) {
    return D_008C2F78[0];
}

extern f32 D_008C2F7C[];

f32 pxyWorkFloat2F7CGet(void) {
    return D_008C2F7C[0];
}

extern f32 D_008C2F80[];

f32 pxyWorkFloat2F80Get(void) {
    return D_008C2F80[0];
}

extern f32 D_008C2F90[];

f32 pxyWorkFloat2F90Get(void) {
    return D_008C2F90[0];
}

extern f32 D_008C2F94[];

f32 pxyWorkFloat2F94Get(void) {
    return D_008C2F94[0];
}

extern s32 D_008C2F74[];

void pxyWorkFlag2F74Set(void) {
    D_008C2F74[0] = 1;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA828);

s32 func_001A3D90(s32 mode);

void pxyPlayEntryGroupFive(void) {
    func_001A3D90(5);
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA968);
