#include "common.h"
#include "shared.h"
#include "main/assert_fail.h"
#include "main/tu007.h"
#include "main/xenoActor.h"
#include "main/xgl_2.h"
#define PAD_SELECT 0x0100
#define PAD_START 0x0800

enum CullingTestRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,
    XAct_Rsrc_XAP
};

typedef struct CullingTestPaths {
    const char *mdl;
    const char *tex;
    const char *xhr;
    const char *xap;
} CullingTestPaths;

typedef struct CullingTestPathTable {
    CullingTestPaths actor[2];
} CullingTestPathTable;

extern PadRecord PadData[2];
XenoActor *func_0028D670(void);
int func_0028D760(XenoActor *actor);
void func_0019C538(int rsrc);
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
void xglCullingMapClear(void);
void xglRenderClearFrame(void);
void xglSleep(void);
u32 xglSRand(void);
void setCurrentStudio(int studio);
void studioCameraAt(XglCamera **camera, int index);
void pxySetHeaderScanEnabled(int enabled);
int testLoadFile(int *loadAddr, const char *path);

/* Culling test: loads the test map and two actors, then runs the pad loop until SELECT+START. */
void cullingTestMain(void) {
    CullingTestPathTable actorPathTable = { {
        { "host0:/home/xeno/work/iwase/boy400.pxy", "host0:/home/xeno/work/iwase/boy400.txy",
          "host0:/home/xeno/work/iwase/boy400.xhr", "host0:/home/xeno/work/iwase/kosmos01.xap" },
        { "host0:/home/xeno/work/iwase/kosmos01.pxy", "host0:/home/xeno/work/iwase/kosmos01.txy",
          "host0:/home/xeno/work/iwase/kosmos01.xhr", "host0:/home/xeno/work/iwase/kosmos01.xap" }
    } };
    int loadCursor;
    XglCamera *camera;
    XenoActor *mapActor;
    XenoActor *actors[2];
    CullingTestPaths *actorPaths;
    XenoActor **actorSlot;
    u32 i;
    int holdMode;

    loadCursor = 0xE00000;
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
        assertFailed("pMap", "CullingTest.euc.c", 65);
    }
    mapActor->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, "host0:/home/xeno/work/iwase/anim_test.pxy");
    mapActor->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, "host0:/home/xeno/work/iwase/anim_test.txy");
    if (mapActor->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed("pMap->rsrc[XAct_Rsrc_MDL]", "CullingTest.euc.c", 68);
    }
    if (mapActor->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed("pMap->rsrc[XAct_Rsrc_TEX]", "CullingTest.euc.c", 69);
    }
    func_0028D760(mapActor);

    actorPaths = actorPathTable.actor;
    actorSlot = actors;
    for (i = 0; i < 2; i++) {
        *actorSlot = func_0028D670();
        (*actorSlot)->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, actorPaths->mdl);
        (*actorSlot)->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, actorPaths->tex);
        (*actorSlot)->rsrc[XAct_Rsrc_XHR] = testLoadFile(&loadCursor, actorPaths->xhr);
        (*actorSlot)->rsrc[XAct_Rsrc_XAP] = testLoadFile(&loadCursor, actorPaths->xap);
        if ((*actorSlot)->rsrc[XAct_Rsrc_MDL] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_MDL] != NULL", "CullingTest.euc.c", 81);
        }
        if ((*actorSlot)->rsrc[XAct_Rsrc_TEX] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_TEX] != NULL", "CullingTest.euc.c", 82);
        }
        if ((*actorSlot)->rsrc[XAct_Rsrc_XHR] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_XHR] != NULL", "CullingTest.euc.c", 83);
        }
        if ((*actorSlot)->rsrc[XAct_Rsrc_XAP] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_XAP] != NULL", "CullingTest.euc.c", 84);
        }
        ((DoublewordVector4 *) &(*actorSlot)->position)->vector.x += (float) i;
        func_0028D760(*actorSlot);
        actorSlot++;
        actorPaths++;
    }

    xglCullingMapClear();
    func_0019C538(mapActor->rsrc[XAct_Rsrc_MDL]);
    if ((PadData[0].heldAndPressed & (PAD_SELECT | (PAD_START << 16))) != (PAD_SELECT | (PAD_START << 16))) {
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
    XenoActorReleaseAll();
    xglRenderClearFrame();
}
