#include "common.h"
#include "main/nml_model_set.h"

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

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E5B0);

typedef int s32;

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
    int stateFlags = gPxyModelRenderStateStorage.texFlags | PXY_TEX_PROREAL;

    gPxyModelRenderStateStorage.texFlags = stateFlags;
    return stateFlags;
}

extern unsigned int D_008C2C90[];

void pxyStateSetWord0(unsigned int word0)
{
    D_008C2C90[0] = word0;
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

void func_001975C8(void *block);

void pxyModelRenderStateInitLight(void) {
    func_001975C8(&gPxyModelRenderStateStorage);
}
