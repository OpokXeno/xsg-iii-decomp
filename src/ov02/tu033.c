/*
 * OV02 original TU 33: 0x00a8d018..0x00a8d0f8 (3 functions)
 */
#include "common.h"

struct SbThread;
int vmArgGet(struct SbThread *thread, int argIndex);
int vmKeyedEntryGet(struct SbThread *thread, int key);
void ov02SetEventFlag(int eventFlagId);

int sbNativeFlagSetByKey(struct SbThread *call)
{
    ov02SetEventFlag(vmKeyedEntryGet(call, vmArgGet(call, 1)));
    return 1;
}

typedef int s32;
s32 ov02ResetEventFlagsNow();

s32 sbNativeFlagReset(void) {
    ov02ResetEventFlagsNow();
    return 1;
}

s32 func_002C3590(s32, s32, s32);
s32 seVoiceEnable();

s32 sbNativeSeVoicePlay(struct SbThread *thread) {
    s32 secondArg;
    s32 voiceId;
    int entryKey;
    int keyedEntry;

    voiceId = vmArgGet(thread, 1);
    secondArg = vmArgGet(thread, 2);
    entryKey = vmArgGet(thread, 3);
    keyedEntry = vmKeyedEntryGet(thread, entryKey);
    func_002C3590(voiceId, secondArg, keyedEntry);
    seVoiceEnable();
    return 1;
}
