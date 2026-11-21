#include "common.h"

typedef signed char s8;
typedef int s32;
typedef short s16;
typedef float f32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;

/* Stream writer state (global gStreamWriter). */
/* Address of a byte at a runtime offset from a base pointer. */
#define BYTE_OFFSET(base, offset) ((u8 *) (base) + (offset))
/* The s32 entry at byte offset 8 + index * 4 of a SlotRuntime (indexed by index). */
#define SLOT_TABLE_VALUE(state, index) (*(s32 *) BYTE_OFFSET(state, ((index) << 2) + 8))

typedef struct StreamWriter {
    u8 unk_00[0x10];
    s32 cursor;
    u8 *sectionBuffer;
    s32 channelId;
    s16 channelHandle;
    u8 unk_1E[0xA];
    s32 streamBits;
    u8 unk_2C[8];
    s32 flags34;
} StreamWriter;

extern StreamWriter gStreamWriter[];

typedef struct SlotSavedValue {
    s32 storedValue;
    s32 unk_04;
} SlotSavedValue;

typedef struct SlotRuntime {
    u8 unk_00[0xC];
    void *callback;
    s32 unk_10;
    u8 unk_14[4];
    SlotSavedValue savedValues[8];
    s32 valueSelector;
} SlotRuntime;

typedef struct SlotBinding {
    s32 bindingBits;
    u8 unk_04[4];
    SlotRuntime *runtime;
    s16 channelHandle;
} SlotBinding;

typedef struct SlotInstance {
    u8 unk_00[2];
    s16 initialValue;
    s16 unk_04;
    s16 unk_06;
    u8 unk_08[0x28];
    f32 scale;
} SlotInstance;

extern SlotBinding **gSlotBindingTable;

typedef struct ActorCommand {
    u8 unk_00[0xE];
    u16 issuedBits;
    u8 unk_10[0x10];
    f32 unk_20;
} ActorCommand;

typedef struct ActorRef {
    u8 unk_00[0xC];
    s16 slotId;
} ActorRef;

typedef struct ActorLink {
    u8 unk_00[0x3C];
    f32 speed;
} ActorLink;

typedef struct ActorParams {
    u8 unk_00[0x10];
    f32 unk_10;
    f32 unk_14;
} ActorParams;

typedef struct ActorSlot {
    u8 unk_00[0x54];
    f32 motionValue;
} ActorSlot;

typedef struct ActorBinding {
    u8 unk_00[0x1C];
    ActorLink *motionLink;
    u8 unk_20[0xC];
    ActorParams *motionParams;
} ActorBinding;

typedef struct ActorRuntime {
    u8 unk_00[0x30];
    f32 unk_30;
    u8 unk_34[4];
    f32 unk_38;
    u8 unk_3C[0x1C];
    f32 stepRate;
    u8 unk_5C[0x20];
    ActorCommand *effectCommand;
    u8 unk_80[8];
    void *unk_88;
    u8 unk_8C[4];
    ActorLink *activeMotionLink;
    u8 unk_94[8];
    s32 unk_9C;
    ActorSlot *actorSlot;
    u8 unk_A4[4];
    ActorRef *actorRef;
    ActorBinding *binding_AC;
} ActorRuntime;

extern ActorRuntime *gActorRuntime[];
extern ActorCommand *gActorCommand[];
extern s32 gSysHandle[];
s32 func_002A6FA8();

void sysHandleSetOrCreate(s32 requestedHandle) {
    s32 createdHandle;

    if (requestedHandle != 0) {
        gSysHandle[0] = requestedHandle;
        return;
    }
    createdHandle = func_002A6FA8(0x100, -1);
    gSysHandle[0] = createdHandle;
}

void slotBindNoop(s16 slotId) {

}

void slotUnbindNoop(void) {

}

