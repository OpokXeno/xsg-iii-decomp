#include "common.h"

typedef int s32;
typedef unsigned int u32;
typedef signed char s8;
typedef unsigned char u8;

#define ENVELOPE_STEP_CAP 0xA

#define offsetof(type, field) ((u32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((void *) (member) - offsetof(type, field)))

/* Counter with a [start, end) range (same layout as include/main/tu378.h). */
typedef struct WindowCursor {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 stepPosition;
    /* 0x08 */ s32 lastStepPosition;
    /* 0x0C */ s32 windowStart;
    /* 0x10 */ s32 windowEnd;
    /* 0x14 */ s32 windowLastIndex;
    /* 0x18 */ s32 savedStepPosition;
    /* 0x1C */ s32 flag_a;
    /* 0x20 */ s32 flag_b;
    /* 0x24 */ s8 flag_c;
} WindowCursor;

/* The per-context block (0x1DC4 bytes, cleared by seChannelBankClear in tu380.c). */
typedef struct EnvelopeContext {
    s32 envelopeOutput;           /* +0x00: the first word, read through the object by seVoiceEnvelopeUpdate */
    u8 unk_04[0x08 - 0x04];
    s32 limit_08;                 /* +0x08: the cap of the counter range in counterRangeSetup */
    u8 unk_0C[0x44C - 0x0C];
    s32 envelopeStepLevel;        /* +0x44C: the table value of the current step */
    u8 unk_450[0x458 - 0x450];
    s32 windowEnd;                /* +0x458 */
    s32 envelopeStepIndex;        /* +0x45C */
    s32 windowStart;              /* +0x460 */
    u8 unk_464[0x4A0 - 0x464];
    s32 envelopeStepTable[0x647]; /* +0x4A0: indexed by envelopeStepIndex */
    s32 envelopeMode;             /* +0x1DBC: 0 selects the first mode function, otherwise the second */
    u8 unk_1DC0[0x1DC4 - 0x1DC0];
} EnvelopeContext;

/* The sound-side object at gEnvelopeObject (0x1250 past the state block). */
typedef struct EnvelopeObject {
    u8 unk_00[0x10];
    u8 plane_10[0xD0 - 0x10];     /* +0x10: njPlaneInit target */
    u8 plane_D0[0x190 - 0xD0];    /* +0xD0: njPlaneInit target */
    s32 limit_190;                /* +0x190: stored only when nonzero */
    s32 outputLevel;              /* +0x194 */
    s8 flag_198;                  /* +0x198 */
    s8 flag_199;                  /* +0x199 */
} EnvelopeObject;

/* The state block at gSeVoiceEnvelopeState: its context pointer, the counter range at +0x141C (gEnvelopeCursor) and a word
 * at +0x1F50. */
typedef struct EnvelopeState {
    EnvelopeContext *envelopeContext;    /* +0x00 */
    u8 unk_04[0x1250 - 0x04];
    EnvelopeObject envelopeObject;       /* +0x1250 (gEnvelopeObject) */
    u8 unk_13EC[0x141C - 0x13EC];
    WindowCursor envelopeCursor;         /* +0x141C */
    u8 unk_1444[0x1F50 - 0x1444];
    s32 unk_1F50;                 /* +0x1F50: cleared by seVoiceEnvelopeUpdate */
} EnvelopeState;

extern EnvelopeState gSeVoiceEnvelopeState;
extern WindowCursor gEnvelopeCursor;
extern WindowCursor gCapturedEnvelopeCursor;
extern EnvelopeObject gEnvelopeObject;
extern s32 gSeVoiceFlag[];
extern unsigned char D_00402858[];

/* The state block as seen from its cursor symbol gEnvelopeCursor (the cursor is at +0x141C). */
#define gEnvelopeStateOfCursor (*CONTAINER_OF(&gEnvelopeCursor, EnvelopeState, envelopeCursor))
/* The state block as seen from the object symbol gEnvelopeObject (the object is at +0x1250). */
#define gEnvelopeStateOfObject (*CONTAINER_OF(&gEnvelopeObject, EnvelopeState, envelopeObject))

void decodeCounterSetFlagC(WindowCursor *cursor, s8 value);
void decodeCounterSetFlagA(WindowCursor *cursor, s8 value);
void decodeCounterSetDecodedLastIndex(WindowCursor *cursor, s32 value);
void decodeCounterSetStart(WindowCursor *cursor, s32 value);
void decodeCounterSetEnd(WindowCursor *cursor, s32 value);
void decodeCounterClear(WindowCursor *cursor);
void decodeCounterSet(WindowCursor *cursor, s32 start, s32 end, s32 last);
s32 decodeCounterGetCount(WindowCursor *cursor);
s32 decodeCounterGetEnd(WindowCursor *cursor);
s32 decodeCounterGetStart(WindowCursor *cursor);
s32 counterRangeSnapshot(WindowCursor *cursor);
void sineWavesReset();

void counterNoop(void) {

}

void seVoiceEnvelopeBindContext(EnvelopeContext *envelopeContext) {
    gSeVoiceEnvelopeState.envelopeContext = envelopeContext;
}

void counterRangeLoadBounds(void) {
    EnvelopeState *envelopeState;

    envelopeState = &gEnvelopeStateOfCursor;
    decodeCounterSetEnd(&gEnvelopeCursor, envelopeState->envelopeContext->windowEnd);
    decodeCounterSetStart(&gEnvelopeCursor, envelopeState->envelopeContext->windowStart);
}

void counterRangeClearBounds(void) {
    decodeCounterSetEnd(&gEnvelopeCursor, 0);
    decodeCounterSetStart(&gEnvelopeCursor, 0);
}

