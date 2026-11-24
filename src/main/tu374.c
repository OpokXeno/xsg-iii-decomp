#include "common.h"

typedef signed char s8;

typedef int s32;

typedef unsigned char u8;

typedef float f32;

typedef short s16;

extern s32 seVoiceEnabled[];

s32 seVoiceIsEnabled(void) {
    return seVoiceEnabled[0];
}

/* The voice state block at D_005EF3E0. */
typedef struct SeVoiceState {
    f32 x;                              /* +0x00: -1.0f until set */
    f32 y;                              /* +0x04: -1.0f until set */
    u8 unk_08[0x10 - 0x08];
    u8 planeHostState[0x110 - 0x10];          /* +0x10: passed to planeHostInit, planeHostSetWordE0 and planeHostSetWordE4 */
    u8 voiceEntry[0x210 - 0x110];       /* +0x110: passed to njStringSetCounter and func_002D8358 */
    s32 unk_210;
    s32 unk_214;
    s32 unk_218;
    s32 unk_21C;
    s32 unk_220;                        /* -1 after init */
    s32 spawnOffsetX;
    s32 unk_228;
    s32 spawnRow;
    s32 isBusy;                        /* non-zero: the voice is busy */
    s32 spawnPairEnabled;
    u8 stringBuffer[0x800];
    s8 spawnPriority;
} SeVoiceState;

extern SeVoiceState D_005EF3E0[];
s32 planeHostInit();
s32 planeHostSetWordE4();
s32 planeHostSetWordE0();
void *memset(void *destination, int value, unsigned int count);

void seVoiceStateInit(void)
{
  SeVoiceState *voiceState = D_005EF3E0;
  u8 *planeHostState;

  planeHostState = voiceState->planeHostState;
  voiceState->x = -1.0f;
  voiceState->unk_210 = 0;
  voiceState->y = -1.0f;
  voiceState->unk_220 = -1;
  voiceState->unk_214 = 0;
  voiceState->unk_218 = 0;
  voiceState->unk_21C = 0;
  voiceState->spawnRow = 0;
  voiceState->unk_228 = 0;
  voiceState->spawnPairEnabled = 0;
  voiceState->spawnPriority = 0;
  voiceState->isBusy = 0;
  memset(voiceState->stringBuffer, 0, 0x800);
  planeHostInit(planeHostState);
  planeHostSetWordE0(planeHostState, 0x18);
  planeHostSetWordE4(planeHostState, 2);
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C2E58);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C3590);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C3668);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C3700);

s32 func_002C3700();
s32 func_002C91B8(void *);
s32 njPlaneSpawnPair(s32, s32, s8);
s32 njStringSetCounter(void *, void *);
s32 func_002D8358(void *);

s32 seVoiceUpdate(void)
{
  SeVoiceState *voiceState;
  s32 voiceStatus;
  void *voiceEntry;

  voiceState = D_005EF3E0;
  voiceStatus = voiceState->isBusy;
  voiceEntry = voiceState->voiceEntry;
  if (voiceStatus != 0)
  {
    func_002C3700();
    func_002C91B8(voiceState->planeHostState);
    njStringSetCounter(voiceEntry, voiceState->stringBuffer);
    func_002D8358(voiceEntry);
    voiceStatus = voiceState->spawnPairEnabled;
    if (voiceStatus != 0)
    {
      return njPlaneSpawnPair((voiceState->x + 20.0f) + voiceState->spawnOffsetX, (voiceState->y + 13.0f) + ((voiceState->spawnRow * 0x18) - 0x18), voiceState->spawnPriority);
    }
  }
  return voiceStatus;
}

void seVoiceEnable(void) {
    extern s32 seVoiceStartDefault();
    seVoiceStartDefault(4);
    seVoiceEnabled[0] = 1;
}

void seVoiceDisable(void) {
    extern void seVoiceStartDefault();
    seVoiceStartDefault(5);
    seVoiceEnabled[0] = 0;
}

void njPlaneInit();

#include "main/njPlane.h"

void njPlaneSetOffset(NjPlane *, f32, f32);
void njPlaneSetOrigin(NjPlane *, f32, f32);
void njPlaneSetLevel(NjPlane *, f32);
void func_002D10D0();

/* A plane record with the 0xC0-byte transform buffer around it: the plane is the first 0x80 bytes. */
typedef struct SeVoicePlane {
    NjPlane plane;
    unsigned char planeTail[0xC0 - 0x80];
} SeVoicePlane;

