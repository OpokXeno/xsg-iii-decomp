#include "common.h"

typedef unsigned char u8;
typedef signed char s8;
typedef int s32;
typedef float f32;
typedef unsigned int u32;
extern u8 gTextStepCounter;
extern f32 D_003E3D30[];
extern s32 gTextLabelBox[];
extern u8 gTextLineBuffer[];

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

void func_00267D78();
void fontStateMarkDirty();
void slotReleaseOwner();
void func_00269D60(TextStep *textStep);
void func_00270750();
void textDrawModeSet(s32 mode);
void textDrawFlagSet();
void rgSingletonIDEnable();
void rgSingletonIDDisable();
void func_00273920();
void func_00273E38();
void func_0026A4E8();
void func_0026B580(s32 key, TextStep *textStep);
void packetBufferSetCurrent(s32 key);
s8 *packetSpanUpdate(s8 *cursor, s8 *windowEnd, void *buffer, s32 advance);
s32 func_002738E8();
s32 xglFontGetStringWidth();
s32 xglFontPrint();
void textTagClearKey(void *key);
void blendStateInit(TextStep *textStep);
void textStepSetIndex(TextStep *textStep, s32 index);
void setMovieParams(s32 mode, f32 x, f32 y);

void textStepCounterAdvance(void) {
    gTextStepCounter += 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_00269C98);

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_00269D60);

void stepStateResetToPending(TextStep *textStep) {
    slotReleaseOwner();
    textTagClearKey(textStep);
    textStep->unk_04 = 0;
    textStep->statusBits = (textStep->statusBits & ~1) | 0x800;
}

s8 *func_00272CD8();
s32 textSkipOpenBracket();
void stepStateResetWindow(TextStep *textStep);
void stepStateSetFirstTag(TextStep *textStep, s32 firstTag);
void stepStateSetSecondTag(TextStep *textStep, s32 secondTag);
void func_0026A808();
void func_0026A878();
void func_0026AA88();
void textDrawModeSet();
s8 *func_00272D18(s8 *windowStart, s8 index);
s32 textSkipOpenBracket();

void stepStateSetup(TextStep *textStep, s32 textSource, s32 requestedMode) {
    u8 resolvedMode;
    s8 *textStart;
    s32 labelLength;

    resolvedMode = requestedMode;
    if (textStep->textFlags & 1) {
        textDrawModeSet(1);
        stepStateSetFirstTag(textStep, 0x6E3E3030);
        stepStateSetSecondTag(textStep, 0x6E3E3030);
    } else {
        textDrawModeSet(0);
        stepStateSetFirstTag(textStep, 0x6E2F2F2F);
        stepStateSetSecondTag(textStep, 0x786C6448);
    }
    textStart = func_00272CD8(textSource);
    textStep->textStart = textStart;
    labelLength = textSkipOpenBracket(textSource);
    textStep->labelLength = labelLength;
    if (resolvedMode == 0) {
        func_0026AA88(&resolvedMode, textStep, textStep->textStart);
    }
    func_0026A878(textStep, resolvedMode);
    func_0026A808(textStep, textStep->textStart);
    func_00269D60(textStep);
    stepStateResetWindow(textStep);
}

void stepStateUpdateWindowEnd(TextStep *textStep);

void stepStateUpdateWindowEnd(TextStep *textStep) {
    s8 *windowStart;

    windowStart = textStep->windowStart;
    if (windowStart != 0) {
        textStep->windowEnd = func_00272D18(windowStart, textStep->activeStepIndex);
    }
}

void stepStateSetOrigin(TextStep *textStep, f32 x, f32 y) {
    textStep->rectLeft = x;
    textStep->rectTop = y;
    textStep->rectRight = x + textStep->offsetX;
    textStep->rectBottom = y + textStep->offsetY;
}

void stepStateSetRectFixed(TextStep *textStep, f32 left, f32 top, f32 right, f32 bottom) {
    textStep->rectLeft = ((s32) left * 0x10) + 0x7000;
    textStep->rectTop = ((s32) top * 0x10) + 0x7200;
    textStep->rectRight = ((s32) right * 0x10) + 0x7000;
    textStep->rectBottom = ((s32) bottom * 0x10) + 0x7200;
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A080);

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A138);

void stepStateSetOffsetIfNonZero(TextStep *textStep, f32 x, f32 y) {
    if (x != 0.0f) {
        textStep->offsetX = x;
    }
    if (y != 0.0f) {
        textStep->offsetY = y;
    }
}

void stepStateUpdateWindowEnd(TextStep *textStep);

