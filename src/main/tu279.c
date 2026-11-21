#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu279", func_0026B010);

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;
typedef short s16;
typedef unsigned short u16;

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

/* Port 0 of PadData (the controller record of xgl_pad.c; include/main/tu007.h has the full layout). */
typedef struct PadPort {
    u8 unk_00[0x28];
    u16 held;              /* 0x28 */
    u16 pressed;           /* 0x2A */
} PadPort;

extern PadPort PadData[2];
extern u16 gPadDataPressed;     /* PadData[0].pressed, one symbol with the same address */

void textTagTickCountdown(TextTag *tag)
{
    unsigned int ticks;
    ticks = tag->ticksLeft;
    if (ticks != 0)
    {
        tag->ticksLeft = ticks + 0xFF;
        return;
    }
    tag->tagFlags &= ~0x80;
}

void textTagAdvanceFrame(TextTag *tag)
{
    tag->frameCount = tag->frameCount + 1;
    if (tag->frameCount >= 0xC)
    {
        tag->tagFlags &= ~0x200;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu279", func_0026B1A0);

s32 stepStateResetToPending(void *);
f32 func_0026A558();

void textTagTickTimer(TextTag *tag) {
    f32 remaining;

    remaining = tag->timer - func_0026A558();
    tag->timer = remaining;
    if (!(remaining > 0.0f)) {
        tag->timer = 0.0f;
        tag->tagFlags &= ~0x64;
        stepStateResetToPending(tag);
    }
}

void textTagAdvanceProgress(TextTag *tag) {
    f32 progress;

    progress = tag->progress + func_0026A558();
    tag->progress = progress;
    if (!(progress < 1.0f)) {
        tag->progress = 1.0f;
        tag->tagFlags &= ~0x10;
    }
}

s32 stepStateMarkStarted();

void textTagUpdateWaitState(TextTag *tag)
{
    extern s32 stepStateSyncWindow();
    s32 updatedFlags;
    s32 *updatedFlagsPointer;

    if (tag->readCursor == tag->writeCursor && tag->bufferHead == 0 && *tag->readCursor != 0)
    {
        if (tag->inputHandlingBits & 4)
        {
            stepStateSyncWindow(tag);
            stepStateMarkStarted(tag, 0);
        }
        if (tag->inputHandlingBits & 8)
        {
            if (!(!(gPadDataPressed & 0x20)))
            {
                stepStateSyncWindow(tag);
                stepStateMarkStarted(tag, 0);
            }
            else
            {
                /* The flags go through a pointer: the compiled code keeps the local in memory. */
                updatedFlags = tag->tagFlags | 0x100;
                updatedFlagsPointer = &updatedFlags;
                tag->tagFlags = *updatedFlagsPointer;
            }
        }
    }
}

void textTagPollPadInput(TextTag *tag)
{
    extern s32 stepStateAdvanceText();
    if (tag->inputHandlingBits & 8)
    {
        if (PadData[0].pressed & 0x20)
        {
            stepStateAdvanceText(tag, 2);
            return;
        }
        if (PadData[0].held & 0x20)
        {
            stepStateAdvanceText(tag, 1);
            return;
        }
        stepStateAdvanceText(tag, 0);
        return;
    }
    stepStateAdvanceText(tag, 0);
}

void textTagDrainNodes(TextTag *tag) {
    extern s32 stepStateSyncWindow(void *);
    extern s32 stepStateAdvanceText(void *, s32);
    s8 *previousNode;

    if (gPadDataPressed & 0x40) {
        tag->unk_62 = 0;
        do {
            previousNode = tag->writeCursor;
            stepStateAdvanceText(tag, 0);
            if (*tag->writeCursor != 0) {
                stepStateSyncWindow(tag);
            }
        } while (previousNode != tag->writeCursor);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu279", func_0026B480);

extern s32 D_00550870;
s32 stepStateUpdateHold(void *);
s32 func_0030EF98(s32, s32 *);

void textTagTryResolveWait(TextTag *tag) {
    if (func_0030EF98(tag->unk_4C, &D_00550870) == 0) {
        stepStateUpdateHold(tag);
        return;
    }
    tag->tagFlags |= 0x800;
}
