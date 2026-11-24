#include "common.h"

#include "shared.h"
#include "main/assert_fail.h"
#include "main/tu007.h"
#include "main/xenoActor.h"
#include "main/xgl_2.h"

int runWithStateBlockOnce(int mode);

void blendModeClear(void) {
    runWithStateBlockOnce(0);
}

void blendNoopStart(void) {
}

void blendNoopStop(void) {
}

#define PAD_SELECT 0x0100
#define PAD_START 0x0800

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

extern BlendTestPathTable D_00539F48;
extern PadRecord PadData[2];
extern const char D_00539F68[];
extern const char D_00539F70[];
extern const char D_00539F80[];
extern const char D_00539FB0[];
extern const char D_00539FE0[];
extern const char D_0053A000[];
extern const char D_0053A020[];
extern const char D_0053A048[];
extern const char D_0053A070[];
extern const char D_0053A098[];
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
void xglCameraSetWindow(XglCamera *camera, int left, int top, int right, int bottom);
XglCamera *xglStudioSelectGetActiveCamera(int studio_index);
void xglRenderClearFrame(void);
void xglSleep(void);
u32 xglSRand(void);
void setCurrentStudio(int studio);
void studioCameraAt(XglCamera **camera, int index);
void pxySetHeaderScanEnabled(int enabled);
int testLoadFile(int *loadAddr, const char *path);

/* Blend test: loads the test map and two actors, then runs the pad loop until SELECT+START. */
void blendTestMain(void) {
    BlendTestPathTable actorPathTable;
    int loadCursor;
    XglCamera *camera;
    XenoActor *mapActor;
    XenoActor *actors[2];
    u32 i;
    int holdMode;

    loadCursor = 0xE00000;
    actorPathTable = D_00539F48;
    holdMode = 0;
    XenoActorPoolInitDefault();
    xglStudioReset();
    setCurrentStudio(0);
    xglStudioMainCameraInit();
    camera = xglStudioGetCamera2(0);
    camera->cameraState = 2;
    xglCameraSetPadPort(camera, 0);

    mapActor = func_0028D670();
    if (mapActor == 0) {
        assertFailed(D_00539F68, D_00539F70, 62);
    }
    mapActor->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, D_00539F80);
    mapActor->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, D_00539FB0);
    if (mapActor->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(D_00539FE0, D_00539F70, 65);
    }
    if (mapActor->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(D_0053A000, D_00539F70, 66);
    }

    for (i = 0; i < 2; i++) {
        actors[i] = func_0028D670();
        actors[i]->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_MDL]);
        actors[i]->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_TEX]);
        actors[i]->rsrc[XAct_Rsrc_XHR] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_XHR]);
        actors[i]->rsrc[XAct_Rsrc_XAP] = testLoadFile(&loadCursor, actorPathTable.file[i][PATH_XAP]);
        if (actors[i]->rsrc[XAct_Rsrc_MDL] == 0) {
            assertFailed(D_0053A020, D_00539F70, 77);
        }
        if (actors[i]->rsrc[XAct_Rsrc_TEX] == 0) {
            assertFailed(D_0053A048, D_00539F70, 78);
        }
        if (actors[i]->rsrc[XAct_Rsrc_XHR] == 0) {
            assertFailed(D_0053A070, D_00539F70, 79);
        }
        if (actors[i]->rsrc[XAct_Rsrc_XAP] == 0) {
            assertFailed(D_0053A098, D_00539F70, 80);
        }
        ((DoublewordVector4 *) &actors[i]->position)->vector.x += (float) i;
        func_0028D760(actors[i]);
    }

    if ((PadData[0].heldAndPressed & (PAD_SELECT | (PAD_START << 16))) != (PAD_SELECT | (PAD_START << 16))) {
        do {
            studioCameraAt(&camera, 0);
            pxySetHeaderScanEnabled(1);
            if (PadData[0].held & 0x8000) {
                xglCameraSetWindow(xglStudioSelectGetActiveCamera(0), 0, 0, 512, 448);
            } else {
                xglCameraSetWindow(xglStudioSelectGetActiveCamera(0), 0, 28, 512, 348);
            }
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
        } while ((PadData[0].heldAndPressed & (PAD_SELECT | (PAD_START << 16))) !=
                 (PAD_SELECT | (PAD_START << 16)));
    }
    xglCameraSetWindow(xglStudioSelectGetActiveCamera(0), 0, 0, 512, 448);
    XenoActorReleaseAll();
    xglRenderClearFrame();
}