void slotStateReset(SlotBinding *slotBinding)
{
  SlotSavedValue *recordCursor;
  s32 i;
  SlotRuntime *slotRuntime;
  slotRuntime = slotBinding->runtime;
  if (slotRuntime != 0)
  {
    recordCursor = &slotRuntime->savedValues[7];
    slotRuntime->unk_10 = 0;
    slotRuntime->callback = 0;
    for (i = 0; i < 8; i++)
    {
      recordCursor->storedValue = 0;
      recordCursor--;
    }

  }
}

void slotSavedValuesClear(SlotBinding *slotBinding) {
    SlotSavedValue *recordCursor;
    s32 i;
    SlotRuntime *slotRuntime;

    slotRuntime = slotBinding->runtime;
    if (slotRuntime != 0) {
        slotRuntime->unk_10 = 0;
        recordCursor = &slotRuntime->savedValues[7];
        for (i = 0; i < 8; i++) {
            recordCursor->storedValue = 0;
            recordCursor--;
        }
    }
}


INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A138);

extern s32 func_00279AC0();
s32 channelEntryClearActive();

void streamHandleRecreate(void)
{
  if (gStreamWriter[0].channelHandle != -1)
  {
    channelEntryClearActive(gStreamWriter[0].channelHandle);
  }
  gStreamWriter[0].channelHandle = func_00279AC0(gStreamWriter[0].channelId, 0x3260);
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A220);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A2A0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A328);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A388);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A460);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A4D0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A728);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A7E8);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027A8C0);

void slotStateReset(SlotBinding *slotBinding);
void func_002792D0();
void func_0027A138();

void slotFlagsNotify(s32 slotNumber) {
    extern void slotStateReset();
    s32 flag_bit;
    SlotBinding *slotBinding;

    slotBinding = gSlotBindingTable[slotNumber];
    flag_bit = slotBinding->bindingBits & 0x10000000;
    if (flag_bit != 0) {
        slotBinding->bindingBits = flag_bit;
        if (slotBinding->runtime != 0) {
            func_002792D0(slotBinding->channelHandle);
            slotStateReset(gSlotBindingTable[slotNumber]);
            func_0027A138(gSlotBindingTable[slotNumber]);
        }
    }
}


INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027AB00);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027AB98);

void slotStateSetReady(s32 slotNumber) {
    SlotBinding *slotBinding;
    s32 bindingBits;

    slotBinding = gSlotBindingTable[slotNumber];
    bindingBits = slotBinding->bindingBits;
    if (bindingBits & 0x10000000) {
        slotBinding->bindingBits = (bindingBits & ~0xF) | 2;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027AC30);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027AC90);

SlotInstance *slotInstanceFetch(s32 slotNumber, s32 valueSelector, s32 summedLength);

f32 slotGetScale(s32 slotNumber) {
    return slotInstanceFetch(slotNumber, 0, 0)->scale;
}


void slotSetScale(s32 slotNumber, f32 scale) {
    slotInstanceFetch(slotNumber, 0, 0)->scale = scale;
}

void slotSavedValuesClear(SlotBinding *slotBinding);
void func_0027BF98();
void func_0027DB68();

void slotSetValueSelector(s32 slotNumber, s32 valueSelector) {
    extern void slotSavedValuesClear();
    SlotBinding *slotBinding;
    SlotBinding *refreshedBinding;

    slotBinding = gSlotBindingTable[slotNumber];
    if (slotBinding->runtime != 0) {
        slotSavedValuesClear(slotBinding);
        refreshedBinding = gSlotBindingTable[slotNumber];
        refreshedBinding->runtime->valueSelector = valueSelector;
        func_0027BF98(refreshedBinding);
        func_0027DB68(gSlotBindingTable[slotNumber]);
    }
}


INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027AE00);

extern unsigned char func_00283710[];

