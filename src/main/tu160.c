#include "common.h"

#include "main/tu007.h"

void xglPadSetRepeat(int, int, int, int);

typedef int s32;

typedef signed char s8;

typedef short s16;

typedef unsigned char u8;

typedef unsigned short u16;

static u16 gStatusBitsLow;
static u16 gStatusBitsHigh;
static unsigned char gPadStatusMasks[16];

/* Status header record at gPadStatusMasks: the pad masks of the frame (+0x00 held, +0x02 pressed, +0x04 repeated,
 * +0x06 pressed in the previous frame) and the two group words at +0x08 and +0x0A. */
typedef struct PadStatusMasks {
    unsigned short held;
    unsigned short pressed;
    unsigned short repeated;
    unsigned short previousPressed;
    unsigned short unk_08;
    unsigned short unk_0A;
} PadStatusMasks;

#define gPadStatus (*(PadStatusMasks *) gPadStatusMasks)

/* Resets the pad status words and sets both status masks to all bits. */
void padStatusMasksClear(void) {
    xglPadSetRepeat(0, 0xf000, 8, 1);
    gPadStatus.held = 0;
    gPadStatus.pressed = 0;
    gPadStatus.repeated = 0;
    gStatusBitsHigh = -1;
    gPadStatus.previousPressed = 0;
    gStatusBitsLow = -1;
}

extern PadRecord PadData[];

typedef struct PadGroupHalves {
    unsigned char unmodeled_00[0x6A];
    unsigned short unk_6A;
    unsigned short unk_6C;
} PadGroupHalves;

#define gPadGroup (*(PadGroupHalves *) &PadData[0])

/* Per-frame pad update: the previous frame's pressed bits move to previousPressed, then the direction bits of
 * PadData[0] and the two group words are masked into the status record. */
void padStatusSampleGroups(void) {
    gPadStatus.previousPressed = gPadStatus.pressed;
    gPadStatus.held = PadData[0].directionHeld & gStatusBitsLow;
    gPadStatus.pressed = PadData[0].directionPressed & gStatusBitsLow;
    gPadStatus.repeated = PadData[0].directionRepeated & gStatusBitsLow;
    gPadStatus.unk_08 = gPadGroup.unk_6A & gStatusBitsHigh;
    gPadStatus.unk_0A = gPadGroup.unk_6C & gStatusBitsHigh;
}

void clearStatusBits(s32 clearLowBits, s32 clearHighBits)
{
    s32 lowKeepMask;
    s32 highKeepMask;

    lowKeepMask = ~(clearLowBits & 0xFFFF);
    highKeepMask = ~(clearHighBits & 0xFFFF);
    gStatusBitsLow &= lowKeepMask;
    gStatusBitsHigh &= highKeepMask;
}

void setStatusBits(s32 setLowBits, s32 setHighBits) {
    gStatusBitsLow |= setLowBits;
    gStatusBitsHigh |= setHighBits;
}

/* ORs the low bits into the direction words and the high bits into the group words. */
void padStatusSetBits(u16 lowBits, u16 highBits) {
    gPadStatus.held = lowBits | gPadStatus.held;
    gPadStatus.pressed = lowBits | gPadStatus.pressed;
    gPadStatus.repeated = lowBits | gPadStatus.repeated;
    gPadStatus.unk_08 = highBits | gPadStatus.unk_08;
    gPadStatus.unk_0A = highBits | gPadStatus.unk_0A;
}

s32 statusHeaderIsReadyMode20(void)
{
    s32 isModeMatch;
    isModeMatch = 0;
    if (gPadStatus.pressed == 0x20)
    {
        isModeMatch = gPadStatus.previousPressed == 0;
    }
    return isModeMatch;
}

s32 statusHeaderIsReadyMode40(void)
{
    s32 isModeMatch;
    isModeMatch = 0;
    if (gPadStatus.pressed == 0x40)
    {
        isModeMatch = gPadStatus.previousPressed == 0;
    }
    return isModeMatch;
}

s32 statusHeaderTestFlags(void)
{
    s32 isBit11Set;

    isBit11Set = 0;
    if ((gPadStatus.held & 0x10C) == 0x10C)
    {
        isBit11Set = (gPadStatus.pressed & 0x800) != isBit11Set;
    }
    return isBit11Set;
}

/* A 16-bit value that steps toward a target: fields 0 and 2 of the record at +0x54/+0x58. */
typedef struct SteppedValue {
    s16 level;
    s16 targetLevel;
} SteppedValue;

