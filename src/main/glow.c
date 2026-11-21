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
extern void *gifStateModeHook[];
extern const char glowAssertLandscapeActorMessage[];
extern const char glowAssertFileName[];
extern const char glowLandscapeModelPath[];
extern const char glowLandscapeTexturePath[];
extern const char glowAssertLandscapeModelMessage[];
extern const char glowAssertLandscapeTextureMessage[];
extern const char glowAssertChaosActorMessage[];
extern const char glowChaosModelPath[];
extern const char glowChaosTexturePath[];
extern const char glowAssertChaosModelMessage[];
extern const char glowAssertChaosTextureMessage[];
struct XglCamera;
float xglAtan2Angle(float x, float y);
void assertFailed(const char *message, const char *file, int line);
void xglRenderClearFrame(void);
void xglSleep(void);
void XenoActorReleaseAll(void);
void studioCameraAt(struct XglCamera **camera, int index);
void pxyEnableFlag8(void);
void func_002911C0(void);
void func_002912B0(void);
void XenoActorPoolInitDefault(void);
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

void glowUpdateEmpty(void) {

}

void glowRunScene(void)
{
    struct XenoActor *pLandscape;
    int loadAddr;
    struct XglCamera *camera;
    struct XenoActor *pChaos;

    loadAddr = 0x00e00000;
    XenoActorPoolInitDefault();
    pLandscape = func_0028D670();
    if (pLandscape == 0) {
        assertFailed(glowAssertLandscapeActorMessage, glowAssertFileName, 167);
    }
    pLandscape->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadAddr, glowLandscapeModelPath);
    pLandscape->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadAddr, glowLandscapeTexturePath);
    if (pLandscape->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(glowAssertLandscapeModelMessage, glowAssertFileName, 170);
    }
    if (pLandscape->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(glowAssertLandscapeTextureMessage, glowAssertFileName, 171);
    }

    pChaos = func_0028D670();
    if (pChaos == 0) {
        assertFailed(glowAssertChaosActorMessage, glowAssertFileName, 174);
    }
    pChaos->rsrc[XAct_Rsrc_MDL] = testLoadFile(&loadAddr, glowChaosModelPath);
    pChaos->rsrc[XAct_Rsrc_TEX] = testLoadFile(&loadAddr, glowChaosTexturePath);
    if (pChaos->rsrc[XAct_Rsrc_MDL] == 0) {
        assertFailed(glowAssertChaosModelMessage, glowAssertFileName, 177);
    }
    if (pChaos->rsrc[XAct_Rsrc_TEX] == 0) {
        assertFailed(glowAssertChaosTextureMessage, glowAssertFileName, 178);
    }
    pChaos->position.x = 2.0f;

    while ((PadData[0].heldAndPressed & (PAD_SELECT | (PAD_START << 16))) != (PAD_SELECT | (PAD_START << 16))) {
        xglRenderClearFrame();
        glowUpdateFromPad(pChaos);
        studioCameraAt(&camera, 0);
        pxyEnableFlag8();
        func_002911C0();
        func_002912B0();
        xglSleep();
    }
    XenoActorReleaseAll();
    xglRenderClearFrame();
    gifStateModeHook[0] = 0;
}
