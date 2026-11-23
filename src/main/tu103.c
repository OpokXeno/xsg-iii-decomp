#include "common.h"

typedef int s32;

typedef struct RgDispModelHead {
    unsigned char pad_00[0x24];
    s32 actorId;
} RgDispModelHead;

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D05B8);

extern s32 gShapeAngleSecondTable[];

s32 wordTableGetEntry(s32 entryNumber) {
    return gShapeAngleSecondTable[entryNumber];

}

RgDispModelHead *RgDispModelFindByKey();

s32 rgDispModelGetActorId(void) {
    RgDispModelHead *dispModel;
    s32 actorId;

    dispModel = RgDispModelFindByKey();
    actorId = 0;
    if (dispModel != 0) {
        actorId = dispModel->actorId;
    }
    return actorId;
}

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D0660);

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D06D0);

INCLUDE_ASM("asm/main/nonmatchings/tu103", func_001D0760);
