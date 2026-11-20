#include "common.h"
#include "shared.h"
#include "main/Packet.h"
#include "main/TiWatSim.h"
#include "main/tu007.h"
#include "main/tu045.h"
#include "main/xgl_2.h"
#include "main/xgl_render.h"

/* Row-major 4x4 float matrix. */
struct PxyHeader;
void tiWaterObjectUpdate(struct PxyHeader *header, int stepMode);

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
    ((u64)(tbp) | ((u64)(tbw) << 14) | ((u64)(psm) << 20) | ((u64)(tw) << 26) | ((u64)(th) << 30) \
     | ((u64)(tcc) << 34) | ((u64)(tfx) << 35) | ((u64)(cbp) << 37) | ((u64)(cpsm) << 51) \
     | ((u64)(csm) << 55) | ((u64)(csa) << 56) | ((u64)(cld) << 61))
#define GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
    ((u64)(lcm) | ((u64)(mxl) << 2) | ((u64)(mmag) << 5) | ((u64)(mmin) << 6) | ((u64)(mtba) << 9) \
     | ((u64)(l) << 19) | ((u64)(k) << 32))
#define GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
    ((u64)(wms) | ((u64)(wmt) << 2) | ((u64)(minu) << 4) | ((u64)(maxu) << 14) | ((u64)(minv) << 24) \
     | ((u64)(maxv) << 34))
#define GS_SET_ALPHA(a, b, c, d, fix) \
    ((u64)(a) | ((u64)(b) << 2) | ((u64)(c) << 4) | ((u64)(d) << 6) | ((u64)(fix) << 32))
#define GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
    ((u64)(ate) | ((u64)(atst) << 1) | ((u64)(aref) << 4) | ((u64)(afail) << 12) | ((u64)(date) << 14) \
     | ((u64)(datm) << 15) | ((u64)(zte) << 16) | ((u64)(ztst) << 17))
#define GS_SET_FRAME(fbp, fbw, psm, fbmsk) \
    ((u64)(fbp) | ((u64)(fbw) << 16) | ((u64)(psm) << 24) | ((u64)(fbmsk) << 32))
#define GS_SET_ZBUF(zbp, psm, zmsk) ((u64)(zbp) | ((u64)(psm) << 24) | ((u64)(zmsk) << 32))
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
void tiWatSimStepRain(TiWatSimItem *item);
void tiWatSimRandomizeItem(TiWatSimItem *item, int range);
void tiWatSimUpdate(float step, TiWatSimItem *item);
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

int tiWaterRegisterSub(TiWaterSub *sub);
/* The water state template sent by tiWaterSendStateTemplate, and its length for sceVif1PkAddDataN. */
extern u64 tiWaterStateTemplate[];

#define TI_WATER_STATE_TEMPLATE_COUNT 0x3C
/* The two words tiWaterSendStateTemplate patches: the data of the FRAME_1 and TEX0_1 A+D pairs. */
#define TI_WATER_TEMPLATE_FRAME_1 (0x60 / 8)
#define TI_WATER_TEMPLATE_TEX0_1 (0x70 / 8)

void tiWaterSendStateTemplate(void);
void tiWaterSendGrid(TiWatSimItem *item, void *program);
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

/* Draws one water item: sends its frame, values and direction, then for each enabled pass the GS state * (clamp, frame on the display page, blend, the frame page as texture, Z test) and the VU1 program with * the item's grid. */ void tiWaterDrawItem(TiWatSimItem *item);
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

INCLUDE_ASM("asm/main/nonmatchings/TiWater", tiWaterRegisterSub);

INCLUDE_ASM("asm/main/nonmatchings/TiWater", tiWaterSendStateTemplate);

/* Scaffold data: the render state u16 words (flip base at +0x0E, display base at +0x16), the two grid headers of
 * the passes and the depth program flag (gp-relative). */
extern u16 D_004C8390[];
extern unsigned char D_003EC020[];
extern unsigned char D_003EC030[];
extern int D_00550E2C;

/* Draws one water item: sends its frame, values and direction, then for each enabled pass the GS state
 * (clamp, frame on the display page, blend, the frame page as texture, Z test) and the VU1 program with
 * the item's grid. */
void tiWaterDrawItem(TiWatSimItem *item)
{
    XglPacket *packet = xglPacketGetCurrent();
    XglCamera *camera = xglStudioSelectGetActiveCamera(0);

    tiWaterSendFrame(camera, item->transform.rows);
    tiWaterSendItemPair(item->drawPair[0], item->drawPair[1]);
    tiWaterSendDirection(camera, item->transform.rows);
    if (item->flags & TI_WATER_ITEM_DRAW_SURFACE) {
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
        tiWaterSendGrid(item, D_003EC020);
    }
    if (item->flags & TI_WATER_ITEM_DRAW_DEPTH) {
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
        if (D_00550E2C != 0) {
            sceVif1PkAddCode(packet, TI_WATER_VIF_MSCAL(D_7F0));
        } else {
            sceVif1PkAddCode(packet, TI_WATER_VIF_MSCAL(D_9F0));
        }
        tiWaterSendGrid(item, D_003EC030);
    }
}

/* Draw callback: in pass 0, draws every registered table (CLAMP_1 and the state template first when
 * it has items, then each item) and empties the list; the packet state is reset afterwards. */
