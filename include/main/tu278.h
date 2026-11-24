#ifndef INCLUDE_MAIN_TU278_H
#define INCLUDE_MAIN_TU278_H

typedef unsigned char u8;

typedef int s32;

typedef signed char s8;

typedef float f32;

/* Text step object. textStepSetIndex (tu281) takes the same object. Offsets are the engine's. */
typedef struct TextStep {
    s32 statusBits;          /* +0x00: status bits (0x8 dirty, 0x10 started, 0x80 hold, 0x800 text pending) */
    s32 unk_04;         /* +0x04: cleared by stepStateResetToPending */
    u8 unk_08[0x10 - 0x08];
    f32 offsetX;        /* +0x10: added to rectX0 for rectX1 */
    f32 offsetY;        /* +0x14: added to rectY0 for rectY1 */
    u8 unk_18[0x20 - 0x18];
    f32 rectLeft;         /* +0x20 */
    f32 rectTop;         /* +0x24 */
    f32 rectRight;         /* +0x28 */
    f32 rectBottom;         /* +0x2C */
    f32 baseScale;         /* +0x30 */
    f32 secondaryScale;         /* +0x34 */
    u8 unk_38[0x3C - 0x38];
    s32 tagA;           /* +0x3C */
    s32 tagB;           /* +0x40 */
    s32 textFlags;          /* +0x44: bit 0 selects the movie path */
    s32 labelLength;    /* +0x48 */
    s8 *textStart;     /* +0x4C: text start pointer */
    s8 *windowStart;    /* +0x50 */
    s8 *cursor;         /* +0x54 */
    s8 *windowEnd;      /* +0x58 */
    s32 auxValue;         /* +0x5C */
    u8 packetBuffer[3];       /* +0x60: buffer passed to packetSpanUpdate */
    u8 dirty;           /* +0x63 */
    u8 requestedIndex;  /* +0x64 */
    u8 text[0x3B];      /* +0x65 */
    s32 unk_A0;         /* +0xA0 */
    u8 activeStepIndex;           /* +0xA4: current step index */
    u8 unk_A5;
    u8 hold;            /* +0xA6 */
    u8 unk_A7;
    s8 printOffsetX;    /* +0xA8 */
    s8 printOffsetY;    /* +0xA9 */
} TextStep;

void stepStateSyncWindow(TextStep *textStep);

void stepStateAdvanceText(TextStep *textStep, s32 advance);

#endif /* INCLUDE_MAIN_TU278_H */
