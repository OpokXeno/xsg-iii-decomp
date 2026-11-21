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

/* The cursor block at seVoiceCursor: the position at +0x00/+0x04, a sub-block at +0x80 and the flags at +0x460..+0x46C. */
typedef struct SeVoiceCursor {
    f32 x;                              /* +0x00 */
    f32 y;                              /* +0x04 */
    u8 unk_08[0x80 - 0x08];
    u8 subBlock[0x460 - 0x80];          /* +0x80: passed to func_002C66D0 */
    s32 unk_460;                        /* +0x460 */
    s32 unk_464;                        /* +0x464 */
    s32 unk_468;                        /* +0x468 */
    s32 cursorEnabled;                  /* +0x46C */
} SeVoiceCursor;
extern SeVoiceCursor seVoiceCursor[];

void seVoiceCursorSetEnable(s32 enableCursor)
{
  SeVoiceCursor *cursor = seVoiceCursor;
  s32 cursorEnable;
  cursorEnable = enableCursor;
  cursor->unk_464 = 0;
  cursor->unk_460 = 0;
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
  cursor->unk_468 = 0;
  cursor->cursorEnabled = 0;
  cursor->y = posY;
  return func_002C66D0(cursor->subBlock);
}

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C47A0);

INCLUDE_ASM("asm/main/nonmatchings/tu374", func_002C4858);

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