s32 seVoiceGetFlag(void) {
    return gSeVoiceFlag[0];
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DBD80);

void counterRangeCapture(void)
{
  WindowCursor *captureCursor;
  captureCursor = &gCapturedEnvelopeCursor;
  counterRangeSnapshot(&gCapturedEnvelopeCursor);
  decodeCounterGetEnd(&gCapturedEnvelopeCursor);
  captureCursor = &gCapturedEnvelopeCursor;
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DBF28);

void seVoiceEnvelopeReadCapturedStep(void) {
    decodeCounterGetCount(&gCapturedEnvelopeCursor);
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC0A0);

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC138);

void sineWavesReset();
void func_002DC0A0();
void func_002DC138();

void counterRangeSetup(void) {
    EnvelopeState *envelopeState;
    s32 envelopeMode;
    s32 stepLimit;

    envelopeMode = gSeVoiceEnvelopeState.envelopeContext->envelopeMode;
    if (envelopeMode == 0) {
        func_002DC0A0();
    } else {
        func_002DC138();
    }
    envelopeState = &gEnvelopeStateOfCursor;
    decodeCounterClear(&gEnvelopeCursor);
    decodeCounterSetFlagC(&gEnvelopeCursor, 2);
    decodeCounterSetFlagA(&gEnvelopeCursor, 0);
    stepLimit = envelopeState->envelopeContext->limit_08;
    if (stepLimit < ENVELOPE_STEP_CAP) {
        decodeCounterSetDecodedLastIndex(&gEnvelopeCursor, stepLimit);
        decodeCounterSet(&gEnvelopeCursor, 0, 0, envelopeState->envelopeContext->limit_08);
    } else {
        decodeCounterSetDecodedLastIndex(&gEnvelopeCursor, stepLimit);
        decodeCounterSet(&gEnvelopeCursor, 0, 0, ENVELOPE_STEP_CAP);
    }
    sineWavesReset();
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC3D0);

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC448);

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC5A0);

void seVoiceEnvelopeObjectSetLevel(EnvelopeObject *envelopeObject, s32 outputLevel);
s32 func_002DC448();
s32 func_002DC5A0();

void seVoiceEnvelopeUpdate(void)
{
    EnvelopeState *envelopeState;
    WindowCursor *envelopeCursor;
    s32 gateWordOrWindowEnd;
    s32 windowStart;
    s32 envelopeStepLevel;
    s32 envelopeStepIndex;
    s32 envelopeMode;

    envelopeState = &gSeVoiceEnvelopeState;
    envelopeCursor = &envelopeState->envelopeCursor;
    envelopeState->unk_1F50 = 0;
    envelopeState->envelopeContext->windowStart = 0;
    gateWordOrWindowEnd = D_00402858[0];
    if (!(gateWordOrWindowEnd & 1))
    {
        counterRangeSnapshot(envelopeCursor);
    }
    gateWordOrWindowEnd = decodeCounterGetEnd(envelopeCursor);
    envelopeState->envelopeContext->windowEnd = gateWordOrWindowEnd;
    windowStart = decodeCounterGetStart(envelopeCursor);
    envelopeState->envelopeContext->windowStart = windowStart;
    envelopeStepIndex = envelopeState->envelopeContext->windowEnd + windowStart;
    envelopeMode = envelopeState->envelopeContext->envelopeMode;
    envelopeState->envelopeContext->envelopeStepIndex = envelopeStepIndex;
    envelopeStepLevel = envelopeState->envelopeContext->envelopeStepTable[envelopeStepIndex];
    envelopeState->envelopeContext->envelopeStepLevel = envelopeStepLevel;
    if (envelopeMode == 0)
    {
        func_002DC448();
    }
    else
    {
        func_002DC5A0();
    }
    seVoiceEnvelopeObjectSetLevel(&gEnvelopeObject, gEnvelopeStateOfObject.envelopeContext->envelopeOutput);
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC860);

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DC938);

s32 func_002DC860();
s32 func_002DC938();

void seVoiceEnvelopeUpdateByMode(void) {
    s32 envelopeMode;

    envelopeMode = gSeVoiceEnvelopeState.envelopeContext->envelopeMode;
    if (envelopeMode == 0) {
        func_002DC860();
        return;
    }
    func_002DC938();
}

void seVoiceEnvelopeReadStep(void) {
    decodeCounterGetCount(&gEnvelopeCursor);
}

s32 njPlaneInit(void *element);

void seVoiceEnvelopeObjectInit(EnvelopeObject *envelopeObject)
{
  njPlaneInit(envelopeObject->plane_10);
  njPlaneInit(envelopeObject->plane_D0);
  envelopeObject->flag_198 = 0;
  envelopeObject->flag_199 = 0;
}

void counterObjectSetLimit(EnvelopeObject *envelopeObject, s32 limit) {
    if (limit != 0) {
        envelopeObject->limit_190 = limit;
    }
}

void seVoiceEnvelopeObjectSetLevel(EnvelopeObject *envelopeObject, s32 outputLevel) {
    envelopeObject->outputLevel = outputLevel;
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DCDB8);

void seVoiceEnvelopeObjectSetFlagB(EnvelopeObject *envelopeObject, s8 value) {
    envelopeObject->flag_199 = value;
}

void seVoiceEnvelopeObjectSetFlagA(EnvelopeObject *envelopeObject, s8 value) {
    envelopeObject->flag_198 = value;
}

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DCE80);

INCLUDE_ASM("asm/main/nonmatchings/tu396", func_002DD248);