void tiWaterDrawAll(int pass)
{
    XglPacket *packet = xglPacketGetCurrent();
    unsigned int i;
    unsigned int j;
    TiWatSimItem *item;

    if (pass != 0) {
        return;
    }
    for (i = 0; i < tiWaterSubCount; i++) {
        item = tiWaterSubList[i]->items;
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
            tiWaterDrawItem(item);
            item++;
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
extern const float D_0054F824;
void XenoActorEnableChannel(XenoActor *actor);

void tiWaterUpdateSource(XenoActor *actor)
{
    int i;

    actor->previousPosition.x = actor->position.x;
    actor->previousPosition.z = actor->position.z;
    actor->position.x += PadData[0].leftX * D_0054F824;
    actor->position.z += PadData[0].leftY * D_0054F824;
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
void tiWaterStepSub(TiWaterSub *sub)
{
    TiWatSimItem *item = sub->items;
    unsigned int i;

    for (i = 0; i < sub->active; i++) {
        tiWatSimStepRain(item);
        tiWatSimUpdate(1.0f, item);
        item++;
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

extern void *D_004C83D8[];
extern const char D_0053B818[];
extern const char D_0053B828[];
extern const char D_0053B838[];
extern const char D_0053B868[];
extern const char D_0053B898[];
extern const char D_0053B8B8[];
extern const char D_0053B8D8[];
extern const char D_0053B8E0[];
extern const char D_0053B908[];
extern const char D_0053B930[];
extern const char D_0053B958[];
extern const char D_0053B988[];
extern const char D_0053B9A8[];
extern const char D_0053B9C8[];
extern const char D_0053B9E8[];
extern const char D_0053BA08[];
extern const DoublewordAlignedVector4 D_0053BA30;
void assertFailed(const char *message, const char *file, int line);
void xglRenderClearFrame(void);
void xglSleep(void);
void XenoActorReleaseAll(void);
void func_0019B708(struct XglCamera **camera, int index);
void func_0019EAF0(void);
void func_002911C0(void);
void func_002912B0(void);
void func_0028D4B0(void);
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
    func_0028D4B0();
    pLandscape = func_0028D670();
    if (pLandscape == 0) {
        assertFailed(D_0053B818, D_0053B828, 569);
    }
    pLandscape->rsrc[XAct_Rsrc_MDL] = testLoadFile(&heap, D_0053B838);
    pLandscape->rsrc[XAct_Rsrc_TEX] = testLoadFile(&heap, D_0053B868);
    if (pLandscape->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(D_0053B898, D_0053B828, 572);
    }
    if (pLandscape->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(D_0053B8B8, D_0053B828, 573);
    }
    pKosmos = func_0028D670();
    if (pKosmos == 0) {
        assertFailed(D_0053B8D8, D_0053B828, 576);
    }
    pKosmos->rsrc[XAct_Rsrc_MDL] = testLoadFile(&heap, D_0053B8E0);
    pKosmos->rsrc[XAct_Rsrc_TEX] = testLoadFile(&heap, D_0053B908);
    pKosmos->rsrc[XAct_Rsrc_XHR] = testLoadFile(&heap, D_0053B930);
    pKosmos->rsrc[XAct_Rsrc_XAP] = testLoadFile(&heap, D_0053B958);
    pKosmos->rsrc[XAct_Rsrc_XAP_Ext2] = testLoadFile(&heap, D_0053B958);
    if (pKosmos->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(D_0053B988, D_0053B828, 587);
    }
    if (pKosmos->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(D_0053B9A8, D_0053B828, 588);
    }
    if (pKosmos->rsrc[XAct_Rsrc_XHR] == 0) {
        assertFailed(D_0053B9C8, D_0053B828, 589);
    }
    if (pKosmos->rsrc[XAct_Rsrc_XAP] == 0) {
        assertFailed(D_0053B9E8, D_0053B828, 590);
    }
    if (pKosmos->rsrc[XAct_Rsrc_XAP_Ext2] == 0) {
        assertFailed(D_0053BA08, D_0053B828, 591);
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
    color = D_0053BA30;
    nmlModelSetMulColor(&color);
    while ((PadData[0].heldAndPressed & TI_WATER_EXIT) != TI_WATER_EXIT) {
        xglRenderClearFrame();
        func_0019EAF0();
        if ((unsigned short) (PadData[0].pressed & PAD_L2)) {
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
        func_0019B708(&camera, 0);
        func_002911C0();
        func_002912B0();
        tiWaterUpdateSource(pKosmos);
        xglSleep();
    }
    XenoActorReleaseAll();
    xglRenderClearFrame();
    D_004C83D8[0] = 0;
}

/* Per-object update: the word at +0xC0 of the pxy header (waterFlag) is the offset of its water table; registers the
 * table when it has items, steps it when stepMode is 0, and installs the draw callback tiWaterDrawAll. */
void tiWaterObjectUpdate(PxyHeader *header, int stepMode) {
    TiWaterSub *sub;

    sub = (TiWaterSub *) ((char *) header + header->waterFlag);
    if (sub->active != 0) {
        tiWaterRegisterSub(sub);
        if (stepMode == 0) {
            tiWaterStepSub(sub);
        }
        pxyRequestTextureCopyPass();
        pxySetDrawCallback(tiWaterDrawAll);
    }
}
