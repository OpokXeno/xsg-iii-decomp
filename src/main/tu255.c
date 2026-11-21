#include "common.h"

typedef int s32;
struct SbThread;
s32 vmArgGet(struct SbThread *thread, s32 argIndex);
void cameraSlotSetState(s32 slot, s32 value);

s32 scriptCameraSetSlot(struct SbThread *vm) {
    s32 cameraSlotIndex;
    s32 cameraSlotState;

    cameraSlotIndex = vmArgGet(vm, 1);
    cameraSlotState = vmArgGet(vm, 2);
    cameraSlotSetState(cameraSlotIndex, cameraSlotState);
    return 1;
}

void cameraSlotSetStateById(s32 kind, s32 id, s32 slot, s32 value);

s32 scriptCameraSetSlotById(struct SbThread *vm) {
    s32 cameraEntryKind;
    s32 cameraEntryId;
    s32 cameraSlotIndex;
    s32 cameraSlotState;

    cameraEntryKind = vmArgGet(vm, 1) - 1;
    cameraEntryId = vmArgGet(vm, 2) - 1;
    cameraSlotIndex = vmArgGet(vm, 3);
    cameraSlotState = vmArgGet(vm, 4);
    cameraSlotSetStateById(cameraEntryKind, cameraEntryId, cameraSlotIndex, cameraSlotState);
    return 1;
}

void cameraScriptActivate(s32 index, s32 mode);

s32 scriptObjectActivate(struct SbThread *vm) {
    s32 cameraEntryIndex;
    s32 activeObjectMode;

    cameraEntryIndex = vmArgGet(vm, 1) - 1;
    activeObjectMode = vmArgGet(vm, 2);
    cameraScriptActivate(cameraEntryIndex, activeObjectMode);
    return 1;
}

void cameraScriptClear(void);

s32 scriptObjectClear(void) {
    cameraScriptClear();
    return 1;
}

void objectSetScriptIndex(s32 index);

s32 scriptObjectSetIndex(struct SbThread *vm) {
    s32 activeObjectSlot;

    activeObjectSlot = vmArgGet(vm, 1);
    objectSetScriptIndex(activeObjectSlot - 1);
    return 1;
}

void objectSetScriptCount(s32 count);

s32 scriptObjectSetCount(struct SbThread *vm) {
    s32 value;

    value = vmArgGet(vm, 1);
    objectSetScriptCount(value);
    return 1;
}