void slotInstanceInit(s32 slotNumber, s32 initialValue)
{
  SlotInstance *slotInstance;
  SlotBinding *slotBinding;
  slotInstance = slotInstanceFetch(slotNumber, 0, 8);
  slotInstance->initialValue = initialValue;
  slotBinding = gSlotBindingTable[slotNumber];
  slotInstance->unk_06 = -1;
  slotBinding->runtime->callback = func_00283710;
  slotBinding->bindingBits = slotBinding->bindingBits | 0x100000;
  slotInstance->unk_04 = 0;
}


void slotDisarmClearFlag(s32 slotNumber)
{
  SlotBinding *slotBinding;
  s32 bindingBits;
  s32 flags34;
  slotBinding = gSlotBindingTable[slotNumber];
  bindingBits = slotBinding->bindingBits & 0xFFEFFFFF;
  slotBinding->bindingBits = bindingBits;
  slotBinding->runtime->callback = 0;
  flags34 = gStreamWriter[0].flags34 & (~1);
  gStreamWriter[0].flags34 = flags34;
}

extern unsigned char func_00283940[];

void slotArmWithCallback(s32 slotNumber)
{
  s32 bindingBits;
  SlotBinding *slotBinding;
  slotBinding = gSlotBindingTable[slotNumber];
  slotBinding->runtime->callback = func_00283940;
  bindingBits = slotBinding->bindingBits | 0x100000;
  slotBinding->bindingBits = bindingBits;
}

void slotDisarm(s32 slotNumber) {
    SlotBinding *slotBinding;

    slotBinding = gSlotBindingTable[slotNumber];
    slotBinding->bindingBits &= 0xFFEFFFFF;
    slotBinding->runtime->callback = 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027AFA8);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B020);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B068);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B0E0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B128);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B1C8);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B250);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B2B0);

s32 streamSetValueFlag(s32 channelId)
{
  s32 streamBits;
  streamBits = gStreamWriter[0].streamBits | 1;
  gStreamWriter[0].channelId = channelId;
  gStreamWriter[0].streamBits = streamBits;
  return streamBits;
}

extern s32 gPendingLoadCount[];

void pendingCountSet(s32 pendingCount) {
    gPendingLoadCount[0] = pendingCount;
}

s32 pendingCountGet(void) {
    return gPendingLoadCount[0];
}

extern unsigned char gSectionOffsetBytes[];

s32 accumulateTableBytes(s32 summedLength)
{
  s32 runningSum;
  u32 summedIndex;
  int zero;
  u8 *summedByte;
  runningSum = 0;
  summedIndex = 0;
  zero = 0;
  if (summedLength != zero)
  {
    loop_2:
    summedByte = &gSectionOffsetBytes[summedIndex];

    summedIndex += 1;
    runningSum += *summedByte;
    if (summedIndex < 0xAU)
    {
      if (summedIndex != summedLength)
      {
        goto loop_2;
      }
    }
  }
  return runningSum;

}

s32 accumulateTableBytes(s32 count);

void streamBeginSection(SlotBinding *slotBinding, s32 valueSelector, s32 summedLength)
{
    extern s32 accumulateTableBytes();
  s32 sectionAddress;
  sectionAddress = SLOT_TABLE_VALUE(slotBinding->runtime, valueSelector) + accumulateTableBytes(summedLength);
  gStreamWriter[0].sectionBuffer = (u8 *) sectionAddress;
  gStreamWriter[0].cursor = 0;
}


void streamBeginSection(SlotBinding *owner, s32 index, s32 count);

/* The 0x10-byte header written at the start of each stream record. */
typedef struct StreamRecordHeader {
    u8 unk_00[4];
    s32 unk_04;
    u8 unk_08[4];
    s16 recordId;
    s16 unk_0E;
} StreamRecordHeader;

