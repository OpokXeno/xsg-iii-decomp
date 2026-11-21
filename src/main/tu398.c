#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002DFE48);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002DFEB8);

typedef signed char s8;
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

/* Layout owned by tu378.c (counter with a [start, end) range). */
typedef struct DecodeCounter {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 counter;
    /* 0x08 */ s32 counter_last;
    /* 0x0C */ s32 range_start;
    /* 0x10 */ s32 range_end;
    /* 0x14 */ s32 range_last;
    /* 0x18 */ s32 counter_saved;
    /* 0x1C */ s32 flag_a;
    /* 0x20 */ s32 flag_b;
    /* 0x24 */ s8 flag_c;
} DecodeCounter;

/* Decode state block at gDecodeStateBlock. Only the offsets this TU touches are named. The same block is also reached
 * through gDecodeObjectBlock (base + 0x17B0) and through gDecodeProgressStorage (the DecodeCounter at base + 0x1210); the macros below
 * keep each access on the original symbol, so the object code stays the same. */
#define offsetof(type, field) ((u32) &((type *) 0)->field)
#define CONTAINER_OF(member, type, field) ((type *) ((member) - offsetof(type, field)))

typedef struct DecodeState {
    /* 0x0000 */ u8 unk_0000[0x1210];
    /* 0x1210 */ DecodeCounter decodeProgress;
    /* 0x1238 */ s32 decodeProgressEnd;
    /* 0x123C */ s32 slotColumn;
    /* 0x1240 */ s32 decodeProgressStart;
    /* 0x1244 */ u8 unk_1244[0x24];
    /* 0x1268 */ s32 decodeProgressCount;
    /* 0x126C */ s32 slotRow;
    /* 0x1270 */ s32 slotGrid[0x134];
    /* 0x1740 */ s32 streamHandle;
    /* 0x1744 */ u8 unk_1744[0xC];
    /* 0x1750 */ s32 decodeResult;
    /* 0x1754 */ s32 buffer1754;
    /* 0x1758 */ s32 buffer1758;
    /* 0x175C */ u8 unk_175C[0x4];
    /* 0x1760 */ s32 buffer1760;
    /* 0x1764 */ s32 slotRowWord;
    /* 0x1768 */ s32 flagB;
    /* 0x176C */ s32 flagA;
    /* 0x1770 */ s32 valueSrc;
    /* 0x1774 */ s32 valueDst;
    /* 0x1778 */ u8 unk_1778[0x38];
    /* 0x17B0 */ u8 objectBlock[0x678]; /* gDecodeObjectBlock */
    /* 0x1E28 */ s32 decodeMode;
    /* 0x1E2C */ s32 started;
    /* 0x1E30 */ s32 ready;
} DecodeState;

/* Owner pointer of a source object (the owner is the decode state block). */
typedef struct DecodeSource {
    /* 0x00 */ u8 unk_00[0x10];
    /* 0x10 */ DecodeState *owner;
} DecodeSource;

#define gDecodeState (gDecodeStateBlock[0])
#define gDecodeObjectState (*CONTAINER_OF(gDecodeObjectBlock, DecodeState, objectBlock))
#define gDecodeProgressState (*CONTAINER_OF(gDecodeProgressStorage, DecodeState, decodeProgress))
#define gDecodeStatus (gDecodeStatusStorage[0])
#define gGameLoopFlags (gGameLoopRuntimeStorage[0])
#define gVmHandle (gVmHandleStorage[0])
#define gDecodeFlagC (gDecodeFlagCStorage[0])
#define gDecodeFlagD (gDecodeFlagDStorage[0])
#define gDecodeSecondaryFlag (gDecodeSecondaryFlagStorage[0])

extern s32 gDecodeStatusStorage[];
extern s32 gDecodeFlagCStorage[];
extern s32 gDecodeFlagDStorage[];
extern s32 gDecodeSecondaryFlagStorage[];
extern s32 gGameLoopRuntimeStorage[];
extern s32 gVmHandleStorage[];

s32 decodeGetStatus(void) {
    return gDecodeStatus;
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002DFF10);

void decodeCloseStream(void);
void decodeFreeBuffers(void);
void func_001B6AB0();

void decodeShutdown(void) {
    extern void func_001B6798();
    extern void decodeCloseStream();
    extern void decodeFreeBuffers();
    s32 newVmFlags;

    newVmFlags = gGameLoopFlags | 0x400;
    gGameLoopFlags = newVmFlags;
    decodeCloseStream();
    decodeFreeBuffers();
    func_001B6798(gVmHandle);
    func_001B6AB0();
}

extern DecodeState gDecodeStateBlock[];
extern unsigned char gDecodeObjectBlock[];
s32 func_001B5F28();
void xcBlockClear(void *block);
s32 func_001B70F0();

void decodeOpenStream(void)
{
    extern s32 func_001B6520();
    s32 commandBuffer[8];
    s32 newStreamHandle;

    newStreamHandle = func_001B6520(0xC800);
    gDecodeState.streamHandle = newStreamHandle;
    xcBlockClear(&commandBuffer[0]);
    commandBuffer[2] = gDecodeState.streamHandle;
    commandBuffer[1] = 0x0900000B;
    func_001B70F0(&commandBuffer[0]);
    func_001B5F28(0, 0, 0);
}

void decodeCloseStream(void)
{
    extern s32 func_001B6798();

    if (gDecodeState.streamHandle != 0)
    {
        func_001B6798(gDecodeState.streamHandle);
    }
    gDecodeState.streamHandle = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E00B8);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E0190);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E02F8);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E0368);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E03F8);

#include "main/tu378.h"

extern unsigned char gDecodeProgressStorage[];
#define gDecodeProgress (*(DecodeCounter *) gDecodeProgressStorage)


