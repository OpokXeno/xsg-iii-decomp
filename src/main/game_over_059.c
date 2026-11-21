#include "common.h"
#include "shared.h"

typedef int s32;
extern unsigned char D_0031A560[];
extern unsigned char gGameOverSourceRecord[];
void func_0030C130(void *source, void *destination, s32 size);

void gameOverCopyRecord(void) {
    func_0030C130(gGameOverSourceRecord, D_0031A560, 0x44);
}

INCLUDE_ASM("asm/main/nonmatchings/game_over_059", func_001B5B78);

INCLUDE_ASM("asm/main/nonmatchings/game_over_059", func_001B5C38);

INCLUDE_ASM("asm/main/nonmatchings/game_over_059", func_001B5C68);

extern s32 gGameOverSlotTable[];

void *gameOverSlotAt(s32 slotNumber) {
    return &gGameOverSlotTable[slotNumber - 1];
}

INCLUDE_ASM("asm/main/nonmatchings/game_over_059", func_001B5D30);

INCLUDE_ASM("asm/main/nonmatchings/game_over_059", func_001B5D70);

void func_00265DC8(void);
void func_00265E68(void);

void gameOverStepPair(void) {
    func_00265DC8();
    func_00265E68();
}