void streamBeginRecord(s32 slotNumber, s32 valueSelector, s32 summedLength, s32 recordId)
{
  s32 cursor;
  StreamRecordHeader *streamRecordHeader;
  streamBeginSection(gSlotBindingTable[slotNumber], valueSelector, summedLength);
  streamRecordHeader = (StreamRecordHeader *) gStreamWriter[0].sectionBuffer;
  streamRecordHeader->unk_04 = 0;
  streamRecordHeader->recordId = recordId;
  cursor = gStreamWriter[0].cursor + 0x10;
  streamRecordHeader->unk_0E = 0;
  gStreamWriter[0].cursor = cursor;
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B450);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B4A8);

s8 *streamPutByte(s32 streamByte)
{
  s8 *writeAddress;
  s32 cursor;
  writeAddress = (s8 *) (gStreamWriter[0].sectionBuffer + gStreamWriter[0].cursor);
  *writeAddress = streamByte;
  cursor = gStreamWriter[0].cursor + 1;
  gStreamWriter[0].cursor = cursor;
  return writeAddress;
}

void streamPutHalfword(s32 streamHalfword)
{
  s32 cursor;
  s16 *writeAddress;
  cursor = gStreamWriter[0].cursor + 2;
  writeAddress = (s16 *) (gStreamWriter[0].sectionBuffer + gStreamWriter[0].cursor);
  gStreamWriter[0].cursor = cursor;
  *writeAddress = streamHalfword;
}

s32 *streamPutWord(s32 streamWord)
{
  s32 *writeAddress;
  s32 cursor;
  writeAddress = (s32 *) (gStreamWriter[0].sectionBuffer + gStreamWriter[0].cursor);
  *writeAddress = streamWord;
  cursor = gStreamWriter[0].cursor + 4;
  gStreamWriter[0].cursor = cursor;
  return writeAddress;
}

void streamPutFloat(f32 streamFloat)
{
  s32 cursor;
  f32 *writeAddress;
  cursor = gStreamWriter[0].cursor + 4;
  writeAddress = (f32 *) (gStreamWriter[0].sectionBuffer + gStreamWriter[0].cursor);
  gStreamWriter[0].cursor = cursor;
  *writeAddress = streamFloat;
}


INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B590);

void streamFlushNoop(void) {

}

SlotInstance *slotInstanceFetch(s32 slotNumber, s32 valueSelector, s32 summedLength)
{
    extern s32 accumulateTableBytes();
  return (SlotInstance *) (SLOT_TABLE_VALUE(gSlotBindingTable[slotNumber]->runtime, valueSelector) + accumulateTableBytes(summedLength));
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B638);

SlotBinding *slotBindingFetch(s32 slotNumber) {
    return gSlotBindingTable[slotNumber];
}

s32 slotSelectedTableValueGet(SlotBinding *slotBinding)
{
  SlotRuntime *slotRuntime;
  slotRuntime = slotBinding->runtime;
  return SLOT_TABLE_VALUE(slotRuntime, slotRuntime->valueSelector);
}

s32 slotRecordValueGet(SlotBinding *slotBinding, s32 savedValueIndex) {
    return slotBinding->runtime->savedValues[savedValueIndex].unk_04;
}

ActorCommand *actorRequestGet(void) {
    return gActorCommand[0];
}

ActorRuntime *actorStateGet(void) {
    return gActorRuntime[0];
}

extern s32 gActorId[];

s32 actorIdGet(void) {
    return gActorId[0];
}

void actorIdSet(s32 actorId) {
    gActorId[0] = actorId;
}

extern s32 D_005A8198[];

s32 sceneWordGet(void) {
    return D_005A8198[0];
}

extern s32 D_005A81A0[];

s32 sceneCountGet(void) {
    return D_005A81A0[0];
}

extern s32 D_005A81A4[];

s32 sceneFlagWordGet(void) {
    return D_005A81A4[0];
}

extern s32 D_005A81B4[];

s32 sceneBit1Get(void) {
    return D_005A81B4[0] & 1;
}

s32 sceneBit8Get(void) {
    return D_005A81B4[0] & 8;
}

