#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BB5B0);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BB7F0);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BB8C8);

typedef int s32;
typedef signed char s8;
typedef short s16;
s32 soundSlotsReset();
s32 soundWorkSource(s32, s32, s32);
s32 soundSlotStatus(s32);
void xglSleep(void);

s32 soundWaitAndInitSlots(void)
{
  s32 soundSlotIndex;
  s32 soundWorkSourceCount;
  s32 value;
  soundSlotIndex = 0;
  soundSlotsReset();
  do
  {
    while ((value = soundSlotStatus(soundSlotIndex)) != 0)
    {
      xglSleep();
    }

    soundSlotIndex += 1;
  }
  while (soundSlotIndex < 2);
  soundWorkSourceCount = 0;
  value = 0;
  do
  {
    soundWorkSourceCount += 1;
    soundWorkSource(value, 0, 0);
    value = soundWorkSourceCount;
  }
  while (soundWorkSourceCount < 4);
  return 1;
}

/* .data word at 0x3E46B0 (the game loop runtime block, the same one include/main/tu118.h views). */
typedef struct GameLoopRuntime {
    s32 loopFlags;                  /* +0x00 */
    unsigned char unk_04[0x15C - 0x04];
    s16 effectMode;                   /* +0x15C */
    unsigned char unk_15E[0x160 - 0x15E];
    s32 param160;               /* +0x160 */
    s32 param164;               /* +0x164 */
    s32 param168;               /* +0x168 */
} GameLoopRuntime;

extern unsigned char gGameLoopRuntimeStorage[];
/* The game loop runtime block at gGameLoopRuntimeStorage, read through one typed view. */
#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)
s32 func_001BE470();
struct SefSlotEffectRecord *func_001BEE80();
s32 func_001C0570();

/* The entry func_001BEE80 returns: the id at +0x12 is what func_001BE470 takes. */
typedef struct SefSlotEffectRecord {
    unsigned char unmodeled_00[0x12];
    s16 id;                             /* +0x12 */
} SefSlotEffectRecord;

void sefStartCurrentEffect(void) {
    s32 slotNumber;
    SefSlotEffectRecord *effectRecord;

    if (!(gGameLoopRuntime.loopFlags & 0x400000)) {
        slotNumber = func_001C0570(0);
        if (slotNumber != 0) {
            effectRecord = func_001BEE80(slotNumber + 0x40);
            if (effectRecord != 0) {
                func_001BE470(effectRecord->id, 1);
            }
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBA68);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBB28);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBB80);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBBB8);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBC68);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBCB8);

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBDA0);

void sefSetEffectParams(s32 effectMode, s32 first, s32 second, s32 third, s32 audibleFlag) {
    s32 flagsWithModeBit;
    s32 flagsWithSilentBit;
    s32 flagsAfterModeReset;

    flagsAfterModeReset = (gGameLoopRuntime.loopFlags | 0x10000) & 0xFFFDFF7F;
    gGameLoopRuntime.loopFlags = flagsAfterModeReset;
    if (effectMode > 0) {
        flagsWithModeBit = flagsAfterModeReset | 0x20000;
        gGameLoopRuntime.loopFlags = flagsWithModeBit;
    }
    if (audibleFlag == 0) {
        flagsWithSilentBit = gGameLoopRuntime.loopFlags | 0x80;
        gGameLoopRuntime.loopFlags = flagsWithSilentBit;
    }
    gGameLoopRuntime.effectMode = effectMode;
    gGameLoopRuntime.param160 = first;
    gGameLoopRuntime.param164 = second;
    gGameLoopRuntime.param168 = third;
}

void sefFlagSetBit26(void) {
    s32 flagsWithBit26;

    flagsWithBit26 = gGameLoopRuntime.loopFlags | 0x04000000;
    gGameLoopRuntime.loopFlags = flagsWithBit26;
}

s32 func_0018FAC8(s32);
s32 vmStreamPoll(s32);
s32 func_001B9B88(s32);
s32 func_001BB8C8();
s32 func_A85A20();
s32 func_AA0C30();
s32 xglCdLoadOverlay(s32);
s32 xglRenderClearFrame();

void sceneOverlayReset(void) {
    xglCdLoadOverlay(4);
    func_AA0C30();
    xglCdLoadOverlay(2);
    xglRenderClearFrame();
    func_0018FAC8(0);
    func_001BB8C8();
    vmStreamPoll(1);
    func_001B9B88(1);
    func_A85A20();
}

INCLUDE_ASM("asm/main/nonmatchings/tu065", func_001BBF20);
