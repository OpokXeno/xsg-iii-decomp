#include "common.h"

#include "shared.h"
#include "gs.h"
#include "main/xgl_2.h"
#include "main/tu007.h"
#include "main/xenoActor.h"
#include "main/assert_fail.h"
INCLUDE_ASM("asm/main/nonmatchings/ScisTest", func_0024BF58);

void func_0024BF58(void);

enum BlendTestRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,
    XAct_Rsrc_XAP
};

enum BlendTestPath {
    PATH_MDL,
    PATH_TEX,
    PATH_XHR,
    PATH_XAP,
    PATH_COUNT
};

typedef struct BlendTestPathTable {
    const char *file[2][PATH_COUNT];
} BlendTestPathTable;

extern const BlendTestPathTable scisTestActorPathTable;
extern PadRecord PadData[2];
XenoActor *func_0028D670(void);
int func_0028D760(XenoActor *actor);
void func_002911C0(void);
void func_002912B0(void);
void XenoActorPoolInitDefault(void);
void XenoActorReleaseAll(void);
void XenoActorSetHoldFlag(void);
void XenoActorClearHoldFlag(void);
void XenoActorEnableChannel(XenoActor *actor);
void XenoActorSetChannelValue(XenoActor *actor, float value);
void XenoActorSetMode(XenoActor *actor, u32 mode);
void xglStudioReset(void);
void xglStudioMainCameraInit(void);
void xglCameraSetPadPort(XglCamera *camera, int port);
void xglRenderClearFrame(void);
void xglSleep(void);
u32 xglSRand(void);
void setCurrentStudio(int studio);
void studioCameraAt(XglCamera **camera, int index);
void pxySetHeaderScanEnabled(int enabled);
int testLoadFile(int *loadAddr, const char *path);

