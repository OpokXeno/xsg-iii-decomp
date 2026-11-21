#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;

/* The 0xF0-byte state record at gTextCommandStateStorage (fields by the offsets the functions use). */
typedef struct TextCommandState {
    u8 commandFlags;                       /* 0x00: bit 0x2 set by textCommandStateSetKinds */
    u8 unk_01;
    u8 mirrorFlag;                  /* 0x02: nonzero negates the first value (with mirrorSuppressFlags bit 0x100 clear) */
    u8 mirrorSecondMode;                        /* 0x03: >= 3 negates the second value */
    u8 unk_04[0x7C];
    s32 mirrorSuppressFlags;                     /* 0x80: bit 0x100 */
    u8 unk_84[0x10];
    f32 keptScaledValue;                     /* 0x94: written by textCommandStateSetScaledValue unless commandFlags bit 0x2 is set */
    f32 lastScaledValue;                     /* 0x98: written by textCommandStateSetScaledValue */
    u8 unk_9C[0x44];
    f32 scale;                      /* 0xE0 */
    s8 primaryTextKind;                      /* 0xE4: set by textCommandStateSetKinds */
    s8 secondaryTextKind;                      /* 0xE5: set by textCommandStateSetKinds */
    u8 unk_E6[2];
    void *callback;                 /* 0xE8: func_00265398 */
    u8 unk_EC[4];
} TextCommandState;

extern unsigned char gTextCommandStateStorage[];
extern unsigned char func_00265398[];
void *memset();
/* The state record at gTextCommandStateStorage, read through one typed view. */
#define gTextCommandState (*(TextCommandState *) gTextCommandStateStorage)

void textCommandStateReset(void) {
    memset(gTextCommandStateStorage, 0, 0xF0);
    gTextCommandState.callback = func_00265398;
}

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025E470);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025E698);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025E838);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025E8F0);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025EBF0);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025ED28);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025EF60);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025EFF0);

void stateMirrorScalePair(f32 *xPosition, f32 *yPosition)
{
  f32 negatedSecond;
  f32 negatedFirst;
  f32 scaledFirst;

  if (gTextCommandState.mirrorSecondMode >= 3U)
  {
    negatedSecond = -(*yPosition);
    *yPosition = negatedSecond;
  }
  if ((!(gTextCommandState.mirrorSuppressFlags & 0x100)) && (gTextCommandState.mirrorFlag != 0))
  {
    negatedFirst = -(*xPosition);
    *xPosition = negatedFirst;
  }
  scaledFirst = (*xPosition) * gTextCommandState.scale;
  *xPosition = scaledFirst;
}

void textCommandStateSetScaledValue(f32 unscaledValue)
{
  f32 scaledValue;
  f32 mirroredValue;

  mirroredValue = unscaledValue;
  if ((!(gTextCommandState.mirrorSuppressFlags & 0x100)) && (gTextCommandState.mirrorFlag != 0))
  {
    mirroredValue = -mirroredValue;
  }
  scaledValue = mirroredValue * gTextCommandState.scale;
  gTextCommandState.lastScaledValue = scaledValue;
  if (!(gTextCommandState.commandFlags & 2))
  {
    gTextCommandState.keptScaledValue = scaledValue;
  }
}

void textCommandStateSetKinds(s32 primaryTextKind, s32 secondaryTextKind)
{
  gTextCommandState.secondaryTextKind = secondaryTextKind;
  gTextCommandState.primaryTextKind = primaryTextKind;
  gTextCommandState.commandFlags |= 2;
}

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025F158);

INCLUDE_ASM("asm/main/nonmatchings/tu244", func_0025F2C8);
