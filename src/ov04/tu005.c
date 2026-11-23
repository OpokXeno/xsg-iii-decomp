/*
 * OV04 original TU 5: 0x00a8deb0..0x00a90940 (14 functions)
 */
#include "common.h"
#include "main/tu007.h"

typedef short s16;
typedef int s32;
typedef float f32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
s32 func_00A99788(void);

/* Screen mode control block at gScreenModeControlStorage: screen mode at +0x16, countdown at +0x1A, script step flags at +0x00. Only the fields these functions touch are named. */
typedef struct ScreenModeControl {
    s32 modeBits;                        /* 0x00 */
    u8 unk_04[0x08 - 0x04];
    s32 pulseTimer;                             /* 0x08: frames, counted up every update */
    s32 waveStep;                               /* 0x0C: step of the title wave, 0..31 */
    u8 unk_10[0x16 - 0x10];
    s16 screenMode;                          /* 0x16 */
    u8 unk_18[0x1A - 0x18];
    s16 countdownTicks;                        /* 0x1A: frames, -1 once the countdown has ended */
    u8 unk_1C[0x26 - 0x1C];
    s16 unlockedStages;                         /* 0x26 */
    u8 unk_28[0x94 - 0x28];
    s16 unlockStep;                             /* 0x94: next unlock announcement, 26 when done */
    u8 unk_96[0x9E - 0x96];
    s16 soundTestIndex;                         /* 0x9E */
    u8 unk_A0[0xFC - 0xA0];
    s16 soundTestHandle;                        /* 0xFC: voice of the playing sound test track */
    u8 unk_FE[0x17C - 0xFE];
    s32 unk_17C;                        /* 0x17C */
} ScreenModeControl;

extern unsigned char gScreenModeControlStorage[];
#define gScreenModeControl (*(ScreenModeControl *) gScreenModeControlStorage)

void screenModeClear(void)
{
    gScreenModeControl.screenMode = 0;
    gScreenModeControl.countdownTicks = 0;
    func_00A99788();
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8DED8);

void func_00A989B0(s32 id, s32 row, s32 x, s32 y, s32 size, s32 flags, s32 target, f32 scale);
s32 func_00AA32C8(s32 value);

void screenModeSetup(void)
{
    func_00AA32C8(-1);
    gScreenModeControl.unk_17C = 0;
    func_00A989B0(0x380, 0x2A, 0x20, 0x140, 0x20, 0, -1, 1.0f);
    gScreenModeControl.screenMode = 1;
    gScreenModeControl.modeBits |= 0x800000;
    gScreenModeControl.countdownTicks = 0;
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8DF98);

void screenCountdownStart(s16 countdownTicks)
{
    gScreenModeControl.screenMode = 2;
    gScreenModeControl.countdownTicks = countdownTicks;
}

s32 func_00A8E110(void);
void ov04SetValueNotifyA(s16 value);

