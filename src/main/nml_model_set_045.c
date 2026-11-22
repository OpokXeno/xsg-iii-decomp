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

/* PxyModelRenderState: defined after PxyBatchTable below. */

typedef struct PxyBatch {
    s16 firstDrawEntry; /* +0x00: first entry in pxyDrawEntryTable */
    s16 drawEntryCount;
    float depth;
    u32 batchFlags;
    struct PxyDrawPacket *packet; /* +0x0c */
    s16 unk10;
    s16 nextBatch; /* +0x12: index of the next batch, < 0 at the end */
    struct PxyBatch *translucent; /* +0x14 */
    u8 unk18[8];
} PxyBatch;

typedef struct PxyDrawPacket {
    u32 renderStatus; /* +0x00 */
    int camera; /* +0x04: camera that owns the model */
    u8 unk08[0xAC - 0x08];
    int frameMask; /* +0xac */
    u8 unkB0[0xE4 - 0xB0];
    u32 flags; /* +0xe4: bit 0 skips the translucent passes, bit 2 draws with the alternate light tables */
    u8 unkE8[0x130 - 0xE8];
    Matrix worldMatrix; /* +0x130: X2's +0x120 */
    u8 unk170[0x180 - 0x170];
    int cameraMask[4]; /* +0x180: nonzero where a camera draws the model */
    u8 unk190[0x1D0 - 0x190];
    void (*preDraw)(int camera, int drawPass); /* +0x1d0 */
    u8 unk1D4[0x1E0 - 0x1D4];
    void (*postDraw)(int camera, int drawPass); /* +0x1e0 */
} PxyDrawPacket;

typedef struct PxyBatchTable {
    PxyBatch batches[256];
    int unk2000;
    int unk2004;
    int count; /* +0x2008 */
    int unk200C;
} PxyBatchTable;

typedef struct PxyModelRenderState { XglLightSet light; u8 unk180[0x300 - 0x180]; u32 renderStatus; /* +0x300: nmlRenderStatus, the flag word of the model code */ u8 unk304[0x314 - 0x304]; PxyModelHeader *modelHeader; /* +0x314: pxyCurrentHeader */ u8 unk318[0x394 - 0x318]; int renderLevel; /* +0x394: pxyRenderLevel */ u8 unk398[0x3AC - 0x398]; int visibleOverride; /* +0x3ac */ Matrix *motionGlobalMatrices; /* +0x3b0 */ s16 clipPlaneCount; /* +0x3b4 */ u16 clipPlaneMaskedCount; /* +0x3b6 */ PxyPlaneTable *clipPlanes; /* +0x3b8 */ Matrix *globalMatrices; /* +0x3bc: pxyGlobalMatrices, one 0x40-byte matrix per joint */ float clipPlaneMaskedX; /* +0x3c0 */ u32 *clipPlaneMask; /* +0x3c4 */ float clipPlaneMaskedY; /* +0x3c8 */ float clipPlaneMaskedZ; /* +0x3cc */ u8 unk3D0[0x3E4 - 0x3D0]; int stateFlags3E4; /* +0x3e4: bit 0x8 is set by pxyStateSetFlag3E4Bit3 */ u8 unk3E8[0x480 - 0x3E8]; u32 cameraVisible[4]; /* +0x480: per camera, nonzero when the model is visible */ u8 unk490[0x4E0 - 0x490]; float trackValues[8][4]; /* +0x4e0: result stored at index k + 1 */ PxyBestRecord bestRecords[13]; /* +0x560: written by pxyEvaluateTracks, read by pxyCalcFogFactors */ void (*drawCallbacks[16])(int); /* +0x630: registered by pxyRegisterDrawCallback, called with the camera index */ int overlayBatchCount; /* +0x670: batches of pxyState.overlayBatches */ u8 unk674[0x684 - 0x674]; PxyBatch *firstBatch; /* +0x684: head of the sorted batch list, linked by nextBatch */ u8 unk688[0x698 - 0x688]; u32 drawFlags; /* +0x698: the pxy flag word, bit 0x1000 is set by pxyForceCameraVisibleSet */ u8 unk69C[0x6A8 - 0x69C]; int freezeAnimation; /* +0x6a8: pxyForcedVisibilityRequested */ u8 unk6AC[0x6B0 - 0x6AC]; int camera; /* +0x6b0: camera being drawn */ int drawPass; /* +0x6b4: 0 opaque, 1 translucent; selects batchTables */ u8 unk6B8[0x6EC - 0x6B8]; int modelHiddenFlag; /* +0x6ec: set when the model is hidden */ u8 unk6F0[0x740 - 0x6F0]; Vector4 boundSphere; /* +0x740: pxyBoundSphere, centre and radius */ u8 unk750[0x770 - 0x750]; PxyBatch *overlayBatches[32]; /* +0x770: drawn before and after the sorted batches */ u8 unk7F0[0xA70 - 0x7F0]; PxyBatchTable batchTables[2]; /* +0xa70: one per draw pass */ u8 unk4A90[0x10880 - 0x4A90]; int flagSearchHeaders[256]; /* +0x10880: headers of the flags-0x10 search, count at +0x5c0; bound from the object extent up to pxyDrawEntryTable at 0x97A980 */ } PxyModelRenderState;

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
    xglMatrixUnit(clipMatrix);
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

