#include "common.h"
#include "shared.h"
#include "main/Packet.h"
#include "main/TiWatSim.h"
#include "main/tu007.h"
#include "main/tu045.h"
#include "main/xgl_2.h"
#include "main/xgl_render.h"

/* Address at a byte offset read from a header (the offset is data, not a field): base + off bytes. */
#define BYTE_OFFSET(base, off) ((unsigned char *) (base) + (off))
/* The same address typed as `type *`, so each use needs no cast. */
#define BYTE_OFFSET_AS(type, base, off) ((type *) BYTE_OFFSET(base, off))

struct PxyModelHeader;
void tiWaterObjectUpdate(struct PxyModelHeader *pxyModelPack, int stepMode);

#define GS_TEX0_1 0x06
#define GS_CLAMP_1 0x08
#define GS_TEX1_1 0x14
#define GS_TEXFLUSH 0x3F
#define GS_ALPHA_1 0x42
#define GS_TEST_1 0x47
#define GS_FBA_1 0x4A
#define GS_FRAME_1 0x4C
#define GS_ZBUF_1 0x4E
#define GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld) \
    ((tbp) | ((tbw) << 14) | ((psm) << 20) | ((tw) << 26) | ((u64)(th) << 30) \
     | ((u64)(tcc) << 34) | ((u64)(tfx) << 35) | ((u64)(cbp) << 37) | ((u64)(cpsm) << 51) \
     | ((u64)(csm) << 55) | ((u64)(csa) << 56) | ((u64)(cld) << 61))
#define GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
    ((lcm) | ((mxl) << 2) | ((mmag) << 5) | ((mmin) << 6) | ((mtba) << 9) \
     | ((l) << 19) | ((u64)(k) << 32))
#define GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
    ((wms) | ((wmt) << 2) | ((minu) << 4) | ((maxu) << 14) | ((u64)(minv) << 24) \
     | ((u64)(maxv) << 34))
#define GS_SET_ALPHA(a, b, c, d, fix) \
    ((a) | ((b) << 2) | ((c) << 4) | ((d) << 6) | ((u64)(fix) << 32))
#define GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
    ((u64)(ate) | ((atst) << 1) | ((aref) << 4) | ((afail) << 12) | ((date) << 14) \
     | ((datm) << 15) | ((zte) << 16) | ((ztst) << 17))
#define GS_SET_FRAME(fbp, fbw, psm, fbmsk) \
    ((fbp) | ((fbw) << 16) | ((psm) << 24) | ((u64)(fbmsk) << 32))
#define GS_SET_ZBUF(zbp, psm, zmsk) ((zbp) | ((psm) << 24) | ((u64)(zmsk) << 32))
/* CLAMP wrap modes. */
#define GS_REGION_CLAMP 2

struct XglPacket;
void nmlModelUseSubWindow(unsigned int index, int enabled);
void gifStateAddReg(int gifAddress, u64 gifData);
void gifStateSetDecrease(f32 decrease);
void gifStateSetCenter(int centerX, int centerY);
void gifStateSetEffectAlpha(int alphaType, int alpha);
void gifStateSetFbp(int fbp);
void gifStateSetBlur(int blur);
void gifStateReset(void);
void gifStateFlush(void);
void xglMatrixInverse(Matrix *destination, Matrix *source);
union Matrix;
void nmlPacketClrTextureCache(void);
void nmlPacketClrMatrixCache(void);
void nmlPacketClrModelCache(void);
void xglPacketModeReset(void);
void xglPacketInitVuStateArg(int value);
void pxyRequestTextureCopyPass(void);

#define PAD_TRIANGLE 0x0010
#define PAD_CROSS 0x0040
#define PAD_SELECT 0x0100
#define PAD_UP 0x1000
#define PAD_DOWN 0x4000

/* A 128-bit store or load of the +0x30 analog group (a C store of a zero is `por $4,$0,$0; sq $4,0x30(pad)`;
 * QuadwordZero would drop the por and change the register allocation of xglPadRead). */
