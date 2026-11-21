#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C7F68);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8068);

typedef int s32;
typedef float f32;

typedef struct GameStateParts {
    unsigned char pad_0000[0x830];
    s32 partsSelf;
} GameStateParts;

s32 sbNativeReturnZero(void) {
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8100);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8268);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C85C8);

s32 VM_PLUGIN_CNT_GET();
void xenoActorLightSlotClear(s32 index);
void sbSlotJumpRequest(s32 thread, s32 key);
s32 vmArgGet(s32 thread, s32 index);

s32 sbNativeLightClear(s32 thread) {
    s32 lightSlot;
    if (VM_PLUGIN_CNT_GET() != 1) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    lightSlot = vmArgGet(thread, 1);
    xenoActorLightSlotClear(lightSlot);
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C87B0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8850);

GameStateParts *stateBlockGet();
s32 xrgObjectSetPartsTime(s32 self, s32 partNumber, f32 fraction);

s32 sbNativeSetPartsFirstFraction(s32 thread) {
    GameStateParts *gameState;
    s32 partNumber;
    s32 percent;

    gameState = stateBlockGet();
    if (gameState != 0) {
        if (VM_PLUGIN_CNT_GET(thread) != 2) {
            sbSlotJumpRequest(thread, 2);
            return 0;
        }
        partNumber = vmArgGet(thread, 1);
        percent = vmArgGet(thread, 2);
        xrgObjectSetPartsTime(gameState->partsSelf, partNumber, percent / 100.0f);
        return 1;
    }
    return 0;
}

s32 xrgObjectSetPartsWeight(s32 self, s32 partNumber, f32 fraction);

s32 sbNativeSetPartsSecondFraction(s32 thread) {
    GameStateParts *gameState;
    s32 partNumber;
    s32 percent;

    gameState = stateBlockGet();
    if (gameState != 0) {
        if (VM_PLUGIN_CNT_GET(thread) != 2) {
            sbSlotJumpRequest(thread, 2);
            return 0;
        }
        partNumber = vmArgGet(thread, 1);
        percent = vmArgGet(thread, 2);
        xrgObjectSetPartsWeight(gameState->partsSelf, partNumber, percent / 100.0f);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8AF8);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8BC0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8CD0);

s32 sbNativeReturnOne(void) {
    return 1;
}

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8DC0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8E68);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C8F00);

s32 MMathMakeRandomInt();

s32 sbNativeRandomRange(s32 thread) {
    s32 spanLength;
    s32 randomLow;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    randomLow = vmArgGet(thread, 1);
    spanLength = vmArgGet(thread, 2) - randomLow;
    return randomLow + (MMathMakeRandomInt() % (spanLength + 1));
}

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9020);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C91F0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9370);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C94F0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C95D0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C96C8);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9778);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9850);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C98D8);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C99F8);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9D28);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9E50);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001C9F68);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA0D8);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA260);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA3E8);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA560);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA618);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA758);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA7F0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CA880);

s32 *func_001C0570(s32 dispModelIndex);
s32 func_001C1320(s32 *dispModel, s32 mode);

s32 sbNativeEntrySetFlag(s32 thread) {
    s32 *dispModel;
    s32 dispModelIndex;

    if (VM_PLUGIN_CNT_GET() != 2) {
        sbSlotJumpRequest(thread, 2);
        return 0;
    }
    dispModelIndex = vmArgGet(thread, 1);
    dispModel = func_001C0570(dispModelIndex);
    if (dispModel != 0) {
        if (vmArgGet(thread, 2) != 0) {
            return 0;
        }
        *dispModel = (*dispModel & 0xFF7FFFFF) | 0x800;
        func_001C1320(dispModel, 1);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CAA48);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CAAC0);

INCLUDE_ASM("asm/main/nonmatchings/tu086", func_001CAC00);
