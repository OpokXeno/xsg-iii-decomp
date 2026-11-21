/*
 * OV02 original TU 34: 0x00a8d0f8..0x00a8d218 (5 functions)
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/ov02/tu034", func_00A8D0F8);

typedef int s32;
s32 seVoiceDisable();

s32 sbNativeSeVoiceStop(void) {
    seVoiceDisable();
    return 1;
}

typedef struct ValidValueBlock {
    unsigned char unk00[0x18];
    s32 validFlags;
    unsigned char unk1C[0x8];
    s32 validValue;
} ValidValueBlock;

extern ValidValueBlock gRoboValidValueBlock;

s32 roboValidValueGet(void) {
    if (!(gRoboValidValueBlock.validFlags & 0x20)) {
        return -1;
    }
    return gRoboValidValueBlock.validValue;
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu034", func_00A8D188);

s32 seVoiceSetCurrent(s32);

void seVoiceClearCurrent(void) {
    seVoiceSetCurrent(0);
}
