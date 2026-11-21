#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BAC8);

typedef int s32;
void movieSlotsSetForOwner(s32 owner, s32 blendResetMode);
s32 vmArgGet(void *thread, s32 argIndex);

s32 sbNativeStopOwnedEntries(void *thread) {
    s32 ownerId;

    ownerId = vmArgGet(thread, 1);
    movieSlotsSetForOwner(ownerId, 0);
    return 1;
}

s32 func_0026C9A0(s32 ownerId);

s32 sbNativeQueryOwnedEntry(void *thread, s32 *outOwnedEntryFound) {
    s32 ownedEntryFound;
    s32 ownerId;

    ownerId = vmArgGet(thread, 1);
    ownedEntryFound = func_0026C9A0(ownerId);
    if (ownedEntryFound == 1) {
        *outOwnedEntryFound = ownedEntryFound;
    }
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BC50);

INCLUDE_ASM("asm/main/nonmatchings/tu282", func_0026BCD8);
