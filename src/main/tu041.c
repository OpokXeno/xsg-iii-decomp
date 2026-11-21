#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E510);

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

INCLUDE_ASM("asm/main/nonmatchings/tu041", func_0019E7F0);

extern unsigned int D_008C2C90[];

void pxyStateSetWord0(unsigned int word0)
{
    D_008C2C90[0] = word0;
}

typedef struct PxyModelRenderState {
    unsigned char unmodeled_000[0x300];
    unsigned int renderStatus; /* +0x300 */
} PxyModelRenderState;

extern PxyModelRenderState gPxyModelRenderStateStorage;

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
