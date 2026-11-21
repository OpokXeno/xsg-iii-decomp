#include "common.h"

typedef int s32;

typedef struct RgDispModelHead {
    unsigned char pad_00[0x24];
    s32 actorId;
} RgDispModelHead;

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D05B8);

extern s32 D_00532FC0[];

s32 wordTableGetEntry(s32 entryNumber) {
    return D_00532FC0[entryNumber];

}

RgDispModelHead *func_001C0570();

s32 rgDispModelGetActorId(void) {
    RgDispModelHead *dispModel;
    s32 actorId;

    dispModel = func_001C0570();
    actorId = 0;
    if (dispModel != 0) {
        actorId = dispModel->actorId;
    }
    return actorId;
}

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D0660);

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D06D0);

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D0760);