typedef struct PxyShuffleTable {
    int pairs[8][2];
} PxyShuffleTable;

extern const PxyShuffleTable pxyHeaderShuffleTable;

/* Undoes the in-place scramble of a pxy modelHeader when flags bit 31 is set. */
void pxyUnscrambleHeader(PxyModelHeader *modelHeader)
{
    u8 *scrambledHeaderBytes;
    int i;
    u32 j;
    u8 pairSwapTemp;
    PxyShuffleTable shuffleTable;
    u8 shuffleTemp;

    if (modelHeader->flags & 0x80000000) {
        scrambledHeaderBytes = (u8 *)modelHeader;
        shuffleTable = pxyHeaderShuffleTable;
        for (i = *modelHeader->fileSize - 1; i >= 16; i--) {
            j = (2121 * i - 909) * (i - 1) + 0x16253495;
            j = j - 1;
            j = j % i;
            if (scrambledHeaderBytes[i] && scrambledHeaderBytes[j] && j >= 16) {
                shuffleTemp = scrambledHeaderBytes[i];
                scrambledHeaderBytes[i] = scrambledHeaderBytes[j];
                scrambledHeaderBytes[j] = shuffleTemp;
            }
        }
        for (i = 4; i < (*modelHeader->fileSize >> 2); i++) {
            if (i * 4 >= *modelHeader->fileSize) {
                continue;
            }
            if (scrambledHeaderBytes[i * 4] || scrambledHeaderBytes[i * 4 + 1] || scrambledHeaderBytes[i * 4 + 2] || scrambledHeaderBytes[i * 4 + 3]) {
                pairSwapTemp = scrambledHeaderBytes[i * 4 + shuffleTable.pairs[i % 8][0]];
                scrambledHeaderBytes[i * 4 + shuffleTable.pairs[i % 8][0]] = scrambledHeaderBytes[i * 4 + shuffleTable.pairs[i % 8][1]];
                scrambledHeaderBytes[i * 4 + shuffleTable.pairs[i % 8][1]] = pairSwapTemp;
            }
        }
        modelHeader->flags &= 0x7fffffff;
    }
}

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

#include "gs.h"

/* The X3 layout of the packet a batch draws: X2's PxyPacket with 0x10 more bytes below +0x9c. */

typedef union XglDirectQueueEntry {
    struct {
        u64 registerValue;
        u64 registerAddress;
    };
    u32 tagAndHeaderWords[4];
} XglDirectQueueEntry;

/* ---- pxyDrawBatches ---- */
/* The render targets of xgl_render.c's state block (D_004C8390). */
typedef struct XglRenderTargets {
    u8 unk00[2];
    s16 psm;                 /* +0x02 */
    s16 screenWidth;         /* +0x04 */
    s16 screenHeight;        /* +0x06 */
    u16 depthBase;           /* +0x08 */
    u8 unk0A[0x12 - 0x0A];
    u16 renderTextureBase;   /* +0x12 */
} XglRenderTargets;