/* Blend test data: the vector tables read by func_0024BF58 and the path table of scisTestRun. */
const u32 D_0053B440[4] = { 0x80, 0x80, 0x80, 0x80 };
const float D_0053B450[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const float D_0053B460[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const char D_0053B470[48] = "host0:/home/xeno/work/iwase/EP3/C3jr00.pxy";
const char D_0053B4A0[48] = "host0:/home/xeno/work/iwase/EP3/C3jr00.txy";
const char D_0053B4D0[48] = "host0:/home/xeno/work/iwase/EP3/C3jr00.xhr";
const char D_0053B500[48] = "host0:/home/xeno/work/iwase/EP3/C3jr00.xap";
const char D_0053B530[48] = "host0:/home/xeno/work/iwase/EP3/C3ziggy00.pxy";
const char D_0053B560[48] = "host0:/home/xeno/work/iwase/EP3/C3ziggy00.txy";
const char D_0053B590[48] = "host0:/home/xeno/work/iwase/EP3/C3ziggy00.xhr";
const BlendTestPathTable scisTestActorPathTable = {
    {
        { D_0053B470, D_0053B4A0, D_0053B4D0, D_0053B500 },
        { D_0053B530, D_0053B560, D_0053B590, D_0053B500 },
    },
};

/* Blend test scene: loads the map and the two actor sets of actorPathTable, places the actors, then runs the per-frame
 * control loop until the pad holds 0x0100 with 0x0800 pressed (heldAndPressed, low and high halfword). */
void scisTestRun(void) {
    BlendTestPathTable actorPathTable;
    int loadCursor;
    XglCamera *camera;
    XenoActor *mapActor;
    XenoActor *actors[2];
    u32 i;
    int holdMode;

    loadCursor = 0xE00000;
    actorPathTable = scisTestActorPathTable;
    holdMode = 0;
    XenoActorPoolInitDefault();
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera = ((XglCamera * (*) (int camera_index)) xglStudioGetCamera2)(0);
    camera->cameraState = 2;
    xglCameraSetPadPort(camera, 0);

    mapActor = func_0028D670();
    if (mapActor == 0) {
        assertFailed("pMap", "ScisTest.euc.c", 150);
    }
    mapActor->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, "host0:/home/xeno/work/iwase/E2_MOM04_EVA.pxy");
    mapActor->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, "host0:/home/xeno/work/iwase/E2_MOM04_EVA.txy");
    if (mapActor->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed("pMap->rsrc[XAct_Rsrc_MDL]", "ScisTest.euc.c", 153);
    }
    if (mapActor->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed("pMap->rsrc[XAct_Rsrc_TEX]", "ScisTest.euc.c", 154);
    }

    for (i = 0; i < 2; i++) {
        actors[i] = func_0028D670();
        actors[i]->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_MDL]);
        actors[i]->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_TEX]);
        actors[i]->rsrc[XAct_Rsrc_XHR] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_XHR]);
        actors[i]->rsrc[XAct_Rsrc_XAP] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_XAP]);
        if (actors[i]->rsrc[XAct_Rsrc_MDL] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_MDL] != NULL", "ScisTest.euc.c", 165);
        }
        if (actors[i]->rsrc[XAct_Rsrc_TEX] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_TEX] != NULL", "ScisTest.euc.c", 166);
        }
        if (actors[i]->rsrc[XAct_Rsrc_XHR] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_XHR] != NULL", "ScisTest.euc.c", 167);
        }
        if (actors[i]->rsrc[XAct_Rsrc_XAP] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_XAP] != NULL", "ScisTest.euc.c", 168);
        }
        ((DoublewordVector4 *) &actors[i]->position)->vector.x += (float) i;
        ((DoublewordVector4 *) &actors[i]->position)->vector.z -= 1.0f;
        func_0028D760(actors[i]);
    }

    if ((PadData[0].heldAndPressed & (0x0100 | (0x0800 << 16))) != (0x0100 | (0x0800 << 16))) {
        do {
            studioCameraAt(&camera, 0);
            pxySetHeaderScanEnabled(1);
            if (PadData[0].repeated & 0x10) {
                OffsetTable *xapModeTable = actors[0]->rsrc[XAct_Rsrc_XAP];
                XenoActorSetChannelValue(actors[0], 10.0f);
                XenoActorEnableChannel(actors[0]);
                XenoActorSetMode(actors[0], xglSRand() % xapModeTable->count);
            }
            if (PadData[0].repeated & 0x40) {
                OffsetTable *xapModeTable = actors[1]->rsrc[XAct_Rsrc_XAP];
                XenoActorSetChannelValue(actors[1], 20.0f);
                XenoActorEnableChannel(actors[1]);
                XenoActorSetMode(actors[1], xglSRand() % xapModeTable->count);
            }
            func_0024BF58();
            if (PadData[0].pressed & 0x1000) {
                holdMode = holdMode != 1;
            }
            if (PadData[0].repeated & 0x4000) {
                holdMode = 2;
            }
            if (holdMode != 1) {
                func_002911C0();
            }
            if (holdMode == 1) {
                XenoActorSetHoldFlag();
            } else {
                XenoActorClearHoldFlag();
            }
            if (holdMode == 2) {
                holdMode = 1;
            }
            func_002912B0();
            xglSleep();
        } while ((PadData[0].heldAndPressed & (0x0100 | (0x0800 << 16))) != (0x0100 | (0x0800 << 16)));
    }
    XenoActorReleaseAll();
    xglRenderClearFrame();
}

const unsigned char D_0053B740[16] = { 0x0d, 0x02, 0x49, 0x6e, 0x74, 0x56, 0x61, 0x6c, 0x34, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B750[16] = { 0x0d, 0x02, 0x49, 0x6e, 0x74, 0x56, 0x61, 0x6c, 0x33, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B760[16] = { 0x0d, 0x02, 0x49, 0x6e, 0x74, 0x56, 0x61, 0x6c, 0x32, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B770[16] = { 0x0d, 0x02, 0x49, 0x6e, 0x74, 0x56, 0x61, 0x6c, 0x31, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B780[16] = { 0x0d, 0x02, 0x44, 0x65, 0x63, 0x72, 0x65, 0x61, 0x73, 0x65, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B790[16] = { 0x0d, 0x02, 0x42, 0x6c, 0x75, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B7A0[16] = { 0x0d, 0x02, 0x46, 0x42, 0x50, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B7B0[16] = { 0x0d, 0x02, 0x41, 0x6c, 0x70, 0x68, 0x61, 0x20, 0x20, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B7C0[16] = { 0x0d, 0x02, 0x41, 0x6c, 0x70, 0x68, 0x61, 0x54, 0x79, 0x70, 0x65, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B7D0[16] = { 0x0d, 0x02, 0x63, 0x79, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B7E0[16] = { 0x0d, 0x02, 0x63, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x25, 0x66, 0x00, 0x00 };
const unsigned char D_0053B7F0[16] = { 0x0d, 0x02, 0x45, 0x66, 0x66, 0x65, 0x63, 0x74, 0x4e, 0x75, 0x6d, 0x20, 0x25, 0x66, 0x00, 0x00 };
