#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B64D8);

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6520);

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6628);

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6798);

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6840);

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6890);

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;

/*
 * Block header that sits 0x40 bytes before the payload handed to these functions.
 * magic is 0xAAAA while the block is live; nextTag links the active list; tagWords[] is
 * indexed by a field number (field N is at header + 0x10 + 4 * N).
 */
typedef struct PoolBlockTag PoolBlockTag;
struct PoolBlockTag {
    u16 magic;                  /* 0x00 */
    u8 unk_02[0xA];             /* 0x02 */
    PoolBlockTag *nextTag;            /* 0x0C */
    s32 tagWords[12];             /* 0x10..0x3F; the payload starts at 0x40 */
};

extern PoolBlockTag *gPoolActiveTagHead;
void func_001B6798(void *);

s32 poolBlockTagWordGet(void *payloadAddr, s32 fieldNumber) {
    PoolBlockTag *blockTag;

    blockTag = payloadAddr;
    blockTag = blockTag - 1;   /* sizeof(PoolBlockTag) == 0x40: the header is one block before the payload */
    if (blockTag->magic == 0xAAAA) {
        return blockTag->tagWords[fieldNumber];
    }
    return -1;
}

s32 poolBlockTagWordSet(void *payloadAddr, s32 fieldNumber, s32 storedWord) {
    PoolBlockTag *blockTag;

    blockTag = payloadAddr;
    blockTag = blockTag - 1;   /* sizeof(PoolBlockTag) == 0x40: the header is one block before the payload */
    if (blockTag->magic == 0xAAAA) {
        blockTag->tagWords[fieldNumber] = storedWord;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6918);

s32 poolReleaseMatching(s32 fieldNumber, s32 expected, s32 mask) {
    s32 releasedCount;
    PoolBlockTag *blockTag;

    blockTag = gPoolActiveTagHead;
    releasedCount = 0;
    if (blockTag != 0) {
        do {
            if ((blockTag->magic == 0xAAAA) && ((blockTag->tagWords[fieldNumber] & mask) == expected)) {
                releasedCount += 1;
                func_001B6798(&blockTag->tagWords[12]);
                blockTag = gPoolActiveTagHead;
            } else {
                blockTag = blockTag->nextTag;
            }
        } while (blockTag != 0);
    }
    return releasedCount;
}

s32 poolReleaseNonMatching(s32 fieldNumber, s32 expected, s32 mask) {
    s32 releasedCount;
    PoolBlockTag *blockTag;

    blockTag = gPoolActiveTagHead;
    releasedCount = 0;
    if (blockTag != 0) {
        do {
            if ((blockTag->magic == 0xAAAA) && ((blockTag->tagWords[fieldNumber] & mask) != expected)) {
                releasedCount += 1;
                func_001B6798(&blockTag->tagWords[12]);
                blockTag = gPoolActiveTagHead;
            } else {
                blockTag = blockTag->nextTag;
            }
        } while (blockTag != 0);
    }
    return releasedCount;
}

INCLUDE_ASM("asm/main/nonmatchings/tu055", func_001B6AB0);
