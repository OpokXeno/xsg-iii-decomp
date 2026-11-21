#include "common.h"

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002AD598);

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002AD640);

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002ADAC0);

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002ADD48);

typedef int s32;
typedef float f32;
typedef unsigned char u8;

typedef struct SlotLevelEntry {
    f32 slotLevel;
    u8 unk_04[0x4];
} SlotLevelEntry;

extern f32 gSlotLevelMinimum[];
extern SlotLevelEntry gSlotLevelTable[];

void entryStoreClamped(s32 slotNumber, f32 requestedLevel) {
    SlotLevelEntry *slotLevelEntry;
    f32 minimum;
    f32 valueF;

    valueF = requestedLevel;
    minimum = gSlotLevelMinimum[0];
    slotLevelEntry = &gSlotLevelTable[slotNumber];
    slotLevelEntry->slotLevel = valueF;
    if (valueF < minimum) {
        slotLevelEntry->slotLevel = minimum;
    }
}

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002AE030);

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002AE350);

INCLUDE_ASM("asm/main/nonmatchings/tu359", func_002AE530);

extern f32 gWavePlaneOffset[];
s32 slotPairActive();
s32 sineWaveValue16();
void njPlaneInit();
void njPlaneSetOffset(s32 *, f32, f32);
void njPlaneSetLevel(s32 *, f32);
void njPlaneSetMode();
void func_002CFFF0();

void wavePlaneDraw(void) {
    s32 wavePlane[0x30];
    s32 sineValue;

    if (slotPairActive() != 0) {
        njPlaneInit(&wavePlane[0]);
        njPlaneSetOffset(&wavePlane[0], gWavePlaneOffset[0], gWavePlaneOffset[1]);
        njPlaneSetMode(&wavePlane[0], 2);
        sineValue = sineWaveValue16();
        njPlaneSetLevel(&wavePlane[0], 0x80 - (sineValue * 2));
        func_002CFFF0(&wavePlane[0], 0x15, 0);
    }
}

s32 func_002BB450(s32);
s32 effectPoolSetTails(s32);

void slotValueSetEnabled(s32 actorHandle, s32 level) {
    effectPoolSetTails(level);
    func_002BB450(actorHandle);
}