typedef unsigned int PadQuadword __attribute__((mode(TI)));

extern PadRecord PadData[2];
void tiWatSimStepRain(TiWatSimItem *waterItem);
void tiWatSimRandomizeItem(TiWatSimItem *waterItem, int range);
void tiWatSimUpdate(float step, TiWatSimItem *waterItem);
extern int tiWaterDebugMode;
extern int tiWaterSubCount;
extern TiWaterSub *tiWaterSubList[];
extern const char *tiWaterDebugLabels[];
extern float *tiWaterDebugValues[];
extern float tiWaterDebugSteps[];
extern float tiWaterCx;
extern float tiWaterCy;
extern float tiWaterAlphaType;
extern float tiWaterAlpha;
extern float tiWaterFbp;
extern float tiWaterBlur;
extern float tiWaterDecrease;
extern float tiWaterIntVals[4];
extern XglRenderState sRender;
void xglFontDebugPrintf(int x, int y, const char *format, ...);
struct XglCamera;
struct XglCamera *xglStudioSelectGetActiveCamera(int studio_index);
void xglCurMulRotateY(float angle);
void xglCurMulScaleXYZ(const f32 scale[3]);
void xglCurMatrixGet(Matrix4 matrix);
void xglCurSetTranslate(const Vector4 *vector);
XglPacket *xglPacketGetCurrent(void);
void sceVif1PkCnt(XglPacket *packet, unsigned int count);
void sceVif1PkAddCode(XglPacket *packet, unsigned int tag);
void sceVif1PkAddDataN(XglPacket *packet, const void *data, unsigned int qwc);
void sceVif1PkOpenDirectHLCode(XglPacket *packet, unsigned int count);
void sceVif1PkCloseDirectHLCode(XglPacket *packet);
void tiWaterSendFrame(XglCamera *camera, Matrix4 data);
void tiWaterSendDirection(XglCamera *camera, Matrix4 transform);
void tiWaterSendItemPair(float x, float y);
/* debug panel mode, toggled by select in the scene loop */
extern int tiWaterDebugIndex;
extern const char tiWaterMenuFormat[];
extern const char tiWaterMenuMarker[];

/* Number of debug parameters the panel cycles through. */
#define TI_WATER_DEBUG_PARAMS 12
/* Parameters shown at once, and the line height. */
#define TI_WATER_DEBUG_LINES 3
#define TI_WATER_DEBUG_LINE_HEIGHT 9
/* The "WAT " tag at the start of a water table. */
#define TI_WATER_SUB_MAGIC 0x20544157
/* Most water tables tiWaterSubList holds. */
#define TI_WATER_MAX_SUBS 64
/* Item flag: the item is set up (its block, height buffers, transforms and random heights). */
#define TI_WATER_ITEM_SET_UP 0x100
/* Range of the random heights given to a new item. */
#define TI_WATER_RANDOM_RANGE 30

/* Registers a water table in tiWaterSubList and sets each of its items up once: the block and the height
 * buffers (offsets from the table), the translate, rotate and scale transform, its inverse and random heights.
 * Returns 0 when the list is full or the table is not a water table, 1 otherwise. */
int tiWaterRegisterSub(TiWaterSub *waterSub);
/* The water state template sent by tiWaterSendStateTemplate, and its length for sceVif1PkAddDataN. */
extern u64 tiWaterStateTemplate[];

#define TI_WATER_STATE_TEMPLATE_COUNT 0x3C
/* The two words tiWaterSendStateTemplate patches: the data of the FRAME_1 and TEX0_1 A+D pairs. */
#define TI_WATER_TEMPLATE_FRAME_1 (0x60 / 8)
#define TI_WATER_TEMPLATE_TEX0_1 (0x70 / 8)

void tiWaterSendStateTemplate(void);
void tiWaterSendGrid(TiWatSimItem *waterItem, void *program);
/* VU1 microprogram entry points of the water drawing (VU1 micro memory addresses, linked as absolute symbols
 * like sv_get_image_item.c's D_50 / D_80) and the grid tables sent with them. */
