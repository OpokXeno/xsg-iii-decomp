#include "common.h"
#include "shared.h"
#include "main/tu045.h"
#include "main/tu141.h"
#include "main/xgl_2.h"
#include "vu0_macro.h"
#include "quadword.h"

typedef int s32;

typedef signed char s8;

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

/* A clip plane while it is built: the plane equation, written as one vector or element by element. */

/* The clip plane table of a draw packet as this function fills it. */
typedef struct PxyClipPlaneTable {
    PxyVector plane[2];
    PxyVector angle[2];
} PxyClipPlaneTable;

typedef struct PxyDrawPacket {
    u32 renderStatus; /* +0x00 */
    int camera; /* +0x04: camera that owns the model */
    int drawMode; /* +0x08: 1 and 4 cast shadows */
    u8 unk0C[0x14 - 0x0C];
    PxyModelHeader *header; /* +0x14 */
    PxyDrawPartRange *ranges; /* +0x18: [0] opaque, [1] translucent */
    u8 unk1C[0x24 - 0x1C];
    PxyPart *parts; /* +0x24 */
    u8 unk28[0x38 - 0x28];
    u8 *partFlags; /* +0x38 */
    u8 unk3C[4];
    TxyBlock *textureTable; /* +0x40 */
    u8 unk44[0x64 - 0x44];
    union Matrix *boneMatrices; /* +0x64: one per part matrix index */
    u8 unk68[0x70 - 0x68];
    void *geometryFogFactors; /* +0x70 */
    XglLightSet *light; /* +0x74 */
    u8 unk78[0x90 - 0x78];
    struct PxyShadow *shadow; /* +0x90: the shadow projector */
    int renderLevel; /* +0x94 */
    u8 unk98[0xA8 - 0x98];
    float (*shadowMatrix)[4]; /* +0xa8 */
    int frameMask; /* +0xac */
    u8 unkB0[0xB4 - 0xB0];
    s16 planeCount; /* +0xb4 */
    u8 unkB6[2];
    struct PxyClipPlaneTable *planes; /* +0xb8: clip planes */
    u8 unkBC[0xD0 - 0xBC];
    float lowerClipHeight; /* +0xd0 */
    u8 unkD4[4];
    float upperClipHeight; /* +0xd8 */
    u8 unkDC[4];
    u8 *partIndexList; /* +0xe0: 16 part indices ended by 0xff */
    u32 flags; /* +0xe4: bit 0 skips the translucent passes, bit 2 draws with the alternate light tables */
    u8 unkE8[0xF0 - 0xE8];
    Matrix4 localMatrix; /* +0xf0: model to world */
    Matrix worldMatrix; /* +0x130: X2's +0x120 */
    u8 unk170[0x180 - 0x170];
    int cameraMask[4]; /* +0x180: nonzero where a camera draws the model */
    u8 unk190[0x1C0 - 0x190];
    Vector4 center; /* +0x1c0: bounding sphere centre and radius (the shadow position) */
    void (*preDraw)(int camera, int drawPass); /* +0x1d0 */
    u8 unk1D4[0x1E0 - 0x1D4];
    void (*postDraw)(int camera, int drawPass); /* +0x1e0 */
} PxyDrawPacket;

typedef struct PxyDrawSlot {
    struct PxyDrawPacket *packet;
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
/* One 8-byte record of the draw entry table: the part and the packet it belongs to. */
typedef struct PxyDrawEntry {
    PxyPart *part;
    struct PxyDrawPacket *packet;
} PxyDrawEntry;

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

/* The 16-byte tail of a batch table. As in X2's PxyBatchTableTail the record is doubleword aligned: with a
 * word-aligned record GCC folds the field offset into the loop pointer, the original keeps it as the store offset. */
typedef union PxyBatchTableTail {
    struct {
        int unk2000;
        int unk2004;
        int count; /* +0x2008 */
        int unk200C;
    };
    u64 doublewords[2];
} PxyBatchTableTail;

typedef struct PxyBatchTable {
    PxyBatch batches[256];
    PxyBatchTableTail tail; /* +0x2000 */
} PxyBatchTable;

typedef struct PxyModelRenderState {
    XglLightSet light;
    float lightSlotTable[24][4]; /* +0x180 (X2 pxyLightSlotTable) */
    u32 renderStatus; /* +0x300: nmlRenderStatus, the flag word of the model code */
    int activeCameraIndex; /* +0x304: studio of the camera the model is entered for */
    int shadowMode; /* +0x308: pxyShadowMode */
    PxyHideMask *hideMask; /* +0x30c: one bit per part matrix */
    u8 *secondPartMask; /* +0x310: per part matrix, zero hides the part */
    PxyModelHeader *modelHeader; /* +0x314: pxyCurrentHeader */
    PxyDrawPartRange *partRanges; /* +0x318: [0] opaque, [1] translucent */
    void *headerTable58; /* +0x31c */
    void *headerTable6C; /* +0x320 */
    PxyPart *parts; /* +0x324 */
    Matrix *defaultPartMatrices; /* +0x328 */
    Matrix *defaultJointMatrices; /* +0x32c */
    Matrix *jointMatrices; /* +0x330 */
    void *headerTable64; /* +0x334 */
    u8 *partVisibilityFlags; /* +0x338: one byte per part, bit 0x80 visible */
    u8 *secondPartFlags; /* +0x33c: per part, the secondPartMask byte of its matrix */
    TxyBlock *txyBlock; /* +0x340 */
    void *headerTable3C; /* +0x344 */
    PxyModelHeader *parentHeader; /* +0x348 */
    float reflTransparency; /* +0x34c */
    float transparency; /* +0x350 */
    float circleShadowScale; /* +0x354 */
    void *headerTable74; /* +0x358 */
    void *headerTable78; /* +0x35c */
    float shadowAlpha; /* +0x360 */
    Matrix *partMatrices; /* +0x364 */
    float shadowOffsetX; /* +0x368 */
    float shadowOffsetY; /* +0x36c */
    void *geometryFogFactors; /* +0x370 */
    XglLightSet *currentLight; /* +0x374 */
    u8 unk378[8];
    long long stencilAlpha; /* +0x380 */
    long long stencilMode; /* +0x388 */
    PxyShadow *currentShadow; /* +0x390 */
    int renderLevel; /* +0x394: pxyRenderLevel */
    float modelScaleMax; /* +0x398 */
    float animTime; /* +0x39c */
    int clipEnabled; /* +0x3a0 */
    float shadowPlaneHeight; /* +0x3a4 */
    float (*shadowMatrix)[4]; /* +0x3a8: pxyShadowMatrix, the rows of a 4x4 matrix */
    int visibleOverride; /* +0x3ac */
    Matrix *motionGlobalMatrices; /* +0x3b0 */
    s16 clipPlaneCount; /* +0x3b4 */
    u16 clipPlaneMaskedCount; /* +0x3b6 */
    PxyPlaneTable *clipPlanes; /* +0x3b8 */
    Matrix *globalMatrices; /* +0x3bc: pxyGlobalMatrices, one 0x40-byte matrix per joint */
    float clipPlaneMaskedX; /* +0x3c0 */
    u32 *clipPlaneMask; /* +0x3c4 */
    float clipPlaneMaskedY; /* +0x3c8 */
    float clipPlaneMaskedZ; /* +0x3cc */
    float shadowHeightLower; /* +0x3d0 */
    int modelSetIndex; /* +0x3d4 */
    float shadowHeightUpper; /* +0x3d8 */
    float fadeMetric; /* +0x3dc: pxyCalcFadeMetric of the model */
    u8 *colorIndexSource; /* +0x3e0 */
    int stateFlags3E4; /* +0x3e4: bit 0x8 is set by pxyStateSetFlag3E4Bit3 */
    float fadeAlpha; /* +0x3e8 */
    u8 unk3EC[4];
    Matrix drawMatrix; /* +0x3f0: nmlModelDrawMatrix */
    Matrix drawInverseMatrix; /* +0x430 */
    Vector4 modelScale; /* +0x470 */
    u32 cameraVisible[4]; /* +0x480: per camera, nonzero when the model is visible */
    int cameraFog[4]; /* +0x490: per camera, the fog factor << 4 (0xA00 under white fog) */
    PxyVector fogColor; /* +0x4a0 */
    AlignedVector4 fogParams; /* +0x4b0 */
    Vector4 modelPosition; /* +0x4c0 */
    int latchedActorState; /* +0x4d0 */
    u8 unk4D4[0x4E0 - 0x4D4];
    int latchedSecondState; /* +0x4e0 */
    u8 unk4E4[0x4F0 - 0x4E4];
    float trackValues[7][4]; /* +0x4f0 */
    PxyBestRecord bestRecords[13]; /* +0x560: written by pxyEvaluateTracks, read by pxyCalcFogFactors */
    u8 unk630[0x40];
    int overlayBatchCount; /* +0x670: batches of pxyState.overlayBatches */
    int shadowBatchCount; /* +0x674 (X2 pxyShadowBatchCount) */
    int separateBatchCount; /* +0x678 (X2 pxySeparateBatchCount) */
    int extraBatchCount; /* +0x67c (X2 pxyExtraBatchCount) */
    int slotWriteIndex; /* +0x680: next slot of the ring */
    PxyBatch *firstBatch; /* +0x684: head of the sorted batch list, linked by nextBatch */
    float sortOffset; /* +0x688: nmlModelSortOffset */
    int batchedModelCount; /* +0x68c */
    int unk690;
    int pendingFlag; /* +0x694 */
    u32 drawFlags; /* +0x698: the pxy flag word, bit 0x1000 is set by pxyForceCameraVisibleSet */
    int drawCallbackCount; /* +0x69c: counted up by func_0019ED68 */
    u8 unk6A0[4];
    int drawCallback; /* +0x6a4 */
    int freezeAnimation; /* +0x6a8: pxyForcedVisibilityRequested */
    int shadowCasterMode; /* +0x6ac: 1 draws the opaque pass only */
    int camera; /* +0x6b0: camera being drawn */
    int drawPass; /* +0x6b4: 0 opaque, 1 translucent; selects batchTables */
    u8 unk6B8[4];
    int entryGroupMask; /* +0x6bc */
    int entryCount; /* +0x6c0 */
    u8 unk6C4[0x6D8 - 0x6C4];
    int opaqueEntryCursor; /* +0x6d8 */
    int translucentEntryCursor; /* +0x6dc */
    float opaqueEntryUsagePercent; /* +0x6e0 */
    float translucentEntryUsagePercent; /* +0x6e4 */
    float shadowScale; /* +0x6e8 */
    int modelHiddenFlag; /* +0x6ec: set when the model is hidden */
    PxyBatch *lastBatch; /* +0x6f0 (X2 pxyLastBatch) */
    int unk6F4;
    u8 unk6F8[8];
    void (*drawCallbacks[16])(int); /* +0x700: registered by func_0019ED68 */
    Vector4 boundSphere; /* +0x740: pxyBoundSphere, centre and radius */
    PxyVector globalFogColor; /* +0x750 */
    AlignedVector4 globalFogParams; /* +0x760 */
    PxyBatch *overlayBatches[32]; /* +0x770: drawn before and after the sorted batches */
    PxyBatch *shadowBatches[32]; /* +0x7f0: batches of the shadow casters */
    u8 unk870[0xA70 - 0x870];
    PxyBatchTable batchTables[2]; /* +0xa70: one per draw pass */
    PxyDrawSlot slots[PXY_SLOT_COUNT]; /* +0x4a90 */
    u8 unk4AD0[0x10880 - 0x4AD0];
    int flagSearchHeaders[256]; /* +0x10880: headers of the flags-0x10 search, count at +0x5c0; bound from the object extent up to pxyDrawEntryTable at 0x97A980 */
    u8 unk10C80[0x10D50 - 0x10C80];
    PxyDrawEntry drawEntryTable[6656]; /* +0x10d50: opaque entries below 4096, translucent from 4096 */
    u8 resetWork[0x30]; /* +0x1dd50 */
} PxyModelRenderState;

void pxyResetGlobals(void);

extern PxyModelRenderState gPxyModelRenderStateStorage;
extern int pxyCameraEnabled[4];

/* Empties both batch tables and the batch list words (X2 pxyResetBatchTables). */
void pxyResetBatchTables(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        gPxyModelRenderStateStorage.batchTables[i].tail.count = 0;
        gPxyModelRenderStateStorage.batchTables[i].tail.unk2000 = 0;
        gPxyModelRenderStateStorage.batchTables[i].tail.unk2004 = 0;
        memset(gPxyModelRenderStateStorage.batchTables[i].batches, 0, sizeof(gPxyModelRenderStateStorage.batchTables[i].batches));
    }
    gPxyModelRenderStateStorage.separateBatchCount = 0;
    gPxyModelRenderStateStorage.overlayBatchCount = 0;
    gPxyModelRenderStateStorage.shadowBatchCount = 0;
    gPxyModelRenderStateStorage.firstBatch = 0;
    gPxyModelRenderStateStorage.lastBatch = 0;
    gPxyModelRenderStateStorage.extraBatchCount = 0;
}

void pxyFinishInit(void) {
    pxyResetBatchTables();
}

void pxyClearPacketSlots(void) {
    s32 packetSlotNumber;
    s32 *slotPacket;

    gPxyModelRenderStateStorage.slotWriteIndex = 0;
    slotPacket = &((s32 *) &gPxyModelRenderStateStorage)[0x12B0];
    for (packetSlotNumber = 0; packetSlotNumber < 4; packetSlotNumber++) {
        *slotPacket = 0;
        slotPacket -= 4;
    }
}

void pxyClearPacketSlots();

void pxyInitPacketSlots(void) {
    pxyClearPacketSlots();
}

#define pxyState gPxyModelRenderStateStorage

