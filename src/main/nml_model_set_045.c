#include "common.h"
#include "shared.h"
#include "main/tu045.h"
#include "main/tu141.h"
#include "main/xgl_2.h"

/* Row-major 4x4 float matrix. */
/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
void xglMatrixUnit(Matrix4 destination);
struct PxyPartRange;
union Matrix;
struct PxyShadow;
struct PxyHideMask;
void *nmlPacketSetAttributeData16N(void *matrices, int count);
void nmlModelDirectSend(int key, void *data, int qwc);
void nmlModelDirectSendFull(int key, void *data, int qwc, int flag, int state);
void pxyAbortModel(void);
struct PxyHeader;
void *memset(void *destination, int value, unsigned int size);

/* pxySetClipPlaneWithAngle fills it (at most two entries, five quadwords allocated). It is the PxyClipPlanes of a packet
 * with float-array rows: indexing a row as v[i] folds the field offset into the address the way the original does, a
 * vector member does not (pxyClipPlanes, the table under construction, is the only user). */
/* A plane or angle row as pxySetClipPlaneMasked reads it: words 0..2 are compared as integers. */
typedef union PxyRowBits {
    Vector4Elements row;
    u32 words[4];
} PxyRowBits;

typedef struct PxyPlaneTable {
    PxyRowBits plane[2]; /* +0x00: plane normal and distance */
    PxyRowBits angle[2]; /* +0x20: rotX, rotY, then the next rotX, rotY */
} PxyPlaneTable;

#define PXY_SLOT_COUNT 4 /* ring of recently drawn packets */

typedef struct PxySlot {
    PxyPacket *packet;
    int batch;
    int translucentBatch;
    int unk0C;
} PxySlot;

typedef union PxyBestRecord {
    struct {
        float best;  /* +0x00 */
        int index;   /* +0x04 */
        int count;   /* +0x08 */
        int unk0C;   /* +0x0c: not read here */
    };
    int words[4];
    u64 doublewords[2]; /* the records are doubleword aligned */
} PxyBestRecord;

typedef struct PxyState { XglLightSet light; u8 unk180[0x300 - 0x180]; u32 renderStatus; /* +0x300: nmlRenderStatus, the flag word of the model code */ u8 unk304[0x314 - 0x304]; PxyHeader *currentHeader; /* +0x314: pxyCurrentHeader */ u8 unk318[0x394 - 0x318]; int renderLevel; /* +0x394: pxyRenderLevel */ u8 unk398[0x3AC - 0x398]; int visibleOverride; /* +0x3ac */ Matrix *motionGlobalMatrices; /* +0x3b0 */ s16 clipPlaneCount; /* +0x3b4 */ u16 clipPlaneMaskedCount; /* +0x3b6 */ PxyPlaneTable *clipPlanes; /* +0x3b8 */ Matrix *globalMatrices; /* +0x3bc: pxyGlobalMatrices, one 0x40-byte matrix per joint */ float clipPlaneMaskedX; /* +0x3c0 */ u32 *clipPlaneMask; /* +0x3c4 */ float clipPlaneMaskedY; /* +0x3c8 */ float clipPlaneMaskedZ; /* +0x3cc */ u8 unk3D0[0x480 - 0x3D0]; u32 cameraVisible[4]; /* +0x480: per camera, nonzero when the model is visible */ u8 unk490[0x4E0 - 0x490]; float trackValues[8][4]; /* +0x4e0: result stored at index k + 1 */ PxyBestRecord bestRecords[13]; /* +0x560: written by pxyEvaluateTracks, read by pxyCalcFogFactors */ void (*drawCallbacks[16])(int); /* +0x630: registered by pxyRegisterDrawCallback, called with the camera index */ u8 unk670[0x698 - 0x670]; u32 flags; /* +0x698: the pxy flag word, bit 0x1000 is set by func_0019F610 */ u8 unk69C[0x6A8 - 0x69C]; int forcedVisibilityRequested; /* +0x6a8: pxyForcedVisibilityRequested */ u8 unk6AC[0x6EC - 0x6AC]; int modelHiddenFlag; /* +0x6ec: set when the model is hidden */ u8 unk6F0[0x740 - 0x6F0]; Vector4 boundSphere; /* +0x740: pxyBoundSphere, centre and radius */ u8 unk750[0x820 - 0x750]; struct PxyBatch *unk820[0xA0]; /* +0x820 */ u8 unkAA0[0x49C0 - 0xAA0]; PxySlot slots[PXY_SLOT_COUNT]; /* +0x49c0 */ u8 unk4A00[0x10880 - 0x4A00]; int headerList[256]; /* +0x10880: headers of the flags-0x10 search, count at +0x5c0; bound from the object extent up to pxyDrawEntryTable at 0x97A980 */ } PxyState;

typedef struct PxyBatch {
    s16 start; /* +0x00: first entry in pxyDrawEntryTable */
    s16 count;
    float depth;
    u32 flags;
    PxyPacket *packet; /* +0x0c */
    s16 unk10;
    s16 next; /* +0x12: index of the next batch, < 0 at the end */
    struct PxyBatch *translucent; /* +0x14 */
    u8 unk18[8];
} PxyBatch;

/* One 16-byte record of the pxy entry table pxyEntryQueue. */
typedef struct PxyEntry {
    void *data; /* +0x00: the packet data, copied to the attribute buffer when the key is even */
    unsigned short value; /* +0x04: low half of the second argument */
    unsigned char group; /* +0x06: bit index into pxyEntryGroupMask */
    unsigned char flag; /* +0x07 */
    int state; /* +0x08 */
    unsigned char row; /* +0x0C: key / 12 */
    unsigned char unmodeled_0d[3];
} PxyEntry;

