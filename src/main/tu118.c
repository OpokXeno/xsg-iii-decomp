#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9078);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9100);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E91E8);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9300);

#include "main/tu118.h"
typedef int s32;

/* The effect record of sefEffectRecords, 0x150 bytes each (sef_135.h has the full layout); only inUse is read here. */
typedef struct SefEffectRecord {
    unsigned char unmodeled_00[0x50];
    s32 inUse;                      /* +0x50: zero for a free record */
    unsigned char unmodeled_54[0x150 - 0x54];
} SefEffectRecord;

/* The object an effect handle names: kind 0 marks an effect object, any other value an actor. */
typedef struct SefEffectObject {
    s32 handleClass;                       /* +0x00 */
    unsigned char unmodeled_04[0x24 - 0x04];
    s32 playState;                      /* +0x24: 0, 1 or 2 (set by the sefObjectSetState functions below) */
    unsigned char unmodeled_28[0x13E - 0x28];
    s16 effectIndex;                /* +0x13E: index into sefEffectRecords */
    unsigned char unmodeled_140[0x14A - 0x140];
    u8 flag14A;                     /* +0x14A */
} SefEffectObject;

s32 func_001F5590();
s32 func_001F7138();
s32 func_002002C8(s16);
extern SefEffectRecord *sefEffectRecords;

void sefEffectHandleRelease(SefEffectObject *effectObject)
{
  s32 effectIndex;
  if (effectObject != 0)
  {
    if (effectObject->handleClass == 0)
    {
      effectIndex = effectObject->effectIndex;
      if (sefEffectRecords[effectIndex].inUse != 0)
      {
        func_001F7138(effectIndex, 1);
        func_002002C8(effectIndex);
      }
    }
    else
    {
      func_001F5590(effectObject);
    }
  }
}

#include "main/tu118.h"

extern SefEntryTable sefEffectEntryTable;

int sefEffectFlagTest(int mask);
void sefRunEffectStep(void);
void func_00203450(void);

void sefFrameUpdate(void)
{
    if (gGameLoopRuntimeStorage.flags & 0x400) {
        sefResetDrawState();
        return;
    }
    if (sefEffectFlagTest(1) == 0) {
        sefRunEffectStep();
    }
    func_00203450();
}

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9500);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9660);

s32 func_001E9500(s32, f32, f32);

void sefEffectStartScaled(s32 id, f32 effectScale) {
    func_001E9500(id, effectScale, 1.0f);
}

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9790);

void func_001E9790();

void sefObjectSetStateIdle(SefEffectObject *effectObject) {
    func_001E9790();
    effectObject->playState = 0;
}

void sefObjectSetStatePlay(SefEffectObject *effectObject) {
    func_001E9790();
    effectObject->playState = 1;
}

void sefEffectSetStop(SefEffectObject *effectObject) {
    func_001E9790();
    effectObject->playState = 2;
}

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9888);

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001E9F48);

int sefActorFlagsGet(int actor)
{
    SefEntryTable *actorTable;
    SefEntry *actorEntry;
    int i;

    if (actor != 0) {
        actorTable = &sefEffectEntryTable;
        i = 0;
        if (actorTable->effectEntryCount > 0) {
            do {
                actorEntry = &actorTable->effectEntries[i];
                if (actorEntry->id == actor) {
                    return actorEntry->effectEntryFlags;
                }
                i++;
            } while (i < actorTable->effectEntryCount);
        }
    }
    return 0;
}

u8 sefEffectFlagGet(SefEffectObject *effectObject) {
    if ((effectObject != 0) && (effectObject->handleClass == 0)) {
        return effectObject->flag14A;
    }
    return 0U;
}

INCLUDE_ASM("asm/main/nonmatchings/tu118", func_001EA050);