void pxyResetGlobals(void)
{
    xglMatrixUnit(pxyState.drawMatrix.rows);
    xglMatrixUnit(pxyState.drawInverseMatrix.rows);
    pxyState.renderStatus = 0;
    pxyState.stateFlags3E4 = 0;
    pxyState.shadowMode = 0;
    pxyState.activeCameraIndex = 0;
    pxyState.reflTransparency = 1.0f;
    pxyState.transparency = 1.0f;
    pxyState.circleShadowScale = 1.0f;
    pxyState.modelScaleMax = 1.0f;
    pxyState.modelScale.x = 1.0f;
    pxyState.modelScale.y = 1.0f;
    pxyState.modelScale.z = 1.0f;
    pxyState.modelScale.w = 1.0f;
    pxyState.cameraFog[0] = 0xFF0;
    pxyState.cameraFog[1] = 0xFF0;
    pxyState.cameraFog[2] = 0xFF0;
    pxyState.cameraFog[3] = 0xFF0;
    pxyState.shadowOffsetX = 0;
    pxyState.shadowOffsetY = 0;
    pxyState.renderLevel = 0;
    pxyState.visibleOverride = 0;
    pxyState.clipPlaneCount = 0;
    pxyState.shadowHeightUpper = 0.5f;
    pxyState.shadowHeightLower = -1.0f;
    QuadwordZero(&pxyState.modelPosition);
    pxyState.fadeAlpha = 1.0f;
    pxyState.hideMask = 0;
    pxyState.modelHeader = 0;
    pxyState.partRanges = 0;
    pxyState.headerTable58 = 0;
    pxyState.headerTable6C = 0;
    pxyState.parts = 0;
    pxyState.defaultPartMatrices = 0;
    pxyState.defaultJointMatrices = 0;
    pxyState.jointMatrices = 0;
    pxyState.headerTable64 = 0;
    pxyState.partVisibilityFlags = 0;
    pxyState.txyBlock = 0;
    pxyState.headerTable3C = 0;
    pxyState.parentHeader = 0;
    pxyState.headerTable74 = 0;
    pxyState.headerTable78 = 0;
    pxyState.partMatrices = 0;
    pxyState.geometryFogFactors = 0;
    pxyState.currentLight = &pxyState.light;
    pxyState.currentShadow = 0;
    pxyState.clipEnabled = 0;
    pxyState.shadowMatrix = 0;
    pxyState.motionGlobalMatrices = 0;
    pxyState.clipPlanes = 0;
    pxyState.globalMatrices = 0;
    pxyState.modelSetIndex = 0;
    pxyState.clipPlaneMask = 0;
    pxyState.colorIndexSource = 0;
    pxyState.latchedActorState = 0;
    pxyState.latchedSecondState = 0;
    pxyState.secondPartMask = 0;
    pxyState.sortOffset = 0;
    pxyState.freezeAnimation = 0;
    pxyState.shadowScale = 1.0f;
    memset(pxyState.resetWork, 0, sizeof(pxyState.resetWork));
}

void pxyInitGlobals(void) {
    PxyModelRenderState *renderBlock;

    pxyResetGlobals();
    renderBlock = &gPxyModelRenderStateStorage;
    renderBlock->shadowCasterMode = 1;
    renderBlock->translucentEntryCursor = 0x1000;
    renderBlock->batchedModelCount = 0;
    renderBlock->unk690 = 0;
    renderBlock->opaqueEntryCursor = 0;
    renderBlock->unk6F4 = 0;
}

void pxyInitGlobals();

void pxyInitModelCounters(void) {
    pxyInitGlobals();
}

/* The global colour scale pxySetColorScale sets (X2 pxyGlobalColor). */
extern Vector4 pxyGlobalColor;

/* X2 pxyClearEntryTable. */
void pxyClearEntryTable(void)
{
    int i;

    pxyGlobalColor.x = 1.0f;
    pxyGlobalColor.y = 1.0f;
    pxyGlobalColor.z = 1.0f;
    pxyGlobalColor.w = 1.0f;
    gPxyModelRenderStateStorage.entryCount = 0;
    gPxyModelRenderStateStorage.entryGroupMask = 0;
    gPxyModelRenderStateStorage.pendingFlag = 0;
    for (i = 0; i < 16; i++) {
        gPxyModelRenderStateStorage.drawCallbacks[i] = 0;
    }
    gPxyModelRenderStateStorage.drawCallbackCount = 0;
    gPxyModelRenderStateStorage.drawFlags = 0;
    gPxyModelRenderStateStorage.drawCallback = 0;
}

void func_001975C8(void *block);

/* X2 pxyInitTables: camera 0 on, the default light at the start of the block, the light slot table cleared with
 * unit x in its first seven rows, the usage percentages cleared, then the entry table. */
void pxyInitTables(void)
{
    pxyCameraEnabled[0] = 1;
    pxyCameraEnabled[1] = 0;
    pxyCameraEnabled[2] = 0;
    pxyCameraEnabled[3] = 0;
    func_001975C8(&gPxyModelRenderStateStorage);
    memset(gPxyModelRenderStateStorage.lightSlotTable, 0, sizeof(gPxyModelRenderStateStorage.lightSlotTable));
    gPxyModelRenderStateStorage.lightSlotTable[0][0] = 1.0f;
    gPxyModelRenderStateStorage.lightSlotTable[1][0] = 1.0f;
    gPxyModelRenderStateStorage.lightSlotTable[2][0] = 1.0f;
    gPxyModelRenderStateStorage.lightSlotTable[3][0] = 1.0f;
    gPxyModelRenderStateStorage.lightSlotTable[4][0] = 1.0f;
    gPxyModelRenderStateStorage.lightSlotTable[5][0] = 1.0f;
    gPxyModelRenderStateStorage.lightSlotTable[6][0] = 1.0f;
    gPxyModelRenderStateStorage.opaqueEntryUsagePercent = 0;
    gPxyModelRenderStateStorage.translucentEntryUsagePercent = 0;
    pxyClearEntryTable();
}

extern int gPxyFadeRampIndexTable[];
extern int gPxyFadeRampTable[];

void pxyBuildFadeRamp(float spread)
{
    int *ramp;
    int i;
    float fadeLevel;
    float fadeStep;

    ramp = gPxyFadeRampTable;
    fadeLevel = 0.0f;
    fadeStep = spread;
    ramp[gPxyFadeRampIndexTable[0]] = 0x80000000;
    for (i = 1; i < 32; i++) {
        if ((int)fadeLevel < 129) {
            ramp[gPxyFadeRampIndexTable[i]] = (128 - (int)fadeLevel) << 24;
        } else {
            ramp[gPxyFadeRampIndexTable[i]] = 0;
        }
        fadeLevel = fadeLevel + fadeStep;
        fadeStep = fadeStep + (spread * 0.2f + 0.0f);
    }
}

/* The upload header of a circle shadow packet: four VIF codes (the last one DIRECT) and the GIF tag of the
 * image transfer. */
typedef struct PxyUploadHeader {
    unsigned int vifCode[4];
    unsigned int gifTag[4];
} PxyUploadHeader;

/* The circle shadow texture: its upload header and 64x64 PSMT8 levels, a filled disc that fades out to the edge. */
typedef struct PxyCircleShadowTexture {
    PxyUploadHeader header;
    unsigned char level[64][64];
} PxyCircleShadowTexture;

extern PxyCircleShadowTexture pxyCircleShadowTexture;
/* The palette upload header of the circle shadow; the colours are pxyBuildFadeRamp's table right after it. */
extern PxyUploadHeader pxyCircleShadowClut;
/* Set once CONSTRUCT_CIRCLR_SHADOW has built the circle shadow texture and its palette. */
extern int pxyShadowTexturesReady;

/* Radius (in texels) of the circle shadow and its fade width. */
#define PXY_CIRCLE_SHADOW_RADIUS 32.0f
#define PXY_CIRCLE_SHADOW_FADE 31.0f
/* The darkest level of the shadow texture. */

/* The fade ramp spread of the shadow palette. */
#define PXY_CIRCLE_SHADOW_RAMP_SPREAD 0.3f

/* Builds the 64x64 circle shadow levels once (full inside radius 1, fading to 0 at
 * radius 32), the upload headers of the texture and palette, and the palette's fade ramp. */
void CONSTRUCT_CIRCLR_SHADOW(void)
{
    int row;
    float maxLevel;
    int column;
    PxyUploadHeader *packet;

    maxLevel = PXY_CIRCLE_SHADOW_FADE;
    if (pxyShadowTexturesReady != 0) {
        return;
    }
    for (row = 0; row < 64; row++) {
        for (column = 0; column < 64; column++) {
            float x;
            float y;
            float radius;

            x = (float) column - PXY_CIRCLE_SHADOW_RADIUS;
            y = (float) row - PXY_CIRCLE_SHADOW_RADIUS;
            radius = __builtin_sqrtf(x * x + y * y);
            if (radius <= 1.0f) {
                radius = 0.0f;
            } else if (PXY_CIRCLE_SHADOW_RADIUS < radius) {
                radius = 1.0f;
            } else {
                radius = (radius - 1.0f) / PXY_CIRCLE_SHADOW_FADE;
            }
            radius = (1.0f - radius) * maxLevel;
            pxyCircleShadowTexture.level[row][column] = (unsigned int) radius;
        }
    }
    packet = &pxyCircleShadowClut;
    packet->vifCode[0] = 0;
    packet->vifCode[1] = 0;
    packet->vifCode[2] = 0;
    packet->vifCode[3] = 0x50000041;
    packet->gifTag[0] = 0x8040;
    packet->gifTag[1] = 0x08000000;
    packet->gifTag[2] = 0;
    packet->gifTag[3] = 0;
    packet = &pxyCircleShadowTexture.header;
    packet->vifCode[0] = 0;
    packet->vifCode[1] = 0;
    packet->vifCode[2] = 0;
    packet->vifCode[3] = 0x50000101;
    packet->gifTag[0] = 0x8100;
    packet->gifTag[1] = 0x08000000;
    packet->gifTag[2] = 0;
    packet->gifTag[3] = 0;
    pxyBuildFadeRamp(PXY_CIRCLE_SHADOW_RAMP_SPREAD);
    pxyShadowTexturesReady = 1;
}

extern s32 gPxyLightDirtyFlag;

void pxyStateSetLightDirty(void) {
    gPxyLightDirtyFlag = 1;
}

extern s32 gPxyDirtyFlag;

void pxyStateSetDirty(void) {
    gPxyDirtyFlag = 1;
}

/* Episode I calls this nmlModelSetTexProreal. */
#define PXY_TEX_PROREAL 0x1

int func_0019E7F0(void)
{
    int stateFlags = gPxyModelRenderStateStorage.stateFlags3E4 | PXY_TEX_PROREAL;

    gPxyModelRenderStateStorage.stateFlags3E4 = stateFlags;
    return stateFlags;
}

void pxyStateSetWord0(u8 *word0)
{
    gPxyModelRenderStateStorage.colorIndexSource = word0;
}

void pxyRenderStatusSetBit10(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x400;
}

void pxyModelRenderStateLoadLight(const void *lightSet)
{
    __builtin_memcpy(&gPxyModelRenderStateStorage, lightSet, 0x180);
}

PxyModelRenderState *pxyModelRenderStateGet(void) {
    return &gPxyModelRenderStateStorage;
}

void pxyModelRenderStateInitLight(void) {
    func_001975C8(&gPxyModelRenderStateStorage);
}

void cfeDirectionFromAngles(float *direction, const float *angles);
void xglVectorMulMat(Vector4 *destination, Matrix4 matrix, const Vector4 *vector);
void xglMatrixStackUnit(void);
void xglMatrixStackRotX(float angle);
void xglMatrixStackRotY(float angle);
void xglMatrixStackSave(Matrix4 matrix);
extern const AlignedVector4 cfeForwardVector;
const AlignedVector4 cfeForwardVector = { 0.0f, 0.0f, 1.0f, 1.0f };

void pxySetSyncMode(s32 syncMode) {
    gPxyModelRenderStateStorage.shadowCasterMode = syncMode;
}

/* The vector cfeForwardVector rotated by angles[0] about X and angles[1] about Y. */
void cfeDirectionFromAngles(float *direction, const float *angles)
{
    Matrix4 matrix;
    AlignedVector4 vector = cfeForwardVector;

    xglMatrixStackUnit();
    xglMatrixStackRotX(angles[0]);
    xglMatrixStackRotY(angles[1]);
    xglMatrixStackSave(matrix);
    xglVectorMulMat(&vector, matrix, &vector);
    QuadwordCopy(direction, &vector);
}


int strcmp(unsigned char *geometryEntry, const char *name);

/* Index of the first geometry geometryEntry (112 bytes each) of a "pxy" pack whose name matches partName; -1 when none does. */
/* A "pxy" pack as cfeModelFindPart reads it: the first doubleword holds the "pxy" magic in its low three bytes. */
typedef union PxyPackView {
    u64 signature;
    PxyModelHeader header;
} PxyPackView;

int cfeModelFindPart(PxyPackView *pack, const char *partName) {
    unsigned char *geometryEntry;
    int i;

    if (partName != 0 && pack != 0 && (pack->signature & 0xFFFFFF) == 0x797870) {
        geometryEntry = (unsigned char *)pack + pack->header.geometryOffset;
        for (i = 0; i < pack->header.geometryCount; i++) {
            if (strcmp(geometryEntry, partName) == 0) {
                return i;
            }
            geometryEntry += 112;
        }
    }
    return -1;
}

/* The address of geometry entry `index` (112 bytes each) of a "pxy" pack, 0 when the pack is not one or the
 * index is out of range (X2 pxyGetEntry). */
unsigned char *pxyGetEntry(PxyPackView *pack, int index) {
    unsigned char *geometryEntry;

    if (pack != 0 && (pack->signature & 0xFFFFFF) == 0x797870) {
        geometryEntry = (unsigned char *)pack + pack->header.geometryOffset;
        if (pack->header.geometryCount > index) {
            return geometryEntry + index * 112;
        }
    }
    return 0;
}

