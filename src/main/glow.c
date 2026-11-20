#include "common.h"
#include "shared.h"
#define PAD_UP 0x1000
#define PAD_DOWN 0x4000
#define PAD_SELECT 0x0100
#define PAD_START 0x0800

enum GlowRsrc {
    XAct_Rsrc_MDL,
    XAct_Rsrc_TEX
};

#include "main/tu007.h"
#include "main/xenoActor.h"


extern struct PadRecord PadData[2];
extern void *D_004C83D8[];
extern const char D_0053BD60[];
extern const char D_0053BD70[];
extern const char D_0053BD80[];
extern const char D_0053BDA8[];
extern const char D_0053BDD0[];
extern const char D_0053BDF0[];
extern const char D_0053BE10[];
extern const char D_0053BE18[];
extern const char D_0053BE48[];
extern const char D_0053BE78[];
extern const char D_0053BE98[];
struct XglCamera;
float xglAtan2Angle(float x, float y);
void assertFailed(const char *message, const char *file, int line);
void xglRenderClearFrame(void);
void xglSleep(void);
void XenoActorReleaseAll(void);
void func_0019B708(struct XglCamera **camera, int index);
void func_0019EAF0(void);
void func_002911C0(void);
void func_002912B0(void);
void func_0028D4B0(void);
struct XenoActor *func_0028D670(void);
int testLoadFile(int *loadAddr, const char *path);

void glowUpdateFromPad(struct XenoActor *actor)
{
    actor->position.x += PadData[0].leftX * 0.001f;
    actor->position.z += PadData[0].leftY * 0.001f;
    if (PadData[0].held & PAD_UP) {
        actor->position.y += 0.1f;
    }
    if (PadData[0].held & PAD_DOWN) {
        actor->position.y -= 0.1f;
    }
    if (PadData[0].leftX != 0 || PadData[0].leftY != 0) {
        actor->rotation.y = xglAtan2Angle(PadData[0].leftY, PadData[0].leftX);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/glow", func_00258850);

void glowRunScene(void)
{
    struct XenoActor *pLandscape;
    int loadAddr;
    struct XglCamera *camera;
    struct XenoActor *pChaos;

    loadAddr = 0x00e00000;
    func_0028D4B0();
    pLandscape = func_0028D670();
    if (pLandscape == 0) {
        assertFailed(D_0053BD60, D_0053BD70, 167);
    }
    pLandscape->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadAddr, D_0053BD80);
    pLandscape->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadAddr, D_0053BDA8);
    if (pLandscape->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(D_0053BDD0, D_0053BD70, 170);
    }
    if (pLandscape->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(D_0053BDF0, D_0053BD70, 171);
    }

    pChaos = func_0028D670();
    if (pChaos == 0) {
        assertFailed(D_0053BE10, D_0053BD70, 174);
    }
    pChaos->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadAddr, D_0053BE18);
    pChaos->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadAddr, D_0053BE48);
    if (pChaos->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(D_0053BE78, D_0053BD70, 177);
    }
    if (pChaos->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(D_0053BE98, D_0053BD70, 178);
    }
    pChaos->position.x = 2.0f;

    while ((PadData[0].heldAndPressed & (PAD_SELECT | (PAD_START << 16))) != (PAD_SELECT | (PAD_START << 16))) {
        xglRenderClearFrame();
        glowUpdateFromPad(pChaos);
        func_0019B708(&camera, 0);
        func_0019EAF0();
        func_002911C0();
        func_002912B0();
        xglSleep();
    }
    XenoActorReleaseAll();
    xglRenderClearFrame();
    D_004C83D8[0] = 0;
}
