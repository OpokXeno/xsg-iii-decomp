#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CB238);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CB410);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CB638);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CB6C8);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CB8B0);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CB8E0);

typedef signed char s8;
typedef int s32;
typedef float f32;

/* The game loop runtime block (gGameLoopRuntimeStorage): the script VM globals the natives write. */
typedef struct GameLoopRuntime {
    s32 loopFlags;              /* 0x00: bit 0x2000 is set or cleared by vmSetFlag2000 */
    unsigned char unk_04[0x10];
    unsigned char colorBytes[3];    /* 0x14: written by vmSetGlobalColor */
    unsigned char unk_17[0x111];
    s32 activeHandle;           /* 0x128: written by vmSetGlobalTag */
    s32 pendingHandle;              /* 0x12C: activeHandle with the 0x06010000 class bits */
} GameLoopRuntime;

/* The record at object + 0x150 that the natives write flags and values into. */
typedef struct SbVmState {
    unsigned char unk_000[0x50];
    s32 objectFlags;           /* 0x1A0: bits set or cleared by sbSetObjectFlagBits */
    unsigned char unk_1A4[0x1C];
    f32 unk_1C0;
    unsigned char unk_1C4[0x8];
    s32 firstWord;
    s32 secondWord;
} SbVmState;

/* Object record the natives write into (the object pointer is the first argument of sbSetObjectFlagBits and friends). */
typedef struct SbVmObject {
    unsigned char unk_000[0x134];
    f32 objectScale;
    unsigned char unk_138[0x18];
    SbVmState vmState;        /* +0x150 */
    unsigned char unk_1D4[0x850 - 0x1D4];
    f32 colorRed;           /* 0x850: set by vmSetColor */
    f32 colorGreen;         /* 0x854 */
    f32 colorBlue;          /* 0x858 */
} SbVmObject;

extern unsigned char gGameLoopRuntimeStorage[];
#define gGameLoopRuntime (*(GameLoopRuntime *) gGameLoopRuntimeStorage)
s32 VM_PLUGIN_CNT_GET();
s32 sbSlotJumpRequest();
s32 vmArgGet();

/* Sets the flag 0x2000 of the global state when argument 1 is nonzero, clears it otherwise. */
s32 vmSetFlag2000(s32 thread) {
    s32 flagsWithBit;
    s32 flagsWithoutBit;

    if (VM_PLUGIN_CNT_GET(thread) != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    if (vmArgGet(thread, 1) != 0) {
        flagsWithBit = gGameLoopRuntime.loopFlags | 0x2000;
        gGameLoopRuntime.loopFlags = flagsWithBit;
    } else {
        flagsWithoutBit = gGameLoopRuntime.loopFlags & ~0x2000;
        gGameLoopRuntime.loopFlags = flagsWithoutBit;
    }
    return 1;
}

void *stateBlockGet();

/* Stores the three colour arguments (0..255) as floats in the colour fields of the object returned by stateBlockGet. */
s32 vmSetColor(s32 thread)
{
  f32 red;
  f32 green;
  f32 blue;
  SbVmObject *scriptObject;
  if (VM_PLUGIN_CNT_GET() != 3)
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  red = vmArgGet(thread, 1);
  green = vmArgGet(thread, 2);
  blue = vmArgGet(thread, 3);
  scriptObject = stateBlockGet();
  scriptObject->colorBlue = blue / 255.0f;
  scriptObject->colorGreen = green / 255.0f;
  scriptObject->colorRed = red / 255.0f;
  return 1;
}

s32 vmSetGlobalColorBytes(s32 thread)
{
  s32 red;
  s32 green;
  s32 blue;
  if (VM_PLUGIN_CNT_GET(thread) != 3)
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  red = vmArgGet(thread, 1) & 0xFF;
  green = vmArgGet(thread, 2) & 0xFF;
  blue = vmArgGet(thread, 3);
  gGameLoopRuntime.colorBytes[1] = green;
  gGameLoopRuntime.colorBytes[2] = blue;
  gGameLoopRuntime.colorBytes[0] = red;
  return 1;
}

s32 vmSetGlobalTag(s32 thread)
{
  s32 tagArg;
  s32 tag;
  if (VM_PLUGIN_CNT_GET(thread) != 1)
  {
    sbSlotJumpRequest(thread, 2);
    return 0;
  }
  tagArg = vmArgGet(thread, 1);
  tag = (tagArg & 0xFFFF) | 0x06010000;
  gGameLoopRuntime.activeHandle = tagArg;
  gGameLoopRuntime.pendingHandle = tag;
  return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CBC98);

/* Sets (setBits != 0) or clears the bits of mask in the flags at 0x1A0 of the object. */
void sbSetObjectFlagBits(SbVmObject *scriptObject, s32 mask, s32 setBits) {
    SbVmState *vmState;

    vmState = &scriptObject->vmState;
    if (setBits != 0) {
        vmState->objectFlags |= mask;
        return;
    }
    vmState->objectFlags &= ~mask;
}

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CBD90);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CC0D0);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CC120);

INCLUDE_ASM("asm/main/nonmatchings/tu088", func_001CC1E8);

void sbSetObjectScale(SbVmObject *scriptObject, f32 scale) {
    scriptObject->objectScale = scale;
    scriptObject->vmState.unk_1C0 = scale * 3.0f;
}

void sbSetObjectFloatAlt(SbVmObject *scriptObject, f32 value) {
    scriptObject->vmState.unk_1C0 = value;
}

void sbSetObjectWordPair(SbVmObject *scriptObject, s32 firstWord, s32 secondWord) {
    SbVmState *vmState;

    vmState = &scriptObject->vmState;
    vmState->firstWord = firstWord;
    vmState->secondWord = secondWord;
}