extern unsigned char D_688[];
/* the surface pass */
extern unsigned char D_7F0[];
/* the depth pass while tiWaterDepthProgramSelect is set */
extern unsigned char D_9F0[];
/* the depth pass otherwise */
extern int tiWaterDepthProgramSelect;
extern unsigned char tiWaterSurfaceGridHeader[];
extern unsigned char tiWaterDepthGridHeader[];

/* VIF MSCAL: start the VU1 microprogram at the given address (in 8-byte units). */
#define TI_WATER_VIF_MSCAL(address) (0x14000000 | ((unsigned int) (address) >> 3))
/* Item flags: the two draw passes. */
#define TI_WATER_ITEM_DRAW_SURFACE 0x4
#define TI_WATER_ITEM_DRAW_DEPTH 0x2

/* Draws one water item: sends its frame, values and direction, then for each enabled pass the GS state
 * (clamp, frame on the display page, blend, the frame page as texture, Z test) and the VU1 program with
 * the item's grid. */
void tiWaterDrawItem(TiWatSimItem *waterItem);
/* Data of this TU (the scaffold items, in address order). */
extern const char tiWaterMenuFormat[24];
extern const char tiWaterMenuMarker[8];

/* Resets the debug panel mode and its selected parameter to zero. */
void tiWaterDebugReset(void) {
    tiWaterDebugMode = 0;
    tiWaterDebugIndex = 0;
}

/* Draws the debug panel (the selected parameter and the next two, the selected one marked) and
 * changes the selected value by its step: triangle adds, cross subtracts, held while select is
 * held and repeated otherwise. */
void tiWaterDebugDrawParams(void)
{
    unsigned int i;
    unsigned int line;
    unsigned short buttons;

    line = 0;
    xglFontDebugPrintf(0, 0, tiWaterMenuFormat, tiWaterDebugIndex + 1, TI_WATER_DEBUG_PARAMS);
    i = tiWaterDebugIndex;
    for (; line < TI_WATER_DEBUG_LINES && i < TI_WATER_DEBUG_PARAMS; i++, line++) {
        if (tiWaterDebugIndex == i) {
            xglFontDebugPrintf(0, (line + 1) * TI_WATER_DEBUG_LINE_HEIGHT, tiWaterMenuMarker);
        }
        xglFontDebugPrintf(10, (line + 1) * TI_WATER_DEBUG_LINE_HEIGHT, tiWaterDebugLabels[i], *tiWaterDebugValues[i]);
    }
    buttons = PadData[0].held;
    if (!(buttons & PAD_SELECT)) {
        buttons = PadData[0].repeated;
    }
    if (buttons & PAD_TRIANGLE) {
        *tiWaterDebugValues[tiWaterDebugIndex] += tiWaterDebugSteps[tiWaterDebugIndex];
    }
    if (buttons & PAD_CROSS) {
        *tiWaterDebugValues[tiWaterDebugIndex] -= tiWaterDebugSteps[tiWaterDebugIndex];
    }
}

/* Debug panel step: up and down on pad 0 select the parameter (wrapping), the panel is drawn and
 * every parameter is applied to the GIF state and the four model sub windows. */
void tiWaterDebugUpdate(void)
{
    if (PadData[0].pressed & PAD_UP) {
        if (--tiWaterDebugIndex < 0) {
            tiWaterDebugIndex = TI_WATER_DEBUG_PARAMS - 1;
        }
    }
    if (PadData[0].pressed & PAD_DOWN) {
        if (++tiWaterDebugIndex >= TI_WATER_DEBUG_PARAMS) {
            tiWaterDebugIndex = 0;
        }
    }
    tiWaterDebugDrawParams();
    gifStateSetCenter(tiWaterCx, tiWaterCy);
    gifStateSetEffectAlpha(tiWaterAlphaType, tiWaterAlpha);
    gifStateSetDecrease(tiWaterDecrease);
    gifStateSetFbp(tiWaterFbp);
    gifStateSetBlur(tiWaterBlur);
    nmlModelUseSubWindow(0, tiWaterIntVals[0]);
    nmlModelUseSubWindow(1, tiWaterIntVals[1]);
    nmlModelUseSubWindow(2, tiWaterIntVals[2]);
    nmlModelUseSubWindow(3, tiWaterIntVals[3]);
}