void screenCountdownTick(void)
{
    u16 remaining;
    s32 remainingAsSigned;
    unsigned int countdownStep;

    remaining = gScreenModeControl.countdownTicks;
    countdownStep = 1;
    gScreenModeControl.countdownTicks -= countdownStep;
    remainingAsSigned = remaining << 0x10;
    if (remainingAsSigned <= 0)
    {
        func_00A8E110();
        ov04SetValueNotifyA(0x14);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8E110);

#define PAD_L2 0x0001
#define PAD_R2 0x0002
#define PAD_L1 0x0004
#define PAD_R1 0x0008
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE 0x0020
#define PAD_CROSS 0x0040
#define PAD_SQUARE 0x0080
#define PAD_SELECT 0x0100
#define PAD_START 0x0800
#define PAD_UP 0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN 0x4000
#define PAD_LEFT 0x8000

/* Button and pad bits above the held halfword when the pad halfwords are read as one doubleword. */
#define PAD_PRESSED(buttons) ((buttons) << 16)

#define SCREEN_MODE_SOUND_TEST 0x10000000

#define COUNTDOWN_ENDED -1
#define COUNTDOWN_FRAMES 60

/* Bits of the option word gOptionWords[509] that the cheat codes toggle. */
#define OPTION_STAGE_ONMEMORY 0x2000
#define OPTION_EDIT_UNLIMITED 0x4000
#define OPTION_MASTER_CODE 0x8000

#define FIRST_UNLOCK_STEP_ENTRY 0x1BF
#define SOUND_TEST_ENTRY 0x30B
#define SOUND_TEST_TRACKS 53
#define SOUND_TEST_MUSIC_TRACKS 43
#define UNLOCK_STEP_COUNT 26

typedef union TransformPosition {
    Vector4 vector;
    s32 ivector[4];
} TransformPosition;

/* One 0x80-byte record of the table at gPlacedRecordTable (see ov04/tu008); only the fields this function touches are named. */
typedef struct PlacedRecordView {
    u8 unmodeled_00[8];
    f32 height;                 /* 0x08 */
    u8 unmodeled_0C[0x20];
    f32 pulseScale;             /* 0x2C */
    u8 unmodeled_30[0x20];
    TransformPosition color;    /* 0x50 */
    TransformPosition glow;     /* 0x60 */
    u8 unmodeled_70[0x10];
} PlacedRecordView;

/* One 0xC0-byte transform of the pool at gTransformPool (see ov04/tu004). */
typedef struct TransformSlotView {
    s16 kind;                   /* 0x00: model kind, -1 hidden */
    u8 unmodeled_02[0xBE];
} TransformSlotView;

extern TransformSlotView *gTransformPool;
extern PlacedRecordView *gPlacedRecordTable;
extern struct PadRecord D_4B3470[2]; /* both controller ports */
extern s16 gWaveColorPatterns[][3];
extern s16 gSoundTestTracks[][2];
extern s8 gCheatCodesEnabled;
extern s8 gMasterCodeActive;
extern s32 gOptionWords[];
extern s32 D_00AADE14;

s32 Get_Rnd(s32 minimum, s32 maximum);
void soundCueByKind(s16 cueGroup, s16 voiceOffset);
s32 intMaxCapped(s32 leftValue, s32 rightValue, s32 limit);
void func_00A88550(s16 entry, s32 mode);
f32 Get_ParaboraValue(s16 arcStepIndex, s32 arcStepCount, f32 arcStartValue, f32 arcEndValue, f32 height);
void recordSetScaleXY(s16 recordNumber, f32 scaleX, f32 scaleY);
void ov04CallEntryRoutine(s16 entryId);
void boardLiftsUpdate(void);
void func_00A969B8(s32 button);
s32 func_00A969F8(char *code);
void func_00A98730(char *text, s32 x, s32 y, s32 width, s32 height, s32 flags, s32 frames, f32 scale);
void ov04SlotsClearUpper(void);
void func_0018F098(s32 handle);
s16 func_00A9D770(s32 voice, s16 bank, s16 track);
void func_00A9D1A0(s32 enable);
void func_00A8F780(void);
void func_00A97170(s32 mode);
void func_00A986D0(s32 first, s32 count);

/* One frame of the pad-display screen: runs the countdown, colours and waves the button records, steps the lifts
 * (boardLiftsUpdate), announces unlocks, pulses the record of each pressed button, runs the sound test and checks the
 * cheat codes typed with the pad (FREE ALL STAGE, NOW SOUND TEST, EDIT LIMITED/UNLIMITED, SECRET STAGE,
 * STAGE RAPIDLOAD/ONMEMORY, MASTER CODE). START leaves the screen. */
void screenModePadDisplayUpdate(void)
{
    s16 colorPattern;
    PlacedRecordView *colorRecord;
    double waveDistance;
    s32 buttonRecordIndex;
    s32 announcementReady = 0;
    s32 codeButtonPressed = 0;

    gTransformPool[4].kind = -1;
    if (gScreenModeControl.countdownTicks == 10) {
        soundCueByKind(Get_Rnd(0, 3), 1);
    }
    if (gScreenModeControl.countdownTicks != COUNTDOWN_ENDED) {
        if (gScreenModeControl.countdownTicks++ >= COUNTDOWN_FRAMES) {
            func_00A88550(intMaxCapped(gScreenModeControl.unlockedStages, 1, 6) + 0x5C, 1);
            gScreenModeControl.modeBits |= 0x300;
            gScreenModeControl.countdownTicks = COUNTDOWN_ENDED;
            gScreenModeControl.screenMode = 3;
            colorPattern = Get_Rnd(0, 7);
            colorRecord = gPlacedRecordTable;
            for (buttonRecordIndex = 0; buttonRecordIndex < 15; buttonRecordIndex++, colorRecord++) {
                colorRecord->color.vector.x = 160 - (160 - gWaveColorPatterns[colorPattern][0]) * buttonRecordIndex / 15;
                colorRecord->color.vector.y = 160 - (160 - gWaveColorPatterns[colorPattern][1]) * buttonRecordIndex / 15;
                colorRecord->color.vector.z = 160 - (160 - gWaveColorPatterns[colorPattern][2]) * buttonRecordIndex / 15;
                colorRecord->glow.vector.x = 8.0f;
                colorRecord->glow.vector.y = 8.0f;
                colorRecord->glow.vector.z = 8.0f;
            }
        }
    } else {
        gPlacedRecordTable[10].height = Get_ParaboraValue(gScreenModeControl.waveStep % 32, 32, 174.0f, 174.0f, -64.0f);
        waveDistance = 16 - gScreenModeControl.waveStep;
        if (waveDistance < 0.0) {
            waveDistance = -waveDistance;
        }
        gPlacedRecordTable[10].pulseScale = waveDistance * 0.1f / 16 + 0.9f;
        gScreenModeControl.waveStep = (gScreenModeControl.waveStep + 1) % 32;
        recordSetScaleXY(Get_Rnd(0, 16) + 42, Get_Rnd(50, 150) / 125.0f, 4.0f);
        recordSetScaleXY(Get_Rnd(0, 16) + 42, Get_Rnd(50, 150) / 125.0f, 4.0f);
    }
    boardLiftsUpdate();
    gScreenModeControl.pulseTimer++;
    if ((gScreenModeControl.modeBits & SCREEN_MODE_SOUND_TEST) && gScreenModeControl.pulseTimer % 120 == 0) {
        for (; gScreenModeControl.unlockStep < UNLOCK_STEP_COUNT; gScreenModeControl.unlockStep++) {
            switch (gScreenModeControl.unlockStep) {
            case 0:
                if (!(D_00AADE14 & 0x80)) {
                    announcementReady = 1;
                }
                break;
            case 1:
            case 2:
                if (D_00AADE14 & 0x80) {
                    announcementReady = 1;
                }
                break;
            case 3:
            case 4:
                if (gScreenModeControl.unlockedStages == 1) {
                    announcementReady = 1;
                }
                break;
            case 5:
                if (gScreenModeControl.unlockedStages == 2) {
                    announcementReady = 1;
                }
                break;
            case 6:
                if (gScreenModeControl.unlockedStages == 3) {
                    announcementReady = 1;
                }
                break;
            case 7:
                if (gScreenModeControl.unlockedStages == 4) {
                    announcementReady = 1;
                }
                break;
            case 8:
                if (gScreenModeControl.unlockedStages == 5) {
                    announcementReady = 1;
                }
                break;
            case 9:
                if (gScreenModeControl.unlockedStages == 6) {
                    announcementReady = 1;
                }
                break;
            default:
                announcementReady = 1;
                break;
            }
            if (announcementReady == 1) {
                ov04CallEntryRoutine(FIRST_UNLOCK_STEP_ENTRY + gScreenModeControl.unlockStep++);
                break;
            }
        }
    }
    if (D_4B3470[0].pressed & PAD_SELECT) {
        ov04CallEntryRoutine(Get_Rnd(0, 28) + 0x1D9);
        gScreenModeControl.unlockStep = UNLOCK_STEP_COUNT;
    }
#define PRESS_RECORD(port, button, index) \
    if (D_4B3470[port].pressed & (button)) { \
        gPlacedRecordTable[index].pulseScale = 2.0f; \
        recordSetScaleXY(index, 1.0f, 4.0f); \
    }
#define PRESS_CODE_RECORD(button, index) \
    if (D_4B3470[0].pressed & (button)) { \
        gPlacedRecordTable[index].pulseScale = 2.0f; \
        recordSetScaleXY(index, 1.0f, 4.0f); \
        func_00A969B8(button); \
        codeButtonPressed = 1; \
    }
    PRESS_CODE_RECORD(PAD_TRIANGLE, 0);
    PRESS_CODE_RECORD(PAD_CROSS, 1);
    PRESS_CODE_RECORD(PAD_SQUARE, 2);
    PRESS_CODE_RECORD(PAD_CIRCLE, 3);
    PRESS_CODE_RECORD(PAD_UP, 4);
    PRESS_CODE_RECORD(PAD_DOWN, 5);
    PRESS_RECORD(0, PAD_LEFT, 6);
    PRESS_RECORD(0, PAD_RIGHT, 7);
    PRESS_CODE_RECORD(PAD_L1, 8);
    PRESS_RECORD(0, PAD_L2, 9);
    PRESS_CODE_RECORD(PAD_R1, 10);
    PRESS_CODE_RECORD(PAD_R2, 11);
    PRESS_CODE_RECORD(PAD_SELECT, 12);
    PRESS_RECORD(1, PAD_TRIANGLE, 2);
    PRESS_RECORD(1, PAD_CROSS, 3);
    PRESS_RECORD(1, PAD_SQUARE, 4);
    PRESS_RECORD(1, PAD_CIRCLE, 5);
    PRESS_RECORD(1, PAD_UP, 6);
    PRESS_RECORD(1, PAD_DOWN, 7);
    PRESS_RECORD(1, PAD_LEFT, 8);
    PRESS_RECORD(1, PAD_RIGHT, 9);
    PRESS_RECORD(1, PAD_L1, 10);
    PRESS_RECORD(1, PAD_L2, 11);
    PRESS_RECORD(1, PAD_R1, 12);
    PRESS_RECORD(1, PAD_R2, 13);
    PRESS_RECORD(1, PAD_SELECT, 14);
#undef PRESS_RECORD
#undef PRESS_CODE_RECORD
    if (gScreenModeControl.modeBits & SCREEN_MODE_SOUND_TEST) {
        if (D_4B3470[0].pressed & PAD_UP) {
            gScreenModeControl.soundTestIndex = (gScreenModeControl.soundTestIndex + SOUND_TEST_TRACKS - 1) % SOUND_TEST_TRACKS;
            ov04CallEntryRoutine(gScreenModeControl.soundTestIndex + SOUND_TEST_ENTRY);
        }
        if (D_4B3470[0].pressed & PAD_DOWN) {
            gScreenModeControl.soundTestIndex = (gScreenModeControl.soundTestIndex + 1) % SOUND_TEST_TRACKS;
            ov04CallEntryRoutine(gScreenModeControl.soundTestIndex + SOUND_TEST_ENTRY);
        }
        if (gScreenModeControl.soundTestIndex < SOUND_TEST_MUSIC_TRACKS) {
            if (D_4B3470[0].heldAndPressed & PAD_PRESSED(PAD_CIRCLE | PAD_CROSS)) {
                func_0018F098(gScreenModeControl.soundTestHandle);
                gScreenModeControl.soundTestHandle = func_00A9D770(-1, gSoundTestTracks[gScreenModeControl.soundTestIndex][0],
                                                                   gSoundTestTracks[gScreenModeControl.soundTestIndex][1]);
            }
            if (D_4B3470[0].heldAndPressed & PAD_PRESSED(PAD_TRIANGLE | PAD_SQUARE)) {
                func_0018F098(gScreenModeControl.soundTestHandle);
            }
        } else {
            if (D_4B3470[0].pressed & PAD_CIRCLE) {
                soundCueByKind(0, gScreenModeControl.soundTestIndex - SOUND_TEST_MUSIC_TRACKS + 1);
            }
            if (D_4B3470[0].pressed & PAD_CROSS) {
                soundCueByKind(1, gScreenModeControl.soundTestIndex - SOUND_TEST_MUSIC_TRACKS + 1);
            }
            if (D_4B3470[0].pressed & PAD_TRIANGLE) {
                soundCueByKind(2, gScreenModeControl.soundTestIndex - SOUND_TEST_MUSIC_TRACKS + 1);
            }
            if (D_4B3470[0].pressed & PAD_SQUARE) {
                soundCueByKind(3, gScreenModeControl.soundTestIndex - SOUND_TEST_MUSIC_TRACKS + 1);
            }
        }
    }
    if (codeButtonPressed == 1 && gCheatCodesEnabled == 1) {
        if (func_00A969F8("LRLRLR88224466Z") == 1) {
            soundCueByKind(0, 3);
            ov04SlotsClearUpper();
            gScreenModeControl.unlockedStages = 6;
            func_00A98730("FREE ALL STAGE!", 0x54, 0x4C, 0x40, 0x18, 0, 0x78, 0.75f);
            ov04CallEntryRoutine(0x236);
        }
        if (func_00A969F8("UDLR8246Z") == 1) {
            soundCueByKind(0, 4);
            gScreenModeControl.modeBits |= SCREEN_MODE_SOUND_TEST;
            func_00A98730("NOW SOUND TEST!", 0x54, 0x4C, 0x40, 0x18, 0, 0x78, 0.75f);
            gScreenModeControl.soundTestIndex = 0;
            gScreenModeControl.unlockStep = UNLOCK_STEP_COUNT;
            ov04CallEntryRoutine(0x233);
            ov04CallEntryRoutine(SOUND_TEST_ENTRY);
        }
        if (func_00A969F8("4646LRLR4646LRLRZ") == 1) {
            if (gOptionWords[509] & OPTION_EDIT_UNLIMITED) {
                soundCueByKind(1, 2);
                gOptionWords[509] &= ~OPTION_EDIT_UNLIMITED;
                func_00A98730("EDIT LIMITED!", 0x54, 0x64, 0x40, 0x18, 0, 0x78, 0.75f);
                ov04CallEntryRoutine(0x238);
            } else {
                soundCueByKind(1, 1);
                gOptionWords[509] |= OPTION_EDIT_UNLIMITED;
                func_00A98730("EDIT UNLIMITED!", 0x54, 0x4C, 0x40, 0x18, 0, 0x78, 0.75f);
                ov04CallEntryRoutine(0x237);
            }
        }
        if (func_00A969F8("UUDDLRLR62") == 1) {
            soundCueByKind(1, 7);
            gScreenModeControl.unlockedStages = intMaxCapped(gScreenModeControl.unlockedStages, 0, 8);
            func_00A98730("SECRET STAGE!", 0x54, 0x64, 0x40, 0x18, 0, 0x78, 0.75f);
        }
        if (func_00A969F8("2648LLLLRRRRZ") == 1) {
            if (gOptionWords[509] & OPTION_STAGE_ONMEMORY) {
                soundCueByKind(1, 6);
                gOptionWords[509] &= ~OPTION_STAGE_ONMEMORY;
                func_00A98730("STAGE RAPIDLOAD!", 0x54, 0x40, 0x40, 0x18, 0, 0x78, 0.75f);
                ov04CallEntryRoutine(0x234);
            } else {
                soundCueByKind(0, 5);
                gOptionWords[509] |= OPTION_STAGE_ONMEMORY;
                func_00A98730("STAGE ONMEMORY!", 0x54, 0x4C, 0x40, 0x18, 0, 0x78, 0.75f);
                ov04CallEntryRoutine(0x235);
            }
        }
        if (func_00A969F8("882264U8U4U2U6D8D4D2D6LRZLRZ") == 1) {
            soundCueByKind(0, 5);
            gCheatCodesEnabled = 1;
            gMasterCodeActive = 1;
            gOptionWords[509] |= OPTION_MASTER_CODE;
            func_00A9D1A0(1);
            func_00A98730("MASTER CODE ON!", 0x54, 0x4C, 0x40, 0x18, 0, 0x78, 0.75f);
        }
    }
    if (D_4B3470[0].pressed & PAD_START) {
        func_00A8F780();
        func_00A97170(0);
        func_00A986D0(0, 14);
        func_00A986D0(42, 73);
    }
}

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8F780);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8F928);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8FBD8);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A8FE30);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A90348);

INCLUDE_ASM("asm/nonmatchings/ov04/tu005", func_00A90478);