extern XglRenderTargets D_004C8390;
extern XglDirectQueueEntry *xglDirectQueueBuffer;
extern int xglDirectQueueCount;

/* Per-pass values of the frame blur, copied to the stack (unsized: .rodata, not gp data): the scaffold .rodata images at 0x541E78 (alphas 40, 48)
 * and 0x541E80 (texel shifts 2, 1). */
typedef struct BlurPassValues {
    int value[2];
} BlurPassValues;

extern const BlurPassValues pxyBlurPassAlphaTable[];
extern const BlurPassValues D_00541E80[];
void nmlPacketClrModelCache(void);
void nmlPacketClrMatrixCache(void);
void nmlPacketClrTextureCache(void);
void pxyPacketSetCacheValue(s32 cacheValue);
void pxyPacketUploadLightTables(s32 alternate);
float func_001AEE50(void);
float func_001AEF08(void);
void nmlPacketGsInit(void);
void nmlPacketAddGsFlush(void);
void nmlPacketAddGsFlushWide(void);
void func_0019D990(int frameMask, u32 frameBase);
void func_001A3E30(int group, int camera);
void func_001A69D8(int mode, PxyBatch *batch, u32 frameBase, int flags);
void func_001A7690(int mode, PxyBatch *batch, u32 frameBase, int flags);
void func_001A62A8(PxyBatch *batch, u32 frameBase);
void func_001A5D30(PxyBatch *batch, u32 frameBase);
void func_001A6660(PxyBatch *batch, u32 frameBase);
void func_001A52C8(u32 frameBase);
void func_001A5108(u32 frameBase);
void func_001A57A0(u32 frameBase);

#define pxyState gPxyModelRenderStateStorage
#define SCREEN_HALF_W (D_004C8390.screenWidth / 2)
#define SCREEN_HALF_H (D_004C8390.screenHeight / 2)

