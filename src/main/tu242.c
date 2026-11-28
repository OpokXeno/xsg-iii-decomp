#include "common.h"

typedef unsigned char u8;

typedef int s32;

typedef unsigned int u32;

typedef float f32;

const char D_00530A40[16] = "\xa5" "\xb9" "\xa5" "\xaf" "\xa5" "\xea" "Cam";

extern unsigned char gBattleRecordBuffer[];
extern void *gBattleRecordBufferBaseStorage[];

/* The word at gBattleRecordBufferBaseStorage: the record base address. */
#define gBattleRecordBufferBase (*(f32 **) gBattleRecordBufferBaseStorage)

static f32 *gBattleRecordCursor;
void *memset(void *s, s32 c, u32 n);

void battleRecordReset(void) {
    memset(gBattleRecordBuffer, 0, 0x180);
    gBattleRecordCursor = gBattleRecordBufferBase;
}

s32 battleRecordIsFull(void)
{
  f32 *cursor;
  f32 *threshold;
  cursor = gBattleRecordCursor;
  threshold = gBattleRecordBufferBase + 0x16C / sizeof(f32);
  if (cursor < threshold)
  {
    return 0;
  }
  return 1;
}

s32 battleSetActorSlots(s32 first, s32 second);
s32 vmArgGet(s32 vm, s32 index);

s32 vmOpCallWithTwoArgs(s32 vm) {
    s32 actorSlotA;
    s32 actorSlotB;

    actorSlotA = vmArgGet(vm, 1);
    actorSlotB = vmArgGet(vm, 2);
    battleSetActorSlots(actorSlotA, actorSlotB);
    return 1;
}

void textStateReset(void);
void textStateSuspend(void);
void textSubmitRandomEntry(s32 *candidateIds, s32 actorSlotA, s32 actorSlotB);

typedef struct BattleModeState {
    u8 battleMode;
    u8 firstActorSlot;
    u8 secondActorSlot;
    u8 unk_03[3];
} BattleModeState;

extern BattleModeState gBattleModeActors;
extern void *gBattleTextCandidateIds[];
s32 battleRecordIsFull();
f32 *battleRecordAdvance(f32 *recordCursor);

/* "Start" native of the bcam table: resets the text state and the record buffer, then appends a type 2 record with one value. */
s32 bcamStart(s32 vm) {
    f32 cameraArg;
    f32 *recordCursor;

    if (battleRecordIsFull() != 0) {
        return 1;
    }
    textStateReset();
    textSubmitRandomEntry(gBattleTextCandidateIds, gBattleModeActors.firstActorSlot, gBattleModeActors.secondActorSlot);
    battleRecordReset();
    textStateSuspend();
    *gBattleRecordCursor = 2.0f;
    cameraArg = vmArgGet(vm, 1);
    recordCursor = gBattleRecordCursor;
    recordCursor[1] = cameraArg;
    gBattleRecordCursor = battleRecordAdvance(recordCursor);
    return 1;
}

extern s32 gBattleRecordPayloadWords[];

f32 *battleRecordAdvance(f32 *recordCursor) {
    f32 recordTypeId;

    recordTypeId = recordCursor[0];
    if (!(!(recordTypeId <= 0.0f) && !(recordTypeId >= 10.0f))) {
        return recordCursor;
    }
    return recordCursor + gBattleRecordPayloadWords[(s32) recordTypeId] + 1;
}

float func_0030A4E0(double value);
double func_00309EE0(double lhs, double rhs);

/* "Pos" native of the bcam table: appends a type 4 record with four values, each scaled by 0.01 and then converted to float. */
s32 bcamPos(s32 vm) {
    f32 cameraArg;
    f32 *recordCursor;
    f32 *recordHead;
    f32 *fieldCursor;
    double percentScale;

    if (battleRecordIsFull() != 0) {
        return 1;
    }
    recordHead = gBattleRecordCursor;
    *recordHead = 4.0f;
    cameraArg = vmArgGet(vm, 1);
    percentScale = 0.01;
    fieldCursor = gBattleRecordCursor;
    fieldCursor[1] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    cameraArg = vmArgGet(vm, 2);
    fieldCursor = gBattleRecordCursor;
    fieldCursor[2] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    cameraArg = vmArgGet(vm, 3);
    fieldCursor = gBattleRecordCursor;
    fieldCursor[3] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    cameraArg = vmArgGet(vm, 4);
    recordCursor = gBattleRecordCursor;
    recordCursor[4] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    gBattleRecordCursor = battleRecordAdvance(recordCursor);
    return 1;
}