/* Registers a water table in tiWaterSubList and sets each of its items up once: the block and the height
 * buffers (offsets from the table), the translate, rotate and scale transform, its inverse and random heights.
 * Returns 0 when the list is full or the table is not a water table, 1 otherwise. */
int tiWaterRegisterSub(TiWaterSub *waterSub)
{
    unsigned int i;
    TiWatSimItem *waterItem;
    TiWaterSub *registeredSub;

    if (tiWaterSubCount >= TI_WATER_MAX_SUBS) {
        return 0;
    }
    if (waterSub->magic != TI_WATER_SUB_MAGIC) {
        return 0;
    }
    registeredSub = waterSub;
    if (!registeredSub->active) {
        return 1;
    }
    tiWaterSubList[tiWaterSubCount++] = registeredSub;
    waterItem = registeredSub->items;
    for (i = 0; i < registeredSub->active; i += 1, waterItem++) {
        if ((waterItem->flags & TI_WATER_ITEM_SET_UP) == 0) {
            waterItem->height_buffer_a = BYTE_OFFSET_AS(float, registeredSub, waterItem->height_offset_a);
            waterItem->height_buffer_b = BYTE_OFFSET_AS(float, registeredSub, waterItem->height_offset_b);
            waterItem->block = (u8 *) registeredSub;
            xglCurSetTranslate(&waterItem->translate);
            xglCurMulRotateY(waterItem->rotateY);
            xglCurMulScaleXYZ(&waterItem->scale.x);
            xglCurMatrixGet(waterItem->transform.rows);
            xglMatrixInverse(&waterItem->matrix, &waterItem->transform);
            tiWatSimRandomizeItem(waterItem, TI_WATER_RANDOM_RANGE);
            waterItem->flags |= TI_WATER_ITEM_SET_UP;
        }
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/TiWater", tiWaterSendStateTemplate);

/* Scaffold data: the render state u16 words (flip base at +0x0E, display base at +0x16). */
extern u16 D_004C8390[];

/* Draws one water item: sends its frame, values and direction, then for each enabled pass the GS state
 * (clamp, frame on the display page, blend, the frame page as texture, Z test) and the VU1 program with
 * the item's grid. */
void tiWaterDrawItem(TiWatSimItem *waterItem)
{
    XglPacket *packet = xglPacketGetCurrent();
    XglCamera *camera = xglStudioSelectGetActiveCamera(0);

    tiWaterSendFrame(camera, waterItem->transform.rows);
    tiWaterSendItemPair(waterItem->drawPair[0], waterItem->drawPair[1]);
    tiWaterSendDirection(camera, waterItem->transform.rows);
    if (waterItem->flags & TI_WATER_ITEM_DRAW_SURFACE) {
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 256, 510, 0, 222));
        gifStateAddReg(GS_FBA_1, 1);
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(D_004C8390[11], 8, 0, 0));
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 1, 2, 1, 0x4F));
        gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(D_004C8390[7] << 5, 8, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, 0x31, 1));
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
        sceVif1PkCnt(packet, 0);
        sceVif1PkAddCode(packet, TI_WATER_VIF_MSCAL(D_688));
        tiWaterSendGrid(waterItem, tiWaterSurfaceGridHeader);
    }
    if (waterItem->flags & TI_WATER_ITEM_DRAW_DEPTH) {
        sceVif1PkCnt(packet, 0);
        sceVif1PkOpenDirectHLCode(packet, 0);
        gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(GS_REGION_CLAMP, GS_REGION_CLAMP, 0, 254, 0, 222));
        gifStateAddReg(GS_FBA_1, 1);
        gifStateAddReg(GS_FRAME_1, GS_SET_FRAME(D_004C8390[11], 8, 0, 0));
        gifStateAddReg(GS_ALPHA_1, GS_SET_ALPHA(0, 2, 2, 1, 0x40));
        gifStateAddReg(GS_TEX0_1, GS_SET_TEX0(D_004C8390[7] << 5, 8, 0, 8, 8, 1, 0, 0, 0, 0, 0, 0));
        gifStateAddReg(GS_TEX1_1, GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0));
        gifStateAddReg(GS_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
        gifStateAddReg(GS_TEXFLUSH, 0);
        gifStateAddReg(GS_ZBUF_1, GS_SET_ZBUF(0, 0x31, 0));
        gifStateFlush();
        sceVif1PkCloseDirectHLCode(packet);
        sceVif1PkCnt(packet, 0);
        if (tiWaterDepthProgramSelect != 0) {
            sceVif1PkAddCode(packet, TI_WATER_VIF_MSCAL(D_7F0));
        } else {
            sceVif1PkAddCode(packet, TI_WATER_VIF_MSCAL(D_9F0));
        }
        tiWaterSendGrid(waterItem, tiWaterDepthGridHeader);
    }
}