void pxyDrawBatches(int drawMode, u32 frameBase)
{
    XglDirectQueueEntry **queueBuffer = &xglDirectQueueBuffer;
    int *queueCount = &xglDirectQueueCount;
    int lightFlags;
    PxyBatchTable *batchTable;
    int i;
    PxyBatch *batch;

    nmlPacketClrModelCache();
    nmlPacketClrMatrixCache();
    pxyState.drawPass = 0;
    pxyPacketSetCacheValue(pxyState.drawPass);
    for (i = 0; i < pxyState.overlayBatchCount; i++) {
        batch = pxyState.overlayBatches[i];
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0)
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            continue;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        if (drawMode == 0 && batch->packet->preDraw != 0) {
            batch->packet->preDraw(pxyState.camera, pxyState.drawPass);
        }
        func_001A69D8(drawMode, batch, frameBase, 0);
        if (drawMode == 0 && batch->packet->postDraw != 0) {
            batch->packet->postDraw(pxyState.camera, pxyState.drawPass);
        }
    }
    if (drawMode == 0) {
        func_001A3E30(4, pxyState.camera);
    }
    lightFlags = 0;
    batchTable = &pxyState.batchTables[pxyState.drawPass];
    batch = pxyState.firstBatch;
    while (pxyState.firstBatch != 0) {
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0 && !(batch->packet->renderStatus & 0x10))
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            if (batch->nextBatch < 0) {
                break;
            }
            batch = &batchTable->batches[batch->nextBatch];
            continue;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        if ((lightFlags & 1) && !(batch->packet->flags & 4)) {
            pxyPacketUploadLightTables(0);
            lightFlags &= ~1;
        }
        if (batch->packet->flags & 4) {
            pxyPacketUploadLightTables(1);
            lightFlags |= 1;
        }
        if (drawMode == 0 && batch->packet->preDraw != 0) {
            batch->packet->preDraw(pxyState.camera, pxyState.drawPass);
        }
        if (batch->packet->renderStatus & 0x80000) {
            u64 *queueWrite;
            u64 *queueStart;
            int queueIndex;

            func_0019D990(batch->packet->frameMask, frameBase);
            nmlPacketGsInit();
            queueIndex = xglDirectQueueCount;
            queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
            queueStart = queueWrite;
            GS_PUT(queueWrite, GS_FBA_1, 1);
            *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
            nmlPacketAddGsFlush();
        }
        func_001A69D8(drawMode, batch, frameBase, 0);
        if (batch->packet->renderStatus & 0x80000) {
            int y;
            union { int rgba[4]; } color;

            color.rgba[0] = 0x80;
            color.rgba[1] = 0x80;
            color.rgba[2] = 0x80;
            color.rgba[3] = 0x80;
            {
                u64 *queueWrite;
                u64 *queueStart;
                int queueIndex;

                nmlPacketGsInit();
                queueIndex = xglDirectQueueCount;
                queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                queueStart = queueWrite;
                GS_PUT(queueWrite, GS_FBA_1, 0);
                *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                nmlPacketAddGsFlush();
            }
            for (y = 0; y < D_004C8390.screenHeight; y += SCREEN_HALF_H) {
                int pass;

                {
                    u64 *queueWrite;
                    u64 *queueStart;
                    int queueIndex;

                    nmlPacketGsInit();
                    queueIndex = xglDirectQueueCount;
                    queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                    queueStart = queueWrite;
                    GS_PUT(queueWrite, GS_FRAME_1, GS_SET_FRAME(D_004C8390.renderTextureBase, D_004C8390.screenWidth / 64, D_004C8390.psm, 0));
                    GS_PUT(queueWrite, GS_TEXFLUSH, 0);
                    GS_PUT(queueWrite, GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, 1, 1));
                    GS_PUT(queueWrite, GS_CLAMP_1, 0);
                    GS_PUT(queueWrite, GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0, 0x3F800000));
                    GS_PUT(queueWrite, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
                    GS_PUT(queueWrite, GS_TEX0_1, GS_SET_TEX0(frameBase << 5, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 1));
                    GS_PUT(queueWrite, GS_TEX1_1, 0);
                    GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
                    GS_PUT(queueWrite, GS_UV, GS_SET_UV(0, (y << 4)));
                    GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((0x800 - SCREEN_HALF_W) << 4), ((0x800 - SCREEN_HALF_H) << 4), 0));
                    GS_PUT(queueWrite, GS_UV, GS_SET_UV((D_004C8390.screenWidth << 4), ((SCREEN_HALF_H + y) << 4)));
                    GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W + 0x7FF) << 4), (0x7FF << 4), 0));
                    GS_PUT(queueWrite, GS_RGBAQ, GS_SET_RGBAQ(color.rgba[0], color.rgba[1], color.rgba[2], color.rgba[3], 0x3F800000));
                    GS_PUT(queueWrite, GS_TEST_1, GS_SET_TEST(1, 5, 0x80, 0, 0, 0, 1, 1));
                    GS_PUT(queueWrite, GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
                    GS_PUT(queueWrite, GS_FBA_1, 1);
                    *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                    nmlPacketAddGsFlushWide();
                }
                for (pass = 0; pass < 1; pass++) {
                    BlurPassValues alphas = pxyBlurPassAlphaTable[0];
                    BlurPassValues shifts = D_00541E80[0];
                    int alpha;
                    int shift;

                    alpha = alphas.value[pass];
                    shift = shifts.value[pass];
                    {
                        u64 *queueWrite;
                        u64 *queueStart;
                        int queueIndex;

                        nmlPacketGsInit();
                        queueIndex = xglDirectQueueCount;
                        queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                        queueStart = queueWrite;
                        GS_PUT(queueWrite, GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, alpha));
                        GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV(9, (y << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((shift - SCREEN_HALF_W + 0x800) << 4), ((0x800 - SCREEN_HALF_H) << 4), 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV(((D_004C8390.screenWidth - shift) << 4) + 9, ((SCREEN_HALF_H + y) << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W + 0x800) << 4), 0x8000, 0));
                        *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                        nmlPacketAddGsFlushWide();
                    }
                    {
                        u64 *queueWrite;
                        u64 *queueStart;
                        int queueIndex;

                        nmlPacketGsInit();
                        queueIndex = xglDirectQueueCount;
                        queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                        queueStart = queueWrite;
                        GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV((shift << 4) + 9, (y << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((0x800 - SCREEN_HALF_W) << 4), ((0x800 - SCREEN_HALF_H) << 4), 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV((D_004C8390.screenWidth << 4) + 9, ((SCREEN_HALF_H + y) << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W - shift + 0x800) << 4), 0x8000, 0));
                        *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                        nmlPacketAddGsFlushWide();
                    }
                    {
                        u64 *queueWrite;
                        u64 *queueStart;
                        int queueIndex;

                        nmlPacketGsInit();
                        queueIndex = xglDirectQueueCount;
                        queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                        queueStart = queueWrite;
                        GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV(8, ((shift + y) << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((0x800 - SCREEN_HALF_W) << 4), ((0x800 - SCREEN_HALF_H) << 4), 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV((D_004C8390.screenWidth << 4) + 8, ((SCREEN_HALF_H + y) << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W + 0x800) << 4), ((0x800 - shift) << 4), 0));
                        *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                        nmlPacketAddGsFlushWide();
                    }
                    {
                        u64 *queueWrite;
                        u64 *queueStart;
                        int queueIndex;

                        nmlPacketGsInit();
                        queueIndex = xglDirectQueueCount;
                        queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                        queueStart = queueWrite;
                        GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 1, 0, 1, 0, 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV(8, (y << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((0x800 - SCREEN_HALF_W) << 4), ((shift - SCREEN_HALF_H + 0x800) << 4), 0));
                        GS_PUT(queueWrite, GS_UV, GS_SET_UV((D_004C8390.screenWidth << 4) + 8, ((SCREEN_HALF_H + y - shift) << 4) + 9));
                        GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W + 0x800) << 4), 0x8000, 0));
                        *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                        nmlPacketAddGsFlushWide();
                    }
                }
                {
                    u64 *queueWrite;
                    u64 *queueStart;
                    int queueIndex;

                    nmlPacketGsInit();
                    queueIndex = xglDirectQueueCount;
                    queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                    queueStart = queueWrite;
                    GS_PUT(queueWrite, GS_FBA_1, 0);
                    GS_PUT(queueWrite, GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
                    GS_PUT(queueWrite, GS_TEST_1, GS_SET_TEST(1, 5, 0x80, 0, 0, 0, 1, 1));
                    GS_PUT(queueWrite, GS_TEX1_1, 0);
                    GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
                    GS_PUT(queueWrite, GS_UV, GS_SET_UV(0, (y << 4)));
                    GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((0x800 - SCREEN_HALF_W) << 4), ((0x800 - SCREEN_HALF_H) << 4), 0));
                    GS_PUT(queueWrite, GS_UV, GS_SET_UV((D_004C8390.screenWidth << 4), ((SCREEN_HALF_H + y) << 4)));
                    GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W + 0x7FF) << 4), (0x7FF << 4), 0));
                    *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                    nmlPacketAddGsFlushWide();
                }
                {
                    u64 *queueWrite;
                    u64 *queueStart;
                    int queueIndex;

                    nmlPacketGsInit();
                    queueIndex = xglDirectQueueCount;
                    queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
                    queueStart = queueWrite;
                    GS_PUT(queueWrite, GS_FRAME_1, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm, 0));
                    GS_PUT(queueWrite, GS_TEXFLUSH, 0);
                    GS_PUT(queueWrite, GS_RGBAQ, GS_SET_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x3F800000));
                    GS_PUT(queueWrite, GS_TEX0_1, GS_SET_TEX0(D_004C8390.renderTextureBase << 5, 8, 0, 9, 9, 1, 0, 0, 0, 0, 0, 1));
                    GS_PUT(queueWrite, GS_TEST_1, GS_SET_TEST(1, 5, 0x80, 0, 1, 0, 1, 1));
                    GS_PUT(queueWrite, GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
                    GS_PUT(queueWrite, GS_PRIM, GS_SET_PRIM(6, 0, 1, 0, 0, 0, 1, 0, 0));
                    GS_PUT(queueWrite, GS_UV, GS_SET_UV(0, 0));
                    GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((0x800 - SCREEN_HALF_W) << 4), ((y - SCREEN_HALF_H + 0x800) << 4), 0));
                    GS_PUT(queueWrite, GS_UV, GS_SET_UV((D_004C8390.screenWidth << 4), (SCREEN_HALF_H << 4)));
                    GS_PUT(queueWrite, GS_XYZF2, GS_SET_XYZ(((D_004C8390.screenWidth - SCREEN_HALF_W + 0x7FF) << 4), ((y + 0x7FF) << 4), 0));
                    GS_PUT(queueWrite, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
                    *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
                    nmlPacketAddGsFlushWide();
                }
            }
            nmlPacketClrTextureCache();
        }
        if (drawMode == 0 && batch->packet->postDraw != 0) {
            batch->packet->postDraw(pxyState.camera, pxyState.drawPass);
        }
        if (batch->nextBatch < 0) {
            break;
        }
        batch = &batchTable->batches[batch->nextBatch];
    }
    if (!(pxyState.drawFlags & 0x40)) {
        pxyPacketUploadLightTables(0);
    }
    {
        u64 *queueWrite;
        u64 *queueStart;
        int queueIndex;

        nmlPacketGsInit();
        queueIndex = xglDirectQueueCount;
        queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
        queueStart = queueWrite;
        GS_PUT(queueWrite, GS_FRAME_1, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm, 0));
        GS_PUT(queueWrite, GS_FRAME_2, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm, 0xFF000000));
        *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
        nmlPacketAddGsFlush();
    }
    if (drawMode == 0) {
        func_001A3E30(0, pxyState.camera);
    }
    pxyState.drawPass = 1;
    pxyPacketSetCacheValue(pxyState.drawPass);
    batch = pxyState.firstBatch;
    lightFlags = 0;
    batchTable = &pxyState.batchTables[0];
    while (pxyState.firstBatch != 0) {
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0 && (batch->packet->renderStatus & 0x40000)
              && !(batch->packet->renderStatus & 0x10) && !(batch->packet->flags & 1))
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            if (batch->nextBatch < 0) {
                break;
            }
            batch = &batchTable->batches[batch->nextBatch];
            continue;
        }
        if ((lightFlags & 1) && !(batch->packet->flags & 4)) {
            pxyPacketUploadLightTables(0);
            lightFlags &= ~1;
        }
        if (batch->packet->flags & 4) {
            pxyPacketUploadLightTables(1);
            lightFlags |= 1;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            func_001A62A8(batch->translucent, frameBase);
        }
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            func_001A5D30(batch->translucent, frameBase);
        }
        func_001A69D8(drawMode, batch->translucent, frameBase, 0);
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            pxyState.drawPass = 0;
            pxyPacketSetCacheValue(pxyState.drawPass);
            func_001A6660(batch, frameBase);
            pxyState.drawPass = 1;
            pxyPacketSetCacheValue(pxyState.drawPass);
        }
        if (batch->nextBatch < 0) {
            break;
        }
        batch = &batchTable->batches[batch->nextBatch];
    }
    if (!(pxyState.drawFlags & 0x40)) {
        pxyPacketUploadLightTables(0);
    }
    if ((pxyState.drawFlags & 0x100000) && drawMode == 0) {
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        func_001A52C8(frameBase);
        func_001A5108(frameBase);
        func_001A57A0(frameBase);
    }
    for (i = 0; i < pxyState.overlayBatchCount; i++) {
        batch = pxyState.overlayBatches[i];
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0)
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            continue;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            func_001A62A8(batch->translucent, frameBase);
        }
        pxyState.drawPass = 0;
        pxyPacketSetCacheValue(pxyState.drawPass);
        if (drawMode == 0 || drawMode == 4) {
            func_001A5D30(batch->translucent, frameBase);
        }
        pxyState.drawPass = 1;
        pxyPacketSetCacheValue(pxyState.drawPass);
        func_001A69D8(drawMode, batch->translucent, frameBase, 0);
    }
    pxyState.drawPass = 0;
    pxyPacketSetCacheValue(pxyState.drawPass);
    for (i = 0; i < pxyState.overlayBatchCount; i++) {
        batch = pxyState.overlayBatches[i];
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0)
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            continue;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        func_001A7690(drawMode, batch, frameBase, 0);
    }
    pxyState.drawPass = 1;
    pxyPacketSetCacheValue(pxyState.drawPass);
    for (i = 0; i < pxyState.overlayBatchCount; i++) {
        batch = pxyState.overlayBatches[i];
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0)
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            continue;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            func_001A62A8(batch->translucent, frameBase);
        }
        if (drawMode == 0 || drawMode == 4) {
            func_001A5D30(batch->translucent, frameBase);
        }
        func_001A7690(drawMode, batch->translucent, frameBase, 0);
    }
    if (!(pxyState.drawFlags & 0x100000) && drawMode == 0) {
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        func_001A52C8(frameBase);
        func_001A5108(frameBase);
        func_001A57A0(frameBase);
    }
    lightFlags = 0;
    batchTable = &pxyState.batchTables[0];
    batch = pxyState.firstBatch;
    while (pxyState.firstBatch != 0) {
        if (!((pxyCameraEnabled[pxyState.camera] == 1 || pxyState.camera == batch->packet->camera)
              && batch->packet->cameraMask[pxyState.camera] != 0 && !(batch->packet->renderStatus & 0x40000)
              && !(batch->packet->renderStatus & 0x10) && !(batch->packet->flags & 1))
            || (drawMode == 4 && !(batch->packet->renderStatus & 0x20000) && !(batch->batchFlags & 0x20000))
            || (drawMode == 1 && !(batch->batchFlags & 0x4000000)) || (drawMode == 2 && !(batch->batchFlags & 0x2000000))) {
            if (batch->nextBatch < 0) {
                break;
            }
            batch = &batchTable->batches[batch->nextBatch];
            continue;
        }
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        if ((lightFlags & 1) && !(batch->packet->flags & 4)) {
            pxyPacketUploadLightTables(0);
            lightFlags &= ~1;
        }
        if (batch->packet->flags & 4) {
            pxyPacketUploadLightTables(1);
            lightFlags |= 1;
        }
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            func_001A62A8(batch->translucent, frameBase);
        }
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            func_001A5D30(batch->translucent, frameBase);
        }
        func_001A69D8(drawMode, batch->translucent, frameBase, 0);
        if (drawMode == 0 || drawMode == 2 || drawMode == 4) {
            pxyState.drawPass = 0;
            pxyPacketSetCacheValue(pxyState.drawPass);
            func_001A6660(batch, frameBase);
            pxyState.drawPass = 1;
            pxyPacketSetCacheValue(pxyState.drawPass);
        }
        if (batch->nextBatch < 0) {
            break;
        }
        batch = &batchTable->batches[batch->nextBatch];
    }
    if (!(pxyState.drawFlags & 0x40)) {
        pxyPacketUploadLightTables(0);
    }
    {
        u64 *queueWrite;
        u64 *queueStart;
        int queueIndex;

        nmlPacketGsInit();
        queueIndex = xglDirectQueueCount;
        queueWrite = (u64 *)&(*queueBuffer)[queueIndex];
        queueStart = queueWrite;
        GS_PUT(queueWrite, GS_FRAME_1, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm, 0));
        GS_PUT(queueWrite, GS_FRAME_2, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm, 0xFF000000));
        *queueCount = queueIndex + (((u32)queueWrite - (u32)queueStart) >> 4);
        nmlPacketAddGsFlush();
    }
    if (drawMode == 0) {
        if (func_001AEE50() > 85.0f || func_001AEF08() > 85.0f) {
            return;
        }
        func_001A3E30(1, pxyState.camera);
    }
}

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
s32 pxyClearEntryTable();

void pxyInitModuleState(void) {
    pxyInitPacketSlots();
    pxyInitGlobals();
    pxyClearEntryTable();
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