void stepStateResetWindow(TextStep *textStep) {
    s8 *textStart;

    textStart = textStep->textStart;
    textStep->windowStart = textStart;
    textStep->cursor = textStart;
    stepStateUpdateWindowEnd(textStep);
}

void blendStateSetModeFlag(s32 *statusBits, s32 blendMode) {
    fontStateMarkDirty();
    if (blendMode == 1) {
        *statusBits = *statusBits | 0x20;
    } else if (blendMode == 2) {
        *statusBits = *statusBits | 0x40;
    }
}

void blendStateResetMode(TextStep *textStep, s32 blendMode) {
    slotReleaseOwner();
    textTagClearKey(textStep);
    blendStateInit(textStep);
    if (blendMode == 1) {
        textStep->statusBits = textStep->statusBits | 0x20;
    } else if (blendMode == 2) {
        textStep->statusBits = textStep->statusBits | 0x40;
    }
}

void stepStateMarkStarted(TextStep *textStep, s32 requestedStepIndex) {
    s32 nextStatusBits;

    nextStatusBits = textStep->statusBits | 0x10;
    textStep->unk_A0 = 0;
    textStep->statusBits = nextStatusBits;
    if (requestedStepIndex != 0) {
        textStepSetIndex(textStep, requestedStepIndex);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A2E0);

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A370);

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A4E8);

void textFlushCallback(void) {
    func_00267D78();
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A558);

void stepStateSyncWindow(TextStep *textStep) {
    slotReleaseOwner();
    textTagClearKey(textStep);
    textStep->windowStart = textStep->windowEnd;
    stepStateUpdateWindowEnd(textStep);
}

void blendStateResetMode(TextStep *textStep, s32 blendMode);

void stepStateUpdateHold(TextStep *textStep) {
    if (textStep->hold == 0) {
        blendStateResetMode(textStep, 0);
    }
}

void stepStateAdvanceText(TextStep *textStep, s32 advance)
{
    packetBufferSetCurrent(0x70003000);
    textStep->cursor = packetSpanUpdate(textStep->cursor, textStep->windowEnd, textStep->packetBuffer, advance);
    packetBufferSetCurrent(0);
    if ((textStep->requestedIndex != 0) && (textStep->activeStepIndex != textStep->requestedIndex)) {
        stepStateMarkStarted(textStep, textStep->requestedIndex);
        func_00269D60(textStep);
        stepStateUpdateWindowEnd(textStep);
    }
    if (textStep->dirty != 0) {
        textStep->statusBits |= 8;
    }
    if (*(s8 *) 0x70003000 != 0) {
        func_0026B580(0x70003000, textStep);
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A6D0);

void stepStateFetchOffset(f32 *offsetX, f32 *offsetY, TextStep *textStep) {
    *offsetX = textStep->offsetX;
    *offsetY = textStep->offsetY;
}

void stepStateSetAuxValue(TextStep *textStep, s32 auxValue) {
    textStep->auxValue = auxValue;
}

void stepStateSetSecondaryScale(TextStep *textStep, f32 secondaryScale) {
    textStep->secondaryScale = secondaryScale;
}

void stepStateSetFirstTag(TextStep *textStep, s32 firstTag) {
    textStep->tagA = firstTag;
}

void stepStateSetSecondTag(TextStep *textStep, s32 secondTag) {
    textStep->tagB = secondTag;
}

void stepStateSetBaseScale(TextStep *textStep, f32 baseScale) {
    textStep->baseScale = baseScale;
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A7C0);

void stepStateSetHold(TextStep *textStep, s8 hold) {
    textStep->hold = hold;
    textStep->statusBits |= 0x80;
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A808);

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026A878);

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026AA88);

void stepStateSubmitMovie(TextStep *textStep);
s32 textFindLineBreak(TextStep *textStep);
void stepStateQueueFlush(TextStep *textStep);
void stepStateLayoutText(TextStep *textStep);
void stepStateDrawText(TextStep *textStep);
void func_0026AC38(TextStep *textStep);
void setMovieParams(s32, f32, f32);
void func_00273920();

void stepStateDraw(TextStep *textStep)
{
    s32 textFlags;

    textFlags = textStep->textFlags;
    if (textFlags & 1) {
        if (!(textFlags & 2)) {
            setMovieParams(0, 0.0f, 0.0f);
        } else {
            setMovieParams(1, 0.0f, 0.0f);
        }
        stepStateSubmitMovie(textStep);
    } else {
        setMovieParams(0, textStep->rectLeft, textStep->rectTop);
        func_0026AC38(textStep);
    }
    if (textStep->textFlags & 1) {
        if (!(textStep->statusBits & 0x16)) {
            stepStateLayoutText(textStep);
            textFindLineBreak(textStep);
            stepStateQueueFlush(textStep);
        }
    } else if (!(textStep->statusBits & 6)) {
        stepStateDrawText(textStep);
        if (!(textStep->statusBits & 0x10)) {
            textFindLineBreak(textStep);
            stepStateQueueFlush(textStep);
        }
    }
    func_00273920();
}

