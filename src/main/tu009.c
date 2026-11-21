#include "common.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;

typedef struct FlagBlock {
    u8 unk_00[0x2C];
    u16 flags2C;
    u16 flags2E;
    u16 flags30;
    u16 flags32;
} FlagBlock;

s32 flagsHaveBits2E(FlagBlock *flagBlock, s32 mask) {
    return (~flagBlock->flags2E & mask) == 0;
}

s32 flagsHaveBits2C(FlagBlock *flagBlock, s32 mask) {
    return (~flagBlock->flags2C & mask) == 0;
}

s32 flagsHaveBits32(FlagBlock *flagBlock, s32 mask) {
    return (~flagBlock->flags32 & mask) == 0;
}

s32 flagsHaveBits30(FlagBlock *flagBlock, s32 mask) {
    return (~flagBlock->flags30 & mask) == 0;
}
