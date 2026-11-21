#include "common.h"

extern unsigned char *gBufferCursor;
extern unsigned char *gBufferArenaStart;

void bufferCursorClear(void) {
    gBufferArenaStart = 0;
    gBufferCursor = 0;
}

extern unsigned char gBufferArena[];

void bufferPointersRewind(void)
{
    gBufferCursor = gBufferArena;
    gBufferArenaStart = gBufferArena;
}

INCLUDE_ASM("asm/main/nonmatchings/tu158", func_00213700);
