#include "common.h"

typedef int s32;
typedef unsigned char u8;
typedef short s16;
typedef float f32;

typedef struct LinkedObject {
    s32 delegated;          /* 0x00: nonzero routes value writes to the child object */
    s32 linkBits;              /* 0x04: bit 0x400000 is set or cleared by objectSetLinkedFlag */
    u8 unk_08[2];
    s16 linkedValue;        /* 0x0A */
    u8 unk_0C[0x20];
    f32 childFloat;     /* 0x2C: written on the child when delegated */
    u8 unk_30[0x1C];
    f32 ownFloat;         /* 0x4C */
    u8 unk_50[0xC];
    s16 childKey;           /* 0x5C: key for func_001F9A68 */
    u8 unk_5E[2];
    s32 pendingFlags;       /* 0x60 */
    s32 pendingValue;       /* 0x64 */
    u8 unk_68[0xD6];
    s16 linkIndex;          /* 0x13E */
} LinkedObject;

void objectSetLinkedValue(LinkedObject *linkedObject, s32 value);
void objectSetLinkedFlag(LinkedObject *linkedObject, s32 value);

void objectApplyLinkedFlags(LinkedObject *linkedObject, s32 flagMask, s32 linkedValue) {
    if (linkedObject != 0) {
        if (flagMask & 0x400000) {
            objectSetLinkedFlag(linkedObject, linkedValue == 0);
        }
        if (flagMask & 0x200000) {
            objectSetLinkedValue(linkedObject, linkedValue);
        }
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA128);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA2B0);

s32 func_001EA128(LinkedObject *linkedObject, s32 flags, s32 value);

void objectApplyLinkedValue(LinkedObject *linkedObject, s32 linkedValue) {
    func_001EA128(linkedObject, 0x200000, linkedValue);
}

s32 func_001EA2B0(LinkedObject *linkedObject, s32 index, s32 flags, s32 value);

void objectApplyLinkedValueIndexed(LinkedObject *linkedObject, s32 slotIndex, s32 linkedValue) {
    func_001EA2B0(linkedObject, slotIndex, 0x200000, linkedValue);
}

void objectApplyLinkedFlag(LinkedObject *linkedObject, s32 linkEnabled) {
    func_001EA128(linkedObject, 0x400000, linkEnabled);
}

void objectApplyLinkedFlagIndexed(LinkedObject *linkedObject, s32 slotIndex, s32 linkEnabled) {
    func_001EA2B0(linkedObject, slotIndex, 0x400000, linkEnabled);
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA448);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA518);

typedef struct VuQuadword {
    s32 lanes[4];
} VuQuadword;

void *vuLaneMaskQuadsWrite(void *quadStorage)
{
  VuQuadword *quads;
  void *laneMaskQuad;
  quads = quadStorage;
  quads[0].lanes[0] = 0;
  quads[0].lanes[1] = 1;
  quads[0].lanes[2] = 1;
  quads[0].lanes[3] = 0;
  __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&quads[1]) : "memory");
  laneMaskQuad = &quads[2];
  __asm__ __volatile__("vmaxw.xyzw vf1, vf0, vf0w" : : : "memory");
  __asm__ __volatile__("sqc2 vf1, 0(%0)" : : "r"(laneMaskQuad) : "memory");
  __asm__ __volatile__("sqc2 vf0, 0(%0)" : : "r"(&quads[3]) : "memory");
  return laneMaskQuad;
}

s32 func_001F6698();

void sceneTickSubsystem(void) {
    func_001F6698();
}

s32 func_001E8E70();
void srsSetEffectFlag(s32, s32);

void sceneInitSubsystems(void) {
    func_001E8E70();
    srsSetEffectFlag(0, 1);
}

extern unsigned char gEffectBuffer[];
void clearEffectTable(void);
void *memset(void *destination, s32 value, unsigned int size);

void sceneClearBufferAndInit(void) {
    memset(gEffectBuffer, 0, 0xF00);
    clearEffectTable();
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA6B0);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA798);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA920);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA960);

void objectSetPendingFlags(LinkedObject *linkedObject, s32 newPendingFlags, s32 newPendingValue) {
    if ((linkedObject != 0) && (linkedObject->delegated == 0)) {
        linkedObject->pendingFlags = newPendingFlags;
        linkedObject->pendingValue = newPendingValue;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EA9C8);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAA08);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAA70);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAA98);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAB00);

s32 func_001F7668(s16);
LinkedObject *func_001F9A68(s16);

void linkedObjectSetFloat(LinkedObject *linkedObject, f32 linkedFloat) {
    LinkedObject *childObject;

    if (linkedObject != 0) {
        if (linkedObject->delegated == 0) {
            linkedObject->ownFloat = linkedFloat;
            func_001F7668(linkedObject->linkIndex);
            return;
        }
        childObject = func_001F9A68(linkedObject->childKey);
        if (childObject != 0) {
            childObject->childFloat = linkedFloat;
        }
    }
}

s32 func_001F7740(s16 linkIndex, s32 value);

void objectSetLinkedValue(LinkedObject *linkedObject, s32 linkedValue)
{
  s32 newPendingFlags;
  if (linkedObject != 0)
  {
    if (linkedObject->delegated == 0)
    {
      linkedObject->linkedValue = linkedValue;
      func_001F7740(linkedObject->linkIndex, linkedValue);
    }
    else
    {
      newPendingFlags = (linkedObject->pendingFlags & (~0x100)) | ((linkedValue & 1) << 8);
      linkedObject->pendingFlags = newPendingFlags;
    }
  }
}

s32 func_001F77D8(s16 linkIndex, s32 value);

void objectSetLinkedFlag(LinkedObject *linkedObject, s32 linkEnabled)
{
  s32 newPendingFlags;
  s32 flagsWithLinked;
  s32 flagsWithoutLinked;
  if (linkedObject != 0)
  {
    if (linkedObject->delegated == 0)
    {
      if (linkEnabled != 0)
      {
        flagsWithLinked = linkedObject->linkBits | 0x400000;
        linkedObject->linkBits = flagsWithLinked;
      }
      else
      {
        flagsWithoutLinked = linkedObject->linkBits & 0xFFBFFFFF;
        linkedObject->linkBits = flagsWithoutLinked;
      }
      func_001F77D8(linkedObject->linkIndex, linkEnabled);
    }
    else
    {
      newPendingFlags = (linkedObject->pendingFlags & (~0x200)) | ((linkEnabled & 1) << 9);
      linkedObject->pendingFlags = newPendingFlags;
    }
  }
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EACA8);

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EADA0);

s32 func_001E9300(s32);
s32 func_002002C8();

void checkResultReport(s32 errorCode) {
    if (errorCode < 0) {
        func_001E9300(0x1006402A);
        return;
    }
    func_002002C8();
}

extern s32 gSceneLoadState;

s32 stateIsReady(void) {
    return gSceneLoadState == 3;
}

extern u8 *D_004C83F0[];
void func_001E8498();
extern unsigned char func_001EAEE0[];

void sceneCallbackReset(void) {
    gSceneLoadState = 0;
    func_001E8498();
    D_004C83F0[0] = func_001EAEE0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu119", func_001EAEE0);
