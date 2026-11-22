#include "common.h"

typedef signed char s8;
typedef unsigned char u8;
typedef int s32;
typedef float f32;
extern unsigned char gNjPlaneRegistryStorage[];
extern unsigned char gNjPlaneBlockStorage[];
extern unsigned char gNjPlaneWorkBuffer[];

/* State block at gNjPlaneRegistryStorage; the words at 0x11690..0x116A4 are cleared by njPlaneBlockInit. */
typedef struct NjPlaneRegistry {
    u8 unk_00[0x11690];
    s32 pendingValue;
    s32 unk_11694;
    s32 unk_11698;
    s32 unk_1169C;
    s32 unk_116A0;
    s32 enabled;
} NjPlaneRegistry;
#define gNjPlaneRegistry (*(NjPlaneRegistry *) gNjPlaneRegistryStorage)
void func_002C0C68();
void njStateNoOp();
void njPlaneInit();
#include "main/njPlane.h"

/* Plane slot at gNjPlaneBlockStorage: an NjPlane at +0x00, slots are 0xC0 bytes apart. */
typedef struct NjPlaneSlot {
    NjPlane plane;
    u8 unk_80[0x40];
} NjPlaneSlot;

/* Block at gNjPlaneBlockStorage: five plane slots, then 0x80 primary and 0x80 secondary 0x100-byte string buffers. */
typedef struct NjPlaneBlock {
    NjPlaneSlot planeSlots[5];
    u8 primary[0x80][0x100];
    u8 secondary[0x80][0x100];
} NjPlaneBlock;
#define gNjPlaneBlock (*(NjPlaneBlock *) gNjPlaneBlockStorage)
void njPlaneSetLevel(NjPlane *, f32);
void func_002D6978();
void njStringSetZoom();
void njStringSetFlags();
void *memset(void *destination, s32 value, unsigned int size);

void njPlaneBlockInit(void)
{
  s32 stringIndex;
  NjPlane *lastSlotPlane;
  u8 *primaryText;
  u8 *secondaryText;
  njPlaneInit(&gNjPlaneBlock.planeSlots[0]);
  lastSlotPlane = &gNjPlaneBlock.planeSlots[4].plane;
  njPlaneInit(&gNjPlaneBlock.planeSlots[1]);
  njPlaneInit(&gNjPlaneBlock.planeSlots[2]);
  njPlaneInit(&gNjPlaneBlock.planeSlots[3]);
  njPlaneInit(lastSlotPlane);
  stringIndex = 0;
  njPlaneSetTint(lastSlotPlane, 95.0f, 255.0f, 227.0f);
  njPlaneSetLevel(lastSlotPlane, 90.0f);
  do
  {
    primaryText = gNjPlaneBlock.primary[stringIndex];
    secondaryText = gNjPlaneBlock.secondary[stringIndex];
    stringIndex += 1;
    func_002D6978(primaryText);
    njStringSetFlags(primaryText, 0x80C);
    func_002D6978(secondaryText);
    njStringSetFlags(secondaryText, 0xA0C);
    njStringSetZoom(secondaryText, 0x10, 0x10);
  }
  while (stringIndex < 0x80);
  memset(gNjPlaneWorkBuffer, 0, 0xC00);
  func_002C0C68();
  njStateNoOp();
  gNjPlaneRegistry.pendingValue = 0;
  gNjPlaneRegistry.unk_11694 = 0;
  gNjPlaneRegistry.unk_11698 = 0;
  gNjPlaneRegistry.unk_1169C = 0;
  gNjPlaneRegistry.enabled = 0;
  gNjPlaneRegistry.unk_116A0 = 0;
}

void njPlaneNoopStep(void) {

}

extern unsigned char gNjPlanePendingStorage[];
#define gNjPlanePending (*(s32 *) gNjPlanePendingStorage)

void njPlanePendingSet(s32 pendingValue) {
    gNjPlanePending = pendingValue;
}

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B4FE0);

extern unsigned char gNjPlaneCurrentStorage[];
#define gNjPlaneCurrent (*(s32 *) gNjPlaneCurrentStorage)

s32 njPlaneCurrentGet(void) {
    return gNjPlaneCurrent;
}

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B52A0);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B52D8);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B5480);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B55E0);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B56A8);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B58D0);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B5D00);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B5EC0);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B6078);

INCLUDE_ASM("asm/main/nonmatchings/tu366", func_002B6A80);
