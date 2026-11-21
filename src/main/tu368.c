#include "common.h"

typedef int s32;
typedef float f32;
typedef signed char s8;
typedef unsigned char u8;
#include "main/njPlane.h"

/* Slot record, stride 0x370: the slots of the state block gSceneStateBlock and of gSceneSlots / gSceneFlagSlots. */
typedef struct SceneSlot {
    s32 activityMode;               /* 0x00 */
    s32 unk_04;             /* 0x04 */
    s32 flagPositive;            /* 0x08 */
    s32 enabled;            /* 0x0C */
    u8 unk_10[0x360];
} SceneSlot;

/* Record of gEffectPool: 0xE40 bytes, eight of them; the words at +0x00, +0x04, +0x24 and +0x30 are used here. */
typedef struct EffectPoolRecord {
    s32 headWordA;
    s32 headWordB;
    u8 unk_08[0x24 - 0x08];
    s32 tailWordA;
    u8 unk_28[0x30 - 0x28];
    s32 tailWordB;
    u8 unk_34[0xE40 - 0x34];
} EffectPoolRecord;

/* Cell array of the plane group: 0xC0-byte cells from +0x140 on, passed to the cell helpers by address. */
typedef struct PlaneCellArray {
    u8 unk_000[0x140];
    u8 cells[14][0xC0];
} PlaneCellArray;

/* State block gSceneStateBlock. The words at 0x3740..0x38FF are reached by byte offset from its address. */
typedef struct SceneSlotBlock {
    u8 unk_0000[0x338];
    SceneSlot slots[16];    /* slots[0] at 0x338, slots[4] at 0x10F8 */
} SceneSlotBlock;

/* Word view of the state block gSceneStateBlock: two 32-word tables at 0x3740 and 0x37C0, flag words at 0x38E4..0x38FC. */
typedef struct SceneWordBlock {
    u8 unk_0000[0x3740];
    s32 table_3740[32];
    s32 table_37C0[32];
    u8 unk_3840[0x38E4 - 0x3840];
    s32 flagRequested;
    u8 unk_38E8[0x38EC - 0x38E8];
    s32 flagDone;
    u8 unk_38F0[0x38F4 - 0x38F0];
    s32 resetCounter;
    u8 unk_38F8[0x38FC - 0x38F8];
    s8 counterLimit;
} SceneWordBlock;

/* Object of the plane group: three NjPlane records plus the words this TU reads and writes. */
typedef struct PlaneGroup {
    u8 unk_000[0xF0];
    NjPlane plane_f0;       /* 0xF0 */
    u8 unk_170[0x40];
    NjPlane plane_1b0;      /* 0x1B0 */
    u8 unk_230[0x40];
    NjPlane plane_270;      /* 0x270 */
    u8 unk_2F0[0x50];
    f32 alpha;            /* 0x340 */
    u8 unk_344[0x24];
    s32 highlightFlag;            /* 0x368 */
} PlaneGroup;

extern unsigned char gSceneStateBlock[];
#define gSceneWordBlock (*(SceneWordBlock *) gSceneStateBlock)
#define gSceneSlotBlock (*(SceneSlotBlock *) gSceneStateBlock)

void njPlaneInit(NjPlane *plane);
void njPlaneSetOffset(NjPlane *plane, f32 x, f32 y);

void planeGroupInit(PlaneGroup *planeGroup) {
    NjPlane *plane;

    plane = &planeGroup->plane_f0;
    njPlaneInit(plane);
    njPlaneInit(&planeGroup->plane_1b0);
    njPlaneInit(&planeGroup->plane_270);
    njPlaneSetTint(plane, 13.0f, 13.0f, 24.0f);
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B7928);

void planeGroupSetOffset(PlaneGroup *planeGroup, f32 x, f32 y) {
    f32 shiftedX;

    njPlaneSetOffset(&planeGroup->plane_f0, x, y);
    shiftedX = x + 2.0f;
    njPlaneSetOffset(&planeGroup->plane_1b0, shiftedX, y + 2.0f);
    njPlaneSetOffset(&planeGroup->plane_270, shiftedX, (y + planeGroup->plane_f0.origin_y) - 4.0f);
}