/* Draw callback: in pass 0, draws every registered table (CLAMP_1 and the state template first when
 * it has items, then each item) and empties the list; the packet state is reset afterwards. */
void tiWaterDrawAll(int pass)
{
    XglPacket *packet = xglPacketGetCurrent();
    unsigned int i;
    unsigned int j;
    TiWatSimItem *waterItem;

    if (pass != 0) {
        return;
    }
    for (i = 0; i < tiWaterSubCount; i++) {
        waterItem = tiWaterSubList[i]->items;
        gifStateReset();
        if (tiWaterSubList[i]->active != 0) {
            sceVif1PkCnt(packet, 0);
            sceVif1PkOpenDirectHLCode(packet, 0);
            gifStateAddReg(GS_CLAMP_1, GS_SET_CLAMP(0, 0, 0xFF, 0xFF, 0, 0));
            gifStateFlush();
            sceVif1PkCloseDirectHLCode(packet);
            tiWaterSendStateTemplate();
        }
        for (j = 0; j < tiWaterSubList[i]->active; j++) {
            tiWaterDrawItem(waterItem);
            waterItem++;
        }
    }
    tiWaterSubCount = 0;
    xglPacketModeReset();
    xglPacketInitVuStateArg(pass);
    nmlPacketClrTextureCache();
    nmlPacketClrModelCache();
    nmlPacketClrMatrixCache();
}

float atanf(float value);

/* The angle of (x, y) in [0, 2 pi): atan(y / x) moved into the right half turn. */
float tiWaterVectorAngle(float x, float y)
{
    float angle;

    if (x == 0.0f) {
        x = 1e-05f;
    }
    angle = atanf(y / x);
    if (x > 0.0f) {
        if (angle < 0.0f) {
            angle += 6.283184f;
        }
    } else {
        angle += 3.1415927f;
    }
    return angle;
}

#include "main/xenoActor.h"

void tiWatSimDisturbAll(TiWaterSub *table, Vector4 *pPos);
void tiWatSimStoreSourcePos(const Vector4 *position);
extern const float tiWaterSourceMoveScale;
void XenoActorEnableChannel(XenoActor *actor);

void tiWaterUpdateSource(XenoActor *actor)
{
    int i;

    actor->previousPosition.x = actor->position.x;
    actor->previousPosition.z = actor->position.z;
    actor->position.x += PadData[0].leftX * tiWaterSourceMoveScale;
    actor->position.z += PadData[0].leftY * tiWaterSourceMoveScale;
    if (PadData[0].held & PAD_CROSS) {
        XenoActorEnableChannel(actor);
    }
    if (PadData[0].leftX != 0 || PadData[0].leftY != 0) {
        actor->rotation.y = tiWaterVectorAngle(PadData[0].leftY, PadData[0].leftX);
        for (i = 0; i < tiWaterSubCount; i++) {
            tiWatSimDisturbAll(tiWaterSubList[i], &actor->position);
        }
        (void) __builtin_sqrtf(PadData[0].leftY * PadData[0].leftY + PadData[0].leftX * PadData[0].leftX);
    }
    tiWatSimStoreSourcePos(&actor->position);
}