void pxySetShadowOffset(f32 offsetX, f32 offsetY)
{
    gPxyModelRenderStateStorage.shadowOffsetX = offsetX;
    gPxyModelRenderStateStorage.shadowOffsetY = offsetY;
}

void pxyRequestTextureCopyPass(void)
{
    gPxyModelRenderStateStorage.drawFlags |= 0x2;
}

void pxyEnableFlag8(void)
{
    gPxyModelRenderStateStorage.drawFlags |= 0x8;
}

void pxyEnableFlag800000(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x800000;
}

void pxySetOverlayAlphaMaskMode(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x2000;
}

void pxySkipOverlayDraw(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x200;
}

void pxyRequestFrameClearOnly(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x4000;
}

void pxySetShadowUploadFlag(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x20000;
}

void pxyFreezeAnimation(void)
{
    gPxyModelRenderStateStorage.freezeAnimation = 1;
    gPxyModelRenderStateStorage.renderStatus |= 0x2;
}

void pxySetHeaderScanEnabled(s32 enabled) {
    if (enabled != 0) {
        gPxyModelRenderStateStorage.drawFlags |= 0x10;
    }
}

void pxyEnableFlag80000(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x80000;
}

void pxySetShadowProjectOff(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x40000;
}

void pxySkipTextureUpload(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x40;
}

void pxySetBlackFog(void) {
    gPxyModelRenderStateStorage.drawFlags |= 0x80;
}

void pxySetStateFlag4(void)
{
    gPxyModelRenderStateStorage.stateFlags3E4 |= 0x4;
}

void pxyEnableRenderFlag800(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x800;
}

void pxyEnableRenderFlag20000(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x20000;
    gPxyModelRenderStateStorage.drawFlags |= 0x8000;
}

void pxySetStoredValue(s32 storedValue) {
    gPxyModelRenderStateStorage.clipEnabled = storedValue;
}

void pxySetAnimTime(float time)
{
    gPxyModelRenderStateStorage.animTime = time;
    gPxyModelRenderStateStorage.renderStatus |= 0x100;
}

void pxyEnableRenderFlag40000(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x40000;
    gPxyModelRenderStateStorage.drawFlags |= 0x100000;
}

void pxyEnableRenderFlag10000(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x10000;
}

void pxySetPendingFlag(void) {
    gPxyModelRenderStateStorage.pendingFlag = 1;
}

void pxyLatchValueA(s32 valueToLatch)
{
    if (gPxyModelRenderStateStorage.latchedActorState == 0)
    {
        gPxyModelRenderStateStorage.latchedActorState = valueToLatch;
    }
}

void pxyLatchValueB(s32 valueToLatch)
{
    if (gPxyModelRenderStateStorage.latchedSecondState == 0)
    {
        gPxyModelRenderStateStorage.latchedSecondState = valueToLatch;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019ED68);

void pxyEnableFlag400000(s32 enabled) {
    if (enabled != 0) {
        gPxyModelRenderStateStorage.drawFlags |= 0x400000;
    }
}

void pxySetDrawCallback(s32 drawCallback) {
    gPxyModelRenderStateStorage.drawCallback = drawCallback;
}

void pxySetCameraEnabled(s32 camera, s32 enabled) {
    pxyCameraEnabled[camera] = enabled;
}

void pxyAddRenderLevel(int levelBits)
{
    gPxyModelRenderStateStorage.renderLevel |= levelBits;
}

void pxySetShadowMode(int shadowMode, float alpha, float planeHeight, float scale)
{
    if (alpha <= 0.0f) {
        return;
    }
    gPxyModelRenderStateStorage.shadowScale = scale;
    gPxyModelRenderStateStorage.shadowAlpha = alpha;
    gPxyModelRenderStateStorage.shadowPlaneHeight = planeHeight;
    gPxyModelRenderStateStorage.shadowMode = shadowMode;
    if (shadowMode == 4 || shadowMode == 1 || shadowMode == 5) {
        gPxyModelRenderStateStorage.renderStatus |= 0x2;
    }
}

void pxySetPlaneParams(f32 paramX, f32 paramY)
{
    gPxyModelRenderStateStorage.shadowHeightUpper = paramX;
    gPxyModelRenderStateStorage.shadowHeightLower = paramY;
}

void pxySetJointMatrices(Matrix *joints, Matrix *globalMatrices)
{
    gPxyModelRenderStateStorage.jointMatrices = joints;
    gPxyModelRenderStateStorage.globalMatrices = globalMatrices;
}

void pxySetMotionGlobalMatrices(Matrix *motionMatrices)
{
  gPxyModelRenderStateStorage.motionGlobalMatrices = motionMatrices;
}

void nmlModelSetStoreValue(Matrix *storedValue) {
    gPxyModelRenderStateStorage.partMatrices = storedValue;
}

void nmlModelSetFogCol(Vector4 *color)
{
    gPxyModelRenderStateStorage.fogColor.vector = *color;
    gPxyModelRenderStateStorage.renderStatus |= 1;
}

/* A vector kept as a plain float[4] (like sceVu0FVECTOR); its w is element 3. Vector4 here changes the
 * address arithmetic of the w stores, so the casts to Vector4 * at the calls stay. */
XglCamera *xglStudioSelectGetActiveCamera(int studio_index);

void nmlModelFogPara(Vector4 *out, f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    f32 shadeMin;
    f32 shadeMax;

    shadeMin = (1.0f - fogMin) * 255.0f;
    shadeMax = (1.0f - fogMax) * 255.0f;
    if (shadeMax < 0.0f) {
        shadeMax = 0.0f;
    }
    if (255.0f < shadeMax) {
        shadeMax = 255.0f;
    }
    if (shadeMin < 0.0f) {
        shadeMin = 0.0f;
    }
    if (255.0f < shadeMin) {
        shadeMin = 255.0f;
    }
    out->x = shadeMax;
    out->y = shadeMin;
    out->z = (((shadeMax - shadeMin) * (fogFar + fogNear)) / (fogFar - fogNear) + (shadeMax + shadeMin)) * 0.5f;
    out->w = (fogFar * fogNear) * (shadeMin - shadeMax) / (fogFar - fogNear);
}

void nmlModelSetFogDist(f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    nmlModelFogPara((Vector4 *)&gPxyModelRenderStateStorage.fogParams, fogNear, fogFar, fogMin, fogMax);
    gPxyModelRenderStateStorage.renderStatus |= 1;
}

void _CurMatrixSet(Matrix4 *matrix);
void _CurRotTransPersFog(Vector4 *projected, const Vector4 *position, const Vector4 *fog);

int nmlModelGetFogParaStudio(const Vector4 *position, int studio)
{
    int fogShade;
    Vector4 projected;
    XglCamera *camera;
    Vector4 *fogPara;

    fogShade = 255;
    fogPara = (Vector4 *)&gPxyModelRenderStateStorage.fogParams;
    camera = xglStudioSelectGetActiveCamera(studio);
    if (camera != 0) {
        _CurMatrixSet(&camera->volume[0].viewMatrix);
        _CurRotTransPersFog(&projected, position, fogPara);
        fogShade = (u32)projected.w;
        if (projected.z < 0.0f) {
            fogShade = (u32)fogPara->y;
        }
    }
    return fogShade;
}

void nmlModelSetFogFarDist(f32 fogNear, f32 fogFar, f32 fogMin, f32 fogMax) {
    nmlModelFogPara((Vector4 *)&gPxyModelRenderStateStorage.globalFogParams, fogNear, fogFar, fogMin, fogMax);
    gPxyModelRenderStateStorage.drawFlags |= 0x20;
}

void nmlModelSetGlobalFogCol(Vector4 *color)
{
    gPxyModelRenderStateStorage.globalFogColor.vector = *color;
    gPxyModelRenderStateStorage.drawFlags |= 0x20;
}

void nmlModelSetSortOffset(f32 sortOffset) {
    gPxyModelRenderStateStorage.sortOffset = sortOffset;
}

void pxySetColorScale(Vector4 *color)
{
    pxyGlobalColor = *color;
}

void nmlModelSetCurrentPtr(void *modelSetCurrent)
{
    gPxyModelRenderStateStorage.hideMask = modelSetCurrent;
}

void nmlModelSetIndexValue(int modelSetIndex) {
    gPxyModelRenderStateStorage.modelSetIndex = modelSetIndex;
}

void nmlModelSetTexture(TxyBlock *block)
{
    u8 *bytes;
    u32 byteIndex;

    if ((u32)block & 0xF) {
        return;
    }
    if (block != 0 && block->magic[0] == 't' && block->magic[1] == 'x' && block->magic[2] == 'y') {
        if (block->flags & 0x80000000) {
            bytes = (u8 *)block;
            byteIndex = 16;
            while (byteIndex < block->size) {
                if (bytes[byteIndex] == 1) {
                    bytes[byteIndex] = -1;
                } else if (bytes[byteIndex] != 0) {
                    bytes[byteIndex] = bytes[byteIndex] + 0xFF;
                }
                byteIndex++;
            }
            block->flags &= 0x7FFFFFFF;
        }
        gPxyModelRenderStateStorage.txyBlock = block;
    }
}

void pxySetModelPosition(Vector4 *modelPosition)
{
    gPxyModelRenderStateStorage.modelPosition = *modelPosition;
}

void pxySetLightPtr(PxyModelHeader *light)
{
    gPxyModelRenderStateStorage.parentHeader = light;
}

void nmlModelSetDrawMatrix(Matrix4 matrix)
{
    QuadwordCopyMatrix(gPxyModelRenderStateStorage.drawMatrix.rows, matrix);
}

/* X2 nmlModelSetScale: keeps the scale and its largest component. */
void nmlModelSetScale(const Vector4 *scale)
{
    gPxyModelRenderStateStorage.modelScale = *scale;
    gPxyModelRenderStateStorage.modelScaleMax = scale->x;
    if (gPxyModelRenderStateStorage.modelScaleMax < scale->y) {
        gPxyModelRenderStateStorage.modelScaleMax = scale->y;
    }
    if (gPxyModelRenderStateStorage.modelScaleMax < scale->z) {
        gPxyModelRenderStateStorage.modelScaleMax = scale->z;
    }
}

void nmlModelSetZwrite(int zwrite) {
    if (zwrite != 0) {
        gPxyModelRenderStateStorage.renderStatus |= 8;
    }
}

void nmlModelSetTransparency(f32 transparency)
{
    f32 clamped;

    clamped = transparency;
    if (clamped < 0.0f)
    {
        clamped = 0.0f;
    }
    if (clamped > 1.0f)
    {
        clamped = 1.0f;
    }
    gPxyModelRenderStateStorage.transparency = clamped;
    gPxyModelRenderStateStorage.circleShadowScale = clamped;
}

void nmlModelSetStoredMode(u8 *storedMode) {
    gPxyModelRenderStateStorage.secondPartMask = storedMode;
}

void nmlModelSetGlobalAlpha(f32 alpha) {
    f32 clamped;

    clamped = alpha;
    if (clamped < 0.0f) {
        clamped = 0.0f;
    }
    if (clamped > 1.0f) {
        clamped = 1.0f;
    }
    gPxyModelRenderStateStorage.reflTransparency = clamped;
}

void nmlModelSetStencil(int stencil) {
    if (stencil != 0) {
        gPxyModelRenderStateStorage.renderStatus |= 4;
    }
}

void nmlModelSetStoredFlag(XglLightSet *storedFlag) {
    gPxyModelRenderStateStorage.currentLight = storedFlag;
}

void *xglPacketReserve(int count);

/* Copies the light set into the packet and keeps the copy's address (X2 pxySetLightCopy). */
void pxySetLightCopy(XglLightSet *light)
{
    XglLightSet *copy;

    copy = xglPacketReserve(0x19);
    gPxyModelRenderStateStorage.currentLight = copy;
    *copy = *light;
}

void nmlModelSetFlagBit40(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x40;
}

void nmlModelSetStencilAlpha(long long alpha)
{
    unsigned int renderFlags;

    renderFlags = gPxyModelRenderStateStorage.renderStatus | 0x80;
    gPxyModelRenderStateStorage.stencilAlpha = alpha;
    gPxyModelRenderStateStorage.stencilMode = 0x42;
    gPxyModelRenderStateStorage.renderStatus = renderFlags;
}

void nmlModelSetFlagBit80000(void) {
    gPxyModelRenderStateStorage.renderStatus |= 0x80000;
}

void nmlModelSetFlagBit200000(void)
{
    gPxyModelRenderStateStorage.renderStatus |= 0x200000;
}

void nmlModelSetStoredValue(int storedValue) {
    gPxyModelRenderStateStorage.activeCameraIndex = storedValue;
}

void pxyAbortModel(void) {
    pxyResetGlobals();
}

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019F678);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019F778);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_0019FA20);

#include "gs.h"

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

#define PXY_SHADOW_QUADWORDS 0x36

/* A vector the pxy code also moves as four words. */
void xglVectorNormal(Vector4 *destination, const Vector4 *source);
void xglVectorScale(Vector4 *destination, Vector4 *source, float scale);
void xglVectorScaleXYZ(Vector4 *destination, const Vector4 *source, float scale);
f32 xglPointLength(void *from, void *to);
void xglMatrixScale(Matrix *destination, Matrix *source, Vector4 *scale);
void xglMatrixStackTrans(Vector4 *translation);
void xglMatrixStackRotZ(float angle);
void xglMatrixStackScale(Vector4 *scale);
void xglMatrixStackInverse(void);
void xglMatrixStackMul(Matrix4 matrix);
void xglMatrixInverse(Matrix4 inverseMatrix, Matrix4 matrix);
void xglVectorMulMat(Vector4 *transformedVector, Matrix4 matrix, const Vector4 *vector);
struct XglCamera *xglStudioSelectGetActiveCamera(int cameraIndex);
void _ModelCalcClipInitMat1(float (*planeSet)[4], Matrix4 matrix);
int _ModelCalcClipMat1(Vector4 *point);
int xglCullingPollCheck(XglCamera *camera, Vector4 *sphere);
float atan2f(float, float);