void streamSetFlagTwo(void)
{
  gStreamWriter[0].streamBits = gStreamWriter[0].streamBits | 2;
}

extern unsigned char D_005A81C0[];

u8 *auxBufferGet(void) {
    return D_005A81C0;
}

void auxBufferNoop(void) {

}

ActorCommand *actorRequestGet(void);
ActorRuntime *actorStateGet(void);

void actorRequestBindPrimary(void) {
    extern s32 func_00279590(s16, ActorSlot *, s32, s32, f32);
    u16 issuedBits;
    ActorRuntime *actorRuntime;
    ActorCommand *actorCommand;

    actorRuntime = actorStateGet();
    actorCommand = actorRequestGet();
    issuedBits = actorCommand->issuedBits;
    actorRuntime->unk_88 = actorCommand;
    if (!(issuedBits & 1 & 0xFFFF)) {
        actorCommand->issuedBits = issuedBits | 1;
        func_00279590(actorRuntime->actorRef->slotId, actorRuntime->actorSlot, 0, 0x1021, 0.0f);
    }
}

void actorRequestBindSecondary(void) {
    extern s32 func_00279590(s16, ActorSlot *, s32, s32, f32);
    u16 issuedBits;
    ActorRuntime *actorRuntime;
    ActorCommand *actorCommand;

    actorRuntime = actorStateGet();
    actorCommand = actorRequestGet();
    issuedBits = actorCommand->issuedBits;
    actorRuntime->unk_88 = actorCommand;
    if (!(issuedBits & 1 & 0xFFFF)) {
        actorCommand->issuedBits = issuedBits | 1;
        func_00279590(actorRuntime->actorRef->slotId, actorRuntime->actorSlot, 0, 0x1021, 0.0f);
    }
}

void actorRequestBindTertiary(void) {
    extern s32 func_00279590(s16, ActorSlot *, s32, s32, f32);
    u16 issuedBits;
    ActorRuntime *actorRuntime;
    ActorCommand *actorCommand;

    actorRuntime = actorStateGet();
    actorCommand = actorRequestGet();
    issuedBits = actorCommand->issuedBits;
    actorRuntime->unk_88 = actorCommand;
    if (!(issuedBits & 1 & 0xFFFF)) {
        actorCommand->issuedBits = issuedBits | 1;
        func_00279590(actorRuntime->actorRef->slotId, actorRuntime->actorSlot, 0, 0x1021, 0.0f);
    }
}

void slotBindNoop(s16 slotId);
extern f32 gEffectScaleA;
void slotRecordSetScale(s16, f32);