void setSteppedValue(SteppedValue *stepped, s16 currentLevel, s16 targetLevel) {
    stepped->level = currentLevel;
    stepped->targetLevel = targetLevel;
}

void setSteppedTarget(SteppedValue *stepped, s16 targetLevel) {
    stepped->targetLevel = targetLevel;
}

extern u8 gVsyncsPerFrame;

/* Moves the level one step toward targetLevel per frame; the step is gVsyncsPerFrame in 1/16 units and the level
 * stops on the target. */
void stepValueTowardTarget(SteppedValue *stepped) {
    s32 currentLevel;
    s32 targetLevel;
    s32 stepAmount;
    s32 unsignedLevel;

    currentLevel = stepped->level;
    targetLevel = stepped->targetLevel;
    stepAmount = (gVsyncsPerFrame << 0x18) >> 0x14;
    unsignedLevel = (u16) stepped->level;
    if (currentLevel < targetLevel) {
        if ((targetLevel - currentLevel) >= stepAmount) {
            stepped->level = stepAmount + unsignedLevel;
            return;
        }
        stepped->level = stepped->targetLevel;
        return;
    }
    if (targetLevel < currentLevel) {
        if ((currentLevel - targetLevel) < stepAmount) {
            stepped->level = stepped->targetLevel;
            return;
        }
        stepped->level = unsignedLevel - stepAmount;
    }
}

/* gSwapWordsFlag is the flag that says the two swapBlock below are exchanged. */
extern s8 gSwapWordsFlag;
extern unsigned char D_004C8390[];

/* The two 32-bit swapBlock that the swap flag exchanges (fields at +0x38 and +0x44). */
typedef struct SwapWordBlock {
    unsigned char unmodeled_00[0x38];
    s32 swapWordA;
    unsigned char unmodeled_3C[0x44 - 0x3C];
    s32 swapWordB;
} SwapWordBlock;

#define gSwapWordBlock (*(SwapWordBlock *) D_004C8390)

typedef struct Cmd8Block {
    unsigned char packet[8];
} Cmd8Block;

extern const unsigned char gSwapCommandTemplate[];
void scratchpadSendCmd0A();

/* kept: the one-byte struct store keeps the flag in its own alias set (a plain store of 1 schedules differently). */
typedef struct SwapFlag {
    s8 isSwapped;
} SwapFlag;

/* Sends scratchpad command 0x0A with the argument, then exchanges the two words and sets the flag. Does nothing
 * while the flag is already set. */
void swapWordsSendCommand(s32 argument) {
    Cmd8Block command = *(Cmd8Block *) gSwapCommandTemplate;
    SwapWordBlock *swapBlock;
    SwapFlag swapped;
    s32 savedWordA;

    if (!gSwapWordsFlag) {
        *(s32 *) command.packet = argument;
        scratchpadSendCmd0A(&command);
        swapBlock = &gSwapWordBlock;
        swapped.isSwapped = 1;
        *(SwapFlag *) &gSwapWordsFlag = swapped;
        savedWordA = swapBlock->swapWordA;
        swapBlock->swapWordA = swapBlock->swapWordB;
        swapBlock->swapWordB = savedWordA;
    }
}

/* If the flag is set, clears it and exchanges the two swapBlock back. */
void swapWordsRestore(void) {
    s32 savedWordA;
    s32 savedWordB;

    if (gSwapWordsFlag != 0) {
        gSwapWordsFlag = 0;
        savedWordA = gSwapWordBlock.swapWordA;
        savedWordB = gSwapWordBlock.swapWordB;
        gSwapWordBlock.swapWordB = savedWordA;
        gSwapWordBlock.swapWordA = savedWordB;
    }
}

extern s32 gFontEscapeBuffer;
extern s32 gFontNumberBuffer;
void bufferCursorClear();
void func_00221C50();
void func_00225780();
void sceneReset();

void initScratchpadBuffers(void) {
    gFontEscapeBuffer = 0x70000000;
    gFontNumberBuffer = 0x70000100;
    bufferCursorClear();
    sceneReset();
    func_00221C50();
    func_00225780();
}

extern void (*D_004C83D4[])(void);

void completionFlagsReset(void);
void completionCallbacksRun(void);
extern u32 gBlockStatusBits;