INCLUDE_ASM("asm/main/nonmatchings/tu278", func_0026AC38);

void stepStateSubmitMovie(TextStep *textStep) {
    func_002738E8(func_00273E38, textStep);
}

void func_0026A6D0(void *, s8 *, s8 *, s32, TextStep *);

s32 textFindLineBreak(TextStep *textStep)
{
    s32 textFlags;
    s32 lineBreakResult;
    s32 lineBreakStatus;
    s8 *windowEnd;
    s8 *textStart;
    s8 *candidate;
    s8 *cursor;

    textFlags = textStep->textFlags;
    lineBreakStatus = textFlags & 0x100;
    if ((lineBreakStatus != 0) && ((cursor = textStep->cursor, windowEnd = textStep->windowEnd, lineBreakStatus = cursor < windowEnd, lineBreakStatus == 0)))
    {
        if ((textFlags & 0x200) || ((lineBreakStatus = textStep->statusBits & 0x108, lineBreakStatus != 0)))
        {
            textStart = textStep->textStart;
            candidate = cursor - 2;
            if (((candidate >= textStart) && (candidate[0] == '\\')) && (cursor[-1] == 'n'))
            {
                for (;;)
                {
                    cursor = candidate;
                    candidate = cursor - 2;
                    if (candidate[0] == '\\')
                    {
                        if (cursor[-1] == 'n')
                        {
                            continue;
                        }
                    }
                    break;
                }

            }
            func_0026A6D0(gTextLineBuffer, textStep->windowStart, cursor, 0, textStep);
            lineBreakResult = func_002738E8(func_0026A4E8, gTextLineBuffer);
            lineBreakStatus = lineBreakResult;
        }
    }
    return lineBreakStatus;
}

void textFlushCallback(void);

void stepStateQueueFlush(TextStep *textStep) {
    func_002738E8(textFlushCallback, textStep);
}

void func_00269280(s8 *, s8 *);
s32 func_00273F78(s32, s32);
s32 func_0030E4FC(s32, s32);
void func_00272680(s32, s8 *, s8 *, u8 *);
s32 func_00272A10(s32, s32);

void stepStateLayoutText(TextStep *textStep) {
    s32 labelOffset;

    labelOffset = 0;
    func_00272680(0x70000000, textStep->windowStart, textStep->cursor, textStep->text);
    if (textStep->labelLength != 0) {
        labelOffset = func_0030E4FC(0x70000000, 0) + 5;
        func_00272A10(labelOffset, textStep->labelLength);
    }
    func_00273F78(0x70000000, labelOffset);
    func_00269280(textStep->windowStart, textStep->cursor);
}

void textDrawFlagSet();
s32 xglFontGetStringWidth();
s32 xglFontPrint();

void stepStateDrawText(TextStep *textStep) {
    s32 labelLength;
    s32 textX;
    s32 textY;

    if (textStep->textFlags & 0x800) {
        textDrawFlagSet(1);
        func_002738E8(rgSingletonIDEnable, 0);
    }
    labelLength = textStep->labelLength;
    if (labelLength != 0) {
        func_00272A10(0x70000000, labelLength);
        if (!(textStep->textFlags & 0x40)) {
            textX = gTextLabelBox[0] + xglFontGetStringWidth(0x70000000);
            textY = gTextLabelBox[1] + 0x14;
            gTextLabelBox[4] = textX;
            gTextLabelBox[5] = textY;
            func_002738E8(func_00270750, gTextLabelBox);
        }
        xglFontPrint((s32) (D_003E3D30[0] + 25.0f), (s32) (D_003E3D30[1] + -16.0f), 0xFFFFFF, 0x70000000);
    }
    func_00272680(0x70000000, textStep->windowStart, textStep->cursor, textStep->text);
    xglFontPrint((s32) (textStep->printOffsetX + D_003E3D30[0]), (s32) (textStep->printOffsetY + D_003E3D30[1] + 4.0f), 0xFFFFFF, 0x70000000);
    if (textStep->textFlags & 0x800) {
        textDrawFlagSet(0);
        func_002738E8(rgSingletonIDDisable, 0);
    }
}