void seVoiceDrawShadowPair(s32 x, s32 y, s32 z, s32 level) {
    SeVoicePlane transformA;
    SeVoicePlane transformB;
    f32 levelF;

    njPlaneInit(&transformA.plane);
    levelF = level;
    njPlaneSetOffset(&transformA.plane, x, y - 2);
    njPlaneSetOrigin(&transformA.plane, z, 5.0f);
    njPlaneSetTint(&transformA.plane, 0.0f, 0.0f, 0.0f);
    njPlaneSetLevel(&transformA.plane, levelF);
    njPlaneInit(&transformB.plane);
    njPlaneSetOffset(&transformB.plane, x + 2, y);
    njPlaneSetOrigin(&transformB.plane, z - 4, 1.0f);
    njPlaneSetTint(&transformB.plane, 165.0f, 165.0f, 165.0f);
    njPlaneSetLevel(&transformB.plane, levelF);
    func_002D10D0(&transformA.plane);
    func_002D10D0(&transformB.plane);
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C3C20);

void seVoiceSlotClear(void *voiceSlot) {
    memset(voiceSlot, 0, 0x40);
}

s32 XenoActorMulVector(s32, s32, void *, s32);

/* One voice slot (0x40 bytes each in seVoiceSlots): the actor, a transform buffer at +0x10 and the linked flag at +0x34. */
typedef struct SeVoiceSlot {
    s32 actor;                          /* +0x00 */
    u8 unk_04[0x10 - 0x04];
    u8 transformBuffer[0x34 - 0x10];             /* +0x10: passed to XenoActorMulVector */
    s32 linked;                         /* +0x34 */
    u8 unk_38[0x40 - 0x38];
} SeVoiceSlot;

void seVoiceSlotSetActor(SeVoiceSlot *voiceSlot, s32 actor) {
    voiceSlot->actor = actor;
    if ((actor != 0) && (voiceSlot->linked == 0)) {
        XenoActorMulVector(actor, 2, voiceSlot->transformBuffer, 1);
        voiceSlot->linked = 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C40D8);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C4368);

void seVoiceSlotClear(void *voiceSlot);
extern unsigned char seVoiceSlots[];

void seVoiceSlotsClear(void)
{
  s32 voiceSlotIndex;
  u8 *nextVoiceSlot;
  u8 *voiceSlot;
  nextVoiceSlot = seVoiceSlots;
  voiceSlot = seVoiceSlots;
  for (voiceSlotIndex = 0; voiceSlotIndex < 5; voiceSlotIndex++)
  {
    seVoiceSlotClear(voiceSlot);
    nextVoiceSlot += 0x40;
    voiceSlot = nextVoiceSlot;
  }

}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C45E8);

void seVoiceSlotsReset(void)
{
  s32 voiceSlotIndex;
  u8 *nextVoiceSlot;
  u8 *voiceSlot;
  nextVoiceSlot = seVoiceSlots;
  voiceSlot = seVoiceSlots;
  for (voiceSlotIndex = 0; voiceSlotIndex < 5; voiceSlotIndex++)
  {
    seVoiceSlotClear(voiceSlot);
    nextVoiceSlot += 0x40;
    voiceSlot = nextVoiceSlot;
  }

}

s32 func_002C40D8();

void seVoiceSlotsRefresh(void)
{
  s32 voiceSlotIndex;
  u8 *nextVoiceSlot;
  u8 *voiceSlot;
  nextVoiceSlot = seVoiceSlots;
  voiceSlot = seVoiceSlots;
  for (voiceSlotIndex = 0; voiceSlotIndex < 5; voiceSlotIndex++)
  {
    func_002C40D8(voiceSlot);
    nextVoiceSlot += 0x40;
    voiceSlot = nextVoiceSlot;
  }

}

s32 func_002C4368();

void seVoiceSlotsRelease(void)
{
  s32 voiceSlotIndex;
  u8 *nextVoiceSlot;
  u8 *voiceSlot;
  nextVoiceSlot = seVoiceSlots;
  voiceSlot = seVoiceSlots;
  for (voiceSlotIndex = 0; voiceSlotIndex < 5; voiceSlotIndex++)
  {
    func_002C4368(voiceSlot);
    nextVoiceSlot += 0x40;
    voiceSlot = nextVoiceSlot;
  }

}