/* "Look" native of the bcam table: appends a type 3 record with four values, each scaled by 0.01 and then converted to float. */
s32 bcamLook(s32 vm) {
    f32 cameraArg;
    f32 *recordCursor;
    f32 *recordHead;
    f32 *fieldCursor;
    double percentScale;

    if (battleRecordIsFull() != 0) {
        return 1;
    }
    recordHead = gBattleRecordCursor;
    *recordHead = 3.0f;
    cameraArg = vmArgGet(vm, 1);
    percentScale = 0.01;
    fieldCursor = gBattleRecordCursor;
    fieldCursor[1] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    cameraArg = vmArgGet(vm, 2);
    fieldCursor = gBattleRecordCursor;
    fieldCursor[2] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    cameraArg = vmArgGet(vm, 3);
    fieldCursor = gBattleRecordCursor;
    fieldCursor[3] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    cameraArg = vmArgGet(vm, 4);
    recordCursor = gBattleRecordCursor;
    recordCursor[4] = func_0030A4E0(func_00309EE0(cameraArg, percentScale));
    gBattleRecordCursor = battleRecordAdvance(recordCursor);
    return 1;
}

/* "Cvr" native of the bcam table: appends a type 5 record with two values. */
s32 bcamCvr(s32 vm) {
    f32 *recordCursor;
    f32 cameraArg;

    if (battleRecordIsFull() != 0) {
        return 1;
    }
    *gBattleRecordCursor = 5.0f;
    gBattleRecordCursor[1] = vmArgGet(vm, 1);
    cameraArg = vmArgGet(vm, 2);
    recordCursor = gBattleRecordCursor;
    recordCursor[2] = cameraArg;
    gBattleRecordCursor = battleRecordAdvance(recordCursor);
    return 1;
}

/* "RotZ" native of the bcam table: appends a type 6 record whose one value is a percentage turned into radians (x 0.01, x pi/180). */
s32 bcamRotZ(s32 vm) {
    f32 cameraArg;
    f32 *recordCursor;
    f32 *recordHead;

    if (battleRecordIsFull() != 0) {
        return 1;
    }
    recordHead = gBattleRecordCursor;
    *recordHead = 6.0f;
    cameraArg = vmArgGet(vm, 1);
    recordCursor = gBattleRecordCursor;
    recordCursor[1] = func_0030A4E0(func_00309EE0(func_00309EE0(cameraArg, 0.01), 0.017453279346227646));
    gBattleRecordCursor = battleRecordAdvance(recordCursor);
    return 1;
}

f32 *battleRecordAdvance(f32 *record);

/* "Wait" native of the bcam table: appends a type 1 record with one value while the buffer has room. */
s32 bcamWait(s32 vm) {
    f32 cameraArg;
    f32 *recordCursor;

    if (battleRecordIsFull() == 0) {
        *gBattleRecordCursor = 1.0f;
        cameraArg = vmArgGet(vm, 1);
        recordCursor = gBattleRecordCursor;
        recordCursor[1] = cameraArg;
        gBattleRecordCursor = battleRecordAdvance(recordCursor);
    }
    /* kept: the do-while(0) decides the order of the saved-register reloads. */
    do { return 1; } while (0);
}

s32 textSetScaleRecord(s32 kind, f32 a, f32 b, f32 c, f32 d, f32 e);

/* "Over" native of the bcam table: five values, each a scale converted to float (the first two also to radians), and the mode from argument 6, passed to textSetScaleRecord. */
s32 bcamOver(s32 vm) {
    f32 scaleA;
    f32 scaleB;
    f32 scaleC;
    f32 scaleD;
    f32 scaleE;

    scaleA = func_0030A4E0(func_00309EE0(func_00309EE0((f32) vmArgGet(vm, 1), 0.01), 0.017453279346227646));
    scaleB = func_0030A4E0(func_00309EE0(func_00309EE0((f32) vmArgGet(vm, 2), 0.01), 0.017453279346227646));
    scaleC = func_0030A4E0(func_00309EE0((f32) vmArgGet(vm, 3), 0.01));
    scaleD = func_0030A4E0(func_00309EE0((f32) vmArgGet(vm, 4), 0.01));
    scaleE = func_0030A4E0(func_00309EE0((f32) vmArgGet(vm, 5), 0.01));
    textSetScaleRecord(vmArgGet(vm, 6), scaleA, scaleB, scaleC, scaleD, scaleE);
    return 1;
}

s32 textEndScene();

s32 bcamOvercam(void) {
    textEndScene();
    return 1;
}

void actorSlotSetRadius(s32 index, f32 value);

/* "Def" native of the bcam table: stores one scaled value (x 0.01) in slot 0..7 through actorSlotSetRadius. */
s32 bcamDef(s32 vm) {
    s32 bcamSlot;
    f32 scaledValue;

    bcamSlot = vmArgGet(vm, 1);
    if (bcamSlot < 0) {
        return 0;
    }
    if (bcamSlot >= 8) {
        return 0;
    }
    scaledValue = vmArgGet(vm, 2);
    scaledValue = func_0030A4E0(func_00309EE0(scaledValue, 0.01));
    actorSlotSetRadius(bcamSlot, scaledValue);
    return 1;
}

unsigned char gBattleRecordBuffer[384] = { 0 };

void *gBattleRecordBufferBaseStorage[2] = { gBattleRecordBuffer, (void *) D_00530A40 };

void *gBattleTextCandidateIds[2] = { gBattleRecordBufferBaseStorage, 0 };

s32 gBattleRecordPayloadWords[8] = {
    0, 1, 1, 4, 4, 2, 1, 0
};
