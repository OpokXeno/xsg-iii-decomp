#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272650);

typedef signed char s8;

extern s8 gTextMovieMode;

void textDrawModeSet(s8 movieMode) {
    gTextMovieMode = movieMode;
}

extern s8 gTextSingletonIdDraw;

void textDrawFlagSet(s8 enabled) {
    gTextSingletonIdDraw = enabled;
}

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272680);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272A10);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272B38);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272CD8);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272D18);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272D88);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272DD8);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272E28);

typedef unsigned char u8;

typedef int s32;

typedef unsigned int u32;

/* One 8-byte packet entry: a tag byte, then a 32-bit value (see packetEntryAppend). */
typedef struct PacketEntry {
    u8 tag;
    u8 unk01[3];
    s32 packetWord;
} PacketEntry;

/* 0x10-byte span buffer used by packetSpanUpdate: the span starts at +5. */
typedef struct PacketSpanBuffer {
    u8 unk00[2];
    u8 windowMode;
    u8 unk03;
    u8 unk04;
    u8 windowBytes[7];
} PacketSpanBuffer;

extern PacketEntry *packetWriteCursor;

void packetBufferSetCurrent(PacketEntry *writeCursor) {
    packetWriteCursor = writeCursor;
    if (writeCursor != 0) {
        writeCursor->tag = 0;
    }
}

u32 func_00273308(u32 position, s32 mode, PacketSpanBuffer *buffer, s32 advance);

void *memcpy(void *destination, void *source, unsigned int size);

void *memset(void *destination, int value, unsigned int count);

u32 packetSpanUpdate(u32 position, s32 windowMode, PacketSpanBuffer *window, s32 advance) {
    s32 windowShift;
    u8 *windowStart;
    s32 spanLength;
    u32 limit;
    u32 startPosition;

    windowStart = window->windowBytes;
    startPosition = position;
    window->unk04 = 0;
    limit = func_00273308(position, windowMode, window, advance);
    if (window->windowMode == 0) {
        memset(windowStart, 0, 7);
    } else if (startPosition < limit) {
        spanLength = limit - startPosition;
        windowShift = spanLength >> 1;
        if (windowShift < 7) {
            memcpy(windowStart, windowStart + windowShift, 7 - windowShift);
            memset((windowStart - windowShift) + 7, 3, windowShift);
        } else {
            memset(windowStart, 3, 7);
        }
    }
    return limit;
}

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272F50);

s32 func_0030C108(s32);

void packetSubmitAfterHeader(s32 packetHeaderSlot) {
    func_0030C108(packetHeaderSlot + 2);
}

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272FA0);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00272FD8);

s32 packetSlotAfterHeader(s32 packetHeaderSlot) {
    return packetHeaderSlot + 2;
}

s8 *textSkipOpenBracket(s8 *text)
{
  int skipBytes;
  return ((*text) != 0x5B) ? (0) : (text + (1 & (skipBytes = 0xFFFFFFFFu)));
}

s8 *func_0030E4FC(s8 *, s32);

s8 *textFindCloseAngle(s8 *text) {
    return func_0030E4FC(text, 0x3E);
}

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00273048);

s8 *textFindCloseBracket(s8 *text) {
    return func_0030E4FC(text, 0x5D);
}

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_002730D0);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00273288);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_002732C8);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_00273308);

INCLUDE_ASM("asm/main/nonmatchings/tu292", func_002734B8);

extern s32 D_003E3D08[];

s32 packetTableGet(s32 packetTableKey)
{
    if (packetTableKey < 0) {
        packetTableKey = 1;
    }
    if (packetTableKey >= 9) {
        packetTableKey = 0x96;
    }
    return D_003E3D08[packetTableKey];
}

void packetEntryAppend(s32 wordToAppend) {
    PacketEntry *nextPacketEntry;

    if (packetWriteCursor != 0) {
        packetWriteCursor->tag = 1;
        nextPacketEntry = packetWriteCursor + 1;
        packetWriteCursor->packetWord = wordToAppend;
        packetWriteCursor = nextPacketEntry;
        nextPacketEntry->tag = 0;
    }
}