void decodeCounterReset(void) {
    decodeCounterSetStart(&gDecodeProgress, 0);
    decodeCounterSetEnd(&gDecodeProgress, 0);
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E06C0);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E0858);

void decodeNoop(void) {

}

s32 func_001E8AF0();
s32 func_001E8CB8(s32);
s32 func_0028D670();
void decodeCounterSetFlagA(DecodeCounter *counter, s32 value);
void decodeCounterSetFlagB(DecodeCounter *counter, s32 value);
s32 func_002E0190(s32 *slotRowWords, s32 row, s32 value);
s32 func_002E0EE0();
s32 func_002E16F8();

void decodeStateInit(void)
{
    extern s32 func_002E1778();
    s32 slotRowWords[0xC];

    gDecodeState.decodeResult = func_0028D670();
    gDecodeState.ready = 1;
    gDecodeState.decodeMode = 0;
    func_002E16F8();
    func_002E1778();
    func_002E0190(&slotRowWords[0], gDecodeState.slotRow,
                  gDecodeState.slotGrid[gDecodeState.slotRow * 0x64 + gDecodeState.slotColumn]);
    gDecodeState.slotRowWord = slotRowWords[4];
    func_002E0EE0();
    decodeCounterSetFlagA(&gDecodeState.decodeProgress, 2);
    decodeCounterSetFlagB(&gDecodeState.decodeProgress, 0);
    func_001E8AF0();
    func_001E8CB8(2);
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E0B68);

void decodeTryStart(void);
s32 pxyEnableFlag8();
s32 pxyEnableFlag800000();
s32 sefFrameUpdate();
s32 func_002912B0();
s32 decodeCounterGetCount(DecodeCounter *counter);
s32 decodeCounterGetStart(DecodeCounter *counter);
s32 func_002CF1A0(u8 *base);
s32 func_002E0B68();

void decodeStateRefresh(void)
{
    extern s32 func_002911C0();
    extern s32 func_002E18A8();
    s32 value;

    func_002CF1A0(gDecodeObjectBlock);
    value = gDecodeObjectState.started;
    if (value != 0)
    {
        func_002E18A8();
        func_002E0B68();
        value = gDecodeObjectState.valueSrc;
        gDecodeObjectState.valueDst = value;
        sefFrameUpdate();
        pxyEnableFlag800000();
        pxyEnableFlag8();
        func_002911C0();
        func_002912B0();
    }
    else
    {
        decodeTryStart();
    }
    gDecodeProgressState.decodeProgressStart = decodeCounterGetStart(&gDecodeProgress);
    gDecodeProgressState.decodeProgressEnd = decodeCounterGetEnd(&gDecodeProgress);
    gDecodeProgressState.decodeProgressCount = decodeCounterGetCount(&gDecodeProgress);
}

void decodeFlushOnFlagC(void) {
    if (gDecodeFlagC != 0) {
        func_002912B0();
    }
}

void func_002E1E30();
void func_002E24B8();

void decodeUpdate(void) {
    func_002E1E30();
    if (gDecodeFlagD != 0) {
        func_002E24B8();
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E0D40);

void decodeAllocBuffers(void) {
    extern s32 func_001B6520(s32);

    gDecodeState.buffer1754 = func_001B6520(0x190000);
    gDecodeState.buffer1758 = func_001B6520(0x214000);
    gDecodeState.buffer1760 = func_001B6520(0x7800);
}

s32 XenoActorReleaseAll();

void decodeFreeBuffers(void) {
    extern s32 func_001B6798(s32);

    func_001B6798(gDecodeState.buffer1760);
    func_001B6798(gDecodeState.buffer1758);
    func_001B6798(gDecodeState.buffer1754);
    XenoActorReleaseAll();
}

void decodeSetModeFromFlags(DecodeSource *decodeSource)
{
    DecodeState *owner;

    owner = decodeSource->owner;
    if (gDecodeState.flagA != 0)
    {
        owner->decodeMode = 2;
        return;
    }
    if (gDecodeState.flagB != 0)
    {
        owner->decodeMode = 3;
        return;
    }
    owner->decodeMode = 4;
}

void decodeSetModeFromSecondaryFlag(DecodeSource *decodeSource) {
    DecodeState *owner;

    owner = decodeSource->owner;
    if (gDecodeSecondaryFlag != 0) {
        owner->decodeMode = 3;
        return;
    }
    owner->decodeMode = 4;
}

s32 func_001B60B8(s32 channel);

void decodeCheckChannelDone(DecodeSource *decodeSource) {
    DecodeState *owner;

    owner = decodeSource->owner;
    if (func_001B60B8(0) == 1) {
        owner->decodeMode = 4;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E0EE0);

void decodeStart(void);
void func_002E1158();

void decodeTryStart(void)
{
    extern void decodeStart();

    gDecodeState.started = 0;
    if (gDecodeState.decodeMode != 4)
    {
        return;
    }
    func_002E1158();
    gDecodeState.started = 1;
    decodeStart();
}

extern s8 gVsyncsPerFrame;
void func_002E1300();

void decodeStart(void) {
    extern void func_002911C0();
    extern void func_002E1778();
    extern void func_002E18A8();
    func_002E1300();
    func_002911C0();
    gVsyncsPerFrame = 2;
    func_002E1778();
    func_002E18A8();
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1158);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1300);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1428);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1498);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1520);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E16F8);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1778);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E18A8);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1DE0);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E1E30);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E21B8);

void decodeSetFlag(s32 newDecodeFlagD) {
    gDecodeFlagD = newDecodeFlagD;
}

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E24B8);

INCLUDE_ASM("asm/main/nonmatchings/tu398", func_002E26D8);