void planeGroupSetTint(PlaneGroup *planeGroup, s32 x, s32 y, s32 z) {
    f32 fx;
    f32 fy;
    f32 fz;

    fx = x;
    fy = y;
    fz = z;
    njPlaneSetTint(&planeGroup->plane_1b0, fx, fy, fz);
    njPlaneSetTint(&planeGroup->plane_270, fx, fy, fz);
}

void planeGroupRefresh(PlaneGroup *planeGroup) {
    extern s32 func_002D10D0();
    NjPlane *tintedPlaneA;
    NjPlane *tintedPlaneB;

    tintedPlaneA = &planeGroup->plane_1b0;
    tintedPlaneB = &planeGroup->plane_270;
    if (planeGroup->highlightFlag != 0) {
        njPlaneSetTint(tintedPlaneA, 255.0f, 0.0f, 0.0f);
        njPlaneSetTint(tintedPlaneB, 255.0f, 0.0f, 0.0f);
    }
    func_002D10D0(&planeGroup->plane_f0);
    func_002D10D0(tintedPlaneA);
    func_002D10D0(tintedPlaneB);
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B7B50);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B7C08);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B7C50);

extern s32 D_005EE350[];

void valueTableSet(s32 tableIndex, s32 tableValue) {
    D_005EE350[tableIndex] = tableValue;
}

void planeGroupSetAlpha(PlaneGroup *planeGroup, s32 alpha) {
    planeGroup->alpha = alpha;
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B7CD0);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B80D8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B8188);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B86D0);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B8AA8);

void planeGroupNoopSync(void) {

}

void planeGroupNoopReset(void) {

}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B8C38);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B8E20);

extern s32 D_005EE45C[];

void counterSet(s32 counterValue) {
    D_005EE45C[0] = counterValue;
}

void sceneWordTablesClear(void) {
    s32 tableCellIndex;

    for (tableCellIndex = 0; tableCellIndex < 32; tableCellIndex++) {
        gSceneWordBlock.table_3740[tableCellIndex] = 0;
        gSceneWordBlock.table_37C0[tableCellIndex] = 0;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B9050);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B91F8);

extern s8 gSceneStatusFlag[];

s32 sceneStatusFlagIsOne(void) {
    return gSceneStatusFlag[0] == 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B9468);

extern SceneSlot gSceneSlots[];

void slotActivityModeSet(s32 slotNumber, s32 newActivityMode) {
    if (newActivityMode != 1) {
        gSceneSlots[slotNumber].activityMode = newActivityMode;
    }
}

extern SceneSlot gSceneFlagSlots[];

void slotSetFlagPositive(s32 slotNumber, s32 amount) {
    SceneSlot *flaggedSlot;

    flaggedSlot = &gSceneFlagSlots[slotNumber];
    flaggedSlot->flagPositive = 0;
    if (amount > 0) {
        flaggedSlot->flagPositive = 1;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B95A0);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B99B8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B9D98);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002B9F60);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BA060);

s32 slotCountIdle(void)
{
  s32 scanOffset;
  s32 idleSlotCount;
  SceneSlot *slotCursor;

  idleSlotCount = 0;
  scanOffset = 0;
  if (gSceneSlotBlock.slots[4].activityMode != 1)
  {
    slotCursor = &gSceneSlotBlock.slots[4];
    idleSlotCount = 1;
    for (;;)
    {
      scanOffset += 1;
      slotCursor += 1;
      if ((scanOffset < 0xC) && (slotCursor->activityMode != 1))
      {
        idleSlotCount += 1;
        continue;
      }
      break;
    }

  }
  return idleSlotCount;
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BA230);

s8 sceneStatusFlagGet(void) {
    return gSceneStatusFlag[0];
}

s32 slotPairActive(void)
{
  if (((gSceneSlotBlock.slots[0].enabled != 0) && (gSceneSlotBlock.slots[0].activityMode == 1)) || ((gSceneSlotBlock.slots[1].enabled != 0) && (gSceneSlotBlock.slots[1].activityMode == 1)))
  {
    return 1;
  }
  return 0;
}

s32 func_002B8C38();

void sceneFlagRequestAcknowledge(void)
{
  if (gSceneWordBlock.flagRequested != 0)
  {
    gSceneWordBlock.flagDone = 1;
  }
  func_002B8C38(0);
}

