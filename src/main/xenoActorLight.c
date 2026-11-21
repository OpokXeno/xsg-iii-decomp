#include "common.h"

typedef unsigned char u8;
typedef int s32;

/* Header of an XST file: the tag "XST " read as one word (low byte first) and as bytes. */
typedef union XenoActorLightXstHeader {
    s32 tag;
    u8 tagBytes[4];
} XenoActorLightXstHeader;

/* Global state of the light actor loader at xenoActorLightLoader[0]. */
typedef struct XenoActorLightState {
    XenoActorLightXstHeader *xstHeader; /* 0x00: header of the loaded XST file, 0 when none */
    s32 unk_04;
    s32 unk_08;
    u8 unk_0C[0x4];
    u8 lightBlockZero[0x180];           /* 0x10 */
    u8 lightBlocks[33][0x180];            /* 0x190 */
} XenoActorLightState;

extern unsigned char D_004AE160[];
extern XenoActorLightState xenoActorLightLoader[];
extern s32 xenoActorLightFlags[];
extern XenoActorLightXstHeader *xenoActorLightSecondaryHeader[];
extern s32 xenoActorLightRequestedCount[];
extern unsigned char D_004ADFE0[];
extern unsigned char xenoActorLightBadTagFormat[];

typedef struct XenoActorLightSlot {
    s32 state;
    u8 unk_04[0x1C];
} XenoActorLightSlot;

extern XenoActorLightSlot D_004B1300[];

void func_002922B0(u8 *block);
s32 printf(const char *format, ...);
void pxySetHeaderScanEnabled(s32 mode);
void func_00293240(void);
void xenoActorLightApplyFlag(void);

u8 *xenoActorLightScratchGet(void) {
    return D_004AE160;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_002922B0);

s32 xenoActorLightAdd(s32 left, s32 right) {
    return left + right;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_002923C8);

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_00292A70);

void xenoActorLightFlagsClear(void) {
    xenoActorLightFlags[0] = 0;
}

void xenoActorLightStateInit(void) {
    s32 actorBlockIndex;

    xenoActorLightLoader[0].xstHeader = 0;
    xenoActorLightLoader[0].unk_04 = 0;
    xenoActorLightLoader[0].unk_08 = 0;
    func_002922B0(xenoActorLightLoader[0].lightBlockZero);
    for (actorBlockIndex = 0; actorBlockIndex < 33; actorBlockIndex++) {
        func_002922B0(xenoActorLightLoader[0].lightBlocks[actorBlockIndex]);
    }
}

void xenoActorLightSetPrimaryHeader(XenoActorLightXstHeader *xstHeader) {
    if (xstHeader->tag != 0x20545358) {
        printf(xenoActorLightBadTagFormat, xstHeader->tagBytes[0], xstHeader->tagBytes[1], xstHeader->tagBytes[2], xstHeader->tagBytes[3]);
    }
    xenoActorLightLoader[0].xstHeader = xstHeader;
}

void xenoActorLightSetCounter(s32 requestedCount) {
    s32 clamped;

    clamped = requestedCount;
    if (requestedCount < 0) {
        clamped = 0;
    }
    xenoActorLightRequestedCount[0] = clamped;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_002931E0);

u8 *xenoActorLightTableGet(void) {
    return D_004ADFE0;
}

s32 xenoActorLightHasHeader(void) {
    return xenoActorLightLoader[0].xstHeader != 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_00293240);

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_002933D8);

void xenoActorLightApplyFlag(void) {
    if (xenoActorLightFlags[0] & 1) {
        pxySetHeaderScanEnabled(0);
    } else {
        pxySetHeaderScanEnabled(1);
    }
}

s32 xenoActorLightFlagIsSet(void)
{
  if ((xenoActorLightFlags[0] & 1) != 0)
  {
    return 1;
  }
  return 0;
}

void xenoActorLightSetSecondaryHeader(XenoActorLightXstHeader *xstHeader) {
    if (xstHeader->tag != 0x20545358) {
        printf(xenoActorLightBadTagFormat, xstHeader->tagBytes[0], xstHeader->tagBytes[1], xstHeader->tagBytes[2], xstHeader->tagBytes[3]);
    }
    xenoActorLightSecondaryHeader[0] = xstHeader;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_002937C8);

void xenoActorLightResetIfLoaded(void) {
    if (xenoActorLightLoader[0].xstHeader != 0) {
        xenoActorLightLoader[0].unk_08 = 0;
        func_002922B0(xenoActorLightLoader[0].lightBlockZero);
        func_00293240();
        xenoActorLightApplyFlag();
    }
}

void xenoActorLightStateClear(void) {
    xenoActorLightLoader[0].xstHeader = 0;
    xenoActorLightLoader[0].unk_04 = 0;
}

void xenoActorLightSlotClear(s32 lightSlotIndex) {
    D_004B1300[lightSlotIndex].state = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_00293A08);

INCLUDE_ASM("asm/main/nonmatchings/xenoActorLight", func_00293AB8);