extern PxyState pxyState;
extern PxyEntry pxyEntryQueue[400];
extern unsigned char pxyResetWork[48];
void xglVectorClear(Vector4 *destination);
extern int nmlRenderStatus;
extern int pxyActiveCameraIndex;
extern int pxyShadowMode;
extern PxyHideMask *pxyHideMask;
extern PxyHeader *pxyCurrentHeader;
extern PxyPartRange *pxyPartRanges;
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
extern PxyHeader *pxyParentHeader;
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

void xglVectorClear(register Vector4 *destination);

void pxyAbortModel(void) {
    pxyResetGlobals();
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F598);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F5A8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F5B8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F5E0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F5F8);

extern PxyState D_008C28B0;

void func_0019F610(void) {
    D_008C28B0.flags |= 0x1000;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F628);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F640);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F650);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F668);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F678);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019F778);

void *func_001AE988(int count);

void pxySetClipPlaneMasked(PxyRowBits *plane, PxyRowBits *angle, int count, float x, float y, float z, u32 *mask) {
    if (plane->words[0] != 0 || plane->words[1] != 0 || plane->words[2] != 0 || angle->words[0] != 0 || angle->words[1] != 0 || angle->words[2] != 0) {
        PxyState *state = &D_008C28B0;
        PxyPlaneTable *planes;

        if (state->clipPlanes == 0) {
            state->clipPlanes = func_001AE988(5);
        }
        planes = state->clipPlanes;
        planes->plane[0] = *plane;
        state->clipPlaneCount = 1;
        planes->angle[0] = *angle;
        state->clipPlaneMaskedCount = (u16)count;
        state->clipPlaneMask = mask;
        state->clipPlaneMaskedZ = z;
        state->clipPlaneMaskedX = x;
        state->renderStatus |= 0x8000;
        state->clipPlaneMaskedY = y;
        state->renderLevel |= 8;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019FA20);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_0019FB28);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A09C0);

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
    float result;
    XglCamera *camera;

    result = 0.0f;
    camera = xglStudioSelectGetActiveCamera(cameraIndex);
    if (camera == 0) {
        return result;
    }
    _CurMatrixSet(&camera->volume[0].viewMatrix);
    _CurSetViewScaleTrans(&camera->screen.screenScale, &camera->screen.screenOffset);
    _CurRotTransPersClipScreen(&screen, point);
    result = (float)screen.z * scale / tanf(camera->travel.fov * 0.5f);
    if (result < 0.0f) {
        result = 10000000.0f;
    }
    return result;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A12A0);

extern int D_004C8B00[4];
void func_00196200(Matrix4 destination);
void _ModelCalcClipInitMat1(float (*planeSet)[4], Matrix4 matrix);
int _ModelCalcClipMat1(Vector4 *point);
int func_0019C800(XglCamera *camera, Vector4 *sphere);

void pxyCalcCameraVisibility(void)
{
    int i;
    int anyVisible;
    Matrix4 clipMatrix;
    PxyHeader *header;
    int forceMask;
    XglCamera *camera;
    int visible;

    anyVisible = 0;
    header = D_008C28B0.currentHeader;
    forceMask = (D_008C28B0.renderStatus & 0x200000) | (D_008C28B0.flags & 0x1000);
    func_00196200(clipMatrix);
    for (i = 0; i < 4; i++) {
        camera = xglStudioSelectGetActiveCamera(i);
        if (D_004C8B00[i] != 0 && camera != 0) {
            if ((D_008C28B0.renderStatus & 0x800) || (D_008C28B0.renderLevel & 0x8) || (header->flags & 0x88) || forceMask != 0) {
                D_008C28B0.cameraVisible[i] = 1;
            } else {
                _ModelCalcClipInitMat1(camera->volume[0].planeSet[0], clipMatrix);
                visible = _ModelCalcClipMat1(&D_008C28B0.boundSphere);
                if (visible && (header->flags & 2)) {
                    visible = func_0019C800(camera, &D_008C28B0.boundSphere) == 0;
                }
                if ((D_008C28B0.cameraVisible[i] = visible)) {
                    anyVisible = 1;
                } else if ((D_008C28B0.renderLevel & 0x80) && D_008C28B0.globalMatrices != 0 && !(D_008C28B0.flags & 0x10)) {
                    int j;

                    _ModelCalcClipInitMat1(camera->volume[0].planeSet[0], clipMatrix);
                    for (j = 0; j < header->nJointMatrix; j++) {
                        if (_ModelCalcClipMat1(&D_008C28B0.globalMatrices[j].vectors[3])) {
                            D_008C28B0.cameraVisible[i] = 1;
                            anyVisible = 1;
                            break;
                        }
                    }
                }
            }
            if (D_008C28B0.renderStatus & 0x2) {
                D_008C28B0.cameraVisible[i] = 1;
            }
        }
    }
    if (anyVisible == 0) {
        if (!(D_008C28B0.renderStatus & 0x800) && !(D_008C28B0.renderLevel & 0x8) && !(header->flags & 0x88) && forceMask == 0) {
            D_008C28B0.renderStatus |= 0x10;
        }
    }
    if (D_008C28B0.forcedVisibilityRequested != 0 && D_008C28B0.visibleOverride == 0) {
        D_008C28B0.renderStatus |= 0x10;
    }
    if (D_008C28B0.renderStatus & 0x10) {
        D_008C28B0.modelHiddenFlag = 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1688);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A17F8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1B10);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A1D18);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3638);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3D90);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3E30);

void nmlModelDirectSend(int key, void *data, int qwc)
{
    nmlModelDirectSendFull(key, data, qwc, 0, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3F30);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001A3F50);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA730);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA768);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA798);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA7B0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA7C8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA7D8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA7E8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA7F8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA808);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA818);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA828);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA950);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set_045", func_001AA968);