/* Sets up the shadow projector of the current model: its bounds from the light direction, the light, view and
 * projection matrices, and the texture and lighting data the shadow pass draws with. */
void pxySetupShadowProjector(PxyModelHeader *modelHeader)
{
    int texWidth = 0x400;
    int texHeight = 0x200;
    PxyShadow *shadow = pxyState.currentShadow;
    volatile float rotX;
    volatile float rotY;
    volatile float horizontal;
    Matrix inverse;
    PxyVector scale = { { pxyState.shadowScale * 128.0f, pxyState.shadowScale * -128.0f, 8000.0f, 1.0f } };
    PxyVector size = { { 128.0f, 64.0f, 1.0f, 1.0f } };
    PxyVector offset = { { 2045.0f, 2045.0f, 0.0f, 1.0f } };
    AlignedVector4 direction;
    float zoom = 1.0f;
    float extent = modelHeader->sphere.w;

    if (pxyState.drawFlags & 0x10) {
        if (pxyState.drawMatrix.rows[3][0] != 0.0f || pxyState.drawMatrix.rows[3][1] != 0.0f || pxyState.drawMatrix.rows[3][2] != 0.0f) {
        }
        if (pxyState.fadeMetric < 139000.0f) {
            pxyState.shadowMode = 0;
            return;
        } else if (pxyState.fadeMetric < 173000.0f) {
            pxyState.shadowAlpha *= 1.0f - (173000.0f - pxyState.fadeMetric) / 34000.0f;
        }
    }
    if (pxyState.currentShadow == 0) {
        pxyState.currentShadow = xglPacketReserve(PXY_SHADOW_QUADWORDS);
    }
    shadow = pxyState.currentShadow;
    QuadwordCopy(&shadow->center, &pxyState.modelPosition);
    extent = modelHeader->sphere.w * 3.0f;
    if (modelHeader->sphere.w >= 1.0f) {
        extent = modelHeader->sphere.w * 5.0f;
    }
    if (extent < 0.968f) {
        extent = 0.968f;
    }
    zoom = 0.968f / extent;
    scale.vector.x *= zoom;
    scale.vector.y *= zoom;
    if (pxyState.currentLight->shadows[0].w == 0.0f) {
        direction.x = pxyState.currentLight->directions[0][0];
        direction.y = 2.0f;
        direction.z = pxyState.currentLight->directions[0][2];
        direction.w = 1.0f;
        if (pxyState.drawFlags & 0x10) {
            direction.y = pxyState.currentLight->directions[0][1];
        }
    } else {
        direction.x = pxyState.currentLight->shadows[0].x;
        direction.y = pxyState.currentLight->shadows[0].y;
        direction.z = pxyState.currentLight->shadows[0].z;
        direction.w = 1.0f;
    }
    xglVectorNormal(&direction, &direction);
    QuadwordCopy(&shadow->min, &shadow->center);
    QuadwordCopy(&shadow->max, &shadow->center);
    if (pxyState.drawFlags & 0x10) {
        float tilt;

        shadow->min.x -= modelHeader->sphere.w;
        shadow->min.y -= modelHeader->sphere.w;
        shadow->min.z -= modelHeader->sphere.w;
        shadow->max.x += modelHeader->sphere.w;
        shadow->max.y += modelHeader->sphere.w;
        shadow->max.z += modelHeader->sphere.w;
        tilt = 1.0f;
        if (direction.y != 0.0f) {
            tilt = __builtin_fabsf((direction.x * direction.x + direction.z * direction.z) / direction.y);
        }
        if (tilt < 1.0f) {
            tilt = 1.0f;
        }
        if (direction.x > 0.0f) {
            shadow->min.x -= direction.x * tilt * modelHeader->sphere.w * 1.3f;
            shadow->max.x += direction.x * tilt * modelHeader->sphere.w * 0.3f;
        } else {
            shadow->max.x -= direction.x * tilt * modelHeader->sphere.w * 1.3f;
            shadow->min.x += direction.x * tilt * modelHeader->sphere.w * 0.3f;
        }
        if (direction.z > 0.0f) {
            shadow->min.z -= direction.z * tilt * modelHeader->sphere.w * 1.3f;
            shadow->max.z += direction.z * tilt * modelHeader->sphere.w * 0.3f;
        } else {
            shadow->max.z -= direction.z * tilt * modelHeader->sphere.w * 1.3f;
            shadow->min.z += direction.z * tilt * modelHeader->sphere.w * 0.3f;
        }
        if (!(direction.y > 0.0f)) {
            shadow->max.y += modelHeader->sphere.w;
            shadow->min.y -= modelHeader->sphere.w * 0.5f;
        } else {
            shadow->min.y -= modelHeader->sphere.w * 1.5f;
            shadow->max.y -= modelHeader->sphere.w * 0.5f;
        }
    } else {
        float tilt;
        float lift;

        shadow->min.x -= modelHeader->sphere.w;
        shadow->min.y -= modelHeader->sphere.w;
        shadow->min.z -= modelHeader->sphere.w;
        shadow->max.x += modelHeader->sphere.w;
        shadow->max.y += modelHeader->sphere.w;
        shadow->max.z += modelHeader->sphere.w;
        tilt = 1.0f;
        lift = 0.0f;
        if (modelHeader->sphere.w >= 5.0f) {
            lift = modelHeader->sphere.w * 1.9f;
        } else if (modelHeader->sphere.w >= 2.0f) {
            lift = modelHeader->sphere.w;
        }
        if (direction.y != 0.0f) {
            tilt = __builtin_fabsf((direction.x * direction.x + direction.z * direction.z) / direction.y);
        }
        if (tilt < 1.0f) {
            tilt = 1.0f;
        }
        if (direction.x != 0.0f || direction.z != 0.0f) {
            if (direction.x > 0.0f) {
                shadow->min.x -= __builtin_fabsf(direction.x * tilt * modelHeader->sphere.w * 1.5f + lift);
                shadow->max.x += __builtin_fabsf(direction.x * tilt * modelHeader->sphere.w * 0.3f);
            } else {
                shadow->max.x += __builtin_fabsf(direction.x * tilt * modelHeader->sphere.w * 1.5f + lift);
                shadow->min.x -= __builtin_fabsf(direction.x * tilt * modelHeader->sphere.w * 0.3f);
            }
            if (direction.z > 0.0f) {
                shadow->min.z -= __builtin_fabsf(direction.z * tilt * modelHeader->sphere.w * 1.5f + lift);
                shadow->max.z += __builtin_fabsf(direction.z * tilt * modelHeader->sphere.w * 0.3f);
            } else {
                shadow->max.z += __builtin_fabsf(direction.z * tilt * modelHeader->sphere.w * 1.5f + lift);
                shadow->min.z -= __builtin_fabsf(direction.z * tilt * modelHeader->sphere.w * 0.3f);
            }
        }
        if (direction.y > 0.0f) {
            shadow->min.y -= modelHeader->sphere.w * 1.5f;
            shadow->max.y -= modelHeader->sphere.w * 0.5f;
        } else {
            shadow->max.y += modelHeader->sphere.w;
            shadow->min.y -= modelHeader->sphere.w * 0.5f;
        }
    }
    shadow->bounds.x = (shadow->max.x + shadow->min.x) * 0.5f;
    shadow->bounds.y = (shadow->max.y + shadow->min.y) * 0.5f;
    shadow->bounds.z = (shadow->max.z + shadow->min.z) * 0.5f;
    shadow->bounds.w = xglPointLength(&shadow->max, &shadow->min) * 0.5f;
    {
        XglCamera *camera = xglStudioSelectGetActiveCamera(0);

        if (camera != 0) {
            Matrix clip;

            xglMatrixUnit(clip.rows);
            _ModelCalcClipInitMat1(camera->volume[0].planeSet[0], clip.rows);
            if (_ModelCalcClipMat1(&shadow->bounds) == 0) {
                pxyState.renderStatus &= ~0x2;
            } else if ((pxyState.drawFlags & 0x10) && xglCullingPollCheck(camera, &shadow->bounds)) {
                pxyState.renderStatus &= ~0x2;
            }
            if ((pxyState.renderStatus & 0x20) && (pxyState.renderStatus & 0x10)) {
                pxyState.renderStatus &= ~0x2;
            }
        }
    }
    horizontal = __builtin_sqrtf(direction.x * direction.x + direction.z * direction.z);
    rotY = -atan2f(direction.x, -direction.z);
    rotX = atan2f(direction.y, horizontal);
    {
        AlignedVector4 eye = direction;

        xglVectorScale(&eye, &eye, 0.0f);
        Vu0AddVectorXYZ(&eye, &eye, &shadow->center);
        xglMatrixStackUnit();
        xglMatrixStackTrans(&eye);
        xglMatrixStackRotY(rotY);
        xglMatrixStackRotX(rotX);
        xglMatrixStackInverse();
        xglMatrixStackSave(shadow->view.rows);
        xglVectorNormal(&direction, &direction);
        xglVectorScale(&direction, &direction, 3.0f);
        Vu0AddVectorXYZ(&direction, &direction, &shadow->center);
        xglMatrixStackUnit();
        xglMatrixStackTrans(&direction);
        xglMatrixStackRotY(rotY);
        xglMatrixStackRotX(rotX);
        xglMatrixStackRotZ(1.5707964f);
        xglMatrixStackSave(shadow->light.rows);
        xglMatrixStackInverse();
        xglMatrixStackSave(inverse.rows);
        xglMatrixStackUnit();
        offset.vector.x -= (D_004C8390.screenWidth - size.vector.x) * 0.5f;
        offset.vector.y -= (D_004C8390.screenHeight - size.vector.y) * 0.5f;
        xglMatrixStackTrans(&offset.vector);
        xglMatrixStackScale(&scale.vector);
        xglMatrixStackMul(inverse.rows);
        xglMatrixStackSave(shadow->project.rows);
        memset(&shadow->lighting, 0, sizeof(shadow->lighting));
        if (pxyState.shadowMode == 4) {
            shadow->alphaData = 0x42;
        } else {
            shadow->alphaData = 0x89;
        }
        shadow->x = 0;
        shadow->y = 0;
        shadow->width = 0x80;
        shadow->height = 0x40;
        shadow->repeatU = 0;
        shadow->repeatV = 0;
        shadow->frameBase = D_004C8390.renderTextureBase;
        shadow->scale.vector.x = 64.0f / (zoom * 128.0f);
        shadow->scale.vector.y = 64.0f / (zoom * 128.0f);
        shadow->unk330 = 0;
        shadow->clutOffset = 0;
        shadow->psm = GS_PSMCT32;
        {
            PxyVector depth = { { 0.0f, 0.0f, 1000.0f, 1.0f } };
            Matrix texture;
            int wrapU;
            int wrapV;
            int minU;
            int maxU;
            int minV;
            int maxV;

            scale.vector.x = (float)shadow->width * 0.015625f * shadow->scale.vector.x;
            scale.vector.y = (float)shadow->height * 0.015625f * shadow->scale.vector.y;
            scale.vector.z = scale.vector.w = 1.0f;
            xglMatrixScale(&texture, &shadow->light, &scale.vector);
            xglVectorMulMat(&shadow->unk2D0, texture.rows, &depth.vector);
            Vu0SubVectorXYZ(&shadow->direction, &shadow->unk2D0, texture.rows[3]);
            xglMatrixInverse(shadow->textureInverse.rows, texture.rows);
            xglVectorNormal(&shadow->direction, &shadow->direction);
            switch (shadow->psm) {
            case GS_PSMT4:
                shadow->tex0 = GS_SET_TEX0(shadow->frameBase << 5, 16, GS_PSMT4, 10, 9, 1, 0,
                                           (shadow->frameBase << 5) + shadow->clutOffset, 0, 0, 0, 1);
                break;
            case GS_PSMT8:
                shadow->tex0 = GS_SET_TEX0(shadow->frameBase << 5, 16, GS_PSMT8, 10, 9, 1, 0,
                                           (shadow->frameBase << 5) + shadow->clutOffset, 0, 0, 0, 1);
                break;
            case GS_PSMCT32:
                shadow->tex0 = GS_SET_TEX0(shadow->frameBase << 5, 8, GS_PSMCT32, 9, 9, 1, 0, 0, 0, 0, 0, 1);
                texWidth = 0x200;
                texHeight = 0x200;
                break;
            }
            wrapU = shadow->repeatU ? GS_REGION_REPEAT : GS_REGION_CLAMP;
            wrapV = shadow->repeatV ? GS_REGION_REPEAT : GS_REGION_CLAMP;
            minU = shadow->repeatU ? shadow->width - 1 : shadow->x;
            maxU = shadow->repeatU ? shadow->x : shadow->x + shadow->width - 1;
            minV = shadow->repeatV ? shadow->height - 1 : shadow->y;
            maxV = shadow->repeatV ? shadow->y : shadow->y + shadow->height - 1;
            shadow->clamp = GS_SET_CLAMP(wrapU, wrapV, minU, maxU, minV, maxV);
            shadow->uv.vector.y = (float)shadow->width / (float)texWidth;
            shadow->uv.vector.x = (float)shadow->x / (float)texWidth + shadow->uv.vector.y * 0.5f;
            shadow->uv.vector.w = -(float)shadow->height / (float)texHeight;
            shadow->uv.vector.z = (float)shadow->y / (float)texHeight - shadow->uv.vector.w * 0.5f;
            xglVectorScaleXYZ(&shadow->direction, &shadow->direction, -1.0f);
            shadow->lighting.direction.x = shadow->direction.x;
            shadow->lighting.direction.y = shadow->direction.y;
            shadow->lighting.direction.z = shadow->direction.z;
            shadow->lighting.color[0] = shadow->lighting.color[1] = shadow->lighting.color[2] = 1.0f;
        }
    }
}

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A0A10);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A0ED0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A1140);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A12A0);

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