s32 memset();

void sceneStateBlockClear(void) {
    memset(gSceneStateBlock, 0, 0x3900);
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BA858);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BAA60);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BABC8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BAD78);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB048);

void sceneCounterLimitReset(void)
{
  gSceneWordBlock.counterLimit = 8;
  gSceneWordBlock.resetCounter = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB0D8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB1C8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB2A8);

extern s32 D_005514B4;
extern s32 D_005514B8;
extern EffectPoolRecord gEffectPool[];
s32 func_002BB698();

void effectPoolInit(void)
{
  s32 effectIndex;
  EffectPoolRecord *effectRecord;

  effectRecord = gEffectPool;
  for (effectIndex = 0; effectIndex < 8; effectIndex++)
  {
    func_002BB698(effectRecord);
    effectRecord++;
  }

  D_005514B4 = 0;
  D_005514B8 = 0;
}

/* Walks the records of gEffectPool from the last (index 7) down; the field address is stepped one record at a time.
   `- -7` keeps the record base add separate from the symbol, as in the original. */
void effectPoolClearTails(void)
{
  s32 effectIndex;
  s32 *tailWordPtr;
  EffectPoolRecord *effectRecord;

  effectRecord = gEffectPool - -7;
  tailWordPtr = &effectRecord->tailWordA;
  for (effectIndex = 0; effectIndex < 8; effectIndex++)
  {
    *tailWordPtr = 0;
    tailWordPtr -= sizeof(EffectPoolRecord) / sizeof(s32);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB450);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB4E8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB5A0);

void effectPoolClearHeads(void) {
    s32 effectIndex;
    EffectPoolRecord *effectRecord;

    effectRecord = gEffectPool;
    for (effectIndex = 0; effectIndex < 8; effectIndex++) {
        effectRecord->headWordA = 0;
        effectRecord->headWordB = 0;
        effectRecord++;
    }
}

/* Walks the records of gEffectPool from the last (index 7) down; the field address is stepped one record at a time.
   `- -7` keeps the record base add separate from the symbol, as in the original. */
void effectPoolSetTails(s32 tailValue)
{
  s32 effectIndex;
  s32 *tailWordPtr;
  EffectPoolRecord *effectRecord;

  effectRecord = gEffectPool - -7;
  tailWordPtr = &effectRecord->tailWordB;
  for (effectIndex = 0; effectIndex < 8; effectIndex++)
  {
    *tailWordPtr = tailValue;
    tailWordPtr -= sizeof(EffectPoolRecord) / sizeof(s32);
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB698);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BB9B0);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BBA40);

s32 func_002CFFF0(u8 *cell, s32 kind, s32 flag);
s32 func_002D0828(u8 *cell, s32 kind, s32 flag);

void planeArrayInit(PlaneCellArray *cells) {
    extern s32 func_002D10D0(u8 *);

    func_002CFFF0(cells->cells[0], 0x25, 0);
    func_002CFFF0(cells->cells[1], 0x25, 0);
    func_002CFFF0(cells->cells[2], 0x25, 0);
    func_002CFFF0(cells->cells[3], 0x25, 0);
    func_002D0828(cells->cells[4], 0x26, 0);
    func_002D0828(cells->cells[5], 0x26, 0);
    func_002D0828(cells->cells[6], 0x26, 0);
    func_002D0828(cells->cells[7], 0x26, 0);
    func_002D0828(cells->cells[8], 0x27, 0);
    func_002D0828(cells->cells[9], 0x27, 0);
    func_002D10D0(cells->cells[10]);
    func_002D10D0(cells->cells[11]);
    func_002D10D0(cells->cells[12]);
    func_002D10D0(cells->cells[13]);
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BC690);

extern unsigned char D_005D0380[];

void auxBufferClearA(void) {
    memset(D_005D0380, 0, 0x140);
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BC7D8);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BCA30);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BCBB8);

extern unsigned char D_005D04C0[];

void auxBufferClearB(void) {
    memset(D_005D04C0, 0, 0x140);
}

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BCD20);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BCF78);

INCLUDE_ASM("asm/main/nonmatchings/tu368", func_002BD160);