/* Steps the rain and the simulation of every item of the sub-object. */
void tiWaterStepSub(TiWaterSub *waterSub)
{
    TiWatSimItem *waterItem = waterSub->items;
    unsigned int i;

    for (i = 0; i < waterSub->active; i++) {
        tiWatSimStepRain(waterItem);
        tiWatSimUpdate(1.0f, waterItem);
        waterItem++;
    }
}

#define PAD_L2 0x0001
#define PAD_START 0x0800
#define TI_WATER_EXIT (PAD_SELECT | (PAD_START << 16))
#define TI_WATER_FRAMES 120.0f

enum TiWaterRsrc {
    XAct_Rsrc_MDL = 0,
    XAct_Rsrc_TEX = 1,
    XAct_Rsrc_XHR = 2,
    XAct_Rsrc_XAP = 4,
    XAct_Rsrc_XAP_Ext2 = 6
};

extern void *gifStateModeHook[];
extern const char tiWaterAssertLandscapeMessage[];
extern const char tiWaterSourceFileName[];
extern const char tiWaterLandscapeMdlPath[];
extern const char tiWaterLandscapeTexPath[];
extern const char tiWaterAssertLandscapeMdlMessage[];
extern const char tiWaterAssertLandscapeTexMessage[];
extern const char tiWaterAssertKosmosMessage[];
extern const char tiWaterKosmosMdlPath[];
extern const char tiWaterKosmosTexPath[];
extern const char tiWaterKosmosXhrPath[];
extern const char tiWaterKosmosXapPath[];
extern const char tiWaterAssertKosmosMdlMessage[];
extern const char tiWaterAssertKosmosTexMessage[];
extern const char tiWaterAssertKosmosXhrMessage[];
extern const char tiWaterAssertKosmosXapMessage[];
extern const char tiWaterAssertKosmosXapExtSlotMessage[];
extern const DoublewordAlignedVector4 tiWaterModelMulColor;
void assertFailed(const char *message, const char *file, int line);
void xglRenderClearFrame(void);
void xglSleep(void);
void XenoActorReleaseAll(void);
void studioCameraAt(struct XglCamera **camera, int index);
void pxyEnableFlag8(void);
void func_002911C0(void);
void func_002912B0(void);
void XenoActorPoolInitDefault(void);
XenoActor *func_0028D670(void);
int testLoadFile(int *loadAddr, const char *path);
void func_0028D760(XenoActor *actor);
void XenoActorSetChannelValue(XenoActor *actor, float value);
void XenoActorSetMode(XenoActor *actor, int mode);
void nmlModelSetMulColor(const DoublewordAlignedVector4 *color);