void actorEffectRequestBindA(void) {
    extern void func_00279590(s16, ActorSlot *, void *, s32, f32);
    u16 issuedBits;
    ActorRuntime *actorRuntime;
    ActorCommand *actorCommand;

    actorRuntime = actorStateGet();
    actorCommand = actorRuntime->effectCommand;
    issuedBits = actorCommand->issuedBits;
    if (!(issuedBits & 1 & 0xFFFF)) {
        actorCommand->issuedBits = issuedBits | 1;
        func_00279590(actorRuntime->actorRef->slotId, actorRuntime->actorSlot, actorCommand->unk_10, 0x10105011, actorCommand->unk_20);
        slotRecordSetScale(actorRuntime->actorRef->slotId, gEffectScaleA);
    }
    slotBindNoop(actorRuntime->actorRef->slotId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027B970);

extern f32 gEffectScaleB;

void actorEffectRequestBindB(void) {
    extern void func_00279590(s16, ActorSlot *, s32, s32, f32);
    u16 issuedBits;
    ActorRuntime *actorRuntime;
    ActorCommand *actorCommand;

    actorRuntime = actorStateGet();
    actorCommand = actorRuntime->effectCommand;
    issuedBits = actorCommand->issuedBits;
    if (!(issuedBits & 1 & 0xFFFF)) {
        actorCommand->issuedBits = issuedBits | 1;
        func_00279590(actorRuntime->actorRef->slotId, actorRuntime->actorSlot, actorRuntime->unk_9C, 0x10105021, actorCommand->unk_20);
        slotRecordSetScale(actorRuntime->actorRef->slotId, gEffectScaleB);
    }
    slotBindNoop(actorRuntime->actorRef->slotId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027BA90);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027BCE0);

extern f32 gEffectScaleC;

void actorEffectRequestBindC(void) {
    extern void func_00279590(s16, ActorSlot *, void *, s32, f32);
    u16 issuedBits;
    ActorRuntime *actorRuntime;
    ActorCommand *actorCommand;

    actorRuntime = actorStateGet();
    actorCommand = actorRuntime->effectCommand;
    issuedBits = actorCommand->issuedBits;
    if (!(issuedBits & 1 & 0xFFFF)) {
        actorCommand->issuedBits = issuedBits | 1;
        func_00279590(actorRuntime->actorRef->slotId, actorRuntime->actorSlot, actorCommand->unk_10 + 0x14, 0x10105021, actorCommand->unk_20);
        slotRecordSetScale(actorRuntime->actorRef->slotId, gEffectScaleC);
    }
    slotBindNoop(actorRuntime->actorRef->slotId);
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027BF98);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C0B0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C100);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C130);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C1C0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C380);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C6A8);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027C928);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027CB60);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027CCB0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027CF30);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D188);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D1E8);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D2B0);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D360);

extern f32 gActorSpeedThreshold;
extern f32 gActorMinSpeed;
extern f32 gActorMaxSpeed;
f32 func_00285D50(f32, f32, f32, f32);

f32 actorStepRate(void) {
    ActorRuntime *actorRuntime;
    ActorLink *actorLink;
    f32 linkSpeed;
    f32 maxSpeed;
    f32 minSpeed;
    f32 outputRate;
    f32 scaled;
    f32 clamped;
    f32 stepped;

    actorRuntime = actorStateGet();
    outputRate = gActorSpeedThreshold;
    minSpeed = gActorMinSpeed;
    actorLink = actorRuntime->binding_AC->motionLink;
    maxSpeed = gActorMaxSpeed;
    linkSpeed = actorLink->speed;
    actorRuntime->activeMotionLink = actorLink;
    actorRuntime->stepRate = linkSpeed;
    if (!(linkSpeed < outputRate)) {
        scaled = linkSpeed * 3.0f;
        actorRuntime->stepRate = scaled;
        if (maxSpeed <= scaled) {
            actorRuntime->stepRate = maxSpeed;
        }
        clamped = actorRuntime->stepRate;
        if (clamped <= minSpeed) {
            actorRuntime->stepRate = minSpeed;
            clamped = minSpeed;
        }
        stepped = func_00285D50(actorRuntime->unk_30, actorRuntime->unk_38, actorRuntime->actorSlot->motionValue, clamped);
        outputRate = stepped;
        actorRuntime->actorSlot->motionValue = outputRate;
    }
    return outputRate;
}

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D4B8);

f32 func_00285EA0(void *, f32, f32, f32);

void actorStepParams(void)
{
  ActorRuntime *actorRuntime;
  ActorParams *actorParams;
  ActorSlot *actorSlot;
  do {
    actorRuntime = actorStateGet();
    actorSlot = actorRuntime->actorSlot;
    actorParams = actorRuntime->binding_AC->motionParams;
    actorRuntime->unk_88 = actorParams;
  } while (0);
  actorRuntime->actorSlot->motionValue = func_00285EA0(actorSlot, actorParams->unk_10, actorSlot->motionValue, actorParams->unk_14);
}


INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D590);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D770);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D818);

INCLUDE_ASM("asm/main/nonmatchings/tu298", func_0027D9E0);