/* The per-camera fog factors of one geometry, at pxyState.geometryFogFactors. */
typedef struct PxyGeometryFog {
    int camera[4];
} PxyGeometryFog;

int nmlModelGetFogParaStudio(const Vector4 *position, int studio);

static inline void pxyVectorAddXYZ(Vector4 *destination, Vector4 *first, Vector4 *second)
{
    Vu0AddVectorXYZ(destination, first, second);
}

/* pxyCalcFogFactors: under fog, the fog factor of every enabled camera for the model and for each geometry. */
void pxyCalcFogFactors(PxyModelHeader *modelHeader)
{
    int i;
    int j;

    if (!(pxyState.renderStatus & 1)) {
        return;
    }
    for (i = 0; i < 4; i++) {
        if (pxyCameraEnabled[i] != 0) {
            if (pxyState.renderStatus & 0x400) {
                pxyState.cameraFog[i] = 0xA00;
            } else {
                pxyState.cameraFog[i] =
                    nmlModelGetFogParaStudio(&pxyState.boundSphere, i) << 4;
            }
            if (pxyState.geometryFogFactors != 0) {
                for (j = 0; j < modelHeader->geometryCount; j++) {
                    AlignedVector4 position;

                    pxyVectorAddXYZ(&position, (Vector4 *) pxyState.partMatrices[j].rows[3],
                                    &pxyState.boundSphere);
                    ((PxyGeometryFog *) pxyState.geometryFogFactors)[j].camera[i] =
                        nmlModelGetFogParaStudio(&position, i) << 4;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A17F8);

typedef struct PxyShuffleTable {
    int pairs[8][2];
} PxyShuffleTable;

extern const PxyShuffleTable pxyHeaderShuffleTable;
const PxyShuffleTable pxyHeaderShuffleTable = { {
    { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 0, 2 }, { 1, 3 }, { 2, 0 }, { 3, 1 } } };

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

#define PXY_BATCH_MAX 256
#define PXY_OPAQUE_ENTRY_END 4096      /* end of the opaque draw-entry range */
#define PXY_TRANSLUCENT_ENTRY_END 6656 /* end of the translucent draw-entry range */
/* Bits of the per-part flag bytes (PxyDrawPacket.partFlags). */
#define PXY_PART_VISIBLE 0x80 /* set for every part when the model is entered */
#define PXY_PART_MERGED 0x40  /* translucent part already attached to the previous entry */
#define PXY_PART_LOD_MASK 0xF /* level-of-detail bits tested for header flag 1 */

/* The depth-sort record of a model with header flags 0x1000, at header->sortEntryOffset, one per part matrix. */
typedef struct PxySortEntry {
    u8 unk00[0xC];
    s16 matrix; /* +0x0c */
    s16 key; /* +0x0e */
} PxySortEntry;

typedef struct PxySortItem {
    Vector4Elements position;
    s16 start; /* +0x10 */
    s16 count;
    s16 key;
    s16 matrix;
    float depth; /* +0x18 */
    u8 unk1C[4];
} PxySortItem;

typedef struct PxySortGroup {
    int count;
    s16 start; /* +0x04 */
    s16 entryCount; /* +0x06 */
    u8 unk08[8];
    PxySortItem items[32]; /* +0x10 */
    PxySortItem *sorted[32]; /* +0x410 */
} PxySortGroup;

typedef struct PxySortWork {
    PxySortGroup groups[8];
    int groupCount; /* +0x2480 */
    int entryCount;
    PxyDrawEntry *entries; /* +0x2488 */
    PxySortItem *item; /* +0x248c */
    PxySortGroup *group; /* +0x2490 */
    int i; /* +0x2494 */
    PxySortEntry *sortEntries; /* +0x2498 */
    PxySortEntry *sortEntry; /* +0x249c */
    int j; /* +0x24a0 */
    PxyDrawEntry *out; /* +0x24a4 */
    PxySortItem **cursor; /* +0x24a8 */
    PxySortItem *swapped; /* +0x24ac */
    Matrix *matrixSet; /* +0x24b0 */
    Matrix *matrices; /* +0x24b4 */
    u8 unk24B8[8];
    float matrix[4][4]; /* +0x24c0 */
    Vector4 eye; /* +0x2500 */
    PxyDrawEntry buffer[0x35E]; /* +0x2510 */
} PxySortWork;

/* Address of draw entry INDEX of ENTRIES, computed as an integer with the scaled index first. */
#define PXY_ENTRY_AT(entries, index) ((PxyDrawEntry *) ((index) * sizeof(PxyDrawEntry) + (u32) (entries)))
#define PXY_HEADER_AT(header, offset) ((void *)((u8 *)(header) + (offset)))

void func_001A0A10(PxyModelHeader *header);
void func_001A12A0(PxyModelHeader *header);
void pxyCalcFogFactors(PxyModelHeader *header);
void func_001A17F8(PxyModelHeader *header);
void pxySetupShadowProjector(PxyModelHeader *header);
void func_001A0ED0(PxyDrawSlot *slot, PxyDrawPacket *packet);
void xglMatrixStackLoad(Matrix4 matrix);
void xglMatrixStackMulVector(Vector4 *transformedVector, const Vector4 *vector);
void *xglPacketCopy16(const void *data, unsigned int qwc);
void *memcpy(void *destination, const void *source, unsigned int size);

/* Validates a "pxy" model, sets up its per-model state and queues its parts as draw batches; 1 when the model is
 * rejected. */
int pxySubmitModel(PxyModelHeader *modelHeader)
{
    XglCamera *camera;
    PxyDrawSlot *newSlot;
    PxyDrawSlot *slot;
    PxyRowBits vector;
    Vector4 direction;
    PxySortWork work;
    PxyDrawPacket *parentPacket;
    int i;
    float fadeFullDistance;
    float fadeZeroDistance;
    float fadeFactor;
    PxyDrawPacket *packet;
    PxyDrawPartRange *range;
    PxyDrawPartRange *translucentRange;
    PxyDrawEntry *entry;
    PxyDrawEntry *translucentEntry;
    PxyBatchTable *table;
    PxyBatch *batch;
    PxyBatchTable *translucentTable;
    PxyBatch *translucentBatch;
    PxySortWork *w;

    camera = xglStudioSelectGetActiveCamera(pxyState.activeCameraIndex);
    newSlot = 0;
    slot = 0;
    pxyState.modelHiddenFlag = 0;
    if (camera == 0) {
        pxyResetGlobals();
        return 1;
    }
    if (pxyState.batchedModelCount >= PXY_BATCH_MAX) {
        pxyResetGlobals();
        return 1;
    }
    if (pxyState.batchTables[0].tail.count >= PXY_BATCH_MAX) {
        pxyResetGlobals();
        return 1;
    }
    if ((u32)modelHeader & 0xF) {
        pxyResetGlobals();
        return 1;
    }
    if (modelHeader == 0 || modelHeader->magic[0] != 'p' || modelHeader->magic[1] != 'x' || modelHeader->magic[2] != 'y' || (int)(modelHeader->version * 10.0f) <= 0) {
        pxyResetGlobals();
        return 1;
    }
    pxyUnscrambleHeader(modelHeader);
    func_001A0A10(modelHeader);
    if ((modelHeader->flags & 0x40) && (pxyState.drawFlags & 2)) {
        pxyState.drawFlags |= 1;
    }
    if (((modelHeader->flags & 0x100) && (pxyState.drawFlags & 8)) || (pxyState.stateFlags3E4 & 8)) {
        pxyState.drawFlags |= 4;
    }
    if (modelHeader->flags & 0x800) {
        pxyState.drawFlags |= 0x8000;
    }
    if ((pxyState.drawFlags & 0x20) && !(pxyState.renderLevel & 0x100)) {
        pxyState.fogParams = pxyState.globalFogParams;
        pxyState.fogColor = pxyState.globalFogColor;
        pxyState.renderStatus |= 1;
    }
    if (modelHeader->flags & 0x400) {
        pxyState.renderStatus |= 0x1000;
    } else if (modelHeader->flags & 0x481) {
        pxyState.renderStatus |= 0x1000;
    }
    if (modelHeader->flags & 2) {
        if (pxyState.shadowMode == 1 || pxyState.shadowMode == 5 || pxyState.shadowMode == 4) {
            pxyState.renderLevel |= 0x80;
        }
    }
    if (pxyState.renderLevel & 0x10) {
        pxyState.visibleOverride = -1;
    }
    if (pxyState.drawFlags & 0x80) {
        pxyState.fogColor.vector.x = 0;
        pxyState.fogColor.vector.y = 0;
        pxyState.fogColor.vector.z = 0;
    }
    if (pxyState.drawFlags & 0x400) {
        pxyState.renderStatus |= 0x40000;
    }
    if (pxyState.renderStatus & 0x400) {
        pxyState.renderStatus |= 1;
        pxyState.fogColor.vector.x = 255.0f;
        pxyState.fogColor.vector.y = 255.0f;
        pxyState.fogColor.vector.z = 255.0f;
    }
    if (pxyState.currentLight->shadows[0].w == 0.0f) {
        vector.row.component[0] = pxyState.currentLight->directions[0][0];
        vector.row.component[2] = pxyState.currentLight->directions[0][2];
    } else {
        vector.row.component[0] = pxyState.currentLight->shadows[0].x;
        vector.row.component[2] = pxyState.currentLight->shadows[0].z;
    }
    if (vector.row.component[0] == 0.0f && vector.row.component[2] == 0.0f) {
        pxyState.renderStatus |= 0x20;
    }
    pxyState.batchedModelCount++;
    pxyState.modelHeader = modelHeader;
    pxyState.parts = PXY_HEADER_AT(modelHeader, modelHeader->partOffset);
    pxyState.partRanges = PXY_HEADER_AT(modelHeader, modelHeader->rangeOffset);
    pxyState.headerTable58 = PXY_HEADER_AT(modelHeader, modelHeader->offset58);
    pxyState.headerTable6C = PXY_HEADER_AT(modelHeader, modelHeader->geometryOffset);
    pxyState.headerTable3C = PXY_HEADER_AT(modelHeader, modelHeader->offset3C);
    pxyState.defaultPartMatrices = PXY_HEADER_AT(modelHeader, modelHeader->entryMatrixOffset);
    pxyState.defaultJointMatrices = PXY_HEADER_AT(modelHeader, modelHeader->offset70);
    pxyState.headerTable64 = PXY_HEADER_AT(modelHeader, modelHeader->jointMatrixOffset);
    pxyState.headerTable74 = PXY_HEADER_AT(modelHeader, modelHeader->offset74);
    pxyState.headerTable78 = PXY_HEADER_AT(modelHeader, modelHeader->offset78);
    if ((pxyState.renderStatus & 0x800) || (modelHeader->flags & 0x88)) {
        if (pxyState.renderStatus & 1) {
            pxyState.geometryFogFactors = xglPacketReserve(modelHeader->geometryCount);
        }
    }
    pxyState.partVisibilityFlags = xglPacketReserve(modelHeader->partCount / 16 + 1);
    xglVectorScaleXYZ(&pxyState.modelPosition, &pxyState.modelPosition, pxyState.modelScaleMax);
    memset(pxyState.partVisibilityFlags, PXY_PART_VISIBLE, modelHeader->partCount);
    if (pxyState.secondPartMask != 0) {
        pxyState.secondPartFlags = xglPacketReserve(modelHeader->partCount / 16 + 1);
        memset(pxyState.secondPartFlags, 0xFF, modelHeader->partCount);
    } else {
        pxyState.secondPartFlags = 0;
    }
    if (pxyState.partMatrices == 0) {
        pxyState.partMatrices = pxyState.defaultPartMatrices;
    }
    xglMatrixInverse(pxyState.drawInverseMatrix.rows, pxyState.drawMatrix.rows);
    if ((pxyState.renderStatus & 0xE000) && pxyState.clipPlanes != 0) {
        vector.row = (Vector4Elements){ { 0.0f, 0.0f, 1.0f, 0.0f } };
        for (i = 0; i < pxyState.clipPlaneCount; i++) {
            pxyState.clipPlanes->plane[i].row.component[3] = 1.0f;
            xglMatrixStackLoad(pxyState.drawInverseMatrix.rows);
            xglMatrixStackMulVector(&pxyState.clipPlanes->plane[i].row.vector, &pxyState.clipPlanes->plane[i].row.vector);
            xglMatrixStackRotY(pxyState.clipPlanes->angle[i].row.component[1]);
            xglMatrixStackRotX(pxyState.clipPlanes->angle[i].row.component[0]);
            xglMatrixStackMulVector(&vector.row.vector, &vector.row.vector);
            vector.row.component[3] = -(vector.row.component[0] * pxyState.clipPlanes->plane[i].row.component[0]
                                    + vector.row.component[1] * pxyState.clipPlanes->plane[i].row.component[1]
                                    + vector.row.component[2] * pxyState.clipPlanes->plane[i].row.component[2]);
            pxyState.clipPlanes->plane[i] = vector;
            pxyState.clipPlanes->angle[i].row.component[0] = pxyState.clipPlanes->angle[i].row.component[2];
            pxyState.clipPlanes->angle[i].row.component[1] = pxyState.clipPlanes->angle[i].row.component[3];
            pxyState.clipPlanes->angle[i].row.component[2] = pxyState.clipPlanes->angle[i].row.component[0]
                                                             + pxyState.clipPlanes->angle[i].row.component[1];
        }
    }
    if (pxyState.parentHeader != 0) {
        int index;
        int remaining;

        index = pxyState.slotWriteIndex;
        remaining = PXY_SLOT_COUNT;
        do {
            if (--index < 0) {
                index = PXY_SLOT_COUNT - 1;
            }
            remaining--;
            if (pxyState.slots[index].packet != 0 && pxyState.slots[index].packet->header == pxyState.parentHeader) {
                slot = &pxyState.slots[index];
                parentPacket = slot->packet;
                pxyState.currentLight = parentPacket->light;
                pxyState.currentShadow = parentPacket->shadow;
                pxyState.shadowMatrix = parentPacket->shadowMatrix;
                if (pxyState.txyBlock == 0) {
                    pxyState.txyBlock = parentPacket->textureTable;
                }
                break;
            }
        } while (remaining > 0);
        pxyCalcFogFactors(modelHeader);
    } else {
        newSlot = &pxyState.slots[pxyState.slotWriteIndex++];
        if (pxyState.slotWriteIndex >= PXY_SLOT_COUNT) {
            pxyState.slotWriteIndex = 0;
        }
        func_001A12A0(modelHeader);
        pxyCalcCameraVisibility();
        pxyCalcFogFactors(modelHeader);
    }
    if ((pxyState.drawFlags & 0x80010) && (modelHeader->flags & 2)) {
        fadeFullDistance = 1073000.0f;
        fadeZeroDistance = 839000.0f;
        if (pxyState.fadeMetric < fadeZeroDistance) {
            pxyState.reflTransparency = 0;
            pxyState.fadeAlpha = 0;
        } else if (pxyState.fadeMetric < fadeFullDistance) {
            pxyState.reflTransparency *= 1.0f - (fadeFullDistance - pxyState.fadeMetric) / (fadeFullDistance - fadeZeroDistance);
            pxyState.fadeAlpha *= 1.0f - (fadeFullDistance - pxyState.fadeMetric) / (fadeFullDistance - fadeZeroDistance);
        }
        if (pxyState.renderLevel & 0x800) {
            pxyState.reflTransparency = 0;
        }
        if (pxyState.reflTransparency == 0.0f) {
            pxyState.renderLevel |= 0x800;
        }
    }
    if (pxyState.renderLevel & 0x1000) {
        float visibleDistance;
        float hiddenDistance;

        visibleDistance = 223000.0f;
        hiddenDistance = 209000.0f;
        fadeFactor = 1.0f;
        if (pxyState.fadeMetric < hiddenDistance) {
            fadeFactor = 0;
        } else if (pxyState.fadeMetric < visibleDistance) {
            fadeFactor *= 1.0f - (visibleDistance - pxyState.fadeMetric) / (visibleDistance - hiddenDistance);
        }
        if (fadeFactor <= 0.0f) {
            pxyResetGlobals();
            return 0;
        }
        if (fadeFactor < 1.0f) {
            pxyState.renderStatus |= 4;
            pxyState.reflTransparency *= fadeFactor;
            pxyState.transparency *= fadeFactor;
            pxyState.circleShadowScale *= fadeFactor;
            pxyState.fadeAlpha *= fadeFactor;
        }
    }
    if (!(pxyState.renderStatus & 0x10) || (pxyState.renderStatus & 2)) {
        func_001A17F8(modelHeader);
        if (modelHeader->geometryCount < 0x100 && pxyState.hideMask != 0) {
            int index;
            PxyPart *part;

            part = pxyState.parts;
            for (index = 0; index < modelHeader->partCount; index++, part++) {
                if (pxyState.hideMask->bits[(s16)(part->matrixIndex >> 5)] & (1 << (s16)(part->matrixIndex & 0x1F))) {
                    pxyState.partVisibilityFlags[index] &= ~PXY_PART_VISIBLE;
                }
            }
        }
    }
    if ((modelHeader->flags & 0x80) && modelHeader->geometryCount < 0x100 && pxyState.secondPartMask != 0) {
        int index;
        PxyPart *part;

        part = pxyState.parts;
        for (index = 0; index < modelHeader->partCount; index++, part++) {
            u8 *mask = pxyState.secondPartMask;

            pxyState.secondPartFlags[index] = mask[part->matrixIndex];
            if (pxyState.secondPartFlags[index] == 0) {
                pxyState.partVisibilityFlags[index] &= ~PXY_PART_VISIBLE;
            }
        }
    }
    if (modelHeader->nJointMatrix != 0 && pxyState.jointMatrices == 0) {
        pxyState.jointMatrices = pxyState.defaultJointMatrices;
    }
    if (pxyState.parentHeader == 0) {
        switch (pxyState.shadowMode) {
        case 1:
        case 4:
        case 5:
            if (!(modelHeader->flags & 1)) {
                pxySetupShadowProjector(modelHeader);
            }
            break;
        case 3:
            pxyState.shadowMatrix = xglPacketReserve(4);
            xglMatrixUnit(pxyState.shadowMatrix);
            if (pxyState.currentLight->shadows[0].w == 0.0f) {
                direction.x = pxyState.currentLight->directions[0][0];
                direction.y = 2.0f;
                direction.z = pxyState.currentLight->directions[0][2];
                if (pxyState.drawFlags & 0x10) {
                    direction.y = pxyState.currentLight->directions[0][1];
                }
            } else {
                direction.x = pxyState.currentLight->shadows[0].x;
                direction.y = pxyState.currentLight->shadows[0].y;
                direction.z = pxyState.currentLight->shadows[0].z;
            }
            xglVectorNormal(&direction, &direction);
            if (direction.y != 0.0f) {
                pxyState.shadowMatrix[1][0] = -direction.x / direction.y;
                pxyState.shadowMatrix[1][2] = -direction.z / direction.y;
            } else {
                pxyState.shadowMatrix[1][0] = 0;
                pxyState.shadowMatrix[1][2] = 0;
            }
            pxyState.shadowMatrix[1][1] = 0;
            pxyState.shadowMatrix[3][1] = pxyState.drawMatrix.rows[3][1] + 0.01f;
            pxyState.shadowMatrix[3][0] = -pxyState.drawMatrix.rows[3][1] * pxyState.shadowMatrix[1][0];
            pxyState.shadowMatrix[3][2] = -pxyState.drawMatrix.rows[3][1] * pxyState.shadowMatrix[1][2];
            pxyState.shadowMatrix[3][1] = pxyState.shadowPlaneHeight + 0.01f;
            pxyState.shadowMatrix[3][0] = -pxyState.shadowPlaneHeight * pxyState.shadowMatrix[1][0];
            pxyState.shadowMatrix[3][2] = -pxyState.shadowPlaneHeight * pxyState.shadowMatrix[1][2];
            break;
        case 2:
            break;
        }
    }
    packet = xglPacketCopy16(&pxyState.renderStatus, 0x37);
    range = packet->ranges;
    translucentRange = &packet->ranges[1];
    entry = &pxyState.drawEntryTable[pxyState.opaqueEntryCursor];
    translucentEntry = &pxyState.drawEntryTable[pxyState.translucentEntryCursor];
    table = &pxyState.batchTables[0];
    batch = &table->batches[table->tail.count];
    translucentTable = &pxyState.batchTables[1];
    translucentBatch = &translucentTable->batches[translucentTable->tail.count];
    if (slot == 0) {
        batch->firstDrawEntry = pxyState.opaqueEntryCursor;
        batch->drawEntryCount = 0;
        batch->depth = xglPointLength(&packet->center, &camera->travel.position) + pxyState.sortOffset;
        batch->batchFlags = packet->renderStatus;
        batch->unk10 = -1;
        batch->nextBatch = -1;
        batch->packet = packet;
        batch->translucent = translucentBatch;
        translucentBatch->firstDrawEntry = pxyState.translucentEntryCursor;
        translucentBatch->drawEntryCount = 0;
        translucentBatch->depth = 0;
        translucentBatch->batchFlags = packet->renderStatus;
        translucentBatch->unk10 = -1;
        translucentBatch->nextBatch = -1;
        translucentBatch->packet = packet;
    } else {
        batch = &table->batches[slot->batch];
        translucentBatch = &translucentTable->batches[slot->translucentBatch];
        translucentBatch->batchFlags |= 0x80000000;
        batch->batchFlags |= 0x400000;
        translucentBatch->batchFlags |= 0x400000;
        translucentBatch->batchFlags |= packet->renderStatus & 8;
        entry = &pxyState.drawEntryTable[batch->firstDrawEntry + batch->drawEntryCount];
        translucentEntry = &pxyState.drawEntryTable[translucentBatch->firstDrawEntry + translucentBatch->drawEntryCount];
    }
    if ((pxyState.renderStatus & 0x8800) || (modelHeader->flags & 0x88)) {
        batch->batchFlags |= 0x40000000;
        translucentBatch->batchFlags |= 0x40000000;
    }
    if (modelHeader->flags & 0x40) {
        batch->batchFlags |= 0x4000000;
        translucentBatch->batchFlags |= 0x4000000;
    }
    if (modelHeader->flags & 0x100) {
        batch->batchFlags |= 0x2000000;
        translucentBatch->batchFlags |= 0x2000000;
    }
    if (pxyState.stateFlags3E4 & 8) {
        batch->batchFlags |= 0x2000000;
        translucentBatch->batchFlags |= 0x2000000;
    }
    if ((modelHeader->flags & 0x800) || (pxyState.renderStatus & 0x20000)) {
        batch->batchFlags |= 0x20000;
        translucentBatch->batchFlags |= 0x20000;
    }
    if (pxyState.renderLevel & 0x40) {
        batch->batchFlags |= 0x100000;
        translucentBatch->batchFlags |= 0x100000;
    }
    if (modelHeader->flags & 0x200) {
        batch->batchFlags |= 0x800000;
        translucentBatch->batchFlags |= 0x800000;
    }
    if (packet->geometryFogFactors != 0) {
        batch->batchFlags |= 0x80000000;
        translucentBatch->batchFlags |= 0x80000000;
    }
    if ((modelHeader->flags & 0x20) || (pxyState.renderStatus & 8)) {
        batch->batchFlags |= 0x8000000;
        translucentBatch->batchFlags |= 0x8000000;
    }
    if (!(packet->renderStatus & 4)) {
        int layer;
        int count;
        int k;
        PxyPart *part;

        for (layer = 0; layer < modelHeader->layerCount; layer++) {
            part = &packet->parts[range->first];
            count = range->count;
            if (!(pxyState.renderStatus & 0x10) || (pxyState.renderStatus & 2)) {
                for (k = 0; k < count; k++, part++) {
                    if (part->layer == layer) {
                        if (!(packet->partFlags[part->index] & PXY_PART_VISIBLE)) {
                            continue;
                        }
                        if ((modelHeader->flags & 1) && !(packet->partFlags[part->index] & PXY_PART_LOD_MASK)) {
                            continue;
                        }
                        if (batch->firstDrawEntry + batch->drawEntryCount >= PXY_OPAQUE_ENTRY_END) {
                            break;
                        }
                        entry->part = part;
                        (entry++)->packet = packet;
                        batch->drawEntryCount++;
                    }
                }
            }
        }
    }
    if (packet->renderStatus & 4) {
        int layer;
        int count;
        int k;
        PxyPart *part;

        for (layer = 0; layer < modelHeader->layerCount; layer++) {
            part = &packet->parts[range->first];
            count = range->count;
            if (!(pxyState.renderStatus & 0x10) || (pxyState.renderStatus & 2)) {
                for (k = 0; k < count; k++, part++) {
                    if (part->layer == layer) {
                        if (!(packet->partFlags[part->index] & PXY_PART_VISIBLE)) {
                            continue;
                        }
                        if (part->flags & 0x10) {
                            continue;
                        }
                        if ((modelHeader->flags & 1) && !(packet->partFlags[part->index] & PXY_PART_LOD_MASK)) {
                            continue;
                        }
                        if (translucentBatch->firstDrawEntry + translucentBatch->drawEntryCount >= PXY_TRANSLUCENT_ENTRY_END) {
                            break;
                        }
                        translucentEntry->part = part;
                        (translucentEntry++)->packet = packet;
                        translucentBatch->drawEntryCount++;
                    }
                }
            }
        }
    }
    if (!(packet->flags & 1)) {
        int layer;
        int count;
        int k;
        int m;
        PxyPart *part;

        if (!(pxyState.renderStatus & 0x10) || (pxyState.renderStatus & 2)) {
            for (layer = modelHeader->layerCount - 1; layer >= 0; layer--) {
                part = &packet->parts[translucentRange->first];
                if (modelHeader->flags & 0x200) {
                    for (k = 0; k < translucentRange->count; k++, part++) {
                        if (part->layer == layer) {
                            if (packet->partFlags[part->index] & PXY_PART_MERGED) {
                                continue;
                            }
                            if (((part->flags & 0x80) || !(part->flags & 0x40)) && !(part->flags & 0x200)) {
                                if (!(part->flags & 0x240)) {
                                    if (!(packet->partFlags[part->index] & PXY_PART_VISIBLE)) {
                                        continue;
                                    }
                                    if ((modelHeader->flags & 1) && !(packet->partFlags[part->index] & PXY_PART_LOD_MASK)) {
                                        continue;
                                    }
                                }
                                if (translucentBatch->firstDrawEntry + translucentBatch->drawEntryCount >= PXY_TRANSLUCENT_ENTRY_END) {
                                    break;
                                }
                                translucentEntry->part = part;
                                (translucentEntry++)->packet = packet;
                                translucentBatch->drawEntryCount++;
                                if (part->flags & 0x80) {
                                    for (m = k + 1; m < translucentRange->count; m++) {
                                        if (part[1].flags & 0x80) {
                                            break;
                                        }
                                        if (part[1].flags & 0x240) {
                                            if (translucentBatch->firstDrawEntry + translucentBatch->drawEntryCount >= PXY_TRANSLUCENT_ENTRY_END) {
                                                break;
                                            }
                                            k++;
                                            part++;
                                            translucentEntry->part = part;
                                            (translucentEntry++)->packet = packet;
                                            packet->partFlags[part->index] |= PXY_PART_MERGED;
                                            translucentBatch->drawEntryCount++;
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (packet->renderLevel & 2) {
                        break;
                    }
                } else {
                    count = translucentRange->count;
                    if (!(modelHeader->flags & 0x1000) || modelHeader->sortEntryOffset == 0) {
                        for (k = 0; k < count; k++, part++) {
                            if (part->layer == layer) {
                                if (!(packet->partFlags[part->index] & PXY_PART_VISIBLE)) {
                                    continue;
                                }
                                if ((modelHeader->flags & 1) && !(packet->partFlags[part->index] & PXY_PART_LOD_MASK)) {
                                    continue;
                                }
                                if (translucentBatch->firstDrawEntry + translucentBatch->drawEntryCount >= PXY_TRANSLUCENT_ENTRY_END) {
                                    break;
                                }
                                translucentEntry->part = part;
                                (translucentEntry++)->packet = packet;
                                translucentBatch->drawEntryCount++;
                            }
                        }
                    } else {
                        w = &work;
                        w->sortEntries = PXY_HEADER_AT(modelHeader, modelHeader->sortEntryOffset);
                        w->entryCount = 0;
                        w->group = w->groups;
                        w->entries = translucentEntry;
                        w->groupCount = 0;
                        w->group->count = 0;
                        w->item = w->group->items;
                        w->item->count = 0;
                        w->matrixSet = pxyState.globalMatrices;
                        w->matrices = pxyState.jointMatrices;
                        QuadwordCopyMatrix(w->matrix, pxyState.drawMatrix.rows);
                        QuadwordCopy(&w->eye, &camera->travel.position);
                        for (k = 0; k < count; k++, part++) {
                            if (part->layer == layer) {
                                if (!(packet->partFlags[part->index] & PXY_PART_VISIBLE)) {
                                    continue;
                                }
                                if ((modelHeader->flags & 1) && !(packet->partFlags[part->index] & PXY_PART_LOD_MASK)) {
                                    continue;
                                }
                                if (translucentBatch->firstDrawEntry + translucentBatch->drawEntryCount >= PXY_TRANSLUCENT_ENTRY_END) {
                                    break;
                                }
                                translucentEntry->part = part;
                                (translucentEntry++)->packet = packet;
                                translucentBatch->drawEntryCount++;
                                w->entryCount++;
                            }
                        }
                        for (w->i = 0; w->i < w->entryCount; w->i++) {
                            w->sortEntry = &w->sortEntries[w->entries[w->i].part->matrixIndex];
                            if (w->sortEntry->key >= 0) {
                                if (w->item->count == 0 || w->item->key != w->sortEntry->key) {
                                    if (w->item->count != 0) {
                                        w->item++;
                                        w->group->count++;
                                    }
                                    w->item->start = w->i;
                                    w->item->count = 1;
                                    w->item->key = w->sortEntry->key;
                                    w->item->matrix = w->sortEntry->matrix;
                                    QuadwordCopy(w->item, w->sortEntry);
                                } else {
                                    w->item->count++;
                                }
                                if (w->i == w->entryCount - 1) {
                                    w->group->count++;
                                    w->group++;
                                    w->groupCount++;
                                    w->group->count = 0;
                                    w->item = w->group->items;
                                    w->item->count = 0;
                                }
                            } else if (w->item->count != 0) {
                                w->group->count++;
                                w->group++;
                                w->groupCount++;
                                w->group->count = 0;
                                w->item = w->group->items;
                                w->item->count = 0;
                            }
                        }
                        w->group = w->groups;
                        for (w->i = 0; w->i < w->groupCount; w->i++, w->group++) {
                            w->item = w->group->items;
                            w->group->start = w->group->items[0].start;
                            w->group->entryCount = 0;
                            for (w->j = 0; w->j < w->group->count; w->j++, w->item++) {
                                w->group->entryCount += w->item->count;
                                w->group->sorted[w->j] = w->item;
                                w->item->position.component[3] = 1.0f;
                                if (w->item->matrix >= 0 && w->matrixSet != 0 && w->matrices != 0) {
                                    xglMatrixStackLoad(w->matrix);
                                    xglMatrixStackMul(w->matrices[w->item->matrix].rows);
                                    xglMatrixStackMulVector(&w->item->position.vector, &w->item->position.vector);
                                    w->item->depth = xglPointLength(&w->item->position.vector, &w->eye);
                                } else {
                                    xglMatrixStackLoad(w->matrix);
                                    xglMatrixStackMulVector(&w->item->position.vector, &w->item->position.vector);
                                    w->item->depth = xglPointLength(&w->item->position.vector, &w->eye);
                                }
                            }
                            do {
                                w->swapped = 0;
                                w->cursor = w->group->sorted;
                                for (w->j = 0; w->j < w->group->count - 1; w->j++, w->cursor++) {
                                    if (w->cursor[0]->depth < w->cursor[1]->depth) {
                                        w->swapped = *w->cursor;
                                        *w->cursor = w->cursor[1];
                                        w->cursor[1] = w->swapped;
                                    }
                                }
                            } while (w->swapped != 0);
                            w->out = w->buffer;
                            w->cursor = w->group->sorted;
                            for (w->j = 0; w->j < w->group->count; w->j++, w->cursor++) {
                                memcpy(w->out, PXY_ENTRY_AT(w->entries, (*w->cursor)->start), (*w->cursor)->count * 8);
                                w->out += (*w->cursor)->count;
                            }
                            memcpy(PXY_ENTRY_AT(w->entries, w->group->start), w->buffer, w->group->entryCount * 8);
                        }
                    }
                }
            }
        }
    }
    func_001A0ED0(newSlot, packet);
    pxyState.opaqueEntryCursor += batch->drawEntryCount;
    pxyState.translucentEntryCursor += translucentBatch->drawEntryCount;
    pxyResetGlobals();
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A3638);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A3D90);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A3E30);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", nmlModelDirectSendFull);

/* kept apart: the real PxyDrawWork is declared after this function; same layout. */
typedef struct PxyDrawWorkView {
    PxyDrawEntry *entry;       /* +0x00 */
    int i;                            /* +0x04 */
    u32 batchFlags;                   /* +0x08 */
    int count;                        /* +0x0c */
    PxyDrawPacket *packet;     /* +0x10 */
    PxyDrawPacket *lastPacket; /* +0x14 */
    int mode;                         /* +0x18 */
    PxyPart *part;                    /* +0x1c */
    u32 partFlags;                    /* +0x20 */
    u32 packetFlags;                  /* +0x24 */
    int unk28;                        /* +0x28 */
    u8 cameraBit;                     /* +0x2c */
    u8 unk2D[3];
    u32 frameMask;          /* +0x30 */
    u32 frameBase;          /* +0x34 */
    PxyBatch *batch; /* +0x38 */
} PxyDrawWorkView;

int pxyPacketExcludesPart(PxyDrawPacket *packet, PxyDrawWorkView *work);

/* Returns 1 when the packet has a part index list (partIndexList, ended by 0xFF) that does not contain the part of the
 * work record, 0 otherwise: the part is excluded from the packet. */
int pxyPacketExcludesPart(PxyDrawPacket *packet, PxyDrawWorkView *work) {
    if (packet->partIndexList != 0) {
        int i;

        for (i = 0; i < 16; i++) {
            if (packet->partIndexList[i] == 0xFF) {
                break;
            }
            if (work->part->index == packet->partIndexList[i]) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

#include "gs.h"

/* One value of the scratchpad block at 0x70000000 (the effect work area). */
#define SEF_SCRATCHPAD(type, offset) (*(type *) (0x70000000 + (offset)))

#include "main/xgl_direct_queue.h"

/* The X3 layout of the packet a batch draws: X2's PxyPacket with 0x10 more bytes below +0x9c. */

/* ---- pxyDrawBatches ---- */
extern XglDirectQueueEntry *xglDirectQueueBuffer;
extern int xglDirectQueueCount;

/* Per-pass values of the frame blur, copied to the stack (unsized: .rodata, not gp data): the scaffold .rodata images at 0x541E78 (alphas 40, 48)
 * and 0x541E80 (texel shifts 2, 1). */
typedef struct BlurPassValues {
    int value[2];
} BlurPassValues;

extern const BlurPassValues pxyBlurPassAlphaTable[];
extern const BlurPassValues D_00541E80[];
const BlurPassValues pxyBlurPassAlphaTable[] = { { 40, 48 } };
const BlurPassValues D_00541E80[] = { { 2, 1 } };
const char D_00541E88[24] = "pxyFlush";
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

/* ---- pxyDrawShadowCaster ---- */
/* Draw state kept in the EE scratchpad while the batches are drawn. */
typedef struct PxyDrawWork {
    PxyDrawEntry *entry; /* +0x00 */
    int i; /* +0x04 */
    u32 batchFlags; /* +0x08 */
    int count; /* +0x0c */
    PxyDrawPacket *packet; /* +0x10 */
    PxyDrawPacket *lastPacket; /* +0x14 */
    int mode; /* +0x18 */
    PxyPart *part; /* +0x1c */
    u32 partFlags; /* +0x20 */
    u32 packetFlags; /* +0x24 */
    int unk28; /* +0x28 */
    u8 cameraBit; /* +0x2c */
    u8 unk2D[3];
    u32 frameMask; /* +0x30 */
    u32 frameBase; /* +0x34 */
    PxyBatch *batch; /* +0x38 */
} PxyDrawWork;

#define pxyDrawWork (&SEF_SCRATCHPAD(PxyDrawWork, 0x0))

typedef struct XglPacket XglPacket;

typedef unsigned int XglQuadword __attribute__((mode(TI)));

extern PxyDrawEntry D_008D3600[];
void func_001AAE70(int mode, PxyDrawPacket *packet);
void pxyPacketSelectMode(PxyDrawPacket *packet);
void func_001ACB50(void);
void func_001AB4E0(void);
void pxyDrawPartData(void);
XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkOpenUpkCode(XglPacket *packet, unsigned int address, unsigned int format, unsigned int a3,
                          unsigned int a4);
void sceVif1PkAddUpkData128N(XglPacket *packet, const void *data, unsigned int count);
void sceVif1PkAddUpkData128(XglPacket *packet, XglQuadword data);
void sceVif1PkCloseUpkCode(XglPacket *packet);
void xglMatrixMul(Matrix4 productMatrix, Matrix4 left, Matrix4 right);

/* Draws the batches of shadow caster `caster` into its shadow texture at frameBase. */
void pxyDrawShadowCasterTexture(int caster, u32 frameBase)
{
    PxyDrawWork *drawWork;
    PxyDrawEntry *drawEntries;
    int overlayIndex;
    PxyDrawPacket *packet;
    PxyShadow *shadow;
    int savedPass;

    packet = pxyState.shadowBatches[caster]->packet;
    shadow = packet->shadow;
    savedPass = pxyState.drawPass;
    /* One pointer to the scratchpad block, set once; every access goes through it. */
    drawWork = pxyDrawWork;
    drawWork->frameBase = frameBase;
    drawWork->unk28 = 0;
    nmlPacketClrModelCache();
    nmlPacketClrMatrixCache();
    {
        u64 *gifWrite;
        u64 *gifStart;
        int queueIndex;

        nmlPacketGsInit();
        queueIndex = xglDirectQueueCount;
        gifWrite = (u64 *)&xglDirectQueueBuffer[queueIndex];
        gifStart = gifWrite;
        GS_PUT(gifWrite, GS_ZBUF_1, GS_SET_ZBUF(D_004C8390.depthBase, 1, 1));
        GS_PUT(gifWrite, GS_FBA_1, 0);
        GS_PUT(gifWrite, GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        GS_PUT(gifWrite, GS_CLAMP_1, shadow->clamp);
        GS_PUT(gifWrite, GS_TEX0_1, shadow->tex0);
        GS_PUT(gifWrite, GS_ALPHA_1, shadow->alphaData);
        xglDirectQueueCount = queueIndex + (((u32)gifWrite - (u32)gifStart) >> 4);
    }
    nmlPacketAddGsFlushWide();
    for (overlayIndex = 0; overlayIndex < pxyState.overlayBatchCount; overlayIndex++) {
        int pass;
        PxyVector clipPlane;
        XglPacket *shadowQueue;
        Matrix4 uploadMatrix;
        AlignedVector4 partBounds[2];
        AlignedVector4 position;
        AlignedVector4 sphere;
        Matrix4 partMatrix;

        drawWork->batch = pxyState.overlayBatches[overlayIndex];
        if ((pxyState.drawFlags & 0x10) && !(pxyState.drawFlags & 0x40000)
            && (packet->drawMode == 1 || packet->drawMode == 4) && packet->lowerClipHeight < packet->upperClipHeight) {
            clipPlane.vector = (AlignedVector4){ 0.0f, -1.0f, 0.0f, 0.0f };
            clipPlane.vector.w = -(clipPlane.vector.x * packet->center.x + clipPlane.vector.y * packet->lowerClipHeight + clipPlane.vector.z * packet->center.z);
            if (drawWork->batch->packet->planes == 0) {
                drawWork->batch->packet->planes = xglPacketReserve(5);
            }
            drawWork->batch->packet->planes->plane[0] = clipPlane;
            drawWork->batch->packet->planes->plane[1] = clipPlane;
            clipPlane.vector = (AlignedVector4){ 0.0f, 1.0f, 0.0f, 0.0f };
            clipPlane.vector.w = -(clipPlane.vector.x * packet->center.x + clipPlane.vector.y * packet->upperClipHeight + clipPlane.vector.z * packet->center.z);
            if (drawWork->batch->packet->planes == 0) {
                drawWork->batch->packet->planes = xglPacketReserve(5);
            }
            drawWork->batch->packet->planes->plane[1] = clipPlane;
            drawWork->batch->packet->planeCount = 2;
            drawWork->batch->packet->renderStatus |= 0x4000;
        }
        nmlPacketGsInit();
        {
            u64 *gifWrite;
            u64 *gifStart;
            int queueIndex;

            queueIndex = xglDirectQueueCount;
            gifWrite = (u64 *)&xglDirectQueueBuffer[queueIndex];
            gifStart = gifWrite;
            GS_PUT(gifWrite, GS_FRAME_1, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm,
                                               drawWork->batch->packet->frameMask | 0xFF000000));
            GS_PUT(gifWrite, GS_FRAME_2, GS_SET_FRAME(frameBase, D_004C8390.screenWidth / 64, D_004C8390.psm,
                                               drawWork->batch->packet->frameMask | 0xFF000000));
            xglDirectQueueCount = queueIndex + (((u32)gifWrite - (u32)gifStart) >> 4);
        }
        nmlPacketAddGsFlush();
        for (pass = 0; pass < 2; pass++) {
            drawWork->lastPacket = drawWork->batch->packet;
            pxyState.drawPass = pass;
            pxyPacketSetCacheValue(pxyState.drawPass);
            if (pass != 0) {
                drawWork->batch = drawWork->batch->translucent;
            }
            if (!((pxyCameraEnabled[pxyState.camera] == 1
                   || pxyState.camera == drawWork->batch->packet->camera)
                  && drawWork->batch->packet->cameraMask[pxyState.camera] != 0)) {
                continue;
            }
            drawWork->batch->packet->renderStatus |= 0x20000000;
            func_001AAE70(drawWork->batch->packet->renderStatus & 4, drawWork->batch->packet);
            pxyPacketSelectMode(drawWork->batch->packet);
            shadowQueue = xglPacketGetCurrent();
            xglMatrixStackLoad(shadow->textureInverse.rows);
            xglMatrixStackMul(drawWork->batch->packet->localMatrix);
            xglMatrixStackSave(uploadMatrix);
            sceVif1PkCnt(shadowQueue, 0);
            sceVif1PkOpenUpkCode(shadowQueue, 0x39E, 0x6C, 1, 1);
            sceVif1PkAddUpkData128N(shadowQueue, uploadMatrix, 4);
            sceVif1PkCloseUpkCode(shadowQueue);
            sceVif1PkOpenUpkCode(shadowQueue, 0x3C3, 0x6C, 1, 1);
            sceVif1PkAddUpkData128(shadowQueue, shadow->uv.quadword);
            sceVif1PkCloseUpkCode(shadowQueue);
            xglMatrixUnit(uploadMatrix);
            shadow->center.w = 1.0f;
            shadow->lighting.direction.w = 0;
            QuadwordCopy(uploadMatrix[3], &shadow->lighting.direction);
            uploadMatrix[0][3] = uploadMatrix[3][0];
            uploadMatrix[1][3] = uploadMatrix[3][1];
            uploadMatrix[2][3] = uploadMatrix[3][2];
            uploadMatrix[0][2] = shadow->center.x;
            uploadMatrix[1][2] = shadow->center.y;
            uploadMatrix[2][2] = shadow->center.z;
            uploadMatrix[0][0] = 128.0f / (packet->center.w * 2.0f);
            /* Two words of the uploaded matrix are integer flags, written through the Matrix view. */
            if (packet->drawMode == 4) {
                ((Matrix *)uploadMatrix)->words[0][1] = 1;
            }
            ((Matrix *)uploadMatrix)->words[1][0] = 1;
            sceVif1PkOpenUpkCode(shadowQueue, 0x392, 0x6C, 1, 1);
            sceVif1PkAddUpkData128N(shadowQueue, uploadMatrix, 4);
            sceVif1PkCloseUpkCode(shadowQueue);
            /* The parts of this batch in the draw entry table. */
            drawEntries = D_008D3600;
            drawWork->entry = &drawEntries[drawWork->batch->firstDrawEntry];
            for (drawWork->i = 0; drawWork->i < drawWork->batch->drawEntryCount;
                 drawWork->i++, drawWork->entry++) {
                drawWork->packet = drawWork->entry->packet;
                drawWork->part = drawWork->entry->part;
                if (drawWork->packet->header->flags & 0x80) {
                    break;
                }
                if (pxyPacketExcludesPart(packet, drawWork)) {
                    continue;
                }
                if (drawWork->part->flags & 0x1000) {
                    continue;
                }
                if (pxyState.drawFlags & 0x10) {
                    if (drawWork->part->flags & 0x200000) {
                        continue;
                    }
                    if (pxyState.shadowCasterMode == 2) {
                        if (pass != 0 && !(drawWork->part->flags & 0x20000)) {
                            continue;
                        }
                        if (pass == 0 && !(drawWork->part->flags & 0x20000)
                            && (drawWork->part->flags & 0x10)) {
                            continue;
                        }
                    } else if (pxyState.shadowCasterMode == 1 && (drawWork->part->flags & 0x10)) {
                        continue;
                    }
                } else if (pxyState.shadowCasterMode == 1 && (drawWork->part->flags & 0x10)) {
                    continue;
                }
                if (!(pxyState.drawFlags & 0x400000)) {
                    if (pxyState.drawFlags & 0x10) {
                        Vu0AddVectorXYZ(&partBounds[0], &drawWork->part->boundsMax, drawWork->batch->packet->localMatrix[3]);
                        Vu0AddVectorXYZ(&partBounds[1], &drawWork->part->boundsMin, drawWork->batch->packet->localMatrix[3]);
                        if (shadow->min.x > partBounds[0].x || shadow->min.y > partBounds[0].y || shadow->min.z > partBounds[0].z
                            || shadow->max.x < partBounds[1].x || shadow->max.y < partBounds[1].y || shadow->max.z < partBounds[1].z) {
                            continue;
                        }
                    } else {
                        clipPlane.vector = drawWork->part->sphere;
                        clipPlane.vector.w = 1.0f;
                        xglVectorMulMat(&partBounds[0], drawWork->batch->packet->localMatrix, &clipPlane.vector);
                        if (shadow->min.x > partBounds[0].x + drawWork->part->sphere.w
                            || shadow->min.y > partBounds[0].y + drawWork->part->sphere.w
                            || shadow->min.z > partBounds[0].z + drawWork->part->sphere.w
                            || shadow->max.x < partBounds[0].x - drawWork->part->sphere.w
                            || shadow->max.y < partBounds[0].y - drawWork->part->sphere.w
                            || shadow->max.z < partBounds[0].z - drawWork->part->sphere.w) {
                            continue;
                        }
                    }
                }
                drawWork->packet->renderStatus |= 0x20000000;
                func_001ACB50();
                if ((drawWork->batch->batchFlags & 0x40000000) || drawWork->lastPacket != drawWork->packet) {
                    func_001AB4E0();
                }
                pxyDrawPartData();
                drawWork->packet->renderStatus &= ~0x20000000;
                drawWork->lastPacket = drawWork->packet;
            }
            if (drawWork->i < drawWork->batch->drawEntryCount) {
                for (; drawWork->i < drawWork->batch->drawEntryCount; drawWork->i++, drawWork->entry++) {
                    Matrix *boneMatrix = &drawWork->entry->packet->boneMatrices[drawWork->entry->part->matrixIndex];
                    XglPacket *partQueue;

                    drawWork->packet = drawWork->entry->packet;
                    drawWork->part = drawWork->entry->part;
                    if (drawWork->part->flags & 0x1000) {
                        continue;
                    }
                    if (pxyState.drawFlags & 0x10) {
                        if (drawWork->part->flags & 0x200000) {
                            continue;
                        }
                        if (pxyState.shadowCasterMode == 2) {
                            if (pass != 0 && !(drawWork->part->flags & 0x20000)) {
                                continue;
                            }
                            if (pass == 0 && !(drawWork->part->flags & 0x20000)
                                && (drawWork->part->flags & 0x10)) {
                                continue;
                            }
                        } else if (pxyState.shadowCasterMode == 1
                                   && (drawWork->part->flags & 0x10)) {
                            continue;
                        }
                    } else if (pxyState.shadowCasterMode == 1 && (drawWork->part->flags & 0x10)) {
                        continue;
                    }
                    if (!(pxyState.drawFlags & 0x400000)) {
                        if (pxyState.drawFlags & 0x10) {
                            clipPlane.vector = drawWork->part->sphere;
                            clipPlane.vector.w = 1.0f;
                            xglMatrixStackLoad(drawWork->entry->packet->localMatrix);
                            xglMatrixStackMul(boneMatrix->rows);
                            xglMatrixStackMulVector(&clipPlane.vector, &clipPlane.vector);
                            clipPlane.vector.w = drawWork->part->sphere.w;
                            if (shadow->min.x > clipPlane.vector.x + drawWork->part->sphere.w
                                || shadow->min.y > clipPlane.vector.y + drawWork->part->sphere.w
                                || shadow->min.z > clipPlane.vector.z + drawWork->part->sphere.w
                                || shadow->max.x < clipPlane.vector.x - drawWork->part->sphere.w
                                || shadow->max.y < clipPlane.vector.y - drawWork->part->sphere.w
                                || shadow->max.z < clipPlane.vector.z - drawWork->part->sphere.w) {
                                continue;
                            }
                        } else {
                            sphere = drawWork->part->sphere;
                            sphere.w = 1.0f;
                            xglVectorMulMat(&position, boneMatrix->rows, &sphere);
                            if (shadow->min.x > position.x + drawWork->part->sphere.w
                                || shadow->min.y > position.y + drawWork->part->sphere.w
                                || shadow->min.z > position.z + drawWork->part->sphere.w
                                || shadow->max.x < position.x - drawWork->part->sphere.w
                                || shadow->max.y < position.y - drawWork->part->sphere.w
                                || shadow->max.z < position.z - drawWork->part->sphere.w) {
                                continue;
                            }
                        }
                    }
                    partQueue = xglPacketGetCurrent();
                    xglMatrixMul(partMatrix, shadow->textureInverse.rows, drawWork->packet->localMatrix);
                    xglMatrixMul(partMatrix, partMatrix,
                                 drawWork->packet->boneMatrices[drawWork->part->matrixIndex].rows);
                    sceVif1PkCnt(partQueue, 0);
                    sceVif1PkOpenUpkCode(partQueue, 0x39E, 0x6C, 1, 1);
                    sceVif1PkAddUpkData128N(partQueue, partMatrix, 4);
                    sceVif1PkCloseUpkCode(partQueue);
                    sceVif1PkOpenUpkCode(partQueue, 0x3C3, 0x6C, 1, 1);
                    sceVif1PkAddUpkData128(partQueue, shadow->uv.quadword);
                    sceVif1PkCloseUpkCode(partQueue);
                    xglMatrixMul(partMatrix, drawWork->packet->localMatrix,
                                 drawWork->packet->boneMatrices[drawWork->part->matrixIndex].rows);
                    xglMatrixInverse(partMatrix, partMatrix);
                    shadow->center.w = 1.0f;
                    xglVectorMulMat(&sphere, partMatrix, &shadow->center);
                    shadow->lighting.direction.w = 0;
                    xglVectorMulMat((Vector4 *)partMatrix[3], partMatrix, &shadow->lighting.direction);
                    partMatrix[0][3] = partMatrix[3][0];
                    partMatrix[1][3] = partMatrix[3][1];
                    partMatrix[2][3] = partMatrix[3][2];
                    partMatrix[0][2] = sphere.x;
                    partMatrix[1][2] = sphere.y;
                    partMatrix[2][2] = sphere.z;
                    partMatrix[0][0] = 128.0f / (packet->center.w * 2.0f);
                    if (packet->drawMode == 4) {
                        ((Matrix *)partMatrix)->words[0][1] = 1;
                    }
                    ((Matrix *)partMatrix)->words[1][0] = 1;
                    sceVif1PkOpenUpkCode(partQueue, 0x392, 0x6C, 1, 1);
                    sceVif1PkAddUpkData128N(partQueue, partMatrix, 4);
                    sceVif1PkCloseUpkCode(partQueue);
                    drawWork->packet->renderStatus |= 0x20000000;
                    func_001ACB50();
                    if ((drawWork->batch->batchFlags & 0x40000000) || drawWork->lastPacket != drawWork->packet) {
                        func_001AB4E0();
                    }
                    pxyDrawPartData();
                    drawWork->packet->renderStatus &= ~0x20000000;
                    drawWork->lastPacket = drawWork->packet;
                }
            }
            drawWork->batch->packet->renderStatus &= ~0x20000000;
            if (pxyState.shadowCasterMode == 1) {
                break;
            }
        }
        if ((pxyState.drawFlags & 0x10) && (packet->drawMode == 1 || packet->drawMode == 4)) {
            drawWork->batch->packet->renderStatus &= ~0x4000;
        }
    }
    nmlPacketClrTextureCache();
    nmlPacketClrModelCache();
    nmlPacketClrMatrixCache();
    pxyState.drawPass = savedPass;
    pxyPacketSetCacheValue(pxyState.drawPass);
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A5108);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A52C8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A57A0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A5D30);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A62A8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A6660);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A69D8);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A7690);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001A9E00);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001AA1B0);

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001AA580);

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
void pxyClearEntryTable(void);

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

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001AA828);

s32 func_001A3D90(s32 mode);

void pxyPlayEntryGroupFive(void) {
    func_001A3D90(5);
}

INCLUDE_ASM("asm/main/nonmatchings/nml_model_set", func_001AA968);

/* ---- data of this TU (scaffold items recovered in C) ---- */
int gPxyFadeRampIndexTable[32] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    16, 17, 18, 19, 20, 21, 22, 23,
    8, 9, 10, 11, 12, 13, 14, 15,
    24, 25, 26, 27, 28, 29, 30, 31
};
int pxyCameraEnabled[4] = { 0 };
Vector4 pxyGlobalColor = { 0 };

s32 D_00551240 = 0;
s32 gPxyDirtyFlag = 0;
int pxyShadowTexturesReady = 0;

const float D_0054FFF4 = 0.2f;

/* .bss: the render state (its inner labels are fields), then the shadow texture, its CLUT and the fade ramp table. */
PxyModelRenderState gPxyModelRenderStateStorage;
PxyCircleShadowTexture pxyCircleShadowTexture;
PxyUploadHeader pxyCircleShadowClut;
int gPxyFadeRampTable[1024];
