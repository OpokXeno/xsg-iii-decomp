/*
 * OV02 original TU 89: 0x00ab1310..0x00ab1918 (15 functions)
 */
#include "common.h"

typedef signed char s8;
typedef int s32;
extern const char gDispModelPoolFailMessage[];
extern const char gDispModelSourceFile[];
/* Command record that xcBlockClear zeroes (0x18 bytes); the fields used here are 0x00..0x10. */
typedef struct DispModelBinding {
    s32 bindStatus;                                     /* 0x00 */
    s8 *name;                                       /* 0x04 */
    s32 dispModelHandle;                                     /* 0x08 */
    s32 unk_0C;                                     /* 0x0C */
    s32 unk_10;                                     /* 0x10 */
} DispModelBinding;
void assertFailed(const char *message, const char *file, int line);
s32 poolBlockTagWordSet();
void xcBlockClear(void *block);
s32 func_001B7190();
int RgDispModelFindByName(const char *name);

void RgDispModelCreateIfMissing(s8 *name) {
    DispModelBinding binding;
    s32 dispModelHandle;

    if (RgDispModelFindByName(name) == 0) {
        xcBlockClear(&binding);
        binding.bindStatus = 0;
        binding.name = name;
        binding.dispModelHandle = 0;
        binding.unk_0C = 0;
        binding.unk_10 = 0;
        dispModelHandle = func_001B7190(&binding.bindStatus);
        if (poolBlockTagWordSet(dispModelHandle, 0, name) == 0) {
            assertFailed(gDispModelPoolFailMessage, gDispModelSourceFile, 0x46);
        }
    }
}

extern const char gDispModelNotFoundMessage[];
s32 func_001B70F0();
void RgDispModelBindRecord(s8 *name, s32 dispModelHandle) {
    DispModelBinding binding;

    if (dispModelHandle == 0) {
        assertFailed(gDispModelNotFoundMessage, gDispModelSourceFile, 0x4F);
    }
    xcBlockClear(&binding);
    binding.bindStatus = 0;
    binding.name = name;
    binding.dispModelHandle = dispModelHandle;
    binding.unk_0C = 0;
    binding.unk_10 = 0;
    func_001B70F0(&binding.bindStatus);
    if (poolBlockTagWordSet(dispModelHandle, 0, name) == 0) {
        assertFailed(gDispModelPoolFailMessage, gDispModelSourceFile, 0x5A);
    }
}

s32 func_001B6918(s32 unit, s32 channel, const char *command, s32 timeout);
void RgDispModelBindRecord();

void RgDispModelBindNamed(s8 *name, s8 *modelName) {
    s32 dispModelHandle;
    dispModelHandle = func_001B6918(0, 1, modelName, -1);
    RgDispModelBindRecord(name, dispModelHandle);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1470);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1570);

int RgDispModelFindByName(const char *name) {
    return func_001B6918(0, 0, name, -1);
}

void assertFailed(const char *message, const char *file, int line);
int RgDispModelFindByName(const char *name);
void RgDispModelSetLocal(int dispModel);
extern const char gDispModelSourceFile[];
extern const char gDispModelNotFoundMessage[];

void RgDispModelSetLocalByName(const char *name) {
    int dispModel;

    dispModel = RgDispModelFindByName(name);
    if (dispModel == 0) {
        assertFailed(gDispModelNotFoundMessage, gDispModelSourceFile, 178);
    }
    RgDispModelSetLocal(dispModel);
}

s32 func_001B60B8(s32);

s32 RgDispModelIsBusy(void) {
    return func_001B60B8(3) != 0;
}

s32 RgDispModelIsBusy();
void xglSleep();

void RgDispModelWaitIdle(void) {
    while (RgDispModelIsBusy() != 0) {
        xglSleep();
    }
}

void func_00A84998();

void RgDispModelFlush(void) {
    func_00A84998();
}

extern s32 gStringTableNullEntry;
s32 *stringTableGet();

s32 *stringTableEntryGet(s32 stringIndex) {
    if (stringIndex < 0) {
        return &gStringTableNullEntry;
    }
    return stringTableGet();
}

extern s32 gBattleResultBuffer;
s32 memset(s32 *, s32, s32);

void battleResultReset(void) {
    memset(&gBattleResultBuffer, 0, 0x50);
}

s32 srsStartResourceLoad();

void battleResultActivate(s32 battleResultSlot) {
    srsStartResourceLoad(battleResultSlot, 1);
}

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB17B0);

INCLUDE_ASM("asm/nonmatchings/ov02/tu089", func_00AB1880);
