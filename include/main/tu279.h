#ifndef INCLUDE_MAIN_TU279_H
#define INCLUDE_MAIN_TU279_H

typedef int s32;

typedef unsigned char u8;

typedef signed char s8;

typedef short s16;

typedef float f32;

/* A text tag: the record stepStateAdvanceText and textTagClearKey work on (tu278.c, tu267). Offsets are the ones
 * this file reads and writes; the gaps are not modelled. */
typedef struct TextTag {
    s32 tagFlags;             /* 0x00: TEXT_TAG_* bits */
    u8 unk_04[0x34];
    f32 timer;             /* 0x38: counts down by func_0026A558() per frame */
    u8 unk_3C[0x8];
    s32 inputHandlingBits;        /* 0x44: bit 4 and bit 8 select the input handling */
    u8 unk_48[0x4];
    s32 unk_4C;            /* 0x4C: argument of func_0030EF98 */
    u8 unk_50[0x4];
    s8 *readCursor;        /* 0x54: byte cursor into the tag's text */
    s8 *writeCursor;       /* 0x58 */
    u8 unk_5C[0x4];
    s16 bufferHead;        /* 0x60: first halfword of the buffer at 0x60 */
    s8 unk_62;
    u8 unk_63[0x3D];
    f32 progress;          /* 0xA0: 0..1 */
    u8 unk_A4[0x2];
    u8 ticksLeft;          /* 0xA6: frames left before flag 0x80 clears */
    u8 unk_A7[0x3];
    s8 frameCount;         /* 0xAA: frames since the tag was shown, wraps at 12 */
} TextTag;

#endif /* INCLUDE_MAIN_TU279_H */
