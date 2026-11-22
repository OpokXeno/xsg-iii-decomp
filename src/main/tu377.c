#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu377", func_002C7148);

INCLUDE_ASM("asm/main/nonmatchings/tu377", func_002C71C0);

typedef unsigned char u8;

typedef signed char s8;

typedef short s16;

typedef unsigned short u16;

typedef int s32;

/* Voice status block at gSeVoiceStatus; seUpdateVoiceStart (tu387) tests voiceFlags at +0x04 against 0x20 and 0x40. */
typedef struct SeVoiceStatus {
    u16 statusBits00;  /* +0x00 */
    u16 statusBits02;  /* +0x02 */
    u16 voiceFlags;    /* +0x04 */
    u16 statusBits06;  /* +0x06 */
    u8 pad_08[4];
    s8 byte0C;                      /* +0x0C */
    s8 byte0D;                      /* +0x0D */
    s8 byte0E;                      /* +0x0E */
    s8 byte0F;                      /* +0x0F */
} SeVoiceStatus;

extern SeVoiceStatus gSeVoiceStatus[];

SeVoiceStatus *seVoiceStatusGet(void) {
    return gSeVoiceStatus;
}

extern s16 seVoiceCurrentIndex;

void seVoiceStatusReset(void)
{
  seVoiceCurrentIndex = -1;
  gSeVoiceStatus[0].byte0F = 0;
  gSeVoiceStatus[0].byte0E = 0;
  gSeVoiceStatus[0].byte0D = 0;
  gSeVoiceStatus[0].byte0C = 0;
  gSeVoiceStatus[0].statusBits06 = 0;
  gSeVoiceStatus[0].voiceFlags = 0;
  gSeVoiceStatus[0].statusBits02 = 0;
  gSeVoiceStatus[0].statusBits00 = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu377", func_002C72D8);

s32 seVoiceStatusAnyBits(s32 mask) {
    return (gSeVoiceStatus[0].statusBits00 & mask) != 0;
}

extern u16 gSeVoiceStatusBits02[];

s32 seVoiceStatusAnyBits02(s32 mask) {
    return (gSeVoiceStatusBits02[0] & mask) != 0;
}

extern u16 gSeVoiceFlags[];

s32 seVoiceFlagsTest(s32 mask) {
    return (gSeVoiceFlags[0] & mask) != 0;
}

#include "main/tu007.h"

extern PadRecord PadData[];

s32 padCirclePressedAlone(s32 mask) {
    if ((PadData[0].directionHeld & 0xF00F) == 0) {
        if ((PadData[0].directionPressed & mask) != 0) {
            return 1;
        }
    }
    return 0;
}