typedef struct SeVoiceCursor {
    f32 x;                              /* +0x00 */
    f32 y;                              /* +0x04 */
    u8 unk_08[0x80 - 0x08];
    u8 subBlock[0x460 - 0x80];          /* +0x80: passed to func_002C66D0 */
    s32 pageTimer;                      /* +0x460: frames on the current page */
    s32 page;                           /* +0x464: page shown by seVoiceCursorDrawStatusPanel */
    s32 alpha;                          /* +0x468 */
    s32 cursorEnabled;                  /* +0x46C */
} SeVoiceCursor;

extern SeVoiceCursor seVoiceCursor[];

void seVoiceCursorSetEnable(s32 enableCursor)
{
  SeVoiceCursor *cursor = seVoiceCursor;
  s32 cursorEnable;
  cursorEnable = enableCursor;
  cursor->page = 0;
  cursor->pageTimer = 0;
  cursor->cursorEnabled = cursorEnable;
}

extern f32 seVoiceCursorStartPos[];
s32 func_002C66D0(void *);

s32 seVoiceCursorReset(void)
{
  SeVoiceCursor *cursor = seVoiceCursor;
  f32 posX;
  f32 posY;
  posX = seVoiceCursorStartPos[0];
  posY = seVoiceCursorStartPos[1];
  cursor->x = posX;
  cursor->alpha = 0;
  cursor->cursorEnabled = 0;
  cursor->y = posY;
  return func_002C66D0(cursor->subBlock);
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C47A0);

typedef unsigned int u32;

typedef unsigned short u16;

typedef unsigned long long u64;

/* The status record the voice cursor screen reads (gFallbackSlotRecord): its title, the character id, three
 * current/maximum gauges, the bitmask of the pages to cycle through and the name passed to the cursor entry. */
typedef struct SeVoiceStatusRecord {
    u8 unk_000[0x8];
    u8 *title;                          /* +0x008 */
    u8 unk_00C[0x124 - 0x00C];
    s32 characterId;                    /* +0x124 */
    s32 gaugeA;                         /* +0x128 */
    s32 gaugeAMax;                      /* +0x12C */
    s32 gaugeB;                         /* +0x130 */
    s32 gaugeBMax;                      /* +0x134 */
    s32 gaugeC;                         /* +0x138 */
    s32 gaugeCMax;                      /* +0x13C */
    u8 unk_140[0xCC8 - 0x140];
    s32 pageMask;                       /* +0xCC8: bit n shows page n; -2 shows the single default page */
    u8 unk_CCC[0xCE2 - 0xCCC];
    u8 name[0xE];                       /* +0xCE2 */
} SeVoiceStatusRecord;

extern SeVoiceStatusRecord gFallbackSlotRecord;

/* The character entry func_00A84758 returns: a counter, two values and three item slots. The record is
 * only halfword aligned, so its leading word is read unaligned. */
typedef unsigned int UnalignedU32 __attribute__((aligned(1)));

typedef struct SeVoiceCharacterStats {
    UnalignedU32 counter;               /* +0x00 */
    s16 valueA;                         /* +0x04 */
    s16 valueB;                         /* +0x06 */
    u8 unk_08[0xB - 0x8];
    u8 itemIndex[3];                    /* +0x0B: 1-based */
    u8 itemVariant[3];                  /* +0x0E */
} SeVoiceCharacterStats;

/* The character info func_00A841D0 returns: the mode in the low two bits of flags, a marker in bit 4. */
typedef struct SeVoiceCharacterInfo {
    u8 unk_00[0x1A];
    u16 flags;                          /* +0x1A */
} SeVoiceCharacterInfo;

void fontQueueRectTwoColors(s16 *frame, s32 colorTop, s32 colorBottom, s32 mode);
void fontQueueColorState(s32 color);
void func_0023C588(s32 x, s32 y, u8 *text, s32 size);
void func_0023CE40(s32 x, s32 y, s32 size);
void xglFontDrawDigits(s32 x, s32 y, s32 value, s32 size);
void xglFontDrawUnsignedNumber(s32 x, s32 y, u32 value, s32 size);
void xglFontSetDigitStep(s32 step);
s32 xglFontGetColorIndex(void);
s32 xglFontGetStateMask(void);
void xglFontSetColorIndex(s32 colorIndex);
void func_002D6978(u8 *text);
void func_002D6AA8(u8 *text, u64 color);
s32 func_002D6BA0(u8 *text);
void njStringSetZoom(u8 *text, s32 zoomX, s32 zoomY);
void njStringSetAlpha(u8 *text, s32 alpha);
void njStringSetFlags(u8 *text, s32 styleFlags);
void njStringSetPosition(u8 *text, f32 x, f32 y);
void njPlaneSetMode(NjPlane *plane, s32 mode);
void *func_00A84758(s32 characterId);
s16 *func_00A847F0(s32 characterId);
void *func_00A841D0(s32 characterId);
u8 *func_00A85028(s32 messageIndex);
u8 *func_00A85070(s32 group, s32 index, s32 variant);
u8 *func_002C7A98(s32 itemIndex, s32 variant);
u8 *sceneLinkLookupEntry(s32 index);
struct SeVoiceEntry;
void seVoiceEntrySetName(struct SeVoiceEntry *voiceEntry, u8 *nameBytes);
void func_002C6840(struct SeVoiceEntry *voiceEntry);

