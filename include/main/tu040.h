#ifndef INCLUDE_MAIN_TU040_H
#define INCLUDE_MAIN_TU040_H

typedef int s32;

typedef unsigned char u8;

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

#endif /* INCLUDE_MAIN_TU040_H */
