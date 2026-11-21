#include "common.h"
#include "shared.h"

struct SbThread;

int bgmIndexGet(void);
extern int bgmIndex;

typedef int s32;
s32 func_00A8D928(s32 index);

s32 sbNativeIndexForward(struct SbThread *call) {
    int forwardedIndex;
    forwardedIndex = vmArgGet(call, 1);
    func_00A8D928(forwardedIndex);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8D928);

INCLUDE_ASM("asm/nonmatchings/ov02/sb_natives_unit_037", func_00A8DA50);

int vmArgGet(struct SbThread *thread, int argIndex);

int bgmPlay(struct SbThread *call)
{
    bgmIndex = vmArgGet(call, 1);
    return 1;
}

/* Returns the background music index that bgmPlay stored. */
int bgmIndexGet(void) {
    return bgmIndex;
}

void func_0018FAC8(int value);

int sbNativeValueForward(struct SbThread *call)
{
    func_0018FAC8(vmArgGet(call, 1));
    return 1;
}

s32 soundSlotStartNumbered(s32 slot, s32 entry);
s32 soundSlotSetVolume(s32 slot, s32 volume, s32 mode);

s32 sbNativeSoundSlot0Play(struct SbThread *call) {
    int entryKey;
    int soundEntry;
    entryKey = vmArgGet(call, 1);
    soundEntry = vmKeyedEntryGet(call, entryKey);
    soundSlotStartNumbered(0, soundEntry);
    soundSlotSetVolume(0, 0x7F, 0xF);
    return 1;
}

s32 sbNativeSoundSlot1Play(struct SbThread *call) {
    int entryKey;
    int soundEntry;
    entryKey = vmArgGet(call, 1);
    soundEntry = vmKeyedEntryGet(call, entryKey);
    soundSlotStartNumbered(1, soundEntry);
    soundSlotSetVolume(1, 0x7F, 0);
    return 1;
}

s32 soundSlotStatus(s32 slot);

s32 sbNativeSoundSlot1Wait(s32 unused_arg0, s32 *done) {
    if ((soundSlotStatus(1) - 5) >= 2U) {
        *done = 1;
    }
    return 1;
}

void sceneMaskSet(int value);
int vmKeyedEntryGet(struct SbThread *thread, int value);
void func_00A9DFB0(int a, int b, int *c, int d);
extern int gRunEffEntry;

int runEffSet(struct SbThread *call)
{
    sceneMaskSet(0);
    gRunEffEntry = vmKeyedEntryGet(call, vmArgGet(call, 1));
    func_00A9DFB0(0, 0, &gRunEffEntry, 0);
    return 1;
}

s32 func_00A9DA28();
s32 func_00A9E4D8();

s32 runEffWait(s32 unused_arg0, s32 *done) {
    func_00A9DA28();
    if (func_00A9E4D8() == 0) {
        *done = 1;
    } else {
        sceneMaskSet(1);
    }
    return 1;
}

#include "main/xenoActor.h"
#include "main/xgl_2.h"

struct BattleUnitPartial {
    unsigned char unmodeled_00[0x2b4];
    struct XenoActor *actor;
};


struct BattleUnitPartial *unitTblEntryGet(int index);

int batScale(struct SbThread *call)
{
    struct BattleUnitPartial *battleUnit;
    float scale;

    battleUnit = unitTblEntryGet(vmArgGet(call, 1));
    scale = vmArgGet(call, 2) / 100.0f;
    ((Vector4Elements *)&battleUnit->actor->scale)->component[0] = scale;
    ((Vector4Elements *)&battleUnit->actor->scale)->component[1] = scale;
    ((Vector4Elements *)&battleUnit->actor->scale)->component[2] = scale;
    return 1;
}
