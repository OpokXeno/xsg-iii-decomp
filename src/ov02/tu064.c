/*
 * OV02 original TU 64: 0x00a9ba10..0x00a9d888 (16 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9BA10);

typedef int s32;
typedef long long s64;
/* One GIF A+D entry: the data doubleword first, then the GS register address. */
typedef struct GifAdPair {
    s64 gsRegisterValue;
    s64 gsRegister;
} GifAdPair;

extern GifAdPair *gGifAdWriteCursor;
extern s32 gGifAdBufferReady;
extern const char gGifAdAssertFile[];
extern const char gGifAdAssertMessage[];
void assertFailed(const char *message, const char *file, int line);

void gsPacketAddAD(s64 gsRegister, s64 gsRegisterValue) {
    GifAdPair *adPair;

    if (gGifAdBufferReady == 0) {
        assertFailed(gGifAdAssertMessage, gGifAdAssertFile, 0x48);
    }
    adPair = gGifAdWriteCursor;
    adPair->gsRegister = gsRegister;
    adPair->gsRegisterValue = gsRegisterValue;
    gGifAdWriteCursor += 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9BAF8);

void gsPacketAddAD();

void gsPacketResetContextState(void) {
    if (gGifAdBufferReady == 0) {
        assertFailed(gGifAdAssertMessage, gGifAdAssertFile, 0x5D);
    }
    gsPacketAddAD(0x3F, 0);
    gsPacketAddAD(0x42, 0x44);
    gsPacketAddAD(8, 5);
    gsPacketAddAD(0x14, 0);
    gsPacketAddAD(0x4E, 0x31000000);
    gsPacketAddAD(0x47, 0x50000);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9BBF0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9BE10);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9BFE0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9C190);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9C330);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9C600);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9C918);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9CE98);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9D2E0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9D540);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9D5E8);

INCLUDE_ASM("asm/nonmatchings/ov02/tu064", func_00A9D750);
