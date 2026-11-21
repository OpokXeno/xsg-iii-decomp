#include "common.h"

typedef int s32;
s32 VM_PLUGIN_CNT_GET();
s32 func_00245308();
s32 xglCdAddReadyBits();
s32 sbSlotJumpRequest();
s32 vmArgGet();
s32 vmArgTagGet();
s32 vmKeyedEntryGet();

s32 sbNativeJumpToKeyedEntry(s32 vm) {
    s32 keyedEntryKey;
    s32 keyedEntryText;

    if (VM_PLUGIN_CNT_GET(vm) >= 2) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    if (!((vmArgTagGet(vm, 1) & 0xFF) == 2)) {
        return 0;
    }
    keyedEntryKey = vmArgGet(vm, 1);
    keyedEntryText = vmKeyedEntryGet(vm, keyedEntryKey);
    xglCdAddReadyBits(1);
    func_00245308(keyedEntryText);
    return 1;
}

s32 sbNativeAlwaysSucceed(void) {
    return 1;
}

s32 func_001B3858();

s32 sbNativeCallKeyedEntry(s32 vm) {
    s32 keyedEntryKey;
    s32 keyedEntryText;

    if (VM_PLUGIN_CNT_GET(vm) >= 2) {
        sbSlotJumpRequest(vm, 2);
        return 0;
    }
    if (!((vmArgTagGet(vm, 1) & 0xFF) == 2)) {
        return 0;
    }
    keyedEntryKey = vmArgGet(vm, 1);
    keyedEntryText = vmKeyedEntryGet(vm, keyedEntryKey);
    func_001B3858(keyedEntryText);
    return 1;
}