/* Layout point n of seVoiceCursorStartPos, relative to the cursor. */
#define SE_VOICE_LAYOUT_X(point) (seVoiceCursor->x + seVoiceCursorStartPos[(point) * 2])
#define SE_VOICE_LAYOUT_Y(point) (seVoiceCursor->y + seVoiceCursorStartPos[(point) * 2 + 1])
/* A screen pixel as a GS primitive coordinate (12.4 fixed point, centred and snapped). */
#define SE_VOICE_GS_X(x) ((((x) * 16) + 0x6FF8) & 0xFFF8)
#define SE_VOICE_GS_Y(y) ((((y) * 16) + 0x71F8) & 0xFFF8)

/* A screen pixel as an unsnapped GS primitive coordinate (12.4 fixed point, centred). */
static __inline__ s32 seVoiceGsX(s32 x)
{
    return x * 16 + 0x6FF8;
}

static __inline__ s32 seVoiceGsY(s32 y)
{
    return y * 16 + 0x71F8;
}

/* Text and digit positions: layout point converted to a snapped GS coordinate; the offsets are added after the
 * float position is truncated to a pixel (the _SHIFTED forms add a float shift before it). */
#define SE_VOICE_TEXT_X(point) SE_VOICE_GS_X((s32) SE_VOICE_LAYOUT_X(point))
#define SE_VOICE_TEXT_Y(point) SE_VOICE_GS_Y((s32) SE_VOICE_LAYOUT_Y(point))
#define SE_VOICE_TEXT_Y_BELOW(point, pixels) SE_VOICE_GS_Y((s32) SE_VOICE_LAYOUT_Y(point) + (pixels))
#define SE_VOICE_TEXT_X_SHIFTED(point, shift) SE_VOICE_GS_X((s32) (SE_VOICE_LAYOUT_X(point) + (shift)))
#define SE_VOICE_TEXT_X_SHIFTED_INT(point, pixels) SE_VOICE_GS_X((s32) SE_VOICE_LAYOUT_X(point) + (pixels))

#define SE_VOICE_COLOR(rgb) (((u64) seVoiceCursor->alpha << 24) | (rgb))

/* Draws the character status panel of the voice cursor from gFallbackSlotRecord: shaded halves, title, kind, the
 * gauges, the cycling page name, up to three item slots and the four arrow planes; compact drops the third row. */
