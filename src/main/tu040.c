#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu040", func_0019E148);

typedef int s32;
typedef signed char s8;
typedef unsigned char u8;
s32 func_0019E148();

void pxyFinishInit(void) {
    func_0019E148();
}

extern unsigned char gPxyModelRenderStateStorage[];

/* Module globals at gPxyModelRenderStateStorage (pxy); offsets as used by the functions below, gaps are unknown. */
struct PxyModelRenderState {
    u8 unk000[0x680];
    s32 packetSlotWriteCursor;
    u8 unk684[0x8];
    s32 unk68C;
    s32 unk690;
    u8 unk694[0x18];
    s32 shadowCasterMode;
    u8 unk6B0[0x28];
    s32 unk6D8;
    s32 translucentEntryCursor;
    u8 unk6E0[0x14];
    s32 unk6F4;
};

/* Typed view of gPxyModelRenderStateStorage as the module globals. */
#define gPxyModelRenderState (*(struct PxyModelRenderState *) gPxyModelRenderStateStorage)

void pxyClearPacketSlots(void) {
    s32 packetSlotNumber;
    s32 *slotPacket;

    gPxyModelRenderState.packetSlotWriteCursor = 0;
    slotPacket = &((s32 *) gPxyModelRenderStateStorage)[0x12B0];
    for (packetSlotNumber = 0; packetSlotNumber < 4; packetSlotNumber++) {
        *slotPacket = 0;
        slotPacket -= 4;
    }
}

void pxyClearPacketSlots();

void pxyInitPacketSlots(void) {
    pxyClearPacketSlots();
}

INCLUDE_ASM("asm/main/nonmatchings/tu040", pxyResetGlobals);

void pxyResetGlobals();

void pxyInitGlobals(void) {
    struct PxyModelRenderState *renderBlock;

    pxyResetGlobals();
    renderBlock = &gPxyModelRenderState;
    renderBlock->shadowCasterMode = 1;
    renderBlock->translucentEntryCursor = 0x1000;
    renderBlock->unk68C = 0;
    renderBlock->unk690 = 0;
    renderBlock->unk6D8 = 0;
    renderBlock->unk6F4 = 0;
}

void pxyInitGlobals();

void pxyInitModelCounters(void) {
    pxyInitGlobals();
}

INCLUDE_ASM("asm/main/nonmatchings/tu040", func_0019E410);

INCLUDE_ASM("asm/main/nonmatchings/tu040", func_0019E480);