void tiWaterSceneLoop(void)
{
    DoublewordAlignedVector4 color;
    int heap;
    struct XglCamera *camera;
    XenoActor *pLandscape;
    XenoActor *pKosmos;

    heap = 0x00e00000;
    XenoActorPoolInitDefault();
    pLandscape = func_0028D670();
    if (pLandscape == 0) {
        assertFailed(tiWaterAssertLandscapeMessage, tiWaterSourceFileName, 569);
    }
    pLandscape->rsrc[XAct_Rsrc_MDL] = testLoadFile(&heap, tiWaterLandscapeMdlPath);
    pLandscape->rsrc[XAct_Rsrc_TEX] = testLoadFile(&heap, tiWaterLandscapeTexPath);
    if (pLandscape->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(tiWaterAssertLandscapeMdlMessage, tiWaterSourceFileName, 572);
    }
    if (pLandscape->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(tiWaterAssertLandscapeTexMessage, tiWaterSourceFileName, 573);
    }
    pKosmos = func_0028D670();
    if (pKosmos == 0) {
        assertFailed(tiWaterAssertKosmosMessage, tiWaterSourceFileName, 576);
    }
    pKosmos->rsrc[XAct_Rsrc_MDL] = testLoadFile(&heap, tiWaterKosmosMdlPath);
    pKosmos->rsrc[XAct_Rsrc_TEX] = testLoadFile(&heap, tiWaterKosmosTexPath);
    pKosmos->rsrc[XAct_Rsrc_XHR] = testLoadFile(&heap, tiWaterKosmosXhrPath);
    pKosmos->rsrc[XAct_Rsrc_XAP] = testLoadFile(&heap, tiWaterKosmosXapPath);
    pKosmos->rsrc[XAct_Rsrc_XAP_Ext2] = testLoadFile(&heap, tiWaterKosmosXapPath);
    if (pKosmos->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(tiWaterAssertKosmosMdlMessage, tiWaterSourceFileName, 587);
    }
    if (pKosmos->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(tiWaterAssertKosmosTexMessage, tiWaterSourceFileName, 588);
    }
    if (pKosmos->rsrc[XAct_Rsrc_XHR] == 0) {
        assertFailed(tiWaterAssertKosmosXhrMessage, tiWaterSourceFileName, 589);
    }
    if (pKosmos->rsrc[XAct_Rsrc_XAP] == 0) {
        assertFailed(tiWaterAssertKosmosXapMessage, tiWaterSourceFileName, 590);
    }
    if (pKosmos->rsrc[XAct_Rsrc_XAP_Ext2] == 0) {
        assertFailed(tiWaterAssertKosmosXapExtSlotMessage, tiWaterSourceFileName, 591);
    }
    pKosmos->drawFlags |= 0x01000000;
    func_0028D760(pKosmos);
    XenoActorSetChannelValue(pKosmos, 8.0f);
    XenoActorSetMode(pKosmos, 0);
    tiWaterDebugReset();
    gifStateSetCenter(tiWaterCx, tiWaterCy);
    gifStateSetEffectAlpha(tiWaterAlphaType, tiWaterAlpha);
    gifStateSetDecrease(tiWaterDecrease);
    gifStateSetFbp(tiWaterFbp);
    gifStateSetBlur(tiWaterBlur);
    color = tiWaterModelMulColor;
    nmlModelSetMulColor(&color);
    while ((PadData[0].heldAndPressed & TI_WATER_EXIT) != TI_WATER_EXIT) {
        xglRenderClearFrame();
        pxyEnableFlag8();
        if (PadData[0].pressed & PAD_L2) {
            if (++tiWaterDebugMode >= 2) {
                tiWaterDebugMode = 0;
            }
        }
        if (tiWaterDebugMode == 1) {
            tiWaterDebugUpdate();
        }
        pLandscape->frame += 1.0f;
        if (TI_WATER_FRAMES <= pLandscape->frame) {
            pLandscape->frame = 0.0f;
        }
        studioCameraAt(&camera, 0);
        func_002911C0();
        func_002912B0();
        tiWaterUpdateSource(pKosmos);
        xglSleep();
    }
    XenoActorReleaseAll();
    xglRenderClearFrame();
    gifStateModeHook[0] = 0;
}

/* Per-object update: the word at +0xC0 of the pxy header (waterFlag) is the offset of its water table; registers the
 * table when it has items, steps it unless isPaused is set, and installs the draw callback tiWaterDrawAll. */
void tiWaterObjectUpdate(PxyModelHeader *pxyModelPack, int isPaused) {
    TiWaterSub *waterSub;

    waterSub = BYTE_OFFSET_AS(TiWaterSub, pxyModelPack, pxyModelPack->waterFlag);
    if (waterSub->active != 0) {
        tiWaterRegisterSub(waterSub);
        if (!isPaused) {
            tiWaterStepSub(waterSub);
        }
        pxyRequestTextureCopyPass();
        pxySetDrawCallback(tiWaterDrawAll);
    }
}