void seVoiceCursorDrawStatusPanel(s8 compact)
{
    s16 frame[4];
    u8 *pageNames[16];
    u8 valueText[0x100];
    u8 labelText[0x100];
    SeVoicePlane arrows[4];
    SeVoiceCharacterStats *characterStats;
    s16 *characterKindWord;
    SeVoiceCharacterInfo *characterInfo;
    s32 savedColorIndex;
    s32 savedStateMask;
    u8 *modeIcon;
    u8 *markerIcon;
    u8 *kindName;
    u8 *itemName;
    s32 lineOffset;
    s32 loopIndex;
    s32 pageMask;
    s32 pageCount;
    s32 alpha;

    modeIcon = 0;
    markerIcon = 0;
    savedColorIndex = xglFontGetColorIndex();
    savedStateMask = xglFontGetStateMask();
    memset(pageNames, 0, sizeof(pageNames));
    lineOffset = 24;
    if (compact != 0) {
        lineOffset = 0;
    }
    kindName = 0;
    func_002D6978(valueText);
    njStringSetZoom(valueText, 15, 15);
    njStringSetAlpha(valueText, seVoiceCursor->alpha);
    njStringSetFlags(valueText, 0x80C);
    func_002D6978(labelText);
    njStringSetZoom(labelText, 15, 15);
    njStringSetAlpha(labelText, seVoiceCursor->alpha);
    njStringSetFlags(labelText, 0x80C);
    for (loopIndex = 0; loopIndex < 4; loopIndex++) {
        njPlaneInit(&arrows[loopIndex].plane);
        njPlaneSetLevel(&arrows[loopIndex].plane, seVoiceCursor->alpha);
        njPlaneSetMode(&arrows[loopIndex].plane, 2);
    }

    characterStats = func_00A84758(gFallbackSlotRecord.characterId);
    characterKindWord = func_00A847F0(gFallbackSlotRecord.characterId);
    characterInfo = func_00A841D0(gFallbackSlotRecord.characterId);
    if (characterInfo != 0) {
        if ((characterInfo->flags & 3) == 0) {
            modeIcon = func_00A85028(0x42);
        } else if ((characterInfo->flags & 3) == 2) {
            modeIcon = func_00A85028(0x43);
        } else if ((characterInfo->flags & 3) == 1) {
            modeIcon = func_00A85028(0x44);
        } else {
            modeIcon = 0;
        }
        if (characterInfo->flags & 0x10) {
            markerIcon = func_00A85028(0x66);
        } else {
            markerIcon = 0;
        }

        pageMask = gFallbackSlotRecord.pageMask;
        pageCount = 0;
        if (pageMask == -2) {
            pageNames[0] = func_00A85028(0x67);
        } else {
            for (loopIndex = 1; loopIndex < 16; loopIndex++) {
                if ((pageMask >> loopIndex) & 1) {
                    pageNames[pageCount++] = func_00A85070(0, loopIndex, 0);
                }
            }
        }

        seVoiceCursor->pageTimer++;
        if (seVoiceCursor->pageTimer >= 31) {
            seVoiceCursor->page++;
            if (seVoiceCursor->page > pageCount - 1) {
                seVoiceCursor->page = 0;
            }
            seVoiceCursor->pageTimer = 0;
        }
    }

    xglFontSetColorIndex(0xFFFF01);
    alpha = seVoiceCursor->alpha;
    if (alpha > 96) {
        alpha = 96;
    }
    if (alpha < 0) {
        alpha = 0;
    }
    frame[0] = seVoiceGsX((s32) seVoiceCursor->x);
    frame[1] = seVoiceGsY((s32) seVoiceCursor->y);
    frame[2] = seVoiceGsX((s32) seVoiceCursor->x + 256);
    frame[3] = seVoiceGsY((s32) seVoiceCursor->y + 239);
    fontQueueRectTwoColors(frame, alpha << 24, 0, 0x101);
    frame[0] = seVoiceGsX((s32) seVoiceCursor->x + 256);
    frame[1] = seVoiceGsY((s32) seVoiceCursor->y);
    frame[2] = seVoiceGsX((s32) seVoiceCursor->x + 512);
    frame[3] = seVoiceGsY((s32) seVoiceCursor->y + 239);
    fontQueueRectTwoColors(frame, alpha << 24, 0, 0x1010);

    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(1), SE_VOICE_LAYOUT_Y(1));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x808080));
    njStringSetCounter(valueText, gFallbackSlotRecord.title);
    func_002D8358(valueText);

    if (characterKindWord != 0) {
        s32 kindMask = *characterKindWord;

        kindMask &= 0xF0;

        if (kindMask == 0x10) {
            kindName = func_00A85028(0x47);
        }
        if (kindMask == 0x20) {
            kindName = func_00A85028(0x48);
        }
        if (kindMask == 0x30) {
            kindName = func_00A85028(0x49);
        }
        if (kindMask == 0) {
            kindName = func_00A85028(0x46);
        }
        if (kindMask == 0x40) {
            kindName = func_00A85028(0x6B);
        }
    }
    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(11), SE_VOICE_LAYOUT_Y(11) + lineOffset);
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x808080));
    njStringSetCounter(valueText, kindName);
    func_002D8358(valueText);

    njStringSetPosition(labelText, SE_VOICE_LAYOUT_X(29), SE_VOICE_LAYOUT_Y(29));
    func_002D6AA8(labelText, SE_VOICE_COLOR(0x545934));
    njStringSetCounter(labelText, sceneLinkLookupEntry(0x2C));
    func_002D8358(labelText);

    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(32), SE_VOICE_LAYOUT_Y(32));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x545934));
    njStringSetCounter(valueText, sceneLinkLookupEntry(0x2E));
    func_002D8358(valueText);
    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(33), SE_VOICE_LAYOUT_Y(32));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x545934));
    njStringSetCounter(valueText, sceneLinkLookupEntry(0x30));
    func_002D8358(valueText);
    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(32), SE_VOICE_LAYOUT_Y(33));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x545934));
    njStringSetCounter(valueText, sceneLinkLookupEntry(0x2F));
    func_002D8358(valueText);
    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(33), SE_VOICE_LAYOUT_Y(33));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x545934));
    njStringSetCounter(valueText, sceneLinkLookupEntry(0x2D));
    func_002D8358(valueText);

    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(31), SE_VOICE_LAYOUT_Y(31));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x545934));
    njStringSetCounter(valueText, sceneLinkLookupEntry(0x2D));
    func_002D8358(valueText);
    njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(31) + 55.0f, SE_VOICE_LAYOUT_Y(31));
    func_002D6AA8(valueText, SE_VOICE_COLOR(0x808080));
    njStringSetCounter(valueText, pageNames[seVoiceCursor->page]);
    func_002D8358(valueText);

    if (characterStats != 0) {
        u8 missingName[2] = "-";

        itemName = func_002C7A98(characterStats->itemIndex[0] - 1, characterStats->itemVariant[0]);
        if (itemName == 0) {
            itemName = missingName;
        }
        njStringSetZoom(valueText, 14, 14);
        njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(26) + 16.0f, SE_VOICE_LAYOUT_Y(26) - 5.0f);
        func_002D6AA8(valueText, SE_VOICE_COLOR(0x808080));
        njStringSetCounter(valueText, itemName);
        func_002D8358(valueText);

        itemName = func_002C7A98(characterStats->itemIndex[1] - 1, characterStats->itemVariant[1]);
        if (itemName == 0) {
            itemName = missingName;
        }
        njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(27) + 16.0f, SE_VOICE_LAYOUT_Y(27) - 5.0f);
        func_002D6AA8(valueText, SE_VOICE_COLOR(0x808080));
        njStringSetCounter(valueText, itemName);
        func_002D8358(valueText);

        if (compact == 0) {
            itemName = func_002C7A98(characterStats->itemIndex[2] - 1, characterStats->itemVariant[2]);
            if (itemName == 0) {
                itemName = missingName;
            }
            njStringSetPosition(valueText, SE_VOICE_LAYOUT_X(28) + 16.0f, SE_VOICE_LAYOUT_Y(28) - 5.0f);
            func_002D6AA8(valueText, SE_VOICE_COLOR(0x808080));
            njStringSetCounter(valueText, itemName);
            func_002D8358(valueText);
        }
    }

    xglFontFreeBuffer();
    fontQueueColorState(SE_VOICE_COLOR(0x545934));
    func_0023C588(SE_VOICE_TEXT_X(3), SE_VOICE_TEXT_Y(3), sceneLinkLookupEntry(0), 13);
    if (compact == 0) {
        func_0023C588(SE_VOICE_TEXT_X(4), SE_VOICE_TEXT_Y(4), sceneLinkLookupEntry(1), 13);
        func_0023C588(SE_VOICE_TEXT_X(5), SE_VOICE_TEXT_Y(5), sceneLinkLookupEntry(2), 13);
    }
    func_0023C588(SE_VOICE_TEXT_X(6), SE_VOICE_TEXT_Y_BELOW(6, lineOffset), sceneLinkLookupEntry(0x20), 13);
    func_0023C588(SE_VOICE_TEXT_X(7), SE_VOICE_TEXT_Y_BELOW(7, lineOffset), sceneLinkLookupEntry(0x2B), 13);
    func_0023C588(SE_VOICE_TEXT_X(8), SE_VOICE_TEXT_Y_BELOW(8, lineOffset), sceneLinkLookupEntry(0x21), 13);
    func_0023C588(SE_VOICE_TEXT_X(9), SE_VOICE_TEXT_Y_BELOW(9, lineOffset), sceneLinkLookupEntry(0x22), 13);
    func_0023C588(SE_VOICE_TEXT_X(10), SE_VOICE_TEXT_Y_BELOW(10, lineOffset), sceneLinkLookupEntry(0x23), 13);
    func_0023C588(SE_VOICE_TEXT_X(25), SE_VOICE_TEXT_Y(25), sceneLinkLookupEntry(0x24), 13);
    fontQueueColorState(SE_VOICE_COLOR(0x562E2D));
    func_0023C588(SE_VOICE_TEXT_X(26), SE_VOICE_TEXT_Y(26), sceneLinkLookupEntry(0x25), 13);
    func_0023C588(SE_VOICE_TEXT_X(27), SE_VOICE_TEXT_Y(27), sceneLinkLookupEntry(0x26), 13);
    if (compact == 0) {
        func_0023C588(SE_VOICE_TEXT_X(28), SE_VOICE_TEXT_Y(28), sceneLinkLookupEntry(0x27), 13);
    }

    fontQueueColorState(SE_VOICE_COLOR(0x545934));
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(3, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y(3), ":", 13);
    if (compact == 0) {
        func_0023C588(SE_VOICE_TEXT_X_SHIFTED(4, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y(4), ":", 13);
        func_0023C588(SE_VOICE_TEXT_X_SHIFTED(5, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y(5), ":", 13);
    }
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(6, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y_BELOW(6, lineOffset), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(7, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y_BELOW(7, lineOffset), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(8, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y_BELOW(8, lineOffset), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(9, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y_BELOW(9, lineOffset), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(10, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y_BELOW(10, lineOffset), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(25, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y(25), ":", 13);
    func_0023C588(SE_VOICE_GS_X((s32) (SE_VOICE_LAYOUT_X(29) + func_002D6BA0(labelText) + 4.0f)), SE_VOICE_TEXT_Y_BELOW(29, 6), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED(31, seVoiceCursorStartPos[4]), SE_VOICE_TEXT_Y_BELOW(31, 6), ":", 13);

    fontQueueColorState(SE_VOICE_COLOR(0x808080));
    xglFontSetDigitStep(10);
    xglFontDrawDigits(SE_VOICE_TEXT_X(12), SE_VOICE_TEXT_Y(12), gFallbackSlotRecord.gaugeA, 16);
    func_0023CE40(SE_VOICE_TEXT_X(13), SE_VOICE_TEXT_Y(13), 16);
    xglFontDrawUnsignedNumber(SE_VOICE_TEXT_X(14), SE_VOICE_TEXT_Y(14), gFallbackSlotRecord.gaugeAMax, 16);
    if (compact == 0) {
        xglFontDrawDigits(SE_VOICE_TEXT_X(15), SE_VOICE_TEXT_Y(15), gFallbackSlotRecord.gaugeB, 16);
        func_0023CE40(SE_VOICE_TEXT_X(16), SE_VOICE_TEXT_Y(16), 16);
        xglFontDrawUnsignedNumber(SE_VOICE_TEXT_X(17), SE_VOICE_TEXT_Y(17), gFallbackSlotRecord.gaugeBMax, 16);
        xglFontDrawDigits(SE_VOICE_TEXT_X(18), SE_VOICE_TEXT_Y(18), gFallbackSlotRecord.gaugeC, 16);
        func_0023CE40(SE_VOICE_TEXT_X(19), SE_VOICE_TEXT_Y(19), 16);
        xglFontDrawUnsignedNumber(SE_VOICE_TEXT_X(20), SE_VOICE_TEXT_Y(20), gFallbackSlotRecord.gaugeCMax, 16);
    }
    if (modeIcon != 0) {
        func_0023C588(SE_VOICE_TEXT_X(21), SE_VOICE_TEXT_Y_BELOW(21, lineOffset), modeIcon, 15);
    }
    if (markerIcon != 0) {
        func_0023C588(SE_VOICE_TEXT_X(14), SE_VOICE_TEXT_Y_BELOW(21, lineOffset), markerIcon, 15);
    }
    if (characterStats != 0) {
        xglFontDrawUnsignedNumber(SE_VOICE_TEXT_X(22), SE_VOICE_TEXT_Y_BELOW(22, lineOffset), characterStats->counter, 16);
        xglFontDrawUnsignedNumber(SE_VOICE_TEXT_X(23), SE_VOICE_TEXT_Y_BELOW(23, lineOffset), characterStats->valueA, 16);
        xglFontDrawUnsignedNumber(SE_VOICE_TEXT_X(24), SE_VOICE_TEXT_Y_BELOW(24, lineOffset), characterStats->valueB, 16);
    }

    fontQueueColorState(SE_VOICE_COLOR(0x808080));
    njPlaneSetOffset(&arrows[0].plane, SE_VOICE_LAYOUT_X(32) - 24.0f, SE_VOICE_LAYOUT_Y(32) + 4.0f);
    njPlaneSetOffset(&arrows[1].plane, SE_VOICE_LAYOUT_X(33) - 24.0f, SE_VOICE_LAYOUT_Y(32) + 4.0f);
    njPlaneSetOffset(&arrows[2].plane, SE_VOICE_LAYOUT_X(32) - 24.0f, SE_VOICE_LAYOUT_Y(33) + 4.0f);
    njPlaneSetOffset(&arrows[3].plane, SE_VOICE_LAYOUT_X(33) - 24.0f, SE_VOICE_LAYOUT_Y(33) + 4.0f);
    func_002CFFF0(&arrows[0].plane, 0x79, 0);
    func_002CFFF0(&arrows[1].plane, 0x78, 0);
    func_002CFFF0(&arrows[2].plane, 0x76, 0);
    func_002CFFF0(&arrows[3].plane, 0x77, 0);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED_INT(32, -8), SE_VOICE_TEXT_Y_BELOW(32, 6), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED_INT(33, -8), SE_VOICE_TEXT_Y_BELOW(32, 6), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED_INT(32, -8), SE_VOICE_TEXT_Y_BELOW(33, 6), ":", 13);
    func_0023C588(SE_VOICE_TEXT_X_SHIFTED_INT(33, -8), SE_VOICE_TEXT_Y_BELOW(33, 6), ":", 13);
    seVoiceEntrySetName((struct SeVoiceEntry *) seVoiceCursor->subBlock, gFallbackSlotRecord.name);
    func_002C6840((struct SeVoiceEntry *) seVoiceCursor->subBlock);
    fontQueueColorState(savedStateMask);
    xglFontSetColorIndex(savedColorIndex);
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C6098);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C61C0);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C61E8);

/* The block at seVoiceBank; its flag at +0x994 gates the cleanup in seVoiceBankRelease. */
typedef struct SeVoiceBank {
    u8 unk_000[0x10];
    u8 planeRecords[12][0xC0];              /* +0x10: twelve sub-blocks, 0xC0 apart */
    u8 unk_910[0x994 - 0x910];
    s32 active;                         /* +0x994 */
} SeVoiceBank;

extern SeVoiceBank seVoiceBank[];

#define gSeVoiceBank (seVoiceBank[0])

s32 xglFontFreeBuffer();
s32 func_002CFFF0();
s32 func_002D0828();

void seVoiceBankRelease(void)
{
  if (gSeVoiceBank.active == 0)
  {
    return;
  }
  xglFontFreeBuffer();
  func_002CFFF0(gSeVoiceBank.planeRecords[0], 0x25, 0);
  func_002CFFF0(gSeVoiceBank.planeRecords[1], 0x25, 0);
  func_002CFFF0(gSeVoiceBank.planeRecords[2], 0x25, 0);
  func_002CFFF0(gSeVoiceBank.planeRecords[3], 0x25, 0);
  func_002D0828(gSeVoiceBank.planeRecords[4], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[5], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[6], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[7], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[8], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[9], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[10], 0x26, 0);
  func_002D0828(gSeVoiceBank.planeRecords[11], 0x26, 0);
}

extern s32 seCurrentVoice[];

void seVoiceSetCurrent(s32 voiceId) {
    s32 currentVoice;

    currentVoice = voiceId;
    seCurrentVoice[0] = currentVoice;
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C66D0);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C6758);

/* The voice entry block at +0x3D0: the id halfword, a five-byte name and the priority byte at +0x3D7. */
typedef struct SeVoiceEntry {
    u8 unk_000[0x3D0];
    s16 id;                             /* +0x3D0 */
    u8 name[0x3D7 - 0x3D2];             /* +0x3D2 */
    s8 priority;                        /* +0x3D7 */
} SeVoiceEntry;

void seVoiceEntrySetPriority(SeVoiceEntry *voiceEntry, s8 priority) {
    voiceEntry->priority = priority;
}

void seVoiceEntrySetId(SeVoiceEntry *voiceEntry, s16 id) {
    voiceEntry->id = id;
}

void seVoiceEntrySetName(SeVoiceEntry *voiceEntry, u8 *nameBytes) {
    s32 charIndex;
    u8 *nameByte;
    u8 *namePos;

    namePos = voiceEntry->name;
    charIndex = 0;
    do {
        nameByte = nameBytes + charIndex;
        charIndex += 1;
        *namePos = *nameByte;
        namePos += 1;
    } while (charIndex < 5);
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C67B8);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C6840);
