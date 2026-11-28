#include "common.h"
#include "shared.h"
#include "main/assert_fail.h"
#include "main/tu007.h"
#include "main/xenoActor.h"
#include "main/xgl_2.h"
#define PAD_SELECT 0x0100
#define PAD_START 0x0800

enum XstAnimTestRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX,
    XAct_Rsrc_XHR,
    XAct_Rsrc_3,
    XAct_Rsrc_XST = 14
};

typedef struct XstAnimTestPaths {
    const char *mdl;
    const char *tex;
    const char *xhr;
    const char *xst;
} XstAnimTestPaths;

typedef struct XstAnimTestPathTable {
    XstAnimTestPaths actor[2];
} XstAnimTestPathTable;

const XstAnimTestPathTable gXstAnimTestActorPaths = {
    {
        { "host0:/home/xeno/work/iwase/xst_test/gaignunH01.pxy",
          "host0:/home/xeno/work/iwase/xst_test/gaignunH01.txy",
          "host0:/home/xeno/work/iwase/xst_test/gaignunH01.xhr",
          "host0:/home/xeno/work/iwase/xst_test/gaignunH01.xst" },
        { "host0:/home/xeno/work/iwase/xst_test/jrH01.pxy",
          "host0:/home/xeno/work/iwase/xst_test/jrH01.txy",
          "host0:/home/xeno/work/iwase/xst_test/jrH01.xhr",
          "host0:/home/xeno/work/iwase/xst_test/jrH01.xst" },
    }
};
extern PadRecord PadData[2];
XenoActor *func_0028D670(void);
int func_0028D760(XenoActor *actor);
void func_002911C0(void);
void func_002912B0(void);
void XenoActorPoolInitDefault(void);
void XenoActorReleaseAll(void);
void XenoActorSetHoldFlag(void);
void XenoActorClearHoldFlag(void);
void xglStudioReset(void);
void xglStudioMainCameraInit(void);
void xglCameraSetPadPort(XglCamera *camera, int port);
void xglRenderClearFrame(void);
void xglSleep(void);
void setCurrentStudio(int studio);
void studioCameraAt(XglCamera **camera, int index);
void pxySetHeaderScanEnabled(int enabled);
int testLoadFile(int *loadAddr, const char *path);

/* Test loop for the XstAnimTest scene: loads the map and two actors (gaignun, jr) from host0 paths,
   then runs until SELECT and START are held together. Pad bits 0x10/0x80 step frame up/down,
   0x1000 toggles hold mode, 0x4000 sets hold mode 2 (which ends as 1). */
void XstAnimTestMain(void) {
    XstAnimTestPathTable actorPathTable = gXstAnimTestActorPaths;
    int loadCursor;
    XglCamera *camera;
    XenoActor *mapActor;
    XenoActor *actors[2];
    XstAnimTestPaths *actorPaths;
    u32 i;
    int holdMode;
    int frame;

    loadCursor = 0xE00000;
    frame = 0;
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
        assertFailed("pMap", "XstAnimTest.euc.c", 52);
    }
    mapActor->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, "host0:/home/xeno/work/iwase/E2_MOM04_EVA.pxy");
    mapActor->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, "host0:/home/xeno/work/iwase/E2_MOM04_EVA.txy");
    if (mapActor->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed("pMap->rsrc[XAct_Rsrc_MDL]", "XstAnimTest.euc.c", 55);
    }
    if (mapActor->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed("pMap->rsrc[XAct_Rsrc_TEX]", "XstAnimTest.euc.c", 56);
    }

    for (i = 0; i < 2; i++) {
        actors[i] = func_0028D670();
        actorPaths = &actorPathTable.actor[i];
        actors[i]->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadCursor, actorPaths->mdl);
        actors[i]->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadCursor, actorPaths->tex);
        actors[i]->rsrc[XAct_Rsrc_XHR] = testLoadFile(&loadCursor, actorPaths->xhr);
        actors[i]->rsrc[XAct_Rsrc_XST] = testLoadFile(&loadCursor, actorPaths->xst);
        if (actors[i]->rsrc[XAct_Rsrc_MDL] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_MDL] != NULL", "XstAnimTest.euc.c", 67);
        }
        if (actors[i]->rsrc[XAct_Rsrc_TEX] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_TEX] != NULL", "XstAnimTest.euc.c", 68);
        }
        if (actors[i]->rsrc[XAct_Rsrc_XHR] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_XHR] != NULL", "XstAnimTest.euc.c", 69);
        }
        if (actors[i]->rsrc[XAct_Rsrc_XST] == 0) {
            assertFailed("pAct[i]->rsrc[XAct_Rsrc_XST] != NULL", "XstAnimTest.euc.c", 70);
        }
        ((DoublewordVector4 *) &actors[i]->position)->vector.x += (float) i;
        func_0028D760(actors[i]);
    }

    if ((PadData[0].heldAndPressed & (PAD_SELECT | (PAD_START << 16))) != (PAD_SELECT | (PAD_START << 16))) {
        do {
            studioCameraAt(&camera, 0);
            pxySetHeaderScanEnabled(1);
            if (PadData[0].held & 0x80) {
                frame--;
            }
            if (PadData[0].held & 0x10) {
                frame++;
            }
            for (i = 0; i < 2; i++) {
                actors[i]->frame = (float) frame;
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