/* Resets the completion callbacks, the pad status and the block status counter; the first completionCallback slot becomes
 * completionCallbacksRun, which runs the completion list. */
void completionBlockClear(void) {
    completionFlagsReset();
    padStatusMasksClear();
    gBlockStatusBits = 0;
    D_004C83D4[0] = completionCallbacksRun;
}

float xglSin(float);
extern u8 D_00550B88;
extern u8 D_00550B89;
extern u8 D_00550B8A;
void pxySetHeaderScanEnabled(int enabled);
void xglSleep(void);
void VibrationEngine(void);

/* One block status step: the 16-bit counter moves on, and the three status bytes follow sine waves of 64, 128 and
 * 256 steps, each mapped to 0..128. BLOCK_STATUS_WAVE takes the counter mask and the angle scale of its wave. */
#define BLOCK_STATUS_WAVE(mask, scale) \
    ((int) (xglSin((gBlockStatusBits & (mask)) * 3.14159265f * (scale)) * 64.0f) + 64)

#define BLOCK_STATUS_ADVANCE() \
    gBlockStatusBits = (gBlockStatusBits + 1) & 0xFFFF; \
    D_00550B88 = BLOCK_STATUS_WAVE(0x3F, 0.03125f); \
    D_00550B89 = BLOCK_STATUS_WAVE(0x7F, 0.015625f); \
    D_00550B8A = BLOCK_STATUS_WAVE(0xFF, 0.0078125f)

/* Frame step of the block status: the pad update, then one block status step. */
void statusBlockFrameUpdate(void) {
    pxySetHeaderScanEnabled(1);
    xglSleep();
    VibrationEngine();
    padStatusSampleGroups();
    BLOCK_STATUS_ADVANCE();
}

/* Pointer to the status header record at gPadStatusMasks. */
u8 *getStatusHeader(void) {
    return gPadStatusMasks;
}

void blockStatusBitsSet(s32 newStatusBits) {
    gBlockStatusBits = newStatusBits;
}

/* One block status step without the pad update (same statements as statusBlockFrameUpdate after padStatusSampleGroups). */
void blockStatusAdvance(void) {
    BLOCK_STATUS_ADVANCE();
}

s32 blockStatusBitsGet(void) {
    return gBlockStatusBits;
}

u8 statusByte0Get(void) {
    return D_00550B88;
}

u8 statusByte1Get(void) {
    return D_00550B89;
}

u8 statusByte2Get(void) {
    return D_00550B8A;
}

typedef void (*CompletionCallback)(void);

extern CompletionCallback gCompletionCallbacks[];
extern int gCompletionCallbackCount;

/* kept: the one-member struct view of gCompletionCallbackCount decides the emitted order of the count accesses. */
typedef struct CompletionCount {
    int queuedCallbacks;
} CompletionCount;

#define CALLBACK_COUNT (((CompletionCount *) &gCompletionCallbackCount)->queuedCallbacks)

/* Appends a completionCallback to the completion list and keeps it null-terminated; a null completionCallback clears the list. */
void completionCallbackAdd(CompletionCallback completionCallback) {
    if (completionCallback) {
        gCompletionCallbacks[CALLBACK_COUNT++] = completionCallback;
        gCompletionCallbacks[gCompletionCallbackCount] = 0;
    } else {
        gCompletionCallbackCount = 0;
        gCompletionCallbacks[0] = 0;
    }
}

void xglFontFlush(void);

/* Runs the completion callbacks in order until the null terminator, then clears the list and calls xglFontFlush. */
void completionCallbacksRun(void) {
    int callbackIndex = 0;

    while (gCompletionCallbacks[callbackIndex]) {
        gCompletionCallbacks[callbackIndex++]();
    }
    completionCallbackAdd(0);
    xglFontFlush();
}

/* Command template of swapWordsSendCommand: the 8-byte packet header. */
const unsigned char gSwapCommandTemplate[8] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };

/* Completion callbacks: a fixed table of six entries, cleared at boot. */
CompletionCallback gCompletionCallbacks[6] = { 0 };

/* Block status bits and the wave values, the swap flag and the font buffers. */
u32 gBlockStatusBits = 0;
u8 D_00550B88 = 0;
u8 D_00550B89 = 0;
u8 D_00550B8A = 0;
s8 gSwapWordsFlag = 0;
s32 gFontEscapeBuffer = 0;
s32 gFontNumberBuffer = 0;
int gCompletionCallbackCount = 0;
